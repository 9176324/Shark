/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   deleteva.c

Abstract:

    This module contains the routines for deleting virtual address space.

--*/

#include "mi.h"

#if defined (_WIN64) && defined (DBG_VERBOSE)

typedef struct _MI_TRACK_USE {

    PFN_NUMBER Pfn;
    PVOID Va;
    ULONG Id;
    ULONG PfnUse;
    ULONG PfnUseCounted;
    ULONG TickCount;
    PKTHREAD Thread;
    PEPROCESS Process;
} MI_TRACK_USE, *PMI_TRACK_USE;

ULONG MiTrackUseSize = 8192;
PMI_TRACK_USE MiTrackUse;
LONG MiTrackUseIndex;

VOID
MiInitUseCounts (
    VOID
    )
{
    MiTrackUse = ExAllocatePoolWithTag (NonPagedPool,
                                        MiTrackUseSize * sizeof (MI_TRACK_USE),
                                        'lqrI');
    ASSERT (MiTrackUse != NULL);
}


VOID
MiCheckUseCounts (
    PVOID TempHandle,
    PVOID Va,
    ULONG Id
    )

/*++

Routine Description:

    This routine ensures that all the counters are correct.

Arguments:

    TempHandle - Supplies the handle for used page table counts.

    Va - Supplies the virtual address.

    Id - Supplies the ID.

Return Value:

    None.

Environment:

    Kernel mode, called with APCs disabled, working set mutex and PFN lock
    held.

--*/

{
    LOGICAL LogIt;
    ULONG i;
    ULONG TempHandleCount;
    ULONG TempCounted;
    PMMPTE TempPage;
    KIRQL OldIrql;
    ULONG Index;
    PFN_NUMBER PageFrameIndex;
    PMI_TRACK_USE Information;
    LARGE_INTEGER TimeStamp;
    PMMPFN Pfn1;
    PEPROCESS Process;

    Process = PsGetCurrentProcess ();

    //
    // TempHandle is really the PMMPFN containing the UsedPageTableEntries.
    //

    Pfn1 = (PMMPFN)TempHandle;
    PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

    TempHandleCount = MI_GET_USED_PTES_FROM_HANDLE (TempHandle);

    if (Id & 0x80000000) {
        ASSERT (TempHandleCount != 0);
    }

    TempPage = (PMMPTE) MiMapPageInHyperSpace (Process, PageFrameIndex, &OldIrql);

    TempCounted = 0;

    for (i = 0; i < PTE_PER_PAGE; i += 1) {
        if (TempPage->u.Long != 0) {
            TempCounted += 1;
        }
        TempPage += 1;
    }

    LogIt = TRUE;

    if (LogIt == TRUE) {

        //
        // Capture information
        //

        Index = InterlockedExchangeAdd(&MiTrackUseIndex, 1);                    
        Index &= (MiTrackUseSize - 1);

        Information = &(MiTrackUse[Index]);

        Information->Thread = KeGetCurrentThread();
        Information->Process = (PEPROCESS)((ULONG_PTR)PsGetCurrentProcess () + KeGetCurrentProcessorNumber ());
        Information->Va = Va;
        Information->Id = Id;
        KeQueryTickCount(&TimeStamp);
        Information->TickCount = TimeStamp.LowPart;
        Information->Pfn = PageFrameIndex;
        Information->PfnUse = TempHandleCount;
        Information->PfnUseCounted = TempCounted;

        if (TempCounted != TempHandleCount) {
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                "MiCheckUseCounts %p %x %x %x %x\n", Va, Id, PageFrameIndex, TempHandleCount, TempCounted);
            DbgBreakPoint ();
        }
    }

    MiUnmapPageInHyperSpace (Process, TempPage, OldIrql);
    return;
}
#endif


VOID
MiDeleteVirtualAddresses (
    IN PUCHAR Va,
    IN PUCHAR EndingAddress,
    IN PMMVAD Vad
    )

/*++

Routine Description:

    This routine deletes the specified virtual address range within
    the current process.

Arguments:

    Va - Supplies the first virtual address to delete.

    EndingAddress - Supplies the last address to delete.

    Vad - Supplies the virtual address descriptor which maps this range
          or NULL if we are not concerned about views.  From the Vad the
          range of prototype PTEs is determined and this information is
          used to uncover if the PTE refers to a prototype PTE or a fork PTE.

Return Value:

    None.

Environment:

    Kernel mode, APC_LEVEL, called with address space and working set mutexes
    held.  The working set mutex may be released and reacquired to fault
    pages in.

--*/

