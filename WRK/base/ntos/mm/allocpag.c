/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   allocpag.c

Abstract:

    This module contains the routines which allocate and deallocate
    one or more pages from paged or nonpaged pool.

--*/

#include "mi.h"

#if DBG
extern ULONG MiShowStuckPages;
#endif

PVOID
MiFindContiguousMemoryInPool (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN PVOID CallingAddress
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MiInitializeNonPagedPool)
#pragma alloc_text(INIT, MiAddExpansionNonPagedPool)
#pragma alloc_text(INIT, MiInitializePoolEvents)
#pragma alloc_text(INIT, MiSyncCachedRanges)

#pragma alloc_text(PAGE, MmAvailablePoolInPages)
#pragma alloc_text(PAGE, MiFindContiguousMemory)
#pragma alloc_text(PAGELK, MiFindContiguousMemoryInPool)
#pragma alloc_text(PAGELK, MiFindLargePageMemory)

#pragma alloc_text(PAGE, MiCheckSessionPoolAllocations)
#pragma alloc_text(PAGE, MiSessionPoolVector)
#pragma alloc_text(PAGE, MiSessionPoolMutex)
#pragma alloc_text(PAGE, MiInitializeSessionPool)
#pragma alloc_text(PAGE, MiFreeSessionPoolBitMaps)

#pragma alloc_text(POOLMI, MiAllocatePoolPages)
#pragma alloc_text(POOLMI, MiFreePoolPages)
#endif

ULONG MmPagedPoolCommit;        // used by the debugger

SLIST_HEADER MiNonPagedPoolSListHead;
ULONG MiNonPagedPoolSListMaximum = 4;

SLIST_HEADER MiPagedPoolSListHead;
ULONG MiPagedPoolSListMaximum = 8;

PFN_NUMBER MmAllocatedNonPagedPool;
PFN_NUMBER MiStartOfInitialPoolFrame;
PFN_NUMBER MiEndOfInitialPoolFrame;

PVOID MmNonPagedPoolEnd0;
PVOID MmNonPagedPoolExpansionStart;

LIST_ENTRY MmNonPagedPoolFreeListHead[MI_MAX_FREE_LIST_HEADS];

PMMPFN MiCachedNonPagedPool;
PFN_NUMBER MiCachedNonPagedPoolCount;

extern POOL_DESCRIPTOR NonPagedPoolDescriptor;

extern PFN_NUMBER MmFreedExpansionPoolMaximum;

extern KGUARDED_MUTEX MmPagedPoolMutex;

#define MM_SMALL_ALLOCATIONS 4

#if DBG

ULONG MiClearCache;

//
// Set this to a nonzero (ie: 10000) value to cause every pool allocation to
// be checked and an ASSERT fires if the allocation is larger than this value.
//

ULONG MmCheckRequestInPages = 0;

//
// Set this to a nonzero (ie: 0x23456789) value to cause this pattern to be
// written into freed nonpaged pool pages.
//

ULONG MiFillFreedPool = 0;
#endif

PFN_NUMBER MiExpansionPoolPagesInUse;
PFN_NUMBER MiExpansionPoolPagesInitialCharge;

ULONG MmUnusedSegmentForceFreeDefault = 30;

extern ULONG MmUnusedSegmentForceFree;

//
// For debugging purposes.
//

typedef enum _MM_POOL_TYPES {
    MmNonPagedPool,
    MmPagedPool,
    MmSessionPagedPool,
    MmMaximumPoolType
} MM_POOL_TYPES;

typedef enum _MM_POOL_PRIORITIES {
    MmHighPriority,
    MmNormalPriority,
    MmLowPriority,
    MmMaximumPoolPriority
} MM_POOL_PRIORITIES;

typedef enum _MM_POOL_FAILURE_REASONS {
    MmNonPagedNoPtes,
    MmPriorityTooLow,
    MmNonPagedNoPagesAvailable,
    MmPagedNoPtes,
    MmSessionPagedNoPtes,
    MmPagedNoPagesAvailable,
    MmSessionPagedNoPagesAvailable,
    MmPagedNoCommit,
    MmSessionPagedNoCommit,
    MmNonPagedNoResidentAvailable,
    MmNonPagedNoCommit,
    MmMaximumFailureReason
} MM_POOL_FAILURE_REASONS;

ULONG MmPoolFailures[MmMaximumPoolType][MmMaximumPoolPriority];
ULONG MmPoolFailureReasons[MmMaximumFailureReason];

typedef enum _MM_PREEMPTIVE_TRIMS {
    MmPreemptForNonPaged,
    MmPreemptForPaged,
    MmPreemptForNonPagedPriority,
    MmPreemptForPagedPriority,
    MmMaximumPreempt
} MM_PREEMPTIVE_TRIMS;

ULONG MmPreemptiveTrims[MmMaximumPreempt];


VOID
MiProtectFreeNonPagedPool (
    IN PVOID VirtualAddress,
    IN ULONG SizeInPages
    )

/*++

Routine Description:

    This function protects freed nonpaged pool.

Arguments:

    VirtualAddress - Supplies the freed pool address to protect.

    SizeInPages - Supplies the size of the request in pages.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    ULONG i;
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PVOID FlushVa[MM_MAXIMUM_FLUSH_COUNT];

    if (MI_IS_PHYSICAL_ADDRESS (VirtualAddress)) {
        return;
    }

    //
    // Prevent anyone from touching the free non paged pool.
    //

    PointerPte = MiGetPteAddress (VirtualAddress);
    LastPte = PointerPte + SizeInPages;

    do {

        PteContents = *PointerPte;

        PteContents.u.Hard.Valid = 0;
        PteContents.u.Soft.Prototype = 1;

        MI_WRITE_INVALID_PTE (PointerPte, PteContents);

        PointerPte += 1;

    } while (PointerPte < LastPte);

    if (SizeInPages == 1) {
        MI_FLUSH_SINGLE_TB (VirtualAddress, TRUE);
    }
    else if (SizeInPages < MM_MAXIMUM_FLUSH_COUNT) {

        for (i = 0; i < SizeInPages; i += 1) {
            FlushVa[i] = VirtualAddress;
            VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);
        }

        MI_FLUSH_MULTIPLE_TB (SizeInPages, &FlushVa[0], TRUE);
    }
    else {
        MI_FLUSH_ENTIRE_TB (0xB);
    }

    return;
}


LOGICAL
MiUnProtectFreeNonPagedPool (
    IN PVOID VirtualAddress,
    IN ULONG SizeInPages
    )

/*++

Routine Description:

    This function unprotects freed nonpaged pool.

Arguments:

    VirtualAddress - Supplies the freed pool address to unprotect.

    SizeInPages - Supplies the size of the request in pages - zero indicates
                  to keep going until there are no more protected PTEs (ie: the
                  caller doesn't know how many protected PTEs there are).

Return Value:

    TRUE if pages were unprotected, FALSE if not.

Environment:

    Kernel mode.

--*/

{
    PMMPTE PointerPte;
    MMPTE PteContents;
    ULONG PagesDone;

    PagesDone = 0;

    //
    // Unprotect the previously freed pool so it can be manipulated
    //

    if (MI_IS_PHYSICAL_ADDRESS(VirtualAddress) == 0) {

        PointerPte = MiGetPteAddress((PVOID)VirtualAddress);

        PteContents = *PointerPte;

        while (PteContents.u.Hard.Valid == 0 && PteContents.u.Soft.Prototype == 1) {

            PteContents.u.Hard.Valid = 1;
            PteContents.u.Soft.Prototype = 0;
    
            MI_WRITE_VALID_PTE (PointerPte, PteContents);

            PagesDone += 1;

            if (PagesDone == SizeInPages) {
                break;
            }

            PointerPte += 1;
            PteContents = *PointerPte;
        }
    }

    if (PagesDone == 0) {
        return FALSE;
    }

    return TRUE;
}


VOID
MiProtectedPoolInsertList (
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry,
    IN LOGICAL InsertHead
    )

/*++

Routine Description:

    This function inserts the entry into the protected list.

Arguments:

    ListHead - Supplies the list head to add onto.

    Entry - Supplies the list entry to insert.

    InsertHead - If TRUE, insert at the head otherwise at the tail.

Return Value:

    None.

Environment:

    Kernel mode.

--*/
{
    PVOID FreeFlink;
    PVOID FreeBlink;
    PVOID VirtualAddress;

    //
    // Either the flink or the blink may be pointing
    // at protected nonpaged pool.  Unprotect now.
    //

    FreeFlink = (PVOID)0;
    FreeBlink = (PVOID)0;

    if (IsListEmpty(ListHead) == 0) {

        VirtualAddress = (PVOID)ListHead->Flink;
        if (MiUnProtectFreeNonPagedPool (VirtualAddress, 1) == TRUE) {
            FreeFlink = VirtualAddress;
        }
    }

    if (((PVOID)Entry == ListHead->Blink) == 0) {
        VirtualAddress = (PVOID)ListHead->Blink;
        if (MiUnProtectFreeNonPagedPool (VirtualAddress, 1) == TRUE) {
            FreeBlink = VirtualAddress;
        }
    }

    if (InsertHead == TRUE) {
        InsertHeadList (ListHead, Entry);
    }
    else {
        InsertTailList (ListHead, Entry);
    }

    if (FreeFlink) {
        //
        // Reprotect the flink.
        //

        MiProtectFreeNonPagedPool (FreeFlink, 1);
    }

    if (FreeBlink) {
        //
        // Reprotect the blink.
        //

        MiProtectFreeNonPagedPool (FreeBlink, 1);
    }
}


VOID
MiProtectedPoolRemoveEntryList (
    IN PLIST_ENTRY Entry
    )

/*++

Routine Description:

    This function unlinks the list pointer from protected freed nonpaged pool.

Arguments:

    Entry - Supplies the list entry to remove.

Return Value:

    None.

Environment:

    Kernel mode.

--*/
{
    PVOID FreeFlink;
    PVOID FreeBlink;
    PVOID VirtualAddress;

    //
    // Either the flink or the blink may be pointing
    // at protected nonpaged pool.  Unprotect now.
    //

    FreeFlink = (PVOID)0;
    FreeBlink = (PVOID)0;

    if (IsListEmpty(Entry) == 0) {

        VirtualAddress = (PVOID)Entry->Flink;
        if (MiUnProtectFreeNonPagedPool (VirtualAddress, 1) == TRUE) {
            FreeFlink = VirtualAddress;
        }
    }

    if (((PVOID)Entry == Entry->Blink) == 0) {
        VirtualAddress = (PVOID)Entry->Blink;
        if (MiUnProtectFreeNonPagedPool (VirtualAddress, 1) == TRUE) {
            FreeBlink = VirtualAddress;
        }
    }

    RemoveEntryList (Entry);

    if (FreeFlink) {
        //
        // Reprotect the flink.
        //

        MiProtectFreeNonPagedPool (FreeFlink, 1);
    }

    if (FreeBlink) {
        //
        // Reprotect the blink.
        //

        MiProtectFreeNonPagedPool (FreeBlink, 1);
    }
}


VOID
MiTrimSegmentCache (
    VOID
    )

/*++

Routine Description:

    This function initiates trimming of the segment cache.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel Mode Only.

--*/

{
    KIRQL OldIrql;
    LOGICAL SignalDereferenceThread;
    LOGICAL SignalSystemCache;

    SignalDereferenceThread = FALSE;
    SignalSystemCache = FALSE;

    LOCK_PFN2 (OldIrql);

    if (MmUnusedSegmentForceFree == 0) {

        if (!IsListEmpty(&MmUnusedSegmentList)) {

            SignalDereferenceThread = TRUE;
            MmUnusedSegmentForceFree = MmUnusedSegmentForceFreeDefault;
        }
        else {
            if (!IsListEmpty(&MmUnusedSubsectionList)) {
                SignalDereferenceThread = TRUE;
                MmUnusedSegmentForceFree = MmUnusedSegmentForceFreeDefault;
            }

            if (MiUnusedSubsectionPagedPool < 4 * PAGE_SIZE) {

                //
                // No unused segments and tossable subsection usage is low as
                // well.  Start unmapping system cache views in an attempt
                // to get back the paged pool containing its prototype PTEs.
                //
    
                SignalSystemCache = TRUE;
            }
        }
    }

    UNLOCK_PFN2 (OldIrql);

    if (SignalSystemCache == TRUE) {
        if (CcHasInactiveViews() == TRUE) {
            if (SignalDereferenceThread == FALSE) {
                LOCK_PFN2 (OldIrql);
                if (MmUnusedSegmentForceFree == 0) {
                    SignalDereferenceThread = TRUE;
                    MmUnusedSegmentForceFree = MmUnusedSegmentForceFreeDefault;
                }
                UNLOCK_PFN2 (OldIrql);
            }
        }
    }

    if (SignalDereferenceThread == TRUE) {
        KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
    }
}


