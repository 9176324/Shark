/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.

Module Name:

   pagfault.c

Abstract:

    This module contains the pager for memory management.

--*/

#include "mi.h"

#define STATUS_PTE_CHANGED      0x87303000
#define STATUS_REFAULT          0xC7303001

ULONG MmInPageSupportMinimum = 4;

ULONG MiInPageSinglePages;

extern PMMPTE MmSharedUserDataPte;

extern PVOID MmSpecialPoolStart;
extern PVOID MmSpecialPoolEnd;

ULONG MiFaultRetries;
ULONG MiUserFaultRetries;

ULONG MmClusterPageFileReads;

#define MI_PROTOTYPE_WSINDEX    ((ULONG)-1)

NTSTATUS
MiResolvePageFileFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    OUT PMMPTE CapturedPteContents,
    IN PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    );

NTSTATUS
MiResolveProtoPteFault (
    IN ULONG_PTR StoreInstruction,
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN OUT PMMPFN *LockedProtoPfn,
    IN PMMINPAGE_SUPPORT *ReadBlock,
    OUT PMMPTE CapturedPteContents,
    IN PEPROCESS Process,
    IN KIRQL OldIrql,
    IN PVOID TrapInformation
    );

VOID
MiHandleBankedSection (
    IN PVOID VirtualAddress,
    IN PMMVAD Vad
    );

NTSTATUS
MiResolveMappedFileFault (
    IN PMMPTE PointerPte,
    IN PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    );

NTSTATUS
MiResolveTransitionFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN KIRQL OldIrql,
    OUT PMMINPAGE_SUPPORT *InPageBlock
    );

NTSTATUS
MiCompleteProtoPteFault (
    IN ULONG_PTR StoreInstruction,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN KIRQL OldIrql,
    IN OUT PMMPFN *LockedProtoPfn
    );

ULONG MmMaxTransitionCluster = 8;


NTSTATUS
MiDispatchFault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN LOGICAL RecheckAccess,
    IN PEPROCESS Process,
    IN PVOID TrapInformation,
    IN PMMVAD Vad
    )

/*++

Routine Description:

    This routine dispatches a page fault to the appropriate
    routine to complete the fault.

Arguments:

    FaultStatus - Supplies fault status information bits.

    VirtualAddress - Supplies the faulting address.

    PointerPte - Supplies the PTE for the faulting address.

    PointerProtoPte - Supplies a pointer to the prototype PTE to fault in,
                      NULL if no prototype PTE exists.

    RecheckAccess - Supplies TRUE if the prototype PTE needs to be checked for
                    access permissions - this is only an issue for forked
                    prototype PTEs that are no-access.

    Process - Supplies a pointer to the process object.  If this
              parameter is NULL, then the fault is for system
              space and the process's working set lock is not held.
              If this parameter is HYDRA_PROCESS, then the fault is for session
              space and the process's working set lock is not held - rather
              the session space's working set lock is held.

    Vad - Supplies the VAD used for sections.  May optionally be NULL even
          for section-based faults, so use purely as an opportunistic hint.

    TrapInformation - Supplies the trap information.

Return Value:

    NTSTATUS.

Environment:

    Kernel mode, working set mutex held.

--*/

{
#if DBG
    KIRQL EntryIrql;
    MMWSLE ProtoProtect2;
#endif

#define VARIOUS_FLAGS_DIRTY_PTE                 0x01
#define VARIOUS_FLAGS_PFN_HELD                  0x02
#define VARIOUS_FLAGS_ACCESS_CHECK_NEEDED       0x04
#define VARIOUS_FLAGS_LOG_HARD_FAULT            0x08
#define VARIOUS_FLAGS_ENTERED_CRITICAL_REGION   0x10

    //
    // Miscellaneous unrelated flags kept here in one ULONG to save stack space.
    //

    ULONG VariousFlags;
    MMPTE OriginalPte;
    MMPTE TempPte;
    MMPTE RealPteContents;
    MMPTE NewPteContents;
    MM_PROTECTION_MASK Protection;
    ULONG FreeBit;
    MMWSLE ProtoProtect;
    PFILE_OBJECT FileObject;
    LONGLONG FileOffset;
    PSUBSECTION Subsection;
    ULONG Flags;
    PVOID UsedPageTableHandle;
    ULONG_PTR i;
    ULONG_PTR NumberOfProtos;
    ULONG_PTR MaxProtos;
    ULONG_PTR ProtosProcessed;
    NTSTATUS status;
    PMMINPAGE_SUPPORT ReadBlock;
    PMMINPAGE_SUPPORT CapturedEvent;
    KIRQL OldIrql;
    PPFN_NUMBER Page;
    PFN_NUMBER PageFrameIndex;
    LONG NumberOfBytes;
    PMMPTE CheckPte;
    PMMPTE ReadPte;
    PMMPFN PfnClusterPage;
    PMMPFN Pfn1;
    PMMSUPPORT SessionWs;
    PETHREAD WsThread;
    PERFINFO_HARDPAGEFAULT_INFORMATION HardFaultEvent;
    LARGE_INTEGER IoStartTime;
    ULONG_PTR StoreInstruction;
    PMMPFN LockedProtoPfn;
    WSLE_NUMBER WorkingSetIndex;
    PMMPFN Pfn2;
    PMMPTE ContainingPageTablePointer;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;
    PCONTROL_AREA ControlArea;
    PMMCLONE_DESCRIPTOR CloneDescriptor;

#if DBG
    EntryIrql = KeGetCurrentIrql ();
    ASSERT (EntryIrql <= APC_LEVEL);
    ASSERT (KeAreAllApcsDisabled () == TRUE);
    SATISFY_OVERZEALOUS_COMPILER (OriginalPte.u.Long = (ULONG_PTR)-1);
#endif

    VariousFlags = 0;
    LockedProtoPfn = NULL;
    SessionWs = NULL;
    StoreInstruction = MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus);
    WsThread = PsGetCurrentThread ();

    //
    // Initializing ReadBlock & ReadPte is not needed for correctness, but
    // without it the compiler cannot compile this code W4 to check for use of
    // uninitialized variables.
    //

    OldIrql = MM_NOIRQL;
    ReadPte = NULL;
    ReadBlock = NULL;
    ProtoProtect.u1.Long = 0;

    if (PointerProtoPte != NULL) {

        ASSERT (!MI_IS_PHYSICAL_ADDRESS(PointerProtoPte));

        CheckPte = MiGetPteAddress (PointerProtoPte);

        if (VirtualAddress < MmSystemRangeStart) {

            NumberOfProtos = 1;
            SATISFY_OVERZEALOUS_COMPILER (UsedPageTableHandle = NULL);
            SATISFY_OVERZEALOUS_COMPILER (Pfn2 = NULL);
            SATISFY_OVERZEALOUS_COMPILER (Protection = 0);
            SATISFY_OVERZEALOUS_COMPILER (RealPteContents.u.Long = 0);

            if ((PointerPte->u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) &&
                (PointerPte->u.Proto.ReadOnly == 0)) {

                //
                // Kernel mode access must be verified, go the long way below.
                //

                VariousFlags |= VARIOUS_FLAGS_ACCESS_CHECK_NEEDED;
            }
            else {

                //
                // Opportunistically cluster the transition faults as needed.
                // When the Vad is non-NULL, the proper access checks have
                // already been applied across the range (as long as the
                // PTEs are zero).
                //
    
                if ((Vad != NULL) &&
                    (Vad->u.VadFlags.VadType != VadImageMap) &&
                    (Vad->u2.VadFlags2.ExtendableFile == 0) &&
                    (MmAvailablePages > MM_ENORMOUS_LIMIT) &&
                    ((Process->Vm.Flags.MaximumWorkingSetHard == 0) ||
                     (Process->Vm.WorkingSetSize + MmMaxTransitionCluster <= Process->Vm.MaximumWorkingSetSize)) &&
                    (RecheckAccess == FALSE)) {
    
                    NumberOfProtos = MmMaxTransitionCluster;
    
                    //
                    // Ensure the cluster doesn't cross the VAD contiguous PTE
                    // limits.
                    //

                    MaxProtos = Vad->LastContiguousPte - PointerProtoPte + 1;
    
                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }
    
                    //
                    // Ensure the cluster doesn't cross the page containing
                    // the real page table page as we only locked down the
                    // single page.
                    //

                    MaxProtos = (PAGE_SIZE - BYTE_OFFSET (PointerPte)) / sizeof (MMPTE);
                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }
    
                    //
                    // Ensure the cluster doesn't cross the page containing
                    // the prototype PTEs as we only locked down the single
                    // page.
                    //

                    MaxProtos = (PAGE_SIZE - BYTE_OFFSET (PointerProtoPte)) / sizeof (MMPTE);
                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }
    
                    //
                    // Ensure the cluster doesn't cross the VAD limits.
                    //

                    MaxProtos = Vad->EndingVpn - MI_VA_TO_VPN (VirtualAddress) + 1;
    
                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }
    
                    //
                    // Ensure enough WSLEs are available so we cannot fail to
                    // insert the cluster later.
                    //

                    MaxProtos = 1;
                    WorkingSetIndex = MmWorkingSetList->FirstFree;

                    if ((NumberOfProtos > 1) &&
                        (WorkingSetIndex != WSLE_NULL_INDEX)) {

                        do {
                            if (MmWsle[WorkingSetIndex].u1.Long == (WSLE_NULL_INDEX << MM_FREE_WSLE_SHIFT)) {
                                break;
                            }
                            MaxProtos += 1;
                            WorkingSetIndex = (WSLE_NUMBER) (MmWsle[WorkingSetIndex].u1.Long >> MM_FREE_WSLE_SHIFT);
                        } while (MaxProtos < NumberOfProtos);
                    }

                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }

                    //
                    // We have computed the maximum cluster size.  Fill the PTEs
                    // and increment the use counts on the page table page for
                    // each PTE we fill (regardless of whether the prototype
                    // cluster pages are already in transition).
                    //

                    ASSERT (VirtualAddress <= MM_HIGHEST_USER_ADDRESS);

                    for (i = 1; i < NumberOfProtos; i += 1) {
                        if ((PointerPte + i)->u.Long != MM_ZERO_PTE) {
                            break;
                        }
                        MI_WRITE_INVALID_PTE (PointerPte + i, *PointerPte);
                    }

                    NumberOfProtos = i;

                    if (NumberOfProtos > 1) {
                        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (VirtualAddress);
                        MI_INCREMENT_USED_PTES_BY_HANDLE_CLUSTER (UsedPageTableHandle, NumberOfProtos - 1);

                        //
                        // The protection code for the real PTE comes from
                        // the real PTE as it was placed there earlier during
                        // the handling of this fault.
                        //

                        ProtoProtect.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE (PointerPte);
                        //
                        // Build up a valid PTE so that when the PFN lock is
                        // held, the only additional update to it is for the
                        // actual PFN.
                        //
    
                        MI_MAKE_VALID_USER_PTE (RealPteContents,
                                                0,
                                                ProtoProtect.u1.e1.Protection,
                                                PointerPte);

                        if ((StoreInstruction != 0) &&
                            ((ProtoProtect.u1.e1.Protection & MM_COPY_ON_WRITE_MASK) != MM_COPY_ON_WRITE_MASK)) {
                            MI_SET_PTE_DIRTY (RealPteContents);
                            VariousFlags |= VARIOUS_FLAGS_DIRTY_PTE;
                        }

                        ContainingPageTablePointer = MiGetPteAddress (PointerPte);
                        Pfn2 = MI_PFN_ELEMENT (ContainingPageTablePointer->u.Hard.PageFrameNumber);
                    }
                }
            }

            ProtosProcessed = 0;

            //
            // Acquire the PFN lock to synchronize access to prototype PTEs.
            // This is required as the working set mutex does not prevent
            // multiple processes from operating on the same prototype PTE.
            //

            VariousFlags |= VARIOUS_FLAGS_PFN_HELD;
            LOCK_PFN (OldIrql);

            if (CheckPte->u.Hard.Valid == 0) {

                //
                // The working set pushlock must be released here because this
                // process' working set may be large, but the number of
                // available pages may be low.  In this case, this thread
                // could end up needing a page to read the prototype PTE
                // into and would thus wait in MiEnsureAvailablePageOrWait,
                // but since this would be as part of handling the a "new"
                // fault, it would not release the process working set.
                //
                // Increment the number of mapped views so the control area
                // (and the prototype PTEs) cannot be immediately deleted since
                // the working set pushlock is going to be released and
                // reacquired.
                //

                CloneDescriptor = NULL;
                ControlArea = NULL;

                if ((Vad == NULL) &&
                    ((PAGE_ALIGN (VirtualAddress) != (PVOID) MM_SHARED_USER_DATA_VA))) {
                    Vad = MiLocateAddress (VirtualAddress);
                    ASSERT (Vad != NULL);
                }

                if (Vad != NULL) {
                    if (Vad->u.VadFlags.PrivateMemory == 0) {
                        ControlArea = Vad->ControlArea;
                        ASSERT (ControlArea->NumberOfMappedViews >= 1);
                        ControlArea->NumberOfMappedViews += 1;
                    }
                    else {

                        //
                        // Forked private VADs are prototype PTE-backed but
                        // have no control areas.  Reference their clone
                        // descriptor instead.
                        //

                        ASSERT (Process->CloneRoot != NULL);
                        CloneDescriptor = MiLocateCloneAddress (Process,
                                                                PointerProtoPte);

                        ASSERT (CloneDescriptor != NULL);
                        ASSERT (CloneDescriptor->NumberOfReferences >= 1);
                        CloneDescriptor->NumberOfReferences += 1;
                        CloneDescriptor->FinalNumberOfReferences += 1;
                    }
                }

                UNLOCK_PFN (OldIrql);
                UNLOCK_WS (WsThread, Process);

                MmAccessFault (FALSE, PointerProtoPte, KernelMode, NULL);

                //
                // Note that the VAD may no longer exist at this point !
                //

                if (ControlArea != NULL) {

                    LOCK_PFN (OldIrql);
    
                    ASSERT (ControlArea->NumberOfMappedViews >= 1);
                    ControlArea->NumberOfMappedViews -= 1;

                    //
                    // See if the control area should be deleted - this
                    // returns with the PFN lock released.
                    //

                    MiCheckControlArea (ControlArea, OldIrql);
                }
                else if (CloneDescriptor != NULL) {

                    LOCK_WS (WsThread, Process);
                    LOCK_PFN (OldIrql);

                    MiDecrementCloneBlockReference (CloneDescriptor,
                                                    NULL,
                                                    Process,
                                                    NULL,
                                                    OldIrql);
                    UNLOCK_PFN (OldIrql);

                    ASSERT (EntryIrql == KeGetCurrentIrql ());
                    ASSERT (KeAreAllApcsDisabled () == TRUE);

                    return STATUS_SUCCESS;
                }

                LOCK_WS (WsThread, Process);

                ASSERT (EntryIrql == KeGetCurrentIrql ());
                ASSERT (KeAreAllApcsDisabled () == TRUE);

                //
                // Reissue the faulting instruction since while the working set
                // pushlock was released, the address space may have changed.
                //

                return STATUS_SUCCESS;
            }

            TempPte = *PointerProtoPte;

            if (RecheckAccess == TRUE) {

                //
                // This is a forked process so shared prototype PTEs
                // may actually be fork clone prototypes.  These have
                // the protection within the fork clone yet the
                // hardware PTEs always share it.  This must be
                // checked here for the case where the NO_ACCESS
                // permission has been put into the fork clone because
                // it would not necessarily be in the hardware PTEs like
                // it is for normal prototypes.
                //
                // First make sure the proto is in transition or paged
                // out as only these states can be no access.
                //

                if ((TempPte.u.Hard.Valid == 0) &&
                    (TempPte.u.Soft.Prototype == 0)) {

                    ProtoProtect.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE (&TempPte);
                    if (ProtoProtect.u1.e1.Protection == MM_NOACCESS) {
                        ASSERT (MiLocateCloneAddress (Process, PointerProtoPte) != NULL);
                        UNLOCK_PFN (OldIrql);
                        return STATUS_ACCESS_VIOLATION;
                    }
                }
            }

            //
            // If the fault can be handled inline (prototype transition or
            // valid for example), then process it here (eliminating
            // locked page charges, etc) to reduce PFN hold times.
            //

            if ((VariousFlags & VARIOUS_FLAGS_ACCESS_CHECK_NEEDED) == 0) {

                while (TRUE) {
    
                    if (TempPte.u.Hard.Valid == 1) {

                        //
                        // Prototype PTE is valid.
                        //

                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
                        Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
                        Pfn1->u2.ShareCount += 1;
    
                    }
                    else if ((TempPte.u.Soft.Prototype == 0) &&
                             (TempPte.u.Soft.Transition == 1)) {
    
                        //
                        // This is a fault on a PTE which ultimately
                        // decodes to a prototype PTE referencing a
                        // page already in the cache.
                        //
                        // Optimize this path as every cycle counts.
                        //
        
                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&TempPte);
                        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        
                        ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);
        
                        if ((Pfn1->u3.e1.ReadInProgress == 1) ||
                            (Pfn1->u4.InPageError == 1) ||
                            (MmAvailablePages < MM_HIGH_LIMIT)) {
        
                            break;
                        }
        
                        MiUnlinkPageFromList (Pfn1);
        
                        //
                        // Update the PFN database - the reference count
                        // must be incremented as the share count is
                        // going to go from zero to 1.
                        //
        
                        ASSERT (Pfn1->u2.ShareCount == 0);
        
                        //
                        // The PFN reference count will be 1 already
                        // here if the modified writer has begun a write
                        // of this page.  Otherwise it's ordinarily 0.
                        //
                        // Note there is no need to apply locked page
                        // charges for this page as we know that the share
                        // count is zero (and the reference count is
                        // unknown).  But since we are incrementing both
                        // the share and reference counts by one, the
                        // page will retain its current locked charge
                        // regardless of whether or not it is currently
                        // set.
                        //
        
                        InterlockedIncrementPfn ((PSHORT)&Pfn1->u3.e2.ReferenceCount);
        
                        //
                        // Update the transition PTE.
                        //
        
                        Pfn1->u2.ShareCount += 1;
                        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        
                        MI_MAKE_TRANSITION_PROTOPTE_VALID (TempPte,
                                                           PointerProtoPte);
        
                        //
                        // If the modified field is set in the PFN database
                        // and this page is not copy on write, then set
                        // the dirty bit.  This can be done as the modified
                        // page will not be written to the paging file
                        // until this PTE is made invalid.
                        //
        
                        if ((Pfn1->u3.e1.Modified) &&
                            (TempPte.u.Hard.Write) &&
                            (TempPte.u.Hard.CopyOnWrite == 0)) {
        
                            MI_SET_PTE_DIRTY (TempPte);
                        }
                        else {
                            MI_SET_PTE_CLEAN (TempPte);
                        }
        
                        MI_WRITE_VALID_PTE (PointerProtoPte, TempPte);
        
                        ASSERT (PointerPte->u.Hard.Valid == 0);
                    }
                    else {
                        break;
                    }
    
                    ProtosProcessed += 1;
    
                    if (ProtosProcessed == NumberOfProtos) {
    
                        //
                        // This is the last (or only) PFN so use
                        // MiCompleteProtoPteFault so the PFN lock is released 
                        // as quickly as possible.
                        //
    
                        MiCompleteProtoPteFault (StoreInstruction,
                                                 VirtualAddress,
                                                 PointerPte,
                                                 PointerProtoPte,
                                                 OldIrql,
                                                 &LockedProtoPfn);
    
                        VariousFlags &= ~VARIOUS_FLAGS_PFN_HELD;
                        break;
                    }
    
                    //
                    // Just finish the PFN work here but not the working
                    // set or prefetcher actions as the PFN lock is held
                    // and we want to minimize PFN hold time.
                    //
    
                    ASSERT (PointerProtoPte->u.Hard.Valid == 1);

                    Pfn1->u3.e1.PrototypePte = 1;

                    //
                    // Prototype PTE is now valid, make the PTE valid.
                    //
                    // A PTE just went from not present, not transition to
                    // present.  The share count and valid count must be
                    // updated in the page table page which contains this PTE.
                    //

                    Pfn2->u2.ShareCount += 1;

                    //
                    // Ensure the user's attributes do not conflict with
                    // the PFN attributes.
                    //

                    NewPteContents.u.Long = RealPteContents.u.Long;
                    ASSERT (NewPteContents.u.Long != 0);

                    if (Pfn1->u3.e1.CacheAttribute == MiCached) {
                        NOTHING;
                    }
                    else if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {

                        Protection = (MM_PROTECTION_MASK) ProtoProtect.u1.e1.Protection;
                        Protection &= ~(MM_NOCACHE | MM_WRITECOMBINE);

                        Protection |= MM_NOCACHE;
                        NewPteContents.u.Long = 0;
                    }
                    else if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
                        Protection = (MM_PROTECTION_MASK) ProtoProtect.u1.e1.Protection;
                        Protection &= ~(MM_NOCACHE | MM_WRITECOMBINE);
                        Protection |= MM_WRITECOMBINE;
                        NewPteContents.u.Long = 0;
                    }

                    if (NewPteContents.u.Long == 0) {
                        MI_MAKE_VALID_USER_PTE (NewPteContents,
                                                0,
                                                Protection,
                                                PointerPte);

                        if ((StoreInstruction != 0) &&
                            ((Protection & MM_COPY_ON_WRITE_MASK) != MM_COPY_ON_WRITE_MASK)) {
                            MI_SET_PTE_DIRTY (NewPteContents);
                        }
                    }

                    NewPteContents.u.Hard.PageFrameNumber = PageFrameIndex;

#if DBG

                    //
                    // The protection code for the real PTE comes from
                    // the real PTE as it was placed there above.
                    //

                    ProtoProtect2.u1.Long = 0;
                    ASSERT (PointerPte->u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED);
                    ProtoProtect2.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE(PointerPte);

                    MI_MAKE_VALID_PTE (OriginalPte,
                                       PageFrameIndex,
                                       ProtoProtect2.u1.e1.Protection,
                                       PointerPte);

                    if ((StoreInstruction != 0) &&
                        ((ProtoProtect2.u1.e1.Protection & MM_COPY_ON_WRITE_MASK) != MM_COPY_ON_WRITE_MASK)) {
                        MI_SET_PTE_DIRTY (OriginalPte);
                    }

                    {
                        MMPTE RealPte2;

                        RealPte2.u.Long = RealPteContents.u.Long;
                        RealPte2.u.Hard.PageFrameNumber = PageFrameIndex;
                        ASSERT (OriginalPte.u.Long == RealPte2.u.Long);
                    }
