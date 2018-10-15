/*++ BUILD Version: 0009    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntregapi.h

Abstract:

    This module contains the registration apis and related structures,
    in the forms for use with the Nt api set (as opposed to the win api set).

--*/

#ifndef _NTREGAPI_
#define _NTREGAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// begin_winnt
//

// begin_ntddk begin_wdm begin_nthal
//
// Registry Specific Access Rights.
//

#define KEY_QUERY_VALUE         (0x0001)
#define KEY_SET_VALUE           (0x0002)
#define KEY_CREATE_SUB_KEY      (0x0004)
#define KEY_ENUMERATE_SUB_KEYS  (0x0008)
#define KEY_NOTIFY              (0x0010)
#define KEY_CREATE_LINK         (0x0020)
#define KEY_WOW64_32KEY         (0x0200)
#define KEY_WOW64_64KEY         (0x0100)
#define KEY_WOW64_RES           (0x0300)

#define KEY_READ                ((STANDARD_RIGHTS_READ       |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY)                 \
                                  &                           \
                                 (~SYNCHRONIZE))


#define KEY_WRITE               ((STANDARD_RIGHTS_WRITE      |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY)         \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_EXECUTE             ((KEY_READ)                   \
                                  &                           \
                                 (~SYNCHRONIZE))

#define KEY_ALL_ACCESS          ((STANDARD_RIGHTS_ALL        |\
                                  KEY_QUERY_VALUE            |\
                                  KEY_SET_VALUE              |\
                                  KEY_CREATE_SUB_KEY         |\
                                  KEY_ENUMERATE_SUB_KEYS     |\
                                  KEY_NOTIFY                 |\
                                  KEY_CREATE_LINK)            \
                                  &                           \
                                 (~SYNCHRONIZE))

//
// Open/Create Options
//

#define REG_OPTION_RESERVED         (0x00000000L)   // Parameter is reserved

#define REG_OPTION_NON_VOLATILE     (0x00000000L)   // Key is preserved
                                                    // when system is rebooted

#define REG_OPTION_VOLATILE         (0x00000001L)   // Key is not preserved
                                                    // when system is rebooted

#define REG_OPTION_CREATE_LINK      (0x00000002L)   // Created key is a
                                                    // symbolic link

#define REG_OPTION_BACKUP_RESTORE   (0x00000004L)   // open for backup or restore
                                                    // special access rules
                                                    // privilege required

#define REG_OPTION_OPEN_LINK        (0x00000008L)   // Open symbolic link

#define REG_LEGAL_OPTION            \
                (REG_OPTION_RESERVED            |\
                 REG_OPTION_NON_VOLATILE        |\
                 REG_OPTION_VOLATILE            |\
                 REG_OPTION_CREATE_LINK         |\
                 REG_OPTION_BACKUP_RESTORE      |\
                 REG_OPTION_OPEN_LINK)

//
// Key creation/open disposition
//

#define REG_CREATED_NEW_KEY         (0x00000001L)   // New Registry Key created
#define REG_OPENED_EXISTING_KEY     (0x00000002L)   // Existing Key opened

//
// hive format to be used by Reg(Nt)SaveKeyEx
//
#define REG_STANDARD_FORMAT     1
#define REG_LATEST_FORMAT       2
#define REG_NO_COMPRESSION      4

//
// Key restore flags
//

#define REG_WHOLE_HIVE_VOLATILE     (0x00000001L)   // Restore whole hive volatile
#define REG_REFRESH_HIVE            (0x00000002L)   // Unwind changes to last flush
#define REG_NO_LAZY_FLUSH           (0x00000004L)   // Never lazy flush this hive
#define REG_FORCE_RESTORE           (0x00000008L)   // Force the restore process even when we have open handles on subkeys

//
// Unload Flags
//
#define REG_FORCE_UNLOAD            1

// end_ntddk end_wdm end_nthal

//
// Notify filter values
//
#define REG_NOTIFY_CHANGE_NAME          (0x00000001L) // Create or delete (child)
#define REG_NOTIFY_CHANGE_ATTRIBUTES    (0x00000002L)
#define REG_NOTIFY_CHANGE_LAST_SET      (0x00000004L) // time stamp
#define REG_NOTIFY_CHANGE_SECURITY      (0x00000008L)

#define REG_LEGAL_CHANGE_FILTER                 \
                (REG_NOTIFY_CHANGE_NAME          |\
                 REG_NOTIFY_CHANGE_ATTRIBUTES    |\
                 REG_NOTIFY_CHANGE_LAST_SET      |\
                 REG_NOTIFY_CHANGE_SECURITY)

//
// end_winnt
//

// Boot condition flags (for NtInitializeRegistry)

#define REG_INIT_BOOT_SM         (0x0000)    // Init called from SM after Autocheck, etc.
#define REG_INIT_BOOT_SETUP      (0x0001)    // Init called from text-mode setup