POOL_TYPE
MmDeterminePoolType (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This function determines which pool a virtual address resides within.

Arguments:

    VirtualAddress - Supplies the virtual address to determine which pool
                     it resides within.

Return Value:

    Returns the POOL_TYPE (PagedPool, NonPagedPool, PagedPoolSession or
            NonPagedPoolSession).

Environment:

    Kernel Mode Only.

--*/

{
    if ((VirtualAddress >= MmPagedPoolStart) &&
        (VirtualAddress <= MmPagedPoolEnd)) {
        return PagedPool;
    }

    if (MI_IS_SESSION_POOL_ADDRESS (VirtualAddress) == TRUE) {
        return PagedPoolSession;
    }

    return NonPagedPool;
}


PVOID
MiSessionPoolVector (
    VOID
    )

/*++

Routine Description:

    This function returns the session pool descriptor for the current session.

Arguments:

    None.

Return Value:

    Pool descriptor.

--*/

{
    PAGED_CODE ();

    return (PVOID)&MmSessionSpace->PagedPool;
}


SIZE_T
MmAvailablePoolInPages (
    IN POOL_TYPE PoolType
    )

/*++

Routine Description:

    This function returns the number of pages available for the given pool.
    Note that it does not account for any executive pool fragmentation.

Arguments:

    PoolType - Supplies the type of pool to retrieve information about.

Return Value:

    The number of full pool pages remaining.

Environment:

    PASSIVE_LEVEL, no mutexes or locks held.

--*/

{
    SIZE_T FreePoolInPages;
    SIZE_T FreeCommitInPages;

#if !DBG
    UNREFERENCED_PARAMETER (PoolType);
#endif

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (PoolType == PagedPool);

    FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

    FreeCommitInPages = MmTotalCommitLimitMaximum - MmTotalCommittedPages;

    if (FreePoolInPages > FreeCommitInPages) {
        FreePoolInPages = FreeCommitInPages;
    }

    return FreePoolInPages;
}


LOGICAL
MmResourcesAvailable (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN EX_POOL_PRIORITY Priority
    )

/*++

Routine Description:

    This function examines various resources to determine if this
    pool allocation should be allowed to proceed.

Arguments:

    PoolType - Supplies the type of pool to retrieve information about.

    NumberOfBytes - Supplies the number of bytes to allocate.

    Priority - Supplies an indication as to how important it is that this
               request succeed under low available resource conditions.                       
Return Value:

    TRUE if the pool allocation should be allowed to proceed, FALSE if not.

--*/

{
    KIRQL OldIrql;
    PFN_NUMBER NumberOfPages;
    SIZE_T FreePoolInBytes;
    LOGICAL Status;
    MM_POOL_PRIORITIES Index;

    ASSERT (Priority != HighPoolPriority);
    ASSERT ((PoolType & MUST_SUCCEED_POOL_TYPE_MASK) == 0);

    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);

    if ((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {
        FreePoolInBytes = ((MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool) << PAGE_SHIFT);
    }
    else if (PoolType & SESSION_POOL_MASK) {
        FreePoolInBytes = MmSessionPoolSize - (MmSessionSpace->PagedPoolInfo.AllocatedPagedPool << PAGE_SHIFT);
    }
    else {
        FreePoolInBytes = ((MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool) << PAGE_SHIFT);
    }

    Status = FALSE;

    //
    // Check available VA space.
    //

    if (Priority == NormalPoolPriority) {
        if ((SIZE_T)NumberOfBytes + 512*1024 > FreePoolInBytes) {
            if ((PsGetCurrentThread()->MemoryMaker == 0) || (KeIsExecutingDpc ())) {
                goto nopool;
            }
        }
    }
    else {
        if ((SIZE_T)NumberOfBytes + 2*1024*1024 > FreePoolInBytes) {
            if ((PsGetCurrentThread()->MemoryMaker == 0) || (KeIsExecutingDpc ())) {
                goto nopool;
            }
        }
    }

    //
    // Paged allocations (session and normal) can also fail for lack of commit.
    //

    if ((PoolType & BASE_POOL_TYPE_MASK) == PagedPool) {
        if (MmTotalCommittedPages + NumberOfPages > MmTotalCommitLimitMaximum) {
            if (PsGetCurrentThread()->MemoryMaker == 0) {
                MiIssuePageExtendRequestNoWait (NumberOfPages);
                goto nopool;
            }
        }
    }

    //
    // If a substantial amount of free pool is still available, return TRUE now.
    //

    if (((SIZE_T)NumberOfBytes + 10*1024*1024 < FreePoolInBytes) ||
        (MmNumberOfPhysicalPages < 256 * 1024)) {
        return TRUE;
    }

    //
    // This pool allocation is permitted, but because we're starting to run low,
    // trigger a round of dereferencing in parallel before returning success.
    // Note this is only done on machines with at least 1GB of RAM as smaller
    // configuration machines will already trigger this due to physical page
    // consumption.
    //

    Status = TRUE;

nopool:

    //
    // Running low on pool - if this request is not for session pool,
    // force unused segment trimming when appropriate.
    //

    if ((PoolType & SESSION_POOL_MASK) == 0) {

        if ((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {

            MmPreemptiveTrims[MmPreemptForNonPagedPriority] += 1;

            OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

            //
            // Only pulse the event if it is not already set.  This saves
            // on dispatcher lock contention, but more importantly since
            // KePulse always clears the event it saves us having to check
            // whether to set it (and potentially do the setting) afterwards.
            //

            if (MiLowNonPagedPoolEvent->Header.SignalState == 0) {
                KePulseEvent (MiLowNonPagedPoolEvent, 0, FALSE);
            }

            KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock,
                                     OldIrql);
        }
        else {

            MmPreemptiveTrims[MmPreemptForPagedPriority] += 1;

            KeAcquireGuardedMutex (&MmPagedPoolMutex);

            //
            // Only pulse the event if it is not already set.  This saves
            // on dispatcher lock contention, but more importantly since
            // KePulse always clears the event it saves us having to check
            // whether to set it (and potentially do the setting) afterwards.
            //

            if (MiLowPagedPoolEvent->Header.SignalState == 0) {
                KePulseEvent (MiLowPagedPoolEvent, 0, FALSE);
            }

            KeReleaseGuardedMutex (&MmPagedPoolMutex);
        }

        if (MI_UNUSED_SEGMENTS_SURPLUS()) {
            KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
        }
        else {
            MiTrimSegmentCache ();
        }
    }

    if (Status == FALSE) {

        //
        // Log this failure for debugging purposes.
        //

        if (Priority == NormalPoolPriority) {
            Index = MmNormalPriority;
        }
        else {
            Index = MmLowPriority;
        }

        if ((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {
            MmPoolFailures[MmNonPagedPool][Index] += 1;
        }
        else if (PoolType & SESSION_POOL_MASK) {
            MmPoolFailures[MmSessionPagedPool][Index] += 1;
            MmSessionSpace->SessionPoolAllocationFailures[0] += 1;
        }
        else {
            MmPoolFailures[MmPagedPool][Index] += 1;
        }

        MmPoolFailureReasons[MmPriorityTooLow] += 1;
    }

    return Status;
}


VOID
MiFreeNonPagedPool (
    IN PVOID StartingAddress,
    IN PFN_NUMBER NumberOfPages
    )

/*++

Routine Description:

    This function releases virtually mapped nonpaged expansion pool.

Arguments:

    StartingAddress - Supplies the starting address.

    NumberOfPages - Supplies the number of pages to free.

Return Value:

    None.

Environment:

    These functions are used by the internal Mm page allocation/free routines
    only and should not be called directly.

    The nonpaged pool spinlock must be held at DISPATCH_LEVEL when calling
    this function.

--*/

{
    PFN_NUMBER i;
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PFN_NUMBER ResAvailToReturn;
    PFN_NUMBER PageFrameIndex;

    PointerPte = MiGetPteAddress (StartingAddress);

    //
    // Return commitment.
    //

    MiReturnCommitment (NumberOfPages);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_NONPAGED_POOL_EXPANSION,
                     NumberOfPages);

    ResAvailToReturn = 0;

    LOCK_PFN_AT_DPC ();

    if (MiExpansionPoolPagesInUse > MiExpansionPoolPagesInitialCharge) {
        ResAvailToReturn = MiExpansionPoolPagesInUse - MiExpansionPoolPagesInitialCharge;
    }
    MiExpansionPoolPagesInUse -= NumberOfPages;

    for (i = 0; i < NumberOfPages; i += 1) {

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        //
        // Set the pointer to the PTE as empty so the page
        // is deleted when the reference count goes to zero.
        //

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        ASSERT (Pfn1->u2.ShareCount == 1);
        Pfn1->u2.ShareCount = 0;
        MI_SET_PFN_DELETED (Pfn1);
#if DBG
        Pfn1->u3.e1.PageLocation = StandbyPageList;
#endif
        MiDecrementReferenceCount (Pfn1, PageFrameIndex);

        MI_WRITE_ZERO_PTE (PointerPte);

        PointerPte += 1;
    }

    //
    // No TB flush is done here - the system PTE management code will
    // lazily flush as needed.
    //

    UNLOCK_PFN_FROM_DPC ();

    //
    // Generally there is no need to update resident available
    // pages at this time as it has all been done during initialization.
    // However, only some of the expansion pool was charged at init, so
    // calculate how much (if any) resident available page charge to return.
    //

    if (ResAvailToReturn > NumberOfPages) {
        ResAvailToReturn = NumberOfPages;
    }

    if (ResAvailToReturn != 0) {
        MI_INCREMENT_RESIDENT_AVAILABLE (ResAvailToReturn, MM_RESAVAIL_FREE_EXPANSION_NONPAGED_POOL);
    }

    PointerPte -= NumberOfPages;

    MiReleaseSystemPtes (PointerPte,
                         (ULONG)NumberOfPages,
                         NonPagedPoolExpansion);
}

LOGICAL
MiFreeAllExpansionNonPagedPool (
    VOID
    )

/*++

Routine Description:

    This function releases all virtually mapped nonpaged expansion pool.

Arguments:

    None.

Return Value:

    TRUE if pages were freed, FALSE if not.

Environment:

    Kernel mode.

--*/

{
    ULONG Index;
    KIRQL OldIrql;
    PLIST_ENTRY Entry;
    LOGICAL FreedPool;
    PMMFREE_POOL_ENTRY FreePageInfo;

    FreedPool = FALSE;

    OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

    for (Index = 0; Index < MI_MAX_FREE_LIST_HEADS; Index += 1) {

        Entry = MmNonPagedPoolFreeListHead[Index].Flink;

        while (Entry != &MmNonPagedPoolFreeListHead[Index]) {

            if (MmProtectFreedNonPagedPool == TRUE) {
                MiUnProtectFreeNonPagedPool ((PVOID)Entry, 0);
            }

            //
            // The list is not empty, see if this one is virtually
            // mapped.
            //

            FreePageInfo = CONTAINING_RECORD(Entry,
                                             MMFREE_POOL_ENTRY,
                                             List);

            if ((!MI_IS_PHYSICAL_ADDRESS(FreePageInfo)) &&
                ((PVOID)FreePageInfo >= MmNonPagedPoolExpansionStart)) {

                if (MmProtectFreedNonPagedPool == FALSE) {
                    RemoveEntryList (&FreePageInfo->List);
                }
                else {
                    MiProtectedPoolRemoveEntryList (&FreePageInfo->List);
                }

                MmNumberOfFreeNonPagedPool -= FreePageInfo->Size;
                ASSERT ((LONG)MmNumberOfFreeNonPagedPool >= 0);

                FreedPool = TRUE;

                MiFreeNonPagedPool ((PVOID)FreePageInfo,
                                    FreePageInfo->Size);

                Index = (ULONG)-1;
                break;
            }

            Entry = FreePageInfo->List.Flink;

            if (MmProtectFreedNonPagedPool == TRUE) {
                MiProtectFreeNonPagedPool ((PVOID)FreePageInfo,
                                           (ULONG)FreePageInfo->Size);
            }
        }
    }

    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

    return FreedPool;
}

VOID
MiMarkPoolLargeSession (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This function marks a NONPAGED pool allocation as being of
    type large session.

Arguments:

    VirtualAddress - Supplies the virtual address of the pool allocation.

Return Value:

    None.

Environment:

    This function is used by the general pool allocation routines
    and should not be called directly.

    Kernel mode, IRQL <= DISPATCH_LEVEL.

--*/

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;

    ASSERT (PAGE_ALIGN (VirtualAddress) == VirtualAddress);

    if (MI_IS_PHYSICAL_ADDRESS (VirtualAddress)) {

        //
        // On certain architectures, virtual addresses
        // may be physical and hence have no corresponding PTE.
        //

        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (VirtualAddress);
    }
    else {
        PointerPte = MiGetPteAddress (VirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    }

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    LOCK_PFN2 (OldIrql);

    ASSERT (Pfn1->u3.e1.StartOfAllocation == 1);
    ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);

    Pfn1->u3.e1.LargeSessionAllocation = 1;

    UNLOCK_PFN2 (OldIrql);

    return;
}


LOGICAL
MiIsPoolLargeSession (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This function determines whether the argument nonpaged allocation was
    marked as a large session allocation.

Arguments:

    VirtualAddress - Supplies the virtual address of the pool allocation.

Return Value:

    None.

Environment:

    This function is used by the general pool allocation routines
    and should not be called directly.

    Kernel mode, IRQL <= DISPATCH_LEVEL.

--*/

{
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;

    ASSERT (PAGE_ALIGN (VirtualAddress) == VirtualAddress);

    if (MI_IS_PHYSICAL_ADDRESS (VirtualAddress)) {

        //
        // On certain architectures, virtual addresses
        // may be physical and hence have no corresponding PTE.
        //

        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (VirtualAddress);
    }
    else {
        PointerPte = MiGetPteAddress (VirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    }

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    ASSERT (Pfn1->u3.e1.StartOfAllocation == 1);

    if (Pfn1->u3.e1.LargeSessionAllocation == 0) {
        return FALSE;
    }

    return TRUE;
}


PVOID
MiAllocatePoolPages (
    IN POOL_TYPE PoolType,
    IN SIZE_T SizeInBytes
    )

/*++

Routine Description:

    This function allocates a set of pages from the specified pool
    and returns the starting virtual address to the caller.

Arguments:

    PoolType - Supplies the type of pool from which to obtain pages.

    SizeInBytes - Supplies the size of the request in bytes.  The actual
                  size returned is rounded up to a page boundary.

Return Value:

    Returns a pointer to the allocated pool, or NULL if no more pool is
    available.

Environment:

    These functions are used by the general pool allocation routines
    and should not be called directly.

    Kernel mode, IRQL at DISPATCH_LEVEL.

--*/

{
    PFN_NUMBER SizeInPages;
    ULONG TimeStamp;
    ULONG LastTimeStamp;
    ULONG StartPosition;
    ULONG EndPosition;
    PMMPTE StartingPte;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PMMPFN PfnList;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PVOID BaseVa;
    KIRQL OldIrql;
    PFN_NUMBER i;
    PFN_NUMBER j;
    PLIST_ENTRY Entry;
    PLIST_ENTRY ListHead;
    PLIST_ENTRY LastListHead;
    PMMFREE_POOL_ENTRY FreePageInfo;
    PMM_SESSION_SPACE SessionSpace;
    PMM_PAGED_POOL_INFO PagedPoolInfo;
    PVOID VirtualAddress;
    PVOID VirtualAddressSave;
    ULONG_PTR Index;
    ULONG PageTableCount;
    PFN_NUMBER FreePoolInPages;
    LOGICAL FlushedTb;
    ULONG FlushCount;
    PVOID VaFlushList[MM_MAXIMUM_FLUSH_COUNT];
    LOGICAL PreviousPteNeededFlush;

    SizeInPages = BYTES_TO_PAGES (SizeInBytes);

#if DBG
    if (MmCheckRequestInPages != 0) {
        ASSERT (SizeInPages < MmCheckRequestInPages);
    }
#endif

    if ((PoolType & BASE_POOL_TYPE_MASK) == NonPagedPool) {

        if ((SizeInPages == 1) &&
            (ExQueryDepthSList (&MiNonPagedPoolSListHead) != 0)) {

            BaseVa = InterlockedPopEntrySList (&MiNonPagedPoolSListHead);

            if (BaseVa != NULL) {

                if (PoolType & POOL_VERIFIER_MASK) {
                    if (MI_IS_PHYSICAL_ADDRESS(BaseVa)) {
                        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (BaseVa);
                    }
                    else {
                        PointerPte = MiGetPteAddress(BaseVa);
                        ASSERT (PointerPte->u.Hard.Valid == 1);
                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    }
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    Pfn1->u4.VerifierAllocation = 1;
                }

                return BaseVa;
            }
        }

        Index = SizeInPages - 1;

        if (Index >= MI_MAX_FREE_LIST_HEADS) {
            Index = MI_MAX_FREE_LIST_HEADS - 1;
        }

        //
        // NonPaged pool is linked together through the pages themselves.
        //

        ListHead = &MmNonPagedPoolFreeListHead[Index];
        LastListHead = &MmNonPagedPoolFreeListHead[MI_MAX_FREE_LIST_HEADS];

        OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

        do {

            Entry = ListHead->Flink;

            while (Entry != ListHead) {

                if (MmProtectFreedNonPagedPool == TRUE) {
                    MiUnProtectFreeNonPagedPool ((PVOID)Entry, 0);
                }
    
                //
                // The list is not empty, see if this one has enough space.
                //
    
                FreePageInfo = CONTAINING_RECORD(Entry,
                                                 MMFREE_POOL_ENTRY,
                                                 List);
    
                ASSERT (FreePageInfo->Signature == MM_FREE_POOL_SIGNATURE);
                if (FreePageInfo->Size >= SizeInPages) {
    
                    //
                    // This entry has sufficient space, remove
                    // the pages from the end of the allocation.
                    //
    
                    FreePageInfo->Size -= SizeInPages;
    
                    BaseVa = (PVOID)((PCHAR)FreePageInfo +
                                            (FreePageInfo->Size  << PAGE_SHIFT));
    
                    if (MmProtectFreedNonPagedPool == FALSE) {
                        RemoveEntryList (&FreePageInfo->List);
                    }
                    else {
                        MiProtectedPoolRemoveEntryList (&FreePageInfo->List);
                    }

                    if (FreePageInfo->Size != 0) {
    
                        //
                        // Insert any remainder into the correct list.
                        //
    
                        Index = (ULONG)(FreePageInfo->Size - 1);
                        if (Index >= MI_MAX_FREE_LIST_HEADS) {
                            Index = MI_MAX_FREE_LIST_HEADS - 1;
                        }

                        if (MmProtectFreedNonPagedPool == FALSE) {
                            InsertTailList (&MmNonPagedPoolFreeListHead[Index],
                                            &FreePageInfo->List);
                        }
                        else {
                            MiProtectedPoolInsertList (&MmNonPagedPoolFreeListHead[Index],
                                                       &FreePageInfo->List,
                                                       FALSE);

                            MiProtectFreeNonPagedPool ((PVOID)FreePageInfo,
                                                       (ULONG)FreePageInfo->Size);
                        }
                    }
    
                    //
                    // Adjust the number of free pages remaining in the pool.
                    //
    
                    MmNumberOfFreeNonPagedPool -= SizeInPages;
                    ASSERT ((LONG)MmNumberOfFreeNonPagedPool >= 0);
    
                    //
                    // Mark start and end of allocation in the PFN database.
                    //
    
                    if (MI_IS_PHYSICAL_ADDRESS(BaseVa)) {
    
                        //
                        // On certain architectures, virtual addresses
                        // may be physical and hence have no corresponding PTE.
                        //
    
                        PointerPte = NULL;
                        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (BaseVa);
                    }
                    else {
                        PointerPte = MiGetPteAddress(BaseVa);
                        ASSERT (PointerPte->u.Hard.Valid == 1);
                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    }
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
                    ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
                    ASSERT (Pfn1->u4.VerifierAllocation == 0);
    
                    Pfn1->u3.e1.StartOfAllocation = 1;
    
                    if (PoolType & POOL_VERIFIER_MASK) {
                        Pfn1->u4.VerifierAllocation = 1;
                    }

                    //
                    // Calculate the ending PTE's address.
                    //
    
                    if (SizeInPages != 1) {
                        if (PointerPte == NULL) {
                            Pfn1 += SizeInPages - 1;
                        }
                        else {
                            PointerPte += SizeInPages - 1;
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
                    }
    
                    ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);
    
                    Pfn1->u3.e1.EndOfAllocation = 1;
    
                    MmAllocatedNonPagedPool += SizeInPages;

                    FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

                    if (FreePoolInPages < MiHighNonPagedPoolThreshold) {

                        //
                        // Read the state directly instead of calling
                        // KeReadStateEvent since we are holding the nonpaged
                        // pool lock and want to keep instructions at a
                        // minimum.
                        //

                        if (MiHighNonPagedPoolEvent->Header.SignalState != 0) {
                            KeClearEvent (MiHighNonPagedPoolEvent);
                        }
                        if (FreePoolInPages <= MiLowNonPagedPoolThreshold) {
                            if (MiLowNonPagedPoolEvent->Header.SignalState == 0) {
                                KeSetEvent (MiLowNonPagedPoolEvent, 0, FALSE);
                            }
                        }
                    }

                    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock,
                                             OldIrql);

                    return BaseVa;
                }
    
                Entry = FreePageInfo->List.Flink;
    
                if (MmProtectFreedNonPagedPool == TRUE) {
                    MiProtectFreeNonPagedPool ((PVOID)FreePageInfo,
                                               (ULONG)FreePageInfo->Size);
                }
            }

            ListHead += 1;

        } while (ListHead < LastListHead);

        KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

        //
        // No more entries on the list, expand nonpaged pool if
        // possible to satisfy this request.
        //
        // If pool is starting to run low then free some page cache up now.
        // While this can never guarantee pool allocations will succeed,
        // it does give allocators a better chance.
        //

        FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;
        if (FreePoolInPages < (3 * 1024 * 1024) / PAGE_SIZE) {
            MmPreemptiveTrims[MmPreemptForNonPaged] += 1;
            MiTrimSegmentCache ();
        }

#if defined (_WIN64)
        if (SizeInPages >= _4gb) {
            return NULL;
        }
#endif

        //
        // Try to find system PTEs to expand the pool into.
        //

        StartingPte = MiReserveSystemPtes ((ULONG)SizeInPages,
                                           NonPagedPoolExpansion);

        if (StartingPte == NULL) {

            //
            // There are no free physical PTEs to expand nonpaged pool.
            //
            // Check to see if there are too many unused segments lying
            // around.  If so, set an event so they get deleted.
            //

            if (MI_UNUSED_SEGMENTS_SURPLUS()) {
                KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
            }

            //
            // If there are any cached expansion PTEs, free them now in
            // an attempt to get enough contiguous VA for our caller.
            //

            if ((SizeInPages > 1) && (MmNumberOfFreeNonPagedPool != 0)) {

                if (MiFreeAllExpansionNonPagedPool () == TRUE) {

                    StartingPte = MiReserveSystemPtes ((ULONG)SizeInPages,
                                                       NonPagedPoolExpansion);
                }
            }

            if (StartingPte == NULL) {

                MmPoolFailures[MmNonPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmNonPagedNoPtes] += 1;

                //
                // Running low on pool - force unused segment trimming.
                //
            
                MiTrimSegmentCache ();

                return NULL;
            }
        }

        //
        // See if the allocation can be satisfied from expansion slush ...
        //

        PfnList = NULL;

        if (MiCachedNonPagedPoolCount >= SizeInPages) {

            i = SizeInPages;

            LOCK_PFN2 (OldIrql);

            if (MiCachedNonPagedPoolCount >= i) {
                Pfn1 = MiCachedNonPagedPool;
                PfnList = Pfn1;
                do {
                    ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
                    ASSERT (Pfn1->u2.ShareCount == 1);
                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                    ASSERT (Pfn1->OriginalPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE);
                    ASSERT (Pfn1->u4.MustBeCached == 1);

                    ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);

                    ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                    ASSERT (Pfn1->u4.VerifierAllocation == 0);

                    ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
                    Pfn1 = (PMMPFN) Pfn1->u1.Event;
                    i -= 1;
                } while (i != 0);

                MiCachedNonPagedPool = Pfn1;
                MiCachedNonPagedPoolCount -= SizeInPages;
#if DBG
                if (MiCachedNonPagedPoolCount == 0) {
                    ASSERT (MiCachedNonPagedPool == NULL);
                }
#endif
            }

            UNLOCK_PFN2 (OldIrql);
        }

        //
        // Charge commitment as nonpaged pool uses physical memory.
        //

        if ((PfnList == NULL) &&
            (MiChargeCommitmentCantExpand (SizeInPages, FALSE) == FALSE)) {

            if ((PsGetCurrentThread()->MemoryMaker == 1) &&
                (!KeIsExecutingDpc ())) {

                MiChargeCommitmentCantExpand (SizeInPages, TRUE);
            }
            else {
                MiReleaseSystemPtes (StartingPte,
                                     (ULONG)SizeInPages,
                                     NonPagedPoolExpansion);

                MmPoolFailures[MmNonPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmNonPagedNoCommit] += 1;
                MiTrimSegmentCache ();
                return NULL;
            }
        }

        PointerPte = StartingPte;
        TempPte = ValidKernelPte;
        i = SizeInPages;

        MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

        if (PfnList != NULL) {
            Pfn1 = PfnList;
            ASSERT (PfnList->u3.e1.StartOfAllocation == 0);
            ASSERT (PfnList->u4.VerifierAllocation == 0);
            PfnList->u3.e1.StartOfAllocation = 1;
            if (PoolType & POOL_VERIFIER_MASK) {
                PfnList->u4.VerifierAllocation = 1;
            }
    
            //
            // The lock must be acquired prior to filling any PTEs to
            // prevent a free happening in parallel from mistakenly trying
            // to coalesce with this allocation !
            //

            OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

            do {

                ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                ASSERT (Pfn1->u2.ShareCount == 1);
                ASSERT (Pfn1->OriginalPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE);
                ASSERT (Pfn1->u4.MustBeCached == 1);
                ASSERT ((Pfn1->u3.e1.StartOfAllocation == 0) ||
                        (Pfn1 == PfnList));
                ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);

                ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
                ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

                ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                ASSERT ((Pfn1->u4.VerifierAllocation == 0) ||
                        ((Pfn1 == PfnList) && (PoolType & POOL_VERIFIER_MASK)));

                Pfn1->PteAddress = PointerPte;
                Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPte));
    
                TempPte.u.Hard.PageFrameNumber = Pfn1 - MmPfnDatabase;
                MI_WRITE_VALID_PTE (PointerPte, TempPte);

                SizeInPages -= 1;
                if (SizeInPages == 0) {
                    Pfn1->u3.e1.EndOfAllocation = 1;
                    MmAllocatedNonPagedPool += i;
                    goto AllocationSucceeded;
                }

                Pfn1 = (PMMPFN) Pfn1->u1.Event;
                PointerPte += 1;

            } while (TRUE);
        }
        else {
            OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);
        }

        MmAllocatedNonPagedPool += SizeInPages;

        LOCK_PFN_AT_DPC ();

        //
        // Make sure we have 1 more than the number of pages
        // requested available.
        //

        if (MmAvailablePages <= SizeInPages) {

            UNLOCK_PFN_FROM_DPC ();

            //
            // There are no free physical pages to expand nonpaged pool.
            //

            MmAllocatedNonPagedPool -= SizeInPages;

            KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

            MmPoolFailureReasons[MmNonPagedNoPagesAvailable] += 1;

            MmPoolFailures[MmNonPagedPool][MmHighPriority] += 1;

            MiReturnCommitment (SizeInPages);

            MiReleaseSystemPtes (StartingPte,
                                 (ULONG)SizeInPages,
                                 NonPagedPoolExpansion);

            MiTrimSegmentCache ();

            return NULL;
        }

        //
        // Charge resident available pages now for any excess.
        //

        MiExpansionPoolPagesInUse += SizeInPages;
        if (MiExpansionPoolPagesInUse > MiExpansionPoolPagesInitialCharge) {
            j = MiExpansionPoolPagesInUse - MiExpansionPoolPagesInitialCharge;
            if (j > SizeInPages) {
                j = SizeInPages;
            }
            if (MI_NONPAGEABLE_MEMORY_AVAILABLE() >= (SPFN_NUMBER)j) {
                MI_DECREMENT_RESIDENT_AVAILABLE (j, MM_RESAVAIL_ALLOCATE_EXPANSION_NONPAGED_POOL);
            }
            else {
                MiExpansionPoolPagesInUse -= SizeInPages;
                UNLOCK_PFN_FROM_DPC ();

                MmAllocatedNonPagedPool -= SizeInPages;

                KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

                MmPoolFailureReasons[MmNonPagedNoResidentAvailable] += 1;

                MmPoolFailures[MmNonPagedPool][MmHighPriority] += 1;

                MiReturnCommitment (SizeInPages);

                MiReleaseSystemPtes (StartingPte,
                                    (ULONG)SizeInPages,
                                    NonPagedPoolExpansion);

                MiTrimSegmentCache ();

                return NULL;
            }
        }
    
        MM_TRACK_COMMIT (MM_DBG_COMMIT_NONPAGED_POOL_EXPANSION, SizeInPages);

        //
        // Expand the pool.
        //

        FlushedTb = FALSE;

        do {
            PageFrameIndex = MiRemoveAnyPage (
                                MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u2.ShareCount = 1;
            Pfn1->PteAddress = PointerPte;
            Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
            Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPte));

            Pfn1->u3.e1.PageLocation = ActiveAndValid;

            //
            // The entire TB must be flushed if we are changing cache
            // attributes.
            //
            // KeFlushSingleTb cannot be used because we don't know
            // what virtual address(es) this physical frame was last mapped at.
            //
            // Note we can skip the flush if we've already done it once in
            // this loop already because the PFN lock is held throughout.
            //

            if ((Pfn1->u3.e1.CacheAttribute != MiCached) && (FlushedTb == FALSE)) {
                MI_FLUSH_TB_FOR_INDIVIDUAL_ATTRIBUTE_CHANGE (PageFrameIndex,
                                                             MiCached);
                FlushedTb = TRUE;
            }

            Pfn1->u3.e1.CacheAttribute = MiCached;
            Pfn1->u3.e1.LargeSessionAllocation = 0;
            Pfn1->u4.VerifierAllocation = 0;

            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPte, TempPte);
            PointerPte += 1;
            SizeInPages -= 1;
        } while (SizeInPages > 0);

        Pfn1->u3.e1.EndOfAllocation = 1;

        Pfn1 = MI_PFN_ELEMENT (StartingPte->u.Hard.PageFrameNumber);
        Pfn1->u3.e1.StartOfAllocation = 1;

        ASSERT (Pfn1->u4.VerifierAllocation == 0);

        if (PoolType & POOL_VERIFIER_MASK) {
            Pfn1->u4.VerifierAllocation = 1;
        }

        UNLOCK_PFN_FROM_DPC ();

