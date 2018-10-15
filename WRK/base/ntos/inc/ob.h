/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ob.h

Abstract:

    This module contains the object manager structure public data
    structures and procedure prototypes to be used within the NT
    system.

--*/

#ifndef _OB_
#define _OB_

//
// System Initialization procedure for OB subcomponent of NTOS
//
BOOLEAN
ObInitSystem( VOID );

VOID
ObShutdownSystem(
    IN ULONG Phase
    );

NTSTATUS
ObInitProcess(
    PEPROCESS ParentProcess OPTIONAL,
    PEPROCESS NewProcess
    );

VOID
ObInitProcess2(
    PEPROCESS NewProcess
    );

VOID
ObKillProcess(
    PEPROCESS Process
    );

VOID
ObClearProcessHandleTable (
    PEPROCESS Process
    );

VOID
ObDereferenceProcessHandleTable (
    PEPROCESS SourceProcess
    );

PHANDLE_TABLE
ObReferenceProcessHandleTable (
    PEPROCESS SourceProcess
    );

ULONG
ObGetProcessHandleCount (
    PEPROCESS Process
    );


NTSTATUS
ObDuplicateObject (
    IN PEPROCESS SourceProcess,
    IN HANDLE SourceHandle,
    IN PEPROCESS TargetProcess OPTIONAL,
    OUT PHANDLE TargetHandle OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG HandleAttributes,
    IN ULONG Options,
    IN KPROCESSOR_MODE PreviousMode
    );

// begin_ntddk begin_wdm begin_nthal begin_ntosp begin_ntifs
//
// Object Manager types
//

typedef struct _OBJECT_HANDLE_INFORMATION {
    ULONG HandleAttributes;
    ACCESS_MASK GrantedAccess;
} OBJECT_HANDLE_INFORMATION, *POBJECT_HANDLE_INFORMATION;

// end_ntddk end_wdm end_nthal end_ntifs

typedef struct _OBJECT_DUMP_CONTROL {
    PVOID Stream;
    ULONG Detail;
} OB_DUMP_CONTROL, *POB_DUMP_CONTROL;

typedef VOID (*OB_DUMP_METHOD)(
    IN PVOID Object,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );

typedef enum _OB_OPEN_REASON {
    ObCreateHandle,
    ObOpenHandle,
    ObDuplicateHandle,
    ObInheritHandle,
    ObMaxOpenReason
} OB_OPEN_REASON;


typedef NTSTATUS (*OB_OPEN_METHOD)(
    IN OB_OPEN_REASON OpenReason,
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG HandleCount
    );

typedef BOOLEAN (*OB_OKAYTOCLOSE_METHOD)(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN HANDLE Handle,
    IN KPROCESSOR_MODE PreviousMode
    );

typedef VOID (*OB_CLOSE_METHOD)(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    );

typedef VOID (*OB_DELETE_METHOD)(
    IN  PVOID   Object
    );

typedef NTSTATUS (*OB_PARSE_METHOD)(
    IN PVOID ParseObject,
    IN PVOID ObjectType,
    IN OUT PACCESS_STATE AccessState,
    IN KPROCESSOR_MODE AccessMode,
    IN ULONG Attributes,
    IN OUT PUNICODE_STRING CompleteName,
    IN OUT PUNICODE_STRING RemainingName,
    IN OUT PVOID Context OPTIONAL,
    IN PSECURITY_QUALITY_OF_SERVICE SecurityQos OPTIONAL,
    OUT PVOID *Object
    );

typedef NTSTATUS (*OB_SECURITY_METHOD)(
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    );

typedef NTSTATUS (*OB_QUERYNAME_METHOD)(
    IN PVOID Object,
    IN BOOLEAN HasObjectName,
    OUT POBJECT_NAME_INFORMATION ObjectNameInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength,
    IN KPROCESSOR_MODE Mode
    );

/*

    A security method and its caller must obey the following w.r.t.
    capturing and probing parameters:

    For a query operation, the caller must pass a kernel space address for
    CapturedLength.  The caller should be able to assume that it points to
    valid data that will not change.  In addition, the SecurityDescriptor
    parameter (which will receive the result of the query operation) must
    be probed for write up to the length given in CapturedLength.  The
    security method itself must always write to the SecurityDescriptor
    buffer in a try clause in case the caller de-allocates it.

    For a set operation, the SecurityDescriptor parameter must have
    been captured via SeCaptureSecurityDescriptor.  This parameter is
    not optional, and therefore may not be NULL.

*/



//
// Prototypes for Win32 WindowStation and Desktop object callout routines
//
typedef struct _WIN32_OPENMETHOD_PARAMETERS {
   OB_OPEN_REASON OpenReason;
   PEPROCESS Process;
   PVOID Object;
   ACCESS_MASK GrantedAccess;
   ULONG HandleCount;
} WIN32_OPENMETHOD_PARAMETERS, *PKWIN32_OPENMETHOD_PARAMETERS;

