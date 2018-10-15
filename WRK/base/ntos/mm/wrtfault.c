/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   wrtfault.c

Abstract:

    This module contains the copy on write routine for memory management.

--*/

#include "mi.h"


LOGICAL
FASTCALL
MiCopyOnWrite (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte
    )

/*++

Routine Description:

    This routine performs a copy on write operation for the specified
    virtual address.

Arguments:

    FaultingAddress - Supplies the virtual address which caused the fault.

    PointerPte - Supplies the pointer to the PTE which caused the page fault.

Return Value:

    Returns TRUE if the page was actually split, FALSE if not.

Environment:

    Kernel mode, APCs disabled, working set mutex held.

--*/

{
    MMPTE TempPte;
    MMPTE TempPte2;
    PMMPTE MappingPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NewPageIndex;
    PVOID CopyTo;
    PVOID CopyFrom;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PEPROCESS CurrentProcess;
    PMMCLONE_BLOCK CloneBlock;
    PMMCLONE_DESCRIPTOR CloneDescriptor;
    WSLE_NUMBER WorkingSetIndex;
    LOGICAL FakeCopyOnWrite;
    PMMWSL WorkingSetList;
    PVOID SessionSpace;
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;

    //
    // This is called from MmAccessFault, the PointerPte is valid
    // and the working set mutex ensures it cannot change state.
    //
    // Capture the PTE contents to TempPte.
    //

    TempPte = *PointerPte;
    ASSERT (TempPte.u.Hard.Valid == 1);

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    //
    // Check to see if this is a prototype PTE with copy on write enabled.
    //

    FakeCopyOnWrite = FALSE;
    CurrentProcess = PsGetCurrentProcess ();
    CloneBlock = NULL;

    if (FaultingAddress >= (PVOID) MmSessionBase) {

        WorkingSetList = MmSessionSpace->Vm.VmWorkingSetList;
        ASSERT (Pfn1->u3.e1.PrototypePte == 1);
        SessionSpace = (PVOID) MmSessionSpace;

        MM_SESSION_SPACE_WS_LOCK_ASSERT ();

        if (MmSessionSpace->ImageLoadingCount != 0) {

            NextEntry = MmSessionSpace->ImageList.Flink;
    
            while (NextEntry != &MmSessionSpace->ImageList) {
    
                Image = CONTAINING_RECORD (NextEntry, IMAGE_ENTRY_IN_SESSION, Link);
    
                if ((FaultingAddress >= Image->Address) &&
                    (FaultingAddress <= Image->LastAddress)) {
    
                    if (Image->ImageLoading) {
    
                        ASSERT (Pfn1->u3.e1.PrototypePte == 1);
    
                        TempPte.u.Hard.CopyOnWrite = 0;
                        TempPte.u.Hard.Write = 1;
    
                        //
                        // The page is no longer copy on write, update the PTE
                        // setting both the dirty bit and the accessed bit.
                        //
                        // Even though the page's current backing is the image
                        // file, the modified writer will convert it to
                        // pagefile backing when it notices the change later.
                        //
    
                        MI_SET_PTE_DIRTY (TempPte);
                        MI_SET_ACCESSED_IN_PTE (&TempPte, 1);
    
                        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);
    
                        //
                        // The TB entry must be flushed as the valid PTE with
                        // the dirty bit clear has been fetched into the TB. If
                        // it isn't flushed, another fault is generated as the
                        // dirty bit is not set in the cached TB entry.
                        //
    
                        MI_FLUSH_SINGLE_TB (FaultingAddress, TRUE);
    
                        return FALSE;
                    }
                    break;
                }
    
                NextEntry = NextEntry->Flink;
            }
        }
    }
    else {
        WorkingSetList = MmWorkingSetList;
        SessionSpace = NULL;

        //
        // If a fork operation is in progress, block until the fork is
        // completed, then retry the whole operation as the state of
        // everything may have changed between when the mutexes were
        // released and reacquired.
        //

        if (CurrentProcess->ForkInProgress != NULL) {
            if (MiWaitForForkToComplete (CurrentProcess) == TRUE) {
                return FALSE;
            }
        }

        if (TempPte.u.Hard.CopyOnWrite == 0) {

            //
            // This is a fork page which is being made private in order
            // to change the protection of the page.
            // Do not make the page writable.
            //

            FakeCopyOnWrite = TRUE;
        }
    }

    WorkingSetIndex = MiLocateWsle (FaultingAddress,
                                    WorkingSetList,
                                    Pfn1->u1.WsIndex,
                                    FALSE);

    //
    // The page must be copied into a new page.
    //

    LOCK_PFN (OldIrql);

    if ((MmAvailablePages < MM_HIGH_LIMIT) &&
        (MiEnsureAvailablePageOrWait (SessionSpace != NULL ? HYDRA_PROCESS : CurrentProcess, OldIrql))) {

        //
        // A wait operation was performed to obtain an available
        // page and the working set mutex and PFN lock have
        // been released and various things may have changed for
        // the worse.  Rather than examine all the conditions again,
        // return and if things are still proper, the fault will
        // be taken again.
        //

        UNLOCK_PFN (OldIrql);
        return FALSE;
    }

    //
    // This must be a prototype PTE.  Perform the copy on write.
    //

    ASSERT (Pfn1->u3.e1.PrototypePte == 1);

    //
    // A page is being copied and made private, the global state of
    // the shared page needs to be updated at this point on certain
    // hardware.  This is done by ORing the dirty bit into the modify bit in
    // the PFN element.
    //
    // Note that a session page cannot be dirty (no POSIX-style forking is
    // supported for these drivers).
    //

    if (SessionSpace != NULL) {
        ASSERT ((TempPte.u.Hard.Valid == 1) && (TempPte.u.Hard.Write == 0));
        ASSERT (!MI_IS_PTE_DIRTY (TempPte));

        NewPageIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_SESSION(MmSessionSpace));
    }
    else {
        MI_CAPTURE_DIRTY_BIT_TO_PFN (PointerPte, Pfn1);
        CloneBlock = (PMMCLONE_BLOCK) Pfn1->PteAddress;

        //
        // Get a new page with the same color as this page.
        //

        NewPageIndex = MiRemoveAnyPage (
                        MI_PAGE_COLOR_PTE_PROCESS(PageFrameIndex,
                                              &CurrentProcess->NextPageColor));
    }

    MiInitializeCopyOnWritePfn (NewPageIndex,
                                PointerPte,
                                WorkingSetIndex,
                                WorkingSetList);

    UNLOCK_PFN (OldIrql);

    InterlockedIncrement (&KeGetCurrentPrcb ()->MmCopyOnWriteCount);

    CopyFrom = PAGE_ALIGN (FaultingAddress);

    MappingPte = MiReserveSystemPtes (1, SystemPteSpace);

    if (MappingPte != NULL) {

        MI_MAKE_VALID_KERNEL_PTE (TempPte2,
                                  NewPageIndex,
                                  MM_READWRITE,
                                  MappingPte);

        MI_SET_PTE_DIRTY (TempPte2);

        if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {
            MI_DISABLE_CACHING (TempPte2);
        }
        else if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
            MI_SET_PTE_WRITE_COMBINE (TempPte2);
        }

        MI_WRITE_VALID_PTE (MappingPte, TempPte2);

        CopyTo = MiGetVirtualAddressMappedByPte (MappingPte);
    }
    else {

        CopyTo = MiMapPageInHyperSpace (CurrentProcess,
                                        NewPageIndex,
                                        &OldIrql);
    }

    KeCopyPage (CopyTo, CopyFrom);

    if (MappingPte != NULL) {
        MiReleaseSystemPtes (MappingPte, 1, SystemPteSpace);
    }
    else {
        MiUnmapPageInHyperSpace (CurrentProcess, CopyTo, OldIrql);
    }

    if (!FakeCopyOnWrite) {

        //
        // If the page was really a copy on write page, make it
        // accessed, dirty and writable.  Also, clear the copy-on-write
        // bit in the PTE.
        //

        MI_SET_PTE_DIRTY (TempPte);
        TempPte.u.Hard.Write = 1;
        MI_SET_ACCESSED_IN_PTE (&TempPte, 1);
        TempPte.u.Hard.CopyOnWrite = 0;
    }

    //
    // Regardless of whether the page was really a copy on write,
    // the frame field of the PTE must be updated.
    //

    TempPte.u.Hard.PageFrameNumber = NewPageIndex;

    //
    // If the modify bit is set in the PFN database for the
    // page, the data cache must be flushed.  This is due to the
    // fact that this process may have been cloned and the cache
    // still contains stale data destined for the page we are
    // going to remove.
    //

    ASSERT (TempPte.u.Hard.Valid == 1);

    MI_WRITE_VALID_PTE_NEW_PAGE (PointerPte, TempPte);

    //
    // Flush the TB entry for this page.
    //

    if (SessionSpace == NULL) {

        MI_FLUSH_SINGLE_TB (FaultingAddress, FALSE);

        //
        // Increment the number of private pages.
        //

        CurrentProcess->NumberOfPrivatePages += 1;
    }
    else {

        MI_FLUSH_SINGLE_TB (FaultingAddress, TRUE);

        ASSERT (Pfn1->u3.e1.PrototypePte == 1);
    }

    //
    // Decrement the share count for the page which was copied
    // as this PTE no longer refers to it.
    //

    LOCK_PFN (OldIrql);

    MiDecrementShareCount (Pfn1, PageFrameIndex);

    if (SessionSpace == NULL) {

        CloneDescriptor = MiLocateCloneAddress (CurrentProcess,
                                                (PVOID)CloneBlock);

        if (CloneDescriptor != NULL) {

            //
            // Decrement the reference count for the clone block,
            // note that this could release and reacquire the mutexes.
            //

            MiDecrementCloneBlockReference (CloneDescriptor,
                                            CloneBlock,
                                            CurrentProcess,
                                            NULL,
                                            OldIrql);
        }
    }

    UNLOCK_PFN (OldIrql);
    return TRUE;
}


