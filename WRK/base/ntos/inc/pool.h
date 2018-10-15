/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    pool.h

Abstract:

    Private executive data structures and prototypes for pool management.

    There are a number of different pool types:
        1. NonPaged.
        2. Paged.
        3. Session (always paged, but virtualized per TS session).

--*/

#ifndef _POOL_
#define _POOL_

#if !DBG
#define NO_POOL_CHECKS 1
#endif

#define POOL_CACHE_SUPPORTED 0
#define POOL_CACHE_ALIGN 0

#define NUMBER_OF_POOLS 2

#if defined(NT_UP)
#define NUMBER_OF_PAGED_POOLS 2
#else
#define NUMBER_OF_PAGED_POOLS 4
#endif

#define BASE_POOL_TYPE_MASK 1

#define MUST_SUCCEED_POOL_TYPE_MASK 2

#define CACHE_ALIGNED_POOL_TYPE_MASK 4

#define SESSION_POOL_MASK 32

#define POOL_VERIFIER_MASK 64

#define POOL_DRIVER_MASK 128        // Note this cannot encode into a header.

//
// WARNING: POOL_QUOTA_MASK is overloaded by POOL_QUOTA_FAIL_INSTEAD_OF_RAISE
//          which is exported from ex.h.
//
// WARNING: POOL_RAISE_IF_ALLOCATION_FAILURE is exported from ex.h with a
//          value of 16.
//
// These definitions are used to control the raising of an exception as the
// result of quota and allocation failures.
//

#define POOL_QUOTA_MASK 8

#define POOL_TYPE_MASK (3)

//
// Size of a pool page.
//
// This must be greater than or equal to the page size.
//

#define POOL_PAGE_SIZE  PAGE_SIZE

//
// The page size must be a multiple of the smallest pool block size.
//
// Define the block size.
//

#if (PAGE_SIZE == 0x4000)
#define POOL_BLOCK_SHIFT 5
#elif (PAGE_SIZE == 0x2000)
#define POOL_BLOCK_SHIFT 4
#else
C_ASSERT(PAGE_SIZE == 0x1000);
#if defined (_WIN64)
#define POOL_BLOCK_SHIFT 4
#else
#define POOL_BLOCK_SHIFT 3
#endif

#endif

#define POOL_LIST_HEADS (POOL_PAGE_SIZE / (1 << POOL_BLOCK_SHIFT))

#define PAGE_ALIGNED(p) (!(((ULONG_PTR)p) & (POOL_PAGE_SIZE - 1)))

//
// Define page end macro.
//

#define PAGE_END(Address) (((ULONG_PTR)(Address) & (PAGE_SIZE - 1)) == 0)

//
// Define pool descriptor structure.
//

typedef struct _POOL_DESCRIPTOR {
    POOL_TYPE PoolType;
    ULONG PoolIndex;
    ULONG RunningAllocs;
    ULONG RunningDeAllocs;
    ULONG TotalPages;
    ULONG TotalBigPages;
    ULONG Threshold;
    PVOID LockAddress;
    PVOID PendingFrees;
    LONG PendingFreeDepth;
    SIZE_T TotalBytes;
    SIZE_T Spare0;
    LIST_ENTRY ListHeads[POOL_LIST_HEADS];
} POOL_DESCRIPTOR, *PPOOL_DESCRIPTOR;

//
//      Caveat Programmer:
//
//              The pool header must be QWORD (8 byte) aligned in size.  If it
//              is not, the pool allocation code will trash the allocated
//              buffer.
//
//
//
// The layout of the pool header is:
//
//         31              23         16 15             7            0
//         +----------------------------------------------------------+
//         | Current Size |  PoolType+1 |  Pool Index  |Previous Size |
//         +----------------------------------------------------------+
//         |   ProcessBilled   (NULL if not allocated with quota)     |
//         +----------------------------------------------------------+
//         | Zero or more longwords of pad such that the pool header  |
//         | is on a cache line boundary and the pool body is also    |
//         | on a cache line boundary.                                |
//         +----------------------------------------------------------+
//
//      PoolBody:
//
//         +----------------------------------------------------------+
//         |  Used by allocator, or when free FLINK into sized list   |
//         +----------------------------------------------------------+
//         |  Used by allocator, or when free BLINK into sized list   |
//         +----------------------------------------------------------+
//         ... rest of pool block...
//
//
// N.B. The size fields of the pool header are expressed in units of the
//      smallest pool block size.
//
// N.B. AMD64 has a 16 byte allocation size and 4096 byte page size. Therefore,
//      byte fields in the pool header are adequate.
//