#endif

                    MI_SNAP_DATA (Pfn1, PointerProtoPte, 6);

                    //
                    // If this is a store instruction and the page is not
                    // copy on write, then set the modified bit in the PFN
                    // database and the dirty bit in the PTE.  The PTE is
                    // not set dirty even if the modified bit is set so
                    // writes to the page can be tracked for FlushVirtualMemory.
                    //

                    if (VariousFlags & VARIOUS_FLAGS_DIRTY_PTE) {

                        OriginalPte = Pfn1->OriginalPte;

#if DBG
                        if (OriginalPte.u.Soft.Prototype == 1) {

                            PCONTROL_AREA ControlArea;

                            Subsection = MiGetSubsectionAddress (&OriginalPte);
                            ControlArea = Subsection->ControlArea;

                            if (ControlArea->DereferenceList.Flink != NULL) {
                                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                                    "MM: page fault completing to dereferenced CA %p %p %p\n",
                                                ControlArea, Pfn1, PointerPte);
                                DbgBreakPoint ();
                            }
                        }
#endif

                        MI_SET_MODIFIED (Pfn1, 1, 0xA);

                        if ((OriginalPte.u.Soft.Prototype == 0) &&
                            (Pfn1->u3.e1.WriteInProgress == 0)) {

                            FreeBit = GET_PAGING_FILE_OFFSET (OriginalPte);

                            if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                                MiReleaseConfirmedPageFileSpace (OriginalPte);
                            }

                            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                        }
                    }

                    ASSERT (PointerPte == MiGetPteAddress (VirtualAddress));

                    MI_WRITE_VALID_PTE (PointerPte, NewPteContents);

                    PointerProtoPte += 1;
                    TempPte = *PointerProtoPte;
                    PointerPte += 1;
                    VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress + PAGE_SIZE);
                }
            }

            if (ProtosProcessed != 0) {

                //
                // At least the first VA was handled and that was the one
                // that caused the fault so just return now as any other
                // VAs were purely optional.
                //

                if (VariousFlags & VARIOUS_FLAGS_PFN_HELD) {

                    //
                    // The last speculative VA was not made valid and the PFN
                    // lock is still held.  Release the PFN lock now.
                    //

                    UNLOCK_PFN (OldIrql);
                    InterlockedExchangeAdd (&KeGetCurrentPrcb ()->MmTransitionCount,
                                            (LONG) ProtosProcessed);
                }
                else {

                    //
                    // The last speculative VA was made valid and was also
                    // inserted into the working set list.  Subtract one from
                    // the count of protos that need working set insertions
                    // below.
                    //

                    InterlockedExchangeAdd (&KeGetCurrentPrcb ()->MmTransitionCount,
                                            (LONG) ProtosProcessed);

                    ProtosProcessed -= 1;
                }

                //
                // Back the locals up to the last "made-valid" VA where
                // the working set insertions need to begin.
                //
                // Add working set entries for the cluster of addresses.
                //
                // Note because we checked the WSLE list above prior
                // to clustering (and the working set mutex has never been
                // released), we are guaranteed that the working set list
                // insertions below cannot fail.
                //

                Subsection = NULL;
                SATISFY_OVERZEALOUS_COMPILER (FileObject = NULL);
                SATISFY_OVERZEALOUS_COMPILER (FileOffset = 0);
                SATISFY_OVERZEALOUS_COMPILER (Flags = 0);

                while (ProtosProcessed != 0) {

                    PointerProtoPte -= 1;
                    PointerPte -= 1;
                    VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress - PAGE_SIZE);
                    ProtosProcessed -= 1;

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                    ASSERT (ProtoProtect.u1.e1.Protection != MM_ZERO_ACCESS);

                    ASSERT (MI_IS_PAGE_TABLE_ADDRESS(PointerPte));
                    ASSERT (PointerPte->u.Hard.Valid == 1);

                    WorkingSetIndex = MiAllocateWsle (&Process->Vm,
                                                      PointerPte,
                                                      Pfn1,
                                                      ProtoProtect.u1.Long);

                    ASSERT (WorkingSetIndex != 0);

                    //
                    // Log prefetch fault information.
                    //
                    // Note that the process' working set mutex is still
                    // held so any other faults or operations on user
                    // addresses by other threads in this process
                    // will block for the duration of this call.
                    //

                    if ((Subsection == NULL) &&
                        (CCPF_IS_PREFETCHER_ACTIVE()) &&
                        (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

                        Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);

                        FileObject = Subsection->ControlArea->FilePointer;
                        FileOffset = MiStartingOffset (Subsection, PointerProtoPte);

                        Flags = 0;

                        //
                        // Image pages are not speculatively transition
                        // clustered so this must be a data page we are telling
                        // the prefetcher about.
                        //

                        ASSERT (Subsection->ControlArea->u.Flags.Image == 0);

                        if (Subsection->ControlArea->u.Flags.Rom) {
                            Flags |= CCPF_TYPE_ROM;
                        }
                    }

                    if (Subsection != NULL) {
                        CcPfLogPageFault (FileObject, FileOffset, Flags);
                    }
                }

                ASSERT (EntryIrql == KeGetCurrentIrql ());
                ASSERT (EntryIrql <= APC_LEVEL);
                ASSERT (KeAreAllApcsDisabled () == TRUE);

                return STATUS_PAGE_FAULT_TRANSITION;
            }

            ASSERT (VariousFlags & VARIOUS_FLAGS_PFN_HELD);

            LockedProtoPfn = MI_PFN_ELEMENT (CheckPte->u.Hard.PageFrameNumber);
            MI_ADD_LOCKED_PAGE_CHARGE (LockedProtoPfn);
            ASSERT (LockedProtoPfn->u3.e2.ReferenceCount > 1);

            ASSERT (PointerPte->u.Hard.Valid == 0);
        }
        else {
            LOCK_PFN (OldIrql);

            if (CheckPte->u.Hard.Valid == 0) {

                //
                // Make sure the prototype PTEs are in memory.  If not, since
                // this is a system address, just convert the fault as though
                // it happened on the prototype PTE instead.
                //

                ASSERT ((Process == NULL) || (Process == HYDRA_PROCESS));

                UNLOCK_PFN (OldIrql);

                VirtualAddress = PointerProtoPte;
                PointerPte = CheckPte;
                PointerProtoPte = NULL;

                //
                // The page that contains the prototype PTE is not in memory.
                //

                if (Process == HYDRA_PROCESS) {

                    //
                    // We were called while holding this session space's
                    // working set lock.  But we need to fault in a
                    // prototype PTE which is in system paged pool. This
                    // must be done under the system working set lock.
                    //
                    // So we release the session space WSL lock and get
                    // the system working set lock.  When done
                    // we return STATUS_MORE_PROCESSING_REQUIRED
                    // so our caller will call us again to handle the
                    // actual prototype PTE fault.
                    //

                    ASSERT (MI_IS_SESSION_ADDRESS (VirtualAddress) == FALSE);

                    SessionWs = &MmSessionSpace->GlobalVirtualAddress->Vm;

                    UNLOCK_WORKING_SET (WsThread, SessionWs);

                    //
                    // Clear Process as the system working set is now held.
                    //

                    Process = NULL;

                    LOCK_SYSTEM_WS (WsThread);

                    if (PointerPte->u.Hard.Valid != 0) {

                        //
                        // The prototype PTE was made valid by some other
                        // thread while we waited for the system working
                        // set pushlock, so just return and reprocess the fault.
                        //

                        UNLOCK_SYSTEM_WS (WsThread);

                        LOCK_WORKING_SET (WsThread, SessionWs);

                        ASSERT (LockedProtoPfn == NULL);
                        ASSERT (EntryIrql == KeGetCurrentIrql ());
                        ASSERT (KeAreAllApcsDisabled () == TRUE);

                        return STATUS_SUCCESS;
                    }
                }

                goto NonProtoFault;
            }
            else if (PointerPte->u.Hard.Valid == 1) {

                //
                // PTE was already made valid by the cache manager support
                // routines.
                //

                UNLOCK_PFN (OldIrql);

                return STATUS_SUCCESS;
            }
        }

        status = MiResolveProtoPteFault (StoreInstruction,
                                         VirtualAddress,
                                         PointerPte,
                                         PointerProtoPte,
                                         &LockedProtoPfn,
                                         &ReadBlock,
                                         &OriginalPte,
                                         Process,
                                         OldIrql,
                                         TrapInformation);

        //
        // Returns with PFN lock released.
        //

        ReadPte = PointerProtoPte;

        ASSERT (KeGetCurrentIrql() <= APC_LEVEL);
        ASSERT (KeAreAllApcsDisabled () == TRUE);
    }
    else {

NonProtoFault:

        TempPte = *PointerPte;
        ASSERT (TempPte.u.Hard.Valid == 0);
        ASSERT (TempPte.u.Soft.Prototype == 0);
        ASSERT (TempPte.u.Long != 0);

        if (TempPte.u.Soft.Transition != 0) {

            //
            // This is a transition page.
            //

            CapturedEvent = NULL;
            status = MiResolveTransitionFault (VirtualAddress,
                                               PointerPte,
                                               Process,
                                               MM_NOIRQL,
                                               &CapturedEvent);
            if (CapturedEvent != NULL) {
                MiFreeInPageSupportBlock (CapturedEvent);
            }

        }
        else if (TempPte.u.Soft.PageFileHigh == 0) {

            //
            // Demand zero fault.
            //

            status = MiResolveDemandZeroFault (VirtualAddress,
                                               PointerPte,
                                               Process,
                                               MM_NOIRQL);
        }
        else {

            //
            // Page resides in paging file.
            //

            ReadPte = PointerPte;
            LOCK_PFN (OldIrql);

            TempPte = *PointerPte;
            ASSERT (TempPte.u.Long != 0);

            if ((TempPte.u.Hard.Valid == 0) &&
                (TempPte.u.Soft.Prototype == 0) &&
                (TempPte.u.Soft.Transition == 0)) {

                status = MiResolvePageFileFault (VirtualAddress,
                                                 PointerPte,
                                                 &OriginalPte,
                                                 &ReadBlock,
                                                 Process,
                                                 OldIrql);
            }
            else {
                UNLOCK_PFN (OldIrql);
                status = STATUS_REFAULT;
            }
        }
    }

    //
    // Issue the I/O and/or finish completing the soft fault.
    //

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    if (NT_SUCCESS(status)) {

        if (LockedProtoPfn != NULL) {

            //
            // Unlock page containing prototype PTEs.
            //

            ASSERT (PointerProtoPte != NULL);
            LOCK_PFN (OldIrql);

            //
            // The reference count on the prototype PTE page will
            // always be greater than 1 if it is a genuine prototype
            // PTE pool allocation.  However, if it is a fork
            // prototype PTE allocation, it is possible the pool has
            // already been deallocated and in this case, the LockedProtoPfn
            // frame below will be in transition limbo with a share
            // count of 0 and a reference count of 1 awaiting our
            // final dereference below which will put it on the free list.
            //

            ASSERT (LockedProtoPfn->u3.e2.ReferenceCount >= 1);
            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (LockedProtoPfn);
            UNLOCK_PFN (OldIrql);
        }

        if (SessionWs != NULL) {
            UNLOCK_SYSTEM_WS (WsThread);
            LOCK_WORKING_SET (WsThread, SessionWs);
        }

        ASSERT (EntryIrql == KeGetCurrentIrql ());
        ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

        return status;
    }

    if (status == STATUS_ISSUE_PAGING_IO) {

        ASSERT (ReadPte != NULL);
        ASSERT (ReadBlock != NULL);

        if (PointerProtoPte == NULL) {
            OriginalPte = *ReadPte;
        }
        else {

            //
            // Since we may be holding a process or session working set
            // pushlock, we cannot reference the prototype PTE here as it
            // may have been trimmed already (this is possible when the PFN
            // lock was released) - so our callees have already captured
            // it for us.
            //

            ASSERT (OriginalPte.u.Hard.Valid == 0);
            ASSERT (OriginalPte.u.Soft.Prototype == 0);
            ASSERT (OriginalPte.u.Soft.Transition == 1);
        }

        CapturedEvent = (PMMINPAGE_SUPPORT)ReadBlock->Pfn->u1.Event;

        WsThread->ActiveFaultCount += 1;

        if (Process == HYDRA_PROCESS) {
            UNLOCK_WORKING_SET (WsThread,
                                &MmSessionSpace->GlobalVirtualAddress->Vm);
            ASSERT (KeGetCurrentIrql () <= APC_LEVEL);
            ASSERT (KeAreAllApcsDisabled () == TRUE);
        }
        else if (Process != NULL) {

            //
            // APCs must be explicitly disabled to prevent suspend APCs from
            // interrupting this thread before the I/O has been issued.
            // Otherwise a shared page I/O can stop any other thread that
            // references it indefinitely until the suspend is released.
            //

            KeEnterCriticalRegionThread (&WsThread->Tcb);

            UNLOCK_WS (WsThread, Process);

            VariousFlags |= VARIOUS_FLAGS_ENTERED_CRITICAL_REGION;
        }
        else {
            UNLOCK_SYSTEM_WS (WsThread);
            ASSERT (KeGetCurrentIrql () <= APC_LEVEL);
            ASSERT (KeAreAllApcsDisabled () == TRUE);
        }

        if (PERFINFO_IS_GROUP_ON (PERF_FILE_IO)) {
            VariousFlags |= VARIOUS_FLAGS_LOG_HARD_FAULT;

            PerfTimeStamp (IoStartTime);
        }
        else {
            SATISFY_OVERZEALOUS_COMPILER (IoStartTime.QuadPart = 0);
        }

        //
        // Assert no reads issued here are marked as prefetched.
        //

        ASSERT (ReadBlock->u1.e1.PrefetchMdlHighBits == 0);

        //
        // Issue the read request.
        //

        status = IoPageRead (ReadBlock->FilePointer,
                             &ReadBlock->Mdl,
                             &ReadBlock->ReadOffset,
                             &ReadBlock->Event,
                             &ReadBlock->IoStatus);

        if (!NT_SUCCESS(status)) {

            //
            // Set the event as the I/O system doesn't set it on errors.
            //

            ReadBlock->IoStatus.Status = status;
            ReadBlock->IoStatus.Information = 0;
            KeSetEvent (&ReadBlock->Event, 0, FALSE);
        }

        //
        // Initializing PageFrameIndex is not needed for correctness, but
        // without it the compiler cannot compile this code W4 to check
        // for use of uninitialized variables.
        //

        PageFrameIndex = (PFN_NUMBER)-1;

        //
        // Wait for the I/O operation.
        //

        status = MiWaitForInPageComplete (ReadBlock->Pfn,
                                          ReadPte,
                                          VirtualAddress,
                                          &OriginalPte,
                                          CapturedEvent,
                                          Process);

        WsThread->ActiveFaultCount -= 1;

        if (VariousFlags & VARIOUS_FLAGS_ENTERED_CRITICAL_REGION) {
            KeLeaveCriticalRegionThread (&WsThread->Tcb);
        }

        if (VariousFlags & VARIOUS_FLAGS_LOG_HARD_FAULT) {
            PerfTimeStamp (HardFaultEvent.IoTime);
            HardFaultEvent.IoTime.QuadPart -= IoStartTime.QuadPart;
        }

        //
        // MiWaitForInPageComplete RETURNS WITH THE WORKING SET LOCK
        // AND PFN LOCK HELD!!!
        //

        //
        // This is the thread which owns the event, clear the event field
        // in the PFN database.
        //

        Pfn1 = ReadBlock->Pfn;
        Page = &ReadBlock->Page[0];
        NumberOfBytes = (LONG)ReadBlock->Mdl.ByteCount;
        CheckPte = ReadBlock->BasePte;

        while (NumberOfBytes > 0) {

            //
            // Don't remove the page we just brought in to
            // satisfy this page fault.
            //

            if (CheckPte != ReadPte) {
                PfnClusterPage = MI_PFN_ELEMENT (*Page);
                MI_SNAP_DATA (PfnClusterPage, PfnClusterPage->PteAddress, 0xB);
                ASSERT (PfnClusterPage->u4.PteFrame == Pfn1->u4.PteFrame);
#if DBG
                if (PfnClusterPage->u4.InPageError) {
                    ASSERT (status != STATUS_SUCCESS);
                }
#endif
                if (PfnClusterPage->u3.e1.ReadInProgress != 0) {

                    ASSERT (PfnClusterPage->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                    PfnClusterPage->u3.e1.ReadInProgress = 0;

                    if (PfnClusterPage->u4.InPageError == 0) {
                        PfnClusterPage->u1.Event = NULL;
                    }
                }
                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (PfnClusterPage);
            }
            else {
                PageFrameIndex = *Page;
                MI_SNAP_DATA (MI_PFN_ELEMENT (PageFrameIndex),
                              MI_PFN_ELEMENT (PageFrameIndex)->PteAddress,
                              0xC);
            }

            CheckPte += 1;
            Page += 1;
            NumberOfBytes -= PAGE_SIZE;
        }

        if (status != STATUS_SUCCESS) {

            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (MI_PFN_ELEMENT(PageFrameIndex));

            if (status != STATUS_PTE_CHANGED) {

                //
                // An I/O error occurred during the page read
                // operation.  All the pages which were just
                // put into transition should be put onto the
                // free list if InPageError is set, and their
                // PTEs restored to the proper contents.
                //
    
                Page = &ReadBlock->Page[0];
    
                NumberOfBytes = ReadBlock->Mdl.ByteCount;
    
                while (NumberOfBytes > 0) {
    
                    PfnClusterPage = MI_PFN_ELEMENT (*Page);
    
                    if ((PfnClusterPage->u4.InPageError == 1) &&
                        (PfnClusterPage->u3.e2.ReferenceCount == 0)) {
    
                        PfnClusterPage->u4.InPageError = 0;

                        //
                        // Only restore the transition PTE if the address
                        // space still exists.  Another thread may have
                        // deleted the VAD while this thread waited for the
                        // fault to complete - in this case, the frame
                        // will be marked as free already.
                        //

                        if (PfnClusterPage->u3.e1.PageLocation != FreePageList) {
                            ASSERT (PfnClusterPage->u3.e1.PageLocation ==
                                                            StandbyPageList);
                            MiUnlinkPageFromList (PfnClusterPage);
                            ASSERT (PfnClusterPage->u3.e2.ReferenceCount == 0);
                            MiRestoreTransitionPte (PfnClusterPage);
                            MiInsertPageInFreeList (*Page);
                        }
                    }
                    Page += 1;
                    NumberOfBytes -= PAGE_SIZE;
                }
            }

            if (LockedProtoPfn != NULL) {

                //
                // Unlock page containing prototype PTEs.
                //

                ASSERT (PointerProtoPte != NULL);

                //
                // The reference count on the prototype PTE page will
                // always be greater than 1 if it is a genuine prototype
                // PTE pool allocation.  However, if it is a fork
                // prototype PTE allocation, it is possible the pool has
                // already been deallocated and in this case, the LockedProtoPfn
                // frame below will be in transition limbo with a share
                // count of 0 and a reference count of 1 awaiting our
                // final dereference below which will put it on the free list.
                //

                ASSERT (LockedProtoPfn->u3.e2.ReferenceCount >= 1);
                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (LockedProtoPfn);
            }

            UNLOCK_PFN (OldIrql);

            if (SessionWs != NULL) {
                UNLOCK_SYSTEM_WS (WsThread);
                LOCK_WORKING_SET (WsThread, SessionWs);
            }

            MiFreeInPageSupportBlock (CapturedEvent);

            if (status == STATUS_PTE_CHANGED) {

                //
                // State of PTE changed during I/O operation, just
                // return success and refault.
                //

                status = STATUS_SUCCESS; 
            }
            else if (status == STATUS_REFAULT) {

                //
                // The I/O operation to bring in a system page failed
                // due to insufficient resources.  Set the status to one
                // of the MmIsRetryIoStatus codes so our caller will
                // delay and retry.
                //

                status = STATUS_NO_MEMORY;
            }

            ASSERT (EntryIrql == KeGetCurrentIrql ());
            return status;
        }

        //
        // PTE is still in transition state, same protection, etc.
        //

        ASSERT (Pfn1->u4.InPageError == 0);

        if (Pfn1->u2.ShareCount == 0) {
            MI_REMOVE_LOCKED_PAGE_CHARGE (Pfn1);
        }

        Pfn1->u2.ShareCount += 1;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;

        //
        // Ensure the proper cache attribute is used both in the PTE and
        // recorded in the PFN.
        //

        CacheAttribute = MiCached;
        Protection = (MM_PROTECTION_MASK) ReadPte->u.Soft.Protection;
        if (MI_IS_WRITECOMBINE (Protection)) {
            CacheAttribute = MI_TRANSLATE_CACHETYPE (MiWriteCombined, 0);
        }
        else if (MI_IS_NOCACHE (Protection)) {
            CacheAttribute = MI_TRANSLATE_CACHETYPE (MiNonCached, 0);
        }

        if (Pfn1->u3.e1.CacheAttribute != CacheAttribute) {
            MI_FLUSH_TB_FOR_INDIVIDUAL_ATTRIBUTE_CHANGE (PageFrameIndex,
                                                         CacheAttribute);
            Pfn1->u3.e1.CacheAttribute = CacheAttribute;
        }

        //
        // Fill the PTE.
        //

        MI_MAKE_TRANSITION_PTE_VALID (TempPte, ReadPte);
        if (StoreInstruction && TempPte.u.Hard.Write) {
            MI_SET_PTE_DIRTY (TempPte);
        }
        MI_WRITE_VALID_PTE (ReadPte, TempPte);

        if (PointerProtoPte != NULL) {

            //
            // The prototype PTE has been made valid, now make the
            // original PTE valid.  The original PTE must still be invalid
            // otherwise MiWaitForInPageComplete would have returned
            // a collision status.
            //

            ASSERT (PointerPte->u.Hard.Valid == 0);

            //
            // PTE is not valid, continue with operation.
            //

            status = MiCompleteProtoPteFault (StoreInstruction,
                                              VirtualAddress,
                                              PointerPte,
                                              PointerProtoPte,
                                              OldIrql,
                                              &LockedProtoPfn);

            //
            // Returns with PFN lock released!
            //

            ASSERT (KeAreAllApcsDisabled () == TRUE);
        }
        else {

            ASSERT (LockedProtoPfn == NULL);

            ASSERT (Pfn1->u3.e1.PrototypePte == 0);

            UNLOCK_PFN (OldIrql);

            WorkingSetIndex = MiAddValidPageToWorkingSet (VirtualAddress,
                                                          ReadPte,
                                                          Pfn1,
                                                          0);

            if (WorkingSetIndex == 0) {

                //
                // Trim the page since we couldn't add it to the working
                // set list at this time.
                //

                MiTrimPte (VirtualAddress,
                           ReadPte,
                           Pfn1,
                           Process,
                           ZeroPte);

                status = STATUS_NO_MEMORY;
            }

            ASSERT (KeAreAllApcsDisabled () == TRUE);
        }

        if (VariousFlags & VARIOUS_FLAGS_LOG_HARD_FAULT) {

            HardFaultEvent.ReadOffset = ReadBlock->ReadOffset;
            HardFaultEvent.VirtualAddress = VirtualAddress;
            HardFaultEvent.FileObject = ReadBlock->FilePointer;
            HardFaultEvent.ThreadId = HandleToUlong (WsThread->Cid.UniqueThread);
            HardFaultEvent.ByteCount = ReadBlock->Mdl.ByteCount;

            PerfInfoLogBytes (PERFINFO_LOG_TYPE_HARDFAULT, 
                              &HardFaultEvent, 
                              sizeof(HardFaultEvent));
        }

        MiFreeInPageSupportBlock (CapturedEvent);

        if (status == STATUS_SUCCESS) {
            status = STATUS_PAGE_FAULT_PAGING_FILE;
        }
    }

    if ((status == STATUS_REFAULT) || (status == STATUS_PTE_CHANGED)) {
        status = STATUS_SUCCESS;
    }

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    if (SessionWs != NULL) {
        UNLOCK_SYSTEM_WS (WsThread);
        ASSERT (KeAreAllApcsDisabled () == TRUE);
        LOCK_WORKING_SET (WsThread, SessionWs);
    }

    if (LockedProtoPfn != NULL) {

        //
        // Unlock page containing prototype PTEs.
        //

        ASSERT (PointerProtoPte != NULL);
        LOCK_PFN (OldIrql);

        //
        // The reference count on the prototype PTE page will
        // always be greater than 1 if it is a genuine prototype
        // PTE pool allocation.  However, if it is a fork
        // prototype PTE allocation, it is possible the pool has
        // already been deallocated and in this case, the LockedProtoPfn
        // frame below will be in transition limbo with a share
        // count of 0 and a reference count of 1 awaiting our
        // final dereference below which will put it on the free list.
        //

        ASSERT (LockedProtoPfn->u3.e2.ReferenceCount >= 1);
        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (LockedProtoPfn);
        UNLOCK_PFN (OldIrql);
    }

    ASSERT (EntryIrql == KeGetCurrentIrql ());
    ASSERT (KeAreAllApcsDisabled () == TRUE);

    return status;
}