AllocationSucceeded:

        FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

        if (FreePoolInPages < MiHighNonPagedPoolThreshold) {

            //
            // Read the state directly instead of calling
            // KeReadStateEvent since we are holding the nonpaged
            // pool lock and want to keep instructions at a
            // minimum.
            //

            if (MiHighNonPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiHighNonPagedPoolEvent);
            }
            if (FreePoolInPages <= MiLowNonPagedPoolThreshold) {
                if (MiLowNonPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiLowNonPagedPoolEvent, 0, FALSE);
                }
            }
        }

        KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

        BaseVa = MiGetVirtualAddressMappedByPte (StartingPte);

        return BaseVa;
    }

    //
    // Paged Pool.
    //

    if ((PoolType & SESSION_POOL_MASK) == 0) {

        //
        // If pool is starting to run low then free some page cache up now.
        // While this can never guarantee pool allocations will succeed,
        // it does give allocators a better chance.
        //

        FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

        if (FreePoolInPages < (5 * 1024 * 1024) / PAGE_SIZE) {
            MmPreemptiveTrims[MmPreemptForPaged] += 1;
            MiTrimSegmentCache ();
        }
#if DBG
        if (MiClearCache != 0) {
            LARGE_INTEGER CurrentTime;

            KeQueryTickCount(&CurrentTime);

            if ((CurrentTime.LowPart & MiClearCache) == 0) {

                MmPreemptiveTrims[MmPreemptForPaged] += 1;
                MiTrimSegmentCache ();
            }
        }
#endif

        if ((SizeInPages == 1) &&
            (ExQueryDepthSList (&MiPagedPoolSListHead) != 0)) {

            BaseVa = InterlockedPopEntrySList (&MiPagedPoolSListHead);

            if (BaseVa != NULL) {
                return BaseVa;
            }
        }

        SessionSpace = NULL;
        PagedPoolInfo = &MmPagedPoolInfo;

        KeAcquireGuardedMutex (&MmPagedPoolMutex);
    }
    else {
        SessionSpace = SESSION_GLOBAL (MmSessionSpace);
        PagedPoolInfo = &SessionSpace->PagedPoolInfo;

        KeAcquireGuardedMutex (&SessionSpace->PagedPoolMutex);
    }

    StartPosition = RtlFindClearBitsAndSet (
                               PagedPoolInfo->PagedPoolAllocationMap,
                               (ULONG)SizeInPages,
                               PagedPoolInfo->PagedPoolHint);

    if (StartPosition == NO_BITS_FOUND) {

        //
        // No room in pool - attempt to expand the paged pool.  If there is
        // a surplus of unused segments (or subsections) get rid of some now.
        //

        if (MI_UNUSED_SEGMENTS_SURPLUS()) {
            KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
        }

        StartPosition = (((ULONG)SizeInPages - 1) / PTE_PER_PAGE) + 1;

        //
        // Make sure there is enough space to create at least some
        // page table pages.  Note if we can create even one it's worth
        // doing as there may be free space in the already existing pool
        // (at the end) - and this can be concatenated with the expanded
        // portion below into one big allocation.
        //

        if (PagedPoolInfo->NextPdeForPagedPoolExpansion >
            MiGetPteAddress (PagedPoolInfo->LastPteForPagedPool)) {

NoVaSpaceLeft:

            //
            // Can't expand pool any more.  If this request is not for session
            // pool, force unused segment trimming when appropriate.
            //

            if (SessionSpace == NULL) {

                KeReleaseGuardedMutex (&MmPagedPoolMutex);

                MmPoolFailures[MmPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmPagedNoPtes] += 1;

                //
                // Running low on pool - force unused segment trimming.
                //
            
                MiTrimSegmentCache ();

                return NULL;
            }

            KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);

            MmPoolFailures[MmSessionPagedPool][MmHighPriority] += 1;
            MmPoolFailureReasons[MmSessionPagedNoPtes] += 1;

            SessionSpace->SessionPoolAllocationFailures[1] += 1;

            return NULL;
        }

        if (((StartPosition - 1) + PagedPoolInfo->NextPdeForPagedPoolExpansion) >
            MiGetPteAddress (PagedPoolInfo->LastPteForPagedPool)) {

            PageTableCount = (ULONG)(MiGetPteAddress (PagedPoolInfo->LastPteForPagedPool) - PagedPoolInfo->NextPdeForPagedPoolExpansion + 1);
            ASSERT (PageTableCount < StartPosition);
            StartPosition = PageTableCount;
        }
        else {
            PageTableCount = StartPosition;
        }

        if (SessionSpace) {
            TempPte = ValidKernelPdeLocal;
        }
        else {
            TempPte = ValidKernelPde;
        }

        //
        // Charge commitment for the pagetable pages for paged pool expansion.
        //

        if (MiChargeCommitmentCantExpand (PageTableCount, FALSE) == FALSE) {

            if ((PsGetCurrentThread()->MemoryMaker == 1) &&
                (!KeIsExecutingDpc ())) {

                MiChargeCommitmentCantExpand (PageTableCount, TRUE);
            }
            else {
                if (SessionSpace) {
                    KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);
                }
                else {
                    KeReleaseGuardedMutex (&MmPagedPoolMutex);
                }
                MmPoolFailures[MmPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmPagedNoCommit] += 1;
                MiTrimSegmentCache ();

                return NULL;
            }
        }

        EndPosition = (ULONG)((PagedPoolInfo->NextPdeForPagedPoolExpansion -
                          MiGetPteAddress(PagedPoolInfo->FirstPteForPagedPool)) *
                          PTE_PER_PAGE);

        //
        // Expand the pool.
        //

        PointerPte = PagedPoolInfo->NextPdeForPagedPoolExpansion;
        VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
        VirtualAddressSave = VirtualAddress;

        LOCK_PFN (OldIrql);

        //
        // Make sure we have 1 more than the number of pages
        // requested available.
        //

        if (MmAvailablePages <= PageTableCount) {

            //
            // There are no free physical pages to expand paged pool.
            //

            UNLOCK_PFN (OldIrql);

            if (SessionSpace == NULL) {
                KeReleaseGuardedMutex (&MmPagedPoolMutex);
                MmPoolFailures[MmPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmPagedNoPagesAvailable] += 1;
            }
            else {
                KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);
                MmPoolFailures[MmSessionPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmSessionPagedNoPagesAvailable] += 1;
                SessionSpace->SessionPoolAllocationFailures[2] += 1;
            }

            MiReturnCommitment (PageTableCount);

            return NULL;
        }

        MM_TRACK_COMMIT (MM_DBG_COMMIT_PAGED_POOL_PAGETABLE, PageTableCount);

        //
        // Update the count of available resident pages.
        //

        MI_DECREMENT_RESIDENT_AVAILABLE (PageTableCount, MM_RESAVAIL_ALLOCATE_PAGETABLES_FOR_PAGED_POOL);

        //
        // Allocate the page table pages for the pool expansion.
        //

        do {
            ASSERT (PointerPte->u.Hard.Valid == 0);

            PageFrameIndex = MiRemoveAnyPage (
                                MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

            //
            // Map valid PDE into system (or session) address space.
            //

#if (_MI_PAGING_LEVELS >= 3)

            MiInitializePfnAndMakePteValid (PageFrameIndex, PointerPte, TempPte);

#else

            if (SessionSpace) {

                Index = (ULONG)(PointerPte - MiGetPdeAddress (MmSessionBase));
                ASSERT (SessionSpace->PageTables[Index].u.Long == 0);
                SessionSpace->PageTables[Index] = TempPte;

                MiInitializePfnForOtherProcess (PageFrameIndex,
                                                PointerPte,
                                                SessionSpace->SessionPageDirectoryIndex);

                MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC1, 1);
            }
            else {
                MmSystemPagePtes [((ULONG_PTR)PointerPte &
                    (PD_PER_SYSTEM * (sizeof(MMPTE) * PDE_PER_PAGE) - 1)) / sizeof(MMPTE)] = TempPte;
                MiInitializePfnForOtherProcess (PageFrameIndex,
                                                PointerPte,
                                                MmSystemPageDirectory[(PointerPte - MiGetPdeAddress(0)) / PDE_PER_PAGE]);
            }

            MI_WRITE_VALID_PTE (PointerPte, TempPte);
#endif

            PointerPte += 1;
            VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);
            StartPosition -= 1;

        } while (StartPosition > 0);

        UNLOCK_PFN (OldIrql);

        //
        // Clear the bitmap locations for the expansion area to indicate it
        // is available for consumption.
        //

        RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap,
                      EndPosition,
                      (ULONG) PageTableCount * PTE_PER_PAGE);

        //
        // Denote where to start the next pool expansion.
        //

        PagedPoolInfo->NextPdeForPagedPoolExpansion += PageTableCount;

        //
        // Mark the PTEs for the expanded pool no-access.
        //

        MiZeroMemoryPte (VirtualAddressSave,
                         PageTableCount * (PAGE_SIZE / sizeof (MMPTE)));

        if (SessionSpace) {

            InterlockedExchangeAddSizeT (&SessionSpace->CommittedPages,
                                         PageTableCount);

            MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_PAGETABLE_ALLOC, PageTableCount);
            InterlockedExchangeAddSizeT (&SessionSpace->NonPageablePages,
                                         PageTableCount);
        }

        //
        // Start searching from the beginning of the bitmap as we may be
        // able to coalesce an earlier entry and only use part of the expansion
        // we just did.  This is not only important to reduce fragmentation but
        // in fact, required for the case where we could not expand enough
        // to cover the entire allocation and thus, must coalesce backwards
        // in order to satisfy the request.
        //

        StartPosition = RtlFindClearBitsAndSet (
                                   PagedPoolInfo->PagedPoolAllocationMap,
                                   (ULONG)SizeInPages,
                                   0);

        if (StartPosition == NO_BITS_FOUND) {
            goto NoVaSpaceLeft;
        }
    }

    //
    // This is paged pool, the start and end can't be saved
    // in the PFN database as the page isn't always resident
    // in memory.  The ideal place to save the start and end
    // would be in the prototype PTE, but there are no free
    // bits.  To solve this problem, a bitmap which parallels
    // the allocation bitmap exists which contains set bits
    // in the positions where an allocation ends.  This
    // allows pages to be deallocated with only their starting
    // address.
    //
    // For sanity's sake, the starting address can be verified
    // from the 2 bitmaps as well.  If the page before the starting
    // address is not allocated (bit is zero in allocation bitmap)
    // then this page is obviously a start of an allocation block.
    // If the page before is allocated and the other bit map does
    // not indicate the previous page is the end of an allocation,
    // then the starting address is wrong and a bugcheck should
    // be issued.
    //

    if (SizeInPages == 1) {
        PagedPoolInfo->PagedPoolHint = StartPosition + (ULONG)SizeInPages;
    }

    //
    // If paged pool has been configured as non-pageable, commitment has
    // already been charged so just set the length and return the address.
    //

    if ((MmDisablePagingExecutive & MM_PAGED_POOL_LOCKED_DOWN) &&
        (SessionSpace == NULL)) {

        BaseVa = (PVOID)((PCHAR) MmPagedPoolStart + ((ULONG_PTR)StartPosition << PAGE_SHIFT));

#if DBG
        PointerPte = MiGetPteAddress (BaseVa);
        for (i = 0; i < SizeInPages; i += 1) {
            ASSERT (PointerPte->u.Hard.Valid == 1);
            PointerPte += 1;
        }
#endif
    
        EndPosition = StartPosition + (ULONG)SizeInPages - 1;
        RtlSetBit (PagedPoolInfo->EndOfPagedPoolBitmap, EndPosition);
    
        if (PoolType & POOL_VERIFIER_MASK) {
            RtlSetBit (VerifierLargePagedPoolMap, StartPosition);
        }
    
        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     SizeInPages);
    
        FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

        if (FreePoolInPages < MiHighPagedPoolThreshold) {

            //
            // Read the state directly instead of calling
            // KeReadStateEvent since we are holding the paged
            // pool mutex and want to keep instructions at a
            // minimum.
            //

            if (MiHighPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiHighPagedPoolEvent);
            }
            if (FreePoolInPages <= MiLowPagedPoolThreshold) {
                if (MiLowPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiLowPagedPoolEvent, 0, FALSE);
                }
            }
        }

        KeReleaseGuardedMutex (&MmPagedPoolMutex);

        return BaseVa;
    }

    if (MiChargeCommitmentCantExpand (SizeInPages, FALSE) == FALSE) {

        if ((PsGetCurrentThread()->MemoryMaker == 1) &&
            (!KeIsExecutingDpc ())) {

            MiChargeCommitmentCantExpand (SizeInPages, TRUE);
        }
        else {
            RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap,
                          StartPosition,
                          (ULONG)SizeInPages);
    
            //
            // Could not commit the page(s), return NULL indicating
            // no pool was allocated.  Note that the lack of commit may be due
            // to unused segments and the MmSharedCommit, prototype PTEs, etc
            // associated with them.  So force a reduction now.
            //
    
            if (SessionSpace == NULL) {
                KeReleaseGuardedMutex (&MmPagedPoolMutex);

                MmPoolFailures[MmPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmPagedNoCommit] += 1;
            }
            else {
                KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);

                MmPoolFailures[MmSessionPagedPool][MmHighPriority] += 1;
                MmPoolFailureReasons[MmSessionPagedNoCommit] += 1;
                SessionSpace->SessionPoolAllocationFailures[3] += 1;
            }

            MiIssuePageExtendRequestNoWait (SizeInPages);

            MiTrimSegmentCache ();

            return NULL;
        }
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_PAGED_POOL_PAGES, SizeInPages);

    EndPosition = StartPosition + (ULONG)SizeInPages - 1;
    RtlSetBit (PagedPoolInfo->EndOfPagedPoolBitmap, EndPosition);

    if (SessionSpace) {
        KeReleaseGuardedMutex (&SessionSpace->PagedPoolMutex);
        InterlockedExchangeAddSizeT (&SessionSpace->CommittedPages,
                                     SizeInPages);
        MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_COMMIT_PAGEDPOOL_PAGES, (ULONG)SizeInPages);
        BaseVa = (PVOID)((PCHAR)SessionSpace->PagedPoolStart +
                                ((ULONG_PTR)StartPosition << PAGE_SHIFT));

        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     SizeInPages);
    }
    else {
        if (PoolType & POOL_VERIFIER_MASK) {
            RtlSetBit (VerifierLargePagedPoolMap, StartPosition);
        }

        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     SizeInPages);

        FreePoolInPages = MmSizeOfPagedPoolInPages - PagedPoolInfo->AllocatedPagedPool;

        if (FreePoolInPages < MiHighPagedPoolThreshold) {

            //
            // Read the state directly instead of calling
            // KeReadStateEvent since we are holding the paged
            // pool mutex and want to keep instructions at a
            // minimum.
            //

            if (MiHighPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiHighPagedPoolEvent);
            }
            if (FreePoolInPages <= MiLowPagedPoolThreshold) {
                if (MiLowPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiLowPagedPoolEvent, 0, FALSE);
                }
            }
        }

        KeReleaseGuardedMutex (&MmPagedPoolMutex);
        InterlockedExchangeAdd ((PLONG) &MmPagedPoolCommit, (LONG)SizeInPages);
        BaseVa = (PVOID)((PCHAR) MmPagedPoolStart +
                                ((ULONG_PTR)StartPosition << PAGE_SHIFT));
    }

    InterlockedExchangeAddSizeT (&PagedPoolInfo->PagedPoolCommit,
                                 SizeInPages);

    //
    // Carefully check the TB flush time stamps to decide if a flush is needed.
    //

    FlushCount = 0;
    LastTimeStamp = 0;
    PreviousPteNeededFlush = FALSE;
    PointerPte = MiGetPteAddress (BaseVa);

    for (i = 0; i < SizeInPages; i += 1, PointerPte += 1) {
        ASSERT (PointerPte->u.Hard.Valid == 0);

        TimeStamp = (ULONG) PointerPte->u.Soft.PageFileHigh;

        if (TimeStamp == 0) {

            //
            // This entry has already been flushed.
            //

            PreviousPteNeededFlush = FALSE;
            LastTimeStamp = TimeStamp;
            continue;
        }

        if (TimeStamp == LastTimeStamp) {

            if (PreviousPteNeededFlush == TRUE) {

                //
                // If this entry is the same as the prior one then it must be
                // treated the same with respect to flushes.
                //

                VaFlushList[FlushCount] = MiGetVirtualAddressMappedByPte (PointerPte);
                FlushCount += 1;
                if (FlushCount == MM_MAXIMUM_FLUSH_COUNT) {
                    FlushCount = 0;
                    MI_FLUSH_ENTIRE_TB (0x1E);
                    break;
                }
            }

            continue;
        }

        if (MiCompareTbFlushTimeStamp (TimeStamp, MI_PTE_LOOKUP_NEEDED)) {

            VaFlushList[FlushCount] = MiGetVirtualAddressMappedByPte (PointerPte);
            FlushCount += 1;
            if (FlushCount == MM_MAXIMUM_FLUSH_COUNT) {
                FlushCount = 0;
                MI_FLUSH_ENTIRE_TB (0x1E);
                break;
            }
            PreviousPteNeededFlush = TRUE;
        }
        else {
            PreviousPteNeededFlush = FALSE;
        }

        LastTimeStamp = TimeStamp;
    }

    //
    // Flush the TB entries for any relevant pages.
    //

    if (FlushCount == 0) {
        NOTHING;
    }
    else if (FlushCount == 1) {
        MI_FLUSH_SINGLE_TB (VaFlushList[0], TRUE);
    }
    else {
        ASSERT (FlushCount < MM_MAXIMUM_FLUSH_COUNT);
        MI_FLUSH_MULTIPLE_TB (FlushCount, &VaFlushList[0], TRUE);
    }

    TempPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;

    MI_ADD_EXECUTE_TO_INVALID_PTE_IF_PAE (TempPte);

    PointerPte = MiGetPteAddress (BaseVa);

    StartingPte = PointerPte + SizeInPages;

    //
    // Fill the PTEs inline instead of using MiFillMemoryPte because on
    // most platforms MiFillMemoryPte degrades to a function call and 
    // typically only a small number of PTEs are filled here.
    //

    do {
        MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        PointerPte += 1;
    } while (PointerPte < StartingPte);

    return BaseVa;
}

ULONG
MiFreePoolPages (
    IN PVOID StartingAddress
    )

/*++

Routine Description:

    This function returns a set of pages back to the pool from
    which they were obtained.  Once the pages have been deallocated
    the region provided by the allocation becomes available for
    allocation to other callers, i.e. any data in the region is now
    trashed and cannot be referenced.

Arguments:

    StartingAddress - Supplies the starting address which was returned
                      in a previous call to MiAllocatePoolPages.

Return Value:

    Returns the number of pages deallocated.

Environment:

    These functions are used by the general pool allocation routines
    and should not be called directly.

--*/