typedef
NTSTATUS
(*PKWIN32_OPENMETHOD_CALLOUT) ( PKWIN32_OPENMETHOD_PARAMETERS );
extern PKWIN32_OPENMETHOD_CALLOUT ExDesktopOpenProcedureCallout;
extern PKWIN32_OPENMETHOD_CALLOUT ExWindowStationOpenProcedureCallout;



typedef struct _WIN32_OKAYTOCLOSEMETHOD_PARAMETERS {
   PEPROCESS Process;
   PVOID Object;
   HANDLE Handle;
   KPROCESSOR_MODE PreviousMode;
} WIN32_OKAYTOCLOSEMETHOD_PARAMETERS, *PKWIN32_OKAYTOCLOSEMETHOD_PARAMETERS;

typedef
NTSTATUS
(*PKWIN32_OKTOCLOSEMETHOD_CALLOUT) ( PKWIN32_OKAYTOCLOSEMETHOD_PARAMETERS );
extern PKWIN32_OKTOCLOSEMETHOD_CALLOUT ExDesktopOkToCloseProcedureCallout;
extern PKWIN32_OKTOCLOSEMETHOD_CALLOUT ExWindowStationOkToCloseProcedureCallout;



typedef struct _WIN32_CLOSEMETHOD_PARAMETERS {
   PEPROCESS Process;
   PVOID Object;
   ACCESS_MASK GrantedAccess;
   ULONG ProcessHandleCount;
   ULONG SystemHandleCount;
} WIN32_CLOSEMETHOD_PARAMETERS, *PKWIN32_CLOSEMETHOD_PARAMETERS;
typedef
NTSTATUS
(*PKWIN32_CLOSEMETHOD_CALLOUT) ( PKWIN32_CLOSEMETHOD_PARAMETERS );
extern PKWIN32_CLOSEMETHOD_CALLOUT ExDesktopCloseProcedureCallout;
extern PKWIN32_CLOSEMETHOD_CALLOUT ExWindowStationCloseProcedureCallout;



typedef struct _WIN32_DELETEMETHOD_PARAMETERS {
   PVOID Object;
} WIN32_DELETEMETHOD_PARAMETERS, *PKWIN32_DELETEMETHOD_PARAMETERS;
typedef
NTSTATUS
(*PKWIN32_DELETEMETHOD_CALLOUT) ( PKWIN32_DELETEMETHOD_PARAMETERS );
extern PKWIN32_DELETEMETHOD_CALLOUT ExDesktopDeleteProcedureCallout;
extern PKWIN32_DELETEMETHOD_CALLOUT ExWindowStationDeleteProcedureCallout;


typedef struct _WIN32_PARSEMETHOD_PARAMETERS {
   PVOID ParseObject;
   PVOID ObjectType;
   PACCESS_STATE AccessState;
   KPROCESSOR_MODE AccessMode;
   ULONG Attributes;
   PUNICODE_STRING CompleteName;
   PUNICODE_STRING RemainingName;
   PVOID Context OPTIONAL;
   PSECURITY_QUALITY_OF_SERVICE SecurityQos;
   PVOID *Object;
} WIN32_PARSEMETHOD_PARAMETERS, *PKWIN32_PARSEMETHOD_PARAMETERS;
typedef
NTSTATUS
(*PKWIN32_PARSEMETHOD_CALLOUT) ( PKWIN32_PARSEMETHOD_PARAMETERS );
extern PKWIN32_PARSEMETHOD_CALLOUT ExWindowStationParseProcedureCallout;


//
// Object Type Structure
//

typedef struct _OBJECT_TYPE_INITIALIZER {
    USHORT Length;
    BOOLEAN UseDefaultObject;
    BOOLEAN CaseInsensitive;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    BOOLEAN MaintainTypeList;
    POOL_TYPE PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
    OB_DUMP_METHOD DumpProcedure;
    OB_OPEN_METHOD OpenProcedure;
    OB_CLOSE_METHOD CloseProcedure;
    OB_DELETE_METHOD DeleteProcedure;
    OB_PARSE_METHOD ParseProcedure;
    OB_SECURITY_METHOD SecurityProcedure;
    OB_QUERYNAME_METHOD QueryNameProcedure;
    OB_OKAYTOCLOSE_METHOD OkayToCloseProcedure;
} OBJECT_TYPE_INITIALIZER, *POBJECT_TYPE_INITIALIZER;

#define OBJECT_LOCK_COUNT 4

