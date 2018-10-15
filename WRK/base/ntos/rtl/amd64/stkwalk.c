/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    stkwalk.c

Abstract:

    This module implements the routine to get the callers and the callers
    caller address.

--*/

#include "ntrtlp.h"

//
// Counter for the number of simultaneous stack walks in the system. The
// counter is relevant for only for memory management.
//
                              
LONG RtlpStackWalksInProgress;

ULONG
RtlpWalkFrameChainExceptionFilter (
    ULONG ExceptionCode,
    PVOID ExceptionRecord
    );

PRUNTIME_FUNCTION
RtlpConvertFunctionEntry (
    IN PRUNTIME_FUNCTION FunctionEntry,
    IN ULONG64 ImageBase
    );

PRUNTIME_FUNCTION
RtlpLookupFunctionEntryForStackWalks (
    IN ULONG64 ControlPc,
    OUT PULONG64 ImageBase
    )

/*++

Routine Description:

    This function searches the currently active function tables for an entry
    that corresponds to the specified control PC. This function does not
    consider the runtime function table.

Arguments:

    ControlPc - Supplies the address of an instruction within the specified
        function.

    ImageBase - Supplies the address of a variable that receives the image base
        if a function table entry contains the specified control PC.

Return Value:

    If there is no entry in the function table for the specified PC, then
    NULL is returned.  Otherwise, the address of the function table entry
    that corresponds to the specified PC is returned.

--*/

{

    ULONG64 BaseAddress;
    ULONG64 BeginAddress;
    ULONG64 EndAddress;
    PRUNTIME_FUNCTION FunctionEntry;
    PRUNTIME_FUNCTION FunctionTable;
    LONG High;
    ULONG Index;
    LONG Low;
    LONG Middle;
    ULONG RelativePc;
    ULONG SizeOfTable;

    //
    // Attempt to find an image that contains the specified control PC. If
    // an image is found, then search its function table for a function table
    // entry that contains the specified control PC. 
    // If an image is not found then do not search the dynamic function table.
    // The dynamic function table has the same lock as the loader lock, 
    // and searching that table could deadlock a caller of RtlpWalkFrameChain
    //

    //
    // attempt to find a matching entry in the loaded module list.
    //

    FunctionTable = RtlLookupFunctionTable((PVOID)ControlPc,
                                            (PVOID *)ImageBase,
                                            &SizeOfTable);

    //
    // If a function table is located, then search for a function table
    // entry that contains the specified control PC.
    //

    if (FunctionTable != NULL) {
        Low = 0;
        High = (SizeOfTable / sizeof(RUNTIME_FUNCTION)) - 1;
        RelativePc = (ULONG)(ControlPc - *ImageBase);
        while (High >= Low) {

            //
            // Compute next probe index and test entry. If the specified
            // control PC is greater than of equal to the beginning address
            // and less than the ending address of the function table entry,
            // then return the address of the function table entry. Otherwise,
            // continue the search.
            //

            Middle = (Low + High) >> 1;
            FunctionEntry = &FunctionTable[Middle];

            if (RelativePc < FunctionEntry->BeginAddress) {
                High = Middle - 1;

            } else if (RelativePc >= FunctionEntry->EndAddress) {
                Low = Middle + 1;

            } else {
                break;
            }
        }

        if (High < Low) {
            FunctionEntry = NULL;
        }

    } else {
    
        FunctionEntry = NULL;
        
    }

    return RtlpConvertFunctionEntry(FunctionEntry, *ImageBase);
}


DECLSPEC_NOINLINE
USHORT
RtlCaptureStackBackTrace (
    IN ULONG FramesToSkip,
    IN ULONG FramesToCapture,
    OUT PVOID *BackTrace,
    OUT PULONG BackTraceHash OPTIONAL
    )

/*++

Routine Description:

    This routine captures a stack back trace by walking up the stack and
    recording the information for each frame.

     N.B. This is an exported function that MUST probe the ability to take
          page faults.

Arguments:

    FramesToSkip - Supplies the number of frames to skip over at the start
        of the back trace.

    FramesToCapture - Supplies the number of frames to be captured.

    BackTrace - Supplies a pointer to the back trace buffer.

    BackTraceHash - Optionally supples a pointer to the computed hash value.

Return Value:

     The number of captured frames is returned as the function value.

--*/

