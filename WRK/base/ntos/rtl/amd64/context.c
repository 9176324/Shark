/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    context.c

Abstract:

    This module implements user mode callable context manipulation routines.
    The interfaces exported from this module are portable, but they must
    be re-implemented for each architecture.

--*/

#include "ntrtlp.h"

#pragma alloc_text(PAGE, RtlInitializeContext)
#pragma alloc_text(PAGE, RtlRemoteCall)

VOID
RtlInitializeContext(
    IN HANDLE Process,
    OUT PCONTEXT Context,
    IN PVOID Parameter OPTIONAL,
    IN PVOID InitialPc OPTIONAL,
    IN PVOID InitialSp OPTIONAL
    )

/*++

Routine Description:

    This function initializes a context record so that it can be used in a
    subsequent call to create thread.

Arguments:

    Process - Supplies a handle to the process in which a thread is being
        created.

    Context - Supplies a pointer to a context record.

    InitialPc - Supplies an initial program counter value.

    InitialSp - Supplies an initial stack pointer value.

Return Value:

    STATUS_BAD_INITIAL_STACK is raised if initial stack pointer value is not
    properly aligned.

--*/

{

    RTL_PAGED_CODE();

    //
    // Check stack alignment.
    //

    if (((ULONG64)InitialSp & 0xf) != 0) {
        RtlRaiseStatus(STATUS_BAD_INITIAL_STACK);
    }

    //
    // Initialize the EFflags field.
    //

    Context->EFlags = EFLAGS_IF_MASK;

    //
    // Initialize the integer registers.
    //

    Context->Rax = 0L;
    Context->Rcx = 2L;
    Context->Rbx = 1L;
    Context->Rsp = (ULONG64)InitialSp;
    Context->Rbp = 0L;
    Context->Rsi = 4L;
    Context->Rdi = 5L;
    Context->R8 = 8;
    Context->R9 = 9;
    Context->R10 = 10;
    Context->R11 = 11;
    Context->R12 = 12;
    Context->R13 = 13;
    Context->R14 = 14;
    Context->R15 = 15;

    //
    // Initialize the floating point state.
    //
    // N.B. This includes both the legacy and extended floating state.
    //

    RtlZeroMemory(&Context->FltSave, sizeof(XMM_SAVE_AREA32));
    Context->MxCsr = INITIAL_MXCSR;
    Context->FltSave.ControlWord = INITIAL_FPCSR;
    Context->FltSave.MxCsr = INITIAL_MXCSR;

    //
    // Initialize the program counter.
    //

    Context->Rip = (ULONG64)InitialPc;

    //
    // Set context record flags.
    //

    Context->ContextFlags = CONTEXT_FULL;

    //
    // Set the initial context of the thread in a machine specific way.
    //

    Context->Rcx = (ULONG64)Parameter;

    //
    // Unique stamp to tell wow64 that this is an RtlCreateUserThread context
    // rather than BaseCreateThread context.
    //

    Context->R9 = 0xf0e0d0c0a0908070UI64;
    return;
}

NTSTATUS
RtlRemoteCall (
    HANDLE Process,
    HANDLE Thread,
    PVOID CallSite,
    ULONG ArgumentCount,
    PULONG_PTR Arguments,
    BOOLEAN PassContext,
    BOOLEAN AlreadySuspended
    )

/*++

Routine Description:

    This function calls a procedure in another thread/process using the
    system functions get and set context. Parameters are passed to the
    target procedure via the nonvolatile registers.

Arguments:

    Process - Supplies an open handle to the target process.

    Thread - Supplies an open handle to the target thread within the target
        process.

    CallSite - Supplies the address of the procedure to call in the target
        process.

    ArgumentCount - Supplies the number of parameters to pass to the target
        procedure.

    Arguments - Supplies a pointer to the array of parameters to pass.

    PassContext - Supplies a boolean value that determines whether a parameter
        is to be passed that points to a context record.

    AlreadySuspended - Supplies a boolean value that determines whether the
        target thread is already in a suspended or waiting state.

Return Value:

    Status - Status value

--*/

{

    CONTEXT Context;
    ULONG Index;
    ULONG64 NewSp;
    NTSTATUS Status;

    RTL_PAGED_CODE();

    //
    // Check if too many arguments are specified.
    //

    if (ArgumentCount > 4) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // If necessary, suspend the target thread before getting the thread's
    // current state.
    //

    if (AlreadySuspended == FALSE) {
        Status = NtSuspendThread(Thread, NULL);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    //
    // Get the current context of the target thread.
    //

    Context.ContextFlags = CONTEXT_FULL;
    Status = NtGetContextThread(Thread, &Context);
    if (!NT_SUCCESS(Status)) {
        if (AlreadySuspended == FALSE) {
            NtResumeThread(Thread, NULL);
        }

        return Status;
    }

    if (AlreadySuspended != FALSE) {
        Context.Rax = STATUS_ALERTED;
    }

    //
    // Write previous thread context into the stack of the target thread.
    //

    NewSp = Context.Rsp - sizeof(CONTEXT);
	Status = NtWriteVirtualMemory(Process,
				                  (PVOID)NewSp,
				                  &Context,
				                  sizeof(CONTEXT),
				                  NULL);

	if (!NT_SUCCESS(Status)) {
        if (AlreadySuspended == FALSE) {
            NtResumeThread(Thread, NULL);
        }

	    return Status;
	}

    //
    // Pass the parameters to the target thread via the nonvolatile registers
    // R11-R15.
    //

    Context.Rsp = NewSp;
    if (PassContext != FALSE) {
        Context.R11 = NewSp;
        for (Index = 0; Index < ArgumentCount; Index += 1) {
            (&Context.R12)[Index] = Arguments[Index];
        }

    } else {
        for (Index = 0; Index < ArgumentCount; Index += 1) {
            (&Context.R11)[Index] = Arguments[Index];
        }
    }

    //
    // Set the address of the target code into RIP and set the thread context
    // to cause the target procedure to be executed.
    //

    Context.Rip = (ULONG64)CallSite;
    Status = NtSetContextThread(Thread, &Context);
    if (AlreadySuspended == FALSE) {
        NtResumeThread(Thread, NULL);
    }

    return Status;
}

