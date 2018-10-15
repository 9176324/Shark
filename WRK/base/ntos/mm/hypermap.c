/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

   hypermap.c

Abstract:

    This module contains the routines which map physical pages into
    reserved PTEs within hyper space.

--*/

#include "mi.h"

PMMPTE MiFirstReservedZeroingPte;


PVOID
MiMapPageInHyperSpace (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex,
    IN PKIRQL OldIrql
    )

/*++

Routine Description:

    This procedure maps the specified physical page into hyper space
    and returns the virtual address which maps the page.

    ************************************
    *                                  *
    * Returns with a spin lock held!!! *
    *                                  *
    ************************************

Arguments:

    Process - Supplies the current process.

    PageFrameIndex - Supplies the physical page number to map.

    OldIrql - Supplies a pointer in which to return the entry IRQL.

Return Value:

    Returns the address where the requested page was mapped.

    RETURNS WITH THE HYPERSPACE SPIN LOCK HELD!!!!

    The routine MiUnmapHyperSpaceMap MUST be called to release the lock!!!!

Environment:

    Kernel mode.

--*/

{
    MMPTE TempPte;
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PFN_NUMBER offset;

    ASSERT (PageFrameIndex != 0);
    ASSERT (MI_IS_PFN (PageFrameIndex));

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    TempPte = ValidPtePte;

    if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
        MI_SET_PTE_WRITE_COMBINE (TempPte);
    }
    else if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {
        MI_DISABLE_CACHING (TempPte);
    }

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPte = MmFirstReservedMappingPte;

#if defined(NT_UP)
    UNREFERENCED_PARAMETER (Process);
#endif

    LOCK_HYPERSPACE (Process, OldIrql);

    //
    // Get offset to first free PTE.
    //

    offset = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    if (offset == 0) {

        //
        // All the reserved PTEs have been used, make them all invalid.
        //

#if DBG
        {
        PMMPTE LastPte;

        LastPte = PointerPte + NUMBER_OF_MAPPING_PTES;

        do {
            ASSERT (LastPte->u.Long == 0);
            LastPte -= 1;
        } while (LastPte > PointerPte);
        }
#endif

        //
        // Use the page frame number field of the first PTE as an
        // offset into the available mapping PTEs.
        //

        offset = NUMBER_OF_MAPPING_PTES;

        //
        // Flush entire TB only on processors executing this process.
        //

        MI_FLUSH_PROCESS_TB (FALSE);
    }

    //
    // Change offset for next time through.
    //

    PointerPte->u.Hard.PageFrameNumber = offset - 1;

    //
    // Point to free entry and make it valid.
    //

    PointerPte += offset;
    ASSERT (PointerPte->u.Hard.Valid == 0);

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    //
    // Return the VA that maps the page.
    //

    return MiGetVirtualAddressMappedByPte (PointerPte);
}


