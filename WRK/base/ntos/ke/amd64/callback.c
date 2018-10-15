/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    callback.c

Abstract:

    This module implements user mode call back services.

--*/

#include "ki.h"

#pragma alloc_text(PAGE, KeUserModeCallback)

VOID
KeCheckIfStackExpandCalloutActive (
    VOID
    )

/*++

Routine Description:

    This function check whether a kernel stack expand callout is active for
    the current thread and bugchecks if the result is positive.

Arguments:

    None.

Return Value:

    None.

--*/

{

    PKERNEL_STACK_CONTROL Control;
    PKTHREAD Thread;

    Thread = KeGetCurrentThread();
    Control = (PKERNEL_STACK_CONTROL)Thread->InitialStack;
    if (Control->Previous.StackBase != 0) {
        KeBugCheckEx(KERNEL_EXPAND_STACK_ACTIVE, (ULONG64)Thread, 0, 0, 0);
    }

    return;
}

NTSTATUS
KeUserModeCallback (
    IN ULONG ApiNumber,
    IN PVOID InputBuffer,
    IN ULONG InputLength,
    OUT PVOID *OutputBuffer,
    IN PULONG OutputLength
    )

/*++

Routine Description:

    This function call out from kernel mode to a user mode function.

Arguments:

    ApiNumber - Supplies the API number.

    InputBuffer - Supplies a pointer to a structure that is copied
        to the user stack.

    InputLength - Supplies the length of the input structure.

    Outputbuffer - Supplies a pointer to a variable that receives
        the address of the output buffer.

    Outputlength - Supplies a pointer to a variable that receives
        the length of the output buffer.

Return Value:

    If the callout cannot be executed, then an error status is returned.
    Otherwise, the status returned by the callback function is returned.

--*/

{

    volatile ULONG BatchCount;
    PUCALLOUT_FRAME CalloutFrame;
    ULONG Length;
    ULONG64 OldStack;
    NTSTATUS Status;
    PKTRAP_FRAME TrapFrame;

    ASSERT(KeGetPreviousMode() == UserMode);

    //
    // Check if a kernel stack expand callout is active.
    //

    KeCheckIfStackExpandCalloutActive();

    //
    // Get the user mode stack pointer and attempt to copy input buffer
    // to the user stack.
    //

    TrapFrame = KeGetCurrentThread()->TrapFrame;
    OldStack = TrapFrame->Rsp;
    try {

        //
        // Compute new user mode stack address, probe for writability, and
        // copy the input buffer to the user stack.
        //

        Length = ((InputLength + STACK_ROUND) & ~STACK_ROUND) + UCALLOUT_FRAME_LENGTH;
        CalloutFrame = (PUCALLOUT_FRAME)((OldStack - Length) & ~STACK_ROUND);
        ProbeForWrite(CalloutFrame, Length, STACK_ALIGN);
        RtlCopyMemory(CalloutFrame + 1, InputBuffer, InputLength);

        //
        // Fill in callout arguments.
        //

        CalloutFrame->Buffer = (PVOID)(CalloutFrame + 1);
        CalloutFrame->Length = InputLength;
        CalloutFrame->ApiNumber = ApiNumber;
        CalloutFrame->MachineFrame.Rsp = OldStack;
        CalloutFrame->MachineFrame.Rip = TrapFrame->Rip;

    //
    // If an exception occurs during the probe of the user stack, then
    // always handle the exception and return the exception code as the
    // status value.
    //

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    //
    // Call user mode.
    //

    TrapFrame->Rsp = (ULONG64)CalloutFrame;
    Status = KiCallUserMode(OutputBuffer, OutputLength);

    //
    // When returning from user mode, any drawing done to the GDI TEB
    // batch must be flushed.
    //
    // N.B. It is possible to fault while referencing the user TEB. If
    //      a fault occurs, then always flush the batch count.
    //

    BatchCount = 1;
    try {
        BatchCount = ((PTEB)KeGetCurrentThread()->Teb)->GdiBatchCount;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }   

    if (BatchCount > 0) {
        TrapFrame->Rsp -= 256;
        KeGdiFlushUserBatch();
    }

    TrapFrame->Rsp = OldStack;
    return Status;
}

