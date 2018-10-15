/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    apcuser.c

Abstract:

    This module implements the machine dependent code necessary to initialize
    a user mode APC.

Environment:

    IRQL APC_LEVEL.

--*/

#include "ki.h"

DECLSPEC_NOINLINE
VOID
KiContinuePreviousModeUser (
    IN PCONTEXT ContextRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This function is called to copy the specified context record when the
    previous mode is user. Its only purpose is to save stack space in the
    caller. 

    N.B. This routine assumes that the caller has protected access to the
       specified context record.

Arguments:

    ContextRecord - Supplies a pointer to a context record.

    ExceptionFrame - Supplies a pointer to an exception frame.

    TrapFrame - Supplies a pointer to a trap frame.

Return Value:

    None.

--*/

{

    CONTEXT ContextRecord2;

    //
    // Copy the context record to a stack local context record.
    //
    // N.B. The context record has already been probed for read
    //      access.
    //

    RtlCopyMemory(&ContextRecord2, ContextRecord, sizeof(CONTEXT));

    //
    // Move information from the context record to the exception and trap
    // frames.
    //

    KeContextToKframes(TrapFrame,
                       ExceptionFrame,
                       &ContextRecord2,
                       ContextRecord2.ContextFlags,
                       UserMode);

    return;
}

NTSTATUS
KiContinueEx (
    IN PCONTEXT ContextRecord,
    IN BOOLEAN TestAlert,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This function is called to copy the specified context frame to the
    specified exception and trap frames for the continue system service.

    N.B. If the previous mode is user mode, alerts are being tested, a user
         APC is available to execute, and the specified context record was
         previously placed on the stack by the user APC initialization routine,
         then bypass context record copy to the kernel frames and back again.

Arguments:

    ContextRecord - Supplies a pointer to a context record.

    TestAlert - Supplies a boolean value that determines whether a test alert
       is to be performed.

    ExceptionFrame - Supplies a pointer to an exception frame.

    TrapFrame - Supplies a pointer to a trap frame.

Return Value:

    STATUS_ACCESS_VIOLATION is returned if the context record is not readable
        from user mode.

    STATUS_DATATYPE_MISALIGNMENT is returned if the context record is not
        properly aligned.

    STATUS_SUCCESS + 1 is returned if the context frame is copied successfully
        to the specified exception and trap frames.

    STATUS_SUCCESS is returned if the context record copy was bypassed and
        another user APC is ready to be delivered.

--*/

{

    KIRQL OldIrql;
    NTSTATUS Status;
    PKTHREAD Thread;
    ULONG64 UserStack;

    //
    // Synchronize with other context operations.
    //
    // If the current IRQL is less than APC_LEVEL, then raise IRQL to APC level.
    // 

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < APC_LEVEL) {
        KfRaiseIrql(APC_LEVEL);
    }

    //
    // If the previous mode was not kernel mode, then attempt to bypass
    // the copy of the context record. Otherwise, copy context to kernel
    // frames directly.
    // 
      
    Status = STATUS_SUCCESS + 1;
    Thread = KeGetCurrentThread();
    if (Thread->PreviousMode != KernelMode) {
        try {
            if (TestAlert != FALSE) {
                ProbeForWriteSmallStructure(ContextRecord,
                                            sizeof(CONTEXT),
                                            CONTEXT_ALIGN);

                KeTestAlertThread(UserMode);
                UserStack = (ContextRecord->Rsp  - sizeof(MACHINE_FRAME)) & ~STACK_ROUND;
                if (((UserStack - sizeof(CONTEXT)) == (ULONG64)ContextRecord) &&
                    (Thread->ApcState.UserApcPending != FALSE)) {

                    //
                    // Save the context record and exception frame addresses
                    // in the trap frame and deliver the user APC bypassing
                    // the context copy.
                    //

                    TrapFrame->ContextRecord = (ULONG64)ContextRecord;
                    TrapFrame->ExceptionFrame = (ULONG64)ExceptionFrame;
                    KiDeliverApc(UserMode, NULL, TrapFrame);
                    Status = STATUS_SUCCESS;
                    leave;
                }

            } else {
                ProbeForReadSmallStructure(ContextRecord,
                                           sizeof(CONTEXT),
                                           CONTEXT_ALIGN);
            }

            KiContinuePreviousModeUser(ContextRecord,
                                       ExceptionFrame,
                                       TrapFrame);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }

    } else {
        KeContextToKframes(TrapFrame,
                           ExceptionFrame,
                           ContextRecord,
                           ContextRecord->ContextFlags,
                           KernelMode);

        if (TestAlert != FALSE) {
            KeTestAlertThread(KernelMode);
        }
    }

    //
    // If the old IRQL was less than APC level, then lower the IRQL to its
    // previous value.
    //

    if (OldIrql < APC_LEVEL) {
        KeLowerIrql(OldIrql);
    }

    return Status;
}

VOID
KiInitializeUserApc (
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN PKNORMAL_ROUTINE NormalRoutine,
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

/*++

Routine Description:

    This function is called to initialize the context for a user mode APC.

Arguments:

    ExceptionFrame - Supplies a pointer to an exception frame.

    TrapFrame - Supplies a pointer to a trap frame.

    NormalRoutine - Supplies a pointer to the user mode APC routine.

    NormalContext - Supplies a pointer to the user context for the APC
        routine.

    SystemArgument1 - Supplies the first system supplied value.

    SystemArgument2 - Supplies the second system supplied value.

Return Value:

    None.

--*/

{

    PCONTEXT ContextRecord;
    EXCEPTION_RECORD ExceptionRecord;
    PMACHINE_FRAME MachineFrame;

    //
    // Transfer the context information to the user stack, initialize the
    // APC routine parameters, and modify the trap frame so execution will
    // continue in user mode at the user mode APC dispatch routine.
    //

    try {

        //
        // If the exception frame address is NULL, then the context copy
        // has been bypassed and the context is already on the user stack.
        //

        if (ExceptionFrame == NULL) {

            //
            // The address of the already copied context record and the real
            // exception frame address are passed via unused fields in the
            // trap frame.
            //
            // N.B. The context record has been probed for write access.
            //

            ContextRecord = (PCONTEXT)TrapFrame->ContextRecord;
            ExceptionFrame = (PKEXCEPTION_FRAME)TrapFrame->ExceptionFrame;

        } else {

            //
            // Compute address of aligned machine frame, compute address of
            // context record, and probe user stack for writeability.
            //
    
            MachineFrame =
                (PMACHINE_FRAME)((TrapFrame->Rsp - sizeof(MACHINE_FRAME)) & ~STACK_ROUND);
    
            ContextRecord = (PCONTEXT)((ULONG64)MachineFrame - sizeof(CONTEXT));
            ProbeForWriteSmallStructure(ContextRecord,
                                        sizeof(MACHINE_FRAME) + CONTEXT_LENGTH,
                                        STACK_ALIGN);
    
            //
            // Move machine state from trap and exception frames to the context
            // record on the user stack.
            //
    
            ContextRecord->ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
            KeContextFromKframes(TrapFrame, ExceptionFrame, ContextRecord);

            //
            // Fill in machine frame information.
            //
    
            MachineFrame->Rsp = ContextRecord->Rsp;
            MachineFrame->Rip = ContextRecord->Rip;
        }

        //
        // Initialize the user APC parameters.
        //

        ContextRecord->P1Home = (ULONG64)NormalContext;
        ContextRecord->P2Home = (ULONG64)SystemArgument1;
        ContextRecord->P3Home = (ULONG64)SystemArgument2;
        ContextRecord->P4Home = (ULONG64)NormalRoutine;

        //
        // Set the address new stack pointer in the current trap frame and
        // the continuation address so control will be transferred to the user
        // APC dispatcher.
        //

        TrapFrame->Rsp = (ULONG64)ContextRecord;
        TrapFrame->Rip = (ULONG64)KeUserApcDispatcher;

    } except (KiCopyInformation(&ExceptionRecord,
                                (GetExceptionInformation())->ExceptionRecord)) {

        //
        // Lower the IRQL to PASSIVE_LEVEL, set the exception address to
        // the current program address, and raise an exception by calling
        // the exception dispatcher.
        //
        // N.B. The IRQL is lowered to PASSIVE_LEVEL to allow APC interrupts
        //      during the dispatching of the exception. The current thread
        //      will be terminated during the dispatching of the exception,
        //      but lowering of the IRQL is required to enable the debugger
        //      to obtain the context of the current thread.
        //

        KeLowerIrql(PASSIVE_LEVEL);
        ExceptionRecord.ExceptionAddress = (PVOID)(TrapFrame->Rip);
        KiDispatchException(&ExceptionRecord,
                            ExceptionFrame,
                            TrapFrame,
                            UserMode,
                            TRUE);
    }

    return;
}