NTSTATUS
MiResolveDemandZeroFault (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This routine resolves a demand zero page fault.

Arguments:

    VirtualAddress - Supplies the faulting address.

    PointerPte - Supplies the PTE for the faulting address.

    Process - Supplies a pointer to the process object.  If this
              parameter is NULL, then the fault is for system
              space and the process's working set lock is not held.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at (MM_NOIRQL
              if the caller does not hold the PFN lock).  If the caller holds
              the PFN lock, the lock cannot be dropped, and the page should
              not be added to the working set at this time.


Return Value:

    NTSTATUS.

Environment:

    Kernel mode, PFN lock held conditionally.

--*/


{
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    MMPTE TempPte;
    ULONG PageColor;
    LOGICAL NeedToZero;
    LOGICAL BarrierNeeded;
    ULONG BarrierStamp;
    WSLE_NUMBER WorkingSetIndex;
    LOGICAL ZeroPageNeeded;
    LOGICAL CallerHeldPfn;

    NeedToZero = FALSE;
    BarrierNeeded = FALSE;
    CallerHeldPfn = TRUE;

    //
    // Initializing BarrierStamp is not needed for
    // correctness but without it the compiler cannot compile this code
    // W4 to check for use of uninitialized variables.
    //

    BarrierStamp = 0;

    //
    // Initialize variables assuming the operation will succeed.
    // If it fails (lack of pages or whatever), it's ok that the
    // process' NextPageColor got bumped anyway.  The goal is to do
    // as much as possible without holding the PFN lock.
    //

    if ((Process > HYDRA_PROCESS) && (OldIrql == MM_NOIRQL)) {

        ASSERT (MI_IS_PAGE_TABLE_ADDRESS (PointerPte));

        //
        // If a fork operation is in progress and the faulting thread
        // is not the thread performing the fork operation, block until
        // the fork is completed.
        //

        if (Process->ForkInProgress != NULL) {
            if (MiWaitForForkToComplete (Process) == TRUE) {
                return STATUS_REFAULT;
            }
        }

        PageColor = MI_PAGE_COLOR_VA_PROCESS (VirtualAddress,
                                              &Process->NextPageColor);

        ASSERT (PageColor != 0xFFFFFFFF);
        ZeroPageNeeded = TRUE;
    }
    else {
        if (OldIrql != MM_NOIRQL) {
            ZeroPageNeeded = TRUE;
        }
        else {
            ZeroPageNeeded = FALSE;

            //
            // For session space, the BSS of an image is typically mapped
            // directly as an image, but in the case of images that have
            // outstanding user references at the time of section creation,
            // the image is copied to a pagefile backed section and then
            // mapped in session view space (the destination is mapped in
            // system view space).  See MiSessionWideReserveImageAddress.
            //

            if ((Process == HYDRA_PROCESS) &&
                ((MI_IS_SESSION_IMAGE_ADDRESS (VirtualAddress)) ||
                 ((VirtualAddress >= (PVOID) MiSessionViewStart) &&
                  (VirtualAddress < (PVOID) MiSessionSpaceWs)))) {

                ZeroPageNeeded = TRUE;
            }
        }

        PageColor = 0xFFFFFFFF;
    }

    if (OldIrql == MM_NOIRQL) {
        CallerHeldPfn = FALSE;
        LOCK_PFN (OldIrql);
    }

    MM_PFN_LOCK_ASSERT();

    ASSERT (PointerPte->u.Hard.Valid == 0);

    //
    // Check to see if a page is available, if a wait is
    // returned, do not continue, just return success.
    //

    if ((MmAvailablePages >= MM_HIGH_LIMIT) ||
        (!MiEnsureAvailablePageOrWait (Process, OldIrql))) {

        if (PageColor != 0xFFFFFFFF) {

            //
            // This page is for a user process and so must be zeroed.
            //

            PageFrameIndex = MiRemoveZeroPageIfAny (PageColor);

            if (PageFrameIndex) {

                //
                // This barrier check is needed after zeroing the page
                // and before setting the PTE valid.  Note since the PFN
                // database entry is used to hold the sequence timestamp,
                // it must be captured now.  Check it at the last possible
                // moment.
                //

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                BarrierStamp = (ULONG)Pfn1->u4.PteFrame;
            }
            else {
                PageFrameIndex = MiRemoveAnyPage (PageColor);
                NeedToZero = TRUE;
            }
        }
        else {

            //
            // As this is a system page, there is no need to
            // remove a page of zeroes, it must be initialized by
            // the system before being used.
            //

            PageColor = MI_GET_PAGE_COLOR_FROM_VA (VirtualAddress);

            if (ZeroPageNeeded) {
                PageFrameIndex = MiRemoveZeroPage (PageColor);
            }
            else {
                PageFrameIndex = MiRemoveAnyPage (PageColor);
            }
        }

        MiInitializePfn (PageFrameIndex, PointerPte, 1);

        if (CallerHeldPfn == FALSE) {
            UNLOCK_PFN (OldIrql);
            if (Process > HYDRA_PROCESS) {
                Process->NumberOfPrivatePages += 1;
                BarrierNeeded = TRUE;
            }
        }

        InterlockedIncrement (&KeGetCurrentPrcb ()->MmDemandZeroCount);

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        if (NeedToZero) {

            PMMPTE ZeroPte;
            PVOID ZeroAddress;

            ASSERT (CallerHeldPfn == FALSE);

            ZeroPte = MiReserveSystemPtes (1, SystemPteSpace);

            if (ZeroPte != NULL) {

                TempPte = ValidKernelPte;
                TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

                if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                }
                else if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {
                    MI_DISABLE_CACHING (TempPte);
                }

                MI_WRITE_VALID_PTE (ZeroPte, TempPte);

                ZeroAddress = MiGetVirtualAddressMappedByPte (ZeroPte);

                KeZeroSinglePage (ZeroAddress);

                MiReleaseSystemPtes (ZeroPte, 1, SystemPteSpace);
            }
            else {
                MiZeroPhysicalPage (PageFrameIndex);
            }

            //
            // Note the stamping must occur after the page is zeroed.
            //

            MI_BARRIER_STAMP_ZEROED_PAGE (&BarrierStamp);
        }

        //
        // As this page is demand zero, set the modified bit in the
        // PFN database element and set the dirty bit in the PTE.
        //

        MI_SNAP_DATA (Pfn1, PointerPte, 5);

        if (PointerPte <= MiHighestUserPte) {
            MI_MAKE_VALID_USER_PTE (TempPte,
                                    PageFrameIndex,
                                    PointerPte->u.Soft.Protection,
                                    PointerPte);
        }
        else {

            //
            // Might be system or session or user page directories,
            // so go the long way to find out.
            //

            MI_MAKE_VALID_PTE (TempPte,
                               PageFrameIndex,
                               PointerPte->u.Soft.Protection,
                               PointerPte);
        }

        if (TempPte.u.Hard.Write != 0) {
            MI_SET_PTE_DIRTY (TempPte);
        }

        if (BarrierNeeded) {
            MI_BARRIER_SYNCHRONIZE (BarrierStamp);
        }

        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        if (CallerHeldPfn == FALSE) {

            ASSERT (Pfn1->u1.Event == 0);

            ASSERT (Pfn1->u3.e1.PrototypePte == 0);

            WorkingSetIndex = MiAddValidPageToWorkingSet (VirtualAddress,
                                                          PointerPte,
                                                          Pfn1,
                                                          0);
            if (WorkingSetIndex == 0) {

                //
                // Trim the page since we couldn't add it to the working
                // set list at this time.
                //

                MiTrimPte (VirtualAddress,
                           PointerPte,
                           Pfn1,
                           Process,
                           ZeroPte);

                return STATUS_NO_MEMORY;
            }
        }
        return STATUS_PAGE_FAULT_DEMAND_ZERO;
    }

    if (CallerHeldPfn == FALSE) {
        UNLOCK_PFN (OldIrql);
    }
    return STATUS_REFAULT;
}


NTSTATUS
MiResolveTransitionFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PEPROCESS CurrentProcess,
    IN KIRQL OldIrql,
    OUT PMMINPAGE_SUPPORT *InPageBlock
    )

/*++

Routine Description:

    This routine resolves a transition page fault.

Arguments:

    FaultingAddress - Supplies the faulting address.

    PointerPte - Supplies the PTE for the faulting address.

    CurrentProcess - Supplies a pointer to the process object.  If this
                     parameter is NULL, then the fault is for system
                     space and the process's working set lock is not held.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at.

    InPageBlock - Supplies a pointer to an inpage block pointer.  The caller
                  must initialize this to NULL on entry.  This routine
                  sets this to a non-NULL value to signify an inpage block
                  the caller must free when the caller releases the PFN lock.

Return Value:

    status, either STATUS_SUCCESS, STATUS_REFAULT or an I/O status
    code.

Environment:

    Kernel mode, PFN lock may optionally be held.

--*/

{
    MM_PROTECTION_MASK Protection;
    MMPFNENTRY PfnFlags;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    MMPTE TempPte;
    MMPTE TempPte2;
    NTSTATUS status;
    NTSTATUS PfnStatus;
    PMMINPAGE_SUPPORT CapturedEvent;
    PETHREAD CurrentThread;
    PMMPTE PointerToPteForProtoPage;
    WSLE_NUMBER WorkingSetIndex;
    ULONG PfnLockHeld;
    LOGICAL EnteredCritical;

    //
    // ***********************************************************
    //      Transition PTE.
    // ***********************************************************
    //

    //
    // A transition PTE is either on the free or modified list,
    // on neither list because of its ReferenceCount
    // or currently being read in from the disk (read in progress).
    // If the page is read in progress, this is a collided page
    // and must be handled accordingly.
    //

    ASSERT (*InPageBlock == NULL);

    if (OldIrql == MM_NOIRQL) {

        PfnLockHeld = FALSE;

        //
        // Read the PTE now without the PFN lock so that the PFN entry
        // calculations, etc can be done in advance.  If it turns out the PTE
        // changed after the lock is acquired (should be rare), then
        // recalculate.
        //

        TempPte2 = *PointerPte;

        PageFrameIndex = (PFN_NUMBER) TempPte2.u.Hard.PageFrameNumber;
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        ASSERT (OldIrql == MM_NOIRQL);
        LOCK_PFN (OldIrql);

        TempPte = *PointerPte;

        if ((TempPte.u.Soft.Valid == 0) &&
            (TempPte.u.Soft.Prototype == 0) &&
            (TempPte.u.Soft.Transition == 1)) {

            if (TempPte2.u.Long != TempPte.u.Long) {
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&TempPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            }

            NOTHING;
        }
        else {
            UNLOCK_PFN (OldIrql);
            return STATUS_REFAULT;
        }
    }
    else {

        PfnLockHeld = TRUE;
        ASSERT (OldIrql != MM_NOIRQL);
        TempPte = *PointerPte;

        ASSERT ((TempPte.u.Soft.Valid == 0) &&
                (TempPte.u.Soft.Prototype == 0) &&
                (TempPte.u.Soft.Transition == 1));

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&TempPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    }

    //
    // Still in transition format.
    //

    InterlockedIncrement (&KeGetCurrentPrcb ()->MmTransitionCount);

    if (Pfn1->u4.InPageError) {

        //
        // There was an in-page read error and there are other
        // threads colliding for this page, delay to let the
        // other threads complete and return.  Snap relevant PFN fields
        // before releasing the lock as the page may immediately get 
        // reused.
        //

        PfnFlags = Pfn1->u3.e1;
        status = Pfn1->u1.ReadStatus;

        if (!PfnLockHeld) {
            UNLOCK_PFN (OldIrql);
        }

        if (PfnFlags.ReadInProgress) {

            //
            // This only occurs when the page is being reclaimed by the
            // compression reaper.  In this case, the page is still on the
            // transition list (so the ReadStatus is really a flink) so
            // substitute a retry status which will induce a delay so the
            // compression reaper can finish taking the page (and PTE).
            //

            return STATUS_NO_MEMORY;
        }

        ASSERT (!NT_SUCCESS(status));

        return status;
    }

    if (Pfn1->u3.e1.ReadInProgress) {

        //
        // Collided page fault.
        //

        CapturedEvent = (PMMINPAGE_SUPPORT)Pfn1->u1.Event;

        CurrentThread = PsGetCurrentThread ();

        if (CapturedEvent->Thread == CurrentThread) {

            //
            // This detects when the Io APC completion routine accesses
            // the same user page (ie: during an overlapped I/O) that
            // the user thread has already faulted on.
            //
            // This can result in a fatal deadlock and so must
            // be detected here.  Return a unique status code so the
            // (legitimate) callers know this has happened so it can be
            // handled properly, ie: Io must request a callback from
            // the Mm once the first fault has completed.
            //
            // Note that non-legitimate callers must get back a failure
            // status so the thread can be terminated.
            //

#if DBG

#if defined (_AMD64_)

            {
                //
                // Do not assert if there are stack walks in progress since
                // this can be ok. The stack walking code will deal with
                // the in page error raised due to this situation.
                //

                extern LONG RtlpStackWalksInProgress;

                if (RtlpStackWalksInProgress == 0) {
                    ASSERT (CurrentThread->ActiveFaultCount >= 1);
                }
            }

#else
            ASSERT (CurrentThread->ActiveFaultCount >= 1);

#endif

#endif

            CurrentThread->ApcNeeded = 1;

            if (!PfnLockHeld) {
                UNLOCK_PFN (OldIrql);
            }
            return STATUS_MULTIPLE_FAULT_VIOLATION;
        }

        //
        // Increment the reference count for the page so it won't be
        // reused until all collisions have been completed.
        //

        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

        InterlockedIncrementPfn ((PSHORT)&Pfn1->u3.e2.ReferenceCount);

        //
        // Careful synchronization is applied to the WaitCount field so
        // that freeing of the inpage block can occur lock-free.  Note
        // that the ReadInProgress bit on each PFN is set and cleared while
        // holding the PFN lock.  Inpage blocks are always (and must be)
        // freed _AFTER_ the ReadInProgress bit is cleared.
        //

        InterlockedIncrement (&CapturedEvent->WaitCount);

        UNLOCK_PFN (OldIrql);

        CurrentThread->ActiveFaultCount += 1;
        EnteredCritical = FALSE;

        if (CurrentProcess == HYDRA_PROCESS) {
            UNLOCK_WORKING_SET (CurrentThread,
                                &MmSessionSpace->GlobalVirtualAddress->Vm);
        }
        else if (CurrentProcess != NULL) {

            //
            // APCs must be explicitly disabled to prevent suspend APCs from
            // interrupting this thread before the wait has been issued.
            // Otherwise the APC can result in this page being locked
            // indefinitely until the suspend is released.
            //

            KeEnterCriticalRegionThread (&CurrentThread->Tcb);

            UNLOCK_WS (CurrentThread, CurrentProcess);
            EnteredCritical = TRUE;
        }
        else {
            UNLOCK_SYSTEM_WS (CurrentThread);
        }

        //
        // Set the inpage block address as the waitcount was incremented
        // above and therefore the free must be done by our caller.
        //

        *InPageBlock = CapturedEvent;

        status = MiWaitForInPageComplete (Pfn1,
                                          PointerPte,
                                          FaultingAddress,
                                          &TempPte,
                                          CapturedEvent,
                                          CurrentProcess);

        //
        // MiWaitForInPageComplete RETURNS WITH THE WORKING SET LOCK
        // AND PFN LOCK HELD!!!
        //

        CurrentThread->ActiveFaultCount -= 1;

        if (EnteredCritical == TRUE) {
            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);
        }

        ASSERT (Pfn1->u3.e1.ReadInProgress == 0);

        if (status != STATUS_SUCCESS) {
            PfnStatus = Pfn1->u1.ReadStatus;
            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1);

            //
            // Check to see if an I/O error occurred on this page.
            // If so, try to free the physical page, wait a
            // half second and return a status of PTE_CHANGED.
            // This will result in success being returned to
            // the user and the fault will occur again and should
            // not be a transition fault this time.
            //

            if (Pfn1->u4.InPageError == 1) {
                ASSERT (!NT_SUCCESS(PfnStatus));
                status = PfnStatus;
                if (Pfn1->u3.e2.ReferenceCount == 0) {

                    Pfn1->u4.InPageError = 0;

                    //
                    // Only restore the transition PTE if the address
                    // space still exists.  Another thread may have
                    // deleted the VAD while this thread waited for the
                    // fault to complete - in this case, the frame
                    // will be marked as free already.
                    //

                    if (Pfn1->u3.e1.PageLocation != FreePageList) {
                        ASSERT (Pfn1->u3.e1.PageLocation ==
                                                        StandbyPageList);
                        MiUnlinkPageFromList (Pfn1);
                        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
                        MiRestoreTransitionPte (Pfn1);
                        MiInsertPageInFreeList (PageFrameIndex);
                    }
                }
            }

            if (!PfnLockHeld) {
                UNLOCK_PFN (OldIrql);
            }

            //
            // Instead of returning status, always return STATUS_REFAULT.
            // This is to support filesystems that save state in the
            // ETHREAD of the thread that serviced the fault !  Since
            // collided threads never enter the filesystem, their ETHREADs
            // haven't been hacked up.  Since this only matters when
            // errors occur (specifically STATUS_VERIFY_REQUIRED today),
            // retry any failed I/O in the context of each collider
            // to give the filesystems ample opportunity.
            //

            return STATUS_REFAULT;
        }
    }
    else {

        //
        // PTE refers to a normal transition PTE.
        //

        ASSERT ((SPFN_NUMBER)MmAvailablePages >= 0);

        //
        // Check available pages so that a machine which is low on memory
        // can stop this thread from gobbling up the pages from every modified
        // write that completes because that would starve waiting threads.
        //
        // Another scenario is if the system is utilizing a hardware
        // compression cache.  Checking ensures that only a safe amount
        // of the compressed virtual cache is directly mapped so that
        // if the hardware gets into trouble, we can bail it out.
        //

        if ((MmAvailablePages < MM_HIGH_LIMIT) &&
            ((MmAvailablePages == 0) ||
             (PsGetCurrentThread()->MemoryMaker == 0) &&
             (MiEnsureAvailablePageOrWait (CurrentProcess, OldIrql)))) {

            //
            // A wait operation was performed which dropped the locks,
            // repeat this fault.
            //

            if (!PfnLockHeld) {
                UNLOCK_PFN (OldIrql);
            }

            //
            // Note our caller will delay execution after releasing the
            // working set mutex in order to make pages available.
            //

            return STATUS_NO_MEMORY;
        }

        ASSERT (Pfn1->u4.InPageError == 0);
        if (Pfn1->u3.e1.PageLocation == ActiveAndValid) {

            //
            // This page must contain an MmSt allocation of prototype PTEs.
            // Because these types of pages reside in paged pool (or special
            // pool) and are part of the system working set, they can be
            // trimmed at any time regardless of the share count.  However,
            // if the share count is nonzero, then the page state will
            // remain active and the page will remain in memory - but the
            // PTE will be set to the transition state.  Make the page
            // valid without incrementing the reference count, but
            // increment the share count.
            //

            ASSERT (((Pfn1->PteAddress >= MiGetPteAddress(MmPagedPoolStart)) &&
                    (Pfn1->PteAddress <= MiGetPteAddress(MmPagedPoolEnd))) ||
                    ((Pfn1->PteAddress >= MiGetPteAddress(MmSpecialPoolStart)) &&
                    (Pfn1->PteAddress <= MiGetPteAddress(MmSpecialPoolEnd))));

            //
            // Don't increment the valid PTE count for the
            // page table page.
            //

            ASSERT (Pfn1->u2.ShareCount != 0);
            ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
        }
        else {

            MiUnlinkPageFromList (Pfn1);

            //
            // Update the PFN database - the reference count must be
            // incremented as the share count is going to go from zero to 1.
            //

            ASSERT (Pfn1->u2.ShareCount == 0);

            //
            // The PFN reference count will be 1 already here if the
            // modified writer has begun a write of this page.  Otherwise
            // it's ordinarily 0.
            //

            MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1);
        }
    }

    //
    // Join with collided page fault code to handle updating
    // the transition PTE.
    //

    ASSERT (Pfn1->u4.InPageError == 0);

    if (Pfn1->u2.ShareCount == 0) {
        MI_REMOVE_LOCKED_PAGE_CHARGE (Pfn1);
    }

    Pfn1->u2.ShareCount += 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;

    //
    // Paged pool is trimmed without regard to sharecounts.
    // This means a paged pool PTE can be in transition while
    // the page is still marked active.
    //
    // Note this check only needs to be done for system space addresses
    // as user space address faults lock down the page containing the
    // prototype PTE entries before processing the fault.
    //
    // One example is a system cache fault - the FaultingAddress is a
    // system cache virtual address, the PointerPte points at the pool
    // allocation containing the relevant prototype PTEs.  This page
    // may have been trimmed because it isn't locked down during
    // processing of system space virtual address faults.
    //

    if (FaultingAddress >= MmSystemRangeStart) {

        PointerToPteForProtoPage = MiGetPteAddress (PointerPte);

        TempPte = *PointerToPteForProtoPage;

        if ((TempPte.u.Hard.Valid == 0) &&
            (TempPte.u.Soft.Transition == 1)) {

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&TempPte);
            Pfn2 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT ((Pfn2->u3.e1.ReadInProgress == 0) &&
                (Pfn2->u4.InPageError));

            ASSERT (Pfn2->u3.e1.PageLocation == ActiveAndValid);

            ASSERT (((Pfn2->PteAddress >= MiGetPteAddress(MmPagedPoolStart)) &&
                    (Pfn2->PteAddress <= MiGetPteAddress(MmPagedPoolEnd))) ||
                    ((Pfn2->PteAddress >= MiGetPteAddress(MmSpecialPoolStart)) &&
                    (Pfn2->PteAddress <= MiGetPteAddress(MmSpecialPoolEnd))));

            //
            // Don't increment the valid PTE count for the
            // paged pool page.
            //

            ASSERT (Pfn2->u2.ShareCount != 0);
            ASSERT (Pfn2->u3.e2.ReferenceCount != 0);
            ASSERT (Pfn2->u3.e1.CacheAttribute == MiCached);

            //
            // Ensure the PTE mapping does not conflict with the PFN attributes.
            //

            Protection = (MM_PROTECTION_MASK) Pfn2->OriginalPte.u.Soft.Protection;
            Protection &= ~(MM_NOCACHE | MM_WRITECOMBINE);

            if (Pfn2->u3.e1.CacheAttribute == MiCached) {
                NOTHING;
            }
            else if (Pfn2->u3.e1.CacheAttribute == MiNonCached) {
                Protection |= MM_NOCACHE;
            }
            else if (Pfn2->u3.e1.CacheAttribute == MiWriteCombined) {
                Protection |= MM_WRITECOMBINE;
            }

            MI_MAKE_VALID_PTE (TempPte,
                               PageFrameIndex,
                               Protection,
                               PointerToPteForProtoPage);

            MI_WRITE_VALID_PTE (PointerToPteForProtoPage, TempPte);
        }
    }

    MI_MAKE_TRANSITION_PTE_VALID (TempPte, PointerPte);

    //
    // If the modified field is set in the PFN database and this
    // page is not copy on modify, then set the dirty bit.
    // This can be done as the modified page will not be
    // written to the paging file until this PTE is made invalid.
    //

    if ((Pfn1->u3.e1.Modified && TempPte.u.Hard.Write) &&
        (TempPte.u.Hard.CopyOnWrite == 0)) {

        MI_SET_PTE_DIRTY (TempPte);
    }
    else {
        MI_SET_PTE_CLEAN (TempPte);
    }

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    if (!PfnLockHeld) {

        ASSERT (Pfn1->u3.e1.PrototypePte == 0);

        UNLOCK_PFN (OldIrql);

        WorkingSetIndex = MiAddValidPageToWorkingSet (FaultingAddress,
                                                      PointerPte,
                                                      Pfn1,
                                                      0);

        if (WorkingSetIndex == 0) {

            //
            // Trim the page since we couldn't add it to the working
            // set list at this time.
            //

            MiTrimPte (FaultingAddress,
                       PointerPte,
                       Pfn1,
                       CurrentProcess,
                       ZeroPte);


            return STATUS_NO_MEMORY;
        }
    }
    return STATUS_PAGE_FAULT_TRANSITION;
}