{
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    WSLE_NUMBER WsPfnIndex;
    ULONG InvalidPtes;
    LOGICAL PfnHeld;
    PVOID TempVa;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE ProtoPte;
    PMMPTE LastProtoPte;
    PMMPTE LastPte;
    PMMPTE LastPteThisPage;
    MMPTE TempPte;
    PEPROCESS CurrentProcess;
    PSUBSECTION Subsection;
    PVOID UsedPageTableHandle;
    KIRQL OldIrql;
    MMPTE_FLUSH_LIST FlushList;
    ULONG Waited;
    LOGICAL Skipped;
    LOGICAL AddressSpaceDeletion;
#if DBG
    PMMPTE ProtoPte2;
    PMMPTE LastProtoPte2;
    PMMCLONE_BLOCK CloneBlock;
    PMMCLONE_DESCRIPTOR CloneDescriptor;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PVOID UsedPageDirectoryHandle;
    PVOID TempHandle;
#endif

    FlushList.Count = 0;

    CurrentProcess = PsGetCurrentProcess ();

    PointerPpe = MiGetPpeAddress (Va);
    PointerPde = MiGetPdeAddress (Va);
    PointerPte = MiGetPteAddress (Va);
    PointerPxe = MiGetPxeAddress (Va);
    LastPte = MiGetPteAddress (EndingAddress);
    PfnHeld = FALSE;
    OldIrql = MM_NOIRQL;
    Skipped = TRUE;

    SATISFY_OVERZEALOUS_COMPILER (Subsection = NULL);

    if ((Vad == NULL) ||
        (Vad->u.VadFlags.PrivateMemory) ||
        (Vad->FirstPrototypePte == NULL)) {

        ProtoPte = NULL;
        LastProtoPte = NULL;
    }
    else {
        ProtoPte = Vad->FirstPrototypePte;
        LastProtoPte = (PMMPTE) 4;
    }

    if (CurrentProcess->CloneRoot == NULL) {
        AddressSpaceDeletion = TRUE;
    }
    else {
        AddressSpaceDeletion = FALSE;
    }

    do {

        //
        // Attempt to leap forward skipping over empty page directories
        // and page tables where possible.
        //

#if (_MI_PAGING_LEVELS >= 3)
restart:
#endif

        while (MiDoesPxeExistAndMakeValid (PointerPxe,
                                           CurrentProcess,
                                           MM_NOIRQL,
                                           &Waited) == FALSE) {
    
            //
            // This extended page directory parent entry is empty,
            // go to the next one.
            //
    
            Skipped = TRUE;
            PointerPxe += 1;
            PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            Va = MiGetVirtualAddressMappedByPte (PointerPte);
    
            if (Va > EndingAddress) {
    
                //
                // All done, return.
                //
    
                return;
            }
        }
    
        while (MiDoesPpeExistAndMakeValid (PointerPpe,
                                           CurrentProcess,
                                           MM_NOIRQL,
                                           &Waited) == FALSE) {
    
            //
            // This page directory parent entry is empty, go to the next one.
            //
    
            Skipped = TRUE;
            PointerPpe += 1;
            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            Va = MiGetVirtualAddressMappedByPte (PointerPte);
    
            if (Va > EndingAddress) {
    
                //
                // All done, return.
                //
    
                return;
            }
#if (_MI_PAGING_LEVELS >= 4)
            if (MiIsPteOnPdeBoundary (PointerPpe)) {
                PointerPxe += 1;
                ASSERT (PointerPxe == MiGetPteAddress (PointerPpe));
                goto restart;
            }
#endif
        }

#if (_MI_PAGING_LEVELS >= 3) && defined (DBG)
        MI_CHECK_USED_PTES_HANDLE (PointerPte);
        TempHandle = MI_GET_USED_PTES_HANDLE (PointerPte);
        ASSERT ((MI_GET_USED_PTES_FROM_HANDLE (TempHandle) != 0) ||
                (PointerPde->u.Long == 0));
#endif

        while (MiDoesPdeExistAndMakeValid (PointerPde,
                                           CurrentProcess,
                                           MM_NOIRQL,
                                           &Waited) == FALSE) {
    
            //
            // This page directory entry is empty, go to the next one.
            //
    
            Skipped = TRUE;
            PointerPde += 1;
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            Va = MiGetVirtualAddressMappedByPte (PointerPte);
    
            if (Va > EndingAddress) {
    
                //
                // All done, return.
                //
    
                return;
            }
    
#if (_MI_PAGING_LEVELS >= 3)
            if (MiIsPteOnPdeBoundary (PointerPde)) {
                PointerPpe += 1;
                ASSERT (PointerPpe == MiGetPteAddress (PointerPde));
                PointerPxe = MiGetPteAddress (PointerPpe);
                goto restart;
            }
#endif

#if DBG
            if ((LastProtoPte != NULL)  &&
                (Vad->u2.VadFlags2.ExtendableFile == 0)) {

                ProtoPte2 = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN (Va));
                Subsection = MiLocateSubsection (Vad,MI_VA_TO_VPN (Va));
                LastProtoPte2 = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
                if (Vad->u.VadFlags.VadType != VadImageMap) {
                    if ((ProtoPte2 < Subsection->SubsectionBase) ||
                        (ProtoPte2 >= LastProtoPte2)) {
                        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                            "bad proto PTE %p va %p Vad %p sub %p\n",
                            ProtoPte2, Va, Vad, Subsection);
                        DbgBreakPoint ();
                    }
                }
            }
#endif
        }
    
        //
        // The PPE and PDE are now valid, get the page table use address
        // as it changes whenever the PDE does.
        //

