/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   sysptes.c

Abstract:

    This module contains the routines which reserve and release
    system wide PTEs reserved within the non paged portion of the
    system space.  These PTEs are used for mapping I/O devices
    and mapping kernel stacks for threads.

--*/

#include "mi.h"

#ifdef MI_TB_FLUSH_TYPE_MAX
ULONG MiFlushType[MI_TB_FLUSH_TYPE_MAX];
#endif

VOID
MiFeedSysPtePool (
    IN ULONG Index
    );

ULONG
MiGetSystemPteListCount (
    IN ULONG ListSize
    );

VOID
MiPteSListExpansionWorker (
    IN PVOID Context
    );

LOGICAL
MiRecoverExtraPtes (
    ULONG NumberOfPtes
    );

#if !defined (_WIN64)
extern ULONG MiSpecialPoolExtraCount;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiInitializeSystemPtes)
#pragma alloc_text(PAGE,MiPteSListExpansionWorker)
#pragma alloc_text(MISYSPTE,MiReserveAlignedSystemPtes)
#pragma alloc_text(MISYSPTE,MiReserveSystemPtes)
#pragma alloc_text(MISYSPTE,MiFeedSysPtePool)
#pragma alloc_text(MISYSPTE,MiReleaseSystemPtes)
#pragma alloc_text(MISYSPTE,MiGetSystemPteListCount)
#endif

PVOID MiLowestSystemPteVirtualAddress;

ULONG MmTotalSystemPtes;
ULONG MmTotalFreeSystemPtes[MaximumPtePoolTypes];
PMMPTE MmSystemPtesStart[MaximumPtePoolTypes];
PMMPTE MmSystemPtesEnd[MaximumPtePoolTypes];
ULONG MmPteFailures[MaximumPtePoolTypes];
MMPTE MiAddPtesList;

PMMPTE MiPteStart;
PRTL_BITMAP MiPteStartBitmap;
PRTL_BITMAP MiPteEndBitmap;
extern KSPIN_LOCK MiPteTrackerLock;

ULONG MiSystemPteAllocationFailed;

extern PMMPTE MmSharedUserDataPte;

//
// Keep a list of the PTE ranges.
//

ULONG MiPteRangeIndex;
MI_PTE_RANGES MiPteRanges[MI_NUMBER_OF_PTE_RANGES];

#if defined (_AMD64_)

//
// AMD64 has a 4k page size.
// Small stacks consume 7 pages (including the guard page).
// Large stacks consume 19 pages (including the guard page).
//
// PTEs are binned at sizes 1, 2, 4, 7, 8, 16 and 19.
//

#define MM_SYS_PTE_TABLES_MAX 7

#define MM_PTE_TABLE_LIMIT 19

ULONG MmSysPteIndex[MM_SYS_PTE_TABLES_MAX] = {1,2,4,7,8,16,MM_PTE_TABLE_LIMIT};

UCHAR MmSysPteTables[MM_PTE_TABLE_LIMIT+1] = {0,0,1,2,2,3,3,3,4,5,5,5,5,5,5,5,5,6,6,6};

ULONG MmSysPteMinimumFree [MM_SYS_PTE_TABLES_MAX] = {100,50,30,100,20,20,20};

#else

//
// x86 has a 4k page size.
// Small stacks consume 4 pages (including the guard page).
// Large stacks consume 16 pages (including the guard page).
//
// PTEs are binned at sizes 1, 2, 4, 8, and 16.
//

#define MM_SYS_PTE_TABLES_MAX 5

#define MM_PTE_TABLE_LIMIT 16

ULONG MmSysPteIndex[MM_SYS_PTE_TABLES_MAX] = {1,2,4,8,MM_PTE_TABLE_LIMIT};

UCHAR MmSysPteTables[MM_PTE_TABLE_LIMIT+1] = {0,0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};

ULONG MmSysPteMinimumFree [MM_SYS_PTE_TABLES_MAX] = {100,50,30,20,20};

#endif

KSPIN_LOCK MiSystemPteSListHeadLock;
SLIST_HEADER MiSystemPteSListHead;

#define MM_MIN_SYSPTE_FREE 500
#define MM_MAX_SYSPTE_FREE 3000

ULONG MmSysPteListBySizeCount [MM_SYS_PTE_TABLES_MAX];

//
// Initial sizes for PTE lists.
//

#define MM_PTE_LIST_1  400
#define MM_PTE_LIST_2  100
#define MM_PTE_LIST_4   60
#define MM_PTE_LIST_6  100
#define MM_PTE_LIST_8   50
#define MM_PTE_LIST_9   50
#define MM_PTE_LIST_16  40
#define MM_PTE_LIST_18  40
#define MM_PTE_LIST_19  40

PVOID MiSystemPteNBHead[MM_SYS_PTE_TABLES_MAX];
LONG MiSystemPteFreeCount[MM_SYS_PTE_TABLES_MAX];

ULONG MiSysPteTimeStamp[MaximumPtePoolTypes];

#if defined(_WIN64)
#define MI_MAXIMUM_SLIST_PTE_PAGES 16
#else
#define MI_MAXIMUM_SLIST_PTE_PAGES 8
#endif

typedef struct _MM_PTE_SLIST_EXPANSION_WORK_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    LONG Active;
    ULONG SListPages;
} MM_PTE_SLIST_EXPANSION_WORK_CONTEXT, *PMM_PTE_SLIST_EXPANSION_WORK_CONTEXT;

MM_PTE_SLIST_EXPANSION_WORK_CONTEXT MiPteSListExpand;

