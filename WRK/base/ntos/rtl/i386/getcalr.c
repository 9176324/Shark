/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    getcalr.c

Abstract:

    This module contains routines to get runtime stack traces 
    for the x86 architecture.

--*/

#include <ntos.h>
#include <ntrtl.h>
#include "ntrtlp.h"
#include <nturtl.h>
#include <zwapi.h>
#include <stktrace.h>

//
// Forward declarations.
//

BOOLEAN
RtlpCaptureStackLimits (
    ULONG_PTR HintAddress,
    PULONG_PTR StartStack,
    PULONG_PTR EndStack
    );

BOOLEAN
RtlpStkIsPointerInDllRange (
    ULONG_PTR Value
    );

BOOLEAN
RtlpStkIsPointerInNtdllRange (
    ULONG_PTR Value
    );

VOID
RtlpCaptureContext (
    OUT PCONTEXT ContextRecord
    );

BOOLEAN
NtdllOkayToLockRoutine(
    IN PVOID Lock
    );

ULONG
RtlpWalkFrameChainExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord
    );

//
// Fuzzy stack traces
//

ULONG
RtlpWalkFrameChainFuzzy (
    OUT PVOID *Callers,
    IN ULONG Count
    );


/////////////////////////////////////////////////////////////////////
//////////////////////////////////////////// RtlCaptureStackBackTrace
/////////////////////////////////////////////////////////////////////

USHORT
RtlCaptureStackBackTrace(
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash
    )
/*++

Routine Description:

    This routine walks up the stack frames, capturing the return address from
    each frame requested.

Arguments:

    FramesToSkip - frames detected but not included in the stack trace

    FramesToCapture - frames to be captured in the stack trace buffer.
        One of the frames will be for RtlCaptureStackBackTrace.

    BackTrace - stack trace buffer

    BackTraceHash - very simple hash value that can be used to organize
      hash tables. It is just an arithmetic sum of the pointers in the
      stack trace buffer. If NULL then no hash value is computed.

Return Value:

     Number of return addresses returned in the stack trace buffer.

--*/
{
    PVOID Trace [2 * MAX_STACK_DEPTH];
    ULONG FramesFound;
    ULONG HashValue;
    ULONG Index;

    //
    // One more frame to skip for the "capture" function (RtlWalkFrameChain).
    //

    FramesToSkip += 1;

    //
    // Sanity checks.
    //

    if (FramesToCapture + FramesToSkip >= 2 * MAX_STACK_DEPTH) {
        return 0;
    }

    FramesFound = RtlWalkFrameChain (Trace,
                                     FramesToCapture + FramesToSkip,
                                     0);

    if (FramesFound <= FramesToSkip) {
        return 0;
    }

    Index = 0;

    for (HashValue = 0; Index < FramesToCapture; Index++) {

        if (FramesToSkip + Index >= FramesFound) {
            break;
        }

        BackTrace[Index] = Trace[FramesToSkip + Index];
        HashValue += PtrToUlong(BackTrace[Index]);
    }

    if (BackTraceHash != NULL) {

        *BackTraceHash = HashValue;
    }

    //
    // Zero the temporary buffer used to get the stack trace
    // so that we do not leave garbage on the stack. This will
    // improve debugging when we need to look manually at a stack.
    //
    // N.B. We cannot use here a simple call to RtlZeroMemory because the
    //      compiler will optimize away that call since it is performed 
    //      for a buffer that gets out of scope. 
    //
    
    {
        volatile PVOID * Pointer = (volatile PVOID *)Trace;
        SIZE_T Count = sizeof(Trace) / sizeof (PVOID);

        while (Count > 0) {

            *Pointer = NULL;

            Pointer += 1;
            Count -= 1;
        }
    }

    return (USHORT)Index;
}



/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// RtlWalkFrameChain
/////////////////////////////////////////////////////////////////////

#define SIZE_1_KB  ((ULONG_PTR) 0x400)
#define SIZE_1_GB  ((ULONG_PTR) 0x40000000)

#define PAGE_START(address) (((ULONG_PTR)address) & ~((ULONG_PTR)PAGE_SIZE - 1))

#if FPO
#pragma optimize( "y", off ) // disable FPO
#endif

ULONG
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    )

