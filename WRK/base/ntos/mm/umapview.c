/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   umapview.c

Abstract:

    This module contains the routines which implement the
    NtUnmapViewOfSection service.

--*/

#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtUnmapViewOfSection)
#pragma alloc_text(PAGE,MmUnmapViewOfSection)
#endif


NTSTATUS
NtUnmapViewOfSection(
    __in HANDLE ProcessHandle,
    __in PVOID BaseAddress
    )

/*++

Routine Description:

    This function unmaps a previously created view to a section.

Arguments:

    ProcessHandle - Supplies an open handle to a process object.

    BaseAddress - Supplies the base address of the view.

Return Value:

    NTSTATUS.

--*/

{
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE ();

    PreviousMode = KeGetPreviousMode ();

    if ((PreviousMode == UserMode) && (BaseAddress > MM_HIGHEST_USER_ADDRESS)) {
        return STATUS_NOT_MAPPED_VIEW;
    }

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_VM_OPERATION,
                                        PsProcessType,
                                        PreviousMode,
                                        (PVOID *)&Process,
                                        NULL);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    Status = MiUnmapViewOfSection (Process, BaseAddress, 0);
    ObDereferenceObject (Process);

    return Status;
}

NTSTATUS
MiUnmapViewOfSection (
    IN PEPROCESS Process,
    IN PVOID BaseAddress,
    IN ULONG Flags
    )

/*++

Routine Description:

    This function unmaps a previously created view to a section.

Arguments:

    Process - Supplies a referenced pointer to a process object.

    BaseAddress - Supplies the base address of the view.

    Flags - Supplies a bitfield directive.

Return Value:

    NTSTATUS.

--*/

{
    PMMVAD Vad;
    PMMVAD PreviousVad;
    PMMVAD NextVad;
    PETHREAD Thread;
    PEPROCESS CurrentProcess;
    SIZE_T RegionSize;
    PVOID UnMapImageBase;
    PVOID StartingVa;
    PVOID EndingVa;
    NTSTATUS status;
    LOGICAL Attached;
    KAPC_STATE ApcState;

    PAGED_CODE();

    Attached = FALSE;
    UnMapImageBase = NULL;

    Thread = PsGetCurrentThread ();
    CurrentProcess = PsGetCurrentProcessByThread (Thread);

    //
    // If the specified process is not the current process, attach
    // to the specified process.
    //

    if (CurrentProcess != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    //
    // Get the address creation mutex to block multiple threads from
    // creating or deleting address space at the same time and
    // get the working set mutex so virtual address descriptors can
    // be removed.  Raise IRQL to block APCs.
    //

    if ((Flags & UNMAP_ADDRESS_SPACE_HELD) == 0) {
        LOCK_ADDRESS_SPACE (Process);
    }

    //
    // Make sure the address space was not deleted, if so, return an error.
    //

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        if ((Flags & UNMAP_ADDRESS_SPACE_HELD) == 0) {
            UNLOCK_ADDRESS_SPACE (Process);
        }
        status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }

    //
    // Find the associated vad.
    //

    Vad = MiLocateAddress (BaseAddress);

    if ((Vad == NULL) || (Vad->u.VadFlags.PrivateMemory)) {

        //
        // No Virtual Address Descriptor located for Base Address.
        //

        if ((Flags & UNMAP_ADDRESS_SPACE_HELD) == 0) {
            UNLOCK_ADDRESS_SPACE (Process);
        }
        status = STATUS_NOT_MAPPED_VIEW;
        goto ErrorReturn;
    }

    StartingVa = MI_VPN_TO_VA (Vad->StartingVpn);
    EndingVa = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);

    //
    // If this Vad is for an image section, then
    // get the base address of the section.
    //

    ASSERT (Process == PsGetCurrentProcess ());

    if (Vad->u.VadFlags.VadType == VadImageMap) {
        UnMapImageBase = StartingVa;
    }

    RegionSize = PAGE_SIZE + ((Vad->EndingVpn - Vad->StartingVpn) << PAGE_SHIFT);

    if (Vad->u.VadFlags.NoChange == 1) {

        //
        // An attempt is being made to delete a secured VAD, check
        // the whole VAD to see if this deletion is allowed.
        //

        status = MiCheckSecuredVad (Vad,
                                    StartingVa,
                                    RegionSize,
                                    MM_SECURE_DELETE_CHECK);

        if (!NT_SUCCESS (status)) {
            if ((Flags & UNMAP_ADDRESS_SPACE_HELD) == 0) {
                UNLOCK_ADDRESS_SPACE (Process);
            }
            goto ErrorReturn;
        }
    }

    PreviousVad = MiGetPreviousVad (Vad);
    NextVad = MiGetNextVad (Vad);

    if (Vad->u.VadFlags.VadType == VadRotatePhysical) {

        if ((Flags & UNMAP_ROTATE_PHYSICAL_OK) == 0) {

            //
            // This caller is treating the VA space via the section APIs,
            // but he's not supposed to know that internally we converted
            // the private memory request into a section.  So fail this.
            //

            if ((Flags & UNMAP_ADDRESS_SPACE_HELD) == 0) {
                UNLOCK_ADDRESS_SPACE (Process);
            }
            status = STATUS_NOT_MAPPED_VIEW;
            goto ErrorReturn;
        }

        MiRemoveVadCharges (Vad, Process);

        LOCK_WS_UNSAFE (Thread, Process);

        MiPhysicalViewRemover (Process, Vad);
    }
    else {
        MiRemoveVadCharges (Vad, Process);

        LOCK_WS_UNSAFE (Thread, Process);
    }

    MiRemoveVad (Vad, Process);

    //
    // NOTE: MiRemoveMappedView releases the working set pushlock
    // before returning.
    //

    MiRemoveMappedView (Process, Vad);

    //
    // Return commitment for page table pages if possible.
    //

    MiReturnPageTablePageCommitment (StartingVa,
                                     EndingVa,
                                     Process,
                                     PreviousVad,
                                     NextVad);

    //
    // Update the current virtual size in the process header.
    //

    Process->VirtualSize -= RegionSize;
    if ((Flags & UNMAP_ADDRESS_SPACE_HELD) == 0) {
        UNLOCK_ADDRESS_SPACE (Process);
    }

    ExFreePool (Vad);
    status = STATUS_SUCCESS;