VOID
MiDumpSystemPtes (
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

ULONG
MiCountFreeSystemPtes (
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

PVOID
MiGetHighestPteConsumer (
    OUT PULONG_PTR NumberOfPtes
    );

VOID
MiCheckPteReserve (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes
    );

VOID
MiCheckPteRelease (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes
    );

extern ULONG MiCacheOverride[4];

#if DBG
extern PFN_NUMBER MiCurrentAdvancedPages;
extern PFN_NUMBER MiAdvancesGiven;
extern PFN_NUMBER MiAdvancesFreed;
#endif

PVOID
MiMapLockedPagesInUserSpace (
     IN PMDL MemoryDescriptorList,
     IN PVOID StartingVa,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseVa
     );

VOID
MiUnmapLockedPagesInUserSpace (
     IN PVOID BaseAddress,
     IN PMDL MemoryDescriptorList
     );

VOID
MiInsertPteTracker (
    IN PMDL MemoryDescriptorList,
    IN ULONG Flags,
    IN LOGICAL IoMapping,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute,
    IN PVOID MyCaller,
    IN PVOID MyCallersCaller
    );

VOID
MiRemovePteTracker (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID VirtualAddress,
    IN PFN_NUMBER NumberOfPtes
    );

LOGICAL
MiGetSystemPteAvailability (
    IN ULONG NumberOfPtes,
    IN MM_PAGE_PRIORITY Priority
    );

//
// Define inline functions to pack and unpack pointers in the platform
// specific non-blocking queue pointer structure.
//

typedef struct _PTE_SLIST {
    union {
        struct {
            SINGLE_LIST_ENTRY ListEntry;
        } Slist;
        NBQUEUE_BLOCK QueueBlock;
    } u1;
} PTE_SLIST, *PPTE_SLIST;

#if defined (_AMD64_)

typedef union _PTE_QUEUE_POINTER {
    struct {
        LONG64 PointerPte : 48;
        ULONG64 TimeStamp : 16;
    };

    LONG64 Data;
} PTE_QUEUE_POINTER, *PPTE_QUEUE_POINTER;

#elif defined(_X86_)

typedef union _PTE_QUEUE_POINTER {
    struct {
        LONG PointerPte;
        LONG TimeStamp;
    };

    LONG64 Data;
} PTE_QUEUE_POINTER, *PPTE_QUEUE_POINTER;

#else

#error "no target architecture"

#endif



#if defined(_AMD64_)

__inline
VOID
PackPTEValue (
    IN PPTE_QUEUE_POINTER Entry,
    IN PMMPTE PointerPte,
    IN ULONG TimeStamp
    )
{
    Entry->PointerPte = (LONG64)PointerPte;
    Entry->TimeStamp = (LONG64)TimeStamp;
    return;
}

__inline
PMMPTE
UnpackPTEPointer (
    IN PPTE_QUEUE_POINTER Entry
    )
{
    return (PMMPTE)(Entry->PointerPte);
}

#define SYSPTES_FLUSH_COUNTER_MASK 0xFFFF

#elif defined(_X86_)

__inline
VOID
PackPTEValue (
    IN PPTE_QUEUE_POINTER Entry,
    IN PMMPTE PointerPte,
    IN ULONG TimeStamp
    )
{
    Entry->PointerPte = (LONG)PointerPte;
    Entry->TimeStamp = (LONG)TimeStamp;
    return;
}

__inline
PMMPTE
UnpackPTEPointer (
    IN PPTE_QUEUE_POINTER Entry
    )
{
    return (PMMPTE)(Entry->PointerPte);
}

#define SYSPTES_FLUSH_COUNTER_MASK 0xFFFFFFFF

#else

#error "no target architecture"

#endif

__inline
ULONG
UnpackPTETimeStamp (
    IN PPTE_QUEUE_POINTER Entry
    )
{
    return (ULONG)(Entry->TimeStamp);
}


PMMPTE
MiReserveSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

/*++

Routine Description:

    This function locates the specified number of unused PTEs
    within the non paged portion of system space.

Arguments:

    NumberOfPtes - Supplies the number of PTEs to locate.

    SystemPtePoolType - Supplies the PTE type of the pool to expand, one of
                        SystemPteSpace or NonPagedPoolExpansion.

Return Value:

    Returns the address of the first PTE located.
    NULL if no system PTEs can be located.

Environment:

    Kernel mode, DISPATCH_LEVEL or below.

--*/

{
    ULONG i;
    PMMPTE PointerPte;
    ULONG Index;
    ULONG TimeStamp;
    PTE_QUEUE_POINTER Value;

    if (SystemPtePoolType == SystemPteSpace) {

        if (NumberOfPtes <= MM_PTE_TABLE_LIMIT) {
            Index = MmSysPteTables [NumberOfPtes];
            ASSERT (NumberOfPtes <= MmSysPteIndex[Index]);

            if (ExRemoveHeadNBQueue (MiSystemPteNBHead[Index], (PULONG64)&Value) == TRUE) {
                InterlockedDecrement ((PLONG)&MmSysPteListBySizeCount[Index]);

                PointerPte = UnpackPTEPointer (&Value);

                TimeStamp = UnpackPTETimeStamp (&Value);

                ASSERT (PointerPte >= MmSystemPtesStart[SystemPtePoolType]);
                ASSERT (PointerPte <= MmSystemPtesEnd[SystemPtePoolType]);

#if DBG
                PointerPte->u.List.NextEntry = 0xABCDE;
                NumberOfPtes = MmSysPteIndex[Index];
                ASSERT (NumberOfPtes != 0);
                PointerPte += NumberOfPtes;

                do {
                    PointerPte -= 1;
                    ASSERT (PointerPte->u.Hard.Valid == 0);
                    NumberOfPtes -= 1;
                } while (NumberOfPtes != 0);
#endif

                i = MmSysPteMinimumFree[Index];

                if (MmSysPteListBySizeCount[Index] < i) {
                    MiFeedSysPtePool (Index);
                }

                //
                // The last thing is to check whether the TB needs flushing.
                //

                if (MiCompareTbFlushTimeStamp (TimeStamp, SYSPTES_FLUSH_COUNTER_MASK)) {
                    MI_FLUSH_ENTIRE_TB (0xC);
                }

                if (MmTrackPtes & 0x2) {
                    MiCheckPteReserve (PointerPte, MmSysPteIndex[Index]);
                }

                return PointerPte;
            }

            //
            // Fall through and go the long way to satisfy the PTE request.
            //

            NumberOfPtes = MmSysPteIndex [Index];
        }
    }

    PointerPte = MiReserveAlignedSystemPtes (NumberOfPtes,
                                             SystemPtePoolType,
                                             0);

    if (PointerPte == NULL) {
        MiSystemPteAllocationFailed += 1;
    }

#if DBG
    if (PointerPte != NULL) {

        ASSERT (NumberOfPtes != 0);
        PointerPte += NumberOfPtes;

        do {
            PointerPte -= 1;
            ASSERT (PointerPte->u.Hard.Valid == 0);
            NumberOfPtes -= 1;
        } while (NumberOfPtes != 0);
    }
#endif

    return PointerPte;
}

VOID
MiFeedSysPtePool (
    IN ULONG Index
    )

/*++

Routine Description:

    This routine adds PTEs to the nonblocking queue lists.

Arguments:

    Index - Supplies the index for the nonblocking queue list to fill.

Return Value:

    None.

Environment:

    Kernel mode, internal to SysPtes.

--*/

{
    ULONG i;
    ULONG NumberOfPtes;
    PMMPTE PointerPte;

    if (MmTotalFreeSystemPtes[SystemPteSpace] < MM_MIN_SYSPTE_FREE) {
#if defined (_X86_)
        MiRecoverExtraPtes (PTE_PER_PAGE);
#endif
        return;
    }

    //
    // Check the shared user data PTE to ensure that we have reached Mm
    // phase 1 initialization - this means that the Ex worker package is
    // ready to go (it is initialized in phase 1 just before Mm), so that
    // MiReleaseSystemPtes can queue SLIST expansion if needed.
    //

    if (MmSharedUserDataPte == NULL) {
        return;
    }

    NumberOfPtes = MmSysPteIndex[Index];

    for (i = 0; i < 10 ; i += 1) {

        PointerPte = MiReserveAlignedSystemPtes (NumberOfPtes,
                                                 SystemPteSpace,
                                                 0);
        if (PointerPte == NULL) {
            return;
        }

        MiReleaseSystemPtes (PointerPte, NumberOfPtes, SystemPteSpace);
    }

    return;
}


PMMPTE
MiReserveAlignedSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType,
    IN ULONG Alignment
    )

/*++

Routine Description:

    This function locates the specified number of unused PTEs to locate
    within the non paged portion of system space.

Arguments:

    NumberOfPtes - Supplies the number of PTEs to locate.

    SystemPtePoolType - Supplies the PTE type of the pool to expand, one of
                        SystemPteSpace or NonPagedPoolExpansion.

    Alignment - Supplies the virtual address alignment for the address
                the returned PTE maps. For example, if the value is 64K,
                the returned PTE will map an address on a 64K boundary.
                An alignment of zero means to align on a page boundary.

Return Value:

    Returns the address of the first PTE located.
    NULL if no system PTEs can be located.

Environment:

    Kernel mode, DISPATCH_LEVEL or below.

--*/

{
    PMMPTE PointerPte;
    PMMPTE PointerFollowingPte;
    PMMPTE Previous;
    ULONG_PTR SizeInSet;
    KIRQL OldIrql;
    ULONG MaskSize;
    ULONG NumberOfRequiredPtes;
    ULONG OffsetSum;
    ULONG PtesToObtainAlignment;
    PMMPTE NextSetPointer;
    ULONG_PTR LeftInSet;
    ULONG_PTR PteOffset;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];
    PVOID BaseAddress;
    ULONG TimeStamp;
    ULONG j;

    MaskSize = (Alignment - 1) >> (PAGE_SHIFT - PTE_SHIFT);

    OffsetSum = (Alignment >> (PAGE_SHIFT - PTE_SHIFT));

#if defined (_X86_)
restart:
#endif

    //
    // The nonpaged PTE pool uses the invalid PTEs to define the pool
    // structure.   A global pointer points to the first free set
    // in the list, each free set contains the number free and a pointer
    // to the next free set.  The free sets are kept in an ordered list
    // such that the pointer to the next free set is always greater
    // than the address of the current free set.
    //
    // As to not limit the size of this pool, two PTEs are used
    // to define a free region.  If the region is a single PTE, the
    // prototype field within the PTE is set indicating the set
    // consists of a single PTE.
    //
    // The page frame number field is used to define the next set
    // and the number free.  The two flavors are:
    //
    //                           o          V
    //                           n          l
    //                           e          d
    //  +-----------------------+-+----------+
    //  |  next set             |0|0        0|
    //  +-----------------------+-+----------+
    //  |  number in this set   |0|0        0|
    //  +-----------------------+-+----------+
    //
    //
    //  +-----------------------+-+----------+
    //  |  next set             |1|0        0|
    //  +-----------------------+-+----------+
    //  ...
    //

    //
    // Acquire the system space lock to synchronize access.
    //

    MiLockSystemSpace (OldIrql);

    PointerPte = &MmFirstFreeSystemPte[SystemPtePoolType];

    if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {

        //
        // End of list and none found.
        //

        MiUnlockSystemSpace (OldIrql);
#if defined (_X86_)
        if ((SystemPtePoolType == SystemPteSpace) &&
            (MiRecoverExtraPtes (NumberOfPtes) == TRUE)) {

            goto restart;
        }
#endif
        MmPteFailures[SystemPtePoolType] += 1;
        return NULL;
    }

    Previous = PointerPte;

    PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;

    if (Alignment <= PAGE_SIZE) {

        //
        // Don't deal with alignment issues.
        //

        while (TRUE) {

            if (PointerPte->u.List.OneEntry) {

                if (NumberOfPtes == 1) {
                    goto ExactFit;
                }

                goto NextEntry;
            }

            PointerFollowingPte = PointerPte + 1;
            SizeInSet = (ULONG_PTR) PointerFollowingPte->u.List.NextEntry;

            if (NumberOfPtes < SizeInSet) {

                //
                // Get the PTEs from this set and reduce the size of the
                // set.  Note that the size of the current set cannot be 1.
                //

                if ((SizeInSet - NumberOfPtes) == 1) {

                    //
                    // Collapse to the single PTE format.
                    //

                    PointerPte->u.List.OneEntry = 1;
                }
                else {

                    //
                    // Get the required PTEs from the end of the set.
                    //

                    PointerFollowingPte->u.List.NextEntry = SizeInSet - NumberOfPtes;
                }

                MmTotalFreeSystemPtes[SystemPtePoolType] -= NumberOfPtes;
#if DBG
                if (MmDebug & MM_DBG_SYS_PTES) {
                    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                             MiCountFreeSystemPtes (SystemPtePoolType));
                }
#endif

                //
                // Release the lock and flush the TB.
                //

                MiUnlockSystemSpace (OldIrql);

                PointerPte += (SizeInSet - NumberOfPtes);
                break;
            }

            if (NumberOfPtes == SizeInSet) {

ExactFit:

                //
                // Satisfy the request with this complete set and change
                // the list to reflect the fact that this set is gone.
                //

                Previous->u.List.NextEntry = PointerPte->u.List.NextEntry;

                MmTotalFreeSystemPtes[SystemPtePoolType] -= NumberOfPtes;
#if DBG
                if (MmDebug & MM_DBG_SYS_PTES) {
                    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                             MiCountFreeSystemPtes (SystemPtePoolType));
                }
#endif

                //
                // Release the lock and flush the TB.
                //

                MiUnlockSystemSpace (OldIrql);
                break;
            }

NextEntry:

            //
            // Point to the next set and try again.
            //

            if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {

                //
                // End of list and none found.
                //

                MiUnlockSystemSpace (OldIrql);
#if defined (_X86_)
                if ((SystemPtePoolType == SystemPteSpace) &&
                    (MiRecoverExtraPtes (NumberOfPtes) == TRUE)) {

                    goto restart;
                }
#endif
                MmPteFailures[SystemPtePoolType] += 1;
                return NULL;
            }
            Previous = PointerPte;
            PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
            ASSERT (PointerPte > Previous);
        }
    }
    else {

        //
        // Deal with the alignment issues.
        //

        while (TRUE) {

            if (PointerPte->u.List.OneEntry) {

                //
                // Initializing PointerFollowingPte is not needed for
                // correctness, but without it the compiler cannot compile
                // this code W4 to check for use of uninitialized variables.
                //

                PointerFollowingPte = NULL;
                SizeInSet = 1;
            }
            else {
                PointerFollowingPte = PointerPte + 1;
                SizeInSet = (ULONG_PTR) PointerFollowingPte->u.List.NextEntry;
            }

            PtesToObtainAlignment = (ULONG)
                (((OffsetSum - ((ULONG_PTR)PointerPte & MaskSize)) & MaskSize) >>
                    PTE_SHIFT);

            NumberOfRequiredPtes = NumberOfPtes + PtesToObtainAlignment;

            if (NumberOfRequiredPtes < SizeInSet) {

                //
                // Get the PTEs from this set and reduce the size of the
                // set.  Note that the size of the current set cannot be 1.
                //
                // This current block will be slit into 2 blocks if
                // the PointerPte does not match the alignment.
                //

                //
                // Check to see if the first PTE is on the proper
                // alignment, if so, eliminate this block.
                //

                LeftInSet = SizeInSet - NumberOfRequiredPtes;

                //
                // Set up the new set at the end of this block.
                //

                NextSetPointer = PointerPte + NumberOfRequiredPtes;
                NextSetPointer->u.List.NextEntry =
                                       PointerPte->u.List.NextEntry;

                PteOffset = (ULONG_PTR)(NextSetPointer - MmSystemPteBase);

                if (PtesToObtainAlignment == 0) {

                    Previous->u.List.NextEntry += NumberOfRequiredPtes;

                }
                else {

                    //
                    // Point to the new set at the end of the block
                    // we are giving away.
                    //

                    PointerPte->u.List.NextEntry = PteOffset;

                    //
                    // Update the size of the current set.
                    //

                    if (PtesToObtainAlignment == 1) {

                        //
                        // Collapse to the single PTE format.
                        //

                        PointerPte->u.List.OneEntry = 1;

                    }
                    else {

                        //
                        // Set the set size in the next PTE.
                        //

                        PointerFollowingPte->u.List.NextEntry =
                                                        PtesToObtainAlignment;
                    }
                }

                //
                // Set up the new set at the end of the block.
                //

                if (LeftInSet == 1) {
                    NextSetPointer->u.List.OneEntry = 1;
                }
                else {
                    NextSetPointer->u.List.OneEntry = 0;
                    NextSetPointer += 1;
                    NextSetPointer->u.List.NextEntry = LeftInSet;
                }
                MmTotalFreeSystemPtes[SystemPtePoolType] -= NumberOfPtes;
#if DBG
                if (MmDebug & MM_DBG_SYS_PTES) {
                    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                             MiCountFreeSystemPtes (SystemPtePoolType));
                }
#endif

                //
                // Release the lock and flush the TB.
                //

                MiUnlockSystemSpace (OldIrql);

                PointerPte += PtesToObtainAlignment;
                break;
            }

            if (NumberOfRequiredPtes == SizeInSet) {

                //
                // Satisfy the request with this complete set and change
                // the list to reflect the fact that this set is gone.
                //

                if (PtesToObtainAlignment == 0) {

                    //
                    // This block exactly satisfies the request.
                    //

                    Previous->u.List.NextEntry =
                                            PointerPte->u.List.NextEntry;

                }
                else {

                    //
                    // A portion at the start of this block remains.
                    //

                    if (PtesToObtainAlignment == 1) {

                        //
                        // Collapse to the single PTE format.
                        //

                        PointerPte->u.List.OneEntry = 1;

                    }
                    else {
                      PointerFollowingPte->u.List.NextEntry =
                                                        PtesToObtainAlignment;

                    }
                }

                MmTotalFreeSystemPtes[SystemPtePoolType] -= NumberOfPtes;
#if DBG
                if (MmDebug & MM_DBG_SYS_PTES) {
                    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                             MiCountFreeSystemPtes (SystemPtePoolType));
                }
#endif

                //
                // Release the lock and flush the TB.
                //

                MiUnlockSystemSpace (OldIrql);

                PointerPte += PtesToObtainAlignment;
                break;
            }

            //
            // Point to the next set and try again.
            //

            if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {

                //
                // End of list and none found.
                //

                MiUnlockSystemSpace (OldIrql);
#if defined (_X86_)
                if ((SystemPtePoolType == SystemPteSpace) &&
                    (MiRecoverExtraPtes (NumberOfPtes) == TRUE)) {

                    goto restart;
                }
#endif
                MmPteFailures[SystemPtePoolType] += 1;
                return NULL;
            }
            Previous = PointerPte;
            PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
            ASSERT (PointerPte > Previous);
        }
    }

    //
    // If needed, flush the TB.
    //
    // Capture the global timestamp into a local so the compare will use
    // the same value throughout.
    //

    TimeStamp = *(volatile PULONG) &MiSysPteTimeStamp[SystemPtePoolType];

    if (MiCompareTbFlushTimeStamp (TimeStamp, SYSPTES_FLUSH_COUNTER_MASK) == FALSE) {
        //
        // The TB has been flushed since the last release so nothing
        // needs to be done here.
        //

        NOTHING;
    }
    else if (NumberOfPtes >= MM_MAXIMUM_FLUSH_COUNT) {
        MI_FLUSH_ENTIRE_TB (0xD);
    }
    else {

        BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);

        if (NumberOfPtes == 1) {
            MI_FLUSH_SINGLE_TB (BaseAddress, TRUE);
        }
        else {
    
            ASSERT (NumberOfPtes < MM_MAXIMUM_FLUSH_COUNT);
            Previous = PointerPte;
    
            for (j = 0; j < NumberOfPtes; j += 1) {
    
                VaFlushList[j] = BaseAddress;
    
                //
                // PTEs being freed better be invalid.
                //
    
                ASSERT (Previous->u.Hard.Valid == 0);
    
                Previous->u.Long = 0;
                BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
                Previous += 1;
            }
    
            MI_FLUSH_MULTIPLE_TB (NumberOfPtes, &VaFlushList[0], TRUE);
        }
    }

    if ((MmTrackPtes & 0x2) &&
        (SystemPtePoolType == SystemPteSpace)) {

        MiCheckPteReserve (PointerPte, NumberOfPtes);
    }

    return PointerPte;
}