/*++

Routine Description:

    This function tries to walk the EBP chain and fill out a vector of
    return addresses. It is possible that the function cannot fill the
    requested number of callers. In this case the function will just return
    with a smaller stack trace. In kernel mode the function should not take
    any exceptions (page faults) because it can be called at all sorts of
    irql levels.

    The `Flags' parameter is used for future extensions. A zero value will be
    compatible with new stack walking algorithms.
    
    A value of 1 for `Flags' means we are running in K-mode and we want to get
    the user mode stack trace.

Return value:

    The number of identified return addresses on the stack. This can be less
    then the Count requested.

--*/

{

    ULONG_PTR Fp, NewFp, ReturnAddress;
    ULONG Index;
    ULONG_PTR StackEnd, StackStart;
    BOOLEAN Result;
    BOOLEAN InvalidFpValue;

    //
    // Get the current EBP pointer which is supposed to
    // be the start of the EBP chain.
    //

    _asm mov Fp, EBP;

    StackStart = Fp;
    InvalidFpValue = FALSE;

    if (Flags == 0) {
        if (! RtlpCaptureStackLimits (Fp, &StackStart, &StackEnd)) {
            return 0;
        }
    }


    try {

        //
        // If we need to get the user mode stack trace from kernel mode
        // figure out the proper limits.
        //

        if (Flags == 1) {

            PKTHREAD Thread = KeGetCurrentThread ();
            PTEB Teb;
            PKTRAP_FRAME TrapFrame;
            ULONG_PTR Esp;

            TrapFrame = Thread->TrapFrame;
            Teb = Thread->Teb;

            //
            // If this is a system thread, it has no Teb and no kernel mode
            // stack, so check for it so we don't dereference NULL.
            //
            // If there is no trap frame (probably an APC), or it's attached,
            // or the irql is greater than dispatch, this code can't log a
            // stack.
            //

            if (Teb == NULL || 
                IS_SYSTEM_ADDRESS((PVOID)TrapFrame) == FALSE || 
                (PVOID)TrapFrame <= Thread->StackLimit ||
                (PVOID)TrapFrame >= Thread->StackBase ||
                KeIsAttachedProcess() || 
                (KeGetCurrentIrql() >= DISPATCH_LEVEL)) {

                return 0;
            }

            StackStart = (ULONG_PTR)(Teb->NtTib.StackLimit);
            StackEnd = (ULONG_PTR)(Teb->NtTib.StackBase);
            Fp = (ULONG_PTR)(TrapFrame->Ebp);

            if (StackEnd <= StackStart) {
                return 0;
            }
            
            ProbeForRead (StackStart, StackEnd - StackStart, sizeof (UCHAR));
        }
        
        for (Index = 0; Index < Count; Index += 1) {

            if (Fp >= StackEnd || 
                ( (Index == 0)?
                      (Fp < StackStart):
                      (Fp <= StackStart) ) ||
                StackEnd - Fp < sizeof(ULONG_PTR) * 2) {
                break;
            }

            NewFp = *((PULONG_PTR)(Fp + 0));
            ReturnAddress = *((PULONG_PTR)(Fp + sizeof(ULONG_PTR)));

            //
            // Figure out if the new frame pointer is ok. This validation
            // should avoid all exceptions in kernel mode because we always
            // read within the current thread's stack and the stack is
            // guaranteed to be in memory (no page faults). It is also guaranteed
            // that we do not take random exceptions in user mode because we always
            // keep the frame pointer within stack limits.
            //

            if (! (Fp < NewFp && NewFp < StackEnd)) {

                InvalidFpValue = TRUE;
            }

            //
            // Figure out if the return address is ok. If return address
            // is a stack address or <64k then something is wrong. There is
            // no reason to return garbage to the caller therefore we stop.
            //

            if (StackStart < ReturnAddress && ReturnAddress < StackEnd) {
                break;
            }

            if (Flags == 0 && IS_SYSTEM_ADDRESS((PVOID)ReturnAddress) == FALSE) {
                break;
            }

            //
            // Store new fp and return address and move on.
            // If the new FP value is bogus but the return address
            // looks ok then we still save the address.
            //

            Callers[Index] = (PVOID)ReturnAddress;
            
            if (InvalidFpValue) {

                Index += 1;
                break;
            }
            else {

                Fp = NewFp;
            }
        }
    }
    except (RtlpWalkFrameChainExceptionFilter (_exception_code(), _exception_info())) {

        Index = 0;
    }

    //
    // Return the number of return addresses identified on the stack.
    //

    return Index;

}


#if FPO
#pragma optimize( "y", off ) // disable FPO
#endif


/////////////////////////////////////////////////////////////////////
////////////////////////////////////////////// RtlCaptureStackContext
/////////////////////////////////////////////////////////////////////