#if (_MI_PAGING_LEVELS >= 4)
        ASSERT64 (PointerPxe->u.Hard.Valid == 1);
#endif
        ASSERT64 (PointerPpe->u.Hard.Valid == 1);
        ASSERT (PointerPde->u.Hard.Valid == 1);
        ASSERT (Va <= EndingAddress);

        MI_CHECK_USED_PTES_HANDLE (Va);
        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (Va);

#if (_MI_PAGING_LEVELS >= 3) && defined (DBG)
        ASSERT ((MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) != 0) ||
                (PointerPte->u.Long == 0));
#endif

        //
        // If we skipped chunks of address space, the prototype PTE pointer
        // must be updated now so VADs that span multiple subsections
        // are handled properly.
        //

        if ((Skipped == TRUE) && (LastProtoPte != NULL)) {

            ProtoPte = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN(Va));
            Subsection = MiLocateSubsection (Vad, MI_VA_TO_VPN(Va));

            if (Subsection != NULL) {
                LastProtoPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
#if DBG
                if (Vad->u.VadFlags.VadType != VadImageMap) {
                    if ((ProtoPte < Subsection->SubsectionBase) ||
                        (ProtoPte >= LastProtoPte)) {
                        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                            "bad proto PTE %p va %p Vad %p sub %p\n",
                            ProtoPte, Va, Vad, Subsection);
                        DbgBreakPoint ();
                    }
                }
