/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    pshelper.c

Abstract:

    EPROCESS and ETHREAD field access for NTOS-external components

--*/

#include "psp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, PsIsProcessBeingDebugged)
#pragma alloc_text (PAGE, PsIsThreadImpersonating)
#pragma alloc_text (PAGE, PsReferenceProcessFilePointer)
#pragma alloc_text (PAGE, PsSetProcessWin32Process)
#pragma alloc_text (PAGE, PsSetProcessSecurityPort)
#pragma alloc_text (PAGE, PsSetJobUIRestrictionsClass)
#pragma alloc_text (PAGE, PsSetProcessWindowStation)
#pragma alloc_text (PAGE, PsGetProcessSecurityPort)
#pragma alloc_text (PAGE, PsSetThreadWin32Thread)
#pragma alloc_text (PAGE, PsGetProcessExitProcessCalled)
#pragma alloc_text (PAGE, PsGetThreadSessionId)
#pragma alloc_text (PAGE, PsSetProcessPriorityClass)
#pragma alloc_text (PAGE, PsIsSystemProcess)
#endif

/*++
--*/
#undef PsGetCurrentProcess
PEPROCESS
PsGetCurrentProcess(
    VOID
    )
{
    return _PsGetCurrentProcess();
}

ULONG PsGetCurrentProcessSessionId(
    VOID
    )
{
    return MmGetSessionId (_PsGetCurrentProcess());
}

#undef PsGetCurrentThread
PETHREAD
PsGetCurrentThread(
    VOID
    )
{
    return _PsGetCurrentThread();
}

PVOID
PsGetCurrentThreadStackBase(
    VOID
    )
{
    return KeGetCurrentThread()->StackBase;
}

PVOID
PsGetCurrentThreadStackLimit(
    VOID
    )
{
    return KeGetCurrentThread()->StackLimit;
}

CCHAR
PsGetCurrentThreadPreviousMode(
    VOID
    )
{
    return KeGetPreviousMode();
}

PERESOURCE
PsGetJobLock(
    __in PEJOB Job
    )
{
    return &Job->JobLock;
}

ULONG
PsGetJobSessionId(
    __in PEJOB Job
    )
{
    return Job->SessionId;
}

ULONG
PsGetJobUIRestrictionsClass(
    __in PEJOB Job
    )
{
    return Job->UIRestrictionsClass;
}

LONGLONG
PsGetProcessCreateTimeQuadPart(
    __in PEPROCESS Process
    )
{
    return Process->CreateTime.QuadPart;
}

PVOID
PsGetProcessDebugPort(
    __in PEPROCESS Process
    )
{
    return Process->DebugPort;
}


