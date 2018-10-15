/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   pfnlist.c

Abstract:

    This module contains the routines to manipulate pages within the
    Page Frame Database.

Revision History:

--*/
#include "mi.h"

//
// The following line will generate an error if the number of colored
// free page lists is not 2, ie ZeroedPageList and FreePageList.  If
// this number is changed, the size of the FreeCount array in the kernel
// node structure (KNODE) must be updated.
//

C_ASSERT(StandbyPageList == 2);

KEVENT MmAvailablePagesEventHigh;

PFN_NUMBER MmTransitionPrivatePages;
PFN_NUMBER MmTransitionSharedPages;

#define MI_TALLY_TRANSITION_PAGE_ADDITION(Pfn) \
    if (Pfn->u3.e1.PrototypePte) { \
        MmTransitionSharedPages += 1; \
    } \
    else { \
        MmTransitionPrivatePages += 1; \
    }

#define MI_TALLY_TRANSITION_PAGE_REMOVAL(Pfn) \
    if (Pfn->u3.e1.PrototypePte) { \
        MmTransitionSharedPages -= 1; \
    } \
    else { \
        MmTransitionPrivatePages -= 1; \
    }

//
// This counter is used to determine if standby pages are being cannibalized
// for use as free pages and therefore more aging should be attempted.
//

ULONG MmStandbyRePurposed;

MM_LDW_WORK_CONTEXT MiLastChanceLdwContext;
    
ULONG MiAvailablePagesEventLowSets;
ULONG MiAvailablePagesEventHighSets;

extern ULONG MmSystemShutdown;

extern NTSTATUS MiLastModifiedWriteError;
extern NTSTATUS MiLastMappedWriteError;

PFN_NUMBER
FASTCALL
MiRemovePageByColor (
    IN PFN_NUMBER Page,
    IN ULONG PageColor
    );

VOID
FASTCALL
MiLogPfnInformation (
    IN PMMPFN Pfn1,
    IN USHORT Reason
    );

extern LOGICAL MiZeroingDisabled;

#if DBG
#if defined(_X86_) || defined(_AMD64_)
ULONG MiSaveStacks = 1;
#endif
#endif

#define MI_SNAP_PFN(_Pfn, dest, callerid)

VOID
MiInitializePfnTracing (
    VOID
    )
{
}


VOID
FASTCALL
MiInsertPageInFreeList (
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    This procedure inserts a page at the end of the free list.

Arguments:

    PageFrameIndex - Supplies the physical page number to insert in the list.

Return Value:

    None.

Environment:

    PFN lock held.

--*/

{
    PFN_NUMBER last;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    ULONG Color;
    MMLISTS ListName;
    PMMPFNLIST ListHead;
    PMMCOLOR_TABLES ColorHead;

    MM_PFN_LOCK_ASSERT();
    ASSERT ((PageFrameIndex != 0) &&
            (PageFrameIndex <= MmHighestPhysicalPage) &&
            (PageFrameIndex >= MmLowestPhysicalPage));

    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

    MI_SNAP_DATA (Pfn1, Pfn1->PteAddress, 8);

    MI_SNAP_PFN(Pfn1, FreePageList, 0x1);

    if (PERFINFO_IS_GROUP_ON(PERF_MEMORY)) {
        MiLogPfnInformation (Pfn1, PERFINFO_LOG_TYPE_INSERTINFREELIST);
    }

#if DBG
#if defined(_X86_)
    if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
        Pfn1->u4.MustBeCached = 0;
    }
#endif
#endif

    ASSERT (Pfn1->u4.MustBeCached == 0);

    //
    // The page is being reused, so reset its priority.
    //

    MI_RESET_PFN_PRIORITY (Pfn1);

    ASSERT (Pfn1->u3.e1.Rom != 1);

    if (Pfn1->u3.e1.RemovalRequested == 1) {
        MiInsertPageInList (&MmBadPageListHead, PageFrameIndex);
        return;
    }

    ListHead = &MmFreePageListHead;
    ListName = FreePageList;

#if DBG
#if defined(_X86_) || defined(_AMD64_)
    if ((MiSaveStacks != 0) && (MmFirstReservedMappingPte != NULL)) {

        ULONG_PTR StackPointer;
        ULONG_PTR StackBytes;
        PULONG_PTR DataPage;
        PEPROCESS Process;

        MiGetStackPointer (&StackPointer);

        Process = PsGetCurrentProcess ();

        DataPage = MiMapPageInHyperSpaceAtDpc (Process, PageFrameIndex);

        //
        // Copy the callstack into the middle of the page (since special pool
        // is using the beginning of the freed page).
        //

        DataPage[PAGE_SIZE / (2 * sizeof(ULONG_PTR))] = StackPointer;
    
        //
        // For now, don't get fancy with copying more than what's in the current
        // stack page.  To do so would require checking the thread stack limits,
        // DPC stack limits, etc.
        //
    
        StackBytes = PAGE_SIZE - BYTE_OFFSET(StackPointer);
        DataPage[PAGE_SIZE / (2 * sizeof (ULONG_PTR)) + 1] = StackBytes;
    
        if (StackBytes != 0) {
    
            if (StackBytes > MI_STACK_BYTES) {
                StackBytes = MI_STACK_BYTES;
            }
    
            RtlCopyMemory ((PVOID)&DataPage[PAGE_SIZE / (2 * sizeof (ULONG_PTR)) + 2], (PVOID)StackPointer, StackBytes);
        }

        MiUnmapPageInHyperSpaceFromDpc (Process, DataPage);
    }
#endif
#endif

    ASSERT (Pfn1->u4.VerifierAllocation == 0);

    //
    // Check to ensure the reference count for the page is zero.
    //

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);

    ListHead->Total += 1;  // One more page on the list.

    //
    // Inserting the page at the front would make better use of the hardware
    // caches, but it makes debugging much harder because the pages get
    // reused so quickly.  For now, continue to insert at the back of the list.
    //

    last = ListHead->Blink;

    if (last != MM_EMPTY_LIST) {
        Pfn2 = MI_PFN_ELEMENT (last);
        Pfn2->u1.Flink = PageFrameIndex;
    }
    else {

        //
        // List is empty, add the page to the ListHead.
        //

        ListHead->Flink = PageFrameIndex;
    }

    ListHead->Blink = PageFrameIndex;
    Pfn1->u1.Flink = MM_EMPTY_LIST;
    Pfn1->u2.Blink = last;

    Pfn1->u3.e1.PageLocation = ListName;
    Pfn1->u4.InPageError = 0;
    Pfn1->u4.AweAllocation = 0;

    //
    // Update the count of usable pages in the system.  If the count
    // transitions from 0 to 1, the event associated with available
    // pages should become true.
    //

    MmAvailablePages += 1;

    //
    // A page has just become available, check to see if the
    // page wait events should be signaled.
    //

    if (MmAvailablePages <= MM_HIGH_LIMIT) {
        if (MmAvailablePages == MM_HIGH_LIMIT) {
            KeSetEvent (&MmAvailablePagesEventHigh, 0, FALSE);
            MiAvailablePagesEventHighSets += 1;
        }
        else if (MmAvailablePages == MM_LOW_LIMIT) {
            KeSetEvent (&MmAvailablePagesEvent, 0, FALSE);
            MiAvailablePagesEventLowSets += 1;
        }
    }

    //
    // Signal applications if the freed page crosses a threshold.
    //

    if (MmAvailablePages == MmLowMemoryThreshold) {
        KeClearEvent (MiLowMemoryEvent);
    }
    else if (MmAvailablePages == MmHighMemoryThreshold) {
        KeSetEvent (MiHighMemoryEvent, 0, FALSE);
    }

#if defined(MI_MULTINODE)

    //
    // Increment the free page count for this node.
    //

    if (KeNumberNodes > 1) {
        KeNodeBlock[Pfn1->u3.e1.PageColor]->FreeCount[ListName]++;
    }

#endif

    //
    // We are adding a page to the free page list.
    // Add the page to the end of the correct colored page list.
    //

    Color = MI_GET_COLOR_FROM_LIST_ENTRY(PageFrameIndex, Pfn1);

    ColorHead = &MmFreePagesByColor[ListName][Color];

    if (ColorHead->Flink == MM_EMPTY_LIST) {

        //
        // This list is empty, add this as the first and last
        // entry.
        //

        Pfn1->u4.PteFrame = MM_EMPTY_LIST;
        ColorHead->Flink = PageFrameIndex;
    }
    else {
        Pfn2 = (PMMPFN)ColorHead->Blink;
        Pfn1->u4.PteFrame = MI_PFN_ELEMENT_TO_INDEX (Pfn2);
        Pfn2->OriginalPte.u.Long = PageFrameIndex;
    }
    ColorHead->Blink = Pfn1;
    ColorHead->Count += 1;
    Pfn1->OriginalPte.u.Long = MM_EMPTY_LIST;

    if ((ListHead->Total >= MmMinimumFreePagesToZero) &&
        (MmZeroingPageThreadActive == FALSE)) {

        //
        // There are enough pages on the free list, start
        // the zeroing page thread.
        //

        MmZeroingPageThreadActive = TRUE;
        KeSetEvent (&MmZeroingPageEvent, 0, FALSE);
    }

    return;
}