ErrorReturn:

    if (UnMapImageBase) {
        DbgkUnMapViewOfSection (UnMapImageBase);
    }
    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }

    return status;
}

NTSTATUS
MmUnmapViewOfSection (
    __in PEPROCESS Process,
    __in PVOID BaseAddress
     )

/*++

Routine Description:

    This function unmaps a previously created view to a section.

Arguments:

    Process - Supplies a referenced pointer to a process object.

    BaseAddress - Supplies the base address of the view.

Return Value:

    NTSTATUS.

--*/

{
    return MiUnmapViewOfSection (Process, BaseAddress, 0);
}

VOID
MiDecrementSubsections (
    IN PSUBSECTION FirstSubsection,
    IN PSUBSECTION LastSubsection OPTIONAL
    )
/*++

Routine Description:

    This function decrements the subsections, inserting them on the unused
    subsection list if they qualify.

Arguments:

    FirstSubsection - Supplies the subsection to start at.

    LastSubsection - Supplies the last subsection to insert.  Supplies NULL
                     to decrement all the subsections in the chain.

Return Value:

    None.

Environment:

    PFN lock held.

--*/
{
    PMSUBSECTION MappedSubsection;

    ASSERT ((FirstSubsection->ControlArea->u.Flags.Image == 0) &&
            (FirstSubsection->ControlArea->FilePointer != NULL) &&
            (FirstSubsection->ControlArea->u.Flags.PhysicalMemory == 0));

    MM_PFN_LOCK_ASSERT();

    do {
        MappedSubsection = (PMSUBSECTION) FirstSubsection;

        ASSERT (MappedSubsection->DereferenceList.Flink == NULL);

        ASSERT (((LONG_PTR)MappedSubsection->NumberOfMappedViews >= 1) ||
                (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 1));

        MappedSubsection->NumberOfMappedViews -= 1;

        if ((MappedSubsection->NumberOfMappedViews == 0) &&
            (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 0)) {

            //
            // Insert this subsection into the unused subsection list.
            //

            InsertTailList (&MmUnusedSubsectionList,
                            &MappedSubsection->DereferenceList);

            MI_UNUSED_SUBSECTIONS_COUNT_INSERT (MappedSubsection);
        }

        if (ARGUMENT_PRESENT (LastSubsection)) {
            if (FirstSubsection == LastSubsection) {
                break;
            }
        }
        else {
            if (FirstSubsection->NextSubsection == NULL) {
                break;
            }
        }

        FirstSubsection = FirstSubsection->NextSubsection;
    } while (TRUE);
}


