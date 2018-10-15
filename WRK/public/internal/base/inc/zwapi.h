//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
// If you do not agree to the terms, do not use the code.
//

NTSYSAPI
NTSTATUS
NTAPI
ZwDelayExecution (
    __in BOOLEAN Alertable,
    __in PLARGE_INTEGER DelayInterval
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemEnvironmentValue (
    __in PUNICODE_STRING VariableName,
    __out_bcount(ValueLength) PWSTR VariableValue,
    __in USHORT ValueLength,
    __out_opt PUSHORT ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetSystemEnvironmentValue (
    __in PUNICODE_STRING VariableName,
    __in PUNICODE_STRING VariableValue
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemEnvironmentValueEx (
    __in PUNICODE_STRING VariableName,
    __in LPGUID VendorGuid,
    __out_bcount_opt(*ValueLength) PVOID Value,
    __inout PULONG ValueLength,
    __out_opt PULONG Attributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetSystemEnvironmentValueEx (
    __in PUNICODE_STRING VariableName,
    __in LPGUID VendorGuid,
    __in_bcount_opt(ValueLength) PVOID Value,
    __in ULONG ValueLength,
    __in ULONG Attributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateSystemEnvironmentValuesEx (
    __in ULONG InformationClass,
    __out PVOID Buffer,
    __inout PULONG BufferLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAddBootEntry (
    __in PBOOT_ENTRY BootEntry,
    __out_opt PULONG Id
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteBootEntry (
    __in ULONG Id
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwModifyBootEntry (
    __in PBOOT_ENTRY BootEntry
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateBootEntries (
    __out_bcount_opt(*BufferLength) PVOID Buffer,
    __inout PULONG BufferLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryBootEntryOrder (
    __out_ecount_opt(*Count) PULONG Ids,
    __inout PULONG Count
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetBootEntryOrder (
    __in_ecount(Count) PULONG Ids,
    __in ULONG Count
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryBootOptions (
    __out_bcount_opt(*BootOptionsLength) PBOOT_OPTIONS BootOptions,
    __inout PULONG BootOptionsLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetBootOptions (
    __in PBOOT_OPTIONS BootOptions,
    __in ULONG FieldsToChange
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwTranslateFilePath (
    __in PFILE_PATH InputFilePath,
    __in ULONG OutputType,
    __out_bcount_opt(*OutputFilePathLength) PFILE_PATH OutputFilePath,
    __inout_opt PULONG OutputFilePathLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAddDriverEntry (
    __in PEFI_DRIVER_ENTRY DriverEntry,
    __out_opt PULONG Id
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteDriverEntry (
    __in ULONG Id
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwModifyDriverEntry (
    __in PEFI_DRIVER_ENTRY DriverEntry
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateDriverEntries (
    __out_bcount(*BufferLength) PVOID Buffer,
    __inout PULONG BufferLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDriverEntryOrder (
    __out_ecount(*Count) PULONG Ids,
    __inout PULONG Count
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetDriverEntryOrder (
    __in_ecount(Count) PULONG Ids,
    __in ULONG Count
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwClearEvent (
    __in HANDLE EventHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateEvent (
    __out PHANDLE EventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in EVENT_TYPE EventType,
    __in BOOLEAN InitialState
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenEvent (
    __out PHANDLE EventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwPulseEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryEvent (
    __in HANDLE EventHandle,
    __in EVENT_INFORMATION_CLASS EventInformationClass,
    __out_bcount(EventInformationLength) PVOID EventInformation,
    __in ULONG EventInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwResetEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetEvent (
    __in HANDLE EventHandle,
    __out_opt PLONG PreviousState
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetEventBoostPriority (
    __in HANDLE EventHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateEventPair (
    __out PHANDLE EventPairHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenEventPair (
    __out PHANDLE EventPairHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWaitLowEventPair ( 
    __in HANDLE EventPairHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWaitHighEventPair (
    __in HANDLE EventPairHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetLowWaitHighEventPair (
    __in HANDLE EventPairHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetHighWaitLowEventPair (
    __in HANDLE EventPairHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetLowEventPair (
    __in HANDLE EventPairHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetHighEventPair (
    __in HANDLE EventPairHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateMutant (
    __out PHANDLE MutantHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in BOOLEAN InitialOwner
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenMutant (
    __out PHANDLE MutantHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryMutant (
    __in HANDLE MutantHandle,
    __in MUTANT_INFORMATION_CLASS MutantInformationClass,
    __out_bcount(MutantInformationLength) PVOID MutantInformation,
    __in ULONG MutantInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReleaseMutant (
    __in HANDLE MutantHandle,
    __out_opt PLONG PreviousCount
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSemaphore (
    __out PHANDLE SemaphoreHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in LONG InitialCount,
    __in LONG MaximumCount
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenSemaphore(
    __out PHANDLE SemaphoreHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySemaphore (
    __in HANDLE SemaphoreHandle,
    __in SEMAPHORE_INFORMATION_CLASS SemaphoreInformationClass,
    __out_bcount(SemaphoreInformationLength) PVOID SemaphoreInformation,
    __in ULONG SemaphoreInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReleaseSemaphore(
    __in HANDLE SemaphoreHandle,
    __in LONG ReleaseCount,
    __out_opt PLONG PreviousCount
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateTimer (
    __out PHANDLE TimerHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in TIMER_TYPE TimerType
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenTimer (
    __out PHANDLE TimerHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCancelTimer (
    __in HANDLE TimerHandle,
    __out_opt PBOOLEAN CurrentState
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryTimer (
    __in HANDLE TimerHandle,
    __in TIMER_INFORMATION_CLASS TimerInformationClass,
    __out_bcount(TimerInformationLength) PVOID TimerInformation,
    __in ULONG TimerInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetTimer (
    __in HANDLE TimerHandle,
    __in PLARGE_INTEGER DueTime,
    __in_opt PTIMER_APC_ROUTINE TimerApcRoutine,
    __in_opt PVOID TimerContext,
    __in BOOLEAN ResumeTimer,
    __in_opt LONG Period,
    __out_opt PBOOLEAN PreviousState
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemTime (
    __out PLARGE_INTEGER SystemTime
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetSystemTime (
    __in_opt PLARGE_INTEGER SystemTime,
    __out_opt PLARGE_INTEGER PreviousTime
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryTimerResolution (
    __out PULONG MaximumTime,
    __out PULONG MinimumTime,
    __out PULONG CurrentTime
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetTimerResolution (
    __in ULONG DesiredTime,
    __in BOOLEAN SetResolution,
    __out PULONG ActualTime
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAllocateLocallyUniqueId (
    __out PLUID Luid
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetUuidSeed (
    __in PCHAR Seed
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAllocateUuids (
    __out PULARGE_INTEGER Time,
    __out PULONG Range,
    __out PULONG Sequence,
    __out PCHAR Seed
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateProfile (
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
NTSYSAPI
NTSTATUS
NTAPI
ZwStartProfile (
    __in HANDLE ProfileHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwStopProfile (
    __in HANDLE ProfileHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetIntervalProfile (
    __in ULONG Interval,
    __in KPROFILE_SOURCE Source
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryIntervalProfile (
    __in KPROFILE_SOURCE ProfileSource,
    __out PULONG Interval
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryPerformanceCounter (
    __out PLARGE_INTEGER PerformanceCounter,
    __out_opt PLARGE_INTEGER PerformanceFrequency
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateKeyedEvent (
    __out PHANDLE KeyedEventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG Flags
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenKeyedEvent (
    __out PHANDLE KeyedEventHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReleaseKeyedEvent (
    __in HANDLE KeyedEventHandle,
    __in PVOID KeyValue,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWaitForKeyedEvent (
    __in HANDLE KeyedEventHandle,
    __in PVOID KeyValue,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySystemInformation (
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
    __in ULONG SystemInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetSystemInformation (
    __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
    __in_bcount_opt(SystemInformationLength) PVOID SystemInformation,
    __in ULONG SystemInformationLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSystemDebugControl (
    __in SYSDBG_COMMAND Command,
    __inout_bcount_opt(InputBufferLength) PVOID InputBuffer,
    __in ULONG InputBufferLength,
    __out_bcount_opt(OutputBufferLength) PVOID OutputBuffer,
    __in ULONG OutputBufferLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwRaiseHardError (
    __in NTSTATUS ErrorStatus,
    __in ULONG NumberOfParameters,
    __in ULONG UnicodeStringParameterMask,
    __in_ecount(NumberOfParameters) PULONG_PTR Parameters,
    __in ULONG ValidResponseOptions,
    __out PULONG Response
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDefaultLocale (
    __in BOOLEAN UserProfile,
    __out PLCID DefaultLocaleId
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetDefaultLocale (
    __in BOOLEAN UserProfile,
    __in LCID DefaultLocaleId
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInstallUILanguage (
    __out LANGID *InstallUILanguageId
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDefaultUILanguage (
    __out LANGID *DefaultUILanguageId
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetDefaultUILanguage (
    __in LANGID DefaultUILanguageId
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetDefaultHardErrorPort(
    __in HANDLE DefaultHardErrorPort
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwShutdownSystem (
    __in SHUTDOWN_ACTION Action
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDisplayString (
    __in PUNICODE_STRING String
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAddAtom (
    __in_bcount_opt(Length) PWSTR AtomName,
    __in ULONG Length,
    __out_opt PRTL_ATOM Atom
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFindAtom (
    __in_bcount_opt(Length) PWSTR AtomName,
    __in ULONG Length,
    __out_opt PRTL_ATOM Atom
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteAtom (
    __in RTL_ATOM Atom
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationAtom(
    __in RTL_ATOM Atom,
    __in ATOM_INFORMATION_CLASS AtomInformationClass,
    __out_bcount(AtomInformationLength) PVOID AtomInformation,
    __in ULONG AtomInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCancelIoFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateNamedPipeFile (
     __out PHANDLE FileHandle,
     __in ULONG DesiredAccess,
     __in POBJECT_ATTRIBUTES ObjectAttributes,
     __out PIO_STATUS_BLOCK IoStatusBlock,
     __in ULONG ShareAccess,
     __in ULONG CreateDisposition,
     __in ULONG CreateOptions,
     __in ULONG NamedPipeType,
     __in ULONG ReadMode,
     __in ULONG CompletionMode,
     __in ULONG MaximumInstances,
     __in ULONG InboundQuota,
     __in ULONG OutboundQuota,
     __in_opt PLARGE_INTEGER DefaultTimeout
     );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateMailslotFile (
     __out PHANDLE FileHandle,
     __in ULONG DesiredAccess,
     __in POBJECT_ATTRIBUTES ObjectAttributes,
     __out PIO_STATUS_BLOCK IoStatusBlock,
     __in ULONG CreateOptions,
     __in ULONG MailslotQuota,
     __in ULONG MaximumMessageSize,
     __in PLARGE_INTEGER ReadTimeout
     );
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteFile (
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFlushBuffersFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwNotifyChangeDirectoryFile (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in ULONG CompletionFilter,
    __in BOOLEAN WatchTree
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryAttributesFile (
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __out PFILE_BASIC_INFORMATION FileInformation
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryFullAttributesFile(
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __out PFILE_NETWORK_OPEN_INFORMATION FileInformation
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryEaFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in BOOLEAN ReturnSingleEntry,
    __in_bcount_opt(EaListLength) PVOID EaList,
    __in ULONG EaListLength,
    __in_opt PULONG EaIndex OPTIONAL,
    __in BOOLEAN RestartScan
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateFile (
    __out PHANDLE FileHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_opt PLARGE_INTEGER AllocationSize,
    __in ULONG FileAttributes,
    __in ULONG ShareAccess,
    __in ULONG CreateDisposition,
    __in ULONG CreateOptions,
    __in_bcount_opt(EaLength) PVOID EaBuffer,
    __in ULONG EaLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDeviceIoControlFile (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in ULONG IoControlCode,
    __in_bcount_opt(InputBufferLength) PVOID InputBuffer,
    __in ULONG InputBufferLength,
    __out_bcount_opt(OutputBufferLength) PVOID OutputBuffer,
    __in ULONG OutputBufferLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFsControlFile (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in ULONG FsControlCode,
    __in_bcount_opt(InputBufferLength) PVOID InputBuffer,
    __in ULONG InputBufferLength,
    __out_bcount_opt(OutputBufferLength) PVOID OutputBuffer,
    __in ULONG OutputBufferLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwLockFile (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in PLARGE_INTEGER ByteOffset,
    __in PLARGE_INTEGER Length,
    __in ULONG Key,
    __in BOOLEAN FailImmediately,
    __in BOOLEAN ExclusiveLock
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenFile (
    __out PHANDLE FileHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in ULONG ShareAccess,
    __in ULONG OpenOptions
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDirectoryFile (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID FileInformation,
    __in ULONG Length,
    __in FILE_INFORMATION_CLASS FileInformationClass,
    __in BOOLEAN ReturnSingleEntry,
    __in_opt PUNICODE_STRING FileName,
    __in BOOLEAN RestartScan
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID FileInformation,
    __in ULONG Length,
    __in FILE_INFORMATION_CLASS FileInformationClass
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryQuotaInformationFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in BOOLEAN ReturnSingleEntry,
    __in_bcount_opt(SidListLength) PVOID SidList,
    __in ULONG SidListLength,
    __in_opt PSID StartSid,
    __in BOOLEAN RestartScan
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryVolumeInformationFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID FsInformation,
    __in ULONG Length,
    __in FS_INFORMATION_CLASS FsInformationClass
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReadFile (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __out_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in_opt PLARGE_INTEGER ByteOffset,
    __in_opt PULONG Key
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID FileInformation,
    __in ULONG Length,
    __in FILE_INFORMATION_CLASS FileInformationClass
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetQuotaInformationFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID Buffer,
    __in ULONG Length
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetVolumeInformationFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID FsInformation,
    __in ULONG Length,
    __in FS_INFORMATION_CLASS FsInformationClass
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWriteFile (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID Buffer,
    __in ULONG Length,
    __in_opt PLARGE_INTEGER ByteOffset,
    __in_opt PULONG Key
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwUnlockFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in PLARGE_INTEGER ByteOffset,
    __in PLARGE_INTEGER Length,
    __in ULONG Key
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReadFileScatter (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in PFILE_SEGMENT_ELEMENT SegmentArray,
    __in ULONG Length,
    __in_opt PLARGE_INTEGER ByteOffset,
    __in_opt PULONG Key
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetEaFile (
    __in HANDLE FileHandle,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_bcount(Length) PVOID Buffer,
    __in ULONG Length
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWriteFileGather (
    __in HANDLE FileHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in PFILE_SEGMENT_ELEMENT SegmentArray,
    __in ULONG Length,
    __in_opt PLARGE_INTEGER ByteOffset,
    __in_opt PULONG Key
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwLoadDriver (
    __in PUNICODE_STRING DriverServiceName
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwUnloadDriver (
    __in PUNICODE_STRING DriverServiceName
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateIoCompletion (
    __out PHANDLE IoCompletionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG Count OPTIONAL
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenIoCompletion (
    __out PHANDLE IoCompletionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryIoCompletion (
    __in HANDLE IoCompletionHandle,
    __in IO_COMPLETION_INFORMATION_CLASS IoCompletionInformationClass,
    __out_bcount(IoCompletionInformation) PVOID IoCompletionInformation,
    __in ULONG IoCompletionInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetIoCompletion (
    __in HANDLE IoCompletionHandle,
    __in PVOID KeyContext,
    __in_opt PVOID ApcContext,
    __in NTSTATUS IoStatus,
    __in ULONG_PTR IoStatusInformation
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwRemoveIoCompletion (
    __in HANDLE IoCompletionHandle,
    __out PVOID *KeyContext,
    __out PVOID *ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in_opt PLARGE_INTEGER Timeout
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCallbackReturn (
    __in_bcount_opt(OutputLength) PVOID OutputBuffer,
    __in ULONG OutputLength,
    __in NTSTATUS Status
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDebugFilterState (
    __in ULONG ComponentId,
    __in ULONG Level
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetDebugFilterState (
    __in ULONG ComponentId,
    __in ULONG Level,
    __in BOOLEAN State
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwYieldExecution (
    VOID
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreatePort(
    __out PHANDLE PortHandle,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG MaxConnectionInfoLength,
    __in ULONG MaxMessageLength,
    __in_opt ULONG MaxPoolUsage
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateWaitablePort(
    __out PHANDLE PortHandle,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in ULONG MaxConnectionInfoLength,
    __in ULONG MaxMessageLength,
    __in_opt ULONG MaxPoolUsage
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwConnectPort(
    __out PHANDLE PortHandle,
    __in PUNICODE_STRING PortName,
    __in PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    __inout_opt PPORT_VIEW ClientView,
    __inout_opt PREMOTE_PORT_VIEW ServerView,
    __out_opt PULONG MaxMessageLength,
    __inout_opt PVOID ConnectionInformation,
    __inout_opt PULONG ConnectionInformationLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSecureConnectPort(
    __out PHANDLE PortHandle,
    __in PUNICODE_STRING PortName,
    __in PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    __inout_opt PPORT_VIEW ClientView,
    __in_opt PSID RequiredServerSid,
    __inout_opt PREMOTE_PORT_VIEW ServerView,
    __out_opt PULONG MaxMessageLength,
    __inout_opt PVOID ConnectionInformation,
    __inout_opt PULONG ConnectionInformationLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwListenPort(
    __in HANDLE PortHandle,
    __out PPORT_MESSAGE ConnectionRequest
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAcceptConnectPort(
    __out PHANDLE PortHandle,
    __in_opt PVOID PortContext,
    __in PPORT_MESSAGE ConnectionRequest,
    __in BOOLEAN AcceptConnection,
    __inout_opt PPORT_VIEW ServerView,
    __out_opt PREMOTE_PORT_VIEW ClientView
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCompleteConnectPort(
    __in HANDLE PortHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwRequestPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE RequestMessage
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwRequestWaitReplyPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE RequestMessage,
    __out PPORT_MESSAGE ReplyMessage
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReplyPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE ReplyMessage
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReplyWaitReplyPort(
    __in HANDLE PortHandle,
    __inout PPORT_MESSAGE ReplyMessage
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReplyWaitReceivePort(
    __in HANDLE PortHandle,
    __out_opt PVOID *PortContext ,
    __in_opt PPORT_MESSAGE ReplyMessage,
    __out PPORT_MESSAGE ReceiveMessage
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReplyWaitReceivePortEx(
    __in HANDLE PortHandle,
    __out_opt PVOID *PortContext,
    __in_opt PPORT_MESSAGE ReplyMessage,
    __out PPORT_MESSAGE ReceiveMessage,
    __in_opt PLARGE_INTEGER Timeout
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwImpersonateClientOfPort(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE Message
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReadRequestData(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE Message,
    __in ULONG DataEntryIndex,
    __out_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesRead
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWriteRequestData(
    __in HANDLE PortHandle,
    __in PPORT_MESSAGE Message,
    __in ULONG DataEntryIndex,
    __in_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesWritten
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationPort(
    __in HANDLE PortHandle,
    __in PORT_INFORMATION_CLASS PortInformationClass,
    __out_bcount(Length) PVOID PortInformation,
    __in ULONG Length,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSection (
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt PLARGE_INTEGER MaximumSize,
    __in ULONG SectionPageProtection,
    __in ULONG AllocationAttributes,
    __in_opt HANDLE FileHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenSection (
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwMapViewOfSection (
    __in HANDLE SectionHandle,
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __in ULONG_PTR ZeroBits,
    __in SIZE_T CommitSize,
    __inout_opt PLARGE_INTEGER SectionOffset,
    __inout PSIZE_T ViewSize,
    __in SECTION_INHERIT InheritDisposition,
    __in ULONG AllocationType,
    __in ULONG Win32Protect
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwUnmapViewOfSection (
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwExtendSection (
    __in HANDLE SectionHandle,
    __inout PLARGE_INTEGER NewSectionSize
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAreMappedFilesTheSame (
    __in PVOID File1MappedAsAnImage,
    __in PVOID File2MappedAsFile
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAllocateVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __in ULONG_PTR ZeroBits,
    __inout PSIZE_T RegionSize,
    __in ULONG AllocationType,
    __in ULONG Protect
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFreeVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in ULONG FreeType
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReadVirtualMemory (
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress,
    __out_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesRead
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWriteVirtualMemory (
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress,
    __in_bcount(BufferSize) CONST VOID *Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesWritten
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFlushVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __out PIO_STATUS_BLOCK IoStatus
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwLockVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in ULONG MapType
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwUnlockVirtualMemory ( 
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in ULONG MapType
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwProtectVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in ULONG NewProtect,
    __out PULONG OldProtect
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryVirtualMemory (
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __in MEMORY_INFORMATION_CLASS MemoryInformationClass,
    __out_bcount(MemoryInformationLength) PVOID MemoryInformation,
    __in SIZE_T MemoryInformationLength,
    __out_opt PSIZE_T ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySection (
    __in HANDLE SectionHandle,
    __in SECTION_INFORMATION_CLASS SectionInformationClass,
    __out_bcount(SectionInformationLength) PVOID SectionInformation,
    __in SIZE_T SectionInformationLength,
    __out_opt PSIZE_T ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwMapUserPhysicalPages (
    __in PVOID VirtualAddress,
    __in ULONG_PTR NumberOfPages,
    __in_ecount_opt(NumberOfPages) PULONG_PTR UserPfnArray
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwMapUserPhysicalPagesScatter (
    __in_ecount(NumberOfPages) PVOID *VirtualAddresses,
    __in ULONG_PTR NumberOfPages,
    __in_ecount_opt(NumberOfPages) PULONG_PTR UserPfnArray
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAllocateUserPhysicalPages (
    __in HANDLE ProcessHandle,
    __inout PULONG_PTR NumberOfPages,
    __out_ecount(*NumberOfPages) PULONG_PTR UserPfnArray
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFreeUserPhysicalPages (
    __in HANDLE ProcessHandle,
    __inout PULONG_PTR NumberOfPages,
    __in_ecount(*NumberOfPages) PULONG_PTR UserPfnArray
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwGetWriteWatch (
    __in HANDLE ProcessHandle,
    __in ULONG Flags,
    __in PVOID BaseAddress,
    __in SIZE_T RegionSize,
    __out_ecount(*EntriesInUserAddressArray) PVOID *UserAddressArray,
    __inout PULONG_PTR EntriesInUserAddressArray,
    __out PULONG Granularity
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwResetWriteWatch (
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __in SIZE_T RegionSize
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreatePagingFile (
    __in PUNICODE_STRING PageFileName,
    __in PLARGE_INTEGER MinimumSize,
    __in PLARGE_INTEGER MaximumSize,
    __in ULONG Priority
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFlushInstructionCache (
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress,
    __in SIZE_T Length
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFlushWriteBuffer (
    VOID
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryObject (
    __in HANDLE Handle,
    __in OBJECT_INFORMATION_CLASS ObjectInformationClass,
    __out_bcount_opt(ObjectInformationLength) PVOID ObjectInformation,
    __in ULONG ObjectInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationObject (
    __in HANDLE Handle,
    __in OBJECT_INFORMATION_CLASS ObjectInformationClass,
    __in_bcount(ObjectInformationLength) PVOID ObjectInformation,
    __in ULONG ObjectInformationLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDuplicateObject (
    __in HANDLE SourceProcessHandle,
    __in HANDLE SourceHandle,
    __in_opt HANDLE TargetProcessHandle,
    __out_opt PHANDLE TargetHandle,
    __in ACCESS_MASK DesiredAccess,
    __in ULONG HandleAttributes,
    __in ULONG Options
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwMakeTemporaryObject (
    __in HANDLE Handle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwMakePermanentObject (
    __in HANDLE Handle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSignalAndWaitForSingleObject (
    __in HANDLE SignalHandle,
    __in HANDLE WaitHandle,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWaitForSingleObject (
    __in HANDLE Handle,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWaitForMultipleObjects (
    __in ULONG Count,
    __in_ecount(Count) HANDLE Handles[],
    __in WAIT_TYPE WaitType,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwWaitForMultipleObjects32 (
    __in ULONG Count,
    __in_ecount(Count) LONG Handles[],
    __in WAIT_TYPE WaitType,
    __in BOOLEAN Alertable,
    __in_opt PLARGE_INTEGER Timeout
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetSecurityObject (
    __in HANDLE Handle,
    __in SECURITY_INFORMATION SecurityInformation,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySecurityObject (
    __in HANDLE Handle,
    __in SECURITY_INFORMATION SecurityInformation,
    __out_bcount_opt(Length) PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in ULONG Length,
    __out PULONG LengthNeeded
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwClose (
    __in HANDLE Handle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateDirectoryObject (
    __out PHANDLE DirectoryHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenDirectoryObject (
    __out PHANDLE DirectoryHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryDirectoryObject (
    __in HANDLE DirectoryHandle,
    __out_bcount_opt(Length) PVOID Buffer,
    __in ULONG Length,
    __in BOOLEAN ReturnSingleEntry,
    __in BOOLEAN RestartScan,
    __inout PULONG Context,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateSymbolicLinkObject (
    __out PHANDLE LinkHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in PUNICODE_STRING LinkTarget
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenSymbolicLinkObject (
    __out PHANDLE LinkHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQuerySymbolicLinkObject (
    __in HANDLE LinkHandle,
    __inout PUNICODE_STRING LinkTarget,
    __out_opt PULONG ReturnedLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwGetPlugPlayEvent (
    __in HANDLE EventHandle,
    __in_opt PVOID Context,
    __out_bcount(EventBufferSize) PPLUGPLAY_EVENT_BLOCK EventBlock,
    __in  ULONG EventBufferSize
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwPlugPlayControl(
    __in PLUGPLAY_CONTROL_CLASS PnPControlClass,
    __inout_bcount(PnPControlDataLength) PVOID PnPControlData,
    __in ULONG PnPControlDataLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwPowerInformation(
    __in POWER_INFORMATION_LEVEL InformationLevel,
    __in_bcount_opt(InputBufferLength) PVOID InputBuffer,
    __in ULONG InputBufferLength,
    __out_bcount_opt(OutputBufferLength) PVOID OutputBuffer,
    __in ULONG OutputBufferLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetThreadExecutionState(
    __in EXECUTION_STATE esFlags,               // ES_xxx flags
    __out EXECUTION_STATE *PreviousFlags
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwRequestWakeupLatency(
    __in LATENCY_TIME latency
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwInitiatePowerAction(
    __in POWER_ACTION SystemAction,
    __in SYSTEM_POWER_STATE MinSystemState,
    __in ULONG Flags,                 // POWER_ACTION_xxx flags
    __in BOOLEAN Asynchronous
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetSystemPowerState(
    __in POWER_ACTION SystemAction,
    __in SYSTEM_POWER_STATE MinSystemState,
    __in ULONG Flags                  // POWER_ACTION_xxx flags
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwGetDevicePowerState(
    __in HANDLE Device,
    __out DEVICE_POWER_STATE *State
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCancelDeviceWakeupRequest(
    __in HANDLE Device
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwRequestDeviceWakeup(
    __in HANDLE Device
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateProcess (
    __out PHANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in HANDLE ParentProcess,
    __in BOOLEAN InheritObjectTable,
    __in_opt HANDLE SectionHandle,
    __in_opt HANDLE DebugPort,
    __in_opt HANDLE ExceptionPort
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateProcessEx (
    __out PHANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in HANDLE ParentProcess,
    __in ULONG Flags,
    __in_opt HANDLE SectionHandle,
    __in_opt HANDLE DebugPort,
    __in_opt HANDLE ExceptionPort,
    __in ULONG JobMemberLevel
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcess (
    __out PHANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt PCLIENT_ID ClientId
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwTerminateProcess (
    __in_opt HANDLE ProcessHandle,
    __in NTSTATUS ExitStatus
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationProcess (
    __in HANDLE ProcessHandle,
    __in PROCESSINFOCLASS ProcessInformationClass,
    __out_bcount(ProcessInformationLength) PVOID ProcessInformation,
    __in ULONG ProcessInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwGetNextProcess (
    __in HANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in ULONG HandleAttributes,
    __in ULONG Flags,
    __out PHANDLE NewProcessHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwGetNextThread (
    __in HANDLE ProcessHandle,
    __in HANDLE ThreadHandle,
    __in ACCESS_MASK DesiredAccess,
    __in ULONG HandleAttributes,
    __in ULONG Flags,
    __out PHANDLE NewThreadHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryPortInformationProcess (
    VOID
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationProcess (
    __in HANDLE ProcessHandle,
    __in PROCESSINFOCLASS ProcessInformationClass,
    __in_bcount(ProcessInformationLength) PVOID ProcessInformation,
    __in ULONG ProcessInformationLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateThread (
    __out PHANDLE ThreadHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in HANDLE ProcessHandle,
    __out PCLIENT_ID ClientId,
    __in PCONTEXT ThreadContext,
    __in PINITIAL_TEB InitialTeb,
    __in BOOLEAN CreateSuspended
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenThread (
    __out PHANDLE ThreadHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt PCLIENT_ID ClientId
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwTerminateThread (
    __in_opt HANDLE ThreadHandle,
    __in NTSTATUS ExitStatus
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSuspendThread (
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwResumeThread (
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSuspendProcess (
    __in HANDLE ProcessHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwResumeProcess (
    __in HANDLE ProcessHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwGetContextThread (
    __in HANDLE ThreadHandle,
    __inout PCONTEXT ThreadContext
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetContextThread (
    __in HANDLE ThreadHandle,
    __in PCONTEXT ThreadContext
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationThread (
    __in HANDLE ThreadHandle,
    __in THREADINFOCLASS ThreadInformationClass,
    __out_bcount(ThreadInformationLength) PVOID ThreadInformation,
    __in ULONG ThreadInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationThread (
    __in HANDLE ThreadHandle,
    __in THREADINFOCLASS ThreadInformationClass,
    __in_bcount(ThreadInformationLength) PVOID ThreadInformation,
    __in ULONG ThreadInformationLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAlertThread (
    __in HANDLE ThreadHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAlertResumeThread (
    __in HANDLE ThreadHandle,
    __out_opt PULONG PreviousSuspendCount
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwImpersonateThread (
    __in HANDLE ServerThreadHandle,
    __in HANDLE ClientThreadHandle,
    __in PSECURITY_QUALITY_OF_SERVICE SecurityQos
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwTestAlert (
    VOID
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwRegisterThreadTerminatePort (
    __in HANDLE PortHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetLdtEntries (
    __in ULONG Selector0,
    __in ULONG Entry0Low,
    __in ULONG Entry0Hi,
    __in ULONG Selector1,
    __in ULONG Entry1Low,
    __in ULONG Entry1Hi
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueueApcThread (
    __in HANDLE ThreadHandle,
    __in PPS_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcArgument1,
    __in_opt PVOID ApcArgument2,
    __in_opt PVOID ApcArgument3
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateJobObject (
    __out PHANDLE JobHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenJobObject (
    __out PHANDLE JobHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAssignProcessToJobObject (
    __in HANDLE JobHandle,
    __in HANDLE ProcessHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwTerminateJobObject (
    __in HANDLE JobHandle,
    __in NTSTATUS ExitStatus
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwIsProcessInJob (
    __in HANDLE ProcessHandle,
    __in_opt HANDLE JobHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateJobSet (
    __in ULONG NumJob,
    __in_ecount(NumJob) PJOB_SET_ARRAY UserJobSet,
    __in ULONG Flags
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationJobObject (
    __in_opt HANDLE JobHandle,
    __in JOBOBJECTINFOCLASS JobObjectInformationClass,
    __out_bcount(JobObjectInformationLength) PVOID JobObjectInformation,
    __in ULONG JobObjectInformationLength,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationJobObject (
    __in HANDLE JobHandle,
    __in JOBOBJECTINFOCLASS JobObjectInformationClass,
    __in_bcount(JobObjectInformationLength) PVOID JobObjectInformation,
    __in ULONG JobObjectInformationLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateKey(
    __out PHANDLE KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __reserved ULONG TitleIndex,
    __in_opt PUNICODE_STRING Class,
    __in ULONG CreateOptions,
    __out_opt PULONG Disposition
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteKey(
    __in HANDLE KeyHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteValueKey(
    __in HANDLE KeyHandle,
    __in PUNICODE_STRING ValueName
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateKey(
    __in HANDLE KeyHandle,
    __in ULONG Index,
    __in KEY_INFORMATION_CLASS KeyInformationClass,
    __out_bcount_opt(Length) PVOID KeyInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwEnumerateValueKey(
    __in HANDLE KeyHandle,
    __in ULONG Index,
    __in KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_bcount_opt(Length) PVOID KeyValueInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFlushKey(
    __in HANDLE KeyHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwInitializeRegistry(
    __in USHORT BootCondition
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwNotifyChangeKey(
    __in HANDLE KeyHandle,
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in ULONG CompletionFilter,
    __in BOOLEAN WatchTree,
    __out_bcount_opt(BufferSize) PVOID Buffer,
    __in ULONG BufferSize,
    __in BOOLEAN Asynchronous
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwNotifyChangeMultipleKeys(
    __in HANDLE MasterKeyHandle,
    __in_opt ULONG Count,
    __in_ecount_opt(Count) OBJECT_ATTRIBUTES SlaveObjects[],
    __in_opt HANDLE Event,
    __in_opt PIO_APC_ROUTINE ApcRoutine,
    __in_opt PVOID ApcContext,
    __out PIO_STATUS_BLOCK IoStatusBlock,
    __in ULONG CompletionFilter,
    __in BOOLEAN WatchTree,
    __out_bcount_opt(BufferSize) PVOID Buffer,
    __in ULONG BufferSize,
    __in BOOLEAN Asynchronous
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwLoadKey(
    __in POBJECT_ATTRIBUTES TargetKey,
    __in POBJECT_ATTRIBUTES SourceFile
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwLoadKey2(
    __in POBJECT_ATTRIBUTES   TargetKey,
    __in POBJECT_ATTRIBUTES   SourceFile,
    __in ULONG                Flags
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwLoadKeyEx(
    __in POBJECT_ATTRIBUTES   TargetKey,
    __in POBJECT_ATTRIBUTES   SourceFile,
    __in ULONG                Flags,
    __in_opt HANDLE           TrustClassKey 
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenKey(
    __out PHANDLE KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryKey(
    __in HANDLE KeyHandle,
    __in KEY_INFORMATION_CLASS KeyInformationClass,
    __out_bcount_opt(Length) PVOID KeyInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryValueKey(
    __in HANDLE KeyHandle,
    __in PUNICODE_STRING ValueName,
    __in KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_bcount_opt(Length) PVOID KeyValueInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryMultipleValueKey(
    __in HANDLE KeyHandle,
    __inout_ecount(EntryCount) PKEY_VALUE_ENTRY ValueEntries,
    __in ULONG EntryCount,
    __out_bcount(*BufferLength) PVOID ValueBuffer,
    __inout PULONG BufferLength,
    __out_opt PULONG RequiredBufferLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwReplaceKey(
    __in POBJECT_ATTRIBUTES NewFile,
    __in HANDLE             TargetHandle,
    __in POBJECT_ATTRIBUTES OldFile
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwRenameKey(
    __in HANDLE           KeyHandle,
    __in PUNICODE_STRING  NewName
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCompactKeys(
    __in ULONG Count,
    __in_ecount(Count) HANDLE KeyArray[]
            );
NTSYSAPI
NTSTATUS
NTAPI
ZwCompressKey(
    __in HANDLE Key
            );
NTSYSAPI
NTSTATUS
NTAPI
ZwRestoreKey(
    __in HANDLE KeyHandle,
    __in HANDLE FileHandle,
    __in ULONG Flags
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSaveKey(
    __in HANDLE KeyHandle,
    __in HANDLE FileHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSaveKeyEx(
    __in HANDLE KeyHandle,
    __in HANDLE FileHandle,
    __in ULONG  Format
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSaveMergedKeys(
    __in HANDLE HighPrecedenceKeyHandle,
    __in HANDLE LowPrecedenceKeyHandle,
    __in HANDLE FileHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetValueKey(
    __in HANDLE KeyHandle,
    __in PUNICODE_STRING ValueName,
    __in_opt ULONG TitleIndex,
    __in ULONG Type,
    __in_bcount_opt(DataSize) PVOID Data,
    __in ULONG DataSize
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwUnloadKey(
    __in POBJECT_ATTRIBUTES TargetKey
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwUnloadKey2(
    __in POBJECT_ATTRIBUTES   TargetKey,
    __in ULONG                Flags
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwUnloadKeyEx(
    __in POBJECT_ATTRIBUTES TargetKey,
    __in_opt HANDLE Event
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationKey(
    __in HANDLE KeyHandle,
    __in KEY_SET_INFORMATION_CLASS KeySetInformationClass,
    __in_bcount(KeySetInformationLength) PVOID KeySetInformation,
    __in ULONG KeySetInformationLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryOpenSubKeys(
    __in POBJECT_ATTRIBUTES TargetKey,
    __out PULONG  HandleCount
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryOpenSubKeysEx(
    __in POBJECT_ATTRIBUTES   TargetKey,
    __in ULONG                BufferLength,
    __out_bcount(BufferLength) PVOID               Buffer,
    __out PULONG              RequiredSize
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwLockRegistryKey(
    __in HANDLE           KeyHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwLockProductActivationKeys(
    __inout_opt ULONG   *pPrivateVer,
    __out_opt ULONG   *pSafeMode
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAccessCheck (
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in HANDLE ClientToken,
    __in ACCESS_MASK DesiredAccess,
    __in PGENERIC_MAPPING GenericMapping,
    __out_bcount(*PrivilegeSetLength) PPRIVILEGE_SET PrivilegeSet,
    __inout PULONG PrivilegeSetLength,
    __out PACCESS_MASK GrantedAccess,
    __out PNTSTATUS AccessStatus
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAccessCheckByType (
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in_opt PSID PrincipalSelfSid,
    __in HANDLE ClientToken,
    __in ACCESS_MASK DesiredAccess,
    __in_ecount(ObjectTypeListLength) POBJECT_TYPE_LIST ObjectTypeList,
    __in ULONG ObjectTypeListLength,
    __in PGENERIC_MAPPING GenericMapping,
    __out_bcount(*PrivilegeSetLength) PPRIVILEGE_SET PrivilegeSet,
    __inout PULONG PrivilegeSetLength,
    __out PACCESS_MASK GrantedAccess,
    __out PNTSTATUS AccessStatus
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAccessCheckByTypeResultList (
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in_opt PSID PrincipalSelfSid,
    __in HANDLE ClientToken,
    __in ACCESS_MASK DesiredAccess,
    __in_ecount(ObjectTypeListLength) POBJECT_TYPE_LIST ObjectTypeList,
    __in ULONG ObjectTypeListLength,
    __in PGENERIC_MAPPING GenericMapping,
    __out_bcount(*PrivilegeSetLength) PPRIVILEGE_SET PrivilegeSet,
    __inout PULONG PrivilegeSetLength,
    __out_ecount(ObjectTypeListLength) PACCESS_MASK GrantedAccess,
    __out_ecount(ObjectTypeListLength) PNTSTATUS AccessStatus
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCreateToken(
    __out PHANDLE TokenHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in TOKEN_TYPE TokenType,
    __in PLUID AuthenticationId,
    __in PLARGE_INTEGER ExpirationTime,
    __in PTOKEN_USER User,
    __in PTOKEN_GROUPS Groups,
    __in PTOKEN_PRIVILEGES Privileges,
    __in_opt PTOKEN_OWNER Owner,
    __in PTOKEN_PRIMARY_GROUP PrimaryGroup,
    __in_opt PTOKEN_DEFAULT_DACL DefaultDacl,
    __in PTOKEN_SOURCE TokenSource
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCompareTokens(
    __in HANDLE FirstTokenHandle,
    __in HANDLE SecondTokenHandle,
    __out PBOOLEAN Equal
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenThreadToken(
    __in HANDLE ThreadHandle,
    __in ACCESS_MASK DesiredAccess,
    __in BOOLEAN OpenAsSelf,
    __out PHANDLE TokenHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenThreadTokenEx(
    __in HANDLE ThreadHandle,
    __in ACCESS_MASK DesiredAccess,
    __in BOOLEAN OpenAsSelf,
    __in ULONG HandleAttributes,
    __out PHANDLE TokenHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcessToken(
    __in HANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __out PHANDLE TokenHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenProcessTokenEx(
    __in HANDLE ProcessHandle,
    __in ACCESS_MASK DesiredAccess,
    __in ULONG HandleAttributes,
    __out PHANDLE TokenHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDuplicateToken(
    __in HANDLE ExistingTokenHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in BOOLEAN EffectiveOnly,
    __in TOKEN_TYPE TokenType,
    __out PHANDLE NewTokenHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwFilterToken (
    __in HANDLE ExistingTokenHandle,
    __in ULONG Flags,
    __in_opt PTOKEN_GROUPS SidsToDisable,
    __in_opt PTOKEN_PRIVILEGES PrivilegesToDelete,
    __in_opt PTOKEN_GROUPS RestrictedSids,
    __out PHANDLE NewTokenHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwImpersonateAnonymousToken(
    __in HANDLE ThreadHandle
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwQueryInformationToken (
    __in HANDLE TokenHandle,
    __in TOKEN_INFORMATION_CLASS TokenInformationClass,
    __out_bcount_part_opt(TokenInformationLength,*ReturnLength) PVOID TokenInformation,
    __in ULONG TokenInformationLength,
    __out PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwSetInformationToken (
    __in HANDLE TokenHandle,
    __in TOKEN_INFORMATION_CLASS TokenInformationClass,
    __in_bcount(TokenInformationLength) PVOID TokenInformation,
    __in ULONG TokenInformationLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAdjustPrivilegesToken (
    __in HANDLE TokenHandle,
    __in BOOLEAN DisableAllPrivileges,
    __in_opt PTOKEN_PRIVILEGES NewState,
    __in_opt ULONG BufferLength,
    __out_bcount_part_opt(BufferLength,*ReturnLength) PTOKEN_PRIVILEGES PreviousState,
    __out_opt PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAdjustGroupsToken (
    __in HANDLE TokenHandle,
    __in BOOLEAN ResetToDefault,
    __in PTOKEN_GROUPS NewState ,
    __in_opt ULONG BufferLength ,
    __out_bcount_part_opt(BufferLength, *ReturnLength) PTOKEN_GROUPS PreviousState ,
    __out PULONG ReturnLength
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwPrivilegeCheck (
    __in HANDLE ClientToken,
    __inout PPRIVILEGE_SET RequiredPrivileges,
    __out PBOOLEAN Result
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAccessCheckAndAuditAlarm (
    __in PUNICODE_STRING SubsystemName,
    __in_opt PVOID HandleId,
    __in PUNICODE_STRING ObjectTypeName,
    __in PUNICODE_STRING ObjectName,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in ACCESS_MASK DesiredAccess,
    __in PGENERIC_MAPPING GenericMapping,
    __in BOOLEAN ObjectCreation,
    __out PACCESS_MASK GrantedAccess,
    __out PNTSTATUS AccessStatus,
    __out PBOOLEAN GenerateOnClose
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAccessCheckByTypeAndAuditAlarm (
    __in PUNICODE_STRING SubsystemName,
    __in_opt PVOID HandleId,
    __in PUNICODE_STRING ObjectTypeName,
    __in PUNICODE_STRING ObjectName,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in_opt PSID PrincipalSelfSid,
    __in ACCESS_MASK DesiredAccess,
    __in AUDIT_EVENT_TYPE AuditType,
    __in ULONG Flags,
    __in_ecount_opt(ObjectTypeListLength) POBJECT_TYPE_LIST ObjectTypeList,
    __in ULONG ObjectTypeListLength,
    __in PGENERIC_MAPPING GenericMapping,
    __in BOOLEAN ObjectCreation,
    __out PACCESS_MASK GrantedAccess,
    __out PNTSTATUS AccessStatus,
    __out PBOOLEAN GenerateOnClose
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAccessCheckByTypeResultListAndAuditAlarm (
    __in PUNICODE_STRING SubsystemName,
    __in_opt PVOID HandleId,
    __in PUNICODE_STRING ObjectTypeName,
    __in PUNICODE_STRING ObjectName,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in_opt PSID PrincipalSelfSid,
    __in ACCESS_MASK DesiredAccess,
    __in AUDIT_EVENT_TYPE AuditType,
    __in ULONG Flags,
    __in_ecount_opt(ObjectTypeListLength) POBJECT_TYPE_LIST ObjectTypeList,
    __in ULONG ObjectTypeListLength,
    __in PGENERIC_MAPPING GenericMapping,
    __in BOOLEAN ObjectCreation,
    __out_ecount(ObjectTypeListLength) PACCESS_MASK GrantedAccess,
    __out_ecount(ObjectTypeListLength) PNTSTATUS AccessStatus,
    __out PBOOLEAN GenerateOnClose
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwAccessCheckByTypeResultListAndAuditAlarmByHandle (
    __in PUNICODE_STRING SubsystemName,
    __in_opt PVOID HandleId,
    __in HANDLE ClientToken,
    __in PUNICODE_STRING ObjectTypeName,
    __in PUNICODE_STRING ObjectName,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in_opt PSID PrincipalSelfSid,
    __in ACCESS_MASK DesiredAccess,
    __in AUDIT_EVENT_TYPE AuditType,
    __in ULONG Flags,
    __in_ecount_opt(ObjectTypeListLength) POBJECT_TYPE_LIST ObjectTypeList,
    __in ULONG ObjectTypeListLength,
    __in PGENERIC_MAPPING GenericMapping,
    __in BOOLEAN ObjectCreation,
    __out_ecount(ObjectTypeListLength) PACCESS_MASK GrantedAccess,
    __out_ecount(ObjectTypeListLength) PNTSTATUS AccessStatus,
    __out PBOOLEAN GenerateOnClose
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwOpenObjectAuditAlarm (
    __in PUNICODE_STRING SubsystemName,
    __in_opt PVOID HandleId,
    __in PUNICODE_STRING ObjectTypeName,
    __in PUNICODE_STRING ObjectName,
    __in_opt PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in HANDLE ClientToken,
    __in ACCESS_MASK DesiredAccess,
    __in ACCESS_MASK GrantedAccess,
    __in_opt PPRIVILEGE_SET Privileges,
    __in BOOLEAN ObjectCreation,
    __in BOOLEAN AccessGranted,
    __out PBOOLEAN GenerateOnClose
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwPrivilegeObjectAuditAlarm (
    __in PUNICODE_STRING SubsystemName,
    __in_opt PVOID HandleId,
    __in HANDLE ClientToken,
    __in ACCESS_MASK DesiredAccess,
    __in PPRIVILEGE_SET Privileges,
    __in BOOLEAN AccessGranted
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwCloseObjectAuditAlarm (
    __in PUNICODE_STRING SubsystemName,
    __in_opt PVOID HandleId,
    __in BOOLEAN GenerateOnClose
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwDeleteObjectAuditAlarm (
    __in PUNICODE_STRING SubsystemName,
    __in_opt PVOID HandleId,
    __in BOOLEAN GenerateOnClose
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwPrivilegedServiceAuditAlarm (
    __in PUNICODE_STRING SubsystemName,
    __in PUNICODE_STRING ServiceName,
    __in HANDLE ClientToken,
    __in PPRIVILEGE_SET Privileges,
    __in BOOLEAN AccessGranted
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwTraceEvent(
    __in HANDLE TraceHandle,
    __in ULONG Flags,
    __in ULONG FieldSize,
    __in PVOID Fields
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwContinue (
    __in PCONTEXT ContextRecord,
    __in BOOLEAN TestAlert
    );
NTSYSAPI
NTSTATUS
NTAPI
ZwRaiseException (
    __in PEXCEPTION_RECORD ExceptionRecord,
    __in PCONTEXT ContextRecord,
    __in BOOLEAN FirstChance
    );

#if _MSC_VER > 1000
#pragma once
#endif

