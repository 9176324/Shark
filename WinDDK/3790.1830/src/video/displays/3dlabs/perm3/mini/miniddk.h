/***************************************************************************\
*
*                        ************************
*                        * MINIPORT SAMPLE CODE *
*                        ************************
*
* Module Name:
*
*   miniddk.h
*
* Abstract:
*
*   This module contains the definitions for the Permedia3 miniport driver.
*
* Environment:
*
*   Kernel mode
*
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All Rights Reserved.
*
\***************************************************************************/

//
// ntddk.h can't be included, so what in need is copied here.
//

#ifdef _X86_
#define PAGE_SIZE 0x1000
#else
#define PAGE_SIZE 0x2000
#endif

#ifndef FORCEINLINE
#if (_MSC_VER >= 1200)
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __inline
#endif
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#define NTKERNELAPI DECLSPEC_IMPORT

typedef enum _MEMORY_CACHING_TYPE_ORIG {
    MmFrameBufferCached = 2
} MEMORY_CACHING_TYPE_ORIG;

typedef enum _MEMORY_CACHING_TYPE {
    MmNonCached = FALSE,
    MmCached = TRUE,
    MmWriteCombined = MmFrameBufferCached,
    MmHardwareCoherentCached,
    MmNonCachedUnordered,       // IA64
    MmUSWCCached,
    MmMaximumCacheType
} MEMORY_CACHING_TYPE;

NTKERNELAPI
PVOID
MmAllocateContiguousMemorySpecifyCache (
    IN SIZE_T NumberOfBytes,
    IN PHYSICAL_ADDRESS LowestAcceptableAddress,
    IN PHYSICAL_ADDRESS HighestAcceptableAddress,
    IN PHYSICAL_ADDRESS BoundaryAddressMultiple OPTIONAL,
    IN MEMORY_CACHING_TYPE CacheType
    );

NTKERNELAPI
VOID
MmFreeContiguousMemorySpecifyCache (
  IN PVOID BaseAddress,
  IN SIZE_T NumberOfBytes,
  IN MEMORY_CACHING_TYPE CacheType
  );

NTKERNELAPI
PHYSICAL_ADDRESS
  MmGetPhysicalAddress(
  IN PVOID BaseAddress
  );

typedef struct _LIST_ENTRY {
   struct _LIST_ENTRY *Flink;
   struct _LIST_ENTRY *Blink;
} LIST_ENTRY, *PLIST_ENTRY, *RESTRICTED_POINTER PRLIST_ENTRY;

VOID
FORCEINLINE
InitializeListHead(
    IN PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))



VOID
FORCEINLINE
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
}

PLIST_ENTRY
FORCEINLINE
RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



PLIST_ENTRY
FORCEINLINE
RemoveTailList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


VOID
FORCEINLINE
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


VOID
FORCEINLINE
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}

// Common dispatcher object header
//
// N.B. The size field contains the number of dwords in the structure.
//
typedef struct _DISPATCHER_HEADER {
    UCHAR Type;
    UCHAR Absolute;
    UCHAR Size;
    UCHAR Inserted;
    LONG SignalState;
    LIST_ENTRY WaitListHead;
} DISPATCHER_HEADER;

//
// Event object
//

typedef struct _KEVENT {
    DISPATCHER_HEADER Header;
} KEVENT, *PKEVENT, *RESTRICTED_POINTER PRKEVENT;

typedef enum _EVENT_TYPE {
    NotificationEvent,
    SynchronizationEvent
    } EVENT_TYPE;

NTKERNELAPI
VOID
KeInitializeEvent (
    IN PRKEVENT Event,
    IN EVENT_TYPE Type,
    IN BOOLEAN State
    );

//
// Wait reasons
//

typedef enum _KWAIT_REASON {
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel,
    MaximumWaitReason
    } KWAIT_REASON;

typedef CCHAR KPROCESSOR_MODE;

typedef enum _MODE {
    KernelMode,
    UserMode,
    MaximumMode
} MODE;

// typedef LONG NTSTATUS;