VOID
MiRemoveMappedView (
    IN PEPROCESS CurrentProcess,
    IN PMMVAD Vad
    )

/*++

Routine Description:

    This function removes the mapping from the current process's
    address space.  The physical VAD may be a normal mapping (backed by
    a control area) or it may have no control area (it was mapped by a driver).

Arguments:

    CurrentProcess - Supplies a referenced pointer to the current process object.

    Vad - Supplies the VAD which maps the view.

Return Value:

    None.

Environment:

    APC level, working set mutex pushlock and address creation mutex held.

    NOTE:  THE WORKING SET PUSHLOCK IS RELEASED BEFORE RETURNING !!!!

           The working set pushlock must be acquired UNSAFE by the caller.

--*/

{
    KIRQL OldIrql;
    PCONTROL_AREA ControlArea;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE ProtoPte;
    PMMPTE LastPte;
    PFN_NUMBER PdePage;
    PVOID TempVa;
    MMPTE_FLUSH_LIST PteFlushList;
    PVOID UsedPageTableHandle;
    PMMPFN Pfn2;
    PSUBSECTION FirstSubsection;
    PSUBSECTION LastSubsection;
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PMMPTE EndPde;
    PETHREAD CurrentThread;
#if DBG
    PMMPTE EndProto;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PointerPpe;
    PVOID UsedPageDirectoryHandle;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
    PVOID UsedPageDirectoryParentHandle;
#endif

    ControlArea = Vad->ControlArea;
    CurrentThread = PsGetCurrentThread ();

    if (Vad->u.VadFlags.VadType == VadDevicePhysicalMemory) {

        if (((PMMVAD_LONG)Vad)->u4.Banked != NULL) {
            ExFreePool (((PMMVAD_LONG)Vad)->u4.Banked);
        }

        //
        // This is a physical memory view.  The pages map physical memory
        // and are not accounted for in the working set list or in the PFN
        // database.
        //

        MiPhysicalViewRemover (CurrentProcess, Vad);

        //
        // Set count so only flush entire TB operations are performed.
        //

        PteFlushList.Count = MM_MAXIMUM_FLUSH_COUNT;

        PointerPde = MiGetPdeAddress (MI_VPN_TO_VA (Vad->StartingVpn));
        PointerPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->StartingVpn));
        LastPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->EndingVpn));

        ASSERT (PointerPde->u.Hard.Valid == 1);

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

            MiFreeLargePages (MI_VPN_TO_VA (Vad->StartingVpn),
                              MI_VPN_TO_VA_ENDING (Vad->EndingVpn),
                              TRUE);

            UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);
            LOCK_PFN (OldIrql);
        }
        else {
    
            //
            // Remove the PTES from the address space.
            //
    
            PdePage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
    
            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (MI_VPN_TO_VA (Vad->StartingVpn));
    
            LOCK_PFN (OldIrql);

            while (PointerPte <= LastPte) {
    
                if (MiIsPteOnPdeBoundary (PointerPte)) {
    
                    PointerPde = MiGetPteAddress (PointerPte);
                    PdePage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
    
                    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (MiGetVirtualAddressMappedByPte (PointerPte));
                }
    
                //
                // Decrement the count of non-zero page table entries for this
                // page table.
                //
    
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
    
                MI_WRITE_ZERO_PTE (PointerPte);
    
                Pfn2 = MI_PFN_ELEMENT (PdePage);
                MiDecrementShareCountInline (Pfn2, PdePage);
    
                //
                // If all the entries have been eliminated from the previous
                // page table page, delete the page table page itself.  And if
                // this results in an empty page directory page, then delete
                // that too.
                //
    
                if (MI_GET_USED_PTES_FROM_HANDLE(UsedPageTableHandle) == 0) {
    
                    TempVa = MiGetVirtualAddressMappedByPte(PointerPde);
    
                    PteFlushList.Count = MM_MAXIMUM_FLUSH_COUNT;
    
#if (_MI_PAGING_LEVELS >= 3)
                    UsedPageDirectoryHandle = MI_GET_USED_PTES_HANDLE (PointerPte);
    
                    MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryHandle);
#endif
    
                    MiDeletePte (PointerPde,
                                 TempVa,
                                 FALSE,
                                 CurrentProcess,
                                 NULL,
                                 &PteFlushList,
                                 OldIrql);
    
                    //
                    // Add back in the private page MiDeletePte subtracted.
                    //
    
                    CurrentProcess->NumberOfPrivatePages += 1;
    
#if (_MI_PAGING_LEVELS >= 3)
    
                    if (MI_GET_USED_PTES_FROM_HANDLE(UsedPageDirectoryHandle) == 0) {
    
                        PointerPpe = MiGetPdeAddress (PointerPte);
                        TempVa = MiGetVirtualAddressMappedByPte (PointerPpe);
    
                        PteFlushList.Count = MM_MAXIMUM_FLUSH_COUNT;
    
#if (_MI_PAGING_LEVELS >= 4)
                        UsedPageDirectoryParentHandle = MI_GET_USED_PTES_HANDLE (PointerPde);
    
                        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryParentHandle);
#endif
    
                        MiDeletePte (PointerPpe,
                                     TempVa,
                                     FALSE,
                                     CurrentProcess,
                                     NULL,
                                     &PteFlushList,
                                     OldIrql);
    
                        //
                        // Add back in the private page MiDeletePte subtracted.
                        //
        
                        CurrentProcess->NumberOfPrivatePages += 1;
    
#if (_MI_PAGING_LEVELS >= 4)
    
                        if (MI_GET_USED_PTES_FROM_HANDLE(UsedPageDirectoryParentHandle) == 0) {
    
                            PointerPxe = MiGetPpeAddress(PointerPte);
                            TempVa = MiGetVirtualAddressMappedByPte(PointerPxe);
    
                            PteFlushList.Count = MM_MAXIMUM_FLUSH_COUNT;
    
                            MiDeletePte (PointerPxe,
                                         TempVa,
                                         FALSE,
                                         CurrentProcess,
                                         NULL,
                                         &PteFlushList,
                                         OldIrql);
    
                            //
                            // Add back in the private page MiDeletePte subtracted.
                            //
        
                            CurrentProcess->NumberOfPrivatePages += 1;
                        }
#endif
    
                    }
#endif
                }
                PointerPte += 1;
            }
            MI_FLUSH_PROCESS_TB (FALSE);
            UNLOCK_PFN (OldIrql);
            UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);
            LOCK_PFN (OldIrql);
        }
    }
    else {

        if (Vad->u2.VadFlags2.ExtendableFile) {
            PMMEXTEND_INFO ExtendedInfo;
            PMMVAD_LONG VadLong;

            //
            // Release the working set pushlock before referencing the
            // segment since it is pageable.
            //

            UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

            ExtendedInfo = NULL;
            VadLong = (PMMVAD_LONG) Vad;

            KeAcquireGuardedMutexUnsafe (&MmSectionBasedMutex);
            ASSERT (Vad->ControlArea->Segment->ExtendInfo == VadLong->u4.ExtendedInfo);
            VadLong->u4.ExtendedInfo->ReferenceCount -= 1;
            if (VadLong->u4.ExtendedInfo->ReferenceCount == 0) {
                ExtendedInfo = VadLong->u4.ExtendedInfo;
                VadLong->ControlArea->Segment->ExtendInfo = NULL;
            }
            KeReleaseGuardedMutexUnsafe (&MmSectionBasedMutex);
            if (ExtendedInfo != NULL) {
                ExFreePool (ExtendedInfo);
            }

            LOCK_WS_UNSAFE (CurrentThread, CurrentProcess);
        }

        FirstSubsection = NULL;
        LastSubsection = NULL;

        if (Vad->u.VadFlags.VadType != VadImageMap) {

            if (ControlArea->FilePointer != NULL) {

                if ((Vad->u.VadFlags.Protection == MM_READWRITE) ||
                    (Vad->u.VadFlags.Protection == MM_EXECUTE_READWRITE)) {

                    //
                    // Adjust the count of writable user mappings
                    // to support transactions.
                    //
    
                    InterlockedDecrement ((PLONG)&ControlArea->WritableUserReferences);
                }

                FirstSubsection = MiLocateSubsection (Vad, Vad->StartingVpn);

                ASSERT (FirstSubsection != NULL);

                //
                // Note LastSubsection may be NULL for extendable VADs when the
                // EndingVpn is past the end of the section.  In this case,
                // all the subsections can be safely decremented.
                //

                LastSubsection = MiLocateSubsection (Vad, Vad->EndingVpn);
            }
        }
        else {

            //
            // Handle this specially if it is an image view using large pages.
            //

            TempVa = MI_VPN_TO_VA (Vad->StartingVpn);
            PointerPde = MiGetPdeAddress (TempVa);

            if (
#if (_MI_PAGING_LEVELS>=4)
                (MiGetPxeAddress(TempVa)->u.Hard.Valid == 1) &&
#endif
#if (_MI_PAGING_LEVELS>=3)
                (MiGetPpeAddress(TempVa)->u.Hard.Valid == 1) &&
#endif
                (PointerPde->u.Hard.Valid == 1) &&
                (MI_PDE_MAPS_LARGE_PAGE (PointerPde))) {

                MiFreeLargePages (TempVa,
                                  MI_VPN_TO_VA_ENDING (Vad->EndingVpn),
                                  FALSE);

                UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

                MiReleasePhysicalCharges (Vad->EndingVpn - Vad->StartingVpn + 1,
                                          CurrentProcess);

                //
                // Decrement the count of the number of views for the
                // control area.  This requires the PFN lock to be held.
                //
            
                LOCK_PFN (OldIrql);

                ControlArea->NumberOfMappedViews -= 1;
                ControlArea->NumberOfUserReferences -= 1;
            
                //
                // Check to see if the control area should be deleted.
                // This routine releases the PFN lock.
                //
            
                MiCheckControlArea (ControlArea, OldIrql);

                return;
            }
        }

        if (Vad->u.VadFlags.VadType == VadLargePageSection) {

            PointerPde = MiGetPdeAddress (MI_VPN_TO_VA (Vad->StartingVpn));
            EndPde = MiGetPdeAddress (MI_VPN_TO_VA_ENDING (Vad->EndingVpn));

            ControlArea = Vad->ControlArea;

#if DBG
            //
            // Release the working set pushlock before referencing the
            // segment since it is pageable.
            //

            UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);
            ASSERT (ControlArea->Segment->SegmentFlags.LargePages == 1);
            LOCK_WS_UNSAFE (CurrentThread, CurrentProcess);
#endif

            ProtoPte = Vad->FirstPrototypePte;

            do {
                TempPte = *PointerPde;
                ASSERT (TempPte.u.Hard.Valid == 1);
                ASSERT (MI_PDE_MAPS_LARGE_PAGE (&TempPte));

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                EndPfn = Pfn1 + (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

                ASSERT (PageFrameIndex % (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT) == 0);

#if DBG
                //
                // Release the working set pushlock before referencing the
                // prototype PTEs since they are pageable.
                //

                UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

                ASSERT (ProtoPte->u.Hard.Valid == 1);
                ASSERT (PageFrameIndex == MI_GET_PAGE_FRAME_FROM_PTE (ProtoPte));
                EndProto = ProtoPte + (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

                while (ProtoPte < EndProto) {
                    TempPte = *ProtoPte;
                    ASSERT (TempPte.u.Hard.Valid == 1);

                    ASSERT (MI_GET_PAGE_FRAME_FROM_PTE (&TempPte) == PageFrameIndex);
                    ASSERT (MI_PFN_ELEMENT(PageFrameIndex)->PteAddress == ProtoPte);

                    ProtoPte += 1;
                    PageFrameIndex += 1;
                }

                LOCK_WS_UNSAFE (CurrentThread, CurrentProcess);
#endif

                LOCK_PFN (OldIrql);

                do {
                    ASSERT (Pfn1->u2.ShareCount >= 1);
                    Pfn1->u2.ShareCount -= 1;
                    Pfn1 += 1;
                } while (Pfn1 < EndPfn);

                MI_WRITE_ZERO_PTE (PointerPde);

                MI_FLUSH_PROCESS_TB (FALSE);

                UNLOCK_PFN (OldIrql);

                PointerPde += 1;

            } while (PointerPde <= EndPde);

#if defined (_WIN64)
            MiDeletePageDirectoriesForPhysicalRange (
                                    MI_VPN_TO_VA (Vad->StartingVpn),
                                    MI_VPN_TO_VA_ENDING (Vad->EndingVpn));
#endif
        }
        else {
            MiDeleteVirtualAddresses (MI_VPN_TO_VA (Vad->StartingVpn),
                                      MI_VPN_TO_VA_ENDING (Vad->EndingVpn),
                                      Vad);
        }

        UNLOCK_WS_UNSAFE (CurrentThread, CurrentProcess);

        if (FirstSubsection != NULL) {

            //
            // The subsections can only be decremented after all the
            // PTEs have been cleared and PFN sharecounts decremented so no
            // prototype PTEs will be valid if it is indeed the final subsection
            // dereference.  This is critical so the dereference segment
            // thread doesn't free pool containing valid prototype PTEs.
            //

            LOCK_PFN (OldIrql);
            MiDecrementSubsections (FirstSubsection, LastSubsection);
        }
        else {
            LOCK_PFN (OldIrql);
        }
    }

    //
    // Only physical VADs mapped by drivers don't have control areas.
    // If this view has a control area, the view count must be decremented now.
    //

    if (ControlArea) {

        //
        // Decrement the count of the number of views for the
        // Segment object.  This requires the PFN lock to be held (it is
        // already).
        //
    
        ControlArea->NumberOfMappedViews -= 1;
        ControlArea->NumberOfUserReferences -= 1;
    
        //
        // Check to see if the control area (segment) should be deleted.
        // This routine releases the PFN lock.
        //
    
        MiCheckControlArea (ControlArea, OldIrql);
    }
    else {

        UNLOCK_PFN (OldIrql);

        //
        // Even though it says short VAD in VadFlags, it better be a long VAD.
        //

        ASSERT (Vad->u.VadFlags.VadType == VadDevicePhysicalMemory);
        ASSERT (((PMMVAD_LONG)Vad)->u4.Banked == NULL);
        ASSERT (Vad->ControlArea == NULL);
        ASSERT (Vad->FirstPrototypePte == NULL);
    }
    
    return;
}