#endif
            }
            else {

                //
                // The Vad span is larger than the section being mapped.
                // Null the proto PTE local as no more proto PTEs will
                // need to be deleted at this point.
                //

                LastProtoPte = NULL;
            }
        }

        //
        // A valid address has been located, examine and delete each PTE.
        //

        InvalidPtes = 0;

        if (AddressSpaceDeletion == TRUE) {

            //
            // The working set mutex is held so no valid PTEs can be trimmed.
            // Take advantage of this fact and remove the WSLEs for all valid
            // PTEs now since the PFN lock is not held.  This is only done
            // for non-forked processes as deletion of forked PTEs below may
            // drop the working set mutex which would introduce races wth
            // WSLE deletion being done here.
            //
            // The deleting of the PTEs will require the PFN lock.
            //
    
            ASSERT (CurrentProcess->CloneRoot == NULL);

            LastPteThisPage = (PMMPTE)(((ULONG_PTR)PointerPte | (PAGE_SIZE - 1)) + 1) - 1;
            if (LastPteThisPage > LastPte) {
                LastPteThisPage = LastPte;
            }
    
            TempVa = MiGetVirtualAddressMappedByPte (LastPteThisPage);
    
            do {
                TempPte = *LastPteThisPage;
        
                if (TempPte.u.Hard.Valid != 0) {
#ifdef _X86_
#if DBG
#if !defined(NT_UP)
    
                    if (TempPte.u.Hard.Writable == 1) {
                        ASSERT (TempPte.u.Hard.Dirty == 1);
                    }
#endif //NTUP
#endif //DBG
#endif //X86
    
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    WsPfnIndex = Pfn1->u1.WsIndex;
    
                    //
                    // PTE is valid - find the WSLE for this page and eliminate it.
                    //
    
                    MiTerminateWsle (TempVa, &CurrentProcess->Vm, WsPfnIndex);
                }
        
                LastPteThisPage -= 1;
                TempVa = (PVOID)((ULONG_PTR)TempVa - PAGE_SIZE);
    
            } while (LastPteThisPage >= PointerPte);
        }

        do {
    
            TempPte = *PointerPte;
    
            if (TempPte.u.Long != 0) {
    
                //
                // One less used page table entry in this page table page.
                //
    
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
    
                if (IS_PTE_NOT_DEMAND_ZERO (TempPte)) {
    
                    if (LastProtoPte != NULL) {
                        if (ProtoPte >= LastProtoPte) {
                            ProtoPte = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN(Va));
                            Subsection = MiLocateSubsection (Vad, MI_VA_TO_VPN(Va));
    
                            //
                            // Subsection may be NULL if this PTE contains the
                            // "search the VAD tree" encoding and the
                            // corresponding prototype PTE is in the unused
                            // PTE range of the segment - ie: a thread tried
                            // to reach beyond the end of his section,
                            // we encode the PTE this way initially during
                            // the fault processing and then later during
                            // the fault give the thread the AV - we don't
                            // clear the PTE encoding then, so we have to
                            // handle it now.
                            //
    
                            if (Subsection != NULL) {
                                LastProtoPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
                            }
                            else {
    
                                //
                                // No more prototype PTEs need to be deleted
                                // as we've passed the end of the valid portion
                                // of the segment, so clear LastProtoPte.
                                //
    
                                LastProtoPte = NULL;
                            }
                        }
#if DBG
                        if ((Vad->u.VadFlags.VadType != VadImageMap) && (LastProtoPte != NULL)) {
                            if ((ProtoPte < Subsection->SubsectionBase) ||
                                (ProtoPte >= LastProtoPte)) {
                                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                                    "bad proto PTE %p va %p Vad %p sub %p\n",
                                    ProtoPte, Va, Vad, Subsection);
                                DbgBreakPoint();
                            }
                        }
#endif
                    }
    
                    if ((TempPte.u.Hard.Valid == 0) &&
                        (TempPte.u.Soft.Prototype == 1) &&
                        (AddressSpaceDeletion == TRUE)) {
    
                        //
                        // Pure (ie: not forked) prototype PTEs don't need PFN
                        // protection to be deleted.
                        //
    
                        ASSERT (CurrentProcess->CloneRoot == NULL);

#if DBG
                        if ((PointerPte <= MiHighestUserPte) &&
                            (TempPte.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) &&
                            (ProtoPte != MiPteToProto (PointerPte))) {
    
                            //
                            // Make sure the prototype PTE is a fork
                            // prototype PTE.
                            //
    
                            CloneBlock = (PMMCLONE_BLOCK)MiPteToProto (PointerPte);
                            CloneDescriptor = MiLocateCloneAddress (CurrentProcess, (PVOID)CloneBlock);
    
                            if (CloneDescriptor == NULL) {
                                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                                    "0PrototypePte %p Clone desc %p %p\n",
                                    PrototypePte, CloneDescriptor, PointerPte);
                                ASSERT (FALSE);
                            }
                        }
#endif
    
                        InvalidPtes += 1;
                        MI_WRITE_ZERO_PTE (PointerPte);
                    }
                    else {
                        if (PfnHeld == FALSE) {
                            PfnHeld = TRUE;
                            LOCK_PFN (OldIrql);
                        }
        
                        Waited = MiDeletePte (PointerPte,
                                              (PVOID)Va,
                                              AddressSpaceDeletion,
                                              CurrentProcess,
                                              ProtoPte,
                                              &FlushList,
                                              OldIrql);
                
#if (_MI_PAGING_LEVELS >= 3)
        
                        //
                        // This must be recalculated here if MiDeletePte
                        // dropped the PFN lock (this can happen when
                        // dealing with POSIX forked pages).  Since the
                        // used PTE count is kept in the PFN entry
                        // which could have been paged out and replaced
                        // during this window, recomputation of its
                        // address (not the contents) is necessary.
                        //
        
                        if (Waited != 0) {
                            MI_CHECK_USED_PTES_HANDLE (Va);
                            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (Va);
                        }
#endif
        
                        InvalidPtes = 0;
                    }
                }
                else {
                    InvalidPtes += 1;
                    MI_WRITE_ZERO_PTE (PointerPte);
                }
            }
            else {
                InvalidPtes += 1;
            }
    
            if (InvalidPtes == 16) {
    
                if (PfnHeld == TRUE) {
    
                    if (FlushList.Count != 0) {
                        MiFlushPteList (&FlushList);
                    }
    
                    ASSERT (OldIrql != MM_NOIRQL);
                    UNLOCK_PFN (OldIrql);
                    PfnHeld = FALSE;
                }
                else {
                    ASSERT (FlushList.Count == 0);
                }
    
                InvalidPtes = 0;
            }
    
            Va += PAGE_SIZE;
            PointerPte += 1;
            ProtoPte += 1;
    
            ASSERT64 (PointerPpe->u.Hard.Valid == 1);
            ASSERT (PointerPde->u.Hard.Valid == 1);
    
            //
            // If not at the end of a page table and still within the specified
            // range, then just march directly on to the next PTE.
            //
    
        }
        while ((!MiIsVirtualAddressOnPdeBoundary(Va)) && (Va <= EndingAddress));

        //
        // The virtual address is on a page directory boundary:
        //
        // 1. Flush the PTEs for the previous page table page.
        // 2. Delete the previous page directory & page table if appropriate.
        // 3. Attempt to leap forward skipping over empty page directories
        //    and page tables where possible.
        //

        //
        // If all the entries have been eliminated from the previous
        // page table page, delete the page table page itself.
        //
    
        if (PfnHeld == TRUE) {
            if (FlushList.Count != 0) {
                MiFlushPteList (&FlushList);
            }
        }
        else {
            ASSERT (FlushList.Count == 0);
        }

        //
        // If all the entries have been eliminated from the previous
        // page table page, delete the page table page itself.
        //

        ASSERT64 (PointerPpe->u.Hard.Valid == 1);
        ASSERT (PointerPde->u.Hard.Valid == 1);

