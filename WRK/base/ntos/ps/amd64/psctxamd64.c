/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    psctx.c

Abstract:

    This procedure implements Get/Set Context Thread

--*/

#include "psp.h"

//
// Define context exception flags.
//

#define CONTEXT_EXCEPTION_FLAGS (CONTEXT_EXCEPTION_ACTIVE | CONTEXT_SERVICE_ACTIVE)

//
// Define forward referenced functions.
//

PXMM_SAVE_AREA32
PspGetSetContextInternal (
    IN PKAPC Apc,
    IN PVOID OperationType,
    OUT PKEVENT *Event
    );

#pragma alloc_text(PAGE, PspGetContext)
#pragma alloc_text(PAGE, PspGetSetContextInternal)
#pragma alloc_text(PAGE, PspSetContext)

VOID
PspGetContext (
    IN PKTRAP_FRAME TrapFrame,
    IN PKNONVOLATILE_CONTEXT_POINTERS ContextPointers,
    IN OUT PCONTEXT ContextRecord
    )

/*++

Routine Description:

    This function selectively moves the contents of the specified trap frame
    and nonvolatile context to the specified context record.

Arguments:

    TrapFrame - Supplies the contents of a trap frame.

    ContextPointers - Supplies the address of context pointers record.

    ContextRecord - Supplies the address of a context record.

Return Value:

    None.

--*/