typedef struct _OBJECT_TYPE {
    ERESOURCE Mutex;
    LIST_ENTRY TypeList;
    UNICODE_STRING Name;            // Copy from object header for convenience
    PVOID DefaultObject;
    ULONG Index;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    OBJECT_TYPE_INITIALIZER TypeInfo;
#ifdef POOL_TAGGING
    ULONG Key;
#endif //POOL_TAGGING
    ERESOURCE ObjectLocks[ OBJECT_LOCK_COUNT ];
} OBJECT_TYPE, *POBJECT_TYPE;

//
// Object Directory Structure
//

#define NUMBER_HASH_BUCKETS 37
#define OBJ_INVALID_SESSION_ID 0xFFFFFFFF

typedef struct _OBJECT_DIRECTORY {
    struct _OBJECT_DIRECTORY_ENTRY *HashBuckets[ NUMBER_HASH_BUCKETS ];
    EX_PUSH_LOCK Lock;
    struct _DEVICE_MAP *DeviceMap;
    ULONG SessionId;
} OBJECT_DIRECTORY, *POBJECT_DIRECTORY;
// end_ntosp

//
// Object Directory Entry Structure
//
typedef struct _OBJECT_DIRECTORY_ENTRY {
    struct _OBJECT_DIRECTORY_ENTRY *ChainLink;
    PVOID Object;
    ULONG HashValue;
} OBJECT_DIRECTORY_ENTRY, *POBJECT_DIRECTORY_ENTRY;


//
// Symbolic Link Object Structure
//

typedef struct _OBJECT_SYMBOLIC_LINK {
    LARGE_INTEGER CreationTime;
    UNICODE_STRING LinkTarget;
    UNICODE_STRING LinkTargetRemaining;
    PVOID LinkTargetObject;
    ULONG DosDeviceDriveIndex;  // 1-based index into KUSER_SHARED_DATA.DosDeviceDriveType
} OBJECT_SYMBOLIC_LINK, *POBJECT_SYMBOLIC_LINK;


//
// Device Map Structure
//

typedef struct _DEVICE_MAP {
    POBJECT_DIRECTORY DosDevicesDirectory;
    POBJECT_DIRECTORY GlobalDosDevicesDirectory;
    ULONG ReferenceCount;
    ULONG DriveMap;
    UCHAR DriveType[ 32 ];
} DEVICE_MAP, *PDEVICE_MAP;

extern PDEVICE_MAP ObSystemDeviceMap;

//
// Object Handle Count Database
//

typedef struct _OBJECT_HANDLE_COUNT_ENTRY {
    PEPROCESS Process;
    ULONG HandleCount;
} OBJECT_HANDLE_COUNT_ENTRY, *POBJECT_HANDLE_COUNT_ENTRY;

typedef struct _OBJECT_HANDLE_COUNT_DATABASE {
    ULONG CountEntries;
    OBJECT_HANDLE_COUNT_ENTRY HandleCountEntries[ 1 ];
} OBJECT_HANDLE_COUNT_DATABASE, *POBJECT_HANDLE_COUNT_DATABASE;

//
// Object Header Structure
//
// The SecurityQuotaCharged is the amount of quota charged to cover
// the GROUP and DISCRETIONARY ACL fields of the security descriptor
// only.  The OWNER and SYSTEM ACL fields get charged for at a fixed
// rate that may be less than or greater than the amount actually used.
//
// If the object has no security, then the SecurityQuotaCharged and the
// SecurityQuotaInUse fields are set to zero.
//
// Modification of the OWNER and SYSTEM ACL fields should never fail
// due to quota exceeded problems.  Modifications to the GROUP and
// DISCRETIONARY ACL fields may fail due to quota exceeded problems.
//
//


typedef struct _OBJECT_CREATE_INFORMATION {
    ULONG Attributes;
    HANDLE RootDirectory;
    PVOID ParseContext;
    KPROCESSOR_MODE ProbeMode;
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    ULONG SecurityDescriptorCharge;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PSECURITY_QUALITY_OF_SERVICE SecurityQos;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
} OBJECT_CREATE_INFORMATION;

// begin_ntosp
typedef struct _OBJECT_CREATE_INFORMATION *POBJECT_CREATE_INFORMATION;;

typedef struct _OBJECT_HEADER {
    LONG_PTR PointerCount;
    union {
        LONG_PTR HandleCount;
        PVOID NextToFree;
    };
    POBJECT_TYPE Type;
    UCHAR NameInfoOffset;
    UCHAR HandleInfoOffset;
    UCHAR QuotaInfoOffset;
    UCHAR Flags;

    union {
        POBJECT_CREATE_INFORMATION ObjectCreateInfo;
        PVOID QuotaBlockCharged;
    };

    PSECURITY_DESCRIPTOR SecurityDescriptor;
    QUAD Body;
} OBJECT_HEADER, *POBJECT_HEADER;
// end_ntosp

