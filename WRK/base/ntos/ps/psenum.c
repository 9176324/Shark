/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    psenum.c

Abstract:

    This module enumerates the actve processes in the system

--*/

#include "psp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PsEnumProcesses)
#pragma alloc_text(PAGE, PsGetNextProcess)
#pragma alloc_text(PAGE, PsQuitNextProcess)
#pragma alloc_text(PAGE, PsEnumProcessThreads)
#pragma alloc_text(PAGE, PsGetNextProcessThread)
#pragma alloc_text(PAGE, PsQuitNextProcessThread)
#pragma alloc_text(PAGE, PsGetNextJob)
#pragma alloc_text(PAGE, PsGetNextJobProcess)
#pragma alloc_text(PAGE, PsQuitNextJob)
#pragma alloc_text(PAGE, PsQuitNextJobProcess)
#pragma alloc_text(PAGE, PspGetNextJobProcess)
#pragma alloc_text(PAGE, PspQuitNextJobProcess)
#pragma alloc_text(PAGE, NtGetNextProcess)
#pragma alloc_text(PAGE, NtGetNextThread)
#endif

NTSTATUS
PsEnumProcesses (
    IN PROCESS_ENUM_ROUTINE CallBack,
    IN PVOID Context
    )
/*++

Routine Description:

    This function calls the callback routine for each active process in the system.
    Process objects in the process of being deleted are skipped.
    Returning anything but a success code from the callback routine terminates the enumeration at that point.
    Processes may be referenced and used later safely.

Arguments:

    CallBack - Routine to be called with its first parameter the enumerated process

Return Value:

    NTSTATUS - Status of call

--*/
{
    PLIST_ENTRY ListEntry;
    PEPROCESS Process, NewProcess;
    PETHREAD CurrentThread;
    NTSTATUS Status;

    Process = NULL;

    CurrentThread = PsGetCurrentThread ();

    PspLockProcessList (CurrentThread);

    for (ListEntry = PsActiveProcessHead.Flink;
         ListEntry != &PsActiveProcessHead;
         ListEntry = ListEntry->Flink) {

        NewProcess = CONTAINING_RECORD (ListEntry, EPROCESS, ActiveProcessLinks);
        if (ObReferenceObjectSafe (NewProcess)) {

            PspUnlockProcessList (CurrentThread);

            if (Process != NULL) {
                ObDereferenceObject (Process);
            }

            Process = NewProcess;

            Status = CallBack (Process, Context);

            if (!NT_SUCCESS (Status)) {
                ObDereferenceObject (Process);
                return Status;
            }

            PspLockProcessList (CurrentThread);

        }
    }

    PspUnlockProcessList (CurrentThread);

    if (Process != NULL) {
        ObDereferenceObject (Process);
    }

    return STATUS_SUCCESS;
}

PEPROCESS
PsGetNextProcess (
    IN PEPROCESS Process
    )
/*++

Routine Description:

    This function allows code to enumerate all the active processes in the system.
    The first process (if Process is NULL) or subsequent process (if process not NULL) is returned on
    each call.
    If process is not NULL then this process must have previously been obtained by a call to PsGetNextProcess.
    Enumeration may be terminated early by calling PsQuitNextProcess on the last non-NULL process
    returned by PsGetNextProcess.

    Processes may be referenced and used later safely.

    For example, to enumerate all system processes in a loop use this code fragment:

    for (Process = PsGetNextProcess (NULL);
         Process != NULL;
         Process = PsGetNextProcess (Process)) {
         ...
         ...
         //
         // Early terminating conditions are handled like this:
         //
         if (NeedToBreakOutEarly) {
             PsQuitNextProcess (Process);
             break;
         }
    }
    

Arguments:

    Process - Process to get the next process from or NULL for the first process

Return Value:

    PEPROCESS - Next process or NULL if no more processes available

--*/
{
    PEPROCESS NewProcess = NULL;
    PETHREAD CurrentThread;
    PLIST_ENTRY ListEntry;

    CurrentThread = PsGetCurrentThread ();

    PspLockProcessList (CurrentThread);

    for (ListEntry = (Process == NULL) ? PsActiveProcessHead.Flink : Process->ActiveProcessLinks.Flink;
         ListEntry != &PsActiveProcessHead;
         ListEntry = ListEntry->Flink) {

        NewProcess = CONTAINING_RECORD (ListEntry, EPROCESS, ActiveProcessLinks);

        //
        // Processes are removed from this list during process objected deletion (object reference count goes
        // to zero). To prevent double deletion of the process we need to do a safe reference here.
        //
        if (ObReferenceObjectSafe (NewProcess)) {
            break;
        }
        NewProcess = NULL;
    }
    PspUnlockProcessList (CurrentThread);

    if (Process != NULL) {
        ObDereferenceObject (Process);
    }

    return NewProcess;
}