VOID
MiPurgeImageSection (
    IN PCONTROL_AREA ControlArea,
    IN KIRQL OldIrql
    )

/*++

Routine Description:

    This function locates subsections within an image section that
    contain global memory and resets the global memory back to
    the initial subsection contents.

    Note, that for this routine to be called the section is not
    referenced nor is it mapped in any process.

Arguments:

    ControlArea - Supplies a pointer to the control area for the section.

    OldIrql - Supplies the IRQL the caller acquired the PFN lock at.

Return Value:

    None.

Environment:

    PFN LOCK held, working set pushlock NOT held.

--*/

{
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE LastPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER PageTableFrameIndex;
    MMPTE PteContents;
    MMPTE NewContents;
    MMPTE NewContentsDemandZero;
    ULONG SizeOfRawData;
    ULONG OffsetIntoSubsection;
    PSUBSECTION Subsection;
#if DBG
    ULONG DelayCount = 0;
#endif

    ASSERT (ControlArea->u.Flags.Image != 0);

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    //
    // Loop through all the subsections.
    //

    do {

        if (Subsection->u.SubsectionFlags.GlobalMemory == 1) {

            NewContents.u.Long = 0;
            NewContentsDemandZero.u.Long = 0;
            SizeOfRawData = 0;
            OffsetIntoSubsection = 0;

            //
            // Purge this section.
            //

            if (Subsection->StartingSector != 0) {

                //
                // This is not a demand zero section.
                //

                NewContents.u.Long = MiGetSubsectionAddressForPte(Subsection);
                NewContents.u.Soft.Prototype = 1;

                SizeOfRawData = (Subsection->NumberOfFullSectors << MMSECTOR_SHIFT) |
                               Subsection->u.SubsectionFlags.SectorEndOffset;
            }

            NewContents.u.Soft.Protection =
                                       Subsection->u.SubsectionFlags.Protection;
            NewContentsDemandZero.u.Soft.Protection =
                                        NewContents.u.Soft.Protection;

            PointerPte = Subsection->SubsectionBase;
            LastPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
            ControlArea = Subsection->ControlArea;

            if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {
                MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
            }

            while (PointerPte < LastPte) {

                if (MiIsPteOnPdeBoundary(PointerPte)) {

                    //
                    // We are on a page boundary, make sure this PTE is resident.
                    //

                    if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {
                        MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                    }
                }

                PteContents = *PointerPte;
                if (PteContents.u.Long == 0) {

                    //
                    // No more valid PTEs to deal with.
                    //

                    break;
                }

                ASSERT (PteContents.u.Hard.Valid == 0);

                if ((PteContents.u.Soft.Prototype == 0) &&
                    (PteContents.u.Soft.Transition == 1)) {

                    //
                    // The prototype PTE is in transition format.
                    //

                    Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

                    //
                    // If the prototype PTE is no longer pointing to
                    // the original image page (not in protopte format),
                    // or has been modified, remove it from memory.
                    //

                    if ((Pfn1->u3.e1.Modified == 1) ||
                        (Pfn1->OriginalPte.u.Soft.Prototype == 0)) {

                        ASSERT (Pfn1->OriginalPte.u.Hard.Valid == 0);

                        //
                        // This is a transition PTE which has been
                        // modified or is no longer in protopte format.
                        //

                        if (Pfn1->u3.e2.ReferenceCount != 0) {

                            //
                            // There must be an I/O in progress on this
                            // page.  Wait for the I/O operation to complete.
                            //

                            UNLOCK_PFN (OldIrql);

                            //
                            // Drain the deferred lists as these pages may be
                            // sitting in there right now.
                            //

                            MiDeferredUnlockPages (0);

                            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);

                            //
                            // Redo the loop.
                            //
#if DBG
                            if ((DelayCount % 1024) == 0) {
                                DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                                    "MMFLUSHSEC: waiting for i/o to complete PFN %p\n",
                                    Pfn1);
                            }
                            DelayCount += 1;
#endif

                            PointerPde = MiGetPteAddress (PointerPte);
                            LOCK_PFN (OldIrql);

                            if (PointerPde->u.Hard.Valid == 0) {
                                MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                            }
                            continue;
                        }

                        ASSERT (!((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                           (Pfn1->OriginalPte.u.Soft.Transition == 1)));

                        MI_WRITE_INVALID_PTE (PointerPte, Pfn1->OriginalPte);
                        ASSERT (Pfn1->OriginalPte.u.Hard.Valid == 0);

                        //
                        // Only reduce the number of PFN references if
                        // the original PTE is still in prototype PTE
                        // format.
                        //

                        if (Pfn1->OriginalPte.u.Soft.Prototype == 1) {
                            ControlArea->NumberOfPfnReferences -= 1;
                            ASSERT ((LONG)ControlArea->NumberOfPfnReferences >= 0);
                        }
                        MiUnlinkPageFromList (Pfn1);

                        MI_SET_PFN_DELETED (Pfn1);

                        PageTableFrameIndex = Pfn1->u4.PteFrame;
                        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                        //
                        // If the reference count for the page is zero, insert
                        // it into the free page list, otherwise leave it alone
                        // and when the reference count is decremented to zero
                        // the page will go to the free list.
                        //

                        if (Pfn1->u3.e2.ReferenceCount == 0) {
                            MiReleasePageFileSpace (Pfn1->OriginalPte);
                            MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents));
                        }

                        MI_WRITE_INVALID_PTE (PointerPte, NewContents);
                    }
                } else {

                    //
                    // Prototype PTE is not in transition format.
                    //

                    if (PteContents.u.Soft.Prototype == 0) {

                        //
                        // This refers to a page in the paging file,
                        // as it no longer references the image,
                        // restore the PTE contents to what they were
                        // at the initial image creation.
                        //

                        if (PteContents.u.Long != NoAccessPte.u.Long) {
                            MiReleasePageFileSpace (PteContents);
                            MI_WRITE_INVALID_PTE (PointerPte, NewContents);
                        }
                    }
                }
                PointerPte += 1;
                OffsetIntoSubsection += PAGE_SIZE;

                if (OffsetIntoSubsection >= SizeOfRawData) {

                    //
                    // There are trailing demand zero pages in this
                    // subsection, set the PTE contents to be demand
                    // zero for the remainder of the PTEs in this
                    // subsection.
                    //

                    NewContents = NewContentsDemandZero;
                }

#if DBG
                DelayCount = 0;
#endif

            }
        }

        Subsection = Subsection->NextSubsection;

    } while (Subsection != NULL);

    return;
}