VOID
MiIssueNoPtesBugcheck (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

/*++

Routine Description:

    This function bugchecks when no PTEs are left.

Arguments:

    SystemPtePoolType - Supplies the PTE type of the pool that is empty.

    NumberOfPtes - Supplies the number of PTEs requested that failed.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    PVOID HighConsumer;
    ULONG_PTR HighPteUse;

    if (SystemPtePoolType == SystemPteSpace) {

        HighConsumer = MiGetHighestPteConsumer (&HighPteUse);

        if (HighConsumer != NULL) {
            KeBugCheckEx (DRIVER_USED_EXCESSIVE_PTES,
                          (ULONG_PTR)HighConsumer,
                          HighPteUse,
                          MmTotalFreeSystemPtes[SystemPtePoolType],
                          MmNumberOfSystemPtes);
        }
    }

    KeBugCheckEx (NO_MORE_SYSTEM_PTES,
                  (ULONG_PTR)SystemPtePoolType,
                  NumberOfPtes,
                  MmTotalFreeSystemPtes[SystemPtePoolType],
                  MmNumberOfSystemPtes);
}

VOID
MiPteSListExpansionWorker (
    IN PVOID Context
    )

/*++

Routine Description:

    This routine is the worker routine to add additional SLISTs for the
    system PTE nonblocking queues.

Arguments:

    Context - Supplies a pointer to the MM_PTE_SLIST_EXPANSION_WORK_CONTEXT.

Return Value:

    None.

Environment:

    Kernel mode, PASSIVE_LEVEL.

--*/

{
    ULONG i;
    ULONG SListEntries;
    PPTE_SLIST SListChunks;
    PMM_PTE_SLIST_EXPANSION_WORK_CONTEXT Expansion;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    Expansion = (PMM_PTE_SLIST_EXPANSION_WORK_CONTEXT) Context;

    ASSERT (Expansion->Active == 1);

    if (Expansion->SListPages < MI_MAXIMUM_SLIST_PTE_PAGES) {

        //
        // Allocate another page of SLIST entries for the
        // nonblocking PTE queues.
        //

        SListChunks = (PPTE_SLIST) ExAllocatePoolWithTag (NonPagedPool,
                                                          PAGE_SIZE,
                                                          'PSmM');

        if (SListChunks != NULL) {

            //
            // Carve up the pages into SLIST entries (with no pool headers).
            //

            Expansion->SListPages += 1;

            SListEntries = PAGE_SIZE / sizeof (PTE_SLIST);

            for (i = 0; i < SListEntries; i += 1) {
                InterlockedPushEntrySList (&MiSystemPteSListHead,
                                           (PSLIST_ENTRY)SListChunks);
                SListChunks += 1;
            }
        }
    }

    ASSERT (Expansion->Active == 1);
    InterlockedExchange (&Expansion->Active, 0);
}

PVOID
MmMapLockedPagesSpecifyCache (
     __in PMDL MemoryDescriptorList,
     __in KPROCESSOR_MODE AccessMode,
     __in MEMORY_CACHING_TYPE CacheType,
     __in_opt PVOID RequestedAddress,
     __in ULONG BugCheckOnFailure,
     __in MM_PAGE_PRIORITY Priority
     )

/*++

Routine Description:

    This function maps physical pages described by a memory descriptor
    list into the system virtual address space or the user portion of
    the virtual address space.

Arguments:

    MemoryDescriptorList - Supplies a valid Memory Descriptor List which has
                           been updated by MmProbeAndLockPages.

    AccessMode - Supplies an indicator of where to map the pages;
                 KernelMode indicates that the pages should be mapped in the
                 system part of the address space, UserMode indicates the
                 pages should be mapped in the user part of the address space.

    CacheType - Supplies the type of cache mapping to use for the MDL.
                MmCached indicates "normal" kernel or user mappings.

    RequestedAddress - Supplies the base user address of the view.

                       This is only treated as an address if the AccessMode
                       is UserMode.  If the initial value of this argument
                       is not NULL, then the view will be allocated starting
                       at the specified virtual address rounded down to the
                       next 64kb address boundary. If the initial value of
                       this argument is NULL, then the operating system
                       will determine where to allocate the view.

                       If the AccessMode is KernelMode, then this argument is
                       treated as a bit field of attributes.

    BugCheckOnFailure - Supplies whether to bugcheck if the mapping cannot be
                        obtained.  This flag is only checked if the MDL's
                        MDL_MAPPING_CAN_FAIL is zero, which implies that the
                        default MDL behavior is to bugcheck.  This flag then
                        provides an additional avenue to avoid the bugcheck.
                        Done this way in order to provide WDM compatibility.

    Priority - Supplies an indication as to how important it is that this
               request succeed under low available PTE conditions.

Return Value:

    Returns the base address where the pages are mapped.  The base address
    has the same offset as the virtual address in the MDL.

    This routine will raise an exception if the processor mode is USER_MODE
    and quota limits or VM limits are exceeded.

Environment:

    Kernel mode.  DISPATCH_LEVEL or below if access mode is KernelMode,
                  APC_LEVEL or below if access mode is UserMode.

--*/

{
    ULONG i;
    ULONG TimeStamp;
    PTE_QUEUE_POINTER Value;
    ULONG Index;
    KIRQL OldIrql;
    CSHORT IoMapping;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PMMPTE PointerPte;
    PVOID BaseVa;
    MMPTE TempPte;
    MMPTE DefaultPte;
    PVOID StartingVa;
    PVOID CallingAddress;
    PVOID CallersCaller;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn2;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    //
    // If this assert fires, the MiPlatformCacheAttributes array
    // initialization needs to be checked.
    //

    ASSERT (MmMaximumCacheType == 6);

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    ASSERT (MemoryDescriptorList->ByteCount != 0);

    if (AccessMode == KernelMode) {

        Page = (PPFN_NUMBER) (MemoryDescriptorList + 1);
        NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                               MemoryDescriptorList->ByteCount);

        LastPage = Page + NumberOfPages;

        //
        // Map the pages into the system part of the address space as
        // kernel read/write.
        //

        ASSERT ((MemoryDescriptorList->MdlFlags & (
                        MDL_MAPPED_TO_SYSTEM_VA |
                        MDL_SOURCE_IS_NONPAGED_POOL |
                        MDL_PARTIAL_HAS_BEEN_MAPPED)) == 0);

        ASSERT ((MemoryDescriptorList->MdlFlags & (
                        MDL_PAGES_LOCKED |
                        MDL_PARTIAL)) != 0);

        //
        // Make sure there are enough PTEs of the requested size.
        // Try to ensure available PTEs inline when we're rich.
        // Otherwise go the long way.
        //

        if ((Priority != HighPagePriority) &&
            ((LONG)(NumberOfPages) > (LONG)MmTotalFreeSystemPtes[SystemPteSpace] - 2048) &&
            (MiGetSystemPteAvailability ((ULONG)NumberOfPages, Priority) == FALSE) && 
            ((PsGetCurrentThread()->MemoryMaker == 0) || KeIsExecutingDpc ())) {
            return NULL;
        }

        IoMapping = MemoryDescriptorList->MdlFlags & MDL_IO_SPACE;

        CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

        //
        // If a noncachable mapping is requested, none of the pages in the
        // requested MDL can reside in a large page.  Otherwise we would be
        // creating an incoherent overlapping TB entry as the same physical
        // page would be mapped by 2 different TB entries with different
        // cache attributes.
        //

        if (CacheAttribute != MiCached) {

            LOCK_PFN2 (OldIrql);

            do {

#pragma prefast(suppress: 2000, "SAL 1.2 needed for accurate MDL struct annotation.")
                if (*Page == MM_EMPTY_LIST) {
                    break;
                }

                PageFrameIndex = *Page;

                if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {

                    UNLOCK_PFN2 (OldIrql);

                    MiNonCachedCollisions += 1;

                    if (((MemoryDescriptorList->MdlFlags & MDL_MAPPING_CAN_FAIL) == 0) && (BugCheckOnFailure)) {

                        KeBugCheckEx (MEMORY_MANAGEMENT,
                                      0x1000,
                                      (ULONG_PTR)MemoryDescriptorList,
                                      (ULONG_PTR)PageFrameIndex,
                                      (ULONG_PTR)CacheAttribute);
                    }
                    return NULL;
                }

                Page += 1;
            } while (Page < LastPage);

            UNLOCK_PFN2 (OldIrql);

            Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
        }
   
        PointerPte = NULL;

        if (NumberOfPages <= MM_PTE_TABLE_LIMIT) {

            Index = MmSysPteTables [NumberOfPages];
            ASSERT (NumberOfPages <= MmSysPteIndex[Index]);

            if (ExRemoveHeadNBQueue (MiSystemPteNBHead[Index], (PULONG64)&Value) == TRUE) {
                InterlockedDecrement ((PLONG)&MmSysPteListBySizeCount[Index]);

                PointerPte = UnpackPTEPointer (&Value);

                ASSERT (PointerPte >= MmSystemPtesStart[SystemPteSpace]);
                ASSERT (PointerPte <= MmSystemPtesEnd[SystemPteSpace]);

                TimeStamp = UnpackPTETimeStamp (&Value);

                ASSERT (PointerPte >= MmSystemPtesStart[SystemPteSpace]);
                ASSERT (PointerPte <= MmSystemPtesEnd[SystemPteSpace]);

#if DBG
                PointerPte->u.List.NextEntry = 0xABCDE;

                for (i = 0; i < MmSysPteIndex[Index]; i += 1) {
                    ASSERT (PointerPte->u.Hard.Valid == 0);
                    PointerPte += 1;
                }

                PointerPte -= i;
#endif

                i = MmSysPteMinimumFree[Index];

                if (MmSysPteListBySizeCount[Index] < i) {
                    MiFeedSysPtePool (Index);
                }

                //
                // The last thing is to check whether the TB needs flushing.
                //

                if (MiCompareTbFlushTimeStamp (TimeStamp, SYSPTES_FLUSH_COUNTER_MASK)) {
                    MI_FLUSH_ENTIRE_TB (0xE);
                }

                if (MmTrackPtes & 0x2) {
                    MiCheckPteReserve (PointerPte, MmSysPteIndex[Index]);
                }
            }
            else {

                //
                // Fall through and go the long way to satisfy the PTE request.
                //

                NumberOfPages = MmSysPteIndex [Index];
            }
        }

        if (PointerPte == NULL) {

            PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages,
                                              SystemPteSpace);

            if (PointerPte == NULL) {

                if (((MemoryDescriptorList->MdlFlags & MDL_MAPPING_CAN_FAIL) == 0) &&
                    (BugCheckOnFailure)) {

                    MiIssueNoPtesBugcheck ((ULONG)NumberOfPages, SystemPteSpace);
                }

                //
                // Not enough system PTES are available.
                //

                return NULL;
            }
        }

        BaseVa = (PVOID)((PCHAR)MiGetVirtualAddressMappedByPte (PointerPte) +
                                MemoryDescriptorList->ByteOffset);

        TempPte = ValidKernelPte;

        MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

        if (CacheAttribute != MiCached) {

            switch (CacheAttribute) {

                case MiNonCached:
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }

            MI_PREPARE_FOR_NONCACHED (CacheAttribute);
        }

        OldIrql = HIGH_LEVEL;
        DefaultPte = TempPte;

        do {

            if (*Page == MM_EMPTY_LIST) {
                break;
            }
            ASSERT (PointerPte->u.Hard.Valid == 0);
    
            if ((IoMapping == 0) || (MI_IS_PFN (*Page))) {

                Pfn2 = MI_PFN_ELEMENT (*Page);
                ASSERT (Pfn2->u3.e2.ReferenceCount != 0);

                if (CacheAttribute == (MI_PFN_CACHE_ATTRIBUTE)Pfn2->u3.e1.CacheAttribute) {
                    TempPte.u.Hard.PageFrameNumber = *Page;
                    MI_WRITE_VALID_PTE (PointerPte, TempPte);
                }
                else {

                    TempPte = ValidKernelPte;

                    switch (Pfn2->u3.e1.CacheAttribute) {
    
                        case MiCached:
    
                            //
                            // The caller asked for a noncached or
                            // writecombined mapping, but the page is
                            // already mapped cached by someone else.
                            // Override the caller's request in order
                            // to keep the TB page attribute coherent.
                            //
    
                            MiCacheOverride[0] += 1;
                            break;
    
                        case MiNonCached:
    
                            //
                            // The caller asked for a cached or
                            // writecombined mapping, but the page is
                            // already mapped noncached by someone else.
                            // Override the caller's request in order to
                            // keep the TB page attribute coherent.
                            //

                            MiCacheOverride[1] += 1;
                            MI_DISABLE_CACHING (TempPte);
                            break;
    
                        case MiWriteCombined:
    
                            //
                            // The caller asked for a cached or noncached
                            // mapping, but the page is already mapped
                            // writecombined by someone else.  Override the
                            // caller's request in order to keep the TB page
                            // attribute coherent.
                            //

                            MiCacheOverride[2] += 1;
                            MI_SET_PTE_WRITE_COMBINE (TempPte);
                            break;
    
                        case MiNotMapped:
    
                            //
                            // This better be for a page allocated with
                            // MmAllocatePagesForMdl.  Otherwise it might be a
                            // page on the freelist which could subsequently be
                            // given out with a different attribute !
                            //
    
                            ASSERT ((Pfn2->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                                    (Pfn2->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));
    
                            if (OldIrql == HIGH_LEVEL) {
                                LOCK_PFN2 (OldIrql);
                            }
    
                            switch (CacheAttribute) {
    
                                case MiCached:
                                    Pfn2->u3.e1.CacheAttribute = MiCached;
                                    break;
    
                                case MiNonCached:
                                    Pfn2->u3.e1.CacheAttribute = MiNonCached;
                                    MI_DISABLE_CACHING (TempPte);
                                    break;
    
                                case MiWriteCombined:
                                    Pfn2->u3.e1.CacheAttribute = MiWriteCombined;
                                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                                    break;
    
                                default:
                                    ASSERT (FALSE);
                                    break;
                            }
                            break;
    
                        default:
                            ASSERT (FALSE);
                            break;
                    }

                    TempPte.u.Hard.PageFrameNumber = *Page;
                    MI_WRITE_VALID_PTE (PointerPte, TempPte);

                    //
                    // We had to override the requested cache type for the
                    // current page, so reset the PTE for the next page back
                    // to the original entry requested cache type.
                    //

                    TempPte = DefaultPte;
                }
            }
            else {
                TempPte.u.Hard.PageFrameNumber = *Page;
                MI_WRITE_VALID_PTE (PointerPte, TempPte);
            }

            Page += 1;
            PointerPte += 1;
        } while (Page < LastPage);
    
        if (OldIrql != HIGH_LEVEL) {
            UNLOCK_PFN2 (OldIrql);
        }

        ASSERT ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);
        MemoryDescriptorList->MappedSystemVa = BaseVa;

        MemoryDescriptorList->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

        if (MmTrackPtes & 0x1) {

            RtlGetCallersAddress (&CallingAddress, &CallersCaller);

            MiInsertPteTracker (MemoryDescriptorList,
                                0,
                                IoMapping,
                                CacheAttribute,
                                CallingAddress,
                                CallersCaller);
        }

        if ((MemoryDescriptorList->MdlFlags & MDL_PARTIAL) != 0) {
            MemoryDescriptorList->MdlFlags |= MDL_PARTIAL_HAS_BEEN_MAPPED;
        }

        return BaseVa;
    }

    return MiMapLockedPagesInUserSpace (MemoryDescriptorList,
                                        StartingVa,
                                        CacheType,
                                        RequestedAddress);
}

