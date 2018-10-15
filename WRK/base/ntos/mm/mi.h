/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    mi.h

Abstract:

    This module contains the private data structures and procedure
    prototypes for the memory management system.

--*/

#ifndef _MI_
#define _MI_

#pragma warning(disable:4214)   // bit field types other than int
#pragma warning(disable:4201)   // nameless struct/union
#pragma warning(disable:4324)   // alignment sensitive to declspec
#pragma warning(disable:4127)   // condition expression is constant
#pragma warning(disable:4115)   // named type definition in parentheses
#pragma warning(disable:4232)   // dllimport not static
#pragma warning(disable:4206)   // translation unit empty

#include "ntos.h"
#include "ntimage.h"
#include "fsrtl.h"
#include "zwapi.h"
#include "pool.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "safeboot.h"
#include "triage.h"

#if defined(_X86_)
#include "..\mm\i386\mi386.h"

#elif defined(_AMD64_)
#include "..\mm\amd64\miamd.h"

#else
#error "mm: a target architecture must be defined."
#endif

#if defined (_WIN64)
#define ASSERT32(exp)
#define ASSERT64(exp)   ASSERT(exp)

//
// This macro is used to satisfy the compiler -
// note the assignments are not needed for correctness
// but without it the compiler cannot compile this code
// W4 to check for use of uninitialized variables.
//

#define SATISFY_OVERZEALOUS_COMPILER(x) x
#else
#define ASSERT32(exp)   ASSERT(exp)
#define ASSERT64(exp)
#define SATISFY_OVERZEALOUS_COMPILER(x) x
#endif

//
// Special pool constants
//
#define MI_SPECIAL_POOL_PAGEABLE         0x8000
#define MI_SPECIAL_POOL_VERIFIER         0x4000
#define MI_SPECIAL_POOL_IN_SESSION       0x2000
#define MI_SPECIAL_POOL_PTE_PAGEABLE     0x0002
#define MI_SPECIAL_POOL_PTE_NONPAGEABLE  0x0004
#define MI_SPECIAL_POOL_PTE_FREED        0x0008


#define _1gb  0x40000000                // 1 gigabyte
#define _2gb  0x80000000                // 2 gigabytes
#define _3gb  0xC0000000                // 3 gigabytes
#define _4gb 0x100000000                // 4 gigabytes

#define MM_FLUSH_COUNTER_MASK (0xFFFFF)

#define MM_FREE_WSLE_SHIFT 4

#define WSLE_NULL_INDEX ((((WSLE_NUMBER)-1) >> MM_FREE_WSLE_SHIFT))

#define MM_FREE_POOL_SIGNATURE (0x50554F4C)

#define MM_MINIMUM_PAGED_POOL_NTAS ((SIZE_T)(48*1024*1024))

#define MM_ALLOCATION_FILLS_VAD ((PMMPTE)(ULONG_PTR)~3)

#define MM_WORKING_SET_LIST_SEARCH 17

#define MM_FLUID_WORKING_SET 8

#define MM_FLUID_PHYSICAL_PAGES 32  // see MmResidentPages below.

#define MM_USABLE_PAGES_FREE 32

#define X64K (ULONG)65536

#define MM_HIGHEST_VAD_ADDRESS ((PVOID)((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (64 * 1024)))


#define MM_WS_NOT_LISTED    ((PLIST_ENTRY)0)
#define MM_WS_TRIMMING      ((PLIST_ENTRY)1)
#define MM_WS_SWAPPED_OUT   ((PLIST_ENTRY)2)

#if DBG
#define MM_IO_IN_PROGRESS ((PLIST_ENTRY)97)
#endif

#define MM4K_SHIFT    12  // MUST BE LESS THAN OR EQUAL TO PAGE_SHIFT
#define MM4K_MASK  0xfff

#define MMSECTOR_SHIFT 9  // MUST BE LESS THAN OR EQUAL TO PAGE_SHIFT

#define MMSECTOR_MASK 0x1ff

#define MM_LOCK_BY_REFCOUNT 0

#define MM_LOCK_BY_NONPAGE 1

#define MM_MAXIMUM_WRITE_CLUSTER (MM_MAXIMUM_DISK_IO_SIZE / PAGE_SIZE)

//
// Number of PTEs to flush singularly before flushing the entire TB.
//

#define MM_MAXIMUM_FLUSH_COUNT (FLUSH_MULTIPLE_MAXIMUM + 1)

//
// Page protections
//

typedef ULONG WIN32_PROTECTION_MASK;
typedef PULONG PWIN32_PROTECTION_MASK;

typedef ULONG MM_PROTECTION_MASK;
typedef PULONG PMM_PROTECTION_MASK;

#define MM_ZERO_ACCESS         0  // this value is not used.
#define MM_READONLY            1
#define MM_EXECUTE             2
#define MM_EXECUTE_READ        3
#define MM_READWRITE           4  // bit 2 is set if this is writable.
#define MM_WRITECOPY           5
#define MM_EXECUTE_READWRITE   6
#define MM_EXECUTE_WRITECOPY   7

#define MM_NOCACHE            0x8
#define MM_GUARD_PAGE         0x10
#define MM_DECOMMIT           0x10   // NO_ACCESS, Guard page
#define MM_NOACCESS           0x18   // NO_ACCESS, Guard_page, nocache.
#define MM_UNKNOWN_PROTECTION 0x100  // bigger than 5 bits!

#define MM_INVALID_PROTECTION ((ULONG)-1)  // bigger than 5 bits!

#define MM_PROTECTION_WRITE_MASK     4
#define MM_PROTECTION_COPY_MASK      1
#define MM_PROTECTION_OPERATION_MASK 7 // mask off guard page and nocache.
#define MM_PROTECTION_EXECUTE_MASK   2

#define MM_SECURE_DELETE_CHECK 0x55

//
// Add guard page to the argument protection.
//

#define MI_ADD_GUARD(ProtectCode)   (ProtectCode |= MM_GUARD_PAGE);
#define MI_IS_GUARD(ProtectCode)    ((ProtectCode >> 3) == (MM_GUARD_PAGE >> 3))

//
// Add no cache to the argument protection.
//

#define MI_ADD_NOCACHE(ProtectCode)     (ProtectCode |= MM_NOCACHE);
#define MI_IS_NOCACHE(ProtectCode)      ((ProtectCode >> 3) == (MM_NOCACHE >> 3))

//
// Add write combined to the argument protection.
//

#define MM_WRITECOMBINE (MM_NOCACHE | MM_GUARD_PAGE)
#define MI_ADD_WRITECOMBINE(ProtectCode)   (ProtectCode |= MM_WRITECOMBINE);
#define MI_IS_WRITECOMBINE(ProtectCode)    (((ProtectCode >> 3) == (MM_WRITECOMBINE >> 3)) && (ProtectCode & 0x7))

#if defined(_X86PAE_)

//
// PAE mode makes most kernel resources executable to improve
// compatibility with existing driver binaries.
//

#define MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE(TempPte)         \
                    ASSERT ((TempPte).u.Hard.Valid == 1);   \
                    ((TempPte).u.Long &= ~MmPaeMask);

#define MI_ADD_EXECUTE_TO_INVALID_PTE_IF_PAE(TempPte)       \
                    ASSERT ((TempPte).u.Hard.Valid == 0);   \
                    ((TempPte).u.Soft.Protection |= MM_EXECUTE);

#else

//
// NT64 drivers derived from 32-bit source have to be recompiled so there's
// no need to make everything executable - drivers can specify it explicitly.
//

#define MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE(TempPte)
#define MI_ADD_EXECUTE_TO_INVALID_PTE_IF_PAE(TempPte)
#endif

//
// Debug flags
//

#define MM_DBG_PTE_UPDATE       0x2
#define MM_DBG_DUMP_WSL         0x4
#define MM_DBG_PAGEFAULT        0x8
#define MM_DBG_WS_EXPANSION     0x10
#define MM_DBG_MOD_WRITE        0x20
#define MM_DBG_CHECK_PTE        0x40
#define MM_DBG_VAD_CONFLICT     0x80
#define MM_DBG_SECTIONS         0x100
#define MM_DBG_STOP_ON_WOW64_ACCVIO   0x200
#define MM_DBG_SYS_PTES         0x400
#define MM_DBG_COLLIDED_PAGE    0x1000
#define MM_DBG_DUMP_BOOT_PTES   0x2000
#define MM_DBG_FORK             0x4000
#define MM_DBG_PAGE_IN_LIST     0x40000
#define MM_DBG_PRIVATE_PAGES    0x100000
#define MM_DBG_WALK_VAD_TREE    0x200000
#define MM_DBG_SWAP_PROCESS     0x400000
#define MM_DBG_LOCK_CODE        0x800000
#define MM_DBG_STOP_ON_ACCVIO   0x1000000
#define MM_DBG_PAGE_REF_COUNT   0x2000000
#define MM_DBG_SHOW_FAULTS      0x40000000
#define MM_DBG_SESSIONS         0x80000000

//
// If the PTE.protection & MM_COPY_ON_WRITE_MASK == MM_COPY_ON_WRITE_MASK
// then the PTE is copy on write.
//

#define MM_COPY_ON_WRITE_MASK  5

extern ULONG MmProtectToValue[32];

extern
#if defined(_WIN64) || defined(_X86PAE_)
ULONGLONG
#else
ULONG
#endif
MmProtectToPteMask[32];
extern ULONG MmMakeProtectNotWriteCopy[32];
extern ACCESS_MASK MmMakeSectionAccess[8];
extern ACCESS_MASK MmMakeFileAccess[8];


//
// Time constants
//

extern const LARGE_INTEGER MmSevenMinutes;
const extern LARGE_INTEGER MmOneSecond;
const extern LARGE_INTEGER MmTwentySeconds;
const extern LARGE_INTEGER MmSeventySeconds;
const extern LARGE_INTEGER MmShortTime;
const extern LARGE_INTEGER MmHalfSecond;
const extern LARGE_INTEGER Mm30Milliseconds;
extern LARGE_INTEGER MmCriticalSectionTimeout;

//
// A month's worth
//

extern ULONG MmCritsectTimeoutSeconds;

//
// this is the csrss process !
//

extern PEPROCESS ExpDefaultErrorPortProcess;

extern SIZE_T MmExtendedCommit;

extern SIZE_T MmTotalProcessCommit;

#if !defined(_WIN64)
extern LIST_ENTRY MmProcessList;
extern PMMPTE MiLargePageHyperPte;
extern PMMPTE MiInitialSystemPageDirectory;
#endif

//
// The total number of pages needed for the loader to successfully hibernate.
//

extern PFN_NUMBER MmHiberPages;

//
//  The counters and reasons to retry IO to protect against verifier induced
//  failures and temporary conditions.
//

extern ULONG MiIoRetryMask;
extern ULONG MiFaultRetryMask;
extern ULONG MiUserFaultRetryMask;

#define MmIsRetryIoStatus(S) (((S) == STATUS_INSUFFICIENT_RESOURCES) || \
                              ((S) == STATUS_WORKING_SET_QUOTA) ||      \
                              ((S) == STATUS_NO_MEMORY))

#if defined (_MI_MORE_THAN_4GB_)

extern PFN_NUMBER MiNoLowMemory;

#if defined (_WIN64)
#define MI_MAGIC_4GB_RECLAIM     0xffffedf0
#else
#define MI_MAGIC_4GB_RECLAIM     0xffedf0
#endif

#define MI_LOWMEM_MAGIC_BIT     (0x80000000)

extern PRTL_BITMAP MiLowMemoryBitMap;
#endif

//
// This is a version of COMPUTE_PAGES_SPANNED that works for 32 and 64 ranges.
//

#define MI_COMPUTE_PAGES_SPANNED(Va, Size) \
    ((((ULONG_PTR)(Va) & (PAGE_SIZE -1)) + (Size) + (PAGE_SIZE - 1)) >> PAGE_SHIFT)

//++
//
// ULONG
// MI_CONVERT_FROM_PTE_PROTECTION (
//     IN ULONG PROTECTION_MASK
//     )
//
// Routine Description:
//
//  This routine converts a PTE protection into a Protect value.
//
// Arguments:
//
//
// Return Value:
//
//     Returns the
//
//--

#define MI_CONVERT_FROM_PTE_PROTECTION(PROTECTION_MASK)      \
                                     (MmProtectToValue[PROTECTION_MASK])

#define MI_IS_PTE_PROTECTION_COPY_WRITE(PROTECTION_MASK)  \
   (((PROTECTION_MASK) & MM_COPY_ON_WRITE_MASK) == MM_COPY_ON_WRITE_MASK)

//++
//
// ULONG
// MI_ROUND_TO_64K (
//     IN ULONG LENGTH
//     )
//
// Routine Description:
//
//
// The ROUND_TO_64k macro takes a LENGTH in bytes and rounds it up to a multiple
// of 64K.
//
// Arguments:
//
//     LENGTH - LENGTH in bytes to round up to 64k.
//
// Return Value:
//
//     Returns the LENGTH rounded up to a multiple of 64k.
//
//--

#define MI_ROUND_TO_64K(LENGTH)  (((LENGTH) + X64K - 1) & ~((ULONG_PTR)X64K - 1))

extern ULONG MiLastVadBit;

//++
//
// ULONG
// MI_ROUND_WOULD_WRAP (
//     IN ULONG LENGTH,
//     IN ULONG ALIGNMENT
//     )
//
// Routine Description:
//
//
// The ROUND_WOULD_WRAP macro takes a LENGTH in bytes and checks whether
// rounding it up to a multiple of the alignment would wrap.
//
// This assumes both arguments are declared as unsigned.
//
// Arguments:
//
//     LENGTH - LENGTH in bytes to round up to.
//
//     ALIGNMENT - alignment to round to, must be a power of 2, e.g, 2**n.
//
// Return Value:
//
//     TRUE if using MI_ROUND_TO_SIZE would wrap, FALSE if not.
//
//--

#define MI_ROUND_WOULD_WRAP(LENGTH,ALIGNMENT)     \
                    (((LENGTH) + ((ALIGNMENT) - 1)) <= (LENGTH))

//++
//
// ULONG
// MI_ROUND_TO_SIZE (
//     IN ULONG LENGTH,
//     IN ULONG ALIGNMENT
//     )
//
// Routine Description:
//
//
// The ROUND_TO_SIZE macro takes a LENGTH in bytes and rounds it up to a
// multiple of the alignment.
//
// Arguments:
//
//     LENGTH - LENGTH in bytes to round up to.
//
//     ALIGNMENT - alignment to round to, must be a power of 2, e.g, 2**n.
//
// Return Value:
//
//     Returns the LENGTH rounded up to a multiple of the alignment.
//
//--

#define MI_ROUND_TO_SIZE(LENGTH,ALIGNMENT)     \
                    (((LENGTH) + ((ALIGNMENT) - 1)) & ~((ALIGNMENT) - 1))

//++
//
// PVOID
// MI_64K_ALIGN (
//     IN PVOID VA
//     )
//
// Routine Description:
//
//
// The MI_64K_ALIGN macro takes a virtual address and returns a 64k-aligned
// virtual address for that page.
//
// Arguments:
//
//     VA - Virtual address.
//
// Return Value:
//
//     Returns the 64k aligned virtual address.
//
//--

#define MI_64K_ALIGN(VA) ((PVOID)((ULONG_PTR)(VA) & ~((LONG)X64K - 1)))


//++
//
// PVOID
// MI_ALIGN_TO_SIZE (
//     IN PVOID VA
//     IN ULONG ALIGNMENT
//     )
//
// Routine Description:
//
//
// The MI_ALIGN_TO_SIZE macro takes a virtual address and returns a
// virtual address for that page with the specified alignment.
//
// Arguments:
//
//     VA - Virtual address.
//
//     ALIGNMENT - alignment to round to, must be a power of 2, e.g, 2**n.
//
// Return Value:
//
//     Returns the aligned virtual address.
//
//--

#define MI_ALIGN_TO_SIZE(VA,ALIGNMENT) ((PVOID)((ULONG_PTR)(VA) & ~((ULONG_PTR) ALIGNMENT - 1)))

//++
//
// LONGLONG
// MI_STARTING_OFFSET (
//     IN PSUBSECTION SUBSECT
//     IN PMMPTE PTE
//     )
//
// Routine Description:
//
//    This macro takes a pointer to a PTE within a subsection and a pointer
//    to that subsection and calculates the offset for that PTE within the
//    file.
//
// Arguments:
//
//     PTE - PTE within subsection.
//
//     SUBSECT - Subsection
//
// Return Value:
//
//     Offset for issuing I/O from.
//
//--

#define MI_STARTING_OFFSET(SUBSECT,PTE) \
           (((LONGLONG)((ULONG_PTR)((PTE) - ((SUBSECT)->SubsectionBase))) << PAGE_SHIFT) + \
             ((LONGLONG)((SUBSECT)->StartingSector) << MMSECTOR_SHIFT));


// NTSTATUS
// MiFindEmptyAddressRangeDown (
//    IN ULONG_PTR SizeOfRange,
//    IN PVOID HighestAddressToEndAt,
//    IN ULONG_PTR Alignment,
//    OUT PVOID *Base
//    )
//
//
// Routine Description:
//
//    The function examines the virtual address descriptors to locate
//    an unused range of the specified size and returns the starting
//    address of the range.  This routine looks from the top down.
//
// Arguments:
//
//    SizeOfRange - Supplies the size in bytes of the range to locate.
//
//    HighestAddressToEndAt - Supplies the virtual address to begin looking
//                            at.
//
//    Alignment - Supplies the alignment for the address.  Must be
//                 a power of 2 and greater than the page_size.
//
// Return Value:
//
//    Returns the starting address of a suitable range.
//

#define MiFindEmptyAddressRangeDown(Root,SizeOfRange,HighestAddressToEndAt,Alignment,Base) \
               (MiFindEmptyAddressRangeDownTree(                             \
                    (SizeOfRange),                                           \
                    (HighestAddressToEndAt),                                 \
                    (Alignment),                                             \
                    Root,                                                    \
                    (Base)))

// PMMVAD
// MiGetPreviousVad (
//     IN PMMVAD Vad
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically precedes the specified virtual
//     address descriptor.
//
// Arguments:
//
//     Vad - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     next address range, NULL if none.
//
//

#define MiGetPreviousVad(VAD) ((PMMVAD)MiGetPreviousNode((PMMADDRESS_NODE)(VAD)))


// PMMVAD
// MiGetNextVad (
//     IN PMMVAD Vad
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically follows the specified address range.
//
// Arguments:
//
//     VAD - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     next address range, NULL if none.
//

#define MiGetNextVad(VAD) ((PMMVAD)MiGetNextNode((PMMADDRESS_NODE)(VAD)))



// PMMVAD
// MiGetFirstVad (
//     Process
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically is first within the address space.
//
// Arguments:
//
//     Process - Specifies the process in which to locate the VAD.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     first address range, NULL if none.

#define MiGetFirstVad(Process) \
    ((PMMVAD)MiGetFirstNode(&Process->VadRoot))


LOGICAL
MiCheckForConflictingVadExistence (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

// PMMVAD
// MiCheckForConflictingVad (
//     IN PVOID StartingAddress,
//     IN PVOID EndingAddress
//     )
//
// Routine Description:
//
//     The function determines if any addresses between a given starting and
//     ending address is contained within a virtual address descriptor.
//
// Arguments:
//
//     StartingAddress - Supplies the virtual address to locate a containing
//                       descriptor.
//
//     EndingAddress - Supplies the virtual address to locate a containing
//                       descriptor.
//
// Return Value:
//
//     Returns a pointer to the first conflicting virtual address descriptor
//     if one is found, otherwise a NULL value is returned.
//

#define MiCheckForConflictingVad(CurrentProcess,StartingAddress,EndingAddress) \
    ((PMMVAD)MiCheckForConflictingNode(                                   \
                    MI_VA_TO_VPN(StartingAddress),                        \
                    MI_VA_TO_VPN(EndingAddress),                          \
                    &CurrentProcess->VadRoot))

// PMMCLONE_DESCRIPTOR
// MiGetNextClone (
//     IN PMMCLONE_DESCRIPTOR Clone
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically follows the specified address range.
//
// Arguments:
//
//     Clone - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     next address range, NULL if none.
//
//

#define MiGetNextClone(CLONE) \
 ((PMMCLONE_DESCRIPTOR)MiGetNextNode((PMMADDRESS_NODE)(CLONE)))



// PMMCLONE_DESCRIPTOR
// MiGetPreviousClone (
//     IN PMMCLONE_DESCRIPTOR Clone
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically precedes the specified virtual
//     address descriptor.
//
// Arguments:
//
//     Clone - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     next address range, NULL if none.


#define MiGetPreviousClone(CLONE)  \
             ((PMMCLONE_DESCRIPTOR)MiGetPreviousNode((PMMADDRESS_NODE)(CLONE)))



// PMMCLONE_DESCRIPTOR
// MiGetFirstClone (
//     )
//
// Routine Description:
//
//     This function locates the virtual address descriptor which contains
//     the address range which logically is first within the address space.
//
// Arguments:
//
//     None.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor containing the
//     first address range, NULL if none.
//


#define MiGetFirstClone(_CurrentProcess) \
        (((PMM_AVL_TABLE)(_CurrentProcess->CloneRoot))->NumberGenericTableElements == 0 ? NULL : (PMMCLONE_DESCRIPTOR)MiGetFirstNode((PMM_AVL_TABLE)(_CurrentProcess->CloneRoot)))



// VOID
// MiInsertClone (
//     IN PMMCLONE_DESCRIPTOR Clone
//     )
//
// Routine Description:
//
//     This function inserts a virtual address descriptor into the tree and
//     reorders the splay tree as appropriate.
//
// Arguments:
//
//     Clone - Supplies a pointer to a virtual address descriptor.
//
//
// Return Value:
//
//     None.
//

#define MiInsertClone(_CurrentProcess, CLONE) \
    {                                           \
        ASSERT ((CLONE)->NumberOfPtes != 0);     \
        ASSERT (_CurrentProcess->CloneRoot != NULL); \
        MiInsertNode(((PMMADDRESS_NODE)(CLONE)),(PMM_AVL_TABLE)(_CurrentProcess->CloneRoot)); \
    }




// VOID
// MiRemoveClone (
//     IN PMMCLONE_DESCRIPTOR Clone
//     )
//
// Routine Description:
//
//     This function removes a virtual address descriptor from the tree and
//     reorders the splay tree as appropriate.
//
// Arguments:
//
//     Clone - Supplies a pointer to a virtual address descriptor.
//
// Return Value:
//
//     None.
//

#define MiRemoveClone(_CurrentProcess, CLONE) \
    ASSERT (_CurrentProcess->CloneRoot != NULL); \
    ASSERT (((PMM_AVL_TABLE)_CurrentProcess->CloneRoot)->NumberGenericTableElements != 0); \
    MiRemoveNode((PMMADDRESS_NODE)(CLONE),(PMM_AVL_TABLE)(_CurrentProcess->CloneRoot));



// PMMCLONE_DESCRIPTOR
// MiLocateCloneAddress (
//     IN PVOID VirtualAddress
//     )
//
// /*++
//
// Routine Description:
//
//     The function locates the virtual address descriptor which describes
//     a given address.
//
// Arguments:
//
//     VirtualAddress - Supplies the virtual address to locate a descriptor
//                      for.
//
// Return Value:
//
//     Returns a pointer to the virtual address descriptor which contains
//     the supplied virtual address or NULL if none was located.
//

#define MiLocateCloneAddress(_CurrentProcess, VA)                           \
    (_CurrentProcess->CloneRoot ?                                           \
        ((PMMCLONE_DESCRIPTOR)MiLocateAddressInTree(((ULONG_PTR)VA),        \
                   (PMM_AVL_TABLE)(_CurrentProcess->CloneRoot))) :     \
        NULL)


#define MI_VA_TO_PAGE(va) ((ULONG_PTR)(va) >> PAGE_SHIFT)

#define MI_VA_TO_VPN(va)  ((ULONG_PTR)(va) >> PAGE_SHIFT)

#define MI_VPN_TO_VA(vpn)  (PVOID)((vpn) << PAGE_SHIFT)

#define MI_VPN_TO_VA_ENDING(vpn)  (PVOID)(((vpn) << PAGE_SHIFT) | (PAGE_SIZE - 1))

#define MiGetByteOffset(va) ((ULONG_PTR)(va) & (PAGE_SIZE - 1))

//
// Make a write-copy PTE, only writable.
//

#define MI_MAKE_PROTECT_NOT_WRITE_COPY(PROTECT) \
            (MmMakeProtectNotWriteCopy[PROTECT])

//
// Define macros to lock and unlock the PFN database.
//

#define MiLockPfnDatabase(OldIrql) \
    OldIrql = KeAcquireQueuedSpinLock(LockQueuePfnLock)

#define MiUnlockPfnDatabase(OldIrql) \
    KeReleaseQueuedSpinLock(LockQueuePfnLock, OldIrql)

#define MiLockPfnDatabaseAtDpcLevel() \
    KeAcquireQueuedSpinLockAtDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueuePfnLock])

#define MiUnlockPfnDatabaseFromDpcLevel() \
    KeReleaseQueuedSpinLockFromDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueuePfnLock])

#define MiReleasePfnLock() \
    KeReleaseQueuedSpinLockFromDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueuePfnLock])

#define MiLockSystemSpace(OldIrql) \
    OldIrql = KeAcquireQueuedSpinLock(LockQueueSystemSpaceLock)

#define MiUnlockSystemSpace(OldIrql) \
    KeReleaseQueuedSpinLock(LockQueueSystemSpaceLock, OldIrql)

#define MiLockSystemSpaceAtDpcLevel() \
    KeAcquireQueuedSpinLockAtDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueueSystemSpaceLock])

#define MiUnlockSystemSpaceFromDpcLevel() \
    KeReleaseQueuedSpinLockFromDpcLevel(&KeGetCurrentPrcb()->LockQueue[LockQueueSystemSpaceLock])

#define MI_MAX_PFN_CALLERS  500

typedef struct _MMPFNTIMINGS {
    LARGE_INTEGER HoldTime;     // Low bit is set if another processor waited
    PVOID AcquiredAddress;
    PVOID ReleasedAddress;
} MMPFNTIMINGS, *PMMPFNTIMINGS;

extern ULONG MiPfnTimings;
extern PVOID MiPfnAcquiredAddress;
extern MMPFNTIMINGS MiPfnSorted[];
extern LARGE_INTEGER MiPfnAcquired;
extern LARGE_INTEGER MiPfnReleased;
extern LARGE_INTEGER MiPfnThreshold;

PVOID
MiGetExecutionAddress (
    VOID
    );

LARGE_INTEGER
MiQueryPerformanceCounter (
    IN PLARGE_INTEGER PerformanceFrequency 
    );

VOID
MiAddLockToTable (
    IN PVOID AcquireAddress,
    IN PVOID ReleaseAddress,
    IN LARGE_INTEGER HoldTime
    );

#if defined(_X86_) || defined(_AMD64_)
#define MI_GET_EXECUTION_ADDRESS(varname) varname = MiGetExecutionAddress();
#else
#define MI_GET_EXECUTION_ADDRESS(varname) varname = NULL;
#endif

#define LOCK_PFN_TIMESTAMP()
#define UNLOCK_PFN_TIMESTAMP()

#if DBG

PKTHREAD MmPfnOwner;

#define MI_SET_PFN_OWNER() ASSERT (MmPfnOwner == NULL);                     \
                          MmPfnOwner = KeGetCurrentThread ();

#define MI_CLEAR_PFN_OWNER() ASSERT (MmPfnOwner == KeGetCurrentThread ());  \
                             MmPfnOwner = NULL;

#else
#define MI_SET_PFN_OWNER()
#define MI_CLEAR_PFN_OWNER()
#endif

#define LOCK_PFN(OLDIRQL) ASSERT (KeGetCurrentIrql() <= APC_LEVEL);         \
                          MiLockPfnDatabase(OLDIRQL);                       \
                          MI_SET_PFN_OWNER ();                              \
                          LOCK_PFN_TIMESTAMP();

#define UNLOCK_PFN(OLDIRQL)                                                 \
    ASSERT (OLDIRQL <= APC_LEVEL);                                          \
    MI_CLEAR_PFN_OWNER ();                                                  \
    MI_CHECK_TB ();                                                         \
    UNLOCK_PFN_TIMESTAMP();                                                 \
    MiUnlockPfnDatabase(OLDIRQL);                                           \
    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

#define LOCK_PFN2(OLDIRQL) ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);   \
                          MiLockPfnDatabase(OLDIRQL);                       \
                          MI_SET_PFN_OWNER ();                              \
                          LOCK_PFN_TIMESTAMP();

#define UNLOCK_PFN2(OLDIRQL)                                                \
    ASSERT (OLDIRQL <= DISPATCH_LEVEL);                                     \
    MI_CLEAR_PFN_OWNER ();                                                  \
    MI_CHECK_TB ();                                                         \
    UNLOCK_PFN_TIMESTAMP();                                                 \
    MiUnlockPfnDatabase(OLDIRQL);                                           \
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

#define LOCK_PFN_AT_DPC() ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);    \
                          MiLockPfnDatabaseAtDpcLevel();                    \
                          MI_SET_PFN_OWNER ();                              \
                          LOCK_PFN_TIMESTAMP();

#define UNLOCK_PFN_FROM_DPC()                                               \
    UNLOCK_PFN_TIMESTAMP();                                                 \
    MI_CLEAR_PFN_OWNER ();                                                  \
    MI_CHECK_TB ();                                                         \
    MiUnlockPfnDatabaseFromDpcLevel();                                      \
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

#define UNLOCK_PFN_AND_THEN_WAIT(OLDIRQL)                                   \
                {                                                           \
                    KIRQL XXX;                                              \
                    PKTHREAD THREAD;                                        \
                    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);          \
                    ASSERT (OLDIRQL <= APC_LEVEL);                          \
                    KiLockDispatcherDatabase (&XXX);                        \
                    MI_CLEAR_PFN_OWNER ();                                  \
                    MI_CHECK_TB ();                                         \
                    UNLOCK_PFN_TIMESTAMP();                                 \
                    MiReleasePfnLock();                                     \
                    THREAD = KeGetCurrentThread ();                         \
                    THREAD->WaitIrql = OLDIRQL;                             \
                    THREAD->WaitNext = TRUE;                                \
                }

extern KMUTANT MmSystemLoadLock;

#if DBG
#define SYSLOAD_LOCK_OWNED_BY_ME()      ASSERT (MmSystemLoadLock.OwnerThread == KeGetCurrentThread())
#else
#define SYSLOAD_LOCK_OWNED_BY_ME()
#endif

#if DBG

#define MM_PFN_LOCK_ASSERT() { \
        KIRQL _OldIrql; \
        _OldIrql = KeGetCurrentIrql(); \
        ASSERT (_OldIrql == DISPATCH_LEVEL); \
        ASSERT (MmPfnOwner == KeGetCurrentThread()); \
    }

extern PETHREAD MiExpansionLockOwner;

#define MM_SET_EXPANSION_OWNER()  ASSERT (MiExpansionLockOwner == NULL); \
                                  MiExpansionLockOwner = PsGetCurrentThread();

#define MM_CLEAR_EXPANSION_OWNER()  ASSERT (MiExpansionLockOwner == PsGetCurrentThread()); \
                                    MiExpansionLockOwner = NULL;

#else
#define MM_PFN_LOCK_ASSERT()
#define MM_SET_EXPANSION_OWNER()
#define MM_CLEAR_EXPANSION_OWNER()
#endif // DBG


#define LOCK_EXPANSION(OLDIRQL)     ASSERT (KeGetCurrentIrql() <= APC_LEVEL); \
                                ExAcquireSpinLock (&MmExpansionLock, &OLDIRQL);\
                                MM_SET_EXPANSION_OWNER ();

#define UNLOCK_EXPANSION(OLDIRQL)    MM_CLEAR_EXPANSION_OWNER (); \
                                ExReleaseSpinLock (&MmExpansionLock, OLDIRQL); \
                                ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

#define LOCK_EXPANSION2(OLDIRQL)     ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL); \
                                ExAcquireSpinLock (&MmExpansionLock, &OLDIRQL);\
                                MM_SET_EXPANSION_OWNER ();

#define UNLOCK_EXPANSION2(OLDIRQL)    MM_CLEAR_EXPANSION_OWNER (); \
                                ExReleaseSpinLock (&MmExpansionLock, OLDIRQL); \
                                ASSERT (KeGetCurrentIrql() <= DISPATCH_LEVEL);

#define LOCK_WS_TIMESTAMP(PROCESS)
#define UNLOCK_WS_TIMESTAMP(PROCESS)

#define MM_WS_LOCK_ASSERT(WSINFO)                               \
        if (WSINFO == &MmSystemCacheWs) {                       \
            ASSERT ((PsGetCurrentThread ()->OwnsSystemWorkingSetExclusive) || \
                    (PsGetCurrentThread ()->OwnsSystemWorkingSetShared)); \
        }           \
        else if ((WSINFO)->Flags.SessionSpace == 1) {      \
            ASSERT ((PsGetCurrentThread ()->OwnsSessionWorkingSetExclusive) || \
                    (PsGetCurrentThread ()->OwnsSessionWorkingSetShared)); \
        }           \
        else {      \
            ASSERT ((PsGetCurrentThread ()->OwnsProcessWorkingSetExclusive) || \
                    (PsGetCurrentThread ()->OwnsProcessWorkingSetShared)); \
        }

