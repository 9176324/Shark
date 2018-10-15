/*++ BUILD Version: 0004    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntldr.h

Abstract:

    This module implements the public interfaces of the Loader (Ldr)
    subsystem. Ldr is coupled with the session manager. It is not
    a separate process.

--*/

#ifndef _NTLDRAPI_
#define _NTLDRAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Private flags for loader data table entries
//

#define LDRP_STATIC_LINK                0x00000002
#define LDRP_IMAGE_DLL                  0x00000004
#define LDRP_LOAD_IN_PROGRESS           0x00001000
#define LDRP_UNLOAD_IN_PROGRESS         0x00002000
#define LDRP_ENTRY_PROCESSED            0x00004000
#define LDRP_ENTRY_INSERTED             0x00008000
#define LDRP_CURRENT_LOAD               0x00010000
#define LDRP_FAILED_BUILTIN_LOAD        0x00020000
#define LDRP_DONT_CALL_FOR_THREADS      0x00040000
#define LDRP_PROCESS_ATTACH_CALLED      0x00080000
#define LDRP_DEBUG_SYMBOLS_LOADED       0x00100000
#define LDRP_IMAGE_NOT_AT_BASE          0x00200000
#define LDRP_COR_IMAGE                  0x00400000
#define LDRP_COR_OWNS_UNMAP             0x00800000
#define LDRP_SYSTEM_MAPPED              0x01000000
#define LDRP_IMAGE_VERIFYING            0x02000000
#define LDRP_DRIVER_DEPENDENT_DLL       0x04000000
#define LDRP_ENTRY_NATIVE               0x08000000
#define LDRP_REDIRECTED                 0x10000000
#define LDRP_NON_PAGED_DEBUG_INFO       0x20000000
#define LDRP_MM_LOADED                  0x40000000
#define LDRP_COMPAT_DATABASE_PROCESSED  0x80000000

//
// Loader Data Table. Used to track DLLs loaded into an
// image.
//

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY HashLinks;
        struct {
            PVOID SectionPointer;
            ULONG CheckSum;
        };
    };
    union {
        struct {
            ULONG TimeDateStamp;
        };
        struct {
            PVOID LoadedImports;
        };
    };
    struct _ACTIVATION_CONTEXT * EntryPointActivationContext;

    PVOID PatchInformation;

} LDR_DATA_TABLE_ENTRY, *PLDR_DATA_TABLE_ENTRY;

typedef const struct _LDR_DATA_TABLE_ENTRY *PCLDR_DATA_TABLE_ENTRY;

typedef struct _KLDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    PVOID ExceptionTable;
    ULONG ExceptionTableSize;
    // ULONG padding on IA64
    PVOID GpValue;
    PNON_PAGED_DEBUG_INFO NonPagedDebugInfo;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT __Unused5;
    PVOID SectionPointer;
    ULONG CheckSum;
    // ULONG padding on IA64
    PVOID LoadedImports;
    PVOID PatchInformation;
} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;

typedef struct _LDR_DATA_TABLE_ENTRY32 {
    LIST_ENTRY32 InLoadOrderLinks;
    LIST_ENTRY32 InMemoryOrderLinks;
    LIST_ENTRY32 InInitializationOrderLinks;
    ULONG DllBase;
    ULONG EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING32 FullDllName;
    UNICODE_STRING32 BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY32 HashLinks;
        struct {
            ULONG SectionPointer;
            ULONG  CheckSum;
        };
    };
    union {
        struct {
            ULONG  TimeDateStamp;
        };
        struct {
            ULONG LoadedImports;
        };
    };

    //
    // NOTE : Do not grow this structure at the dump files used a packed
    // array of these structures.
    //

} LDR_DATA_TABLE_ENTRY32, *PLDR_DATA_TABLE_ENTRY32;