NTSTATUS
MiResolvePageFileFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    OUT PMMPTE CapturedPteContents,
    OUT PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This routine builds the MDL and other structures to allow a
    read operation on a page file for a page fault.

Arguments:

    FaultingAddress - Supplies the faulting address.

    PointerPte - Supplies the PTE for the faulting address.

    CapturedPteContents - The contents of the prototype PTE on exit are
                          captured here.  This is because the caller needs
                          to compare it later to ensure things haven't changed,
                          but the caller is unable to capture it on return
                          because the caller may be holding the pushlock of a
                          process or session working set (not system) and thus
                          cannot access the prototype PTE since it can be
                          paged as soon as the PFN lock is released by us.

                          Note this only has meaning if the caller is actually
                          going to issue the I/O.

    ReadBlock - Supplies a pointer to put the address of the read block which
                needs to be completed before an I/O can be issued.

    Process - Supplies a pointer to the process object.  If this
              parameter is NULL, then the fault is for system
              space and the process's working set lock is not held.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at.

Return Value:

    status.  A status value of STATUS_ISSUE_PAGING_IO is returned
    if this function completes successfully.

Environment:

    Kernel mode, PFN lock held.

--*/

{
    PMDL Mdl;
    ULONG i;
    PMMPTE BasePte;
    PMMPTE FaultingPte;
    PMMPTE CheckPte;
    PMMPTE FirstPte;
    PMMPTE LastPte;
    PSUBSECTION Subsection;
    ULONG ReadSize;
    LARGE_INTEGER StartingOffset;
    PFN_NUMBER PageFrameIndex;
    PPFN_NUMBER MdlPage;
    ULONG PageFileNumber;
    ULONG ClusterSize;
    ULONG BackwardPageCount;
    ULONG ForwardPageCount;
    ULONG MaxForwardPageCount;
    ULONG MaxBackwardPageCount;
    WSLE_NUMBER WorkingSetIndex;
    ULONG PageColor;
    MMPTE TempPte;
    MMPTE ComparePte;
    PMMINPAGE_SUPPORT ReadBlockLocal;
    PETHREAD CurrentThread;
    PMMVAD Vad;
    NTSTATUS Status;
    PKPRCB Prcb;

    // **************************************************
    //    Page File Read
    // **************************************************

    //
    // Calculate the VBN for the in-page operation.
    //

    TempPte = *PointerPte;

    ASSERT (TempPte.u.Hard.Valid == 0);
    ASSERT (TempPte.u.Soft.Prototype == 0);
    ASSERT (TempPte.u.Soft.Transition == 0);

    MM_PFN_LOCK_ASSERT();

    if ((MmAvailablePages < MM_HIGH_LIMIT) &&
        (MiEnsureAvailablePageOrWait (Process, OldIrql))) {

        //
        // A wait operation was performed which dropped the locks,
        // repeat this fault.
        //

        UNLOCK_PFN (OldIrql);
        return STATUS_REFAULT;
    }

    ReadBlockLocal = MiGetInPageSupportBlock (OldIrql, &Status);

    if (ReadBlockLocal == NULL) {

        UNLOCK_PFN (OldIrql);

        ASSERT (!NT_SUCCESS (Status));

        return Status;
    }

    //
    // Transition collisions rely on the entire PFN (including the event field)
    // being initialized, the ReadBlockLocal's event being not-signaled,
    // and the ReadBlockLocal's thread and waitcount being initialized.
    //
    // All of this has been done by MiGetInPageSupportBlock already except
    // the PFN settings.  The PFN lock can be safely released once
    // this is done.
    //

    ReadSize = 1;
    BasePte = NULL;
    FaultingPte = PointerPte;

    if (MI_IS_PAGE_TABLE_ADDRESS (PointerPte)) {
        WorkingSetIndex = 1;
    }
    else {
        WorkingSetIndex = MI_PROTOTYPE_WSINDEX;
    }

    //
    // Capture the desired cluster size.
    //

    ClusterSize = MmClusterPageFileReads;
    ASSERT (ClusterSize <= MM_MAXIMUM_READ_CLUSTER_SIZE);

    if (MiInPageSinglePages != 0) {
        MiInPageSinglePages -= 1;
    }
    else if ((ClusterSize > 1) && (MmAvailablePages > MM_PLENTY_FREE_LIMIT)) {

        //
        // Maybe this condition should be only on free+zeroed pages (ie: don't
        // include standby).  Maybe it should look at the recycle rate of
        // the standby list, etc, etc.
        //

        ASSERT (ClusterSize <= MmAvailablePages);

        //
        // Attempt to cluster ahead and behind.
        //

        MaxForwardPageCount = PTE_PER_PAGE - (BYTE_OFFSET (PointerPte) / sizeof (MMPTE));
        ASSERT (MaxForwardPageCount != 0);
        MaxBackwardPageCount = PTE_PER_PAGE - MaxForwardPageCount;
        MaxForwardPageCount -= 1;

        if (WorkingSetIndex == MI_PROTOTYPE_WSINDEX) {

            //
            // This is a pagefile read for a shared memory (prototype PTE)
            // backed section.   Stay within the prototype PTE pool allocation.
            //
            // The prototype PTE pool start and end must be carefully
            // calculated (remember the user's view may be smaller or larger
            // than this).  Don't bother walking the entire VAD tree if it is
            // very large as this can take a significant amount of time.
            //

            if ((FaultingAddress <= MM_HIGHEST_USER_ADDRESS) &&
                (Process->VadRoot.NumberGenericTableElements < 128)) {

                Vad = MiLocateAddress (FaultingAddress);

                if (Vad != NULL) {
                    Subsection = MiLocateSubsection (Vad,
                                            MI_VA_TO_VPN(FaultingAddress));

                    if (Subsection != NULL) {
                        FirstPte = &Subsection->SubsectionBase[0];
                        LastPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
                        if ((ULONG)(LastPte - PointerPte - 1) < MaxForwardPageCount) {
                            MaxForwardPageCount = (ULONG)(LastPte - PointerPte - 1);
                        }

                        if ((ULONG)(PointerPte - FirstPte) < MaxBackwardPageCount) {
                            MaxBackwardPageCount = (ULONG)(PointerPte - FirstPte);
                        }
                    }
                    else {
                        ClusterSize = 0;
                    }
                }
                else {
                    ClusterSize = 0;
                }
            }
            else {
                ClusterSize = 0;
            }
        }

        CurrentThread = PsGetCurrentThread();

        if (CurrentThread->ForwardClusterOnly) {

            MaxBackwardPageCount = 0;

            if (MaxForwardPageCount == 0) {

                //
                // This PTE is the last one in the page table page and
                // no backwards clustering is enabled for this thread so
                // no clustering can be done.
                //

                ClusterSize = 0;
            }
        }

        if (ClusterSize != 0) {

            if (MaxForwardPageCount > ClusterSize) {
                MaxForwardPageCount = ClusterSize;
            }

            ComparePte = TempPte;
            CheckPte = PointerPte + 1;
            ForwardPageCount = MaxForwardPageCount;

            //
            // Try to cluster forward within the page of PTEs.
            //

            while (ForwardPageCount != 0) {

                ASSERT (MiIsPteOnPdeBoundary (CheckPte) == 0);

                ComparePte.u.Soft.PageFileHigh += 1;

                if (CheckPte->u.Long != ComparePte.u.Long) {
                    break;
                }

                ForwardPageCount -= 1;
                CheckPte += 1;
            }

            ReadSize += (MaxForwardPageCount - ForwardPageCount);

            //
            // Try to cluster backward within the page of PTEs.  Donate
            // any unused forward cluster space to the backwards gathering
            // but keep the entire transfer within the MDL.
            //

            ClusterSize -= (MaxForwardPageCount - ForwardPageCount);

            if (MaxBackwardPageCount > ClusterSize) {
                MaxBackwardPageCount = ClusterSize;
            }

            ComparePte = TempPte;
            BasePte = PointerPte;
            CheckPte = PointerPte;
            BackwardPageCount = MaxBackwardPageCount;

            while (BackwardPageCount != 0) {

                ASSERT (MiIsPteOnPdeBoundary(CheckPte) == 0);

                CheckPte -= 1;
                ComparePte.u.Soft.PageFileHigh -= 1;

                if (CheckPte->u.Long != ComparePte.u.Long) {
                    break;
                }

                BackwardPageCount -= 1;
            }

            ReadSize += (MaxBackwardPageCount - BackwardPageCount);
            BasePte -= (MaxBackwardPageCount - BackwardPageCount);
        }
    }

    if (ReadSize == 1) {

        //
        // Get a page and put the PTE into the transition state with the
        // read-in-progress flag set.
        //

        if (Process == HYDRA_PROCESS) {
            PageColor = MI_GET_PAGE_COLOR_FROM_SESSION (MmSessionSpace);
        }
        else if (Process == NULL) {
            PageColor = MI_GET_PAGE_COLOR_FROM_VA(FaultingAddress);
        }
        else {
            PageColor = MI_PAGE_COLOR_VA_PROCESS (FaultingAddress,
                                                  &Process->NextPageColor);
        }

        PageFrameIndex = MiRemoveAnyPage (PageColor);

        MiInitializeReadInProgressSinglePfn (PageFrameIndex,
                                             PointerPte,
                                             &ReadBlockLocal->Event,
                                             WorkingSetIndex);

        MI_RETRIEVE_USED_PAGETABLE_ENTRIES_FROM_PTE (ReadBlockLocal, &TempPte);
    }
    else {

        Mdl = &ReadBlockLocal->Mdl;
        MdlPage = &ReadBlockLocal->Page[0];

        ASSERT (ReadSize <= MmAvailablePages);

        for (i = 0; i < ReadSize; i += 1) {

            //
            // Get a page and put the PTE into the transition state with the
            // read-in-progress flag set.
            //

            if (Process == HYDRA_PROCESS) {
                PageColor = MI_GET_PAGE_COLOR_FROM_SESSION (MmSessionSpace);
            }
            else if (Process == NULL) {
                PageColor = MI_GET_PAGE_COLOR_FROM_VA(FaultingAddress);
            }
            else {
                PageColor = MI_PAGE_COLOR_VA_PROCESS (FaultingAddress,
                                                      &Process->NextPageColor);
            }

            *MdlPage = MiRemoveAnyPage (PageColor);
            MdlPage += 1;
        }

        ReadSize *= PAGE_SIZE;

        //
        // Note PageFrameIndex is the actual frame that was requested by
        // this caller.  All the other frames will be put in transition
        // when the inpage completes (provided there are no colliding threads).
        //

        MdlPage = &ReadBlockLocal->Page[0];
        PageFrameIndex = *(MdlPage + (PointerPte - BasePte));

        //
        // Initialize the MDL for this request.
        //

        MmInitializeMdl (Mdl,
                         MiGetVirtualAddressMappedByPte (BasePte),
                         ReadSize);

        Mdl->MdlFlags |= (MDL_PAGES_LOCKED | MDL_IO_PAGE_READ);

        //
        // Set PointerPte and TempPte to the base of the cluster so the
        // correct starting offset can be calculated below.  Note this must
        // be done before MiInitializeReadInProgressPfn overwrites the PTEs.
        //

        PointerPte = BasePte;
        TempPte = *PointerPte;
        ASSERT (TempPte.u.Soft.Prototype == 0);
        ASSERT (TempPte.u.Soft.Transition == 0);

        //
        // Put the PTEs into the transition state with the
        // read-in-progress flag set.
        //

        MiInitializeReadInProgressPfn (Mdl,
                                       BasePte,
                                       &ReadBlockLocal->Event,
                                       WorkingSetIndex);

        MI_ZERO_USED_PAGETABLE_ENTRIES_IN_INPAGE_SUPPORT(ReadBlockLocal);
    }

    //
    // Capture the faulting PTE's contents (it will be in transition now)
    // prior to releasing the PFN lock.  The contents of this prototype PTE
    // are used by the caller to compare it later to ensure things haven't
    // changed, but the caller is unable to capture it on return because
    // the caller may be holding the pushlock of a process or session working
    // set (not system) and thus cannot access the prototype PTE since it can be
    // paged as soon as the PFN lock is released here.
    //
    // Note this only has meaning if the caller is actually going to issue
    // the I/O, ie: we must return STATUS_ISSUE_PAGING_IO here.
    //

    *CapturedPteContents = *FaultingPte;

    ASSERT (CapturedPteContents->u.Hard.Valid == 0);
    ASSERT (CapturedPteContents->u.Soft.Prototype == 0);
    ASSERT (CapturedPteContents->u.Soft.Transition == 1);

    UNLOCK_PFN (OldIrql);

    Prcb = KeGetCurrentPrcb ();
    InterlockedExchangeAdd (&Prcb->MmPageReadCount,
                            (LONG) (ReadSize >> PAGE_SHIFT));
    InterlockedIncrement (&Prcb->MmPageReadIoCount);

    *ReadBlock = ReadBlockLocal;

    PageFileNumber = GET_PAGING_FILE_NUMBER (TempPte);
    StartingOffset.LowPart = GET_PAGING_FILE_OFFSET (TempPte);

    ASSERT (StartingOffset.LowPart <= MmPagingFile[PageFileNumber]->Size);

    StartingOffset.HighPart = 0;
    StartingOffset.QuadPart = StartingOffset.QuadPart << PAGE_SHIFT;

    ReadBlockLocal->FilePointer = MmPagingFile[PageFileNumber]->File;

#if DBG

    if (((StartingOffset.QuadPart >> PAGE_SHIFT) < 8192) && (PageFileNumber == 0)) {
        if ((MmPagingFileDebug[StartingOffset.QuadPart >> PAGE_SHIFT] & ~0x1f) !=
               ((ULONG_PTR)PointerPte << 3)) {
            if ((MmPagingFileDebug[StartingOffset.QuadPart >> PAGE_SHIFT] & ~0x1f) !=
                  ((ULONG_PTR)(MiGetPteAddress(FaultingAddress)) << 3)) {

                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                    "MMINPAGE: Mismatch PointerPte %p Offset %I64X info %p\n",
                         PointerPte,
                         StartingOffset.QuadPart >> PAGE_SHIFT,
                         MmPagingFileDebug[StartingOffset.QuadPart >> PAGE_SHIFT]);

                DbgBreakPoint ();
            }
        }
    }

#endif //DBG

    ReadBlockLocal->ReadOffset = StartingOffset;
    ReadBlockLocal->BasePte = PointerPte;

    //
    // Build a single page MDL for the request unless it was a cluster -
    // clustered MDLs have already been constructed.
    //

    if (ReadSize == 1) {
        MmInitializeMdl (&ReadBlockLocal->Mdl, PAGE_ALIGN(FaultingAddress), PAGE_SIZE);
        ReadBlockLocal->Mdl.MdlFlags |= (MDL_PAGES_LOCKED | MDL_IO_PAGE_READ);
        ReadBlockLocal->Page[0] = PageFrameIndex;
    }

    ReadBlockLocal->Pfn = MI_PFN_ELEMENT (PageFrameIndex);

    return STATUS_ISSUE_PAGING_IO;
}

NTSTATUS
MiResolveProtoPteFault (
    IN ULONG_PTR StoreInstruction,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN OUT PMMPFN *LockedProtoPfn,
    OUT PMMINPAGE_SUPPORT *ReadBlock,
    OUT PMMPTE CapturedPteContents,
    IN PEPROCESS Process,
    IN KIRQL OldIrql,
    IN PVOID TrapInformation
    )