#if !defined(NT_UP)

LOGICAL
MiSetDirtyBit (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN ULONG PfnHeld
    )

/*++

Routine Description:

    This routine sets dirty in the specified PTE and the modify bit in the
    corresponding PFN element.  If any page file space is allocated, it
    is deallocated.

Arguments:

    FaultingAddress - Supplies the faulting address.

    PointerPte - Supplies a pointer to the corresponding valid PTE.

    PfnHeld - Supplies TRUE if the PFN lock is already held.

Return Value:

    TRUE if action was taken, FALSE if not.

Environment:

    Kernel mode, APCs disabled, working set pushlock held.

--*/

{
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;

    //
    // The page is NOT copy on write, update the PTE setting both the
    // dirty bit and the accessed bit. Note, that as this PTE is in
    // the TB, the TB must be flushed.
    //

    TempPte = *PointerPte;

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);

    //
    // This may be a PTE from a rotate physical frame so there may be no
    // corresponding PFN for it.
    //

    if (!MI_IS_PFN (PageFrameIndex)) {
        return FALSE;
    }

    MI_SET_PTE_DIRTY (TempPte);
    MI_SET_ACCESSED_IN_PTE (&TempPte, 1);

    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

    //
    // Check state of PFN lock and if not held, don't update PFN database.
    //

    if (PfnHeld) {

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        //
        // Set the modified field in the PFN database, also, if the physical
        // page is currently in a paging file, free up the page file space
        // as the contents are now worthless.
        //

        if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
            (Pfn1->u3.e1.WriteInProgress == 0)) {

            //
            // This page is in page file format, deallocate the page file space.
            //

            MiReleasePageFileSpace (Pfn1->OriginalPte);

            //
            // Change original PTE to indicate no page file space is reserved,
            // otherwise the space will be deallocated when the PTE is
            // deleted.
            //

            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
        }

        MI_SET_MODIFIED (Pfn1, 1, 0x17);
    }

    //
    // The TB entry must be flushed as the valid PTE with the dirty bit clear
    // has been fetched into the TB.  If it isn't flushed, another fault
    // is generated as the dirty bit is not set in the cached TB entry.
    //

    KeFillEntryTb (FaultingAddress);
    return TRUE;
}
#endif