VOID
MmUnmapLockedPages (
    __in PVOID BaseAddress,
    __in PMDL MemoryDescriptorList
    )

/*++

Routine Description:

    This routine unmaps locked pages which were previously mapped via
    an MmMapLockedPages call.

Arguments:

    BaseAddress - Supplies the base address where the pages were previously
                  mapped.

    MemoryDescriptorList - Supplies a valid Memory Descriptor List which has
                           been updated by MmProbeAndLockPages.

Return Value:

    None.

Environment:

    Kernel mode.  DISPATCH_LEVEL or below if base address is within
    system space; APC_LEVEL or below if base address is user space.

    Note that in some instances the PFN lock is held by the caller.

--*/

{
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerPte;
    PVOID StartingVa;
    PPFN_NUMBER Page;
    ULONG TimeStamp;
    PTE_QUEUE_POINTER Value;
    ULONG Index;
    PFN_NUMBER i;
#if DBG
    PMMPFN Pfn3;
#endif

    ASSERT (MemoryDescriptorList->ByteCount != 0);

    if (BaseAddress > MM_HIGHEST_USER_ADDRESS) {

        ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PARENT_MAPPED_SYSTEM_VA) == 0);
        StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                        MemoryDescriptorList->ByteOffset);

        NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                               MemoryDescriptorList->ByteCount);

        ASSERT (NumberOfPages != 0);

        ASSERT (MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA);

        ASSERT (!MI_IS_PHYSICAL_ADDRESS (BaseAddress));

        PointerPte = MiGetPteAddress (BaseAddress);

        //
        // Check to make sure the PTE address is within bounds.
        //

        ASSERT (PointerPte >= MmSystemPtesStart[SystemPteSpace]);
        ASSERT (PointerPte <= MmSystemPtesEnd[SystemPteSpace]);

