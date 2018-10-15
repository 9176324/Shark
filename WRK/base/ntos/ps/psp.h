/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    psp.h

Abstract:

    Private Interfaces for process structure.

--*/

#ifndef _PSP_
#define _PSP_

#pragma warning(disable:4054)   // Cast of function pointer to PVOID
#pragma warning(disable:4055)   // Cast of function pointer
#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4127)   // condition expression is constant
#pragma warning(disable:4152)   // Casting function pointers
#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4214)   // bit field types other than int
#pragma warning(disable:4324)   // alignment sensitive to declspec
#pragma warning(disable:4327)   // alignment on assignment
#pragma warning(disable:4328)   // alignment on assignment

#include "ntos.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "zwapi.h"
#include "ki.h"
#if defined(_X86_)
#include <vdmntos.h>
#endif
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include <string.h>
#if defined(_WIN64)
#include <wow64t.h>
#endif

//
// Working Set Watcher is 8kb. This lets us watch about 4mb of working
// set.
//

#define WS_CATCH_SIZE 8192
#define WS_OVERHEAD 16
#define MAX_WS_CATCH_INDEX (((WS_CATCH_SIZE-WS_OVERHEAD)/sizeof(PROCESS_WS_WATCH_INFORMATION)) - 2)

//
// Process Quota Charges:
//
//  PagedPool
//      Directory Base Page - PAGE_SIZE
//
//  NonPaged
//      Object Body         - sizeof(EPROCESS)
//

#define PSP_PROCESS_PAGED_CHARGE    (PAGE_SIZE)
#define PSP_PROCESS_NONPAGED_CHARGE (sizeof(EPROCESS))

//
// Thread Quota Charges:
//
//  PagedPool
//      Kernel Stack        - 0
//
//  NonPaged
//      Object Body         - sizeof(ETHREAD)
//

#define PSP_THREAD_PAGED_CHARGE     (0)
#define PSP_THREAD_NONPAGED_CHARGE  (sizeof(ETHREAD))

//
// Define routines to get trap and exception frame addresses
//

#define PSPALIGN_DOWN(address,amt) ((ULONG)(address) & ~(( amt ) - 1))

#define PSPALIGN_UP(address,amt) (PSPALIGN_DOWN( (address + (amt) - 1), (amt) ))


#if defined(_AMD64_)

FORCEINLINE
PKTRAP_FRAME
PspGetBaseTrapFrame (
    PETHREAD Thread
    )

{
    ULONG64 InitialStack;
    PKERNEL_STACK_CONTROL StackControl;

    InitialStack = (ULONG64)Thread->Tcb.InitialStack;
    StackControl = (PKERNEL_STACK_CONTROL)InitialStack;
    while (StackControl->Previous.StackBase != 0) {
        InitialStack = StackControl->Previous.InitialStack;
        StackControl = (PKERNEL_STACK_CONTROL)InitialStack;
    }

    return (PKTRAP_FRAME)(InitialStack - KTRAP_FRAME_LENGTH);
}

#define PspGetBaseExceptionFrame(Thread) ((PKEXCEPTION_FRAME)((ULONG_PTR)PspGetBaseTrapFrame(Thread) - \
                                                              KEXCEPTION_FRAME_LENGTH))

#elif defined(_X86_)

#define PspGetBaseTrapFrame(Thread) (PKTRAP_FRAME)((ULONG_PTR)Thread->Tcb.InitialStack - \
                                                   PSPALIGN_UP(sizeof(KTRAP_FRAME),KTRAP_FRAME_ALIGN) - \
                                                   sizeof(FX_SAVE_AREA))

#define PspGetBaseExceptionFrame(Thread) (NULL)

#else

#error "no target architecture"

#endif // defined(_AMD64_)

typedef struct _GETSETCONTEXT {
    KAPC Apc;
    KPROCESSOR_MODE Mode;
    KEVENT OperationComplete;
    CONTEXT Context;
    KNONVOLATILE_CONTEXT_POINTERS NonVolatileContext;
} GETSETCONTEXT, *PGETSETCONTEXT;