#if FPO
#pragma optimize( "y", off ) // disable FPO
#endif

ULONG
RtlCaptureStackContext (
    OUT PULONG_PTR Callers,
    OUT PRTL_STACK_CONTEXT Context,
    IN ULONG Limit
    )
/*++

Routine Description:

    This routine will detect up to `Limit' potential callers from the stack.

    A potential caller is a pointer (PVOID) that points into one of the
    regions occupied by modules loaded into the process space (user mode -- dlls)
    or kernel space (kernel mode -- drivers).

    Note. Based on experiments you need to save at least 64 pointers to be sure you
    get a complete stack.

Arguments:

    Callers - vector to be filled with potential return addresses. Its size is
        expected to be `Limit'. If it is not null then Context should be null.

    Context - if not null the caller wants the stack context to be saved here
        as opposed to the Callers parameter.

    Limit - # of pointers that can be written into Callers and Offsets.

Return value:

    The number of potential callers detected and written into the
    `Callers' buffer.

--*/
{
    ULONG_PTR Current;
    ULONG_PTR Value;
    ULONG Index;
    ULONG_PTR Offset;
    ULONG_PTR StackStart;
    ULONG_PTR StackEnd;
    ULONG_PTR Hint;
    ULONG_PTR Caller;
    ULONG_PTR ContextSize;

    //
    // Avoid weird conditions. Doing this in an ISR is never a good idea.
    //

    if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
        return 0;
    }

    if (Limit == 0) {
        return 0;
    }

    Caller = (ULONG_PTR)_ReturnAddress();

    if (Context) {
        Context->Entry[0].Data = Caller;
        ContextSize = sizeof(RTL_STACK_CONTEXT) + (Limit - 1) * sizeof (RTL_STACK_CONTEXT_ENTRY);
    }
    else {
        Callers[0] = Caller;
    }

    //
    // Get stack limits
    //

    _asm mov Hint, EBP;


    if (! RtlpCaptureStackLimits (Hint, &StackStart, &StackEnd)) {
        return 0;
    }

    //
    // Synchronize stack traverse pointer to the next word after the first
    // return address.
    //

    for (Current = StackStart; Current < StackEnd; Current += sizeof(ULONG_PTR)) {

        if (*((PULONG_PTR)Current) == Caller) {
            break;
        }
    }

    if (Context) {
        Context->Entry[0].Address = Current;
    }

    //
    // Iterate the stack and pickup potential callers on the way.
    //

    Current += sizeof(ULONG_PTR);

    Index = 1;

    for ( ; Current < StackEnd; Current += sizeof(ULONG_PTR)) {

        //
        // If potential callers buffer is full then wrap this up.
        //

        if (Index == Limit) {
            break;
        }

        //
        // Skip `Callers' buffer because it will give false positives.
        // It is very likely for this to happen because most probably the buffer
        // is allocated somewhere upper in the call chain.
        //

        if (Context) {

            if (Current >= (ULONG_PTR)Context && Current < (ULONG_PTR)Context + ContextSize ) {
                continue;
            }

        }
        else {

            if ((PULONG_PTR)Current >= Callers && (PULONG_PTR)Current < Callers + Limit ) {
                continue;
            }
        }

        Value = *((PULONG_PTR)Current);

        //
        // Skip small numbers.
        //

        if (Value <= 0x10000) {
            continue;
        }

        //
        // Skip stack pointers.
        //

        if (Value >= StackStart && Value <= StackEnd) {
            continue;
        }

        //
        // Check if `Value' points inside one of the loaded modules.
        //

        if (RtlpStkIsPointerInDllRange (Value)) {

            if (Context) {

                Context->Entry[Index].Address = Current;
                Context->Entry[Index].Data = Value;
            }
            else {

                Callers[Index] = Value;
            }

            Index += 1;
        }

    }

    if (Context) {
        Context->NumberOfEntries = Index;
    }

    return Index;
}


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////// Dll ranges bitmap
/////////////////////////////////////////////////////////////////////

//
// DLL ranges bitmap
//
// This range scheme is needed in order to capture stack contexts on x86
// machines fast. On IA64 there are totally different algorithms for getting
// stack traces.
//
// Every bit represents 1Mb of virtual space. Since we use the code either
// in user mode or kernel mode the first bit of a pointer is not interesting.
// Therefore we have to represent 2Gb / 1Mb regions. This totals 256 bytes.
//
// The bits are set only in loader code paths when a DLL (or driver) gets loaded.
// The writing is protected by the loader lock. The bits are read in stack
// capturing function.The reading does not require lock protection.
//

