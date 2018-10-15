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

    EXCEPTION_RECORD ExceptionRecord;
    CONTEXT ContextFrame;
    LONG Length;
    ULONG UserStack, TopOfStack;
    PKAPC_RECORD ApcRecord;

    //
    // APCs are not defined for V86 mode; however, it is possible a
    // thread is trying to set it's context to V86 mode - this isn't
    // going to work, but we don't want to crash the system so we
    // check for the possibility before hand.
    //

    if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
        return ;
    }

    //
    // Move machine state from trap and exception frames to the context frame.
    //

    ContextFrame.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;
    KeContextFromKframes(TrapFrame, ExceptionFrame, &ContextFrame);

    //
    // Transfer the context information to the user stack, initialize the
    // APC routine parameters, and modify the trap frame so execution will
    // continue in user mode at the user mode APC dispatch routine.
    //


    try {        
        C_ASSERT (CONTEXT_ALIGN >= __alignof(EXCEPTION_REGISTRATION_RECORD));
        C_ASSERT (CONTEXT_ALIGN >= __alignof(KAPC_RECORD));

        ASSERT ((TrapFrame->SegCs & MODE_MASK) != KernelMode); // Assert usermode frame

        //
        // Compute length of context record and new aligned user stack pointer.
        // Make sure to include space for a double-word aligned exception registration
        // record as well. For compatibility the exception registration must follow the 
        // context record.
        //

        TopOfStack = (ContextFrame.Esp & ~(__alignof(EXCEPTION_REGISTRATION_RECORD)-1));
        Length = CONTEXT_ALIGNED_SIZE + sizeof(KAPC_RECORD);
        UserStack = ((TopOfStack - sizeof (EXCEPTION_REGISTRATION_RECORD)) & ~CONTEXT_ROUND) - 
            Length;

        //
        // Probe user stack area for writability and then transfer the context 
        // record to the user stack. Note the minimum alignment is used as
        // the above code ensures the proper alignment of the addresses.
        //

        ProbeForWrite ((PCHAR)UserStack, TopOfStack-UserStack, 1);
        NT_ASSERT (((ULONG_PTR)(UserStack + sizeof(KAPC_RECORD)) & CONTEXT_ROUND) == 0);
        RtlCopyMemory ((PULONG)(UserStack + sizeof(KAPC_RECORD)),
                     &ContextFrame, sizeof(CONTEXT));

        //
        // Force correct R3 selectors into TrapFrame.
        //

        TrapFrame->SegCs = SANITIZE_SEG(KGDT_R3_CODE, UserMode);
        TrapFrame->HardwareSegSs = SANITIZE_SEG(KGDT_R3_DATA, UserMode);
        TrapFrame->SegDs = SANITIZE_SEG(KGDT_R3_DATA, UserMode);
        TrapFrame->SegEs = SANITIZE_SEG(KGDT_R3_DATA, UserMode);
        TrapFrame->SegFs = SANITIZE_SEG(KGDT_R3_TEB, UserMode);
        TrapFrame->SegGs = 0;
        TrapFrame->EFlags = SANITIZE_FLAGS( ContextFrame.EFlags, UserMode );

        //
        // If thread is supposed to have IOPL, then force it on in eflags
        //

        if (KeGetCurrentThread()->Iopl) {
            TrapFrame->EFlags |= (EFLAGS_IOPL_MASK & -1);  // IOPL = 3
        }

        //
        // Set the address of the user APC routine, the APC parameters, the
        // new frame pointer, and the new stack pointer in the current trap
        // frame. Set the continuation address so control will be transferred
        // to the user APC dispatcher.
        //

        TrapFrame->HardwareEsp = UserStack;
        TrapFrame->Eip = (ULONG)KeUserApcDispatcher;
        TrapFrame->ErrCode = 0;

        ApcRecord = (PKAPC_RECORD)UserStack;
        ApcRecord->NormalRoutine = NormalRoutine;
        ApcRecord->NormalContext = NormalContext;
        ApcRecord->SystemArgument1 = SystemArgument1;
        ApcRecord->SystemArgument2 = SystemArgument2;
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
        ExceptionRecord.ExceptionAddress = (PVOID)(TrapFrame->Eip);
        KiDispatchException(&ExceptionRecord,
                            ExceptionFrame,
                            TrapFrame,
                            UserMode,
                            TRUE);

    }
    return;
}