/*++

Routine Description:

    This routine resolves a prototype PTE fault.

Arguments:

    StoreInstruction - Supplies nonzero if the instruction is trying
                       to modify the faulting address (i.e. write
                       access required).

    FaultingAddress - Supplies the faulting address.

    PointerPte - Supplies the PTE for the faulting address.

    PointerProtoPte - Supplies a pointer to the prototype PTE to fault in.

    LockedProtoPfn - Supplies a non-NULL pointer to the prototype PTE's PFN
                     that was locked down by the caller, or NULL if the caller
                     did not lock down any PFN.  This routine may unlock
                     the PFN - if so, it must also clear this pointer.

    ReadBlock - Supplies a pointer to put the address of the read block which
                needs to be completed before an I/O can be issued.

    CapturedPteContents - The contents of the prototype PTE on exit are
                          captured here.  This is because the caller needs
                          to compare it later to ensure things haven't changed,
                          but the caller is unable to capture it on return
                          because the caller may be holding the pushlock of a
                          process or session working set (not system) and thus
                          cannot access the prototype PTE since it can be
                          paged as soon as the PFN lock is released by us.

                          Note this only has meaning if the caller is actually
                          going to issue the I/O (ie, this routine must be
                          returning STATUS_ISSUE_PAGING_IO).

    Process - Supplies a pointer to the process object.  If this
              parameter is NULL, then the fault is for system
              space and the process's working set lock is not held.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at.

    TrapInformation - Supplies the trap information.

Return Value:

    NTSTATUS: STATUS_SUCCESS, STATUS_REFAULT, or an I/O status code.

Environment:

    Kernel mode, PFN lock held.

--*/
{
    MMPTE TempPte;
    MMPTE RealPte;
    MMPTE NewPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    NTSTATUS status;
    ULONG CopyOnWrite;
    LOGICAL PfnHeld;
    PMMINPAGE_SUPPORT CapturedEvent;

    //
    // Note the PFN lock must be held as the routine to locate a working
    // set entry decrements the share count of PFN elements.
    //

    MM_PFN_LOCK_ASSERT ();

    ASSERT (PointerPte->u.Hard.Valid == 0);
    ASSERT (PointerPte->u.Soft.Prototype == 1);

    TempPte = *PointerProtoPte;

    //
    // The page containing the prototype PTE is resident,
    // handle the fault referring to the prototype PTE.
    // If the prototype PTE is already valid, make this
    // PTE valid and up the share count etc.
    //

    if (TempPte.u.Hard.Valid) {

        //
        // Prototype PTE is valid, count this as a transition fault.
        //

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        Pfn1->u2.ShareCount += 1;

        InterlockedIncrement (&KeGetCurrentPrcb ()->MmTransitionCount);

        return MiCompleteProtoPteFault (StoreInstruction,
                                        FaultingAddress,
                                        PointerPte,
                                        PointerProtoPte,
                                        OldIrql,
                                        LockedProtoPfn);
    }

    //
    // Check to make sure the prototype PTE is committed.
    //

    if (TempPte.u.Long == 0) {
        MI_BREAK_ON_AV (FaultingAddress, 0xA);
        UNLOCK_PFN (OldIrql);
        return STATUS_ACCESS_VIOLATION;
    }

    CapturedEvent = NULL;

    //
    // If the PTE indicates that the protection field to be
    // checked is in the prototype PTE, check it now.
    //

    CopyOnWrite = FALSE;

    RealPte = *PointerPte;

    if (RealPte.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) {
        if (RealPte.u.Proto.ReadOnly == 0) {

            //
            // Check for kernel mode access, we have already verified
            // that the user has access to the virtual address.
            //

            status = MiAccessCheck (PointerProtoPte,
                                    StoreInstruction,
                                    KernelMode,
                                    MI_GET_PROTECTION_FROM_SOFT_PTE (&TempPte),
                                    TrapInformation,
                                    TRUE);

            if (status != STATUS_SUCCESS) {

                if ((StoreInstruction) &&
                    (MI_IS_SESSION_ADDRESS (FaultingAddress)) &&
                    (MmSessionSpace->ImageLoadingCount != 0)) {
        
                    PLIST_ENTRY NextEntry;
                    PIMAGE_ENTRY_IN_SESSION Image;

                    NextEntry = MmSessionSpace->ImageList.Flink;

                    while (NextEntry != &MmSessionSpace->ImageList) {

                        Image = CONTAINING_RECORD (NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

                        if ((FaultingAddress >= Image->Address) &&
                            (FaultingAddress <= Image->LastAddress)) {

                            if (Image->ImageLoading) {

                                //
                                // Temporarily allow the write so that
                                // relocations and import snaps can be
                                // completed.
                                //
                                // Even though the page's current backing
                                // is the image file, the modified writer
                                // will convert it to pagefile backing
                                // when it notices the change later.
                                //

                                goto done;
                            }
                            break;
                        }
                        NextEntry = NextEntry->Flink;
                    }
                }

                MI_BREAK_ON_AV (FaultingAddress, 9);
                UNLOCK_PFN (OldIrql);
                return status;
            }
            if ((TempPte.u.Soft.Protection & MM_COPY_ON_WRITE_MASK) ==
                 MM_COPY_ON_WRITE_MASK) {
                CopyOnWrite = TRUE;
            }
        }
    }
    else {
        if ((RealPte.u.Soft.Protection & MM_COPY_ON_WRITE_MASK) ==
             MM_COPY_ON_WRITE_MASK) {
            CopyOnWrite = TRUE;
        }
    }

    //
    // If the fault is in user space and this is a cloned process then
    // don't take the optimized demand zero copy on write path as we
    // want to go the long way so the clone block can be properly
    // decremented in MiCopyOnWrite.
    //

    if ((PointerPte <= MiHighestUserPte) &&
        (Process > PREFETCH_PROCESS) &&
        (Process->CloneRoot != NULL)) {

        CopyOnWrite = FALSE;
    }

done:

    if ((!IS_PTE_NOT_DEMAND_ZERO (TempPte)) && (CopyOnWrite)) {

        //
        // The prototype PTE is demand zero and copy on
        // write.  Make this PTE a private demand zero PTE.
        //

        ASSERT (Process != NULL);

        UNLOCK_PFN (OldIrql);

        NewPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

        ASSERT (RealPte.u.Hard.Valid == 0);

        if (RealPte.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) {

            //
            // Get the protection from the prototype PTE.
            //

            if (TempPte.u.Soft.Protection & MM_EXECUTE) {
                NewPte.u.Long = (MM_EXECUTE_READWRITE << MM_PROTECT_FIELD_SHIFT);
            }
        }
        else {

            //
            // The prototype PTE protection has been overridden by the
            // protection in the real PTE.
            //

            if (RealPte.u.Soft.Protection & MM_EXECUTE) {
                NewPte.u.Long = (MM_EXECUTE_READWRITE << MM_PROTECT_FIELD_SHIFT);
            }
        }

        MI_WRITE_INVALID_PTE (PointerPte, NewPte);

        status = MiResolveDemandZeroFault (FaultingAddress,
                                           PointerPte,
                                           Process,
                                           MM_NOIRQL);
        return status;
    }

    //
    // Make the prototype PTE valid, the prototype PTE is in
    // one of these 4 states:
    //
    //   demand zero
    //   transition
    //   paging file
    //   mapped file
    //

    if (TempPte.u.Soft.Prototype == 1) {

        //
        // Mapped File.
        //

        status = MiResolveMappedFileFault (PointerProtoPte,
                                           ReadBlock,
                                           Process,
                                           OldIrql);

        if (status == STATUS_ISSUE_PAGING_IO) {

            *CapturedPteContents = *PointerProtoPte;

            ASSERT (CapturedPteContents->u.Hard.Valid == 0);
            ASSERT (CapturedPteContents->u.Soft.Prototype == 0);
            ASSERT (CapturedPteContents->u.Soft.Transition == 1);
        }

        //
        // Returns with PFN lock held.
        //

        PfnHeld = TRUE;
    }
    else if (TempPte.u.Soft.Transition == 1) {

        //
        // Transition.
        //

        ASSERT (OldIrql != MM_NOIRQL);

        status = MiResolveTransitionFault (FaultingAddress,
                                           PointerProtoPte,
                                           Process,
                                           OldIrql,
                                           &CapturedEvent);
        //
        // Returns with PFN lock held.
        //

        PfnHeld = TRUE;
    }
    else if (TempPte.u.Soft.PageFileHigh == 0) {

        //
        // Demand Zero.
        //

        ASSERT (OldIrql != MM_NOIRQL);

        status = MiResolveDemandZeroFault (FaultingAddress,
                                           PointerProtoPte,
                                           Process,
                                           OldIrql);

        //
        // Returns with PFN lock held.
        //

        PfnHeld = TRUE;
    }
    else {

        //
        // Paging file.
        //

        status = MiResolvePageFileFault (FaultingAddress,
                                         PointerProtoPte,
                                         CapturedPteContents,
                                         ReadBlock,
                                         Process,
                                         OldIrql);

#if DBG
        if (status == STATUS_ISSUE_PAGING_IO) {
            ASSERT (CapturedPteContents->u.Hard.Valid == 0);
            ASSERT (CapturedPteContents->u.Soft.Prototype == 0);
            ASSERT (CapturedPteContents->u.Soft.Transition == 1);
        }
#endif

        //
        // Returns with PFN lock released.
        //

        ASSERT (KeAreAllApcsDisabled () == TRUE);
        PfnHeld = FALSE;
    }

    if (NT_SUCCESS (status)) {

        ASSERT (PointerPte->u.Hard.Valid == 0);

        status = MiCompleteProtoPteFault (StoreInstruction,
                                          FaultingAddress,
                                          PointerPte,
                                          PointerProtoPte,
                                          OldIrql,
                                          LockedProtoPfn);

        if (CapturedEvent != NULL) {
            MiFreeInPageSupportBlock (CapturedEvent);
        }
    }
    else {

        if (PfnHeld == TRUE) {
            UNLOCK_PFN (OldIrql);
        }

        ASSERT (KeAreAllApcsDisabled () == TRUE);

        if (CapturedEvent != NULL) {
            MiFreeInPageSupportBlock (CapturedEvent);
        }
    }

    return status;
}



NTSTATUS
MiCompleteProtoPteFault (
    IN ULONG_PTR StoreInstruction,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN KIRQL OldIrql,
    IN OUT PMMPFN *LockedProtoPfn
    )

/*++

Routine Description:

    This routine completes a prototype PTE fault.  It is invoked
    after a read operation has completed bringing the data into
    memory.

Arguments:

    StoreInstruction - Supplies nonzero if the instruction is trying
                       to modify the faulting address (i.e. write
                       access required).

    FaultingAddress - Supplies the faulting address.

    PointerPte - Supplies the PTE for the faulting address.

    PointerProtoPte - Supplies a pointer to the prototype PTE to fault in,
                      NULL if no prototype PTE exists.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at.

    LockedProtoPfn - Supplies a pointer to a non-NULL prototype PTE's PFN
                     pointer that was locked down by the caller, or a
                     pointer to NULL if the caller did not lock down any
                     PFN.  This routine may unlock the PFN - if so, it
                     must also clear this pointer.

Return Value:

    NTSTATUS.

Environment:

    Kernel mode, PFN lock held.

--*/
{
    NTSTATUS Status;
    ULONG FreeBit;
    MMPTE TempPte;
    MMPTE ProtoPteContents;
    MMPTE OriginalPte;
    MMWSLE ProtoProtect;
    LOGICAL SameProtectAsProto;
    ULONG MarkPageDirty;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE ContainingPageTablePointer;
    PFILE_OBJECT FileObject;
    LONGLONG FileOffset;
    PSUBSECTION Subsection;
    MMSECTION_FLAGS ControlAreaFlags;
    ULONG Flags;
    WSLE_NUMBER WorkingSetIndex;
    PEPROCESS CurrentProcess;
    MM_PROTECTION_MASK Protection;

    MM_PFN_LOCK_ASSERT();

    ASSERT (PointerProtoPte->u.Hard.Valid == 1);

    ProtoPteContents.u.Long = PointerProtoPte->u.Long;

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&ProtoPteContents);
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->u3.e1.PrototypePte = 1;

    //
    // Capture prefetch fault information.
    //

    OriginalPte = Pfn1->OriginalPte;

    //
    // Prototype PTE is now valid, make the PTE valid.
    //
    // A PTE just went from not present, not transition to
    // present.  The share count and valid count must be
    // updated in the page table page which contains this PTE.
    //

    ContainingPageTablePointer = MiGetPteAddress (PointerPte);
    Pfn2 = MI_PFN_ELEMENT (ContainingPageTablePointer->u.Hard.PageFrameNumber);
    Pfn2->u2.ShareCount += 1;

    ProtoProtect.u1.Long = 0;
    if (PointerPte->u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {

        //
        // The protection code for the real PTE comes from the real PTE as
        // it was placed there earlier during the handling of this fault.
        //

        ProtoProtect.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE (PointerPte);
        SameProtectAsProto = FALSE;
    }
    else {

        //
        // Use the protection in the prototype PTE to initialize the real PTE.
        //

        ProtoProtect.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE (&OriginalPte);
        SameProtectAsProto = TRUE;

        MI_ASSERT_NOT_SESSION_DATA (PointerPte);

        if ((StoreInstruction != 0) &&
            ((ProtoProtect.u1.e1.Protection & MM_PROTECTION_WRITE_MASK) == 0)) {

            //
            // This is the errant case where the user is trying to write
            // to a readonly subsection in the image.  Since we're more than
            // halfway through the fault, take the easy way to clean this up -
            // treat the access as a read for the rest of this trip through
            // the fault.  We'll then immediately refault when the instruction
            // is rerun (because it's really a write), and then we'll notice
            // that the user's PTE is not copy-on-write (or even writable!)
            // and return a clean access violation.
            //

            StoreInstruction = 0;
        }
    }

    MI_SNAP_DATA (Pfn1, PointerProtoPte, 0xD);

    MarkPageDirty = 0;

    //
    // If this is a store instruction and the page is not copy on
    // write, then set the modified bit in the PFN database and
    // the dirty bit in the PTE.  The PTE is not set dirty even
    // if the modified bit is set so writes to the page can be
    // tracked for FlushVirtualMemory.
    //

    if ((StoreInstruction != 0) &&
        ((ProtoProtect.u1.e1.Protection & MM_COPY_ON_WRITE_MASK) != MM_COPY_ON_WRITE_MASK)) {

        MarkPageDirty = 1;

#if DBG
        if (OriginalPte.u.Soft.Prototype == 1) {

            PCONTROL_AREA ControlArea;

            Subsection = MiGetSubsectionAddress (&OriginalPte);
            ControlArea = Subsection->ControlArea;

            if (ControlArea->DereferenceList.Flink != NULL) {
                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                    "MM: page fault completing to dereferenced CA %p %p %p\n",
                                ControlArea, Pfn1, PointerPte);
                DbgBreakPoint ();
            }
        }
#endif

        MI_SET_MODIFIED (Pfn1, 1, 0xA);

        if ((OriginalPte.u.Soft.Prototype == 0) &&
            (Pfn1->u3.e1.WriteInProgress == 0)) {

            FreeBit = GET_PAGING_FILE_OFFSET (OriginalPte);

            if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                MiReleaseConfirmedPageFileSpace (OriginalPte);
            }

            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
        }
    }

    if (*LockedProtoPfn != NULL) {

        //
        // Unlock page containing prototype PTEs.
        //
        // The reference count on the prototype PTE page will
        // always be greater than 1 if it is a genuine prototype
        // PTE pool allocation.  However, if it is a fork
        // prototype PTE allocation, it is possible the pool has
        // already been deallocated and in this case, the LockedProtoPfn
        // frame below will be in transition limbo with a share
        // count of 0 and a reference count of 1 awaiting our
        // final dereference below which will put it on the free list.
        //

        ASSERT ((*LockedProtoPfn)->u3.e2.ReferenceCount >= 1);
        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (*LockedProtoPfn);

        //
        // Tell our caller we've unlocked it for him.
        //

        *LockedProtoPfn = NULL;
    }

    UNLOCK_PFN (OldIrql);

    //
    // Ensure the user's attributes do not conflict with the PFN attributes.
    //

    Protection = (MM_PROTECTION_MASK) ProtoProtect.u1.e1.Protection;
    Protection &= ~(MM_NOCACHE | MM_WRITECOMBINE);

    if (Pfn1->u3.e1.CacheAttribute == MiCached) {
        NOTHING;
    }
    else if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {
        Protection |= MM_NOCACHE;
    }
    else if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
        Protection |= MM_WRITECOMBINE;
    }

    if (FaultingAddress < MmSystemRangeStart) {
        MI_MAKE_VALID_USER_PTE (TempPte,
                                PageFrameIndex,
                                Protection,
                                PointerPte);
    }
    else {

        //
        // Might be system or session, so go the long way to find out.
        //

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           Protection,
                           PointerPte);
    }

    if (MarkPageDirty != 0) {
        MI_SET_PTE_DIRTY (TempPte);
    }

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    if (SameProtectAsProto == TRUE) {
        ProtoProtect.u1.e1.Protection = MM_ZERO_ACCESS;
    }

    WorkingSetIndex = MiAddValidPageToWorkingSet (FaultingAddress,
                                                  PointerPte,
                                                  Pfn1,
                                                  (ULONG) ProtoProtect.u1.Long);

    if (WorkingSetIndex == 0) {

        if (SameProtectAsProto == FALSE) {

            //
            // The protection for the prototype PTE is in the WSLE.
            //

            ASSERT (ProtoProtect.u1.e1.Protection != MM_ZERO_ACCESS);

            TempPte.u.Long = 0;
            TempPte.u.Soft.Protection =
                MI_GET_PROTECTION_FROM_WSLE (&ProtoProtect);
            TempPte.u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;
        }
        else {

            //
            // The protection is in the prototype PTE.
            //

            TempPte.u.Long = MiProtoAddressForPte (Pfn1->PteAddress);
        }

        TempPte.u.Proto.Prototype = 1;

        //
        // Trim the page since we couldn't add it to the working
        // set list at this time.
        //

        if (FaultingAddress < MmSystemRangeStart) {
            CurrentProcess = PsGetCurrentProcess ();
        }
        else if ((MI_IS_SESSION_ADDRESS (FaultingAddress)) ||
                 (MI_IS_SESSION_PTE (FaultingAddress))) {
            CurrentProcess = HYDRA_PROCESS;
        }
        else {
            CurrentProcess = NULL;
        }

        MiTrimPte (FaultingAddress,
                   PointerPte,
                   Pfn1,
                   CurrentProcess,
                   TempPte);

        Status = STATUS_NO_MEMORY;
    }
    else {
        Status = STATUS_SUCCESS;
    }

    //
    // Log prefetch fault information now that the PFN lock has been released
    // and the PTE has been made valid.  This minimizes PFN lock contention,
    // allows CcPfLogPageFault to allocate (and fault on) pool, and allows other
    // threads in this process to execute without faulting on this address.
    //
    // Note that the process' working set mutex is still held so any other
    // faults or operations on user addresses by other threads in this process
    // will block for the duration of this call.
    //

    if ((CCPF_IS_PREFETCHER_ACTIVE()) && (OriginalPte.u.Soft.Prototype == 1)) {

        Subsection = MiGetSubsectionAddress (&OriginalPte);

        FileObject = Subsection->ControlArea->FilePointer;
        FileOffset = MiStartingOffset (Subsection, PointerProtoPte);
        ControlAreaFlags = Subsection->ControlArea->u.Flags;

        Flags = 0;
        if (ControlAreaFlags.Image) {
            Flags |= CCPF_TYPE_IMAGE;
        }
        if (ControlAreaFlags.Rom) {
            Flags |= CCPF_TYPE_ROM;
        }
        CcPfLogPageFault (FileObject, FileOffset, Flags);
    }

    ASSERT (PointerPte == MiGetPteAddress (FaultingAddress));

    return Status;
}