typedef struct _OBJECT_HEADER_QUOTA_INFO {
    ULONG PagedPoolCharge;
    ULONG NonPagedPoolCharge;
    ULONG SecurityDescriptorCharge;
    PEPROCESS ExclusiveProcess;
#ifdef _WIN64
    ULONG64  Reserved;   // Win64 requires these structures to be 16 byte aligned.
#endif
} OBJECT_HEADER_QUOTA_INFO, *POBJECT_HEADER_QUOTA_INFO;

typedef struct _OBJECT_HEADER_HANDLE_INFO {
    union {
        POBJECT_HANDLE_COUNT_DATABASE HandleCountDataBase;
        OBJECT_HANDLE_COUNT_ENTRY SingleEntry;
    };
} OBJECT_HEADER_HANDLE_INFO, *POBJECT_HEADER_HANDLE_INFO;

// begin_ntosp
typedef struct _OBJECT_HEADER_NAME_INFO {
    POBJECT_DIRECTORY Directory;
    UNICODE_STRING Name;
    ULONG QueryReferences;
#if DBG
    ULONG Reserved2;
    LONG DbgDereferenceCount;
#ifdef _WIN64
    ULONG64  Reserved3;   // Win64 requires these structures to be 16 byte aligned.
#endif
#endif
} OBJECT_HEADER_NAME_INFO, *POBJECT_HEADER_NAME_INFO;
// end_ntosp

typedef struct _OBJECT_HEADER_CREATOR_INFO {
    LIST_ENTRY TypeList;
    HANDLE CreatorUniqueProcess;
    USHORT CreatorBackTraceIndex;
    USHORT Reserved;
} OBJECT_HEADER_CREATOR_INFO, *POBJECT_HEADER_CREATOR_INFO;

#define OB_FLAG_NEW_OBJECT              0x01
#define OB_FLAG_KERNEL_OBJECT           0x02
#define OB_FLAG_CREATOR_INFO            0x04
#define OB_FLAG_EXCLUSIVE_OBJECT        0x08
#define OB_FLAG_PERMANENT_OBJECT        0x10
#define OB_FLAG_DEFAULT_SECURITY_QUOTA  0x20
#define OB_FLAG_SINGLE_HANDLE_ENTRY     0x40
#define OB_FLAG_DELETED_INLINE          0x80

// begin_ntosp
#define OBJECT_TO_OBJECT_HEADER( o ) \
    CONTAINING_RECORD( (o), OBJECT_HEADER, Body )
// end_ntosp

#define OBJECT_HEADER_TO_EXCLUSIVE_PROCESS( oh ) ((oh->Flags & OB_FLAG_EXCLUSIVE_OBJECT) == 0 ? \
    NULL : (((POBJECT_HEADER_QUOTA_INFO)((PCHAR)(oh) - (oh)->QuotaInfoOffset))->ExclusiveProcess))

FORCEINLINE
POBJECT_HEADER_QUOTA_INFO
OBJECT_HEADER_TO_QUOTA_INFO_EXISTS (
    IN POBJECT_HEADER ObjectHeader
    )
{
    ASSERT(ObjectHeader->QuotaInfoOffset != 0);
    return (POBJECT_HEADER_QUOTA_INFO)((PUCHAR)ObjectHeader -
                                       ObjectHeader->QuotaInfoOffset);
}

FORCEINLINE
POBJECT_HEADER_QUOTA_INFO
OBJECT_HEADER_TO_QUOTA_INFO (
    IN POBJECT_HEADER ObjectHeader
    )
{
    POBJECT_HEADER_QUOTA_INFO quotaInfo;

    if (ObjectHeader->QuotaInfoOffset != 0) {
        quotaInfo = OBJECT_HEADER_TO_QUOTA_INFO_EXISTS(ObjectHeader);
        __assume(quotaInfo != NULL);
    } else {
        quotaInfo = NULL;
    }

    return quotaInfo;
}

FORCEINLINE
POBJECT_HEADER_HANDLE_INFO
OBJECT_HEADER_TO_HANDLE_INFO_EXISTS (
    IN POBJECT_HEADER ObjectHeader
    )
{
    ASSERT(ObjectHeader->HandleInfoOffset != 0);
    return (POBJECT_HEADER_HANDLE_INFO)((PUCHAR)ObjectHeader -
                                        ObjectHeader->HandleInfoOffset);
}

FORCEINLINE
POBJECT_HEADER_HANDLE_INFO
OBJECT_HEADER_TO_HANDLE_INFO (
    IN POBJECT_HEADER ObjectHeader
    )
{
    POBJECT_HEADER_HANDLE_INFO handleInfo;

    if (ObjectHeader->HandleInfoOffset != 0) {
        handleInfo = OBJECT_HEADER_TO_HANDLE_INFO_EXISTS(ObjectHeader);
        __assume(handleInfo != NULL);
    } else {
        handleInfo = NULL;
    }

    return handleInfo;
}

