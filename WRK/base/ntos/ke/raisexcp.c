/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    raisexcp.c

Abstract:

    This module implements the internal kernel code to continue execution
    and raise a exception.

--*/

#include "ki.h"

#if !defined(_AMD64_)

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
    // Probe and copy the context record to a stack local context record.
    //

    ProbeForReadSmallStructure(ContextRecord, sizeof(CONTEXT), CONTEXT_ALIGN);
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
KiContinue (
    IN PCONTEXT ContextRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame
    )

/*++

Routine Description:

    This function is called to copy the specified context frame to the
    specified exception and trap frames for the continue system service.

Arguments:

    ContextRecord - Supplies a pointer to a context record.

    ExceptionFrame - Supplies a pointer to an exception frame.

    TrapFrame - Supplies a pointer to a trap frame.

Return Value:

    STATUS_ACCESS_VIOLATION is returned if the context record is not readable
        from user mode.

    STATUS_DATATYPE_MISALIGNMENT is returned if the context record is not
        properly aligned.

    STATUS_SUCCESS is returned if the context frame is copied successfully
        to the specified exception and trap frames.

--*/

{

    KIRQL OldIrql;
    NTSTATUS Status;

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
    // If the previous mode was not kernel mode, then use wrapper function
    // to copy context to kernel frames. Otherwise, copy context to kernel
    // frames directly.
    // 
      
    Status = STATUS_SUCCESS;
    if (KeGetPreviousMode() != KernelMode) {
        try {
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

#endif

NTSTATUS
KiRaiseException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN BOOLEAN FirstChance
    )

/*++

Routine Description:

    This function is called to raise an exception. The exception can be
    raised as a first or second chance exception.

Arguments:

    ExceptionRecord - Supplies a pointer to an exception record.

    ContextRecord - Supplies a pointer to a context record.

    ExceptionFrame - Supplies a pointer to an exception frame.

    TrapFrame - Supplies a pointer to a trap frame.

    FirstChance - Supplies a boolean value that specifies whether this is
        the first (TRUE) or second (FALSE) chance for the exception.

Return Value:

    STATUS_ACCESS_VIOLATION is returned if either the exception or the context
        record is not readable from user mode.

    STATUS_DATATYPE_MISALIGNMENT is returned if the exception record or the
        context record are not properly aligned.

    STATUS_INVALID_PARAMETER is returned if the number of exception parameters
        is greater than the maximum allowable number of exception parameters.

    STATUS_SUCCESS is returned if the exception is dispatched and handled.

--*/

{

    CONTEXT ContextRecord2;
    EXCEPTION_RECORD ExceptionRecord2;
    ULONG Length;
    ULONG Parameters;
    KPROCESSOR_MODE PreviousMode;

    //
    // Get the previous processor mode and probe the specified exception and
    // context records if necessary.
    //

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {
        try {
            ProbeForReadSmallStructure(ContextRecord,
                                       sizeof(CONTEXT),
                                       sizeof(UCHAR));
    
            Parameters = ProbeAndReadUlong(&ExceptionRecord->NumberParameters);
            if (Parameters > EXCEPTION_MAXIMUM_PARAMETERS) {
                return STATUS_INVALID_PARAMETER;
            }
    
            //
            // The exception record structure is defined unlike others with
            // trailing information as being its maximum size rather than
            // just a single trailing element.
            //
    
            Length = FIELD_OFFSET(EXCEPTION_RECORD, ExceptionInformation[Parameters]);

            __assume(Length != 0);

            ProbeForRead(ExceptionRecord, Length, sizeof(UCHAR));
    
            //
            // Copy the exception and context record to local storage so an
            // access violation cannot occur during exception dispatching.
            //
    
            RtlCopyMemory(&ContextRecord2, ContextRecord, sizeof(CONTEXT));
            RtlCopyMemory(&ExceptionRecord2, ExceptionRecord, Length);
            ContextRecord = &ContextRecord2;
            ExceptionRecord = &ExceptionRecord2;
    
            //
            // Make sure the number of parameter is correct in the copied
            // exception record.
            //
    
            ExceptionRecord->NumberParameters = Parameters;
    
        } except(EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }

    //
    // Move information from the context record to the exception and trap
    // frames.
    //

    KeContextToKframes(TrapFrame,
                       ExceptionFrame,
                       ContextRecord,
                       ContextRecord->ContextFlags,
                       PreviousMode);

    //
    // Make sure the reserved bit is clear in the exception code and
    // perform exception dispatching.
    //
    // N.B. The reserved bit is used to differentiate internally generated
    //      codes from codes generated by application programs.
    //

    ExceptionRecord->ExceptionCode &= ~KI_EXCEPTION_INTERNAL;
    KiDispatchException(ExceptionRecord,
                        ExceptionFrame,
                        TrapFrame,
                        PreviousMode,
                        FirstChance);

    return STATUS_SUCCESS;
}