NTSTATUS
MiResolveMappedFileFault (
    IN PMMPTE PointerPte,
    OUT PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This routine builds the MDL and other structures to allow a
    read operation on a mapped file for a page fault.

Arguments:

    PointerPte - Supplies the PTE for the faulting address.

    ReadBlock - Supplies a pointer to put the address of the read block which
                needs to be completed before an I/O can be issued.

    Process - Supplies a pointer to the process object.  If this
              parameter is NULL, then the fault is for system
              space and the process's working set lock is not held.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at.

Return Value:

    status.  A status value of STATUS_ISSUE_PAGING_IO is returned
    if this function completes successfully.

Environment:

    Kernel mode, PFN lock held.

--*/

{
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PMDL Mdl;
    ULONG ReadSize;
    PETHREAD CurrentThread;
    PPFN_NUMBER Page;
    PPFN_NUMBER EndPage;
    PMMPTE BasePte;
    PMMPTE CheckPte;
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER TempOffset;
    PPFN_NUMBER FirstMdlPage;
    PMMINPAGE_SUPPORT ReadBlockLocal;
    ULONG PageColor;
    ULONG ClusterSize;
    PFN_NUMBER AvailablePages;
    NTSTATUS Status;
    PKPRCB Prcb;

    ClusterSize = 0;

    ASSERT (PointerPte->u.Soft.Prototype == 1);

    // *********************************************
    //   Mapped File (subsection format)
    // *********************************************

    if ((MmAvailablePages < MM_HIGH_LIMIT) &&
        (MiEnsureAvailablePageOrWait (Process, OldIrql))) {

        //
        // A wait operation was performed which dropped the locks,
        // repeat this fault.
        //

        return STATUS_REFAULT;
    }

    //
    // Calculate address of subsection for this prototype PTE.
    //

    Subsection = MiGetSubsectionAddress (PointerPte);

    ControlArea = Subsection->ControlArea;

    if (ControlArea->u.Flags.FailAllIo) {
        return STATUS_IN_PAGE_ERROR;
    }

    if (PointerPte >= &Subsection->SubsectionBase[Subsection->PtesInSubsection]) {

        //
        // Attempt to read past the end of this subsection.
        //

        return STATUS_ACCESS_VIOLATION;
    }

    ASSERT (ControlArea->u.Flags.Rom != 1);

    CurrentThread = PsGetCurrentThread ();

    ReadBlockLocal = MiGetInPageSupportBlock (OldIrql, &Status);

    if (ReadBlockLocal == NULL) {
        ASSERT (!NT_SUCCESS (Status));
        return Status;
    }

    *ReadBlock = ReadBlockLocal;

    //
    // Build an MDL for the request.
    //

    Mdl = &ReadBlockLocal->Mdl;

    FirstMdlPage = &ReadBlockLocal->Page[0];
    Page = FirstMdlPage;

#if DBG
    RtlFillMemoryUlong (Page, (MM_MAXIMUM_READ_CLUSTER_SIZE+1) * sizeof(PFN_NUMBER), 0xf1f1f1f1);
#endif //DBG

    ReadSize = PAGE_SIZE;
    BasePte = PointerPte;

    //
    // Should we attempt to perform page fault clustering?
    //

    AvailablePages = MmAvailablePages;

    if (MiInPageSinglePages != 0) {
        AvailablePages = 0;
        MiInPageSinglePages -= 1;
    }

    if ((!CurrentThread->DisablePageFaultClustering) &&
        (ControlArea->u.Flags.NoModifiedWriting == 0)) {

        if ((AvailablePages > (MmFreeGoal * 2))
                 ||
         (((ControlArea->u.Flags.Image != 0) ||
            (CurrentThread->ForwardClusterOnly)) &&
         (AvailablePages > MM_HIGH_LIMIT))) {

            //
            // Cluster up to n pages.  This one + n-1.
            //

            ASSERT (MM_HIGH_LIMIT > MM_MAXIMUM_READ_CLUSTER_SIZE + 16);
            ASSERT (AvailablePages > MM_MAXIMUM_READ_CLUSTER_SIZE + 16);

            if (ControlArea->u.Flags.Image == 0) {
                ASSERT (CurrentThread->ReadClusterSize <=
                            MM_MAXIMUM_READ_CLUSTER_SIZE);
                ClusterSize = CurrentThread->ReadClusterSize;
            }
            else {
                ClusterSize = MmDataClusterSize;
                if (Subsection->u.SubsectionFlags.Protection &
                                            MM_PROTECTION_EXECUTE_MASK ) {
                    ClusterSize = MmCodeClusterSize;
                }
            }
            EndPage = Page + ClusterSize;

            CheckPte = PointerPte + 1;

            //
            // Try to cluster within the page of PTEs.
            //

            while ((MiIsPteOnPdeBoundary(CheckPte) == 0) &&
               (Page < EndPage) &&
               (CheckPte <
                 &Subsection->SubsectionBase[Subsection->PtesInSubsection])
                      && (CheckPte->u.Long == BasePte->u.Long)) {

                ControlArea->NumberOfPfnReferences += 1;
                ReadSize += PAGE_SIZE;
                Page += 1;
                CheckPte += 1;
            }

            if ((Page < EndPage) && (!CurrentThread->ForwardClusterOnly)) {

                //
                // Attempt to cluster going backwards from the PTE.
                //

                CheckPte = PointerPte - 1;

                while ((((ULONG_PTR)CheckPte & (PAGE_SIZE - 1)) !=
                                            (PAGE_SIZE - sizeof(MMPTE))) &&
                        (Page < EndPage) &&
                         (CheckPte >= Subsection->SubsectionBase) &&
                         (CheckPte->u.Long == BasePte->u.Long)) {

                    ControlArea->NumberOfPfnReferences += 1;
                    ReadSize += PAGE_SIZE;
                    Page += 1;
                    CheckPte -= 1;
                }
                BasePte = CheckPte + 1;
            }
        }
    }

    //
    //
    // Calculate the offset to read into the file.
    //  offset = base + ((thispte - basepte) << PAGE_SHIFT)
    //

    StartingOffset.QuadPart = MiStartingOffset (Subsection, BasePte);

    TempOffset = MiEndingOffset (Subsection);

    ASSERT (StartingOffset.QuadPart < TempOffset.QuadPart);

    //
    // Remove pages to fill in the MDL.  This is done here as the
    // base PTE has been determined and can be used for virtual
    // aliasing checks.
    //

    EndPage = FirstMdlPage;
    CheckPte = BasePte;

    while (EndPage < Page) {
        if (Process == HYDRA_PROCESS) {
            PageColor = MI_GET_PAGE_COLOR_FROM_SESSION (MmSessionSpace);
        }
        else if (Process == NULL) {
            PageColor = MI_GET_PAGE_COLOR_FROM_PTE (CheckPte);
        }
        else {
            PageColor = MI_PAGE_COLOR_PTE_PROCESS (CheckPte,
                                                   &Process->NextPageColor);
        }
        *EndPage = MiRemoveAnyPage (PageColor);

        EndPage += 1;
        CheckPte += 1;
    }

    if (Process == HYDRA_PROCESS) {
        PageColor = MI_GET_PAGE_COLOR_FROM_SESSION (MmSessionSpace);
    }
    else if (Process == NULL) {
        PageColor = MI_GET_PAGE_COLOR_FROM_PTE (CheckPte);
    }
    else {
        PageColor = MI_PAGE_COLOR_PTE_PROCESS (CheckPte,
                                               &Process->NextPageColor);
    }

    //
    // Check to see if the read will go past the end of the file,
    // If so, correct the read size and get a zeroed page.
    //

    Prcb = KeGetCurrentPrcb ();
    InterlockedIncrement (&Prcb->MmPageReadIoCount);

    InterlockedExchangeAdd (&Prcb->MmPageReadCount,
                            (LONG) (ReadSize >> PAGE_SHIFT));

    if ((ControlArea->u.Flags.Image) &&
        (((UINT64)StartingOffset.QuadPart + ReadSize) > (UINT64)TempOffset.QuadPart)) {

        ASSERT ((ULONG)(TempOffset.QuadPart - StartingOffset.QuadPart)
                > (ReadSize - PAGE_SIZE));

        ReadSize = (ULONG)(TempOffset.QuadPart - StartingOffset.QuadPart);

        //
        // Round the offset to a 512-byte offset as this will help filesystems
        // optimize the transfer.  Note that filesystems will always zero fill
        // the remainder between VDL and the next 512-byte multiple and we have
        // already zeroed the whole page.
        //

        ReadSize = ((ReadSize + MMSECTOR_MASK) & ~MMSECTOR_MASK);

        PageFrameIndex = MiRemoveZeroPage (PageColor);
    }
    else {

        //
        // We are reading a complete page, no need to get a zeroed page.
        //

        PageFrameIndex = MiRemoveAnyPage (PageColor);
    }

    //
    // Increment the PFN reference count in the control area for
    // the subsection (the PFN lock is required to modify this field).
    //

    ControlArea->NumberOfPfnReferences += 1;
    *Page = PageFrameIndex;

    PageFrameIndex = *(FirstMdlPage + (PointerPte - BasePte));

    //
    // Get a page and put the PTE into the transition state with the
    // read-in-progress flag set.
    //

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    //
    // Initialize MDL for request.
    //

    MmInitializeMdl (Mdl,
                     MiGetVirtualAddressMappedByPte (BasePte),
                     ReadSize);
    Mdl->MdlFlags |= (MDL_PAGES_LOCKED | MDL_IO_PAGE_READ);

#if DBG
    if (ReadSize > ((ClusterSize + 1) << PAGE_SHIFT)) {
        KeBugCheckEx (MEMORY_MANAGEMENT, 0x777,(ULONG_PTR)Mdl, (ULONG_PTR)Subsection,
                        (ULONG)TempOffset.LowPart);
    }
#endif //DBG

    MiInitializeReadInProgressPfn (Mdl,
                                   BasePte,
                                   &ReadBlockLocal->Event,
                                   MI_PROTOTYPE_WSINDEX);

    MI_ZERO_USED_PAGETABLE_ENTRIES_IN_INPAGE_SUPPORT(ReadBlockLocal);

    ReadBlockLocal->ReadOffset = StartingOffset;
    ReadBlockLocal->FilePointer = ControlArea->FilePointer;
    ReadBlockLocal->BasePte = BasePte;
    ReadBlockLocal->Pfn = Pfn1;

    return STATUS_ISSUE_PAGING_IO;
}

NTSTATUS
MiWaitForInPageComplete (
    IN PMMPFN Pfn2,
    IN PMMPTE PointerPte,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPteContents,
    IN PMMINPAGE_SUPPORT InPageSupport,
    IN PEPROCESS CurrentProcess
    )

/*++

Routine Description:

    Waits for a page read to complete.

Arguments:

    Pfn - Supplies a pointer to the PFN element for the page being read.

    PointerPte - Supplies a pointer to the PTE that is in the transition
                 state.  This can be a prototype PTE address.

    FaultingAddress - Supplies the faulting address.

    PointerPteContents - Supplies the contents of the PTE before the
                         working set lock was released.

    InPageSupport - Supplies a pointer to the inpage support structure
                    for this read operation.

Return Value:

    Returns the status of the inpage operation.

    Note that the working set mutex and PFN lock are held upon return !!!

Environment:

    Kernel mode, APCs disabled.  Neither the working set lock nor
    the PFN lock may be held.

--*/

{
    PMMVAD ProtoVad;
    PMMPTE NewPointerPte;
    PMMPTE ProtoPte;
    PMMPFN Pfn1;
    PMMPFN Pfn;
    PULONG Va;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    ULONG Offset;
    ULONG Protection;
    PMDL Mdl;
    KIRQL OldIrql;
    NTSTATUS status;
    NTSTATUS status2;
    PEPROCESS Process;
    PETHREAD Thread;

    Thread = PsGetCurrentThread ();

    //
    // Wait for the I/O to complete.  Note that we can't wait for all
    // the objects simultaneously as other threads/processes could be
    // waiting for the same event.  The first thread which completes
    // the wait and gets the PFN lock may reuse the event for another
    // fault before this thread completes its wait.
    //

    KeWaitForSingleObject( &InPageSupport->Event,
                           WrPageIn,
                           KernelMode,
                           FALSE,
                           NULL);

    if (CurrentProcess == HYDRA_PROCESS) {
        LOCK_WORKING_SET (Thread, &MmSessionSpace->GlobalVirtualAddress->Vm);
    }
    else if (CurrentProcess == PREFETCH_PROCESS) {
        NOTHING;
    }
    else if (CurrentProcess != NULL) {
        LOCK_WS (Thread, CurrentProcess);
    }
    else {
        LOCK_SYSTEM_WS (Thread);
    }

    LOCK_PFN (OldIrql);

    ASSERT (Pfn2->u3.e2.ReferenceCount != 0);

    //
    // Check to see if this is the first thread to complete the in-page
    // operation.
    //

    Pfn = InPageSupport->Pfn;
    if (Pfn2 != Pfn) {
        ASSERT (Pfn2->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
        Pfn2->u3.e1.ReadInProgress = 0;
    }

    //
    // Another thread has already serviced the read, check the
    // io-error flag in the PFN database to ensure the in-page
    // was successful.
    //

    if (Pfn2->u4.InPageError == 1) {
        ASSERT (!NT_SUCCESS(Pfn2->u1.ReadStatus));

        if (MmIsRetryIoStatus(Pfn2->u1.ReadStatus)) {
            return STATUS_REFAULT;
        }
        return Pfn2->u1.ReadStatus;
    }

    if (InPageSupport->u1.e1.Completed == 0) {

        //
        // The ReadInProgress bit for the dummy page is constantly cleared
        // below as there are generally multiple inpage blocks pointing to
        // the same dummy page.
        //

        ASSERT ((Pfn->u3.e1.ReadInProgress == 1) ||
                (Pfn->PteAddress == MI_PF_DUMMY_PAGE_PTE));

        InPageSupport->u1.e1.Completed = 1;

        Mdl = &InPageSupport->Mdl;

        if (InPageSupport->u1.e1.PrefetchMdlHighBits != 0) {

            //
            // This is a prefetcher-issued read.
            //

            Mdl = MI_EXTRACT_PREFETCH_MDL (InPageSupport);
        }

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        ASSERT (Pfn->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

        Pfn->u3.e1.ReadInProgress = 0;
        Pfn->u1.Event = NULL;

#if defined (_WIN64)
        //
        // Page directory and page table pages are never clustered,
        // ensure this is never violated as only one UsedPageTableEntries
        // is kept in the inpage support block.
        //

        if (InPageSupport->UsedPageTableEntries) {
            Page = (PPFN_NUMBER)(Mdl + 1);
            LastPage = Page + ((Mdl->ByteCount - 1) >> PAGE_SHIFT);
            ASSERT (Page == LastPage);
        }
#endif

        MI_INSERT_USED_PAGETABLE_ENTRIES_IN_PFN(Pfn, InPageSupport);

        //
        // Check the IO_STATUS_BLOCK to ensure the in-page completed successfully.
        //

        if (!NT_SUCCESS(InPageSupport->IoStatus.Status)) {

            if (InPageSupport->IoStatus.Status == STATUS_END_OF_FILE) {

                //
                // An attempt was made to read past the end of file
                // zero all the remaining bytes in the read.
                //

                if (Pfn->OriginalPte.u.Soft.Prototype == 0) {

                    //
                    // Something is very wrong for this to happen on a pagefile
                    // read.
                    //

                    KeBugCheckEx (KERNEL_DATA_INPAGE_ERROR,
                                  3,
                                  (ULONG_PTR) FaultingAddress,
                                  (ULONG_PTR) InPageSupport,
                                  0);
                }

                Page = (PPFN_NUMBER)(Mdl + 1);
                LastPage = Page + ((Mdl->ByteCount - 1) >> PAGE_SHIFT);

                while (Page <= LastPage) {
                    MiZeroPhysicalPage (*Page);

                    MI_ZERO_USED_PAGETABLE_ENTRIES_IN_PFN(MI_PFN_ELEMENT(*Page));

                    Page += 1;
                }

            }
            else {

                //
                // In page io error occurred.
                //

                status = InPageSupport->IoStatus.Status;
                status2 = InPageSupport->IoStatus.Status;

                if (status != STATUS_VERIFY_REQUIRED) {

                    LOGICAL Retry;

                    Retry = FALSE;
#if DBG
                    DbgPrintEx (DPFLTR_MM_ID, DPFLTR_INFO_LEVEL, 
                                    "MM: inpage I/O error %X\n",
                                    InPageSupport->IoStatus.Status);
#endif

                    //
                    // If this page is for paged pool or for paged
                    // kernel code or page table pages, bugcheck.
                    //

                    if ((FaultingAddress > MM_HIGHEST_USER_ADDRESS) &&
                        (!MI_IS_SYSTEM_CACHE_ADDRESS(FaultingAddress))) {

                        if (MmIsRetryIoStatus(status)) {

                            if (MiInPageSinglePages == 0) {
                                MiInPageSinglePages = 30;
                            }

                            MiFaultRetries -= 1;
                            if (MiFaultRetries & MiFaultRetryMask) {
                                Retry = TRUE;
                            }
                        }

                        if (Retry == FALSE) {

                            ULONG_PTR PteContents;

                            //
                            // The prototype PTE resides in paged pool which may
                            // not be resident at this point.  Check first.
                            //

                            if (MiIsAddressValid (PointerPte, FALSE) == TRUE) {
                                PteContents = *(PULONG_PTR)PointerPte;
                            }
                            else {
                                PteContents = (ULONG_PTR)-1;
                            }

                            KeBugCheckEx (KERNEL_DATA_INPAGE_ERROR,
                                          (ULONG_PTR)PointerPte,
                                          status,
                                          (ULONG_PTR)FaultingAddress,
                                          PteContents);
                        }
                        status2 = STATUS_REFAULT;
                    }
                    else {

                        if (MmIsRetryIoStatus(status)) {

                            if (MiInPageSinglePages == 0) {
                                MiInPageSinglePages = 30;
                            }

                            MiUserFaultRetries -= 1;
                            if (MiUserFaultRetries & MiUserFaultRetryMask) {
                                Retry = TRUE;
                            }
                        }

                        if (Retry == TRUE) {
                            status2 = STATUS_REFAULT;
                        }
                    }
                }

                Page = (PPFN_NUMBER)(Mdl + 1);
                LastPage = Page + ((Mdl->ByteCount - 1) >> PAGE_SHIFT);

#if DBG
                Process = PsGetCurrentProcess ();
#endif
                while (Page <= LastPage) {
                    Pfn1 = MI_PFN_ELEMENT (*Page);
                    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
                    Pfn1->u4.InPageError = 1;
                    Pfn1->u1.ReadStatus = status;

#if DBG
                    Va = (PULONG)MiMapPageInHyperSpaceAtDpc (Process, *Page);
                    RtlFillMemoryUlong (Va, PAGE_SIZE, 0x50444142);
                    MiUnmapPageInHyperSpaceFromDpc (Process, Va);
#endif
                    Page += 1;
                }

                return status2;
            }
        }
        else {

            MiFaultRetries = 0;
            MiUserFaultRetries = 0;

            if (InPageSupport->IoStatus.Information != Mdl->ByteCount) {

                ASSERT (InPageSupport->IoStatus.Information != 0);

                //
                // Less than a full page was read - zero the remainder
                // of the page if the page is filesystem backed (this can
                // be legitimate as we are reading whole pages and the
                // end of file may not be page aligned.
                //
                // However, if the page is pagefile backed, then this
                // underreading is a fatal driver error - and it would
                // would be a mistake to zero trailing portions of pages
                // to hide that.
                //

                if (Pfn->OriginalPte.u.Soft.Prototype == 0) {
                    KeBugCheckEx (KERNEL_DATA_INPAGE_ERROR,
                                  4,
                                  (ULONG_PTR) FaultingAddress,
                                  (ULONG_PTR) InPageSupport,
                                  0);
                }

                Page = (PPFN_NUMBER)(Mdl + 1);
                LastPage = Page + ((Mdl->ByteCount - 1) >> PAGE_SHIFT);
                Page += ((InPageSupport->IoStatus.Information - 1) >> PAGE_SHIFT);

                Offset = BYTE_OFFSET (InPageSupport->IoStatus.Information);

                if (Offset != 0) {
                    Process = PsGetCurrentProcess ();
                    Va = (PULONG)((PCHAR)MiMapPageInHyperSpaceAtDpc (Process, *Page)
                                + Offset);

                    RtlZeroMemory (Va, PAGE_SIZE - Offset);
                    MiUnmapPageInHyperSpaceFromDpc (Process, Va);
                }

                //
                // Zero any remaining pages within the MDL.
                //

                Page += 1;

                while (Page <= LastPage) {
                    MiZeroPhysicalPage (*Page);
                    Page += 1;
                }
            }

            //
            // If any filesystem return non-zeroed data for any slop
            // after the VDL but before the next 512-byte offset then this
            // non-zeroed data will overwrite our zeroed page.  This would
            // need to be checked for and cleaned up here.  Note that the only
            // reason Mm even rounds the MDL request up to a 512-byte offset
            // is so filesystems receive a transfer they can handle optimally,
            // but any transfer size has always worked (although non-512 byte
            // multiples end up getting posted by the filesystem).
            //
        }
    }

    //
    // Prefetcher-issued reads only put prototype PTEs into transition and
    // never fill actual hardware PTEs so these can be returned now.
    //

    if (CurrentProcess == PREFETCH_PROCESS) {
        return STATUS_SUCCESS;
    }

    //
    // Check to see if the faulting PTE has changed.
    //

    NewPointerPte = MiFindActualFaultingPte (FaultingAddress);

    //
    // If this PTE is in prototype PTE format, make the pointer to the
    // PTE point to the prototype PTE.
    //

    if (NewPointerPte == NULL) {
        return STATUS_PTE_CHANGED;
    }

    if (NewPointerPte != PointerPte) {

        //
        // Check to make sure the NewPointerPte is not a prototype PTE
        // which refers to the page being made valid.
        //

        if (NewPointerPte->u.Soft.Prototype == 1) {
            if (NewPointerPte->u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {

                ProtoPte = MiCheckVirtualAddress (FaultingAddress,
                                                  &Protection,
                                                  &ProtoVad);

            }
            else {
                ProtoPte = MiPteToProto (NewPointerPte);
            }

            //
            // Make sure the prototype PTE refers to the PTE made valid.
            //

            if (ProtoPte != PointerPte) {
                return STATUS_PTE_CHANGED;
            }

            //
            // If the only difference is the owner mask, everything is okay.
            //

            if (ProtoPte->u.Long != PointerPteContents->u.Long) {
                return STATUS_PTE_CHANGED;
            }
        }
        else {
            return STATUS_PTE_CHANGED;
        }
    }
    else {

        if (NewPointerPte->u.Long != PointerPteContents->u.Long) {
            return STATUS_PTE_CHANGED;
        }
    }
    return STATUS_SUCCESS;
}

PMMPTE
MiFindActualFaultingPte (
    IN PVOID FaultingAddress
    )

/*++

Routine Description:

    This routine locates the actual PTE which must be made resident in order
    to complete this fault.  Note that for certain cases multiple faults
    are required to make the final page resident.

Arguments:

    FaultingAddress - Supplies the virtual address which caused the fault.

Return Value:

    The PTE to be made valid to finish the fault, NULL if the fault should
    be retried.

Environment:

    Kernel mode, APCs disabled, working set mutex held.

--*/

{
    PMMVAD ProtoVad;
    PMMPTE ProtoPteAddress;
    PMMPTE PointerPte;
    PMMPTE PointerFaultingPte;
    ULONG Protection;

    if (MI_IS_PHYSICAL_ADDRESS (FaultingAddress)) {
        return NULL;
    }

#if (_MI_PAGING_LEVELS >= 4)

    PointerPte = MiGetPxeAddress (FaultingAddress);

    if (PointerPte->u.Hard.Valid == 0) {

        //
        // Page directory parent page is not valid.
        //

        return PointerPte;
    }

#endif

#if (_MI_PAGING_LEVELS >= 3)

    PointerPte = MiGetPpeAddress (FaultingAddress);

    if (PointerPte->u.Hard.Valid == 0) {

        //
        // Page directory page is not valid.
        //

        return PointerPte;
    }

#endif

    PointerPte = MiGetPdeAddress (FaultingAddress);

    if (PointerPte->u.Hard.Valid == 0) {

        //
        // Page table page is not valid.
        //

        return PointerPte;
    }

    PointerPte = MiGetPteAddress (FaultingAddress);

    if (PointerPte->u.Hard.Valid == 1) {

        //
        // Page is already valid, no need to fault it in.
        //

        return NULL;
    }

    if (PointerPte->u.Soft.Prototype == 0) {

        //
        // Page is not a prototype PTE, make this PTE valid.
        //

        return PointerPte;
    }

    //
    // Check to see if the PTE which maps the prototype PTE is valid.
    //

    if (PointerPte->u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {

        //
        // Protection is here, PTE must be located in VAD.
        //

        ProtoPteAddress = MiCheckVirtualAddress (FaultingAddress,
                                                 &Protection,
                                                 &ProtoVad);

        if (ProtoPteAddress == NULL) {

            //
            // No prototype PTE means another thread has deleted the VAD while
            // this thread waited for the inpage to complete.  Certainly NULL
            // must be returned so a stale PTE is not modified - the instruction
            // will then be reexecuted and an access violation delivered.
            //

            return NULL;
        }

    }
    else {

        //
        // Protection is in ProtoPte.
        //

        ProtoPteAddress = MiPteToProto (PointerPte);
    }

    PointerFaultingPte = MiFindActualFaultingPte (ProtoPteAddress);

    if (PointerFaultingPte == NULL) {
        return PointerPte;
    }

    return PointerFaultingPte;
}

PMMPTE
MiCheckVirtualAddress (
    IN PVOID VirtualAddress,
    OUT PULONG ProtectCode,
    OUT PMMVAD *VadOut
    )

/*++

Routine Description:

    This function examines the virtual address descriptors to see
    if the specified virtual address is contained within any of
    the descriptors.  If a virtual address descriptor is found
    which contains the specified virtual address, a PTE is built
    from information within the virtual address descriptor and
    returned to the caller.

Arguments:

    VirtualAddress - Supplies the virtual address to locate within
                     a virtual address descriptor.

    ProtectCode - Supplies a pointer to a variable that will receive the
                  protection to insert the actual PTE.

    Vad - Supplies a pointer to a variable that will receive the pointer
          to the VAD that was used for validation (or NULL if no VAD was
          used).

Return Value:

    Returns the PTE which corresponds to the supplied virtual address.
    If no virtual address descriptor is found, a zero PTE is returned.

Environment:

    Kernel mode, APCs disabled, working set mutex held.

--*/

{
    PMMVAD Vad;
    PMMPTE PointerPte;
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;

    *VadOut = NULL;

    if (VirtualAddress <= MM_HIGHEST_USER_ADDRESS) {

        if (PAGE_ALIGN(VirtualAddress) == (PVOID) MM_SHARED_USER_DATA_VA) {

            //
            // This is the page that is double mapped between
            // user mode and kernel mode.  Map it as read only.
            //

            *ProtectCode = MM_READONLY;

            return MmSharedUserDataPte;
        }

        Vad = MiLocateAddress (VirtualAddress);

        if (Vad == NULL) {
            *ProtectCode = MM_NOACCESS;
            return NULL;
        }

        //
        // A virtual address descriptor which contains the virtual address
        // has been located.  Build the PTE from the information within
        // the virtual address descriptor.
        //

        if (Vad->u.VadFlags.VadType == VadDevicePhysicalMemory) {

            //
            // This is definitely a banked section.
            //

            MiHandleBankedSection (VirtualAddress, Vad);
            *ProtectCode = MM_NOACCESS;
            return NULL;
        }

        if (Vad->u.VadFlags.PrivateMemory == 1) {

            //
            // This is a private region of memory.  Check to make
            // sure the virtual address has been committed.  Note that
            // addresses are dense from the bottom up.
            //

            if (Vad->u.VadFlags.VadType == VadAwe) {

                //
                // These mappings only fault if the access is bad.
                //

                *ProtectCode = MM_NOACCESS;
                return NULL;
            }

            if (Vad->u.VadFlags.MemCommit == 1) {
                *ProtectCode = MI_GET_PROTECTION_FROM_VAD(Vad);
                return NULL;
            }

            //
            // The address is reserved but not committed.
            //

            *ProtectCode = MM_NOACCESS;
            return NULL;
        }
        else {

            //
            // This virtual address descriptor refers to a
            // section, calculate the address of the prototype PTE
            // and construct a pointer to the PTE.
            //
            //*******************************************************
            //*******************************************************
            // well here's an interesting problem, how do we know
            // how to set the attributes on the PTE we are creating
            // when we can't look at the prototype PTE without
            // potentially incurring a page fault.  In this case
            // PteTemplate would be zero.
            //*******************************************************
            //*******************************************************
            //

            if (Vad->u.VadFlags.VadType == VadImageMap) {

                //
                // PTE and proto PTEs have the same protection for images.
                //

                *ProtectCode = MM_UNKNOWN_PROTECTION;
            }
            else {
                *ProtectCode = MI_GET_PROTECTION_FROM_VAD(Vad);

                //
                // Opportunistic clustering can use the identical protections
                // so give our caller the green light.
                //

                if (Vad->u2.VadFlags2.ExtendableFile == 0) {
                    *VadOut = Vad;
                }
            }
            PointerPte = (PMMPTE)MiGetProtoPteAddress(Vad,
                                                MI_VA_TO_VPN (VirtualAddress));
            if (PointerPte == NULL) {
                *ProtectCode = MM_NOACCESS;
            }
            if (Vad->u2.VadFlags2.ExtendableFile) {

                //
                // Make sure the data has been committed.
                //

                if ((MI_VA_TO_VPN (VirtualAddress) - Vad->StartingVpn) >
                    (ULONG_PTR)((((PMMVAD_LONG)Vad)->u4.ExtendedInfo->CommittedSize - 1)
                                                 >> PAGE_SHIFT)) {
                    *ProtectCode = MM_NOACCESS;
                }
            }
            return PointerPte;
        }

    }
    else if (MI_IS_PAGE_TABLE_ADDRESS(VirtualAddress)) {

        //
        // The virtual address is within the space occupied by PDEs,
        // make the PDE valid.
        //

        if (((PMMPTE)VirtualAddress >= MiGetPteAddress (MM_PAGED_POOL_START)) &&
            ((PMMPTE)VirtualAddress <= MmPagedPoolInfo.LastPteForPagedPool)) {

            *ProtectCode = MM_NOACCESS;
            return NULL;
        }

        *ProtectCode = MM_READWRITE;
        return NULL;
    }
    else if (MI_IS_SESSION_ADDRESS (VirtualAddress) == TRUE) {

        //
        // See if the session space address is copy on write.
        //

        MM_SESSION_SPACE_WS_LOCK_ASSERT ();

        PointerPte = NULL;
        *ProtectCode = MM_NOACCESS;

        NextEntry = MmSessionSpace->ImageList.Flink;

        while (NextEntry != &MmSessionSpace->ImageList) {

            Image = CONTAINING_RECORD(NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

            if ((VirtualAddress >= Image->Address) && (VirtualAddress <= Image->LastAddress)) {
                PointerPte = Image->PrototypePtes +
                    (((PCHAR)VirtualAddress - (PCHAR)Image->Address) >> PAGE_SHIFT);
                *ProtectCode = MM_EXECUTE_WRITECOPY;
                break;
            }

            NextEntry = NextEntry->Flink;
        }

        return PointerPte;
    }

    //
    // Address is in system space.
    //

    *ProtectCode = MM_NOACCESS;
    return NULL;
}

#if (_MI_PAGING_LEVELS < 3)

NTSTATUS
FASTCALL
MiCheckPdeForPagedPool (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This function copies the Page Table Entry for the corresponding
    virtual address from the system process's page directory.

    This allows page table pages to be lazily evaluated for things
    like paged pool and per-session mappings.

Arguments:

    VirtualAddress - Supplies the virtual address in question.

Return Value:

    Either success or access violation.

Environment:

    Kernel mode, DISPATCH level or below.

--*/

{
    PMMPTE PointerPde;
    NTSTATUS status;

    if (MI_IS_SESSION_ADDRESS (VirtualAddress) == TRUE) {

         //
         // Virtual address in the session space range.
         //

         return MiCheckPdeForSessionSpace (VirtualAddress);
    }

    if (MI_IS_SESSION_PTE (VirtualAddress) == TRUE) {

         //
         // PTE for the session space range.
         //

         return MiCheckPdeForSessionSpace (VirtualAddress);
    }

    status = STATUS_SUCCESS;

    if (MI_IS_KERNEL_PAGE_TABLE_ADDRESS(VirtualAddress)) {

        //
        // PTE for paged pool.
        //

        PointerPde = MiGetPteAddress (VirtualAddress);
        status = STATUS_WAIT_1;
    }
    else if (VirtualAddress < MmSystemRangeStart) {

        return STATUS_ACCESS_VIOLATION;

    }
    else {

        //
        // Virtual address in paged pool range.
        //

        PointerPde = MiGetPdeAddress (VirtualAddress);
    }

    //
    // Locate the PDE for this page and make it valid.
    //

    if (PointerPde->u.Hard.Valid == 0) {

        //
        // The MI_WRITE_VALID_PTE macro cannot be used here because
        // its ASSERTs could mistakenly fire as multiple processors
        // may execute the instruction below without synchronization.
        //

        InterlockedExchangePte (PointerPde,
                        MmSystemPagePtes [((ULONG_PTR)PointerPde &
                               (PD_PER_SYSTEM * (sizeof(MMPTE) * PDE_PER_PAGE) - 1)) / sizeof(MMPTE)].u.Long);
    }
    return status;
}


NTSTATUS
FASTCALL
MiCheckPdeForSessionSpace (
    IN PVOID VirtualAddress
    )

/*++

Routine Description:

    This function copies the Page Table Entry for the corresponding
    session virtual address from the current session's data structures.

    This allows page table pages to be lazily evaluated for session mappings.
    The caller must check for the current process having a session space.

Arguments:

    VirtualAddress - Supplies the virtual address in question.

Return Value:

    STATUS_WAIT_1 - The mapping has been made valid, retry the fault.

    STATUS_SUCCESS - Did not handle the fault, continue further processing.

    !STATUS_SUCCESS - An access violation has occurred - raise an exception.

Environment:

    Kernel mode, DISPATCH level or below.

--*/

{
    MMPTE TempPde;
    PMMPTE PointerPde;
    PVOID  SessionVirtualAddress;
    ULONG  Index;

    //
    // First check whether the reference was to a page table page which maps
    // session space.  If so, the PDE is retrieved from the session space
    // data structure and made valid.
    //

    if (MI_IS_SESSION_PTE (VirtualAddress) == TRUE) {

        //
        // Verify that the current process has a session space.
        //

        PointerPde = MiGetPdeAddress (MmSessionSpace);

        if (PointerPde->u.Hard.Valid == 0) {

#if DBG
            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
                "MiCheckPdeForSessionSpace: No current session for PTE %p\n",
                VirtualAddress);

            DbgBreakPoint ();
#endif
            return STATUS_ACCESS_VIOLATION;
        }

        SessionVirtualAddress = MiGetVirtualAddressMappedByPte ((PMMPTE) VirtualAddress);

        PointerPde = MiGetPteAddress (VirtualAddress);

        if (PointerPde->u.Hard.Valid == 1) {

            //
            // The PDE is already valid - another thread must have
            // won the race.  Just return.
            //

            return STATUS_WAIT_1;
        }

        //
        // Calculate the session space PDE index and load the
        // PDE from the session space table for this session.
        //

        Index = MiGetPdeSessionIndex (SessionVirtualAddress);

        TempPde.u.Long = MmSessionSpace->PageTables[Index].u.Long;

        if (TempPde.u.Hard.Valid == 1) {

            //
            // The MI_WRITE_VALID_PTE macro cannot be used here because
            // its ASSERTs could mistakenly fire as multiple processors
            // may execute the instruction below without synchronization.
            //

            InterlockedExchangePte (PointerPde, TempPde.u.Long);
            return STATUS_WAIT_1;
        }

#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MiCheckPdeForSessionSpace: No Session PDE for PTE %p, %p\n",
            PointerPde->u.Long, SessionVirtualAddress);

        DbgBreakPoint ();
#endif
        return STATUS_ACCESS_VIOLATION;
    }

    if (MI_IS_SESSION_ADDRESS (VirtualAddress) == FALSE) {

        //
        // Not a session space fault - tell the caller to try other handlers.
        //

        return STATUS_SUCCESS;
    }

    //
    // Handle PDE faults for references in the session space.
    // Verify that the current process has a session space.
    //

    PointerPde = MiGetPdeAddress (MmSessionSpace);

    if (PointerPde->u.Hard.Valid == 0) {

#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MiCheckPdeForSessionSpace: No current session for VA %p\n",
            VirtualAddress);

        DbgBreakPoint ();
#endif
        return STATUS_ACCESS_VIOLATION;
    }

    PointerPde = MiGetPdeAddress (VirtualAddress);

    if (PointerPde->u.Hard.Valid == 0) {

        //
        // Calculate the session space PDE index and load the
        // PDE from the session space table for this session.
        //

        Index = MiGetPdeSessionIndex (VirtualAddress);

        PointerPde->u.Long = MmSessionSpace->PageTables[Index].u.Long;

        if (PointerPde->u.Hard.Valid == 1) {
            return STATUS_WAIT_1;
        }

#if DBG
        DbgPrintEx (DPFLTR_MM_ID, DPFLTR_ERROR_LEVEL, 
            "MiCheckPdeForSessionSpace: No Session PDE for VA %p, %p\n",
            PointerPde->u.Long, VirtualAddress);

        DbgBreakPoint ();
#endif

        return STATUS_ACCESS_VIOLATION;
    }

    //
    // Tell the caller to continue with other fault handlers.
    //

    return STATUS_SUCCESS;
}
#endif


VOID
MiInitializePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN ULONG ModifiedState
    )

/*++

Routine Description:

    This function initializes the specified PFN element to the
    active and valid state.

Arguments:

    PageFrameIndex - Supplies the page frame number to initialize.

    PointerPte - Supplies the pointer to the PTE which caused the page fault.

    ModifiedState - Supplies the state to set the modified field in the PFN
                    element for this page, either 0 or 1.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, PFN lock held.

--*/

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    MM_PROTECTION_MASK Protection;
    MI_PFN_CACHE_ATTRIBUTE NewCacheAttribute;

    MM_PFN_LOCK_ASSERT();

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->PteAddress = PointerPte;
    NewCacheAttribute = MiCached;

    //
    // If the PTE is currently valid, an address space is being built,
    // just make the original PTE demand zero.
    //

    if (PointerPte->u.Hard.Valid == 1) {

        //
        // The only time this should happen is for the initial process pages
        // created (and initialized) out of context.  They must be in order
        // to attach and then initialize the process, and
        // MmCreateProcessAddressSpace assumes responsibility for detecting
        // and preventing TB attribute conflicts for this case, so we don't
        // need to here.
        //

        ASSERT (PsGetCurrentProcess()->Vm.WorkingSetSize == 0);

        ASSERT (!MI_IS_WRITE_COMBINE_ENABLED (PointerPte));
        ASSERT (!MI_IS_CACHING_DISABLED (PointerPte));
        ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

#if defined (_AMD64_)
        if (PointerPte->u.Hard.NoExecute == 0) {
            Pfn1->OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
        }
#endif

#if defined(_X86PAE_)
        if (MmPaeMask != 0) {
            if ((PointerPte->u.Long & MmPaeMask) == 0) {
                Pfn1->OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
            }
        }
#endif
    }
    else {

        Pfn1->OriginalPte = *PointerPte;

        ASSERT (!((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                    (Pfn1->OriginalPte.u.Soft.Transition == 1)));

        Protection = (MM_PROTECTION_MASK) PointerPte->u.Soft.Protection;

        if (MI_IS_WRITECOMBINE (Protection)) {
            NewCacheAttribute = MI_TRANSLATE_CACHETYPE (MiWriteCombined, 0);
        }
        else if (MI_IS_NOCACHE (Protection)) {
            NewCacheAttribute = MI_TRANSLATE_CACHETYPE (MiNonCached, 0);
        }

        if (Pfn1->u3.e1.CacheAttribute != NewCacheAttribute) {
            MI_FLUSH_TB_FOR_INDIVIDUAL_ATTRIBUTE_CHANGE (PageFrameIndex,
                                                         NewCacheAttribute);
            Pfn1->u3.e1.CacheAttribute = NewCacheAttribute;
        }
    }

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount += 1;

    Pfn1->u2.ShareCount += 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;

    if (ModifiedState == 1) {
        MI_SET_MODIFIED (Pfn1, 1, 0xB);
    }
    else {
        MI_SET_MODIFIED (Pfn1, 0, 0x26);
    }

#if defined (_WIN64)
    Pfn1->UsedPageTableEntries = 0;
#endif

    //
    // Determine the page frame number of the page table page which
    // contains this PTE.
    //

    PteFramePointer = MiGetPteAddress (PointerPte);
    if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)PteFramePointer->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }
    PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    ASSERT (PteFramePage != 0);
    Pfn1->u4.PteFrame = PteFramePage;

    //
    // Increment the share count for the page table page containing
    // this PTE.
    //

    Pfn2 = MI_PFN_ELEMENT (PteFramePage);

    Pfn2->u2.ShareCount += 1;

    return;
}

VOID
MiInitializePfnAndMakePteValid (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN MMPTE NewPteContents
    )

/*++

Routine Description:

    This function initializes the specified PFN element to the
    active and valid state.  After initializing the PFN, the PTE is
    then immediately made valid.  Note the order is important because
    the TB may need to be flushed to avoid a prior stale system PTE
    mapping (these are lazy-flushed) from causing a TB attribute conflict.

Arguments:

    PageFrameIndex - Supplies the page frame number to initialize.

    PointerPte - Supplies the pointer to the PTE which caused the page fault.

    NewPteContents - Supplies the contents to insert in the PTE.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, PFN lock held.

--*/

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    MI_PFN_CACHE_ATTRIBUTE NewCacheAttribute;

    MM_PFN_LOCK_ASSERT();

    ASSERT (PointerPte->u.Hard.Valid == 0);

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->PteAddress = PointerPte;

    //
    // An address space is being built, just make the original PTE demand zero.
    //

    Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

#if defined (_AMD64_)
    if (NewPteContents.u.Hard.NoExecute == 0) {
        Pfn1->OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
    }
#endif

#if defined(_X86PAE_)
    if (MmPaeMask != 0) {
        if ((NewPteContents.u.Long & MmPaeMask) == 0) {
            Pfn1->OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
        }
    }
#endif

    NewCacheAttribute = MiCached;

    if (MI_IS_WRITE_COMBINE_ENABLED (&NewPteContents)) {
        Pfn1->OriginalPte.u.Soft.Protection |= MM_WRITECOMBINE;
        NewCacheAttribute = MI_TRANSLATE_CACHETYPE (MiWriteCombined, 0);
    }
    else if (MI_IS_CACHING_DISABLED (&NewPteContents)) {
        Pfn1->OriginalPte.u.Soft.Protection |= MM_NOCACHE;
        NewCacheAttribute = MI_TRANSLATE_CACHETYPE (MiNonCached, 0);
    }

    //
    // Flush the TB if the attribute for this PFN is changing.
    //

    if (Pfn1->u3.e1.CacheAttribute != NewCacheAttribute) {
        MI_FLUSH_TB_FOR_INDIVIDUAL_ATTRIBUTE_CHANGE (PageFrameIndex,
                                                     NewCacheAttribute);
        Pfn1->u3.e1.CacheAttribute = NewCacheAttribute;
    }

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount += 1;

    Pfn1->u2.ShareCount += 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;

    MI_SET_MODIFIED (Pfn1, 1, 0xB);

#if defined (_WIN64)
    Pfn1->UsedPageTableEntries = 0;
#endif

    //
    // Determine the page frame number of the page table page which
    // contains this PTE.
    //

    PteFramePointer = MiGetPteAddress (PointerPte);
    if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)PteFramePointer->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }
    PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    ASSERT (PteFramePage != 0);
    Pfn1->u4.PteFrame = PteFramePage;

    //
    // Increment the share count for the page table page containing
    // this PTE.
    //

    Pfn2 = MI_PFN_ELEMENT (PteFramePage);

    Pfn2->u2.ShareCount += 1;

    MI_WRITE_VALID_PTE (PointerPte, NewPteContents);

    return;
}