#if (_MI_PAGING_LEVELS >= 3)
        MI_CHECK_USED_PTES_HANDLE (PointerPte - 1);
#endif

        if ((MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) &&
            (PointerPde->u.Long != 0)) {

            if (PfnHeld == FALSE) {
                PfnHeld = TRUE;
                LOCK_PFN (OldIrql);
            }

#if (_MI_PAGING_LEVELS >= 3)
            UsedPageDirectoryHandle = MI_GET_USED_PTES_HANDLE (PointerPte - 1);
            MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryHandle);
#endif

            TempVa = MiGetVirtualAddressMappedByPte(PointerPde);
            MiDeletePte (PointerPde,
                         TempVa,
                         FALSE,
                         CurrentProcess,
                         NULL,
                         NULL,
                         OldIrql);

#if (_MI_PAGING_LEVELS >= 3)
            if ((MI_GET_USED_PTES_FROM_HANDLE (UsedPageDirectoryHandle) == 0) &&
                (PointerPpe->u.Long != 0)) {
    
#if (_MI_PAGING_LEVELS >= 4)
                UsedPageDirectoryHandle = MI_GET_USED_PTES_HANDLE (PointerPde);
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryHandle);
#endif

                TempVa = MiGetVirtualAddressMappedByPte(PointerPpe);
                MiDeletePte (PointerPpe,
                             TempVa,
                             FALSE,
                             CurrentProcess,
                             NULL,
                             NULL,
                             OldIrql);

#if (_MI_PAGING_LEVELS >= 4)
                if ((MI_GET_USED_PTES_FROM_HANDLE (UsedPageDirectoryHandle) == 0) &&
                    (PointerPxe->u.Long != 0)) {

                    TempVa = MiGetVirtualAddressMappedByPte(PointerPxe);
                    MiDeletePte (PointerPxe,
                                 TempVa,
                                 FALSE,
                                 CurrentProcess,
                                 NULL,
                                 NULL,
                                 OldIrql);
                }
#endif
    
            }
#endif
            ASSERT (OldIrql != MM_NOIRQL);
            UNLOCK_PFN (OldIrql);
            PfnHeld = FALSE;
        }

        if (PfnHeld == TRUE) {
            ASSERT (OldIrql != MM_NOIRQL);
            UNLOCK_PFN (OldIrql);
            PfnHeld = FALSE;
        }

        if (Va > EndingAddress) {
        
            //
            // All done, return.
            //
        
            return;
        }

        PointerPde = MiGetPdeAddress (Va);
        PointerPpe = MiGetPpeAddress (Va);
        PointerPxe = MiGetPxeAddress (Va);

        Skipped = FALSE;

    } while (TRUE);
}


ULONG
MiDeletePte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN ULONG AddressSpaceDeletion,
    IN PEPROCESS CurrentProcess,
    IN PMMPTE PrototypePte,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This routine deletes the contents of the specified PTE.  The PTE
    can be in one of the following states:

        - active and valid
        - transition
        - in paging file
        - in prototype PTE format

Arguments:

    PointerPte - Supplies a pointer to the PTE to delete.

    VirtualAddress - Supplies the virtual address which corresponds to
                     the PTE.  This is used to locate the working set entry
                     to eliminate it.

    AddressSpaceDeletion - Supplies TRUE if the address space is being
                           deleted, FALSE otherwise.  If TRUE is specified
                           the TB is not flushed and valid addresses are
                           not removed from the working set.


    CurrentProcess - Supplies a pointer to the current process.

    PrototypePte - Supplies a pointer to the prototype PTE which currently
                   or originally mapped this page.  This is used to determine
                   if the PTE is a fork PTE and should have its reference block
                   decremented.

    PteFlushList - Supplies a flush list to use if the TB flush can be
                   deferred to the caller.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at.

Return Value:

    Nonzero if this routine released mutexes and locks, FALSE if not.

Environment:

    Kernel mode, APCs disabled, PFN lock and working set mutex held.

--*/

