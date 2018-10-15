/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   vfzwapi.h

Abstract:

    Zw interfaces verifier.

--*/

#ifndef _VF_ZWAPI_
#define _VF_ZWAPI_


#define DECLARE_ZW_VERIFIER_THUNK(Name) #Name,(PDRIVER_VERIFIER_THUNK_ROUTINE)Vf##Name

//NTSYSAPI
NTSTATUS
NTAPI
VfZwAccessCheckAndAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN PUNICODE_STRING ObjectTypeName,
    IN PUNICODE_STRING ObjectName,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ACCESS_MASK DesiredAccess,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwAddBootEntry (
    IN PBOOT_ENTRY BootEntry,
    OUT PULONG Id OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwAddDriverEntry (
    IN PEFI_DRIVER_ENTRY DriverEntry,
    OUT PULONG Id OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwAdjustPrivilegesToken (
    IN HANDLE TokenHandle,
    IN BOOLEAN DisableAllPrivileges,
    IN PTOKEN_PRIVILEGES NewState OPTIONAL,
    IN ULONG BufferLength OPTIONAL,
    OUT PTOKEN_PRIVILEGES PreviousState OPTIONAL,
    OUT PULONG ReturnLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwAlertThread(
    IN HANDLE ThreadHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwAllocateVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwAssignProcessToJobObject(
    IN HANDLE JobHandle,
    IN HANDLE ProcessHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCancelIoFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCancelTimer (
    IN HANDLE TimerHandle,
    OUT PBOOLEAN CurrentState OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwClearEvent (
    IN HANDLE EventHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwClose(
    IN HANDLE Handle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCloseObjectAuditAlarm (
    IN PUNICODE_STRING SubsystemName,
    IN PVOID HandleId,
    IN BOOLEAN GenerateOnClose
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwConnectPort(
    OUT PHANDLE PortHandle,
    IN PUNICODE_STRING PortName,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos,
    IN OUT PPORT_VIEW ClientView OPTIONAL,
    IN OUT PREMOTE_PORT_VIEW ServerView OPTIONAL,
    OUT PULONG MaxMessageLength OPTIONAL,
    IN OUT PVOID ConnectionInformation OPTIONAL,
    IN OUT PULONG ConnectionInformationLength OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCreateDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCreateEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN EVENT_TYPE EventType,
    IN BOOLEAN InitialState
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCreateJobObject (
    OUT PHANDLE JobHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCreateSection (
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize OPTIONAL,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCreateSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PUNICODE_STRING LinkTarget
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwCreateTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN TIMER_TYPE TimerType
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwDeleteBootEntry (
    IN ULONG Id
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwDeleteDriverEntry (
    IN ULONG Id
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwDeleteFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwDeleteKey(
    IN HANDLE KeyHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwDeviceIoControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwDisplayString(
    IN PUNICODE_STRING String
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwDuplicateObject(
    IN HANDLE SourceProcessHandle,
    IN HANDLE SourceHandle,
    IN HANDLE TargetProcessHandle OPTIONAL,
    OUT PHANDLE TargetHandle OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwDuplicateToken(
    IN HANDLE ExistingTokenHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE TokenType,
    OUT PHANDLE NewTokenHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwEnumerateBootEntries (
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwEnumerateDriverEntries (
    OUT PVOID Buffer,
    IN OUT PULONG BufferLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwFlushInstructionCache (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress OPTIONAL,
    IN SIZE_T Length
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwFlushKey(
    IN HANDLE KeyHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwFlushVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwFsControlFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG FsControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwInitiatePowerAction(
    IN POWER_ACTION SystemAction,
    IN SYSTEM_POWER_STATE MinSystemState,
    IN ULONG Flags,                 // POWER_ACTION_xxx flags
    IN BOOLEAN Asynchronous
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwIsProcessInJob (
    IN HANDLE ProcessHandle,
    IN HANDLE JobHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwLoadDriver(
    IN PUNICODE_STRING DriverServiceName
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwLoadKey(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN POBJECT_ATTRIBUTES SourceFile
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwMakeTemporaryObject(
    IN HANDLE Handle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwMapViewOfSection(
    IN HANDLE SectionHandle,
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN SIZE_T CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset OPTIONAL,
    IN OUT PSIZE_T ViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwModifyBootEntry (
    IN PBOOT_ENTRY BootEntry
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwModifyDriverEntry (
    IN PEFI_DRIVER_ENTRY DriverEntry
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwNotifyChangeKey(
    IN HANDLE KeyHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN Asynchronous
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenDirectoryObject(
    OUT PHANDLE DirectoryHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenEvent (
    OUT PHANDLE EventHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenJobObject(
    OUT PHANDLE JobHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenProcess (
    OUT PHANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenProcessToken(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE TokenHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenProcessTokenEx(
    IN HANDLE ProcessHandle,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenSection(
    OUT PHANDLE SectionHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenSymbolicLinkObject(
    OUT PHANDLE LinkHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenThread (
    OUT PHANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN PCLIENT_ID ClientId OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenThreadToken(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    OUT PHANDLE TokenHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenThreadTokenEx(
    IN HANDLE ThreadHandle,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN OpenAsSelf,
    IN ULONG HandleAttributes,
    OUT PHANDLE TokenHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwOpenTimer (
    OUT PHANDLE TimerHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwPulseEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryBootEntryOrder (
    OUT PULONG Ids,
    IN OUT PULONG Count
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryBootOptions (
    OUT PBOOT_OPTIONS BootOptions,
    IN OUT PULONG BootOptionsLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryDefaultLocale(
    IN BOOLEAN UserProfile,
    OUT PLCID DefaultLocaleId
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryDefaultUILanguage(
    OUT LANGID *DefaultUILanguageId
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryDirectoryFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN BOOLEAN ReturnSingleEntry,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN BOOLEAN RestartScan
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryDirectoryObject(
    IN HANDLE DirectoryHandle,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN BOOLEAN RestartScan,
    IN OUT PULONG Context,
    OUT PULONG ReturnLength OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryDriverEntryOrder (
    OUT PULONG Ids,
    IN OUT PULONG Count
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryEaFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN PVOID EaList OPTIONAL,
    IN ULONG EaListLength,
    IN PULONG EaIndex OPTIONAL,
    IN BOOLEAN RestartScan
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryFullAttributesFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryInformationJobObject(
    IN HANDLE JobHandle,
    IN JOBOBJECTINFOCLASS JobObjectInformationClass,
    OUT PVOID JobObjectInformation,
    IN ULONG JobObjectInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    OUT PVOID ThreadInformation,
    IN ULONG ThreadInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryInformationToken (
    IN HANDLE TokenHandle,
    IN TOKEN_INFORMATION_CLASS TokenInformationClass,
    OUT PVOID TokenInformation,
    IN ULONG TokenInformationLength,
    OUT PULONG ReturnLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryInstallUILanguage(
    OUT LANGID *InstallUILanguageId
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    OUT PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryObject(
    IN HANDLE Handle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    OUT PVOID ObjectInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQuerySection(
    IN HANDLE SectionHandle,
    IN SECTION_INFORMATION_CLASS SectionInformationClass,
    OUT PVOID SectionInformation,
    IN SIZE_T SectionInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQuerySecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Length,
    OUT PULONG LengthNeeded
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQuerySymbolicLinkObject(
    IN HANDLE LinkHandle,
    IN OUT PUNICODE_STRING LinkTarget,
    OUT PULONG ReturnedLength OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQuerySystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    OUT PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwQueryVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwReplaceKey(
    IN POBJECT_ATTRIBUTES NewFile,
    IN HANDLE             TargetHandle,
    IN POBJECT_ATTRIBUTES OldFile
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwRequestWaitReplyPort(
    IN HANDLE PortHandle,
    IN PPORT_MESSAGE RequestMessage,
    OUT PPORT_MESSAGE ReplyMessage
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwResetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwRestoreKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle,
    IN ULONG  Flags
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSaveKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSaveKeyEx(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle,
    IN ULONG  Format
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetBootEntryOrder (
    IN PULONG Ids,
    IN ULONG Count
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetBootOptions (
    IN PBOOT_OPTIONS BootOptions,
    IN ULONG FieldsToChange
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetDefaultLocale(
    IN BOOLEAN UserProfile,
    IN LCID DefaultLocaleId
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetDefaultUILanguage(
    IN LANGID DefaultUILanguageId
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetDriverEntryOrder (
    IN PULONG Ids,
    IN ULONG Count
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetEaFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetEvent (
    IN HANDLE EventHandle,
    OUT PLONG PreviousState OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetInformationJobObject(
    IN HANDLE JobHandle,
    IN JOBOBJECTINFOCLASS JobObjectInformationClass,
    IN PVOID JobObjectInformation,
    IN ULONG JobObjectInformationLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetInformationObject(
    IN HANDLE Handle,
    IN OBJECT_INFORMATION_CLASS ObjectInformationClass,
    IN PVOID ObjectInformation,
    IN ULONG ObjectInformationLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetInformationProcess(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    IN PVOID ProcessInformation,
    IN ULONG ProcessInformationLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetInformationThread(
    IN HANDLE ThreadHandle,
    IN THREADINFOCLASS ThreadInformationClass,
    IN PVOID ThreadInformation,
    IN ULONG ThreadInformationLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetSecurityObject(
    IN HANDLE Handle,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetSystemInformation (
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    IN PVOID SystemInformation,
    IN ULONG SystemInformationLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetSystemTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER PreviousTime OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetTimer (
    IN HANDLE TimerHandle,
    IN PLARGE_INTEGER DueTime,
    IN PTIMER_APC_ROUTINE TimerApcRoutine OPTIONAL,
    IN PVOID TimerContext OPTIONAL,
    IN BOOLEAN ResumeTimer,
    IN LONG Period OPTIONAL,
    OUT PBOOLEAN PreviousState OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwSetVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwTerminateJobObject(
    IN HANDLE JobHandle,
    IN NTSTATUS ExitStatus
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwTerminateProcess(
    IN HANDLE ProcessHandle OPTIONAL,
    IN NTSTATUS ExitStatus
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwTranslateFilePath (
    IN PFILE_PATH InputFilePath,
    IN ULONG OutputType,
    OUT PFILE_PATH OutputFilePath,
    IN OUT PULONG OutputFilePathLength
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwUnloadDriver(
    IN PUNICODE_STRING DriverServiceName
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwUnloadKey(
    IN POBJECT_ATTRIBUTES TargetKey
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwUnmapViewOfSection(
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwWaitForMultipleObjects(
    IN ULONG Count,
    IN HANDLE Handles[],
    IN WAIT_TYPE WaitType,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    );
//NTSYSAPI
NTSTATUS
NTAPI
VfZwYieldExecution (
    VOID
    );

#endif