//
// System working set synchronization definitions.
//

#define MM_ANY_WS_LOCK_HELD(THREAD)                                          \
        (THREAD->SameThreadApcFlags & PS_SAME_THREAD_FLAGS_OWNS_A_WORKING_SET)

#define MM_SYSTEM_WS_LOCK_TIMESTAMP()                           \
        LOCK_WS_TIMESTAMP(((PEPROCESS)&MiSystemCacheDummyProcess));

#define MM_SYSTEM_WS_UNLOCK_TIMESTAMP()                         \
        UNLOCK_WS_TIMESTAMP(((PEPROCESS)&MiSystemCacheDummyProcess));

#define LOCK_SYSTEM_WS(THREAD)                                              \
            ASSERT (PsGetCurrentThread() == THREAD);                        \
            KeEnterGuardedRegionThread (&THREAD->Tcb);                      \
            ASSERT (!MM_ANY_WS_LOCK_HELD(THREAD));                          \
            ExAcquirePushLockExclusive (&MmSystemCacheWs.WorkingSetMutex);  \
            ASSERT (THREAD->OwnsSystemWorkingSetShared == 0);               \
            ASSERT (THREAD->OwnsSystemWorkingSetExclusive == 0);            \
            THREAD->OwnsSystemWorkingSetExclusive = 1;                      \
            MM_SYSTEM_WS_LOCK_TIMESTAMP();

#define LOCK_SYSTEM_WS_SHARED(THREAD)                                       \
            ASSERT (PsGetCurrentThread() == THREAD);                        \
            KeEnterGuardedRegionThread (&THREAD->Tcb);                      \
            ASSERT (!MM_ANY_WS_LOCK_HELD(THREAD));                          \
            ExAcquirePushLockShared (&MmSystemCacheWs.WorkingSetMutex);     \
            ASSERT (THREAD->OwnsSystemWorkingSetShared == 0);               \
            ASSERT (THREAD->OwnsSystemWorkingSetExclusive == 0);            \
            THREAD->OwnsSystemWorkingSetShared = 1;                         \
            MM_SYSTEM_WS_LOCK_TIMESTAMP();

#define UNLOCK_SYSTEM_WS(THREAD)                                            \
            MM_SYSTEM_WS_UNLOCK_TIMESTAMP();                                \
            ASSERT (THREAD->OwnsSystemWorkingSetExclusive == 1);            \
            THREAD->OwnsSystemWorkingSetExclusive = 0;                      \
            ExReleasePushLockExclusive (&MmSystemCacheWs.WorkingSetMutex);  \
            KeLeaveGuardedRegionThread (&THREAD->Tcb);

#define UNLOCK_SYSTEM_WS_SHARED(THREAD)                                     \
            MM_SYSTEM_WS_UNLOCK_TIMESTAMP();                                \
            ASSERT (THREAD->OwnsSystemWorkingSetShared == 1);               \
            ASSERT (THREAD->OwnsSystemWorkingSetExclusive == 0);            \
            THREAD->OwnsSystemWorkingSetShared = 0;                         \
            ExReleasePushLockShared (&MmSystemCacheWs.WorkingSetMutex);     \
            KeLeaveGuardedRegionThread (&THREAD->Tcb);

//
// Generic working set synchronization definitions.
//

#define LOCK_WORKING_SET(THREAD, WSINFO)                                \
            KeEnterGuardedRegionThread (&THREAD->Tcb);                  \
            ASSERT (MI_IS_SESSION_ADDRESS(WSINFO) == FALSE);            \
            ASSERT (!MM_ANY_WS_LOCK_HELD(THREAD));                          \
            ExAcquirePushLockExclusive (&(WSINFO)->WorkingSetMutex);    \
            if (WSINFO == &MmSystemCacheWs) {                           \
                ASSERT ((THREAD->OwnsSystemWorkingSetExclusive == 0) && \
                        (THREAD->OwnsSystemWorkingSetShared == 0));     \
                THREAD->OwnsSystemWorkingSetExclusive = 1;              \
            }                                                           \
            else if ((WSINFO)->Flags.SessionSpace == 1) {               \
                ASSERT ((THREAD->OwnsSessionWorkingSetExclusive == 0) && \
                        (THREAD->OwnsSessionWorkingSetShared == 0));    \
                THREAD->OwnsSessionWorkingSetExclusive = 1;             \
            }                                                           \
            else {                                                      \
                ASSERT ((THREAD->OwnsProcessWorkingSetExclusive == 0) && \
                        (THREAD->OwnsProcessWorkingSetShared == 0));    \
                THREAD->OwnsProcessWorkingSetExclusive = 1;             \
            }

#define UNLOCK_WORKING_SET(THREAD, WSINFO)                              \
            ASSERT (MI_IS_SESSION_ADDRESS(WSINFO) == FALSE);            \
            MM_WS_LOCK_ASSERT(WSINFO);                                  \
            if (WSINFO == &MmSystemCacheWs) {                           \
                THREAD->OwnsSystemWorkingSetExclusive = 0;              \
            }                                                           \
            else if ((WSINFO)->Flags.SessionSpace == 1) {               \
                THREAD->OwnsSessionWorkingSetExclusive = 0;             \
            }                                                           \
            else {                                                      \
                THREAD->OwnsProcessWorkingSetExclusive = 0;             \
            }                                                           \
            ExReleasePushLockExclusive (&(WSINFO)->WorkingSetMutex);    \
            KeLeaveGuardedRegionThread (&THREAD->Tcb);

//
// Session working set synchronization definitions.
//

#define MM_SESSION_SPACE_WS_LOCK_ASSERT()                               \
            MM_WS_LOCK_ASSERT (&MmSessionSpace->Vm);

//
// Process working set synchronization definitions.
//

#define MI_WS_OWNER(PROCESS) (KeGetCurrentThread()->ApcState.Process == (PKPROCESS)(PROCESS) && ((PsGetCurrentThread ()->OwnsProcessWorkingSetExclusive) || (PsGetCurrentThread ()->OwnsProcessWorkingSetShared)))

#define MI_NOT_WS_OWNER(PROCESS) (!MI_WS_OWNER(PROCESS))

#define MI_IS_WS_UNSAFE(PROCESS) ((PROCESS)->Vm.Flags.AcquiredUnsafe == 1)

#define LOCK_WS(THREAD, PROCESS)                                        \
            ASSERT (THREAD->OwnsProcessWorkingSetShared == 0);          \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);       \
            KeEnterGuardedRegionThread (&THREAD->Tcb);                  \
            ASSERT (!MM_ANY_WS_LOCK_HELD(THREAD));                      \
            ExAcquirePushLockExclusive (&((PROCESS)->Vm.WorkingSetMutex));   \
            LOCK_WS_TIMESTAMP(PROCESS);                                 \
            ASSERT (!MI_IS_WS_UNSAFE(PROCESS));                         \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);       \
            THREAD->OwnsProcessWorkingSetExclusive = 1;

#define LOCK_WS_SHARED(THREAD,PROCESS)                                  \
            ASSERT (THREAD->OwnsProcessWorkingSetShared == 0);          \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);       \
            KeEnterGuardedRegionThread (&THREAD->Tcb);                  \
            ASSERT (!MM_ANY_WS_LOCK_HELD(THREAD));                          \
            ExAcquirePushLockShared (&((PROCESS)->Vm.WorkingSetMutex)); \
            LOCK_WS_TIMESTAMP(PROCESS);                                 \
            ASSERT (!MI_IS_WS_UNSAFE(PROCESS));                         \
            ASSERT (THREAD->OwnsProcessWorkingSetShared == 0);          \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);       \
            THREAD->OwnsProcessWorkingSetShared = 1;

#define LOCK_WS_SHARED_CROSS_PROCESS(THREAD,PROCESS)                    \
            ASSERT (THREAD->OwnsProcessWorkingSetShared == 0);          \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);       \
            KeEnterGuardedRegionThread (&THREAD->Tcb);                  \
            ASSERT (!MM_ANY_WS_LOCK_HELD(THREAD));                          \
            ExAcquirePushLockShared (&((PROCESS)->Vm.WorkingSetMutex)); \
            ASSERT (THREAD->OwnsProcessWorkingSetShared == 0);          \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);

#define LOCK_WS_UNSAFE(THREAD, PROCESS)                                 \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);       \
            ASSERT (KeAreAllApcsDisabled () == TRUE);                   \
            ASSERT (!MM_ANY_WS_LOCK_HELD(THREAD));                          \
            ExAcquirePushLockExclusive (&((PROCESS)->Vm.WorkingSetMutex));\
            LOCK_WS_TIMESTAMP(PROCESS);                                 \
            ASSERT (!MI_IS_WS_UNSAFE(PROCESS));                         \
            (PROCESS)->Vm.Flags.AcquiredUnsafe = 1;                     \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);       \
            THREAD->OwnsProcessWorkingSetExclusive = 1;

#define MI_MUST_BE_UNSAFE(PROCESS)                                      \
            ASSERT (KeGetCurrentIrql() <= APC_LEVEL);                   \
            ASSERT (KeAreAllApcsDisabled () == TRUE);                   \
            ASSERT (MI_WS_OWNER(PROCESS));                              \
            ASSERT (MI_IS_WS_UNSAFE(PROCESS));

#define MI_MUST_BE_SAFE(PROCESS)                                        \
            ASSERT (MI_WS_OWNER(PROCESS));                              \
            ASSERT (!MI_IS_WS_UNSAFE(PROCESS));

#define UNLOCK_WS(THREAD, PROCESS)                                      \
            MI_MUST_BE_SAFE(PROCESS);                                   \
            UNLOCK_WS_TIMESTAMP(PROCESS);                               \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 1);       \
            THREAD->OwnsProcessWorkingSetExclusive = 0;                 \
            ExReleasePushLockExclusive (&((PROCESS)->Vm.WorkingSetMutex)); \
            KeLeaveGuardedRegionThread (&THREAD->Tcb);

#define UNLOCK_WS_SHARED(THREAD,PROCESS)                                \
            MI_MUST_BE_SAFE(PROCESS);                                   \
            UNLOCK_WS_TIMESTAMP(PROCESS);                               \
            ASSERT (THREAD->OwnsProcessWorkingSetShared == 1);          \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);       \
            THREAD->OwnsProcessWorkingSetShared = 0;                    \
            ExReleasePushLockShared (&((PROCESS)->Vm.WorkingSetMutex)); \
            KeLeaveGuardedRegionThread (&THREAD->Tcb);

#define UNLOCK_WS_SHARED_CROSS_PROCESS(THREAD,PROCESS)                  \
            ASSERT (THREAD->OwnsProcessWorkingSetShared == 0);          \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 0);       \
            ExReleasePushLockShared (&((PROCESS)->Vm.WorkingSetMutex)); \
            KeLeaveGuardedRegionThread (&THREAD->Tcb);

#define UNLOCK_WS_UNSAFE(THREAD, PROCESS)                               \
            MI_MUST_BE_UNSAFE(PROCESS);                                 \
            ASSERT (KeAreAllApcsDisabled () == TRUE);                   \
            (PROCESS)->Vm.Flags.AcquiredUnsafe = 0;                     \
            UNLOCK_WS_TIMESTAMP(PROCESS);                               \
            ASSERT (THREAD->OwnsProcessWorkingSetExclusive == 1);       \
            THREAD->OwnsProcessWorkingSetExclusive = 0;                 \
            ExReleasePushLockExclusive(&((PROCESS)->Vm.WorkingSetMutex)); \
            ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

//
// Address space synchronization definitions.
//

#define LOCK_ADDRESS_SPACE(PROCESS)                                  \
            KeAcquireGuardedMutex (&((PROCESS)->AddressCreationLock));

#define LOCK_WS_AND_ADDRESS_SPACE(THREAD, PROCESS)                  \
        LOCK_ADDRESS_SPACE(PROCESS);                                \
        LOCK_WS_UNSAFE(THREAD, PROCESS)

#define UNLOCK_WS_AND_ADDRESS_SPACE(THREAD, PROCESS)                \
        UNLOCK_WS_UNSAFE(THREAD, PROCESS);                          \
        UNLOCK_ADDRESS_SPACE(PROCESS);

#define UNLOCK_ADDRESS_SPACE(PROCESS)                               \
            KeReleaseGuardedMutex (&((PROCESS)->AddressCreationLock));

//
// The working set lock may have been acquired safely or unsafely.
// Release and reacquire it regardless.
//

#define UNLOCK_WS_REGARDLESS(THREAD, PROCESS, WSHELDSAFE, WSHELDSHARED) \
            ASSERT (MI_WS_OWNER (PROCESS));                         \
            if (MI_IS_WS_UNSAFE (PROCESS)) {                        \
                UNLOCK_WS_UNSAFE (THREAD, PROCESS);                 \
                WSHELDSAFE = FALSE;                                 \
                WSHELDSHARED = FALSE;                               \
            }                                                       \
            else if (THREAD->OwnsProcessWorkingSetExclusive == 1) { \
                UNLOCK_WS (THREAD, PROCESS);                        \
                WSHELDSAFE = TRUE;                                  \
                WSHELDSHARED = FALSE;                               \
            }                                                       \
            else {                                                  \
                UNLOCK_WS_SHARED (THREAD, PROCESS);                 \
                WSHELDSAFE = TRUE;                                  \
                WSHELDSHARED = TRUE;                                \
            }

#define LOCK_WS_REGARDLESS(THREAD, PROCESS, WSHELDSAFE, WSHELDSHARED) \
            if (WSHELDSAFE == TRUE) {                               \
                if (WSHELDSHARED == FALSE) {                        \
                    LOCK_WS (THREAD, PROCESS);                      \
                }                                                   \
                else {                                              \
                    LOCK_WS_SHARED (THREAD, PROCESS);               \
                }                                                   \
            }                                                       \
            else {                                                  \
                LOCK_WS_UNSAFE (THREAD, PROCESS);                   \
            }

//
// Hyperspace synchronization definitions.
//

#define MI_HYPER_ADDRESS(VA)   TRUE

#define LOCK_HYPERSPACE(_Process, OLDIRQL)                      \
    ASSERT (_Process == PsGetCurrentProcess ());                \
    ExAcquireSpinLock (&_Process->HyperSpaceLock, OLDIRQL);

#define UNLOCK_HYPERSPACE(_Process, VA, OLDIRQL)                \
    ASSERT (_Process == PsGetCurrentProcess ());                \
    if (OLDIRQL != MM_NOIRQL) {                                 \
        ASSERT (MI_HYPER_ADDRESS (VA));                         \
        MiGetPteAddress(VA)->u.Long = 0;                        \
        ExReleaseSpinLock (&_Process->HyperSpaceLock, OLDIRQL); \
    }                                                           \
    else {                                                      \
        ASSERT (!MI_HYPER_ADDRESS (VA));                        \
    }

#define LOCK_HYPERSPACE_AT_DPC(_Process)                        \
    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);              \
    ASSERT (_Process == PsGetCurrentProcess ());                \
    ExAcquireSpinLockAtDpcLevel (&_Process->HyperSpaceLock);

#define UNLOCK_HYPERSPACE_FROM_DPC(_Process, VA)                \
    ASSERT (_Process == PsGetCurrentProcess ());                \
    if (MI_HYPER_ADDRESS (VA)) {                                \
        ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);          \
        MiGetPteAddress(VA)->u.Long = 0;                        \
        ExReleaseSpinLockFromDpcLevel (&_Process->HyperSpaceLock); \
    }

#define MiUnmapPageInHyperSpace(_Process, VA, OLDIRQL) UNLOCK_HYPERSPACE(_Process, VA, OLDIRQL)

#define MiUnmapPageInHyperSpaceFromDpc(_Process, VA) UNLOCK_HYPERSPACE_FROM_DPC(_Process, VA)


//
// TB flushing and debugging definitions.
//

#if DBG

#define MI_TB_FLUSH_TYPE_MAX    64
extern ULONG MiFlushType[MI_TB_FLUSH_TYPE_MAX];

#define MI_INCREMENT_TB_FLUSH_TYPE(CallerId)                            \
            ASSERT (CallerId < MI_TB_FLUSH_TYPE_MAX);                   \
            MiFlushType[CallerId] += 1;
#else
#define MI_INCREMENT_TB_FLUSH_TYPE(CallerId)
#endif

#define MI_FLUSH_ENTIRE_TB(CallerId)                                    \
        MI_INCREMENT_TB_FLUSH_TYPE(CallerId);                           \
        KeFlushEntireTb (TRUE, TRUE);

#define MI_FLUSH_MULTIPLE_TB(Number, Virtual, AllProcessors)            \
        KeFlushMultipleTb (Number, Virtual, AllProcessors);

#define MI_FLUSH_SINGLE_TB(Virtual, AllProcessors)                      \
        KeFlushSingleTb (Virtual, AllProcessors);

#define MI_FLUSH_PROCESS_TB(AllProcessors)                              \
        ASSERT (AllProcessors == FALSE);                                \
        KeFlushProcessTb ();

#define MI_FLUSH_CURRENT_TB()                                           \
        KeFlushCurrentTb ();

//
// There is no flush single on just the current processor so specify FALSE
// and hope we don't IPI too many others.
//

#define MI_FLUSH_CURRENT_TB_SINGLE(Virtual)                             \
        KeFlushSingleTb (Virtual, FALSE);

#define MI_CHECK_TB()
#define MI_INSERT_VALID_PTE(PointerPte)
#define MI_REMOVE_PTE(PointerPte)


#define ZERO_LARGE(LargeInteger)                \
        (LargeInteger).LowPart = 0;             \
        (LargeInteger).HighPart = 0;

#define NO_BITS_FOUND   ((ULONG)-1)

//++
//
// ULONG
// MI_CHECK_BIT (
//     IN PULONG_PTR ARRAY
//     IN ULONG BIT
//     )
//
// Routine Description:
//
//     The MI_CHECK_BIT macro checks to see if the specified bit is
//     set within the specified array.
//
//     Note that for NT64, the ARRAY must be 8-byte aligned as the intrinsics
//     rely on this.
//
// Arguments:
//
//     ARRAY - First element of the array to check.
//
//     BIT - bit number (first bit is 0) to check.
//
// Return Value:
//
//     Returns the value of the bit (0 or 1).
//
//--

#if _MI_USE_BIT_INTRINSICS
#define MI_CHECK_BIT(ARRAY,BIT)  \
        BitTest64 ((LONG64 *)ARRAY, BIT)
#else
#define MI_CHECK_BIT(ARRAY,BIT)  \
        (((ULONG)(((PULONG)ARRAY)[(BIT) / (sizeof(ULONG)*8)]) >> ((BIT) & 0x1F)) & 1)
#endif


//++
//
// VOID
// MI_SET_BIT (
//     IN PULONG_PTR ARRAY
//     IN ULONG BIT
//     )
//
// Routine Description:
//
//     The MI_SET_BIT macro sets the specified bit within the
//     specified array.
//
//     Note that for NT64, the ARRAY must be 8-byte aligned as the intrinsics
//     rely on this.
//
// Arguments:
//
//     ARRAY - First element of the array to set.
//
//     BIT - bit number.
//
// Return Value:
//
//     None.
//
//--

#if _MI_USE_BIT_INTRINSICS
#define MI_SET_BIT(ARRAY,BIT)  \
        BitTestAndSet64 ((LONG64 *)ARRAY, BIT)
#else
#define MI_SET_BIT(ARRAY,BIT)  \
        ((PULONG)ARRAY)[(BIT) / (sizeof(ULONG)*8)] |= (1 << ((BIT) & 0x1F))
#endif


//++
//
// VOID
// MI_CLEAR_BIT (
//     IN PULONG_PTR ARRAY
//     IN ULONG BIT
//     )
//
// Routine Description:
//
//     The MI_CLEAR_BIT macro sets the specified bit within the
//     specified array.
//
//     Note that for NT64, the ARRAY must be 8-byte aligned as the intrinsics
//     rely on this.
//
// Arguments:
//
//     ARRAY - First element of the array to clear.
//
//     BIT - bit number.
//
// Return Value:
//
//     None.
//
//--

#if _MI_USE_BIT_INTRINSICS
#define MI_CLEAR_BIT(ARRAY,BIT)  \
        BitTestAndReset64 ((LONG64 *)ARRAY, BIT)
#else
#define MI_CLEAR_BIT(ARRAY,BIT)  \
        ((PULONG)ARRAY)[(BIT) / (sizeof(ULONG)*8)] &= ~(1 << ((BIT) & 0x1F))
#endif

#if defined (_WIN64)
#define MI_MAGIC_AWE_PTEFRAME   0x1ffedcbffffedcb
#else
#define MI_MAGIC_AWE_PTEFRAME   0x1ffedcb
#endif

#define MI_PFN_IS_AWE(Pfn1)                                     \
        ((Pfn1->u2.ShareCount <= 3) &&                          \
         (Pfn1->u3.e1.PageLocation == ActiveAndValid) &&        \
         (Pfn1->u4.VerifierAllocation == 0) &&                  \
         (Pfn1->u3.e1.LargeSessionAllocation == 0) &&           \
         (Pfn1->u3.e1.StartOfAllocation == 1) &&                \
         (Pfn1->u3.e1.EndOfAllocation == 1) &&                  \
         (Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME))

//
// The cache type definitions are carefully chosen to line up with the
// MEMORY_CACHING_TYPE definitions to ease conversions.  Any changes here must
// be reflected throughout the code.
//

typedef enum _MI_PFN_CACHE_ATTRIBUTE {
    MiNonCached,
    MiCached,
    MiWriteCombined,
    MiNotMapped
} MI_PFN_CACHE_ATTRIBUTE, *PMI_PFN_CACHE_ATTRIBUTE;

//
// This conversion array is unfortunately needed because not all
// hardware platforms support all possible cache values.  Note that
// the first range is for system RAM, the second range is for I/O space.
//

extern MI_PFN_CACHE_ATTRIBUTE MiPlatformCacheAttributes[2 * MmMaximumCacheType];

//++
//
// MI_PFN_CACHE_ATTRIBUTE
// MI_TRANSLATE_CACHETYPE (
//     IN MEMORY_CACHING_TYPE InputCacheType,
//     IN ULONG IoSpace
//     )
//
// Routine Description:
//
//     Returns the hardware supported cache type for the requested cachetype.
//
// Arguments:
//
//     InputCacheType - Supplies the desired cache type.
//
//     IoSpace - Supplies nonzero (not necessarily 1 though) if this is
//               I/O space, zero if it is main memory.
//
// Return Value:
//
//     The actual cache type.
//
//--
FORCEINLINE
MI_PFN_CACHE_ATTRIBUTE
MI_TRANSLATE_CACHETYPE(
    IN MEMORY_CACHING_TYPE InputCacheType,
    IN ULONG IoSpace
    )
{
    ASSERT (InputCacheType <= MmWriteCombined);

    if (IoSpace != 0) {
        IoSpace = MmMaximumCacheType;
    }
    return MiPlatformCacheAttributes[IoSpace + InputCacheType];
}

//++
//
// VOID
// MI_SET_CACHETYPE_TRANSLATION (
//     IN MEMORY_CACHING_TYPE    InputCacheType,
//     IN ULONG                  IoSpace,
//     IN MI_PFN_CACHE_ATTRIBUTE NewAttribute
//     )
//
// Routine Description:
//
//     This function sets the hardware supported cachetype for the
//     specified cachetype.
//
// Arguments:
//
//     InputCacheType - Supplies the desired cache type.
//
//     IoSpace - Supplies nonzero (not necessarily 1 though) if this is
//               I/O space, zero if it is main memory.
//
//     NewAttribute - Supplies the desired attribute.
//
// Return Value:
//
//     None.
//
//--
FORCEINLINE
VOID
MI_SET_CACHETYPE_TRANSLATION(
    IN MEMORY_CACHING_TYPE    InputCacheType,
    IN ULONG                  IoSpace,
    IN MI_PFN_CACHE_ATTRIBUTE NewAttribute
    )
{
    ASSERT (InputCacheType <= MmWriteCombined);

    if (IoSpace != 0) {
        IoSpace = MmMaximumCacheType;
    }

    MiPlatformCacheAttributes[IoSpace + InputCacheType] = NewAttribute;
}

//
// PFN database element.
//

//
// Define pseudo fields for start and end of allocation.
//

#define StartOfAllocation ReadInProgress

#define EndOfAllocation WriteInProgress

#define LargeSessionAllocation PrototypePte

typedef struct _MMPFNENTRY {
    USHORT Modified : 1;
    USHORT ReadInProgress : 1;
    USHORT WriteInProgress : 1;
    USHORT PrototypePte: 1;
    USHORT PageColor : 4;
    USHORT PageLocation : 3;
    USHORT RemovalRequested : 1;

    //
    // The CacheAttribute field is what the current (or last) TB attribute used.
    // This is not cleared when the frame is unmapped because in cases like
    // system PTE mappings for MDLs, we lazy flush the TB (for performance)
    // so we don't know when the final TB flush has really occurred.  To ensure
    // that we never insert a TB that would cause an overlapping condition,
    // whenever a PTE is filled with a freshly allocated frame, the filler
    // assumes responsibility for comparing the new cache attribute with the
    // last known used attribute and if they differ, the filler must flush the
    // entire TB.  KeFlushSingleTb cannot be used because we don't know
    // what virtual address(es) this physical frame was last mapped at.
    //
    // This may result in some TB overflushing in the case
    // where no system PTE was used so it would have been safe to clear the
    // attribute in the PFN, but it reusing a page with a differing attribute
    // should be a fairly rare case anyway.
    //

    USHORT CacheAttribute : 2;
    USHORT Rom : 1;
    USHORT ParityError : 1;
} MMPFNENTRY;

#define MI_PFN_PRIORITY_BITS    3
#define MI_PFN_PRIORITIES       (1 << MI_PFN_PRIORITY_BITS)
#define MI_DEFAULT_PFN_PRIORITY 3

#define MI_SET_PFN_PRIORITY(Pfn, NewPriority)               \
    ASSERT (NewPriority < MI_PFN_PRIORITIES);               \
    Pfn->u4.Priority = NewPriority;

#define MI_GET_PFN_PRIORITY(Pfn)                            \
    ((ULONG)(Pfn)->u4.Priority)

//
// Reset the PFN priority to the default when a page is reused.
// 

#define MI_RESET_PFN_PRIORITY(Pfn)                      \
    MI_SET_PFN_PRIORITY(Pfn, MI_DEFAULT_PFN_PRIORITY);

#if defined (_X86PAE_)
#pragma pack(1)
#endif

typedef struct _MMPFN {
    union {
        PFN_NUMBER Flink;
        WSLE_NUMBER WsIndex;
        PKEVENT Event;
        NTSTATUS ReadStatus;

        //
        // Note: NextStackPfn is actually used as SLIST_ENTRY, however
        // because of its alignment characteristics, using that type would
        // unnecessarily add padding to this structure.
        //

        SINGLE_LIST_ENTRY NextStackPfn;
    } u1;
    PMMPTE PteAddress;
    union {
        PFN_NUMBER Blink;

        //
        // ShareCount transitions are protected by the PFN lock.
        //

        ULONG_PTR ShareCount;
    } u2;
    union {

        //
        // ReferenceCount transitions are generally done with InterlockedXxxPfn
        // sequences, and only the 0->1 and 1->0 transitions are protected
        // by the PFN lock.  Note that a *VERY* intricate synchronization
        // scheme is being used to maximize scalability.
        //

        struct {
            USHORT ReferenceCount;
            MMPFNENTRY e1;
        };
        struct {
            USHORT ReferenceCount;
            USHORT ShortFlags;
        } e2;
    } u3;
#if defined (_WIN64)
    ULONG UsedPageTableEntries;
#endif
    union {
        MMPTE OriginalPte;
        LONG AweReferenceCount;
    };
    union {
        ULONG_PTR EntireFrame;
        struct {
#if defined (_WIN64)
            ULONG_PTR PteFrame: 57;
#else
            ULONG_PTR PteFrame: 25;
#endif
            ULONG_PTR InPageError : 1;
            ULONG_PTR VerifierAllocation : 1;
            ULONG_PTR AweAllocation : 1;
            ULONG_PTR Priority : MI_PFN_PRIORITY_BITS;
            ULONG_PTR MustBeCached : 1;
        };
    } u4;

} MMPFN, *PMMPFN;

#if defined (_X86PAE_)
#pragma pack()
#endif

extern PMMPFN MmPfnDatabase;

#define MI_PFN_ELEMENT(index) (&MmPfnDatabase[index])

//
// No multiplier reciprocal needs to be inlined because the compiler (using Oxt)
// automatically computes the correct number, avoiding the expensive divide
// instruction.
//

#define MI_PFN_ELEMENT_TO_INDEX(_Pfn) ((PFN_NUMBER)(((ULONG_PTR)(_Pfn) - (ULONG_PTR)MmPfnDatabase) / sizeof (MMPFN)))

PVOID
MiGetInstructionPointer (
    VOID
    );

#define MI_SNAP_DIRTY(_Pfn, _NewValue, _Callerid)

#define MI_LOG_PTE_CHANGE(_PointerPte, _PteContents)

#define MI_DEBUGGER_WRITE_VALID_PTE_NEW_PROTECTION(_PointerPte, _PteContents) \
            InterlockedIncrement (&MiInDebugger);                             \
            MI_WRITE_VALID_PTE_NEW_PROTECTION(_PointerPte, _PteContents);     \
            InterlockedDecrement (&MiInDebugger);

#define MI_STAMP_MODIFIED(Pfn,id)

//++
//
// VOID
// MI_SET_MODIFIED (
//     IN PMMPFN Pfn,
//     IN ULONG NewValue,
//     IN ULONG CallerId
//     )
//
// Routine Description:
//
//     Set (or clear) the modified bit in the PFN database element.
//     The PFN lock must be held.
//
// Arguments:
//
//     Pfn - Supplies the PFN to operate on.
//
//     NewValue - Supplies 1 to set the modified bit, 0 to clear it.
//
//     CallerId - Supplies a caller ID useful for debugging purposes.
//
// Return Value:
//
//     None.
//
//--
//
#define MI_SET_MODIFIED(_Pfn, _NewValue, _CallerId)             \
            ASSERT ((_Pfn)->u3.e1.Rom == 0);                    \
            MI_SNAP_DIRTY (_Pfn, _NewValue, _CallerId);         \
            if ((_NewValue) != 0) {                             \
                MI_STAMP_MODIFIED (_Pfn, _CallerId);            \
            }                                                   \
            (_Pfn)->u3.e1.Modified = (_NewValue);

//
// ccNUMA is supported in multiprocessor PAE and WIN64 systems only.
//

#if (defined(_WIN64) || defined(_X86PAE_)) && !defined(NT_UP)
#define MI_MULTINODE

VOID
MiDetermineNode (
    IN PFN_NUMBER Page,
    IN PMMPFN Pfn
    );

#else

#define MiDetermineNode(x,y)    ((y)->u3.e1.PageColor = 0)

#endif

#if defined (_WIN64)

//
// Note there are some places where these portable macros are not currently
// used because we are not in the correct address space required.
//

#define MI_CAPTURE_USED_PAGETABLE_ENTRIES(PFN) \
        ASSERT ((PFN)->UsedPageTableEntries <= PTE_PER_PAGE); \
        (PFN)->OriginalPte.u.Soft.UsedPageTableEntries = (PFN)->UsedPageTableEntries;

#define MI_RETRIEVE_USED_PAGETABLE_ENTRIES_FROM_PTE(RBL, PTE) \
        ASSERT ((PTE)->u.Soft.UsedPageTableEntries <= PTE_PER_PAGE); \
        (RBL)->UsedPageTableEntries = (ULONG)(((PMMPTE)(PTE))->u.Soft.UsedPageTableEntries);

#define MI_ZERO_USED_PAGETABLE_ENTRIES_IN_INPAGE_SUPPORT(INPAGE_SUPPORT) \
            (INPAGE_SUPPORT)->UsedPageTableEntries = 0;

#define MI_ZERO_USED_PAGETABLE_ENTRIES_IN_PFN(PFN) (PFN)->UsedPageTableEntries = 0;

#define MI_INSERT_USED_PAGETABLE_ENTRIES_IN_PFN(PFN, INPAGE_SUPPORT) \
        ASSERT ((INPAGE_SUPPORT)->UsedPageTableEntries <= PTE_PER_PAGE); \
        (PFN)->UsedPageTableEntries = (INPAGE_SUPPORT)->UsedPageTableEntries;

#define MI_ZERO_USED_PAGETABLE_ENTRIES(PFN) \
        (PFN)->UsedPageTableEntries = 0;

#define MI_CHECK_USED_PTES_HANDLE(VA) \
        ASSERT (MiGetPdeAddress(VA)->u.Hard.Valid == 1);

#define MI_GET_USED_PTES_HANDLE(VA) \
        ((PVOID)MI_PFN_ELEMENT((PFN_NUMBER)MiGetPdeAddress(VA)->u.Hard.PageFrameNumber))

#define MI_GET_USED_PTES_FROM_HANDLE(PFN) \
        ((ULONG)(((PMMPFN)(PFN))->UsedPageTableEntries))

#define MI_INCREMENT_USED_PTES_BY_HANDLE(PFN) \
        (((PMMPFN)(PFN))->UsedPageTableEntries += 1); \
        ASSERT (((PMMPFN)(PFN))->UsedPageTableEntries <= PTE_PER_PAGE)

#define MI_INCREMENT_USED_PTES_BY_HANDLE_CLUSTER(PFN,INCR) \
        (((PMMPFN)(PFN))->UsedPageTableEntries += (ULONG)(INCR)); \
        ASSERT (((PMMPFN)(PFN))->UsedPageTableEntries <= PTE_PER_PAGE)