#if DBG
        i = NumberOfPages;
        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        while (i != 0) {
            ASSERT (PointerPte->u.Hard.Valid == 1);
            ASSERT (*Page == MI_GET_PAGE_FRAME_FROM_PTE (PointerPte));
            if (MI_IS_PFN (*Page)) {
                Pfn3 = MI_PFN_ELEMENT (*Page);
                ASSERT (Pfn3->u3.e2.ReferenceCount != 0);
            }

            Page += 1;
            PointerPte += 1;
            i -= 1;
        }
        PointerPte -= NumberOfPages;
#endif

        if (MemoryDescriptorList->MdlFlags & MDL_FREE_EXTRA_PTES) {
            Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
            Page += NumberOfPages;
            ASSERT (*Page <= MiCurrentAdvancedPages);
            NumberOfPages += *Page;
            PointerPte -= *Page;
            ASSERT (PointerPte >= MmSystemPtesStart[SystemPteSpace]);
            ASSERT (PointerPte <= MmSystemPtesEnd[SystemPteSpace]);
            BaseAddress = (PVOID)((PCHAR)BaseAddress - ((*Page) << PAGE_SHIFT));
#if DBG
            InterlockedExchangeAddSizeT (&MiCurrentAdvancedPages, 0 - *Page);
            MiAdvancesFreed += *Page;
#endif
        }

        if (MmTrackPtes != 0) {
            if (MmTrackPtes & 0x1) {
                MiRemovePteTracker (MemoryDescriptorList,
                                    BaseAddress,
                                    NumberOfPages);
            }
            if (MmTrackPtes & 0x2) {
                MiCheckPteRelease (PointerPte, (ULONG) NumberOfPages);
            }
        }

        MemoryDescriptorList->MdlFlags &= ~(MDL_MAPPED_TO_SYSTEM_VA |
                                            MDL_PARTIAL_HAS_BEEN_MAPPED |
                                            MDL_FREE_EXTRA_PTES);

        //
        // If it's a small request (most are), try to finish it inline.
        //

        if (NumberOfPages <= MM_PTE_TABLE_LIMIT) {
    
            Index = MmSysPteTables [NumberOfPages];
    
            ASSERT (NumberOfPages <= MmSysPteIndex [Index]);
    
            if (MmTotalFreeSystemPtes[SystemPteSpace] >= MM_MIN_SYSPTE_FREE) {
    
                //
                // Add to the pool if the size is less than 15 + the minimum.
                //
    
                i = MmSysPteMinimumFree[Index];
                if (MmTotalFreeSystemPtes[SystemPteSpace] >= MM_MAX_SYSPTE_FREE) {
    
                    //
                    // Lots of free PTEs, quadruple the limit.
                    //
    
                    i = i * 4;
                }
                i += 15;

                if (MmSysPteListBySizeCount[Index] <= i) {

                    //
                    // Zero PTEs, then encode the PTE pointer and the TB flush
                    // counter into Value.
                    //

                    MiZeroMemoryPte (PointerPte, NumberOfPages);

                    TimeStamp = KeReadTbFlushTimeStamp ();
            
                    PackPTEValue (&Value, PointerPte, TimeStamp);
            
                    if (ExInsertTailNBQueue (MiSystemPteNBHead[Index], Value.Data) == TRUE) {
                        InterlockedIncrement ((PLONG)&MmSysPteListBySizeCount[Index]);
                        return;
                    }
                }
            }
        }

        if (MmTrackPtes & 0x2) {

            //
            // This release has already been updated in the tracking bitmaps
            // so mark it so that MiReleaseSystemPtes doesn't attempt to do
            // it also.
            //

            PointerPte = (PMMPTE) ((ULONG_PTR)PointerPte | 0x1);
        }
        MiReleaseSystemPtes (PointerPte, (ULONG)NumberOfPages, SystemPteSpace);
    }
    else {
        MiUnmapLockedPagesInUserSpace (BaseAddress, MemoryDescriptorList);
    }

    return;
}

VOID
MiReleaseSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