VOID
FASTCALL
MiInsertPageInList (
    IN PMMPFNLIST ListHead,
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    This procedure inserts a page at the end of the specified list (standby,
    bad, zeroed, modified).

Arguments:

    ListHead - Supplies the listhead of the list in which to insert the
               specified physical page.

    PageFrameIndex - Supplies the physical page number to insert in the list.

Return Value:

    None.

Environment:

    Must be holding the PFN database lock.

--*/

{
    PFN_NUMBER last;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    ULONG Color;
    MMLISTS ListName;
#if MI_BARRIER_SUPPORTED
    ULONG BarrierStamp;
#endif

    ASSERT (ListHead != &MmFreePageListHead);

    MM_PFN_LOCK_ASSERT();
    ASSERT ((PageFrameIndex != 0) &&
            (PageFrameIndex <= MmHighestPhysicalPage) &&
            (PageFrameIndex >= MmLowestPhysicalPage));

    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

    ListName = ListHead->ListName;

#if DBG
    if (ListName != BadPageList) {

#if defined(_X86_)
        if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
            Pfn1->u4.MustBeCached = 0;
        }
#endif

        ASSERT (Pfn1->u4.MustBeCached == 0);
    }
#endif

    MI_SNAP_PFN(Pfn1, ListName, 0x2);

#if DBG
    if (MmDebug & MM_DBG_PAGE_REF_COUNT) {

        if ((ListName == StandbyPageList) || (ListName == ModifiedPageList)) {

            PMMPTE PointerPte;
            PEPROCESS Process;

            if ((Pfn1->u3.e1.PrototypePte == 1)  &&
                (MmIsAddressValid (Pfn1->PteAddress))) {

                Process = NULL;
                PointerPte = Pfn1->PteAddress;
            }
            else {

                //
                // The page containing the prototype PTE is not valid,
                // map the page into hyperspace and reference it that way.
                //

                Process = PsGetCurrentProcess ();
                PointerPte = MiMapPageInHyperSpaceAtDpc (Process, Pfn1->u4.PteFrame);
                PointerPte = (PMMPTE)((PCHAR)PointerPte +
                                        MiGetByteOffset(Pfn1->PteAddress));
            }

            ASSERT ((MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerPte) == PageFrameIndex) ||
                    (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) == PageFrameIndex));
            ASSERT (PointerPte->u.Soft.Transition == 1);
            ASSERT (PointerPte->u.Soft.Prototype == 0);
            if (Process != NULL) {
                MiUnmapPageInHyperSpaceFromDpc (Process, PointerPte);
            }
        }
    }

    if ((ListName == StandbyPageList) || (ListName == ModifiedPageList)) {
        if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
           (Pfn1->OriginalPte.u.Soft.Transition == 1)) {
            KeBugCheckEx (MEMORY_MANAGEMENT, 0x8888, 0,0,0);
        }
    }
#endif

    //
    // Check to ensure the reference count for the page is zero.
    //

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    ASSERT (Pfn1->u3.e1.Rom != 1);

    if (ListHead == &MmStandbyPageListHead) {
        ListHead = &MmStandbyPageListByPriority [Pfn1->u4.Priority];
        ASSERT (ListHead->ListName == ListName);
    }

    ListHead->Total += 1;  // One more page on the list.

    if (ListHead == &MmModifiedPageListHead) {

        //
        // On MIPS R4000 modified pages destined for the paging file are
        // kept on separate lists which group pages of the same color
        // together.
        //

        if (Pfn1->OriginalPte.u.Soft.Prototype == 0) {

            //
            // This page is destined for the paging file (not
            // a mapped file).  Change the list head to the
            // appropriate colored list head.
            //

#if MM_MAXIMUM_NUMBER_OF_COLORS > 1
            ListHead = &MmModifiedPageListByColor [Pfn1->u3.e1.PageColor];
#else
            ListHead = &MmModifiedPageListByColor [0];
#endif
            ASSERT (ListHead->ListName == ListName);
            ListHead->Total += 1;
            MmTotalPagesForPagingFile += 1;
        }
        else {

            //
            // This page is destined for a mapped file (not
            // the paging file).  If there are no other pages currently
            // destined for the mapped file, start our timer so that we can
            // ensure that these pages make it to disk even if we don't pile
            // up enough of them to trigger the modified page writer or need
            // the memory.  If we don't do this here, then for this scenario,
            // only an orderly system shutdown will write them out (days,
            // weeks, months or years later) and any power out in between
            // means we'll have lost the data.
            //

            if (ListHead->Total - MmTotalPagesForPagingFile == 1) {

                //
                // Start the DPC timer because we're the first on the list.
                //

                if (MiTimerPending == FALSE) {
                    MiTimerPending = TRUE;

                    KeSetTimerEx (&MiModifiedPageWriterTimer,
                                  MiModifiedPageLife,
                                  0,
                                  &MiModifiedPageWriterTimerDpc);
                }
            }
        }
    }
    else if ((Pfn1->u3.e1.RemovalRequested == 1) &&
             (ListName <= StandbyPageList)) {

        ListHead->Total -= 1;  // Undo previous increment

        if (ListName == StandbyPageList) {
            Pfn1->u3.e1.PageLocation = StandbyPageList;
            MiRestoreTransitionPte (Pfn1);
        }

        ListHead = &MmBadPageListHead;
        ASSERT (ListHead->ListName == BadPageList);
        ListHead->Total += 1;  // One more page on the list.
        ListName = BadPageList;
    }

    //
    // Pages destined for the zeroed list go to the front to take advantage
    // of cache locality.  All other lists get the page at the rear for LRU
    // longest life.
    //

    if (ListName == ZeroedPageList) {

        last = ListHead->Flink;

        ListHead->Flink = PageFrameIndex;

        Pfn1->u1.Flink = last;
        Pfn1->u2.Blink = MM_EMPTY_LIST;

        if (last != MM_EMPTY_LIST) {
            Pfn2 = MI_PFN_ELEMENT (last);
            Pfn2->u2.Blink = PageFrameIndex;
        }
        else {
            ListHead->Blink = PageFrameIndex;
        }
    }
    else {

        last = ListHead->Blink;

        if (last != MM_EMPTY_LIST) {
            Pfn2 = MI_PFN_ELEMENT (last);
            Pfn2->u1.Flink = PageFrameIndex;
        }
        else {

            //
            // List is empty, add the page to the ListHead.
            //

            ListHead->Flink = PageFrameIndex;
        }

        ListHead->Blink = PageFrameIndex;
        Pfn1->u1.Flink = MM_EMPTY_LIST;
        Pfn1->u2.Blink = last;
    }

    Pfn1->u3.e1.PageLocation = ListName;

    //
    // If the page was placed on the standby or zeroed list,
    // update the count of usable pages in the system.  If the count
    // transitions from 0 to 1, the event associated with available
    // pages should become true.
    //

    if (ListName <= StandbyPageList) {

        MmAvailablePages += 1;

        //
        // A page has just become available, check to see if the
        // page wait events should be signaled.
        //

        if (MmAvailablePages <= MM_HIGH_LIMIT) {
            if (MmAvailablePages == MM_HIGH_LIMIT) {
                KeSetEvent (&MmAvailablePagesEventHigh, 0, FALSE);
                MiAvailablePagesEventHighSets += 1;
            }
            else if (MmAvailablePages == MM_LOW_LIMIT) {
                KeSetEvent (&MmAvailablePagesEvent, 0, FALSE);
                MiAvailablePagesEventLowSets += 1;
            }
        }

        //
        // Signal applications if the freed page crosses a threshold.
        //

        if (MmAvailablePages == MmLowMemoryThreshold) {
            KeClearEvent (MiLowMemoryEvent);
        }
        else if (MmAvailablePages == MmHighMemoryThreshold) {
            KeSetEvent (MiHighMemoryEvent, 0, FALSE);
        }

        if (ListName <= FreePageList) {

            PMMCOLOR_TABLES ColorHead;

            ASSERT (ListName == ZeroedPageList);
            ASSERT (Pfn1->u4.InPageError == 0);

#if defined(MI_MULTINODE)

            //
            // Increment the zero page count for this node.
            //

            if (KeNumberNodes > 1) {
                KeNodeBlock[Pfn1->u3.e1.PageColor]->FreeCount[ZeroedPageList]++;
            }
#endif

            //
            // We are adding a page to the zeroed page list.
            // Add the page to the front of the correct colored page list.
            //

            Color = MI_GET_COLOR_FROM_LIST_ENTRY (PageFrameIndex, Pfn1);

            ColorHead = &MmFreePagesByColor[ZeroedPageList][Color];

            last = ColorHead->Flink;

            Pfn1->OriginalPte.u.Long = last;
            Pfn1->u4.PteFrame = MM_EMPTY_LIST;

            ColorHead->Flink = PageFrameIndex;

            if (last != MM_EMPTY_LIST) {
                Pfn2 = MI_PFN_ELEMENT (last);
                Pfn2->u4.PteFrame = PageFrameIndex;
            }
            else {
                ColorHead->Blink = (PVOID) Pfn1;
            }

            ColorHead->Count += 1;

#if MI_BARRIER_SUPPORTED
            MI_BARRIER_STAMP_ZEROED_PAGE (&BarrierStamp);
            Pfn1->u4.PteFrame = BarrierStamp;
#endif
        }
        else {

            //
            // Transition page list so tally it appropriately.
            //

            MI_TALLY_TRANSITION_PAGE_ADDITION (Pfn1);
        }

        return;
    }

    //
    // Check to see if there are too many modified pages.
    //

    if (ListName == ModifiedPageList) {

        //
        // Transition page list so tally it appropriately.
        //

        MI_TALLY_TRANSITION_PAGE_ADDITION (Pfn1);

        if (Pfn1->OriginalPte.u.Soft.Prototype == 0) {
            ASSERT (Pfn1->OriginalPte.u.Soft.PageFileHigh == 0);
        }

        PsGetCurrentProcess()->ModifiedPageCount += 1;

        if (MmAvailablePages < MM_PLENTY_FREE_LIMIT) {

            //
            // If necessary, start the modified page writer.
            //

            if (MmModifiedPageListHead.Total >= MmModifiedPageMaximum) {
                KeSetEvent (&MmModifiedPageWriterEvent, 0, FALSE);
            }
            else if ((MmAvailablePages < MM_TIGHT_LIMIT) &&
                     (MmModifiedPageListHead.Total >= MmModifiedWriteClusterSize)) {

                KeSetEvent (&MmModifiedPageWriterEvent, 0, FALSE);
            }
        }
    }
    else if (ListName == ModifiedNoWritePageList) {
        ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
        MI_TALLY_TRANSITION_PAGE_ADDITION (Pfn1);
    }

    return;
}