{
    PMMPTE PointerPde;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    MMPTE PteContents;
    WSLE_NUMBER WsPfnIndex;
    PMMCLONE_BLOCK CloneBlock;
    PMMCLONE_DESCRIPTOR CloneDescriptor;
    ULONG DroppedLocks;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;

    MM_PFN_LOCK_ASSERT();

    DroppedLocks = 0;

    PteContents = *PointerPte;

    if (PteContents.u.Hard.Valid == 1) {

#ifdef _X86_
#if DBG
#if !defined(NT_UP)

        if (PteContents.u.Hard.Writable == 1) {
            ASSERT (PteContents.u.Hard.Dirty == 1);
        }
#endif //NTUP
#endif //DBG
#endif //X86

        //
        // PTE is valid.  Check PFN database to see if this is a prototype PTE.
        //

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE(&PteContents);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        WsPfnIndex = Pfn1->u1.WsIndex;

        CloneDescriptor = NULL;

        if (Pfn1->u3.e1.PrototypePte == 1) {

            CloneBlock = (PMMCLONE_BLOCK)Pfn1->PteAddress;

            //
            // Capture the state of the modified bit for this PTE.
            //

            MI_CAPTURE_DIRTY_BIT_TO_PFN (PointerPte, Pfn1);

            //
            // Decrement the share and valid counts of the page table
            // page which maps this PTE.
            //

            PointerPde = MiGetPteAddress (PointerPte);
            if (PointerPde->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
                if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
                    KeBugCheckEx (MEMORY_MANAGEMENT,
                                  0x61940, 
                                  (ULONG_PTR) PointerPte,
                                  (ULONG_PTR) PointerPde->u.Long,
                                  (ULONG_PTR) VirtualAddress);
#if (_MI_PAGING_LEVELS < 3)
                }
#endif
            }

            PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE(PointerPde);
            Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

            MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

            //
            // Decrement the share count for the physical page.
            //

            MiDecrementShareCountInline (Pfn1, PageFrameIndex);

            //
            // Check to see if this is a fork prototype PTE and if so
            // update the clone descriptor address.
            //

            if (PointerPte <= MiHighestUserPte) {

                if (PrototypePte != Pfn1->PteAddress) {

                    //
                    // Locate the clone descriptor within the clone tree.
                    //

                    CloneDescriptor = MiLocateCloneAddress (CurrentProcess, (PVOID)CloneBlock);

                    if (CloneDescriptor == NULL) {

                        if ((PAGE_ALIGN (VirtualAddress) == (PVOID) MM_SHARED_USER_DATA_VA) &&
                            (MmHighestUserAddress > (PVOID) MM_SHARED_USER_DATA_VA)) {

                            //
                            // The shared user data PTE is being deleted.  On
                            // platforms where this is located in the user
                            // portion of the address space (x86 /3GB or any
                            // NT64 machine), this is deleted directly during
                            // process teardown after all the VADs have already
                            // been eliminated.
                            //

                            NOTHING;
                        }
                        else {
                            KeBugCheckEx (MEMORY_MANAGEMENT,
                                          0x400, 
                                          (ULONG_PTR) PointerPte,
                                          (ULONG_PTR) PrototypePte,
                                            (ULONG_PTR) Pfn1->PteAddress);
                        }
                    }
                }
            }
        }
        else {

            if ((PMMPTE)((ULONG_PTR)Pfn1->PteAddress & ~0x1) != PointerPte) {
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x401, 
                              (ULONG_PTR) PointerPte,
                              (ULONG_PTR) PointerPte->u.Long,
                              (ULONG_PTR) Pfn1->PteAddress);
            }

            //
            // Initializing CloneBlock & PointerPde is not needed for
            // correctness but without it the compiler cannot compile this code
            // W4 to check for use of uninitialized variables.
            //

            CloneBlock = NULL;
            PointerPde = NULL;

            ASSERT (Pfn1->u2.ShareCount == 1);

            //
            // This PTE is NOT a prototype PTE, delete the physical page.
            //
            // Decrement the share and valid counts of the page table
            // page which maps this PTE.
            //

            MiDecrementShareCountInline (MI_PFN_ELEMENT(Pfn1->u4.PteFrame),
                                         Pfn1->u4.PteFrame);

            MI_SET_PFN_DELETED (Pfn1);

            //
            // Decrement the share count for the physical page.  As the page
            // is private it will be put on the free list.
            //

            MiDecrementShareCount (Pfn1, PageFrameIndex);

            //
            // Decrement the count for the number of private pages.
            //

            CurrentProcess->NumberOfPrivatePages -= 1;
        }

        //
        // Find the WSLE for this page and eliminate it.
        //
        // If we are deleting the system portion of the address space, do
        // not remove WSLEs or flush translation buffers as there can be
        // no other usage of this address space.
        //

        if (AddressSpaceDeletion == FALSE) {
            MiTerminateWsle (VirtualAddress, &CurrentProcess->Vm, WsPfnIndex);
        }

        MI_WRITE_ZERO_PTE (PointerPte);

        //
        // Flush the entry out of the TB.
        //

        if (!ARGUMENT_PRESENT (PteFlushList)) {
            MI_FLUSH_SINGLE_TB (VirtualAddress, FALSE);
        }
        else {
            if (PteFlushList->Count != MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList->FlushVa[PteFlushList->Count] = VirtualAddress;
                PteFlushList->Count += 1;
            }
        }

        if (AddressSpaceDeletion == FALSE) {

            if (CloneDescriptor != NULL) {

                //
                // Decrement the reference count for the clone block,
                // note that this could release and reacquire
                // the mutexes hence cannot be done until after the
                // working set entry has been removed.
                //

                if (MiDecrementCloneBlockReference (CloneDescriptor,
                                                    CloneBlock,
                                                    CurrentProcess,
                                                    PteFlushList,
                                                    OldIrql)) {

                    //
                    // The working set mutex was released, so the current
                    // current page directory & table page may have been
                    // paged out (note they cannot be deleted because the
                    // address space mutex is always held throughout).
                    //

                    DroppedLocks = 1;

                    //
                    // Ensure the PDE (and any table above it) are still
                    // resident.
                    //

                    MiMakePdeExistAndMakeValid (PointerPde,
                                                CurrentProcess,
                                                OldIrql);
                }
            }
        }
    }
    else if (PteContents.u.Soft.Prototype == 1) {

        //
        // This is a prototype PTE, if it is a fork PTE clean up the
        // fork structures.
        //

        if ((PteContents.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) &&
            (PointerPte <= MiHighestUserPte) &&
            (PrototypePte != MiPteToProto (PointerPte))) {

            CloneBlock = (PMMCLONE_BLOCK) MiPteToProto (PointerPte);
            CloneDescriptor = MiLocateCloneAddress (CurrentProcess,
                                                    (PVOID) CloneBlock);


            if (CloneDescriptor == NULL) {
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x403, 
                              (ULONG_PTR) PointerPte,
                              (ULONG_PTR) PrototypePte,
                              (ULONG_PTR) PteContents.u.Long);
            }

            //
            // Decrement the reference count for the clone block,
            // note that this could release and reacquire
            // the mutexes.
            //

            MI_WRITE_ZERO_PTE (PointerPte);

            if (MiDecrementCloneBlockReference (CloneDescriptor,
                                                CloneBlock,
                                                CurrentProcess,
                                                PteFlushList,
                                                OldIrql)) {

                //
                // The working set mutex was released, so the current
                // current page directory & table page may have been
                // paged out (note they cannot be deleted because the
                // address space mutex is always held throughout).
                //

                DroppedLocks = 1;

                //
                // Ensure the PDE (and any table above it) are still
                // resident.
                //

                PointerPde = MiGetPteAddress (PointerPte);

                MiMakePdeExistAndMakeValid (PointerPde,
                                            CurrentProcess,
                                            OldIrql);
            }
        }
        else {
            MI_WRITE_ZERO_PTE (PointerPte);
        }
    }
    else if (PteContents.u.Soft.Transition == 1) {

        //
        // This is a transition PTE. (Page is private)
        //

        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

        if ((PMMPTE)((ULONG_PTR)Pfn1->PteAddress & ~0x1) != PointerPte) {
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x402, 
                          (ULONG_PTR) PointerPte,
                          (ULONG_PTR) PointerPte->u.Long,
                          (ULONG_PTR) Pfn1->PteAddress);
        }

        MI_SET_PFN_DELETED (Pfn1);

        PageTableFrameIndex = Pfn1->u4.PteFrame;
        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

        //
        // Check the reference count for the page, if the reference
        // count is zero, move the page to the free list, if the reference
        // count is not zero, ignore this page.  When the reference count
        // goes to zero, it will be placed on the free list.
        //

        if (Pfn1->u3.e2.ReferenceCount == 0) {
            MiUnlinkPageFromList (Pfn1);
            MiReleasePageFileSpace (Pfn1->OriginalPte);
            MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&PteContents));
        }

        //
        // Decrement the count for the number of private pages.
        //

        CurrentProcess->NumberOfPrivatePages -= 1;

        MI_WRITE_ZERO_PTE (PointerPte);
    }
    else {

        //
        // Must be page file space.
        //

        if (PteContents.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) {

            if (MiReleasePageFileSpace (PteContents)) {

                //
                // Decrement the count for the number of private pages.
                //

                CurrentProcess->NumberOfPrivatePages -= 1;
            }
        }

        MI_WRITE_ZERO_PTE (PointerPte);
    }

    return DroppedLocks;
}