//
// Values to indicate that a standard Boot has been accepted by the Service Controller.
// The Boot information will be saved by NtInitializeRegistry to a registry ControlSet with an
//
//       ID = [Given Boot Condition Value] - REG_INIT_BOOT_ACCEPTED_BASE
//

#define REG_INIT_BOOT_ACCEPTED_BASE   (0x0002)
#define REG_INIT_BOOT_ACCEPTED_MAX    REG_INIT_BOOT_ACCEPTED_BASE + 999

#define REG_INIT_MAX_VALID_CONDITION  REG_INIT_BOOT_ACCEPTED_MAX

//
// registry limits for value name and key name
//
#define REG_MAX_KEY_VALUE_NAME_LENGTH   32767       // 32k - sanity limit for value name
#define REG_MAX_KEY_NAME_LENGTH         512         // allow for 256 unicode, as promise


// begin_ntddk begin_wdm begin_nthal
//
// Key query structures
//

typedef struct _KEY_BASIC_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable length string
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_NODE_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable length string
//          Class[1];           // Variable length string not declared
} KEY_NODE_INFORMATION, *PKEY_NODE_INFORMATION;

typedef struct _KEY_FULL_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   ClassOffset;
    ULONG   ClassLength;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   MaxClassLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    WCHAR   Class[1];           // Variable length
} KEY_FULL_INFORMATION, *PKEY_FULL_INFORMATION;