VOID
PsQuitNextProcess (
    IN PEPROCESS Process
    )
/*++

Routine Description:

    This function is used to terminate early a process enumeration using PsGetNextProcess

Arguments:

    Process - Non-NULL process previously obtained by a call to PsGetNextProcess.

Return Value:

    None

--*/
{
    ObDereferenceObject (Process);
}

PETHREAD
PsGetNextProcessThread (
    IN PEPROCESS Process,
    IN PETHREAD Thread
    )
/*++

Routine Description:

    This function is used to enumerate the threads in a process.


Arguments:

    Process - Process to enumerate
    Thread  - Thread to start enumeration from. This must have been obtained from previous call to
              PsGetNextProcessThread. If NULL enumeration starts at the first non-terminating thread in the process.

Return Value:

    PETHREAD - Pointer to a non-terminated process thread or a NULL if there are non. This thread must be passed
               either to another call to PsGetNextProcessThread or PsQuitNextProcessThread.

--*/
{
    PLIST_ENTRY ListEntry;
    PETHREAD NewThread, CurrentThread;

    PAGED_CODE ();
 
    CurrentThread = PsGetCurrentThread ();

    PspLockProcessShared (Process, CurrentThread);

    for (ListEntry = (Thread == NULL) ? Process->ThreadListHead.Flink : Thread->ThreadListEntry.Flink;
         ;
         ListEntry = ListEntry->Flink) {
        if (ListEntry != &Process->ThreadListHead) {
            NewThread = CONTAINING_RECORD (ListEntry, ETHREAD, ThreadListEntry);
            //
            // Don't reference a thread thats in its delete routine
            //
            if (ObReferenceObjectSafe (NewThread)) {
                break;
            }
        } else {
            NewThread = NULL;
            break;
        }
    }
    PspUnlockProcessShared (Process, CurrentThread);

    if (Thread != NULL) {
        ObDereferenceObject (Thread);
    }
    return NewThread;
}

VOID
PsQuitNextProcessThread (
    IN PETHREAD Thread
    )
/*++

Routine Description:

    This function quits thread enumeration early.

Arguments:

    Thread - Thread obtained from a call to PsGetNextProcessThread

Return Value:

    None.

--*/
{
    ObDereferenceObject (Thread);
}

NTSTATUS
PsEnumProcessThreads (
    IN PEPROCESS Process,
    IN THREAD_ENUM_ROUTINE CallBack,
    IN PVOID Context
    )
/*++

Routine Description:

    This function calls the callback routine for each active thread in the process.
    Thread objects in the process of being deleted are skipped.
    Returning anything but a success code from the callback routine terminates the enumeration at that point.
    Thread may be referenced and used later safely.

Arguments:

    CallBack - Routine to be called with its first parameter the enumerated process

Return Value:

    NTSTATUS - Status of call

--*/
{
    NTSTATUS Status;
    PETHREAD Thread;

    Status = STATUS_SUCCESS;
    for (Thread = PsGetNextProcessThread (Process, NULL);
         Thread != NULL;
         Thread = PsGetNextProcessThread (Process, Thread)) {
        Status = CallBack (Process, Thread, Context);
        if (!NT_SUCCESS (Status)) {
            PsQuitNextProcessThread (Thread);
            break;
        }
    }
    return Status;
}

PEJOB
PsGetNextJob (
    IN PEJOB Job
    )
