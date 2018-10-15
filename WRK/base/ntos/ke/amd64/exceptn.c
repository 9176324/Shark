/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    exceptn.c

Abstract:

    This module implement the code necessary to dispatch exceptions to the
    proper mode and invoke the exception dispatcher.

--*/

#include "ki.h"

BOOLEAN
KiPreprocessFault (
    IN OUT PEXCEPTION_RECORD ExceptionRecord,
    IN PKTRAP_FRAME TrapFrame,
    IN OUT PCONTEXT ContextRecord,
    IN KPROCESSOR_MODE PreviousMode
    );

VOID
KeContextFromKframes (
    __in PKTRAP_FRAME TrapFrame,
    __in PKEXCEPTION_FRAME ExceptionFrame,
    __inout PCONTEXT ContextRecord
    )

/*++

Routine Description:

    This routine moves the selected contents of the specified trap and
    exception frames into the specified context frame according to the
    specified context flags.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame from which volatile
        context should be copied into the context record.

    ExceptionFrame - Supplies a pointer to an exception frame from which
        context should be copied into the context record.

    ContextRecord - Supplies a pointer to the context frame that receives
        the context copied from the trap and exception frames.

Return Value:

    None.

--*/

{

    ULONG ContextFlags;
    KIRQL OldIrql;

    //
    // Raise IRQL to APC_LEVEL to guarantee that a consistent set of context
    // is transferred from the trap and exception frames.
    //

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < APC_LEVEL) {
        KfRaiseIrql(APC_LEVEL);
    }

    //
    // Set control information if specified.
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
    // Set segment register contents if specified.
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
    // Set integer register contents if specified.
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
        ContextRecord->Rbp = TrapFrame->Rbp;

        ContextRecord->Rbx = ExceptionFrame->Rbx;
        ContextRecord->Rdi = ExceptionFrame->Rdi;
        ContextRecord->Rsi = ExceptionFrame->Rsi;
        ContextRecord->R12 = ExceptionFrame->R12;
        ContextRecord->R13 = ExceptionFrame->R13;
        ContextRecord->R14 = ExceptionFrame->R14;
        ContextRecord->R15 = ExceptionFrame->R15;
    }

    //
    // Set floating point context if specified.
    //
    //

    if ((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) {

        //
        // If the specified mode is user, then save the legacy floating
        // point state.
        //

        if ((TrapFrame->SegCs & MODE_MASK) == UserMode) {
            KeSaveLegacyFloatingPointState(&ContextRecord->FltSave);
        }

        //
        // Set XMM registers Xmm0-Xmm15 and the XMM CSR contents.
        //

        ContextRecord->Xmm0 = TrapFrame->Xmm0;
        ContextRecord->Xmm1 = TrapFrame->Xmm1;
        ContextRecord->Xmm2 = TrapFrame->Xmm2;
        ContextRecord->Xmm3 = TrapFrame->Xmm3;
        ContextRecord->Xmm4 = TrapFrame->Xmm4;
        ContextRecord->Xmm5 = TrapFrame->Xmm5;

        ContextRecord->Xmm6 = ExceptionFrame->Xmm6;
        ContextRecord->Xmm7 = ExceptionFrame->Xmm7;
        ContextRecord->Xmm8 = ExceptionFrame->Xmm8;
        ContextRecord->Xmm9 = ExceptionFrame->Xmm9;
        ContextRecord->Xmm10 = ExceptionFrame->Xmm10;
        ContextRecord->Xmm11 = ExceptionFrame->Xmm11;
        ContextRecord->Xmm12 = ExceptionFrame->Xmm12;
        ContextRecord->Xmm13 = ExceptionFrame->Xmm13;
        ContextRecord->Xmm14 = ExceptionFrame->Xmm14;
        ContextRecord->Xmm15 = ExceptionFrame->Xmm15;

        ContextRecord->MxCsr = TrapFrame->MxCsr;
    }

    //
    //
    // Set debug register contents if requested.
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
    // Lower IRQL to its previous value.
    //

    if (OldIrql < APC_LEVEL) {
        KeLowerIrql(OldIrql);
    }

    return;
}