{
    PMMPFN PfnList;
    KIRQL OldIrql;
    KIRQL PfnIrql;
    ULONG StartPosition;
    ULONG Index;
    PFN_NUMBER i;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER PageFrameIndex;
    PMMPTE PointerPte;
    PMMPTE StartPte;
    PMMPFN Pfn1;
    PMMPFN StartPfn;
    PMMFREE_POOL_ENTRY Entry;
    PMMFREE_POOL_ENTRY NextEntry;
    PMMFREE_POOL_ENTRY LastEntry;
    PMM_PAGED_POOL_INFO PagedPoolInfo;
    PMM_SESSION_SPACE SessionSpace;
    PFN_NUMBER PagesFreed;
    MMPFNENTRY OriginalPfnFlags;
    ULONG_PTR VerifierAllocation;
    PULONG BitMap;
    PKGUARDED_MUTEX PoolMutex;
    PFN_NUMBER FreePoolInPages;
#if DBG
    PMMPTE DebugPte;
    PMMPFN DebugPfn;
    PMMPFN LastDebugPfn;
#endif

    //
    // Determine pool type based on the virtual address of the block
    // to deallocate.
    //
    // This assumes paged pool is virtually contiguous.
    //

    if ((StartingAddress >= MmPagedPoolStart) &&
        (StartingAddress <= MmPagedPoolEnd)) {
        SessionSpace = NULL;
        PagedPoolInfo = &MmPagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)MmPagedPoolStart) >> PAGE_SHIFT);
        PoolMutex = &MmPagedPoolMutex;
    }
    else if (MI_IS_SESSION_POOL_ADDRESS (StartingAddress) == TRUE) {
        SessionSpace = SESSION_GLOBAL (MmSessionSpace);
        ASSERT (SessionSpace);
        PagedPoolInfo = &SessionSpace->PagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)SessionSpace->PagedPoolStart) >> PAGE_SHIFT);
        PoolMutex = &SessionSpace->PagedPoolMutex;
    }
    else {

        if (StartingAddress < MM_SYSTEM_RANGE_START) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x40,
                          (ULONG_PTR)StartingAddress,
                          (ULONG_PTR)MM_SYSTEM_RANGE_START,
                          0);
        }

        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR) MmNonPagedPoolStart) >> PAGE_SHIFT);

        //
        // Check to ensure this page is really the start of an allocation.
        //

        if (MI_IS_PHYSICAL_ADDRESS (StartingAddress)) {

            //
            // On certain architectures, virtual addresses
            // may be physical and hence have no corresponding PTE.
            //

            PointerPte = NULL;
            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (StartingAddress));
            ASSERT (StartPosition < (ULONG) BYTES_TO_PAGES (MmSizeOfNonPagedPoolInBytes));

            if ((StartingAddress < MmNonPagedPoolStart) ||
                (StartingAddress >= MmNonPagedPoolEnd0)) {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0x42,
                              (ULONG_PTR)StartingAddress,
                              0,
                              0);
            }
        }
        else {
            PointerPte = MiGetPteAddress (StartingAddress);

            if (((StartingAddress >= MmNonPagedPoolExpansionStart) &&
                (StartingAddress < MmNonPagedPoolEnd)) ||
                ((StartingAddress >= MmNonPagedPoolStart) &&
                (StartingAddress < MmNonPagedPoolEnd0))) {
                    NOTHING;
            }
            else {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0x43,
                              (ULONG_PTR)StartingAddress,
                              0,
                              0);
            }
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
        }

        if (Pfn1->u3.e1.StartOfAllocation == 0) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x41,
                          (ULONG_PTR) StartingAddress,
                          (ULONG_PTR) MI_PFN_ELEMENT_TO_INDEX (Pfn1),
                          MmHighestPhysicalPage);
        }

        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

        //
        // Hang single page allocations off our slist header.
        //

        if ((Pfn1->u3.e1.EndOfAllocation == 1) &&
            (Pfn1->u4.VerifierAllocation == 0) &&
            (Pfn1->u3.e1.LargeSessionAllocation == 0) &&
            (ExQueryDepthSList (&MiNonPagedPoolSListHead) < MiNonPagedPoolSListMaximum)) {
            InterlockedPushEntrySList (&MiNonPagedPoolSListHead,
                                       (PSLIST_ENTRY) StartingAddress);
            return 1;
        }

        //
        // The nonpaged pool being freed may be the target of a delayed unlock.
        // Since these pages may be immediately released, force any pending
        // delayed actions to occur now.
        //

#if !defined(MI_MULTINODE)
        if (MmPfnDeferredList != NULL) {
            MiDeferredUnlockPages (0);
        }
#else
        //
        // Each and every node's deferred list would have to be checked so
        // we might as well go the long way and just call.
        //

        MiDeferredUnlockPages (0);
#endif

        StartPfn = Pfn1;

        OriginalPfnFlags = Pfn1->u3.e1;
        VerifierAllocation = Pfn1->u4.VerifierAllocation;

        //
        // ASSERT to ensure the pool being freed is not still in use
        // for an I/O.
        //

        if ((Pfn1->u3.e2.ReferenceCount == 0) ||
            ((Pfn1->u3.e2.ReferenceCount > 1) &&
             (Pfn1->u3.e1.WriteInProgress == 0))) {

            MiBadRefCount (Pfn1);
        }

        //
        // Find end of allocation and release the pages.
        //

        if (PointerPte == NULL) {
            SATISFY_OVERZEALOUS_COMPILER (StartPte = NULL);
            while (Pfn1->u3.e1.EndOfAllocation == 0) {
                Pfn1 += 1;

                //
                // ASSERT to ensure the pool being freed is not still in use
                // for an I/O.
                //

                ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
                ASSERT ((Pfn1->u3.e2.ReferenceCount == 1) ||
                        (Pfn1->u3.e1.WriteInProgress == 1));
            }
            NumberOfPages = Pfn1 - StartPfn + 1;
        }
        else {
            StartPte = PointerPte;
            while (Pfn1->u3.e1.EndOfAllocation == 0) {
                PointerPte += 1;
                Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

                //
                // ASSERT to ensure the pool being freed is not still in use
                // for an I/O.
                //

                ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
                ASSERT ((Pfn1->u3.e2.ReferenceCount == 1) ||
                        (Pfn1->u3.e1.WriteInProgress == 1));
            }
            NumberOfPages = PointerPte - StartPte + 1;
        }

        if (VerifierAllocation != 0) {
            VerifierFreeTrackedPool (StartingAddress,
                                     NumberOfPages << PAGE_SHIFT,
                                     NonPagedPool,
                                     FALSE);
        }

#if DBG
        if (MiFillFreedPool != 0) {
            RtlFillMemoryUlong (StartingAddress,
                                PAGE_SIZE * NumberOfPages,
                                MiFillFreedPool);
        }
#endif

        OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

        StartPfn->u3.e1.StartOfAllocation = 0;
        StartPfn->u3.e1.LargeSessionAllocation = 0;
        StartPfn->u4.VerifierAllocation = 0;

        MmAllocatedNonPagedPool -= NumberOfPages;

        FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

        if (FreePoolInPages > MiLowNonPagedPoolThreshold) {

            //
            // Read the state directly instead of calling
            // KeReadStateEvent since we are holding the nonpaged
            // pool lock and want to keep instructions at a
            // minimum.
            //

            if (MiLowNonPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiLowNonPagedPoolEvent);
            }
            if (FreePoolInPages >= MiHighNonPagedPoolThreshold) {
                if (MiHighNonPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiHighNonPagedPoolEvent, 0, FALSE);
                }
            }
        }

        Pfn1->u3.e1.EndOfAllocation = 0;

        if (StartingAddress >= MmNonPagedPoolExpansionStart) {

            //
            // This page was from the expanded pool, should
            // it be freed?
            //
            // NOTE: all pages in the expanded pool area have PTEs
            // so no physical address checks need to be performed.
            //

            if (Pfn1->u4.MustBeCached == 1) {

                //
                // This is a slush page that is concurrently mapped
                // (cached) within the kernel or HAL's large page mapping.
                // To prevent a possible TB attribute conflict, these pages
                // are never put on the general purpose page free lists.
                // Instead they are cached here for nonpaged pool usage only.
                //

                PfnList = NULL;
                PointerPte = StartPte;

                //
                // Hold the nonpaged pool lock until all the PTEs are cleared
                // so another thread trying to coalesce will see consistent
                // values.
                //

                for (i = 0; i < NumberOfPages; i += 1, PointerPte += 1) {

                    ASSERT (PointerPte->u.Hard.Valid == 1);
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                    ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
                    ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                    ASSERT (Pfn1->u2.ShareCount == 1);
                    ASSERT (Pfn1->OriginalPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE);
                    ASSERT (Pfn1->u4.MustBeCached == 1);
                    ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
                    ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);
                    ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                    ASSERT (Pfn1->u4.VerifierAllocation == 0);

                    Pfn1->u1.Event = (PKEVENT) PfnList;
                    PfnList = Pfn1;
                }

                LOCK_PFN2 (PfnIrql);
                StartPfn->u1.Event = (PKEVENT) MiCachedNonPagedPool;
                MiCachedNonPagedPool = PfnList;
                MiCachedNonPagedPoolCount += NumberOfPages;
                UNLOCK_PFN2 (PfnIrql);

                MiReleaseSystemPtes (StartPte,
                                     (ULONG)NumberOfPages,
                                     NonPagedPoolExpansion);

                KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

                //
                // No TB flush is done here - the system PTE management code
                // will lazily flush as needed.
                //

                return (ULONG)NumberOfPages;
            }

            if ((NumberOfPages > 3) ||
                (MmNumberOfFreeNonPagedPool > MmFreedExpansionPoolMaximum) ||
                ((MmResidentAvailablePages < 200) &&
                 (MiExpansionPoolPagesInUse > MiExpansionPoolPagesInitialCharge))) {

                //
                // Free these pages back to the free page list.
                //

                MiFreeNonPagedPool (StartingAddress, NumberOfPages);

                KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

                return (ULONG)NumberOfPages;
            }
        }

        //
        // Add the pages to the list of free pages.
        //

        MmNumberOfFreeNonPagedPool += NumberOfPages;

        //
        // Check to see if the next allocation is free.
        // We cannot walk off the end of nonpaged expansion
        // pages as the highest expansion allocation is always
        // virtual and guard-paged.
        //

        i = NumberOfPages;

        ASSERT (MiEndOfInitialPoolFrame != 0);

        if (MI_PFN_ELEMENT_TO_INDEX (Pfn1) == MiEndOfInitialPoolFrame) {
            PointerPte += 1;
            Pfn1 = NULL;
        }
        else if (PointerPte == NULL) {
            Pfn1 += 1;
            ASSERT ((PCHAR)StartingAddress + NumberOfPages < (PCHAR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes);
        }
        else {
            PointerPte += 1;
            ASSERT ((PCHAR)StartingAddress + NumberOfPages <= (PCHAR)MmNonPagedPoolEnd);

            //
            // Unprotect the previously freed pool so it can be merged.
            //

            if (MmProtectFreedNonPagedPool == TRUE) {
                MiUnProtectFreeNonPagedPool (
                    (PVOID)MiGetVirtualAddressMappedByPte(PointerPte),
                    0);
            }

            if (PointerPte->u.Hard.Valid == 1) {
                Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            }
            else {
                Pfn1 = NULL;
            }
        }

        if ((Pfn1 != NULL) && (Pfn1->u3.e1.StartOfAllocation == 0)) {

            //
            // This range of pages is free.  Remove this entry
            // from the list and add these pages to the current
            // range being freed.
            //

            Entry = (PMMFREE_POOL_ENTRY)((PCHAR)StartingAddress
                                        + (NumberOfPages << PAGE_SHIFT));
            ASSERT (Entry->Signature == MM_FREE_POOL_SIGNATURE);
            ASSERT (Entry->Owner == Entry);

#if DBG
            if (PointerPte == NULL) {

                ASSERT (MI_IS_PHYSICAL_ADDRESS(StartingAddress));

                //
                // On certain architectures, virtual addresses
                // may be physical and hence have no corresponding PTE.
                //

                DebugPfn = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (Entry));
                DebugPfn += Entry->Size;
                if (MI_PFN_ELEMENT_TO_INDEX (DebugPfn - 1) != MiEndOfInitialPoolFrame) {
                    ASSERT (DebugPfn->u3.e1.StartOfAllocation == 1);
                }
            }
            else {
                DebugPte = PointerPte + Entry->Size;
                if ((DebugPte-1)->u.Hard.Valid == 1) {
                    DebugPfn = MI_PFN_ELEMENT ((DebugPte-1)->u.Hard.PageFrameNumber);
                    if (MI_PFN_ELEMENT_TO_INDEX (DebugPfn) != MiEndOfInitialPoolFrame) {
                        if (DebugPte->u.Hard.Valid == 1) {
                            DebugPfn = MI_PFN_ELEMENT (DebugPte->u.Hard.PageFrameNumber);
                            ASSERT (DebugPfn->u3.e1.StartOfAllocation == 1);
                        }
                    }

                }
            }
#endif

            i += Entry->Size;
            if (MmProtectFreedNonPagedPool == FALSE) {
                RemoveEntryList (&Entry->List);
            }
            else {
                MiProtectedPoolRemoveEntryList (&Entry->List);
            }
        }

        //
        // Check to see if the previous page is the end of an allocation.
        // If it is not the end of an allocation, it must be free and
        // therefore this allocation can be tagged onto the end of
        // that allocation.
        //
        // We cannot walk off the beginning of expansion pool because it is
        // guard-paged.  If the initial pool is superpaged instead, we are also
        // safe as the must succeed pages always have EndOfAllocation set.
        //

        Entry = (PMMFREE_POOL_ENTRY)StartingAddress;

        ASSERT (MiStartOfInitialPoolFrame != 0);

        if (MI_PFN_ELEMENT_TO_INDEX (StartPfn) == MiStartOfInitialPoolFrame) {
            Pfn1 = NULL;
        }
        else if (PointerPte == NULL) {
            ASSERT (MI_IS_PHYSICAL_ADDRESS(StartingAddress));
            ASSERT (StartingAddress != MmNonPagedPoolStart);

            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (
                                    (PVOID)((PCHAR)Entry - PAGE_SIZE)));

        }
        else {
            PointerPte -= NumberOfPages + 1;

            //
            // Unprotect the previously freed pool so it can be merged.
            //

            if (MmProtectFreedNonPagedPool == TRUE) {
                MiUnProtectFreeNonPagedPool (
                    (PVOID)MiGetVirtualAddressMappedByPte(PointerPte),
                    0);
            }

            if (PointerPte->u.Hard.Valid == 1) {
                Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            }
            else {
                Pfn1 = NULL;
            }
        }
        if (Pfn1 != NULL) {
            if (Pfn1->u3.e1.EndOfAllocation == 0) {

                //
                // This range of pages is free, add these pages to
                // this entry.  The owner field points to the address
                // of the list entry which is linked into the free pool
                // pages list.
                //

                Entry = (PMMFREE_POOL_ENTRY)((PCHAR)StartingAddress - PAGE_SIZE);
                ASSERT (Entry->Signature == MM_FREE_POOL_SIGNATURE);
                Entry = Entry->Owner;

                //
                // Unprotect the previously freed pool so we can merge it
                //

                if (MmProtectFreedNonPagedPool == TRUE) {
                    MiUnProtectFreeNonPagedPool ((PVOID)Entry, 0);
                }

                //
                // If this entry became larger than MM_SMALL_ALLOCATIONS
                // pages, move it to the tail of the list.  This keeps the
                // small allocations at the front of the list.
                //

                if (Entry->Size < MI_MAX_FREE_LIST_HEADS - 1) {

                    if (MmProtectFreedNonPagedPool == FALSE) {
                        RemoveEntryList (&Entry->List);
                    }
                    else {
                        MiProtectedPoolRemoveEntryList (&Entry->List);
                    }

                    //
                    // Add these pages to the previous entry.
                    //
    
                    Entry->Size += i;

                    Index = (ULONG)(Entry->Size - 1);
            
                    if (Index >= MI_MAX_FREE_LIST_HEADS) {
                        Index = MI_MAX_FREE_LIST_HEADS - 1;
                    }

                    if (MmProtectFreedNonPagedPool == FALSE) {
                        InsertTailList (&MmNonPagedPoolFreeListHead[Index],
                                        &Entry->List);
                    }
                    else {
                        MiProtectedPoolInsertList (&MmNonPagedPoolFreeListHead[Index],
                                          &Entry->List,
                                          Entry->Size < MM_SMALL_ALLOCATIONS ?
                                              TRUE : FALSE);
                    }
                }
                else {

                    //
                    // Add these pages to the previous entry.
                    //
    
                    Entry->Size += i;
                }
            }
        }

        if (Entry == (PMMFREE_POOL_ENTRY)StartingAddress) {

            //
            // This entry was not combined with the previous, insert it
            // into the list.
            //

            Entry->Size = i;

            Index = (ULONG)(Entry->Size - 1);
    
            if (Index >= MI_MAX_FREE_LIST_HEADS) {
                Index = MI_MAX_FREE_LIST_HEADS - 1;
            }

            if (MmProtectFreedNonPagedPool == FALSE) {
                InsertTailList (&MmNonPagedPoolFreeListHead[Index],
                                &Entry->List);
            }
            else {
                MiProtectedPoolInsertList (&MmNonPagedPoolFreeListHead[Index],
                                      &Entry->List,
                                      Entry->Size < MM_SMALL_ALLOCATIONS ?
                                          TRUE : FALSE);
            }
        }

        //
        // Set the owner field in all these pages.
        //

        ASSERT (i != 0);
        NextEntry = (PMMFREE_POOL_ENTRY)StartingAddress;
        LastEntry = (PMMFREE_POOL_ENTRY)((PCHAR)NextEntry + (i << PAGE_SHIFT));

        do {
            NextEntry->Owner = Entry;
#if DBG
            NextEntry->Signature = MM_FREE_POOL_SIGNATURE;
#endif

            NextEntry = (PMMFREE_POOL_ENTRY)((PCHAR)NextEntry + PAGE_SIZE);
        } while (NextEntry != LastEntry);

#if DBG
        NextEntry = Entry;

        if (PointerPte == NULL) {
            ASSERT (MI_IS_PHYSICAL_ADDRESS(StartingAddress));
            DebugPfn = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (NextEntry));
            LastDebugPfn = DebugPfn + Entry->Size;

            for ( ; DebugPfn < LastDebugPfn; DebugPfn += 1) {
                ASSERT ((DebugPfn->u3.e1.StartOfAllocation == 0) &&
                        (DebugPfn->u3.e1.EndOfAllocation == 0));
                ASSERT (NextEntry->Owner == Entry);
                NextEntry = (PMMFREE_POOL_ENTRY)((PCHAR)NextEntry + PAGE_SIZE);
            }
        }
        else {

            for (i = 0; i < Entry->Size; i += 1) {

                DebugPte = MiGetPteAddress (NextEntry);
                DebugPfn = MI_PFN_ELEMENT (DebugPte->u.Hard.PageFrameNumber);
                ASSERT ((DebugPfn->u3.e1.StartOfAllocation == 0) &&
                        (DebugPfn->u3.e1.EndOfAllocation == 0));
                ASSERT (NextEntry->Owner == Entry);
                NextEntry = (PMMFREE_POOL_ENTRY)((PCHAR)NextEntry + PAGE_SIZE);
            }
        }
#endif

        //
        // Prevent anyone from accessing non paged pool after freeing it.
        //

        if (MmProtectFreedNonPagedPool == TRUE) {
            MiProtectFreeNonPagedPool ((PVOID)Entry, (ULONG)Entry->Size);
        }

        KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

        return (ULONG)NumberOfPages;
    }

    //
    // Paged pool.  Need to verify start of allocation using
    // end of allocation bitmap.
    //

    if (!RtlCheckBit (PagedPoolInfo->PagedPoolAllocationMap, StartPosition)) {
        KeBugCheckEx (BAD_POOL_CALLER,
                      0x50,
                      (ULONG_PTR)StartingAddress,
                      (ULONG_PTR)StartPosition,
                      MmSizeOfPagedPoolInBytes);
    }

#if DBG
    if (StartPosition > 0) {

        KeAcquireGuardedMutex (PoolMutex);

        if (RtlCheckBit (PagedPoolInfo->PagedPoolAllocationMap, StartPosition - 1)) {
            if (!RtlCheckBit (PagedPoolInfo->EndOfPagedPoolBitmap, StartPosition - 1)) {

                //
                // In the middle of an allocation... bugcheck.
                //

                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x41286,
                              (ULONG_PTR)PagedPoolInfo->PagedPoolAllocationMap,
                              (ULONG_PTR)PagedPoolInfo->EndOfPagedPoolBitmap,
                              StartPosition);
            }
        }

        KeReleaseGuardedMutex (PoolMutex);
    }
