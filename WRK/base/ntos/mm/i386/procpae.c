/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    procpae.c

Abstract:

    This module contains machine-specific
    routines to support the process structure.

--*/


#include "mi.h"

extern SIZE_T MmProcessCommit;

#if defined(_X86PAE_)


BOOLEAN
MmCreateProcessAddressSpace (
    IN ULONG MinimumWorkingSetSize,
    IN PEPROCESS NewProcess,
    OUT PULONG_PTR DirectoryTableBase
    )

/*++

Routine Description:

    This routine creates an address space which maps the system
    portion and contains a hyper space entry.

Arguments:

    MinimumWorkingSetSize - Supplies the minimum working set size for
                            this address space.  This value is only used
                            to ensure that ample physical pages exist
                            to create this process.

    NewProcess - Supplies a pointer to the process object being created.

    DirectoryTableBase - Returns the value of the newly created
                         address space's Page Directory (PD) page and
                         hyper space page.

Return Value:

    Returns TRUE if an address space was successfully created, FALSE
    if ample physical pages do not exist.

Environment:

    Kernel mode.  APCs Disabled.

--*/

{
    LOGICAL FlushTbNeeded;
    PFN_NUMBER PageDirectoryIndex;
    PFN_NUMBER HyperSpaceIndex;
    PFN_NUMBER PageContainingWorkingSet;
    PFN_NUMBER VadBitMapPage;
    MMPTE TempPte;
    MMPTE TempPte2;
    PEPROCESS CurrentProcess;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    ULONG Color;
    PMMPTE PointerPte;
    ULONG PdeOffset;
    PMMPTE MappingPte;
    PMMPTE PointerFillPte;
    PMMPTE CurrentAddressSpacePde;
    ULONG TopQuad;
    MMPTE TopPte;
    PPAE_ENTRY PaeVa;
    ULONG i;
    PFN_NUMBER PageDirectories[PD_PER_SYSTEM];

    FlushTbNeeded = FALSE;

    //
    // Charge commitment for the page directory pages, working set page table
    // page, and working set list.  If Vad bitmap lookups are enabled, then
    // charge for a page or two for that as well.
    //

    if (MiChargeCommitment (MM_PROCESS_COMMIT_CHARGE, NULL) == FALSE) {
        return FALSE;
    }

    CurrentProcess = PsGetCurrentProcess ();

    NewProcess->NextPageColor = (USHORT) (RtlRandom (&MmProcessColorSeed));
    KeInitializeSpinLock (&NewProcess->HyperSpaceLock);

    TopQuad = MiPaeAllocate (&PaeVa);

    if (TopQuad == 0) {
        MiReturnCommitment (MM_PROCESS_COMMIT_CHARGE);
        return FALSE;
    }

    TempPte = ValidPdePde;
    MI_SET_GLOBAL_STATE (TempPte, 0);

    //
    // Get the PFN lock to get physical pages.
    //

    LOCK_PFN (OldIrql);

    //
    // Check to make sure the physical pages are available.
    //

    if (MI_NONPAGEABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER)MinimumWorkingSetSize){

        UNLOCK_PFN (OldIrql);
        MiPaeFree (PaeVa);
        MiReturnCommitment (MM_PROCESS_COMMIT_CHARGE);

        //
        // Indicate no directory base was allocated.
        //

        return FALSE;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_PROCESS_CREATE, MM_PROCESS_COMMIT_CHARGE);

    MI_DECREMENT_RESIDENT_AVAILABLE (MinimumWorkingSetSize,
                                     MM_RESAVAIL_ALLOCATE_CREATE_PROCESS);

    //
    // Allocate the page directory pages.
    //

    for (i = 0; i < PD_PER_SYSTEM; i += 1) {

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, OldIrql);
        }

        Color =  MI_PAGE_COLOR_PTE_PROCESS (PDE_BASE,
                                            &CurrentProcess->NextPageColor);

        PageDirectories[i] = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

        Pfn1 = MI_PFN_ELEMENT (PageDirectories[i]);

        if (Pfn1->u3.e1.CacheAttribute != MiCached) {
            Pfn1->u3.e1.CacheAttribute = MiCached;
            FlushTbNeeded = TRUE;
        }
    }

    //
    // Initialize the parent page directory entries.
    //

    TopPte.u.Long = TempPte.u.Long & ~MM_PAE_PDPTE_MASK;
    for (i = 0; i < PD_PER_SYSTEM; i += 1) {
        TopPte.u.Hard.PageFrameNumber = PageDirectories[i];
        PaeVa->PteEntry[i].u.Long = TopPte.u.Long;
    }

    NewProcess->PaeTop = (PVOID) PaeVa;
    DirectoryTableBase[0] = TopQuad;

    //
    // Allocate the hyper space page table page.
    //

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    Color = MI_PAGE_COLOR_PTE_PROCESS (MiGetPdeAddress(HYPER_SPACE),
                                       &CurrentProcess->NextPageColor);

    HyperSpaceIndex = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

    Pfn1 = MI_PFN_ELEMENT (HyperSpaceIndex);

    if (Pfn1->u3.e1.CacheAttribute != MiCached) {
        Pfn1->u3.e1.CacheAttribute = MiCached;
        FlushTbNeeded = TRUE;
    }

    //
    // Unlike DirectoryTableBase[0], the HyperSpaceIndex is stored as an
    // absolute PFN and does not need to be below 4GB.
    //

    DirectoryTableBase[1] = HyperSpaceIndex;

    //
    // Remove page(s) for the VAD bitmap.
    //

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    Color = MI_PAGE_COLOR_VA_PROCESS (MmWorkingSetList,
                                      &CurrentProcess->NextPageColor);

    VadBitMapPage = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

    Pfn1 = MI_PFN_ELEMENT (VadBitMapPage);

    if (Pfn1->u3.e1.CacheAttribute != MiCached) {
        Pfn1->u3.e1.CacheAttribute = MiCached;
        FlushTbNeeded = TRUE;
    }

    //
    // Remove a page for the working set list.
    //

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    Color = MI_PAGE_COLOR_VA_PROCESS (MmWorkingSetList,
                                      &CurrentProcess->NextPageColor);

    PageContainingWorkingSet = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

    Pfn1 = MI_PFN_ELEMENT (PageContainingWorkingSet);

    if (Pfn1->u3.e1.CacheAttribute != MiCached) {
        Pfn1->u3.e1.CacheAttribute = MiCached;
        FlushTbNeeded = TRUE;
    }

    UNLOCK_PFN (OldIrql);

    if (FlushTbNeeded == TRUE) {
        MI_FLUSH_TB_FOR_CACHED_ATTRIBUTE ();
    }

    ASSERT (NewProcess->AddressSpaceInitialized == 0);
    PS_SET_BITS (&NewProcess->Flags, PS_PROCESS_FLAGS_ADDRESS_SPACE1);
    ASSERT (NewProcess->AddressSpaceInitialized == 1);

    NewProcess->Vm.MinimumWorkingSetSize = MinimumWorkingSetSize;

    NewProcess->WorkingSetPage = PageContainingWorkingSet;

    //
    // Initialize the PTEs for hyperspace and the VAD bitmap mapping.
    //

    TempPte.u.Hard.PageFrameNumber = VadBitMapPage;

    MappingPte = MiReserveSystemPtes (1, SystemPteSpace);

    if (MappingPte != NULL) {

        MI_MAKE_VALID_KERNEL_PTE (TempPte2,
                                  HyperSpaceIndex,
                                  MM_READWRITE,
                                  MappingPte);

        MI_SET_PTE_DIRTY (TempPte2);

        MI_WRITE_VALID_PTE (MappingPte, TempPte2);

        PointerPte = MiGetVirtualAddressMappedByPte (MappingPte);
    }
    else {
        PointerPte = MiMapPageInHyperSpace (CurrentProcess, HyperSpaceIndex, &OldIrql);
    }

    PointerPte[MiGetPteOffset(VAD_BITMAP_SPACE)] = TempPte;

    TempPte.u.Hard.PageFrameNumber = PageContainingWorkingSet;
    PointerPte[MiGetPteOffset(MmWorkingSetList)] = TempPte;

    if (MappingPte != NULL) {
        MiReleaseSystemPtes (MappingPte, 1, SystemPteSpace);
    }
    else {
        MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);
    }

    //
    // Set the PTE address in the PFN for the page directory page.
    //

    Pfn1 = MI_PFN_ELEMENT (PageDirectories[PD_PER_SYSTEM - 1]);

    Pfn1->PteAddress = (PMMPTE)PDE_BASE;

    //
    // Add the new process to our internal list prior to filling any
    // system PDEs so if a system PDE changes (large page map or unmap)
    // it can mark this process for a subsequent update.
    //

    ASSERT (NewProcess->Pcb.DirectoryTableBase[0] == 0);

    LOCK_EXPANSION (OldIrql);

    InsertTailList (&MmProcessList, &NewProcess->MmProcessLinks);

    UNLOCK_EXPANSION (OldIrql);

    //
    // Map the page directory page in hyperspace.
    //

    MappingPte = MiReserveSystemPtes (1, SystemPteSpace);

    if (MappingPte != NULL) {

        MI_MAKE_VALID_KERNEL_PTE (TempPte2,
                                  PageDirectories[PD_PER_SYSTEM - 1],
                                  MM_READWRITE,
                                  MappingPte);

        MI_SET_PTE_DIRTY (TempPte2);

        MI_WRITE_VALID_PTE (MappingPte, TempPte2);

        PointerPte = MiGetVirtualAddressMappedByPte (MappingPte);
    }
    else {
        PointerPte = MiMapPageInHyperSpace (CurrentProcess, PageDirectories[PD_PER_SYSTEM - 1], &OldIrql);
    }

    CurrentAddressSpacePde = MiGetPdeAddress (0xC0000000);

    //
    // Copy the entire page directory page for the highest GB so all the
    // kernel mappings are inherited.
    //

    RtlCopyMemory (PointerPte, CurrentAddressSpacePde, PAGE_SIZE);

    //
    // Recursively map each page directory page so it points to itself.
    //

    for (i = 0; i < PD_PER_SYSTEM; i += 1) {
        TempPte.u.Hard.PageFrameNumber = PageDirectories[i];
        PointerPte[i] = TempPte;
    }

    //
    // Map the working set page table page.
    //

    PdeOffset = MiGetPdeOffset (HYPER_SPACE);

    TempPte.u.Hard.PageFrameNumber = HyperSpaceIndex;
    PointerPte[PdeOffset] = TempPte;

    //
    // Zero the remaining page directory range used to map the working
    // set list and its hash.
    //

    PdeOffset += 1;
    ASSERT (MiGetPdeOffset (MmHyperSpaceEnd) >= PdeOffset);

    MiZeroMemoryPte (&PointerPte[PdeOffset],
                     (MiGetPdeOffset (MmHyperSpaceEnd) - PdeOffset + 1));

    //
    // The page directory page is now initialized.
    //

    if (MappingPte != NULL) {
        MiReleaseSystemPtes (MappingPte, 1, SystemPteSpace);
    }
    else {
        MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);
    }

    //
    // Map all the virtual space in the 2GB->3GB range when it's not user space.
    // This includes kernel/HAL code & data, the PFN database, initial nonpaged
    // pool, any extra system PTE or system cache areas, system views and
    // session space.
    //

    if (MmSystemRangeStart < (PVOID) 0xC0000000) {

        PageDirectoryIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PaeVa->PteEntry[PD_PER_SYSTEM - 2]);

        MappingPte = MiReserveSystemPtes (1, SystemPteSpace);

        if (MappingPte != NULL) {

            MI_MAKE_VALID_KERNEL_PTE (TempPte2,
                                      PageDirectoryIndex,
                                      MM_READWRITE,
                                      MappingPte);

            MI_SET_PTE_DIRTY (TempPte2);

            MI_WRITE_VALID_PTE (MappingPte, TempPte2);

            PointerPte = MiGetVirtualAddressMappedByPte (MappingPte);
        }
        else {
            PointerPte = MiMapPageInHyperSpace (CurrentProcess, PageDirectoryIndex, &OldIrql);
        }

        PdeOffset = MiGetPdeOffset (MmSystemRangeStart);
        PointerFillPte = &PointerPte[PdeOffset];
        CurrentAddressSpacePde = MiGetPdeAddress (MmSystemRangeStart);

        RtlCopyMemory (PointerFillPte,
                       CurrentAddressSpacePde,
                       PAGE_SIZE - PdeOffset * sizeof (MMPTE));

        if (MappingPte != NULL) {
            MiReleaseSystemPtes (MappingPte, 1, SystemPteSpace);
        }
        else {
            MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);
        }
    }

    InterlockedExchangeAddSizeT (&MmProcessCommit, MM_PROCESS_COMMIT_CHARGE);

    //
    // Up the session space reference count.
    //

    MiSessionAddProcess (NewProcess);

    return TRUE;
}