VOID
MiInitializeReadInProgressSinglePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    )

/*++

Routine Description:

    This function initializes the specified PFN element to the
    transition / read-in-progress state for an in-page operation.

Arguments:

    PageFrameIndex - Supplies the page frame to initialize.

    BasePte - Supplies the pointer to the PTE for the page frame.

    Event - Supplies the event which is to be set when the I/O operation
            completes.

    WorkingSetIndex - Supplies the working set index flag, a value of
                      -1 indicates no WSLE is required because
                      this is a prototype PTE.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, PFN lock held.

--*/

{
    PMMPFN Pfn1;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    MMPTE TempPte;
    MM_PROTECTION_MASK Protection;
    MI_PFN_CACHE_ATTRIBUTE NewCacheAttribute;

    MM_PFN_LOCK_ASSERT();

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->u1.Event = Event;
    Pfn1->PteAddress = BasePte;
    Pfn1->OriginalPte = *BasePte;
    if (WorkingSetIndex == MI_PROTOTYPE_WSINDEX) {
        Pfn1->u3.e1.PrototypePte = 1;
    }

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1);

    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e1.ReadInProgress = 1;

    Protection = (MM_PROTECTION_MASK) BasePte->u.Soft.Protection;

    NewCacheAttribute = MiCached;

    if (MI_IS_WRITECOMBINE (Protection)) {
        NewCacheAttribute = MI_TRANSLATE_CACHETYPE (MiWriteCombined, 0);
    }
    else if (MI_IS_NOCACHE (Protection)) {
        NewCacheAttribute = MI_TRANSLATE_CACHETYPE (MiNonCached, 0);
    }

    if (Pfn1->u3.e1.CacheAttribute != NewCacheAttribute) {
        MI_FLUSH_TB_FOR_INDIVIDUAL_ATTRIBUTE_CHANGE (PageFrameIndex,
                                                     NewCacheAttribute);
        Pfn1->u3.e1.CacheAttribute = NewCacheAttribute;
    }

    Pfn1->u4.InPageError = 0;

    //
    // Determine the page frame number of the page table page which
    // contains this PTE.
    //

    PteFramePointer = MiGetPteAddress (BasePte);
    if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (BasePte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940,
                          (ULONG_PTR)BasePte,
                          (ULONG_PTR)PteFramePointer->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(BasePte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }

    PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    Pfn1->u4.PteFrame = PteFramePage;

    //
    // Put the PTE into the transition state, no cache flush needed as
    // PTE is still not valid.
    //

    MI_MAKE_TRANSITION_PTE (TempPte,
                            PageFrameIndex,
                            BasePte->u.Soft.Protection,
                            BasePte);

    MI_WRITE_INVALID_PTE (BasePte, TempPte);

    //
    // Increment the share count for the page table page containing
    // this PTE as the PTE just went into the transition state.
    //

    ASSERT (PteFramePage != 0);

    Pfn1 = MI_PFN_ELEMENT (PteFramePage);
    Pfn1->u2.ShareCount += 1;

    return;
}

VOID
MiInitializeReadInProgressPfn (
    IN PMDL Mdl,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    )

/*++

Routine Description:

    This function initializes the specified PFN element to the
    transition / read-in-progress state for an in-page operation.


Arguments:

    Mdl - Supplies a pointer to the MDL.

    BasePte - Supplies the pointer to the PTE which the first page in
              the MDL maps.

    Event - Supplies the event which is to be set when the I/O operation
            completes.

    WorkingSetIndex - Supplies the working set index flag, a value of
                      -1 indicates no WSLE is required because
                      this is a prototype PTE.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, PFN lock held.

--*/

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    MMPTE TempPte;
    LONG NumberOfBytes;
    PPFN_NUMBER Page;
    MM_PROTECTION_MASK Protection;
    MI_PFN_CACHE_ATTRIBUTE NewCacheAttribute;
    LOGICAL TbFlushNeeded;

    TbFlushNeeded = FALSE;

    PteFramePage = 0;
    PteFramePointer = 0;
    SATISFY_OVERZEALOUS_COMPILER (NewCacheAttribute = MiCached);

    MM_PFN_LOCK_ASSERT();

    Page = (PPFN_NUMBER)(Mdl + 1);

    NumberOfBytes = Mdl->ByteCount;

    while (NumberOfBytes > 0) {

        Pfn1 = MI_PFN_ELEMENT (*Page);
        Pfn1->u1.Event = Event;
        Pfn1->PteAddress = BasePte;
        Pfn1->OriginalPte = *BasePte;
        if (WorkingSetIndex == MI_PROTOTYPE_WSINDEX) {
            Pfn1->u3.e1.PrototypePte = 1;
        }
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
        MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1);

        Pfn1->u2.ShareCount = 0;
        Pfn1->u3.e1.ReadInProgress = 1;

        Protection = (MM_PROTECTION_MASK) BasePte->u.Soft.Protection;

        NewCacheAttribute = MiCached;

        if (MI_IS_WRITECOMBINE (Protection)) {
            NewCacheAttribute = MI_TRANSLATE_CACHETYPE (MiWriteCombined, 0);
        }
        else if (MI_IS_NOCACHE (Protection)) {
            NewCacheAttribute = MI_TRANSLATE_CACHETYPE (MiNonCached, 0);
        }

        if (Pfn1->u3.e1.CacheAttribute != NewCacheAttribute) {
            TbFlushNeeded = TRUE;
            Pfn1->u3.e1.CacheAttribute = NewCacheAttribute;
        }

        Pfn1->u4.InPageError = 0;

        if ((PteFramePage == 0) || (MiIsPteOnPdeBoundary (BasePte))) {

            //
            // Determine the page frame number of the page table page which
            // contains this PTE.
            //

            if (PteFramePage == 0) {
                PteFramePointer = MiGetPteAddress (BasePte);
            }
            else {
                PteFramePointer += 1;
                ASSERT (PteFramePointer == MiGetPteAddress (BasePte));
            }

            if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
                if (!NT_SUCCESS(MiCheckPdeForPagedPool (BasePte))) {
#endif
                    KeBugCheckEx (MEMORY_MANAGEMENT,
                                  0x61940,
                                  (ULONG_PTR)BasePte,
                                  (ULONG_PTR)PteFramePointer->u.Long,
                                  (ULONG_PTR)MiGetVirtualAddressMappedByPte(BasePte));
#if (_MI_PAGING_LEVELS < 3)
                }
#endif
            }

            PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
            ASSERT (PteFramePage != 0);
        }

        Pfn1->u4.PteFrame = PteFramePage;

        //
        // Put the PTE into the transition state, no TB flush needed as the
        // existing PTE is not valid.
        //

        MI_MAKE_TRANSITION_PTE (TempPte,
                                *Page,
                                BasePte->u.Soft.Protection,
                                BasePte);

        MI_WRITE_INVALID_PTE (BasePte, TempPte);

        //
        // Increment the share count for the page table page containing
        // this PTE as the PTE just went into the transition state.
        //

        ASSERT (PteFramePage != 0);
        Pfn2 = MI_PFN_ELEMENT (PteFramePage);
        Pfn2->u2.ShareCount += 1;

        NumberOfBytes -= PAGE_SIZE;
        Page += 1;
        BasePte += 1;
    }

    if (TbFlushNeeded == TRUE) {
        MI_FLUSH_ENTIRE_TB_FOR_ATTRIBUTE_CHANGE (NewCacheAttribute);
    }

    return;
}

VOID
MiInitializeTransitionPfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte
    )