VOID
FASTCALL
MiInsertZeroListAtBack (
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    This procedure inserts a page at the end of the zeroed list.
    This is only needed at system initialization to keep the higher
    physically numbered pages at the front of the zeroed list (normally
    we only put zeroed pages in the front of the list for better cache
    locality).

Arguments:

    PageFrameIndex - Supplies the physical page number to insert in the list.

Return Value:

    None.

Environment:

    Must be holding the PFN database lock.

--*/

{
    PFN_NUMBER last;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    ULONG Color;
    MMLISTS ListName;
    PMMCOLOR_TABLES ColorHead;
    PMMPFNLIST ListHead;
#if MI_BARRIER_SUPPORTED
    ULONG BarrierStamp;
#endif

    MM_PFN_LOCK_ASSERT();
    ASSERT ((PageFrameIndex != 0) &&
            (PageFrameIndex <= MmHighestPhysicalPage) &&
            (PageFrameIndex >= MmLowestPhysicalPage));

    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

    MI_SNAP_PFN (Pfn1, ZeroedPageList, 0x2);

    //
    // Check to ensure the reference count for the page is zero.
    //

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    ASSERT (Pfn1->u4.MustBeCached == 0);
    ASSERT (Pfn1->u3.e1.Rom == 0);

    if (Pfn1->u3.e1.RemovalRequested == 0) {
        ListHead = &MmZeroedPageListHead;
        ListName = ZeroedPageList;
        MmZeroedPageListHead.Total += 1;    // One more page on the list.
    }
    else {
        ListHead = &MmBadPageListHead;
        ListName = BadPageList;
        ListHead->Total += 1;  // One more page on the list.
    }

    last = ListHead->Blink;

    if (last != MM_EMPTY_LIST) {
        Pfn2 = MI_PFN_ELEMENT (last);
        Pfn2->u1.Flink = PageFrameIndex;
    }
    else {

        //
        // List is empty, add the page to the ListHead.
        //

        ListHead->Flink = PageFrameIndex;
    }

    ListHead->Blink = PageFrameIndex;
    Pfn1->u1.Flink = MM_EMPTY_LIST;
    Pfn1->u2.Blink = last;

    Pfn1->u3.e1.PageLocation = ListName;

    if (ListHead == &MmBadPageListHead) {
        return;
    }

    //
    // Update the count of usable pages in the system.  If the count
    // transitions from 0 to 1, the event associated with available
    // pages should become true.
    //

    MmAvailablePages += 1;

    //
    // A page has just become available, check to see if the
    // page wait events should be signaled.
    //

    if (MmAvailablePages <= MM_HIGH_LIMIT) {
        if (MmAvailablePages == MM_HIGH_LIMIT) {
            KeSetEvent (&MmAvailablePagesEventHigh, 0, FALSE);
            MiAvailablePagesEventHighSets += 1;
        }
        else if (MmAvailablePages == MM_LOW_LIMIT) {
            KeSetEvent (&MmAvailablePagesEvent, 0, FALSE);
            MiAvailablePagesEventLowSets += 1;
        }
    }

    //
    // Signal applications if the freed page crosses a threshold.
    //

    if (MmAvailablePages == MmLowMemoryThreshold) {
        KeClearEvent (MiLowMemoryEvent);
    }
    else if (MmAvailablePages == MmHighMemoryThreshold) {
        KeSetEvent (MiHighMemoryEvent, 0, FALSE);
    }

    ASSERT (ListName == ZeroedPageList);
    ASSERT (Pfn1->u4.InPageError == 0);

#if defined(MI_MULTINODE)

    //
    // Increment the zero page count for this node.
    //

    if (KeNumberNodes > 1) {
        KeNodeBlock[Pfn1->u3.e1.PageColor]->FreeCount[ZeroedPageList]++;
    }
#endif

    //
    // We are adding a page to the zeroed page list.
    // Add the page to the back of the correct colored page list.
    //

    Color = MI_GET_COLOR_FROM_LIST_ENTRY (PageFrameIndex, Pfn1);

    ColorHead = &MmFreePagesByColor[ZeroedPageList][Color];

    Pfn2 = ColorHead->Blink;

    if (Pfn2 == (PVOID) MM_EMPTY_LIST) {
        ColorHead->Flink = PageFrameIndex;
        Pfn1->u4.PteFrame = MM_EMPTY_LIST;
    }
    else {
        Pfn2->OriginalPte.u.Long = PageFrameIndex;
        Pfn1->u4.PteFrame = Pfn2 - MmPfnDatabase;
    }

    Pfn1->OriginalPte.u.Long = MM_EMPTY_LIST;

    ColorHead->Blink = (PVOID) Pfn1;

    ColorHead->Count += 1;

#if MI_BARRIER_SUPPORTED
    MI_BARRIER_STAMP_ZEROED_PAGE (&BarrierStamp);
    Pfn1->u4.PteFrame = BarrierStamp;
#endif

    return;
}


VOID
FASTCALL
MiInsertStandbyListAtFront (
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    This procedure inserts a page at the front of the standby list.

Arguments:

    PageFrameIndex - Supplies the physical page number to insert in the list.

Return Value:

    None.

Environment:

    PFN lock held.

--*/

{
    PFN_NUMBER first;
    PMMPFNLIST ListHead;
    PMMPFN Pfn1;
    PMMPFN Pfn2;

    MM_PFN_LOCK_ASSERT();
    ASSERT ((PageFrameIndex != 0) && (PageFrameIndex <= MmHighestPhysicalPage) &&
        (PageFrameIndex >= MmLowestPhysicalPage));

    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

    MI_SNAP_PFN(Pfn1, StandbyPageList, 0x3);

    MI_SNAP_DATA (Pfn1, Pfn1->PteAddress, 9);

    ASSERT (Pfn1->u4.MustBeCached == 0);

#if DBG
    if (MmDebug & MM_DBG_PAGE_REF_COUNT) {

        PMMPTE PointerPte;
        PEPROCESS Process;

        if ((Pfn1->u3.e1.PrototypePte == 1)  &&
                (MmIsAddressValid (Pfn1->PteAddress))) {
            PointerPte = Pfn1->PteAddress;
            Process = NULL;
        }
        else {

            //
            // The page containing the prototype PTE is not valid,
            // map the page into hyperspace and reference it that way.
            //

            Process = PsGetCurrentProcess ();
            PointerPte = MiMapPageInHyperSpaceAtDpc (Process, Pfn1->u4.PteFrame);
            PointerPte = (PMMPTE)((PCHAR)PointerPte +
                                    MiGetByteOffset(Pfn1->PteAddress));
        }

        ASSERT ((MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerPte) == PageFrameIndex) ||
                (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) == PageFrameIndex));
        ASSERT (PointerPte->u.Soft.Transition == 1);
        ASSERT (PointerPte->u.Soft.Prototype == 0);
        if (Process != NULL) {
            MiUnmapPageInHyperSpaceFromDpc (Process, PointerPte);
        }
    }

    if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
       (Pfn1->OriginalPte.u.Soft.Transition == 1)) {
        KeBugCheckEx (MEMORY_MANAGEMENT, 0x8889, 0,0,0);
    }
#endif

    //
    // Check to ensure the reference count for the page is zero.
    //

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    ASSERT (Pfn1->u3.e1.PrototypePte == 1);
    ASSERT (Pfn1->u3.e1.Rom != 1);

    MmTransitionSharedPages += 1;

    ListHead = &MmStandbyPageListByPriority [Pfn1->u4.Priority];

    ListHead->Total += 1;  // One more page on the list.

    first = ListHead->Flink;
    if (first == MM_EMPTY_LIST) {

        //
        // List is empty add the page to the ListHead.
        //

        ListHead->Blink = PageFrameIndex;
    }
    else {
        Pfn2 = MI_PFN_ELEMENT (first);
        Pfn2->u2.Blink = PageFrameIndex;
    }

    ListHead->Flink = PageFrameIndex;
    Pfn1->u2.Blink = MM_EMPTY_LIST;
    Pfn1->u1.Flink = first;
    Pfn1->u3.e1.PageLocation = StandbyPageList;

    //
    // If the page was placed on the free, standby or zeroed list,
    // update the count of usable pages in the system.  If the count
    // transitions from 0 to 1, the event associated with available
    // pages should become true.
    //

    MmAvailablePages += 1;

    //
    // A page has just become available, check to see if the
    // page wait events should be signaled.
    //

    if (MmAvailablePages <= MM_HIGH_LIMIT) {
        if (MmAvailablePages == MM_HIGH_LIMIT) {
            KeSetEvent (&MmAvailablePagesEventHigh, 0, FALSE);
            MiAvailablePagesEventHighSets += 1;
        }
        else if (MmAvailablePages == MM_LOW_LIMIT) {
            KeSetEvent (&MmAvailablePagesEvent, 0, FALSE);
            MiAvailablePagesEventLowSets += 1;
        }
    }

    //
    // Signal applications if the freed page crosses a threshold.
    //

    if (MmAvailablePages == MmLowMemoryThreshold) {
        KeClearEvent (MiLowMemoryEvent);
    }
    else if (MmAvailablePages == MmHighMemoryThreshold) {
        KeSetEvent (MiHighMemoryEvent, 0, FALSE);
    }

    return;
}

PFN_NUMBER
FASTCALL
MiRemovePageFromList (
    IN PMMPFNLIST ListHead
    )

/*++

Routine Description:

    This procedure removes a page from the head of the specified list (free,
    standby or zeroed).

    This routine clears the flags word in the PFN database, hence the
    PFN information for this page must be initialized.

Arguments:

    ListHead - Supplies the listhead of the list in which to remove the
               specified physical page.

Return Value:

    The physical page number removed from the specified list.

Environment:

    PFN lock held.

--*/