BOOLEAN
PsIsProcessBeingDebugged(
    __in PEPROCESS Process
    )
{
    if (Process->DebugPort != NULL) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOLEAN
PsGetProcessExitProcessCalled(
    __in PEPROCESS Process
    )
{
    return (BOOLEAN) ((Process->Flags&PS_PROCESS_FLAGS_PROCESS_EXITING) != 0);
}

NTSTATUS
PsGetProcessExitStatus(
    __in PEPROCESS Process
    )
{
    return Process->ExitStatus;
}

HANDLE
PsGetProcessId(
    __in PEPROCESS Process
    )
{
    return Process->UniqueProcessId;
}


UCHAR *
PsGetProcessImageFileName(
    __in PEPROCESS Process
    )
{
    return Process->ImageFileName;
}


HANDLE
PsGetProcessInheritedFromUniqueProcessId(
    __in PEPROCESS Process
    )
{
    return Process->InheritedFromUniqueProcessId;
}


PEJOB
PsGetProcessJob(
    __in PEPROCESS Process
    )
{
    return Process->Job;
}


ULONG
PsGetProcessSessionId(
    __in PEPROCESS Process
    )
{
    return MmGetSessionId (Process);
}


ULONG
PsGetProcessSessionIdEx(
    __in PEPROCESS Process
    )
{
    return MmGetSessionIdEx (Process);
}


PVOID
PsGetProcessSectionBaseAddress(
    __in PEPROCESS Process
    )
{
    return Process->SectionBaseAddress;
}


PPEB
PsGetProcessPeb(
    __in PEPROCESS Process
    )
{
    return Process->Peb;
}


UCHAR
PsGetProcessPriorityClass(
    __in PEPROCESS Process
    )
{
    return Process->PriorityClass;
}

HANDLE
PsGetProcessWin32WindowStation(
    __in PEPROCESS Process
    )
{
    return Process->Win32WindowStation;
}



PVOID
PsGetProcessWin32Process(
    __in PEPROCESS Process
    )
{
    return Process->Win32Process;
}


PVOID
PsGetCurrentProcessWin32Process(
    VOID
    )
{
    return PsGetCurrentProcess()->Win32Process;
}


PVOID
PsGetProcessWow64Process(
    __in PEPROCESS Process
    )
{
    return PS_GET_WOW64_PROCESS (Process);
}


PVOID
PsGetCurrentProcessWow64Process(
    VOID
    )
{
    return PS_GET_WOW64_PROCESS (PsGetCurrentProcess());
}


HANDLE
PsGetThreadId(
    __in PETHREAD Thread
     )
{
    return Thread->Cid.UniqueThread;
}


CCHAR
PsGetThreadFreezeCount(
    __in PETHREAD Thread
    )
{
    return Thread->Tcb.FreezeCount;
}


BOOLEAN
PsGetThreadHardErrorsAreDisabled(
    __in PETHREAD Thread)
{
    return (BOOLEAN) (Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED) != 0;
}


PEPROCESS
PsGetThreadProcess(
    __in PETHREAD Thread
    )
{
    return THREAD_TO_PROCESS(Thread);
}


PEPROCESS
PsGetCurrentThreadProcess(
    VOID
    )
{
    return THREAD_TO_PROCESS(_PsGetCurrentThread());
}



HANDLE
PsGetThreadProcessId(
    __in PETHREAD Thread
     )
{
    return Thread->Cid.UniqueProcess;
}


HANDLE
PsGetCurrentThreadProcessId(
    VOID
    )
{
    return _PsGetCurrentThread()->Cid.UniqueProcess;
}


ULONG
PsGetThreadSessionId(
    __in PETHREAD Thread
    )
{
    return MmGetSessionId (THREAD_TO_PROCESS(Thread));
}


PVOID
PsGetThreadTeb(
    __in PETHREAD Thread
    )
{
    return Thread->Tcb.Teb;
}


PVOID
PsGetCurrentThreadTeb(
    VOID
    )
{
    return _PsGetCurrentThread()->Tcb.Teb;
}


PVOID
PsGetThreadWin32Thread(
    __in PETHREAD Thread
     )
{
    return Thread->Tcb.Win32Thread;
}


PVOID
PsGetCurrentThreadWin32Thread(
    VOID
     )
{
    return _PsGetCurrentThread()->Tcb.Win32Thread;
}


PVOID
PsGetCurrentThreadWin32ThreadAndEnterCriticalRegion(
    __out PHANDLE ProcessId
     )
{
    PETHREAD Thread = _PsGetCurrentThread();
    *ProcessId = Thread->Cid.UniqueProcess;
    KeEnterCriticalRegionThread(&Thread->Tcb);
    return Thread->Tcb.Win32Thread;
}


BOOLEAN
PsIsSystemThread(
    __in PETHREAD Thread
     )
{
    return (BOOLEAN)(IS_SYSTEM_THREAD(Thread));
}


BOOLEAN
PsIsSystemProcess(
    __in PEPROCESS Process
     )
{
    return (BOOLEAN)(Process == PsInitialSystemProcess);
}


VOID
PsSetJobUIRestrictionsClass(
    __out PEJOB Job,
    __in ULONG UIRestrictionsClass
    )
{
    Job->UIRestrictionsClass = UIRestrictionsClass;
}


VOID
PsSetProcessPriorityClass(
    __out PEPROCESS Process,
    __in UCHAR PriorityClass
    )
{
    Process->PriorityClass = PriorityClass;
}


NTSTATUS
PsSetProcessWin32Process(
    __in PEPROCESS Process,
    __in PVOID Win32Process,
    __in PVOID PrevWin32Process
    )
{
    NTSTATUS Status;
    PETHREAD CurrentThread;

    Status = STATUS_SUCCESS;

    CurrentThread = _PsGetCurrentThread();

    PspLockProcessExclusive (Process, CurrentThread);

    if (Win32Process != NULL) {
        if ((Process->Flags&PS_PROCESS_FLAGS_PROCESS_DELETE) == 0 && Process->Win32Process == NULL) {
            Process->Win32Process = Win32Process;
        } else {
            Status = STATUS_PROCESS_IS_TERMINATING;
        }
    } else {
        if (Process->Win32Process == PrevWin32Process) {
            Process->Win32Process = NULL;
        } else {
            Status = STATUS_UNSUCCESSFUL;       
        }
    }

    PspUnlockProcessExclusive (Process, CurrentThread);
 
    return Status;
}


VOID
PsSetProcessWindowStation(
    __out PEPROCESS Process,
    __in HANDLE Win32WindowStation
    )
{
     Process->Win32WindowStation = Win32WindowStation;
}


VOID
PsSetThreadHardErrorsAreDisabled(
    __in PETHREAD Thread,
    __in BOOLEAN HardErrorsAreDisabled
    )
{
    if (HardErrorsAreDisabled) {
        PS_SET_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED);
    } else {
        PS_CLEAR_BITS (&Thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED);
    }
}