// begin_ntosp

FORCEINLINE
POBJECT_HEADER_NAME_INFO
OBJECT_HEADER_TO_NAME_INFO_EXISTS (
    IN POBJECT_HEADER ObjectHeader
    )
{
    ASSERT(ObjectHeader->NameInfoOffset != 0);
    return (POBJECT_HEADER_NAME_INFO)((PUCHAR)ObjectHeader -
                                      ObjectHeader->NameInfoOffset);
}

FORCEINLINE
POBJECT_HEADER_NAME_INFO
OBJECT_HEADER_TO_NAME_INFO (
    IN POBJECT_HEADER ObjectHeader
    )
{
    POBJECT_HEADER_NAME_INFO nameInfo;

    if (ObjectHeader->NameInfoOffset != 0) {
        nameInfo = OBJECT_HEADER_TO_NAME_INFO_EXISTS(ObjectHeader);
        __assume(nameInfo != NULL);
    } else {
        nameInfo = NULL;
    }

    return nameInfo;
}

// end_ntosp

FORCEINLINE
POBJECT_HEADER_CREATOR_INFO
OBJECT_HEADER_TO_CREATOR_INFO (
    IN POBJECT_HEADER ObjectHeader
    )
{
    POBJECT_HEADER_CREATOR_INFO creatorInfo;

    if ((ObjectHeader->Flags & OB_FLAG_CREATOR_INFO) != 0) {
        creatorInfo = ((POBJECT_HEADER_CREATOR_INFO)ObjectHeader) - 1;
        __assume(creatorInfo != NULL);
    } else {
        creatorInfo = NULL;
    }

    return creatorInfo;
}

// begin_ntosp

NTKERNELAPI
NTSTATUS
ObCreateObjectType(
    __in PUNICODE_STRING TypeName,
    __in POBJECT_TYPE_INITIALIZER ObjectTypeInitializer,
    __in_opt PSECURITY_DESCRIPTOR SecurityDescriptor,
    __out POBJECT_TYPE *ObjectType
    );

#define OBJ_PROTECT_CLOSE       0x00000001L

// end_ntosp

VOID
FASTCALL
ObFreeObjectCreateInfoBuffer(
    IN POBJECT_CREATE_INFORMATION ObjectCreateInfo
    );

NTSTATUS
ObSetDirectoryDeviceMap (
    OUT PDEVICE_MAP *ppDeviceMap OPTIONAL,
    IN HANDLE DirectoryHandle
    );

ULONG
ObIsLUIDDeviceMapsEnabled (
    );

ULONG
ObGetSecurityMode (
    );

BOOLEAN
ObIsObjectDeletionInline(
    IN PVOID Object
    );

//
//  Object attributes only for internal use
//

#define OBJ_KERNEL_EXCLUSIVE           0x00010000L

#define OBJ_VALID_PRIVATE_ATTRIBUTES   0x00010000L

#define OBJ_ALL_VALID_ATTRIBUTES (OBJ_VALID_PRIVATE_ATTRIBUTES | OBJ_VALID_ATTRIBUTES)


FORCEINLINE
ULONG
ObSanitizeHandleAttributes (
    IN ULONG HandleAttributes,
    IN KPROCESSOR_MODE Mode
    )
{
    if (Mode == KernelMode) {
        return HandleAttributes & OBJ_ALL_VALID_ATTRIBUTES;
    } else {
        return HandleAttributes & (OBJ_ALL_VALID_ATTRIBUTES & ~(OBJ_KERNEL_HANDLE | OBJ_KERNEL_EXCLUSIVE));
    }
}

// begin_nthal

// begin_ntosp

NTKERNELAPI
VOID
ObDeleteCapturedInsertInfo(
    __in PVOID Object
    );


NTKERNELAPI
NTSTATUS
ObCreateObject(
    __in KPROCESSOR_MODE ProbeMode,
    __in POBJECT_TYPE ObjectType,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in KPROCESSOR_MODE OwnershipMode,
    __inout_opt PVOID ParseContext,
    __in ULONG ObjectBodySize,
    __in ULONG PagedPoolCharge,
    __in ULONG NonPagedPoolCharge,
    __out PVOID *Object
    );