{

    ULONG FramesFound;
    ULONG HashValue;
    ULONG Index;
    PVOID Trace[2 * MAX_STACK_DEPTH];

    //
    // In kernel mode avoid running at IRQL levels where page faults cannot
    // be taken. The walking code will access various sections from driver
    // and system images and this will cause page faults. Also the walking
    // code needs to bail out if the current thread is processing a page 
    // fault since collided faults may occur.
    //

    if (MmCanThreadFault() == FALSE) {
        return 0;
    }

    if (PsGetCurrentThread ()->ActiveFaultCount != 0) {
        return 0;
    }

    //
    // If the number of frames to capture plus the number of frames to skip
    // (one additional frame is skipped for the call to walk the chain), then
    // return zero.
    //

    FramesToSkip += 1;
    if ((FramesToCapture + FramesToSkip) >= (2 * MAX_STACK_DEPTH)) {
        return 0;
    }

    //
    // Capture the stack back trace.
    //

    FramesFound = RtlWalkFrameChain(&Trace[0],
                                    FramesToCapture + FramesToSkip,
                                    0);

    //
    // If the number of frames found is less than the number of frames to
    // skip, then return zero.
    //

    if (FramesFound <= FramesToSkip) {
        return 0;
    }

    //
    // Compute the hash value and transfer the captured trace to the back
    // trace buffer.
    //

    HashValue = 0;
    for (Index = 0; Index < FramesToCapture; Index += 1) {
        if (FramesToSkip + Index >= FramesFound) {
            break;
        }

        BackTrace[Index] = Trace[FramesToSkip + Index];
        HashValue += PtrToUlong(BackTrace[Index]);
    }

    if (ARGUMENT_PRESENT(BackTraceHash)) {
        *BackTraceHash = HashValue;
    }

    return (USHORT)Index;
}

#undef RtlGetCallersAddress

DECLSPEC_NOINLINE
VOID
RtlGetCallersAddress (
    OUT PVOID *CallersPc,
    OUT PVOID *CallersCallersPc
    )

/*++

Routine Description:

    This routine returns the address of the call to the routine that called
    this routine, and the address of the call to the routine that called
    the routine that called this routine. For example, if A called B called
    C which called this routine, the return addresses in B and A would be
    returned.

    N.B. This is an exported function that MUST probe the ability to take
         page faults.

Arguments:

    CallersPc - Supplies a pointer to a variable that receives the address
        of the caller of the caller of this routine (B).

    CallersCallersPc - Supplies a pointer to a variable that receives the
        address of the caller of the caller of the caller of this routine
        (A).

Return Value:

    None.

Note:

    If either of the calling stack frames exceeds the limits of the stack,
    they are set to NULL.

--*/

{

    CONTEXT ContextRecord;
    ULONG64 EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 HighLimit;
    ULONG64 ImageBase;
    ULONG64 LowLimit;

    //
    // Assume the function table entries for the various routines cannot be
    // found or there are not three procedure activation records on the stack.
    //

    *CallersPc = NULL;
    *CallersCallersPc = NULL;

    //
    // In kernel mode avoid running at IRQL levels where page faults cannot
    // be taken. The walking code will access various sections from driver
    // and system images and this will cause page faults. Also the walking
    // code needs to bail out if the current thread is processing a page 
    // fault since collided faults may occur.
    //

    if (MmCanThreadFault() == FALSE) {
        return;
    }

    if (PsGetCurrentThread ()->ActiveFaultCount != 0) {
        return;
    }

    //
    // Get current stack limits, capture the current context, virtually
    // unwind to the caller of this routine, and lookup function table entry.
    //

    RtlpGetStackLimits(&LowLimit, &HighLimit);
    RtlCaptureContext(&ContextRecord);
    FunctionEntry = RtlpLookupFunctionEntryForStackWalks(ContextRecord.Rip,
                                                         &ImageBase);

    //
    //  Attempt to unwind to the caller of this routine (C).
    //

    if (FunctionEntry != NULL) {
        RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                         ImageBase,
                         ContextRecord.Rip,
                         FunctionEntry,
                         &ContextRecord,
                         &HandlerData,
                         &EstablisherFrame,
                         NULL);

        //
        // Attempt to unwind to the caller of the caller of this routine (B).
        //

        FunctionEntry = RtlpLookupFunctionEntryForStackWalks(ContextRecord.Rip,
                                                             &ImageBase);

        if ((FunctionEntry != NULL) &&
            ((RtlpIsFrameInBounds(&LowLimit, ContextRecord.Rsp, &HighLimit) == TRUE))) {

            RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                             ImageBase,
                             ContextRecord.Rip,
                             FunctionEntry,
                             &ContextRecord,
                             &HandlerData,
                             &EstablisherFrame,
                             NULL);

            *CallersPc = (PVOID)ContextRecord.Rip;

            //
            // Attempt to unwind to the caller of the caller of the caller
            // of the caller of this routine (A).
            //

            FunctionEntry = RtlpLookupFunctionEntryForStackWalks(ContextRecord.Rip,
                                                                 &ImageBase);

            if ((FunctionEntry != NULL) &&
                ((RtlpIsFrameInBounds(&LowLimit, ContextRecord.Rsp, &HighLimit) == TRUE))) {

                RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                                 ImageBase,
                                 ContextRecord.Rip,
                                 FunctionEntry,
                                 &ContextRecord,
                                 &HandlerData,
                                 &EstablisherFrame,
                                 NULL);

                *CallersCallersPc = (PVOID)ContextRecord.Rip;
            }
        }
    }

    return;
}