// end_wdm
typedef struct _KEY_NAME_INFORMATION {
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable length string
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef struct _KEY_CACHED_INFORMATION {
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable length string
} KEY_CACHED_INFORMATION, *PKEY_CACHED_INFORMATION;

typedef struct _KEY_FLAGS_INFORMATION {
    ULONG   UserFlags;
} KEY_FLAGS_INFORMATION, *PKEY_FLAGS_INFORMATION;

// begin_wdm
typedef enum _KEY_INFORMATION_CLASS {
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation
// end_wdm
    ,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    MaxKeyInfoClass  // MaxKeyInfoClass should always be the last enum
// begin_wdm
} KEY_INFORMATION_CLASS;

typedef struct _KEY_WRITE_TIME_INFORMATION {
    LARGE_INTEGER LastWriteTime;
} KEY_WRITE_TIME_INFORMATION, *PKEY_WRITE_TIME_INFORMATION;

typedef struct _KEY_USER_FLAGS_INFORMATION {
    ULONG   UserFlags;
} KEY_USER_FLAGS_INFORMATION, *PKEY_USER_FLAGS_INFORMATION;

typedef enum _KEY_SET_INFORMATION_CLASS {
    KeyWriteTimeInformation,
    KeyUserFlagsInformation,
    MaxKeySetInfoClass  // MaxKeySetInfoClass should always be the last enum
} KEY_SET_INFORMATION_CLASS;

//
// Value entry query structures
//

typedef struct _KEY_VALUE_BASIC_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataOffset;
    ULONG   DataLength;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable size
//          Data[1];            // Variable size data not declared
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION {
    ULONG   TitleIndex;
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION_ALIGN64 {
    ULONG   Type;
    ULONG   DataLength;
    UCHAR   Data[1];            // Variable size
} KEY_VALUE_PARTIAL_INFORMATION_ALIGN64, *PKEY_VALUE_PARTIAL_INFORMATION_ALIGN64;

typedef struct _KEY_VALUE_ENTRY {
    PUNICODE_STRING ValueName;
    ULONG           DataLength;
    ULONG           DataOffset;
    ULONG           Type;
} KEY_VALUE_ENTRY, *PKEY_VALUE_ENTRY;

typedef enum _KEY_VALUE_INFORMATION_CLASS {
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation,
    KeyValueFullInformationAlign64,
    KeyValuePartialInformationAlign64,
    MaxKeyValueInfoClass  // MaxKeyValueInfoClass should always be the last enum
} KEY_VALUE_INFORMATION_CLASS;


// end_ntddk end_wdm end_nthal
//
// Notify return structures
//

typedef enum _REG_ACTION {
    KeyAdded,
    KeyRemoved,
    KeyModified
} REG_ACTION;

typedef struct _REG_NOTIFY_INFORMATION {
    ULONG           NextEntryOffset;
    REG_ACTION      Action;
    ULONG           KeyLength;
    WCHAR           Key[1];     // Variable size
} REG_NOTIFY_INFORMATION, *PREG_NOTIFY_INFORMATION;

typedef struct {
    HANDLE          PID;        // PID of the process at the time of open
    UNICODE_STRING  KeyName;    // Full name of the key 
} KEY_PID_ARRAY;

typedef struct _KEY_OPEN_SUBKEYS_INFORMATION {
    ULONG               Count;      // number of elements in the below array
    KEY_PID_ARRAY       KeyArray[1];// variable size array; element count above
} KEY_OPEN_SUBKEYS_INFORMATION, *PKEY_OPEN_SUBKEYS_INFORMATION;

//
// Nt level registry API calls
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateKey(
    __out PHANDLE KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __reserved ULONG TitleIndex,
    __in_opt PUNICODE_STRING Class,
    __in ULONG CreateOptions,
    __out_opt PULONG Disposition
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteKey(
    __in HANDLE KeyHandle
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtDeleteValueKey(
    __in HANDLE KeyHandle,
    __in PUNICODE_STRING ValueName
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateKey(
    __in HANDLE KeyHandle,
    __in ULONG Index,
    __in KEY_INFORMATION_CLASS KeyInformationClass,
    __out_bcount_opt(Length) PVOID KeyInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtEnumerateValueKey(
    __in HANDLE KeyHandle,
    __in ULONG Index,
    __in KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_bcount_opt(Length) PVOID KeyValueInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushKey(
    __in HANDLE KeyHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtInitializeRegistry(
    __in USHORT BootCondition
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtNotifyChangeKey(
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

NTSYSCALLAPI
NTSTATUS
NTAPI
NtNotifyChangeMultipleKeys(
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

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLoadKey(
    __in POBJECT_ATTRIBUTES TargetKey,
    __in POBJECT_ATTRIBUTES SourceFile
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLoadKey2(
    __in POBJECT_ATTRIBUTES   TargetKey,
    __in POBJECT_ATTRIBUTES   SourceFile,
    __in ULONG                Flags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLoadKeyEx(
    __in POBJECT_ATTRIBUTES   TargetKey,
    __in POBJECT_ATTRIBUTES   SourceFile,
    __in ULONG                Flags,
    __in_opt HANDLE           TrustClassKey 
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenKey(
    __out PHANDLE KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryKey(
    __in HANDLE KeyHandle,
    __in KEY_INFORMATION_CLASS KeyInformationClass,
    __out_bcount_opt(Length) PVOID KeyInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryValueKey(
    __in HANDLE KeyHandle,
    __in PUNICODE_STRING ValueName,
    __in KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    __out_bcount_opt(Length) PVOID KeyValueInformation,
    __in ULONG Length,
    __out PULONG ResultLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryMultipleValueKey(
    __in HANDLE KeyHandle,
    __inout_ecount(EntryCount) PKEY_VALUE_ENTRY ValueEntries,
    __in ULONG EntryCount,
    __out_bcount(*BufferLength) PVOID ValueBuffer,
    __inout PULONG BufferLength,
    __out_opt PULONG RequiredBufferLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReplaceKey(
    __in POBJECT_ATTRIBUTES NewFile,
    __in HANDLE             TargetHandle,
    __in POBJECT_ATTRIBUTES OldFile
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRenameKey(
    __in HANDLE           KeyHandle,
    __in PUNICODE_STRING  NewName
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCompactKeys(
    __in ULONG Count,
    __in_ecount(Count) HANDLE KeyArray[]
            );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCompressKey(
    __in HANDLE Key
            );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtRestoreKey(
    __in HANDLE KeyHandle,
    __in HANDLE FileHandle,
    __in ULONG Flags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSaveKey(
    __in HANDLE KeyHandle,
    __in HANDLE FileHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSaveKeyEx(
    __in HANDLE KeyHandle,
    __in HANDLE FileHandle,
    __in ULONG  Format
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSaveMergedKeys(
    __in HANDLE HighPrecedenceKeyHandle,
    __in HANDLE LowPrecedenceKeyHandle,
    __in HANDLE FileHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetValueKey(
    __in HANDLE KeyHandle,
    __in PUNICODE_STRING ValueName,
    __in_opt ULONG TitleIndex,
    __in ULONG Type,
    __in_bcount_opt(DataSize) PVOID Data,
    __in ULONG DataSize
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnloadKey(
    __in POBJECT_ATTRIBUTES TargetKey
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnloadKey2(
    __in POBJECT_ATTRIBUTES   TargetKey,
    __in ULONG                Flags
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnloadKeyEx(
    __in POBJECT_ATTRIBUTES TargetKey,
    __in_opt HANDLE Event
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtSetInformationKey(
    __in HANDLE KeyHandle,
    __in KEY_SET_INFORMATION_CLASS KeySetInformationClass,
    __in_bcount(KeySetInformationLength) PVOID KeySetInformation,
    __in ULONG KeySetInformationLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryOpenSubKeys(
    __in POBJECT_ATTRIBUTES TargetKey,
    __out PULONG  HandleCount
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryOpenSubKeysEx(
    __in POBJECT_ATTRIBUTES   TargetKey,
    __in ULONG                BufferLength,
    __out_bcount(BufferLength) PVOID               Buffer,
    __out PULONG              RequiredSize
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLockRegistryKey(
    __in HANDLE           KeyHandle
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLockProductActivationKeys(
    __inout_opt ULONG   *pPrivateVer,
    __out_opt ULONG   *pSafeMode
    );

#ifdef __cplusplus
}
#endif

#endif // _NTREGAPI_