#endif

    //
    // Find the last allocated page and check to see if any
    // of the pages being deallocated are in the paging file.
    //

    BitMap = PagedPoolInfo->EndOfPagedPoolBitmap->Buffer;

    i = StartPosition;

    while (!MI_CHECK_BIT (BitMap, i)) {
        i += 1;
    }

    NumberOfPages = i - StartPosition + 1;

    if (SessionSpace == NULL) {

        if (VerifierLargePagedPoolMap != NULL) {

            BitMap = VerifierLargePagedPoolMap->Buffer;

            if (MI_CHECK_BIT (BitMap, StartPosition)) {

                KeAcquireGuardedMutex (&MmPagedPoolMutex);

                ASSERT (MI_CHECK_BIT (BitMap, StartPosition));

                MI_CLEAR_BIT (BitMap, StartPosition);

                KeReleaseGuardedMutex (&MmPagedPoolMutex);

                VerifierFreeTrackedPool (StartingAddress,
                                         NumberOfPages << PAGE_SHIFT,
                                         PagedPool,
                                         FALSE);
            }
        }

        if ((NumberOfPages == 1) &&
            (ExQueryDepthSList (&MiPagedPoolSListHead) < MiPagedPoolSListMaximum)) {
            InterlockedPushEntrySList (&MiPagedPoolSListHead,
                                       (PSLIST_ENTRY) StartingAddress);
            return 1;
        }

        //
        // If paged pool has been configured as non-pageable, only
        // virtual address space is released.
        //
        
        if (MmDisablePagingExecutive & MM_PAGED_POOL_LOCKED_DOWN) {

            KeAcquireGuardedMutex (&MmPagedPoolMutex);

            //
            // Clear the end of allocation bit in the bit map.
            //
    
            RtlClearBit (PagedPoolInfo->EndOfPagedPoolBitmap, (ULONG)i);
    
            //
            // Clear the allocation bits in the bit map.
            //
        
            RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap,
                          StartPosition,
                          (ULONG)NumberOfPages);
        
            if (StartPosition < PagedPoolInfo->PagedPoolHint) {
                PagedPoolInfo->PagedPoolHint = StartPosition;
            }
        
            InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                         0 - NumberOfPages);

            FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

            if (FreePoolInPages > MiLowPagedPoolThreshold) {

                //
                // Read the state directly instead of calling
                // KeReadStateEvent since we are holding the paged
                // pool mutex and want to keep instructions at a
                // minimum.
                //

                if (MiLowPagedPoolEvent->Header.SignalState != 0) {
                    KeClearEvent (MiLowPagedPoolEvent);
                }
                if (FreePoolInPages >= MiHighPagedPoolThreshold) {
                    if (MiHighPagedPoolEvent->Header.SignalState == 0) {
                        KeSetEvent (MiHighPagedPoolEvent, 0, FALSE);
                    }
                }
            }
    
            KeReleaseGuardedMutex (&MmPagedPoolMutex);

            return (ULONG)NumberOfPages;
        }
    }

    PointerPte = PagedPoolInfo->FirstPteForPagedPool + StartPosition;

    PagesFreed = MiDeleteSystemPageableVm (PointerPte,
                                          NumberOfPages,
                                          0,
                                          NULL);

    ASSERT (PagesFreed == NumberOfPages);

    //
    // Clear the end of allocation bit in the bit map.
    //

    BitMap = PagedPoolInfo->EndOfPagedPoolBitmap->Buffer;

    KeAcquireGuardedMutex (PoolMutex);

    MI_CLEAR_BIT (BitMap, i);

    //
    // Clear the allocation bits in the bit map.
    //

    RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap,
                  StartPosition,
                  (ULONG)NumberOfPages);

    if (StartPosition < PagedPoolInfo->PagedPoolHint) {
        PagedPoolInfo->PagedPoolHint = StartPosition;
    }

    if (SessionSpace) {

        KeReleaseGuardedMutex (PoolMutex);

        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     0 - NumberOfPages);
    
        InterlockedExchangeAddSizeT (&SessionSpace->CommittedPages,
                                     0 - NumberOfPages);
   
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_COMMIT_POOL_FREED,
                              (ULONG)NumberOfPages);
    }
    else {
        InterlockedExchangeAddSizeT (&PagedPoolInfo->AllocatedPagedPool,
                                     0 - NumberOfPages);
    
        FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

        if (FreePoolInPages > MiLowPagedPoolThreshold) {

            //
            // Read the state directly instead of calling
            // KeReadStateEvent since we are holding the paged
            // pool mutex and want to keep instructions at a
            // minimum.
            //

            if (MiLowPagedPoolEvent->Header.SignalState != 0) {
                KeClearEvent (MiLowPagedPoolEvent);
            }
            if (FreePoolInPages >= MiHighPagedPoolThreshold) {
                if (MiHighPagedPoolEvent->Header.SignalState == 0) {
                    KeSetEvent (MiHighPagedPoolEvent, 0, FALSE);
                }
            }
        }

        KeReleaseGuardedMutex (PoolMutex);

        InterlockedExchangeAdd ((PLONG) &MmPagedPoolCommit,
                                (LONG)(0 - NumberOfPages));
    }

    MiReturnCommitment (NumberOfPages);

    InterlockedExchangeAddSizeT (&PagedPoolInfo->PagedPoolCommit,
                                 0 - NumberOfPages);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_PAGED_POOL_PAGES, NumberOfPages);

    return (ULONG)NumberOfPages;
}

VOID
MiInitializePoolEvents (
    VOID
    )

/*++

Routine Description:

    This function initializes the pool event states.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, during initialization.

--*/

{
    KIRQL OldIrql;
    PFN_NUMBER FreePoolInPages;

    //
    // Initialize the paged events.
    //

    KeAcquireGuardedMutex (&MmPagedPoolMutex);

    FreePoolInPages = MmSizeOfPagedPoolInPages - MmPagedPoolInfo.AllocatedPagedPool;

    if (FreePoolInPages >= MiHighPagedPoolThreshold) {
        KeSetEvent (MiHighPagedPoolEvent, 0, FALSE);
    }
    else {
        KeClearEvent (MiHighPagedPoolEvent);
    }

    if (FreePoolInPages <= MiLowPagedPoolThreshold) {
        KeSetEvent (MiLowPagedPoolEvent, 0, FALSE);
    }
    else {
        KeClearEvent (MiLowPagedPoolEvent);
    }

    KeReleaseGuardedMutex (&MmPagedPoolMutex);

    //
    // Initialize the nonpaged events.
    //

    OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

    FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

    if (FreePoolInPages >= MiHighNonPagedPoolThreshold) {
        KeSetEvent (MiHighNonPagedPoolEvent, 0, FALSE);
    }
    else {
        KeClearEvent (MiHighNonPagedPoolEvent);
    }

    if (FreePoolInPages <= MiLowNonPagedPoolThreshold) {
        KeSetEvent (MiLowNonPagedPoolEvent, 0, FALSE);
    }
    else {
        KeClearEvent (MiLowNonPagedPoolEvent);
    }

    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

    return;
}

VOID
MiInitializeNonPagedPool (
    VOID
    )

/*++

Routine Description:

    This function initializes the NonPaged pool.

    NonPaged Pool is linked together through the pages.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, during initialization.

--*/

{
    PFN_NUMBER PagesInPool;
    PFN_NUMBER Size;
    ULONG Index;
    PMMFREE_POOL_ENTRY FreeEntry;
    PMMFREE_POOL_ENTRY FirstEntry;
    PMMPTE PointerPte;
    PVOID EndOfInitialPool;
    PFN_NUMBER PageFrameIndex;

    PAGED_CODE();

    //
    // Initialize the slist heads for free pages (both paged & nonpaged).
    //

    InitializeSListHead (&MiPagedPoolSListHead);
    InitializeSListHead (&MiNonPagedPoolSListHead);

    if (MmNumberOfPhysicalPages >= (2*1024*((1024*1024)/PAGE_SIZE))) {
        MiNonPagedPoolSListMaximum <<= 3;
        MiPagedPoolSListMaximum <<= 3;
    }
    else if (MmNumberOfPhysicalPages >= (1*1024*((1024*1024)/PAGE_SIZE))) {
        MiNonPagedPoolSListMaximum <<= 1;
        MiPagedPoolSListMaximum <<= 1;
    }

    //
    // If the verifier or special pool is enabled, then disable lookasides so
    // driver bugs can be found more quickly.
    //

    if ((MmVerifyDriverBufferLength != (ULONG)-1) ||
        (MmProtectFreedNonPagedPool == TRUE) ||
        ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {

        MiNonPagedPoolSListMaximum = 0;
        MiPagedPoolSListMaximum = 0;
    }

    //
    // Initialize the list heads for free pages.
    //

    for (Index = 0; Index < MI_MAX_FREE_LIST_HEADS; Index += 1) {
        InitializeListHead (&MmNonPagedPoolFreeListHead[Index]);
    }

    //
    // Set up the non paged pool pages.
    //

    FreeEntry = (PMMFREE_POOL_ENTRY) MmNonPagedPoolStart;
    FirstEntry = FreeEntry;

    PagesInPool = BYTES_TO_PAGES (MmSizeOfNonPagedPoolInBytes);

    MmNumberOfFreeNonPagedPool = PagesInPool;

    Index = (ULONG)(MmNumberOfFreeNonPagedPool - 1);
    if (Index >= MI_MAX_FREE_LIST_HEADS) {
        Index = MI_MAX_FREE_LIST_HEADS - 1;
    }

    InsertHeadList (&MmNonPagedPoolFreeListHead[Index], &FreeEntry->List);

    FreeEntry->Size = PagesInPool;
#if DBG
    FreeEntry->Signature = MM_FREE_POOL_SIGNATURE;
#endif
    FreeEntry->Owner = FirstEntry;

    while (PagesInPool > 1) {
        FreeEntry = (PMMFREE_POOL_ENTRY)((PCHAR)FreeEntry + PAGE_SIZE);
#if DBG
        FreeEntry->Signature = MM_FREE_POOL_SIGNATURE;
#endif
        FreeEntry->Owner = FirstEntry;
        PagesInPool -= 1;
    }

    //
    // Initialize the first nonpaged pool PFN.
    //

    if (MI_IS_PHYSICAL_ADDRESS(MmNonPagedPoolStart)) {
        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (MmNonPagedPoolStart);
    }
    else {
        PointerPte = MiGetPteAddress(MmNonPagedPoolStart);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    }
    MiStartOfInitialPoolFrame = PageFrameIndex;

    //
    // Set the last nonpaged pool PFN so coalescing on free doesn't go
    // past the end of the initial pool.
    //


    MmNonPagedPoolEnd0 = (PVOID)((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes);
    EndOfInitialPool = (PVOID)((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes - 1);

    if (MI_IS_PHYSICAL_ADDRESS(EndOfInitialPool)) {
        PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (EndOfInitialPool);
    }
    else {
        PointerPte = MiGetPteAddress(EndOfInitialPool);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    }
    MiEndOfInitialPoolFrame = PageFrameIndex;

    //
    // Set up the system PTEs for nonpaged pool expansion.
    //

    PointerPte = MiGetPteAddress (MmNonPagedPoolExpansionStart);
    ASSERT (PointerPte->u.Hard.Valid == 0);

#if defined (_WIN64)
    Size = BYTES_TO_PAGES ((ULONG_PTR)MmNonPagedPoolEnd - (ULONG_PTR)MmNonPagedPoolExpansionStart);
#else
    Size = BYTES_TO_PAGES (MmMaximumNonPagedPoolInBytes -
                            MmSizeOfNonPagedPoolInBytes);
#endif

    //
    // Insert a guard PTE at the top and bottom of expanded nonpaged pool.
    //

    if (Size != 0) {
        Size -= 2;
        PointerPte += 1;
    }

    ASSERT (MiExpansionPoolPagesInUse == 0);

    //
    // Initialize the nonpaged pool expansion resident available initial charge.
    // Note that MmResidentAvailablePages & MmAvailablePages are not initialized
    // yet, but this amount is subtracted when MmResidentAvailablePages is
    // initialized later.
    //
    // Limit the charge to 1/6 of the amount of physical memory at bootup or
    // 256mb, whichever is less.  Not the virtual size is not reduced because
    // if memory is plentiful, we want to allow the system to grow to its
    // maximal value.
    //
    // Note also the 256mb is not picked randomly - we have seen sparsely
    // populated NT64 machines that need more than 128mb in order to boot
    // because the PFN bitmap (not database) was 142mb.
    //

    MiExpansionPoolPagesInitialCharge = Size;

    if (MiExpansionPoolPagesInitialCharge > MmNumberOfPhysicalPages / 6) {
        MiExpansionPoolPagesInitialCharge = MmNumberOfPhysicalPages / 6;
    }

    if (MiExpansionPoolPagesInitialCharge > (256 * 1024 * 1024) / PAGE_SIZE) {
        MiExpansionPoolPagesInitialCharge = (256 * 1024 * 1024) / PAGE_SIZE;
    }

    MiInitializeSystemPtes (PointerPte, Size, NonPagedPoolExpansion);

    //
    // A guard PTE is built at the top by our caller.  This allows us to
    // freely increment virtual addresses in MiFreePoolPages and just check
    // for a blank PTE.
    //
}

VOID
MiAddExpansionNonPagedPool (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER NumberOfPages,
    IN LOGICAL PfnHeld
    )
{
    KIRQL OldIrql;
    PFN_NUMBER i;
    PFN_NUMBER PageTablePage;
    PMMPFN Pfn1;
    PMMPFN PfnList;
    PMMPTE PointerPte;
    ULONG Color;

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    PointerPte = MiGetPteAddress (MmNonPagedPoolExpansionStart);
    PageTablePage = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPte));

    SATISFY_OVERZEALOUS_COMPILER (OldIrql = MM_NOIRQL);

    if (PfnHeld == FALSE) {
        LOCK_PFN (OldIrql);
    }

    PfnList = MiCachedNonPagedPool;

    for (i = 0; i < NumberOfPages; i += 1) {

        //
        // Initialize the frames properly.
        //

        Color = Pfn1->u3.e1.PageColor;
        Pfn1->u3.e2.ShortFlags = 0;
        Pfn1->u3.e1.PageColor = (UCHAR) Color;

        Pfn1->u3.e2.ReferenceCount = 1;
        Pfn1->u2.ShareCount = 1;
        Pfn1->PteAddress = PointerPte;
        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
        Pfn1->u4.EntireFrame = 0;
        Pfn1->u4.MustBeCached = 1;
        Pfn1->u4.PteFrame = PageTablePage;

        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        Pfn1->u3.e1.CacheAttribute = MiCached;

        ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
        ASSERT (Pfn1->u4.VerifierAllocation == 0);

        Pfn1->u1.Event = (PKEVENT) PfnList;
        PfnList = Pfn1;
        Pfn1 += 1;
    }

    MiCachedNonPagedPool = PfnList;
    MiCachedNonPagedPoolCount += NumberOfPages;

    if (PfnHeld == FALSE) {
        UNLOCK_PFN2 (OldIrql);
    }

    return;
}

VOID
MiCheckSessionPoolAllocations (
    VOID
    )

/*++

Routine Description:

    Ensure that the current session has no pool allocations since it is about
    to exit.  All session allocations must be freed prior to session exit.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    SIZE_T i;
    ULONG PagedAllocations;
    ULONG NonPagedAllocations;
    SIZE_T PagedBytes;
    SIZE_T NonPagedBytes;
    PMMPTE StartPde;
    PMMPTE EndPde;
    PMMPTE PointerPte;
    MMPTE TempPte;
    PVOID VirtualAddress;
    PPOOL_TRACKER_TABLE TrackTable;
    PPOOL_TRACKER_TABLE TrackTableBase;
    SIZE_T NumberOfEntries;

    PAGED_CODE();

    TrackTableBase = MiSessionPoolTrackTable ();
    NumberOfEntries = MiSessionPoolTrackTableSize ();

    //
    // Note the session pool descriptor TotalPages field is not reliable
    // for leak checking because of the fact that nonpaged session allocations
    // are converted to global session allocations - thus when a small nonpaged
    // session allocation results in splitting a full page, the global
    // nonpaged pool descriptor (not the session pool descriptor) is (and must
    // be because of the remaining fragment) charged.
    //

    //
    // Make sure all the pool tracking entries are zeroed out.
    //

    PagedAllocations = 0;
    NonPagedAllocations = 0;
    PagedBytes = 0;
    NonPagedBytes = 0;

    TrackTable = TrackTableBase;

    for (i = 0; i < NumberOfEntries; i += 1) {

        PagedBytes += TrackTable->PagedBytes;
        NonPagedBytes += TrackTable->NonPagedBytes;

        PagedAllocations += (TrackTable->PagedAllocs - TrackTable->PagedFrees);
        NonPagedAllocations += (TrackTable->NonPagedAllocs - TrackTable->NonPagedFrees);

        TrackTable += 1;
    }

    if (PagedBytes != 0) {

        //
        // All page tables for this session's paged pool must be freed by now.
        // Being here means they aren't - this is fatal.  Force in any valid
        // pages so that a debugger can show who the guilty party is.
        //

        StartPde = MiGetPdeAddress (MmSessionSpace->PagedPoolStart);
        EndPde = MiGetPdeAddress (MmSessionSpace->PagedPoolEnd);

        while (StartPde <= EndPde) {

            if (StartPde->u.Long != 0) {

                //
                // Hunt through the page table page for valid pages and force
                // them in.  Note this also forces in the page table page if
                // it is not already.
                //

                PointerPte = MiGetVirtualAddressMappedByPte (StartPde);

                for (i = 0; i < PTE_PER_PAGE; i += 1) {
                    TempPte = *PointerPte;

                    if ((TempPte.u.Hard.Valid == 0) &&
                        (TempPte.u.Soft.Protection != 0) &&
                        (TempPte.u.Soft.Protection != MM_NOACCESS)) {

                        VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                        *(volatile UCHAR *)VirtualAddress = *(volatile UCHAR *)VirtualAddress;

                    }
                    PointerPte += 1;
                }

            }

            StartPde += 1;
        }
    }

    if ((NonPagedBytes != 0) || (PagedBytes != 0)) {

        KeBugCheckEx (SESSION_HAS_VALID_POOL_ON_EXIT,
                      (ULONG_PTR)MmSessionSpace->SessionId,
                      PagedBytes,
                      NonPagedBytes,
#if defined (_WIN64)
                      (((ULONG_PTR) NonPagedAllocations) << 32) | (PagedAllocations)
#else
                      (NonPagedAllocations << 16) | (PagedAllocations)
#endif
                    );
    }

#if DBG

    TrackTable = TrackTableBase;

    for (i = 0; i < NumberOfEntries; i += 1) {

        ASSERT (TrackTable->NonPagedBytes == 0);
        ASSERT (TrackTable->PagedBytes == 0);
        ASSERT (TrackTable->NonPagedAllocs == TrackTable->NonPagedFrees);
        ASSERT (TrackTable->PagedAllocs == TrackTable->PagedFrees);

        if (TrackTable->Key == 0) {
            ASSERT (TrackTable->NonPagedAllocs == 0);
            ASSERT (TrackTable->PagedAllocs == 0);
        }

        TrackTable += 1;
    }

    ASSERT (MmSessionSpace->PagedPool.TotalPages == 0);
    ASSERT (MmSessionSpace->PagedPool.TotalBigPages == 0);
    ASSERT (MmSessionSpace->PagedPool.RunningAllocs ==
            MmSessionSpace->PagedPool.RunningDeAllocs);
#endif
}

NTSTATUS
MiInitializeAndChargePfn (
    OUT PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PFN_NUMBER ContainingPageFrame,
    IN LOGICAL SessionAllocation
    )

/*++

Routine Description:

    Nonpaged wrapper to allocate, initialize and charge for a new page.

Arguments:

    PageFrameIndex - Returns the page frame number which was initialized.

    PointerPde - Supplies the pointer to the PDE to initialize.

    ContainingPageFrame - Supplies the page frame number of the page
                          directory page which contains this PDE.

    SessionAllocation - Supplies TRUE if this allocation is in session space,
                        FALSE otherwise.

Return Value:

    Status of the page initialization.

--*/

{
    MMPTE TempPte;
    KIRQL OldIrql;

    if (SessionAllocation == TRUE) {
        TempPte = ValidKernelPdeLocal;
    }
    else {
        TempPte = ValidKernelPde;
    }

    LOCK_PFN2 (OldIrql);

    if ((MmAvailablePages < MM_MEDIUM_LIMIT) ||
        (MI_NONPAGEABLE_MEMORY_AVAILABLE() <= 1)) {

        UNLOCK_PFN2 (OldIrql);
        return STATUS_NO_MEMORY;
    }

    //
    // Ensure no other thread handled this while this one waited.  If one has,
    // then return STATUS_RETRY so the caller knows to try again.
    //

    if (PointerPde->u.Hard.Valid == 1) {
        UNLOCK_PFN2 (OldIrql);
        return STATUS_RETRY;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_SINGLE_PFN);

    //
    // Allocate and map in the page at the requested address.
    //

    *PageFrameIndex = MiRemoveZeroPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));
    TempPte.u.Hard.PageFrameNumber = *PageFrameIndex;
    MI_WRITE_VALID_PTE (PointerPde, TempPte);

    MiInitializePfnForOtherProcess (*PageFrameIndex,
                                    PointerPde,
                                    ContainingPageFrame);

    //
    // This page will be locked into working set and assigned an index when
    // the working set is set up on return.
    //

    ASSERT (MI_PFN_ELEMENT(*PageFrameIndex)->u1.WsIndex == 0);

    UNLOCK_PFN2 (OldIrql);

    return STATUS_SUCCESS;
}


VOID
MiSessionPageTableRelease (
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    Nonpaged wrapper to release a session pool page table page.

Arguments:

    PageFrameIndex - Returns the page frame number which was initialized.

Return Value:

    None.

--*/

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN Pfn2;

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

    MI_SET_PFN_DELETED (Pfn1);

    LOCK_PFN (OldIrql);

    ASSERT (MmSessionSpace->SessionPageDirectoryIndex == Pfn1->u4.PteFrame);
    ASSERT (Pfn1->u2.ShareCount == 1);

    MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

    MiDecrementShareCount (Pfn1, PageFrameIndex);

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_SESSION_PAGE_TABLE);
}


NTSTATUS
MiInitializeSessionPool (
    VOID
    )

/*++

Routine Description:

    Initialize the current session's pool structure.

Arguments:

    None.

Return Value:

    Status of the pool initialization.

Environment:

    Kernel mode.

--*/

