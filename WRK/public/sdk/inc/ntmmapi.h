/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntmmapi.h

Abstract:

    This is the include file for the Memory Management sub-component of NTOS

--*/

#ifndef _NTMMAPI_
#define _NTMMAPI_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _MEMORY_INFORMATION_CLASS {
    MemoryBasicInformation
#if DEVL
    ,MemoryWorkingSetInformation
#endif
    ,MemoryMappedFilenameInformation
    ,MemoryRegionInformation
    ,MemoryWorkingSetExInformation
} MEMORY_INFORMATION_CLASS;

//
// Memory information structures.
//
// begin_winnt

typedef struct _MEMORY_BASIC_INFORMATION {
    PVOID BaseAddress;
    PVOID AllocationBase;
    ULONG AllocationProtect;
    SIZE_T RegionSize;
    ULONG State;
    ULONG Protect;
    ULONG Type;
} MEMORY_BASIC_INFORMATION, *PMEMORY_BASIC_INFORMATION;

typedef struct _MEMORY_BASIC_INFORMATION32 {
    ULONG BaseAddress;
    ULONG AllocationBase;
    ULONG AllocationProtect;
    ULONG RegionSize;
    ULONG State;
    ULONG Protect;
    ULONG Type;
} MEMORY_BASIC_INFORMATION32, *PMEMORY_BASIC_INFORMATION32;

typedef struct DECLSPEC_ALIGN(16) _MEMORY_BASIC_INFORMATION64 {
    ULONGLONG BaseAddress;
    ULONGLONG AllocationBase;
    ULONG     AllocationProtect;
    ULONG     __alignment1;
    ULONGLONG RegionSize;
    ULONG     State;
    ULONG     Protect;
    ULONG     Type;
    ULONG     __alignment2;
} MEMORY_BASIC_INFORMATION64, *PMEMORY_BASIC_INFORMATION64;

// end_winnt

#if !defined(SORTPP_PASS) && !defined(MIDL_PASS) && !defined(RC_INVOKED) && !defined(_X86AMD64_)
#if defined(_WIN64)
C_ASSERT(sizeof(MEMORY_BASIC_INFORMATION) == sizeof(MEMORY_BASIC_INFORMATION64));
#else
C_ASSERT(sizeof(MEMORY_BASIC_INFORMATION) == sizeof(MEMORY_BASIC_INFORMATION32));
#endif
#endif

typedef struct _MEMORY_WORKING_SET_BLOCK {
    ULONG_PTR Protection : 5;
    ULONG_PTR ShareCount : 3;
    ULONG_PTR Shared : 1;
    ULONG_PTR Node : 3;
#if defined(_WIN64)
    ULONG_PTR VirtualPage : 52;
#else
    ULONG VirtualPage : 20;
#endif
} MEMORY_WORKING_SET_BLOCK, *PMEMORY_WORKING_SET_BLOCK;

typedef struct _MEMORY_WORKING_SET_EX_BLOCK {
    ULONG_PTR Valid : 1;
    ULONG_PTR ShareCount : 3;
    ULONG_PTR Win32Protection : 11;
    ULONG_PTR Shared : 1;
    ULONG_PTR Node : 6;
    ULONG_PTR Locked : 1;
    ULONG_PTR LargePage : 1;
    ULONG_PTR Priority : 3;
    ULONG_PTR Reserved : 5;

#if defined(_WIN64)
    ULONG_PTR ReservedUlong : 32;
#endif
} MEMORY_WORKING_SET_EX_BLOCK, *PMEMORY_WORKING_SET_EX_BLOCK;

typedef struct _MEMORY_WORKING_SET_EX_INFORMATION {
    PVOID VirtualAddress;
    union {
        MEMORY_WORKING_SET_EX_BLOCK VirtualAttributes;
        ULONG_PTR Long;
    } u1;
} MEMORY_WORKING_SET_EX_INFORMATION, *PMEMORY_WORKING_SET_EX_INFORMATION;