PXMM_SAVE_AREA32
KxContextToKframes (
    __inout PKTRAP_FRAME TrapFrame,
    __inout PKEXCEPTION_FRAME ExceptionFrame,
    __in PCONTEXT ContextRecord,
    __in ULONG ContextFlags,
    __in KPROCESSOR_MODE PreviousMode
    )

/*++

Routine Description:

    This routine moves the selected contents of the specified context frame
    into the specified trap and exception frames according to the specified
    context flags.

Arguments:

    TrapFrame - Supplies a pointer to a trap frame that receives the volatile
        context from the context record.

    ExceptionFrame - Supplies a pointer to an exception frame that receives
        the nonvolatile context from the context record.

    ContextRecord - Supplies a pointer to a context frame that contains the
        context that is to be copied into the trap and exception frames.

    ContextFlags - Supplies the set of flags that specify which parts of the
        context frame are to be copied into the trap and exception frames.

    PreviousMode - Supplies the processor mode for which the exception and
        trap frames are being built.

Return Value:

    If the context operation is a set context and the legacy floating state is
    switched for the current thread, then the address of the legacy floating
    save area is returned as the function value. Otherwise, NULL is returned.

--*/

{

    PXMM_SAVE_AREA32 XmmSaveArea;

    //
    // Set control information if specified.
    //

    XmmSaveArea = NULL;
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
        TrapFrame->SegSs = KGDT64_R0_DATA;
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
        TrapFrame->Rbp = ContextRecord->Rbp;

        ExceptionFrame->Rbx = ContextRecord->Rbx;
        ExceptionFrame->Rsi = ContextRecord->Rsi;
        ExceptionFrame->Rdi = ContextRecord->Rdi;
        ExceptionFrame->R12 = ContextRecord->R12;
        ExceptionFrame->R13 = ContextRecord->R13;
        ExceptionFrame->R14 = ContextRecord->R14;
        ExceptionFrame->R15 = ContextRecord->R15;
    }

    //
    // Set floating register contents if requested.
    //

    if ((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) {

        //
        // Set XMM registers Xmm0-Xmm15 and the XMM CSR contents.
        //

        TrapFrame->Xmm0 = ContextRecord->Xmm0;
        TrapFrame->Xmm1 = ContextRecord->Xmm1;
        TrapFrame->Xmm2 = ContextRecord->Xmm2;
        TrapFrame->Xmm3 = ContextRecord->Xmm3;
        TrapFrame->Xmm4 = ContextRecord->Xmm4;
        TrapFrame->Xmm5 = ContextRecord->Xmm5;

        ExceptionFrame->Xmm6 = ContextRecord->Xmm6;
        ExceptionFrame->Xmm7 = ContextRecord->Xmm7;
        ExceptionFrame->Xmm8 = ContextRecord->Xmm8;
        ExceptionFrame->Xmm9 = ContextRecord->Xmm9;
        ExceptionFrame->Xmm10 = ContextRecord->Xmm10;
        ExceptionFrame->Xmm11 = ContextRecord->Xmm11;
        ExceptionFrame->Xmm12 = ContextRecord->Xmm12;
        ExceptionFrame->Xmm13 = ContextRecord->Xmm13;
        ExceptionFrame->Xmm14 = ContextRecord->Xmm14;
        ExceptionFrame->Xmm15 = ContextRecord->Xmm15;

        //
        // Clear all reserved bits in MXCSR.
        //

        TrapFrame->MxCsr = SANITIZE_MXCSR(ContextRecord->MxCsr);

        //
        // If the specified mode is user, then set the legacy floating point
        // state.
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

        if (PreviousMode == UserMode) {
            XmmSaveArea = &ContextRecord->FltSave;
            ContextRecord->FltSave.MxCsr = ReadMxCsr();
            ContextRecord->FltSave.ControlWord =
                                SANITIZE_FCW(ContextRecord->FltSave.ControlWord);
        }
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
        TrapFrame->LastBranchToRip = ContextRecord->LastBranchToRip;
        TrapFrame->LastBranchFromRip = ContextRecord->LastBranchFromRip;
        TrapFrame->LastExceptionToRip = ContextRecord->LastExceptionToRip;
        TrapFrame->LastExceptionFromRip = ContextRecord->LastExceptionFromRip;
        if (PreviousMode != KernelMode) {
            KeGetCurrentThread()->Header.DebugActive =
                                (BOOLEAN)((TrapFrame->Dr7 & DR7_ACTIVE) != 0);
        }
    }

    return XmmSaveArea;
}