/*++

Routine Description:

    This function allows code to enumerate all the active jobs in the system.
    The first job (if Job is NULL) or subsequent jobs (if Job not NULL) is returned on
    each call.
    If Job is not NULL then this job must have previously been obtained by a call to PsGetNextJob.
    Enumeration may be terminated early by calling PsQuitNextJob on the last non-NULL job
    returned by PsGetNextJob.

    Jobs may be referenced and used later safely.

    For example, to enumerate all system jobs in a loop use this code fragment:

    for (Job = PsGetNextJob (NULL);
         Job != NULL;
         Job = PsGetNextJob (Job)) {
         ...
         ...
         //
         // Early terminating conditions are handled like this:
         //
         if (NeedToBreakOutEarly) {
             PsQuitNextJob (Job);
             break;
         }
    }
    

Arguments:

    Job - Job from a previous call to PsGetNextJob or NULL for the first job in the system

Return Value:

    PEJOB - Next job in the system or NULL if none available.

--*/
{
    PEJOB NewJob = NULL;
    PLIST_ENTRY ListEntry;
    PETHREAD CurrentThread;

    CurrentThread = PsGetCurrentThread ();

    PspLockJobListShared (CurrentThread);

    for (ListEntry = (Job == NULL) ? PspJobList.Flink : Job->JobLinks.Flink;
         ListEntry != &PspJobList;
         ListEntry = ListEntry->Flink) {

        NewJob = CONTAINING_RECORD (ListEntry, EJOB, JobLinks);

        //
        // Jobs are removed from this list during job objected deletion (object reference count goes
        // to zero). To prevent double deletion of the job we need to do a safe reference here.
        //
        if (ObReferenceObjectSafe (NewJob)) {
            break;
        }
        NewJob = NULL;
    }

    PspUnlockJobListShared (CurrentThread);

    if (Job != NULL) {
        ObDereferenceObject (Job);
    }

    return NewJob;
}


VOID
PsQuitNextJob (
    IN PEJOB Job
    )
/*++

Routine Description:

    This function is used to terminate early a job enumeration using PsGetNextJob

Arguments:

    Job - Non-NULL job previously obtained by a call to PsGetNextJob.

Return Value:

    None

--*/
{
    ObDereferenceObject (Job);
}

PEPROCESS
PsGetNextJobProcess (
    IN PEJOB Job,
    IN PEPROCESS Process
    )
/*++

Routine Description:

    This function is used to enumerate the processes in a job.


Arguments:

    Job     - Job to Enumerate
    Process - Process to start enumeration from. This must have been obtained from previous call to
              PsGetNextJobProcess. If NULL enumeration starts at the first non-terminating process in the Job.

Return Value:

    PEPROCESS - Pointer to a non-terminated process or a NULL if there are non. This process must be passed
                either to another call to PsGetNextJobProcess or PsQuitNextJobProcess.

--*/
{
    PLIST_ENTRY ListEntry;
    PEPROCESS NewProcess;
    PETHREAD CurrentThread;

    PAGED_CODE ();
 
    CurrentThread = PsGetCurrentThread ();

    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquireResourceExclusiveLite (&Job->JobLock, TRUE);

    for (ListEntry = (Process == NULL) ? Job->ProcessListHead.Flink : Process->JobLinks.Flink;
         ;
         ListEntry = ListEntry->Flink) {
        if (ListEntry != &Job->ProcessListHead) {
            NewProcess = CONTAINING_RECORD (ListEntry, EPROCESS, JobLinks);
            //
            // Don't reference a process thats in its delete routine
            //
            if (ObReferenceObjectSafe (NewProcess)) {
                break;
            }
        } else {
            NewProcess = NULL;
            break;
        }
    }

    ExReleaseResourceLite (&Job->JobLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);


    if (Process != NULL) {
        ObDereferenceObject (Process);
    }
    return NewProcess;
}

VOID
PsQuitNextJobProcess (
    IN PEPROCESS Process
    )
/*++

Routine Description:

    This function quits job process enumeration early.

Arguments:

    Process - Process obtained from a call to PsGetNextJobProcess

Return Value:

    None.

--*/
{
    ObDereferenceObject (Process);
}

PEPROCESS
PspGetNextJobProcess (
    IN PEJOB Job,
    IN PEPROCESS Process
    )