typedef struct _LDR_DATA_TABLE_ENTRY64 {
    LIST_ENTRY64 InLoadOrderLinks;
    LIST_ENTRY64 InMemoryOrderLinks;
    LIST_ENTRY64 InInitializationOrderLinks;
    ULONG64 DllBase;
    ULONG64 EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING64 FullDllName;
    UNICODE_STRING64 BaseDllName;
    ULONG   Flags;
    USHORT  LoadCount;
    USHORT TlsIndex;
    union {
        LIST_ENTRY64 HashLinks;
        struct {
            ULONG64 SectionPointer;
    ULONG   CheckSum;
        };
    };
    union {
        struct {
            ULONG   TimeDateStamp;
        };
        struct {
            ULONG64 LoadedImports;
        };
    };

    //
    // NOTE : Do not grow this structure at the dump files used a packed
    // array of these structures.
    //

} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;

typedef struct _KLDR_DATA_TABLE_ENTRY32 {
    LIST_ENTRY32 InLoadOrderLinks;
    ULONG __Undefined1;
    ULONG __Undefined2;
    ULONG __Undefined3;
    ULONG NonPagedDebugInfo;
    ULONG DllBase;
    ULONG EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING32 FullDllName;
    UNICODE_STRING32 BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT __Undefined5;
    ULONG  __Undefined6;
    ULONG  CheckSum;
    ULONG  TimeDateStamp;

    //
    // NOTE : Do not grow this structure at the dump files used a packed
    // array of these structures.
    //

} KLDR_DATA_TABLE_ENTRY32, *PKLDR_DATA_TABLE_ENTRY32;

typedef struct _KLDR_DATA_TABLE_ENTRY64 {
    LIST_ENTRY64 InLoadOrderLinks;
    ULONG64 __Undefined1;
    ULONG64 __Undefined2;
    ULONG64 __Undefined3;
    ULONG64 NonPagedDebugInfo;
    ULONG64 DllBase;
    ULONG64 EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING64 FullDllName;
    UNICODE_STRING64 BaseDllName;
    ULONG   Flags;
    USHORT  LoadCount;
    USHORT  __Undefined5;
    ULONG64 __Undefined6;
    ULONG   CheckSum;
    ULONG   __padding1;
    ULONG   TimeDateStamp;
    ULONG   __padding2;

    //
    // NOTE : Do not grow this structure at the dump files used a packed
    // array of these structures.
    //

} KLDR_DATA_TABLE_ENTRY64, *PKLDR_DATA_TABLE_ENTRY64;

#define DLL_PROCESS_ATTACH   1    // winnt
#define DLL_THREAD_ATTACH    2    // winnt
#define DLL_THREAD_DETACH    3    // winnt
#define DLL_PROCESS_DETACH   0    // winnt
#define DLL_PROCESS_VERIFIER 4    // winnt

typedef
BOOLEAN
(*PDLL_INIT_ROUTINE) (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    );

typedef
BOOLEAN
(*PPROCESS_STARTER_ROUTINE) (
    IN PVOID RealStartAddress
    );

NTSYSAPI
VOID
LdrProcessStarterHelper(
    IN PPROCESS_STARTER_ROUTINE ProcessStarter,
    IN PVOID RealStartAddress
    );

NTSYSAPI
VOID
NTAPI
LdrShutdownProcess(
    VOID
    );