VOID
KiDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance
    )

/*++

Routine Description:

    This function is called to dispatch an exception to the proper mode and
    to cause the exception dispatcher to be called. If the previous mode is
    kernel, then the exception dispatcher is called directly to process the
    exception. Otherwise the exception record, exception frame, and trap
    frame contents are copied to the user mode stack. The contents of the
    exception frame and trap are then modified such that when control is
    returned, execution will commense in user mode in a routine which will
    call the exception dispatcher.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record.

    ExceptionFrame - Supplies a pointer to an exception frame. For NT386,
        this should be NULL.

    TrapFrame - Supplies a pointer to a trap frame.

    PreviousMode - Supplies the previous processor mode.

    FirstChance - Supplies a boolean value that specifies whether this is
        the first (TRUE) or second (FALSE) chance for the exception.

Return Value:

    None.

--*/

{

    CONTEXT ContextRecord;
    BOOLEAN DebugService;
    EXCEPTION_RECORD ExceptionRecord1;
    BOOLEAN ExceptionWasForwarded = FALSE;
    ULONG64 FaultingRsp;
    PMACHINE_FRAME MachineFrame;
    ULONG64 UserStack1;
    ULONG64 UserStack2;

    //
    // Move machine state from trap and exception frames to a context frame
    // and increment the number of exceptions dispatched.
    //

    KeGetCurrentPrcb()->KeExceptionDispatchCount += 1;
    ContextRecord.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS | CONTEXT_SEGMENTS;
    KeContextFromKframes(TrapFrame, ExceptionFrame, &ContextRecord);

    //
    // If the exception is a break point, then convert the break point to a
    // fault.
    //

    if (ExceptionRecord->ExceptionCode == STATUS_BREAKPOINT) {
        ContextRecord.Rip -= 1;
    }

    //
    // If the exception is an internal general protect fault, invalid opcode,
    // or integer divide by zero, then attempt to resolve the problem without
    // actually raising an exception.
    // 

    if (KiPreprocessFault(ExceptionRecord,
                          TrapFrame,
                          &ContextRecord,
                          PreviousMode) != FALSE) {

        goto Handled1;
    }

    //
    // Select the method of handling the exception based on the previous mode.
    //

    if (PreviousMode == KernelMode) {

        //
        // Previous mode was kernel.
        //
        // If the kernel debugger is active, then give the kernel debugger
        // the first chance to handle the exception. If the kernel debugger
        // handles the exception, then continue execution. Otherwise, attempt
        // to dispatch the exception to a frame based handler. If a frame
        // based handler handles the exception, then continue execution.
        //
        // If a frame based handler does not handle the exception, give the
        // kernel debugger a second chance, if it's present.
        //
        // If the exception is still unhandled call bugcheck.
        //

        if (FirstChance != FALSE) {
            if ((KiDebugRoutine)(TrapFrame,
                                 ExceptionFrame,
                                 ExceptionRecord,
                                 &ContextRecord,
                                 PreviousMode,
                                 FALSE) != FALSE) {

                goto Handled1;
            }

            //
            // Kernel debugger didn't handle exception.
            //
            // If interrupts are disabled, then bugcheck.
            //

            if (RtlDispatchException(ExceptionRecord, &ContextRecord) != FALSE) {
                goto Handled1;
            }
        }

        //
        // This is the second chance to handle the exception.
        //

        if ((KiDebugRoutine)(TrapFrame,
                             ExceptionFrame,
                             ExceptionRecord,
                             &ContextRecord,
                             PreviousMode,
                             TRUE) != FALSE) {

            goto Handled1;
        }

        KeBugCheckEx(KMODE_EXCEPTION_NOT_HANDLED,
                     ExceptionRecord->ExceptionCode,
                     (ULONG64)ExceptionRecord->ExceptionAddress,
                     ExceptionRecord->ExceptionInformation[0],
                     ExceptionRecord->ExceptionInformation[1]);

    } else {

        //
        // Previous mode was user.
        //
        // If this is the first chance and the current process has a debugger
        // port, then send a message to the debugger port and wait for a reply.
        // If the debugger handles the exception, then continue execution. Else
        // transfer the exception information to the user stack, transition to
        // user mode, and attempt to dispatch the exception to a frame based
        // handler. If a frame based handler handles the exception, then continue
        // execution with the continue system service. Else execute the
        // NtRaiseException system service with FirstChance == FALSE, which
        // will call this routine a second time to process the exception.
        //
        // If this is the second chance and the current process has a debugger
        // port, then send a message to the debugger port and wait for a reply.
        // If the debugger handles the exception, then continue execution. Else
        // if the current process has a subsystem port, then send a message to
        // the subsystem port and wait for a reply. If the subsystem handles the
        // exception, then continue execution. Else terminate the process.
        //
        // If the current process is a wow64 process, an alignment fault has
        // occurred, and the AC bit is set in EFLAGS, then clear AC in EFLAGS
        // and continue execution. Otherwise, attempt to resolve the exception.
        //

        if ((PsGetCurrentProcess()->Wow64Process != NULL) &&
            (ExceptionRecord->ExceptionCode == STATUS_DATATYPE_MISALIGNMENT) &&
            ((TrapFrame->EFlags & EFLAGS_AC_MASK) != 0)) {

            TrapFrame->EFlags &= ~EFLAGS_AC_MASK;
            goto Handled2;
        }

        //
        // If the exception happened while executing 32-bit code, then convert
        // the exception to a wow64 exception. These codes are translated later
        // by wow64.
        //

        if ((ContextRecord.SegCs & 0xfff8) == KGDT64_R3_CMCODE) {
            
            switch (ExceptionRecord->ExceptionCode) {
            case STATUS_BREAKPOINT:
                ExceptionRecord->ExceptionCode = STATUS_WX86_BREAKPOINT;
                break;

            case STATUS_SINGLE_STEP:
                ExceptionRecord->ExceptionCode = STATUS_WX86_SINGLE_STEP;
                break;
            }

            //
            // Clear the upper 32-bits of the stack address and 16-byte
            // align the stack address.
            //

            FaultingRsp = (ContextRecord.Rsp & 0xfffffff0UI64);

        } else {
            FaultingRsp = ContextRecord.Rsp;
        }

        if (FirstChance == TRUE) {

            //
            // This is the first chance to handle the exception.
            //
            // If the current processor is not being debugged and user mode
            // exceptions are not being ignored, or this is a debug service,
            // then attempt to handle the exception via the kernel debugger.
            //


            DebugService = KdIsThisAKdTrap(ExceptionRecord,
                                           &ContextRecord,
                                           UserMode);

            if (((PsGetCurrentProcess()->DebugPort == NULL) &&
                 (KdIgnoreUmExceptions == FALSE)) ||
                (DebugService == TRUE)) {

                //
                // Attempt to handle the exception with the kernel debugger.
                //

                if ((KiDebugRoutine)(TrapFrame,
                                     ExceptionFrame,
                                     ExceptionRecord,
                                     &ContextRecord,
                                     PreviousMode,
                                     FALSE) != FALSE) {

                    goto Handled1;
                }
            }

            if ((ExceptionWasForwarded == FALSE) &&
                (DbgkForwardException(ExceptionRecord, TRUE, FALSE))) {

                goto Handled2;
            }

            //
            // Clear the trace flag in the trap frame so a spurious trace
            // trap is guaranteed not to occur in the trampoline code.
            //
    
            TrapFrame->EFlags &= ~EFLAGS_TF_MASK;

            //
            // Transfer exception information to the user stack, transition
            // to user mode, and attempt to dispatch the exception to a frame
            // based handler.
            //

            ExceptionRecord1.ExceptionCode = STATUS_ACCESS_VIOLATION;

        repeat:
            try {

                //
                // Compute address of aligned machine frame, compute address
                // of exception record, compute address of context record,
                // and probe user stack for writeability.
                //

                MachineFrame =
                    (PMACHINE_FRAME)((FaultingRsp - sizeof(MACHINE_FRAME)) & ~STACK_ROUND);

                UserStack1 = (ULONG64)MachineFrame - EXCEPTION_RECORD_LENGTH;
                UserStack2 = UserStack1 - CONTEXT_LENGTH;
                ProbeForWriteSmallStructure((PVOID)UserStack2,
                                            sizeof(MACHINE_FRAME) + EXCEPTION_RECORD_LENGTH + CONTEXT_LENGTH,
                                            STACK_ALIGN);

                //
                // Fill in machine frame information.
                //

                MachineFrame->Rsp = FaultingRsp;
                MachineFrame->Rip = ContextRecord.Rip;

                //
                // Copy exception record to the user stack.
                //

                *(PEXCEPTION_RECORD)UserStack1 = *ExceptionRecord;

                //
                // Copy context record to the user stack.
                //

                *(PCONTEXT)UserStack2 = ContextRecord;

                //
                // Set the address of the new stack pointer in the current
                // trap frame.
                //

                TrapFrame->Rsp = UserStack2;

                //
                // Set the user mode 64-bit code selector.
                //

                TrapFrame->SegCs = KGDT64_R3_CODE | RPL_MASK;

                //
                // Set the address of the exception routine that will call the
                // exception dispatcher and then return to the trap handler.
                // The trap handler will restore the exception and trap frame
                // context and continue execution in the routine that will
                // call the exception dispatcher.
                //

                TrapFrame->Rip = (ULONG64)KeUserExceptionDispatcher;
                return;

            } except (KiCopyInformation(&ExceptionRecord1,
                        (GetExceptionInformation())->ExceptionRecord)) {

                //
                // If the exception is a stack overflow, then attempt to
                // raise the stack overflow exception. Otherwise, the user's
                // stack is not accessible, or is misaligned, and second
                // chance processing is performed.
                //

                if (ExceptionRecord1.ExceptionCode == STATUS_STACK_OVERFLOW) {
                    ExceptionRecord1.ExceptionAddress = ExceptionRecord->ExceptionAddress;
                    *ExceptionRecord = ExceptionRecord1;

                    goto repeat;
                }
            }
        }

        //
        // This is the second chance to handle the exception.
        //

        if (DbgkForwardException(ExceptionRecord, TRUE, TRUE)) {
            goto Handled2;

        } else if (DbgkForwardException(ExceptionRecord, FALSE, TRUE)) {
            goto Handled2;

        } else {
            ZwTerminateProcess(NtCurrentProcess(), ExceptionRecord->ExceptionCode);
            KeBugCheckEx(KMODE_EXCEPTION_NOT_HANDLED,
                         ExceptionRecord->ExceptionCode,
                         (ULONG64)ExceptionRecord->ExceptionAddress,
                         ExceptionRecord->ExceptionInformation[0],
                         ExceptionRecord->ExceptionInformation[1]);
        }
    }

    //
    // Move machine state from context frame to trap and exception frames and
    // then return to continue execution with the restored state.
    //

Handled1:
    KeContextToKframes(TrapFrame,
                       ExceptionFrame,
                       &ContextRecord,
                       ContextRecord.ContextFlags,
                       PreviousMode);

    //
    // Exception was handled by the debugger or the associated subsystem
    // and state was modified, if necessary, using the get state and set
    // state capabilities. Therefore the context frame does not need to
    // be transferred to the trap and exception frames.
    //

Handled2:
    return;
}