//
// These inlines correct an issue where the compiler refetches
// the output object over and over again because it thinks its
// a possible alias for other stores.
//
FORCEINLINE
NTSTATUS
_ObCreateObject(
    __in KPROCESSOR_MODE ProbeMode,
    __in POBJECT_TYPE ObjectType,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in KPROCESSOR_MODE OwnershipMode,
    __inout_opt PVOID ParseContext,
    __in ULONG ObjectBodySize,
    __in ULONG PagedPoolCharge,
    __in ULONG NonPagedPoolCharge,
    __out PVOID *pObject
    )
{
    PVOID Object;
    NTSTATUS Status;

    Status = ObCreateObject (ProbeMode,
                             ObjectType,
                             ObjectAttributes,
                             OwnershipMode,
                             ParseContext,
                             ObjectBodySize,
                             PagedPoolCharge,
                             NonPagedPoolCharge,
                             &Object);
    *pObject = Object;
    return Status;
}

#define ObCreateObject _ObCreateObject

// begin_ntifs

NTKERNELAPI
NTSTATUS
ObInsertObject(
    __in PVOID Object,
    __in_opt PACCESS_STATE PassedAccessState,
    __in_opt ACCESS_MASK DesiredAccess,
    __in ULONG ObjectPointerBias,
    __out_opt PVOID *NewObject,
    __out_opt PHANDLE Handle
    );

// end_nthal end_ntifs

NTKERNELAPI                                                     // ntddk wdm nthal ntifs
NTSTATUS                                                        // ntddk wdm nthal ntifs
ObReferenceObjectByHandle(                                      // ntddk wdm nthal ntifs
    __in HANDLE Handle,                                           // ntddk wdm nthal ntifs
    __in ACCESS_MASK DesiredAccess,                               // ntddk wdm nthal ntifs
    __in_opt POBJECT_TYPE ObjectType,                        // ntddk wdm nthal ntifs
    __in KPROCESSOR_MODE AccessMode,                              // ntddk wdm nthal ntifs
    __out PVOID *Object,                                          // ntddk wdm nthal ntifs
    __out_opt POBJECT_HANDLE_INFORMATION HandleInformation   // ntddk wdm nthal ntifs
    );                                                          // ntddk wdm nthal ntifs

FORCEINLINE
NTSTATUS
_ObReferenceObjectByHandle(
    __in HANDLE Handle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_TYPE ObjectType,
    __in KPROCESSOR_MODE AccessMode,
    __out PVOID *pObject,
    __out_opt POBJECT_HANDLE_INFORMATION pHandleInformation
    )
{
    PVOID Object;
    NTSTATUS Status;

    Status = ObReferenceObjectByHandle (Handle,
                                        DesiredAccess,
                                        ObjectType,
                                        AccessMode,
                                        &Object,
                                        pHandleInformation);
    *pObject = Object;
    return Status;
}

#define ObReferenceObjectByHandle _ObReferenceObjectByHandle

NTKERNELAPI
NTSTATUS
ObReferenceFileObjectForWrite(
    IN HANDLE Handle,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *FileObject,
    OUT POBJECT_HANDLE_INFORMATION HandleInformation
    );

NTKERNELAPI
NTSTATUS
ObOpenObjectByName(
    __in POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt POBJECT_TYPE ObjectType,
    __in KPROCESSOR_MODE AccessMode,
    __inout_opt PACCESS_STATE AccessState,
    __in_opt ACCESS_MASK DesiredAccess,
    __inout_opt PVOID ParseContext,
    __out PHANDLE Handle
    );


NTKERNELAPI                                                     // ntifs
NTSTATUS                                                        // ntifs
ObOpenObjectByPointer(                                          // ntifs
    __in PVOID Object,                                            // ntifs
    __in ULONG HandleAttributes,                                  // ntifs
    __in_opt PACCESS_STATE PassedAccessState,                // ntifs
    __in ACCESS_MASK DesiredAccess,                      // ntifs
    __in_opt POBJECT_TYPE ObjectType,                        // ntifs
    __in KPROCESSOR_MODE AccessMode,                              // ntifs
    __out PHANDLE Handle                                          // ntifs
    );                                                          // ntifs

NTKERNELAPI
NTSTATUS
ObReferenceObjectByName(
    __in PUNICODE_STRING ObjectName,
    __in ULONG Attributes,
    __in_opt PACCESS_STATE AccessState,
    __in_opt ACCESS_MASK DesiredAccess,
    __in POBJECT_TYPE ObjectType,
    __in KPROCESSOR_MODE AccessMode,
    __inout_opt PVOID ParseContext,
    __out PVOID *Object
    );

// end_ntosp

NTKERNELAPI                                                     // ntifs
VOID                                                            // ntifs
ObMakeTemporaryObject(                                          // ntifs
    __in PVOID Object                                             // ntifs
    );                                                          // ntifs

// begin_ntosp