#define MI_DECREMENT_USED_PTES_BY_HANDLE(PFN) \
        (((PMMPFN)(PFN))->UsedPageTableEntries -= 1); \
        ASSERT (((PMMPFN)(PFN))->UsedPageTableEntries < PTE_PER_PAGE)

#else

#define MI_CAPTURE_USED_PAGETABLE_ENTRIES(PFN)
#define MI_RETRIEVE_USED_PAGETABLE_ENTRIES_FROM_PTE(RBL, PTE)
#define MI_ZERO_USED_PAGETABLE_ENTRIES_IN_INPAGE_SUPPORT(INPAGE_SUPPORT)
#define MI_ZERO_USED_PAGETABLE_ENTRIES_IN_PFN(PFN)

#define MI_INSERT_USED_PAGETABLE_ENTRIES_IN_PFN(PFN, INPAGE_SUPPORT)

#define MI_CHECK_USED_PTES_HANDLE(VA)

#define MI_GET_USED_PTES_HANDLE(VA) ((PVOID)&MmWorkingSetList->UsedPageTableEntries[MiGetPdeIndex(VA)])

#define MI_GET_USED_PTES_FROM_HANDLE(PDSHORT) ((ULONG)(*(PUSHORT)(PDSHORT)))

#define MI_INCREMENT_USED_PTES_BY_HANDLE(PDSHORT) \
    ((*(PUSHORT)(PDSHORT)) += 1); \
    ASSERT (((*(PUSHORT)(PDSHORT)) <= PTE_PER_PAGE))

#define MI_INCREMENT_USED_PTES_BY_HANDLE_CLUSTER(PDSHORT,INCR) \
    (*(PUSHORT)(PDSHORT)) = (USHORT)((*(PUSHORT)(PDSHORT)) + (INCR)); \
    ASSERT (((*(PUSHORT)(PDSHORT)) <= PTE_PER_PAGE))

#define MI_DECREMENT_USED_PTES_BY_HANDLE(PDSHORT) \
    ((*(PUSHORT)(PDSHORT)) -= 1); \
    ASSERT (((*(PUSHORT)(PDSHORT)) < PTE_PER_PAGE))

#endif

extern PFN_NUMBER MmDynamicPfn;

extern EX_PUSH_LOCK MmDynamicMemoryLock;

#define MI_INITIALIZE_DYNAMIC_MEMORY_LOCK()   ExInitializePushLock (&MmDynamicMemoryLock);

#define MI_LOCK_DYNAMIC_MEMORY_EXCLUSIVE()    ExAcquirePushLockExclusive (&MmDynamicMemoryLock);

#define MI_UNLOCK_DYNAMIC_MEMORY_EXCLUSIVE()  ExReleasePushLockExclusive (&MmDynamicMemoryLock);

#define MI_LOCK_DYNAMIC_MEMORY_SHARED()    ExAcquirePushLockShared (&MmDynamicMemoryLock);

#define MI_UNLOCK_DYNAMIC_MEMORY_SHARED()  ExReleasePushLockShared (&MmDynamicMemoryLock);

//
// Cache attribute tracking for I/O space mappings.
//

#define MI_IO_BACKTRACE_LENGTH  6

typedef struct _MMIO_TRACKER {
    LIST_ENTRY ListEntry;
    PVOID BaseVa;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    PVOID StackTrace[MI_IO_BACKTRACE_LENGTH];
} MMIO_TRACKER, *PMMIO_TRACKER;

extern KSPIN_LOCK MmIoTrackerLock;
extern LIST_ENTRY MmIoHeader;

extern PCHAR MiCacheStrings[];

MI_PFN_CACHE_ATTRIBUTE
MiInsertIoSpaceMap (
    IN PVOID BaseVa,
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    );

LOGICAL
MiIsProbeActive (
    IN PMMPTE InputPte,
    IN PFN_NUMBER InputPages
    );

//
// Total number of committed pages.
//

extern SIZE_T MmTotalCommittedPages;

extern SIZE_T MmTotalCommitLimit;

extern SIZE_T MmSharedCommit;

#if DBG

extern SPFN_NUMBER MiLockedCommit;

#define MI_INCREMENT_LOCKED_COMMIT()        \
            MiLockedCommit += 1;

#define MI_DECREMENT_LOCKED_COMMIT()        \
            ASSERT (MiLockedCommit > 0);    \
            MiLockedCommit -= 1;

#else

#define MI_INCREMENT_LOCKED_COMMIT()
#define MI_DECREMENT_LOCKED_COMMIT()

#endif

#if defined(_WIN64)

#define MiChargeCommitmentRegardless() \
    MI_INCREMENT_LOCKED_COMMIT(); \
    InterlockedIncrement64 ((PLONGLONG) &MmTotalCommittedPages);

#define MiReturnCommitmentRegardless() \
    MI_DECREMENT_LOCKED_COMMIT(); \
    InterlockedDecrement64 ((PLONGLONG) &MmTotalCommittedPages);

#else

#define MiChargeCommitmentRegardless() \
    MI_INCREMENT_LOCKED_COMMIT(); \
    InterlockedIncrement ((PLONG) &MmTotalCommittedPages);

#define MiReturnCommitmentRegardless() \
    MI_DECREMENT_LOCKED_COMMIT(); \
    InterlockedDecrement ((PLONG) &MmTotalCommittedPages);

#endif

extern ULONG MiChargeCommitmentFailures[3];   // referenced also in mi.h macros.

FORCEINLINE
LOGICAL
MiChargeCommitmentPfnLockHeld (
    IN LOGICAL Force
    )

/*++

Routine Description:

    This routine charges the specified commitment without attempting
    to expand paging files and waiting for the expansion.

Arguments:

    Force - Supplies TRUE if the lock is short-term and should be forced through
            if necessary.

Return Value:

    TRUE if the commitment was permitted, FALSE if not.

Environment:

    Kernel mode, PFN lock is held.

--*/

{
    if (MmTotalCommittedPages > MmTotalCommitLimit - 64) {

        if ((Force == 0) && ((PsGetCurrentThread()->MemoryMaker == 0) || (KeIsExecutingDpc () == TRUE))) {
            MiChargeCommitmentFailures[2] += 1;
            return FALSE;
        }
    }

    //
    // No need to do an InterlockedCompareExchange for this, keep it fast.
    //

    MiChargeCommitmentRegardless ();
                                                             
    return TRUE;
}

//
// Total number of available pages on the system.  This
// is the sum of the pages on the zeroed, free and standby lists.
//

extern PFN_NUMBER MmAvailablePages;

//
// Total number physical pages which would be usable if every process
// was at it's minimum working set size.  This value is initialized
// at system initialization to MmAvailablePages - MM_FLUID_PHYSICAL_PAGES.
// Everytime a thread is created, the kernel stack is subtracted from
// this and every time a process is created, the minimum working set
// is subtracted from this.  If the value would become negative, the
// operation (create process/kernel stack/ adjust working set) fails.
// The PFN LOCK must be owned to manipulate this value.
//

extern SPFN_NUMBER MmResidentAvailablePages;

//
// The total number of pages which would be removed from working sets
// if every working set was at its minimum.
//

extern PFN_NUMBER MmPagesAboveWsMinimum;

//
// If memory is becoming short and MmPagesAboveWsMinimum is
// greater than MmPagesAboveWsThreshold, trim working sets.
//

extern PFN_NUMBER MmPlentyFreePages;

extern PFN_NUMBER MmPagesAboveWsThreshold;

extern LONG MiDelayPageFaults;

#define MM_DBG_COMMIT_NONPAGED_POOL_EXPANSION           0
#define MM_DBG_COMMIT_PAGED_POOL_PAGETABLE              1
#define MM_DBG_COMMIT_PAGED_POOL_PAGES                  2
#define MM_DBG_COMMIT_SESSION_POOL_PAGE_TABLES          3
#define MM_DBG_COMMIT_ALLOCVM1                          4
#define MM_DBG_COMMIT_ALLOCVM_SEGMENT                   5
#define MM_DBG_COMMIT_IMAGE                             6
#define MM_DBG_COMMIT_PAGEFILE_BACKED_SHMEM             7
#define MM_DBG_COMMIT_INDEPENDENT_PAGES                 8
#define MM_DBG_COMMIT_CONTIGUOUS_PAGES                  9
#define MM_DBG_COMMIT_MDL_PAGES                         0xA
#define MM_DBG_COMMIT_NONCACHED_PAGES                   0xB
#define MM_DBG_COMMIT_MAPVIEW_DATA                      0xC
#define MM_DBG_COMMIT_FILL_SYSTEM_DIRECTORY             0xD
#define MM_DBG_COMMIT_EXTRA_SYSTEM_PTES                 0xE
#define MM_DBG_COMMIT_DRIVER_PAGING_AT_INIT             0xF
#define MM_DBG_COMMIT_PAGEFILE_FULL                     0x10
#define MM_DBG_COMMIT_PROCESS_CREATE                    0x11
#define MM_DBG_COMMIT_KERNEL_STACK_CREATE               0x12
#define MM_DBG_COMMIT_SET_PROTECTION                    0x13
#define MM_DBG_COMMIT_SESSION_CREATE                    0x14
#define MM_DBG_COMMIT_SESSION_IMAGE_PAGES               0x15
#define MM_DBG_COMMIT_SESSION_PAGETABLE_PAGES           0x16
#define MM_DBG_COMMIT_SESSION_SHARED_IMAGE              0x17
#define MM_DBG_COMMIT_DRIVER_PAGES                      0x18
#define MM_DBG_COMMIT_INSERT_VAD                        0x19
#define MM_DBG_COMMIT_SESSION_WS_INIT                   0x1A
#define MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_PAGES       0x1B
#define MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_HASHPAGES   0x1C
#define MM_DBG_COMMIT_SPECIAL_POOL_PAGES                0x1D
#define MM_DBG_COMMIT_SPECIAL_POOL_MAPPING_PAGES        0x1E
#define MM_DBG_COMMIT_SMALL                             0x1F
#define MM_DBG_COMMIT_EXTRA_WS_PAGES                    0x20
#define MM_DBG_COMMIT_EXTRA_INITIAL_SESSION_WS_PAGES    0x21
#define MM_DBG_COMMIT_ALLOCVM_PROCESS                   0x22
#define MM_DBG_COMMIT_INSERT_VAD_PT                     0x23
#define MM_DBG_COMMIT_ALLOCVM_PROCESS2                  0x24
#define MM_DBG_COMMIT_CHARGE_NORMAL                     0x25
#define MM_DBG_COMMIT_CHARGE_CAUSE_POPUP                0x26
#define MM_DBG_COMMIT_CHARGE_CANT_EXPAND                0x27
#define MM_DBG_COMMIT_LARGE_VA_PAGES                    0x28
#define MM_DBG_COMMIT_LOAD_SYSTEM_IMAGE_TEMP            0x29

#define MM_DBG_COMMIT_RETURN_NONPAGED_POOL_EXPANSION    0x40
#define MM_DBG_COMMIT_RETURN_PAGED_POOL_PAGES           0x41
#define MM_DBG_COMMIT_RETURN_SESSION_DATAPAGE           0x42
#define MM_DBG_COMMIT_RETURN_ALLOCVM_SEGMENT            0x43
#define MM_DBG_COMMIT_RETURN_ALLOCVM2                   0x44

#define MM_DBG_COMMIT_RETURN_IMAGE_NO_LARGE_CA          0x46
#define MM_DBG_COMMIT_RETURN_PTE_RANGE                  0x47
#define MM_DBG_COMMIT_RETURN_NTFREEVM1                  0x48
#define MM_DBG_COMMIT_RETURN_NTFREEVM2                  0x49
#define MM_DBG_COMMIT_RETURN_INDEPENDENT_PAGES          0x4A
#define MM_DBG_COMMIT_RETURN_AWE_EXCESS                 0x4B
#define MM_DBG_COMMIT_RETURN_MDL_PAGES                  0x4C
#define MM_DBG_COMMIT_RETURN_NONCACHED_PAGES            0x4D
#define MM_DBG_COMMIT_RETURN_SESSION_CREATE_FAILURE     0x4E
#define MM_DBG_COMMIT_RETURN_PAGETABLES                 0x4F
#define MM_DBG_COMMIT_RETURN_PROTECTION                 0x50
#define MM_DBG_COMMIT_RETURN_SEGMENT_DELETE1            0x51
#define MM_DBG_COMMIT_RETURN_SEGMENT_DELETE2            0x52
#define MM_DBG_COMMIT_RETURN_PAGEFILE_FULL              0x53
#define MM_DBG_COMMIT_RETURN_SESSION_DEREFERENCE        0x54
#define MM_DBG_COMMIT_RETURN_VAD                        0x55
#define MM_DBG_COMMIT_RETURN_PROCESS_CREATE_FAILURE1    0x56
#define MM_DBG_COMMIT_RETURN_PROCESS_DELETE             0x57
#define MM_DBG_COMMIT_RETURN_PROCESS_CLEAN_PAGETABLES   0x58
#define MM_DBG_COMMIT_RETURN_KERNEL_STACK_DELETE        0x59
#define MM_DBG_COMMIT_RETURN_SESSION_DRIVER_LOAD_FAILURE1 0x5A
#define MM_DBG_COMMIT_RETURN_DRIVER_INIT_CODE           0x5B
#define MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD              0x5C
#define MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD1             0x5D
#define MM_DBG_COMMIT_RETURN_NORMAL                     0x5E
#define MM_DBG_COMMIT_RETURN_PF_FULL_EXTEND             0x5F
#define MM_DBG_COMMIT_RETURN_EXTENDED                   0x60
#define MM_DBG_COMMIT_RETURN_SEGMENT_DELETE3            0x61
#define MM_DBG_COMMIT_CHARGE_LARGE_PAGES                0x62
#define MM_DBG_COMMIT_RETURN_LARGE_PAGES                0x63

#define MM_TRACK_COMMIT(_index, bump)
#define MM_TRACK_COMMIT_REDUCTION(_index, bump)
#define MI_INCREMENT_TOTAL_PROCESS_COMMIT(_charge)

#define MiReturnCommitment(_QuotaCharge)                                \
            ASSERT ((SSIZE_T)(_QuotaCharge) >= 0);                      \
            ASSERT (MmTotalCommittedPages >= (_QuotaCharge));           \
            InterlockedExchangeAddSizeT (&MmTotalCommittedPages, 0-((SIZE_T)(_QuotaCharge))); \
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_NORMAL, (_QuotaCharge));


//++
// PFN_NUMBER
// MI_NONPAGEABLE_MEMORY_AVAILABLE(
//    VOID
//    );
//
// Routine Description:
//
//    This routine lets callers know how many pages can be charged against
//    the resident available, factoring in earlier Mm promises that
//    may not have been redeemed at this point (ie: nonpaged pool expansion,
//    etc, that must be honored at a later point if requested).
//
// Arguments
//
//    None.
//
// Return Value:
//
//    The number of currently available pages in the resident available.
//
//    N.B.  This is a signed quantity and can be negative.
//
//--
#define MI_NONPAGEABLE_MEMORY_AVAILABLE()                                    \
        ((SPFN_NUMBER)                                                      \
            (MmResidentAvailablePages -                                     \
             MmSystemLockPagesCount))

extern PFN_NUMBER MmSystemLockPagesCount;

//
// Convert these to intrinsics as soon as compiler support is available...
//

#if defined(_X86_)

FORCEINLINE
SHORT
FASTCALL
InterlockedCompareExchange16 (
    IN OUT SHORT volatile *Destination,
    IN SHORT Exchange,
    IN SHORT Comperand
    )
{
    __asm {
        mov     ax, Comperand
        mov     ecx, Destination
        mov     dx, Exchange
        lock cmpxchg word ptr [ecx], dx
    }
}

FORCEINLINE
SHORT
FASTCALL
InterlockedIncrement16 (
    IN OUT PSHORT Addend
    )
{
    __asm {
        mov     ax, 1
        mov     ecx, Addend
        lock xadd    word ptr [ecx], ax
        inc     ax
    }
}

FORCEINLINE
SHORT
FASTCALL
InterlockedDecrement16 (
    IN OUT PSHORT Addend
    )
{
    __asm {
        mov     ax, -1
        mov     ecx, Addend
        lock xadd    word ptr [ecx], ax
        dec     ax
    }
}

#else

#if !defined(_AMD64_)

SHORT
InterlockedCompareExchange16 (
    IN OUT SHORT volatile *Destination,
    IN SHORT Exchange,
    IN SHORT Comperand
    );

SHORT
FASTCALL
InterlockedIncrement16 (
    IN OUT PSHORT Addend
    );

SHORT
FASTCALL
InterlockedDecrement16 (
    IN OUT PSHORT Addend
    );

#endif

#endif

#define MI_SNAP_PFNREF(_Pfn, _NewValue, _Value, _CallerId)

#define InterlockedCompareExchangePfn InterlockedCompareExchange16
#define InterlockedIncrementPfn       InterlockedIncrement16
#define InterlockedDecrementPfn       InterlockedDecrement16

VOID
MiBadRefCount (
    __in PMMPFN Pfn1
    );

VOID
FASTCALL
MiDecrementReferenceCount (
    IN PMMPFN Pfn1,
    IN PFN_NUMBER PageFrameIndex
    );

extern ULONG MmReferenceCountCheck;

FORCEINLINE
VOID
MI_ADD_LOCKED_PAGE_CHARGE (
    IN PMMPFN Pfn1
    )

/*++

Routine Description:

    Charge the systemwide count of locked pages if this is the initial
    lock for this page (multiple concurrent locks are only charged once).

Arguments:

    Pfn - Supplies the PFN entry to operate on.

Return Value:

    TRUE if the charge succeeded, FALSE if not.

Environment:

    Kernel mode.  PFN lock held.

--*/

{
    USHORT NewRefCount;
    LOGICAL ChargedCommit;

    //
    // We must charge systemwide resources (commit & resident available)
    // in advance of incrementing the PFN reference count.  This is because 
    // the PFN reference count is updated lock free and so another thread
    // can pass this one without charging against system, and if this thread
    // were to instead charge afterwards, it may fail below and it would be
    // unable to stop the other thread(s) that have already proceeded.
    //

    if ((Pfn1->u3.e1.PrototypePte == 1) &&
        (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

        //
        // This is a filesystem-backed page - charge commit for
        // it as we have no way to tell when the caller will
        // unlock the page.  Note this is a short term charge so
        // always push it through.
        //

        ChargedCommit = TRUE;
        MiChargeCommitmentPfnLockHeld (TRUE);
    }
    else {
        ChargedCommit = FALSE;
    }

    //
    // Now charge resident available.
    //

    InterlockedIncrementSizeT (&MmSystemLockPagesCount);

    NewRefCount = InterlockedIncrementPfn ((PSHORT)&Pfn1->u3.e2.ReferenceCount);

    if (NewRefCount == 2) {

        if (Pfn1->u2.ShareCount != 0) {

            //
            // This is the first thread to lock the page so keep the charges.
            //

            ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
        }
        else {

            //
            // The page is in transition with no share count so it has already
            // been locked.
            //

            InterlockedDecrementSizeT (&MmSystemLockPagesCount);
            if (ChargedCommit == TRUE) {
                MiReturnCommitment (1);
            }
        }
    }
    else {

        ASSERT (NewRefCount < MmReferenceCountCheck + 0x400);

        //
        // This thread was not the first one to lock this page so return
        // the excess systemwide charges we had to allocate in advance (since
        // some other thread has already incurred them).
        //

        InterlockedDecrementSizeT (&MmSystemLockPagesCount);
        if (ChargedCommit == TRUE) {
            MiReturnCommitment (1);
        }
    }
    
    return;
}

FORCEINLINE
LOGICAL
MI_ADD_LOCKED_PAGE_CHARGE_FOR_PROBE (
    IN PMMPFN Pfn1,
    IN LOGICAL Force,
    IN PMMPTE PointerPte
    )

/*++

Routine Description:

    Charge the systemwide count of locked pages if this is the initial
    lock for this page (multiple concurrent locks are only charged once).
    This routine is written specifically for MmProbeAndLockPages use.

Arguments:

    Pfn - Supplies the PFN entry to operate on.

    Force - Supplies TRUE if the lock is short-term and should be forced through
            if necessary.

    PointerPte - Supplies the PTE entry for the page being locked.  This should
                 only be used for system space addresses where the share count
                 is zero or the page is not active.  This is because this PTE
                 argument is meaningless for large page addresses, but large
                 page kernel addresses would never have a zero sharecount or
                 non-active page state.

Return Value:

    TRUE if the charge succeeded, FALSE if not.

Environment:

    Kernel mode.  PFN lock may be held.

--*/

{
    LOGICAL ChargedCommit;
    LOGICAL FunkyAddress;
    USHORT RefCount;
    USHORT OldRefCount;

#if !DBG
    UNREFERENCED_PARAMETER (PointerPte);
#endif

    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

    if ((Pfn1->u2.ShareCount != 0) &&
        (Pfn1->u3.e1.PageLocation == ActiveAndValid)) {

        FunkyAddress = FALSE;
    }
    else {

        //
        // The only way this can happen is if a driver
        // builds an MDL for a user address and then gets
        // a system VA for it.  The driver may then call MmProbeAndLockPages
        // again, this time with the system VA (instead of the original
        // user VA) for the same range.
        //
        // Thus the PFNs will really point at PFNs which generally
        // require the relevant process' working set pushlock, but in
        // this instance we will be holding the PFN lock (not the process
        // working set pushlock).  The PFN lock is used because it
        // prevents the PFN sharecounts and active states from changing.
        //

        ASSERT (MiGetVirtualAddressMappedByPte (PointerPte) >= MmSystemRangeStart);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        ASSERT (MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber) == Pfn1);
        MM_PFN_LOCK_ASSERT ();
        FunkyAddress = TRUE;
    }

    //
    // We must charge systemwide resources (commit & resident available)
    // in advance of incrementing the PFN reference count.  This is because 
    // the PFN reference count is updated lock free and so another thread
    // can pass this one without charging against system, and if this thread
    // were to instead charge afterwards, it may fail below and it would be
    // unable to stop the other thread(s) that have already proceeded.
    //

    if ((Pfn1->u3.e1.PrototypePte == 1) &&
        (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

        //
        // This is a filesystem-backed page - charge commit for
        // it as we have no way to tell when the caller will
        // unlock the page.
        //

        if (MiChargeCommitmentPfnLockHeld (Force) == FALSE) {
            return FALSE;
        }
        ChargedCommit = TRUE;
    }
    else {
        ChargedCommit = FALSE;
    }

    //
    // Now charge resident available.
    //

    InterlockedIncrementSizeT (&MmSystemLockPagesCount);

    if ((MI_NONPAGEABLE_MEMORY_AVAILABLE() <= 0) && (Force == FALSE)) {
        InterlockedDecrementSizeT (&MmSystemLockPagesCount);
        if (ChargedCommit == TRUE) {
            MiReturnCommitment (1);
        }
        return FALSE;
    }

    do {

        OldRefCount = Pfn1->u3.e2.ReferenceCount;

        ASSERT (OldRefCount != 0);

        if (OldRefCount >= MmReferenceCountCheck) {
            ASSERT (FALSE);
            InterlockedDecrementSizeT (&MmSystemLockPagesCount);
            if (ChargedCommit == TRUE) {
                MiReturnCommitment (1);
            }
            return FALSE;
        }

        RefCount = InterlockedCompareExchangePfn ((PSHORT)&Pfn1->u3.e2.ReferenceCount,
                                                  OldRefCount + 1,
                                                  OldRefCount);

        ASSERT (RefCount != 0);

    } while (OldRefCount != RefCount);

    if ((OldRefCount != 1) || (FunkyAddress == TRUE)) {

        //
        // This thread was not the first one to lock this page so return
        // the excess systemwide charges we had to allocate in advance (since
        // some other thread has already incurred them).
        //

        InterlockedDecrementSizeT (&MmSystemLockPagesCount);

        if (ChargedCommit == TRUE) {
            MiReturnCommitment (1);
        }
    }

    return TRUE;
}

FORCEINLINE
VOID
MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (
    IN PMMPFN Pfn1
    )

/*++

Routine Description:

    Charge the systemwide count of locked pages if this is the initial
    lock for this page (multiple concurrent locks are only charged once).

Arguments:

    Pfn1 - Supplies the PFN entry to operate on.

Return Value:

    None.

Environment:

    Kernel mode.  PFN lock held.

--*/

{
    LOGICAL ChargedCommit;
    USHORT NewRefCount;

    ASSERT (Pfn1->u2.ShareCount == 0);
    ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);

    //
    // We must charge systemwide resources (commit & resident available)
    // in advance of incrementing the PFN reference count.  This is because 
    // the PFN reference count is updated lock free and so another thread
    // can pass this one without charging against system, and if this thread
    // were to instead charge afterwards, it may fail below and it would be
    // unable to stop the other thread(s) that have already proceeded.
    //

    if ((Pfn1->u3.e1.PrototypePte == 1) &&
        (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

        //
        // This is a filesystem-backed page - charge commit for
        // it as we have no way to tell when the caller will
        // unlock the page.  Note this is a short term charge so
        // always push it through.
        //

        ChargedCommit = TRUE;
        MiChargeCommitmentPfnLockHeld (TRUE);
    }
    else {
        ChargedCommit = FALSE;
    }

    //
    // Now charge resident available.
    //

    InterlockedIncrementSizeT (&MmSystemLockPagesCount);

    NewRefCount = InterlockedIncrementPfn ((PSHORT)&Pfn1->u3.e2.ReferenceCount);

    if (NewRefCount != 1) {

        ASSERT (NewRefCount < MmReferenceCountCheck + 0x400);

        //
        // This thread was not the first one to lock this page so return
        // the excess systemwide charges we had to allocate in advance (since
        // some other thread has already incurred them).
        //

        InterlockedDecrementSizeT (&MmSystemLockPagesCount);
        if (ChargedCommit == TRUE) {
            MiReturnCommitment (1);
        }
    }
    else {

        //
        // This is the first thread to lock the page so keep the charges.
        //

        NOTHING;
    }
    
    return;
}

FORCEINLINE
VOID
MI_REMOVE_LOCKED_PAGE_CHARGE (
    IN PMMPFN Pfn1
    )

/*++

Routine Description:

    Remove the charge from the systemwide count of locked pages if this
    is the last lock for this page (multiple concurrent locks are only
    charged once).

    The PFN reference count is NOT decremented, instead the fault handler
    is going to up the share count and make this page valid.

Arguments:

    Pfn1 - Supplies the PFN entry to operate on.

Return Value:

    None.

Environment:

    Kernel mode.  PFN lock held.

--*/

{
    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
    ASSERT (Pfn1->u2.ShareCount == 0);

    if (Pfn1->u3.e2.ReferenceCount == 1) {

        //
        // This page has already been deleted from all process address
        // spaces.  It is sitting in limbo (not on any list) awaiting
        // this last dereference.
        //

        ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);

        if ((Pfn1->u3.e1.PrototypePte == 1) &&
            (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

            MiReturnCommitmentRegardless ();
        }
        InterlockedDecrementSizeT (&MmSystemLockPagesCount);
    }

    return;
}

FORCEINLINE
VOID
MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (
    IN PMMPFN Pfn1
    )

/*++

Routine Description:

    Remove the charge from the systemwide count of locked pages if this
    is the last lock for this page (multiple concurrent locks are only
    charged once).

    The PFN reference checks are carefully ordered so the most common case
    is handled first, the next most common case next, etc.  The case of
    a reference count of 2 occurs more than 1000x (yes, 3 orders of
    magnitude) more than a reference count of 1.  And reference counts of >2
    occur 3 orders of magnitude more frequently than reference counts of
    exactly 1.

    The PFN reference count is then decremented.

Arguments:

    Pfn1 - Supplies the PFN entry to operate on.

Return Value:

    TRUE if the charge succeeded, FALSE if not.

Environment:

    Kernel mode.  PFN lock held.

--*/

{
    USHORT RefCount;
    USHORT OldRefCount;
    PFN_NUMBER PageFrameIndex;

    do {

        OldRefCount = Pfn1->u3.e2.ReferenceCount;

        if (OldRefCount == 0) {
            MiBadRefCount (Pfn1);
        }

        if (OldRefCount == 1) {

            //
            // This page has already been deleted from all process address
            // spaces.  It is sitting in limbo (not on any list) awaiting
            // this last dereference.
            //

            ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
            ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);
            ASSERT (Pfn1->u2.ShareCount == 0);

            if ((Pfn1->u3.e1.PrototypePte == 1) &&
                (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

                MiReturnCommitmentRegardless ();
            }

            InterlockedDecrementSizeT (&MmSystemLockPagesCount);
            PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);
            MiDecrementReferenceCount (Pfn1, PageFrameIndex);

            return;
        }

        RefCount = InterlockedCompareExchangePfn ((PSHORT)&Pfn1->u3.e2.ReferenceCount,
                                                  OldRefCount - 1,
                                                  OldRefCount);

        ASSERT (RefCount != 0);

    } while (OldRefCount != RefCount);

    ASSERT (RefCount > 1);

    if (RefCount == 2) {

        if (Pfn1->u2.ShareCount >= 1) {

            ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);

            if ((Pfn1->u3.e1.PrototypePte == 1) &&
                (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

                MiReturnCommitmentRegardless ();
            }
            InterlockedDecrementSizeT (&MmSystemLockPagesCount);
        }
        else {
            //
            // There are multiple referencers to this page and the
            // page is no longer valid in any process address space.
            // The systemwide lock count can only be decremented
            // by the last dereference.
            //

            NOTHING;
        }
    }
    else {

        //
        // There are still multiple referencers to this page (it may
        // or may not be resident in any process address space).
        // Since the systemwide lock count can only be decremented
        // by the last dereference (and this is not it), no action
        // is taken here.
        //

        NOTHING;
    }
    return;
}

//++
//
// VOID
// MI_ZERO_WSINDEX (
//     IN PMMPFN Pfn
//     )
//
// Routine Description:
//
//     Zero the Working Set Index field of the argument PFN entry.
//     There is a subtlety here on systems where the WsIndex ULONG is
//     overlaid with an Event pointer and sizeof(ULONG) != sizeof(PKEVENT).
//     Note this will need to be updated if we ever decide to allocate bodies of
//     thread objects on 4GB boundaries.
//
// Arguments:
//
//     Pfn - the PFN index to operate on.
//
// Return Value:
//
//     None.
//
//--
//
#define MI_ZERO_WSINDEX(Pfn) \
    Pfn->u1.Event = NULL;

typedef enum _MMSHARE_TYPE {
    Normal,
    ShareCountOnly,
    AndValid
} MMSHARE_TYPE;

typedef struct _MMWSLE_HASH {
    PVOID Key;
    WSLE_NUMBER Index;
} MMWSLE_HASH, *PMMWSLE_HASH;

//++
//
// WSLE_NUMBER
// MI_WSLE_HASH (
//     IN ULONG_PTR VirtualAddress,
//     IN PMMWSL WorkingSetList
//     )
//
// Routine Description:
//
//     Hash the address
//
// Arguments:
//
//     VirtualAddress - the address to hash.
//
//     WorkingSetList - the working set to hash the address into.
//
// Return Value:
//
//     The hash key.
//
//--
//
#define MI_WSLE_HASH(Address, Wsl) \
    ((WSLE_NUMBER)(((ULONG_PTR)PAGE_ALIGN(Address) >> (PAGE_SHIFT - 2)) % \
        ((Wsl)->HashTableSize - 1)))

//
// Working Set List Entry.
//

typedef struct _MMWSLENTRY {
    ULONG_PTR Valid : 1;
    ULONG_PTR LockedInWs : 1;
    ULONG_PTR LockedInMemory : 1;

    //
    // If Protection == MM_ZERO_ACCESS, it means the WSLE is using the
    // same protection as the prototype PTE.  Otherwise the protection is
    // real and potentially different from the prototype PTE.
    //

    ULONG_PTR Protection : 5;

    ULONG_PTR Hashed : 1;
    ULONG_PTR Direct : 1;
    ULONG_PTR Age : 2;
#if MM_VIRTUAL_PAGE_FILLER
    ULONG_PTR Filler : MM_VIRTUAL_PAGE_FILLER;
#endif
    ULONG_PTR VirtualPageNumber : MM_VIRTUAL_PAGE_SIZE;
} MMWSLENTRY;

typedef struct _MMWSLE {
    union {
        PVOID VirtualAddress;
        ULONG_PTR Long;
        MMWSLENTRY e1;
    } u1;
} MMWSLE;

//
// Mask off everything but the VPN and the valid bit so the WSLE compare
// can be done in one instruction.
//

#define MI_GENERATE_VALID_WSLE(Wsle)                   \
        ((PVOID)(ULONG_PTR)((Wsle)->u1.Long & (~(PAGE_SIZE - 1) | 0x1)))

#define MI_GET_PROTECTION_FROM_WSLE(Wsl) ((Wsl)->u1.e1.Protection)

typedef MMWSLE *PMMWSLE;

//
// Working Set List.  Must be quadword sized.
//