VOID
PsSetThreadWin32Thread(
    __inout PETHREAD Thread,
    __in PVOID Win32Thread,
    __in PVOID PrevWin32Thread
    )
{
    if (Win32Thread != NULL) {
        InterlockedExchangePointer(&Thread->Tcb.Win32Thread, Win32Thread);
    } else {
        InterlockedCompareExchangePointer(&Thread->Tcb.Win32Thread, Win32Thread, PrevWin32Thread);
    }
}


PVOID
PsGetProcessSecurityPort(
    __in PEPROCESS Process
    )
{
    return Process->SecurityPort ;
}


NTSTATUS
PsSetProcessSecurityPort(
    __out PEPROCESS Process,
    __in PVOID Port
    )
{
    Process->SecurityPort = Port;
    
    return STATUS_SUCCESS ;
}

BOOLEAN
PsIsThreadImpersonating (
    __in PETHREAD Thread
    )
/*++

Routine Description:

    This routine returns TRUE if the specified thread is impersonating otherwise it returns false.

Arguments:

    Thread - Thread to be queried

Return Value:

    BOOLEAN - TRUE: Thread is impersonating, FALSE: Thread is not impersonating.

--*/
{
    PAGED_CODE ();

    return (BOOLEAN) (PS_IS_THREAD_IMPERSONATING (Thread));
}


NTSTATUS
PsReferenceProcessFilePointer (
    IN PEPROCESS Process,
    OUT PVOID *OutFileObject
    )

/*++

Routine Description:

    This routine returns a referenced pointer to the FilePointer of Process.  
    This is a rundown protected wrapper around MmGetFileObjectForSection.

Arguments:

    Process - Supplies the process to query.

    OutFileObject - Returns the file object backing the requested section if
                    success is returned.

Return Value:

    NTSTATUS.
    
Environment:

    Kernel mode, PASSIVE_LEVEL.

--*/

{
    PFILE_OBJECT FileObject;

    PAGED_CODE();
    
    if (!ExAcquireRundownProtection (&Process->RundownProtect)) {
        return STATUS_UNSUCCESSFUL;
    }

    if (Process->SectionObject == NULL) {
        ExReleaseRundownProtection (&Process->RundownProtect);
        return STATUS_UNSUCCESSFUL;
    }

    FileObject = MmGetFileObjectForSection ((PVOID)Process->SectionObject);

    *OutFileObject = FileObject;

    ObReferenceObject (FileObject);

    ExReleaseRundownProtection (&Process->RundownProtect);

    return STATUS_SUCCESS;
}