NTKERNELAPI
BOOLEAN
ObFindHandleForObject(
    __in PEPROCESS Process,
    __in PVOID Object,
    __in_opt POBJECT_TYPE ObjectType,
    __in_opt POBJECT_HANDLE_INFORMATION MatchCriteria,
    __out PHANDLE Handle
    );

// begin_ntddk begin_wdm begin_nthal begin_ntifs

#define ObDereferenceObject(a)                                     \
        ObfDereferenceObject(a)

#define ObReferenceObject(Object) ObfReferenceObject(Object)

NTKERNELAPI
LONG_PTR
FASTCALL
ObfReferenceObject(
    __in PVOID Object
    );

NTKERNELAPI
NTSTATUS
ObReferenceObjectByPointer(
    __in PVOID Object,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_TYPE ObjectType,
    __in KPROCESSOR_MODE AccessMode
    );

NTKERNELAPI
LONG_PTR
FASTCALL
ObfDereferenceObject(
    __in PVOID Object
    );

// end_ntddk end_wdm end_nthal end_ntifs end_ntosp

NTKERNELAPI
BOOLEAN
FASTCALL
ObReferenceObjectSafe (
    IN PVOID Object
    );

NTKERNELAPI
LONG_PTR
FASTCALL
ObReferenceObjectEx (
    IN PVOID Object,
    IN ULONG Count
    );

LONG_PTR
FASTCALL
ObDereferenceObjectEx (
    IN PVOID Object,
    IN ULONG Count
    );