typedef struct _MMWSL {
    WSLE_NUMBER FirstFree;
    WSLE_NUMBER FirstDynamic;
    WSLE_NUMBER LastEntry;
    WSLE_NUMBER NextSlot;               // The next slot to trim
    PMMWSLE Wsle;
    WSLE_NUMBER LastInitializedWsle;
    WSLE_NUMBER NonDirectCount;
    PMMWSLE_HASH HashTable;
    ULONG HashTableSize;
    ULONG NumberOfCommittedPageTables;
    PVOID HashTableStart;
    PVOID HighestPermittedHashAddress;
    ULONG NumberOfImageWaiters;
    ULONG VadBitMapHint;

#if _WIN64
    PVOID HighestUserAddress;           // Maintained for wow64 processes only
#endif

#if (_MI_PAGING_LEVELS >= 3)
    ULONG MaximumUserPageTablePages;
#endif

#if (_MI_PAGING_LEVELS >= 4)
    ULONG MaximumUserPageDirectoryPages;
    PULONG CommittedPageTables;

    ULONG NumberOfCommittedPageDirectories;
    PULONG CommittedPageDirectories;

    ULONG NumberOfCommittedPageDirectoryParents;
    ULONG_PTR CommittedPageDirectoryParents[(MM_USER_PAGE_DIRECTORY_PARENT_PAGES + sizeof(ULONG_PTR)*8-1)/(sizeof(ULONG_PTR)*8)];

#elif (_MI_PAGING_LEVELS >= 3)
    PULONG CommittedPageTables;

    ULONG NumberOfCommittedPageDirectories;
    ULONG_PTR CommittedPageDirectories[(MM_USER_PAGE_DIRECTORY_PAGES + sizeof(ULONG_PTR)*8-1)/(sizeof(ULONG_PTR)*8)];

#else

    //
    // This must be at the end.
    // Not used in system cache or session working set lists.
    //

    USHORT UsedPageTableEntries[MM_USER_PAGE_TABLE_PAGES];

    ULONG CommittedPageTables[MM_USER_PAGE_TABLE_PAGES/(sizeof(ULONG)*8)];
#endif

} MMWSL, *PMMWSL;

#define MI_LOG_WSLE_CHANGE(_WorkingSetList, _WorkingSetIndex, _WsleValue)

#if defined(_X86_)
extern PMMWSL MmWorkingSetList;
#endif

extern PKEVENT MiHighMemoryEvent;
extern PKEVENT MiLowMemoryEvent;

//
// The claim estimate of unused pages in a working set is limited
// to grow by this amount per estimation period.
//

#define MI_CLAIM_INCR 30

//
// The maximum number of different ages a page can be.
//

#define MI_USE_AGE_COUNT 4
#define MI_USE_AGE_MAX (MI_USE_AGE_COUNT - 1)

//
// If more than this "percentage" of the working set is estimated to
// be used then allow it to grow freely.
//

#define MI_REPLACEMENT_FREE_GROWTH_SHIFT 5

//
// If more than this "percentage" of the working set has been claimed
// then force replacement in low memory.
//

#define MI_REPLACEMENT_CLAIM_THRESHOLD_SHIFT 3

//
// If more than this "percentage" of the working set is estimated to
// be available then force replacement in low memory.
//

#define MI_REPLACEMENT_EAVAIL_THRESHOLD_SHIFT 3

//
// If while doing replacement a page is found of this age or older then
// replace it.  Otherwise the oldest is selected.
//

#define MI_IMMEDIATE_REPLACEMENT_AGE 2

//
// When trimming, use these ages for different passes.
//

#define MI_MAX_TRIM_PASSES 4
#define MI_PASS0_TRIM_AGE 2
#define MI_PASS1_TRIM_AGE 1
#define MI_PASS2_TRIM_AGE 1
#define MI_PASS3_TRIM_AGE 1
#define MI_PASS4_TRIM_AGE 0

//
// If not a forced trim, trim pages older than this age.
//

#define MI_TRIM_AGE_THRESHOLD 2

//
// This "percentage" of a claim is up for grabs in a foreground process.
//

#define MI_FOREGROUND_CLAIM_AVAILABLE_SHIFT 3

//
// This "percentage" of a claim is up for grabs in a background process.
//

#define MI_BACKGROUND_CLAIM_AVAILABLE_SHIFT 1

//++
//
// DWORD
// MI_CALC_NEXT_VALID_ESTIMATION_SLOT (
//     DWORD Previous,
//     DWORD Minimum,
//     DWORD Maximum,
//     MI_NEXT_ESTIMATION_SLOT_CONST NextEstimationSlotConst,
//     PMMWSLE Wsle
//     )
//
// Routine Description:
//
//      We iterate through the working set array in a non-sequential
//      manner so that the sample is independent of any aging or trimming.
//
//      This algorithm walks through the working set with a stride of
//      2^MiEstimationShift elements.
//
// Arguments:
//
//      Previous - Last slot used
//
//      Minimum - Minimum acceptable slot (ie. the first dynamic one)
//
//      Maximum - max slot number + 1
//
//      NextEstimationSlotConst - for this algorithm it contains the stride
//
//      Wsle - the working set array
//
// Return Value:
//
//      Next slot.
//
// Environment:
//
//      Kernel mode, APCs disabled, working set lock held and PFN lock held.
//
//--

typedef struct _MI_NEXT_ESTIMATION_SLOT_CONST {
    WSLE_NUMBER Stride;
} MI_NEXT_ESTIMATION_SLOT_CONST;


#define MI_CALC_NEXT_ESTIMATION_SLOT_CONST(NextEstimationSlotConst, WorkingSetList) \
    (NextEstimationSlotConst).Stride = 1 << MiEstimationShift;

#define MI_NEXT_VALID_ESTIMATION_SLOT(Previous, StartEntry, Minimum, Maximum, NextEstimationSlotConst, Wsle) \
    ASSERT(((Previous) >= Minimum) && ((Previous) <= Maximum)); \
    ASSERT(((StartEntry) >= Minimum) && ((StartEntry) <= Maximum)); \
    do { \
        (Previous) += (NextEstimationSlotConst).Stride; \
        if ((Previous) > Maximum) { \
            (Previous) = Minimum + ((Previous + 1) & (NextEstimationSlotConst.Stride - 1)); \
            StartEntry += 1; \
            (Previous) = StartEntry; \
        } \
        if ((Previous) > Maximum || (Previous) < Minimum) { \
            StartEntry = Minimum; \
            (Previous) = StartEntry; \
        } \
    } while (Wsle[Previous].u1.e1.Valid == 0);

//++
//
// WSLE_NUMBER
// MI_NEXT_VALID_AGING_SLOT (
//     DWORD Previous,
//     DWORD Minimum,
//     DWORD Maximum,
//     PMMWSLE Wsle
//     )
//
// Routine Description:
//
//      This finds the next slot to valid slot to age.  It walks
//      through the slots sequentially.
//
// Arguments:
//
//      Previous - Last slot used
//
//      Minimum - Minimum acceptable slot (ie. the first dynamic one)
//
//      Maximum - Max slot number + 1
//
//      Wsle - the working set array
//
// Return Value:
//
//      None.
//
// Environment:
//
//      Kernel mode, APCs disabled, working set lock held and PFN lock held.
//
//--

#define MI_NEXT_VALID_AGING_SLOT(Previous, Minimum, Maximum, Wsle) \
    ASSERT(((Previous) >= Minimum) && ((Previous) <= Maximum)); \
    do { \
        (Previous) += 1; \
        if ((Previous) > Maximum) { \
            Previous = Minimum; \
        } \
    } while ((Wsle[Previous].u1.e1.Valid == 0));

//++
//
// ULONG
// MI_CALCULATE_USAGE_ESTIMATE (
//     IN PULONG SampledAgeCounts.
//     IN ULONG CounterShift
//     )
//
// Routine Description:
//
//      In Usage Estimation, we count the number of pages of each age in
//      a sample.  The function turns the SampledAgeCounts into an
//      estimate of the unused pages.
//
// Arguments:
//
//      SampledAgeCounts - counts of pages of each different age in the sample
//
//      CounterShift - shift necessary to apply sample to entire WS
//
// Return Value:
//
//      The number of pages to walk in the working set to get a good
//      estimate of the number available.
//
//--

#define MI_CALCULATE_USAGE_ESTIMATE(SampledAgeCounts, CounterShift) \
                (((SampledAgeCounts)[1] + \
                    (SampledAgeCounts)[2] + (SampledAgeCounts)[3]) \
                    << (CounterShift))

//++
//
// VOID
// MI_RESET_WSLE_AGE (
//     IN PMMPTE PointerPte,
//     IN PMMWSLE Wsle
//     )
//
// Routine Description:
//
//      Clear the age counter for the working set entry.
//
// Arguments:
//
//      PointerPte - pointer to the working set list entry's PTE.
//
//      Wsle - pointer to the working set list entry.
//
// Return Value:
//
//      None.
//
//--
#define MI_RESET_WSLE_AGE(PointerPte, Wsle) \
    (Wsle)->u1.e1.Age = 0;

//++
//
// ULONG
// MI_GET_WSLE_AGE (
//     IN PMMPTE PointerPte,
//     IN PMMWSLE Wsle
//     )
//
// Routine Description:
//
//      Clear the age counter for the working set entry.
//
// Arguments:
//
//      PointerPte - pointer to the working set list entry's PTE
//      Wsle - pointer to the working set list entry
//
// Return Value:
//
//      Age group of the working set entry
//
//--
#define MI_GET_WSLE_AGE(PointerPte, Wsle) \
    ((ULONG)((Wsle)->u1.e1.Age))

//++
//
// VOID
// MI_INC_WSLE_AGE (
//     IN PMMPTE PointerPte,
//     IN PMMWSLE Wsle,
//     )
//
// Routine Description:
//
//      Increment the age counter for the working set entry.
//
// Arguments:
//
//      PointerPte - pointer to the working set list entry's PTE.
//
//      Wsle - pointer to the working set list entry.
//
// Return Value:
//
//      None
//
//--

#define MI_INC_WSLE_AGE(PointerPte, Wsle) \
    if ((Wsle)->u1.e1.Age < 3) { \
        (Wsle)->u1.e1.Age += 1; \
    }

//++
//
// VOID
// MI_UPDATE_USE_ESTIMATE (
//     IN PMMPTE PointerPte,
//     IN PMMWSLE Wsle,
//     IN ULONG *SampledAgeCounts
//     )
//
// Routine Description:
//
//      Update the sampled age counts.
//
// Arguments:
//
//      PointerPte - pointer to the working set list entry's PTE.
//
//      Wsle - pointer to the working set list entry.
//
//      SampledAgeCounts - array of age counts to be updated.
//
// Return Value:
//
//      None
//
//--

#define MI_UPDATE_USE_ESTIMATE(PointerPte, Wsle, SampledAgeCounts) \
    (SampledAgeCounts)[(Wsle)->u1.e1.Age] += 1;

//++
//
// BOOLEAN
// MI_WS_GROWING_TOO_FAST (
//     IN PMMSUPPORT VmSupport
//     )
//
// Routine Description:
//
//      Limit the growth rate of processes as the
//      available memory approaches zero.  Note the caller must ensure that
//      MmAvailablePages is low enough so this calculation does not wrap.
//
// Arguments:
//
//      VmSupport - a working set.
//
// Return Value:
//
//      TRUE if the growth rate is too fast, FALSE otherwise.
//
//--

#define MI_WS_GROWING_TOO_FAST(VmSupport) \
    ((VmSupport)->GrowthSinceLastEstimate > \
        (((MI_CLAIM_INCR * (MmAvailablePages*MmAvailablePages)) / (64*64)) + 1))

#define SECTION_BASE_ADDRESS(_NtSection) \
    (*((PVOID *)&(_NtSection)->PointerToRelocations))

#define SECTION_LOCK_COUNT_POINTER(_NtSection) \
    ((PLONG)&(_NtSection)->NumberOfRelocations)

//
// Memory Management Object structures.
//

typedef enum _SECTION_CHECK_TYPE {
    CheckDataSection,
    CheckImageSection,
    CheckUserDataSection,
    CheckBothSection
} SECTION_CHECK_TYPE;

typedef struct _MMEXTEND_INFO {
    UINT64 CommittedSize;
    ULONG ReferenceCount;
} MMEXTEND_INFO, *PMMEXTEND_INFO;

typedef struct _SEGMENT_FLAGS {
    ULONG_PTR TotalNumberOfPtes4132 : 10;
    ULONG_PTR ExtraSharedWowSubsections : 1;
    ULONG_PTR LargePages : 1;
#if defined (_WIN64)
    ULONG_PTR Spare : 52;
#else
    ULONG_PTR Spare : 20;
#endif
} SEGMENT_FLAGS, *PSEGMENT_FLAGS;

typedef struct _SEGMENT {
    struct _CONTROL_AREA *ControlArea;
    ULONG TotalNumberOfPtes;
    ULONG NonExtendedPtes;
    ULONG Spare0;

    UINT64 SizeOfSegment;
    MMPTE SegmentPteTemplate;

    SIZE_T NumberOfCommittedPages;
    PMMEXTEND_INFO ExtendInfo;

    SEGMENT_FLAGS SegmentFlags;
    PVOID BasedAddress;

    //
    // The fields below are for image & pagefile-backed sections only.
    // Common fields are above and new common entries must be added to
    // both the SEGMENT and MAPPED_FILE_SEGMENT declarations.
    //

    union {
        SIZE_T ImageCommitment;     // for image-backed sections only
        PEPROCESS CreatingProcess;  // for pagefile-backed sections only
    } u1;

    union {
        PSECTION_IMAGE_INFORMATION ImageInformation;    // for images only
        PVOID FirstMappedVa;        // for pagefile-backed sections only
    } u2;

    PMMPTE PrototypePte;
    MMPTE ThePtes[MM_PROTO_PTE_ALIGNMENT / PAGE_SIZE];

} SEGMENT, *PSEGMENT;

typedef struct _MAPPED_FILE_SEGMENT {
    struct _CONTROL_AREA *ControlArea;
    ULONG TotalNumberOfPtes;
    ULONG NonExtendedPtes;
    ULONG Spare0;

    UINT64 SizeOfSegment;
    MMPTE SegmentPteTemplate;

    SIZE_T NumberOfCommittedPages;
    PMMEXTEND_INFO ExtendInfo;

    SEGMENT_FLAGS SegmentFlags;
    PVOID BasedAddress;

    struct _MSUBSECTION *LastSubsectionHint;

} MAPPED_FILE_SEGMENT, *PMAPPED_FILE_SEGMENT;

typedef struct _EVENT_COUNTER {
    SLIST_ENTRY ListEntry;
    ULONG RefCount;
    KEVENT Event;
} EVENT_COUNTER, *PEVENT_COUNTER;

typedef struct _MMSECTION_FLAGS {
    unsigned BeingDeleted : 1;
    unsigned BeingCreated : 1;
    unsigned BeingPurged : 1;
    unsigned NoModifiedWriting : 1;

    unsigned FailAllIo : 1;
    unsigned Image : 1;
    unsigned Based : 1;
    unsigned File : 1;

    unsigned Networked : 1;
    unsigned NoCache : 1;
    unsigned PhysicalMemory : 1;
    unsigned CopyOnWrite : 1;

    unsigned Reserve : 1;  // not a spare bit!
    unsigned Commit : 1;
    unsigned FloppyMedia : 1;
    unsigned WasPurged : 1;

    unsigned UserReference : 1;
    unsigned GlobalMemory : 1;
    unsigned DeleteOnClose : 1;
    unsigned FilePointerNull : 1;

    unsigned DebugSymbolsLoaded : 1;
    unsigned SetMappedFileIoComplete : 1;
    unsigned CollidedFlush : 1;
    unsigned NoChange : 1;

    unsigned filler0 : 1;
    unsigned ImageMappedInSystemSpace : 1;
    unsigned UserWritable : 1;
    unsigned Accessed : 1;

    unsigned GlobalOnlyPerSession : 1;
    unsigned Rom : 1;
    unsigned WriteCombined : 1;
    unsigned filler : 1;
} MMSECTION_FLAGS;

typedef struct _CONTROL_AREA {
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    ULONG NumberOfSectionReferences;    // All section refs & image flushes
    ULONG NumberOfPfnReferences;        // valid + transition prototype PTEs
    ULONG NumberOfMappedViews;          // total # mapped views, including
                                        // system cache & system space views
    ULONG NumberOfSystemCacheViews;     // system cache views only
    ULONG NumberOfUserReferences;       // user section & view references
    union {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    PFILE_OBJECT FilePointer;
    PEVENT_COUNTER WaitingForDeletion;
    USHORT ModifiedWriteCount;
    USHORT FlushInProgressCount;
    ULONG WritableUserReferences;
#if !defined (_WIN64)
    ULONG QuadwordPad;
#endif
} CONTROL_AREA, *PCONTROL_AREA;

typedef struct _LARGE_CONTROL_AREA {
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    ULONG NumberOfSectionReferences;
    ULONG NumberOfPfnReferences;
    ULONG NumberOfMappedViews;

    ULONG NumberOfSystemCacheViews;
    ULONG NumberOfUserReferences;
    union {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    PFILE_OBJECT FilePointer;
    PEVENT_COUNTER WaitingForDeletion;
    USHORT ModifiedWriteCount;
    USHORT FlushInProgressCount;
    ULONG WritableUserReferences;
#if !defined (_WIN64)
    ULONG QuadwordPad;
#endif
    PFN_NUMBER StartingFrame;       // only used if Flags.Rom == 1.
    LIST_ENTRY UserGlobalList;
    ULONG SessionId;
} LARGE_CONTROL_AREA, *PLARGE_CONTROL_AREA;

typedef struct _MMSUBSECTION_FLAGS {
    unsigned ReadOnly : 1;
    unsigned ReadWrite : 1;
    unsigned SubsectionStatic : 1;
    unsigned GlobalMemory: 1;
    unsigned Protection : 5;
    unsigned Spare : 1;
    unsigned StartingSector4132 : 10;   // 2 ** (42+12) == 4MB*4GB == 16K TB
    unsigned SectorEndOffset : 12;
} MMSUBSECTION_FLAGS;

typedef struct _SUBSECTION { // Must start on quadword boundary and be quad sized
    PCONTROL_AREA ControlArea;
    union {
        ULONG LongFlags;
        MMSUBSECTION_FLAGS SubsectionFlags;
    } u;
    ULONG StartingSector;
    ULONG NumberOfFullSectors;  // (4GB-1) * 4K == 16TB-4K limit per subsection
    PMMPTE SubsectionBase;
    ULONG UnusedPtes;
    ULONG PtesInSubsection;
    struct _SUBSECTION *NextSubsection;
} SUBSECTION, *PSUBSECTION;

extern const ULONG MMSECT;

//
// Accesses to MMSUBSECTION_FLAGS2 are synchronized via the PFN lock
// (unlike MMSUBSECTION_FLAGS access which is not lock protected at all).
//

typedef struct _MMSUBSECTION_FLAGS2 {
    unsigned SubsectionAccessed : 1;
    unsigned SubsectionConverted : 1;       // only needed for debug
    unsigned Reserved : 30;
} MMSUBSECTION_FLAGS2;

//
// Mapped data file subsection structure.  Not used for images
// or pagefile-backed shared memory.
//

typedef struct _MSUBSECTION { // Must start on quadword boundary and be quad sized
    PCONTROL_AREA ControlArea;
    union {
        ULONG LongFlags;
        MMSUBSECTION_FLAGS SubsectionFlags;
    } u;
    ULONG StartingSector;
    ULONG NumberOfFullSectors;  // (4GB-1) * 4K == 16TB-4K limit per subsection
    PMMPTE SubsectionBase;
    ULONG UnusedPtes;
    ULONG PtesInSubsection;
    struct _SUBSECTION *NextSubsection;
    LIST_ENTRY DereferenceList;
    ULONG_PTR NumberOfMappedViews;
    union {
        ULONG LongFlags2;
        MMSUBSECTION_FLAGS2 SubsectionFlags2;
    } u2;
} MSUBSECTION, *PMSUBSECTION;

#define MI_MAXIMUM_SECTION_SIZE ((UINT64)16*1024*1024*1024*1024*1024 - ((UINT64)1<<MM4K_SHIFT))

VOID
MiDecrementSubsections (
    IN PSUBSECTION FirstSubsection,
    IN PSUBSECTION LastSubsection OPTIONAL
    );

NTSTATUS
MiAddViewsForSectionWithPfn (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    );

NTSTATUS
MiAddViewsForSection (
    IN PMSUBSECTION MappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL,
    IN KIRQL OldIrql,
    OUT PULONG Waited
    );

LOGICAL
MiReferenceSubsection (
    IN PMSUBSECTION MappedSubsection
    );

VOID
MiRemoveViewsFromSection (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    );

VOID
MiRemoveViewsFromSectionWithPfn (
    IN PMSUBSECTION StartMappedSubsection,
    IN UINT64 LastPteOffset OPTIONAL
    );

VOID
MiSubsectionConsistent(
    IN PSUBSECTION Subsection
    );

#if DBG
#define MI_CHECK_SUBSECTION(_subsection) MiSubsectionConsistent((PSUBSECTION)(_subsection))
#else
#define MI_CHECK_SUBSECTION(_subsection)
#endif

//++
// ULONG
// Mi4KStartForSubsection (
//    IN PLARGE_INTEGER address,
//    IN OUT PSUBSECTION subsection
//    );
//
// Routine Description:
//
//    This macro sets into the specified subsection the supplied information
//    indicating the start address (in 4K units) of this portion of the file.
//
// Arguments
//
//    address - Supplies the 64-bit address (in 4K units) of the start of this
//              portion of the file.
//
//    subsection - Supplies the subsection address to store the address in.
//
// Return Value:
//
//    None.
//
//--

#define Mi4KStartForSubsection(address, subsection)  \
   subsection->StartingSector = ((PLARGE_INTEGER)address)->LowPart; \
   subsection->u.SubsectionFlags.StartingSector4132 = \
        (((PLARGE_INTEGER)(address))->HighPart & 0x3ff);

//++
// ULONG
// Mi4KStartFromSubsection (
//    IN OUT PLARGE_INTEGER address,
//    IN PSUBSECTION subsection
//    );
//
// Routine Description:
//
//    This macro gets the start 4K offset from the specified subsection.
//
// Arguments
//
//    address - Supplies the 64-bit address (in 4K units) to place the
//              start of this subsection into.
//
//    subsection - Supplies the subsection address to get the address from.
//
// Return Value:
//
//    None.
//
//--

#define Mi4KStartFromSubsection(address, subsection)  \
   ((PLARGE_INTEGER)address)->LowPart = subsection->StartingSector; \
   ((PLARGE_INTEGER)address)->HighPart = subsection->u.SubsectionFlags.StartingSector4132;

typedef struct _MMDEREFERENCE_SEGMENT_HEADER {
    KSPIN_LOCK Lock;
    KSEMAPHORE Semaphore;
    LIST_ENTRY ListHead;
} MMDEREFERENCE_SEGMENT_HEADER;

//
// This entry is used for calling the segment dereference thread
// to perform page file expansion.  It has a similar structure
// to a control area to allow either a control area or a page file
// expansion entry to be placed on the list.  Note that for a control
// area the segment pointer is valid whereas for page file expansion
// it is null.
//

typedef struct _MMPAGE_FILE_EXPANSION {
    PSEGMENT Segment;
    LIST_ENTRY DereferenceList;
    SIZE_T RequestedExpansionSize;
    SIZE_T ActualExpansion;
    KEVENT Event;
    LONG InProgress;
    ULONG PageFileNumber;
} MMPAGE_FILE_EXPANSION, *PMMPAGE_FILE_EXPANSION;

#define MI_EXTEND_ANY_PAGEFILE      ((ULONG)-1)
#define MI_CONTRACT_PAGEFILES       ((SIZE_T)-1)

typedef struct _MMWORKING_SET_EXPANSION_HEAD {
    LIST_ENTRY ListHead;
} MMWORKING_SET_EXPANSION_HEAD;

#define SUBSECTION_READ_ONLY      1L
#define SUBSECTION_READ_WRITE     2L
#define SUBSECTION_COPY_ON_WRITE  4L
#define SUBSECTION_SHARE_ALLOW    8L

//
// The MMINPAGE_FLAGS relies on the fact that a pool allocation is always
// QUADWORD aligned so the low 3 bits are always available.
//

typedef struct _MMINPAGE_FLAGS {
    ULONG_PTR Completed : 1;
    ULONG_PTR Available1 : 1;
    ULONG_PTR Available2 : 1;
#if defined (_WIN64)
    ULONG_PTR PrefetchMdlHighBits : 61;
#else
    ULONG_PTR PrefetchMdlHighBits : 29;
#endif
} MMINPAGE_FLAGS, *PMMINPAGE_FLAGS;

#define MI_EXTRACT_PREFETCH_MDL(_Support) ((PMDL)((ULONG_PTR)(_Support->u1.PrefetchMdl) & ~(sizeof(QUAD) - 1)))

typedef struct _MMINPAGE_SUPPORT {
    KEVENT Event;
    IO_STATUS_BLOCK IoStatus;
    LARGE_INTEGER ReadOffset;
    LONG WaitCount;
#if defined (_WIN64)
    ULONG UsedPageTableEntries;
#endif
    PETHREAD Thread;
    PFILE_OBJECT FilePointer;
    PMMPTE BasePte;
    PMMPFN Pfn;
    union {
        MMINPAGE_FLAGS e1;
        ULONG_PTR LongFlags;
        PMDL PrefetchMdl;       // Only used under _PREFETCH_
    } u1;
    MDL Mdl;
    PFN_NUMBER Page[MM_MAXIMUM_READ_CLUSTER_SIZE + 1];
    SINGLE_LIST_ENTRY ListEntry;
} MMINPAGE_SUPPORT, *PMMINPAGE_SUPPORT;

#define MI_PF_DUMMY_PAGE_PTE ((PMMPTE)0x23452345)   // Only used by _PREFETCH_

//
// Section support.
//

typedef struct _SECTION {
    MMADDRESS_NODE Address;
    PSEGMENT Segment;
    LARGE_INTEGER SizeOfSection;
    union {
        ULONG LongFlags;
        MMSECTION_FLAGS Flags;
    } u;
    MM_PROTECTION_MASK InitialPageProtection;
} SECTION, *PSECTION;

//
// Banked memory descriptor.  Pointed to by VAD which has
// the PhysicalMemory flags set and the Banked pointer field as
// non-NULL.
//

typedef struct _MMBANKED_SECTION {
    PFN_NUMBER BasePhysicalPage;
    PMMPTE BasedPte;
    ULONG BankSize;
    ULONG BankShift; // shift for PTEs to calculate bank number
    PBANKED_SECTION_ROUTINE BankedRoutine;
    PVOID Context;
    PMMPTE CurrentMappedPte;
    MMPTE BankTemplate[1];
} MMBANKED_SECTION, *PMMBANKED_SECTION;


//
// Virtual address descriptor
//
// ***** NOTE **********
//  The first part of a virtual address descriptor is a MMADDRESS_NODE!!!
//

#if defined (_WIN64)

#define COMMIT_SIZE 51

#if ((COMMIT_SIZE + PAGE_SHIFT) < 63)
#error COMMIT_SIZE too small
#endif

#else
#define COMMIT_SIZE 19

#if ((COMMIT_SIZE + PAGE_SHIFT) < 31)
#error COMMIT_SIZE too small
#endif
#endif

#define MM_MAX_COMMIT (((ULONG_PTR) 1 << COMMIT_SIZE) - 1)

#define MM_VIEW_UNMAP 0
#define MM_VIEW_SHARE 1

typedef enum _MI_VAD_TYPE {
    VadNone,
    VadDevicePhysicalMemory,
    VadImageMap,
    VadAwe,
    VadWriteWatch,
    VadLargePages,
    VadRotatePhysical,
    VadLargePageSection
} MI_VAD_TYPE, *PMI_VAD_TYPE;

typedef struct _MMVAD_FLAGS {
    ULONG_PTR CommitCharge : COMMIT_SIZE; // limits system to 4k pages or bigger!
    ULONG_PTR NoChange : 1;
    ULONG_PTR VadType : 3;
    ULONG_PTR MemCommit: 1;
    ULONG_PTR Protection : 5;
    ULONG_PTR Spare : 2;
    ULONG_PTR PrivateMemory : 1;    // used to tell VAD from VAD_SHORT
} MMVAD_FLAGS;

typedef struct _MMVAD_FLAGS2 {
    unsigned FileOffset : 24;       // number of 64k units into file
    unsigned SecNoChange : 1;       // set if SEC_NOCHANGE specified
    unsigned OneSecured : 1;        // set if u3 field is a range
    unsigned MultipleSecured : 1;   // set if u3 field is a list head
    unsigned ReadOnly : 1;          // protected as ReadOnly
    unsigned LongVad : 1;           // set if VAD is a long VAD
    unsigned ExtendableFile : 1;
    unsigned Inherit : 1;           //1 = ViewShare, 0 = ViewUnmap
    unsigned CopyOnWrite : 1;
} MMVAD_FLAGS2;

typedef struct _MMADDRESS_LIST {
    ULONG_PTR StartVpn;
    ULONG_PTR EndVpn;
} MMADDRESS_LIST, *PMMADDRESS_LIST;

typedef struct _MMSECURE_ENTRY {
    union {
        ULONG_PTR LongFlags2;
        MMVAD_FLAGS2 VadFlags2;
    } u2;
    ULONG_PTR StartVpn;
    ULONG_PTR EndVpn;
    LIST_ENTRY List;
} MMSECURE_ENTRY, *PMMSECURE_ENTRY;

typedef struct _ALIAS_VAD_INFO {
    KAPC Apc;
    ULONG NumberOfEntries;
    ULONG MaximumEntries;
} ALIAS_VAD_INFO, *PALIAS_VAD_INFO;

typedef struct _ALIAS_VAD_INFO2 {
    ULONG BaseAddress;
    HANDLE SecureHandle;
} ALIAS_VAD_INFO2, *PALIAS_VAD_INFO2;

typedef struct _MMVAD {
    union {
        LONG_PTR Balance : 2;
        struct _MMVAD *Parent;
    } u1;
    struct _MMVAD *LeftChild;
    struct _MMVAD *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;

    union {
        ULONG_PTR LongFlags;
        MMVAD_FLAGS VadFlags;
    } u;
    PCONTROL_AREA ControlArea;
    PMMPTE FirstPrototypePte;
    PMMPTE LastContiguousPte;
    union {
        ULONG LongFlags2;
        MMVAD_FLAGS2 VadFlags2;
    } u2;
} MMVAD, *PMMVAD;

typedef struct _MMVAD_LONG {
    union {
        LONG_PTR Balance : 2;
        struct _MMVAD *Parent;
    } u1;
    struct _MMVAD *LeftChild;
    struct _MMVAD *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;

    union {
        ULONG_PTR LongFlags;
        MMVAD_FLAGS VadFlags;
    } u;
    PCONTROL_AREA ControlArea;
    PMMPTE FirstPrototypePte;
    PMMPTE LastContiguousPte;
    union {
        ULONG LongFlags2;
        MMVAD_FLAGS2 VadFlags2;
    } u2;
    union {
        LIST_ENTRY List;
        MMADDRESS_LIST Secured;
    } u3;
    union {
        PMMBANKED_SECTION Banked;
        PMMEXTEND_INFO ExtendedInfo;
    } u4;
} MMVAD_LONG, *PMMVAD_LONG;

typedef struct _MMVAD_SHORT {
    union {
        LONG_PTR Balance : 2;
        struct _MMVAD *Parent;
    } u1;
    struct _MMVAD *LeftChild;
    struct _MMVAD *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;

    union {
        ULONG_PTR LongFlags;
        MMVAD_FLAGS VadFlags;
    } u;
} MMVAD_SHORT, *PMMVAD_SHORT;

#define MI_GET_PROTECTION_FROM_VAD(_Vad) ((ULONG)(_Vad)->u.VadFlags.Protection)

typedef struct _MI_PHYSICAL_VIEW {
    union {
        LONG_PTR Balance : 2;
        struct _MMADDRESS_NODE *Parent;
    } u1;
    struct _MMADDRESS_NODE *LeftChild;
    struct _MMADDRESS_NODE *RightChild;
    ULONG_PTR StartingVpn;      // Actually a virtual address, not a VPN
    ULONG_PTR EndingVpn;        // Actually a virtual address, not a VPN
    PMMVAD Vad;
    MI_VAD_TYPE VadType;
    union {
        PMMPTE MappingPte;      // only if Vad->u.VadFlags.RotatePhysical == 1
        PRTL_BITMAP BitMap;     // only if Vad->u.VadFlags.WriteWatch == 1
    } u;
    PMMINPAGE_SUPPORT InPageSupport;
} MI_PHYSICAL_VIEW, *PMI_PHYSICAL_VIEW;

#define MI_PHYSICAL_VIEW_ROOT_KEY   'rpmM'
#define MI_PHYSICAL_VIEW_KEY        'vpmM'
#define MI_ROTATE_PHYSICAL_VIEW_KEY 'ipmM'
#define MI_WRITEWATCH_VIEW_KEY      'wWmM'

//
// Stuff for support of Write Watch.
//

VOID
MiCaptureWriteWatchDirtyBit (
    IN PEPROCESS Process,
    IN PVOID VirtualAddress
    );

//
// Stuff for support of AWE (Address Windowing Extensions).
//

typedef struct _AWEINFO {
    PRTL_BITMAP VadPhysicalPagesBitMap;
    ULONG_PTR VadPhysicalPages;
    ULONG_PTR VadPhysicalPagesLimit;

    //
    // The PushLock is used to allow most of the NtMapUserPhysicalPages{Scatter}
    // to execute in parallel as this is acquired shared for these calls.
    // Exclusive acquisitions are used to protect maps against frees of the
    // pages as well as to protect updates to the AweVadList.  Collisions
    // should be rare because the exclusive acquisitions should be rare.
    //

    PEX_PUSH_LOCK_CACHE_AWARE PushLock;
    PMI_PHYSICAL_VIEW PhysicalViewHint[MAXIMUM_PROCESSORS];

    MM_AVL_TABLE AweVadRoot;
} AWEINFO, *PAWEINFO;

VOID
MiAweViewInserter (
    IN PEPROCESS Process,
    IN PMI_PHYSICAL_VIEW PhysicalView
    );

VOID
MiAweViewRemover (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    );

WIN32_PROTECTION_MASK
MiProtectAweRegion (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN MM_PROTECTION_MASK ProtectionMask
    );

VOID
MiReleasePhysicalCharges (
    IN SIZE_T NumberOfPages,
    IN PEPROCESS Process
    );

#define MI_ALLOCATION_IS_AWE       0x80000000

PMDL
MiAllocatePagesForMdl (
    IN PHYSICAL_ADDRESS LowAddress,
    IN PHYSICAL_ADDRESS HighAddress,
    IN PHYSICAL_ADDRESS SkipBytes,
    IN SIZE_T TotalBytes,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute,
    IN ULONG Flags
    );

//
// Stuff for support of POSIX Fork.
//

typedef struct _MMCLONE_BLOCK {
    MMPTE ProtoPte;
    LONG CloneRefCount;
} MMCLONE_BLOCK, *PMMCLONE_BLOCK;

typedef struct _MMCLONE_HEADER {
    ULONG NumberOfPtes;
    LONG NumberOfProcessReferences;
    PMMCLONE_BLOCK ClonePtes;
} MMCLONE_HEADER, *PMMCLONE_HEADER;

typedef struct _MMCLONE_DESCRIPTOR {
    union {
        LONG_PTR Balance : 2;
        struct _MMADDRESS_NODE *Parent;
    } u1;
    struct _MMADDRESS_NODE *LeftChild;
    struct _MMADDRESS_NODE *RightChild;
    ULONG_PTR StartingVpn;
    ULONG_PTR EndingVpn;
    ULONG NumberOfPtes;
    PMMCLONE_HEADER CloneHeader;
    LONG NumberOfReferences;
    LONG FinalNumberOfReferences;
    SIZE_T PagedPoolQuotaCharge;
} MMCLONE_DESCRIPTOR, *PMMCLONE_DESCRIPTOR;

//
// The following macro allocates and initializes a bitmap from the
// specified pool of the specified size.
//
//      VOID
//      MiCreateBitMap (
//          OUT PRTL_BITMAP *BitMapHeader,
//          IN SIZE_T SizeOfBitMap,
//          IN POOL_TYPE PoolType
//          );
//

#define MiCreateBitMap(BMH,S,P) {                          \
    ULONG _S;                                              \
    ASSERT ((ULONG64)(S) < _4gb);                          \
    _S = sizeof(RTL_BITMAP) + (ULONG)((((S) + 31) / 32) * 4);         \
    *(BMH) = (PRTL_BITMAP)ExAllocatePoolWithTag( (P), _S, '  mM');       \
    if (*(BMH)) { \
        RtlInitializeBitMap( *(BMH), (PULONG)((*(BMH))+1), (ULONG)(S)); \
    }                                                          \
}

#define MiRemoveBitMap(BMH)     {                          \
    ExFreePool(*(BMH));                                    \
    *(BMH) = NULL;                                         \
}