typedef struct _MEMORY_WORKING_SET_INFORMATION {
    ULONG_PTR NumberOfEntries;
    MEMORY_WORKING_SET_BLOCK WorkingSetInfo[1];
} MEMORY_WORKING_SET_INFORMATION, *PMEMORY_WORKING_SET_INFORMATION;

//
// MMPFNLIST_ and MMPFNUSE_ are used to characterize what
// a physical page is being used for.
//

#define MMPFNLIST_ZERO              0
#define MMPFNLIST_FREE              1
#define MMPFNLIST_STANDBY           2
#define MMPFNLIST_MODIFIED          3
#define MMPFNLIST_MODIFIEDNOWRITE   4
#define MMPFNLIST_BAD               5
#define MMPFNLIST_ACTIVE            6
#define MMPFNLIST_TRANSITION        7

#define MMPFNUSE_PROCESSPRIVATE      0
#define MMPFNUSE_FILE                1
#define MMPFNUSE_PAGEFILEMAPPED      2
#define MMPFNUSE_PAGETABLE           3
#define MMPFNUSE_PAGEDPOOL           4
#define MMPFNUSE_NONPAGEDPOOL        5
#define MMPFNUSE_SYSTEMPTE           6
#define MMPFNUSE_SESSIONPRIVATE      7
#define MMPFNUSE_METAFILE            8
#define MMPFNUSE_AWEPAGE             9
#define MMPFNUSE_DRIVERLOCKPAGE     10

typedef struct _MEMORY_FRAME_INFORMATION {
    ULONGLONG UseDescription : 4;   // MMPFNUSE_*
    ULONGLONG ListDescription : 3;  // MMPFNLIST_*
    ULONGLONG Reserved0 : 1;        // Reserved for future expansion
    ULONGLONG Pinned : 1;           // 1 indicates pinned, 0 means not pinned
    ULONGLONG DontUse : 48;         // overlaid with INFORMATION structures
    ULONGLONG Reserved : 7;         // Reserved for future expansion
} MEMORY_FRAME_INFORMATION;

typedef struct _FILEOFFSET_INFORMATION {
    ULONGLONG DontUse : 9;          // overlaid with MEMORY_FRAME_INFORMATION
    ULONGLONG Offset : 48;          // used for mapped files only.
    ULONGLONG Reserved : 7;         // Reserved for future expansion
} FILEOFFSET_INFORMATION;

typedef struct _PAGEDIR_INFORMATION {
    ULONGLONG DontUse : 9;            // overlaid with MEMORY_FRAME_INFORMATION
    ULONGLONG PageDirectoryBase : 48; // used for private pages only.
    ULONGLONG Reserved : 7;           // Reserved for future expansion
} PAGEDIR_INFORMATION;

typedef struct _MMPFN_IDENTITY {
    union {
        MEMORY_FRAME_INFORMATION e1;    // used for all cases.
        FILEOFFSET_INFORMATION e2;      // used for mapped files only.
        PAGEDIR_INFORMATION e3;         // used for private pages only.
    } u1;
    ULONG_PTR PageFrameIndex;           // used for all cases.
    union {
        PVOID FileObject;               // used for mapped files only.
        PVOID VirtualAddress;           // used for everything but mapped files.
    } u2;
} MMPFN_IDENTITY, *PMMPFN_IDENTITY;

typedef struct _MMPFN_MEMSNAP_INFORMATION {
    ULONG_PTR InitialPageFrameIndex;
    ULONG_PTR Count;
} MMPFN_MEMSNAP_INFORMATION, *PMMPFN_MEMSNAP_INFORMATION;

typedef enum _SECTION_INFORMATION_CLASS {
    SectionBasicInformation,
    SectionImageInformation,
    MaxSectionInfoClass  // MaxSectionInfoClass should always be the last enum
} SECTION_INFORMATION_CLASS;