NTSYSAPI
VOID
NTAPI
LdrShutdownThread(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrLoadDll(
    IN PCWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrGetDllHandle(
    IN PCWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrUnloadDll(
    IN PVOID DllHandle
    );

typedef
NTSTATUS
(NTAPI * PLDR_MANIFEST_PROBER_ROUTINE) (
    IN PVOID DllBase,
    IN PCWSTR FullDllPath,
    OUT PVOID *ActivationContext
    );

NTSYSAPI
VOID
NTAPI
LdrSetDllManifestProber(
    IN PLDR_MANIFEST_PROBER_ROUTINE ManifestProberRoutine
    );

#if defined(BLDR_KERNEL_RUNTIME)

typedef ULONG ARC_STATUS;
typedef ARC_STATUS LDR_RELOCATE_IMAGE_RETURN_TYPE;

#else

typedef NTSTATUS LDR_RELOCATE_IMAGE_RETURN_TYPE;

#endif

LDR_RELOCATE_IMAGE_RETURN_TYPE
LdrRelocateImage (
    __in PVOID NewBase,
    __in PCSTR LoaderName,
    __in LDR_RELOCATE_IMAGE_RETURN_TYPE Success,
    __in LDR_RELOCATE_IMAGE_RETURN_TYPE Conflict,
    __in LDR_RELOCATE_IMAGE_RETURN_TYPE Invalid
    );

LDR_RELOCATE_IMAGE_RETURN_TYPE
LdrRelocateImageWithBias (
    __in PVOID NewBase,
    __in LONGLONG Bias,
    __in PCSTR LoaderName,
    __in LDR_RELOCATE_IMAGE_RETURN_TYPE Success,
    __in LDR_RELOCATE_IMAGE_RETURN_TYPE Conflict,
    __in LDR_RELOCATE_IMAGE_RETURN_TYPE Invalid
    );

PIMAGE_BASE_RELOCATION
NTAPI
LdrProcessRelocationBlock(
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONG_PTR Diff
    );

BOOLEAN
NTAPI
LdrVerifyMappedImageMatchesChecksum (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN ULONG FileLength
    );

typedef
VOID
(*PLDR_IMPORT_MODULE_CALLBACK)(
    IN PVOID Parameter,
    PCHAR ModuleName
    );

NTSTATUS
NTAPI
LdrVerifyImageMatchesChecksum (
    IN HANDLE ImageFileHandle,
    IN PLDR_IMPORT_MODULE_CALLBACK ImportCallbackRoutine OPTIONAL,
    IN PVOID ImportCallbackParameter,
    OUT PUSHORT ImageCharacteristics OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrGetProcedureAddress(
    IN PVOID DllHandle,
    IN CONST ANSI_STRING* ProcedureName OPTIONAL,
    IN ULONG ProcedureNumber OPTIONAL,
    OUT PVOID *ProcedureAddress
    );

#define LDR_RESOURCE_ID_NAME_MASK   ((~(ULONG_PTR)0) << 16) /* lower 16bits clear */
#define LDR_RESOURCE_ID_NAME_MINVAL (( (ULONG_PTR)1) << 16) /* 17th bit set */

//
// These are how you currently pass the flag to FindResource.
//
// VIEW_TO_DATAFILE and DATAFILE_TO_VIEW are idempotent,
// so you can covert a datafile to a datafile with VIEW_TO_DATAFILE.
// Think of better names therefore..
//
#define LDR_VIEW_TO_DATAFILE(x) ((PVOID)(((ULONG_PTR)(x)) |  (ULONG_PTR)1))
#define LDR_IS_DATAFILE(x)              (((ULONG_PTR)(x)) &  (ULONG_PTR)1)
#define LDR_IS_VIEW(x)                  (!LDR_IS_DATAFILE(x))
#define LDR_DATAFILE_TO_VIEW(x) ((PVOID)(((ULONG_PTR)(x)) & ~(ULONG_PTR)1))

//
// These functions work on ULONG, ULONG_PTR, ULONG64, etc.
// They do not work on pointers.
//
#define LDR_VIEW_TO_DATAFILE_INTEGER(x) ((x) | 1)
#define LDR_IS_DATAFILE_INTEGER(x)      (((x) & 1) == 1)
#define LDR_IS_VIEW_INTEGER(x)          (((x) & 1) == 0)
#define LDR_DATAFILE_TO_VIEW_INTEGER(x) ((x) - ((x) & 1))

//
// Flags to LdrCreateOutOfProcessImage.
//
// These first two values must not share any bits, even though this is an enum,
// because LDR_DLL_MAPPED_AS_UNREFORMATED_IMAGE is actually changed to one of them
// and then it is treated as bits.
#define LDR_DLL_MAPPED_AS_IMAGE            (0x00000001)
#define LDR_DLL_MAPPED_AS_DATA             (0x00000002)
#define LDR_DLL_MAPPED_AS_UNFORMATED_IMAGE (0x00000003)
#define LDR_DLL_MAPPED_AS_MASK             (0x00000003)

//
// These are flags to a function that doesn't yet exist:
//    LdrpSearchResourceSectionEx and/or LdrpSearchOutOfProcessResourceSection
//
#define LDRP_FIND_RESOURCE_DATA                 (0x00000000)
#define LDRP_FIND_RESOURCE_DIRECTORY            (0x00000002)

//
// Flags to LdrFindResourceEx/LdrpSearchResourceSection/LdrFindOutOfProcessResource.
//
#define LDR_FIND_RESOURCE_LANGUAGE_CAN_FALLBACK            (0x00000000)
#define LDR_FIND_RESOURCE_LANGUAGE_EXACT                   (0x00000004)
#define LDR_FIND_RESOURCE_LANGUAGE_REDIRECT_VERSION        (0x00000008)

NTSYSAPI
NTSTATUS
NTAPI
LdrFindResourceDirectory_U(
    IN PVOID DllHandle,
    IN CONST ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DIRECTORY *ResourceDirectory
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrFindResource_U(
    IN PVOID DllHandle,
    IN CONST ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    );

NTSTATUS
NTAPI
LdrFindResourceEx_U(
    ULONG Flags,
    IN PVOID DllHandle,
    IN CONST ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY *ResourceDataEntry
    );


#ifndef NTOS_KERNEL_RUNTIME

#if !defined(RTL_BUFFER)
// This is duplicated in nturtl.h.

#define RTL_BUFFER RTL_BUFFER

typedef struct _RTL_BUFFER {
    PUCHAR    Buffer;
    PUCHAR    StaticBuffer;
    SIZE_T    Size;
    SIZE_T    StaticSize;
    SIZE_T    ReservedForAllocatedSize; // for future doubling
    PVOID     ReservedForIMalloc; // for future pluggable growth
} RTL_BUFFER, *PRTL_BUFFER;

#endif

//
// This will always contain the MS-DOS header,
// File header, Optional header, and Section headers.
//
// If multiple fast resource loads are needed, we could
// populate it on demand with resource directory stuff too.
//
// This struct is opaque.
// Only access it with Ldr*OutOfProcess* functions.
//
typedef struct _LDR_OUT_OF_PROCESS_IMAGE {
    RTL_BUFFER  HeadersBuffer;
    HANDLE      ProcessHandle;
    ULONG64     DllHandle; // base of mapped section, not kernel handle
    ULONG       Flags;
} LDR_OUT_OF_PROCESS_IMAGE, *PLDR_OUT_OF_PROCESS_IMAGE;

NTSTATUS
NTAPI
LdrCreateOutOfProcessImage(
    IN ULONG                      Flags,
    IN HANDLE                     ProcessHandle,
    IN ULONG64                    DllHandle, // base of mapped section, not kernel handle
    OUT PLDR_OUT_OF_PROCESS_IMAGE Image
    );

//
// - You may destroy an out of process image that is all zeros.
// - You may destroy an out of process image repeatedly.
//

NTSYSAPI
VOID
NTAPI
LdrDestroyOutOfProcessImage(
    IN OUT PLDR_OUT_OF_PROCESS_IMAGE Image
    );

NTSTATUS
NTAPI
LdrFindCreateProcessManifest(
    IN ULONG                         Flags,
    PLDR_OUT_OF_PROCESS_IMAGE        Image,
    IN CONST ULONG_PTR*              ResourceIdPath,
    IN ULONG                         ResourceIdPathLength,
    OUT PIMAGE_RESOURCE_DATA_ENTRY   ResourceDataEntry
    );

NTSTATUS
NTAPI
LdrAccessOutOfProcessResource(
    IN ULONG                            Flags,
    PLDR_OUT_OF_PROCESS_IMAGE           Image,
    IN CONST IMAGE_RESOURCE_DATA_ENTRY* DataEntry,
    OUT PULONG64                        Address OPTIONAL,
    OUT PULONG                          Size OPTIONAL
    );

#endif

// type, id/name, langid
#define LDR_MAXIMUM_RESOURCE_PATH_DEPTH (3)

typedef struct _LDR_ENUM_RESOURCE_ENTRY {
    union {
        ULONG_PTR NameOrId;
        PIMAGE_RESOURCE_DIRECTORY_STRING Name;
        struct {
            USHORT Id;
            USHORT NameIsPresent;
        };
    } Path[ LDR_MAXIMUM_RESOURCE_PATH_DEPTH ];
    PVOID Data;
    ULONG Size;
    ULONG Reserved;
} LDR_ENUM_RESOURCE_ENTRY, *PLDR_ENUM_RESOURCE_ENTRY;

NTSYSAPI
NTSTATUS
NTAPI
LdrEnumResources(
    IN PVOID DllHandle,
    IN CONST ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    IN OUT PULONG NumberOfResources,
    OUT PLDR_ENUM_RESOURCE_ENTRY Resources OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrAccessResource(
    IN PVOID DllHandle,
    IN CONST IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    );

NTSTATUS
NTAPI
LdrFindEntryForAddress(
    IN PVOID Address,
    OUT PLDR_DATA_TABLE_ENTRY *TableEntry
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrDisableThreadCalloutsForDll (
    IN PVOID DllHandle
    );


typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;                 // Not filled in
    PVOID MappedBase;
    PVOID ImageBase;
    ULONG ImageSize;
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;
    USHORT OffsetToFileName;
    UCHAR  FullPathName[ 256 ];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;
    RTL_PROCESS_MODULE_INFORMATION Modules[ 1 ];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;

NTSTATUS
NTAPI
LdrQueryProcessModuleInformation(
    OUT PRTL_PROCESS_MODULES ModuleInformation,
    IN ULONG ModuleInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

#define LDR_IMAGE_FILE_OPTIONS_KEY_LARGE_PAGE_DLLS L"LargePageDLLs"

NTSYSAPI
NTSTATUS
NTAPI
LdrOpenImageFileOptionsKey (
    IN PCUNICODE_STRING ImagePathName,
    IN BOOLEAN Wow64Path,
    OUT PHANDLE KeyHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrQueryImageFileExecutionOptions(
    IN PCUNICODE_STRING ImagePathName,
    IN PCWSTR OptionName,
    IN ULONG Type,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG ResultSize OPTIONAL
    );

NTSYSAPI
NTSTATUS
LdrQueryImageFileExecutionOptionsEx(
    IN PCUNICODE_STRING ImagePathName,
    IN PCWSTR OptionName,
    IN ULONG Type,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG ResultSize OPTIONAL,
    IN BOOLEAN Wow64Path
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrQueryImageFileKeyOption (
    IN HANDLE KeyHandle,
    IN PCWSTR OptionName,
    IN ULONG Type,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG ResultSize OPTIONAL
    );

NTSYSAPI
BOOLEAN
NTAPI
LdrAlternateResourcesEnabled(
    VOID
    );

PVOID
NTAPI
LdrGetAlternateResourceModuleHandle(
    IN PVOID Module
    );

NTSYSAPI
PVOID
NTAPI
LdrLoadAlternateResourceModule(
    IN PVOID Module,
    IN LPCWSTR PathToAlternateModule OPTIONAL
    );

NTSYSAPI
BOOLEAN
NTAPI
LdrUnloadAlternateResourceModule(
    IN PVOID Module
    );

NTSYSAPI
BOOLEAN
NTAPI
LdrFlushAlternateResourceModules(
    VOID
    );

#define LDR_DLL_LOADED_FLAG_RELOCATED (0x00000001)

typedef struct _LDR_DLL_LOADED_NOTIFICATION_DATA {
    ULONG Flags;
    PCUNICODE_STRING FullDllName;
    PCUNICODE_STRING BaseDllName;
    PVOID DllBase;
    ULONG SizeOfImage;
} LDR_DLL_LOADED_NOTIFICATION_DATA, *PLDR_DLL_LOADED_NOTIFICATION_DATA;

typedef const LDR_DLL_LOADED_NOTIFICATION_DATA *PCLDR_DLL_LOADED_NOTIFICATION_DATA;

#define LDR_DLL_UNLOADED_FLAG_PROCESS_TERMINATION (0x00000001)

typedef struct _LDR_DLL_UNLOADED_NOTIFICATION_DATA {
    ULONG Flags;
    PCUNICODE_STRING FullDllName;
    PCUNICODE_STRING BaseDllName;
    PVOID DllBase;
    ULONG SizeOfImage;
} LDR_DLL_UNLOADED_NOTIFICATION_DATA, *PLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef const LDR_DLL_UNLOADED_NOTIFICATION_DATA *PCLDR_DLL_UNLOADED_NOTIFICATION_DATA;

typedef union _LDR_DLL_NOTIFICATION_DATA {
    LDR_DLL_LOADED_NOTIFICATION_DATA Loaded;
    LDR_DLL_UNLOADED_NOTIFICATION_DATA Unloaded;
} LDR_DLL_NOTIFICATION_DATA, *PLDR_DLL_NOTIFICATION_DATA;

typedef const LDR_DLL_NOTIFICATION_DATA *PCLDR_DLL_NOTIFICATION_DATA;

#define LDR_DLL_NOTIFICATION_REASON_LOADED (1)
#define LDR_DLL_NOTIFICATION_REASON_UNLOADED (2)

typedef
VOID (NTAPI *PLDR_DLL_NOTIFICATION_FUNCTION)(
    IN ULONG NotificationReason,
    IN PCLDR_DLL_NOTIFICATION_DATA NotificationData,
    IN PVOID Context
    );

NTSTATUS
NTAPI
LdrRegisterDllNotification(
    IN ULONG Flags,
    IN PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
    IN PVOID Context,
    OUT PVOID *Cookie
    );

NTSTATUS
NTAPI
LdrUnregisterDllNotification(
    IN PVOID Cookie
    );

typedef
VOID (NTAPI *PLDR_LOADED_MODULE_ENUMERATION_CALLBACK_FUNCTION)(
    IN PCLDR_DATA_TABLE_ENTRY DataTableEntry,
    IN PVOID Context,
    IN OUT BOOLEAN *StopEnumeration
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrEnumerateLoadedModules(
    IN ULONG Flags,
    IN PLDR_LOADED_MODULE_ENUMERATION_CALLBACK_FUNCTION CallbackFunction,
    IN PVOID Context
    );

#define LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS (0x00000001)
#define LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY (0x00000002)

#define LDR_LOCK_LOADER_LOCK_DISPOSITION_INVALID (0)
#define LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED (1)
#define LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_NOT_ACQUIRED (2)

NTSYSAPI
NTSTATUS
NTAPI
LdrLockLoaderLock(
    IN ULONG Flags,
    OUT ULONG *Disposition OPTIONAL, // not optional if LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY is set
    OUT PVOID *Cookie
    );

#define LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS (0x00000001)

NTSYSAPI
NTSTATUS
NTAPI
LdrUnlockLoaderLock(
    IN ULONG Flags,
    IN OUT PVOID Cookie
    );

#define LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT (0x00000001)
#define LDR_GET_DLL_HANDLE_EX_PIN                (0x00000002)

NTSYSAPI
NTSTATUS
NTAPI
LdrGetDllHandleEx(
    IN ULONG Flags,
    IN PCWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING DllName,
    OUT PVOID *DllHandle OPTIONAL
    );

#define LDR_ADDREF_DLL_PIN (0x00000001)

NTSYSAPI
NTSTATUS
NTAPI
LdrAddRefDll(
    ULONG               Flags,
    PVOID               DllHandle
    );

typedef
NTSTATUS (NTAPI *PLDR_APP_COMPAT_DLL_REDIRECTION_CALLBACK_FUNCTION)(
    IN ULONG Flags,
    IN PCWSTR DllName,
    IN PCWSTR DllPath OPTIONAL,
    IN OUT PULONG DllCharacteristics OPTIONAL,
    IN PVOID CallbackData,
    OUT PWSTR *EffectiveDllPath
    );

NTSYSAPI
NTSTATUS
NTAPI
LdrSetAppCompatDllRedirectionCallback(
    IN ULONG Flags,
    IN PLDR_APP_COMPAT_DLL_REDIRECTION_CALLBACK_FUNCTION CallbackFunction,
    IN PVOID CallbackData
    );

#ifdef __cplusplus
}
#endif

#endif // _NTLDRAPI_