/*++

Routine Description:

    This function releases the specified number of PTEs
    within the non paged portion of system space.

    Note that the PTEs must be invalid and the page frame number
    must have been set to zero.

Arguments:

    StartingPte - Supplies the address of the first PTE to release.

    NumberOfPtes - Supplies the number of PTEs to release.

    SystemPtePoolType - Supplies the PTE type of the pool to release PTEs to,
                        one of SystemPteSpace or NonPagedPoolExpansion.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    ULONG_PTR Size;
    ULONG i;
    ULONG_PTR PteOffset;
    PMMPTE PointerPte;
    PMMPTE PointerFollowingPte;
    PMMPTE NextPte;
    KIRQL OldIrql;
    ULONG Index;
    ULONG TimeStamp;
    PTE_QUEUE_POINTER Value;
    ULONG ExtensionInProgress;


    if ((MmTrackPtes & 0x2) && (SystemPtePoolType == SystemPteSpace)) {

        //
        // If the low bit is set, this range was never reserved and therefore
        // should not be validated during the release.
        //

        if ((ULONG_PTR)StartingPte & 0x1) {
            StartingPte = (PMMPTE) ((ULONG_PTR)StartingPte & ~0x1);
        }
        else {
            MiCheckPteRelease (StartingPte, NumberOfPtes);
        }
    }

    //
    // Check to make sure the PTE address is within bounds.
    //

    ASSERT (NumberOfPtes != 0);
    ASSERT (StartingPte >= MmSystemPtesStart[SystemPtePoolType]);
    ASSERT (StartingPte <= MmSystemPtesEnd[SystemPtePoolType]);

    //
    // Zero PTEs.
    //

    MiZeroMemoryPte (StartingPte, NumberOfPtes);

    if ((SystemPtePoolType == SystemPteSpace) &&
        (NumberOfPtes <= MM_PTE_TABLE_LIMIT)) {

        //
        // Encode the PTE pointer and the TB flush counter into Value.
        //

        TimeStamp = KeReadTbFlushTimeStamp ();

        PackPTEValue (&Value, StartingPte, TimeStamp);

        Index = MmSysPteTables [NumberOfPtes];

        ASSERT (NumberOfPtes <= MmSysPteIndex [Index]);

        if (MmTotalFreeSystemPtes[SystemPteSpace] >= MM_MIN_SYSPTE_FREE) {

            //
            // Add to the pool if the size is less than 15 + the minimum.
            //

            i = MmSysPteMinimumFree[Index];
            if (MmTotalFreeSystemPtes[SystemPteSpace] >= MM_MAX_SYSPTE_FREE) {

                //
                // Lots of free PTEs, quadruple the limit.
                //

                i = i * 4;
            }
            i += 15;
            if (MmSysPteListBySizeCount[Index] <= i) {

                if (ExInsertTailNBQueue (MiSystemPteNBHead[Index], Value.Data) == TRUE) {
                    InterlockedIncrement ((PLONG)&MmSysPteListBySizeCount[Index]);
                    return;
                }

                //
                // No lookasides are left for inserting this PTE allocation
                // into the nonblocking queues.  Queue an extension to a
                // worker thread so it can be done in a deadlock-free
                // manner.
                //

                if (MiPteSListExpand.SListPages < MI_MAXIMUM_SLIST_PTE_PAGES) {

                    //
                    // If an extension is not in progress then queue one now.
                    //

                    ExtensionInProgress = InterlockedCompareExchange (&MiPteSListExpand.Active, 1, 0);

                    if (ExtensionInProgress == 0) {

                        ExInitializeWorkItem (&MiPteSListExpand.WorkItem,
                                              MiPteSListExpansionWorker,
                                              (PVOID)&MiPteSListExpand);

                        ExQueueWorkItem (&MiPteSListExpand.WorkItem, CriticalWorkQueue);
                    }

                }
            }
        }

        //
        // The insert failed - our lookaside list must be empty or we are
        // low on PTEs systemwide or we already had plenty on our list and
        // didn't try to insert.  Fall through to queue this in the long way.
        //

        NumberOfPtes = MmSysPteIndex [Index];
    }

    PteOffset = (ULONG_PTR)(StartingPte - MmSystemPteBase);

    //
    // Acquire system space spin lock to synchronize access.
    //

    MiLockSystemSpace (OldIrql);

    //
    // Since the PTEs have already been zeroed, snap the TB flush time stamp
    // now (lock-free).
    //
    // Note this can only be set after the system space spin lock is acquired.
    // This prevents a first-thread in which becomes the last thread out from
    // setting a too-old timestamp which would cause PTEs inserted by other
    // threads in the gap to not get TB-flushed on reuse.
    //
    // This lock synchronization is necessary because one timestamp is
    // being used for all the system PTEs (instead of per-chunk) because
    // the different chunks get coalesced below.
    //

    MiSysPteTimeStamp[SystemPtePoolType] = KeReadTbFlushTimeStamp ();

    MmTotalFreeSystemPtes[SystemPtePoolType] += NumberOfPtes;

    PointerPte = &MmFirstFreeSystemPte[SystemPtePoolType];

    while (TRUE) {
        NextPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
        if (PteOffset < PointerPte->u.List.NextEntry) {

            //
            // Insert in the list at this point.  The
            // previous one should point to the new freed set and
            // the new freed set should point to the place
            // the previous set points to.
            //
            // Attempt to combine the clusters before we
            // insert.
            //
            // Locate the end of the current structure.
            //

            ASSERT (((StartingPte + NumberOfPtes) <= NextPte) ||
                    (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST));

            PointerFollowingPte = PointerPte + 1;
            if (PointerPte->u.List.OneEntry) {
                Size = 1;
            }
            else {
                Size = (ULONG_PTR) PointerFollowingPte->u.List.NextEntry;
            }
            if ((PointerPte + Size) == StartingPte) {

                //
                // We can combine the clusters.
                //

                NumberOfPtes += (ULONG)Size;
                PointerFollowingPte->u.List.NextEntry = NumberOfPtes;
                PointerPte->u.List.OneEntry = 0;

                //
                // Point the starting PTE to the beginning of
                // the new free set and try to combine with the
                // following free cluster.
                //

                StartingPte = PointerPte;

            }
            else {

                //
                // Can't combine with previous. Make this PTE the
                // start of a cluster.
                //

                //
                // Point this cluster to the next cluster.
                //

                StartingPte->u.List.NextEntry = PointerPte->u.List.NextEntry;

                //
                // Point the current cluster to this cluster.
                //

                PointerPte->u.List.NextEntry = PteOffset;

                //
                // Set the size of this cluster.
                //

                if (NumberOfPtes == 1) {
                    StartingPte->u.List.OneEntry = 1;

                }
                else {
                    StartingPte->u.List.OneEntry = 0;
                    PointerFollowingPte = StartingPte + 1;
                    PointerFollowingPte->u.List.NextEntry = NumberOfPtes;
                }
            }

            //
            // Attempt to combine the newly created cluster with
            // the following cluster.
            //

            if ((StartingPte + NumberOfPtes) == NextPte) {

                //
                // Combine with following cluster.
                //

                //
                // Set the next cluster to the value contained in the
                // cluster we are merging into this one.
                //

                StartingPte->u.List.NextEntry = NextPte->u.List.NextEntry;
                StartingPte->u.List.OneEntry = 0;
                PointerFollowingPte = StartingPte + 1;

                if (NextPte->u.List.OneEntry) {
                    Size = 1;
                }
                else {
                    NextPte += 1;
                    Size = (ULONG_PTR) NextPte->u.List.NextEntry;
                }
                PointerFollowingPte->u.List.NextEntry = NumberOfPtes + Size;
            }

#if DBG
            if (MmDebug & MM_DBG_SYS_PTES) {
                ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                         MiCountFreeSystemPtes (SystemPtePoolType));
            }
#endif
            MiUnlockSystemSpace (OldIrql);
            return;
        }

        //
        // Point to next freed cluster.
        //

        PointerPte = NextPte;
    }
}

VOID
MiReleaseSplitSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

/*++

Routine Description:

    This function releases the specified number of PTEs
    within the non paged portion of system space.

    Note that the PTEs must be invalid and the page frame number
    must have been set to zero.

    This portion is a split portion from a larger allocation so
    careful updating of the tracking bitmaps must be done here.

Arguments:

    StartingPte - Supplies the address of the first PTE to release.

    NumberOfPtes - Supplies the number of PTEs to release.

    SystemPtePoolType - Supplies the PTE type of the pool to release PTEs to,
                        one of SystemPteSpace or NonPagedPoolExpansion.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    ULONG i;
    ULONG StartBit;
    KIRQL OldIrql;
    PULONG StartBitMapBuffer;
    PULONG EndBitMapBuffer;
    PVOID VirtualAddress;
                
    //
    // Check to make sure the PTE address is within bounds.
    //

    ASSERT (NumberOfPtes != 0);
    ASSERT (StartingPte >= MmSystemPtesStart[SystemPtePoolType]);
    ASSERT (StartingPte <= MmSystemPtesEnd[SystemPtePoolType]);

    if ((MmTrackPtes & 0x2) && (SystemPtePoolType == SystemPteSpace)) {

        ASSERT (MmTrackPtes & 0x2);

        VirtualAddress = MiGetVirtualAddressMappedByPte (StartingPte);

        StartBit = (ULONG) (StartingPte - MiPteStart);

        ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

        //
        // Verify start and size of allocation using the tracking bitmaps.
        //

        StartBitMapBuffer = MiPteStartBitmap->Buffer;
        EndBitMapBuffer = MiPteEndBitmap->Buffer;

        //
        // All the start bits better be set.
        //

        for (i = StartBit; i < StartBit + NumberOfPtes; i += 1) {
            ASSERT (MI_CHECK_BIT (StartBitMapBuffer, i) == 1);
        }

        if (StartBit != 0) {

            if (RtlCheckBit (MiPteStartBitmap, StartBit - 1)) {

                if (!RtlCheckBit (MiPteEndBitmap, StartBit - 1)) {

                    //
                    // In the middle of an allocation - update the previous
                    // so it ends here.
                    //

                    MI_SET_BIT (EndBitMapBuffer, StartBit - 1);
                }
                else {

                    //
                    // The range being freed is the start of an allocation.
                    //
                }
            }
        }

        //
        // Unconditionally set the end bit (and clear any others) in case the
        // split chunk crosses multiple allocations.
        //

        MI_SET_BIT (EndBitMapBuffer, StartBit + NumberOfPtes - 1);

        ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);
    }

    MiReleaseSystemPtes (StartingPte, NumberOfPtes, SystemPteSpace);
}


VOID
MiInitializeSystemPtes (
    IN PMMPTE StartingPte,
    IN PFN_NUMBER NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

/*++

Routine Description:

    This routine initializes the system PTE pool.

Arguments:

    StartingPte - Supplies the address of the first PTE to put in the pool.

    NumberOfPtes - Supplies the number of PTEs to put in the pool.

    SystemPtePoolType - Supplies the PTE type of the pool to initialize, one of
                        SystemPteSpace or NonPagedPoolExpansion.

Return Value:

    none.

Environment:

    Kernel mode.

--*/

{
    ULONG i;
    ULONG TotalPtes;
    ULONG SListEntries;
    SIZE_T SListBytes;
    ULONG TotalChunks;
    PMMPTE PointerPte;
    PPTE_SLIST Chunk;
    PPTE_SLIST SListChunks;

    //
    // Set the base of the system PTE pool to this PTE.  This takes into
    // account that systems may have additional PTE pools below the PTE_BASE.
    //

    ASSERT64 (NumberOfPtes < _4gb);

    MmSystemPteBase = MI_PTE_BASE_FOR_LOWEST_KERNEL_ADDRESS;

    MmSystemPtesStart[SystemPtePoolType] = StartingPte;

    //
    // If there are no PTEs specified, then make a valid chain by indicating
    // that the list is empty.
    //

    if (NumberOfPtes == 0) {
        MmFirstFreeSystemPte[SystemPtePoolType].u.Long = 0;
        MmFirstFreeSystemPte[SystemPtePoolType].u.List.NextEntry =
                                                                MM_EMPTY_LIST;
        MmSystemPtesEnd[SystemPtePoolType] = StartingPte;
        return;
    }

    MmSystemPtesEnd[SystemPtePoolType] = StartingPte + NumberOfPtes - 1;

    //
    // Initialize the specified system PTE pool.
    //

    MiZeroMemoryPte (StartingPte, NumberOfPtes);

    //
    // The page frame field points to the next cluster.  As we only
    // have one cluster at initialization time, mark it as the last
    // cluster.
    //

    StartingPte->u.List.NextEntry = MM_EMPTY_LIST;

    MmFirstFreeSystemPte[SystemPtePoolType].u.Long = 0;
    MmFirstFreeSystemPte[SystemPtePoolType].u.List.NextEntry =
                                                StartingPte - MmSystemPteBase;

    //
    // If there is only one PTE in the pool, then mark it as a one entry
    // PTE. Otherwise, store the cluster size in the following PTE.
    //

    if (NumberOfPtes == 1) {
        StartingPte->u.List.OneEntry = TRUE;

    }
    else {
        StartingPte += 1;
        MI_WRITE_ZERO_PTE (StartingPte);
        StartingPte->u.List.NextEntry = NumberOfPtes;
    }

    //
    // Set the total number of free PTEs for the specified type.
    //

    MmTotalFreeSystemPtes[SystemPtePoolType] = (ULONG) NumberOfPtes;

    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                         MiCountFreeSystemPtes (SystemPtePoolType));

    if (SystemPtePoolType == SystemPteSpace) {

        ULONG Lists[MM_SYS_PTE_TABLES_MAX] = {
#if defined(_AMD64_)
                MM_PTE_LIST_1,
                MM_PTE_LIST_2,
                MM_PTE_LIST_4,
                MM_PTE_LIST_6,
                MM_PTE_LIST_8,
                MM_PTE_LIST_16,
                MM_PTE_LIST_19
#else
                MM_PTE_LIST_1,
                MM_PTE_LIST_2,
                MM_PTE_LIST_4,
                MM_PTE_LIST_8,
                MM_PTE_LIST_16
#endif
        };

        MiLowestSystemPteVirtualAddress = MiGetVirtualAddressMappedByPte (MmSystemPtesStart[SystemPteSpace]);

        MmTotalSystemPtes = (ULONG) NumberOfPtes;

        ASSERT (MiPteRangeIndex == 0);
        MiPteRanges[0].StartingVa = MiLowestSystemPteVirtualAddress;
        MiPteRanges[0].EndingVa = (PVOID) ((PCHAR)MiLowestSystemPteVirtualAddress + (NumberOfPtes << PAGE_SHIFT) - 1);
        MiPteRangeIndex = 1;

        TotalPtes = 0;
        TotalChunks = 0;

        MiAddPtesList.u.List.NextEntry = MM_EMPTY_LIST;

        KeInitializeSpinLock (&MiSystemPteSListHeadLock);
        InitializeSListHead (&MiSystemPteSListHead);

        for (i = 0; i < MM_SYS_PTE_TABLES_MAX ; i += 1) {
            TotalPtes += (Lists[i] * MmSysPteIndex[i]);
            TotalChunks += Lists[i];
        }

        SListBytes = TotalChunks * sizeof (PTE_SLIST);
        SListBytes = MI_ROUND_TO_SIZE (SListBytes, PAGE_SIZE);
        SListEntries = (ULONG)(SListBytes / sizeof (PTE_SLIST));

        SListChunks = (PPTE_SLIST) ExAllocatePoolWithTag (NonPagedPool,
                                                          SListBytes,
                                                          'PSmM');

        if (SListChunks == NULL) {
            MiIssueNoPtesBugcheck (TotalPtes, SystemPteSpace);
        }

        ASSERT (MiPteSListExpand.Active == FALSE);
        ASSERT (MiPteSListExpand.SListPages == 0);

        MiPteSListExpand.SListPages = (ULONG)(SListBytes / PAGE_SIZE);

        ASSERT (MiPteSListExpand.SListPages != 0);

        //
        // Carve up the pages into SLIST entries (with no pool headers).
        //

        Chunk = SListChunks;
        for (i = 0; i < SListEntries; i += 1) {
            InterlockedPushEntrySList (&MiSystemPteSListHead,
                                       (PSLIST_ENTRY)Chunk);
            Chunk += 1;
        }

        //
        // Now that the SLIST is populated, initialize the nonblocking heads.
        //

        for (i = 0; i < MM_SYS_PTE_TABLES_MAX ; i += 1) {
            MiSystemPteNBHead[i] = ExInitializeNBQueueHead (&MiSystemPteSListHead);

            if (MiSystemPteNBHead[i] == NULL) {
                MiIssueNoPtesBugcheck (TotalPtes, SystemPteSpace);
            }
        }

        if (MmTrackPtes & 0x2) {

            //
            // Allocate PTE mapping verification bitmaps.
            //

            ULONG BitmapSize;

#if defined(_WIN64)
            BitmapSize = (ULONG) MmNumberOfSystemPtes;
            MiPteStart = MmSystemPtesStart[SystemPteSpace];
#else
            MiPteStart = MiGetPteAddress (MmSystemRangeStart);
            BitmapSize = ((ULONG_PTR)PTE_TOP + 1) - (ULONG_PTR) MiPteStart;
            BitmapSize /= sizeof (MMPTE);
#endif

            MiCreateBitMap (&MiPteStartBitmap, BitmapSize, NonPagedPool);

            if (MiPteStartBitmap != NULL) {

                MiCreateBitMap (&MiPteEndBitmap, BitmapSize, NonPagedPool);

                if (MiPteEndBitmap == NULL) {
                    ExFreePool (MiPteStartBitmap);
                    MiPteStartBitmap = NULL;
                }
            }

            if ((MiPteStartBitmap != NULL) && (MiPteEndBitmap != NULL)) {
                RtlClearAllBits (MiPteStartBitmap);
                RtlClearAllBits (MiPteEndBitmap);
            }
            MmTrackPtes &= ~0x2;
        }

        //
        // Initialize the by size lists.
        //

        PointerPte = MiReserveSystemPtes (TotalPtes, SystemPteSpace);

        if (PointerPte == NULL) {
            MiIssueNoPtesBugcheck (TotalPtes, SystemPteSpace);
        }

        i = MM_SYS_PTE_TABLES_MAX;
        do {
            i -= 1;
            do {
                Lists[i] -= 1;
                MiReleaseSystemPtes (PointerPte,
                                     MmSysPteIndex[i],
                                     SystemPteSpace);
                PointerPte += MmSysPteIndex[i];
            } while (Lists[i] != 0);
        } while (i != 0);

        //
        // Turn this on after the multiple releases of the binned PTEs (that
        // came from a single reservation) above.
        //

        if (MiPteStartBitmap != NULL) {
            MmTrackPtes |= 0x2;
        }
    }

    return;
}

