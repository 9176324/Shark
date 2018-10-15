
/*++ BUILD Version: 0008    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntexapi.h

Abstract:

    This module is the header file for the all the system services that
    are contained in the "ex" directory.

--*/

#ifndef _NTEXAPI_
#define _NTEXAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Delay thread execution.
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDelayExecution (
    __in BOOLEAN Alertable,
    __in PLARGE_INTEGER DelayInterval
    );

//
// Query and set system environment variables.
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySystemEnvironmentValue (
    __in PUNICODE_STRING VariableName,
    __out_bcount(ValueLength) PWSTR VariableValue,
    __in USHORT ValueLength,
    __out_opt PUSHORT ReturnLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSystemEnvironmentValue (
    __in PUNICODE_STRING VariableName,
    __in PUNICODE_STRING VariableValue
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySystemEnvironmentValueEx (
    __in PUNICODE_STRING VariableName,
    __in LPGUID VendorGuid,
    __out_bcount_opt(*ValueLength) PVOID Value,
    __inout PULONG ValueLength,
    __out_opt PULONG Attributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSystemEnvironmentValueEx (
    __in PUNICODE_STRING VariableName,
    __in LPGUID VendorGuid,
    __in_bcount_opt(ValueLength) PVOID Value,
    __in ULONG ValueLength,
    __in ULONG Attributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateSystemEnvironmentValuesEx (
    __in ULONG InformationClass,
    __out PVOID Buffer,
    __inout PULONG BufferLength
    );

// begin_nthal

#define VARIABLE_ATTRIBUTE_NON_VOLATILE 0x00000001

#define VARIABLE_INFORMATION_NAMES  1
#define VARIABLE_INFORMATION_VALUES 2

typedef struct _VARIABLE_NAME {
    ULONG NextEntryOffset;
    GUID VendorGuid;
    WCHAR Name[ANYSIZE_ARRAY];
} VARIABLE_NAME, *PVARIABLE_NAME;

typedef struct _VARIABLE_NAME_AND_VALUE {
    ULONG NextEntryOffset;
    ULONG ValueOffset;
    ULONG ValueLength;
    ULONG Attributes;
    GUID VendorGuid;
    WCHAR Name[ANYSIZE_ARRAY];
    //UCHAR Value[ANYSIZE_ARRAY];
} VARIABLE_NAME_AND_VALUE, *PVARIABLE_NAME_AND_VALUE;

// end_nthal

//
// Boot entry management APIs.
//

typedef struct _FILE_PATH {
    ULONG Version;
    ULONG Length;
    ULONG Type;
    UCHAR FilePath[ANYSIZE_ARRAY];
} FILE_PATH, *PFILE_PATH;

#define FILE_PATH_VERSION 1

#define FILE_PATH_TYPE_ARC           1
#define FILE_PATH_TYPE_ARC_SIGNATURE 2
#define FILE_PATH_TYPE_NT            3
#define FILE_PATH_TYPE_EFI           4

#define FILE_PATH_TYPE_MIN FILE_PATH_TYPE_ARC
#define FILE_PATH_TYPE_MAX FILE_PATH_TYPE_EFI

typedef struct _WINDOWS_OS_OPTIONS {
    UCHAR Signature[8];
    ULONG Version;
    ULONG Length;
    ULONG OsLoadPathOffset;
    WCHAR OsLoadOptions[ANYSIZE_ARRAY];
    //FILE_PATH OsLoadPath;
} WINDOWS_OS_OPTIONS, *PWINDOWS_OS_OPTIONS;

#define WINDOWS_OS_OPTIONS_SIGNATURE "WINDOWS"

#define WINDOWS_OS_OPTIONS_VERSION 1

typedef struct _BOOT_ENTRY {
    ULONG Version;
    ULONG Length;
    ULONG Id;
    ULONG Attributes;
    ULONG FriendlyNameOffset;
    ULONG BootFilePathOffset;
    ULONG OsOptionsLength;
    UCHAR OsOptions[ANYSIZE_ARRAY];
    //WCHAR FriendlyName[ANYSIZE_ARRAY];
    //FILE_PATH BootFilePath;
} BOOT_ENTRY, *PBOOT_ENTRY;

#define BOOT_ENTRY_VERSION 1

#define BOOT_ENTRY_ATTRIBUTE_ACTIVE             0x00000001
#define BOOT_ENTRY_ATTRIBUTE_DEFAULT            0x00000002
#define BOOT_ENTRY_ATTRIBUTE_WINDOWS            0x00000004
#define BOOT_ENTRY_ATTRIBUTE_REMOVABLE_MEDIA    0x00000008

#define BOOT_ENTRY_ATTRIBUTE_VALID_BITS (  \
            BOOT_ENTRY_ATTRIBUTE_ACTIVE  | \
            BOOT_ENTRY_ATTRIBUTE_DEFAULT   \
            )

typedef struct _BOOT_OPTIONS {
    ULONG Version;
    ULONG Length;
    ULONG Timeout;
    ULONG CurrentBootEntryId;
    ULONG NextBootEntryId;
    WCHAR HeadlessRedirection[ANYSIZE_ARRAY];
} BOOT_OPTIONS, *PBOOT_OPTIONS;

#define BOOT_OPTIONS_VERSION 1

#define BOOT_OPTIONS_FIELD_TIMEOUT              0x00000001
#define BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID   0x00000002
#define BOOT_OPTIONS_FIELD_HEADLESS_REDIRECTION 0x00000004

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAddBootEntry (
    __in PBOOT_ENTRY BootEntry,
    __out_opt PULONG Id
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteBootEntry (
    __in ULONG Id
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtModifyBootEntry (
    __in PBOOT_ENTRY BootEntry
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateBootEntries (
    __out_bcount_opt(*BufferLength) PVOID Buffer,
    __inout PULONG BufferLength
    );

typedef struct _BOOT_ENTRY_LIST {
    ULONG NextEntryOffset;
    BOOT_ENTRY BootEntry;
} BOOT_ENTRY_LIST, *PBOOT_ENTRY_LIST;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryBootEntryOrder (
    __out_ecount_opt(*Count) PULONG Ids,
    __inout PULONG Count
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetBootEntryOrder (
    __in_ecount(Count) PULONG Ids,
    __in ULONG Count
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryBootOptions (
    __out_bcount_opt(*BootOptionsLength) PBOOT_OPTIONS BootOptions,
    __inout PULONG BootOptionsLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetBootOptions (
    __in PBOOT_OPTIONS BootOptions,
    __in ULONG FieldsToChange
    );

#define BOOT_OPTIONS_FIELD_COUNTDOWN            0x00000001
#define BOOT_OPTIONS_FIELD_NEXT_BOOT_ENTRY_ID   0x00000002
#define BOOT_OPTIONS_FIELD_HEADLESS_REDIRECTION 0x00000004

NTSYSCALLAPI
NTSTATUS
NTAPI
NtTranslateFilePath (
    __in PFILE_PATH InputFilePath,
    __in ULONG OutputType,
    __out_bcount_opt(*OutputFilePathLength) PFILE_PATH OutputFilePath,
    __inout_opt PULONG OutputFilePathLength
    );

//
// Driver entry management APIs.
//

typedef struct _EFI_DRIVER_ENTRY {
    ULONG Version;
    ULONG Length;
    ULONG Id;
    ULONG FriendlyNameOffset;
    ULONG DriverFilePathOffset;
    //WCHAR FriendlyName[ANYSIZE_ARRAY];
    //FILE_PATH DriverFilePath;
} EFI_DRIVER_ENTRY, *PEFI_DRIVER_ENTRY;

typedef struct _EFI_DRIVER_ENTRY_LIST {
    ULONG NextEntryOffset;
    EFI_DRIVER_ENTRY DriverEntry;
} EFI_DRIVER_ENTRY_LIST, *PEFI_DRIVER_ENTRY_LIST;

#define EFI_DRIVER_ENTRY_VERSION 1

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAddDriverEntry (
    __in PEFI_DRIVER_ENTRY DriverEntry,
    __out_opt PULONG Id
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteDriverEntry (
    __in ULONG Id
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtModifyDriverEntry (
    __in PEFI_DRIVER_ENTRY DriverEntry
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateDriverEntries (
    __out_bcount(*BufferLength) PVOID Buffer,
    __inout PULONG BufferLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryDriverEntryOrder (
    __out_ecount(*Count) PULONG Ids,
    __inout PULONG Count
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetDriverEntryOrder (
    __in_ecount(Count) PULONG Ids,
    __in ULONG Count
    );

// begin_ntddk 

//
// Firmware Table provider definitions 
//

typedef enum _SYSTEM_FIRMWARE_TABLE_ACTION {
    SystemFirmwareTable_Enumerate,
    SystemFirmwareTable_Get
} SYSTEM_FIRMWARE_TABLE_ACTION;

typedef struct _SYSTEM_FIRMWARE_TABLE_INFORMATION {
    ULONG                           ProviderSignature;
    SYSTEM_FIRMWARE_TABLE_ACTION    Action;
    ULONG                           TableID;
    ULONG                           TableBufferLength;
    UCHAR                           TableBuffer[ANYSIZE_ARRAY];
} SYSTEM_FIRMWARE_TABLE_INFORMATION, *PSYSTEM_FIRMWARE_TABLE_INFORMATION;

typedef NTSTATUS (__cdecl *PFNFTH)(PSYSTEM_FIRMWARE_TABLE_INFORMATION);

typedef struct _SYSTEM_FIRMWARE_TABLE_HANDLER {
    ULONG       ProviderSignature;
    BOOLEAN     Register;
    PFNFTH      FirmwareTableHandler;
    PVOID       DriverObject;
} SYSTEM_FIRMWARE_TABLE_HANDLER, *PSYSTEM_FIRMWARE_TABLE_HANDLER;

// end_ntddk

// begin_ntifs begin_wdm begin_ntddk
//
// Event Specific Access Rights.
//

#define EVENT_QUERY_STATE       0x0001
#define EVENT_MODIFY_STATE      0x0002  // winnt
#define EVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) // winnt

// end_ntifs end_wdm end_ntddk

//
// Event Information Classes.
//

typedef enum _EVENT_INFORMATION_CLASS {
    EventBasicInformation
} EVENT_INFORMATION_CLASS;

//
// Event Information Structures.
//

typedef struct _EVENT_BASIC_INFORMATION {
    EVENT_TYPE EventType;
    LONG EventState;
} EVENT_BASIC_INFORMATION, *PEVENT_BASIC_INFORMATION;

//
// Event object function definitions.
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtClearEvent (
    __in HANDLE EventHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateEvent (
    __out PHANDLE EventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in EVENT_TYPE EventType,
    __in BOOLEAN InitialState
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenEvent (
    __out PHANDLE EventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtPulseEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryEvent (
    __in HANDLE EventHandle,
    __in EVENT_INFORMATION_CLASS EventInformationClass,
    __out_bcount(EventInformationLength) PVOID EventInformation,
    __in ULONG EventInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtResetEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetEventBoostPriority (
    __in HANDLE EventHandle
    );

//
// Event Specific Access Rights.
//

#define EVENT_PAIR_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE)

//
// Event pair object function definitions.
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateEventPair (
    __out PHANDLE EventPairHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenEventPair (
    __out PHANDLE EventPairHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWaitLowEventPair ( 
    __in HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWaitHighEventPair (
    __in HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetLowWaitHighEventPair (
    __in HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetHighWaitLowEventPair (
    __in HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetLowEventPair (
    __in HANDLE EventPairHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetHighEventPair (
    __in HANDLE EventPairHandle
    );

//
// Mutant Specific Access Rights.
//
// begin_winnt

#define MUTANT_QUERY_STATE      0x0001

#define MUTANT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          MUTANT_QUERY_STATE)
// end_winnt

//
// Mutant Information Classes.
//

typedef enum _MUTANT_INFORMATION_CLASS {
    MutantBasicInformation
} MUTANT_INFORMATION_CLASS;

//
// Mutant Information Structures.
//

typedef struct _MUTANT_BASIC_INFORMATION {
    LONG CurrentCount;
    BOOLEAN OwnedByCaller;
    BOOLEAN AbandonedState;
} MUTANT_BASIC_INFORMATION, *PMUTANT_BASIC_INFORMATION;

//
// Mutant object function definitions.
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateMutant (
    __out PHANDLE MutantHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in BOOLEAN InitialOwner
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenMutant (
    __out PHANDLE MutantHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryMutant (
    __in HANDLE MutantHandle,
    __in MUTANT_INFORMATION_CLASS MutantInformationClass,
    __out_bcount(MutantInformationLength) PVOID MutantInformation,
    __in ULONG MutantInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReleaseMutant (
    __in HANDLE MutantHandle,
    __out_opt PLONG PreviousCount
    );

// begin_ntifs begin_wdm begin_ntddk
//
// Semaphore Specific Access Rights.
//

#define SEMAPHORE_QUERY_STATE       0x0001
#define SEMAPHORE_MODIFY_STATE      0x0002  // winnt

#define SEMAPHORE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3) // winnt

// end_ntifs end_wdm end_ntddk

//
// Semaphore Information Classes.
//

typedef enum _SEMAPHORE_INFORMATION_CLASS {
    SemaphoreBasicInformation
} SEMAPHORE_INFORMATION_CLASS;

//
// Semaphore Information Structures.
//

typedef struct _SEMAPHORE_BASIC_INFORMATION {
    LONG CurrentCount;
    LONG MaximumCount;
} SEMAPHORE_BASIC_INFORMATION, *PSEMAPHORE_BASIC_INFORMATION;

//
// Semaphore object function definitions.
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateSemaphore (
    __out PHANDLE SemaphoreHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in LONG InitialCount,
    __in LONG MaximumCount
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenSemaphore(
    __out PHANDLE SemaphoreHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySemaphore (
    __in HANDLE SemaphoreHandle,
    __in SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
    __out_bcount(SemaphoreInformationLength) PVOID SemaphoreInformation,
    __in ULONG SemaphoreInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReleaseSemaphore(
    __in HANDLE SemaphoreHandle,
    __in LONG ReleaseCount,
    __out_opt PLONG PreviousCount
    );

// begin_winnt
//
// Timer Specific Access Rights.
//

#define TIMER_QUERY_STATE       0x0001
#define TIMER_MODIFY_STATE      0x0002

#define TIMER_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                          TIMER_QUERY_STATE|TIMER_MODIFY_STATE)

// end_winnt
//
// Timer Information Classes.
//

typedef enum _TIMER_INFORMATION_CLASS {
    TimerBasicInformation
} TIMER_INFORMATION_CLASS;

//
// Timer Information Structures.
//

typedef struct _TIMER_BASIC_INFORMATION {
    LARGE_INTEGER RemainingTime;
    BOOLEAN TimerState;
} TIMER_BASIC_INFORMATION, *PTIMER_BASIC_INFORMATION;

// begin_ntddk
//
// Timer APC routine definition.
//

typedef
VOID
(*PTIMER_APC_ROUTINE) (
    __in PVOID TimerContext,
    __in ULONG TimerLowValue,
    __in LONG TimerHighValue
    );

// end_ntddk

//
// Timer object function definitions.
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateTimer (
    __out PHANDLE TimerHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in TIMER_TYPE TimerType
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenTimer (
    __out PHANDLE TimerHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCancelTimer (
    __in HANDLE TimerHandle,
    __out_opt PBOOLEAN CurrentState
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryTimer (
    __in HANDLE TimerHandle,
    __in TIMER_INFORMATION_CLASS TimerInformationClass,
    __out_bcount(TimerInformationLength) PVOID TimerInformation,
    __in ULONG TimerInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetTimer (
    __in HANDLE TimerHandle,
    __in PLARGE_INTEGER DueTime,
    __in_opt PTIMER_APC_ROUTINE TimerApcRoutine,
    __in_opt PVOID TimerContext,
    __in BOOLEAN ResumeTimer,
    __in_opt LONG Period,
    __out_opt PBOOLEAN PreviousState
    );

//
// System Time and Timer function definitions
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySystemTime (
    __out PLARGE_INTEGER SystemTime
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSystemTime (
    __in_opt PLARGE_INTEGER SystemTime,
    __out_opt PLARGE_INTEGER PreviousTime
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryTimerResolution (
    __out PULONG MaximumTime,
    __out PULONG MinimumTime,
    __out PULONG CurrentTime
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetTimerResolution (
    __in ULONG DesiredTime,
    __in BOOLEAN SetResolution,
    __out PULONG ActualTime
    );

//
//  Locally Unique Identifier (LUID) allocation
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAllocateLocallyUniqueId (
    __out PLUID Luid
    );

//
//  Universally Unique Identifier (UUID) time allocation
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetUuidSeed (
    __in PCHAR Seed
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAllocateUuids (
    __out PULARGE_INTEGER Time,
    __out PULONG Range,
    __out PULONG Sequence,
    __out PCHAR Seed
    );

//
// Profile Object Definitions
//

#define PROFILE_CONTROL           0x0001
#define PROFILE_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | PROFILE_CONTROL)

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateProfile (
    __out PHANDLE ProfileHandle,
    __in HANDLE Process OPTIONAL,
    __in PVOID ProfileBase,
    __in SIZE_T ProfileSize,
    __in ULONG BucketSize,
    __in PULONG Buffer,
    __in ULONG BufferSize,
    __in KPROFILE_SOURCE ProfileSource,
    __in KAFFINITY Affinity
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtStartProfile (
    __in HANDLE ProfileHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtStopProfile (
    __in HANDLE ProfileHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetIntervalProfile (
    __in ULONG Interval,
    __in KPROFILE_SOURCE Source
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryIntervalProfile (
    __in KPROFILE_SOURCE ProfileSource,
    __out PULONG Interval
    );

//
// Performance Counter Definitions
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryPerformanceCounter (
    __out PLARGE_INTEGER PerformanceCounter,
    __out_opt PLARGE_INTEGER PerformanceFrequency
    );

#define KEYEDEVENT_WAIT 0x0001
#define KEYEDEVENT_WAKE 0x0002
#define KEYEDEVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | KEYEDEVENT_WAIT | KEYEDEVENT_WAKE)

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateKeyedEvent (
    __out PHANDLE KeyedEventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG Flags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenKeyedEvent (
    __out PHANDLE KeyedEventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReleaseKeyedEvent (
    __in HANDLE KeyedEventHandle,
    __in PVOID KeyValue,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWaitForKeyedEvent (
    __in HANDLE KeyedEventHandle,
    __in PVOID KeyValue,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    );

//
// Nt Api Profile Definitions
//
// Nt Api Profiling data structure
//

typedef struct _NAPDATA {
    ULONG NapLock;
    ULONG Calls;
    ULONG TimingErrors;
    LARGE_INTEGER TotalTime;
    LARGE_INTEGER FirstTime;
    LARGE_INTEGER MaxTime;
    LARGE_INTEGER MinTime;
} NAPDATA, *PNAPDATA;

NTSTATUS
NapClearData (
    VOID
    );

NTSTATUS
NapRetrieveData (
    OUT NAPDATA *NapApiData,
    OUT PCHAR **NapApiNames,
    OUT PLARGE_INTEGER *NapCounterFrequency
    );

NTSTATUS
NapGetApiCount (
    OUT PULONG NapApiCount
    );

NTSTATUS
NapPause (
    VOID
    );

NTSTATUS
NapResume (
    VOID
    );

// begin_ntifs begin_ntddk

//
//  Driver Verifier Definitions
//

typedef ULONG_PTR (*PDRIVER_VERIFIER_THUNK_ROUTINE) (
    IN PVOID Context
    );

//
//  This structure is passed in by drivers that want to thunk callers of
//  their exports.
//

typedef struct _DRIVER_VERIFIER_THUNK_PAIRS {
    PDRIVER_VERIFIER_THUNK_ROUTINE  PristineRoutine;
    PDRIVER_VERIFIER_THUNK_ROUTINE  NewRoutine;
} DRIVER_VERIFIER_THUNK_PAIRS, *PDRIVER_VERIFIER_THUNK_PAIRS;

//
//  Driver Verifier flags.
//

#define DRIVER_VERIFIER_SPECIAL_POOLING             0x0001
#define DRIVER_VERIFIER_FORCE_IRQL_CHECKING         0x0002
#define DRIVER_VERIFIER_INJECT_ALLOCATION_FAILURES  0x0004
#define DRIVER_VERIFIER_TRACK_POOL_ALLOCATIONS      0x0008
#define DRIVER_VERIFIER_IO_CHECKING                 0x0010

// end_ntifs end_ntddk

#define DRIVER_VERIFIER_DEADLOCK_DETECTION          0x0020
#define DRIVER_VERIFIER_ENHANCED_IO_CHECKING        0x0040
#define DRIVER_VERIFIER_DMA_VERIFIER                0x0080
#define DRIVER_VERIFIER_HARDWARE_VERIFICATION       0x0100
#define DRIVER_VERIFIER_SYSTEM_BIOS_VERIFICATION    0x0200
#define DRIVER_VERIFIER_EXPOSE_IRP_HISTORY          0x0400

//
// System Information Classes.
//

typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,
    SystemProcessorInformation,             // obsolete...delete
    SystemPerformanceInformation,
    SystemTimeOfDayInformation,
    SystemPathInformation,
    SystemProcessInformation,
    SystemCallCountInformation,
    SystemDeviceInformation,
    SystemProcessorPerformanceInformation,
    SystemFlagsInformation,
    SystemCallTimeInformation,
    SystemModuleInformation,
    SystemLocksInformation,
    SystemStackTraceInformation,
    SystemPagedPoolInformation,
    SystemNonPagedPoolInformation,
    SystemHandleInformation,
    SystemObjectInformation,
    SystemPageFileInformation,
    SystemVdmInstemulInformation,
    SystemVdmBopInformation,
    SystemFileCacheInformation,
    SystemPoolTagInformation,
    SystemInterruptInformation,
    SystemDpcBehaviorInformation,
    SystemFullMemoryInformation,
    SystemLoadGdiDriverInformation,
    SystemUnloadGdiDriverInformation,
    SystemTimeAdjustmentInformation,
    SystemSummaryMemoryInformation,
    SystemMirrorMemoryInformation,
    SystemPerformanceTraceInformation,
    SystemObsolete0,
    SystemExceptionInformation,
    SystemCrashDumpStateInformation,
    SystemKernelDebuggerInformation,
    SystemContextSwitchInformation,
    SystemRegistryQuotaInformation,
    SystemExtendServiceTableInformation,
    SystemPrioritySeperation,
    SystemVerifierAddDriverInformation,
    SystemVerifierRemoveDriverInformation,
    SystemProcessorIdleInformation,
    SystemLegacyDriverInformation,
    SystemCurrentTimeZoneInformation,
    SystemLookasideInformation,
    SystemTimeSlipNotification,
    SystemSessionCreate,
    SystemSessionDetach,
    SystemSessionInformation,
    SystemRangeStartInformation,
    SystemVerifierInformation,
    SystemVerifierThunkExtend,
    SystemSessionProcessInformation,
    SystemLoadGdiDriverInSystemSpace,
    SystemNumaProcessorMap,
    SystemPrefetcherInformation,
    SystemExtendedProcessInformation,
    SystemRecommendedSharedDataAlignment,
    SystemComPlusPackage,
    SystemNumaAvailableMemory,
    SystemProcessorPowerInformation,
    SystemEmulationBasicInformation,
    SystemEmulationProcessorInformation,
    SystemExtendedHandleInformation,
    SystemLostDelayedWriteInformation,
    SystemBigPoolInformation,
    SystemSessionPoolTagInformation,
    SystemSessionMappedViewInformation,
    SystemHotpatchInformation,
    SystemObjectSecurityMode,
    SystemWatchdogTimerHandler,
    SystemWatchdogTimerInformation,
    SystemLogicalProcessorInformation,
    SystemWow64SharedInformation,
    SystemRegisterFirmwareTableInformationHandler,
    SystemFirmwareTableInformation,
    SystemModuleInformationEx,
    SystemVerifierTriageInformation,
    SystemSuperfetchInformation,
    SystemMemoryListInformation,
    SystemFileCacheInformationEx,
    MaxSystemInfoClass  // MaxSystemInfoClass should always be the last enum
} SYSTEM_INFORMATION_CLASS;

//
// System Information Structures.
//

// begin_winnt
#define TIME_ZONE_ID_UNKNOWN  0
#define TIME_ZONE_ID_STANDARD 1
#define TIME_ZONE_ID_DAYLIGHT 2
// end_winnt

typedef struct _SYSTEM_VDM_INSTEMUL_INFO {
    ULONG SegmentNotPresent ;
    ULONG VdmOpcode0F       ;
    ULONG OpcodeESPrefix    ;
    ULONG OpcodeCSPrefix    ;
    ULONG OpcodeSSPrefix    ;
    ULONG OpcodeDSPrefix    ;
    ULONG OpcodeFSPrefix    ;
    ULONG OpcodeGSPrefix    ;
    ULONG OpcodeOPER32Prefix;
    ULONG OpcodeADDR32Prefix;
    ULONG OpcodeINSB        ;
    ULONG OpcodeINSW        ;
    ULONG OpcodeOUTSB       ;
    ULONG OpcodeOUTSW       ;
    ULONG OpcodePUSHF       ;
    ULONG OpcodePOPF        ;
    ULONG OpcodeINTnn       ;
    ULONG OpcodeINTO        ;
    ULONG OpcodeIRET        ;
    ULONG OpcodeINBimm      ;
    ULONG OpcodeINWimm      ;
    ULONG OpcodeOUTBimm     ;
    ULONG OpcodeOUTWimm     ;
    ULONG OpcodeINB         ;
    ULONG OpcodeINW         ;
    ULONG OpcodeOUTB        ;
    ULONG OpcodeOUTW        ;
    ULONG OpcodeLOCKPrefix  ;
    ULONG OpcodeREPNEPrefix ;
    ULONG OpcodeREPPrefix   ;
    ULONG OpcodeHLT         ;
    ULONG OpcodeCLI         ;
    ULONG OpcodeSTI         ;
    ULONG BopCount          ;
} SYSTEM_VDM_INSTEMUL_INFO, *PSYSTEM_VDM_INSTEMUL_INFO;

typedef struct _SYSTEM_TIMEOFDAY_INFORMATION {
    LARGE_INTEGER BootTime;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER TimeZoneBias;
    ULONG TimeZoneId;
    ULONG Reserved;
    ULONGLONG BootTimeBias;
    ULONGLONG SleepTimeBias;
} SYSTEM_TIMEOFDAY_INFORMATION, *PSYSTEM_TIMEOFDAY_INFORMATION;

#if defined(_WIN64)
typedef ULONG SYSINF_PAGE_COUNT;
#else
typedef SIZE_T SYSINF_PAGE_COUNT;
#endif

typedef struct _SYSTEM_BASIC_INFORMATION {
    ULONG Reserved;
    ULONG TimerResolution;
    ULONG PageSize;
    SYSINF_PAGE_COUNT NumberOfPhysicalPages;
    SYSINF_PAGE_COUNT LowestPhysicalPageNumber;
    SYSINF_PAGE_COUNT HighestPhysicalPageNumber;
    ULONG AllocationGranularity;
    ULONG_PTR MinimumUserModeAddress;
    ULONG_PTR MaximumUserModeAddress;
    ULONG_PTR ActiveProcessorsAffinityMask;
    CCHAR NumberOfProcessors;
} SYSTEM_BASIC_INFORMATION, *PSYSTEM_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_INFORMATION {
    USHORT ProcessorArchitecture;
    USHORT ProcessorLevel;
    USHORT ProcessorRevision;
    USHORT Reserved;
    ULONG ProcessorFeatureBits;
} SYSTEM_PROCESSOR_INFORMATION, *PSYSTEM_PROCESSOR_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;          // DEVL only
    LARGE_INTEGER InterruptTime;    // DEVL only
    ULONG InterruptCount;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION, *PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_PROCESSOR_IDLE_INFORMATION {
    ULONGLONG IdleTime;
    ULONGLONG C1Time;
    ULONGLONG C2Time;
    ULONGLONG C3Time;
    ULONG     C1Transitions;
    ULONG     C2Transitions;
    ULONG     C3Transitions;
    ULONG     Padding;
} SYSTEM_PROCESSOR_IDLE_INFORMATION, *PSYSTEM_PROCESSOR_IDLE_INFORMATION;

#define MAXIMUM_NUMA_NODES 16

typedef struct _SYSTEM_NUMA_INFORMATION {
    ULONG       HighestNodeNumber;
    ULONG       Reserved;
    union {
        ULONGLONG   ActiveProcessorsAffinityMask[MAXIMUM_NUMA_NODES];
        ULONGLONG   AvailableMemory[MAXIMUM_NUMA_NODES];
    };
} SYSTEM_NUMA_INFORMATION, *PSYSTEM_NUMA_INFORMATION;

// begin_winnt

typedef enum _LOGICAL_PROCESSOR_RELATIONSHIP {
    RelationProcessorCore,
    RelationNumaNode,
    RelationCache
} LOGICAL_PROCESSOR_RELATIONSHIP;

#define LTP_PC_SMT 0x1

typedef enum _PROCESSOR_CACHE_TYPE {
    CacheUnified,
    CacheInstruction,
    CacheData,
    CacheTrace
} PROCESSOR_CACHE_TYPE;

#define CACHE_FULLY_ASSOCIATIVE 0xFF

typedef struct _CACHE_DESCRIPTOR {
    UCHAR  Level;
    UCHAR  Associativity;
    USHORT LineSize;
    ULONG  Size;
    PROCESSOR_CACHE_TYPE Type;
} CACHE_DESCRIPTOR, *PCACHE_DESCRIPTOR;

typedef struct _SYSTEM_LOGICAL_PROCESSOR_INFORMATION {
    ULONG_PTR   ProcessorMask;
    LOGICAL_PROCESSOR_RELATIONSHIP Relationship;
    union {
        struct {
            UCHAR Flags;
        } ProcessorCore;
        struct {
            ULONG NodeNumber;
        } NumaNode;
        CACHE_DESCRIPTOR Cache;
        ULONGLONG  Reserved[2];
    };
} SYSTEM_LOGICAL_PROCESSOR_INFORMATION, *PSYSTEM_LOGICAL_PROCESSOR_INFORMATION;

// end_winnt

typedef struct _SYSTEM_PROCESSOR_POWER_INFORMATION {
    UCHAR       CurrentFrequency;
    UCHAR       ThermalLimitFrequency;
    UCHAR       ConstantThrottleFrequency;
    UCHAR       DegradedThrottleFrequency;
    UCHAR       LastBusyFrequency;
    UCHAR       LastC3Frequency;
    UCHAR       LastAdjustedBusyFrequency;
    UCHAR       ProcessorMinThrottle;
    UCHAR       ProcessorMaxThrottle;
    ULONG       NumberOfFrequencies;
    ULONG       PromotionCount;
    ULONG       DemotionCount;
    ULONG       ErrorCount;
    ULONG       RetryCount;
    ULONGLONG   CurrentFrequencyTime;
    ULONGLONG   CurrentProcessorTime;
    ULONGLONG   CurrentProcessorIdleTime;
    ULONGLONG   LastProcessorTime;
    ULONGLONG   LastProcessorIdleTime;
} SYSTEM_PROCESSOR_POWER_INFORMATION, *PSYSTEM_PROCESSOR_POWER_INFORMATION;

typedef struct _SYSTEM_QUERY_TIME_ADJUST_INFORMATION {
    ULONG TimeAdjustment;
    ULONG TimeIncrement;
    BOOLEAN Enable;
} SYSTEM_QUERY_TIME_ADJUST_INFORMATION, *PSYSTEM_QUERY_TIME_ADJUST_INFORMATION;

typedef struct _SYSTEM_SET_TIME_ADJUST_INFORMATION {
    ULONG TimeAdjustment;
    BOOLEAN Enable;
} SYSTEM_SET_TIME_ADJUST_INFORMATION, *PSYSTEM_SET_TIME_ADJUST_INFORMATION;

typedef struct _SYSTEM_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleProcessTime;
    LARGE_INTEGER IoReadTransferCount;
    LARGE_INTEGER IoWriteTransferCount;
    LARGE_INTEGER IoOtherTransferCount;
    ULONG IoReadOperationCount;
    ULONG IoWriteOperationCount;
    ULONG IoOtherOperationCount;
    ULONG AvailablePages;
    SYSINF_PAGE_COUNT CommittedPages;
    SYSINF_PAGE_COUNT CommitLimit;
    SYSINF_PAGE_COUNT PeakCommitment;
    ULONG PageFaultCount;
    ULONG CopyOnWriteCount;
    ULONG TransitionCount;
    ULONG CacheTransitionCount;
    ULONG DemandZeroCount;
    ULONG PageReadCount;
    ULONG PageReadIoCount;
    ULONG CacheReadCount;
    ULONG CacheIoCount;
    ULONG DirtyPagesWriteCount;
    ULONG DirtyWriteIoCount;
    ULONG MappedPagesWriteCount;
    ULONG MappedWriteIoCount;
    ULONG PagedPoolPages;
    ULONG NonPagedPoolPages;
    ULONG PagedPoolAllocs;
    ULONG PagedPoolFrees;
    ULONG NonPagedPoolAllocs;
    ULONG NonPagedPoolFrees;
    ULONG FreeSystemPtes;
    ULONG ResidentSystemCodePage;
    ULONG TotalSystemDriverPages;
    ULONG TotalSystemCodePages;
    ULONG NonPagedPoolLookasideHits;
    ULONG PagedPoolLookasideHits;
    ULONG AvailablePagedPoolPages;
    ULONG ResidentSystemCachePage;
    ULONG ResidentPagedPoolPage;
    ULONG ResidentSystemDriverPage;
    ULONG CcFastReadNoWait;
    ULONG CcFastReadWait;
    ULONG CcFastReadResourceMiss;
    ULONG CcFastReadNotPossible;
    ULONG CcFastMdlReadNoWait;
    ULONG CcFastMdlReadWait;
    ULONG CcFastMdlReadResourceMiss;
    ULONG CcFastMdlReadNotPossible;
    ULONG CcMapDataNoWait;
    ULONG CcMapDataWait;
    ULONG CcMapDataNoWaitMiss;
    ULONG CcMapDataWaitMiss;
    ULONG CcPinMappedDataCount;
    ULONG CcPinReadNoWait;
    ULONG CcPinReadWait;
    ULONG CcPinReadNoWaitMiss;
    ULONG CcPinReadWaitMiss;
    ULONG CcCopyReadNoWait;
    ULONG CcCopyReadWait;
    ULONG CcCopyReadNoWaitMiss;
    ULONG CcCopyReadWaitMiss;
    ULONG CcMdlReadNoWait;
    ULONG CcMdlReadWait;
    ULONG CcMdlReadNoWaitMiss;
    ULONG CcMdlReadWaitMiss;
    ULONG CcReadAheadIos;
    ULONG CcLazyWriteIos;
    ULONG CcLazyWritePages;
    ULONG CcDataFlushes;
    ULONG CcDataPages;
    ULONG ContextSwitches;
    ULONG FirstLevelTbFills;
    ULONG SecondLevelTbFills;
    ULONG SystemCalls;
} SYSTEM_PERFORMANCE_INFORMATION, *PSYSTEM_PERFORMANCE_INFORMATION;

typedef struct _SYSTEM_PROCESS_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfThreads;
    LARGE_INTEGER SpareLi1;
    LARGE_INTEGER SpareLi2;
    LARGE_INTEGER SpareLi3;
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ImageName;
    KPRIORITY BasePriority;
    HANDLE UniqueProcessId;
    HANDLE InheritedFromUniqueProcessId;
    ULONG HandleCount;
    ULONG SessionId;
    ULONG_PTR PageDirectoryBase;
    SIZE_T PeakVirtualSize;
    SIZE_T VirtualSize;
    ULONG PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
    SIZE_T PrivatePageCount;
    LARGE_INTEGER ReadOperationCount;
    LARGE_INTEGER WriteOperationCount;
    LARGE_INTEGER OtherOperationCount;
    LARGE_INTEGER ReadTransferCount;
    LARGE_INTEGER WriteTransferCount;
    LARGE_INTEGER OtherTransferCount;
} SYSTEM_PROCESS_INFORMATION, *PSYSTEM_PROCESS_INFORMATION;

typedef struct _SYSTEM_SESSION_PROCESS_INFORMATION {
    ULONG SessionId;
    ULONG SizeOfBuf;
    PVOID Buffer;
} SYSTEM_SESSION_PROCESS_INFORMATION, *PSYSTEM_SESSION_PROCESS_INFORMATION;

typedef struct _SYSTEM_THREAD_INFORMATION {
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;
    KPRIORITY Priority;
    LONG BasePriority;
    ULONG ContextSwitches;
    ULONG ThreadState;
    ULONG WaitReason;
} SYSTEM_THREAD_INFORMATION, *PSYSTEM_THREAD_INFORMATION;

typedef struct _SYSTEM_EXTENDED_THREAD_INFORMATION {
    SYSTEM_THREAD_INFORMATION ThreadInfo;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID Win32StartAddress;
    ULONG_PTR Reserved1;
    ULONG_PTR Reserved2;
    ULONG_PTR Reserved3;
    ULONG_PTR Reserved4;
} SYSTEM_EXTENDED_THREAD_INFORMATION, *PSYSTEM_EXTENDED_THREAD_INFORMATION;

typedef struct _SYSTEM_MEMORY_INFO {
    PUCHAR StringOffset;
    USHORT ValidCount;
    USHORT TransitionCount;
    USHORT ModifiedCount;
    USHORT PageTableCount;
} SYSTEM_MEMORY_INFO, *PSYSTEM_MEMORY_INFO;

typedef struct _SYSTEM_MEMORY_INFORMATION {
    ULONG InfoSize;
    ULONG_PTR StringStart;
    SYSTEM_MEMORY_INFO Memory[1];
} SYSTEM_MEMORY_INFORMATION, *PSYSTEM_MEMORY_INFORMATION;

typedef struct _SYSTEM_CALL_COUNT_INFORMATION {
    ULONG Length;
    ULONG NumberOfTables;
} SYSTEM_CALL_COUNT_INFORMATION, *PSYSTEM_CALL_COUNT_INFORMATION;

typedef struct _SYSTEM_DEVICE_INFORMATION {
    ULONG NumberOfDisks;
    ULONG NumberOfFloppies;
    ULONG NumberOfCdRoms;
    ULONG NumberOfTapes;
    ULONG NumberOfSerialPorts;
    ULONG NumberOfParallelPorts;
} SYSTEM_DEVICE_INFORMATION, *PSYSTEM_DEVICE_INFORMATION;

typedef struct _SYSTEM_EXCEPTION_INFORMATION {
    ULONG AlignmentFixupCount;
    ULONG ExceptionDispatchCount;
    ULONG FloatingEmulationCount;
    ULONG ByteWordEmulationCount;
} SYSTEM_EXCEPTION_INFORMATION, *PSYSTEM_EXCEPTION_INFORMATION;

typedef struct _SYSTEM_KERNEL_DEBUGGER_INFORMATION {
    BOOLEAN KernelDebuggerEnabled;
    BOOLEAN KernelDebuggerNotPresent;
} SYSTEM_KERNEL_DEBUGGER_INFORMATION, *PSYSTEM_KERNEL_DEBUGGER_INFORMATION;

typedef struct _SYSTEM_REGISTRY_QUOTA_INFORMATION {
    ULONG  RegistryQuotaAllowed;
    ULONG  RegistryQuotaUsed;
    SIZE_T PagedPoolSize;
} SYSTEM_REGISTRY_QUOTA_INFORMATION, *PSYSTEM_REGISTRY_QUOTA_INFORMATION;

typedef struct _SYSTEM_GDI_DRIVER_INFORMATION {
    UNICODE_STRING DriverName;
    PVOID ImageAddress;
    PVOID SectionPointer;
    PVOID EntryPoint;
    PIMAGE_EXPORT_DIRECTORY ExportSectionPointer;
    ULONG ImageLength;
} SYSTEM_GDI_DRIVER_INFORMATION, *PSYSTEM_GDI_DRIVER_INFORMATION;

#if DEVL

typedef struct _SYSTEM_FLAGS_INFORMATION {
    ULONG Flags;
} SYSTEM_FLAGS_INFORMATION, *PSYSTEM_FLAGS_INFORMATION;

typedef struct _SYSTEM_CALL_TIME_INFORMATION {
    ULONG Length;
    ULONG TotalCalls;
    LARGE_INTEGER TimeOfCalls[1];
} SYSTEM_CALL_TIME_INFORMATION, *PSYSTEM_CALL_TIME_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO {
    USHORT UniqueProcessId;
    USHORT CreatorBackTraceIndex;
    UCHAR ObjectTypeIndex;
    UCHAR HandleAttributes;
    USHORT HandleValue;
    PVOID Object;
    ULONG GrantedAccess;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO;

typedef struct _SYSTEM_HANDLE_INFORMATION {
    ULONG NumberOfHandles;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO Handles[ 1 ];
} SYSTEM_HANDLE_INFORMATION, *PSYSTEM_HANDLE_INFORMATION;

typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX {
    PVOID Object;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR HandleValue;
    ULONG GrantedAccess;
    USHORT CreatorBackTraceIndex;
    USHORT ObjectTypeIndex;
    ULONG  HandleAttributes;
    ULONG  Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX {
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[ 1 ];
} SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

typedef struct _SYSTEM_OBJECTTYPE_INFORMATION {
    ULONG NextEntryOffset;
    ULONG NumberOfObjects;
    ULONG NumberOfHandles;
    ULONG TypeIndex;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    ULONG PoolType;
    BOOLEAN SecurityRequired;
    BOOLEAN WaitableObject;
    UNICODE_STRING TypeName;
} SYSTEM_OBJECTTYPE_INFORMATION, *PSYSTEM_OBJECTTYPE_INFORMATION;

typedef struct _SYSTEM_OBJECT_INFORMATION {
    ULONG NextEntryOffset;
    PVOID Object;
    HANDLE CreatorUniqueProcess;
    USHORT CreatorBackTraceIndex;
    USHORT Flags;
    LONG PointerCount;
    LONG HandleCount;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    HANDLE ExclusiveProcessId;
    PVOID SecurityDescriptor;
    OBJECT_NAME_INFORMATION NameInfo;
} SYSTEM_OBJECT_INFORMATION, *PSYSTEM_OBJECT_INFORMATION;

typedef struct _SYSTEM_PAGEFILE_INFORMATION {
    ULONG NextEntryOffset;
    ULONG TotalSize;
    ULONG TotalInUse;
    ULONG PeakUsage;
    UNICODE_STRING PageFileName;
} SYSTEM_PAGEFILE_INFORMATION, *PSYSTEM_PAGEFILE_INFORMATION;

typedef struct _SYSTEM_VERIFIER_INFORMATION {
    ULONG NextEntryOffset;
    ULONG Level;
    UNICODE_STRING DriverName;

    ULONG RaiseIrqls;
    ULONG AcquireSpinLocks;
    ULONG SynchronizeExecutions;
    ULONG AllocationsAttempted;

    ULONG AllocationsSucceeded;
    ULONG AllocationsSucceededSpecialPool;
    ULONG AllocationsWithNoTag;
    ULONG TrimRequests;

    ULONG Trims;
    ULONG AllocationsFailed;
    ULONG AllocationsFailedDeliberately;
    ULONG Loads;

    ULONG Unloads;
    ULONG UnTrackedPool;
    ULONG CurrentPagedPoolAllocations;
    ULONG CurrentNonPagedPoolAllocations;

    ULONG PeakPagedPoolAllocations;
    ULONG PeakNonPagedPoolAllocations;

    SIZE_T PagedPoolUsageInBytes;
    SIZE_T NonPagedPoolUsageInBytes;
    SIZE_T PeakPagedPoolUsageInBytes;
    SIZE_T PeakNonPagedPoolUsageInBytes;

} SYSTEM_VERIFIER_INFORMATION, *PSYSTEM_VERIFIER_INFORMATION;

#define MM_WORKING_SET_MAX_HARD_ENABLE      0x1
#define MM_WORKING_SET_MAX_HARD_DISABLE     0x2
#define MM_WORKING_SET_MIN_HARD_ENABLE      0x4
#define MM_WORKING_SET_MIN_HARD_DISABLE     0x8

typedef struct _SYSTEM_FILECACHE_INFORMATION {
    SIZE_T CurrentSize;
    SIZE_T PeakSize;
    ULONG PageFaultCount;
    SIZE_T MinimumWorkingSet;
    SIZE_T MaximumWorkingSet;
    SIZE_T CurrentSizeIncludingTransitionInPages;
    SIZE_T PeakSizeIncludingTransitionInPages;
    ULONG TransitionRePurposeCount;
    ULONG Flags;
} SYSTEM_FILECACHE_INFORMATION, *PSYSTEM_FILECACHE_INFORMATION;

#define FLG_HOTPATCH_KERNEL             0x80000000
#define FLG_HOTPATCH_RELOAD_NTDLL       0x40000000
#define FLG_HOTPATCH_NAME_INFO          0x20000000
#define FLG_HOTPATCH_RENAME_INFO        0x10000000
#define FLG_HOTPATCH_MAP_ATOMIC_SWAP    0x08000000
#define FLG_HOTPATCH_WOW64              0x04000000

#define FLG_HOTPATCH_ACTIVE             0x00000001
#define FLG_HOTPATCH_STATUS_FLAGS       FLG_HOTPATCH_ACTIVE

#define FLG_HOTPATCH_VERIFICATION_ERROR 0x00800000

typedef struct _HOTPATCH_HOOK_DESCRIPTOR{
    ULONG_PTR TargetAddress;
    PVOID MappedAddress;
    ULONG CodeOffset;
    ULONG CodeSize;
    ULONG OrigCodeOffset;
    ULONG ValidationOffset;
    ULONG ValidationSize;
} HOTPATCH_HOOK_DESCRIPTOR, *PHOTPATCH_HOOK_DESCRIPTOR;

typedef struct _SYSTEM_HOTPATCH_CODE_INFORMATION {

    ULONG Flags;
    ULONG InfoSize;
    
    union {
    
        struct {
        
            ULONG DescriptorsCount;
            
            HOTPATCH_HOOK_DESCRIPTOR CodeDescriptors[1]; // variable size structure
            
        } CodeInfo;
        
        struct {
        
            USHORT NameOffset;
            USHORT NameLength;
            
        } KernelInfo;
        
        struct {
        
            USHORT NameOffset;
            USHORT NameLength;
            
            USHORT TargetNameOffset;
            USHORT TargetNameLength;
            
        } UserModeInfo;
        
        struct {
        
            HANDLE FileHandle1;
            PIO_STATUS_BLOCK IoStatusBlock1;
            PFILE_RENAME_INFORMATION RenameInformation1;
            ULONG RenameInformationLength1;
            HANDLE FileHandle2;
            PIO_STATUS_BLOCK IoStatusBlock2;
            PFILE_RENAME_INFORMATION RenameInformation2;
            ULONG RenameInformationLength2;
            
        } RenameInfo;
        
        struct {
        
            HANDLE ParentDirectory;
            HANDLE ObjectHandle1;
            HANDLE ObjectHandle2;
            
        } AtomicSwap;
    };

    //
    //  NOTE Do not add anything after CodeDescriptors array as
    //  it is assumed to have a variable size
    //
    
} SYSTEM_HOTPATCH_CODE_INFORMATION, *PSYSTEM_HOTPATCH_CODE_INFORMATION;

//
// Watchdog Timer
//

typedef enum _WATCHDOG_HANDLER_ACTION {
    WdActionSetTimeoutValue,
    WdActionQueryTimeoutValue,
    WdActionResetTimer,
    WdActionStopTimer,
    WdActionStartTimer,
    WdActionSetTriggerAction,
    WdActionQueryTriggerAction,
    WdActionQueryState
} WATCHDOG_HANDLER_ACTION;

typedef enum _WATCHDOG_INFORMATION_CLASS {
    WdInfoTimeoutValue,
    WdInfoResetTimer,
    WdInfoStopTimer,
    WdInfoStartTimer,
    WdInfoTriggerAction,
    WdInfoState
} WATCHDOG_INFORMATION_CLASS;

typedef
NTSTATUS
(*PWD_HANDLER)(
    IN WATCHDOG_HANDLER_ACTION Action,
    IN PVOID Context,
    IN OUT PULONG DataValue,
    IN BOOLEAN NoLocks
    );

typedef struct _SYSTEM_WATCHDOG_HANDLER_INFORMATION {
    PWD_HANDLER WdHandler;
    PVOID       Context;
} SYSTEM_WATCHDOG_HANDLER_INFORMATION, *PSYSTEM_WATCHDOG_HANDLER_INFORMATION;

#define WDSTATE_FIRED               0x00000001
#define WDSTATE_HARDWARE_ENABLED    0x00000002
#define WDSTATE_STARTED             0x00000004
#define WDSTATE_HARDWARE_PRESENT    0x00000008

typedef struct _SYSTEM_WATCHDOG_TIMER_INFORMATION {
    WATCHDOG_INFORMATION_CLASS  WdInfoClass;
    ULONG                       DataValue;
} SYSTEM_WATCHDOG_TIMER_INFORMATION, *PSYSTEM_WATCHDOG_TIMER_INFORMATION;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)       // unnamed struct/union

typedef struct _SYSTEM_POOL_ENTRY {
    BOOLEAN Allocated;
    BOOLEAN Spare0;
    USHORT AllocatorBackTraceIndex;
    ULONG Size;
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
        PVOID ProcessChargedQuota;
    };
} SYSTEM_POOL_ENTRY, *PSYSTEM_POOL_ENTRY;

typedef struct _SYSTEM_POOL_INFORMATION {
    SIZE_T TotalSize;
    PVOID FirstEntry;
    USHORT EntryOverhead;
    BOOLEAN PoolTagPresent;
    BOOLEAN Spare0;
    ULONG NumberOfEntries;
    SYSTEM_POOL_ENTRY Entries[1];
} SYSTEM_POOL_INFORMATION, *PSYSTEM_POOL_INFORMATION;

typedef struct _SYSTEM_POOLTAG {
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
    ULONG PagedAllocs;
    ULONG PagedFrees;
    SIZE_T PagedUsed;
    ULONG NonPagedAllocs;
    ULONG NonPagedFrees;
    SIZE_T NonPagedUsed;
} SYSTEM_POOLTAG, *PSYSTEM_POOLTAG;

typedef struct _SYSTEM_BIGPOOL_ENTRY {
    union {
        PVOID VirtualAddress;
        ULONG_PTR NonPaged : 1;     // Set to 1 if entry is nonpaged.
    };
    SIZE_T SizeInBytes;
    union {
        UCHAR Tag[4];
        ULONG TagUlong;
    };
} SYSTEM_BIGPOOL_ENTRY, *PSYSTEM_BIGPOOL_ENTRY;

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4201 )
#endif

typedef struct _SYSTEM_POOLTAG_INFORMATION {
    ULONG Count;
    SYSTEM_POOLTAG TagInfo[1];
} SYSTEM_POOLTAG_INFORMATION, *PSYSTEM_POOLTAG_INFORMATION;

typedef struct _SYSTEM_SESSION_POOLTAG_INFORMATION {
    SIZE_T NextEntryOffset;
    ULONG SessionId;
    ULONG Count;
    SYSTEM_POOLTAG TagInfo[1];
} SYSTEM_SESSION_POOLTAG_INFORMATION, *PSYSTEM_SESSION_POOLTAG_INFORMATION;

typedef struct _SYSTEM_BIGPOOL_INFORMATION {
    ULONG Count;
    SYSTEM_BIGPOOL_ENTRY AllocatedInfo[1];
} SYSTEM_BIGPOOL_INFORMATION, *PSYSTEM_BIGPOOL_INFORMATION;

typedef struct _SYSTEM_SESSION_MAPPED_VIEW_INFORMATION {
    SIZE_T NextEntryOffset;
    ULONG SessionId;
    ULONG ViewFailures;
    SIZE_T NumberOfBytesAvailable;
    SIZE_T NumberOfBytesAvailableContiguous;
} SYSTEM_SESSION_MAPPED_VIEW_INFORMATION, *PSYSTEM_SESSION_MAPPED_VIEW_INFORMATION;

typedef struct _SYSTEM_CONTEXT_SWITCH_INFORMATION {
    ULONG ContextSwitches;
    ULONG FindAny;
    ULONG FindLast;
    ULONG FindIdeal;
    ULONG IdleAny;
    ULONG IdleCurrent;
    ULONG IdleLast;
    ULONG IdleIdeal;
    ULONG PreemptAny;
    ULONG PreemptCurrent;
    ULONG PreemptLast;
    ULONG SwitchToIdle;
} SYSTEM_CONTEXT_SWITCH_INFORMATION, *PSYSTEM_CONTEXT_SWITCH_INFORMATION;

typedef struct _SYSTEM_INTERRUPT_INFORMATION {
    ULONG ContextSwitches;
    ULONG DpcCount;
    ULONG DpcRate;
    ULONG TimeIncrement;
    ULONG DpcBypassCount;
    ULONG ApcBypassCount;
} SYSTEM_INTERRUPT_INFORMATION, *PSYSTEM_INTERRUPT_INFORMATION;

typedef struct _SYSTEM_DPC_BEHAVIOR_INFORMATION {
    ULONG Spare;
    ULONG DpcQueueDepth;
    ULONG MinimumDpcRate;
    ULONG AdjustDpcThreshold;
    ULONG IdealDpcRate;
} SYSTEM_DPC_BEHAVIOR_INFORMATION, *PSYSTEM_DPC_BEHAVIOR_INFORMATION;

#endif // DEVL

typedef struct _SYSTEM_LOOKASIDE_INFORMATION {
    USHORT CurrentDepth;
    USHORT MaximumDepth;
    ULONG TotalAllocates;
    ULONG AllocateMisses;
    ULONG TotalFrees;
    ULONG FreeMisses;
    ULONG Type;
    ULONG Tag;
    ULONG Size;
} SYSTEM_LOOKASIDE_INFORMATION, *PSYSTEM_LOOKASIDE_INFORMATION;

typedef struct _SYSTEM_LEGACY_DRIVER_INFORMATION {
    ULONG VetoType;
    UNICODE_STRING VetoList;
} SYSTEM_LEGACY_DRIVER_INFORMATION, *PSYSTEM_LEGACY_DRIVER_INFORMATION;

// begin_winnt

#define PROCESSOR_INTEL_386     386
#define PROCESSOR_INTEL_486     486
#define PROCESSOR_INTEL_PENTIUM 586
#define PROCESSOR_INTEL_IA64    2200
#define PROCESSOR_AMD_X8664     8664
#define PROCESSOR_MIPS_R4000    4000    // incl R4101 & R3910 for Windows CE
#define PROCESSOR_ALPHA_21064   21064
#define PROCESSOR_PPC_601       601
#define PROCESSOR_PPC_603       603
#define PROCESSOR_PPC_604       604
#define PROCESSOR_PPC_620       620
#define PROCESSOR_HITACHI_SH3   10003   // Windows CE
#define PROCESSOR_HITACHI_SH3E  10004   // Windows CE
#define PROCESSOR_HITACHI_SH4   10005   // Windows CE
#define PROCESSOR_MOTOROLA_821  821     // Windows CE
#define PROCESSOR_SHx_SH3       103     // Windows CE
#define PROCESSOR_SHx_SH4       104     // Windows CE
#define PROCESSOR_STRONGARM     2577    // Windows CE - 0xA11
#define PROCESSOR_ARM720        1824    // Windows CE - 0x720
#define PROCESSOR_ARM820        2080    // Windows CE - 0x820
#define PROCESSOR_ARM920        2336    // Windows CE - 0x920
#define PROCESSOR_ARM_7TDMI     70001   // Windows CE
#define PROCESSOR_OPTIL         0x494f  // MSIL

#define PROCESSOR_ARCHITECTURE_INTEL            0
#define PROCESSOR_ARCHITECTURE_MIPS             1
#define PROCESSOR_ARCHITECTURE_ALPHA            2
#define PROCESSOR_ARCHITECTURE_PPC              3
#define PROCESSOR_ARCHITECTURE_SHX              4
#define PROCESSOR_ARCHITECTURE_ARM              5
#define PROCESSOR_ARCHITECTURE_IA64             6
#define PROCESSOR_ARCHITECTURE_ALPHA64          7
#define PROCESSOR_ARCHITECTURE_MSIL             8
#define PROCESSOR_ARCHITECTURE_AMD64            9
#define PROCESSOR_ARCHITECTURE_IA32_ON_WIN64    10

#define PROCESSOR_ARCHITECTURE_UNKNOWN 0xFFFF

// end_winnt

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySystemInformation (
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
    __in ULONG SystemInformationLength,
    __out_opt PULONG ReturnLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetSystemInformation (
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __in_bcount_opt(SystemInformationLength) PVOID SystemInformation,
    __in ULONG SystemInformationLength
    );

//
// SysDbg APIs are available to user-mode processes via
// NtSystemDebugControl.
//

typedef enum _SYSDBG_COMMAND {
    SysDbgQueryModuleInformation,
    SysDbgQueryTraceInformation,
    SysDbgSetTracepoint,
    SysDbgSetSpecialCall,
    SysDbgClearSpecialCalls,
    SysDbgQuerySpecialCalls,
    SysDbgBreakPoint,
    SysDbgQueryVersion,
    SysDbgReadVirtual,
    SysDbgWriteVirtual,
    SysDbgReadPhysical,
    SysDbgWritePhysical,
    SysDbgReadControlSpace,
    SysDbgWriteControlSpace,
    SysDbgReadIoSpace,
    SysDbgWriteIoSpace,
    SysDbgReadMsr,
    SysDbgWriteMsr,
    SysDbgReadBusData,
    SysDbgWriteBusData,
    SysDbgCheckLowMemory,
    SysDbgEnableKernelDebugger,
    SysDbgDisableKernelDebugger,
    SysDbgGetAutoKdEnable,
    SysDbgSetAutoKdEnable,
    SysDbgGetPrintBufferSize,
    SysDbgSetPrintBufferSize,
    SysDbgGetKdUmExceptionEnable,
    SysDbgSetKdUmExceptionEnable,
    SysDbgGetTriageDump,
    SysDbgGetKdBlockEnable,
    SysDbgSetKdBlockEnable,
} SYSDBG_COMMAND, *PSYSDBG_COMMAND;

typedef struct _SYSDBG_VIRTUAL {
    PVOID Address;
    PVOID Buffer;
    ULONG Request;
} SYSDBG_VIRTUAL, *PSYSDBG_VIRTUAL;

typedef struct _SYSDBG_PHYSICAL {
    PHYSICAL_ADDRESS Address;
    PVOID Buffer;
    ULONG Request;
} SYSDBG_PHYSICAL, *PSYSDBG_PHYSICAL;

typedef struct _SYSDBG_CONTROL_SPACE {
    ULONG64 Address;
    PVOID Buffer;
    ULONG Request;
    ULONG Processor;
} SYSDBG_CONTROL_SPACE, *PSYSDBG_CONTROL_SPACE;

typedef struct _SYSDBG_IO_SPACE {
    ULONG64 Address;
    PVOID Buffer;
    ULONG Request;
    INTERFACE_TYPE InterfaceType;
    ULONG BusNumber;
    ULONG AddressSpace;
} SYSDBG_IO_SPACE, *PSYSDBG_IO_SPACE;

typedef struct _SYSDBG_MSR {
    ULONG Msr;
    ULONG64 Data;
} SYSDBG_MSR, *PSYSDBG_MSR;

typedef struct _SYSDBG_BUS_DATA {
    ULONG Address;
    PVOID Buffer;
    ULONG Request;
    BUS_DATA_TYPE BusDataType;
    ULONG BusNumber;
    ULONG SlotNumber;
} SYSDBG_BUS_DATA, *PSYSDBG_BUS_DATA;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSystemDebugControl (
    __in SYSDBG_COMMAND Command,
    __inout_bcount_opt(InputBufferLength) PVOID InputBuffer,
    __in ULONG InputBufferLength,
    __out_bcount_opt(OutputBufferLength) PVOID OutputBuffer,
    __in ULONG OutputBufferLength,
    __out_opt PULONG ReturnLength
    );

typedef enum _HARDERROR_RESPONSE_OPTION {
        OptionAbortRetryIgnore,
        OptionOk,
        OptionOkCancel,
        OptionRetryCancel,
        OptionYesNo,
        OptionYesNoCancel,
        OptionShutdownSystem,
        OptionOkNoWait,
        OptionCancelTryContinue
} HARDERROR_RESPONSE_OPTION;

typedef enum _HARDERROR_RESPONSE {
        ResponseReturnToCaller,
        ResponseNotHandled,
        ResponseAbort,
        ResponseCancel,
        ResponseIgnore,
        ResponseNo,
        ResponseOk,
        ResponseRetry,
        ResponseYes,
        ResponseTryAgain,
        ResponseContinue
} HARDERROR_RESPONSE;

#define HARDERROR_PARAMETERS_FLAGSPOS   4
#define HARDERROR_FLAGS_DEFDESKTOPONLY  0x00020000

#define MAXIMUM_HARDERROR_PARAMETERS    5

#define HARDERROR_OVERRIDE_ERRORMODE    0x10000000

typedef struct _HARDERROR_MSG {
    PORT_MESSAGE h;
    NTSTATUS Status;
    LARGE_INTEGER ErrorTime;
    ULONG ValidResponseOptions;
    ULONG Response;
    ULONG NumberOfParameters;
    ULONG UnicodeStringParameterMask;
    ULONG_PTR Parameters[MAXIMUM_HARDERROR_PARAMETERS];
} HARDERROR_MSG, *PHARDERROR_MSG;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRaiseHardError (
    __in NTSTATUS ErrorStatus,
    __in ULONG NumberOfParameters,
    __in ULONG UnicodeStringParameterMask,
    __in_ecount(NumberOfParameters) PULONG_PTR Parameters,
    __in ULONG ValidResponseOptions,
    __out PULONG Response
    );

// begin_wdm begin_ntddk begin_nthal begin_ntifs

//
// Defined processor features
//

#define PF_FLOATING_POINT_PRECISION_ERRATA  0   // winnt
#define PF_FLOATING_POINT_EMULATED          1   // winnt
#define PF_COMPARE_EXCHANGE_DOUBLE          2   // winnt
#define PF_MMX_INSTRUCTIONS_AVAILABLE       3   // winnt
#define PF_PPC_MOVEMEM_64BIT_OK             4   // winnt
#define PF_ALPHA_BYTE_INSTRUCTIONS          5   // winnt
#define PF_XMMI_INSTRUCTIONS_AVAILABLE      6   // winnt
#define PF_3DNOW_INSTRUCTIONS_AVAILABLE     7   // winnt
#define PF_RDTSC_INSTRUCTION_AVAILABLE      8   // winnt
#define PF_PAE_ENABLED                      9   // winnt
#define PF_XMMI64_INSTRUCTIONS_AVAILABLE   10   // winnt
#define PF_SSE_DAZ_MODE_AVAILABLE          11   // winnt
#define PF_NX_ENABLED                      12   // winnt

typedef enum _ALTERNATIVE_ARCHITECTURE_TYPE {
    StandardDesign,                 // None == 0 == standard design
    NEC98x86,                       // NEC PC98xx series on X86
    EndAlternatives                 // past end of known alternatives
} ALTERNATIVE_ARCHITECTURE_TYPE;

// correctly define these run-time definitions for non X86 machines

#ifndef _X86_

#ifndef IsNEC_98
#define IsNEC_98 (FALSE)
#endif

#ifndef IsNotNEC_98
#define IsNotNEC_98 (TRUE)
#endif

#ifndef SetNEC_98
#define SetNEC_98
#endif

#ifndef SetNotNEC_98
#define SetNotNEC_98
#endif

#endif

#define PROCESSOR_FEATURE_MAX 64

// end_wdm

#if defined(REMOTE_BOOT)
//
// Defined system flags.
//

/* the following two lines should be tagged with "winnt" when REMOTE_BOOT is on. */
#define SYSTEM_FLAG_REMOTE_BOOT_CLIENT 0x00000001
#define SYSTEM_FLAG_DISKLESS_CLIENT    0x00000002
#endif // defined(REMOTE_BOOT)

//
// Define data shared between kernel and user mode.
//
// N.B. User mode has read only access to this data
//

#define MAX_WOW64_SHARED_ENTRIES 16

//
// WARNING: This structure must have exactly the same layout for 32- and
//    64-bit systems. The layout of this structure cannot change and new
//    fields can only be added at the end of the structure (unless a gap
//    can be exploited). Deprecated fields cannot be deleted. Platform
//    specific fields are included on all systems.
//
//    Layout exactness is required for Wow64 support of 32-bit applications
//    on Win64 systems.
//
//    The layout itself cannot change since this structure has been exported
//    in ntddk, ntifs.h, and nthal.h for some time.
//
// Define NX support policy values.
//

#define NX_SUPPORT_POLICY_ALWAYSOFF 0
#define NX_SUPPORT_POLICY_ALWAYSON 1
#define NX_SUPPORT_POLICY_OPTIN 2
#define NX_SUPPORT_POLICY_OPTOUT 3

typedef struct _KUSER_SHARED_DATA {

    //
    // Current low 32-bit of tick count and tick count multiplier.
    //
    // N.B. The tick count is updated each time the clock ticks.
    //

    ULONG TickCountLowDeprecated;
    ULONG TickCountMultiplier;

    //
    // Current 64-bit interrupt time in 100ns units.
    //

    volatile KSYSTEM_TIME InterruptTime;

    //
    // Current 64-bit system time in 100ns units.
    //

    volatile KSYSTEM_TIME SystemTime;

    //
    // Current 64-bit time zone bias.
    //

    volatile KSYSTEM_TIME TimeZoneBias;

    //
    // Support image magic number range for the host system.
    //
    // N.B. This is an inclusive range.
    //

    USHORT ImageNumberLow;
    USHORT ImageNumberHigh;

    //
    // Copy of system root in unicode.
    //

    WCHAR NtSystemRoot[260];

    //
    // Maximum stack trace depth if tracing enabled.
    //

    ULONG MaxStackTraceDepth;

    //
    // Crypto exponent value.
    //

    ULONG CryptoExponent;

    //
    // Time zone ID.
    //

    ULONG TimeZoneId;
    ULONG LargePageMinimum;
    ULONG Reserved2[7];

    //
    // Product type.
    //

    NT_PRODUCT_TYPE NtProductType;
    BOOLEAN ProductTypeIsValid;

    //
    // The NT Version.
    //
    // N. B. Note that each process sees a version from its PEB, but if the
    //       process is running with an altered view of the system version,
    //       the following two fields are used to correctly identify the
    //       version
    //

    ULONG NtMajorVersion;
    ULONG NtMinorVersion;

    //
    // Processor features.
    //

    BOOLEAN ProcessorFeatures[PROCESSOR_FEATURE_MAX];

    //
    // Reserved fields - do not use.
    //

    ULONG Reserved1;
    ULONG Reserved3;

    //
    // Time slippage while in debugger.
    //

    volatile ULONG TimeSlip;

    //
    // Alternative system architecture, e.g., NEC PC98xx on x86.
    //

    ALTERNATIVE_ARCHITECTURE_TYPE AlternativeArchitecture;

    //
    // If the system is an evaluation unit, the following field contains the
    // date and time that the evaluation unit expires. A value of 0 indicates
    // that there is no expiration. A non-zero value is the UTC absolute time
    // that the system expires.
    //

    LARGE_INTEGER SystemExpirationDate;

    //
    // Suite support.
    //

    ULONG SuiteMask;

    //
    // TRUE if a kernel debugger is connected/enabled.
    //

    BOOLEAN KdDebuggerEnabled;

    //
    // NX support policy.
    //

    UCHAR NXSupportPolicy;

    //
    // Current console session Id. Always zero on non-TS systems.
    //

    volatile ULONG ActiveConsoleId;

    //
    // Force-dismounts cause handles to become invalid. Rather than always
    // probe handles, a serial number of dismounts is maintained that clients
    // can use to see if they need to probe handles.
    //

    volatile ULONG DismountCount;

    //
    // This field indicates the status of the 64-bit COM+ package on the
    // system. It indicates whether the Intermediate Language (IL) COM+
    // images need to use the 64-bit COM+ runtime or the 32-bit COM+ runtime.
    //

    ULONG ComPlusPackage;

    //
    // Time in tick count for system-wide last user input across all terminal
    // sessions. For MP performance, it is not updated all the time (e.g. once
    // a minute per session). It is used for idle detection.
    //

    ULONG LastSystemRITEventTickCount;

    //
    // Number of physical pages in the system. This can dynamically change as
    // physical memory can be added or removed from a running system.
    //

    ULONG NumberOfPhysicalPages;

    //
    // True if the system was booted in safe boot mode.
    //

    BOOLEAN SafeBootMode;

    //
    // The following field is used for heap and critcial sectionc tracing. The
    // last bit is set for critical section collision tracing and second last
    // bit is for heap tracing.  Also the first 16 bits are used as counter.
    //

    ULONG TraceLogging;

    //
    // Depending on the processor, the code for fast system call will differ,
    // Stub code is provided pointers below to access the appropriate code.
    //
    // N.B. The following two fields are only used on 32-bit systems.
    //

    ULONGLONG TestRetInstruction;
    ULONG SystemCall;
    ULONG SystemCallReturn;
    ULONGLONG SystemCallPad[3];

    //
    // The 64-bit tick count.
    //

    union {
        volatile KSYSTEM_TIME TickCount;
        volatile ULONG64 TickCountQuad;
    };

    //
    // Cookie for encoding pointers system wide.
    //

    ULONG Cookie;
    
    //
    // Shared information for Wow64 processes.
    //
    
    ULONG Wow64SharedInformation[MAX_WOW64_SHARED_ENTRIES];

} KUSER_SHARED_DATA, *PKUSER_SHARED_DATA;

// end_ntddk end_nthal end_ntifs

#if !defined(SORTPP_PASS) && !defined(MIDL_PASS) && !defined(RC_INVOKED) && !defined(_X86AMD64_)

C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TickCountMultiplier) == 0x4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, InterruptTime) == 0x8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemTime) == 0x14);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeZoneBias) == 0x20);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ImageNumberLow) == 0x2C);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ImageNumberHigh) == 0x2E);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NtSystemRoot) == 0x30);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, MaxStackTraceDepth) == 0x238);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, CryptoExponent) == 0x23C);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeZoneId) == 0x240);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, LargePageMinimum) == 0x244);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Reserved2) == 0x248);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NtProductType) == 0x264);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ProductTypeIsValid) == 0x268);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NtMajorVersion) == 0x26C);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NtMinorVersion) == 0x270);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ProcessorFeatures) == 0x274);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Reserved1) == 0x2B4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Reserved3) == 0x2B8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TimeSlip) == 0x2BC);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, AlternativeArchitecture) == 0x2C0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemExpirationDate) == 0x2C8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SuiteMask) == 0x2D0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, KdDebuggerEnabled) == 0x2D4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NXSupportPolicy) == 0x2D5);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ActiveConsoleId) == 0x2D8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, DismountCount) == 0x2DC);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, ComPlusPackage) == 0x2E0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, LastSystemRITEventTickCount) == 0x2E4);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, NumberOfPhysicalPages) == 0x2E8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SafeBootMode) == 0x2EC);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TraceLogging) == 0x2F0);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TestRetInstruction) == 0x2F8);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemCall) == 0x300);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemCallReturn) == 0x304);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, SystemCallPad) == 0x308);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TickCount) == 0x320);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, TickCountQuad) == 0x320);
C_ASSERT(FIELD_OFFSET(KUSER_SHARED_DATA, Wow64SharedInformation) == 0x334);