DECLSPEC_NOINLINE
ULONG
RtlpWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags,
    IN ULONG FramesToSkip
    )

/*++

Routine Description:

    This function attempts to walk the call chain and capture a vector with
    a specified number of return addresses. It is possible that the function
    cannot capture the requested number of callers, in which case, the number
    of captured return addresses will be returned.

    N.B. The ability to take page faults is checked in the wrapper function.

Arguments:

    Callers - Supplies a pointer to an array that is to received the return
        address values.

    Count - Supplies the number of frames to be walked.

    Flags - Supplies the flags value (unused).

    FramesToSkip - Supplies the number of frames to skip.

Return value:

    The number of captured return addresses.

--*/

{

    CONTEXT ContextRecord;
    ULONG64 EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 HighLimit;
    ULONG64 ImageBase;
    ULONG Index;
    ULONG64 LowLimit;

    //
    // No flag values are currently supported on amd64 platforms.
    //

    if (Flags != 0) {
        return 0;
    }

    //
    // Get current stack limits and capture the current context.
    //

    RtlpGetStackLimits(&LowLimit, &HighLimit);
    RtlCaptureContext(&ContextRecord);

    //
    // Capture the requested number of return addresses if possible.
    //

    InterlockedIncrement (&RtlpStackWalksInProgress);

    Index = 0;
    try {
        while ((Index < Count) && (ContextRecord.Rip != 0)) {

            //
            // Check the next PC value to make sure it is valid in the
            // current process.
            //

            if ((MmIsSessionAddress((PVOID)ContextRecord.Rip) == TRUE && 
                 MmGetSessionId(PsGetCurrentProcess()) == 0) || 
                (MmIsAddressValid((PVOID)ContextRecord.Rip) == FALSE)) {

                break;
            }

            //
            // Lookup the function table entry using the point at which control
            // left the function.
            //

            FunctionEntry = RtlpLookupFunctionEntryForStackWalks(ContextRecord.Rip,
                                                                 &ImageBase);

            //
            // If there is a function table entry for the routine and the stack is
            // within limits, then virtually unwind to the caller of the routine
            // to obtain the return address. Otherwise, discontinue the stack walk.
            //

            if ((FunctionEntry != NULL) &&
                ((RtlpIsFrameInBounds(&LowLimit, ContextRecord.Rsp, &HighLimit) == TRUE))) {

                RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                                 ImageBase,
                                 ContextRecord.Rip,
                                 FunctionEntry,
                                 &ContextRecord,
                                 &HandlerData,
                                 &EstablisherFrame,
                                 NULL);

                if (FramesToSkip != 0){
                    FramesToSkip -= 1;

                } else {
                    Callers[Index] = (PVOID)ContextRecord.Rip;
                    Index += 1;
                }

            } else {
                break;
            }
        }

    } except (RtlpWalkFrameChainExceptionFilter(GetExceptionCode(),
                                                GetExceptionInformation())) {
        
          Index = 0;
    }

    InterlockedDecrement (&RtlpStackWalksInProgress);

    return Index;
}

DECLSPEC_NOINLINE
ULONG
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    )

/*++

Routine Description:

    This is a wrapper function for walk frame chain. It's purpose is to
    prevent entering a function that has a huge stack usage to test some
    if the current code path can take page faults.

    N.B. This is an exported function that MUST probe the ability to take
         page faults.

Arguments:

    Callers - Supplies a pointer to an array that is to received the return
        address values.

    Count - Supplies the number of frames to be walked.

    Flags - Supplies the flags value (unused).

Return value:

    Any return value from RtlpWalkFrameChain.

--*/

{

    //
    // In kernel mode avoid running at IRQL levels where page faults cannot
    // be taken. The walking code will access various sections from driver
    // and system images and this will cause page faults. Also the walking
    // code needs to bail out if the current thread is processing a page 
    // fault since collided faults may occur.
    //

    if (MmCanThreadFault() == FALSE) {
        return 0;
    }
    
    if (PsGetCurrentThread ()->ActiveFaultCount != 0) {
        return 0;
    }

    return RtlpWalkFrameChain(Callers, Count, Flags, 1);
}

