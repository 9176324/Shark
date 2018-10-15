/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    zeropage.c

Abstract:

    This module contains the zero page thread for memory management.

--*/

#include "mi.h"

#define MM_ZERO_PAGE_OBJECT     0
#define PO_SYS_IDLE_OBJECT      1
#define NUMBER_WAIT_OBJECTS     2

#define MACHINE_ZERO_PAGE(ZeroBase,NumberOfBytes) KeZeroPagesFromIdleThread(ZeroBase,NumberOfBytes)

LOGICAL MiZeroingDisabled = FALSE;

#if DBG
ULONG MiInitialZeroNoPtes = 0;
#endif

#if !defined(NT_UP)

LONG MiNextZeroProcessor = (LONG)-1;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiStartZeroPageWorkers)
#endif

#endif

VOID
MmZeroPageThread (
    VOID
    )

/*++

Routine Description:

    Implements the NT zeroing page thread.  This thread runs
    at priority zero and removes a page from the free list,
    zeroes it, and places it on the zeroed page list.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode.

--*/

{
    KIRQL OldIrql;
    PFN_NUMBER PageFrame1;
    LOGICAL ZeroedAlready;
    PFN_NUMBER PageFrame;
    PMMPFN Pfn1;
    PVOID ZeroBase;
    PVOID WaitObjects[NUMBER_WAIT_OBJECTS];
    NTSTATUS Status;
    PVOID StartVa;
    PVOID EndVa;
    PFN_COUNT PagesToZero;
    PFN_COUNT MaximumPagesToZero;
    ULONG Color;
    ULONG StartColor;
    PMMPFN PfnAllocation;
    ULONG SecondaryColorMask;
    PMMCOLOR_TABLES FreePagesByColor;

#if defined(MI_MULTINODE)

    ULONG i;
    ULONG n;
    ULONG LastNodeZeroing;
    KAFFINITY ProcessorMask;

    n = 0;
    LastNodeZeroing = 0;
#endif

    //
    // Make local copies of globals so they don't have to be wastefully
    // refetched while holding the PFN lock.
    //

    FreePagesByColor = MmFreePagesByColor[FreePageList];
    SecondaryColorMask = MmSecondaryColorMask;

    //
    // Before this becomes the zero page thread, free the kernel
    // initialization code.
    //

    MiFindInitializationCode (&StartVa, &EndVa);

    if (StartVa != NULL) {
        MiFreeInitializationCode (StartVa, EndVa);
    }

    MaximumPagesToZero = 1;

#if !defined(NT_UP)

    //
    // Zero groups of pages at once to reduce PFN lock contention.
    // Charge commitment as well as resident available up front since
    // zeroing may get starved priority-wise.
    //
    // Note using MmSecondaryColors here would be excessively wasteful
    // on NUMA systems.  MmSecondaryColorMask + 1 is correct for all platforms.
    //

    PagesToZero = SecondaryColorMask + 1;

    if (PagesToZero > NUMBER_OF_ZEROING_PTES) {
        PagesToZero = NUMBER_OF_ZEROING_PTES;
    }

    if (MiChargeCommitment (PagesToZero, NULL) == TRUE) {

        LOCK_PFN (OldIrql);

        //
        // Check to make sure the physical pages are available.
        //

        if (MI_NONPAGEABLE_MEMORY_AVAILABLE() > (SPFN_NUMBER)(PagesToZero)) {
            MI_DECREMENT_RESIDENT_AVAILABLE (PagesToZero,
                                    MM_RESAVAIL_ALLOCATE_ZERO_PAGE_CLUSTERS);
            MaximumPagesToZero = PagesToZero;
        }

        UNLOCK_PFN (OldIrql);
    }

#endif

    //
    // The following code sets the current thread's base priority to zero
    // and then sets its current priority to zero. This ensures that the
    // thread always runs at a priority of zero.
    //

    KeSetPriorityZeroPageThread (0);

    //
    // Initialize wait object array for multiple wait
    //

    WaitObjects[MM_ZERO_PAGE_OBJECT] = &MmZeroingPageEvent;
    WaitObjects[PO_SYS_IDLE_OBJECT] = &PoSystemIdleTimer;

    Color = 0;
    PfnAllocation = (PMMPFN) MM_EMPTY_LIST;

    //
    // Loop forever zeroing pages.
    //

    do {

        //
        // Wait until there are at least MmZeroPageMinimum pages
        // on the free list.
        //

        Status = KeWaitForMultipleObjects (NUMBER_WAIT_OBJECTS,
                                           WaitObjects,
                                           WaitAny,
                                           WrFreePage,
                                           KernelMode,
                                           FALSE,
                                           NULL,
                                           NULL);

        if (Status == PO_SYS_IDLE_OBJECT) {

            //
            // Raise the priority and base priority of the current thread
            // above zero so it can participate in priority boosts provided
            // during the ready thread scan performed by the balance set
            // manager.
            //

            KeSetPriorityZeroPageThread (1);
            PoSystemIdleWorker (TRUE);

            //
            // Lower the priority and base priority of the current thread
            // back to zero so it will not participate in ready thread scan
            // priority boosts.
            //

            KeSetPriorityZeroPageThread (0);
            continue;
        }

        PagesToZero = 0;

        LOCK_PFN (OldIrql);

        do {

            if (MmFreePageListHead.Total == 0) {

                //
                // No pages on the free list at this time, wait for
                // some more.
                //

                MmZeroingPageThreadActive = FALSE;
                UNLOCK_PFN (OldIrql);
                break;
            }

            if (MiZeroingDisabled == TRUE) {
                MmZeroingPageThreadActive = FALSE;
                UNLOCK_PFN (OldIrql);
                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&MmHalfSecond);
                break;
            }

#if defined(MI_MULTINODE)

            //
            // In a multinode system, zero pages by node.  Resume on
            // the last node examined, find a node with free pages that
            // need to be zeroed.
            //

            if (KeNumberNodes > 1) {

                n = LastNodeZeroing;

                for (i = 0; i < KeNumberNodes; i += 1) {
                    if (KeNodeBlock[n]->FreeCount[FreePageList] != 0) {
                        break;
                    }
                    n = (n + 1) % KeNumberNodes;
                }

                ASSERT (i != KeNumberNodes);
                ASSERT (KeNodeBlock[n]->FreeCount[FreePageList] != 0);

                if (n != LastNodeZeroing) {
                    Color = KeNodeBlock[n]->MmShiftedColor;
                }
            }
#endif
                
            ASSERT (PagesToZero == 0);

            StartColor = Color;

            do {
                            
                PageFrame = FreePagesByColor[Color].Flink;

                if (PageFrame != MM_EMPTY_LIST) {

                    Pfn1 = MI_PFN_ELEMENT (PageFrame);

                    //
                    // Check the frame carefully because a single bit (hardware)
                    // error causing us to zero the wrong frame is very hard
                    // to reconstruct after the fact.
                    //

                    if ((Pfn1->u3.e1.PageLocation != FreePageList) ||
                        (Pfn1->u3.e2.ReferenceCount != 0)) {

                        //
                        // Someone has removed a page from the colored lists
                        // chain without updating the freelist chain.
                        //

                        KeBugCheckEx (PFN_LIST_CORRUPT,
                                      0x8D,
                                      PageFrame,
                                      (Pfn1->u3.e2.ShortFlags << 16) |
                                        Pfn1->u3.e2.ReferenceCount,
                                      (ULONG_PTR) Pfn1->PteAddress);
                    }

                    PageFrame1 = MiRemoveAnyPage (Color);

                    if (PageFrame != PageFrame1) {

                        //
                        // Someone has removed a page from the colored lists
                        // chain without updating the freelist chain.
                        //

                        KeBugCheckEx (PFN_LIST_CORRUPT,
                                      0x8E,
                                      PageFrame,
                                      PageFrame1,
                                      0);
                    }

                    Pfn1->u1.Flink = (PFN_NUMBER) PfnAllocation;


                    //
                    // Temporarily mark the page as bad so that contiguous
                    // memory allocators won't steal it when we release
                    // the PFN lock below.  This also prevents the
                    // MiIdentifyPfn code from trying to identify it as
                    // we haven't filled in all the fields yet.
                    //

                    Pfn1->u3.e1.PageLocation = BadPageList;

                    PfnAllocation = Pfn1;

                    PagesToZero += 1;
                }

                //
                // March to the next color - this will be used to finish
                // filling the current chunk or to start the next one.
                //

                Color = (Color & ~SecondaryColorMask) |
                        ((Color + 1) & SecondaryColorMask);

                if (PagesToZero == MaximumPagesToZero) {
                    break;
                }

                if (Color == StartColor) {
                    break;
                }

            } while (TRUE);

            ASSERT (PfnAllocation != (PMMPFN) MM_EMPTY_LIST);

            UNLOCK_PFN (OldIrql);

#if defined(MI_MULTINODE)

            //
            // If a node switch is in order, do it now that the PFN
            // lock has been released.
            //

            if ((KeNumberNodes > 1) && (n != LastNodeZeroing)) {
                LastNodeZeroing = n;

                ProcessorMask = KeNodeBlock[n]->ProcessorMask;

                //
                // Only affinitize if the node has a processor.  Otherwise
                // just stay with the last ideal processor that was set.
                //

                if (ProcessorMask != 0) {

                    KeFindFirstSetLeftAffinity (ProcessorMask, &i);

                    if (i != NO_BITS_FOUND) {
                        KeSetIdealProcessorThread (KeGetCurrentThread(), (UCHAR)i);
                    }
                }
            }

#endif

            ZeroedAlready = FALSE;

            if (ZeroedAlready == FALSE) {
                ZeroBase = MiMapPagesToZeroInHyperSpace (PfnAllocation, PagesToZero);

                MACHINE_ZERO_PAGE (ZeroBase, PagesToZero << PAGE_SHIFT);

                MiUnmapPagesInZeroSpace (ZeroBase, PagesToZero);
            }

            PagesToZero = 0;

            Pfn1 = PfnAllocation;

            LOCK_PFN (OldIrql);

            do {

                PageFrame = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

                Pfn1 = (PMMPFN) Pfn1->u1.Flink;

                MiInsertPageInList (&MmZeroedPageListHead, PageFrame);

            } while (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

            //
            // We just finished processing a cluster of pages - briefly
            // release the PFN lock to allow other threads to make progress.
            //

            UNLOCK_PFN (OldIrql);

            PfnAllocation = (PMMPFN) MM_EMPTY_LIST;

            LOCK_PFN (OldIrql);

        } while (TRUE);

    } while (TRUE);
}

