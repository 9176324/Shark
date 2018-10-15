/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    thredini.c

Abstract:

    This module implements the machine dependent function to set the initial
    context and data alignment handling mode for a process or thread object.

--*/

#include "ki.h"

VOID
KiInitializeContextThread (
    IN PKTHREAD Thread,
    IN PKSYSTEM_ROUTINE SystemRoutine,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext OPTIONAL,
    IN PCONTEXT ContextRecord OPTIONAL
    )

/*++

Routine Description:

    This function initializes the machine dependent context of a thread
    object.

    N.B. This function does not check if context record is accessible.
         It is assumed the the caller of this routine is either prepared
         to handle access violations or has probed and copied the context
         record as appropriate.

Arguments:

    Thread - Supplies a pointer to a dispatcher object of type thread.

    SystemRoutine - Supplies a pointer to the system function that is to be
        called when the thread is first scheduled for execution.

    StartRoutine - Supplies an optional pointer to a function that is to be
        called after the system has finished initializing the thread. This
        parameter is specified if the thread is a system thread and will
        execute totally in kernel mode.

    StartContext - Supplies an optional pointer to a data structure that
        will be passed to the StartRoutine as a parameter. This parameter
        is specified if the thread is a system thread and will execute
        totally in kernel mode.

    ContextRecord - Supplies an optional pointer a context record which
        contains the initial user mode state of the thread. This parameter
        is specified if the thread will execute in user mode.

Return Value:

    None.

--*/

{

    CONTEXT ContextFrame;
    PKEXCEPTION_FRAME ExFrame;
    ULONG64 InitialStack;
    PXMM_SAVE_AREA32 NpxFrame;
    PKSTART_FRAME SfFrame;
    PKERNEL_STACK_CONTROL StackControl;
    PKSWITCH_FRAME SwFrame;
    PKTRAP_FRAME TrFrame;

    //
    // Allocate a legacy floating point save area at the base of the thread
    // stack and record the initial stack as this address. All threads have
    // a legacy floating point save are to avoid special cases in the context
    // switch code.
    //

    InitialStack = (ULONG64)Thread->InitialStack;
    NpxFrame = (PXMM_SAVE_AREA32)(InitialStack - KERNEL_STACK_CONTROL_LENGTH);
    RtlZeroMemory(NpxFrame, KERNEL_STACK_CONTROL_LENGTH);

    //
    // Initialize the current kernel stack segment descriptor in the kernel
    // stack control area. This descriptor is used to control kernel stack
    // expansion from drivers.
    //
    // N.B. The previous stack segment descriptor is zeroed.
    //

    StackControl = (PKERNEL_STACK_CONTROL)NpxFrame;
    StackControl->Current.StackBase = InitialStack;
    StackControl->Current.ActualLimit = InitialStack - KERNEL_STACK_SIZE;

    //
    // If a context record is specified, then initialize a trap frame, and
    // an exception frame with the specified user mode context.
    //
    // N.B. The initial context of a thread cannot set the debug or floating
    //      state.
    //

    if (ARGUMENT_PRESENT(ContextRecord)) {
        ContextFrame = *ContextRecord;
        ContextRecord = &ContextFrame;
        ContextRecord->ContextFlags &= ~(CONTEXT_DEBUG_REGISTERS | CONTEXT_FLOATING_POINT);
        ContextRecord->ContextFlags |= CONTEXT_CONTROL;

        //
        // Allocate a trap frame, an exception frame, and a context switch
        // frame.
        //

        TrFrame = (PKTRAP_FRAME)(((ULONG64)NpxFrame - KTRAP_FRAME_LENGTH));
        ExFrame = (PKEXCEPTION_FRAME)(((ULONG64)TrFrame - KEXCEPTION_FRAME_LENGTH));
        SwFrame = (PKSWITCH_FRAME)(((ULONG64)ExFrame - KSWITCH_FRAME_LENGTH));

        //
        // Set CS and SS for user mode 64-bit execution in the machine frame.
        //

        ContextRecord->SegCs = KGDT64_R3_CODE | RPL_MASK;
        ContextRecord->SegSs = KGDT64_R3_DATA | RPL_MASK;

        //
        // The main entry point for the user thread will be jumped to via a
        // continue operation from the user APC dispatcher. Therefore, the
        // user stack must be initialized to an 8 mod 16 boundary.
        //
        // In addition, we must have room for the home addresses for the
        // first four parameters.
        //

        ContextRecord->Rsp =
            (ContextRecord->Rsp & ~STACK_ROUND) - ((4 * 8) + 8);

        //
        // Zero the exception and trap frames and copy information from the
        // specified context frame to the trap and exception frames.
        //
        // N.B. The function that performs the context to kernel frames does
        //      not initialize the legacy floating context.
        //

        RtlZeroMemory(ExFrame, sizeof(KEXCEPTION_FRAME));
        RtlZeroMemory(TrFrame, sizeof(KTRAP_FRAME));
        KxContextToKframes(TrFrame,
                           ExFrame,
                           ContextRecord,
                           ContextRecord->ContextFlags,
                           UserMode);

        //
        // Initialize user thread startup information.
        //

        ExFrame->P1Home = (ULONG64)StartContext;
        ExFrame->P2Home = (ULONG64)StartRoutine;
        ExFrame->P3Home = (ULONG64)SystemRoutine;
        ExFrame->Return = (ULONG64)KiStartUserThreadReturn;

        //
        // Initialize start address.
        //

        SwFrame->Return = (ULONG64)KiStartUserThread;

        //
        // Set the initial legacy floating point control/tag word state and
        // the XMM control/status state.
        //

        NpxFrame->ControlWord = INITIAL_FPCSR;
        NpxFrame->MxCsr = INITIAL_MXCSR;
        TrFrame->MxCsr = INITIAL_MXCSR;

        //
        // Set legacy floating point state to switch.
        //

        Thread->NpxState = LEGACY_STATE_SWITCH;

        //
        // Set the saved previous processor mode in the thread object.
        //

        Thread->PreviousMode = UserMode;

    } else {

        //
        // Allocate an exception frame and a context switch frame.
        //

        TrFrame = NULL;
        SfFrame = (PKSTART_FRAME)(((ULONG64)NpxFrame - KSTART_FRAME_LENGTH));
        SwFrame = (PKSWITCH_FRAME)(((ULONG64)SfFrame - KSWITCH_FRAME_LENGTH));

        //
        // Initialize the system thread start frame.
        //

        SfFrame->P1Home = (ULONG64)StartContext;
        SfFrame->P2Home = (ULONG64)StartRoutine;
        SfFrame->P3Home = (ULONG64)SystemRoutine;
        SfFrame->Return = 0;

        //
        // Initialize start address.
        //

        SwFrame->Return = (ULONG64)KiStartSystemThread;

        //
        // Set legacy floating point state to unused.
        //

        Thread->NpxState = LEGACY_STATE_UNUSED;

        //
        // Set the previous mode in thread object to kernel.
        //

        Thread->PreviousMode = KernelMode;
    }

    //
    // Initialize context switch frame and set thread start up parameters.
    //

    SwFrame->ApcBypass = APC_LEVEL;
    SwFrame->Rbp = (ULONG64)TrFrame + 128;

    //
    // Set the initial kernel stack pointer.
    //

    Thread->InitialStack = (PVOID)NpxFrame;
    Thread->KernelStack = SwFrame;
    return;
}