{
    PMMPTE PointerPde, PointerPte;
    PFN_NUMBER PageFrameIndex;
    PPOOL_DESCRIPTOR PoolDescriptor;
    PMM_SESSION_SPACE SessionGlobal;
    PMM_PAGED_POOL_INFO PagedPoolInfo;
    NTSTATUS Status;
#if (_MI_PAGING_LEVELS < 3)
    ULONG Index;
#endif
#if DBG
    PMMPTE StartPde;
    PMMPTE EndPde;
#endif

    PAGED_CODE ();

    SessionGlobal = SESSION_GLOBAL(MmSessionSpace);

    KeInitializeGuardedMutex (&SessionGlobal->PagedPoolMutex);

    PoolDescriptor = &MmSessionSpace->PagedPool;

    ExInitializePoolDescriptor (PoolDescriptor,
                                PagedPoolSession,
                                0,
                                0,
                                &SessionGlobal->PagedPoolMutex);

    MmSessionSpace->PagedPoolStart = (PVOID)MiSessionPoolStart;
    MmSessionSpace->PagedPoolEnd = (PVOID)(MiSessionPoolEnd -1);

    PagedPoolInfo = &MmSessionSpace->PagedPoolInfo;
    PagedPoolInfo->PagedPoolCommit = 0;
    PagedPoolInfo->PagedPoolHint = 0;
    PagedPoolInfo->AllocatedPagedPool = 0;

    //
    // Build the page table page for paged pool.
    //

    PointerPde = MiGetPdeAddress (MmSessionSpace->PagedPoolStart);
    MmSessionSpace->PagedPoolBasePde = PointerPde;

    PointerPte = MiGetPteAddress (MmSessionSpace->PagedPoolStart);

    PagedPoolInfo->FirstPteForPagedPool = PointerPte;
    PagedPoolInfo->LastPteForPagedPool = MiGetPteAddress (MmSessionSpace->PagedPoolEnd);

#if DBG
    //
    // Session pool better be unused.
    //

    StartPde = MiGetPdeAddress (MmSessionSpace->PagedPoolStart);
    EndPde = MiGetPdeAddress (MmSessionSpace->PagedPoolEnd);

    while (StartPde <= EndPde) {
        ASSERT (StartPde->u.Long == 0);
        StartPde += 1;
    }
#endif

    //
    // Mark all PDEs as empty.
    //

    MiZeroMemoryPte (PointerPde,
                     (1 + MiGetPdeAddress (MmSessionSpace->PagedPoolEnd) - PointerPde));

    if (MiChargeCommitment (1, NULL) == FALSE) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_COMMIT);
        return STATUS_NO_MEMORY;
    }

    Status = MiInitializeAndChargePfn (&PageFrameIndex,
                                       PointerPde,
                                       MmSessionSpace->SessionPageDirectoryIndex,
                                       TRUE);

    if (!NT_SUCCESS(Status)) {
        MiReturnCommitment (1);
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_RESIDENT);
        return Status;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_POOL_PAGE_TABLES, 1);

    MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_ALLOC, 1);

#if (_MI_PAGING_LEVELS < 3)

    Index = MiGetPdeSessionIndex (MmSessionSpace->PagedPoolStart);

    ASSERT (MmSessionSpace->PageTables[Index].u.Long == 0);
    MmSessionSpace->PageTables[Index] = *PointerPde;

#endif

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_POOL_CREATE, 1);

    InterlockedIncrementSizeT (&MmSessionSpace->NonPageablePages);

    InterlockedIncrementSizeT (&MmSessionSpace->CommittedPages);

    PagedPoolInfo->NextPdeForPagedPoolExpansion = PointerPde + 1;

    //
    // Initialize the bitmaps.
    //

    MiCreateBitMap (&PagedPoolInfo->PagedPoolAllocationMap,
                    MmSessionPoolSize >> PAGE_SHIFT,
                    NonPagedPool);

    if (PagedPoolInfo->PagedPoolAllocationMap == NULL) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_NONPAGED_POOL);
        goto Failure;
    }

    //
    // We start with all pages in the virtual address space as "busy", and
    // clear bits to make pages available as we dynamically expand the pool.
    //

    RtlSetAllBits( PagedPoolInfo->PagedPoolAllocationMap );

    //
    // Indicate first page worth of PTEs are available.
    //

    RtlClearBits (PagedPoolInfo->PagedPoolAllocationMap, 0, PTE_PER_PAGE);

    //
    // Create the end of allocation range bitmap.
    //

    MiCreateBitMap (&PagedPoolInfo->EndOfPagedPoolBitmap,
                    MmSessionPoolSize >> PAGE_SHIFT,
                    NonPagedPool);

    if (PagedPoolInfo->EndOfPagedPoolBitmap == NULL) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_NONPAGED_POOL);
        goto Failure;
    }

    RtlClearAllBits (PagedPoolInfo->EndOfPagedPoolBitmap);

    return STATUS_SUCCESS;

Failure:

    MiFreeSessionPoolBitMaps ();

    MiSessionPageTableRelease (PageFrameIndex);

    MI_WRITE_ZERO_PTE (PointerPde);

    MI_FLUSH_SINGLE_TB (MiGetVirtualAddressMappedByPte (PointerPde), TRUE);

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_POOL_CREATE_FAILED, 1);

    InterlockedDecrementSizeT (&MmSessionSpace->NonPageablePages);

    InterlockedDecrementSizeT (&MmSessionSpace->CommittedPages);

    MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_PAGEDPOOL_PAGETABLE_FREE_FAIL1, 1);

    MiReturnCommitment (1);

    MM_TRACK_COMMIT_REDUCTION (MM_DBG_COMMIT_SESSION_POOL_PAGE_TABLES, 1);

    return STATUS_NO_MEMORY;
}


VOID
MiFreeSessionPoolBitMaps (
    VOID
    )

/*++

Routine Description:

    Free the current session's pool bitmap structures.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    PAGED_CODE();

    if (MmSessionSpace->PagedPoolInfo.PagedPoolAllocationMap ) {
        ExFreePool (MmSessionSpace->PagedPoolInfo.PagedPoolAllocationMap);
        MmSessionSpace->PagedPoolInfo.PagedPoolAllocationMap = NULL;
    }

    if (MmSessionSpace->PagedPoolInfo.EndOfPagedPoolBitmap ) {
        ExFreePool (MmSessionSpace->PagedPoolInfo.EndOfPagedPoolBitmap);
        MmSessionSpace->PagedPoolInfo.EndOfPagedPoolBitmap = NULL;
    }

    return;
}

#if DBG

#define MI_LOG_CONTIGUOUS  100

typedef struct _MI_CONTIGUOUS_ALLOCATORS {
    PVOID BaseAddress;
    SIZE_T NumberOfBytes;
    PVOID CallingAddress;
} MI_CONTIGUOUS_ALLOCATORS, *PMI_CONTIGUOUS_ALLOCATORS;

ULONG MiContiguousIndex;
MI_CONTIGUOUS_ALLOCATORS MiContiguousAllocators[MI_LOG_CONTIGUOUS];

VOID
MiInsertContiguousTag (
    IN PVOID BaseAddress,
    IN SIZE_T NumberOfBytes,
    IN PVOID CallingAddress
    )
{
    KIRQL OldIrql;

#if !DBG
    if ((NtGlobalFlag & FLG_POOL_ENABLE_TAGGING) == 0) {
        return;
    }
#endif

    OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

    if (MiContiguousIndex >= MI_LOG_CONTIGUOUS) {
        MiContiguousIndex = 0;
    }

    MiContiguousAllocators[MiContiguousIndex].BaseAddress = BaseAddress;
    MiContiguousAllocators[MiContiguousIndex].NumberOfBytes = NumberOfBytes;
    MiContiguousAllocators[MiContiguousIndex].CallingAddress = CallingAddress;

    MiContiguousIndex += 1;

    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);
}
#else
#define MiInsertContiguousTag(a, b, c) (c) = (c)
#endif


PVOID
MiFindContiguousMemoryInPool (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN PVOID CallingAddress
    )

/*++

Routine Description:

    This function searches nonpaged pool for contiguous pages to satisfy the
    request.  Note the pool address returned maps these pages as MmCached.

Arguments:

    LowestPfn - Supplies the lowest acceptable physical page number.

    HighestPfn - Supplies the highest acceptable physical page number.

    BoundaryPfn - Supplies the page frame number multiple the allocation must
                  not cross.  0 indicates it can cross any boundary.

    SizeInPages - Supplies the number of pages to allocate.

    CallingAddress - Supplies the calling address of the allocator.

Return Value:

    NULL - a contiguous range could not be found to satisfy the request.

    NON-NULL - Returns a pointer (virtual address in the nonpaged portion
               of the system) to the allocated physically contiguous
               memory.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/
{
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PVOID BaseAddress;
    PVOID BaseAddress2;
    KIRQL OldIrql;
    PMMFREE_POOL_ENTRY FreePageInfo;
    PLIST_ENTRY Entry;
    ULONG Index;
    PFN_NUMBER BoundaryMask;
    ULONG AllocationPosition;
    PVOID Va;
    PFN_NUMBER SpanInPages;
    PFN_NUMBER SpanInPages2;
    PFN_NUMBER FreePoolInPages;

    PAGED_CODE ();

    //
    // Initializing SpanInPages* is not needed for correctness
    // but without it the compiler cannot compile this code
    // W4 to check for use of uninitialized variables.
    //

    SpanInPages = 0;
    SpanInPages2 = 0;

    BaseAddress = NULL;

    BoundaryMask = ~(BoundaryPfn - 1);

    //
    // A suitable pool page was not allocated via the pool allocator.
    // Grab the pool lock and manually search for a page which meets
    // the requirements.
    //

    MmLockPageableSectionByHandle (ExPageLockHandle);

    //
    // Trace through the page allocator's pool headers for a page which
    // meets the requirements.
    //
    // NonPaged pool is linked together through the pages themselves.
    //

    Index = (ULONG)(SizeInPages - 1);

    if (Index >= MI_MAX_FREE_LIST_HEADS) {
        Index = MI_MAX_FREE_LIST_HEADS - 1;
    }

    OldIrql = KeAcquireQueuedSpinLock (LockQueueMmNonPagedPoolLock);

    while (Index < MI_MAX_FREE_LIST_HEADS) {

        Entry = MmNonPagedPoolFreeListHead[Index].Flink;
    
        while (Entry != &MmNonPagedPoolFreeListHead[Index]) {
    
            if (MmProtectFreedNonPagedPool == TRUE) {
                MiUnProtectFreeNonPagedPool ((PVOID)Entry, 0);
            }
    
            //
            // The list is not empty, see if this one meets the physical
            // requirements.
            //
    
            FreePageInfo = CONTAINING_RECORD(Entry,
                                             MMFREE_POOL_ENTRY,
                                             List);
    
            ASSERT (FreePageInfo->Signature == MM_FREE_POOL_SIGNATURE);
            if (FreePageInfo->Size >= SizeInPages) {
    
                //
                // This entry has sufficient space, check to see if the
                // pages meet the physical requirements.
                //
    
                Va = MiCheckForContiguousMemory (PAGE_ALIGN(Entry),
                                                 FreePageInfo->Size,
                                                 SizeInPages,
                                                 LowestPfn,
                                                 HighestPfn,
                                                 BoundaryPfn,
                                                 MiCached);
     
                if (Va != NULL) {

                    //
                    // These pages meet the requirements.  The returned
                    // address may butt up on the end, the front or be
                    // somewhere in the middle.  Split the Entry based
                    // on which case it is.
                    //

                    Entry = PAGE_ALIGN(Entry);
                    if (MmProtectFreedNonPagedPool == FALSE) {
                        RemoveEntryList (&FreePageInfo->List);
                    }
                    else {
                        MiProtectedPoolRemoveEntryList (&FreePageInfo->List);
                    }
    
                    //
                    // Adjust the number of free pages remaining in the pool.
                    // The TotalBigPages calculation appears incorrect for the
                    // case where we're splitting a block, but it's done this
                    // way because ExFreePool corrects it when we free the
                    // fragment block below.  Likewise for
                    // MmAllocatedNonPagedPool and MmNumberOfFreeNonPagedPool
                    // which is corrected by MiFreePoolPages for the fragment.
                    //
    
                    InterlockedExchangeAdd ((PLONG)&NonPagedPoolDescriptor.TotalBigPages,
                                            (LONG)FreePageInfo->Size);

                    InterlockedExchangeAddSizeT (&NonPagedPoolDescriptor.TotalBytes,
                                             FreePageInfo->Size << PAGE_SHIFT);

                    MmAllocatedNonPagedPool += FreePageInfo->Size;

                    FreePoolInPages = MmMaximumNonPagedPoolInPages - MmAllocatedNonPagedPool;

                    if (FreePoolInPages < MiHighNonPagedPoolThreshold) {

                        //
                        // Read the state directly instead of calling
                        // KeReadStateEvent since we are holding the nonpaged
                        // pool lock and want to keep instructions at a
                        // minimum.
                        //

                        if (MiHighNonPagedPoolEvent->Header.SignalState != 0) {
                            KeClearEvent (MiHighNonPagedPoolEvent);
                        }
                        if (FreePoolInPages <= MiLowNonPagedPoolThreshold) {
                            if (MiLowNonPagedPoolEvent->Header.SignalState == 0) {
                                KeSetEvent (MiLowNonPagedPoolEvent, 0, FALSE);
                            }
                        }
                    }

                    MmNumberOfFreeNonPagedPool -= FreePageInfo->Size;
    
                    ASSERT ((LONG)MmNumberOfFreeNonPagedPool >= 0);
    
                    if (Va == Entry) {

                        //
                        // Butted against the front.
                        //

                        AllocationPosition = 0;
                    }
                    else if (((PCHAR)Va + (SizeInPages << PAGE_SHIFT)) == ((PCHAR)Entry + (FreePageInfo->Size << PAGE_SHIFT))) {

                        //
                        // Butted against the end.
                        //

                        AllocationPosition = 2;
                    }
                    else {

                        //
                        // Somewhere in the middle.
                        //

                        AllocationPosition = 1;
                    }

                    //
                    // Pages are being removed from the front of
                    // the list entry and the whole list entry
                    // will be removed and then the remainder inserted.
                    //
    
                    //
                    // Mark start and end for the block at the top of the
                    // list.
                    //
    
                    if (MI_IS_PHYSICAL_ADDRESS(Va)) {
    
                        //
                        // On certain architectures, virtual addresses
                        // may be physical and hence have no corresponding PTE.
                        //
    
                        PointerPte = NULL;
                        Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (Va));
                    }
                    else {
                        PointerPte = MiGetPteAddress(Va);
                        ASSERT (PointerPte->u.Hard.Valid == 1);
                        Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                    }
    
                    ASSERT (Pfn1->u4.VerifierAllocation == 0);
                    ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                    ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
                    Pfn1->u3.e1.StartOfAllocation = 1;
    
                    //
                    // Calculate the ending PFN address, note that since
                    // these pages are contiguous, just add to the PFN.
                    //
    
                    Pfn1 += SizeInPages - 1;
                    ASSERT (Pfn1->u4.VerifierAllocation == 0);
                    ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                    ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);
                    Pfn1->u3.e1.EndOfAllocation = 1;
    
                    if (SizeInPages == FreePageInfo->Size) {
    
                        //
                        // Unlock the pool and return.
                        //

                        KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock,
                                                 OldIrql);

                        BaseAddress = (PVOID)Va;
                        goto Done;
                    }
    
                    BaseAddress = NULL;

                    if (AllocationPosition != 2) {

                        //
                        // The end piece needs to be freed as the removal
                        // came from the front or the middle.
                        //

                        BaseAddress = (PVOID)((PCHAR)Va + (SizeInPages << PAGE_SHIFT));
                        SpanInPages = FreePageInfo->Size - SizeInPages -
                            (((ULONG_PTR)Va - (ULONG_PTR)Entry) >> PAGE_SHIFT);
    
                        //
                        // Mark start and end of the allocation in the PFN database.
                        //
        
                        if (PointerPte == NULL) {
        
                            //
                            // On certain architectures, virtual addresses
                            // may be physical and hence have no corresponding PTE.
                            //
        
                            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (BaseAddress));
                        }
                        else {
                            PointerPte = MiGetPteAddress(BaseAddress);
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
        
                        ASSERT (Pfn1->u4.VerifierAllocation == 0);
                        ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                        ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
                        Pfn1->u3.e1.StartOfAllocation = 1;
        
                        //
                        // Calculate the ending PTE's address, can't depend on
                        // these pages being physically contiguous.
                        //
        
                        if (PointerPte == NULL) {
                            Pfn1 += (SpanInPages - 1);
                        }
                        else {
                            PointerPte += (SpanInPages - 1);
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
                        ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);
                        Pfn1->u3.e1.EndOfAllocation = 1;
        
                        ASSERT (((ULONG_PTR)BaseAddress & (PAGE_SIZE -1)) == 0);
        
                        SpanInPages2 = SpanInPages;
                    }
        
                    BaseAddress2 = BaseAddress;
                    BaseAddress = NULL;

                    if (AllocationPosition != 0) {

                        //
                        // The front piece needs to be freed as the removal
                        // came from the middle or the end.
                        //

                        BaseAddress = (PVOID)Entry;

                        SpanInPages = ((ULONG_PTR)Va - (ULONG_PTR)Entry) >> PAGE_SHIFT;
    
                        //
                        // Mark start and end of the allocation in the PFN database.
                        //
        
                        if (PointerPte == NULL) {
        
                            //
                            // On certain architectures, virtual addresses
                            // may be physical and hence have no corresponding PTE.
                            //
        
                            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (BaseAddress));
                        }
                        else {
                            PointerPte = MiGetPteAddress(BaseAddress);
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
        
                        ASSERT (Pfn1->u4.VerifierAllocation == 0);
                        ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                        ASSERT (Pfn1->u3.e1.StartOfAllocation == 0);
                        Pfn1->u3.e1.StartOfAllocation = 1;
        
                        //
                        // Calculate the ending PTE's address, can't depend on
                        // these pages being physically contiguous.
                        //
        
                        if (PointerPte == NULL) {
                            Pfn1 += (SpanInPages - 1);
                        }
                        else {
                            PointerPte += (SpanInPages - 1);
                            ASSERT (PointerPte->u.Hard.Valid == 1);
                            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
                        }
                        ASSERT (Pfn1->u3.e1.EndOfAllocation == 0);
                        Pfn1->u3.e1.EndOfAllocation = 1;
        
                        ASSERT (((ULONG_PTR)BaseAddress & (PAGE_SIZE -1)) == 0);
                    }
        
                    //
                    // Unlock the pool.
                    //
    
                    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock,
                                             OldIrql);
    
                    //
                    // Free the split entry at BaseAddress back into the pool.
                    // Note that we have overcharged the pool - the entire free
                    // chunk has been billed.  Here we return the piece we
                    // didn't use and correct the momentary overbilling.
                    //
                    // The start and end allocation bits of this split entry
                    // which we just set up enable ExFreePool and his callees
                    // to correctly adjust the billing.
                    //
    
                    if (BaseAddress) {
                        ExInsertPoolTag ('tnoC',
                                         BaseAddress,
                                         SpanInPages << PAGE_SHIFT,
                                         NonPagedPool);
                        ExFreePool (BaseAddress);
                    }
                    if (BaseAddress2) {
                        ExInsertPoolTag ('tnoC',
                                         BaseAddress2,
                                         SpanInPages2 << PAGE_SHIFT,
                                         NonPagedPool);
                        ExFreePool (BaseAddress2);
                    }
                    BaseAddress = Va;
                    goto Done;
                }
            }
            Entry = FreePageInfo->List.Flink;
            if (MmProtectFreedNonPagedPool == TRUE) {
                MiProtectFreeNonPagedPool ((PVOID)FreePageInfo,
                                           (ULONG)FreePageInfo->Size);
            }
        }
        Index += 1;
    }

    //
    // No entry was found in free nonpaged pool that meets the requirements.
    //

    KeReleaseQueuedSpinLock (LockQueueMmNonPagedPoolLock, OldIrql);

Done:

    MmUnlockPageableImageSection (ExPageLockHandle);

    if (BaseAddress) {

        MiInsertContiguousTag (BaseAddress,
                               SizeInPages << PAGE_SHIFT,
                               CallingAddress);

        ExInsertPoolTag ('tnoC',
                         BaseAddress,
                         SizeInPages << PAGE_SHIFT,
                         NonPagedPool);
    }

    return BaseAddress;
}

PFN_NUMBER
MiFindContiguousPages (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType
    )

/*++

Routine Description:

    This function searches nonpaged pool and the free, zeroed,
    and standby lists for contiguous pages that satisfy the
    request.

    Note no virtual address space is used (thus nonpaged pool is not scanned).
    A physical frame number (the caller can map it if he wants to) is returned.

Arguments:

    LowestPfn - Supplies the lowest acceptable physical page number.

    HighestPfn - Supplies the highest acceptable physical page number.

    BoundaryPfn - Supplies the page frame number multiple the allocation must
                  not cross.  0 indicates it can cross any boundary.

    SizeInPages - Supplies the number of pages to allocate.

    CacheType - Supplies the type of cache mapping that will be used for the
                memory.

Return Value:

    0 - a contiguous range could not be found to satisfy the request.

    Nonzero - Returns the base physical frame number to the allocated
              physically contiguous memory.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

    Note that in addition to being called at normal runtime, this routine
    is also called during Phase 0 initialization before the loaded module
    list has been initialized - therefore this routine cannot be made PAGELK
    as we wouldn't know how to find it to ensure it was resident.

--*/