typedef struct _POOL_HEADER {
    union {
        struct {
#if defined(_AMD64_)
            ULONG PreviousSize : 8;
            ULONG PoolIndex : 8;
            ULONG BlockSize : 8;
            ULONG PoolType : 8;
#else
            USHORT PreviousSize : 9;
            USHORT PoolIndex : 7;
            USHORT BlockSize : 9;
            USHORT PoolType : 7;
#endif
        };
        ULONG Ulong1;
    };
#if defined (_WIN64)
    ULONG PoolTag;
#endif
    union {
#if defined (_WIN64)
        EPROCESS *ProcessBilled;
#else
        ULONG PoolTag;
#endif
        struct {
            USHORT AllocatorBackTraceIndex;
            USHORT PoolTagHash;
        };
    };
} POOL_HEADER, *PPOOL_HEADER;

//
// Define size of pool block overhead.
//

#define POOL_OVERHEAD ((LONG)sizeof(POOL_HEADER))

//
// Define size of pool block overhead when the block is on a freelist.
//

#define POOL_FREE_BLOCK_OVERHEAD  (POOL_OVERHEAD + sizeof (LIST_ENTRY))

//
// Define dummy type so computation of pointers is simplified.
//

typedef struct _POOL_BLOCK {
    UCHAR Fill[1 << POOL_BLOCK_SHIFT];
} POOL_BLOCK, *PPOOL_BLOCK;

//
// Define size of smallest pool block.
//

#define POOL_SMALLEST_BLOCK (sizeof(POOL_BLOCK))

//
// Define pool tracking information.
//

#define POOL_BACKTRACEINDEX_PRESENT 0x8000

#if POOL_CACHE_SUPPORTED
#define POOL_BUDDY_MAX PoolBuddyMax
#else
#define POOL_BUDDY_MAX  \
   (POOL_PAGE_SIZE - (POOL_OVERHEAD + POOL_SMALLEST_BLOCK ))
#endif

//
// Pool support routines are not for general consumption.
// These are only used by the memory manager.
//

VOID
ExInitializePoolDescriptor (
    IN PPOOL_DESCRIPTOR PoolDescriptor,
    IN POOL_TYPE PoolType,
    IN ULONG PoolIndex,
    IN ULONG Threshold,
    IN PVOID PoolLock
    );

VOID
ExDrainPoolLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside
    );

VOID
ExDeferredFreePool (
     IN PPOOL_DESCRIPTOR PoolDesc
     );

PVOID
ExCreatePoolTagTable (
    IN ULONG NewProcessorNumber,
    IN UCHAR NodeNumber
    );

VOID
ExDeletePoolTagTable (
    IN ULONG NewProcessorNumber
    );

#define EX_CHECK_POOL_FREES_FOR_ACTIVE_TIMERS         0x1
#define EX_CHECK_POOL_FREES_FOR_ACTIVE_WORKERS        0x2
#define EX_CHECK_POOL_FREES_FOR_ACTIVE_RESOURCES      0x4
#define EX_KERNEL_VERIFIER_ENABLED                    0x8
#define EX_VERIFIER_DEADLOCK_DETECTION_ENABLED       0x10
#define EX_SPECIAL_POOL_ENABLED                      0x20
#define EX_PRINT_POOL_FAILURES                       0x40
#define EX_STOP_ON_POOL_FAILURES                     0x80
#define EX_SEPARATE_HOT_PAGES_DURING_BOOT           0x100
#define EX_DELAY_POOL_FREES                         0x200

VOID
ExSetPoolFlags (
    IN ULONG PoolFlag
    );

//++
//SIZE_T
//EX_REAL_POOL_USAGE (
//    IN SIZE_T SizeInBytes
//    );
//
// Routine Description:
//
//    This routine determines the real pool cost of the supplied allocation.
//
// Arguments
//
//    SizeInBytes - Supplies the allocation size in bytes.
//
// Return Value:
//
//    TRUE if unused segment trimming should be initiated, FALSE if not.
//
//--

#define EX_REAL_POOL_USAGE(SizeInBytes)                             \
        (((SizeInBytes) > POOL_BUDDY_MAX) ?                         \
            (ROUND_TO_PAGES(SizeInBytes)) :                         \
            (((SizeInBytes) + POOL_OVERHEAD + (POOL_SMALLEST_BLOCK - 1)) & ~(POOL_SMALLEST_BLOCK - 1)))

typedef struct _POOL_TRACKER_TABLE {
    ULONG Key;
    ULONG NonPagedAllocs;
    ULONG NonPagedFrees;
    SIZE_T NonPagedBytes;
    ULONG PagedAllocs;
    ULONG PagedFrees;
    SIZE_T PagedBytes;
} POOL_TRACKER_TABLE, *PPOOL_TRACKER_TABLE;

//
// N.B. The last entry of the pool tracker table is used for all overflow
//      table entries.
//

extern PPOOL_TRACKER_TABLE PoolTrackTable;

typedef struct _POOL_TRACKER_BIG_PAGES {
    PVOID Va;
    ULONG Key;
    ULONG NumberOfPages;
    PVOID QuotaObject;
} POOL_TRACKER_BIG_PAGES, *PPOOL_TRACKER_BIG_PAGES;

#endif