#if !defined(NT_UP)


VOID
MiZeroPageWorker (
    IN PVOID Context
    )

/*++

Routine Description:

    This routine is the worker routine executed by all processors so that
    initial page zeroing occurs in parallel.

Arguments:

    Context - Supplies a pointer to the workitem.

Return Value:

    None.

Environment:

    Kernel mode initialization time, PASSIVE_LEVEL.  Because this is INIT
    only code, don't bother charging commit for the pages.

--*/

{
    MMPTE TempPte;
    MMPTE DefaultCachedPte;
    PMMPTE PointerPte;
    KAFFINITY Affinity;
    KIRQL OldIrql;
    PVOID ZeroBase;
    PKTHREAD Thread;
    CCHAR OldProcessor;
    SCHAR OldBasePriority;
    KPRIORITY OldPriority;
    PWORK_QUEUE_ITEM WorkItem;
    PMMPFN Pfn1;
    PFN_COUNT PagesToZero;
    PFN_COUNT MaximumPagesToZero;
    PFN_NUMBER PageFrame;
    PFN_NUMBER PageFrame1;
    PMMPFN PfnAllocation;
    ULONG Color;
    ULONG StartColor;
    LOGICAL ZeroedAlready;
    PMMCOLOR_TABLES FreePagesByColor;
    ULONG SecondaryColorMask;

    WorkItem = (PWORK_QUEUE_ITEM) Context;

    ExFreePool (WorkItem);

    DefaultCachedPte = ValidKernelPte;

    //
    // Make local copies of globals so they don't have to be wastefully
    // refetched while holding the PFN lock.
    //

    FreePagesByColor = MmFreePagesByColor[FreePageList];
    SecondaryColorMask = MmSecondaryColorMask;

    //
    // The following code sets the current thread's base and current priorities
    // to one so all other code (except the zero page thread) can preempt it.
    //

    Thread = KeGetCurrentThread ();
    OldBasePriority = Thread->BasePriority;
    Thread->BasePriority = 1;
    OldPriority = KeSetPriorityThread (Thread, 1);

    //
    // Dispatch each worker thread to the next processor in line.
    //

    OldProcessor = (CCHAR) InterlockedIncrement (&MiNextZeroProcessor);

    Affinity = AFFINITY_MASK (OldProcessor);

    KeSetSystemAffinityThread (Affinity);

    Color = 0;

#if !defined(NT_UP)

    //
    // Make an attempt to stagger the processors ...
    //

    Color = (MmSecondaryColors / KeNumberProcessors) * OldProcessor;
    Color &= SecondaryColorMask;
#endif

#if defined(MI_MULTINODE)
    if (KeNumberNodes > 1) {
        Color = KeGetCurrentNode()->MmShiftedColor;
    }
#endif

    //
    // Zero groups of local pages at once to reduce PFN lock contention.
    //
    // Note using MmSecondaryColors here would be excessively wasteful
    // on NUMA systems.  MmSecondaryColorMask + 1 is correct for all platforms.
    //

    MaximumPagesToZero = SecondaryColorMask + 1;


    //
    // Zero a maximum of 64k at a time since 64k is the largest binned
    // system PTE size (for systems that need to use PTEs for zeroing).
    //

    if (MaximumPagesToZero > (64 * 1024) / PAGE_SIZE) {
        MaximumPagesToZero = (64 * 1024) / PAGE_SIZE;
    }

    PagesToZero = 0;

    PfnAllocation = (PMMPFN) MM_EMPTY_LIST;

    LOCK_PFN (OldIrql);

    do {

        StartColor = Color;

        ASSERT (PagesToZero == 0);
        ASSERT (PfnAllocation == (PMMPFN) MM_EMPTY_LIST);

        do {
                        
            PageFrame = FreePagesByColor[Color].Flink;

            if (PageFrame != MM_EMPTY_LIST) {

                Pfn1 = MI_PFN_ELEMENT (PageFrame);

                //
                // Check the frame carefully because a single bit (hardware)
                // error causing us to zero the wrong frame is very hard
                // to reconstruct after the fact.
                //

                if ((Pfn1->u3.e1.PageLocation != FreePageList) ||
                    (Pfn1->u3.e2.ReferenceCount != 0)) {

                    //
                    // Someone has removed a page from the colored lists
                    // chain without updating the freelist chain.
                    //

                    KeBugCheckEx (PFN_LIST_CORRUPT,
                                  0x8D,
                                  PageFrame,
                                  (Pfn1->u3.e2.ShortFlags << 16) |
                                    Pfn1->u3.e2.ReferenceCount,
                                  (ULONG_PTR) Pfn1->PteAddress);
                }

                PageFrame1 = MiRemoveAnyPage (Color);

                if (PageFrame != PageFrame1) {

                    //
                    // Someone has removed a page from the colored lists
                    // chain without updating the freelist chain.
                    //

                    KeBugCheckEx (PFN_LIST_CORRUPT,
                                  0x8E,
                                  PageFrame,
                                  PageFrame1,
                                  0);
                }

                Pfn1->u1.Flink = (PFN_NUMBER) PfnAllocation;

                //
                // Temporarily mark the page as bad so that contiguous
                // memory allocators won't steal it when we release
                // the PFN lock below.  This also prevents the
                // MiIdentifyPfn code from trying to identify it as
                // we haven't filled in all the fields yet.
                //

                Pfn1->u3.e1.PageLocation = BadPageList;

                PfnAllocation = Pfn1;

                PagesToZero += 1;
            }

            //
            // March to the next color - this will be used to finish
            // filling the current chunk or to start the next one.
            //

            Color = (Color & ~SecondaryColorMask) |
                    ((Color + 1) & SecondaryColorMask);

            if (PagesToZero == MaximumPagesToZero) {
                break;
            }

            if (Color == StartColor) {

                if (PagesToZero == 0) {

                    //
                    // All of the pages for this node have been zeroed, bail.
                    //

                    ASSERT (PfnAllocation == (PMMPFN) MM_EMPTY_LIST);
                    UNLOCK_PFN (OldIrql);
                    goto ZeroingComplete;
                }

                break;
            }

        } while (TRUE);

        ASSERT (PagesToZero != 0);
        ASSERT (PfnAllocation != (PMMPFN) MM_EMPTY_LIST);

        //
        // Use system PTEs instead of hyperspace to zero the page so that
        // a spinlock (ie: interrupts blocked) is not held while zeroing.
        // Since system PTE acquisition is lock free and the TB lazy flushed,
        // this is perhaps the best path regardless.
        //

        UNLOCK_PFN (OldIrql);

        ZeroedAlready = FALSE;

        if (ZeroedAlready == FALSE) {

            Pfn1 = PfnAllocation;

            PointerPte = MiReserveSystemPtes (PagesToZero, SystemPteSpace);
    
            if (PointerPte == NULL) {
    
#if DBG
                MiInitialZeroNoPtes += 1;
#endif

                //
                // Put these pages back on the freelist.
                //
    
                LOCK_PFN (OldIrql);
    
                do {

                    PageFrame = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

                    Pfn1 = (PMMPFN) Pfn1->u1.Flink;

                    MiInsertPageInFreeList (PageFrame);

                } while (Pfn1 != (PMMPFN) MM_EMPTY_LIST);
    
                UNLOCK_PFN (OldIrql);
    
                break;
            }
    
            ZeroBase = MiGetVirtualAddressMappedByPte (PointerPte);
    
            do {

                ASSERT (PointerPte->u.Hard.Valid == 0);

                PageFrame = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

                TempPte = DefaultCachedPte;

                if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                }
                else if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {
                    MI_DISABLE_CACHING (TempPte);
                }

                TempPte.u.Hard.PageFrameNumber = PageFrame;

                MI_WRITE_VALID_PTE (PointerPte, TempPte);

                PointerPte += 1;
    
                Pfn1 = (PMMPFN) Pfn1->u1.Flink;

            } while (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

            KeZeroPages (ZeroBase, PagesToZero << PAGE_SHIFT);
    
            PointerPte -= PagesToZero;

            MiReleaseSystemPtes (PointerPte, PagesToZero, SystemPteSpace);
        }

        PagesToZero = 0;

        Pfn1 = PfnAllocation;

        PfnAllocation = (PMMPFN) MM_EMPTY_LIST;

        LOCK_PFN (OldIrql);

        do {

            PageFrame = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

            Pfn1 = (PMMPFN) Pfn1->u1.Flink;

            MiInsertZeroListAtBack (PageFrame);

        } while (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

    } while (TRUE);

ZeroingComplete:

    //
    // Restore the entry thread priority and processor affinity.
    //

    KeRevertToUserAffinityThread ();

    KeSetPriorityThread (Thread, OldPriority);

    Thread->BasePriority = OldBasePriority;

    return;
}


VOID
MiStartZeroPageWorkers (
    VOID
    )

/*++

Routine Description:

    This routine starts the zero page worker threads.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode initialization phase 1, PASSIVE_LEVEL.

--*/

{
    ULONG i;
    PWORK_QUEUE_ITEM WorkItem;

    for (i = 0; i < (ULONG) KeNumberProcessors; i += 1) {

        WorkItem = ExAllocatePoolWithTag (NonPagedPool,
                                          sizeof (WORK_QUEUE_ITEM),
                                          'wZmM');

        if (WorkItem == NULL) {
            break;
        }

        ExInitializeWorkItem (WorkItem, MiZeroPageWorker, (PVOID) WorkItem);

        ExQueueWorkItem (WorkItem, CriticalWorkQueue);
    }
}

#endif