UCHAR RtlpStkDllRanges [2048 / 8];

BOOLEAN
RtlpStkIsPointerInDllRange (
    ULONG_PTR Value
    )
{
    ULONG Index;

    Value &= ~0x80000000;
    Index = (ULONG)(Value >> 20);

    if (RtlpStkDllRanges[Index >> 3] & (UCHAR)(1 << (Index & 7))) {

        return TRUE;
    }
    else {

        return FALSE;
    }
}

#define SIZE_1_MB 0x100000

#if defined(ROUND_DOWN)
#undef ROUND_DOWN
#endif

#if defined(ROUND_UP)
#undef ROUND_UP
#endif

#define ROUND_DOWN(a,b) ((ULONG_PTR)(a) & ~((ULONG_PTR)(b) - 1))
#define ROUND_UP(a,b) (((ULONG_PTR)(a) + ((ULONG_PTR)(b) - 1)) & ~((ULONG_PTR)(b) - 1))

VOID
RtlpStkMarkDllRange (
    PLDR_DATA_TABLE_ENTRY DllEntry
    )
/*++

Routine description:

    This routine marks the corresponding bits for the loaded dll in the
    RtlpStkDllRanges variable. This global is used within RtlpDetectDllReferences
    to save a stack context.

Arguments:

    Loader structure for a loaded dll.

Return value:

    None.

Environment:

    In user mode this function is called from loader code paths. The Peb->LoaderLock
    is always held while executing this function.

--*/
{
    PVOID Base;
    ULONG Size;
    ULONG_PTR Current;
    ULONG_PTR Start;
    ULONG_PTR End;
    ULONG Index;
    ULONG_PTR Value;

    Base = DllEntry->DllBase;
    Size = DllEntry->SizeOfImage;

    //
    // Find out where is ntdll loaded if we do not know yet.
    //

    Start = ROUND_DOWN(Base, SIZE_1_MB);
    End = ROUND_UP((ULONG_PTR)Base + Size, SIZE_1_MB);

    for (Current = Start; Current < End; Current += SIZE_1_MB) {

        Value = Current & ~0x80000000;

        Index = (ULONG)(Value >> 20);

        RtlpStkDllRanges[Index >> 3] |= (UCHAR)(1 << (Index & 7));
    }
}



BOOLEAN
RtlpCaptureStackLimits (
    ULONG_PTR HintAddress,
    PULONG_PTR StartStack,
    PULONG_PTR EndStack
    )
/*++

Routine Description:

    This routine figures out what are the stack limits for the current thread.
    This is used in other routines that need to grovel the stack for various
    information (e.g. potential return addresses).

    The function is especially tricky in K-mode where the information kept in
    the thread structure about stack limits is not always valid because the
    thread might execute a DPC routine and in this case we use a different stack
    with different limits.

Arguments:

    HintAddress - Address of a local variable or parameter of the caller of the
        function that should be the start of the stack.

    StartStack - start address of the stack (lower value).

    EndStack - end address of the stack (upper value).

Return value:

    False if some weird condition is discovered, like an End lower than a Start.

--*/
{
    //
    // Avoid weird conditions. Doing this in an ISR is never a good idea.
    //

    if (KeGetCurrentIrql() > DISPATCH_LEVEL) {
        return FALSE;
    }

    *StartStack = (ULONG_PTR)(KeGetCurrentThread()->StackLimit);
    *EndStack = (ULONG_PTR)(KeGetCurrentThread()->StackBase);

    if (*StartStack <= HintAddress && HintAddress <= *EndStack) {

        *StartStack = HintAddress;
    }
    else {

        *EndStack = (ULONG_PTR)(KeGetPcr()->Prcb->DpcStack);
        *StartStack = *EndStack - KERNEL_STACK_SIZE;

        //
        // Check if this is within the DPC stack for the current
        // processor.
        //

        if (*EndStack && *StartStack <= HintAddress && HintAddress <= *EndStack) {

            *StartStack = HintAddress;
        }
        else {

            //
            // This is not current thread's stack and is not the DPC stack
            // of the current processor. We will return just the rest of the
            // stack page containing HintAddress as a valid stack range.
            //

            *StartStack = HintAddress;

            *EndStack = (*StartStack + PAGE_SIZE) & ~((ULONG_PTR)PAGE_SIZE - 1);
        }
    }

    return TRUE;
}