/*++

Routine Description:

    This function is used to enumerate the processes in a job with the job lock held.


Arguments:

    Job     - Job to Enumerate
    Process - Process to start enumeration from. This must have been obtained from previous call to
              PsGetNextJobProcess. If NULL enumeration starts at the first non-terminating process in the Job.

Return Value:

    PEPROCESS - Pointer to a non-terminated process or a NULL if there are non. This process must be passed
                either to another call to PsGetNextJobProcess or PsQuitNextJobProcess.

--*/
{
    PLIST_ENTRY ListEntry;
    PEPROCESS NewProcess;

    PAGED_CODE ();
 
    for (ListEntry = (Process == NULL) ? Job->ProcessListHead.Flink : Process->JobLinks.Flink;
         ;
         ListEntry = ListEntry->Flink) {
        if (ListEntry != &Job->ProcessListHead) {
            NewProcess = CONTAINING_RECORD (ListEntry, EPROCESS, JobLinks);
            //
            // Don't reference a process thats in its delete routine
            //
            if (ObReferenceObjectSafe (NewProcess)) {
                break;
            }
        } else {
            NewProcess = NULL;
            break;
        }
    }

    if (Process != NULL) {
        ObDereferenceObjectDeferDelete (Process);
    }
    return NewProcess;
}

VOID
PspQuitNextJobProcess (
    IN PEPROCESS Process
    )
/*++

Routine Description:

    This function quits job process enumeration early.

Arguments:

    Process - Process obtained from a call to PsGetNextJobProcess

Return Value:

    None.

--*/
{
    ObDereferenceObjectDeferDelete (Process);
}

NTSTATUS
NtGetNextProcess (
    __in HANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in ULONG HandleAttributes,
    __in ULONG Flags,
    __out PHANDLE NewProcessHandle
    )
/*++

Routine Description:

    This function gets the next for first process in the list of all processes in the system

Arguments:

    ProcessHandle - Process obtained from a previous call to NtGetNextProcess or NULL for the first process
    DesiredAccess - Access requested for process handle
    HandleAttributes - Handle attributes requested.
    Flags - Flags for the operation
    NewProcessHandle - Pointer to a handle value that is returned on success

Return Value:

    NTSTATUS - Status of operation.

--*/
{
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS Process, NewProcess;
    NTSTATUS Status;
    ACCESS_STATE AccessState;
    AUX_ACCESS_DATA AuxData;
    HANDLE Handle;

    PAGED_CODE ();

    PreviousMode = KeGetPreviousMode ();

    //
    // Sanitize handle attributes
    //
    HandleAttributes = ObSanitizeHandleAttributes (HandleAttributes, PreviousMode);

    //
    // Validate pointer arguments
    //

    try {
        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle (NewProcessHandle);
        }
        *NewProcessHandle = NULL;
    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode ();
    }

    //
    // Check for inclusion of reserved flags and reject the call if present
    //

    if (Flags != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if (ProcessHandle == NULL) {
        Process = NULL;
    } else {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            0,
                                            PsProcessType,
                                            PreviousMode,
                                            &Process,
                                            NULL);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    }

    NewProcess = PsGetNextProcess (Process);

    if (NewProcess == NULL) {
        return STATUS_NO_MORE_ENTRIES;
    }

    Status = SeCreateAccessState (&AccessState,
                                  &AuxData,
                                  DesiredAccess,
                                  &PsProcessType->TypeInfo.GenericMapping);

    if (!NT_SUCCESS (Status)) {
        ObDereferenceObject (NewProcess);
        return Status;
    }

    if (SeSinglePrivilegeCheck (SeDebugPrivilege, PreviousMode)) {
        if (AccessState.RemainingDesiredAccess & MAXIMUM_ALLOWED) {
            AccessState.PreviouslyGrantedAccess |= PROCESS_ALL_ACCESS;
        } else {
            AccessState.PreviouslyGrantedAccess |= AccessState.RemainingDesiredAccess;
        }
        AccessState.RemainingDesiredAccess = 0;
    }


    while (1) {
        if (NewProcess->GrantedAccess != 0) {

            Status = ObOpenObjectByPointer (NewProcess,
                                            HandleAttributes,
                                            &AccessState,
                                            0,
                                            PsProcessType,
                                            PreviousMode,
                                            &Handle);
            if (NT_SUCCESS (Status)) {
                try {
                    *NewProcessHandle = Handle;
                } except (ExSystemExceptionFilter ()) {
                    Status = GetExceptionCode ();
                }
                break;
            }

            if (Status != STATUS_ACCESS_DENIED) {
                break;
            }
        }

        NewProcess = PsGetNextProcess (NewProcess);

        if (NewProcess == NULL) {
            Status = STATUS_NO_MORE_ENTRIES;
            break;
        }
    }

    SeDeleteAccessState (&AccessState);

    if (NewProcess != NULL) {
        ObDereferenceObject (NewProcess);
    }

    return Status;
}

