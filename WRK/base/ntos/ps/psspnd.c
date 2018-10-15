/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    psspnd.c

Abstract:

    This module implements NtSuspendThread and NtResumeThread

--*/

#include "psp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtSuspendThread)
#pragma alloc_text(PAGE, NtResumeThread)
#pragma alloc_text(PAGE, NtAlertThread)
#pragma alloc_text(PAGE, NtAlertResumeThread)
#pragma alloc_text(PAGE, NtTestAlert)
#pragma alloc_text(PAGE, NtSuspendProcess)
#pragma alloc_text(PAGE, NtResumeProcess)

#pragma alloc_text(PAGE, PsSuspendThread)
#pragma alloc_text(PAGE, PsSuspendProcess)
#pragma alloc_text(PAGE, PsResumeProcess)
#pragma alloc_text(PAGE, PsResumeThread)
#endif

NTSTATUS
PsSuspendThread (
    IN PETHREAD Thread,
    OUT PULONG PreviousSuspendCount OPTIONAL
    )
/*++

Routine Description:

    This function suspends the target thread, and optionally
    returns the previous suspend count.

Arguments:

    ThreadHandle - Supplies a handle to the thread object to suspend.

    PreviousSuspendCount - An optional parameter, that if specified
        points to a variable that receives the thread's previous suspend
        count.

Return Value:

    NTSTATUS - Status of call

--*/
{
    NTSTATUS Status;
    ULONG LocalPreviousSuspendCount = 0;

    PAGED_CODE();

    if (Thread == PsGetCurrentThread ()) {
        try {
            LocalPreviousSuspendCount = (ULONG) KeSuspendThread (&Thread->Tcb);
            Status = STATUS_SUCCESS;
        } except ((GetExceptionCode () == STATUS_SUSPEND_COUNT_EXCEEDED)?
                     EXCEPTION_EXECUTE_HANDLER :
                     EXCEPTION_CONTINUE_SEARCH) {
            Status = GetExceptionCode();
        }
    } else {
        //
        // Protect the remote thread from being rundown.
        //
        if (ExAcquireRundownProtection (&Thread->RundownProtect)) {

            //
            // Don't allow suspend if we are being deleted
            //
            if (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_TERMINATED) {
                Status = STATUS_THREAD_IS_TERMINATING;
            } else {
                try {
                    LocalPreviousSuspendCount = (ULONG) KeSuspendThread (&Thread->Tcb);
                    Status = STATUS_SUCCESS;
                } except ((GetExceptionCode () == STATUS_SUSPEND_COUNT_EXCEEDED)?
                              EXCEPTION_EXECUTE_HANDLER :
                              EXCEPTION_CONTINUE_SEARCH) {
                    Status = GetExceptionCode();
                }
                //
                // If deletion was started after we suspended then wake up the thread
                //
                if (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_TERMINATED) {
                    KeForceResumeThread (&Thread->Tcb);
                    LocalPreviousSuspendCount = 0;
                    Status = STATUS_THREAD_IS_TERMINATING;
                }
            }
            ExReleaseRundownProtection (&Thread->RundownProtect);
        } else {
            Status = STATUS_THREAD_IS_TERMINATING;
        }
    }

    if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
        *PreviousSuspendCount = LocalPreviousSuspendCount;
    }
    return Status;
}

NTSTATUS
PsSuspendProcess (
    PEPROCESS Process
    )
/*++

Routine Description:

    This function suspends all the PS threads in a process.

Arguments:

    Process - Process whose threads are to be suspended

Return Value:

    NTSTATUS - Status of operation.

--*/
{
    NTSTATUS Status;
    PETHREAD Thread;

    PAGED_CODE ();


    if (ExAcquireRundownProtection (&Process->RundownProtect)) {

        for (Thread = PsGetNextProcessThread (Process, NULL);
             Thread != NULL;
             Thread = PsGetNextProcessThread (Process, Thread)) {

            PsSuspendThread (Thread, NULL);
        }

        ExReleaseRundownProtection (&Process->RundownProtect);

        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_PROCESS_IS_TERMINATING;
    }

    return Status;
}