{
    PMMCOLOR_TABLES ColorHead;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    ULONG Color;
    MMLISTS ListName;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    MM_PFN_LOCK_ASSERT();

    //
    // For standby removals, the caller is responsible for specifying
    // which prioritized standby list to remove from.
    //

    ASSERT (ListHead != &MmStandbyPageListHead);

    if (ListHead->Total == 0) {
        KeBugCheckEx (PFN_LIST_CORRUPT, 1, (ULONG_PTR)ListHead, MmAvailablePages, 0);
    }

    ListName = ListHead->ListName;
    ASSERT (ListName != ModifiedPageList);

    //
    // Decrement the count of pages on the list and remove the first
    // page from the list.
    //

    ListHead->Total -= 1;
    PageFrameIndex = ListHead->Flink;
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    if (PERFINFO_IS_GROUP_ON(PERF_MEMORY)) {
        MiLogPfnInformation (Pfn1, PERFINFO_LOG_TYPE_REMOVEPAGEFROMLIST);
    }

    ListHead->Flink = Pfn1->u1.Flink;

    //
    // Zero the flink and blink in the PFN database element.
    //

    Pfn1->u1.Flink = 0;         // Assumes Flink width is >= WsIndex width
    Pfn1->u2.Blink = 0;

    //
    // If the last page was removed (the ListHead->Flink is now
    // MM_EMPTY_LIST) make the Listhead->Blink MM_EMPTY_LIST as well.
    //

    if (ListHead->Flink != MM_EMPTY_LIST) {

        //
        // Make the PFN element blink point to MM_EMPTY_LIST signifying this
        // is the first page in the list.
        //

        Pfn2 = MI_PFN_ELEMENT (ListHead->Flink);
        Pfn2->u2.Blink = MM_EMPTY_LIST;
    }
    else {
        ListHead->Blink = MM_EMPTY_LIST;
    }

    //
    // We now have one less page available.
    //

    ASSERT (ListName <= StandbyPageList);

    //
    // Signal if allocating this page caused a threshold cross.
    //

    if (MmAvailablePages == MmHighMemoryThreshold) {
        KeClearEvent (MiHighMemoryEvent);
    }
    else if (MmAvailablePages == MmLowMemoryThreshold) {
        KeSetEvent (MiLowMemoryEvent, 0, FALSE);
    }

    MmAvailablePages -= 1;

    if (ListName == StandbyPageList) {

        //
        // This page is currently in transition, restore the PTE to
        // its original contents so this page can be reused.
        //

        MI_TALLY_TRANSITION_PAGE_REMOVAL (Pfn1);
        MiRestoreTransitionPte (Pfn1);
    }

    if (MmAvailablePages < MmMinimumFreePages) {

        //
        // Obtain free pages.
        //

        MiObtainFreePages ();
    }

    ASSERT ((PageFrameIndex != 0) &&
            (PageFrameIndex <= MmHighestPhysicalPage) &&
            (PageFrameIndex >= MmLowestPhysicalPage));

    //
    // Zero the PFN flags longword.
    //

    Color = Pfn1->u3.e1.PageColor;
    CacheAttribute = Pfn1->u3.e1.CacheAttribute;
    ASSERT (Pfn1->u3.e1.RemovalRequested == 0);
    ASSERT (Pfn1->u3.e1.Rom == 0);
    Pfn1->u3.e2.ShortFlags = 0;
    Pfn1->u3.e1.PageColor = (USHORT) Color;
    Pfn1->u3.e1.CacheAttribute = CacheAttribute;

    if (ListName <= FreePageList) {

        //
        // Update the color lists.
        //

        Color = MI_GET_COLOR_FROM_LIST_ENTRY(PageFrameIndex, Pfn1);
        ColorHead = &MmFreePagesByColor[ListName][Color];
        ASSERT (ColorHead->Flink == PageFrameIndex);
        ColorHead->Flink = (PFN_NUMBER) Pfn1->OriginalPte.u.Long;
        if (ColorHead->Flink != MM_EMPTY_LIST) {
            MI_PFN_ELEMENT (ColorHead->Flink)->u4.PteFrame = MM_EMPTY_LIST;
        }
        else {
            ColorHead->Blink = (PVOID) MM_EMPTY_LIST;
        }
        ASSERT (ColorHead->Count >= 1);
        ColorHead->Count -= 1;
    }

    return PageFrameIndex;
}

VOID
FASTCALL
MiUnlinkPageFromList (
    IN PMMPFN Pfn
    )

/*++

Routine Description:

    This procedure removes a page from the middle of a list.  This is
    designed for the faulting of transition pages from the standby and
    modified list and making them active and valid again.

Arguments:

    Pfn - Supplies a pointer to the PFN database element for the physical
          page to remove from the list.

Return Value:

    none.

Environment:

    Must be holding the PFN database lock.

--*/

{
    PMMPFNLIST ListHead;
    PFN_NUMBER Previous;
    PFN_NUMBER Next;
    PMMPFN Pfn2;

    MM_PFN_LOCK_ASSERT();

    //
    // Page not on standby or modified list, check to see if the
    // page is currently being written by the modified page
    // writer, if so, just return this page.  The reference
    // count for the page will be incremented, so when the modified
    // page write completes, the page will not be put back on
    // the list, rather, it will remain active and valid.
    //

    if (Pfn->u3.e2.ReferenceCount > 0) {

        //
        // The page was not on any "transition lists", check to see
        // if this has I/O in progress.
        //

        if (Pfn->u2.ShareCount == 0) {
            return;
        }
        KdPrint(("MM:attempt to remove page from wrong page list\n"));
        KeBugCheckEx (PFN_LIST_CORRUPT,
                      2,
                      MI_PFN_ELEMENT_TO_INDEX (Pfn),
                      MmHighestPhysicalPage,
                      Pfn->u3.e2.ReferenceCount);
    }

    ListHead = MmPageLocationList[Pfn->u3.e1.PageLocation];

    //
    // Must not remove pages from free or zeroed without updating
    // the colored lists.
    //

    ASSERT (ListHead->ListName >= StandbyPageList);

    if (ListHead == &MmStandbyPageListHead) {

        ASSERT (Pfn->u3.e1.Rom == 0);

        ListHead = &MmStandbyPageListByPriority [Pfn->u4.Priority];

        //
        // Signal if allocating this page caused a threshold cross.
        //

        if (MmAvailablePages == MmHighMemoryThreshold) {
            KeClearEvent (MiHighMemoryEvent);
        }
        else if (MmAvailablePages == MmLowMemoryThreshold) {
            KeSetEvent (MiLowMemoryEvent, 0, FALSE);
        }

        //
        // We now have one less page available.
        //

        MmAvailablePages -= 1;

        MI_TALLY_TRANSITION_PAGE_REMOVAL (Pfn);

        if (MmAvailablePages < MmMinimumFreePages) {

            //
            // Obtain free pages.
            //

            MiObtainFreePages ();
        }
    }
    else if (ListHead == &MmModifiedPageListHead) {

        if (Pfn->OriginalPte.u.Soft.Prototype == 0) {

            //
            // This page is destined for the paging file (not
            // a mapped file).  Change the list head to the
            // appropriate colored list head.
            //
            // On MIPS R4000 modified pages destined for the paging file are
            // kept on separate lists which group pages of the same color
            // together.
            //

            ListHead->Total -= 1;
            MmTotalPagesForPagingFile -= 1;
#if MM_MAXIMUM_NUMBER_OF_COLORS > 1
            ListHead = &MmModifiedPageListByColor [Pfn->u3.e1.PageColor];
#else
            ListHead = &MmModifiedPageListByColor [0];
#endif
        }

        MI_TALLY_TRANSITION_PAGE_REMOVAL (Pfn);
    }
    else if (ListHead == &MmModifiedNoWritePageListHead) {
        MI_TALLY_TRANSITION_PAGE_REMOVAL (Pfn);
    }

    ASSERT (Pfn->u3.e1.WriteInProgress == 0);
    ASSERT (Pfn->u3.e1.ReadInProgress == 0);
    ASSERT (ListHead->Total != 0);

    Next = Pfn->u1.Flink;
    Previous = Pfn->u2.Blink;

    if (Next != MM_EMPTY_LIST) {
        Pfn2 = MI_PFN_ELEMENT(Next);
        Pfn2->u2.Blink = Previous;
    }
    else {
        ListHead->Blink = Previous;
    }

    if (Previous != MM_EMPTY_LIST) {
        Pfn2 = MI_PFN_ELEMENT(Previous);
        Pfn2->u1.Flink = Next;
    }
    else {
        ListHead->Flink = Next;
    }

    Pfn->u1.Flink = 0;         // Assumes Flink width is >= WsIndex width
    Pfn->u2.Blink = 0;

    ListHead->Total -= 1;

    return;
}

VOID
MiUnlinkFreeOrZeroedPage (
    IN PMMPFN Pfn
    )

/*++

Routine Description:

    This procedure removes a page from the middle of a list.  This is
    designed for the removing of free or zeroed pages from the middle of
    their lists.

Arguments:

    Pfn - Supplies a PFN element to remove from the list.

Return Value:

    None.

Environment:

    Must be holding the PFN database lock.

--*/

