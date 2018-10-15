/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    procx86.c

Abstract:

    This module contains machine-specific
    routines to support the process structure.

--*/


#include "mi.h"

extern SIZE_T MmProcessCommit;

#if !defined(_X86PAE_)


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

    //
    // Charge commitment for the page directory pages, working set page table
    // page, and working set list.  If Vad bitmap lookups are enabled, then
    // charge for a page or two for that as well.
    //

    if (MiChargeCommitment (MM_PROCESS_COMMIT_CHARGE, NULL) == FALSE) {
        return FALSE;
    }

    FlushTbNeeded = FALSE;
    CurrentProcess = PsGetCurrentProcess ();

    NewProcess->NextPageColor = (USHORT) (RtlRandom (&MmProcessColorSeed));
    KeInitializeSpinLock (&NewProcess->HyperSpaceLock);

    //
    // Get the PFN lock to get physical pages.
    //

    LOCK_PFN (OldIrql);

    //
    // Check to make sure the physical pages are available.
    //

    if (MI_NONPAGEABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER)MinimumWorkingSetSize){

        UNLOCK_PFN (OldIrql);
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
    // Allocate a page directory page.
    //

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, OldIrql);
    }

    Color =  MI_PAGE_COLOR_PTE_PROCESS (PDE_BASE,
                                        &CurrentProcess->NextPageColor);

    PageDirectoryIndex = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

    Pfn1 = MI_PFN_ELEMENT (PageDirectoryIndex);

    if (Pfn1->u3.e1.CacheAttribute != MiCached) {
        Pfn1->u3.e1.CacheAttribute = MiCached;
        FlushTbNeeded = TRUE;
    }

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

    INITIALIZE_DIRECTORY_TABLE_BASE (&DirectoryTableBase[0], PageDirectoryIndex);

    INITIALIZE_DIRECTORY_TABLE_BASE (&DirectoryTableBase[1], HyperSpaceIndex);

    //
    // Initialize the page reserved for hyper space.
    //

    TempPte = ValidPdePde;
    MI_SET_GLOBAL_STATE (TempPte, 0);

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

    TempPte.u.Hard.PageFrameNumber = VadBitMapPage;
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

    Pfn1 = MI_PFN_ELEMENT (PageDirectoryIndex);

    Pfn1->PteAddress = (PMMPTE)PDE_BASE;

    TempPte = ValidPdePde;
    TempPte.u.Hard.PageFrameNumber = HyperSpaceIndex;
    MI_SET_GLOBAL_STATE (TempPte, 0);

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

    //
    // Map the working set page table page.
    //

    PdeOffset = MiGetPdeOffset (HYPER_SPACE);
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
    // Recursively map the page directory page so it points to itself.
    //

    TempPte.u.Hard.PageFrameNumber = PageDirectoryIndex;
    PointerPte[MiGetPdeOffset(PTE_BASE)] = TempPte;

    if (MappingPte != NULL) {
        MiReleaseSystemPtes (MappingPte, 1, SystemPteSpace);
    }
    else {
        MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);
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
    ULONG i;
    ULONG PdeOffset;
    ULONG PdeEndOffset;
    MMPTE TempPte;
    PFN_NUMBER PageDirectoryIndex;
    PFN_NUMBER TargetPageDirectoryIndex;
    PEPROCESS CurrentProcess;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE TargetPdePage;
    PMMPTE TargetAddressSpacePde;

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);

    CurrentProcess = PsGetCurrentProcess ();

    //
    // Map the page directory page in hyperspace.
    // Note for PAE, this is the high 1GB virtual only.
    //

    ASSERT (Process->Pcb.DirectoryTableBase[0] != 0);
    TargetPageDirectoryIndex = Process->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;

    ASSERT (PsInitialSystemProcess != NULL);
    ASSERT (PsInitialSystemProcess->Pcb.DirectoryTableBase[0] != 0);
    PageDirectoryIndex = PsInitialSystemProcess->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;

    TempPte = ValidKernelPte;
    TempPte.u.Hard.PageFrameNumber = TargetPageDirectoryIndex;
    ASSERT (MiLargePageHyperPte->u.Long == 0);
    MI_WRITE_VALID_PTE (MiLargePageHyperPte, TempPte);
    TargetPdePage = MiGetVirtualAddressMappedByPte (MiLargePageHyperPte);

    //
    // Map the system process page directory as we know that's always kept
    // up to date.
    //

    PointerPte = MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                             PageDirectoryIndex);

    //
    // Copy all system PTE ranges.
    //

    for (i = 0; i < MiPteRangeIndex; i += 1) {

        PdeOffset = MiGetPdeOffset (MiPteRanges[i].StartingVa);
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

    return;
}

#endif