VOID
MiIncrementSystemPtes (
    IN ULONG  NumberOfPtes
    )

/*++

Routine Description:

    This routine increments the total number of PTEs.  This is done
    separately from actually adding the PTEs to the pool so that
    autoconfiguration can use the high number in advance of the PTEs
    actually getting added.

Arguments:

    NumberOfPtes - Supplies the number of PTEs to increment the total by.

Return Value:

    None.

Environment:

    Kernel mode.  Synchronization provided by the caller.

--*/

{
    MmTotalSystemPtes += NumberOfPtes;
}
VOID
MiAddSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG  NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

/*++

Routine Description:

    This routine adds newly created PTEs to the specified pool.

Arguments:

    StartingPte - Supplies the address of the first PTE to put in the pool.

    NumberOfPtes - Supplies the number of PTEs to put in the pool.

    SystemPtePoolType - Supplies the PTE type of the pool to expand, one of
                        SystemPteSpace or NonPagedPoolExpansion.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    PMMPTE EndingPte;

    ASSERT (SystemPtePoolType == SystemPteSpace);

    EndingPte = StartingPte + NumberOfPtes - 1;

    if (StartingPte < MmSystemPtesStart[SystemPtePoolType]) {
        MmSystemPtesStart[SystemPtePoolType] = StartingPte;
        MiLowestSystemPteVirtualAddress = MiGetVirtualAddressMappedByPte (StartingPte);
    }

    if (EndingPte > MmSystemPtesEnd[SystemPtePoolType]) {
        MmSystemPtesEnd[SystemPtePoolType] = EndingPte;
    }

    //
    // Set the low bit to signify this range was never reserved and therefore
    // should not be validated during the release.
    //

    if (MmTrackPtes & 0x2) {
        StartingPte = (PMMPTE) ((ULONG_PTR)StartingPte | 0x1);
    }

    MiReleaseSystemPtes (StartingPte, NumberOfPtes, SystemPtePoolType);
}


ULONG
MiGetSystemPteListCount (
    IN ULONG ListSize
    )

/*++

Routine Description:

    This routine returns the number of free entries of the list which
    covers the specified size.  The size must be less than or equal to the
    largest list index.

Arguments:

    ListSize - Supplies the number of PTEs needed.

Return Value:

    Number of free entries on the list which contains ListSize PTEs.

Environment:

    Kernel mode.

--*/

{
    ULONG Index;

    ASSERT (ListSize <= MM_PTE_TABLE_LIMIT);

    Index = MmSysPteTables [ListSize];

    return MmSysPteListBySizeCount[Index];
}


LOGICAL
MiGetSystemPteAvailability (
    IN ULONG NumberOfPtes,
    IN MM_PAGE_PRIORITY Priority
    )

/*++

Routine Description:

    This routine checks how many SystemPteSpace PTEs are available for the
    requested size.  If plenty are available then TRUE is returned.
    If we are reaching a low resource situation, then the request is evaluated
    based on the argument priority.

Arguments:

    NumberOfPtes - Supplies the number of PTEs needed.

    Priority - Supplies the priority of the request.

Return Value:

    TRUE if the caller should allocate the PTEs, FALSE if not.

Environment:

    Kernel mode.

--*/

{
    ULONG Index;
    ULONG FreePtes;
    ULONG FreeBinnedPtes;

    ASSERT (Priority != HighPagePriority);

    FreePtes = MmTotalFreeSystemPtes[SystemPteSpace];

    if (NumberOfPtes <= MM_PTE_TABLE_LIMIT) {
        Index = MmSysPteTables [NumberOfPtes];
        FreeBinnedPtes = MmSysPteListBySizeCount[Index];

        if (FreeBinnedPtes > MmSysPteMinimumFree[Index]) {
            return TRUE;
        }
        if (FreeBinnedPtes != 0) {
            if (Priority == NormalPagePriority) {
                if (FreeBinnedPtes > 1 || FreePtes > 512) {
                    return TRUE;
                }
#if defined (_X86_)
                if (MiRecoverExtraPtes (NumberOfPtes) == TRUE) {
                    return TRUE;
                }
#endif
                MmPteFailures[SystemPteSpace] += 1;
                return FALSE;
            }
            if (FreePtes > 2048) {
                return TRUE;
            }
#if defined (_X86_)
            if (MiRecoverExtraPtes (NumberOfPtes) == TRUE) {
                return TRUE;
            }
#endif
            MmPteFailures[SystemPteSpace] += 1;
            return FALSE;
        }
    }

    if (Priority == NormalPagePriority) {
        if ((LONG)NumberOfPtes < (LONG)FreePtes - 512) {
            return TRUE;
        }
#if defined (_X86_)
        if (MiRecoverExtraPtes (NumberOfPtes) == TRUE) {
            return TRUE;
        }
#endif
        MmPteFailures[SystemPteSpace] += 1;
        return FALSE;
    }

    if ((LONG)NumberOfPtes < (LONG)FreePtes - 2048) {
        return TRUE;
    }
#if defined (_X86_)
    if (MiRecoverExtraPtes (NumberOfPtes) == TRUE) {
        return TRUE;
    }
#endif
    MmPteFailures[SystemPteSpace] += 1;
    return FALSE;
}

VOID
MiCheckPteReserve (
    IN PMMPTE PointerPte,
    IN ULONG NumberOfPtes
    )