NTSTATUS
PsResumeProcess (
    PEPROCESS Process
    )
/*++

Routine Description:

    This function resumes all the PS threads in a process.

Arguments:

    Process - Process whose threads are to be suspended

Return Value:

    NTSTATUS - Status of operation.

--*/
{
    NTSTATUS Status;
    PETHREAD Thread;

    PAGED_CODE ();

    if (ExAcquireRundownProtection (&Process->RundownProtect)) {

        for (Thread = PsGetNextProcessThread (Process, NULL);
             Thread != NULL;
             Thread = PsGetNextProcessThread (Process, Thread)) {

            KeResumeThread (&Thread->Tcb);
        }

        ExReleaseRundownProtection (&Process->RundownProtect);
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_PROCESS_IS_TERMINATING;
    }

    return Status;
}

NTSTATUS
NtSuspendThread(
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    )
    
/*++

Routine Description:

    This function suspends the target thread, and optionally
    returns the previous suspend count.

Arguments:

    ThreadHandle - Supplies a handle to the thread object to suspend.

    PreviousSuspendCount - An optional parameter, that if specified
        points to a variable that receives the thread's previous suspend
        count.

Return Value:

    NTSTATUS - Status of operation.

--*/

{
    PETHREAD Thread;
    NTSTATUS st;
    ULONG LocalPreviousSuspendCount;
    KPROCESSOR_MODE Mode;

    PAGED_CODE();

    Mode = KeGetPreviousMode ();

    try {

        if (Mode != KernelMode) {
            if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
                ProbeForWriteUlong (PreviousSuspendCount);
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

    st = ObReferenceObjectByHandle (ThreadHandle,
                                    THREAD_SUSPEND_RESUME,
                                    PsThreadType,
                                    Mode,
                                    &Thread,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    st = PsSuspendThread (Thread, &LocalPreviousSuspendCount);

    ObDereferenceObject (Thread);

    try {

        if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
            *PreviousSuspendCount = LocalPreviousSuspendCount;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        st = GetExceptionCode ();

    }

    return st;

}

NTSTATUS
PsResumeThread (
    IN PETHREAD Thread,
    OUT PULONG PreviousSuspendCount OPTIONAL
    )
/*++

Routine Description:

    This function resumes a thread that was previously suspended

Arguments:

    Thread - Thread to resume
    
    PreviousSuspendCount - Optional address of a ULONG to place the previous suspend count in

Return Value:

    NTSTATUS - Status of call

--*/
{
    ULONG LocalPreviousSuspendCount;

    PAGED_CODE();

    LocalPreviousSuspendCount = (ULONG) KeResumeThread (&Thread->Tcb);

    if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
        *PreviousSuspendCount = LocalPreviousSuspendCount;
    }
    
    return STATUS_SUCCESS;
}


NTSTATUS
NtResumeThread(
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    )

/*++

Routine Description:

    This function resumes a thread that was previously suspended

Arguments:

    ThreadHandle - Handle to thread to resume
    
    PreviousSuspendCount - Optional address of a ULONG to place the previous suspend count in

Return Value:

    NTSTATUS - Status of call

--*/

{
    PETHREAD Thread;
    NTSTATUS st;
    KPROCESSOR_MODE Mode;
    ULONG LocalPreviousSuspendCount;

    PAGED_CODE();

    Mode = KeGetPreviousMode ();

    try {

        if (Mode != KernelMode) {
            if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
                ProbeForWriteUlong (PreviousSuspendCount);
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode ();
    }

    st = ObReferenceObjectByHandle (ThreadHandle,
                                    THREAD_SUSPEND_RESUME,
                                    PsThreadType,
                                    Mode,
                                    &Thread,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    PsResumeThread (Thread, &LocalPreviousSuspendCount);

    ObDereferenceObject (Thread);

    try {
        if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
            *PreviousSuspendCount = LocalPreviousSuspendCount;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode ();
    }

    return STATUS_SUCCESS;

}

NTSTATUS
NtSuspendProcess (
    __in HANDLE ProcessHandle
    )
/*++

Routine Description:

    This function suspends all none-exiting threads in the target process

Arguments:

    ProcessHandle - Supplies an open handle to the process to be suspended

Return Value:

    NTSTATUS - Status of operation

--*/
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PEPROCESS Process;

    PAGED_CODE();


    PreviousMode = KeGetPreviousMode ();

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_PORT,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
    if (NT_SUCCESS (Status)) {
        Status = PsSuspendProcess (Process);
        ObDereferenceObject (Process);
    }

    return Status;
}

NTSTATUS
NtResumeProcess (
    __in HANDLE ProcessHandle
    )
/*++

Routine Description:

    This function suspends all none-exiting threads in the target process

Arguments:

    ProcessHandle - Supplies an open handle to the process to be suspended

Return Value:

    NTSTATUS - Status of operation

--*/
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PEPROCESS Process;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode ();

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_SET_PORT,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);
    if (NT_SUCCESS (Status)) {
        Status = PsResumeProcess (Process);
        ObDereferenceObject (Process);
    }

    return Status;
}

NTSTATUS
NtAlertThread(
    __in HANDLE ThreadHandle
    )

/*++

Routine Description:

    This function alerts the target thread using the previous mode
    as the mode of the alert.

Arguments:

    ThreadHandle - Supplies an open handle to the thread to be alerted

Return Value:

    NTSTATUS - Status of operation

--*/

{
    PETHREAD Thread;
    NTSTATUS st;
    KPROCESSOR_MODE Mode;

    PAGED_CODE();

    Mode = KeGetPreviousMode ();

    st = ObReferenceObjectByHandle (ThreadHandle,
                                    THREAD_ALERT,
                                    PsThreadType,
                                    Mode,
                                    &Thread,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    KeAlertThread (&Thread->Tcb,Mode);

    ObDereferenceObject (Thread);

    return STATUS_SUCCESS;

}


NTSTATUS
NtAlertResumeThread(
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    )

/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    NTSTATUS - Status of operation

--*/

{
    PETHREAD Thread;
    NTSTATUS st;
    ULONG LocalPreviousSuspendCount;
    KPROCESSOR_MODE Mode;

    PAGED_CODE();

    Mode = KeGetPreviousMode ();

    try {


        if (Mode != KernelMode ) {
            if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
                ProbeForWriteUlong (PreviousSuspendCount);
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();
    }

    st = ObReferenceObjectByHandle (ThreadHandle,
                                    THREAD_SUSPEND_RESUME,
                                    PsThreadType,
                                    Mode,
                                    &Thread,
                                    NULL);

    if (!NT_SUCCESS (st)) {
        return st;
    }

    LocalPreviousSuspendCount = (ULONG) KeAlertResumeThread (&Thread->Tcb);

    ObDereferenceObject (Thread);

    try {

        if (ARGUMENT_PRESENT (PreviousSuspendCount)) {
            *PreviousSuspendCount = LocalPreviousSuspendCount;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode ();
    }

    return STATUS_SUCCESS;
}


NTSTATUS
NtTestAlert(
    VOID
    )

/*++

Routine Description:

    This function tests the alert flag inside the current thread. If
    an alert is pending for the previous mode, then the alerted status
    is returned, pending APC's may also be delivered at this time.

Arguments:

    None

Return Value:

    STATUS_ALERTED - An alert was pending for the current thread at the
        time this function was called.

    STATUS_SUCCESS - No alert was pending for this thread.

--*/

{

    PAGED_CODE();

    if (KeTestAlertThread(KeGetPreviousMode ())) {
        return STATUS_ALERTED;
    } else {
        return STATUS_SUCCESS;
    }
}