NTSTATUS
ObWaitForSingleObject(
    IN HANDLE Handle,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

VOID
ObDereferenceObjectDeferDelete (
    IN PVOID Object
    );

// begin_ntifs begin_ntosp
NTKERNELAPI
NTSTATUS
ObQueryNameString(
    __in PVOID Object,
    __out_bcount(Length) POBJECT_NAME_INFORMATION ObjectNameInfo,
    __in ULONG Length,
    __out PULONG ReturnLength
    );

// end_ntifs end_ntosp

#if DBG
PUNICODE_STRING
ObGetObjectName(
    IN PVOID Object
    );
#endif // DBG

NTSTATUS
ObQueryTypeName(
    IN PVOID Object,
    PUNICODE_STRING ObjectTypeName,
    IN ULONG Length,
    OUT PULONG ReturnLength
    );

NTSTATUS
ObQueryTypeInfo(
    IN POBJECT_TYPE ObjectType,
    OUT POBJECT_TYPE_INFORMATION ObjectTypeInfo,
    IN ULONG Length,
    OUT PULONG ReturnLength
    );

NTSTATUS
ObDumpObjectByHandle(
    IN HANDLE Handle,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );


NTSTATUS
ObDumpObjectByPointer(
    IN PVOID Object,
    IN POB_DUMP_CONTROL Control OPTIONAL
    );

NTSTATUS
ObSetDeviceMap(
    IN PEPROCESS TargetProcess,
    IN HANDLE DirectoryHandle
    );

NTSTATUS
ObQueryDeviceMapInformation(
    IN PEPROCESS TargetProcess,
    OUT PPROCESS_DEVICEMAP_INFORMATION DeviceMapInformation,
    IN ULONG Flags
    );

VOID
ObInheritDeviceMap(
    IN PEPROCESS NewProcess,
    IN PEPROCESS ParentProcess
    );

VOID
ObDereferenceDeviceMap(
    IN PEPROCESS Process
    );

// begin_ntifs begin_ntddk begin_wdm begin_ntosp

NTKERNELAPI
NTSTATUS
ObGetObjectSecurity(
    __in PVOID Object,
    __out PSECURITY_DESCRIPTOR *SecurityDescriptor,
    __out PBOOLEAN MemoryAllocated
    );

NTKERNELAPI
VOID
ObReleaseObjectSecurity(
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in BOOLEAN MemoryAllocated
    );

// end_ntifs end_ntddk end_wdm

NTKERNELAPI
NTSTATUS
ObLogSecurityDescriptor (
    __in PSECURITY_DESCRIPTOR InputSecurityDescriptor,
    __out PSECURITY_DESCRIPTOR *OutputSecurityDescriptor,
    __in ULONG RefBias
    );

NTKERNELAPI
VOID
ObDereferenceSecurityDescriptor (
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in ULONG Count
    );

VOID
ObReferenceSecurityDescriptor (
    __in PSECURITY_DESCRIPTOR SecurityDescriptor,
    __in ULONG Count
    );

// end_ntosp

NTSTATUS
ObAssignObjectSecurityDescriptor(
    IN PVOID Object,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,
    IN POOL_TYPE PoolType
    );

NTSTATUS
ObValidateSecurityQuota(
    IN PVOID Object,
    IN ULONG NewSize
    );

// begin_ntosp
NTKERNELAPI
BOOLEAN
ObCheckCreateObjectAccess(
    __in PVOID DirectoryObject,
    __in ACCESS_MASK CreateAccess,
    __in PACCESS_STATE AccessState,
    __in PUNICODE_STRING ComponentName,
    __in BOOLEAN TypeMutexLocked,
    __in KPROCESSOR_MODE PreviousMode,
    __out PNTSTATUS AccessStatus
   );

NTKERNELAPI
BOOLEAN
ObCheckObjectAccess(
    __in PVOID Object,
    __inout PACCESS_STATE AccessState,
    __in BOOLEAN TypeMutexLocked,
    __in KPROCESSOR_MODE AccessMode,
    __out PNTSTATUS AccessStatus
    );


NTKERNELAPI
NTSTATUS
ObAssignSecurity(
    __in PACCESS_STATE AccessState,
    __in_opt PSECURITY_DESCRIPTOR ParentDescriptor,
    __in PVOID Object,
    __in POBJECT_TYPE ObjectType
    );
// end_ntosp

NTKERNELAPI                                                     // ntifs
NTSTATUS                                                        // ntifs
ObQueryObjectAuditingByHandle(                                  // ntifs
    __in HANDLE Handle,                                           // ntifs
    __out PBOOLEAN GenerateOnClose                                // ntifs
    );                                                          // ntifs

// begin_ntosp

NTKERNELAPI
NTSTATUS
ObSetSecurityObjectByPointer (
    __in PVOID Object,
    __in SECURITY_INFORMATION SecurityInformation,
    __in PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTKERNELAPI
NTSTATUS
ObSetHandleAttributes (
    __in HANDLE Handle,
    __in POBJECT_HANDLE_FLAG_INFORMATION HandleFlags,
    __in KPROCESSOR_MODE PreviousMode
    );

NTKERNELAPI
NTSTATUS
ObCloseHandle (
    __in HANDLE Handle,
    __in KPROCESSOR_MODE PreviousMode
    );

NTKERNELAPI
NTSTATUS
ObSwapObjectNames (
    IN HANDLE DirectoryHandle,
    IN HANDLE Handle1,
    IN HANDLE Handle2,
    IN ULONG Flags
    );

// end_ntosp

#if DEVL

typedef BOOLEAN (*OB_ENUM_OBJECT_TYPE_ROUTINE)(
    IN PVOID Object,
    IN PUNICODE_STRING ObjectName,
    IN ULONG_PTR HandleCount,
    IN ULONG_PTR PointerCount,
    IN PVOID Parameter
    );

NTSTATUS
ObEnumerateObjectsByType(
    IN POBJECT_TYPE ObjectType,
    IN OB_ENUM_OBJECT_TYPE_ROUTINE EnumerationRoutine,
    IN PVOID Parameter
    );

NTSTATUS
ObGetHandleInformation(
    OUT PSYSTEM_HANDLE_INFORMATION HandleInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ObGetHandleInformationEx (
    OUT PSYSTEM_HANDLE_INFORMATION_EX HandleInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

NTSTATUS
ObGetObjectInformation(
    IN PCHAR UserModeBufferAddress,
    OUT PSYSTEM_OBJECTTYPE_INFORMATION ObjectInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );

// begin_ntosp
NTKERNELAPI
NTSTATUS
ObSetSecurityDescriptorInfo(
    __in PVOID Object,
    __in PSECURITY_INFORMATION SecurityInformation,
    __inout PSECURITY_DESCRIPTOR SecurityDescriptor,
    __inout PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    __in POOL_TYPE PoolType,
    __in PGENERIC_MAPPING GenericMapping
    );
// end_ntosp

NTKERNELAPI
NTSTATUS
ObQuerySecurityDescriptorInfo(
    IN PVOID Object,
    IN PSECURITY_INFORMATION SecurityInformation,
    OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG Length,
    IN PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor
    );

NTSTATUS
ObDeassignSecurity (
    IN OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    );

VOID
ObAuditObjectAccess(
    IN HANDLE Handle,
    IN POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    IN ACCESS_MASK DesiredAccess
    );

NTKERNELAPI
VOID
FASTCALL
ObInitializeFastReference (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    );

NTKERNELAPI
PVOID
FASTCALL
ObFastReferenceObject (
    IN PEX_FAST_REF FastRef
    );

NTKERNELAPI
PVOID
FASTCALL
ObFastReferenceObjectLocked (
    IN PEX_FAST_REF FastRef
    );

NTKERNELAPI
VOID
FASTCALL
ObFastDereferenceObject (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    );

NTKERNELAPI
PVOID
FASTCALL
ObFastReplaceObject (
    IN PEX_FAST_REF FastRef,
    IN PVOID Object
    );

#endif // DEVL

#endif // _OB_