/*++

Routine Description:

    This function checks the reserve of the specified number of system
    space PTEs.

Arguments:

    StartingPte - Supplies the address of the first PTE to reserve.

    NumberOfPtes - Supplies the number of PTEs to reserve.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    ULONG i;
    KIRQL OldIrql;
    ULONG StartBit;
    PULONG StartBitMapBuffer;
    PULONG EndBitMapBuffer;
    PVOID VirtualAddress;
        
    ASSERT (MmTrackPtes & 0x2);

    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    if (NumberOfPtes == 0) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x200,
                      (ULONG_PTR) VirtualAddress,
                      0,
                      0);
    }

    StartBit = (ULONG) (PointerPte - MiPteStart);

    i = StartBit;

    StartBitMapBuffer = MiPteStartBitmap->Buffer;

    EndBitMapBuffer = MiPteEndBitmap->Buffer;

    ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

    for ( ; i < StartBit + NumberOfPtes; i += 1) {
        if (MI_CHECK_BIT (StartBitMapBuffer, i)) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x201,
                          (ULONG_PTR) VirtualAddress,
                          (ULONG_PTR) VirtualAddress + ((i - StartBit) << PAGE_SHIFT),
                          NumberOfPtes);
        }
    }

    RtlSetBits (MiPteStartBitmap, StartBit, NumberOfPtes);

    for (i = StartBit; i < StartBit + NumberOfPtes; i += 1) {
        if (MI_CHECK_BIT (EndBitMapBuffer, i)) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x202,
                          (ULONG_PTR) VirtualAddress,
                          (ULONG_PTR) VirtualAddress + ((i - StartBit) << PAGE_SHIFT),
                          NumberOfPtes);
        }
    }

    MI_SET_BIT (EndBitMapBuffer, i - 1);

    ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);
}

VOID
MiCheckPteRelease (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes
    )

/*++

Routine Description:

    This function checks the release of the specified number of system
    space PTEs.

Arguments:

    StartingPte - Supplies the address of the first PTE to release.

    NumberOfPtes - Supplies the number of PTEs to release.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    ULONG i;
    ULONG Index;
    ULONG StartBit;
    KIRQL OldIrql;
    ULONG CalculatedPtes;
    ULONG NumberOfPtesRoundedUp;
    PULONG StartBitMapBuffer;
    PULONG EndBitMapBuffer;
    PVOID VirtualAddress;
    PVOID LowestVirtualAddress;
    PVOID HighestVirtualAddress;
            
    ASSERT (MmTrackPtes & 0x2);

    VirtualAddress = MiGetVirtualAddressMappedByPte (StartingPte);

    LowestVirtualAddress = MiGetVirtualAddressMappedByPte (MmSystemPtesStart[SystemPteSpace]);

    HighestVirtualAddress = MiGetVirtualAddressMappedByPte (MmSystemPtesEnd[SystemPteSpace]);

    if (NumberOfPtes == 0) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x300,
                      (ULONG_PTR) VirtualAddress,
                      (ULONG_PTR) LowestVirtualAddress,
                      (ULONG_PTR) HighestVirtualAddress);
    }

    if (StartingPte < MmSystemPtesStart[SystemPteSpace]) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x301,
                      (ULONG_PTR) VirtualAddress,
                      (ULONG_PTR) LowestVirtualAddress,
                      (ULONG_PTR) HighestVirtualAddress);
    }

    if (StartingPte > MmSystemPtesEnd[SystemPteSpace]) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x302,
                      (ULONG_PTR) VirtualAddress,
                      (ULONG_PTR) LowestVirtualAddress,
                      (ULONG_PTR) HighestVirtualAddress);
    }

    StartBit = (ULONG) (StartingPte - MiPteStart);

    ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

    //
    // Verify start and size of allocation using the tracking bitmaps.
    //

    if (!RtlCheckBit (MiPteStartBitmap, StartBit)) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x303,
                      (ULONG_PTR) VirtualAddress,
                      NumberOfPtes,
                      0);
    }

    if (StartBit != 0) {

        if (RtlCheckBit (MiPteStartBitmap, StartBit - 1)) {

            if (!RtlCheckBit (MiPteEndBitmap, StartBit - 1)) {

                //
                // In the middle of an allocation... bugcheck.
                //

                KeBugCheckEx (SYSTEM_PTE_MISUSE,
                              0x304,
                              (ULONG_PTR) VirtualAddress,
                              NumberOfPtes,
                              0);
            }
        }
    }

    //
    // Find the last allocated PTE to calculate the correct size.
    //

    EndBitMapBuffer = MiPteEndBitmap->Buffer;

    i = StartBit;
    while (!MI_CHECK_BIT (EndBitMapBuffer, i)) {
        i += 1;
    }

    CalculatedPtes = i - StartBit + 1;
    NumberOfPtesRoundedUp = NumberOfPtes;

    if (CalculatedPtes <= MM_PTE_TABLE_LIMIT) {
        Index = MmSysPteTables [NumberOfPtes];
        NumberOfPtesRoundedUp = MmSysPteIndex [Index];
    }

    if (CalculatedPtes != NumberOfPtesRoundedUp) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x305,
                      (ULONG_PTR) VirtualAddress,
                      NumberOfPtes,
                      CalculatedPtes);
    }

    StartBitMapBuffer = MiPteStartBitmap->Buffer;

    for (i = StartBit; i < StartBit + CalculatedPtes; i += 1) {
        if (MI_CHECK_BIT (StartBitMapBuffer, i) == 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x306,
                          (ULONG_PTR) VirtualAddress,
                          (ULONG_PTR) VirtualAddress + ((i - StartBit) << PAGE_SHIFT),
                          CalculatedPtes);
        }
    }

    RtlClearBits (MiPteStartBitmap, StartBit, CalculatedPtes);

    MI_CLEAR_BIT (EndBitMapBuffer, i - 1);

    ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);
}



#if DBG

VOID
MiDumpSystemPtes (
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )
{
    PMMPTE PointerPte;
    PMMPTE PointerNextPte;
    ULONG_PTR ClusterSize;
    PMMPTE EndOfCluster;

    PointerPte = &MmFirstFreeSystemPte[SystemPtePoolType];
    if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
        return;
    }

    PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;

    for (;;) {
        if (PointerPte->u.List.OneEntry) {
            ClusterSize = 1;
        }
        else {
            PointerNextPte = PointerPte + 1;
            ClusterSize = (ULONG_PTR) PointerNextPte->u.List.NextEntry;
        }

        EndOfCluster = PointerPte + (ClusterSize - 1);

        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_TRACE_LEVEL, 
            "System Pte at %p for %p entries (%p)\n",
                PointerPte, ClusterSize, EndOfCluster);

        if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
            break;
        }

        PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
    }
    return;
}

ULONG
MiCountFreeSystemPtes (
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )
{
    PMMPTE PointerPte;
    PMMPTE PointerNextPte;
    ULONG_PTR ClusterSize;
    ULONG_PTR FreeCount;

    PointerPte = &MmFirstFreeSystemPte[SystemPtePoolType];
    if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
        return 0;
    }

    FreeCount = 0;

    PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;

    for (;;) {
        if (PointerPte->u.List.OneEntry) {
            ClusterSize = 1;

        }
        else {
            PointerNextPte = PointerPte + 1;
            ClusterSize = (ULONG_PTR) PointerNextPte->u.List.NextEntry;
        }

        FreeCount += ClusterSize;
        if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
            break;
        }

        PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
    }

    return (ULONG)FreeCount;
}

#endif

VOID
MiAddExtraSystemPteRanges (
    IN PMMPTE PointerPte,
    IN ULONG NumberOfPtes
    )
{
    PVOID StartingVa;
    KIRQL OldIrql;
    ULONG_PTR PteOffset;
    PMMPTE NextPte;
    PMMPTE ThisPte;

    ASSERT (NumberOfPtes != 0);
    ASSERT (MmSystemPteBase != NULL);

    //
    // Since the next PTE is used to hold the count, don't bother with any
    // single PTE additions (there shouldn't be any of those anyway).
    //

    if (NumberOfPtes == 1) {
        return;
    }

    PteOffset = (ULONG_PTR)(PointerPte - MmSystemPteBase);

    (PointerPte + 1)->u.List.NextEntry = NumberOfPtes;

    ThisPte = &MiAddPtesList;

    StartingVa = MiGetVirtualAddressMappedByPte (PointerPte);

    //
    // Insert the entry keeping the list sorted from small allocations to large.
    // This provides a crude way to keep the larger allocations contiguous for
    // a longer period of time.
    //

    MiLockSystemSpace (OldIrql);

    if (MiPteRangeIndex == MI_NUMBER_OF_PTE_RANGES) {
        ASSERT (FALSE);
        MiUnlockSystemSpace (OldIrql);
        return;
    }

    MiPteRanges[MiPteRangeIndex].StartingVa = StartingVa;
    MiPteRanges[MiPteRangeIndex].EndingVa = (PVOID) ((PCHAR)StartingVa + (NumberOfPtes << PAGE_SHIFT) - 1);
    MiPteRangeIndex += 1;

    while (ThisPte->u.List.NextEntry != MM_EMPTY_PTE_LIST) {
        NextPte = MmSystemPteBase + ThisPte->u.List.NextEntry;
        if (NumberOfPtes < (NextPte + 1)->u.List.NextEntry) {
            break;
        }
        ThisPte = NextPte;
    }

    PointerPte->u.List.NextEntry = ThisPte->u.List.NextEntry;
    ThisPte->u.List.NextEntry = PteOffset;

    MiUnlockSystemSpace (OldIrql);

    return;
}


LOGICAL
MiRecoverExtraPtes (
    IN ULONG NumberOfPtes
    )

/*++

Routine Description:

    This routine is called to recover extra PTEs for the system PTE pool.
    These are not just added in earlier in Phase 0 because the system PTE
    allocator uses the low addresses first which would fragment these
    bigger ranges.

Arguments:

    None.

Return Value:

    TRUE if any PTEs were added, FALSE if not.

Environment:

    Kernel mode.

--*/

{
    KIRQL OldIrql;
    PMMPTE NextPte;
    PMMPTE ThisPte;

    ThisPte = &MiAddPtesList;

    //
    // Quickly do an unsynchronized check so we avoid the spinlock if we
    // have no chance.
    //

    if (ThisPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
        return FALSE;
    }

    //
    // Search for an entry large enough to satisfy the request, if one
    // cannot be found, then remove them all in hopes that adjacent allocations
    // can be coalesced by our caller to satisfy the request.
    //

    MiLockSystemSpace (OldIrql);

    if (ThisPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
        MiUnlockSystemSpace (OldIrql);
        return FALSE;
    }

    do {
        NextPte = MmSystemPteBase + ThisPte->u.List.NextEntry;

        //
        // Just free the whole entry as splitting it into the exact amount
        // can trigger race conditions where another thread allocates a PTE
        // from the front before our caller retries (causing our caller to
        // fail the API).
        //

        if (NumberOfPtes <= (NextPte + 1)->u.List.NextEntry) {

            //
            // The request can only be satisfied by using the entire entry so
            // delink it now.
            //

            ThisPte->u.List.NextEntry = NextPte->u.List.NextEntry;

            MiUnlockSystemSpace (OldIrql);

            NumberOfPtes = (ULONG)((NextPte + 1)->u.List.NextEntry);

            MiAddSystemPtes (NextPte, NumberOfPtes, SystemPteSpace);

            return TRUE;
        }

        ThisPte = NextPte;

    } while (ThisPte->u.List.NextEntry != MM_EMPTY_PTE_LIST);

    MiUnlockSystemSpace (OldIrql);

#if defined(_X86_)
    return MiRecoverSpecialPtes (NumberOfPtes);
#else
    return FALSE;
#endif
}

ULONG
MmGetNumberOfFreeSystemPtes (
    VOID
    )
/*++

Routine Description:

    This routine is count the number of system PTEs left.

Arguments:

    None.

Return Value:

    TRUE if any PTEs were added, FALSE if not.

Environment:

    Kernel mode.

--*/

{
#if !defined (_X86_)
    return MmTotalFreeSystemPtes[0];
#else
    ULONG NumberOfPtes;
    KIRQL OldIrql;
    PMMPTE NextPte;
    PMMPTE ThisPte;

    NumberOfPtes = MmTotalFreeSystemPtes[0];

    ThisPte = &MiAddPtesList;

    //
    // Quickly do an unsynchronized check so we avoid the spinlock if we
    // have no chance.
    //

    if (ThisPte->u.List.NextEntry != MM_EMPTY_PTE_LIST) {

        MiLockSystemSpace (OldIrql);

        while (ThisPte->u.List.NextEntry != MM_EMPTY_PTE_LIST) {

            NextPte = MmSystemPteBase + ThisPte->u.List.NextEntry;

            NumberOfPtes += (ULONG) ((NextPte + 1)->u.List.NextEntry);

            ThisPte = NextPte;
        }

        MiUnlockSystemSpace (OldIrql);
    }

    return NumberOfPtes + MiSpecialPoolExtraCount;
#endif
}