{

    ULONG ContextFlags;

    PAGED_CODE();

    //
    // Get control information if specified.
    //

    ContextFlags = ContextRecord->ContextFlags;
    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        //
        // Set registers RIP, CS, RSP, SS, and EFlags.
        //

        ContextRecord->Rip = TrapFrame->Rip;
        ContextRecord->SegCs = TrapFrame->SegCs;
        ContextRecord->SegSs = TrapFrame->SegSs;
        ContextRecord->Rsp = TrapFrame->Rsp;
        ContextRecord->EFlags = TrapFrame->EFlags;
    }

    //
    // Get segment register contents if specified.
    //

    if ((ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS) {

        //
        // Set segment registers GS, FS, ES, DS.
        //

        ContextRecord->SegDs = KGDT64_R3_DATA | RPL_MASK;
        ContextRecord->SegEs = KGDT64_R3_DATA | RPL_MASK;
        ContextRecord->SegFs = KGDT64_R3_CMTEB | RPL_MASK;
        ContextRecord->SegGs = KGDT64_R3_DATA | RPL_MASK;
    }

    //
    //  Get integer register contents if specified.
    //

    if ((ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

        //
        // Set integer registers RAX, RCX, RDX, RSI, RDI, R8, R9, R10, RBX,
        // RBP, R11, R12, R13, R14, and R15.
        //

        ContextRecord->Rax = TrapFrame->Rax;
        ContextRecord->Rcx = TrapFrame->Rcx;
        ContextRecord->Rdx = TrapFrame->Rdx;
        ContextRecord->R8 = TrapFrame->R8;
        ContextRecord->R9 = TrapFrame->R9;
        ContextRecord->R10 = TrapFrame->R10;
        ContextRecord->R11 = TrapFrame->R11;

        ContextRecord->Rbx = *ContextPointers->Rbx;
        ContextRecord->Rbp = *ContextPointers->Rbp;
        ContextRecord->Rsi = *ContextPointers->Rsi;
        ContextRecord->Rdi = *ContextPointers->Rdi;
        ContextRecord->R12 = *ContextPointers->R12;
        ContextRecord->R13 = *ContextPointers->R13;
        ContextRecord->R14 = *ContextPointers->R14;
        ContextRecord->R15 = *ContextPointers->R15;
    }

    //
    // Get floating point context if specified.
    //

    if ((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) {

        //
        // Set XMM registers Xmm0-Xmm15 and the XMM CSR contents.
        //
        // N.B. The legacy floating state is handled separately.
        //

        ContextRecord->Xmm0 = TrapFrame->Xmm0;
        ContextRecord->Xmm1 = TrapFrame->Xmm1;
        ContextRecord->Xmm2 = TrapFrame->Xmm2;
        ContextRecord->Xmm3 = TrapFrame->Xmm3;
        ContextRecord->Xmm4 = TrapFrame->Xmm4;
        ContextRecord->Xmm5 = TrapFrame->Xmm5;

        ContextRecord->Xmm6 = *ContextPointers->Xmm6;
        ContextRecord->Xmm7 = *ContextPointers->Xmm7;
        ContextRecord->Xmm8 = *ContextPointers->Xmm8;
        ContextRecord->Xmm9 = *ContextPointers->Xmm9;
        ContextRecord->Xmm10 = *ContextPointers->Xmm10;
        ContextRecord->Xmm11 = *ContextPointers->Xmm11;
        ContextRecord->Xmm12 = *ContextPointers->Xmm12;
        ContextRecord->Xmm13 = *ContextPointers->Xmm13;
        ContextRecord->Xmm14 = *ContextPointers->Xmm14;
        ContextRecord->Xmm15 = *ContextPointers->Xmm15;

        ContextRecord->MxCsr = TrapFrame->MxCsr;
    }

    //
    //
    // Get debug register contents if requested.
    //

    if ((ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS) {

        //
        // Set the debug registers DR0, DR1, DR2, DR3, DR6, and DR7.
        //

        if ((TrapFrame->Dr7 & DR7_ACTIVE) != 0) {
            ContextRecord->Dr0 = TrapFrame->Dr0;
            ContextRecord->Dr1 = TrapFrame->Dr1;
            ContextRecord->Dr2 = TrapFrame->Dr2;
            ContextRecord->Dr3 = TrapFrame->Dr3;
            ContextRecord->Dr6 = TrapFrame->Dr6;
            ContextRecord->Dr7 = TrapFrame->Dr7;
            if ((TrapFrame->Dr7 & DR7_LAST_BRANCH) != 0) {
                ContextRecord->LastBranchToRip = TrapFrame->LastBranchToRip;
                ContextRecord->LastBranchFromRip = TrapFrame->LastBranchFromRip;
                ContextRecord->LastExceptionToRip = TrapFrame->LastExceptionToRip;
                ContextRecord->LastExceptionFromRip = TrapFrame->LastExceptionFromRip;

            } else {
                ContextRecord->LastBranchToRip = 0;
                ContextRecord->LastBranchFromRip = 0;
                ContextRecord->LastExceptionToRip = 0;
                ContextRecord->LastExceptionFromRip = 0;
            }

        } else {
            ContextRecord->Dr0 = 0;
            ContextRecord->Dr1 = 0;
            ContextRecord->Dr2 = 0;
            ContextRecord->Dr3 = 0;
            ContextRecord->Dr6 = 0;
            ContextRecord->Dr7 = 0;
            ContextRecord->LastBranchToRip = 0;
            ContextRecord->LastBranchFromRip = 0;
            ContextRecord->LastExceptionToRip = 0;
            ContextRecord->LastExceptionFromRip = 0;
        }
    }

    //
    // Get exception reporting information if requested.
    //

    if ((ContextFlags & CONTEXT_EXCEPTION_REQUEST) != 0) {
        ContextRecord->ContextFlags &= ~CONTEXT_EXCEPTION_FLAGS;
        ContextRecord->ContextFlags |= CONTEXT_EXCEPTION_REPORTING;
        if (TrapFrame->ExceptionActive == 1) {
            ContextRecord->ContextFlags |= CONTEXT_EXCEPTION_ACTIVE;
    
        } else if (TrapFrame->ExceptionActive == 2) {
            ContextRecord->ContextFlags |= CONTEXT_SERVICE_ACTIVE;
        }
    }

    return;
}

VOID
PspSetContext (
    OUT PKTRAP_FRAME TrapFrame,
    OUT PKNONVOLATILE_CONTEXT_POINTERS ContextPointers,
    IN PCONTEXT ContextRecord,
    KPROCESSOR_MODE PreviousMode
    )

/*++

Routine Description:

    This function selectively moves the contents of the specified context
    record to the specified trap frame and nonvolatile context.

Arguments:

    TrapFrame - Supplies the address of a trap frame.

    ContextPointers - Supplies the address of a context pointers record.

    ContextRecord - Supplies the address of a context record.

    ProcessorMode - Supplies the processor mode to use when sanitizing
        the PSR and FSR.

Return Value:

    None.

--*/

{

    ULONG ContextFlags;

    PAGED_CODE();

    //
    // Set control information if specified.
    //

    ContextFlags = ContextRecord->ContextFlags;
    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {
        TrapFrame->EFlags = SANITIZE_EFLAGS(ContextRecord->EFlags, PreviousMode);
        TrapFrame->Rip = ContextRecord->Rip;
        TrapFrame->Rsp = ContextRecord->Rsp;
    }

    //
    // The segment registers DS, ES, FS, and GS are never restored from saved
    // data. However, SS and CS are restored from the trap frame. Make sure
    // that these segment registers have the proper values.
    //

    if (PreviousMode == UserMode) {
        TrapFrame->SegSs = KGDT64_R3_DATA | RPL_MASK;
        if (ContextRecord->SegCs != (KGDT64_R3_CODE | RPL_MASK)) {
            TrapFrame->SegCs = KGDT64_R3_CMCODE | RPL_MASK;

        } else {
            TrapFrame->SegCs = KGDT64_R3_CODE | RPL_MASK;
        }

    } else {
        TrapFrame->SegCs = KGDT64_R0_CODE;
        TrapFrame->SegSs = KGDT64_NULL;
    }

    TrapFrame->Rip = SANITIZE_VA(TrapFrame->Rip, TrapFrame->SegCs, PreviousMode);

    //
    // Set integer registers contents if specified.
    //

    if ((ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

        //
        // Set integer registers RAX, RCX, RDX, RSI, RDI, R8, R9, R10, RBX,
        // RBP, R11, R12, R13, R14, and R15.
        //

        TrapFrame->Rax = ContextRecord->Rax;
        TrapFrame->Rcx = ContextRecord->Rcx;
        TrapFrame->Rdx = ContextRecord->Rdx;
        TrapFrame->R8 = ContextRecord->R8;
        TrapFrame->R9 = ContextRecord->R9;
        TrapFrame->R10 = ContextRecord->R10;
        TrapFrame->R11 = ContextRecord->R11;

        *ContextPointers->Rbx = ContextRecord->Rbx;
        *ContextPointers->Rbp = ContextRecord->Rbp;
        *ContextPointers->Rsi = ContextRecord->Rsi;
        *ContextPointers->Rdi = ContextRecord->Rdi;
        *ContextPointers->R12 = ContextRecord->R12;
        *ContextPointers->R13 = ContextRecord->R13;
        *ContextPointers->R14 = ContextRecord->R14;
        *ContextPointers->R15 = ContextRecord->R15;
    }

    //
    // Set floating register contents if requested.
    //

    if ((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) {

        //
        // Set XMM registers Xmm0-Xmm15 and the XMM CSR contents.
        //
        // N.B. The legacy floating state is handled separately.
        //

        TrapFrame->Xmm0 = ContextRecord->Xmm0;
        TrapFrame->Xmm1 = ContextRecord->Xmm1;
        TrapFrame->Xmm2 = ContextRecord->Xmm2;
        TrapFrame->Xmm3 = ContextRecord->Xmm3;
        TrapFrame->Xmm4 = ContextRecord->Xmm4;
        TrapFrame->Xmm5 = ContextRecord->Xmm5;

        *ContextPointers->Xmm6 = ContextRecord->Xmm6;
        *ContextPointers->Xmm7 = ContextRecord->Xmm7;
        *ContextPointers->Xmm8 = ContextRecord->Xmm8;
        *ContextPointers->Xmm9 = ContextRecord->Xmm9;
        *ContextPointers->Xmm10 = ContextRecord->Xmm10;
        *ContextPointers->Xmm11 = ContextRecord->Xmm11;
        *ContextPointers->Xmm12 = ContextRecord->Xmm12;
        *ContextPointers->Xmm13 = ContextRecord->Xmm13;
        *ContextPointers->Xmm14 = ContextRecord->Xmm14;
        *ContextPointers->Xmm15 = ContextRecord->Xmm15;

        //
        // Clear all reserved bits in MXCSR.
        //

        TrapFrame->MxCsr = SANITIZE_MXCSR(ContextRecord->MxCsr);

        //
        // Clear all reserved bits in legacy floating state.
        //
        // N.B. The legacy floating state is restored if and only if the
        //      request mode is user.
        //
        // N.B. The current MXCSR value is placed in the legacy floating
        //      state so it will get restored if the legacy state is
        //      restored.
        //

        ContextRecord->FltSave.MxCsr = ReadMxCsr();
        ContextRecord->FltSave.ControlWord =
                            SANITIZE_FCW(ContextRecord->FltSave.ControlWord);
    }

    //
    // Set debug register state if specified.
    //

    if ((ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS) {

        //
        // Set the debug registers DR0, DR1, DR2, DR3, DR6, and DR7.
        //

        TrapFrame->Dr0 = SANITIZE_DRADDR(ContextRecord->Dr0, PreviousMode);
        TrapFrame->Dr1 = SANITIZE_DRADDR(ContextRecord->Dr1, PreviousMode);
        TrapFrame->Dr2 = SANITIZE_DRADDR(ContextRecord->Dr2, PreviousMode);
        TrapFrame->Dr3 = SANITIZE_DRADDR(ContextRecord->Dr3, PreviousMode);
        TrapFrame->Dr6 = 0;
        TrapFrame->Dr7 = SANITIZE_DR7(ContextRecord->Dr7, PreviousMode);
        if (PreviousMode != KernelMode) {
            KeGetCurrentThread()->Header.DebugActive =
                                (BOOLEAN)((TrapFrame->Dr7 & DR7_ACTIVE) != 0);
        }
    }

    return;
}

PXMM_SAVE_AREA32
PspGetSetContextInternal (
    IN PKAPC Apc,
    IN PVOID OperationType,
    OUT PKEVENT *Event
    )

/*++

Routine Description:

    This function either captures the state of the current thread, or sets
    the state of the current thread.

Arguments:

    Apc - Supplies a pointer to an APC object.

    OperationType - Supplies the type of context operation to be performed.
        A value of NULL specifies a get context operation and a nonNULL value
        a set context operation.

    Event - Supplies a pointer to a variable that receives the completion
        event address.

Return Value:

    If the context operation is a set context and the legacy floating state is
    switched for the current thread, then the address of the legacy floating
    save area is returned as the function value. Otherwise, NULL is returned.

--*/

{

    PGETSETCONTEXT ContextBlock;
    ULONG ContextFlags;
    PKNONVOLATILE_CONTEXT_POINTERS ContextPointers;
    CONTEXT ContextRecord;
    ULONG64 ControlPc;
    ULONG64 EstablisherFrame;
    PRUNTIME_FUNCTION FunctionEntry;
    PVOID HandlerData;
    ULONG64 ImageBase;
    PETHREAD Thread;
    ULONG64 TrapFrame;
    PXMM_SAVE_AREA32 XmmSaveArea;

    PAGED_CODE();

    //
    // Get the address of the context block and compute the address of the
    // system entry trap frame.
    //

    ContextBlock = CONTAINING_RECORD(Apc, GETSETCONTEXT, Apc);
    ContextPointers = &ContextBlock->NonVolatileContext;
    EstablisherFrame = 0;
    Thread = PsGetCurrentThread();
    TrapFrame = 0;
    if (ContextBlock->Mode == KernelMode) {
        TrapFrame = (ULONG64)Thread->Tcb.TrapFrame;
    }

    if (TrapFrame == 0) {
        TrapFrame = (ULONG64)PspGetBaseTrapFrame(Thread);
    }

    //
    // Capture the current thread context and set the initial control PC
    // value.
    //

    RtlCaptureContext(&ContextRecord);

    //
    // Initialize context pointers for the nonvolatile integer and floating
    // registers.
    //

#if DBG

    RtlZeroMemory(ContextPointers, sizeof(KNONVOLATILE_CONTEXT_POINTERS));

#endif

    ContextPointers->Rbx = &ContextRecord.Rbx;
    ContextPointers->Rsp = &ContextRecord.Rsp;
    ContextPointers->Rbp = &ContextRecord.Rbp;
    ContextPointers->Rsi = &ContextRecord.Rsi;
    ContextPointers->Rdi = &ContextRecord.Rdi;
    ContextPointers->R12 = &ContextRecord.R12;
    ContextPointers->R13 = &ContextRecord.R13;
    ContextPointers->R14 = &ContextRecord.R14;
    ContextPointers->R15 = &ContextRecord.R15;

    ContextPointers->Xmm6 = &ContextRecord.Xmm6;
    ContextPointers->Xmm7 = &ContextRecord.Xmm7;
    ContextPointers->Xmm8 = &ContextRecord.Xmm8;
    ContextPointers->Xmm9 = &ContextRecord.Xmm9;
    ContextPointers->Xmm10 = &ContextRecord.Xmm10;
    ContextPointers->Xmm11 = &ContextRecord.Xmm11;
    ContextPointers->Xmm12 = &ContextRecord.Xmm12;
    ContextPointers->Xmm13 = &ContextRecord.Xmm13;
    ContextPointers->Xmm14 = &ContextRecord.Xmm14;
    ContextPointers->Xmm15 = &ContextRecord.Xmm15;

    //
    // Start with the frame specified by the context record and virtually
    // unwind call frames until the system entry trap frame is encountered.
    //

    do {

        //
        // Lookup the function table entry using the point at which control
        // left the function.
        //

        ControlPc = ContextRecord.Rip;
        FunctionEntry = RtlLookupFunctionEntry(ControlPc, &ImageBase, NULL);

        //
        // If there is a function table entry for the routine, then virtually
        // unwind to the caller of the current routine to obtain the address
        // where control left the caller. Otherwise, the function is a leaf
        // function and the return address register contains the address of
        // where control left the caller.
        //

        if (FunctionEntry != NULL) {
            RtlVirtualUnwind(UNW_FLAG_EHANDLER,
                             ImageBase,
                             ControlPc,
                             FunctionEntry,
                             &ContextRecord,
                             &HandlerData,
                             &EstablisherFrame,
                             ContextPointers);

        } else {
            ContextRecord.Rip = *(PULONG64)(ContextRecord.Rsp);
            ContextRecord.Rsp += 8;
        }

    } while (EstablisherFrame != TrapFrame);

    //
    // If system argument one is nonzero, then set the context of the current
    // thread. Otherwise, get the context of the current thread.
    //

    XmmSaveArea = NULL;
    if (OperationType != NULL) {

        //
        // Set context.
        //
        // If the context mode is user and floating state is being set, then
        // set the address of the legacy floating information.
        //

        ContextFlags = ContextBlock->Context.ContextFlags;
        if ((ContextBlock->Mode == UserMode) &&
            ((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT)) {

            XmmSaveArea = &ContextBlock->Context.FltSave;
        }

        //
        // Set context.
        //

        PspSetContext((PKTRAP_FRAME)TrapFrame,
                      ContextPointers,
                      &ContextBlock->Context,
                      ContextBlock->Mode);

    } else {

        //
        // Get context.
        //
        // If the context mode is user, then save the legacy floating state.
        //
    
        if (ContextBlock->Mode == UserMode) {
            KeSaveLegacyFloatingPointState(&ContextBlock->Context.FltSave);
        }
    
        PspGetContext((PKTRAP_FRAME)TrapFrame,
                       ContextPointers,
                       &ContextBlock->Context);
    }

    *Event = &ContextBlock->OperationComplete;
    return XmmSaveArea;
}