#endif

#define DOSDEVICE_DRIVE_UNKNOWN     0
#define DOSDEVICE_DRIVE_CALCULATE   1
#define DOSDEVICE_DRIVE_REMOVABLE   2
#define DOSDEVICE_DRIVE_FIXED       3
#define DOSDEVICE_DRIVE_REMOTE      4
#define DOSDEVICE_DRIVE_CDROM       5
#define DOSDEVICE_DRIVE_RAMDISK     6

#if defined(USER_SHARED_DATA) && !defined(MIDL_PASS) && !defined(SORTPP_PASS)

FORCEINLINE
ULONGLONG
NtGetTickCount64 (
    VOID
    )

{

    ULARGE_INTEGER TickCount;

#if defined(_WIN64)

    TickCount.QuadPart = USER_SHARED_DATA->TickCountQuad;

#else

    for (;;) {
        TickCount.HighPart = (ULONG) USER_SHARED_DATA->TickCount.High1Time;
        TickCount.LowPart = USER_SHARED_DATA->TickCount.LowPart;
        if (TickCount.HighPart == (ULONG) USER_SHARED_DATA->TickCount.High2Time) {
            break;
        }

#if defined(_X86_)

        _asm { rep nop }

#endif

    }

#endif

    return ((UInt32x32To64(TickCount.LowPart,
                           USER_SHARED_DATA->TickCountMultiplier) >> 24)
            + (UInt32x32To64(TickCount.HighPart,
               	             USER_SHARED_DATA->TickCountMultiplier) << 8));
}

