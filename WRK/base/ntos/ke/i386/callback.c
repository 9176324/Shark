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

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, KeUserModeCallback)
#endif


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

    If the callout cannot be executed, then an error status is
    returned. Otherwise, the status returned by the callback function
    is returned.

--*/

{

    ULONG Length;
    ULONG NewStack;
    ULONG OldStack;
    NTSTATUS Status;
    PULONG UserStack;
    ULONG GdiBatchCount;
    PEXCEPTION_REGISTRATION_RECORD ExceptionList;
    PTEB Teb;

    ASSERT(KeGetPreviousMode() == UserMode);
    ASSERT(KeGetCurrentThread()->ApcState.KernelApcInProgress == FALSE);
    
    //
    // Get the user mode stack pointer and attempt to copy input buffer
    // to the user stack.
    //

    UserStack = KiGetUserModeStackAddress ();
    OldStack = *UserStack;
    
    try {

        //
        // Compute new user mode stack address, probe for writability,
        // and copy the input buffer to the user stack. Leave space for an
        // (doubleword-aligned) exception handler at the top of the callback 
        // stack frame.
        //

        C_ASSERT (__alignof (ULONG) == __alignof (EXCEPTION_REGISTRATION_RECORD));
     
        NewStack = (OldStack - InputLength) & ~(__alignof(EXCEPTION_REGISTRATION_RECORD) - 1);
        Length = 4*sizeof(ULONG) + sizeof(EXCEPTION_REGISTRATION_RECORD);
        ProbeForWrite ((PCHAR)(NewStack - Length), Length + InputLength, sizeof(CHAR));
        RtlCopyMemory ((PVOID)NewStack, InputBuffer, InputLength);
 
        //
        // Push arguments onto user stack. Note space remains for the exception
        // registration record following the callback function arguments.
        //

        NewStack -= Length;
        *((PULONG)NewStack) = 0;
        *(((PULONG)NewStack) + 1) = ApiNumber;
        *(((PULONG)NewStack) + 2) = (ULONG)(NewStack+Length);
        *(((PULONG)NewStack) + 3) = (ULONG)InputLength;

        //
        // Save the exception list in case another handler is defined during
        // the callout.
        //

        Teb = (PTEB) KeGetCurrentThread()->Teb;
        ExceptionList = Teb->NtTib.ExceptionList;

        //
        // Call user mode.
        //

        *UserStack = NewStack;
        Status = KiCallUserMode(OutputBuffer, OutputLength);

        //
        // Restore the exception list, unless a user mode unwind is in progress.
        //

        if (Status != STATUS_CALLBACK_POP_STACK) {
            Teb->NtTib.ExceptionList = ExceptionList;
        } else {

            //
            // In this case, make the restore of the user stack pointer effectively
            // a NOP.
            //

            OldStack = *UserStack;
        }

        //
        // If an exception occurs during the probe of the user stack, then
        // always handle the exception and return the exception code as the
        // status value.
        //

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode ();
    }

    //
    // When returning from user mode, any drawing done to the GDI TEB
    // batch must be flushed.  If the TEB cannot be accessed then blindly
    // flush the GDI batch anyway.
    //

    GdiBatchCount = 1;

    try {
        GdiBatchCount = Teb->GdiBatchCount;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    if (GdiBatchCount > 0) {

        //
        // call GDI batch flush routine
        //

        *UserStack -= 256;
        KeGdiFlushUserBatch ();
    }

    *UserStack = OldStack;
    return Status;
}