{
    PFN_NUMBER Page;
    PMMPFNLIST ListHead;
    PFN_NUMBER Previous;
    PFN_NUMBER Next;
    PMMPFN Pfn2;
    ULONG Color;
    PMMCOLOR_TABLES ColorHead;
    MMLISTS ListName;

    Page = MI_PFN_ELEMENT_TO_INDEX (Pfn);

    MM_PFN_LOCK_ASSERT();

    ListHead = MmPageLocationList[Pfn->u3.e1.PageLocation];
    ListName = ListHead->ListName;
    ASSERT (ListHead->Total != 0);
    ListHead->Total -= 1;

    ASSERT (ListName <= FreePageList);
    ASSERT (Pfn->u3.e1.WriteInProgress == 0);
    ASSERT (Pfn->u3.e1.ReadInProgress == 0);

    Next = Pfn->u1.Flink;
    Previous = Pfn->u2.Blink;

    if (Next != MM_EMPTY_LIST) {
        Pfn2 = MI_PFN_ELEMENT(Next);
        Pfn2->u2.Blink = Previous;
    }
    else {
        ListHead->Blink = Previous;
    }

    if (Previous == MM_EMPTY_LIST) {
        ListHead->Flink = Next;
    }
    else {
        Pfn2 = MI_PFN_ELEMENT(Previous);
        Pfn2->u1.Flink = Next;
    }

    //
    // Remove the page from its colored list.
    //

    Color = MI_GET_COLOR_FROM_LIST_ENTRY(Page, Pfn);

    ColorHead = &MmFreePagesByColor[ListName][Color];

    Next = ColorHead->Flink;

    if (Next == Page) {
        ColorHead->Flink = (PFN_NUMBER) Pfn->OriginalPte.u.Long;
        if (ColorHead->Flink != MM_EMPTY_LIST) {
            MI_PFN_ELEMENT(ColorHead->Flink)->u4.PteFrame = MM_EMPTY_LIST;
        }
        else {
            ColorHead->Blink = (PVOID) MM_EMPTY_LIST;
        }
    }
    else {

        ASSERT (Pfn->u4.PteFrame != MM_EMPTY_LIST);

        Pfn2 = MI_PFN_ELEMENT (Pfn->u4.PteFrame);
        Pfn2->OriginalPte.u.Long = Pfn->OriginalPte.u.Long;

        if (Pfn->OriginalPte.u.Long != MM_EMPTY_LIST) {
            Pfn2 = MI_PFN_ELEMENT (Pfn->OriginalPte.u.Long);
            Pfn2->u4.PteFrame = Pfn->u4.PteFrame;
        }
        else {
            ColorHead->Blink = Pfn2;
        }
    }

    Pfn->u1.Flink = 0;         // Assumes Flink width is >= WsIndex width
    Pfn->u2.Blink = 0;

    ASSERT (ColorHead->Count >= 1);
    ColorHead->Count -= 1;

    //
    // Decrement availability count.
    //

#if defined(MI_MULTINODE)

    if (KeNumberNodes > 1) {
        MI_NODE_FROM_COLOR(Color)->FreeCount[ListName]--;
    }

#endif

    //
    // Signal if allocating this page caused a threshold cross.
    //

    if (MmAvailablePages == MmHighMemoryThreshold) {
        KeClearEvent (MiHighMemoryEvent);
    }
    else if (MmAvailablePages == MmLowMemoryThreshold) {
        KeSetEvent (MiLowMemoryEvent, 0, FALSE);
    }

    MmAvailablePages -= 1;

    if (MmAvailablePages < MmMinimumFreePages) {

        //
        // Obtain free pages.
        //

        MiObtainFreePages ();
    }

    return;
}

LOGICAL
MiDereferenceLastChanceLdw (
    IN PMM_LDW_WORK_CONTEXT LdwContext
    )
{
    KIRQL OldIrql;

    if (LdwContext != &MiLastChanceLdwContext) {
        return FALSE;
    }

    LOCK_PFN (OldIrql);

    ASSERT (MiLastChanceLdwContext.FileObject != NULL);
    MiLastChanceLdwContext.FileObject = NULL;

    UNLOCK_PFN (OldIrql);

    return TRUE;
}

BOOLEAN Mi4dBreak = TRUE;
ULONG Mi4dFiles;
ULONG Mi4dPages;

VOID
MiNoPagesLastChance (
    IN ULONG Limit                
    )
{
    ULONG i;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PSUBSECTION Subsection;
    PFN_NUMBER ModifiedPage;
    PFN_NUMBER PagesOnList;
    PFN_NUMBER FreeSpace;
    PFN_NUMBER GrowthLeft;
    ULONG BitField;
    NTSTATUS Status;
    ULONG BugcheckCode;
    PCONTROL_AREA ControlArea;
    PFILE_OBJECT FilePointer;
    PMM_LDW_WORK_CONTEXT LdwContext;
    LOGICAL MarchOn;

    //
    // This bugcheck can occur for the following reasons:
    //
    // A driver has blocked, deadlocking the modified or mapped
    // page writers.  Examples of this include mutex deadlocks or
    // accesses to paged out memory in filesystem drivers, filter
    // drivers, etc.  This indicates a driver bug.
    //
    // The storage driver(s) are not processing requests.  Examples
    // of this are stranded queues, non-responding drives, etc.  This
    // indicates a driver bug.
    //
    // Not enough pool is available for the storage stack to write out
    // modified pages.  This indicates a driver bug.
    //
    // A high priority realtime thread has starved the balance set
    // manager from trimming pages and/or starved the modified writer
    // from writing them out.  This indicates a bug in the component
    // that created this thread.
    //

    BitField = 0;
    Status = STATUS_SUCCESS;
    BugcheckCode = NO_PAGES_AVAILABLE;
    PagesOnList = MmTotalPagesForPagingFile;

    if (!NT_SUCCESS (MiLastMappedWriteError)) {
        Status = MiLastMappedWriteError;
        BitField |= 0x1;
    }

    if (!NT_SUCCESS (MiLastModifiedWriteError)) {
        Status = MiLastModifiedWriteError;
        BitField |= 0x2;
    }

    GrowthLeft = 0;
    FreeSpace = 0;

    for (i = 0; i < MmNumberOfPagingFiles; i += 1) {
        GrowthLeft += (MmPagingFile[i]->MaximumSize - MmPagingFile[i]->Size);
        FreeSpace += MmPagingFile[i]->FreeSpace;
    }

    if (FreeSpace < (4 * 1024 * 1024) / PAGE_SIZE) {
        BitField |= 0x4;
    }

    if (GrowthLeft < (4 * 1024 * 1024) / PAGE_SIZE) {
        BitField |= 0x8;
    }

    if (MmSystemShutdown != 0) {

        //
        // Because applications are not terminated and drivers are
        // not unloaded, they can continue to access pages even after
        // the modified writer has terminated.  This can cause the
        // system to run out of pages since the pagefile(s) cannot be
        // used.
        //

        BugcheckCode = DISORDERLY_SHUTDOWN;
    }
    else if (MmModifiedNoWritePageListHead.Total >= (MmModifiedPageListHead.Total >> 2)) {
        BugcheckCode = DIRTY_NOWRITE_PAGES_CONGESTION;
        PagesOnList = MmModifiedNoWritePageListHead.Total;
    }
    else if (MmTotalPagesForPagingFile >= (MmModifiedPageListHead.Total >> 2)) {
        BugcheckCode = NO_PAGES_AVAILABLE;
    }
    else {
        BugcheckCode = DIRTY_MAPPED_PAGES_CONGESTION;
    }

    if ((KdPitchDebugger == FALSE) && (KdDebuggerNotPresent == FALSE)) {

        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "Without a debugger attached, the following bugcheck would have occurred.\n");
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "%4lx ", BugcheckCode);
    
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "%p %p %p %p\n",
                  MmModifiedPageListHead.Total,
                  PagesOnList,
                  BitField,
                  Status);
    
        //
        // Pop into the debugger (even on free builds) to determine
        // the cause of the starvation and march on.
        //
    
        if (Mi4dBreak == TRUE) {
            DbgBreakPoint ();
        }

        MarchOn = TRUE;
    }
    else {
        MarchOn = FALSE;
    }

    //
    // Toss modified mapped pages until the limit is reached allowing this
    // thread to make forward progress.  Each toss represents lost data so
    // this is very bad.  But if the filesystem is deadlocked then there is
    // nothing else that can be done other than to crash.  This tossing
    // provides the marginal benefit that at least a delayed write popup
    // event is queued (with the name of the file) and the system is kept
    // alive - the administrator will then *HAVE* to fix these files.  The
    // alternative would be to continue to bugcheck the system which not
    // only impacts availability, but also loses *all* the delayed mapped
    // writes without any event queueing to inform the administrator about
    // which files have been corrupted.
    //

    FilePointer = NULL;
    LdwContext = &MiLastChanceLdwContext;

    LOCK_PFN (OldIrql);

    //
    // If enough free pages have been created then return to give our
    // caller another chance.  Note another thread may have already done 
    // this for us.
    //

    if (MmAvailablePages >= Limit) {
        UNLOCK_PFN (OldIrql);
        return;
    }

    if (LdwContext->FileObject != NULL) {

        //
        // Some other thread is tossing pages right now, just return for
        // another delay.
        //

        UNLOCK_PFN (OldIrql);
        return;
    }

    ModifiedPage = MmModifiedPageListHead.Flink;

    while (ModifiedPage != MM_EMPTY_LIST) {

        //
        // There are modified mapped pages.
        //

        Pfn1 = MI_PFN_ELEMENT (ModifiedPage);

        ModifiedPage = Pfn1->u1.Flink;

        if (Pfn1->OriginalPte.u.Soft.Prototype == 1) {

            //
            // This page is destined for a file.  Chuck it and any other
            // pages destined for the same file.
            //

            Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);

            ControlArea = Subsection->ControlArea;
            ASSERT (ControlArea->FilePointer != NULL);

            if ((!ControlArea->u.Flags.Image) &&
                (!ControlArea->u.Flags.NoModifiedWriting) &&
                ((FilePointer == NULL) || (FilePointer == ControlArea->FilePointer))) {

                ASSERT (ControlArea->NumberOfPfnReferences >= 1);

                MiUnlinkPageFromList (Pfn1);

                //
                // Use the reference count macros so we don't prematurely
                // free the page because the physical page may have
                // references already.
                //

                MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1);

                //
                // Clear the dirty bit in the PFN entry so the page will go
                // to the standby instead of the modified list.  If the page
                // is not modified again before it is reused, the data will
                // be lost.
                //

                MI_SET_MODIFIED (Pfn1, 0, 0x1A);

                //
                // Put the page on the standby list if it is
                // the last reference to the page.
                //

                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);

                Mi4dPages += 1;

                if (FilePointer == NULL) {
                    FilePointer = ControlArea->FilePointer;
                    ASSERT (FilePointer != NULL);
                    ObReferenceObject (FilePointer);
                }

                //
                // Search for any other modified pages in this control area
                // because the entire file needs to be fixed anyway.
                //
            }
        }
    }

    UNLOCK_PFN (OldIrql);

    //
    // If we were able to free up some pages then return to our caller for
    // another attempt/wait if necessary.
    //
    // Otherwise if no debugger is attached, then there's no point in
    // continuing so bugcheck.
    //

    if (FilePointer != NULL) {

        ASSERT (LdwContext->FileObject == NULL);
        LdwContext->FileObject = FilePointer;
        ExInitializeWorkItem (&LdwContext->WorkItem,
                              MiLdwPopupWorker,
                              (PVOID)LdwContext);

        ExQueueWorkItem (&LdwContext->WorkItem, DelayedWorkQueue);
        Mi4dFiles += 1;

        return;
    }

    if (MarchOn == FALSE) {
        KeBugCheckEx (BugcheckCode,
                      MmModifiedPageListHead.Total,
                      PagesOnList,
                      BitField,
                      Status);
    }

    return;
}