typedef struct _SYSTEM_DLL {
    PVOID Section;
    PVOID DllBase;
    PKNORMAL_ROUTINE LoaderInitRoutine;
    EX_PUSH_LOCK DllLock;
} SYSTEM_DLL, PSYSTEM_DLL;

typedef struct _JOB_WORKING_SET_CHANGE_HEAD {
    LIST_ENTRY Links;
    KGUARDED_MUTEX Lock;
    SIZE_T MinimumWorkingSetSize;
    SIZE_T MaximumWorkingSetSize;
} JOB_WORKING_SET_CHANGE_HEAD, *PJOB_WORKING_SET_CHANGE_HEAD;

typedef struct _JOB_WORKING_SET_CHANGE_RECORD {
    LIST_ENTRY Links;
    PEPROCESS Process;
} JOB_WORKING_SET_CHANGE_RECORD, *PJOB_WORKING_SET_CHANGE_RECORD;

JOB_WORKING_SET_CHANGE_HEAD PspWorkingSetChangeHead;

typedef struct _PRIV_CHECK_CTX {
    SECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    PRIVILEGE_SET RequiredPrivileges;
    KPROCESSOR_MODE PreviousMode;
    BOOLEAN AccessGranted;
} PRIV_CHECK_CTX, *PPRIV_CHECK_CTX;

LOGICAL
PspSinglePrivCheck (
    IN LUID PrivilegeValue,
    IN KPROCESSOR_MODE PreviousMode,
    OUT PPRIV_CHECK_CTX PrivCtx
    );

VOID
PspSinglePrivCheckAudit (
    IN LOGICAL PrivUsed,
    IN PPRIV_CHECK_CTX PrivCtx
    );

//
// Private Entry Points
//

VOID
PspProcessDump(
    IN PVOID Object,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );

VOID
PspProcessDelete(
    IN PVOID Object
    );


VOID
PspThreadDump(
    IN PVOID Object,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );

VOID
PspInheritQuota(
    IN PEPROCESS NewProcess,
    IN PEPROCESS ParentProcess
    );

VOID
PspDereferenceQuota(
    IN PEPROCESS Process
    );

VOID
PspThreadDelete(
    IN PVOID Object
    );

NTSTATUS
PspWriteTebImpersonationInfo (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    );

//
// Initialization and loader entrypoints
//