NTKERNELAPI
LONG
KeWaitForSingleObject (
    IN PVOID Object,
    IN KWAIT_REASON WaitReason,
    IN KPROCESSOR_MODE WaitMode,
    IN BOOLEAN Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

typedef struct _EPROCESS *PEPROCESS;

NTKERNELAPI
PEPROCESS
IoGetCurrentProcess(
    );

NTKERNELAPI
HANDLE
PsGetCurrentProcessId(
    );

typedef ULONG ACCESS_MASK;
typedef ACCESS_MASK *PACCESS_MASK;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
#ifdef MIDL_PASS
    [size_is(MaximumLength / 2), length_is((Length) / 2) ] USHORT * Buffer;
#else // MIDL_PASS
    PWSTR  Buffer;
#endif // MIDL_PASS
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;
typedef const UNICODE_STRING *PCUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;        // Points to type SECURITY_DESCRIPTOR
    PVOID SecurityQualityOfService;  // Points to type SECURITY_QUALITY_OF_SERVICE
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;
typedef CONST OBJECT_ATTRIBUTES *PCOBJECT_ATTRIBUTES;

typedef struct _FILE_OBJECT *PFILE_OBJECT;

NTKERNELAPI
LONG                                       // NTSTATUS
MmCreateSection (
    OUT PVOID *SectionObject,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN PLARGE_INTEGER MaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN HANDLE FileHandle OPTIONAL,
    IN PFILE_OBJECT File OPTIONAL
    );

#define SECTION_QUERY       0x0001
#define SECTION_MAP_WRITE   0x0002
#define SECTION_MAP_READ    0x0004
#define SECTION_MAP_EXECUTE 0x0008
#define SECTION_EXTEND_SIZE 0x0010

// ntseapi.h

#define DELETE                           (0x00010000L)
#define READ_CONTROL                     (0x00020000L)
#define WRITE_DAC                        (0x00040000L)
#define WRITE_OWNER                      (0x00080000L)
#define SYNCHRONIZE                      (0x00100000L)

#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)

#define STANDARD_RIGHTS_READ             (READ_CONTROL)
#define STANDARD_RIGHTS_WRITE            (READ_CONTROL)
#define STANDARD_RIGHTS_EXECUTE          (READ_CONTROL)

#define STANDARD_RIGHTS_ALL              (0x001F0000L)

#define SPECIFIC_RIGHTS_ALL              (0x0000FFFFL)

// ntmmapi.h

#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)

#define SEGMENT_ALL_ACCESS SECTION_ALL_ACCESS

#define PAGE_NOACCESS          0x01     // winnt
#define PAGE_READONLY          0x02     // winnt
#define PAGE_READWRITE         0x04     // winnt
#define PAGE_WRITECOPY         0x08     // winnt
#define PAGE_EXECUTE           0x10     // winnt
#define PAGE_EXECUTE_READ      0x20     // winnt
#define PAGE_EXECUTE_READWRITE 0x40     // winnt
#define PAGE_EXECUTE_WRITECOPY 0x80     // winnt
#define PAGE_GUARD            0x100     // winnt
#define PAGE_NOCACHE          0x200     // winnt
#define PAGE_WRITECOMBINE     0x400     // winnt

#define MEM_COMMIT           0x1000     // winnt ntddk wdm
#define MEM_RESERVE          0x2000     // winnt ntddk wdm
#define MEM_DECOMMIT         0x4000     // winnt ntddk wdm
#define MEM_RELEASE          0x8000     // winnt ntddk wdm
#define MEM_FREE            0x10000     // winnt ntddk wdm
#define MEM_PRIVATE         0x20000     // winnt ntddk wdm
#define MEM_MAPPED          0x40000     // winnt ntddk wdm
#define MEM_RESET           0x80000     // winnt ntddk wdm
#define MEM_TOP_DOWN       0x100000     // winnt ntddk wdm
#define MEM_WRITE_WATCH    0x200000     // winnt
#define MEM_PHYSICAL       0x400000     // winnt
#define MEM_LARGE_PAGES  0x20000000     // ntddk wdm
#define MEM_DOS_LIM      0x40000000
#define MEM_4MB_PAGES    0x80000000     // winnt ntddk wdm

#define SEC_BASED          0x200000
#define SEC_NO_CHANGE      0x400000
#define SEC_FILE           0x800000     // winnt
#define SEC_IMAGE         0x1000000     // winnt
#define SEC_RESERVE       0x4000000     // winnt ntddk wdm
#define SEC_COMMIT        0x8000000     // winnt ntifs
#define SEC_NOCACHE      0x10000000     // winnt
#define SEC_GLOBAL       0x20000000

LONG                            // NTSTATUS
MmMapViewInSystemSpace (
    IN PVOID Section,
    OUT PVOID *MappedBase,
    IN OUT PSIZE_T ViewSize
    );

LONG
MmUnmapViewInSystemSpace (
    IN PVOID MappedBase
    );

typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

NTKERNELAPI
LONG
MmMapViewOfSection(
    IN PVOID SectionToMap,
    IN PEPROCESS Process,
    IN OUT PVOID *CapturedBase,
    IN ULONG_PTR ZeroBits,
    IN SIZE_T CommitSize,
    IN OUT PLARGE_INTEGER SectionOffset,
    IN OUT PSIZE_T CapturedViewSize,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG AllocationType,
    IN ULONG Protect
    );

NTKERNELAPI
LONG
MmUnmapViewOfSection(
    IN PEPROCESS Process,
    IN PVOID BaseAddress
     );