ULONG
FASTCALL
MiEnsureAvailablePageOrWait (
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This procedure ensures that a physical page is available on
    the zeroed, free or standby list such that the next call to remove a
    page absolutely will not block.  This is necessary as blocking would
    require a wait which could cause a deadlock condition.

    If a page is available the function returns immediately with a value
    of FALSE indicating no wait operation was performed.  If no physical
    page is available, the thread enters a wait state and the function
    returns the value TRUE when the wait operation completes.

Arguments:

    Process - Supplies a pointer to the current process if, and only if,
              the working set mutex is currently held and should
              be released if a wait operation is issued.  Supplies
              the value NULL otherwise.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at.

Return Value:

    FALSE - if a page was immediately available.

    TRUE - if a wait operation occurred before a page became available.

Environment:

    Must be holding the PFN database lock.

--*/

{
    PVOID Event;
    NTSTATUS Status;
    ULONG Limit;
    LOGICAL WsHeldSafe;
    LOGICAL WsHeldShared;
    PETHREAD Thread;
    PMMSUPPORT Ws;
    PULONG EventSetPointer;
    ULONG EventSetCounter;
    
    MM_PFN_LOCK_ASSERT();

    if (MmAvailablePages >= MM_HIGH_LIMIT) {

        //
        // Pages are available.
        //

        return FALSE;
    }

    //
    // If this thread has explicitly disabled APCs (FsRtlEnterFileSystem
    // does this), then it may be holding resources or mutexes that may in
    // turn be blocking memory making threads from making progress.  We'd
    // like to detect this but cannot (without changing the
    // FsRtlEnterFileSystem macro) since other components (win32k for
    // example) also enter critical regions and then access paged pool.
    //
    // At least give system threads a free pass as they may be worker
    // threads processing potentially blocking items drivers have queued.
    //

    Thread = PsGetCurrentThread ();

    if (Thread->MemoryMaker == 1) {
        if (MmAvailablePages >= MM_LOW_LIMIT) {
            return FALSE;
        }
        Limit = MM_LOW_LIMIT;
        Event = (PVOID) &MmAvailablePagesEvent;
        EventSetPointer = &MiAvailablePagesEventLowSets;
    }
    else {
        Limit = MM_HIGH_LIMIT;
        Event = (PVOID) &MmAvailablePagesEventHigh;
        EventSetPointer = &MiAvailablePagesEventHighSets;
    }

    EventSetCounter = *EventSetPointer;
    ASSERT (MmAvailablePages < Limit);

    //
    // Initializing WsHeldSafe is not needed for
    // correctness but without it the compiler cannot compile this code
    // W4 to check for use of uninitialized variables.
    //

    Ws = NULL;
    WsHeldSafe = FALSE;
    WsHeldShared = FALSE;

    do {

        KeClearEvent ((PKEVENT)Event);

        UNLOCK_PFN (OldIrql);

        if (Process == HYDRA_PROCESS) {
            Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
            UNLOCK_WORKING_SET (Thread, Ws);
        }
        else if (Process != NULL) {

            //
            // The working set lock may have been acquired safely or unsafely
            // by our caller.  Handle both cases here and below.
            //

            UNLOCK_WS_REGARDLESS (Thread, Process, WsHeldSafe, WsHeldShared);
        }
        else {
            if ((Thread->OwnsSystemWorkingSetExclusive) ||
                (Thread->OwnsSystemWorkingSetShared)) {

                Ws = &MmSystemCacheWs;
                UNLOCK_WORKING_SET (Thread, Ws);
            }
        }

        //
        // Wait 70 seconds for pages to become available.  Note this number
        // was picked because it is larger than both SCSI and redirector
        // timeout values.
        //
        // Unfortunately we are using a notification event and may be waiting
        // in some cases with APCs enabled.  Thus inside KeWait, the APC is
        // delivered and then the event gets signaled.  The APC is handled,
        // but the available pages are taken and the event above cleared
        // by other thread(s).  Then this thread looks at the event and
        // sees it isn't signaled (and thus doesn't realize it ever happened)
        // and so goes back into a wait state.  In a pathological case (we
        // have seen this happen), this scenario repeats until the thread's
        // timeout expires and gets returned as such, even though the event
        // has been signaled many times.
        //

        Status = KeWaitForSingleObject (Event,
                                        WrFreePage,
                                        KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER) &MmSeventySeconds);

        if (Status == STATUS_TIMEOUT) {

            if (EventSetCounter == *EventSetPointer) {
                MiNoPagesLastChance (Limit);
            }
        }

        EventSetCounter = *EventSetPointer;

        if (Ws != NULL) {
            LOCK_WORKING_SET (Thread, Ws);
        }
        else if (Process != NULL) {

            //
            // The working set lock may have been acquired safely or unsafely
            // by our caller.  Reacquire it in the same manner our caller did.
            //

            LOCK_WS_REGARDLESS (Thread, Process, WsHeldSafe, WsHeldShared);
        }

        LOCK_PFN (OldIrql);

    } while (MmAvailablePages < Limit);

    return TRUE;
}


PFN_NUMBER
FASTCALL
MiRemoveZeroPage (
    IN ULONG Color
    )

/*++

Routine Description:

    This procedure removes a zero page from either the zeroed, free
    or standby lists (in that order).  If no pages exist on the zeroed
    or free list a transition page is removed from the standby list
    and the PTE (may be a prototype PTE) which refers to this page is
    changed from transition back to its original contents.

    If the page is not obtained from the zeroed list, it is zeroed.

Arguments:

    Color - Supplies the page color for which this page is destined.
            This is used for checking virtual address alignments to
            determine if the D cache needs flushing before the page
            can be reused.

            The above was true when we were concerned about caches
            which are virtually indexed (ie MIPS).  Today we
            are more concerned that we get a good usage spread across
            the L2 caches of most machines.  These caches are physically
            indexed.  By gathering pages that would have the same
            index to the same color, then maximizing the color spread,
            we maximize the effective use of the caches.

            This has been extended for NUMA machines.  The high part
            of the color gives the node color (basically node number).
            If we cannot allocate a page of the requested color, we
            try to allocate a page on the same node before taking a
            page from a different node.

Return Value:

    The physical page number removed from the specified list.

Environment:

    Must be holding the PFN database lock.

--*/