#define MI_INITIALIZE_ZERO_MDL(MDL) { \
    MDL->Next = (PMDL) NULL; \
    MDL->MdlFlags = 0; \
    MDL->StartVa = NULL; \
    MDL->ByteOffset = 0; \
    MDL->ByteCount = 0; \
    }

//
// Page File structures.
//

typedef struct _MMMOD_WRITER_LISTHEAD {
    LIST_ENTRY ListHead;
    KEVENT Event;
} MMMOD_WRITER_LISTHEAD, *PMMMOD_WRITER_LISTHEAD;

typedef struct _MMMOD_WRITER_MDL_ENTRY {
    LIST_ENTRY Links;
    LARGE_INTEGER WriteOffset;
    union {
        IO_STATUS_BLOCK IoStatus;
        LARGE_INTEGER LastByte;
    } u;
    PIRP Irp;
    ULONG_PTR LastPageToWrite;
    PMMMOD_WRITER_LISTHEAD PagingListHead;
    PLIST_ENTRY CurrentList;
    struct _MMPAGING_FILE *PagingFile;
    PFILE_OBJECT File;
    PCONTROL_AREA ControlArea;
    PERESOURCE FileResource;
    LARGE_INTEGER IssueTime;
    MDL Mdl;
    PFN_NUMBER Page[1];
} MMMOD_WRITER_MDL_ENTRY, *PMMMOD_WRITER_MDL_ENTRY;


#define MM_PAGING_FILE_MDLS 2

typedef struct _MMPAGING_FILE {
    PFN_NUMBER Size;
    PFN_NUMBER MaximumSize;
    PFN_NUMBER MinimumSize;
    PFN_NUMBER FreeSpace;
    PFN_NUMBER CurrentUsage;
    PFN_NUMBER PeakUsage;
    PFN_NUMBER HighestPage;
    PFILE_OBJECT File;
    PMMMOD_WRITER_MDL_ENTRY Entry[MM_PAGING_FILE_MDLS];
    UNICODE_STRING PageFileName;
    PRTL_BITMAP Bitmap;
    struct {
        ULONG PageFileNumber :  4;
        ULONG ReferenceCount :  4;      // really only need 1 bit for this.
        ULONG BootPartition  :  1;
        ULONG Reserved       : 23;
    };
    HANDLE FileHandle;
} MMPAGING_FILE, *PMMPAGING_FILE;

//
// System PTE structures.
//

typedef struct _MMFREE_POOL_ENTRY {
    LIST_ENTRY List;        // maintained free&chk, 1st entry only
    PFN_NUMBER Size;        // maintained free&chk, 1st entry only
    ULONG Signature;        // maintained chk only, all entries
    struct _MMFREE_POOL_ENTRY *Owner; // maintained free&chk, all entries
} MMFREE_POOL_ENTRY, *PMMFREE_POOL_ENTRY;


typedef struct _MMLOCK_CONFLICT {
    LIST_ENTRY List;
    PETHREAD Thread;
} MMLOCK_CONFLICT, *PMMLOCK_CONFLICT;

//
// System view structures
//

typedef struct _MMVIEW {
    ULONG_PTR Entry;
    PCONTROL_AREA ControlArea;
} MMVIEW, *PMMVIEW;

//
// The MMSESSION structure represents kernel memory that is only valid on a
// per-session basis, thus the calling thread must be in the proper session
// to access this structure.
//

typedef struct _MMSESSION {

    //
    // Never refer to the SystemSpaceViewLock directly - always use the pointer
    // following it or you will break support for multiple concurrent sessions.
    //

    KGUARDED_MUTEX SystemSpaceViewLock;

    //
    // This points to the mutex above and is needed because the MMSESSION
    // is mapped in session space and the mutex needs to be globally
    // visible for proper KeWaitForSingleObject & KeSetEvent operation.
    //

    PKGUARDED_MUTEX SystemSpaceViewLockPointer;
    PCHAR SystemSpaceViewStart;
    PMMVIEW SystemSpaceViewTable;
    ULONG SystemSpaceHashSize;
    ULONG SystemSpaceHashEntries;
    ULONG SystemSpaceHashKey;
    ULONG BitmapFailures;
    PRTL_BITMAP SystemSpaceBitMap;

} MMSESSION, *PMMSESSION;

extern MMSESSION   MmSession;

#define LOCK_SYSTEM_VIEW_SPACE(_Session) \
            KeAcquireGuardedMutex (_Session->SystemSpaceViewLockPointer)

#define UNLOCK_SYSTEM_VIEW_SPACE(_Session) \
            KeReleaseGuardedMutex (_Session->SystemSpaceViewLockPointer)

//
// List for flushing TBs singularly.
//

typedef struct _MMPTE_FLUSH_LIST {
    ULONG Count;
    PVOID FlushVa[MM_MAXIMUM_FLUSH_COUNT];
} MMPTE_FLUSH_LIST, *PMMPTE_FLUSH_LIST;

//
// List for flushing WSLEs and TBs singularly.
//

typedef struct _MMWSLE_FLUSH_LIST {
    ULONG Count;
    WSLE_NUMBER FlushIndex[MM_MAXIMUM_FLUSH_COUNT];
} MMWSLE_FLUSH_LIST, *PMMWSLE_FLUSH_LIST;

typedef struct _LOCK_TRACKER {
    LIST_ENTRY ListEntry;
    PMDL Mdl;
    PVOID StartVa;
    PFN_NUMBER Count;
    ULONG Offset;
    ULONG Length;
    PFN_NUMBER Page;
    PVOID CallingAddress;
    PVOID CallersCaller;
    ULONG Who;
    PEPROCESS Process;
} LOCK_TRACKER, *PLOCK_TRACKER;

extern LOGICAL  MmTrackLockedPages;
extern BOOLEAN  MiTrackingAborted;

LOGICAL
MiDereferenceIoSpace (
    IN PMDL MemoryDescriptorList
    );

typedef struct _LOCK_HEADER {
    LIST_ENTRY ListHead;
    PFN_NUMBER Count;
    KSPIN_LOCK Lock;
    LOGICAL Valid;
} LOCK_HEADER, *PLOCK_HEADER;

extern LOGICAL MmSnapUnloads;

#define MI_UNLOADED_DRIVERS 50

extern ULONG MmLastUnloadedDriver;
extern PUNLOADED_DRIVERS MmUnloadedDrivers;

extern BOOLEAN MiAllMainMemoryMustBeCached;

extern ULONG MiFullyInitialized;


VOID
MiInitMachineDependent (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiAddHalIoMappings (
    VOID
    );

VOID
MiReportPhysicalMemory (
    VOID
    );

extern PFN_NUMBER MiNumberOfCompressionPages;

NTSTATUS
MiArmCompressionInterrupt (
    VOID
    );

VOID
MiBuildPagedPool (
    VOID
    );

VOID
MiInitializeNonPagedPool (
    VOID
    );

VOID
MiAddExpansionNonPagedPool (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN LOGICAL PfnHeld
    );

VOID
MiInitializePoolEvents (
    VOID
    );

VOID
MiInitializeNonPagedPoolThresholds (
    VOID
    );

LOGICAL
MiInitializeSystemSpaceMap (
    PVOID Session OPTIONAL
    );

VOID
MiFindInitializationCode (
    OUT PVOID *StartVa,
    OUT PVOID *EndVa
    );

VOID
MiFreeInitializationCode (
    IN PVOID StartVa,
    IN PVOID EndVa
    );

extern ULONG MiNonCachedCollisions;

//
// If /NOLOWMEM is used, this is set to the boundary PFN (pages below this
// value are not used whenever possible).
//

extern PFN_NUMBER MiNoLowMemory;

PVOID
MiAllocateLowMemory (
    IN SIZE_T NumberOfBytes,
    IN PFN_NUMBER LowestAcceptablePfn,
    IN PFN_NUMBER HighestAcceptablePfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PVOID CallingAddress,
    IN MEMORY_CACHING_TYPE CacheType,
    IN ULONG Tag
    );

LOGICAL
MiFreeLowMemory (
    IN PVOID BaseAddress,
    IN ULONG Tag
    );

//
// Move drivers out of the low 16mb that ntldr placed them at - this makes more
// memory below 16mb available for ISA-type drivers that cannot run without it.
//

extern LOGICAL MmMakeLowMemory;

VOID
MiRemoveLowPages (
    IN ULONG RemovePhase
    );

ULONG
MiSectionInitialization (
    VOID
    );

#define MI_MAX_DEREFERENCE_CHUNK (64 * 1024 / PAGE_SIZE)

typedef struct _MI_PFN_DEREFERENCE_CHUNK {
    SINGLE_LIST_ENTRY ListEntry;
    CSHORT Flags;
    USHORT NumberOfPages;
    PFN_NUMBER Pfns[MI_MAX_DEREFERENCE_CHUNK];
} MI_PFN_DEREFERENCE_CHUNK, *PMI_PFN_DEREFERENCE_CHUNK;

extern SLIST_HEADER MmPfnDereferenceSListHead;
extern PSLIST_ENTRY MmPfnDeferredList;

#define MI_DEFER_PFN_HELD               0x1
#define MI_DEFER_DRAIN_LOCAL_ONLY       0x2

VOID
MiDeferredUnlockPages (
     ULONG Flags
     );

LOGICAL
MiFreeAllExpansionNonPagedPool (
    VOID
    );

VOID
MiDecrementReferenceCountForAwePage (
    IN PMMPFN Pfn1,
    IN LOGICAL PfnHeld
    );

//++
// VOID
// MiDecrementReferenceCountInline (
//    IN PMMPFN PFN
//    IN PFN_NUMBER FRAME
//    );
//
// Routine Description:
//
//    MiDecrementReferenceCountInline decrements the reference count inline,
//    only calling MiDecrementReferenceCount if the count would go to zero
//    which would cause the page to be released.
//
// Arguments:
//
//    PFN - Supplies the PFN to decrement.
//
//    FRAME - Supplies the frame matching the above PFN.
//
// Return Value:
//
//    None.
//
// Environment:
//
//    PFN lock held.
//
//--

#define MiDecrementReferenceCountInline(PFN, FRAME)                     \
            MM_PFN_LOCK_ASSERT();                                       \
            ASSERT (MI_PFN_ELEMENT(FRAME) == (PFN));                    \
            ASSERT ((FRAME) <= MmHighestPhysicalPage);                  \
            ASSERT ((PFN)->u3.e2.ReferenceCount != 0);                  \
            if ((PFN)->u3.e2.ReferenceCount != 1) {                     \
                InterlockedDecrementPfn ((PSHORT)&((PFN)->u3.e2.ReferenceCount)); \
            }                                                           \
            else {                                                      \
                MiDecrementReferenceCount (PFN, FRAME);                 \
            }

VOID
FASTCALL
MiDecrementShareCount (
    IN PMMPFN Pfn1,
    IN PFN_NUMBER PageFrameIndex
    );

//++
// VOID
// MiDecrementShareCountInline (
//    IN PMMPFN PFN,
//    IN PFN_NUMBER FRAME
//    );
//
// Routine Description:
//
//    MiDecrementShareCountInline decrements the share count inline,
//    only calling MiDecrementShareCount if the count would go to zero
//    which would cause the page to be released.
//
// Arguments:
//
//    PFN - Supplies the PFN to decrement.
//
//    FRAME - Supplies the frame matching the above PFN.
//
// Return Value:
//
//    None.
//
// Environment:
//
//    PFN lock held.
//
//--

#define MiDecrementShareCountInline(PFN, FRAME)                         \
            if ((PFN)->u2.ShareCount != 1) {                            \
                MM_PFN_LOCK_ASSERT();                                   \
                ASSERT ((FRAME) > 0);                                   \
                ASSERT (MI_IS_PFN(FRAME));                              \
                ASSERT (MI_PFN_ELEMENT(FRAME) == (PFN));                \
                ASSERT ((PFN)->u2.ShareCount != 0);                     \
                if ((PFN)->u3.e1.PageLocation != ActiveAndValid && (PFN)->u3.e1.PageLocation != StandbyPageList) {                                      \
                    KeBugCheckEx (PFN_LIST_CORRUPT, 0x99, FRAME, (PFN)->u3.e1.PageLocation, 0);                                                         \
                }                                                       \
                (PFN)->u2.ShareCount -= 1;                              \
                ASSERT ((PFN)->u2.ShareCount < 0xF000000);              \
            }                                                           \
            else {                                                      \
                MiDecrementShareCount (PFN, FRAME);                     \
            }

//
// Routines which operate on the Page Frame Database Lists
//

VOID
FASTCALL
MiInsertPageInList (
    IN PMMPFNLIST ListHead,
    IN PFN_NUMBER PageFrameIndex
    );

VOID
FASTCALL
MiInsertPageInFreeList (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
FASTCALL
MiInsertZeroListAtBack (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
FASTCALL
MiInsertStandbyListAtFront (
    IN PFN_NUMBER PageFrameIndex
    );

PFN_NUMBER
FASTCALL
MiRemovePageFromList (
    IN PMMPFNLIST ListHead
    );

VOID
FASTCALL
MiUnlinkPageFromList (
    IN PMMPFN Pfn
    );

VOID
MiUnlinkFreeOrZeroedPage (
    IN PMMPFN Pfn
    );

VOID
FASTCALL
MiInsertFrontModifiedNoWrite (
    IN PFN_NUMBER PageFrameIndex
    );

//
// These are the thresholds for handing out an available page.
//

#define MM_LOW_LIMIT                2

#define MM_MEDIUM_LIMIT            32

#define MM_HIGH_LIMIT             128

//
// These are thresholds for enabling various optimizations.
//

#define MM_TIGHT_LIMIT            256

#define MM_PLENTY_FREE_LIMIT     1024

#define MM_VERY_HIGH_LIMIT      10000

#define MM_ENORMOUS_LIMIT       20000

ULONG
FASTCALL
MiEnsureAvailablePageOrWait (
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    );

PFN_NUMBER
MiAllocatePfn (
    IN PMMPTE PointerPte,
    IN ULONG Protection
    );

PFN_NUMBER
FASTCALL
MiRemoveAnyPage (
    IN ULONG PageColor
    );

PFN_NUMBER
FASTCALL
MiRemoveZeroPage (
    IN ULONG PageColor
    );

typedef struct _COLORED_PAGE_INFO {
    union {
        PFN_NUMBER PagesLeftToScan;
#if defined(MI_MULTINODE) 
        KAFFINITY Affinity;
#endif
    };
    PFN_COUNT PagesQueued;
    SCHAR BasePriority;
    PMMPFN PfnAllocation;
    KEVENT Event;
} COLORED_PAGE_INFO, *PCOLORED_PAGE_INFO;

VOID
MiZeroInParallel (
    IN PCOLORED_PAGE_INFO ColoredPageInfoBase
    );

VOID
MiStartZeroPageWorkers (
    VOID
    );

VOID
MiPurgeTransitionList (
    VOID
    );

typedef struct _MM_LDW_WORK_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    PFILE_OBJECT FileObject;
} MM_LDW_WORK_CONTEXT, *PMM_LDW_WORK_CONTEXT;

VOID
MiLdwPopupWorker (
    IN PVOID Context
    );

LOGICAL
MiDereferenceLastChanceLdw (
    IN PMM_LDW_WORK_CONTEXT LdwContext
    );

#define MI_LARGE_PAGE_DRIVER_BUFFER_LENGTH 512

extern WCHAR MmLargePageDriverBuffer[];
extern ULONG MmLargePageDriverBufferLength;

VOID
MiInitializeDriverLargePageList (
    VOID
    );

VOID
MiInitializeLargePageSupport (
    VOID
    );

typedef struct _MI_LARGEPAGE_MEMORY_RUN {
    struct _MI_LARGEPAGE_MEMORY_RUN *Next;
    PFN_NUMBER BasePage;
    PFN_NUMBER PageCount;
} MI_LARGEPAGE_MEMORY_RUN, *PMI_LARGEPAGE_MEMORY_RUN;

VOID
MiReturnLargePages (
    IN PMI_LARGEPAGE_MEMORY_RUN LargePageListHead
    );

PMI_LARGEPAGE_MEMORY_RUN
MiAllocateLargeZeroPages (
    IN PFN_NUMBER NumberOfPages,
    IN MM_PROTECTION_MASK ProtectionMask
    );

NTSTATUS
MiAllocateLargePages (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN MM_PROTECTION_MASK ProtectionMask,
    IN LOGICAL CallerHasPages
    );

PFN_NUMBER
MiFindLargePageMemory (
    IN PCOLORED_PAGE_INFO ColoredPageInfo,
    IN PFN_NUMBER SizeInPages,
    IN MM_PROTECTION_MASK ProtectionMask,
    OUT PPFN_NUMBER OutZeroCount
    );

VOID
MiFreeLargePageMemory (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    );

VOID
MiFreeLargePages (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN LOGICAL CallerHadPages
    );

PVOID
MiMapWithLargePages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN ULONG Protection,
    IN MEMORY_CACHING_TYPE CacheType
    );

VOID
MiUnmapLargePages (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes
    );




LOGICAL
MiMustFrameBeCached (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiSyncCachedRanges (
    VOID
    );

LOGICAL
MiAddCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    );

VOID
MiRemoveCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    );

#define MI_PAGE_FRAME_INDEX_MUST_BE_CACHED(PageFrameIndex)                  \
            MiMustFrameBeCached(PageFrameIndex)



VOID
MiFreeContiguousPages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    );

PFN_NUMBER
MiFindContiguousPages (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType
    );

PVOID
MiFindContiguousMemory (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType,
    IN PVOID CallingAddress
    );

PVOID
MiCheckForContiguousMemory (
    IN PVOID BaseAddress,
    IN PFN_NUMBER BaseAddressPages,
    IN PFN_NUMBER SizeInPages,
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute
    );

//
// Routines which operate on the page frame database entry.
//

VOID
MiInitializePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN ULONG ModifiedState
    );

VOID
MiInitializePfnAndMakePteValid (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN MMPTE NewPteContents
    );

VOID
MiInitializePfnForOtherProcess (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN PFN_NUMBER ContainingPageFrame
    );

VOID
MiInitializeCopyOnWritePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMWSL WorkingSetList
    );

VOID
MiInitializeTransitionPfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte
    );

extern SLIST_HEADER MmInPageSupportSListHead;

VOID
MiFreeInPageSupportBlock (
    IN PMMINPAGE_SUPPORT Support
    );

PMMINPAGE_SUPPORT
MiGetInPageSupportBlock (
    IN KIRQL OldIrql,
    IN PNTSTATUS Status
    );

//
// Routines which require a physical page to be mapped into hyperspace
// within the current process.
//

VOID
FASTCALL
MiZeroPhysicalPage (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
FASTCALL
MiRestoreTransitionPte (
    IN PMMPFN Pfn1
    );

PSUBSECTION
MiGetSubsectionAndProtoFromPte (
    IN PMMPTE PointerPte,
    IN PMMPTE *ProtoPte
    );

PVOID
MiMapPageInHyperSpace (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex,
    OUT PKIRQL OldIrql
    );

PVOID
MiMapPageInHyperSpaceAtDpc (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiUnmapPagesInZeroSpace (
    IN PVOID VirtualAddress,
    IN PFN_COUNT NumberOfPages
    );

PFN_NUMBER
MiGetPageForHeader (
    LOGICAL ZeroPage
    );

VOID
MiRemoveImageHeaderPage (
    IN PFN_NUMBER PageFrameNumber
    );

PVOID
MiMapPagesToZeroInHyperSpace (
    IN PMMPFN PfnAllocation,
    IN PFN_COUNT NumberOfPages
    );


//
// Routines to obtain and release system PTEs.
//

extern ULONG MmTotalSystemPtes;
extern PVOID MiLowestSystemPteVirtualAddress;

//
// Keep a list of the extra PTE ranges.
//

typedef struct _MI_PTE_RANGES {
    PVOID StartingVa;
    PVOID EndingVa;
} MI_PTE_RANGES, *PMI_PTE_RANGES;

#if defined (_WIN64)
#define MI_NUMBER_OF_PTE_RANGES 1
#else
#define MI_NUMBER_OF_PTE_RANGES 8
#endif

extern ULONG MiPteRangeIndex;
extern MI_PTE_RANGES MiPteRanges[MI_NUMBER_OF_PTE_RANGES];

PMMPTE
MiReserveSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

PMMPTE
MiReserveAlignedSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType,
    IN ULONG Alignment
    );

VOID
MiReleaseSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

VOID
MiReleaseSplitSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

VOID
MiIncrementSystemPtes (
    IN ULONG  NumberOfPtes
    );

VOID
MiIssueNoPtesBugcheck (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

VOID
MiInitializeSystemPtes (
    IN PMMPTE StartingPte,
    IN PFN_NUMBER NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPteType
    );

NTSTATUS
MiAddMappedPtes (
    IN PMMPTE FirstPte,
    IN PFN_NUMBER NumberOfPtes,
    IN PCONTROL_AREA ControlArea
    );

VOID
MiInitializeIoTrackers (
    VOID
    );

VOID
MiAddMdlTracker (
    IN PMDL MemoryDescriptorList,
    IN PVOID CallingAddress,
    IN PVOID CallersCaller,
    IN PFN_NUMBER NumberOfPagesToLock,
    IN ULONG Who
    );

PVOID
MiMapSinglePage (
     IN PVOID VirtualAddress OPTIONAL,
     IN PFN_NUMBER PageFrameIndex,
     IN MEMORY_CACHING_TYPE CacheType,
     IN MM_PAGE_PRIORITY Priority
     );

VOID
MiUnmapSinglePage (
     IN PVOID BaseAddress
     );

typedef struct _MM_PTE_MAPPING {
    LIST_ENTRY ListEntry;
    PVOID SystemVa;
    PVOID SystemEndVa;
    ULONG Protection;
} MM_PTE_MAPPING, *PMM_PTE_MAPPING;

extern LIST_ENTRY MmProtectedPteList;

extern KSPIN_LOCK MmProtectedPteLock;

LOGICAL
MiCheckSystemPteProtection (
    IN ULONG_PTR StoreInstruction,
    IN PVOID VirtualAddress
    );

//
// Access Fault routines.
//

#define STATUS_ISSUE_PAGING_IO (0xC0033333)

#define MM_NOIRQL (HIGH_LEVEL + 2)

NTSTATUS
MiDispatchFault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAdress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN LOGICAL RecheckAccess,
    IN PEPROCESS Process,
    IN PVOID TrapInformation,
    IN PMMVAD Vad
    );

NTSTATUS
MiResolveDemandZeroFault (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    );

BOOLEAN
MiIsAddressValid (
    IN PVOID VirtualAddress,
    IN LOGICAL UseForceIfPossible
    );

VOID
MiAllowWorkingSetExpansion (
    IN PMMSUPPORT WsInfo
    );

WSLE_NUMBER
MiAddValidPageToWorkingSet (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN ULONG WsleMask
    );

VOID
MiTrimPte (
    IN PVOID VirtualAddress,
    IN PMMPTE ReadPte,
    IN PMMPFN Pfn1,
    IN PEPROCESS CurrentProcess,
    IN MMPTE NewPteContents
    );

NTSTATUS
MiWaitForInPageComplete (
    IN PMMPFN Pfn,
    IN PMMPTE PointerPte,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPteContents,
    IN PMMINPAGE_SUPPORT InPageSupport,
    IN PEPROCESS CurrentProcess
    );

LOGICAL
FASTCALL
MiCopyOnWrite (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte
    );

LOGICAL
MiSetDirtyBit (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN ULONG PfnHeld
    );

PMMPTE
MiFindActualFaultingPte (
    IN PVOID FaultingAddress
    );

VOID
MiInitializeReadInProgressSinglePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    );

VOID
MiInitializeReadInProgressPfn (
    IN PMDL Mdl,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    );

NTSTATUS
MiAccessCheck (
    IN PMMPTE PointerPte,
    IN ULONG_PTR WriteOperation,
    IN KPROCESSOR_MODE PreviousMode,
    IN ULONG Protection,
    IN PVOID TrapInformation,
    IN BOOLEAN CallerHoldsPfnLock
    );

NTSTATUS
FASTCALL
MiCheckForUserStackOverflow (
    IN PVOID FaultingAddress,
    IN PVOID TrapInformation
    );

PMMPTE
MiCheckVirtualAddress (
    IN PVOID VirtualAddress,
    OUT PULONG ProtectCode,
    OUT PMMVAD *VadOut
    );

NTSTATUS
FASTCALL
MiCheckPdeForPagedPool (
    IN PVOID VirtualAddress
    );

#if defined (_WIN64)
#define MI_IS_WOW64_PROCESS(PROCESS) (PROCESS->Wow64Process)
#else
#define MI_IS_WOW64_PROCESS(PROCESS) NULL
#endif

#define MI_BREAK_ON_AV(VirtualAddress, Id)

//
// Routines which operate on an address tree.
//

PMMADDRESS_NODE
FASTCALL
MiGetNextNode (
    IN PMMADDRESS_NODE Node
    );

PMMADDRESS_NODE
FASTCALL
MiGetPreviousNode (
    IN PMMADDRESS_NODE Node
    );


PMMADDRESS_NODE
FASTCALL
MiGetFirstNode (
    IN PMM_AVL_TABLE Root
    );

PMMADDRESS_NODE
MiGetLastNode (
    IN PMM_AVL_TABLE Root
    );

VOID
FASTCALL
MiInsertNode (
    IN PMMADDRESS_NODE Node,
    IN PMM_AVL_TABLE Root
    );

VOID
FASTCALL
MiRemoveNode (
    IN PMMADDRESS_NODE Node,
    IN PMM_AVL_TABLE Root
    );

PMMADDRESS_NODE
FASTCALL
MiLocateAddressInTree (
    IN ULONG_PTR Vpn,
    IN PMM_AVL_TABLE Root
    );

PMMADDRESS_NODE
MiCheckForConflictingNode (
    IN ULONG_PTR StartVpn,
    IN ULONG_PTR EndVpn,
    IN PMM_AVL_TABLE Root
    );

NTSTATUS
MiFindEmptyAddressRangeInTree (
    IN SIZE_T SizeOfRange,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Root,
    OUT PMMADDRESS_NODE *PreviousVad,
    OUT PVOID *Base
    );

NTSTATUS
MiFindEmptyAddressRangeDownTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Root,
    OUT PVOID *Base
    );

NTSTATUS
MiFindEmptyAddressRangeDownBasedTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Root,
    OUT PVOID *Base
    );

PVOID
MiEnumerateGenericTableWithoutSplayingAvl (
    IN PMM_AVL_TABLE Table,
    IN PVOID *RestartKey
    );

VOID
NodeTreeWalk (
    PMMADDRESS_NODE Start
    );

TABLE_SEARCH_RESULT
MiFindNodeOrParent (
    IN PMM_AVL_TABLE Table,
    IN ULONG_PTR StartingVpn,
    OUT PMMADDRESS_NODE *NodeOrParent
    );

//
// Routines which operate on the tree of virtual address descriptors.
//

NTSTATUS
MiInsertVadCharges (
    IN PMMVAD Vad,
    IN PEPROCESS CurrentProcess
    );

VOID
MiRemoveVadCharges (
    IN PMMVAD Vad,
    IN PEPROCESS CurrentProcess
    );

VOID
FORCEINLINE
MiInsertVad (
    IN PMMVAD Vad,
    IN PEPROCESS CurrentProcess
    )

/*++

Routine Description:

    This function inserts a virtual address descriptor into the tree and
    rebalances the AVL tree as appropriate.

Arguments:

    Vad - Supplies a pointer to a virtual address descriptor.

    Process - Supplies a pointer to the current process.

Return Value:

    None.

--*/

{
    PMM_AVL_TABLE Root;

    ASSERT (Vad->EndingVpn >= Vad->StartingVpn);
    ASSERT (CurrentProcess == PsGetCurrentProcess ());

    Root = &CurrentProcess->VadRoot;

    //
    // Set the hint field in the process to this VAD.
    //

    CurrentProcess->VadRoot.NodeHint = Vad;

    MiInsertNode ((PMMADDRESS_NODE)Vad, Root);

    return;
}

VOID
FORCEINLINE
MiRemoveVad (
    IN PMMVAD Vad,
    IN PEPROCESS CurrentProcess
    )

/*++

Routine Description:

    This function removes a virtual address descriptor from the tree and
    rebalances the AVL tree as appropriate.  Any quota or commitment
    charged to the VAD has been (or will be) returned separately.

Arguments:

    Vad - Supplies a pointer to a virtual address descriptor.

    CurrentProcess - Supplies a pointer to the current process.

Return Value:

    None.

--*/

{
    PMM_AVL_TABLE Root;

    ASSERT (CurrentProcess == PsGetCurrentProcess ());

    Root = &CurrentProcess->VadRoot;

    ASSERT (Root->NumberGenericTableElements >= 1);

    MiRemoveNode ((PMMADDRESS_NODE)Vad, Root);

    //
    // If the hint points at the removed VAD, change the hint.
    //

    if (Root->NodeHint == Vad) {

        Root->NodeHint = Root->BalancedRoot.RightChild;

        if(Root->NumberGenericTableElements == 0) {
            Root->NodeHint = NULL;
        }
    }

    return;
}

PMMVAD
FASTCALL
MiLocateAddress (
    IN PVOID VirtualAddress
    );

NTSTATUS
MiFindEmptyAddressRange (
    IN SIZE_T SizeOfRange,
    IN ULONG_PTR Alignment,
    IN ULONG QuickCheck,
    IN PVOID *Base
    );

ULONG
MiQueryAddressState (
    IN PVOID Va,
    IN PMMVAD Vad,
    IN PEPROCESS TargetProcess,
    OUT PULONG ReturnedProtect,
    OUT PVOID *NextVaToQuery
    );

//
// Routines which operate on the clone tree structure.
//


NTSTATUS
MiCloneProcessAddressSpace (
    IN PEPROCESS ProcessToClone,
    IN PEPROCESS ProcessToInitialize
    );


ULONG
MiDecrementCloneBlockReference (
    IN PMMCLONE_DESCRIPTOR CloneDescriptor,
    IN PMMCLONE_BLOCK CloneBlock,
    IN PEPROCESS CurrentProcess,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL,
    IN KIRQL OldIrql
    );