PVOID
MiMapPageInHyperSpaceAtDpc (
    IN PEPROCESS Process,
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    This procedure maps the specified physical page into hyper space
    and returns the virtual address which maps the page.

    ************************************
    *                                  *
    * Returns with a spin lock held!!! *
    *                                  *
    ************************************

Arguments:

    Process - Supplies the current process.

    PageFrameIndex - Supplies the physical page number to map.

Return Value:

    Returns the address where the requested page was mapped.

    RETURNS WITH THE HYPERSPACE SPIN LOCK HELD!!!!

    The routine MiUnmapHyperSpaceMap MUST be called to release the lock!!!!

Environment:

    Kernel mode, DISPATCH_LEVEL on entry.

--*/

{

    MMPTE TempPte;
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PFN_NUMBER offset;

#if defined(NT_UP)
    UNREFERENCED_PARAMETER (Process);
#endif

    ASSERT (KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT (PageFrameIndex != 0);
    ASSERT (MI_IS_PFN (PageFrameIndex));

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    TempPte = ValidPtePte;

    if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
        MI_SET_PTE_WRITE_COMBINE (TempPte);
    }
    else if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {
        MI_DISABLE_CACHING (TempPte);
    }

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    LOCK_HYPERSPACE_AT_DPC (Process);

    //
    // Get offset to first free PTE.
    //

    PointerPte = MmFirstReservedMappingPte;

    offset = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    if (offset == 0) {

        //
        // All the reserved PTEs have been used, make them all invalid.
        //

#if DBG
        {
        PMMPTE LastPte;

        LastPte = PointerPte + NUMBER_OF_MAPPING_PTES;

        do {
            ASSERT (LastPte->u.Long == 0);
            LastPte -= 1;
        } while (LastPte > PointerPte);
        }
#endif

        //
        // Use the page frame number field of the first PTE as an
        // offset into the available mapping PTEs.
        //

        offset = NUMBER_OF_MAPPING_PTES;

        //
        // Flush entire TB only on processors executing this process.
        //

        MI_FLUSH_PROCESS_TB (FALSE);
    }

    //
    // Change offset for next time through.
    //

    PointerPte->u.Hard.PageFrameNumber = offset - 1;

    //
    // Point to free entry and make it valid.
    //

    PointerPte += offset;
    ASSERT (PointerPte->u.Hard.Valid == 0);

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    //
    // Return the VA that maps the page.
    //

    return MiGetVirtualAddressMappedByPte (PointerPte);
}

PVOID
MiMapPagesToZeroInHyperSpace (
    IN PMMPFN Pfn1,
    IN PFN_COUNT NumberOfPages
    )

/*++

Routine Description:

    This procedure maps the specified physical pages for the zero page thread
    and returns the virtual address which maps them.

    This is ONLY to be used by THE zeroing page thread.

Arguments:

    Pfn1 - Supplies the pointer to the physical page numbers to map.

    NumberOfPages - Supplies the number of pages to map.

Return Value:

    Returns the virtual address where the specified physical pages were
    mapped.

Environment:

    PASSIVE_LEVEL.

--*/

{
    PFN_NUMBER offset;
    MMPTE TempPte;
    MMPTE DefaultCachedPte;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (NumberOfPages != 0);
    ASSERT (NumberOfPages <= NUMBER_OF_ZEROING_PTES);

    PointerPte = MiFirstReservedZeroingPte;

    //
    // Get offset to first free PTE.
    //

    offset = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    if (NumberOfPages > offset) {

        //
        // Not enough unused PTEs left, make them all invalid.
        //

#if DBG
        {
        PMMPTE LastPte;

        LastPte = PointerPte + NUMBER_OF_ZEROING_PTES;

        do {
            ASSERT (LastPte->u.Long == 0);
            LastPte -= 1;
        } while (LastPte > PointerPte);
        }
#endif

        //
        // Use the page frame number field of the first PTE as an
        // offset into the available zeroing PTEs.
        //

        offset = NUMBER_OF_ZEROING_PTES;
        PointerPte->u.Hard.PageFrameNumber = offset;

        //
        // Flush entire TB on all processors as this thread may have migrated.
        //

        MI_FLUSH_ENTIRE_TB (0x25);
    }

    //
    // Change offset for next time through.
    //

    PointerPte->u.Hard.PageFrameNumber = offset - NumberOfPages;

    //
    // Point to free entries and make them valid.  Note that the frames
    // are mapped in reverse order but our caller doesn't care anyway.
    //

    PointerPte += (offset + 1);

    DefaultCachedPte = ValidPtePte;

    ASSERT (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

    do {

        PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

        TempPte = DefaultCachedPte;

        if (Pfn1->u3.e1.CacheAttribute == MiWriteCombined) {
            MI_SET_PTE_WRITE_COMBINE (TempPte);
        }
        else if (Pfn1->u3.e1.CacheAttribute == MiNonCached) {
            MI_DISABLE_CACHING (TempPte);
        }

        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

        PointerPte -= 1;

        ASSERT (PointerPte->u.Hard.Valid == 0);

        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        Pfn1 = (PMMPFN) Pfn1->u1.Flink;

    } while (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

    //
    // Return the VA that maps the page.
    //

    return MiGetVirtualAddressMappedByPte (PointerPte);
}

VOID
MiUnmapPagesInZeroSpace (
    IN PVOID VirtualAddress,
    IN PFN_COUNT NumberOfPages
    )

/*++

Routine Description:

    This procedure unmaps the specified physical pages for the zero page thread.

    This is ONLY to be used by THE zeroing page thread.

Arguments:

    VirtualAddress - Supplies the pointer to the physical page numbers to unmap.

    NumberOfPages - Supplies the number of pages to unmap.

Return Value:

    None.

Environment:

    PASSIVE_LEVEL.

--*/

{
    PMMPTE PointerPte;
#if DBG
    PMMPTE PointerPte2;
#endif

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT (NumberOfPages != 0);
    ASSERT (NumberOfPages <= NUMBER_OF_ZEROING_PTES);

    PointerPte = MiGetPteAddress (VirtualAddress);

#if DBG
    PointerPte2 = PointerPte + NumberOfPages - 1;
    do {
        ASSERT (PointerPte2->u.Hard.Valid == 1);
        PointerPte2 -= 1;
    } while (PointerPte2 >= PointerPte);
#endif

    MiZeroMemoryPte (PointerPte, NumberOfPages);

    return;
}