NTSTATUS
NtGetNextThread (
    __in HANDLE ProcessHandle,
    __in HANDLE ThreadHandle,
    __in ACCESS_MASK DesiredAccess,
    __in ULONG HandleAttributes,
    __in ULONG Flags,
    __out PHANDLE NewThreadHandle
    )
/*++

Routine Description:

    This function gets the next for first thread in a process

Arguments:

    ProcessHandle - Process that is being enumerated
    ThreadHandle - Last thread returned by the enumeration routine or NULL if the first is required
    DesiredAccess - Access requested for thread handle
    HandleAttributes - Handle attributes requested.
    Flags - Flags for the operation
    NewThreadHandle - Pointer to a handle value that is returned on success

Return Value:

    NTSTATUS - Status of operation.

--*/
{
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS Process;
    PETHREAD Thread, NewThread;
    NTSTATUS Status;
    ACCESS_STATE AccessState;
    AUX_ACCESS_DATA AuxData;
    HANDLE Handle;

    PAGED_CODE ();

    PreviousMode = KeGetPreviousMode ();

    //
    // Sanitize handle attributes
    //

    HandleAttributes = ObSanitizeHandleAttributes (HandleAttributes, PreviousMode);

    //
    // Validate pointer arguments
    //

    try {
        if (PreviousMode != KernelMode) {
            ProbeForWriteHandle (NewThreadHandle);
        }
        *NewThreadHandle = NULL;
    } except (ExSystemExceptionFilter ()) {
        return GetExceptionCode ();
    }

    //
    // Check for inclusion of reserved flags and reject the call if present
    //

    if (Flags != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_QUERY_INFORMATION,
                                        PsProcessType,
                                        PreviousMode,
                                        &Process,
                                        NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    if (ThreadHandle == NULL) {
        Thread = NULL;
    } else {
        Status = ObReferenceObjectByHandle (ThreadHandle,
                                            0,
                                            PsProcessType,
                                            PreviousMode,
                                            &Thread,
                                            NULL);
        if (!NT_SUCCESS (Status)) {
            ObDereferenceObject (Process);
            return Status;
        }

        //
        // Make sure 
        //

        if (THREAD_TO_PROCESS (Thread) != Process) {
            ObDereferenceObject (Thread);
            ObDereferenceObject (Process);
            return STATUS_INVALID_PARAMETER;
        }
    }

    NewThread = PsGetNextProcessThread (Process, Thread);


    if (NewThread == NULL) {
        ObDereferenceObject (Process);
        return STATUS_NO_MORE_ENTRIES;
    }

    Status = SeCreateAccessState (&AccessState,
                                  &AuxData,
                                  DesiredAccess,
                                  &PsProcessType->TypeInfo.GenericMapping);

    if (!NT_SUCCESS (Status)) {
        ObDereferenceObject (Process);
        ObDereferenceObject (NewThread);
        return Status;
    }

    if (SeSinglePrivilegeCheck (SeDebugPrivilege, PreviousMode)) {
        if (AccessState.RemainingDesiredAccess & MAXIMUM_ALLOWED) {
            AccessState.PreviouslyGrantedAccess |= PROCESS_ALL_ACCESS;
        } else {
            AccessState.PreviouslyGrantedAccess |= AccessState.RemainingDesiredAccess;
        }
        AccessState.RemainingDesiredAccess = 0;
    }


    while (1) {

        if (NewThread->GrantedAccess != 0) {
            Status = ObOpenObjectByPointer (NewThread,
                                            HandleAttributes,
                                            &AccessState,
                                            0,
                                            PsThreadType,
                                            PreviousMode,
                                            &Handle);
            if (NT_SUCCESS (Status)) {
                try {
                    *NewThreadHandle = Handle;
                } except (ExSystemExceptionFilter ()) {
                    Status = GetExceptionCode ();
                }
                break;
            }

            if (Status != STATUS_ACCESS_DENIED) {
                break;
            }
        }

        NewThread = PsGetNextProcessThread (Process, NewThread);

        if (NewThread == NULL) {
            Status = STATUS_NO_MORE_ENTRIES;
            break;
        }
    }

    SeDeleteAccessState (&AccessState);

    ObDereferenceObject (Process);
    if (NewThread != NULL) {
        ObDereferenceObject (NewThread);
    }

    return Status;
}