LOGICAL
MiWaitForForkToComplete (
    IN PEPROCESS CurrentProcess
    );

//
// Routines which operate on the working set list.
//

WSLE_NUMBER
MiAllocateWsle (
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN ULONG_PTR WsleMask
    );

VOID
MiReleaseWsle (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMSUPPORT WsInfo
    );

VOID
MiInitializeWorkingSetList (
    IN PEPROCESS CurrentProcess
    );

VOID
MiGrowWsleHash (
    IN PMMSUPPORT WsInfo
    );

VOID
MiFillWsleHash (
    IN PMMWSL WorkingSetList
    );

WSLE_NUMBER
MiTrimWorkingSet (
    IN WSLE_NUMBER Reduction,
    IN PMMSUPPORT WsInfo,
    IN ULONG TrimAge
    );

#if defined(_AMD64_)
#define MM_PROCESS_COMMIT_CHARGE 6
#elif defined (_X86PAE_)
#define MM_PROCESS_COMMIT_CHARGE 7
#else
#define MM_PROCESS_COMMIT_CHARGE 4
#endif

#define MI_SYSTEM_GLOBAL    0
#define MI_USER_LOCAL       1
#define MI_SESSION_LOCAL    2

LOGICAL
MiTrimAllSystemPageableMemory (
    IN ULONG MemoryType,
    IN LOGICAL PurgeTransition
    );

VOID
MiRemoveWorkingSetPages (
    IN PMMSUPPORT WsInfo
    );

VOID
MiAgeWorkingSet (
    IN PMMSUPPORT VmSupport,
    IN LOGICAL DoAging,
    IN PWSLE_NUMBER WslesScanned,
    IN OUT PPFN_NUMBER TotalClaim,
    IN OUT PPFN_NUMBER TotalEstimatedAvailable
    );

VOID
FASTCALL
MiInsertWsleHash (
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo
    );

VOID
FASTCALL
MiRemoveWsle (
    IN WSLE_NUMBER Entry,
    IN PMMWSL WorkingSetList
    );

VOID
FASTCALL
MiTerminateWsle (
    IN PVOID VirtualAddress,
    IN PMMSUPPORT WsInfo,
    IN WSLE_NUMBER WsPfnIndex
    );

WSLE_NUMBER
FASTCALL
MiLocateWsle (
    IN PVOID VirtualAddress,
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER WsPfnIndex,
    IN LOGICAL Deletion
    );

ULONG
MiFreeWsle (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte
    );

WSLE_NUMBER
MiFreeWsleList (
    IN PMMSUPPORT WsInfo,
    IN PMMWSLE_FLUSH_LIST WsleFlushList
    );

VOID
MiSwapWslEntries (
    IN WSLE_NUMBER SwapEntry,
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo,
    IN LOGICAL EntryNotInHash
    );

VOID
MiRemoveWsleFromFreeList (
    IN WSLE_NUMBER Entry,
    IN PMMWSLE Wsle,
    IN PMMWSL WorkingSetList
    );

ULONG
MiRemovePageFromWorkingSet (
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN PMMSUPPORT WsInfo
    );

#define MI_DELETE_FLUSH_TB  0x1

PFN_NUMBER
MiDeleteSystemPageableVm (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER NumberOfPtes,
    IN ULONG Flags,
    OUT PPFN_NUMBER ResidentPages OPTIONAL
    );

VOID
MiLockCode (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG LockType
    );

PKLDR_DATA_TABLE_ENTRY
MiLookupDataTableEntry (
    IN PVOID AddressWithinSection,
    IN ULONG ResourceHeld
    );

//
// Routines which perform working set management.
//

VOID
MiObtainFreePages (
    VOID
    );

VOID
MiModifiedPageWriter (
    IN PVOID StartContext
    );

VOID
MiMappedPageWriter (
    IN PVOID StartContext
    );

LOGICAL
MiIssuePageExtendRequest (
    IN PMMPAGE_FILE_EXPANSION PageExtend
    );

VOID
MiIssuePageExtendRequestNoWait (
    IN PFN_NUMBER SizeInPages
    );

SIZE_T
MiExtendPagingFiles (
    IN PMMPAGE_FILE_EXPANSION PageExpand
    );

VOID
MiContractPagingFiles (
    VOID
    );

VOID
MiAttemptPageFileReduction (
    VOID
    );

LOGICAL
MiCancelWriteOfMappedPfn (
    IN PFN_NUMBER PageToStop,
    IN KIRQL OldIrql
    );

//
// Routines to delete address space.
//

VOID
MiDeletePteRange (
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte,
    IN LOGICAL AddressSpaceDeletion
    );

VOID
MiDeleteVirtualAddresses (
    IN PUCHAR StartingAddress,
    IN PUCHAR EndingAddress,
    IN PMMVAD Vad
    );

ULONG
MiDeletePte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN ULONG AddressSpaceDeletion,
    IN PEPROCESS CurrentProcess,
    IN PMMPTE PrototypePte,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL,
    IN KIRQL OldIrql
    );

VOID
MiDeleteValidSystemPte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN PMMSUPPORT WsInfo,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL
    );