{
    PMMPTE DummyPte;
    PKTHREAD Thread;
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    KIRQL OldIrql;
    ULONG start;
    PFN_NUMBER count;
    PFN_NUMBER Page;
    PFN_NUMBER LastPage;
    PFN_NUMBER found;
    PFN_NUMBER BoundaryMask;
    PFN_NUMBER PageFrameIndex;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    ULONG RetryCount;
    LOGICAL FlushedTb;

    PAGED_CODE ();

    ASSERT (SizeInPages != 0);

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

    BoundaryMask = ~(BoundaryPfn - 1);

    Pfn1 = NULL;
    DummyPte = MiGetPteAddress (MmNonPagedPoolExpansionStart);

    //
    // Manually search for a page range which meets the requirements.
    //

    Thread = KeGetCurrentThread ();

    KeEnterGuardedRegionThread (Thread);

    MI_LOCK_DYNAMIC_MEMORY_SHARED ();

    //
    // Charge commitment.
    //
    // Then search the PFN database for pages that meet the requirements.
    //

    if (MiChargeCommitmentCantExpand (SizeInPages, FALSE) == FALSE) {
        MI_UNLOCK_DYNAMIC_MEMORY_SHARED ();
        KeLeaveGuardedRegionThread (Thread);
        return 0;
    }

    //
    // Charge resident available pages.
    //

    LOCK_PFN (OldIrql);

    MiDeferredUnlockPages (MI_DEFER_PFN_HELD);

    if ((SPFN_NUMBER)SizeInPages > MI_NONPAGEABLE_MEMORY_AVAILABLE()) {
        UNLOCK_PFN (OldIrql);
        goto Failed;
    }

    //
    // Systems utilizing memory compression may have more
    // pages on the zero, free and standby lists than we
    // want to give out.  Explicitly check MmAvailablePages
    // instead (and recheck whenever the PFN lock is released
    // and reacquired).
    //

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        UNLOCK_PFN (OldIrql);
        goto Failed;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_ALLOCATE_CONTIGUOUS);

    UNLOCK_PFN (OldIrql);

    RetryCount = 4;

Retry:

    start = 0;
    found = 0;

    do {

        count = MmPhysicalMemoryBlock->Run[start].PageCount;
        Page = MmPhysicalMemoryBlock->Run[start].BasePage;

        //
        // Close the gaps, then examine the range for a fit.
        //

        LastPage = Page + count; 

        if (LastPage - 1 > HighestPfn) {
            LastPage = HighestPfn + 1;
        }
    
        if (Page < LowestPfn) {
            Page = LowestPfn;
        }

        if ((count != 0) && (Page + SizeInPages <= LastPage)) {
    
            //
            // A fit may be possible in this run, check whether the pages
            // are on the right list.
            //

            found = 0;
            Pfn1 = MI_PFN_ELEMENT (Page);

            for ( ; Page < LastPage; Page += 1, Pfn1 += 1) {

                if ((Pfn1->u3.e1.PageLocation <= StandbyPageList) &&
                    (Pfn1->u1.Flink != 0) &&
                    (Pfn1->u2.Blink != 0) &&
                    (Pfn1->u3.e2.ReferenceCount == 0) &&
                    ((CacheAttribute == MiCached) || (Pfn1->u4.MustBeCached == 0))) {

                    //
                    // Before starting a new run, ensure that it
                    // can satisfy the boundary requirements (if any).
                    //
                    
                    if ((found == 0) && (BoundaryPfn != 0)) {
                        if (((Page ^ (Page + SizeInPages - 1)) & BoundaryMask) != 0) {
                            //
                            // This run's physical address does not meet the
                            // requirements.
                            //

                            continue;
                        }
                    }

                    found += 1;

                    if (found == SizeInPages) {

                        //
                        // Lock the PFN database and see if the pages are
                        // still available for us.  Note the invariant
                        // condition (boundary conformance) does not need
                        // to be checked again as it was already checked
                        // above.
                        //

                        Pfn1 -= (found - 1);
                        Page -= (found - 1);

                        LOCK_PFN (OldIrql);

                        do {

                            if ((Pfn1->u3.e1.PageLocation <= StandbyPageList) &&
                                (Pfn1->u1.Flink != 0) &&
                                (Pfn1->u2.Blink != 0) &&
                                (Pfn1->u3.e2.ReferenceCount == 0) &&
                                ((CacheAttribute == MiCached) || (Pfn1->u4.MustBeCached == 0))) {

                                NOTHING;            // Good page
                            }
                            else {
                                break;
                            }

                            found -= 1;

                            if (found == 0) {

                                //
                                // All the pages matched the criteria, keep the
                                // PFN lock, remove them and map them for our
                                // caller.
                                //

                                goto Success;
                            }

                            Pfn1 += 1;
                            Page += 1;

                        } while (TRUE);

                        UNLOCK_PFN (OldIrql);

                        //
                        // Restart the search at the first possible page.
                        //

                        found = 0;
                    }
                }
                else {
                    found = 0;
                }
            }
        }
        start += 1;

    } while (start != MmPhysicalMemoryBlock->NumberOfRuns);

    //
    // The desired physical pages could not be allocated - try harder.
    //

    if (InitializationPhase == 0) {
        MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_CONTIGUOUS);

        goto Failed;
    }

    InterlockedIncrement (&MiDelayPageFaults);

    //
    // Attempt to move pages to the standby list.  This is done with
    // gradually increasing aggressiveness so as not to prematurely
    // drain modified writes unless it's truly needed.
    //

    switch (RetryCount) {

        case 4:
            MmEmptyAllWorkingSets ();
            break;

        case 3:
            MiFlushAllPages ();
            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmHalfSecond);
            break;

        case 2:
            MmEmptyAllWorkingSets ();
            MiFlushAllPages ();
            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmOneSecond);
            break;

        case 1:

            //
            // Purge the transition list as transition pages keep
            // page tables from being taken and we are desperate.
            //

            MiPurgeTransitionList ();

            //
            // Empty all the working sets now that the
            // transition list has been purged.  This will put page tables
            // on the modified list.
            //

            MmEmptyAllWorkingSets ();

            //
            // Write out modified pages (including newly trimmed page table
            // pages).
            //

            MiFlushAllPages ();

            //
            // Give the writes a chance to complete so the modified pages
            // can be marked clean and put on the transition list.
            //

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmOneSecond);

            //
            // Purge the transition list one last time to get the now-clean
            // page table pages out.
            //

            MiPurgeTransitionList ();

            //
            // Finally get any straggling active pages onto the transition
            // lists.
            //

            MmEmptyAllWorkingSets ();
            MiFlushAllPages ();

            break;

        default:
            break;
    }

    InterlockedDecrement (&MiDelayPageFaults);

    if (RetryCount != 0) {
        RetryCount -= 1;
        goto Retry;
    }

Failed:

    MI_UNLOCK_DYNAMIC_MEMORY_SHARED ();
    KeLeaveGuardedRegionThread (Thread);

    MiReturnCommitment (SizeInPages);

    return 0;

Success:

    ASSERT (start != MmPhysicalMemoryBlock->NumberOfRuns);

    //
    // A match has been found, remove these pages
    // and return.  The PFN lock is held.
    //

    //
    // Systems utilizing memory compression may have more
    // pages on the zero, free and standby lists than we
    // want to give out.  Explicitly check MmAvailablePages
    // instead (and recheck whenever the PFN lock is
    // released and reacquired).
    //

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        UNLOCK_PFN (OldIrql);
        MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_CONTIGUOUS);
        MiReturnCommitment (SizeInPages);
        goto Failed;
    }

    EndPfn = Pfn1 - SizeInPages + 1;

    FlushedTb = FALSE;

    do {

        if (Pfn1->u3.e1.PageLocation == StandbyPageList) {
            MiUnlinkPageFromList (Pfn1);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            MiRestoreTransitionPte (Pfn1);
        }
        else {
            MiUnlinkFreeOrZeroedPage (Pfn1);
        }

        Pfn1->u3.e2.ReferenceCount = 1;
        Pfn1->u2.ShareCount = 1;
        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;

        //
        // The entire TB must be flushed if we are changing cache
        // attributes.
        //
        // KeFlushSingleTb cannot be used because we don't know
        // what virtual address(es) this physical frame was last mapped at.
        //
        // Note we can skip the flush if we've already done it once in
        // this loop already because the PFN lock is held throughout.
        //

        if ((Pfn1->u3.e1.CacheAttribute != CacheAttribute) && (FlushedTb == FALSE)) {
            PageFrameIndex = Pfn1 - MmPfnDatabase;
            MI_FLUSH_TB_FOR_INDIVIDUAL_ATTRIBUTE_CHANGE (PageFrameIndex,
                                                         CacheAttribute);
            FlushedTb = TRUE;
        }

        Pfn1->u3.e1.CacheAttribute = CacheAttribute;
        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1->u3.e1.EndOfAllocation = 0;
        Pfn1->u3.e1.LargeSessionAllocation = 0;
        Pfn1->u3.e1.PrototypePte = 0;
        Pfn1->u4.VerifierAllocation = 0;

        //
        // Initialize PteAddress so an MiIdentifyPfn scan
        // won't crash.  The real value is put in after the loop.
        //

        Pfn1->PteAddress = DummyPte;

        if (Pfn1 == EndPfn) {
            break;
        }

        Pfn1 -= 1;

    } while (TRUE);

    Pfn1->u3.e1.StartOfAllocation = 1;
    (Pfn1 + SizeInPages - 1)->u3.e1.EndOfAllocation = 1;

    UNLOCK_PFN (OldIrql);

    EndPfn = Pfn1 + SizeInPages;
    ASSERT (EndPfn == MI_PFN_ELEMENT (Page + 1));

    Page = Page - SizeInPages + 1;
    ASSERT (Pfn1 == MI_PFN_ELEMENT (Page));
    ASSERT (Page != 0);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_CONTIGUOUS_PAGES, SizeInPages);

    MI_UNLOCK_DYNAMIC_MEMORY_SHARED ();
    KeLeaveGuardedRegionThread (Thread);

    return Page;
}


VOID
MiFreeContiguousPages (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    )

/*++

Routine Description:

    This function frees the specified physical page range, returning both
    commitment and resident available.

Arguments:

    PageFrameIndex - Supplies the starting physical page number.

    SizeInPages - Supplies the number of pages to free.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

    This is callable from MiReloadBootLoadedDrivers->MiUseDriverLargePages
    during Phase 0.  ExPageLockHandle and other variables won't exist at
    this point, so don't get too fancy here.

--*/

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN EndPfn;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    EndPfn = Pfn1 + SizeInPages;

    LOCK_PFN2 (OldIrql);

    Pfn1->u3.e1.StartOfAllocation = 0;
    (EndPfn - 1)->u3.e1.EndOfAllocation = 0;

    do {
        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageFrameIndex);
        PageFrameIndex += 1;
        Pfn1 += 1;
    } while (Pfn1 < EndPfn);

    UNLOCK_PFN2 (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_CONTIGUOUS);

    MiReturnCommitment (SizeInPages);

    return;
}


PVOID
MiFindContiguousMemory (
    IN PFN_NUMBER LowestPfn,
    IN PFN_NUMBER HighestPfn,
    IN PFN_NUMBER BoundaryPfn,
    IN PFN_NUMBER SizeInPages,
    IN MEMORY_CACHING_TYPE CacheType,
    IN PVOID CallingAddress
    )

/*++

Routine Description:

    This function searches nonpaged pool and the free, zeroed,
    and standby lists for contiguous pages that satisfy the
    request.

Arguments:

    LowestPfn - Supplies the lowest acceptable physical page number.

    HighestPfn - Supplies the highest acceptable physical page number.

    BoundaryPfn - Supplies the page frame number multiple the allocation must
                  not cross.  0 indicates it can cross any boundary.

    SizeInPages - Supplies the number of pages to allocate.

    CacheType - Supplies the type of cache mapping that will be used for the
                memory.

    CallingAddress - Supplies the calling address of the allocator.

Return Value:

    NULL - a contiguous range could not be found to satisfy the request.

    NON-NULL - Returns a pointer (virtual address in the nonpaged portion
               of the system) to the allocated physically contiguous
               memory.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/
{
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    PVOID BaseAddress;
    PFN_NUMBER Page;
    PHYSICAL_ADDRESS PhysicalAddress;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    PAGED_CODE ();

    ASSERT (SizeInPages != 0);

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

    if (CacheAttribute == MiCached) {

        BaseAddress = MiFindContiguousMemoryInPool (LowestPfn,
                                                    HighestPfn,
                                                    BoundaryPfn,
                                                    SizeInPages,
                                                    CallingAddress);
        //
        // An existing range of nonpaged pool satisfies the requirements
        // so return it now.
        //

        if (BaseAddress != NULL) {
            return BaseAddress;
        }
    }

    //
    // Suitable pool was not allocated via the pool allocator.
    // Manually search for a page range which meets the requirements.
    //

    Page = MiFindContiguousPages (LowestPfn,
                                  HighestPfn,
                                  BoundaryPfn,
                                  SizeInPages,
                                  CacheType);

    if (Page == 0) {
        return NULL;
    }

    PhysicalAddress.QuadPart = Page;
    PhysicalAddress.QuadPart = PhysicalAddress.QuadPart << PAGE_SHIFT;

    BaseAddress = MmMapIoSpace (PhysicalAddress,
                                SizeInPages << PAGE_SHIFT,
                                CacheType);

    if (BaseAddress == NULL) {
        MiFreeContiguousPages (Page, SizeInPages);
        return NULL;
    }

    Pfn1 = MI_PFN_ELEMENT (Page);
    EndPfn = Pfn1 + SizeInPages;

    PointerPte = MiGetPteAddress (BaseAddress);
    do {
        Pfn1->PteAddress = PointerPte;
        Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress(PointerPte));
        Pfn1 += 1;
        PointerPte += 1;
    } while (Pfn1 < EndPfn);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_CONTIGUOUS_PAGES, SizeInPages);

    MiInsertContiguousTag (BaseAddress,
                           SizeInPages << PAGE_SHIFT,
                           CallingAddress);

    return BaseAddress;
}


PFN_NUMBER
MiFindLargePageMemory (
    IN PCOLORED_PAGE_INFO ColoredPageInfoBase,
    IN PFN_NUMBER SizeInPages,
    IN MM_PROTECTION_MASK ProtectionMask,
    OUT PPFN_NUMBER OutZeroCount
    )

/*++

Routine Description:

    This function searches the free, zeroed, standby and modified lists
    for contiguous pages to satisfy the request.

    Note the caller must zero the pages on return if these are made visible
    to the user.

Arguments:

    ColoredPageInfoBase - Supplies the colored page info structure to hang
                          allocated pages off of.  This allows the caller to
                          zero only pages that need zeroing, and to easily
                          do those in parallel.

    SizeInPages - Supplies the number of pages to allocate.

    ProtectionMask - Supplies the protection mask the caller is going to use.

    OutZeroCount - Receives the number of pages that need to be zeroed.

Return Value:

    0 - a contiguous range could not be found to satisfy the request.

    NON-0 - Returns the starting page frame number of the allocated physically
            contiguous memory.

Environment:

    Kernel mode, APCs disabled, AddressCreation mutex held.

    The caller must bring in PAGELK.

    The caller has already charged commitment for the range (typically by
    virtue of the VAD insert) so no commit is charged here.

--*/
{
    ULONG Color;
    PKTHREAD Thread;
    ULONG CurrentNodeColor;
    PFN_NUMBER ZeroCount;
    LOGICAL NeedToZero;
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    PMMPFN BoundaryPfn;
    KIRQL OldIrql;
    ULONG start;
    PFN_NUMBER count;
    PFN_NUMBER Page;
    PFN_NUMBER NewPage;
    PFN_NUMBER LastPage;
    PFN_NUMBER found;
    PFN_NUMBER BoundaryMask;
    PCOLORED_PAGE_INFO ColoredPageInfo;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    MEMORY_CACHING_TYPE CacheType;
    LOGICAL FlushTbNeeded;

    FlushTbNeeded = FALSE;

    PAGED_CODE ();

#ifdef _X86_
    ASSERT (KeFeatureBits & KF_LARGE_PAGE);
#endif

    ASSERT (SizeInPages != 0);

    BoundaryMask = (PFN_NUMBER) ((MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT) - 1);

    start = 0;
    found = 0;
    Pfn1 = NULL;
    ZeroCount = 0;

    CacheType = MmCached;
    if (MI_IS_NOCACHE (ProtectionMask)) {
        CacheType = MmNonCached;
    }
    else if (MI_IS_WRITECOMBINE (ProtectionMask)) {
        CacheType = MmWriteCombined;
    }

    CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, 0);

    //
    // Charge resident available pages.
    //

    LOCK_PFN (OldIrql);

    MiDeferredUnlockPages (MI_DEFER_PFN_HELD);

    if ((SPFN_NUMBER)SizeInPages > MI_NONPAGEABLE_MEMORY_AVAILABLE()) {
        UNLOCK_PFN (OldIrql);
        return 0;
    }

    //
    // Systems utilizing memory compression may have more
    // pages on the zero, free and standby lists than we
    // want to give out.  Explicitly check MmAvailablePages
    // instead (and recheck whenever the PFN lock is released
    // and reacquired).
    //

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        UNLOCK_PFN (OldIrql);
        return 0;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_ALLOCATE_LARGE_PAGES);

    UNLOCK_PFN (OldIrql);

    Page = 0;

    //
    // Search the PFN database for pages that meet the requirements.
    // For NUMA configurations, try the current local node first.
    //

#define ANY_COLOR_OK 0xFFFFFFFF      // Indicate any color is ok.

    CurrentNodeColor = ANY_COLOR_OK;

#if defined(MI_MULTINODE)
    if (KeNumberNodes > 1) {
        CurrentNodeColor = KeGetCurrentPrcb()->NodeColor;
    }
#endif

    Thread = KeGetCurrentThread ();

    KeEnterGuardedRegionThread (Thread);

    MI_LOCK_DYNAMIC_MEMORY_SHARED ();

rescan:

    for ( ; start != MmPhysicalMemoryBlock->NumberOfRuns; start += 1) {

        count = MmPhysicalMemoryBlock->Run[start].PageCount;
        Page = MmPhysicalMemoryBlock->Run[start].BasePage;

        //
        // Close the gaps, then examine the range for a fit.
        //

        LastPage = Page + count; 

        if ((Page & BoundaryMask) || (Page == 0)) {
            NewPage = MI_ROUND_TO_SIZE (Page, (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT));

            if (NewPage < Page) {
                continue;
            }

            Page = NewPage;

            if (Page == 0) {
                Page = (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);
            }

            if (Page >= LastPage) {
                continue;
            }
        }

        if (LastPage & BoundaryMask) {
            LastPage &= ~BoundaryMask;

            if (Page >= LastPage) {
                continue;
            }
        }

        if (Page + SizeInPages > LastPage) {
            continue;
        }

        count = LastPage - Page + 1;

        ASSERT (count != 0);
    
        //
        // A fit may be possible in this run, check whether the pages
        // are on the right list.
        //

        found = 0;
        Pfn1 = MI_PFN_ELEMENT (Page);

        while (Page < LastPage) {

            if ((Pfn1->u3.e1.PageLocation <= StandbyPageList) &&
                (Pfn1->u1.Flink != 0) &&
                (Pfn1->u2.Blink != 0) &&
                (Pfn1->u3.e2.ReferenceCount == 0) &&
                ((CurrentNodeColor == ANY_COLOR_OK) ||
                 (Pfn1->u3.e1.PageColor == CurrentNodeColor))) {

                found += 1;

                if (found == SizeInPages) {

                    //
                    // Lock the PFN database and see if the pages are
                    // still available for us.
                    //

                    Pfn1 -= (found - 1);
                    Page -= (found - 1);

                    LOCK_PFN (OldIrql);

                    do {

                        if ((Pfn1->u3.e1.PageLocation <= StandbyPageList) &&
                            (Pfn1->u1.Flink != 0) &&
                            (Pfn1->u2.Blink != 0) &&
                            (Pfn1->u3.e2.ReferenceCount == 0)) {

                            NOTHING;            // Good page
                        }
                        else {
                            break;
                        }

                        found -= 1;

                        if (found == 0) {

                            //
                            // All the pages matched the criteria, keep the
                            // PFN lock, remove them and map them for our
                            // caller.
                            //

                            goto Done;
                        }

                        Pfn1 += 1;
                        Page += 1;

                    } while (TRUE);

#if DBG
                    if (MiShowStuckPages != 0) {
                        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                            "MiFindLargePages : could not claim stolen PFN %p\n",
                            Page);
                        if (MiShowStuckPages & 0x8) {
                            DbgBreakPoint ();
                        }
                    }
#endif
                    UNLOCK_PFN (OldIrql);

                    //
                    // Restart the search at the first possible page.
                    //

                    found = 0;
                }
            }
            else {
#if DBG
                if (MiShowStuckPages != 0) {
                    DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                        "MiFindLargePages : could not claim PFN %p %x %x\n",
                        Page, Pfn1->u3.e1, Pfn1->u4.EntireFrame);
                    if (MiShowStuckPages & 0x8) {
                        DbgBreakPoint ();
                    }
                }
#endif
                found = 0;
            }

            Page += 1;
            Pfn1 += 1;

            if (found == 0) {

                //
                // The last page interrogated wasn't available so skip
                // ahead to the next acceptable boundary.
                //

                NewPage = MI_ROUND_TO_SIZE (Page,
                                (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT));

                if ((NewPage == 0) || (NewPage < Page) || (NewPage >= LastPage)) {

                    //
                    // Skip the rest of this entry.
                    //

                    Page = LastPage;
                    continue;
                }

                Page = NewPage;
                Pfn1 = MI_PFN_ELEMENT (Page);
            }
        }
    }

    //
    // The desired physical pages could not be allocated.  If we were trying
    // for local memory (ie: NUMA machine), then expand our search to include
    // any node and try one last time.
    //

    if (CurrentNodeColor != ANY_COLOR_OK) {
        CurrentNodeColor = ANY_COLOR_OK;
        start = 0;
        found = 0;
        goto rescan;
    }

    MI_UNLOCK_DYNAMIC_MEMORY_SHARED ();
    KeLeaveGuardedRegionThread (Thread);
    MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_LARGE_PAGES);
    return 0;