VOID
MiDeleteValidSystemPte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN PMMSUPPORT WsInfo,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL
    )

/*++

Routine Description:

    This routine deletes the contents of the specified valid system or
    session PTE.  The PTE *MUST* be valid and private (ie: not a prototype).

Arguments:

    PointerPte - Supplies a pointer to the PTE to delete.

    VirtualAddress - Supplies the virtual address which corresponds to
                     the PTE.  This is used to locate the working set entry
                     to eliminate it.

    WsInfo - Supplies the working set structure to update.

    PteFlushList - Supplies a flush list to use if the TB flush can be
                   deferred to the caller.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, PFN lock and working set mutex held.

--*/

{
    PMMPFN Pfn1;
    MMPTE PteContents;
    WSLE_NUMBER WsPfnIndex;
    PFN_NUMBER PageFrameIndex;

    MM_PFN_LOCK_ASSERT();

    PteContents = *PointerPte;

    ASSERT (PteContents.u.Hard.Valid == 1);

#ifdef _X86_
#if DBG
#if !defined(NT_UP)

    if (PteContents.u.Hard.Writable == 1) {
        ASSERT (PteContents.u.Hard.Dirty == 1);
    }
#endif //NTUP
#endif //DBG
#endif //X86

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE(&PteContents);
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    WsPfnIndex = Pfn1->u1.WsIndex;

    ASSERT (Pfn1->u3.e1.PrototypePte == 0);

    if ((PMMPTE)((ULONG_PTR)Pfn1->PteAddress & ~0x1) != PointerPte) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x401, 
                      (ULONG_PTR) PointerPte,
                      (ULONG_PTR) PointerPte->u.Long,
                      (ULONG_PTR) Pfn1->PteAddress);
    }

    ASSERT (Pfn1->u2.ShareCount == 1);

    //
    // This PTE is NOT a prototype PTE, delete the physical page.
    //
    // Decrement the share and valid counts of the page table
    // page which maps this PTE.
    //

    MiDecrementShareCountInline (MI_PFN_ELEMENT(Pfn1->u4.PteFrame),
                                 Pfn1->u4.PteFrame);

    MI_SET_PFN_DELETED (Pfn1);

    //
    // Decrement the share count for the physical page.  As the page
    // is private it will be put on the free list.
    //

    MiDecrementShareCount (Pfn1, PageFrameIndex);

    //
    // Find the WSLE for this page and eliminate it.
    //
    // If we are deleting the system portion of the address space, do
    // not remove WSLEs or flush translation buffers as there can be
    // no other usage of this address space.
    //

    MiTerminateWsle (VirtualAddress, WsInfo, WsPfnIndex);

    //
    // Zero the PTE contents.
    //

    MI_WRITE_ZERO_PTE (PointerPte);

    //
    // Flush the entry out of the TB.
    //

    if (!ARGUMENT_PRESENT (PteFlushList)) {
        MI_FLUSH_SINGLE_TB (VirtualAddress, TRUE);
    }
    else {
        if (PteFlushList->Count != MM_MAXIMUM_FLUSH_COUNT) {
            PteFlushList->FlushVa[PteFlushList->Count] = VirtualAddress;
            PteFlushList->Count += 1;
        }
    }

    if (WsInfo->Flags.SessionSpace == 1) {
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_HASH_SHRINK, 1);
        InterlockedDecrementSizeT (&MmSessionSpace->NonPageablePages);
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_WS_HASHPAGE_FREE, 1);
        InterlockedDecrementSizeT (&MmSessionSpace->CommittedPages);
    }

    return;
}