ULONG
KiCopyInformation (
    IN OUT PEXCEPTION_RECORD ExceptionRecord1,
    IN PEXCEPTION_RECORD ExceptionRecord2
    )

/*++

Routine Description:

    This function is called from an exception filter to copy the exception
    information from one exception record to another when an exception occurs.

Arguments:

    ExceptionRecord1 - Supplies a pointer to the destination exception record.

    ExceptionRecord2 - Supplies a pointer to the source exception record.

Return Value:

    A value of EXCEPTION_EXECUTE_HANDLER is returned as the function value.

--*/

{

    //
    // Copy one exception record to another and return value that causes
    // an exception handler to be executed.
    //

    *ExceptionRecord1 = *ExceptionRecord2;

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOLEAN
KeQueryCurrentStackInformation (
    __out PKERNEL_STACK_LIMITS Type,
    __out PULONG64 LowLimit,
    __out PULONG64 HighLimit
    )

/*++

Routine Description:

    This function determines the current kernel stack type and limits.

Arguments:

    Type - Supplies a pointer to a variable that receives the kernel stack
        type.

    LowLimit - Supplies a pointer to a variable that receives the low
        stack limit.

    HighLimit - Supplies a pointer to a variable that receives the high
        stack limit.

Return Value;

    A value of TRUE is returned if the current stack pointer is within the
    defined limits. Otherwise, a value of FALSE is returned.

--*/

{

    ULONG64 CurrentStack;
    PKERNEL_STACK_CONTROL StackControl;
    PKTHREAD Thread;

    //
    // If a bugcheck is in progress, then return system space as the stack
    // limits. Otherwise, if a DPC is active then return the DPC stack limits.
    // Otherwise, return the thread stack limits.
    //

    if (KeBugCheckActive != FALSE) {
        *Type = BugcheckStackLimits;
        *HighLimit = (ULONG64)MM_SYSTEM_SPACE_END;
        *LowLimit = (ULONG64)MM_KSEG0_BASE;
        return TRUE;

    } else {
        Thread = KeGetCurrentThread();
        if ((KeIsExecutingLegacyDpc() == TRUE) &&
            (Thread != KeGetCurrentPrcb()->IdleThread)) {

            *Type = DPCStackLimits;
            *HighLimit = KeGetDpcStackBase();
            *LowLimit = *HighLimit - KERNEL_STACK_SIZE;

        } else {
            if (Thread->CalloutActive == TRUE) {
                *Type = ExpandedStackLimits;
    
            } else if (Thread->LargeStack == TRUE) {
                *Type = Win32kStackLimits;
    
            } else {
                *Type = NormalStackLimits;
            }

            StackControl = (PKERNEL_STACK_CONTROL)(Thread->InitialStack);
            if (((ULONG64)Thread->StackBase == StackControl->Current.StackBase) &&
                (StackControl->Current.ActualLimit <= (ULONG64)Thread->StackLimit) &&
                ((ULONG64)Thread->StackLimit < StackControl->Current.StackBase)) {

                *HighLimit = (ULONG64)Thread->StackBase;
                *LowLimit = (ULONG64)Thread->StackLimit;

            } else {
                *HighLimit = StackControl->Current.StackBase;
                *LowLimit = StackControl->Current.ActualLimit;
            }
        }

        //
        // Check to determine if the current stack is within the computed
        // limits.
        //
    
        CurrentStack = KeGetCurrentStackPointer();
        if ((*LowLimit <= CurrentStack) && (CurrentStack < *HighLimit)) {
            return TRUE;

        } else {
            return FALSE;
        }
    }
}

NTSTATUS
KeRaiseUserException (
    IN NTSTATUS ExceptionCode
    )

/*++

Routine Description:

    This function causes an exception to be raised in the calling thread's
    user context.

Arguments:

    ExceptionCode - Supplies the status value to be raised.

Return Value:

    The status value that should be returned by the caller.

--*/

{

    PTEB Teb;
    PKTHREAD Thread;
    PKTRAP_FRAME TrapFrame;

    //
    // Save the exception code in the TEB and set the return address in the
    // trap frame to return to the raise user exception code in user mode.
    // This replaces the normal return which would go to the system service
    // dispatch stub. The system service dispatch stub is called thus the
    // return to the system service call site is on the top of the user stack.
    //

    Thread = KeGetCurrentThread();
    TrapFrame = Thread->TrapFrame;
    if ((TrapFrame != NULL) &&
        ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {
        Teb = (PTEB)Thread->Teb;
        try {
            Teb->ExceptionCode = ExceptionCode;
    
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return ExceptionCode;
        }

        TrapFrame->Rip = (ULONG64)KeRaiseUserExceptionDispatcher;
    }

    return ExceptionCode;
}