{
    PFN_NUMBER Page;
    PMMPFN Pfn1;
    PMMCOLOR_TABLES FreePagesByColor;
    PMMPFNLIST ListHead;
#if MI_BARRIER_SUPPORTED
    ULONG BarrierStamp;
#endif
#if defined(MI_MULTINODE)
    PKNODE Node;
    ULONG NodeColor;
    ULONG OriginalColor;
#endif

    MM_PFN_LOCK_ASSERT();
    ASSERT(MmAvailablePages != 0);

    FreePagesByColor = MmFreePagesByColor[ZeroedPageList];

#if defined(MI_MULTINODE)

    //
    // Initializing Node is not needed for correctness, but without it
    // the compiler cannot compile this code W4 to check for use of
    // uninitialized variables.
    //

    Node = NULL;

    NodeColor = Color & ~MmSecondaryColorMask;
    OriginalColor = Color;

    if (KeNumberNodes > 1) {
        Node = MI_NODE_FROM_COLOR(Color);
    }

    do {

#endif

        //
        // Attempt to remove a page from the zeroed page list. If a page
        // is available, then remove it and return its page frame index.
        // Otherwise, attempt to remove a page from the free page list or
        // the standby list.
        //
        // N.B. It is not necessary to change page colors even if the old
        //      color is not equal to the new color. The zero page thread
        //      ensures that all zeroed pages are removed from all caches.
        //

        ASSERT (Color < MmSecondaryColors);
        Page = FreePagesByColor[Color].Flink;

        if (Page != MM_EMPTY_LIST) {

            //
            // Remove the first entry on the zeroed by color list.
            //

#if DBG
            Pfn1 = MI_PFN_ELEMENT(Page);
#endif
            ASSERT ((Pfn1->u3.e1.PageLocation == ZeroedPageList) ||
                    ((Pfn1->u3.e1.PageLocation == FreePageList) &&
                     (FreePagesByColor == MmFreePagesByColor[FreePageList])));

            ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

            Page = MiRemovePageByColor (Page, Color);

            ASSERT (Pfn1 == MI_PFN_ELEMENT(Page));

#if defined(MI_MULTINODE)

            if (FreePagesByColor != MmFreePagesByColor[ZeroedPageList]) {
                goto ZeroPage;
            }

#endif

            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            ASSERT (Pfn1->u2.ShareCount == 0);
            ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

            return Page;

        }

#if defined(MI_MULTINODE)

        //
        // If this is a multinode machine and there are zero
        // pages on this node, select another color on this 
        // node in preference to random selection.
        //

        if (KeNumberNodes > 1) {
            if (Node->FreeCount[ZeroedPageList] != 0) {
                Color = ((Color + 1) & MmSecondaryColorMask) | NodeColor;
                ASSERT(Color != OriginalColor);
                continue;
            }

            //
            // No previously zeroed page with the specified secondary
            // color exists.  Since this is a multinode machine, zero
            // an available local free page now instead of allocating a
            // zeroed page from another node below.
            //

            if (Node->FreeCount[FreePageList] != 0) {
                if (FreePagesByColor != MmFreePagesByColor[FreePageList]) {
                    FreePagesByColor = MmFreePagesByColor[FreePageList];
                    Color = OriginalColor;
                }
                else {
                    Color = ((Color + 1) & MmSecondaryColorMask) | NodeColor;
                    ASSERT(Color != OriginalColor);
                }
                continue;
            }
        }

        break;
    } while (TRUE);

#endif

    //
    // No previously zeroed page with the specified secondary color exists.
    // Try a zeroed page of any color.
    //

    Page = MmZeroedPageListHead.Flink;
    if (Page != MM_EMPTY_LIST) {
#if DBG
        Pfn1 = MI_PFN_ELEMENT(Page);
#endif
        ASSERT (Pfn1->u3.e1.PageLocation == ZeroedPageList);
        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

        Color = MI_GET_COLOR_FROM_LIST_ENTRY(Page, MI_PFN_ELEMENT(Page));

        Page = MiRemovePageByColor (Page, Color);

        ASSERT (Pfn1 == MI_PFN_ELEMENT(Page));
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

        return Page;
    }

    //
    // No zeroed page of the primary color exists, try a free page of the
    // secondary color.  Note in the multinode case this has already been done
    // above.
    //

#if defined(MI_MULTINODE)
    if (KeNumberNodes <= 1) {
#endif
        FreePagesByColor = MmFreePagesByColor[FreePageList];
    
        Page = FreePagesByColor[Color].Flink;
        if (Page != MM_EMPTY_LIST) {
    
            //
            // Remove the first entry on the free list by color.
            //
    
#if DBG
            Pfn1 = MI_PFN_ELEMENT(Page);
#endif
            ASSERT (Pfn1->u3.e1.PageLocation == FreePageList);
            ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
    
            Page = MiRemovePageByColor (Page, Color);

            ASSERT (Pfn1 == MI_PFN_ELEMENT(Page));
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            ASSERT (Pfn1->u2.ShareCount == 0);
            ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

            goto ZeroPage;
        }
#if defined(MI_MULTINODE)
    }
#endif

    Page = MmFreePageListHead.Flink;
    if (Page != MM_EMPTY_LIST) {

        Color = MI_GET_COLOR_FROM_LIST_ENTRY(Page, MI_PFN_ELEMENT(Page));
#if DBG
        Pfn1 = MI_PFN_ELEMENT(Page);
#endif
        Page = MiRemovePageByColor (Page, Color);

        ASSERT (Pfn1 == MI_PFN_ELEMENT(Page));
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
        goto ZeroPage;
    }

    ASSERT (MmZeroedPageListHead.Total == 0);
    ASSERT (MmFreePageListHead.Total == 0);

    //
    // Remove a page from the standby list and restore the original
    // contents of the PTE to free the last reference to the physical page.
    //

    for (ListHead = &MmStandbyPageListByPriority[0];
         ListHead < &MmStandbyPageListByPriority[MI_PFN_PRIORITIES];
         ListHead += 1) {

        if (ListHead->Total != 0) {
            Page = MiRemovePageFromList (ListHead);
            break;
        }
    }

    ASSERT (ListHead < &MmStandbyPageListByPriority[MI_PFN_PRIORITIES]);
    ASSERT ((MI_PFN_ELEMENT(Page))->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
    MmStandbyRePurposed += 1;

    //
    // Zero the page removed from the free or standby list.
    //

ZeroPage:

    Pfn1 = MI_PFN_ELEMENT(Page);

    MiZeroPhysicalPage (Page);

#if MI_BARRIER_SUPPORTED

    //
    // Note the stamping must occur after the page is zeroed.
    //

    MI_BARRIER_STAMP_ZEROED_PAGE (&BarrierStamp);
    Pfn1->u4.PteFrame = BarrierStamp;

#endif

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    ASSERT (Pfn1->u2.ShareCount == 0);
    ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

    return Page;
}

PFN_NUMBER
FASTCALL
MiRemoveAnyPage (
    IN ULONG Color
    )

/*++

Routine Description:

    This procedure removes a page from either the free, zeroed,
    or standby lists (in that order).  If no pages exist on the zeroed
    or free list a transition page is removed from the standby list
    and the PTE (may be a prototype PTE) which refers to this page is
    changed from transition back to its original contents.

    Note pages MUST exist to satisfy this request.  The caller ensures this
    by first calling MiEnsureAvailablePageOrWait.

Arguments:

    Color - Supplies the page color for which this page is destined.
            This is used for checking virtual address alignments to
            determine if the D cache needs flushing before the page
            can be reused.

            The above was true when we were concerned about caches
            which are virtually indexed.   (eg MIPS).   Today we
            are more concerned that we get a good usage spread across
            the L2 caches of most machines.  These caches are physically
            indexed.   By gathering pages that would have the same
            index to the same color, then maximizing the color spread,
            we maximize the effective use of the caches.

            This has been extended for NUMA machines.   The high part
            of the color gives the node color (basically node number).
            If we cannot allocate a page of the requested color, we
            try to allocate a page on the same node before taking a
            page from a different node.

Return Value:

    The physical page number removed from the specified list.

Environment:

    Must be holding the PFN database lock.

--*/

{
    PFN_NUMBER Page;
    PMMPFNLIST ListHead;
#if DBG
    PMMPFN Pfn1;
#endif
#if defined(MI_MULTINODE)
    PKNODE Node;
    ULONG NodeColor;
    ULONG OriginalColor;
    PFN_NUMBER LocalNodePagesAvailable;
#endif

    MM_PFN_LOCK_ASSERT();
    ASSERT(MmAvailablePages != 0);

#if defined(MI_MULTINODE)

    //
    // Bias color to memory node.  The assumption is that if memory
    // of the correct color is not available on this node, it is
    // better to choose memory of a different color if you can stay
    // on this node.
    //

    LocalNodePagesAvailable = 0;
    NodeColor = Color & ~MmSecondaryColorMask;
    OriginalColor = Color;

    if (KeNumberNodes > 1) {
        Node = MI_NODE_FROM_COLOR(Color);
        LocalNodePagesAvailable = (Node->FreeCount[ZeroedPageList] | Node->FreeCount[FreePageList]);
    }

    do {

#endif

        //
        // Check the free page list, and if a page is available
        // remove it and return its value.
        //

        ASSERT (Color < MmSecondaryColors);
        if (MmFreePagesByColor[FreePageList][Color].Flink != MM_EMPTY_LIST) {

            //
            // Remove the first entry on the free by color list.
            //

            Page = MmFreePagesByColor[FreePageList][Color].Flink;
#if DBG
            Pfn1 = MI_PFN_ELEMENT(Page);
#endif
            ASSERT (Pfn1->u3.e1.PageLocation == FreePageList);
            ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

            Page = MiRemovePageByColor (Page, Color);

            ASSERT (Pfn1 == MI_PFN_ELEMENT(Page));
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            ASSERT (Pfn1->u2.ShareCount == 0);
            ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                
            return Page;
        }

        //
        // Try the zero page list by primary color.
        //

        if (MmFreePagesByColor[ZeroedPageList][Color].Flink
                                                        != MM_EMPTY_LIST) {

            //
            // Remove the first entry on the zeroed by color list.
            //

            Page = MmFreePagesByColor[ZeroedPageList][Color].Flink;
#if DBG
            Pfn1 = MI_PFN_ELEMENT(Page);
#endif
            ASSERT (Pfn1->u3.e1.PageLocation == ZeroedPageList);
            ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

            Page = MiRemovePageByColor (Page, Color);

            ASSERT (Pfn1 == MI_PFN_ELEMENT(Page));
            ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                
            return Page;
        }

        //
        // If this is a multinode machine and there are free
        // pages on this node, select another color on this 
        // node in preference to random selection.
        //

#if defined(MI_MULTINODE)

        if (LocalNodePagesAvailable != 0) {
            Color = ((Color + 1) & MmSecondaryColorMask) | NodeColor;
            ASSERT(Color != OriginalColor);
            continue;
        }

        break;
    } while (TRUE);

#endif

    //
    // Check the free page list, and if a page is available
    // remove it and return its value.
    //

    if (MmFreePageListHead.Flink != MM_EMPTY_LIST) {
        Page = MmFreePageListHead.Flink;
        Color = MI_GET_COLOR_FROM_LIST_ENTRY(Page, MI_PFN_ELEMENT(Page));

#if DBG
        Pfn1 = MI_PFN_ELEMENT(Page);
#endif
        ASSERT (Pfn1->u3.e1.PageLocation == FreePageList);
        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

        Page = MiRemovePageByColor (Page, Color);

        ASSERT (Pfn1 == MI_PFN_ELEMENT(Page));
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

        return Page;
    }
    ASSERT (MmFreePageListHead.Total == 0);

    //
    // Check the zeroed page list, and if a page is available
    // remove it and return its value.
    //

    if (MmZeroedPageListHead.Flink != MM_EMPTY_LIST) {
        Page = MmZeroedPageListHead.Flink;
        Color = MI_GET_COLOR_FROM_LIST_ENTRY(Page, MI_PFN_ELEMENT(Page));

#if DBG
        Pfn1 = MI_PFN_ELEMENT(Page);
#endif
        Page = MiRemovePageByColor (Page, Color);

        ASSERT (Pfn1 == MI_PFN_ELEMENT(Page));
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (Pfn1->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

        return Page;
    }
    ASSERT (MmZeroedPageListHead.Total == 0);

    //
    // No pages exist on the free or zeroed list, use the standby list.
    //

    SATISFY_OVERZEALOUS_COMPILER (Page = MM_EMPTY_LIST);

    for (ListHead = &MmStandbyPageListByPriority[0];
         ListHead < &MmStandbyPageListByPriority[MI_PFN_PRIORITIES];
         ListHead += 1) {

        if (ListHead->Total != 0) {
            Page = MiRemovePageFromList (ListHead);
            break;
        }
    }

    ASSERT (ListHead < &MmStandbyPageListByPriority[MI_PFN_PRIORITIES]);
    ASSERT ((MI_PFN_ELEMENT(Page))->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
    MmStandbyRePurposed += 1;

    MI_CHECK_PAGE_ALIGNMENT(Page, Color & MM_COLOR_MASK);
#if DBG
    Pfn1 = MI_PFN_ELEMENT (Page);
#endif
    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    ASSERT (Pfn1->u2.ShareCount == 0);

    return Page;
}


PFN_NUMBER
FASTCALL
MiRemovePageByColor (
    IN PFN_NUMBER Page,
    IN ULONG Color
    )

/*++

Routine Description:

    This procedure removes a page from the front of the free or
    zeroed page list.

Arguments:

    Page - Supplies the physical page number to unlink from the list.

    Color - Supplies the page color for which this page is destined.
            This is used for checking virtual address alignments to
            determine if the D cache needs flushing before the page
            can be reused.

Return Value:

    The page frame number that was unlinked (always equal to the one
    passed in, but returned so the caller's fastcall sequences save
    extra register pushes and pops.

Environment:

    Must be holding the PFN database lock.

--*/

{
    PMMPFNLIST ListHead;
    PMMPFNLIST PrimaryListHead;
    PFN_NUMBER Previous;
    PFN_NUMBER Next;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    ULONG NodeColor;
    MMLISTS ListName;
    PMMCOLOR_TABLES ColorHead;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

    MM_PFN_LOCK_ASSERT();

    Pfn1 = MI_PFN_ELEMENT (Page);
    NodeColor = Pfn1->u3.e1.PageColor;
    CacheAttribute = Pfn1->u3.e1.CacheAttribute;

#if defined(MI_MULTINODE)

    ASSERT (NodeColor == (Color >> MmSecondaryColorNodeShift));

#endif

    if (PERFINFO_IS_GROUP_ON(PERF_MEMORY)) {
        MiLogPfnInformation (Pfn1, PERFINFO_LOG_TYPE_REMOVEPAGEBYCOLOR);
    }

    ListHead = MmPageLocationList[Pfn1->u3.e1.PageLocation];
    ListName = ListHead->ListName;

    ListHead->Total -= 1;

    PrimaryListHead = ListHead;

    Next = Pfn1->u1.Flink;
    Previous = Pfn1->u2.Blink;

    if (Next == MM_EMPTY_LIST) {
        PrimaryListHead->Blink = Previous;
    }
    else {
        Pfn2 = MI_PFN_ELEMENT(Next);
        Pfn2->u2.Blink = Previous;
    }

    if (Previous == MM_EMPTY_LIST) {
        PrimaryListHead->Flink = Next;
    }
    else {
        Pfn2 = MI_PFN_ELEMENT(Previous);
        Pfn2->u1.Flink = Next;
    }

    ASSERT (Pfn1->u3.e1.RemovalRequested == 0);

    //
    // Zero the flags longword, but keep the color and attribute information.
    //

    ASSERT (Pfn1->u3.e1.Rom == 0);
    Pfn1->u3.e2.ShortFlags = 0;
    Pfn1->u3.e1.PageColor = (USHORT) NodeColor;
    Pfn1->u3.e1.CacheAttribute = CacheAttribute;

    Pfn1->u1.Flink = 0;         // Assumes Flink width is >= WsIndex width
    Pfn1->u2.Blink = 0;

    //
    // Update the color lists.
    //

    ASSERT (Color < MmSecondaryColors);

    ColorHead = &MmFreePagesByColor[ListName][Color];
    ASSERT (ColorHead->Count >= 1);
    ColorHead->Flink = (PFN_NUMBER) Pfn1->OriginalPte.u.Long;
    if (ColorHead->Flink != MM_EMPTY_LIST) {
        MI_PFN_ELEMENT (ColorHead->Flink)->u4.PteFrame = MM_EMPTY_LIST;
    }
    else {
        ColorHead->Blink = (PVOID) MM_EMPTY_LIST;
    }

    ColorHead->Count -= 1;

    //
    // Note that we now have one less page available.
    //

#if defined(MI_MULTINODE)
    if (KeNumberNodes > 1) {
        KeNodeBlock[NodeColor]->FreeCount[ListName]--;
    }
#endif

    //
    // Signal if allocating this page caused a threshold cross.
    //

    if (MmAvailablePages == MmHighMemoryThreshold) {
        KeClearEvent (MiHighMemoryEvent);
    }
    else if (MmAvailablePages == MmLowMemoryThreshold) {
        KeSetEvent (MiLowMemoryEvent, 0, FALSE);
    }

    MmAvailablePages -= 1;

    if (MmAvailablePages < MmMinimumFreePages) {

        //
        // Obtain free pages.
        //

        MiObtainFreePages ();
    }

    return Page;
}


VOID
FASTCALL
MiInsertFrontModifiedNoWrite (
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    This procedure inserts a page at the FRONT of the modified no write list.

Arguments:

    PageFrameIndex - Supplies the physical page number to insert in the list.

Return Value:

    None.

Environment:

    Must be holding the PFN database lock.

--*/

{
    PFN_NUMBER first;
    PMMPFN Pfn1;
    PMMPFN Pfn2;

    MM_PFN_LOCK_ASSERT();
    ASSERT ((PageFrameIndex != 0) && (PageFrameIndex <= MmHighestPhysicalPage) &&
        (PageFrameIndex >= MmLowestPhysicalPage));

    //
    // Check to ensure the reference count for the page is zero.
    //

    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

    MI_SNAP_PFN(Pfn1, StandbyPageList, 0x4);

    MI_SNAP_DATA (Pfn1, Pfn1->PteAddress, 0xA);

    ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    ASSERT (Pfn1->u4.MustBeCached == 0);

    MmModifiedNoWritePageListHead.Total += 1;  // One more page on the list.

    MI_TALLY_TRANSITION_PAGE_ADDITION (Pfn1);

    first = MmModifiedNoWritePageListHead.Flink;
    if (first == MM_EMPTY_LIST) {

        //
        // List is empty add the page to the ListHead.
        //

        MmModifiedNoWritePageListHead.Blink = PageFrameIndex;
    }
    else {
        Pfn2 = MI_PFN_ELEMENT (first);
        Pfn2->u2.Blink = PageFrameIndex;
    }

    MmModifiedNoWritePageListHead.Flink = PageFrameIndex;
    Pfn1->u1.Flink = first;
    Pfn1->u2.Blink = MM_EMPTY_LIST;
    Pfn1->u3.e1.PageLocation = ModifiedNoWritePageList;
    return;
}

PFN_NUMBER
MiAllocatePfn (
    IN PMMPTE PointerPte,
    IN ULONG Protection
    )

/*++

Routine Description:

    This procedure allocates and initializes a page of memory.

Arguments:

    PointerPte - Supplies the PTE to initialize.

Return Value:

    The page frame index allocated.

Environment:

    Kernel mode.

--*/
{
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;
    MMPTE DemandZeroPte;

    DemandZeroPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;

    LOCK_PFN (OldIrql);

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    PageFrameIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

    MI_WRITE_INVALID_PTE (PointerPte, DemandZeroPte);

    PointerPte->u.Soft.Protection |= Protection;

    MiInitializePfn (PageFrameIndex, PointerPte, 1);

    UNLOCK_PFN (OldIrql);

    return PageFrameIndex;
}

VOID
FASTCALL
MiLogPfnInformation (
    IN PMMPFN Pfn1,
    IN USHORT Reason
    )
{
    MMPFN_IDENTITY PfnIdentity;

    RtlZeroMemory (&PfnIdentity, sizeof(PfnIdentity));

    MiIdentifyPfn (Pfn1, &PfnIdentity);

    PerfInfoLogBytes (Reason, 
                      &PfnIdentity, 
                      sizeof(PfnIdentity));

    return;
}

VOID
MiPurgeTransitionList (
    VOID
    )
{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;
    PMMPFNLIST ListHead;

    //
    // Run the transition list and free all the entries so transition
    // faults are not satisfied for any of the non modified pages that were
    // freed.
    //

    for (ListHead = &MmStandbyPageListByPriority[0];
         ListHead < &MmStandbyPageListByPriority[MI_PFN_PRIORITIES];
         ListHead += 1) {

        if (ListHead->Total == 0) {
            continue;
        }

        LOCK_PFN (OldIrql);
    
        while (ListHead->Total != 0) {
    
            PageFrameIndex = MiRemovePageFromList (ListHead);
    
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
            ASSERT (Pfn1->u2.ShareCount == 0);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    
            InterlockedIncrementPfn ((PSHORT)&Pfn1->u3.e2.ReferenceCount);
            Pfn1->OriginalPte.u.Long = 0;
    
            MI_SET_PFN_DELETED (Pfn1);
    
            MiDecrementReferenceCount (Pfn1, PageFrameIndex);
        }
    
        UNLOCK_PFN (OldIrql);
    }

    return;
}