// begin_ntddk begin_wdm

//
// Section Information Structures.
//

// end_ntddk end_wdm

typedef struct _SECTIONBASICINFO {
    PVOID BaseAddress;
    ULONG AllocationAttributes;
    LARGE_INTEGER MaximumSize;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4201)       // unnamed struct

typedef struct _SECTION_IMAGE_INFORMATION {
    PVOID TransferAddress;
    ULONG ZeroBits;
    SIZE_T MaximumStackSize;
    SIZE_T CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    BOOLEAN Spare1;
    ULONG LoaderFlags;
    ULONG ImageFileSize;
    ULONG Reserved[ 1 ];
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;


//
// This structure is used only by Wow64 processes. The offsets
// of structure elements should the same as viewed by a native Win64 application.
//
typedef struct _SECTION_IMAGE_INFORMATION64 {
    ULONGLONG TransferAddress;
    ULONG ZeroBits;
    ULONGLONG MaximumStackSize;
    ULONGLONG CommittedStackSize;
    ULONG SubSystemType;
    union {
        struct {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    BOOLEAN Spare1;
    ULONG LoaderFlags;
    ULONG ImageFileSize;
    ULONG Reserved[ 1 ];
} SECTION_IMAGE_INFORMATION64, *PSECTION_IMAGE_INFORMATION64;

#if !defined(SORTPP_PASS) && !defined(MIDL_PASS) && !defined(RC_INVOKED) && defined(_WIN64) && !defined(_X86AMD64_)
C_ASSERT(sizeof(SECTION_IMAGE_INFORMATION) == sizeof(SECTION_IMAGE_INFORMATION64));
#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4201 )
#endif

// begin_ntddk begin_wdm
typedef enum _SECTION_INHERIT {
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT;

//
// Section Access Rights.
//

// begin_winnt
#define SECTION_QUERY                0x0001
#define SECTION_MAP_WRITE            0x0002
#define SECTION_MAP_READ             0x0004
#define SECTION_MAP_EXECUTE          0x0008
#define SECTION_EXTEND_SIZE          0x0010
#define SECTION_MAP_EXECUTE_EXPLICIT 0x0020 // not included in SECTION_ALL_ACCESS

#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)
// end_winnt

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

// end_ntddk end_wdm

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
#define MEM_LARGE_PAGES  0x20000000     // winnt ntddk wdm
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
#define SEC_LARGE_PAGES  0x80000000     // winnt ntddk wdm

#define MEM_IMAGE         SEC_IMAGE     // winnt

#define WRITE_WATCH_FLAG_RESET 0x01     // winnt

#define MAP_PROCESS 1L
#define MAP_SYSTEM  2L

#define MEM_EXECUTE_OPTION_DISABLE 0x1 
#define MEM_EXECUTE_OPTION_ENABLE 0x2
#define MEM_EXECUTE_OPTION_DISABLE_THUNK_EMULATION 0x4
#define MEM_EXECUTE_OPTION_PERMANENT 0x8
#define MEM_EXECUTE_OPTION_EXECUTE_DISPATCH_ENABLE 0x10
#define MEM_EXECUTE_OPTION_IMAGE_DISPATCH_ENABLE 0x20 
#define MEM_EXECUTE_OPTION_VALID_FLAGS 0x3f

// begin_ntifs

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreateSection (
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
    __in_opt PLARGE_INTEGER MaximumSize,
    __in ULONG SectionPageProtection,
    __in ULONG AllocationAttributes,
    __in_opt HANDLE FileHandle
    );

// end_ntifs

NTSYSCALLAPI
NTSTATUS
NTAPI
NtOpenSection (
    __out PHANDLE SectionHandle,
    __in ACCESS_MASK DesiredAccess,
    __in POBJECT_ATTRIBUTES ObjectAttributes
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtMapViewOfSection (
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

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnmapViewOfSection (
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtExtendSection (
    __in HANDLE SectionHandle,
    __inout PLARGE_INTEGER NewSectionSize
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAreMappedFilesTheSame (
    __in PVOID File1MappedAsAnImage,
    __in PVOID File2MappedAsFile
    );

// begin_ntifs

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAllocateVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __in ULONG_PTR ZeroBits,
    __inout PSIZE_T RegionSize,
    __in ULONG AllocationType,
    __in ULONG Protect
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFreeVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in ULONG FreeType
    );

// end_ntifs

NTSYSCALLAPI
NTSTATUS
NTAPI
NtReadVirtualMemory (
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress,
    __out_bcount(BufferSize) PVOID Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesRead
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtWriteVirtualMemory (
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress,
    __in_bcount(BufferSize) CONST VOID *Buffer,
    __in SIZE_T BufferSize,
    __out_opt PSIZE_T NumberOfBytesWritten
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __out PIO_STATUS_BLOCK IoStatus
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtLockVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in ULONG MapType
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtUnlockVirtualMemory ( 
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in ULONG MapType
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtProtectVirtualMemory (
    __in HANDLE ProcessHandle,
    __inout PVOID *BaseAddress,
    __inout PSIZE_T RegionSize,
    __in ULONG NewProtect,
    __out PULONG OldProtect
    );


NTSYSCALLAPI
NTSTATUS
NTAPI
NtQueryVirtualMemory (
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __in MEMORY_INFORMATION_CLASS MemoryInformationClass,
    __out_bcount(MemoryInformationLength) PVOID MemoryInformation,
    __in SIZE_T MemoryInformationLength,
    __out_opt PSIZE_T ReturnLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtQuerySection (
    __in HANDLE SectionHandle,
    __in SECTION_INFORMATION_CLASS SectionInformationClass,
    __out_bcount(SectionInformationLength) PVOID SectionInformation,
    __in SIZE_T SectionInformationLength,
    __out_opt PSIZE_T ReturnLength
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtMapUserPhysicalPages (
    __in PVOID VirtualAddress,
    __in ULONG_PTR NumberOfPages,
    __in_ecount_opt(NumberOfPages) PULONG_PTR UserPfnArray
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtMapUserPhysicalPagesScatter (
    __in_ecount(NumberOfPages) PVOID *VirtualAddresses,
    __in ULONG_PTR NumberOfPages,
    __in_ecount_opt(NumberOfPages) PULONG_PTR UserPfnArray
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtAllocateUserPhysicalPages (
    __in HANDLE ProcessHandle,
    __inout PULONG_PTR NumberOfPages,
    __out_ecount(*NumberOfPages) PULONG_PTR UserPfnArray
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFreeUserPhysicalPages (
    __in HANDLE ProcessHandle,
    __inout PULONG_PTR NumberOfPages,
    __in_ecount(*NumberOfPages) PULONG_PTR UserPfnArray
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtGetWriteWatch (
    __in HANDLE ProcessHandle,
    __in ULONG Flags,
    __in PVOID BaseAddress,
    __in SIZE_T RegionSize,
    __out_ecount(*EntriesInUserAddressArray) PVOID *UserAddressArray,
    __inout PULONG_PTR EntriesInUserAddressArray,
    __out PULONG Granularity
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtResetWriteWatch (
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress,
    __in SIZE_T RegionSize
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtCreatePagingFile (
    __in PUNICODE_STRING PageFileName,
    __in PLARGE_INTEGER MinimumSize,
    __in PLARGE_INTEGER MaximumSize,
    __in ULONG Priority
    );

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushInstructionCache (
    __in HANDLE ProcessHandle,
    __in_opt PVOID BaseAddress,
    __in SIZE_T Length
    );


//
// Coherency related function prototype definitions.
//

NTSYSCALLAPI
NTSTATUS
NTAPI
NtFlushWriteBuffer (
    VOID
    );

#ifdef __cplusplus
}
#endif

#endif  // _NTMMAPI_