ULONG
FASTCALL
MiReleasePageFileSpace (
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

    if (PteContents.u.Soft.Prototype == 1) {

        //
        // Not in page file format.
        //

        return FALSE;
    }

    FreeBit = GET_PAGING_FILE_OFFSET (PteContents);

    if ((FreeBit == 0) || (FreeBit == MI_PTE_LOOKUP_NEEDED)) {

        //
        // Page is not in a paging file, just return.
        //

        return FALSE;
    }

    PageFileNumber = GET_PAGING_FILE_NUMBER (PteContents);

    ASSERT (RtlCheckBit( MmPagingFile[PageFileNumber]->Bitmap, FreeBit) == 1);

#if DBG
    if ((FreeBit < 8192) && (PageFileNumber == 0)) {
        ASSERT ((MmPagingFileDebug[FreeBit] & 1) != 0);
        MmPagingFileDebug[FreeBit] ^= 1;
    }
#endif

    PageFile = MmPagingFile[PageFileNumber];
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

    return TRUE;
}


VOID
FASTCALL
MiUpdateModifiedWriterMdls (
    IN ULONG PageFileNumber
    )

/*++

Routine Description:

    This routine ensures the MDLs for the specified paging file
    are in the proper state such that paging i/o can continue.

Arguments:

    PageFileNumber - Supplies the page file number to check the MDLs for.

Return Value:

    None.

Environment:

    Kernel mode, PFN lock held.

--*/

{
    ULONG i;
    PMMMOD_WRITER_MDL_ENTRY WriterEntry;

    //
    // Put the MDL entries into the active list.
    //

    for (i = 0; i < MM_PAGING_FILE_MDLS; i += 1) {

        if (MmPagingFile[PageFileNumber]->Entry[i]->CurrentList ==
            &MmFreePagingSpaceLow) {

            ASSERT (MmPagingFile[PageFileNumber]->Entry[i]->Links.Flink !=
                                                    MM_IO_IN_PROGRESS);

            //
            // Remove this entry and put it on the active list.
            //

            WriterEntry = MmPagingFile[PageFileNumber]->Entry[i];
            RemoveEntryList (&WriterEntry->Links);
            WriterEntry->CurrentList = &MmPagingFileHeader.ListHead;

            KeSetEvent (&WriterEntry->PagingListHead->Event, 0, FALSE);

            InsertTailList (&WriterEntry->PagingListHead->ListHead,
                            &WriterEntry->Links);
            MmNumberOfActiveMdlEntries += 1;
        }
    }

    return;
}