FORCEINLINE
ULONG
NtGetTickCount (
    VOID
    )

{

#if defined(_WIN64)

    return (ULONG) ((USER_SHARED_DATA->TickCountQuad
                     * USER_SHARED_DATA->TickCountMultiplier)
                    >> 24);

#else

    ULARGE_INTEGER TickCount;

    for (;;) {
        TickCount.HighPart = (ULONG) USER_SHARED_DATA->TickCount.High1Time;
        TickCount.LowPart = USER_SHARED_DATA->TickCount.LowPart;
        if (TickCount.HighPart == (ULONG) USER_SHARED_DATA->TickCount.High2Time) {
            break;
        }

#if defined(_X86_)

        _asm { rep nop }

#endif

    }

    return (ULONG) ((UInt32x32To64(TickCount.LowPart,
	                           USER_SHARED_DATA->TickCountMultiplier) >> 24)
        	    + UInt32x32To64((TickCount.HighPart << 8) & 0xffffffff,
              		            USER_SHARED_DATA->TickCountMultiplier));

#endif

}

#endif // (defined(USER_SHARED_DATA) && !defined(MIDL_PASS) && !defined(SORTPP_PASS))

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryDefaultLocale (
    __in BOOLEAN UserProfile,
    __out PLCID DefaultLocaleId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetDefaultLocale (
    __in BOOLEAN UserProfile,
    __in LCID DefaultLocaleId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInstallUILanguage (
    __out LANGID *InstallUILanguageId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryDefaultUILanguage (
    __out LANGID *DefaultUILanguageId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetDefaultUILanguage (
    __in LANGID DefaultUILanguageId
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetDefaultHardErrorPort(
    __in HANDLE DefaultHardErrorPort
    );

typedef enum _SHUTDOWN_ACTION {
    ShutdownNoReboot,
    ShutdownReboot,
    ShutdownPowerOff
} SHUTDOWN_ACTION;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtShutdownSystem (
    __in SHUTDOWN_ACTION Action
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDisplayString (
    __in PUNICODE_STRING String
    );

//
// Global flags that can be set to control system behavior.
// Flag word is 32 bits.
//

#define FLG_STOP_ON_EXCEPTION           0x00000001      // user and kernel mode
#define FLG_SHOW_LDR_SNAPS              0x00000002      // user and kernel mode
#define FLG_DEBUG_INITIAL_COMMAND       0x00000004      // kernel mode only up until WINLOGON started
#define FLG_STOP_ON_HUNG_GUI            0x00000008      // kernel mode only while running

#define FLG_HEAP_ENABLE_TAIL_CHECK      0x00000010      // user mode only
#define FLG_HEAP_ENABLE_FREE_CHECK      0x00000020      // user mode only
#define FLG_HEAP_VALIDATE_PARAMETERS    0x00000040      // user mode only
#define FLG_HEAP_VALIDATE_ALL           0x00000080      // user mode only

#define FLG_APPLICATION_VERIFIER        0x00000100      // user mode only
#define FLG_POOL_ENABLE_TAGGING         0x00000400      // kernel mode only
#define FLG_HEAP_ENABLE_TAGGING         0x00000800      // user mode only

#define FLG_USER_STACK_TRACE_DB         0x00001000      // x86 user mode only
#define FLG_KERNEL_STACK_TRACE_DB       0x00002000      // x86 kernel mode only at boot time
#define FLG_MAINTAIN_OBJECT_TYPELIST    0x00004000      // kernel mode only at boot time
#define FLG_HEAP_ENABLE_TAG_BY_DLL      0x00008000      // user mode only

#define FLG_DISABLE_STACK_EXTENSION     0x00010000      // user mode only
#define FLG_ENABLE_CSRDEBUG             0x00020000      // kernel mode only at boot time
#define FLG_ENABLE_KDEBUG_SYMBOL_LOAD   0x00040000      // kernel mode only
#define FLG_DISABLE_PAGE_KERNEL_STACKS  0x00080000      // kernel mode only at boot time

#define FLG_ENABLE_SYSTEM_CRIT_BREAKS   0x00100000      // user mode only
#define FLG_HEAP_DISABLE_COALESCING     0x00200000      // user mode only
#define FLG_ENABLE_CLOSE_EXCEPTIONS     0x00400000      // kernel mode only
#define FLG_ENABLE_EXCEPTION_LOGGING    0x00800000      // kernel mode only

#define FLG_ENABLE_HANDLE_TYPE_TAGGING  0x01000000      // kernel mode only
#define FLG_HEAP_PAGE_ALLOCS            0x02000000      // user mode only
#define FLG_DEBUG_INITIAL_COMMAND_EX    0x04000000      // kernel mode only up until WINLOGON started
#define FLG_DISABLE_DBGPRINT            0x08000000      // kernel mode only

#define FLG_CRITSEC_EVENT_CREATION      0x10000000      // user mode only, Force early creation of resource events
#define FLG_LDR_TOP_DOWN                0x20000000      // user mode only, win64 only
#define FLG_ENABLE_HANDLE_EXCEPTIONS    0x40000000      // kernel mode only
#define FLG_DISABLE_PROTDLLS            0x80000000      // user mode only (smss/winlogon)

#define FLG_VALID_BITS                  0xFFFFFDFF

#define FLG_USERMODE_VALID_BITS        (FLG_STOP_ON_EXCEPTION           | \
                                        FLG_SHOW_LDR_SNAPS              | \
                                        FLG_HEAP_ENABLE_TAIL_CHECK      | \
                                        FLG_HEAP_ENABLE_FREE_CHECK      | \
                                        FLG_HEAP_VALIDATE_PARAMETERS    | \
                                        FLG_HEAP_VALIDATE_ALL           | \
                                        FLG_APPLICATION_VERIFIER        | \
                                        FLG_HEAP_ENABLE_TAGGING         | \
                                        FLG_USER_STACK_TRACE_DB         | \
                                        FLG_HEAP_ENABLE_TAG_BY_DLL      | \
                                        FLG_DISABLE_STACK_EXTENSION     | \
                                        FLG_ENABLE_SYSTEM_CRIT_BREAKS   | \
                                        FLG_HEAP_DISABLE_COALESCING     | \
                                        FLG_DISABLE_PROTDLLS            | \
                                        FLG_HEAP_PAGE_ALLOCS            | \
                                        FLG_CRITSEC_EVENT_CREATION      | \
                                        FLG_LDR_TOP_DOWN)

#define FLG_BOOTONLY_VALID_BITS        (FLG_KERNEL_STACK_TRACE_DB       | \
                                        FLG_MAINTAIN_OBJECT_TYPELIST    | \
                                        FLG_ENABLE_CSRDEBUG             | \
                                        FLG_DEBUG_INITIAL_COMMAND       | \
                                        FLG_DEBUG_INITIAL_COMMAND_EX    | \
                                        FLG_DISABLE_PAGE_KERNEL_STACKS)

#define FLG_KERNELMODE_VALID_BITS      (FLG_STOP_ON_EXCEPTION           | \
                                        FLG_SHOW_LDR_SNAPS              | \
                                        FLG_STOP_ON_HUNG_GUI            | \
                                        FLG_POOL_ENABLE_TAGGING         | \
                                        FLG_ENABLE_KDEBUG_SYMBOL_LOAD   | \
                                        FLG_ENABLE_CLOSE_EXCEPTIONS     | \
                                        FLG_ENABLE_EXCEPTION_LOGGING    | \
                                        FLG_ENABLE_HANDLE_TYPE_TAGGING  | \
                                        FLG_DISABLE_DBGPRINT            | \
                                        FLG_ENABLE_HANDLE_EXCEPTIONS      \
                                       )

//
// Routines for manipulating global atoms stored in kernel space
//

typedef USHORT RTL_ATOM, *PRTL_ATOM;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAddAtom (
    __in_bcount_opt(Length) PWSTR AtomName,
    __in ULONG Length,
    __out_opt PRTL_ATOM Atom
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFindAtom (
    __in_bcount_opt(Length) PWSTR AtomName,
    __in ULONG Length,
    __out_opt PRTL_ATOM Atom
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteAtom (
    __in RTL_ATOM Atom
    );

typedef enum _ATOM_INFORMATION_CLASS {
    AtomBasicInformation,
    AtomTableInformation
} ATOM_INFORMATION_CLASS;

typedef struct _ATOM_BASIC_INFORMATION {
    USHORT UsageCount;
    USHORT Flags;
    USHORT NameLength;
    WCHAR Name[ 1 ];
} ATOM_BASIC_INFORMATION, *PATOM_BASIC_INFORMATION;

typedef struct _ATOM_TABLE_INFORMATION {
    ULONG NumberOfAtoms;
    RTL_ATOM Atoms[ 1 ];
} ATOM_TABLE_INFORMATION, *PATOM_TABLE_INFORMATION;

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryInformationAtom(
    __in RTL_ATOM Atom,
    __in ATOM_INFORMATION_CLASS AtomInformationClass,
    __out_bcount(AtomInformationLength) PVOID AtomInformation,
    __in ULONG AtomInformationLength,
    __out_opt PULONG ReturnLength
    );


#ifdef __cplusplus
}
#endif

#endif // _NTEXAPI_