/*++

Routine Description:

    This function initializes the specified PFN element to the
    transition state.  Main use is by MapImageFile to make the
    page which contains the image header transition in the
    prototype PTEs.

Arguments:

    PageFrameIndex - Supplies the page frame index to be initialized.

    PointerPte - Supplies an invalid, non-transition PTE to initialize.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, PFN lock held.

--*/

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    MMPTE TempPte;

    MM_PFN_LOCK_ASSERT();
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->u1.Event = NULL;
    Pfn1->PteAddress = PointerPte;
    Pfn1->OriginalPte = *PointerPte;
    ASSERT (!((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
              (Pfn1->OriginalPte.u.Soft.Transition == 1)));

    //
    // Don't change the reference count (it should already be 1).
    //

    Pfn1->u2.ShareCount = 0;

    //
    // No WSLE is required because this is a prototype PTE.
    //

    Pfn1->u3.e1.PrototypePte = 1;

    Pfn1->u3.e1.PageLocation = TransitionPage;

    //
    // Determine the page frame number of the page table page which
    // contains this PTE.
    //

    PteFramePointer = MiGetPteAddress (PointerPte);
    if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)PteFramePointer->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }

    PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    Pfn1->u4.PteFrame = PteFramePage;

    //
    // Put the PTE into the transition state, no cache flush needed as
    // PTE is still not valid.
    //

    MI_MAKE_TRANSITION_PTE (TempPte,
                            PageFrameIndex,
                            PointerPte->u.Soft.Protection,
                            PointerPte);

    MI_WRITE_INVALID_PTE (PointerPte, TempPte);

    //
    // Increment the share count for the page table page containing
    // this PTE as the PTE just went into the transition state.
    //

    Pfn2 = MI_PFN_ELEMENT (PteFramePage);
    ASSERT (PteFramePage != 0);
    Pfn2->u2.ShareCount += 1;

    return;
}

VOID
MiInitializeCopyOnWritePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMWSL WorkingSetList
    )

/*++

Routine Description:

    This function initializes the specified PFN element to the
    active and valid state for a copy on write operation.

    In this case the page table page which contains the PTE has
    the proper ShareCount.

Arguments:

    PageFrameIndex - Supplies the page frame number to initialize.

    PointerPte - Supplies the pointer to the PTE which caused the
                 page fault.

    WorkingSetIndex - Supplies the working set index for the corresponding
                      virtual address.

    WorkingSetList - Supplies the relevant working set list.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, working set pushlock AND PFN lock held.

--*/

{
    PMMPFN Pfn1;
    PMMWSLE Wsle;
    PMMPTE PteFramePointer;
    PFN_NUMBER OldPageFrameIndex;
    PMMPFN OldPfn;
    PFN_NUMBER PteFramePage;
    PVOID VirtualAddress;
    MM_PROTECTION_MASK ProtectionMask;

    ASSERT (PointerPte->u.Hard.Valid == 1);
    OldPageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    OldPfn = MI_PFN_ELEMENT (OldPageFrameIndex);

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->PteAddress = PointerPte;

    //
    // Get the protection for the page.
    //

    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    Pfn1->OriginalPte.u.Long = 0;

    Wsle = &WorkingSetList->Wsle[WorkingSetIndex];

    ProtectionMask = (MM_PROTECTION_MASK) Wsle->u1.e1.Protection;

    if (ProtectionMask == MM_ZERO_ACCESS) {
        ProtectionMask = MI_GET_PROTECTION_FROM_SOFT_PTE (&OldPfn->OriginalPte);
    }

    Pfn1->OriginalPte.u.Soft.Protection =
            MI_MAKE_PROTECT_NOT_WRITE_COPY (ProtectionMask);

    Wsle->u1.e1.Protection = (MM_PROTECTION_MASK) Pfn1->OriginalPte.u.Soft.Protection;

    //
    // Now initialize the rest of the new PFN entry.
    //

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount += 1;
    Pfn1->u2.ShareCount += 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;

    if (Pfn1->u3.e1.CacheAttribute != OldPfn->u3.e1.CacheAttribute) {
        MI_FLUSH_TB_FOR_INDIVIDUAL_ATTRIBUTE_CHANGE (PageFrameIndex,
                                                     OldPfn->u3.e1.CacheAttribute);
        Pfn1->u3.e1.CacheAttribute = OldPfn->u3.e1.CacheAttribute;
    }

    Pfn1->u1.WsIndex = WorkingSetIndex;

    //
    // Determine the page frame number of the page table page which
    // contains this PTE.
    //

    PteFramePointer = MiGetPteAddress (PointerPte);
    if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)PteFramePointer->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }

    PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    ASSERT (PteFramePage != 0);

    Pfn1->u4.PteFrame = PteFramePage;

    //
    // Set the modified flag in the PFN database as we are writing
    // into this page and the dirty bit is already set in the PTE.
    //

    MI_SET_MODIFIED (Pfn1, 1, 0xC);

    return;
}

BOOLEAN
MiIsAddressValid (
    IN PVOID VirtualAddress,
    IN LOGICAL UseForceIfPossible
    )

/*++

Routine Description:

    For a given virtual address this function returns TRUE if no page fault
    will occur for a read operation on the address, FALSE otherwise.

    Note that after this routine was called, if appropriate locks are not
    held, a non-faulting address could fault.

Arguments:

    VirtualAddress - Supplies the virtual address to check.

    UseForceIfPossible - Supplies TRUE if the address should be forced valid
                         if possible.

Return Value:

    TRUE if no page fault would be generated reading the virtual address,
    FALSE otherwise.

Environment:

    Kernel mode.

--*/

{
    PMMPTE PointerPte;
    UNREFERENCED_PARAMETER (UseForceIfPossible);

#if defined (_AMD64_)

    //
    // If this is within the physical addressing range, just return TRUE.
    //

    if (MI_IS_PHYSICAL_ADDRESS(VirtualAddress)) {

        PFN_NUMBER PageFrameIndex;

        //
        // Only bound with MmHighestPhysicalPage once Mm has initialized.
        //

        if (MmHighestPhysicalPage != 0) {

            PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN(VirtualAddress);

            if (PageFrameIndex > MmHighestPhysicalPage) {
                return FALSE;
            }
        }

        return TRUE;
    }

#endif

    //
    // If the address is not canonical then return FALSE as the caller (which
    // may be the kernel debugger) is not expecting to get an unimplemented
    // address bit fault.
    //

    if (MI_RESERVED_BITS_CANONICAL(VirtualAddress) == FALSE) {
        return FALSE;
    }

#if (_MI_PAGING_LEVELS >= 4)
    PointerPte = MiGetPxeAddress (VirtualAddress);
    if (PointerPte->u.Hard.Valid == 0) {
        return FALSE;
    }
#endif

#if (_MI_PAGING_LEVELS >= 3)
    PointerPte = MiGetPpeAddress (VirtualAddress);

    if (PointerPte->u.Hard.Valid == 0) {
        return FALSE;
    }
#endif

    PointerPte = MiGetPdeAddress (VirtualAddress);
    if (PointerPte->u.Hard.Valid == 0) {
        return FALSE;
    }

    if (MI_PDE_MAPS_LARGE_PAGE (PointerPte)) {
        return TRUE;
    }

    PointerPte = MiGetPteAddress (VirtualAddress);
    if (PointerPte->u.Hard.Valid == 0) {
        return FALSE;
    }

    //
    // Make sure we're not treating a page directory as a page table here for
    // the case where the page directory is mapping a large page.  This is
    // because the large page bit is valid in PDE formats, but reserved in
    // PTE formats and will cause a trap.  A virtual address like c0200000 (on
    // x86) triggers this case.
    //

    if (MI_PDE_MAPS_LARGE_PAGE (PointerPte)) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
MmIsAddressValid (
    __in PVOID VirtualAddress
    )

/*++

Routine Description:

    For a given virtual address this function returns TRUE if no page fault
    will occur for a read operation on the address, FALSE otherwise.

    Note that after this routine was called, if appropriate locks are not
    held, a non-faulting address could fault.

Arguments:

    VirtualAddress - Supplies the virtual address to check.

Return Value:

    TRUE if no page fault would be generated reading the virtual address,
    FALSE otherwise.

Environment:

    Kernel mode.

--*/

{
    return MiIsAddressValid (VirtualAddress, FALSE);
}

VOID
MiInitializePfnForOtherProcess (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN PFN_NUMBER ContainingPageFrame
    )

/*++

Routine Description:

    This function initializes the specified PFN element to the
    active and valid state with the dirty bit on in the PTE and
    the PFN database marked as modified.

    As this PTE is not visible from the current process, the containing
    page frame must be supplied at the PTE contents field for the
    PFN database element are set to demand zero.

Arguments:

    PageFrameIndex - Supplies the page frame number of which to initialize.

    PointerPte - Supplies the pointer to the PTE which caused the
                 page fault.

    ContainingPageFrame - Supplies the page frame number of the page
                          table page which contains this PTE.
                          If the ContainingPageFrame is 0, then
                          the ShareCount for the
                          containing page is not incremented.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, PFN lock held.

--*/

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->PteAddress = PointerPte;

    //
    // Note that pages allocated this way are for the kernel and thus
    // never have split permissions in the PTE or the PFN.
    //

    Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount = 1;

    Pfn1->u2.ShareCount += 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;

    //
    // Set the page attribute to cached even though it isn't really mapped
    // into a TB entry yet - it will be when the I/O completes and in the
    // future, may get paged in and out multiple times and will be marked
    // as cached in those transactions also.  If in fact the driver stack
    // wants to map it some other way for the transfer, the correct mapping
    // will get used regardless.
    //

    if (Pfn1->u3.e1.CacheAttribute != MiCached) {
        MI_FLUSH_TB_FOR_CACHED_ATTRIBUTE ();
        Pfn1->u3.e1.CacheAttribute = MiCached;
    }

    MI_SET_MODIFIED (Pfn1, 1, 0xD);

    Pfn1->u4.InPageError = 0;

    //
    // Increment the share count for the page table page containing
    // this PTE.
    //

    if (ContainingPageFrame != 0) {
        Pfn1->u4.PteFrame = ContainingPageFrame;
        Pfn2 = MI_PFN_ELEMENT (ContainingPageFrame);
        Pfn2->u2.ShareCount += 1;
    }
    return;
}

WSLE_NUMBER
MiAddValidPageToWorkingSet (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN ULONG WsleMask
    )

/*++

Routine Description:

    This routine adds the specified virtual address into the
    appropriate working set list.

Arguments:

    VirtualAddress - Supplies the address to add to the working set list.

    PointerPte - Supplies a pointer to the PTE that is now valid.

    Pfn1 - Supplies the PFN database element for the physical page
           mapped by the virtual address.

    WsleMask - Supplies a mask (protection and flags) to OR into the
               working set list entry.

Return Value:

    Non-zero on success, 0 on failure.

Environment:

    Kernel mode, APCs disabled, working set lock.  PFN lock NOT held.

--*/

{
    WSLE_NUMBER WorkingSetIndex;
    PEPROCESS Process;
    PMMSUPPORT WsInfo;

#if !DBG
    UNREFERENCED_PARAMETER (PointerPte);
#endif

    ASSERT (MI_IS_PAGE_TABLE_ADDRESS(PointerPte));
    ASSERT (PointerPte->u.Hard.Valid == 1);

    if (MI_IS_SESSION_ADDRESS (VirtualAddress) || MI_IS_SESSION_PTE (VirtualAddress)) {
        //
        // Current process's session space working set.
        //

        WsInfo = &MmSessionSpace->Vm;
    }
    else if (MI_IS_PROCESS_SPACE_ADDRESS(VirtualAddress)) {

        //
        // Per process working set.
        //

        Process = PsGetCurrentProcess();
        WsInfo = &Process->Vm;
    }
    else {

        //
        // System cache working set.
        //

        WsInfo = &MmSystemCacheWs;
    }

    WorkingSetIndex = MiAllocateWsle (WsInfo, PointerPte, Pfn1, WsleMask);

    return WorkingSetIndex;
}

VOID
MiTrimPte (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN PEPROCESS CurrentProcess,
    IN MMPTE NewPteContents
    )

/*++

Routine Description:

    This routine removes the specified virtual address from a page table
    page.   Note there is no working set list entry for this address.

Arguments:

    VirtualAddress - Supplies the address to remove.

    PointerPte - Supplies a pointer to the PTE that is now valid.

    Pfn1 - Supplies the PFN database element for the physical page
           mapped by the virtual address.

    CurrentProcess - Supplies NULL (ie: system cache), HYDRA_PROCESS (ie:
                     a session) or anything else (ie: process).

    NewPteContents - Supplies the new PTE contents to place in the PTE.  This
                     is only used for prototype PTEs - private PTEs are
                     always encoded with the Pfn's OriginalPte information.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, working set lock.  PFN lock NOT held.

--*/

{
    KIRQL OldIrql;
    MMPTE TempPte;
    MMPTE PreviousPte;
    PMMPTE ContainingPageTablePage;
    PMMPFN Pfn2;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    PETHREAD CurrentThread;

    CurrentThread = PsGetCurrentThread ();

    PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

    // Working set mutex must be held.
    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);
    ASSERT (KeAreAllApcsDisabled () == TRUE);

    if (Pfn1->u3.e1.PrototypePte) {

        ASSERT (MI_IS_PFN_DELETED (Pfn1) == 0);

        //
        // This is a prototype PTE.  The PFN database does not contain
        // the contents of this PTE it contains the contents of the
        // prototype PTE.  This PTE must be reconstructed to contain
        // a pointer to the prototype PTE.
        //
        // The working set list entry contains information about
        // how to reconstruct the PTE.
        //

        TempPte = NewPteContents;
        ASSERT (NewPteContents.u.Proto.Prototype == 1);

        //
        // Decrement the share count of the containing page table
        // page as the PTE for the removed page is no longer valid
        // or in transition.
        //

        ContainingPageTablePage = MiGetPteAddress (PointerPte);
#if (_MI_PAGING_LEVELS >= 3)
        ASSERT (ContainingPageTablePage->u.Hard.Valid == 1);
#else
        if (ContainingPageTablePage->u.Hard.Valid == 0) {
            if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x61940, 
                              (ULONG_PTR) PointerPte,
                              (ULONG_PTR) ContainingPageTablePage->u.Long,
                              (ULONG_PTR) VirtualAddress);
            }
        }
#endif
        PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (ContainingPageTablePage);
        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

        LOCK_PFN (OldIrql);
        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);
    }
    else {

        //
        // This is a private page, make it transition.
        //
        // Assert that the share count is 1 for all user mode pages.
        //

        ASSERT ((Pfn1->u2.ShareCount == 1) ||
                (VirtualAddress > (PVOID)MM_HIGHEST_USER_ADDRESS));

        if (MI_IS_PFN_DELETED (Pfn1)) {
            TempPte.u.Long = 0;
            Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);
            LOCK_PFN (OldIrql);
            MiDecrementShareCountInline (Pfn2, Pfn1->u4.PteFrame);
        }
        else {
            TempPte = *PointerPte;

            MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                          Pfn1->OriginalPte.u.Soft.Protection);
            LOCK_PFN (OldIrql);
        }
    }

    PreviousPte = *PointerPte;

    ASSERT (PreviousPte.u.Hard.Valid == 1);

    MI_WRITE_INVALID_PTE (PointerPte, TempPte);

    MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);

    if ((CurrentProcess == NULL) || (CurrentProcess == HYDRA_PROCESS)) {
        MI_FLUSH_SINGLE_TB (VirtualAddress, TRUE);
    }
    else {

        MI_FLUSH_SINGLE_TB (VirtualAddress, FALSE);

        if ((Pfn1->u3.e1.PrototypePte == 0) &&
            (MI_IS_PTE_DIRTY(PreviousPte)) &&
            (CurrentProcess->Flags & PS_PROCESS_FLAGS_USING_WRITE_WATCH)) {

            //
            // This process has (or had) write watch VADs.
            // Search now for a write watch region encapsulating
            // the PTE being invalidated.
            //

            MiCaptureWriteWatchDirtyBit (CurrentProcess, VirtualAddress);
        }
    }

    MiDecrementShareCountInline (Pfn1, PageFrameIndex);

    UNLOCK_PFN (OldIrql);
}

PMMINPAGE_SUPPORT
MiGetInPageSupportBlock (
    IN KIRQL OldIrql,
    OUT PNTSTATUS Status
    )

/*++

Routine Description:

    This routine acquires an inpage support block.  If none are available,
    the PFN lock will be released and reacquired to add an entry to the list.
    NULL will then be returned.

Arguments:

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at (or
              MM_NOIRQL if the caller did not acquire the PFN at all).

    Status - Supplies a pointer to a status to return (valid only if the
             PFN lock had to be released, ie: NULL was returned).

Return Value:

    A non-null pointer to an inpage block if one is already available.
    The PFN lock is not released in this path.

    NULL is returned if no inpage blocks were available.  In this path, the
    PFN lock is released and an entry is added - but NULL is still returned
    so the caller is aware that the state has changed due to the lock release
    and reacquisition.

Environment:

    Kernel mode, PFN lock may optionally be held.

--*/

{
    PMMINPAGE_SUPPORT Support;
    PSLIST_ENTRY SingleListEntry;

#if DBG
    if (OldIrql != MM_NOIRQL) {
        MM_PFN_LOCK_ASSERT();
    }
    else {
        ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
    }
#endif

    if (ExQueryDepthSList (&MmInPageSupportSListHead) != 0) {

        SingleListEntry = InterlockedPopEntrySList (&MmInPageSupportSListHead);

        if (SingleListEntry != NULL) {
            Support = CONTAINING_RECORD (SingleListEntry,
                                         MMINPAGE_SUPPORT,
                                         ListEntry);

returnok:
            ASSERT (Support->WaitCount == 1);
            ASSERT (Support->u1.e1.PrefetchMdlHighBits == 0);
            ASSERT (Support->u1.LongFlags == 0);
            ASSERT (KeReadStateEvent (&Support->Event) == 0);
            ASSERT64 (Support->UsedPageTableEntries == 0);

            Support->Thread = PsGetCurrentThread();
#if DBG
            Support->ListEntry.Next = NULL;
#endif

            return Support;
        }
    }

    if (OldIrql != MM_NOIRQL) {
        UNLOCK_PFN (OldIrql);
    }

    Support = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof(MMINPAGE_SUPPORT),
                                     'nImM');

    if (Support != NULL) {

        KeInitializeEvent (&Support->Event, NotificationEvent, FALSE);

        Support->WaitCount = 1;
        Support->u1.LongFlags = 0;
        ASSERT (Support->u1.PrefetchMdl == NULL);
        ASSERT (KeReadStateEvent (&Support->Event) == 0);

#if defined (_WIN64)
        Support->UsedPageTableEntries = 0;
#endif
#if DBG
        Support->Thread = NULL;
#endif

        if (OldIrql == MM_NOIRQL) {
            goto returnok;
        }

        InterlockedPushEntrySList (&MmInPageSupportSListHead,
                                   (PSLIST_ENTRY)&Support->ListEntry);

        //
        // Pool had to be allocated for this inpage block hence the PFN
        // lock had to be released.  No need to delay, but the fault must
        // be retried due to the PFN lock release.  Return a status such
        // that this will occur quickly.
        //

        *Status = STATUS_REFAULT;
    }
    else {

        //
        // No pool is available - don't let a high priority thread consume
        // the machine in a continuous refault stream.  A delay allows
        // other system threads to run which will try to free up more pool.
        // Return a status that will introduce a delay above us in an effort
        // to make pool available.  The advantage of our caller executing the
        // delay is because he'll do this after releasing the relevant
        // working set mutex (if any) so the current process can be trimmed
        // for pages also.
        //

        *Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (OldIrql != MM_NOIRQL) {
        LOCK_PFN (OldIrql);
    }

    return NULL;

}

VOID
MiFreeInPageSupportBlock (
    IN PMMINPAGE_SUPPORT Support
    )

/*++

Routine Description:

    This routine returns the inpage support block to a list of freed blocks.

Arguments:

    Support - Supplies the inpage support block to put on the free list.

Return Value:

    None.

Environment:

    Kernel mode, APC_LEVEL or below.

--*/

{
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    ASSERT (Support->Thread != NULL);
    ASSERT (Support->WaitCount != 0);

    ASSERT ((Support->ListEntry.Next == NULL) ||
            (Support->u1.e1.PrefetchMdlHighBits != 0));

    //
    // An interlock is needed for the WaitCount decrement as an inpage support
    // block can be simultaneously freed by any number of threads.
    //
    // Careful synchronization is applied to the WaitCount field so
    // that freeing of the inpage block can occur lock-free.  Note
    // that the ReadInProgress bit on each PFN is set and cleared while
    // holding the PFN lock.  Inpage blocks are always (and must be)
    // freed _AFTER_ the ReadInProgress bit is cleared.
    //

    if (InterlockedDecrement (&Support->WaitCount) == 0) {

        if (Support->u1.e1.PrefetchMdlHighBits != 0) {
            PMDL Mdl;
            Mdl = MI_EXTRACT_PREFETCH_MDL (Support);
            if (Mdl != &Support->Mdl) {
                ExFreePool (Mdl);
            }
        }

        if (ExQueryDepthSList (&MmInPageSupportSListHead) < MmInPageSupportMinimum) {
            Support->WaitCount = 1;
            Support->u1.LongFlags = 0;
            KeClearEvent (&Support->Event);
#if defined (_WIN64)
            Support->UsedPageTableEntries = 0;
#endif
#if DBG
            Support->Thread = NULL;
#endif

            InterlockedPushEntrySList (&MmInPageSupportSListHead,
                                       (PSLIST_ENTRY)&Support->ListEntry);
            return;
        }
        ExFreePool (Support);
    }

    return;
}

VOID
MiHandleBankedSection (
    IN PVOID VirtualAddress,
    IN PMMVAD Vad
    )

/*++

Routine Description:

    This routine invalidates a bank of video memory, calls out to the
    video driver and then enables the next bank of video memory.

Arguments:

    VirtualAddress - Supplies the address of the faulting page.

    Vad - Supplies the VAD which maps the range.

Return Value:

    None.

Environment:

    Kernel mode, APCs disabled, working set mutex held.

--*/

{
    PMMBANKED_SECTION Bank;
    PMMPTE PointerPte;
    ULONG BankNumber;
    ULONG size;

    Bank = ((PMMVAD_LONG) Vad)->u4.Banked;
    size = Bank->BankSize;

    MiZeroMemoryPte (Bank->CurrentMappedPte, size >> PAGE_SHIFT);

    //
    // Flush the TB as we have invalidated all the PTEs in this range.
    //

    MI_FLUSH_PROCESS_TB (FALSE);

    //
    // Calculate new bank address and bank number.
    //

    PointerPte = MiGetPteAddress (
                        (PVOID)((ULONG_PTR)VirtualAddress & ~((LONG)size - 1)));
    Bank->CurrentMappedPte = PointerPte;

    BankNumber = (ULONG)(((PCHAR)PointerPte - (PCHAR)Bank->BasedPte) >> Bank->BankShift);

    (Bank->BankedRoutine) (BankNumber, BankNumber, Bank->Context);

    //
    // Set the new range valid.
    //

    RtlCopyMemory (PointerPte,
                   &Bank->BankTemplate[0],
                   size >> (PAGE_SHIFT - PTE_SHIFT));

    return;
}