BOOLEAN
PspInitPhase0 (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

BOOLEAN
PspInitPhase1 (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

NTSTATUS
PspInitializeSystemDll( VOID );

NTSTATUS
PspLookupSystemDllEntryPoint(
    IN PSZ EntryPointName,
    OUT PVOID *EntryPointAddress
    );

NTSTATUS
PspLookupKernelUserEntryPoints(
    VOID
    );

USHORT
PspNameToOrdinal(
    IN PSZ EntryPointName,
    IN ULONG DllBase,
    IN ULONG NumberOfNames,
    IN PULONG NameTableBase,
    IN PUSHORT OrdinalTableBase
    );

//
// Internal Creation Functions
//

NTSTATUS
PspCreateProcess(
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ParentProcess OPTIONAL,
    IN ULONG Flags,
    IN HANDLE SectionHandle OPTIONAL,
    IN HANDLE DebugPort OPTIONAL,
    IN HANDLE ExceptionPort OPTIONAL,
    IN ULONG JobMemberLevel
    );

#define PSP_MAX_CREATE_PROCESS_NOTIFY 8

//
// Define process callouts. These are of type PCREATE_PROCESS_NOTIFY_ROUTINE 
// Called on process create and delete.
//
ULONG PspCreateProcessNotifyRoutineCount;
EX_CALLBACK PspCreateProcessNotifyRoutine[PSP_MAX_CREATE_PROCESS_NOTIFY];

#define PSP_MAX_CREATE_THREAD_NOTIFY 8

//
// Define thread callouts. These are of type PCREATE_THREAD_NOTIFY_ROUTINE
// Called on thread create and delete.
//
ULONG PspCreateThreadNotifyRoutineCount;
EX_CALLBACK PspCreateThreadNotifyRoutine[PSP_MAX_CREATE_THREAD_NOTIFY];


#define PSP_MAX_LOAD_IMAGE_NOTIFY 8

//
// Define image load callbacks. These are of type PLOAD_IMAGE_NOTIFY_ROUTINE 
// Called on image load.
//
ULONG PspLoadImageNotifyRoutineCount;
EX_CALLBACK PspLoadImageNotifyRoutine[PSP_MAX_LOAD_IMAGE_NOTIFY];


NTSTATUS
PspCreateThread(
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN HANDLE ProcessHandle,
    IN PEPROCESS ProcessPointer,
    OUT PCLIENT_ID ClientId OPTIONAL,
    IN PCONTEXT ThreadContext OPTIONAL,
    IN PINITIAL_TEB InitialTeb OPTIONAL,
    IN BOOLEAN CreateSuspended,
    IN PKSTART_ROUTINE StartRoutine OPTIONAL,
    IN PVOID StartContext
    );

//
// Startup Routines
//

VOID
PspUserThreadStartup(
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    );

VOID
PspSystemThreadStartup(
    IN PKSTART_ROUTINE StartRoutine,
    IN PVOID StartContext
    );

VOID
PspReaper(
    IN PVOID StartContext
    );

VOID
PspNullSpecialApc(
    IN PKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

//
// Thread Exit Support
//

VOID
PspExitApcRundown(
    IN PKAPC Apc
    );

DECLSPEC_NORETURN
VOID
PspExitThread(
    IN NTSTATUS ExitStatus
    );

NTSTATUS
PspTerminateThreadByPointer(
    IN PETHREAD Thread,
    IN NTSTATUS ExitStatus,
    IN BOOLEAN DirectTerminate
    );


VOID
PspExitSpecialApc(
    IN PKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

VOID
PspExitProcess(
    IN BOOLEAN TrimAddressSpace,
    IN PEPROCESS Process
    );

NTSTATUS
PspWaitForUsermodeExit(
    IN PEPROCESS         Process
    );

//
// Context Management
//

VOID
PspSetContext(
    OUT PKTRAP_FRAME TrapFrame,
    OUT PKNONVOLATILE_CONTEXT_POINTERS NonVolatileContext,
    IN PCONTEXT Context,
    KPROCESSOR_MODE Mode
    );

VOID
PspGetContext(
    IN PKTRAP_FRAME TrapFrame,
    IN PKNONVOLATILE_CONTEXT_POINTERS NonVolatileContext,
    IN OUT PCONTEXT Context
    );

VOID
PspGetSetContextSpecialApc(
    IN PKAPC Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID *NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    );

VOID
PspExitNormalApc(
    IN PVOID NormalContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

//
// private security routines
//

NTSTATUS
PspInitializeProcessSecurity(
    IN PEPROCESS Parent OPTIONAL,
    IN PEPROCESS Child
    );

VOID
PspDeleteProcessSecurity(
  IN PEPROCESS Process
  );

VOID
PspInitializeThreadSecurity(
    IN PEPROCESS Process,
    IN PETHREAD Thread
    );

VOID
PspDeleteThreadSecurity(
    IN PETHREAD Thread
    );

NTSTATUS
PspAssignPrimaryToken(
    IN PEPROCESS Process,
    IN HANDLE Token OPTIONAL,
    IN PACCESS_TOKEN TokenPointer OPTIONAL
    );

NTSTATUS
PspSetPrimaryToken(
    IN HANDLE ProcessHandle,
    IN PEPROCESS ProcessPointer OPTIONAL, 
    IN HANDLE TokenHandle OPTIONAL,
    IN PACCESS_TOKEN TokenPointer OPTIONAL,
    IN BOOLEAN PrivilegeChecked
    );

//
// Ldt support routines
//

#if defined(i386)
NTSTATUS
PspLdtInitialize(
    );
#endif

//
// Vdm support Routines

#if defined(i386)
NTSTATUS
PspVdmInitialize(
    );
#endif

NTSTATUS
PspQueryLdtInformation(
    IN PEPROCESS Process,
    OUT PVOID LdtInformation,
    IN ULONG LdtInformationLength,
    OUT PULONG ReturnLength
    );

NTSTATUS
PspSetLdtInformation(
    IN PEPROCESS Process,
    IN PVOID LdtInformation,
    IN ULONG LdtInformationLength
    );

NTSTATUS
PspSetLdtSize(
    IN PEPROCESS Process,
    IN PVOID LdtSize,
    IN ULONG LdtSizeLength
    );

VOID
PspDeleteLdt(
    IN PEPROCESS Process
    );

//
// Io handling support routines
//


NTSTATUS
PspSetProcessIoHandlers(
    IN PEPROCESS Process,
    IN PVOID IoHandlerInformation,
    IN ULONG IoHandlerLength
    );

VOID
PspDeleteVdmObjects(
    IN PEPROCESS Process
    );

NTSTATUS
PspQueryDescriptorThread (
    PETHREAD Thread,
    PVOID ThreadInformation,
    ULONG ThreadInformationLength,
    PULONG ReturnLength
    );

//
// Job Object Support Routines
//

VOID
PspInitializeJobStructures(
    VOID
    );

VOID
PspInitializeJobStructuresPhase1(
    VOID
    );

VOID
PspJobTimeLimitsWork(
    IN PVOID Context
    );

VOID
PspJobTimeLimitsDpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
PspJobDelete(
    IN PVOID Object
    );

VOID
PspJobClose (
    IN PEPROCESS Process,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

NTSTATUS
PspAddProcessToJob(
    PEJOB Job,
    PEPROCESS Process
    );

VOID
PspRemoveProcessFromJob(
    PEJOB Job,
    PEPROCESS Process
    );

VOID
PspExitProcessFromJob(
    PEJOB Job,
    PEPROCESS Process
    );

VOID
PspApplyJobLimitsToProcessSet(
    PEJOB Job
    );

VOID
PspApplyJobLimitsToProcess(
    PEJOB Job,
    PEPROCESS Process
    );

BOOLEAN
PspTerminateAllProcessesInJob(
    PEJOB Job,
    NTSTATUS Status,
    BOOLEAN IncCounter
    );

VOID
PspFoldProcessAccountingIntoJob(
    PEJOB Job,
    PEPROCESS Process
    );

NTSTATUS
PspCaptureTokenFilter(
    KPROCESSOR_MODE PreviousMode,
    PJOBOBJECT_SECURITY_LIMIT_INFORMATION SecurityLimitInfo,
    PPS_JOB_TOKEN_FILTER * TokenFilter
    );

KPRIORITY
PspComputeQuantumAndPriority(
    __inout PEPROCESS Process,
    __in PSPROCESSPRIORITYMODE PriorityMode,
    __out PSCHAR QuantumReset
    );

VOID
PspShutdownJobLimits(
    VOID
    );

NTSTATUS
PspTerminateProcess(
    PEPROCESS Process,
    NTSTATUS Status
    );


NTSTATUS
PspGetJobFromSet (
    IN PEJOB ParentJob,
    IN ULONG JobMemberLevel,
    OUT PEJOB *pJob);

NTSTATUS
PspWin32SessionCallout(
    IN  PKWIN32_JOB_CALLOUT CalloutRoutine,
    IN  PKWIN32_JOBCALLOUT_PARAMETERS Parameters,
    IN  ULONG SessionId
    );


//
// This test routine is called on checked systems to test this path
//
VOID
PspImageNotifyTest(
    IN PUNICODE_STRING FullImageName,
    IN HANDLE ProcessId,
    IN PIMAGE_INFO ImageInfo
    );

PEPROCESS
PspGetNextJobProcess (
    IN PEJOB Job,
    IN PEPROCESS Process
    );

VOID
PspQuitNextJobProcess (
    IN PEPROCESS Process
    );

VOID
PspInsertQuotaBlock (
    IN PEPROCESS_QUOTA_BLOCK QuotaBlock
    );



VOID
FORCEINLINE
PspInitializeProcessLock (
    IN PEPROCESS Process
    )
{
    ExInitializePushLock (&Process->ProcessLock);
}

VOID
FORCEINLINE
PspLockProcessExclusive (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockExclusive (&Process->ProcessLock);
}

VOID
FORCEINLINE
PspLockProcessShared (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockShared (&Process->ProcessLock);
}

VOID
FORCEINLINE
PspUnlockProcessShared (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    ExReleasePushLockShared (&Process->ProcessLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockProcessExclusive (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    ExReleasePushLockExclusive (&Process->ProcessLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}


//
// Define macros to lock the security fields of the process and thread
//

VOID
FORCEINLINE
PspLockProcessSecurityExclusive (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    PspLockProcessExclusive (Process, CurrentThread);
}

VOID
FORCEINLINE
PspLockProcessSecurityShared (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    PspLockProcessShared (Process, CurrentThread);
}

VOID
FORCEINLINE
PspUnlockProcessSecurityShared (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    PspUnlockProcessShared (Process, CurrentThread);
}

VOID
FORCEINLINE
PspUnlockProcessSecurityExclusive (
    IN PEPROCESS Process,
    IN PETHREAD CurrentThread
    )
{
    PspUnlockProcessExclusive (Process, CurrentThread);
}


VOID
FORCEINLINE
PspInitializeThreadLock (
    IN PETHREAD Thread
    )
{
    ExInitializePushLock (&Thread->ThreadLock);
}

VOID
FORCEINLINE
PspLockThreadSecurityExclusive (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    )
{
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockExclusive (&Thread->ThreadLock);
}

VOID
FORCEINLINE
PspLockThreadSecurityShared (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    )
{
    KeEnterCriticalRegionThread (&CurrentThread->Tcb);
    ExAcquirePushLockShared (&Thread->ThreadLock);
}


VOID
FORCEINLINE
PspUnlockThreadSecurityShared (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    )
{
    ExReleasePushLockShared (&Thread->ThreadLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockThreadSecurityExclusive (
    IN PETHREAD Thread,
    IN PETHREAD CurrentThread
    )
{
    ExReleasePushLockExclusive (&Thread->ThreadLock);
    KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
}

//
// Define macros to lock the global process list
//

extern KGUARDED_MUTEX PspActiveProcessMutex;

VOID
FORCEINLINE
PspInitializeProcessListLock (
    VOID
    )
{
    KeInitializeGuardedMutex (&PspActiveProcessMutex);
};

VOID
FORCEINLINE
PspLockProcessList (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspActiveProcessMutex);
}

VOID
FORCEINLINE
PspUnlockProcessList (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspActiveProcessMutex);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

//
// Routines to lock and unlock the job list mutex
//

extern KGUARDED_MUTEX PspJobListLock;

VOID
FORCEINLINE
PspInitializeJobListLock (
    VOID
    )
{
    KeInitializeGuardedMutex (&PspJobListLock);
}


VOID
FORCEINLINE
PspLockJobListExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspJobListLock);
}

VOID
FORCEINLINE
PspLockJobListShared (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspJobListLock);
}

VOID
FORCEINLINE
PspUnlockJobListExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspJobListLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockJobListShared (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspJobListLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

//
// Routines to lock the job memory list lock
//

VOID
FORCEINLINE
PspInitializeJobLimitsLock (
    IN PEJOB Job
    )
{
    KeInitializeGuardedMutex (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspLockJobLimitsExclusive (
    IN PEJOB    Job,
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspLockJobLimitsExclusiveUnsafe (
    IN PEJOB    Job
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeAcquireGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspLockJobLimitsShared (
    IN PEJOB    Job,
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspLockJobLimitsSharedUnsafe (
    IN PEJOB Job
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeAcquireGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspUnlockJobLimitsExclusive (
    IN PEJOB    Job,
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&Job->MemoryLimitsLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockJobLimitsExclusiveUnsafe (
    IN PEJOB Job
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeReleaseGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

VOID
FORCEINLINE
PspUnlockJobLimitsShared (
    IN PEJOB    Job,
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&Job->MemoryLimitsLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspUnlockJobLimitsSharedUnsafe (
    IN PEJOB Job
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeReleaseGuardedMutexUnsafe (&Job->MemoryLimitsLock);
}

//
// Routines to lock job time limits structures
//
extern KGUARDED_MUTEX PspJobTimeLimitsLock;

VOID
FORCEINLINE
PspInitializeJobTimeLimitsLock (
    VOID
    )
{
    KeInitializeGuardedMutex (&PspJobTimeLimitsLock);
}

VOID
FORCEINLINE
PspLockJobTimeLimitsExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspJobTimeLimitsLock);
}

VOID
FORCEINLINE
PspUnlockJobTimeLimitsExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspJobTimeLimitsLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspLockJobTimeLimitsShared (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspJobTimeLimitsLock);
}

VOID
FORCEINLINE
PspUnlockJobTimeLimitsShared (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspJobTimeLimitsLock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

//
// Routines for locking working set change lock
//
VOID
FORCEINLINE
PspInitializeWorkingSetChangeLock (
    VOID
    )
{
    KeInitializeGuardedMutex (&PspWorkingSetChangeHead.Lock);
}

VOID
FORCEINLINE
PspLockWorkingSetChangeExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeEnterGuardedRegionThread (&CurrentThread->Tcb);
    KeAcquireGuardedMutexUnsafe (&PspWorkingSetChangeHead.Lock);
}

VOID
FORCEINLINE
PspUnlockWorkingSetChangeExclusive (
    IN PETHREAD CurrentThread
    )
{
    KeReleaseGuardedMutexUnsafe (&PspWorkingSetChangeHead.Lock);
    KeLeaveGuardedRegionThread (&CurrentThread->Tcb);
}

VOID
FORCEINLINE
PspLockWorkingSetChangeExclusiveUnsafe (
    VOID
    )
{
    ASSERT (KeAreAllApcsDisabled());
    KeAcquireGuardedMutexUnsafe (&PspWorkingSetChangeHead.Lock);
}

VOID
FORCEINLINE
PspUnlockWorkingSetChangeExclusiveUnsafe (
    VOID
    )
{
    KeReleaseGuardedMutexUnsafe (&PspWorkingSetChangeHead.Lock);
    ASSERT (KeAreAllApcsDisabled());
}

//
//
// Global Data
//

extern PHANDLE_TABLE PspCidTable;
extern HANDLE PspInitialSystemProcessHandle;
extern PACCESS_TOKEN PspBootAccessToken;
extern SYSTEM_DLL PspSystemDll;
extern PETHREAD PspShutdownThread;

extern ULONG PspDefaultPagedLimit;
extern ULONG PspDefaultNonPagedLimit;
extern ULONG PspDefaultPagefileLimit;
extern ULONG PsMinimumWorkingSet;

extern EPROCESS_QUOTA_BLOCK PspDefaultQuotaBlock;
extern BOOLEAN PspDoingGiveBacks;

extern PKWIN32_PROCESS_CALLOUT PspW32ProcessCallout;
extern PKWIN32_THREAD_CALLOUT PspW32ThreadCallout;
extern PKWIN32_JOB_CALLOUT PspW32JobCallout;
extern ULONG PspW32ProcessSize;
extern ULONG PspW32ThreadSize;
extern SCHAR PspForegroundQuantum[3];


#define PSP_NUMBER_OF_SCHEDULING_CLASSES    10
#define PSP_DEFAULT_SCHEDULING_CLASSES      5

extern const SCHAR PspJobSchedulingClasses[PSP_NUMBER_OF_SCHEDULING_CLASSES];
extern BOOLEAN PspUseJobSchedulingClasses;

extern LIST_ENTRY PspJobList;
extern KDPC PspJobLimeLimitsDpc;
extern KTIMER PspJobTimeLimitsTimer;
extern WORK_QUEUE_ITEM PspJobTimeLimitsWorkItem;
extern KSPIN_LOCK PspQuotaLock;

#endif // _PSP_