Done:

    //
    // A match has been found, remove these pages,
    // map them and return.  The PFN lock is held.
    //

    ASSERT (start != MmPhysicalMemoryBlock->NumberOfRuns);
    ASSERT (Page - SizeInPages + 1 != 0);

    //
    // Systems utilizing memory compression may have more
    // pages on the zero, free and standby lists than we
    // want to give out.  Explicitly check MmAvailablePages
    // instead (and recheck whenever the PFN lock is
    // released and reacquired).
    //

    if ((SPFN_NUMBER)SizeInPages > (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {
        UNLOCK_PFN (OldIrql);
        MI_INCREMENT_RESIDENT_AVAILABLE (SizeInPages, MM_RESAVAIL_FREE_LARGE_PAGES);

        MI_UNLOCK_DYNAMIC_MEMORY_SHARED ();
        KeLeaveGuardedRegionThread (Thread);

        return 0;
    }

    EndPfn = Pfn1 - SizeInPages + 1;

    BoundaryPfn = Pfn1 - (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

    do {

        NeedToZero = TRUE;

        if (Pfn1->u3.e1.PageLocation == StandbyPageList) {
            MiUnlinkPageFromList (Pfn1);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            MiRestoreTransitionPte (Pfn1);
        }
        else {
            if (Pfn1->u3.e1.PageLocation == ZeroedPageList) {
                NeedToZero = FALSE;
            }
            MiUnlinkFreeOrZeroedPage (Pfn1);
        }

        Pfn1->u3.e2.ReferenceCount = 1;
        Pfn1->u2.ShareCount = 1;
        MI_SET_PFN_DELETED(Pfn1);
        Pfn1->u4.PteFrame = MI_MAGIC_AWE_PTEFRAME;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;

        if (Pfn1->u3.e1.CacheAttribute != CacheAttribute) {
            FlushTbNeeded = TRUE;
            Pfn1->u3.e1.CacheAttribute = CacheAttribute;
        }

        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1->u3.e1.EndOfAllocation = 0;
        Pfn1->u4.VerifierAllocation = 0;
        Pfn1->u3.e1.LargeSessionAllocation = 0;

        ASSERT (Pfn1->u4.AweAllocation == 0);
        Pfn1->u4.AweAllocation = 1;

        Pfn1->u3.e1.PrototypePte = 0;

        //
        // Add free and standby pages to the list of pages to be zeroed
        // by our caller.
        //

        if (NeedToZero == TRUE) {
            Color = MI_GET_COLOR_FROM_LIST_ENTRY (Page, Pfn1);

            ColoredPageInfo = &ColoredPageInfoBase[Color];

            Pfn1->OriginalPte.u.Long = (ULONG_PTR) ColoredPageInfo->PfnAllocation;
            ColoredPageInfo->PfnAllocation = Pfn1;
            ColoredPageInfo->PagesQueued += 1;
            ZeroCount += 1;
        }
        else {
            Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
        }

        if (Pfn1 == EndPfn) {
            break;
        }

        Pfn1 -= 1;

        if (Pfn1 == BoundaryPfn) {
            BoundaryPfn = Pfn1 - (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);
        }

    } while (TRUE);

    Pfn1->u3.e1.StartOfAllocation = 1;
    (Pfn1 + SizeInPages - 1)->u3.e1.EndOfAllocation = 1;

    UNLOCK_PFN (OldIrql);

    MI_UNLOCK_DYNAMIC_MEMORY_SHARED ();
    KeLeaveGuardedRegionThread (Thread);

    if (FlushTbNeeded) {
        MI_FLUSH_ENTIRE_TB_FOR_ATTRIBUTE_CHANGE (CacheAttribute);
    }

    Page = Page - SizeInPages + 1;
    ASSERT (Page != 0);
    ASSERT (Pfn1 == MI_PFN_ELEMENT (Page));

    MM_TRACK_COMMIT (MM_DBG_COMMIT_CHARGE_LARGE_PAGES, SizeInPages);

    *OutZeroCount = ZeroCount;
    return Page;
}


VOID
MiFreeLargePageMemory (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER SizeInPages
    )

/*++

Routine Description:

    This function returns a contiguous large page allocation to the free
    memory lists.

Arguments:

    VirtualAddress - Supplies the starting page frame index to free.

    SizeInPages - Supplies the number of pages to free.

Return Value:

    None.

Environment:

    Kernel mode, IRQL of APC_LEVEL or below.

--*/
{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PKTHREAD CurrentThread;
    PFN_NUMBER LastPageFrameIndex;
    LONG EntryCount;
    LONG OriginalCount;
    LOGICAL FlushTbNeeded;

    PAGED_CODE ();

    ASSERT (SizeInPages != 0);

    LastPageFrameIndex = PageFrameIndex + SizeInPages;

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    FlushTbNeeded = FALSE;

    //
    // The actual commitment for this range (and its page table pages, etc)
    // is released when the vad is removed.  Because we will release commitment
    // below for each physical page, temporarily increase the charge now so
    // it all balances out.  Block user APCs so a suspend can't stop us.
    //

    CurrentThread = KeGetCurrentThread ();

    KeEnterCriticalRegionThread (CurrentThread);

    MiChargeCommitmentCantExpand (SizeInPages, TRUE);

    LOCK_PFN (OldIrql);

    do {
        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);

        ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
        ASSERT (Pfn1->u3.e1.PrototypePte == 0);
        ASSERT (Pfn1->u4.VerifierAllocation == 0);
        ASSERT (Pfn1->u4.AweAllocation == 1);
        ASSERT (MI_IS_PFN_DELETED (Pfn1) == TRUE);

        //
        // The most likely case is that the pages will be reused with
        // a fully cached attribute.  Convert the pages now (and flush
        // the TB) to avoid having to flush the TB as each page gets
        // reallocated.
        //

        if (Pfn1->u3.e1.CacheAttribute != MiCached) {
            FlushTbNeeded = TRUE;
        }

        Pfn1->u3.e1.CacheAttribute = MiCached;

        Pfn1->u3.e1.StartOfAllocation = 0;
        Pfn1->u3.e1.EndOfAllocation = 0;

        Pfn1->u2.ShareCount = 0;

#if DBG
        Pfn1->u3.e1.PageLocation = StandbyPageList;
#endif

        do {

            EntryCount = ReadForWriteAccess (&Pfn1->AweReferenceCount);

            ASSERT ((LONG)EntryCount > 0);
            ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

            OriginalCount = InterlockedCompareExchange (&Pfn1->AweReferenceCount,
                                                        EntryCount - 1,
                                                        EntryCount);

            if (OriginalCount == EntryCount) {

                //
                // This thread can be racing against other threads
                // calling MmUnlockPages.  All threads can safely do
                // interlocked decrements on the "AWE reference count".
                // Whichever thread drives it to zero is responsible for
                // decrementing the actual PFN reference count (which may
                // be greater than 1 due to other non-AWE API calls being
                // used on the same page).  The thread that drives this
                // reference count to zero must put the page on the actual
                // freelist at that time and decrement various resident
                // available and commitment counters also.
                //

                if (OriginalCount == 1) {

                    //
                    // This thread has driven the AWE reference count to
                    // zero so it must initiate a decrement of the PFN
                    // reference count (while holding the PFN lock), etc.
                    //
                    // This path should be the frequent one since typically
                    // I/Os complete before these types of pages are
                    // freed by the app.
                    //
                    // Note this routine returns resident available and
                    // commitment for the page.
                    //

                    MiDecrementReferenceCountForAwePage (Pfn1, TRUE);
                }

                break;
            }
        } while (TRUE);

        //
        // Nothing magic about the divisor here - just releasing the PFN lock
        // periodically to allow other processors and DPCs a chance to execute.
        //

        if ((PageFrameIndex & 0xF) == 0) {

            if (FlushTbNeeded == TRUE) {
                MI_FLUSH_TB_FOR_CACHED_ATTRIBUTE ();
                FlushTbNeeded = FALSE;
            }

            UNLOCK_PFN (OldIrql);

            LOCK_PFN (OldIrql);
        }

        Pfn1 += 1;
        PageFrameIndex += 1;

    } while (PageFrameIndex <  LastPageFrameIndex);

    if (FlushTbNeeded == TRUE) {
        MI_FLUSH_TB_FOR_CACHED_ATTRIBUTE ();
    }

    UNLOCK_PFN (OldIrql);

    KeLeaveCriticalRegionThread (CurrentThread);

    return;
}

LOGICAL
MmIsSessionAddress (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This function returns TRUE if a session address is specified.
    FALSE is returned if not.

Arguments:

    VirtualAddress - Supplies the address in question.

Return Value:

    See above.

Environment:

    Kernel mode.

--*/

{
    return MI_IS_SESSION_ADDRESS (VirtualAddress);
}

ULONG
MmGetSizeOfBigPoolAllocation (
    IN PVOID StartingAddress
    )

/*++

Routine Description:

    This function returns the number of pages consumed by the argument
    big pool allocation.  It is assumed that the caller still owns the
    allocation (and guarantees it cannot be freed from underneath us)
    so this routine can run lock-free.

Arguments:

    StartingAddress - Supplies the starting address which was returned
                      in a previous call to MiAllocatePoolPages.

Return Value:

    Returns the number of pages allocated.

Environment:

    These functions are used by the general pool free routines
    and should not be called directly.

--*/

{
    PMMPFN StartPfn;
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PMMPTE StartPte;
    ULONG StartPosition;
    PFN_NUMBER i;
    PFN_NUMBER NumberOfPages;
    POOL_TYPE PoolType;
    PMM_PAGED_POOL_INFO PagedPoolInfo;
    PULONG BitMap;
#if DBG
    PMM_SESSION_SPACE SessionSpace;
    PKGUARDED_MUTEX PoolMutex;
#endif

    if ((StartingAddress >= MmPagedPoolStart) &&
        (StartingAddress <= MmPagedPoolEnd)) {
        PoolType = PagedPool;
        PagedPoolInfo = &MmPagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)MmPagedPoolStart) >> PAGE_SHIFT);
#if DBG
        PoolMutex = &MmPagedPoolMutex;
#endif
    }
    else if (MI_IS_SESSION_POOL_ADDRESS (StartingAddress) == TRUE) {
        PoolType = PagedPool;
        ASSERT (MmSessionSpace != NULL);
        PagedPoolInfo = &MmSessionSpace->PagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)MmSessionSpace->PagedPoolStart) >> PAGE_SHIFT);
#if DBG
        SessionSpace = SESSION_GLOBAL (MmSessionSpace);
        PoolMutex = &SessionSpace->PagedPoolMutex;
#endif
    }
    else {

        if (StartingAddress < MM_SYSTEM_RANGE_START) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x44,
                          (ULONG_PTR)StartingAddress,
                          (ULONG_PTR)MM_SYSTEM_RANGE_START,
                          0);
        }

        PoolType = NonPagedPool;
        PagedPoolInfo = &MmPagedPoolInfo;
        StartPosition = (ULONG)(((PCHAR)StartingAddress -
                          (PCHAR)MmNonPagedPoolStart) >> PAGE_SHIFT);

        //
        // Check to ensure this page is really the start of an allocation.
        //

        if (MI_IS_PHYSICAL_ADDRESS (StartingAddress)) {

            //
            // On certain architectures, virtual addresses
            // may be physical and hence have no corresponding PTE.
            //

            PointerPte = NULL;
            Pfn1 = MI_PFN_ELEMENT (MI_CONVERT_PHYSICAL_TO_PFN (StartingAddress));
            ASSERT (StartPosition < (ULONG) BYTES_TO_PAGES (MmSizeOfNonPagedPoolInBytes));

            if ((StartingAddress < MmNonPagedPoolStart) ||
                (StartingAddress >= MmNonPagedPoolEnd0)) {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0x45,
                              (ULONG_PTR)StartingAddress,
                              0,
                              0);
            }
        }
        else {
            PointerPte = MiGetPteAddress (StartingAddress);

            if (((StartingAddress >= MmNonPagedPoolExpansionStart) &&
                (StartingAddress < MmNonPagedPoolEnd)) ||
                ((StartingAddress >= MmNonPagedPoolStart) &&
                (StartingAddress < MmNonPagedPoolEnd0))) {
                    NOTHING;
            }
            else {
                KeBugCheckEx (BAD_POOL_CALLER,
                              0x46,
                              (ULONG_PTR)StartingAddress,
                              0,
                              0);
            }
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
        }

        if (Pfn1->u3.e1.StartOfAllocation == 0) {
            KeBugCheckEx (BAD_POOL_CALLER,
                          0x47,
                          (ULONG_PTR) StartingAddress,
                          (ULONG_PTR) MI_PFN_ELEMENT_TO_INDEX (Pfn1),
                          MmHighestPhysicalPage);
        }

        StartPfn = Pfn1;
        NumberOfPages = 0;

        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

        //
        // Find end of allocation.
        //

        if (PointerPte == NULL) {
            while (Pfn1->u3.e1.EndOfAllocation == 0) {
                Pfn1 += 1;
            }
            NumberOfPages = Pfn1 - StartPfn + 1;
        }
        else {
            StartPte = PointerPte;
            while (Pfn1->u3.e1.EndOfAllocation == 0) {
                PointerPte += 1;
                Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            }
            NumberOfPages = PointerPte - StartPte + 1;
        }

        return (ULONG) NumberOfPages;
    }

    //
    // Paged pool (global or session).
    //
    // Check to ensure this page is really the start of an allocation.
    //

    i = StartPosition;

    //
    // Paged pool.  Need to verify start of allocation using
    // end of allocation bitmap.
    //

    if (!RtlCheckBit (PagedPoolInfo->PagedPoolAllocationMap, StartPosition)) {
        KeBugCheckEx (BAD_POOL_CALLER,
                      0x48,
                      (ULONG_PTR)StartingAddress,
                      (ULONG_PTR)StartPosition,
                      MmSizeOfPagedPoolInBytes);
    }

#if DBG

    if (StartPosition > 0) {

        KeAcquireGuardedMutex (PoolMutex);

        if (RtlCheckBit (PagedPoolInfo->PagedPoolAllocationMap, StartPosition - 1)) {
            if (!RtlCheckBit (PagedPoolInfo->EndOfPagedPoolBitmap, StartPosition - 1)) {

                //
                // In the middle of an allocation... bugcheck.
                //

                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x41286,
                              (ULONG_PTR)PagedPoolInfo->PagedPoolAllocationMap,
                              (ULONG_PTR)PagedPoolInfo->EndOfPagedPoolBitmap,
                              StartPosition);
            }
        }

        KeReleaseGuardedMutex (PoolMutex);
    }
#endif

    //
    // Find the last allocated page.
    //

    BitMap = PagedPoolInfo->EndOfPagedPoolBitmap->Buffer;

    while (!MI_CHECK_BIT (BitMap, i)) {
        i += 1;
    }

    NumberOfPages = i - StartPosition + 1;

    return (ULONG)NumberOfPages;
}

//
// The number of large page ranges must always be larger than the number of
// translation register entries for the target platform.
//

#define MI_MAX_LARGE_PAGE_RANGES 64

typedef struct _MI_LARGE_PAGE_RANGES {
    PFN_NUMBER StartFrame;
    PFN_NUMBER LastFrame;
} MI_LARGE_PAGE_RANGES, *PMI_LARGE_PAGE_RANGES;

ULONG MiLargePageRangeIndex;
MI_LARGE_PAGE_RANGES MiLargePageRanges[MI_MAX_LARGE_PAGE_RANGES];


LOGICAL
MiMustFrameBeCached (
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    This routine checks whether the specified page frame must be mapped
    fully cached because it is already part of a large page which is fully
    cached.  This must be detected otherwise we would be creating an
    incoherent overlapping TB entry as the same physical page would be
    mapped by 2 different TB entries with different cache attributes.

Arguments:

    PageFrameIndex - Supplies the page frame index in question.

Return Value:

    TRUE if the page must be mapped as fully cachable, FALSE if not.

Environment:

    Kernel mode.  IRQL of DISPATCH_LEVEL or below.
    
    PFN lock must be held for the results to relied on, but note callers will
    sometimes call without it for a preliminary scan and then repeat it with
    the lock held.

--*/
{
    PMI_LARGE_PAGE_RANGES Range;
    PMI_LARGE_PAGE_RANGES LastValidRange;

    Range = MiLargePageRanges;
    LastValidRange = MiLargePageRanges + MiLargePageRangeIndex;

    while (Range < LastValidRange) {

        if ((PageFrameIndex >= Range->StartFrame) &&
            (PageFrameIndex <= Range->LastFrame)) {

            return TRUE;
        }

        Range += 1;
    }
    return FALSE;
}

VOID
MiSyncCachedRanges (
    VOID
    )

/*++

Routine Description:

    This routine searches the cached list for PFN-mapped entries and ripples
    the must-be-cached bits into each PFN entry.
    
Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, PFN lock NOT held.

--*/
{
    ULONG i;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN LastPfn;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPageFrameIndex;

    for (i = 0; i < MiLargePageRangeIndex; i += 1) {

        PageFrameIndex = MiLargePageRanges[i].StartFrame;
        LastPageFrameIndex = MiLargePageRanges[i].LastFrame;

        if (MI_IS_PFN (PageFrameIndex)) {
    
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            LastPfn = MI_PFN_ELEMENT (LastPageFrameIndex);
        
            LOCK_PFN (OldIrql);
        
            while (Pfn1 <= LastPfn) {
                Pfn1->u4.MustBeCached = 1;
                Pfn1 += 1;
            }
        
            UNLOCK_PFN (OldIrql);
        }
    
    }

    return;
}

LOGICAL
MiAddCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    )

/*++

Routine Description:

    This routine adds the specified page range to the "must be mapped
    fully cached" list.
    
    This is typically called with a range which is about to be mapped with
    large pages fully cached and so no portion of the range can ever be
    mapped noncached or writecombined otherwise we would be creating an
    incoherent overlapping TB entry as the same physical page would be
    mapped by 2 different TB entries with different cache attributes.

Arguments:

    PageFrameIndex - Supplies the starting page frame index to insert.

    LastPageFrameIndex - Supplies the last page frame index to insert.

Return Value:

    TRUE if the range was successfully inserted, FALSE if not.

Environment:

    Kernel mode, PFN lock NOT held.

--*/
{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN LastPfn;

    if (MiLargePageRangeIndex >= MI_MAX_LARGE_PAGE_RANGES) {
        return FALSE;
    }

    ASSERT (MiLargePageRanges[MiLargePageRangeIndex].StartFrame == 0);
    ASSERT (MiLargePageRanges[MiLargePageRangeIndex].LastFrame == 0);

    MiLargePageRanges[MiLargePageRangeIndex].StartFrame = PageFrameIndex; 
    MiLargePageRanges[MiLargePageRangeIndex].LastFrame = LastPageFrameIndex;

    MiLargePageRangeIndex += 1;

    if ((MiPfnBitMap.Buffer != NULL) && (MI_IS_PFN (PageFrameIndex))) {

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        LastPfn = MI_PFN_ELEMENT (LastPageFrameIndex);
    
        LOCK_PFN (OldIrql);
    
        while (Pfn1 <= LastPfn) {
            Pfn1->u4.MustBeCached = 1;
            Pfn1 += 1;
        }
    
        UNLOCK_PFN (OldIrql);
    }

    return TRUE;
}

VOID
MiRemoveCachedRange (
    IN PFN_NUMBER PageFrameIndex,
    IN PFN_NUMBER LastPageFrameIndex
    )

/*++

Routine Description:

    This routine removes the specified page range from the "must be mapped
    fully cached" list.
    
    This is typically called with a range which was mapped with
    large pages fully cached and so no portion of the range can ever be
    mapped noncached or writecombined otherwise we would be creating an
    incoherent overlapping TB entry as the same physical page would be
    mapped by 2 different TB entries with different cache attributes.

    The range is now being unmapped so we must also remove it from this list.

Arguments:

    PageFrameIndex - Supplies the starting page frame index to remove.

    LastPageFrameIndex - Supplies the last page frame index to remove.

Return Value:

    None.

Environment:

    Kernel mode, PFN lock NOT held.

--*/
{
    ULONG i;
    PMI_LARGE_PAGE_RANGES Range;
    PMMPFN Pfn1;
    PMMPFN LastPfn;
    KIRQL OldIrql;

    ASSERT (MiLargePageRangeIndex <= MI_MAX_LARGE_PAGE_RANGES);

    Range = MiLargePageRanges;

    for (i = 0; i < MiLargePageRangeIndex; i += 1, Range += 1) {

        if ((PageFrameIndex == Range->StartFrame) &&
            (LastPageFrameIndex == Range->LastFrame)) {

            //
            // Found it, slide everything else down to preserve any other
            // non zero ranges.  Decrement the last valid entry so that
            // searches don't need to walk the whole thing.
            //

            while (i < MI_MAX_LARGE_PAGE_RANGES - 1) {
                *Range = *(Range + 1);
                Range += 1;
                i += 1;
            }

            Range->StartFrame = 0;
            Range->LastFrame = 0;

            MiLargePageRangeIndex -= 1;

            if ((MiPfnBitMap.Buffer != NULL) && (MI_IS_PFN (PageFrameIndex))) {
    
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                LastPfn = MI_PFN_ELEMENT (LastPageFrameIndex);
    
                LOCK_PFN (OldIrql);
    
                while (Pfn1 <= LastPfn) {
                    Pfn1->u4.MustBeCached = 0;
                    Pfn1 += 1;
                }
    
                UNLOCK_PFN (OldIrql);
            }

            return;
        }
    }

    ASSERT (FALSE);

    return;
}