LOGICAL
MiCreatePageTablesForPhysicalRange (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

VOID
MiDeletePageTablesForPhysicalRange (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

#if defined (_WIN64)
LOGICAL
MiCreatePageDirectoriesForPhysicalRange (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

VOID
MiDeletePageDirectoriesForPhysicalRange (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );
#endif

FORCEINLINE
VOID
MiFlushPteList (
    IN PMMPTE_FLUSH_LIST PteFlushList
    )

/*++

Routine Description:

    This routine flushes all the PTEs in the PTE flush list.
    If the list has overflowed, the entire TB is flushed.

Arguments:

    PteFlushList - Supplies a pointer to the list to be flushed.

    THIS MUST BE A LIST OF USERMODE ADDRESSES !!!

Return Value:

    None.

Environment:

    Kernel mode, PFN or a pre-process AWE lock may optionally be held.

--*/

{
    ULONG count;

    count = PteFlushList->Count;

    ASSERT (count != 0);

    if (count == 1) {
        MI_FLUSH_SINGLE_TB (PteFlushList->FlushVa[0], FALSE);
    }
    else if (count < MM_MAXIMUM_FLUSH_COUNT) {
        MI_FLUSH_MULTIPLE_TB (count, &PteFlushList->FlushVa[0], FALSE);
    }
    else {

        //
        // Array has overflowed, flush the entire TB.
        //

        MI_FLUSH_PROCESS_TB (FALSE);
    }

    PteFlushList->Count = 0;

    return;
}

ULONG
FASTCALL
MiReleasePageFileSpace (
    IN MMPTE PteContents
    );

VOID
FASTCALL
MiUpdateModifiedWriterMdls (
    IN ULONG PageFileNumber
    );

PVOID
MiAllocateAweInfo (
    VOID
    );

VOID
MiFreeAweInfo (
    IN PAWEINFO AweInfo
    );

PVOID
MiInsertAweInfo (
    IN PAWEINFO AweInfo
    );

VOID
MiRemoveUserPhysicalPagesVad (
    IN PMMVAD_SHORT FoundVad
    );

VOID
MiCleanPhysicalProcessPages (
    IN PEPROCESS Process
    );

PFN_NUMBER
MiResidentPagesForSpan (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

LOGICAL
MiChargeResidentAvailable (
    IN PFN_NUMBER NumberOfPages,
    IN ULONG Id
    );

PVOID
MiCreatePhysicalVadRoot (
    IN PEPROCESS Process,
    IN LOGICAL WsHeld
    );

VOID
MiPhysicalViewRemover (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    );

VOID
MiPhysicalViewAdjuster (
    IN PEPROCESS Process,
    IN PMMVAD OldVad,
    IN PMMVAD NewVad
    );

//
// MM_SYSTEM_PAGE_COLOR - MmSystemPageColor
//
// This variable is updated frequently, on MP systems we keep
// a separate system color per processor to avoid cache line
// thrashing.
//

#if defined(NT_UP)

#define MI_SYSTEM_PAGE_COLOR    MmSystemPageColor

#else

#define MI_SYSTEM_PAGE_COLOR    (KeGetCurrentPrcb()->PageColor)

#endif

#if defined(MI_MULTINODE)

extern PKNODE KeNodeBlock[];

#define MI_NODE_FROM_COLOR(c)                                               \
        (KeNodeBlock[(c) >> MmSecondaryColorNodeShift])

#define MI_GET_COLOR_FROM_LIST_ENTRY(index,pfn)                             \
    ((ULONG)(((pfn)->u3.e1.PageColor << MmSecondaryColorNodeShift) |        \
         MI_GET_SECONDARY_COLOR((index),(pfn))))

#define MI_ADJUST_COLOR_FOR_NODE(c,n)   ((c) | (n)->Color)
#define MI_CURRENT_NODE_COLOR           (KeGetCurrentNodeShiftedColor())

#define MiRemoveZeroPageIfAny(COLOR)                                        \
        KeNumberNodes > 1 ? (KeGetCurrentNode()->FreeCount[ZeroedPageList] != 0) ? MiRemoveZeroPage(COLOR) : 0 :                                           \
    (MmFreePagesByColor[ZeroedPageList][COLOR].Flink != MM_EMPTY_LIST) ?    \
                       MiRemoveZeroPage(COLOR) : 0

#define MI_GET_PAGE_COLOR_NODE(n)                                           \
        (((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask) |                \
         KeNodeBlock[n]->MmShiftedColor)

#else

#define MI_NODE_FROM_COLOR(c)

#define MI_GET_COLOR_FROM_LIST_ENTRY(index,pfn)                             \
         ((ULONG)MI_GET_SECONDARY_COLOR((index),(pfn)))

#define MI_ADJUST_COLOR_FOR_NODE(c,n)   (c)
#define MI_CURRENT_NODE_COLOR           0

#define MiRemoveZeroPageIfAny(COLOR)   \
    (MmFreePagesByColor[ZeroedPageList][COLOR].Flink != MM_EMPTY_LIST) ? \
                       MiRemoveZeroPage(COLOR) : 0

#define MI_GET_PAGE_COLOR_NODE(n)                                           \
        ((MI_SYSTEM_PAGE_COLOR++) & MmSecondaryColorMask)

#endif

FORCEINLINE
PFN_NUMBER
MiRemoveZeroPageMayReleaseLocks (
    IN ULONG Color,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This routine returns a zeroed page.
    
    It may release and reacquire the PFN lock to do so, as well as mapping
    the page in hyperspace to perform the actual zeroing if necessary.

Environment:

    Kernel mode.  PFN lock held, hyperspace lock NOT held.

--*/

{
    PFN_NUMBER PageFrameIndex;

    PageFrameIndex = MiRemoveZeroPageIfAny (Color);

    if (PageFrameIndex == 0) {
        PageFrameIndex = MiRemoveAnyPage (Color);
        UNLOCK_PFN (OldIrql);
        MiZeroPhysicalPage (PageFrameIndex);
        LOCK_PFN (OldIrql);
    }

    return PageFrameIndex;
}

//
// General support routines.
//

#if (_MI_PAGING_LEVELS <= 3)

//++
// PMMPTE
// MiGetPxeAddress (
//    IN PVOID va
//    );
//
// Routine Description:
//
//    MiGetPxeAddress returns the address of the extended page directory parent
//    entry which maps the given virtual address.  This is one level above the
//    page parent directory.
//
// Arguments
//
//    Va - Supplies the virtual address to locate the PXE for.
//
// Return Value:
//
//    The address of the PXE.
//
//--

#define MiGetPxeAddress(va)   ((PMMPTE)0)

//++
// LOGICAL
// MiIsPteOnPxeBoundary (
//    IN PVOID PTE
//    );
//
// Routine Description:
//
//    MiIsPteOnPxeBoundary returns TRUE if the PTE is
//    on an extended page directory parent entry boundary.
//
// Arguments
//
//    PTE - Supplies the PTE to check.
//
// Return Value:
//
//    TRUE if on a boundary, FALSE if not.
//
//--

#define MiIsPteOnPxeBoundary(PTE) (FALSE)

#endif

#if (_MI_PAGING_LEVELS <= 2)

//++
// PMMPTE
// MiGetPpeAddress (
//    IN PVOID va
//    );
//
// Routine Description:
//
//    MiGetPpeAddress returns the address of the page directory parent entry
//    which maps the given virtual address.  This is one level above the
//    page directory.
//
// Arguments
//
//    Va - Supplies the virtual address to locate the PPE for.
//
// Return Value:
//
//    The address of the PPE.
//
//--

#define MiGetPpeAddress(va)  ((PMMPTE)0)

//++
// LOGICAL
// MiIsPteOnPpeBoundary (
//    IN PVOID VA
//    );
//
// Routine Description:
//
//    MiIsPteOnPpeBoundary returns TRUE if the PTE is
//    on a page directory parent entry boundary.
//
// Arguments
//
//    VA - Supplies the virtual address to check.
//
// Return Value:
//
//    TRUE if on a boundary, FALSE if not.
//
//--

#define MiIsPteOnPpeBoundary(PTE) (FALSE)

#endif

ULONG
MiDoesPdeExistAndMakeValid (
    IN PMMPTE PointerPde,
    IN PEPROCESS TargetProcess,
    IN KIRQL OldIrql,
    OUT PULONG Waited
    );

#if (_MI_PAGING_LEVELS >= 3)
#define MiDoesPpeExistAndMakeValid(PPE, PROCESS, PFNLOCKIRQL, WAITED) \
            MiDoesPdeExistAndMakeValid(PPE, PROCESS, PFNLOCKIRQL, WAITED)
#else
#define MiDoesPpeExistAndMakeValid(PPE, PROCESS, PFNLOCKIRQL, WAITED) 1
#endif

#if (_MI_PAGING_LEVELS >= 4)
#define MiDoesPxeExistAndMakeValid(PXE, PROCESS, PFNLOCKIRQL, WAITED) \
            MiDoesPdeExistAndMakeValid(PXE, PROCESS, PFNLOCKIRQL, WAITED)
#else
#define MiDoesPxeExistAndMakeValid(PXE, PROCESS, PFNLOCKIRQL, WAITED) 1
#endif

VOID
MiMakePdeExistAndMakeValid (
    IN PMMPTE PointerPde,
    IN PEPROCESS TargetProcess,
    IN KIRQL OldIrql
    );

ULONG
FASTCALL
MiMakeSystemAddressValid (
    IN PVOID VirtualAddress,
    IN PEPROCESS CurrentProcess
    );

ULONG
FASTCALL
MiMakeSystemAddressValidPfnWs (
    IN PVOID VirtualAddress,
    IN PEPROCESS CurrentProcess,
    IN KIRQL OldIrql
    );

ULONG
FASTCALL
MiMakeSystemAddressValidPfnSystemWs (
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    );

ULONG
FASTCALL
MiMakeSystemAddressValidPfn (
    IN PVOID VirtualAddress,
    IN KIRQL OldIrql
    );


ULONG
FORCEINLINE
MiDoesPdeExistAndMakeValid (
    IN PMMPTE PointerPde,
    IN PEPROCESS TargetProcess,
    IN KIRQL OldIrql,
    OUT PULONG Waited
    )

/*++

Routine Description:

    This routine examines the specified Page Directory Entry to determine
    if the page table page mapped by the PDE exists.

    If the page table page exists and is not currently in memory, the
    working set pushlock and, if held, the PFN lock are released and the
    page table page is faulted into the working set.  The pushlock and PFN
    lock are reacquired.

    If the PDE exists, the function returns TRUE.

Arguments:

    PointerPde - Supplies a pointer to the PDE to examine and potentially
                 bring into the working set.

    TargetProcess - Supplies a pointer to the current process.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at or MM_NOIRQL
              if the caller does not hold the PFN lock.

    Waited - Supplies a pointer to a ULONG to increment if the pushlock is
             released and reacquired.  Note this value may be incremented
             more than once.

Return Value:

    TRUE if the PDE exists, FALSE if the PDE is zero.

Environment:

    Kernel mode, APCs disabled, working set pushlock held.

--*/

{
    PMMPTE PointerPte;

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    if (PointerPde->u.Long == 0) {

        //
        // This page directory entry doesn't exist, return FALSE.
        //

        return FALSE;
    }

    if (PointerPde->u.Hard.Valid == 1) {

        //
        // Already valid.
        //

        return TRUE;
    }

    //
    // Page directory entry exists, it is either valid, in transition
    // or in the paging file.  Fault it in.
    //

    if (OldIrql != MM_NOIRQL) {
        UNLOCK_PFN (OldIrql);
        ASSERT (KeAreAllApcsDisabled () == TRUE);
        *Waited += 1;
    }

    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

    *Waited += MiMakeSystemAddressValid (PointerPte, TargetProcess);

    if (OldIrql != MM_NOIRQL) {
        LOCK_PFN (OldIrql);
    }
    return TRUE;
}

VOID
FASTCALL
MiLockPagedAddress (
    IN PVOID VirtualAddress
    );

VOID
FASTCALL
MiUnlockPagedAddress (
    IN PVOID VirtualAddress
    );

ULONG
FORCEINLINE
MiIsPteDecommittedPage (
    IN PMMPTE PointerPte
    )

/*++

Routine Description:

    This function checks the contents of a PTE to determine if the
    PTE is explicitly decommitted.

    If the PTE is a prototype PTE and the protection is not in the
    prototype PTE, the value FALSE is returned.

Arguments:

    PointerPte - Supplies a pointer to the PTE to examine.

Return Value:

    TRUE if the PTE is in the explicit decommitted state.
    FALSE if the PTE is not in the explicit decommitted state.

Environment:

    Kernel mode, APCs disabled, working set pushlock held.

--*/

{
    MMPTE PteContents;

    PteContents = *PointerPte;

    //
    // If the protection in the PTE is not decommitted, return FALSE.
    //

    if (PteContents.u.Soft.Protection != MM_DECOMMIT) {
        return FALSE;
    }

    //
    // Check to make sure the protection field is really being interpreted
    // correctly.
    //

    if (PteContents.u.Hard.Valid == 1) {

        //
        // The PTE is valid and therefore cannot be decommitted.
        //

        return FALSE;
    }

    if ((PteContents.u.Soft.Prototype == 1) &&
         (PteContents.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED)) {

        //
        // The PTE's protection is not known as it is in
        // prototype PTE format.  Return FALSE.
        //

        return FALSE;
    }

    //
    // It is a decommitted PTE.
    //

    return TRUE;
}

ULONG
FASTCALL
MiIsProtectionCompatible (
    IN ULONG OldProtect,
    IN ULONG NewProtect
    );

ULONG
FASTCALL
MiIsPteProtectionCompatible (
    IN ULONG OldPteProtection,
    IN ULONG NewProtect
    );

ULONG
FASTCALL
MiMakeProtectionMask (
    IN ULONG Protect
    );

ULONG
MiIsEntireRangeCommitted (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

ULONG
MiIsEntireRangeDecommitted (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

LOGICAL
MiCheckProtoPtePageState (
    IN PMMPTE PrototypePte,
    IN KIRQL OldIrql,
    OUT PLOGICAL DroppedPfnLock
    );

//++
// PMMPTE
// MiGetProtoPteAddress (
//    IN PMMPTE VAD,
//    IN ULONG_PTR VPN
//    );
//
// Routine Description:
//
//    MiGetProtoPteAddress returns a pointer to the prototype PTE which
//    is mapped by the given virtual address descriptor and address within
//    the virtual address descriptor.
//
// Arguments
//
//    VAD - Supplies a pointer to the virtual address descriptor that contains
//          the VA.
//
//    VPN - Supplies the virtual page number.
//
// Return Value:
//
//    A pointer to the proto PTE which corresponds to the VA.
//
//--


#define MiGetProtoPteAddress(VAD,VPN)                                        \
    ((((((VPN) - (VAD)->StartingVpn) << PTE_SHIFT) +                         \
      (ULONG_PTR)(VAD)->FirstPrototypePte) <= (ULONG_PTR)(VAD)->LastContiguousPte) ? \
    ((PMMPTE)(((((VPN) - (VAD)->StartingVpn) << PTE_SHIFT) +                 \
        (ULONG_PTR)(VAD)->FirstPrototypePte))) :                                  \
        MiGetProtoPteAddressExtended ((VAD),(VPN)))

PMMPTE
FASTCALL
MiGetProtoPteAddressExtended (
    IN PMMVAD Vad,
    IN ULONG_PTR Vpn
    );

PSUBSECTION
FASTCALL
MiLocateSubsection (
    IN PMMVAD Vad,
    IN ULONG_PTR Vpn
    );

VOID
MiInitializeSystemCache (
    IN ULONG MinimumWorkingSet,
    IN ULONG MaximumWorkingSet
    );

VOID
MiAdjustWorkingSetManagerParameters(
    IN LOGICAL WorkStation
    );

extern PFN_NUMBER MmLowMemoryThreshold;
extern PFN_NUMBER MmHighMemoryThreshold;

extern PFN_NUMBER MiLowPagedPoolThreshold;
extern PFN_NUMBER MiHighPagedPoolThreshold;

extern PFN_NUMBER MiLowNonPagedPoolThreshold;
extern PFN_NUMBER MiHighNonPagedPoolThreshold;

extern PKEVENT MiLowPagedPoolEvent;
extern PKEVENT MiHighPagedPoolEvent;

extern PKEVENT MiLowNonPagedPoolEvent;
extern PKEVENT MiHighNonPagedPoolEvent;

//
// Section support
//

VOID
FASTCALL
MiInsertBasedSection (
    IN PSECTION Section
    );

NTSTATUS
MiMapViewOfPhysicalSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN ULONG ProtectionMask,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType
    );

NTSTATUS
MiMapViewOfDataSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PLARGE_INTEGER SectionOffset,
    IN PSIZE_T CapturedViewSize,
    IN PSECTION Section,
    IN SECTION_INHERIT InheritDisposition,
    IN ULONG ProtectionMask,
    IN SIZE_T CommitSize,
    IN ULONG_PTR ZeroBits,
    IN ULONG AllocationType
    );

#define UNMAP_ADDRESS_SPACE_HELD    0x1
#define UNMAP_ROTATE_PHYSICAL_OK    0x2

NTSTATUS
MiUnmapViewOfSection (
    IN PEPROCESS Process,
    IN PVOID BaseAddress,
    IN ULONG UnmapFlags
    );

VOID
MiRemoveImageSectionObject(
    IN PFILE_OBJECT File,
    IN PCONTROL_AREA ControlArea
    );

VOID
MiAddSystemPtes(
    IN PMMPTE StartingPte,
    IN ULONG  NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

VOID
MiRemoveMappedView (
    IN PEPROCESS CurrentProcess,
    IN PMMVAD Vad
    );

VOID
MiSegmentDelete (
    PSEGMENT Segment
    );

VOID
MiSectionDelete (
    IN PVOID Object
    );

VOID
MiDereferenceSegmentThread (
    IN PVOID StartContext
    );

NTSTATUS
MiCreateImageFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment
    );

NTSTATUS
MiCreateDataFileMap (
    IN PFILE_OBJECT File,
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN ULONG SectionPageProtection,
    IN ULONG AllocationAttributes,
    IN ULONG IgnoreFileSizing
    );

NTSTATUS
MiCreatePagingFileMap (
    OUT PSEGMENT *Segment,
    IN PUINT64 MaximumSize,
    IN ULONG ProtectionMask,
    IN ULONG AllocationAttributes
    );

VOID
MiPurgeSubsectionInternal (
    IN PSUBSECTION Subsection,
    IN ULONG PteOffset
    );

VOID
MiPurgeImageSection (
    IN PCONTROL_AREA ControlArea,
    IN KIRQL OldIrql
    );

PCONTROL_AREA
MiFindImageSectionObject (
    IN PFILE_OBJECT File,
    IN LOGICAL PfnLockHeld,
    OUT PLOGICAL GlobalNeeded
    );

VOID
MiCleanSection (
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL DirtyDataPagesOk
    );

VOID
MiDereferenceControlArea (
    IN PCONTROL_AREA ControlArea
    );

VOID
MiDereferenceControlAreaBySection (
    IN PCONTROL_AREA ControlArea,
    IN ULONG UserRef
    );

VOID
MiCheckControlArea (
    IN PCONTROL_AREA ControlArea,
    IN KIRQL PreviousIrql
    );

NTSTATUS
MiCheckPurgeAndUpMapCount (
    IN PCONTROL_AREA ControlArea,
    IN LOGICAL FailIfSystemViews
    );

VOID
MiCheckForControlAreaDeletion (
    IN PCONTROL_AREA ControlArea
    );

LOGICAL
MiCheckControlAreaStatus (
    IN SECTION_CHECK_TYPE SectionCheckType,
    IN PSECTION_OBJECT_POINTERS SectionObjectPointers,
    IN ULONG DelayClose,
    OUT PCONTROL_AREA *ControlArea,
    OUT PKIRQL OldIrql
    );

extern SLIST_HEADER MmEventCountSListHead;

PEVENT_COUNTER
MiGetEventCounter (
    VOID
    );

VOID
MiFreeEventCounter (
    IN PEVENT_COUNTER Support
    );

ULONG
MiCanFileBeTruncatedInternal (
    IN PSECTION_OBJECT_POINTERS SectionPointer,
    IN PLARGE_INTEGER NewFileSize OPTIONAL,
    IN LOGICAL BlockNewViews,
    OUT PKIRQL PreviousIrql
    );

NTSTATUS
MiFlushSectionInternal (
    IN PMMPTE StartingPte,
    IN PMMPTE FinalPte,
    IN PSUBSECTION FirstSubsection,
    IN PSUBSECTION LastSubsection,
    IN ULONG Flags,
    OUT PIO_STATUS_BLOCK IoStatus
    );

//
// protection stuff...
//

NTSTATUS
MiProtectVirtualMemory (
    IN PEPROCESS Process,
    IN PVOID *CapturedBase,
    IN PSIZE_T CapturedRegionSize,
    IN ULONG Protect,
    IN PULONG LastProtect
    );

ULONG
MiGetPageProtection (
    IN PMMPTE PointerPte
    );

NTSTATUS
MiSetProtectionOnSection (
    IN PEPROCESS Process,
    IN PMMVAD Vad,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN ULONG NewProtect,
    OUT PULONG CapturedOldProtect,
    IN ULONG DontCharge,
    OUT PULONG Locked
    );

NTSTATUS
MiCheckSecuredVad (
    IN PMMVAD Vad,
    IN PVOID Base,
    IN ULONG_PTR Size,
    IN ULONG ProtectionMask
    );

HANDLE
MiSecureVirtualMemory (
    IN PVOID Address,
    IN SIZE_T Size,
    IN ULONG ProbeMode,
    IN LOGICAL AddressSpaceMutexHeld
    );

VOID
MiUnsecureVirtualMemory (
    IN HANDLE SecureHandle,
    IN LOGICAL AddressSpaceMutexHeld
    );

ULONG
MiChangeNoAccessForkPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    );

//
// Routines for charging quota and commitment.
//

VOID
MiTrimSegmentCache (
    VOID
    );

VOID
MiInitializeCommitment (
    VOID
    );

LOGICAL
FASTCALL
MiChargeCommitment (
    IN SIZE_T QuotaCharge,
    IN PEPROCESS Process OPTIONAL
    );

LOGICAL
FASTCALL
MiChargeCommitmentCantExpand (
    IN SIZE_T QuotaCharge,
    IN ULONG MustSucceed
    );

LOGICAL
FASTCALL
MiChargeTemporaryCommitmentForReduction (
    IN SIZE_T QuotaCharge
    );

VOID
MiCauseOverCommitPopup (
    VOID
    );


extern SIZE_T MmTotalCommitLimitMaximum;

SIZE_T
MiCalculatePageCommitment (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

VOID
MiReturnPageTablePageCommitment (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PEPROCESS CurrentProcess,
    IN PMMVAD PreviousVad,
    IN PMMVAD NextVad
    );

VOID
MiFlushAllPages (
    VOID
    );

VOID
MiModifiedPageWriterTimerDispatch (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );


LONGLONG
FORCEINLINE
MiStartingOffset (
    IN PSUBSECTION Subsection,
    IN PMMPTE PteAddress
    )

/*++

Routine Description:

    This function calculates the file offset given a subsection and a PTE
    offset.  Note that images are stored in 512-byte units whereas data is
    stored in 4K units.

Arguments:

    Subsection - Supplies a subsection to reference for the file address.

    PteAddress - Supplies a PTE within the subsection

Return Value:

    Returns the file offset to obtain the backing data from.

--*/

{
    LONGLONG PteByteOffset;
    LARGE_INTEGER StartAddress;

    if (Subsection->ControlArea->u.Flags.Image == 1) {
        return MI_STARTING_OFFSET (Subsection, PteAddress);
    }

    ASSERT (Subsection->SubsectionBase != NULL);

    PteByteOffset = (LONGLONG)((PteAddress - Subsection->SubsectionBase))
                            << PAGE_SHIFT;

    Mi4KStartFromSubsection (&StartAddress, Subsection);

    StartAddress.QuadPart = StartAddress.QuadPart << MM4K_SHIFT;

    PteByteOffset += StartAddress.QuadPart;

    return PteByteOffset;
}

LARGE_INTEGER
FORCEINLINE
MiEndingOffset (
    IN PSUBSECTION Subsection
    )

/*++

Routine Description:

    This function calculates the last valid file offset in a given subsection.
    offset.  Note that images are stored in 512-byte units whereas data is
    stored in 4K units.

Arguments:

    Subsection - Supplies a subsection to reference for the file address.

    PteAddress - Supplies a PTE within the subsection

Return Value:

    Returns the file offset to obtain the backing data from.

--*/

{
    LARGE_INTEGER FileByteOffset;

    if (Subsection->ControlArea->u.Flags.Image == 1) {
        FileByteOffset.QuadPart =
            ((UINT64)Subsection->StartingSector + (UINT64)Subsection->NumberOfFullSectors) <<
                MMSECTOR_SHIFT;
    }
    else {
        Mi4KStartFromSubsection (&FileByteOffset, Subsection);

        FileByteOffset.QuadPart += Subsection->NumberOfFullSectors;

        FileByteOffset.QuadPart = FileByteOffset.QuadPart << MM4K_SHIFT;
    }

    FileByteOffset.QuadPart += Subsection->u.SubsectionFlags.SectorEndOffset;

    return FileByteOffset;
}

#define KERNEL_NAME L"ntoskrnl.exe"
#define HAL_NAME    L"hal.dll"

VOID
MiReloadBootLoadedDrivers (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
MiSetSystemCodeProtection (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG ProtectionMask
    );

VOID
MiWriteProtectSystemImage (
    IN PVOID DllBase
    );

VOID
MiSetIATProtect (
    IN PVOID DllBase,
    IN ULONG Protection
    );

VOID
MiMakeEntireImageCopyOnWrite (
    IN PSUBSECTION Subsection
    );

LOGICAL
MiInitializeLoadedModuleList (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

#define UNICODE_TAB               0x0009
#define UNICODE_LF                0x000A
#define UNICODE_CR                0x000D
#define UNICODE_SPACE             0x0020
#define UNICODE_CJK_SPACE         0x3000

#define UNICODE_WHITESPACE(_ch)     (((_ch) == UNICODE_TAB) || \
                                     ((_ch) == UNICODE_LF) || \
                                     ((_ch) == UNICODE_CR) || \
                                     ((_ch) == UNICODE_SPACE) || \
                                     ((_ch) == UNICODE_CJK_SPACE) || \
                                     ((_ch) == UNICODE_NULL))

extern ULONG MmSpecialPoolTag;
extern PVOID MmSpecialPoolStart;
extern PVOID MmSpecialPoolEnd;
extern PVOID MmSessionSpecialPoolStart;
extern PVOID MmSessionSpecialPoolEnd;

LOGICAL
MiInitializeSpecialPool (
    IN POOL_TYPE PoolType
    );

#if defined (_WIN64)
LOGICAL
MiInitializeSessionSpecialPool (
    VOID
    );

VOID
MiDeleteSessionSpecialPool (
    VOID
    );
#endif

POOL_TYPE
MmQuerySpecialPoolBlockType (
    IN PVOID P
    );

#if defined (_X86_)
LOGICAL
MiRecoverSpecialPtes (
    IN ULONG NumberOfPtes
    );

VOID
MiAddExtraSystemPteRanges (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER NumberOfPtes
    );
#endif

VOID
MiEnableRandomSpecialPool (
    IN LOGICAL Enable
    );

LOGICAL
MiTriageSystem (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiTriageAddDrivers (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiTriageVerifyDriver (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

extern ULONG MmTriageActionTaken;

#if defined (_WIN64)
#define MM_SPECIAL_POOL_PTES ((1024 * 1024) / sizeof (MMPTE))
#else
#define MM_SPECIAL_POOL_PTES (24 * PTE_PER_PAGE)
#endif

#define MI_SUSPECT_DRIVER_BUFFER_LENGTH 512

extern WCHAR MmVerifyDriverBuffer[];
extern ULONG MmVerifyDriverBufferLength;
extern ULONG MmVerifyDriverLevel;

extern LOGICAL MmSnapUnloads;
extern LOGICAL MmProtectFreedNonPagedPool;
extern ULONG MmEnforceWriteProtection;
extern LOGICAL MmTrackLockedPages;
extern ULONG MmTrackPtes;

#define VI_POOL_FREELIST_END  ((ULONG_PTR)-1)

#define VI_POOL_PAGE_HEADER_SIGNATURE 0x21321345

typedef struct _VI_POOL_PAGE_HEADER {
    PSLIST_ENTRY NextPage;
    PVOID VerifierEntry;
    ULONG_PTR Signature;
} VI_POOL_PAGE_HEADER, *PVI_POOL_PAGE_HEADER;

typedef struct _VI_POOL_ENTRY_INUSE {
    PVOID VirtualAddress;
    PVOID CallingAddress;
    SIZE_T NumberOfBytes;
    ULONG_PTR Tag;
} VI_POOL_ENTRY_INUSE, *PVI_POOL_ENTRY_INUSE;

typedef struct _VI_POOL_ENTRY {
    union {
        VI_POOL_PAGE_HEADER PageHeader;
        VI_POOL_ENTRY_INUSE InUse;
        PSLIST_ENTRY NextFree;
    };
} VI_POOL_ENTRY, *PVI_POOL_ENTRY;

#define MI_VERIFIER_ENTRY_SIGNATURE            0x98761940

typedef struct _MI_VERIFIER_DRIVER_ENTRY {
    LIST_ENTRY Links;
    ULONG Loads;
    ULONG Unloads;

    UNICODE_STRING BaseName;
    PVOID StartAddress;
    PVOID EndAddress;

#define VI_VERIFYING_DIRECTLY   0x1
#define VI_VERIFYING_INVERSELY  0x2
#define VI_DISABLE_VERIFICATION 0x4

    ULONG Flags;
    ULONG_PTR Signature;

    SLIST_HEADER PoolPageHeaders;
    SLIST_HEADER PoolTrackers;

    ULONG CurrentPagedPoolAllocations;
    ULONG CurrentNonPagedPoolAllocations;
    ULONG PeakPagedPoolAllocations;
    ULONG PeakNonPagedPoolAllocations;

    SIZE_T PagedBytes;
    SIZE_T NonPagedBytes;
    SIZE_T PeakPagedBytes;
    SIZE_T PeakNonPagedBytes;

} MI_VERIFIER_DRIVER_ENTRY, *PMI_VERIFIER_DRIVER_ENTRY;

typedef struct _MI_VERIFIER_POOL_HEADER {
    PVI_POOL_ENTRY VerifierPoolEntry;
} MI_VERIFIER_POOL_HEADER, *PMI_VERIFIER_POOL_HEADER;

typedef struct _MM_DRIVER_VERIFIER_DATA {
    ULONG Level;
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
    ULONG UserTrims;

    ULONG CurrentPagedPoolAllocations;
    ULONG CurrentNonPagedPoolAllocations;
    ULONG PeakPagedPoolAllocations;
    ULONG PeakNonPagedPoolAllocations;

    SIZE_T PagedBytes;
    SIZE_T NonPagedBytes;
    SIZE_T PeakPagedBytes;
    SIZE_T PeakNonPagedBytes;

    ULONG BurstAllocationsFailedDeliberately;
    ULONG SessionTrims;
    ULONG Reserved[2];

} MM_DRIVER_VERIFIER_DATA, *PMM_DRIVER_VERIFIER_DATA;

VOID
MiInitializeDriverVerifierList (
    VOID
    );

LOGICAL
MiInitializeVerifyingComponents (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

LOGICAL
MiApplyDriverVerifier (
    IN PKLDR_DATA_TABLE_ENTRY,
    IN PMI_VERIFIER_DRIVER_ENTRY Verifier
    );

VOID
MiReApplyVerifierToLoadedModules(
    IN PLIST_ENTRY ModuleListHead
    );

VOID
MiVerifyingDriverUnloading (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

VOID
MiVerifierCheckThunks (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

extern ULONG MiActiveVerifierThunks;
extern LIST_ENTRY MiSuspectDriverList;

extern ULONG MiVerifierThunksAdded;

VOID
MiEnableKernelVerifier (
    VOID
    );

extern LOGICAL KernelVerifier;

extern MM_DRIVER_VERIFIER_DATA MmVerifierData;

#define MI_FREED_SPECIAL_POOL_SIGNATURE 0x98764321

#define MI_STACK_BYTES 1024

typedef struct _MI_FREED_SPECIAL_POOL {
    POOL_HEADER OverlaidPoolHeader;
    MI_VERIFIER_POOL_HEADER OverlaidVerifierPoolHeader;

    ULONG Signature;
    ULONG TickCount;
    ULONG NumberOfBytesRequested;
    ULONG Pageable;

    PVOID VirtualAddress;
    PVOID StackPointer;
    ULONG StackBytes;
    PETHREAD Thread;

    UCHAR StackData[MI_STACK_BYTES];
} MI_FREED_SPECIAL_POOL, *PMI_FREED_SPECIAL_POOL;

//
// Types of resident available page charges.
//

#define MM_RESAVAIL_ALLOCATE_ZERO_PAGE_CLUSTERS          0
#define MM_RESAVAIL_ALLOCATE_PAGETABLES_FOR_PAGED_POOL   1
#define MM_RESAVAIL_ALLOCATE_GROW_BSTORE                 2
#define MM_RESAVAIL_ALLOCATE_CONTIGUOUS                  3
#define MM_RESAVAIL_FREE_OUTPAGE_BSTORE                  4
#define MM_RESAVAIL_FREE_PAGE_DRIVER                     5
#define MM_RESAVAIL_ALLOCATE_CREATE_PROCESS              6
#define MM_RESAVAIL_FREE_DELETE_PROCESS                  7
#define MM_RESAVAIL_FREE_CLEAN_PROCESS2                  8
#define MM_RESAVAIL_ALLOCATE_CREATE_STACK                9

#define MM_RESAVAIL_FREE_DELETE_STACK                   10
#define MM_RESAVAIL_ALLOCATE_GROW_STACK                 11
#define MM_RESAVAIL_FREE_OUTPAGE_STACK                  12
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE_EXCESS       13
#define MM_RESAVAIL_ALLOCATE_LOAD_SYSTEM_IMAGE          14
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE1             15
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE2             16
#define MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE3             17
#define MM_RESAVAIL_FREE_DRIVER_INITIALIZATION          18
#define MM_RESAVAIL_FREE_SET_DRIVER_PAGING              19

#define MM_RESAVAIL_FREE_CONTIGUOUS2                    20
#define MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1           21
#define MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE            22
#define MM_RESAVAIL_FREE_EXPANSION_NONPAGED_POOL        23
#define MM_RESAVAIL_ALLOCATE_EXPANSION_NONPAGED_POOL    24
#define MM_RESAVAIL_ALLOCATE_LOCK_CODE1                 25
#define MM_RESAVAIL_ALLOCATE_LOCK_CODE3                 26
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST            27
#define MM_RESAVAIL_ALLOCATE_INDEPENDENT                28
#define MM_RESAVAIL_ALLOCATE_LOCK_CODE2                 29

#define MM_RESAVAIL_FREE_INDEPENDENT                    30
#define MM_RESAVAIL_ALLOCATE_NONPAGED_SPECIAL_POOL      31
#define MM_RESAVAIL_FREE_CONTIGUOUS                     32
#define MM_RESAVAIL_ALLOCATE_SPECIAL_POOL_EXPANSION     33
#define MM_RESAVAIL_ALLOCATE_FOR_MDL                    34
#define MM_RESAVAIL_FREE_FROM_MDL                       35
#define MM_RESAVAIL_FREE_AWE                            36
#define MM_RESAVAIL_FREE_NONPAGED_SPECIAL_POOL          37
#define MM_RESAVAIL_FREE_FOR_MDL_EXCESS                 38
#define MM_RESAVAIL_ALLOCATE_HOTADD_PFNDB               39

#define MM_RESAVAIL_ALLOCATE_CREATE_SESSION             40
#define MM_RESAVAIL_FREE_CLEAN_PROCESS1                 41
#define MM_RESAVAIL_ALLOCATE_SINGLE_PFN                 42
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST1           43
#define MM_RESAVAIL_ALLOCATE_SESSION_PAGE_TABLES        44
#define MM_RESAVAIL_ALLOCATE_SESSION_IMAGE              45
#define MM_RESAVAIL_ALLOCATE_BUILDMDL                   46
#define MM_RESAVAIL_FREE_BUILDMDL_EXCESS                47
#define MM_RESAVAIL_ALLOCATE_ADD_WS_PAGE                48
#define MM_RESAVAIL_FREE_CREATE_SESSION                 49

#define MM_RESAVAIL_ALLOCATE_INIT_SESSION_WS            50
#define MM_RESAVAIL_FREE_SESSION_PAGE_TABLE             51
#define MM_RESAVAIL_FREE_DEREFERENCE_SESSION            52
#define MM_RESAVAIL_FREE_DEREFERENCE_SESSION_PAGES      53
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST2           54
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST3           55
#define MM_RESAVAIL_FREE_DEREFERENCE_SESSION_WS         56
#define MM_RESAVAIL_FREE_LOAD_SESSION_IMAGE1            57
#define MM_RESAVAIL_ALLOCATE_USER_PAGE_TABLE            58
#define MM_RESAVAIL_FREE_USER_PAGE_TABLE                59

#define MM_RESAVAIL_FREE_HOTADD_MEMORY                  60
#define MM_RESAVAIL_ALLOCATE_HOTREMOVE_MEMORY           61
#define MM_RESAVAIL_FREE_HOTREMOVE_MEMORY1              62
#define MM_RESAVAIL_FREE_HOTREMOVE_FAILED               63
#define MM_RESAVAIL_FREE_HOTADD_ECC                     64
#define MM_RESAVAIL_ALLOCATE_COMPRESSION                65
#define MM_RESAVAIL_FREE_COMPRESSION                    66
#define MM_RESAVAIL_ALLOCATE_LARGE_PAGES                67
#define MM_RESAVAIL_FREE_LARGE_PAGES                    68
#define MM_RESAVAIL_ALLOCATE_LOAD_SYSTEM_IMAGE_TEMP     69

#define MM_RESAVAIL_ALLOCATE_WSLE_HASH                  70
#define MM_RESAVAIL_FREE_WSLE_HASH                      71
#define MM_RESAVAIL_FREE_CLEAN_PROCESS_WS               72
#define MM_RESAVAIL_FREE_SESSION_PAGE_TABLES_EXCESS     73

#define MM_RESAVAIL_ALLOCATE_ROTATE_VAD                 74
#define MM_RESAVAIL_FREE_ROTATE_VAD                     75
#define MM_RESAVAIL_ALLOCATE_LARGE_PAGES_PF_BACKED      76
#define MM_RESAVAIL_FREE_LARGE_PAGES_PF_BACKED          77
#define MM_RESAVAIL_FREE_PAGETABLES_FOR_PAGED_POOL      78
#define MM_RESAVAIL_FREE_ALLOCATE_ADD_WS_PAGE           79
#define MM_RESAVAIL_FREE_ALLOCATE_ADD_WSLE_HASH         80
#define MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST4           81
#define MM_RESAVAIL_ALLOCATE_FOR_PAGEFILE_ZEROING       82
#define MM_RESAVAIL_FREE_FOR_PAGEFILE_ZEROING           83

#define MM_BUMP_COUNTER_MAX 84

extern SIZE_T MmResTrack[MM_BUMP_COUNTER_MAX];

#define MI_INCREMENT_RESIDENT_AVAILABLE(bump, _index)                        \
    InterlockedExchangeAddSizeT (&MmResidentAvailablePages, (SIZE_T)(bump)); \
    ASSERT (_index < MM_BUMP_COUNTER_MAX);                                   \
    InterlockedExchangeAddSizeT (&MmResTrack[_index], (SIZE_T)(bump));

#define MI_DECREMENT_RESIDENT_AVAILABLE(bump, _index)                          \
    InterlockedExchangeAddSizeT (&MmResidentAvailablePages, 0-(SIZE_T)(bump)); \
    ASSERT (_index < MM_BUMP_COUNTER_MAX);                                     \
    InterlockedExchangeAddSizeT (&MmResTrack[_index], (SIZE_T)(bump));

extern ULONG MmLargePageMinimum;

extern const MMPTE ZeroPte;

extern const MMPTE ZeroKernelPte;

extern const MMPTE ValidKernelPteLocal;

extern MMPTE ValidKernelPte;

extern MMPTE ValidKernelPde;

extern const MMPTE ValidKernelPdeLocal;

#if defined (_AMD64_)
extern MMPTE ValidUserPte;
#else
extern const MMPTE ValidUserPte;
#endif

extern const MMPTE ValidPtePte;

extern const MMPTE ValidPdePde;

extern MMPTE DemandZeroPde;

extern const MMPTE DemandZeroPte;

extern MMPTE KernelPrototypePte;

extern const MMPTE TransitionPde;

extern MMPTE PrototypePte;

extern const MMPTE NoAccessPte;

extern ULONG_PTR MmSubsectionBase;

extern ULONG_PTR MmSubsectionTopPage;

extern ULONG ExpMultiUserTS;

//
// Virtual alignment for PTEs (machine specific) minimum value is
// 4k maximum value is 64k.  The maximum value can be raised by
// changing the MM_PROTO_PTE_ALIGNMENT constant and adding more
// reserved mapping PTEs in hyperspace.
//

//
// Total number of physical pages on the system.
//

extern PFN_COUNT MmNumberOfPhysicalPages;

//
// Highest possible physical page number in the system.
//

extern PFN_NUMBER MmHighestPossiblePhysicalPage;

#if defined (_WIN64)

#define MI_DTC_MAX_PAGES ((PFN_NUMBER)(((ULONG64)1024 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DTC_BOOTED_3GB_MAX_PAGES     MI_DTC_MAX_PAGES

#define MI_ADS_MAX_PAGES ((PFN_NUMBER)(((ULONG64)1024 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_SRV_MAX_PAGES ((PFN_NUMBER)(((ULONG64)32 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_PRO_MAX_PAGES ((PFN_NUMBER)(((ULONG64)128 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DEFAULT_MAX_PAGES ((PFN_NUMBER)(((ULONG64)32 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#else

//
//                        WARNING WARNING WARNING
//
//              The PFN u4.PteFrame must be widened before
//              the x86 maximum page limit can be raised.
//

#define MI_DTC_MAX_PAGES ((PFN_NUMBER)(((ULONG64)128 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DTC_BOOTED_3GB_MAX_PAGES ((PFN_NUMBER)(((ULONG64)16 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_ADS_MAX_PAGES ((PFN_NUMBER)(((ULONG64)64 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_SRV_MAX_PAGES ((PFN_NUMBER)(((ULONG64)4 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#define MI_DEFAULT_MAX_PAGES ((PFN_NUMBER)(((ULONG64)4 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

#endif

#define MI_BLADE_MAX_PAGES ((PFN_NUMBER)(((ULONG64)2 * 1024 * 1024 * 1024) >> PAGE_SHIFT))

extern RTL_BITMAP MiPfnBitMap;


FORCEINLINE
LOGICAL
MI_IS_PFN (
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    Check if a given address is backed by RAM or IO space.

Arguments:

    PageFrameIndex - Supplies a page frame number to check.

Return Value:

    TRUE - If the address is backed by RAM.

    FALSE - If the address is IO mapped memory.

Environment:

    Kernel mode.  PFN lock or dynamic memory mutex may be held.

--*/

{
    if (PageFrameIndex > MmHighestPossiblePhysicalPage) {
        return FALSE;
    }

    return MI_CHECK_BIT (MiPfnBitMap.Buffer, PageFrameIndex);
}

extern MMPFNLIST MmZeroedPageListHead;

extern MMPFNLIST MmFreePageListHead;

extern MMPFNLIST MmStandbyPageListHead;

extern MMPFNLIST MmStandbyPageListByPriority[MI_PFN_PRIORITIES];

extern MMPFNLIST MmRomPageListHead;

extern MMPFNLIST MmModifiedPageListHead;

extern MMPFNLIST MmModifiedNoWritePageListHead;

extern MMPFNLIST MmBadPageListHead;

extern PMMPFNLIST MmPageLocationList[NUMBER_OF_PAGE_LISTS];

extern MMPFNLIST MmModifiedPageListByColor[MM_MAXIMUM_NUMBER_OF_COLORS];

//
// Mask for isolating node color from combined node and secondary
// color.
//

extern ULONG MmSecondaryColorNodeMask;

//
// Width of MmSecondaryColorMask in bits.   In multi node systems,
// the node number is combined with the secondary color to make up
// the page color.
//

extern UCHAR MmSecondaryColorNodeShift;

//
// Events for available pages, set means pages are available.
//

extern KEVENT MmAvailablePagesEvent;

extern KEVENT MmAvailablePagesEventHigh;

//
// Event for the zeroing page thread.
//

extern KEVENT MmZeroingPageEvent;

//
// Boolean to indicate if the zeroing page thread is currently
// active.  This is set to true when the zeroing page event is
// set and set to false when the zeroing page thread is done
// zeroing all the pages on the free list.
//

extern BOOLEAN MmZeroingPageThreadActive;

//
// Minimum number of free pages before zeroing page thread starts.
//

extern PFN_NUMBER MmMinimumFreePagesToZero;

//
// Global event to synchronize mapped writing with cleaning segments.
//

extern KEVENT MmMappedFileIoComplete;

//
// Hyper space items.
//

extern PMMPTE MmFirstReservedMappingPte;

extern PMMPTE MmLastReservedMappingPte;

//
// System space sizes - MmNonPagedSystemStart to MM_NON_PAGED_SYSTEM_END
// defines the ranges of PDEs which must be copied into a new process's
// address space.
//

extern PVOID MmNonPagedSystemStart;

extern PCHAR MmSystemSpaceViewStart;

extern LOGICAL MmProtectFreedNonPagedPool;

//
// Pool sizes.
//

extern SIZE_T MmSizeOfNonPagedPoolInBytes;

extern SIZE_T MmMinimumNonPagedPoolSize;

extern SIZE_T MmDefaultMaximumNonPagedPool;

extern ULONG MmMaximumNonPagedPoolPercent;

extern ULONG MmMinAdditionNonPagedPoolPerMb;

extern ULONG MmMaxAdditionNonPagedPoolPerMb;

extern SIZE_T MmSizeOfPagedPoolInBytes;
extern PFN_NUMBER MmSizeOfPagedPoolInPages;

extern SIZE_T MmMaximumNonPagedPoolInBytes;

extern PFN_NUMBER MmMaximumNonPagedPoolInPages;

extern PFN_NUMBER MmAllocatedNonPagedPool;

extern PVOID MmNonPagedPoolExpansionStart;

extern PFN_NUMBER MmNumberOfFreeNonPagedPool;

extern PFN_NUMBER MmNumberOfSystemPtes;

extern ULONG MiRequestedSystemPtes;

extern PMMPTE MmSystemPagePtes;

extern ULONG MmSystemPageDirectory[];

extern SIZE_T MmHeapSegmentReserve;

extern SIZE_T MmHeapSegmentCommit;

extern SIZE_T MmHeapDeCommitTotalFreeThreshold;

extern SIZE_T MmHeapDeCommitFreeBlockThreshold;

#define MI_MAX_FREE_LIST_HEADS  4

extern LIST_ENTRY MmNonPagedPoolFreeListHead[MI_MAX_FREE_LIST_HEADS];

//
// Counter for flushes of the entire TB.
//

extern ULONG MmFlushCounter;

//
// Pool start and end.
//

extern PVOID MmNonPagedPoolStart;

extern PVOID MmNonPagedPoolEnd;

extern PVOID MmPagedPoolStart;

extern PVOID MmPagedPoolEnd;

LOGICAL
FORCEINLINE
MiIsProtoAddressValid (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    For a given paged pool address (containing prototype PTEs) this function
    returns TRUE if no page fault will occur for a write operation on the
    address, FALSE otherwise.

    Various optimizations may be made since this is only ever called with
    these types of addresses...

    Since paged pool is always mapped readwrite, just checking for validity
    is sufficient.  Note also that the reference to the PTE may fault on x86
    because the paged pool page directory entries are lazy filled, but the
    fault handler will take care of this for us.

Arguments:

    VirtualAddress - Supplies the virtual address to check.

Return Value:

    TRUE if no page fault would be generated writing the virtual address,
    FALSE otherwise.

Environment:

    Kernel mode, PFN lock held.

--*/

{
    PMMPTE PointerPte;

    ASSERT (((VirtualAddress >= MmPagedPoolStart) &&
            (VirtualAddress <= MmPagedPoolEnd)) ||

            ((VirtualAddress >= MmSpecialPoolStart) &&
            (VirtualAddress <= MmSpecialPoolEnd)));

#if defined (_AMD64_)
    ASSERT (MI_IS_PHYSICAL_ADDRESS (VirtualAddress) == FALSE);
#endif

    MM_PFN_LOCK_ASSERT();

    //
    // If the address is not canonical then return FALSE as the caller (which
    // may be the kernel debugger) is not expecting to get an unimplemented
    // address bit fault.
    //

    ASSERT (MI_RESERVED_BITS_CANONICAL(VirtualAddress) == TRUE);

#if (_MI_PAGING_LEVELS >= 4)
    ASSERT (MiGetPxeAddress (VirtualAddress)->u.Hard.Valid == 1);
#endif

#if (_MI_PAGING_LEVELS >= 3)
    ASSERT (MiGetPpeAddress (VirtualAddress)->u.Hard.Valid == 1);
    ASSERT (MiGetPdeAddress (VirtualAddress)->u.Hard.Valid == 1);
#endif

#if DBG
    PointerPte = MiGetPdeAddress (VirtualAddress);
    if (PointerPte->u.Hard.Valid == 1) {
        ASSERT (MI_PDE_MAPS_LARGE_PAGE (PointerPte) == FALSE);
    }
#endif

    PointerPte = MiGetPteAddress (VirtualAddress);

    //
    // This reference may (safely) fault on x86.
    //

    if (PointerPte->u.Hard.Valid == 0) {
        return FALSE;
    }

    ASSERT (MI_PDE_MAPS_LARGE_PAGE (PointerPte) == FALSE);

    return TRUE;
}
//
// Pool bit maps and other related structures.
//

typedef struct _MM_PAGED_POOL_INFO {

    PRTL_BITMAP PagedPoolAllocationMap;
    PRTL_BITMAP EndOfPagedPoolBitmap;
    PMMPTE FirstPteForPagedPool;
    PMMPTE LastPteForPagedPool;
    PMMPTE NextPdeForPagedPoolExpansion;
    ULONG PagedPoolHint;
    SIZE_T PagedPoolCommit;
    SIZE_T AllocatedPagedPool;

} MM_PAGED_POOL_INFO, *PMM_PAGED_POOL_INFO;

extern MM_PAGED_POOL_INFO MmPagedPoolInfo;

extern PRTL_BITMAP VerifierLargePagedPoolMap;

//
// MmFirstFreeSystemPte contains the offset from the
// Nonpaged system base to the first free system PTE.
// Note, that an offset of zero indicates an empty list.
//

extern MMPTE MmFirstFreeSystemPte[MaximumPtePoolTypes];

extern ULONG_PTR MiSystemViewStart;

//
// System cache sizes.
//

extern PMMWSL MmSystemCacheWorkingSetList;

extern PMMWSLE MmSystemCacheWsle;

extern PVOID MmSystemCacheStart;

extern PVOID MmSystemCacheEnd;

extern PFN_NUMBER MmSystemCacheWsMinimum;

//
// Virtual alignment for PTEs (machine specific) minimum value is
// 0 (no alignment) maximum value is 64k.  The maximum value can be raised by
// changing the MM_PROTO_PTE_ALIGNMENT constant and adding more
// reserved mapping PTEs in hyperspace.
//

extern ULONG MmAliasAlignment;

//
// Mask to AND with virtual address to get an offset to go
// with the alignment.  This value is page aligned.
//

extern ULONG MmAliasAlignmentOffset;

//
// Mask to and with PTEs to determine if the alias mapping is compatible.
// This value is usually (MmAliasAlignment - 1)
//

extern ULONG MmAliasAlignmentMask;

//
// Cells to track unused thread kernel stacks to avoid TB flushes
// every time a thread terminates.
//

extern ULONG MmMaximumDeadKernelStacks;
extern SLIST_HEADER MmDeadStackSListHead;

//
// MmSystemPteBase contains the address of 1 PTE before
// the first free system PTE (zero indicates an empty list).
// The value of this field does not change once set.
//

extern PMMPTE MmSystemPteBase;

//
// Root of system space virtual address descriptors.  These define
// the pageable portion of the system.
//

extern PMMVAD MmVirtualAddressDescriptorRoot;

extern MM_AVL_TABLE MmSectionBasedRoot;

extern PVOID MmHighSectionBase;

//
// Section commit mutex.
//

extern KGUARDED_MUTEX MmSectionCommitMutex;

//
// Section base address mutex.
//

extern KGUARDED_MUTEX MmSectionBasedMutex;

//
// Resource for section extension.
//

extern ERESOURCE MmSectionExtendResource;
extern ERESOURCE MmSectionExtendSetResource;

//
// Inpage cluster sizes for executable pages (set based on memory size).
//

extern ULONG MmDataClusterSize;

extern ULONG MmCodeClusterSize;

//
// Pagefile creation mutex.
//

extern KGUARDED_MUTEX MmPageFileCreationLock;

//
// Event to set when first paging file is created.
//

extern PKEVENT MmPagingFileCreated;

//
// Paging file debug information.
//

extern ULONG_PTR MmPagingFileDebug[];

//
// Fast mutex which guards the working set list for the system shared
// address space (paged pool, system cache, pageable drivers).
//

extern FAST_MUTEX MmSystemWsLock;

//
// Spin lock for allowing working set expansion.
//

extern KSPIN_LOCK MmExpansionLock;

//
// To prevent optimizations.
//

extern MMPTE GlobalPte;

//
// Page color for system working set.
//

extern ULONG MmSystemPageColor;

extern ULONG MmSecondaryColors;

extern ULONG MmProcessColorSeed;

//
// Set from ntos\config\CMDAT3.C  Used by customers to disable paging
// of executive on machines with lots of memory.  Worth a few TPS on a
// data base server.
//

#define MM_SYSTEM_CODE_LOCKED_DOWN 0x1
#define MM_PAGED_POOL_LOCKED_DOWN  0x2

extern ULONG MmDisablePagingExecutive;


//
// For debugging.


#if DBG
extern ULONG MmDebug;
#endif

//
// Unused segment management
//

extern MMDEREFERENCE_SEGMENT_HEADER MmDereferenceSegmentHeader;

extern LIST_ENTRY MmUnusedSegmentList;

extern LIST_ENTRY MmUnusedSubsectionList;

extern KEVENT MmUnusedSegmentCleanup;

extern ULONG MmConsumedPoolPercentage;

extern ULONG MmUnusedSegmentCount;

extern ULONG MmUnusedSubsectionCount;

extern ULONG MmUnusedSubsectionCountPeak;

extern SIZE_T MiUnusedSubsectionPagedPool;

extern SIZE_T MiUnusedSubsectionPagedPoolPeak;

#define MI_UNUSED_SUBSECTIONS_COUNT_INSERT(_MappedSubsection) \
        MmUnusedSubsectionCount += 1; \
        if (MmUnusedSubsectionCount > MmUnusedSubsectionCountPeak) { \
            MmUnusedSubsectionCountPeak = MmUnusedSubsectionCount; \
        } \
        MiUnusedSubsectionPagedPool += EX_REAL_POOL_USAGE((_MappedSubsection->PtesInSubsection + _MappedSubsection->UnusedPtes) * sizeof (MMPTE)); \
        if (MiUnusedSubsectionPagedPool > MiUnusedSubsectionPagedPoolPeak) { \
            MiUnusedSubsectionPagedPoolPeak = MiUnusedSubsectionPagedPool; \
        } \

#define MI_UNUSED_SUBSECTIONS_COUNT_REMOVE(_MappedSubsection) \
        MmUnusedSubsectionCount -= 1; \
        MiUnusedSubsectionPagedPool -= EX_REAL_POOL_USAGE((_MappedSubsection->PtesInSubsection + _MappedSubsection->UnusedPtes) * sizeof (MMPTE));

#define MI_FILESYSTEM_NONPAGED_POOL_CHARGE 150

#define MI_FILESYSTEM_PAGED_POOL_CHARGE 1024

//++
// LOGICAL
// MI_UNUSED_SEGMENTS_SURPLUS (
//    IN PVOID va
//    );
//
// Routine Description:
//
//    This routine determines whether a surplus of unused
//    segments exist.  If so, the caller can initiate a trim to free pool.
//
// Arguments
//
//    None.
//
// Return Value:
//
//    TRUE if unused segment trimming should be initiated, FALSE if not.
//
//--
#define MI_UNUSED_SEGMENTS_SURPLUS()                                    \
        (((ULONG)((MmPagedPoolInfo.AllocatedPagedPool * 100) / (MmSizeOfPagedPoolInBytes >> PAGE_SHIFT)) > MmConsumedPoolPercentage) || \
        ((ULONG)((MmAllocatedNonPagedPool * 100) / MmMaximumNonPagedPoolInPages) > MmConsumedPoolPercentage))

VOID
MiConvertStaticSubsections (
    IN PCONTROL_AREA ControlArea
    );

//++
// VOID
// MI_INSERT_UNUSED_SEGMENT (
//    IN PCONTROL_AREA _ControlArea
//    );
//
// Routine Description:
//
//    This routine inserts a control area into the unused segment list,
//    also managing the associated pool charges.
//
// Arguments
//
//    _ControlArea - Supplies the control area to obtain the pool charges from.
//
// Return Value:
//
//    None.
//
//--
#define MI_INSERT_UNUSED_SEGMENT(_ControlArea)                               \
        {                                                                    \
           MM_PFN_LOCK_ASSERT();                                             \
           if ((_ControlArea->u.Flags.Image == 0) &&                         \
               (_ControlArea->FilePointer != NULL) &&                        \
               (_ControlArea->u.Flags.PhysicalMemory == 0)) {                \
               MiConvertStaticSubsections(_ControlArea);                     \
           }                                                                 \
           InsertTailList (&MmUnusedSegmentList, &_ControlArea->DereferenceList); \
           MmUnusedSegmentCount += 1; \
        }

//++
// VOID
// MI_UNUSED_SEGMENTS_REMOVE_CHARGE (
//    IN PCONTROL_AREA _ControlArea
//    );
//
// Routine Description:
//
//    This routine manages pool charges during removals of segments from
//    the unused segment list.
//
// Arguments
//
//    _ControlArea - Supplies the control area to obtain the pool charges from.
//
// Return Value:
//
//    None.
//
//--
#define MI_UNUSED_SEGMENTS_REMOVE_CHARGE(_ControlArea)                       \
        {                                                                    \
           MM_PFN_LOCK_ASSERT();                                             \
           MmUnusedSegmentCount -= 1; \
        }

//
// List heads
//

extern MMWORKING_SET_EXPANSION_HEAD MmWorkingSetExpansionHead;

extern MMPAGE_FILE_EXPANSION MmAttemptForCantExtend;

//
// Paging files
//

extern MMMOD_WRITER_LISTHEAD MmPagingFileHeader;

extern MMMOD_WRITER_LISTHEAD MmMappedFileHeader;

extern PMMPAGING_FILE MmPagingFile[MAX_PAGE_FILES];

extern LIST_ENTRY MmFreePagingSpaceLow;

extern ULONG MmNumberOfActiveMdlEntries;

extern ULONG MmNumberOfPagingFiles;

extern KEVENT MmModifiedPageWriterEvent;

extern KEVENT MmCollidedFlushEvent;

extern KEVENT MmCollidedLockEvent;

#define MI_SNAP_DATA(_Pfn, _Pte, _CallerId)

//
// Modified page writer.
//


VOID
FORCEINLINE
MiReleaseConfirmedPageFileSpace (
    IN MMPTE PteContents
    )

/*++

Routine Description:

    This routine frees the paging file allocated to the specified PTE.

Arguments:

    PteContents - Supplies the PTE which is in page file format.

Return Value:

    Returns TRUE if any paging file space was deallocated.

Environment:

    Kernel mode, APCs disabled, PFN lock held.

--*/

{
    ULONG FreeBit;
    ULONG PageFileNumber;
    PMMPAGING_FILE PageFile;

    MM_PFN_LOCK_ASSERT();

    ASSERT (PteContents.u.Soft.Prototype == 0);

    FreeBit = GET_PAGING_FILE_OFFSET (PteContents);

    ASSERT ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED));

    PageFileNumber = GET_PAGING_FILE_NUMBER (PteContents);

    PageFile = MmPagingFile[PageFileNumber];

    ASSERT (RtlCheckBit( PageFile->Bitmap, FreeBit) == 1);

#if DBG
    if ((FreeBit < 8192) && (PageFileNumber == 0)) {
        ASSERT ((MmPagingFileDebug[FreeBit] & 1) != 0);
        MmPagingFileDebug[FreeBit] ^= 1;
    }
#endif

    MI_CLEAR_BIT (PageFile->Bitmap->Buffer, FreeBit);

    PageFile->FreeSpace += 1;
    PageFile->CurrentUsage -= 1;

    //
    // Check to see if we should move some MDL entries for the
    // modified page writer now that more free space is available.
    //

    if ((MmNumberOfActiveMdlEntries == 0) ||
        (PageFile->FreeSpace == MM_USABLE_PAGES_FREE)) {

        MiUpdateModifiedWriterMdls (PageFileNumber);
    }
}

extern PFN_NUMBER MmMinimumFreePages;

extern PFN_NUMBER MmFreeGoal;

extern PFN_NUMBER MmModifiedPageMaximum;

extern ULONG MmModifiedWriteClusterSize;

extern ULONG MmMinimumFreeDiskSpace;

extern ULONG MmPageFileExtension;

extern ULONG MmMinimumPageFileReduction;

extern LARGE_INTEGER MiModifiedPageLife;

extern BOOLEAN MiTimerPending;

extern KEVENT MiMappedPagesTooOldEvent;

extern KDPC MiModifiedPageWriterTimerDpc;

extern KTIMER MiModifiedPageWriterTimer;

//
// System process working set sizes.
//

extern PFN_NUMBER MmSystemProcessWorkingSetMin;

extern PFN_NUMBER MmSystemProcessWorkingSetMax;

extern PFN_NUMBER MmMinimumWorkingSetSize;

//
// Support for debugger's mapping physical memory.
//

extern PMMPTE MmDebugPte;

extern PMMPTE MmCrashDumpPte;

extern ULONG MiOverCommitCallCount;

//
// Event tracing routines
//

extern PPAGE_FAULT_NOTIFY_ROUTINE MmPageFaultNotifyRoutine;

extern SIZE_T MmSystemViewSize;

VOID
FASTCALL
MiIdentifyPfn (
    IN PMMPFN Pfn1,
    OUT PMMPFN_IDENTITY PfnIdentity
    );

//
// This is a special value loaded into an EPROCESS pointer to indicate that
// the action underway is for a Hydra session, not really the current process.
// Any value could have been used here that is not a valid system pointer
// or NULL - 1 was chosen because it simplifies checks for both NULL &
// HydraProcess by comparing for greater than HydraProcess.
//

#define HYDRA_PROCESS   ((PEPROCESS)1)

#define PREFETCH_PROCESS   ((PEPROCESS)2)

#define MI_SESSION_SPACE_STRUCT_SIZE MM_ALLOCATION_GRANULARITY

#if defined (_WIN64)

/*++

  Virtual memory layout of session space when loaded down from
  0x2000.0002.0000.0000 (IA64) or FFFF.F980.0000.0000 (AMD64) :

  Note that the sizes of mapped views, paged pool & images are registry tunable.

                        +------------------------------------+
    2000.0002.0000.0000 |                                    |
                        |   win32k.sys & video drivers       |
                        |             (16MB)                 |
                        |                                    |
                        +------------------------------------+
    2000.0001.FF00.0000 |                                    |
                        |   MM_SESSION_SPACE & Session WSLs  |
                        |              (16MB)                |
                        |                                    |
    2000.0001.FEFF.0000 +------------------------------------+
                        |                                    |
                        |              ...                   |
                        |                                    |
                        +------------------------------------+
    2000.0001.FE80.0000 |                                    |
                        |   Mapped Views for this session    |
                        |              (104MB)               |
                        |                                    |
                        +------------------------------------+
    2000.0001.F800.0000 |                                    |
                        |   Paged Pool for this session      |
                        |              (64MB)                |
                        |                                    |
    2000.0001.F400.0000 +------------------------------------+
                        |   Special Pool for this session    |
                        |              (64MB)                |
                        |                                    |
    2000.0000.0000.0000 +------------------------------------+

--*/

#define MI_SESSION_SPACE_WS_SIZE  ((ULONG_PTR)(16*1024*1024) - MI_SESSION_SPACE_STRUCT_SIZE)

#define MI_SESSION_DEFAULT_IMAGE_SIZE     ((ULONG_PTR)(16*1024*1024))

#define MI_SESSION_DEFAULT_VIEW_SIZE      ((ULONG_PTR)(104*1024*1024))

#define MI_SESSION_DEFAULT_POOL_SIZE      ((ULONG_PTR)(64*1024*1024))

#define MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE (MM_VA_MAPPED_BY_PPE)

#else

/*++

  Virtual memory layout of session space when loaded down from 0xC0000000.

  Note that the sizes of mapped views, paged pool and images are registry
  tunable on 32-bit systems (if NOT booted /3GB, as 3GB has very limited
  address space).

                 +------------------------------------+
        C0000000 |                                    |
                 | win32k.sys, video drivers and any  |
                 | rebased NT4 printer drivers.       |
                 |                                    |
                 |             (8MB)                  |
                 |                                    |
                 +------------------------------------+
        BF800000 |                                    |
                 |   MM_SESSION_SPACE & Session WSLs  |
                 |              (4MB)                 |
                 |                                    |
                 +------------------------------------+
        BF400000 |                                    |
                 |   Mapped views for this session    |
                 |     (20MB by default, but is       |
                 |      registry configurable)        |
                 |                                    |
                 +------------------------------------+
        BE000000 |                                    |
                 |   Paged pool for this session      |
                 |     (16MB by default, but is       |
                 |      registry configurable)        |
                 |                                    |
        BD000000 +------------------------------------+

--*/

#define MI_SESSION_SPACE_WS_SIZE  (4*1024*1024 - MI_SESSION_SPACE_STRUCT_SIZE)

#define MI_SESSION_DEFAULT_IMAGE_SIZE      (8*1024*1024)

#define MI_SESSION_DEFAULT_VIEW_SIZE      (20*1024*1024)

#define MI_SESSION_DEFAULT_POOL_SIZE      (16*1024*1024)

#define MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE \
            (MM_SYSTEM_CACHE_END_EXTRA - MM_KSEG2_BASE)

#endif



#define MI_SESSION_SPACE_DEFAULT_TOTAL_SIZE \
            (MI_SESSION_DEFAULT_IMAGE_SIZE + \
             MI_SESSION_SPACE_STRUCT_SIZE + \
             MI_SESSION_SPACE_WS_SIZE + \
             MI_SESSION_DEFAULT_VIEW_SIZE + \
             MI_SESSION_DEFAULT_POOL_SIZE)

extern ULONG_PTR MmSessionBase;
extern PMMPTE MiSessionBasePte;
extern PMMPTE MiSessionLastPte;

extern ULONG_PTR MiSessionSpaceWs;

extern ULONG_PTR MiSessionViewStart;
extern SIZE_T MmSessionViewSize;

extern ULONG_PTR MiSessionImageStart;
extern ULONG_PTR MiSessionImageEnd;
extern SIZE_T MmSessionImageSize;

extern PMMPTE MiSessionImagePteStart;
extern PMMPTE MiSessionImagePteEnd;

extern ULONG_PTR MiSessionPoolStart;
extern ULONG_PTR MiSessionPoolEnd;
extern SIZE_T MmSessionPoolSize;

extern ULONG_PTR MiSessionSpaceEnd;

extern ULONG MiSessionSpacePageTables;

//
// The number of page table pages required to map all of session space.
//

#define MI_SESSION_SPACE_MAXIMUM_PAGE_TABLES \
            (MI_SESSION_SPACE_MAXIMUM_TOTAL_SIZE / MM_VA_MAPPED_BY_PDE)

extern SIZE_T MmSessionSize;        // size of the entire session space.

//
// Macros to determine if a given address lies in the specified session range.
//

#define MI_IS_SESSION_IMAGE_ADDRESS(VirtualAddress) \
        ((PVOID)(VirtualAddress) >= (PVOID)MiSessionImageStart && (PVOID)(VirtualAddress) < (PVOID)(MiSessionImageEnd))

#define MI_IS_SESSION_POOL_ADDRESS(VirtualAddress) \
        ((PVOID)(VirtualAddress) >= (PVOID)MiSessionPoolStart && (PVOID)(VirtualAddress) < (PVOID)MiSessionPoolEnd)

#define MI_IS_SESSION_ADDRESS(_VirtualAddress) \
        ((PVOID)(_VirtualAddress) >= (PVOID)MmSessionBase && (PVOID)(_VirtualAddress) < (PVOID)(MiSessionSpaceEnd))

#define MI_IS_SESSION_PTE(_Pte) \
        ((PMMPTE)(_Pte) >= MiSessionBasePte && (PMMPTE)(_Pte) < MiSessionLastPte)

#define MI_IS_SESSION_IMAGE_PTE(_Pte) \
        ((PMMPTE)(_Pte) >= MiSessionImagePteStart && (PMMPTE)(_Pte) < MiSessionImagePteEnd)

#define SESSION_GLOBAL(_Session)    (_Session->GlobalVirtualAddress)

#define MM_DBG_SESSION_INITIAL_PAGETABLE_ALLOC          0
#define MM_DBG_SESSION_INITIAL_PAGETABLE_FREE_RACE      1
#define MM_DBG_SESSION_INITIAL_PAGE_ALLOC               2
#define MM_DBG_SESSION_INITIAL_PAGE_FREE_FAIL1          3
#define MM_DBG_SESSION_INITIAL_PAGETABLE_FREE_FAIL1     4
#define MM_DBG_SESSION_WS_PAGE_FREE                     5
#define MM_DBG_SESSION_PAGETABLE_ALLOC                  6
#define MM_DBG_SESSION_SYSMAPPED_PAGES_ALLOC            7
#define MM_DBG_SESSION_WS_PAGETABLE_ALLOC               8
#define MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC        9
#define MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_FREE_FAIL1   10
#define MM_DBG_SESSION_WS_PAGE_ALLOC                    11
#define MM_DBG_SESSION_WS_PAGE_ALLOC_GROWTH             12
#define MM_DBG_SESSION_INITIAL_PAGE_FREE                13
#define MM_DBG_SESSION_PAGETABLE_FREE                   14
#define MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC1       15
#define MM_DBG_SESSION_DRIVER_PAGES_LOCKED              16
#define MM_DBG_SESSION_DRIVER_PAGES_UNLOCKED            17
#define MM_DBG_SESSION_WS_HASHPAGE_ALLOC                18
#define MM_DBG_SESSION_SYSMAPPED_PAGES_COMMITTED        19

#define MM_DBG_SESSION_COMMIT_PAGEDPOOL_PAGES           30
#define MM_DBG_SESSION_COMMIT_DELETE_VM_RETURN          31
#define MM_DBG_SESSION_COMMIT_POOL_FREED                32
#define MM_DBG_SESSION_COMMIT_IMAGE_UNLOAD              33
#define MM_DBG_SESSION_COMMIT_IMAGELOAD_FAILED1         34
#define MM_DBG_SESSION_COMMIT_IMAGELOAD_FAILED2         35
#define MM_DBG_SESSION_COMMIT_IMAGELOAD_NOACCESS        36

#define MM_DBG_SESSION_NP_LOCK_CODE1                    38
#define MM_DBG_SESSION_NP_LOCK_CODE2                    39
#define MM_DBG_SESSION_NP_SESSION_CREATE                40
#define MM_DBG_SESSION_NP_PAGETABLE_ALLOC               41
#define MM_DBG_SESSION_NP_POOL_CREATE                   42
#define MM_DBG_SESSION_NP_COMMIT_IMAGE                  43
#define MM_DBG_SESSION_NP_COMMIT_IMAGE_PT               44
#define MM_DBG_SESSION_NP_INIT_WS                       45
#define MM_DBG_SESSION_NP_WS_GROW                       46
#define MM_DBG_SESSION_NP_HASH_GROW                     47

#define MM_DBG_SESSION_NP_PAGE_DRIVER                   48
#define MM_DBG_SESSION_NP_POOL_CREATE_FAILED            49
#define MM_DBG_SESSION_NP_WS_PAGE_FREE                  50
#define MM_DBG_SESSION_NP_SESSION_DESTROY               51
#define MM_DBG_SESSION_NP_SESSION_PTDESTROY             52
#define MM_DBG_SESSION_NP_DELVA                         53
#define MM_DBG_SESSION_NP_HASH_SHRINK                   54
#define MM_DBG_SESSION_WS_HASHPAGE_FREE                 55

#if DBG
#define MM_SESS_COUNTER_MAX 56

#define MM_BUMP_SESS_COUNTER(_index, bump) \
    if (_index >= MM_SESS_COUNTER_MAX) { \
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL,       \
            "Mm: Invalid bump counter %d %d\n", _index, MM_SESS_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    MmSessionSpace->Debug[_index] += (bump);

typedef struct _MM_SESSION_MEMORY_COUNTERS {
    SIZE_T NonPageablePages;
    SIZE_T CommittedPages;
} MM_SESSION_MEMORY_COUNTERS, *PMM_SESSION_MEMORY_COUNTERS;

#define MM_SESS_MEMORY_COUNTER_MAX  8

#define MM_SNAP_SESS_MEMORY_COUNTERS(_index) \
    if (_index >= MM_SESS_MEMORY_COUNTER_MAX) { \
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL,           \
            "Mm: Invalid session mem counter %d %d\n", _index, MM_SESS_MEMORY_COUNTER_MAX); \
        DbgBreakPoint(); \
    } \
    else { \
        MmSessionSpace->Debug2[_index].NonPageablePages = MmSessionSpace->NonPageablePages; \
        MmSessionSpace->Debug2[_index].CommittedPages = MmSessionSpace->CommittedPages; \
    }

#else
#define MM_BUMP_SESS_COUNTER(_index, bump)
#define MM_SNAP_SESS_MEMORY_COUNTERS(_index)
#endif


#define MM_SESSION_FAILURE_NO_IDS                   0
#define MM_SESSION_FAILURE_NO_COMMIT                1
#define MM_SESSION_FAILURE_NO_RESIDENT              2
#define MM_SESSION_FAILURE_RACE_DETECTED            3
#define MM_SESSION_FAILURE_NO_SYSPTES               4
#define MM_SESSION_FAILURE_NO_PAGED_POOL            5
#define MM_SESSION_FAILURE_NO_NONPAGED_POOL         6
#define MM_SESSION_FAILURE_NO_IMAGE_VA_SPACE        7
#define MM_SESSION_FAILURE_NO_SESSION_PAGED_POOL    8
#define MM_SESSION_FAILURE_NO_AVAILABLE             9
#define MM_SESSION_FAILURE_IMAGE_ZOMBIE             10

#define MM_SESSION_FAILURE_CAUSES                   11

ULONG MmSessionFailureCauses[MM_SESSION_FAILURE_CAUSES];

#define MM_BUMP_SESSION_FAILURES(_index) MmSessionFailureCauses[_index] += 1;

typedef struct _MM_SESSION_SPACE_FLAGS {
    ULONG Initialized : 1;
    ULONG DeletePending : 1;
    ULONG Filler : 30;
} MM_SESSION_SPACE_FLAGS;

//
// The value of SESSION_POOL_SMALL_LISTS is very carefully chosen for each
// architecture to avoid spilling over into an additional session data page.
//

#if defined(_AMD64_)
#define SESSION_POOL_SMALL_LISTS        21
#elif defined(_X86_)
#define SESSION_POOL_SMALL_LISTS        26
#else
#error "no target architecture"
#endif

//
// The session space data structure - allocated per session and only visible at
// MM_SESSION_SPACE_BASE when in the context of a process from the session.
// This virtual address space is rotated at context switch time when switching
// from a process in session A to a process in session B.  This rotation is
// useful for things like providing paged pool per session so many sessions
// won't exhaust the VA space which backs the system global pool.
//
// A kernel PTE is also allocated to double map this page so that global
// pointers can be maintained to provide system access from any process context.
// This is needed for things like mutexes and WSL chains.
//

typedef struct _MM_SESSION_SPACE {

    //
    // This is a pointer in global system address space, used to make various
    // fields that can be referenced from any process visible from any process
    // context.  This is for things like mutexes, WSL chains, etc.
    //

    struct _MM_SESSION_SPACE *GlobalVirtualAddress;

    LONG ReferenceCount;

    union {
        ULONG LongFlags;
        MM_SESSION_SPACE_FLAGS Flags;
    } u;

    ULONG SessionId;

    //
    // This is the list of the processes in this group that have
    // session space entries.
    //

    LIST_ENTRY ProcessList;

    LARGE_INTEGER LastProcessSwappedOutTime;

    //
    // All the page tables for session space use this as their parent.
    // Note that it's not really a page directory - it's really a page
    // table page itself (the one used to map this very structure).
    //
    // This provides a reference to something that won't go away and
    // is relevant regardless of which process within the session is current.
    //

    PFN_NUMBER SessionPageDirectoryIndex;

    //
    // This is the count of non paged allocations to support this session
    // space.  This includes the session structure page table and data pages,
    // WSL page table and data pages, session pool page table pages and session
    // image page table pages.  These are all charged against
    // MmResidentAvailable.
    //

    SIZE_T NonPageablePages;

    //
    // This is the count of pages in this session that have been charged against
    // the systemwide commit.  This includes all the NonPageablePages plus the
    // data pages they typically map.
    //

    SIZE_T CommittedPages;

    //
    // Start of session paged pool virtual space.
    //

    PVOID PagedPoolStart;

    //
    // Current end of pool virtual space. Can be extended to the
    // end of the session space.
    //

    PVOID PagedPoolEnd;

    //
    // PTE pointers for pool.
    //

    PMMPTE PagedPoolBasePde;

    ULONG Color;

    LONG ResidentProcessCount;

    ULONG SessionPoolAllocationFailures[4];

    //
    // This is the list of system images currently valid in
    // this session space.  This information is in addition
    // to the module global information in PsLoadedModuleList.
    //

    LIST_ENTRY ImageList;

    LCID LocaleId;

    //
    // The count of "known attachers and the associated event.
    //

    ULONG AttachCount;

    KEVENT AttachEvent;

    PEPROCESS LastProcess;

    //
    // This is generally decremented in process delete (not clean) so that
    // the session data page and mapping PTE can finally be freed when this
    // reaches zero.  smss is the only process that decrements it in other
    // places as smss never exits.
    //

    LONG ProcessReferenceToSession;

    //
    // This chain is in global system addresses (not session VAs) and can
    // be walked from any system context, ie: for WSL trimming.
    //

    LIST_ENTRY WsListEntry;

    //
    // Session lookasides for fast pool allocation/freeing.
    //

    GENERAL_LOOKASIDE Lookaside[SESSION_POOL_SMALL_LISTS];

    //
    // Support for mapping system views into session space.  Each desktop
    // allocates a 3MB heap and the global system view space is only 48M
    // total.  This would limit us to only 20-30 users - rotating the
    // system view space with each session removes this limitation.
    //

    MMSESSION Session;

    //
    // Session space paged pool support.
    //

    KGUARDED_MUTEX PagedPoolMutex;

    MM_PAGED_POOL_INFO PagedPoolInfo;

    //
    // Working set information.
    //

    MMSUPPORT  Vm;
    PMMWSLE    Wsle;

    PDRIVER_UNLOAD Win32KDriverUnload;

    //
    // Pool descriptor for less than 1 page allocations.
    //

    POOL_DESCRIPTOR PagedPool;

#if (_MI_PAGING_LEVELS >= 3)

    //
    // The page directory that maps session space is saved here so
    // trimmers can attach.
    //

    MMPTE PageDirectory;

#else

    //
    // The second level page tables that map session space are shared
    // by all processes in the session.
    //

    PMMPTE PageTables;

#endif

#if defined (_WIN64)

    //
    // NT64 has enough virtual address space to support per-session special
    // pool.
    //

    PMMPTE SpecialPoolFirstPte;
    PMMPTE SpecialPoolLastPte;
    PMMPTE NextPdeForSpecialPoolExpansion;
    PMMPTE LastPdeForSpecialPoolExpansion;
    PFN_NUMBER SpecialPagesInUse;
#endif

    LONG ImageLoadingCount;

#if DBG
    ULONG Debug[MM_SESS_COUNTER_MAX];

    MM_SESSION_MEMORY_COUNTERS Debug2[MM_SESS_MEMORY_COUNTER_MAX];
#endif

} MM_SESSION_SPACE, *PMM_SESSION_SPACE;

extern PMM_SESSION_SPACE MmSessionSpace;

extern ULONG MiSessionCount;

//
// This flushes just the non-global TB entries.
//

#define MI_FLUSH_SESSION_TB() KeFlushEntireTb (TRUE, TRUE);

//
// The default number of pages for the session working set minimum & maximum.
//

#define MI_SESSION_SPACE_WORKING_SET_MINIMUM 20

#define MI_SESSION_SPACE_WORKING_SET_MAXIMUM 384

NTSTATUS
MiSessionInsertImage (
    IN PVOID BaseAddress
    );

NTSTATUS
MiSessionCommitPageTables (
    PVOID StartVa,
    PVOID EndVa
    );

NTSTATUS
MiInitializeAndChargePfn (
    OUT PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PFN_NUMBER ContainingPageFrame,
    IN LOGICAL SessionAllocation
    );

VOID
MiSessionPageTableRelease (
    IN PFN_NUMBER PageFrameIndex
    );

NTSTATUS
MiInitializeSessionPool (
    VOID
    );

VOID
MiCheckSessionPoolAllocations (
    VOID
    );

VOID
MiFreeSessionPoolBitMaps (
    VOID
    );

VOID
MiDetachSession (
    VOID
    );

VOID
MiAttachSession (
    IN PMM_SESSION_SPACE SessionGlobal
    );

VOID
MiReleaseProcessReferenceToSessionDataPage (
    PMM_SESSION_SPACE SessionGlobal
    );

extern PMMPTE MiHighestUserPte;
extern PMMPTE MiHighestUserPde;
#if (_MI_PAGING_LEVELS >= 4)
extern PMMPTE MiHighestUserPpe;
extern PMMPTE MiHighestUserPxe;
#endif

NTSTATUS
MiEmptyWorkingSet (
    IN PMMSUPPORT WsInfo,
    IN LOGICAL NeedLock
    );

//++
// ULONG
// MiGetPdeSessionIndex (
//    IN PVOID va
//    );
//
// Routine Description:
//
//    MiGetPdeSessionIndex returns the session structure index for the PDE
//    will (or does) map the given virtual address.
//
// Arguments
//
//    Va - Supplies the virtual address to locate the PDE index for.
//
// Return Value:
//
//    The index of the PDE entry.
//
//--

#define MiGetPdeSessionIndex(va)  ((ULONG)(((ULONG_PTR)(va) - (ULONG_PTR)MmSessionBase) >> PDI_SHIFT))

//
// Session space contains the image loader and tracker, virtual
// address allocator, paged pool allocator, system view image mappings,
// and working set for kernel mode virtual addresses that are instanced
// for groups of processes in a Session process group. This
// process group is identified by a SessionId.
//
// Each Session process group's loaded kernel modules, paged pool
// allocations, working set, and mapped system views are separate from
// other Session process groups, even though they have the same
// virtual addresses.
//
// This is to support the Hydra multi-user Windows NT system by
// replicating WIN32K.SYS, and its complement of video and printer drivers,
// desktop heaps, memory allocations, etc.
//

//
// Structure linked into a session space structure to describe
// which system images in PsLoadedModuleTable and
// SESSION_DRIVER_GLOBAL_LOAD_ADDRESS's
// have been allocated for the current session space.
//
// The reference count tracks the number of loads of this image within
// this session.
//

typedef struct _SESSION_GLOBAL_SUBSECTION_INFO {
    ULONG_PTR PteIndex;
    ULONG PteCount;
    ULONG Protection;
} SESSION_GLOBAL_SUBSECTION_INFO, *PSESSION_GLOBAL_SUBSECTION_INFO;

typedef struct _IMAGE_ENTRY_IN_SESSION {
    LIST_ENTRY Link;
    PVOID Address;
    PVOID LastAddress;
    ULONG ImageCountInThisSession;
    LOGICAL ImageLoading;  // Mods to this field protected by system load mutant
    PMMPTE PrototypePtes;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PSESSION_GLOBAL_SUBSECTION_INFO GlobalSubs;
} IMAGE_ENTRY_IN_SESSION, *PIMAGE_ENTRY_IN_SESSION;

extern LIST_ENTRY MiSessionWsList;

NTSTATUS
FASTCALL
MiCheckPdeForSessionSpace(
    IN PVOID VirtualAddress
    );

NTSTATUS
MiShareSessionImage (
    IN PVOID BaseAddress,
    IN PSECTION Section
    );

VOID
MiSessionWideInitializeAddresses (
    VOID
    );

NTSTATUS
MiSessionWideReserveImageAddress (
    IN PSECTION Section,
    OUT PVOID *AssignedAddress,
    OUT PSECTION *NewSectionPointer
    );

VOID
MiInitializeSessionIds (
    VOID
    );

VOID
MiInitializeSessionWsSupport(
    VOID
    );

VOID
MiSessionAddProcess (
    IN PEPROCESS NewProcess
    );

VOID
MiSessionRemoveProcess (
    VOID
    );

VOID
MiRemoveImageSessionWide (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry OPTIONAL,
    IN PVOID BaseAddress,
    IN ULONG_PTR NumberOfBytes
    );

NTSTATUS
MiDeleteSessionVirtualAddresses(
    IN PVOID VirtualAddress,
    IN SIZE_T NumberOfBytes
    );

NTSTATUS
MiUnloadSessionImageByForce (
    IN SIZE_T NumberOfPtes,
    IN PVOID ImageBase
    );

PIMAGE_ENTRY_IN_SESSION
MiSessionLookupImage (
    IN PVOID BaseAddress
    );

VOID
MiSessionUnloadAllImages (
    VOID
    );

VOID
MiFreeSessionSpaceMap (
    VOID
    );

NTSTATUS
MiSessionInitializeWorkingSetList (
    VOID
    );

VOID
MiSessionUnlinkWorkingSet (
    VOID
    );

VOID
MiSessionOutSwapProcess (
    IN PEPROCESS Process
    );

VOID
MiSessionInSwapProcess (
    IN PEPROCESS Process,
    IN LOGICAL Forced
    );

BOOLEAN
FORCEINLINE
MiCompareTbFlushTimeStamp (
    ULONG OldStamp,
    ULONG Mask
    )
{
    ULONG NewStamp;
    ULONG Diff;

    while (1) {

        NewStamp = KeReadTbFlushTimeStamp ();
        Diff = ((NewStamp - OldStamp) & Mask);

#if defined(NT_UP)

        if (Diff != 0) {
            return FALSE;
        }

#else

        //
        // If they are more than two apart it doesn't matter - a flush has
        // occurred.
        //

        if (Diff > 2) {
            return FALSE;
        }

        //
        // If we captured the original stamp when unlocked then we
        // only need the lock and unlock to have occurred.
        //

        if (((OldStamp & 1) == 0) && (Diff >= 2)) {
            return FALSE;
        }

        //
        // If the timestamp is currently locked then just wait
        // until it's unlocked.
        //

        if (NewStamp & 1) {
            KeLoopTbFlushTimeStampUnlocked ();
            continue;
        }
#endif

        //
        // Tell the caller to flush.
        //
        return TRUE;
    }
}



#if !defined (_X86PAE_)

#define MI_GET_DIRECTORY_FRAME_FROM_PROCESS(_Process) \
        MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)(&((_Process)->Pcb.DirectoryTableBase[0])))

#define MI_GET_HYPER_PAGE_TABLE_FRAME_FROM_PROCESS(_Process) \
        MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)(&((_Process)->Pcb.DirectoryTableBase[1])))

#else

#define MI_GET_DIRECTORY_FRAME_FROM_PROCESS(_Process) \
        (MI_GET_PAGE_FRAME_FROM_PTE(((PMMPTE)((_Process)->PaeTop)) + PD_PER_SYSTEM - 1))

#define MI_GET_HYPER_PAGE_TABLE_FRAME_FROM_PROCESS(_Process) \
        ((PFN_NUMBER)((_Process)->Pcb.DirectoryTableBase[1]))

#endif


//
// The LDR_DATA_TABLE_ENTRY->LoadedImports is used as a list of imported DLLs.
//
// This field is zero if the module was loaded at boot time and the
// import information was never filled in.
//
// This field is -1 if no imports are defined by the module.
//
// This field contains a valid paged pool PLDR_DATA_TABLE_ENTRY pointer
// with a low-order (bit 0) tag of 1 if there is only 1 usable import needed
// by this driver.
//
// This field will contain a valid paged pool PLOAD_IMPORTS pointer in all
// other cases (ie: where at least 2 imports exist).
//

typedef struct _LOAD_IMPORTS {
    SIZE_T                  Count;
    PKLDR_DATA_TABLE_ENTRY   Entry[1];
} LOAD_IMPORTS, *PLOAD_IMPORTS;

#define LOADED_AT_BOOT  ((PLOAD_IMPORTS)0)
#define NO_IMPORTS_USED ((PLOAD_IMPORTS)-2)

#define SINGLE_ENTRY(ImportVoid)    ((ULONG)((ULONG_PTR)(ImportVoid) & 0x1))

#define SINGLE_ENTRY_TO_POINTER(ImportVoid)    ((PKLDR_DATA_TABLE_ENTRY)((ULONG_PTR)(ImportVoid) & ~0x1))

#define POINTER_TO_SINGLE_ENTRY(Pointer)    ((PKLDR_DATA_TABLE_ENTRY)((ULONG_PTR)(Pointer) | 0x1))

#define MI_ASSERT_NOT_SESSION_DATA(PTE)
#define MI_LOG_SESSION_DATA_START(DataTableEntry)

//
// This tracks driver-specified individual verifier thunks.
//

typedef struct _DRIVER_SPECIFIED_VERIFIER_THUNKS {
    LIST_ENTRY ListEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ULONG NumberOfThunks;
} DRIVER_SPECIFIED_VERIFIER_THUNKS, *PDRIVER_SPECIFIED_VERIFIER_THUNKS;

#define MI_SNAP_SUB(_Sub, callerid)

//
//  Hot-patching private definitions
//

extern LIST_ENTRY MiHotPatchList;

//
//

extern ULONG MiFlushTbForAttributeChange;
extern ULONG MiFlushCacheForAttributeChange;

VOID
FORCEINLINE
MI_PTE_LOG_ACCESS (
    PMMPTE PointerPte
    )

/*++

Routine Desctiption:

    This function is called to log a virtual address that has been accessed.

Arguments:

    PointerPte - Supplies a pointer to the still-valid PTE.

Return Value:

    None.

Environment:

    Kernel Mode.  IRQL < DISPATCH_LEVEL.

    Working set pushlock held shared or exclusive, APCs disabled.

--*/
{
    UNREFERENCED_PARAMETER (PointerPte);

    return;
}

FORCEINLINE
ULONG
MmGetNodeColor (
    VOID
    )
{
    ULONG Color;

#if !defined(NT_UP)

    PKPRCB Prcb;

    Prcb = KeGetCurrentPrcb ();

    Prcb->PageColor += 1;

    Color = Prcb->PageColor;

    Color &= Prcb->SecondaryColorMask;

    Color |= Prcb->NodeShiftedColor;

#else

    MmSystemPageColor += 1;

    Color = MmSystemPageColor;

    Color &= MmSecondaryColorMask;

#endif

    return Color;
}

//++
// ULONG
// MI_GET_PAGE_COLOR_FROM_PTE (
//    IN PMMPTE PTEADDRESS
//    );
//
// Routine Description:
//
//    This macro determines the pages color based on the PTE address
//    that maps the page.
//
// Arguments:
//
//    PTEADDRESS - Supplies the PTE address the page is (or was) mapped at.
//
// Return Value:
//
//    The page's color.
//
//--

#define MI_GET_PAGE_COLOR_FROM_PTE(PTEADDRESS)  MmGetNodeColor()

//++
// ULONG
// MI_GET_PAGE_COLOR_FROM_VA (
//    IN PVOID ADDRESS
//    );
//
// Routine Description:
//
//    This macro determines the pages color based on the PTE address
//    that maps the page.
//
// Arguments:
//
//    ADDRESS - Supplies the address the page is (or was) mapped at.
//
// Return Value:
//
//    The page's color.
//
//--


#define MI_GET_PAGE_COLOR_FROM_VA(ADDRESS)  MI_GET_PAGE_COLOR_FROM_PTE(ADDRESS)


//++
// ULONG
// MI_GET_PAGE_COLOR_FROM_SESSION (
//    IN PMM_SESSION_SPACE SessionSpace
//    );
//
// Routine Description:
//
//    This macro determines the page's color based on the PTE address
//    that maps the page.
//
// Arguments
//
//    SessionSpace - Supplies the session space the page will be mapped into.
//
// Return Value:
//
//    The page's color.
//
//--


#define MI_GET_PAGE_COLOR_FROM_SESSION(_SessionSpace)  \
         (((ULONG)((++_SessionSpace->Color) & MmSecondaryColorMask)) | MI_CURRENT_NODE_COLOR)


//++
// ULONG
// MI_PAGE_COLOR_PTE_PROCESS (
//    IN PMMPTE PTE,
//    IN PUSHORT COLOR
//    );
//
// Routine Description:
//
//    Select page color for this process.
//
// Arguments
//
//   PTE    Not used.
//   COLOR  Value from which color is determined.   This
//          variable is incremented.
//
// Return Value:
//
//    The page's color.
//
//--


#define MI_PAGE_COLOR_PTE_PROCESS(PTE,COLOR)  \
         (((ULONG)(++(*(COLOR))) & MmSecondaryColorMask) | MI_CURRENT_NODE_COLOR)

//++
// ULONG
// MI_PAGE_COLOR_VA_PROCESS (
//    IN PVOID ADDRESS,
//    IN PEPROCESS COLOR
//    );
//
// Routine Description:
//
//    This macro determines the pages color based on the PTE address
//    that maps the page.
//
// Arguments:
//
//    ADDRESS - Supplies the address the page is (or was) mapped at.
//
// Return Value:
//
//    The pages color.
//
//--

#define MI_PAGE_COLOR_VA_PROCESS(ADDRESS,COLOR) \
         (((ULONG)(++(*(COLOR))) & MmSecondaryColorMask) | MI_CURRENT_NODE_COLOR)

#endif  // MI