PMMPTE MiLargePageHyperPte;


VOID
MiUpdateSystemPdes (
    IN PEPROCESS Process
    )

/*++

Routine Description:

    This routine updates the system PDEs, typically due to a large page
    system PTE mapping being created or destroyed.  This is rare.

    Note this is only needed for 32-bit platforms (64-bit platforms share
    a common top level system page).

Arguments:

    Process - Supplies a pointer to the process to update.

Return Value:

    None.

Environment:

    Kernel mode, expansion lock held.

    The caller acquired the expansion lock prior to clearing the update
    bit from this process.  We must update the PDEs prior to releasing
    it so that any new updates can also be rippled.

--*/

{
    ULONG PdeOffset;
    ULONG PdeEndOffset;
    LOGICAL LowPtes;
    PVOID VirtualAddress;
    MMPTE TempPte;
    PFN_NUMBER PageDirectoryIndex;
    PFN_NUMBER TargetPageDirectoryIndex;
    PEPROCESS CurrentProcess;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE TargetPdePage;
    PMMPTE TargetAddressSpacePde;
    PMMPTE PaeTop;
    ULONG i;

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);

    CurrentProcess = PsGetCurrentProcess ();

    //
    // Map the page directory page in hyperspace.
    // Note for PAE, this is the high 1GB virtual only.
    //

    PaeTop = Process->PaeTop;
    ASSERT (PaeTop != NULL);
    PaeTop += 3;
    ASSERT (PaeTop->u.Hard.Valid == 1);
    TargetPageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

    PaeTop = &MiSystemPaeVa.PteEntry[PD_PER_SYSTEM - 1];
    ASSERT (PaeTop->u.Hard.Valid == 1);
    PageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

    TempPte = ValidKernelPte;
    TempPte.u.Hard.PageFrameNumber = TargetPageDirectoryIndex;
    ASSERT (MiLargePageHyperPte->u.Long == 0);
    MI_WRITE_VALID_PTE (MiLargePageHyperPte, TempPte);
    TargetPdePage = MiGetVirtualAddressMappedByPte (MiLargePageHyperPte);

    LowPtes = FALSE;

    //
    // Map the system process page directory as we know that's always kept
    // up to date.
    //

    PointerPte = MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                             PageDirectoryIndex);

    //
    // Copy all system PTE ranges that reside in the top 1GB.
    //

    for (i = 0; i < MiPteRangeIndex; i += 1) {

        VirtualAddress = MiPteRanges[i].StartingVa;

        if (VirtualAddress < (PVOID) 0xC0000000) {
            LowPtes = TRUE;
            continue;
        }

        PdeOffset = MiGetPdeOffset (VirtualAddress);
        PdeEndOffset = MiGetPdeOffset (MiPteRanges[i].EndingVa);

        PointerPde = &PointerPte[PdeOffset];
        TargetAddressSpacePde = &TargetPdePage[PdeOffset];

        RtlCopyMemory (TargetAddressSpacePde,
                       PointerPde,
                       (PdeEndOffset - PdeOffset + 1) * sizeof (MMPTE));

    }

    MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPte);

    //
    // Just invalidate the mapping on the current processor as we cannot
    // have context switched.
    //

    MI_WRITE_ZERO_PTE (MiLargePageHyperPte);
    MI_FLUSH_CURRENT_TB_SINGLE (TargetPdePage);

    ASSERT (MmSystemRangeStart >= (PVOID) 0x80000000);

    //
    // Copy low additional system PTE ranges (if they exist).
    //

    if (LowPtes == TRUE) {
            
        ASSERT (MmSystemRangeStart < (PVOID) 0xC0000000);

        //
        // Map the target process' page directory.
        //

        PaeTop = Process->PaeTop;
        ASSERT (PaeTop != NULL);
        PaeTop += 2;
        ASSERT (PaeTop->u.Hard.Valid == 1);
        TargetPageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

        TempPte.u.Hard.PageFrameNumber = TargetPageDirectoryIndex;
        ASSERT (MiLargePageHyperPte->u.Long == 0);
        MI_WRITE_VALID_PTE (MiLargePageHyperPte, TempPte);

        //
        // Map the system's page directory.
        //

        PaeTop = &MiSystemPaeVa.PteEntry[PD_PER_SYSTEM - 2];
        ASSERT (PaeTop->u.Hard.Valid == 1);
        PageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

        PdeOffset = MiGetPdeOffset (MmSystemRangeStart);
        TargetAddressSpacePde = &TargetPdePage[PdeOffset];

        PointerPte = MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                 PageDirectoryIndex);

        //
        // Copy all system ranges that reside in the 3rd GB.
        //
    
        for (i = 0; i < MiPteRangeIndex; i += 1) {
    
            VirtualAddress = MiPteRanges[i].StartingVa;
    
            if (VirtualAddress < (PVOID) 0xC0000000) {
    
                PdeOffset = MiGetPdeOffset (VirtualAddress);
                PdeEndOffset = MiGetPdeOffset (MiPteRanges[i].EndingVa);
        
                PointerPde = &PointerPte[PdeOffset];
                TargetAddressSpacePde = &TargetPdePage[PdeOffset];
        
                RtlCopyMemory (TargetAddressSpacePde,
                               PointerPde,
                               (PdeEndOffset - PdeOffset + 1) * sizeof (MMPTE));
            } 
        }
    
        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPte);

        //
        // Just invalidate the mapping on the current processor as we cannot
        // have context switched.
        //

        MI_WRITE_ZERO_PTE (MiLargePageHyperPte);
        MI_FLUSH_CURRENT_TB_SINGLE (TargetPdePage);
    }

    return;
}

#endif

