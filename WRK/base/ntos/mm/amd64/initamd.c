/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    initamd.c

Abstract:

    This module contains the machine dependent initialization for the
    memory management component. It is specifically tailored to the
    AMD64 architecture.

--*/

#include "mi.h"

PFN_NUMBER
MxGetNextPage (
    IN PFN_NUMBER PagesNeeded,
    IN LOGICAL UseSlush
    );

PFN_NUMBER
MxPagesAvailable (
    VOID
    );

VOID
MxConvertToLargePage (
    IN PVOID VirtualAddress,
    IN PVOID EndVirtualAddress
    );

VOID
MxPopulatePageDirectories (
    IN PMMPTE StartPde,
    IN PMMPTE EndPde
    );

VOID
MiComputeInitialLargePage (
    VOID
    );

LOGICAL
MiIsRegularMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_NUMBER PageFrameIndex
    );

PMEMORY_ALLOCATION_DESCRIPTOR
MiFindDescriptor (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_NUMBER PageFrameIndex
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiInitMachineDependent)
#pragma alloc_text(INIT,MxGetNextPage)
#pragma alloc_text(INIT,MxPagesAvailable)
#pragma alloc_text(INIT,MxConvertToLargePage)
#pragma alloc_text(INIT,MiReportPhysicalMemory)
#pragma alloc_text(INIT,MxPopulatePageDirectories)
#pragma alloc_text(INIT,MiComputeInitialLargePage)
#pragma alloc_text(INIT,MiIsRegularMemory)
#pragma alloc_text(INIT,MiFindDescriptor)
#endif

#define MM_LARGE_PAGE_MINIMUM  ((255*1024*1024) >> PAGE_SHIFT)

#define _x1mb (1024*1024)
#define _x1mbnp ((1024*1024) >> PAGE_SHIFT)
#define _x16mb (1024*1024*16)
#define _x16mbnp ((1024*1024*16) >> PAGE_SHIFT)
#define _x4gb (0x100000000UI64)

#define MI_IS_PTE_VALID(PTE) ((PTE)->u.Hard.Valid == 1)

//
// Local data.
//

PFN_NUMBER MiInitialLargePage;
PFN_NUMBER MiInitialLargePageSize;

PFN_NUMBER MxPfnAllocation;
PFN_NUMBER MxTotalFreePages;

PFN_NUMBER MiSlushDescriptorBase;
PFN_NUMBER MiSlushDescriptorCount;

PMEMORY_ALLOCATION_DESCRIPTOR MxFreeDescriptor;
MEMORY_ALLOCATION_DESCRIPTOR MxOldFreeDescriptor;

PMEMORY_ALLOCATION_DESCRIPTOR MxSlushDescriptor1;
MEMORY_ALLOCATION_DESCRIPTOR MxOldSlushDescriptor1;

PMEMORY_ALLOCATION_DESCRIPTOR MxSlushDescriptor2;
MEMORY_ALLOCATION_DESCRIPTOR MxOldSlushDescriptor2;

typedef struct _MI_LARGE_VA_RANGES {
    PVOID VirtualAddress;
    PVOID EndVirtualAddress;
} MI_LARGE_VA_RANGES, *PMI_LARGE_VA_RANGES;

//
// There are potentially 3 large page ranges:
//
// 1. PFN database & initial nonpaged pool
// 2. Kernel code/data
// 3. HAL code/data
//

ULONG MxMapLargePages = 1;

#define MI_MAX_LARGE_VA_RANGES 3

ULONG MiLargeVaRangeIndex;
MI_LARGE_VA_RANGES MiLargeVaRanges[MI_MAX_LARGE_VA_RANGES];

#define MM_PFN_MAPPED_BY_PDE (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT)


PFN_NUMBER
MxGetNextPage (
    IN PFN_NUMBER PagesNeeded,
    IN LOGICAL UseSlush
    )

/*++

Routine Description:

    This function returns the next physical page number from the largest
    largest free descriptor.  If there are not enough physical pages left
    to satisfy the request then a bugcheck is executed since the system
    cannot be initialized.

Arguments:

    PagesNeeded - Supplies the number of pages needed.

    UseSlush - Supplies TRUE if the slush descriptor can be used (ie, the
               allocation will never be freed and will always be cachable).

Return Value:

    The base of the range of physically contiguous pages.

Environment:

    Kernel mode, Phase 0 only.

--*/

{
    PFN_NUMBER PageFrameIndex;

    if (UseSlush == TRUE) {

        if ((MxSlushDescriptor1 != NULL) &&
            (PagesNeeded <= MxSlushDescriptor1->PageCount)) {

            PageFrameIndex = MxSlushDescriptor1->BasePage;

            MxSlushDescriptor1->BasePage += (ULONG) PagesNeeded;
            MxSlushDescriptor1->PageCount -= (ULONG) PagesNeeded;

            return PageFrameIndex;
        }

        if ((MxSlushDescriptor2 != NULL) &&
            (PagesNeeded <= MxSlushDescriptor2->PageCount)) {

            PageFrameIndex = MxSlushDescriptor2->BasePage;

            MxSlushDescriptor2->BasePage += (ULONG) PagesNeeded;
            MxSlushDescriptor2->PageCount -= (ULONG) PagesNeeded;

            return PageFrameIndex;
        }
    }

    //
    // Examine the free descriptor to see if enough usable memory is available.
    //

    if (PagesNeeded > MxFreeDescriptor->PageCount) {

        KeBugCheckEx (INSTALL_MORE_MEMORY,
                      MmNumberOfPhysicalPages,
                      MxFreeDescriptor->PageCount,
                      MxOldFreeDescriptor.PageCount,
                      PagesNeeded);
    }

    PageFrameIndex = MxFreeDescriptor->BasePage;

    MxFreeDescriptor->BasePage += (ULONG) PagesNeeded;
    MxFreeDescriptor->PageCount -= (ULONG) PagesNeeded;

    return PageFrameIndex;
}

PFN_NUMBER
MxPagesAvailable (
    VOID
    )

/*++

Routine Description:

    This function returns the number of pages available.
    
Arguments:

    None.

Return Value:

    The number of physically contiguous pages currently available.

Environment:

    Kernel mode, Phase 0 only.

--*/

{
    return MxFreeDescriptor->PageCount;
}


VOID
MxConvertToLargePage (
    IN PVOID VirtualAddress,
    IN PVOID EndVirtualAddress
    )

/*++

Routine Description:

    This function converts the backing for the supplied virtual address range
    to a large page mapping.
    
Arguments:

    VirtualAddress - Supplies the virtual address to convert to a large page.

    EndVirtualAddress - Supplies the end virtual address to convert to a
                        large page.

Return Value:

    None.

Environment:

    Kernel mode, Phase 1 only.

--*/

{
    ULONG i;
    MMPTE TempPde;
    PMMPTE PointerPde;
    PMMPTE LastPde;
    PMMPTE PointerPte;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    LOGICAL ValidPteFound;
    PFN_NUMBER LargePageBaseFrame;

    ASSERT (MxMapLargePages != 0);

    PointerPde = MiGetPdeAddress (VirtualAddress);
    LastPde = MiGetPdeAddress (EndVirtualAddress);

    TempPde = ValidKernelPde;
    TempPde.u.Hard.LargePage = 1;
    TempPde.u.Hard.Global = 1;

    LOCK_PFN (OldIrql);

    for ( ; PointerPde <= LastPde; PointerPde += 1) {

        ASSERT (PointerPde->u.Hard.Valid == 1);

        if (PointerPde->u.Hard.LargePage == 1) {
            continue;
        }

        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

        //
        // Here's a nasty little hack - the page table page mapping the kernel
        // and HAL (built by the loader) does not necessarily fill all the
        // page table entries (ie: any number of leading entries may be zero).
        //
        // To deal with this, walk forward until a nonzero entry is found
        // and re-index the large page based on this.
        //

        ValidPteFound = FALSE;
        LargePageBaseFrame = (ULONG)-1;
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    
        ASSERT ((PageFrameIndex & (MM_PFN_MAPPED_BY_PDE - 1)) == 0);

        for (i = 0; i < PTE_PER_PAGE; i += 1) {

            ASSERT ((PointerPte->u.Long == 0) ||
                    (ValidPteFound == FALSE) ||
                    (PageFrameIndex == MI_GET_PAGE_FRAME_FROM_PTE (PointerPte)));
            if (PointerPte->u.Hard.Valid == 1) {
                if (ValidPteFound == FALSE) {
                    ValidPteFound = TRUE;
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    LargePageBaseFrame = PageFrameIndex - i;
                }
            }
            PointerPte += 1;
            PageFrameIndex += 1;
        }
    
        if (ValidPteFound == FALSE) {
            continue;
        }

        TempPde.u.Hard.PageFrameNumber = LargePageBaseFrame;

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);

        MI_WRITE_VALID_PTE_NEW_PAGE (PointerPde, TempPde);

        MI_FLUSH_ENTIRE_TB (0xF);

        if (((PageFrameIndex >= MxOldSlushDescriptor1.BasePage) &&
            (PageFrameIndex < MxOldSlushDescriptor1.BasePage + MxOldSlushDescriptor1.PageCount)) ||

            ((PageFrameIndex >= MxOldSlushDescriptor2.BasePage) &&
             (PageFrameIndex < MxOldSlushDescriptor2.BasePage + MxOldSlushDescriptor2.PageCount))) {

            //
            // Excess slush is given to expansion nonpaged pool here to
            // ensure that it is always mapped fully cached since the
            // rest of the large page is inserted as fully cached into
            // the TB.
            //

            MiAddExpansionNonPagedPool (PageFrameIndex, 1, TRUE);
        }
        else {
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            Pfn1->u2.ShareCount = 0;
            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u3.e1.PageLocation = StandbyPageList;
            MI_SET_PFN_DELETED (Pfn1);
            MiDecrementReferenceCount (Pfn1, PageFrameIndex);
        }
    }

    UNLOCK_PFN (OldIrql);
}
LOGICAL
MiIsRegularMemory (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_NUMBER PageFrameIndex
    )

/*++

Routine Description:

    This routine checks whether the argument page frame index represents
    regular memory in the loader descriptor block.  It is only used very
    early during Phase0 init because the MmPhysicalMemoryBlock is not yet
    initialized.

Arguments:

    LoaderBlock  - Supplies a pointer to the firmware setup loader block.

    PageFrameIndex  - Supplies the page frame index to check.

Return Value:

    TRUE if the frame represents regular memory, FALSE if not.

Environment:

    Kernel mode.

--*/

{
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD (NextMd,
                                              MEMORY_ALLOCATION_DESCRIPTOR,
                                              ListEntry);

        if (PageFrameIndex >= MemoryDescriptor->BasePage) {

            if (PageFrameIndex < MemoryDescriptor->BasePage + MemoryDescriptor->PageCount) {

                if ((MemoryDescriptor->MemoryType == LoaderFirmwarePermanent) ||
                    (MemoryDescriptor->MemoryType == LoaderBBTMemory) ||
                    (MemoryDescriptor->MemoryType == LoaderSpecialMemory)) {

                    //
                    // This page lies in a memory descriptor for which we will
                    // never create PFN entries, hence return FALSE.
                    //

                    break;
                }

                return TRUE;
            }
        }
        else {

            //
            // Since the loader memory list is sorted in ascending order,
            // the requested page must not be in the loader list at all.
            //

            break;
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    //
    // The final check before returning FALSE is to ensure that the requested
    // page wasn't one of the ones we used to normal-map the loader mappings,
    // etc.
    //

    if ((PageFrameIndex >= MxOldFreeDescriptor.BasePage) &&
        (PageFrameIndex < MxOldFreeDescriptor.BasePage + MxOldFreeDescriptor.PageCount)) {

        return TRUE;
    }

    if ((PageFrameIndex >= MxOldSlushDescriptor1.BasePage) &&
        (PageFrameIndex < MxOldSlushDescriptor1.BasePage + MxOldSlushDescriptor1.PageCount)) {

        return TRUE;
    }

    if ((PageFrameIndex >= MxOldSlushDescriptor2.BasePage) &&
        (PageFrameIndex < MxOldSlushDescriptor2.BasePage + MxOldSlushDescriptor2.PageCount)) {

        return TRUE;
    }

    if ((PageFrameIndex >= MiSlushDescriptorBase) &&
        (PageFrameIndex < MiSlushDescriptorBase + MiSlushDescriptorCount)) {

        return TRUE;
    }

    return FALSE;
}

VOID
MiReportPhysicalMemory (
    VOID
    )

/*++

Routine Description:

    This routine is called during Phase 0 initialization once the
    MmPhysicalMemoryBlock has been constructed.  It's job is to decide
    which large page ranges to enable later and also to construct a
    large page comparison list so any requests which are not fully cached
    can check this list in order to refuse conflicting requests.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode.  Phase 0 only.

    This is called before any non-MmCached allocations are made.

--*/

{
    ULONG i, j;
    PMMPTE PointerPte;
    LOGICAL EntryFound;
    PFN_NUMBER count;
    PFN_NUMBER Page;
    PFN_NUMBER LastPage;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER LastPageFrameIndex;

    //
    // Examine the physical memory block to see whether large pages should
    // be enabled.  The key point is that all the physical pages within a
    // given large page range must have the same cache attributes (MmCached)
    // in order to maintain TB coherency.  This can be done provided all
    // the pages within the large page range represent real RAM (as described
    // by the loader) so that memory management can control it.  If any
    // portion of the large page range is not RAM, it is possible that it
    // may get used as noncached or writecombined device memory and
    // therefore large pages cannot be used.
    //

    if (MxMapLargePages == 0) {
        return;
    }

    for (i = 0; i < MiLargeVaRangeIndex; i += 1) {
        PointerPte = MiGetPteAddress (MiLargeVaRanges[i].VirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        PointerPte = MiGetPteAddress (MiLargeVaRanges[i].EndVirtualAddress);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        LastPageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        //
        // Round the start down to a page directory boundary and the end to
        // the last page directory entry before the next boundary.
        //

        PageFrameIndex &= ~(MM_PFN_MAPPED_BY_PDE - 1);
        LastPageFrameIndex |= (MM_PFN_MAPPED_BY_PDE - 1);

        EntryFound = FALSE;

        j = 0;
        do {

            count = MmPhysicalMemoryBlock->Run[j].PageCount;
            Page = MmPhysicalMemoryBlock->Run[j].BasePage;

            LastPage = Page + count;

            if ((PageFrameIndex >= Page) && (LastPageFrameIndex < LastPage)) {
                EntryFound = TRUE;
                break;
            }

            j += 1;

        } while (j != MmPhysicalMemoryBlock->NumberOfRuns);

        if (EntryFound == FALSE) {

            //
            // No entry was found that completely spans this large page range.
            // Zero it so this range will not be converted into large pages
            // later.
            //

            DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                "MM: Loader/HAL memory block indicates large pages cannot be used\n");

            MiLargeVaRanges[i].VirtualAddress = NULL;

            //
            // Don't use large pages for anything if any individual range
            // could not be used.  This is because 2 separate ranges may
            // share a straddling large page.  If the first range was unable
            // to use large pages, but the second one does ... then only part
            // of the first range will get large pages if we enable large
            // pages for the second range.  This would be very bad as we use
            // the MI_IS_PHYSICAL macro everywhere and assume the entire
            // range is in or out, so disable all large pages here instead.
            //

            while (i != 0) {

                i -= 1;

                if (MiLargeVaRanges[i].VirtualAddress != NULL) {

                    PointerPte = MiGetPteAddress (MiLargeVaRanges[i].VirtualAddress);
                    ASSERT (PointerPte->u.Hard.Valid == 1);
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                    PointerPte = MiGetPteAddress (MiLargeVaRanges[i].EndVirtualAddress);
                    ASSERT (PointerPte->u.Hard.Valid == 1);
                    LastPageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                    //
                    // Round the start down to a page directory boundary and
                    // the end to the last page directory entry before the
                    // next boundary.
                    //

                    PageFrameIndex &= ~(MM_PFN_MAPPED_BY_PDE - 1);
                    LastPageFrameIndex |= (MM_PFN_MAPPED_BY_PDE - 1);

                    MiRemoveCachedRange (PageFrameIndex, LastPageFrameIndex);
                }
            }

            MiLargeVaRangeIndex = 0;
            break;
        }
        else {
            MiAddCachedRange (PageFrameIndex, LastPageFrameIndex);
        }
    }
}

VOID
MxPopulatePageDirectories (
    IN PMMPTE StartPde,
    IN PMMPTE EndPde
    )

/*++

Routine Description:

    This routine allocates page parents, directories and tables as needed.
    Note any new page tables needed to map the range get zero filled.

Arguments:

    StartPde - Supplies the PDE to begin the population at.

    EndPde - Supplies the PDE to end the population at.

Return Value:

    None.

Environment:

    Kernel mode.  Phase 0 initialization.

--*/

{
    PMMPTE StartPxe;
    PMMPTE StartPpe;
    MMPTE TempPte;
    LOGICAL First;

    First = TRUE;
    TempPte = ValidKernelPte;

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
            First = FALSE;

            StartPxe = MiGetPdeAddress(StartPde);
            if (StartPxe->u.Hard.Valid == 0) {

                //
                // Map in a page directory parent page, using the
                // slush descriptor if one exists.
                //

                TempPte.u.Hard.PageFrameNumber = MxGetNextPage (1, TRUE);
                *StartPxe = TempPte;
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe),
                               PAGE_SIZE);
            }

            StartPpe = MiGetPteAddress(StartPde);
            if (StartPpe->u.Hard.Valid == 0) {

                //
                // Map in a page directory, using the
                // slush descriptor if one exists.
                //

                TempPte.u.Hard.PageFrameNumber = MxGetNextPage (1, TRUE);
                *StartPpe = TempPte;
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe),
                               PAGE_SIZE);
            }
        }

        if (StartPde->u.Hard.Valid == 0) {

            //
            // Map in a page table page, using the
            // slush descriptor if one exists.
            //

            TempPte.u.Hard.PageFrameNumber = MxGetNextPage (1, TRUE);
            *StartPde = TempPte;
            RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPde),
                           PAGE_SIZE);
        }
        StartPde += 1;
    }
}

VOID
MiComputeInitialLargePage (
    VOID
    )

/*++

Routine Description:

    This function computes the number of bytes needed to span the initial
    nonpaged pool and PFN database plus the color arrays.  It rounds this up
    to a large page boundary and carves the memory from the free descriptor.

    If the physical memory is too sparse to use large pages for this, then
    fall back to using small pages.

Arguments:

    None.

Return Value:

    None.

Environment:

    Kernel mode, INIT only.

--*/

{
    PFN_NUMBER i;
    PFN_NUMBER BasePage;
    PFN_NUMBER LastPage;
    UCHAR Associativity;
    SIZE_T NumberOfBytes;
    SIZE_T PfnAllocation;
    SIZE_T MaximumNonPagedPoolInBytesLimit;

    MaximumNonPagedPoolInBytesLimit = 0;

    //
    // Non-paged pool comprises 2 chunks.  The initial nonpaged pool grows
    // up and the expansion nonpaged pool expands downward.
    //
    // Initial non-paged pool is constructed so virtual addresses
    // are also physically contiguous.
    //

    if ((MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT) >
                        (7 * (MxTotalFreePages >> 3))) {

        //
        // More than 7/8 of memory allocated to nonpagedpool, reset to 0.
        //

        MmSizeOfNonPagedPoolInBytes = 0;
    }

    if (MmSizeOfNonPagedPoolInBytes < MmMinimumNonPagedPoolSize) {

        //
        // Calculate the size of nonpaged pool.
        // Use the minimum size, then for every MB above 16mb add extra pages.
        //

        MmSizeOfNonPagedPoolInBytes = MmMinimumNonPagedPoolSize;

        MmSizeOfNonPagedPoolInBytes +=
            ((MxTotalFreePages - _x16mbnp)/_x1mbnp) *
            MmMinAdditionNonPagedPoolPerMb;
    }

    if (MmSizeOfNonPagedPoolInBytes > MM_MAX_INITIAL_NONPAGED_POOL) {
        MmSizeOfNonPagedPoolInBytes = MM_MAX_INITIAL_NONPAGED_POOL;
    }

    //
    // If the registry specifies a total nonpaged pool percentage cap, enforce
    // it here.
    //

    if (MmMaximumNonPagedPoolPercent != 0) {

        if (MmMaximumNonPagedPoolPercent < 5) {
            MmMaximumNonPagedPoolPercent = 5;
        }
        else if (MmMaximumNonPagedPoolPercent > 80) {
            MmMaximumNonPagedPoolPercent = 80;
        }

        //
        // Use the registry-expressed percentage value.
        //
    
        MaximumNonPagedPoolInBytesLimit =
            ((MxTotalFreePages * MmMaximumNonPagedPoolPercent) / 100);

        MaximumNonPagedPoolInBytesLimit *= PAGE_SIZE;

        if (MaximumNonPagedPoolInBytesLimit < 6 * 1024 * 1024) {
            MaximumNonPagedPoolInBytesLimit = 6 * 1024 * 1024;
        }

        if (MmSizeOfNonPagedPoolInBytes > MaximumNonPagedPoolInBytesLimit) {
            MmSizeOfNonPagedPoolInBytes = MaximumNonPagedPoolInBytesLimit;
        }
    }
    
    MmSizeOfNonPagedPoolInBytes = MI_ROUND_TO_SIZE (MmSizeOfNonPagedPoolInBytes,
                                                    PAGE_SIZE);

    //
    // Don't let the initial nonpaged pool choice exceed what's actually
    // available.
    //

    if ((MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT) > MxFreeDescriptor->PageCount / 2) {
        MmSizeOfNonPagedPoolInBytes = (MxFreeDescriptor->PageCount / 2) << PAGE_SHIFT;
    }

    //
    // Compute the secondary color value, allowing overrides from the registry.
    // This is because the color arrays are going to be allocated at the end
    // of the PFN database.
    //
    // Get secondary color value from:
    //
    // (a) from the registry (already filled in) or
    // (b) from the PCR or
    // (c) default value.
    //

    if (MmSecondaryColors == 0) {

        Associativity = KeGetPcr()->SecondLevelCacheAssociativity;

        MmSecondaryColors = KeGetPcr()->SecondLevelCacheSize;

        if (Associativity != 0) {
            MmSecondaryColors /= Associativity;
        }
    }

    MmSecondaryColors = MmSecondaryColors >> PAGE_SHIFT;

    if (MmSecondaryColors == 0) {
        MmSecondaryColors = MM_SECONDARY_COLORS_DEFAULT;
    }
    else {

        //
        // Make sure the value is power of two and within limits.
        //

        if (MmSecondaryColors > MM_SECONDARY_COLORS_MAX) {
            MmSecondaryColors = MM_SECONDARY_COLORS_MAX;
        }
        else if (((MmSecondaryColors & (MmSecondaryColors - 1)) != 0) ||
                 (MmSecondaryColors < MM_SECONDARY_COLORS_MIN)) {

            MmSecondaryColors = MM_SECONDARY_COLORS_DEFAULT;
        }
    }

    MmSecondaryColorMask = MmSecondaryColors - 1;

    //
    // Set the secondary color mask on the boot processor since it is needed
    // very early.
    //

    KeGetCurrentPrcb()->SecondaryColorMask = MmSecondaryColorMask;

    //
    // Determine number of bits in MmSecondayColorMask. This
    // is the number of bits the Node color must be shifted
    // by before it is included in colors.
    //

    i = MmSecondaryColorMask;
    MmSecondaryColorNodeShift = 0;
    while (i) {
        i >>= 1;
        MmSecondaryColorNodeShift += 1;
    }

    //
    // Adjust the number of secondary colors by the number of nodes
    // in the machine.  The secondary color mask is NOT adjusted
    // as it is used to control coloring within a node.  The node
    // color is added to the color AFTER normal color calculations
    // are performed.
    //

    MmSecondaryColors *= KeNumberNodes;

    for (i = 0; i < KeNumberNodes; i += 1) {
        KeNodeBlock[i]->Color = (UCHAR)i;
        KeNodeBlock[i]->MmShiftedColor = (ULONG)(i << MmSecondaryColorNodeShift);
        InitializeSListHead(&KeNodeBlock[i]->DeadStackList);
    }

    //
    // Add in the PFN database size and the array for tracking secondary colors.
    //

    PfnAllocation = MI_ROUND_TO_SIZE (((MmHighestPossiblePhysicalPage + 1) * sizeof(MMPFN)) +
                    (MmSecondaryColors * sizeof(MMCOLOR_TABLES)*2),
                    PAGE_SIZE);

    NumberOfBytes = MmSizeOfNonPagedPoolInBytes + PfnAllocation;

    //
    // Align to large page size boundary, donating any extra to the nonpaged
    // pool.
    //

    NumberOfBytes = MI_ROUND_TO_SIZE (NumberOfBytes, MM_MINIMUM_VA_FOR_LARGE_PAGE);

    MmSizeOfNonPagedPoolInBytes = NumberOfBytes - PfnAllocation;

    MxPfnAllocation = PfnAllocation >> PAGE_SHIFT;

    //
    // Calculate the maximum size of pool.
    //

    if (MmMaximumNonPagedPoolInBytes == 0) {

        //
        // Calculate the size of nonpaged pool, adding extra pages for
        // every MB above 16mb.
        //

        MmMaximumNonPagedPoolInBytes = MmDefaultMaximumNonPagedPool;

        ASSERT (BYTE_OFFSET (MmMaximumNonPagedPoolInBytes) == 0);

        MmMaximumNonPagedPoolInBytes +=
            ((SIZE_T)((MxTotalFreePages - _x16mbnp)/_x1mbnp) *
            MmMaxAdditionNonPagedPoolPerMb);

        if ((MmMaximumNonPagedPoolPercent != 0) &&
            (MmMaximumNonPagedPoolInBytes > MaximumNonPagedPoolInBytesLimit)) {
                MmMaximumNonPagedPoolInBytes = MaximumNonPagedPoolInBytesLimit;
        }
    }

    MmMaximumNonPagedPoolInBytes = MI_ROUND_TO_SIZE (MmMaximumNonPagedPoolInBytes,
                                                  MM_MINIMUM_VA_FOR_LARGE_PAGE);

    MmMaximumNonPagedPoolInBytes += NumberOfBytes;

    if (MmMaximumNonPagedPoolInBytes > MM_MAX_ADDITIONAL_NONPAGED_POOL) {
        MmMaximumNonPagedPoolInBytes = MM_MAX_ADDITIONAL_NONPAGED_POOL;
    }

    MiInitialLargePageSize = NumberOfBytes >> PAGE_SHIFT;

    if ((MmProtectFreedNonPagedPool == FALSE) &&
        (MxMapLargePages != 0) &&
        (MxPfnAllocation <= MxFreeDescriptor->PageCount / 2)) {

        //
        // See if the free descriptor has enough pages of large page alignment
        // to satisfy our calculation.
        //

        BasePage = MI_ROUND_TO_SIZE (MxFreeDescriptor->BasePage,
                                 MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);

        LastPage = MxFreeDescriptor->BasePage + MxFreeDescriptor->PageCount;

        if ((BasePage < MxFreeDescriptor->BasePage) ||
            (BasePage + (NumberOfBytes >> PAGE_SHIFT) > LastPage)) {

            KeBugCheckEx (INSTALL_MORE_MEMORY,
                          NumberOfBytes >> PAGE_SHIFT,
                          MxFreeDescriptor->BasePage,
                          MxFreeDescriptor->PageCount,
                          2);
        }

        if (BasePage == MxFreeDescriptor->BasePage) {

            //
            // The descriptor starts on a large page aligned boundary so
            // remove the large page span from the bottom of the free descriptor.
            //

            MiInitialLargePage = BasePage;

            MxFreeDescriptor->BasePage += (ULONG) MiInitialLargePageSize;
            MxFreeDescriptor->PageCount -= (ULONG) MiInitialLargePageSize;
        }
        else {

            if ((LastPage & ((MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT) - 1)) == 0) {
                //
                // The descriptor ends on a large page aligned boundary so
                // remove the large page span from the top of the free descriptor.
                //

                MiInitialLargePage = LastPage - MiInitialLargePageSize;

                MxFreeDescriptor->PageCount -= (ULONG) MiInitialLargePageSize;
            }
            else {

                //
                // The descriptor does not start or end on a large page aligned
                // address so chop the descriptor.  The excess slush is added to
                // the freelist by our caller.
                //

                MiSlushDescriptorBase = MxFreeDescriptor->BasePage;
                MiSlushDescriptorCount = BasePage - MxFreeDescriptor->BasePage;

                MiInitialLargePage = BasePage;

                MxFreeDescriptor->PageCount -= (ULONG) (MiInitialLargePageSize + MiSlushDescriptorCount);

                MxFreeDescriptor->BasePage = (ULONG) (BasePage + MiInitialLargePageSize);
            }
        }

        MiAddCachedRange (MiInitialLargePage,
                          MiInitialLargePage + MiInitialLargePageSize - 1);
    }
    else {

        //
        // Not enough contiguous physical memory in this machine to use large
        // pages for the PFN database and color heads so fall back to small.
        //
        // Continue to march on so the virtual sizes can still be computed
        // properly.
        //
        // Note this is not large page aligned so it can never be confused with
        // a valid large page start.
        //

        MiInitialLargePage = (PFN_NUMBER) -1;
    }

    MmPfnDatabase = (PMMPFN) ((PCHAR)MmNonPagedPoolEnd - MmMaximumNonPagedPoolInBytes);

    MmNonPagedPoolStart = (PVOID)((PCHAR) MmPfnDatabase + PfnAllocation);

    ASSERT (BYTE_OFFSET (MmNonPagedPoolStart) == 0);

    MmNonPagedPoolExpansionStart = (PVOID)((PCHAR) MmPfnDatabase +
                                        (MiInitialLargePageSize << PAGE_SHIFT));

    MmMaximumNonPagedPoolInBytes = ((PCHAR) MmNonPagedPoolEnd - (PCHAR) MmNonPagedPoolStart);

    MmMaximumNonPagedPoolInPages = (MmMaximumNonPagedPoolInBytes >> PAGE_SHIFT);

    return;
}

PMEMORY_ALLOCATION_DESCRIPTOR
MiFindDescriptor (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PFN_NUMBER PageFrameIndex
    )
{
    PLIST_ENTRY NextMd;
    PFN_NUMBER PageCount;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;

    PageCount = MM_PFN_MAPPED_BY_PDE - (PageFrameIndex & (MM_PFN_MAPPED_BY_PDE - 1));

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD (NextMd,
                                              MEMORY_ALLOCATION_DESCRIPTOR,
                                              ListEntry);

        if (MemoryDescriptor->BasePage == PageFrameIndex) {

            //
            // Found the descriptor following the image.  If
            // it is marked firmware temporary and it spans
            // to the *next* large page boundary, then it's good.
            //

            if ((MemoryDescriptor->MemoryType == LoaderFirmwareTemporary) &&
                (MemoryDescriptor->PageCount == PageCount) &&
                (MemoryDescriptor != MxFreeDescriptor)) {

                return MemoryDescriptor;
            }

            break;
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    return NULL;
}


VOID
MiInitMachineDependent (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This routine performs the necessary operations to enable virtual
    memory. This includes building the page directory parent pages and
    the page directories for the system, building page table pages to map
    the code section, the data section, the stack section and the trap handler.

    It also initializes the PFN database and populates the free list.

Arguments:

    LoaderBlock - Supplies the address of the loader block.

Return Value:

    None.

Environment:

    Kernel mode.

    N.B.  This routine uses memory from the loader block descriptors, but
    the descriptors themselves must be restored prior to return as our caller
    walks them to create the MmPhysicalMemoryBlock.

--*/

{
    PHYSICAL_ADDRESS MaxHotPlugMemoryAddress;
    PVOID EndVirtualAddress;
    PVOID va;
    PVOID SystemPteStart;
    ULONG UseGlobal;
    PFN_NUMBER BasePage;
    PFN_NUMBER PageCount;
    PFN_NUMBER NextPhysicalPage;
    PFN_COUNT FreeNumberOfPages;
    ULONG_PTR DirBase;
    LOGICAL First;
    PMMPFN BasePfn;
    PMMPFN BottomPfn;
    PMMPFN TopPfn;
    PFN_NUMBER i;
    PFN_NUMBER j;
    PFN_NUMBER PdePageNumber;
    PFN_NUMBER PxePage;
    PFN_NUMBER PpePage;
    PFN_NUMBER PdePage;
    PFN_NUMBER PtePage;
    PEPROCESS CurrentProcess;
    PFN_NUMBER MostFreePage;
    PLIST_ENTRY NextMd;
    SIZE_T MaxPool;
    KIRQL OldIrql;
    MMPTE TempPte;
    MMPTE TempPde;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE Pde;
    PMMPTE StartPxe;
    PMMPTE EndPxe;
    PMMPTE StartPpe;
    PMMPTE EndPpe;
    PMMPTE StartPde;
    PMMPTE EndPde;
    PMMPTE StartPte;
    PMMPTE EndPte;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn2;
    PMMPFN Pfn3;
    PMMPFN Pfn4;
    ULONG_PTR Range;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR TempSlushDescriptor;
    ULONG ReturnedLength;
    NTSTATUS status;
    PMMPTE Kseg0StartPte;
    PMMPTE Kseg0EndPte;
    ULONG DummyFlags;

    if (InitializationPhase == 1) {

        //
        // If the number of physical pages is greater than 255mb and the
        // verifier is not enabled, then map the kernel and HAL images
        // with large pages.
        //
        // The PFN database and initial nonpaged pool are already
        // mapped with large pages.
        //

        if (MxMapLargePages != 0) {
            for (i = 0; i < MiLargeVaRangeIndex; i += 1) {
                if (MiLargeVaRanges[i].VirtualAddress != NULL) {
                    MxConvertToLargePage (MiLargeVaRanges[i].VirtualAddress,
                                          MiLargeVaRanges[i].EndVirtualAddress);
                }
            }
        }

        return;
    }

    ASSERT (InitializationPhase == 0);

    //
    // All AMD64 processors support PAT mode and global pages.
    //

    ASSERT (KeFeatureBits & KF_PAT);
    ASSERT (KeFeatureBits & KF_GLOBAL_PAGE);

    MostFreePage = 0;

    ASSERT (KeFeatureBits & KF_LARGE_PAGE);

    ValidKernelPte.u.Long = ValidKernelPteLocal.u.Long;
    ValidKernelPde.u.Long = ValidKernelPdeLocal.u.Long;

    //
    // Note that the PAE mode of the processor does not support the
    // global bit in PDEs which map 4K page table pages.
    //

    TempPte = ValidKernelPte;
    TempPde = ValidKernelPde;

    //
    // Set the directory base for the system process.
    //

    PointerPte = MiGetPxeAddress (PXE_BASE);
    PdePageNumber = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte);

    DirBase = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte) << PAGE_SHIFT;

    PsGetCurrentProcess()->Pcb.DirectoryTableBase[0] = DirBase;
    KeSweepDcache (FALSE);

    //
    // Unmap the user memory space.
    //

    PointerPde = MiGetPxeAddress (0);
    LastPte = MiGetPxeAddress (MM_SYSTEM_RANGE_START);

    MiZeroMemoryPte (PointerPde, LastPte - PointerPde);

    //
    // Get the lower bound of the free physical memory and the number of
    // physical pages by walking the memory descriptor lists.
    //

    MxFreeDescriptor = NULL;
    ASSERT (MxTotalFreePages == 0);
    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        if ((MemoryDescriptor->MemoryType != LoaderFirmwarePermanent) &&
            (MemoryDescriptor->MemoryType != LoaderBBTMemory) &&
            (MemoryDescriptor->MemoryType != LoaderHALCachedMemory) &&
            (MemoryDescriptor->MemoryType != LoaderSpecialMemory)) {

            //
            // This check results in /BURNMEMORY chunks not being counted.
            //

            if (MemoryDescriptor->MemoryType != LoaderBad) {
                MmNumberOfPhysicalPages += MemoryDescriptor->PageCount;
            }

            if (MemoryDescriptor->BasePage < MmLowestPhysicalPage) {
                MmLowestPhysicalPage = MemoryDescriptor->BasePage;
            }

            if ((MemoryDescriptor->BasePage + MemoryDescriptor->PageCount) >
                                                             MmHighestPhysicalPage) {
                MmHighestPhysicalPage =
                        MemoryDescriptor->BasePage + MemoryDescriptor->PageCount - 1;
            }

            //
            // Locate the largest free descriptor.
            //

            if ((MemoryDescriptor->MemoryType == LoaderFree) ||
                (MemoryDescriptor->MemoryType == LoaderLoadedProgram) ||
                (MemoryDescriptor->MemoryType == LoaderFirmwareTemporary) ||
                (MemoryDescriptor->MemoryType == LoaderOsloaderStack)) {

                //
                // Deliberately use >= instead of just > to force our allocation
                // as high as physically possible.  This is to leave low pages
                // for drivers which may require them.
                //

                if (MemoryDescriptor->PageCount >= MostFreePage) {
                    MostFreePage = MemoryDescriptor->PageCount;
                    MxFreeDescriptor = MemoryDescriptor;
                }

                MxTotalFreePages += MemoryDescriptor->PageCount;
            }
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    //
    // This flag is registry-settable so check before overriding.
    //
    // Enabling special IRQL automatically disables mapping the kernel with
    // large pages so we can catch kernel and HAL code.
    //

    if (MmVerifyDriverBufferLength != (ULONG)-1) {
        MmLargePageMinimum = (ULONG)-2;
    }
    else if (MmLargePageMinimum == 0) {
        MmLargePageMinimum = MM_LARGE_PAGE_MINIMUM;
    }

    if (MmNumberOfPhysicalPages <= MmLargePageMinimum) {
        MxMapLargePages = 0;
    }

    //
    // MmDynamicPfn may have been initialized based on the registry to
    // a value representing the highest physical address in gigabytes.
    //

    MmDynamicPfn *= ((1024 * 1024 * 1024) / PAGE_SIZE);

    //
    // Retrieve highest hot plug memory range from the HAL if
    // available and not otherwise retrieved from the registry.
    //

    if (MmDynamicPfn == 0) {

        status = HalQuerySystemInformation(
                     HalQueryMaxHotPlugMemoryAddress,
                     sizeof(PHYSICAL_ADDRESS),
                     (PPHYSICAL_ADDRESS) &MaxHotPlugMemoryAddress,
                     &ReturnedLength);

        if (NT_SUCCESS(status)) {
            ASSERT (ReturnedLength == sizeof(PHYSICAL_ADDRESS));

            MmDynamicPfn = (PFN_NUMBER) (MaxHotPlugMemoryAddress.QuadPart / PAGE_SIZE);
        }
    }

    if (MmDynamicPfn != 0) {
        MmHighestPossiblePhysicalPage = MI_DTC_MAX_PAGES - 1;
        if (MmDynamicPfn - 1 < MmHighestPossiblePhysicalPage) {
            if (MmDynamicPfn - 1 < MmHighestPhysicalPage) {
                MmDynamicPfn = MmHighestPhysicalPage + 1;
            }
            MmHighestPossiblePhysicalPage = MmDynamicPfn - 1;
        }
    }
    else {
        MmHighestPossiblePhysicalPage = MmHighestPhysicalPage;
    }

    //
    // Only machines with at least 5GB of physical memory get to use this.
    //

    if (strstr(LoaderBlock->LoadOptions, "NOLOWMEM")) {
        if (MmNumberOfPhysicalPages >= ((ULONGLONG)5 * 1024 * 1024 * 1024 / PAGE_SIZE)) {
            MiNoLowMemory = (PFN_NUMBER)((ULONGLONG)_4gb / PAGE_SIZE);
        }
    }

    if (MiNoLowMemory != 0) {
        MmMakeLowMemory = TRUE;
    }

    //
    // Save the original descriptor value as everything must be restored
    // prior to this function returning.
    //

    *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldFreeDescriptor = *MxFreeDescriptor;

    if (MmNumberOfPhysicalPages < 2048) {
        KeBugCheckEx(INSTALL_MORE_MEMORY,
                     MmNumberOfPhysicalPages,
                     MmLowestPhysicalPage,
                     MmHighestPhysicalPage,
                     0);
    }

    //
    // Initialize no-execute access permissions.
    //

    for (i = 0; i < 32; i += 1) {
        j = i & 7;
        switch (j) {
            case MM_READONLY:
            case MM_READWRITE:
            case MM_WRITECOPY:
                MmProtectToPteMask[i] |= MmPaeMask;
                break;
            default:
                break;
        }
    }

    ValidUserPte.u.Long |= MmPaeMask;

    //
    // Compute the size of the initial nonpaged pool and the PFN database.
    // This is because we will remove this amount from the free descriptor
    // first and subsequently map it with large TB entries (so it requires
    // natural alignment & size, thus take it before other allocations chip
    // away at the descriptor).
    //

    MiComputeInitialLargePage ();

    //
    // Calculate the starting address for nonpaged system space rounded
    // down to a second level PDE mapping boundary.
    //

    MmNonPagedSystemStart = (PVOID)(((ULONG_PTR)MmPfnDatabase -
                                (((ULONG_PTR)MmNumberOfSystemPtes + 1) * PAGE_SIZE)) &
                                                        (~PAGE_DIRECTORY2_MASK));

    if (MmNonPagedSystemStart < MM_LOWEST_NONPAGED_SYSTEM_START) {
        MmNonPagedSystemStart = MM_LOWEST_NONPAGED_SYSTEM_START;
        MmNumberOfSystemPtes = (ULONG)(((ULONG_PTR)MmPfnDatabase -
                                (ULONG_PTR)MmNonPagedSystemStart) >> PAGE_SHIFT)-1;
        ASSERT (MmNumberOfSystemPtes > 1000);
    }

    //
    // Snap the system PTE start address as page directories and tables
    // will be preallocated for this range.
    //

    SystemPteStart = (PVOID) MmNonPagedSystemStart;

    //
    // If special pool and/or the driver verifier is enabled, reserve
    // extra virtual address space for special pooling now.  For now,
    // arbitrarily don't let it be larger than paged pool (128gb).
    //

    if ((MmVerifyDriverBufferLength != (ULONG)-1) ||
        ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {

        if (MmNonPagedSystemStart > MM_LOWEST_NONPAGED_SYSTEM_START) {
            MaxPool = (ULONG_PTR)MmNonPagedSystemStart -
                      (ULONG_PTR)MM_LOWEST_NONPAGED_SYSTEM_START;
            if (MaxPool > MM_MAX_PAGED_POOL) {
                MaxPool = MM_MAX_PAGED_POOL;
            }
            MmNonPagedSystemStart = (PVOID)((ULONG_PTR)MmNonPagedSystemStart - MaxPool);
        }
        else {

            //
            // This is a pretty large machine.  Take some of the system
            // PTEs and reuse them for special pool.
            //

            MaxPool = (4 * _x4gb);
            ASSERT ((PVOID)MmPfnDatabase > (PVOID)((PCHAR)MmNonPagedSystemStart + MaxPool));
            SystemPteStart = (PVOID)((PCHAR)MmNonPagedSystemStart + MaxPool);

            MmNumberOfSystemPtes = (ULONG)(((ULONG_PTR)MmPfnDatabase -
                            (ULONG_PTR) SystemPteStart) >> PAGE_SHIFT)-1;

        }
        MmSpecialPoolStart = MmNonPagedSystemStart;
        MmSpecialPoolEnd = (PVOID)((ULONG_PTR)MmNonPagedSystemStart + MaxPool);
    }

    //
    // Set the global bit for all PDEs in system space.
    //

    StartPde = MiGetPdeAddress (MM_SYSTEM_SPACE_START);
    EndPde = MiGetPdeAddress (MM_SYSTEM_SPACE_END);
    First = TRUE;

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary(StartPde)) {
            First = FALSE;

            StartPxe = MiGetPdeAddress(StartPde);
            if (StartPxe->u.Hard.Valid == 0) {
                StartPxe += 1;
                StartPpe = MiGetVirtualAddressMappedByPte (StartPxe);
                StartPde = MiGetVirtualAddressMappedByPte (StartPpe);
                continue;
            }

            StartPpe = MiGetPteAddress(StartPde);
            if (StartPpe->u.Hard.Valid == 0) {
                StartPpe += 1;
                StartPde = MiGetVirtualAddressMappedByPte (StartPpe);
                continue;
            }
        }

        TempPte = *StartPde;
        TempPte.u.Hard.Global = 1;
        *StartPde = TempPte;
        StartPde += 1;
    }

    MI_FLUSH_CURRENT_TB ();

    //
    // Allocate page directory parents, directories and page table pages for
    // system PTEs and expansion nonpaged pool.
    //

    TempPte = ValidKernelPte;
    StartPde = MiGetPdeAddress (SystemPteStart);
    EndPde = MiGetPdeAddress ((PCHAR)MmPfnDatabase - 1);

    MxPopulatePageDirectories (StartPde, EndPde);

    StartPde = MiGetPdeAddress ((PVOID)((ULONG_PTR)MmPfnDatabase +
                    (MiInitialLargePageSize << PAGE_SHIFT)));
    EndPde = MiGetPdeAddress ((PCHAR)MmNonPagedPoolEnd - 1);

    MxPopulatePageDirectories (StartPde, EndPde);

    //
    // If the number of physical pages is greater than 255mb and the
    // verifier is not enabled, then map the kernel and HAL images
    // with large pages.
    //

    if (MxMapLargePages != 0) {

        //
        // Add the kernel and HAL ranges to the large page ranges.
        //
        // Ensure any slush in the encompassing large page(s) is always mapped
        // as cached to prevent TB cache attribute conflicts.
        //
        // Note that the MxFreeDescriptor may already be pointing at the slush.
        //

        NextEntry = LoaderBlock->LoadOrderListHead.Flink;

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        //
        // Compute the physical page range spanned by the kernel and the HAL.
        // The potential large page ranges (which may be one single range,
        // or two distinct ranges which could potentially be both physically
        // and virtually discontiguous) these two images reside in must 
        // contain only each other and/or FirmwareTemporary spans.  If there
        // is anything else in the ranges then we cannot guarantee there
        // will be no cache attribute conflicts so don't use large pages.
        //

        TempSlushDescriptor = NULL;
        PointerPte = MiGetPteAddress (DataTableEntry->DllBase);
        ASSERT (MI_IS_PTE_VALID (PointerPte));
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        if ((PageFrameIndex & (MM_PFN_MAPPED_BY_PDE - 1)) == 0) {

            PFN_NUMBER PageFrameIndexLow;
            PKLDR_DATA_TABLE_ENTRY DataTableEntryLow;
            PKLDR_DATA_TABLE_ENTRY DataTableEntrySwap;
            PVOID VirtualAddressLow;

            PageFrameIndexLow = PageFrameIndex;
            DataTableEntryLow = DataTableEntry;

            //
            // The kernel starts on a large page boundary so that's good ...
            //

            VirtualAddressLow = DataTableEntry->DllBase;
            PageFrameIndex += (DataTableEntry->SizeOfImage >> PAGE_SHIFT);

            NextEntry = NextEntry->Flink;

            DataTableEntry = CONTAINING_RECORD (NextEntry,
                                                KLDR_DATA_TABLE_ENTRY,
                                                InLoadOrderLinks);

            PointerPte = MiGetPteAddress (DataTableEntry->DllBase);
            ASSERT (MI_IS_PTE_VALID (PointerPte));

            if (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) + (DataTableEntry->SizeOfImage >> PAGE_SHIFT) == PageFrameIndexLow) {

                //
                // The HAL actually came first followed by the kernel.
                // Invert our locals so we can finish up with a single check
                // below.
                //

                DataTableEntrySwap = DataTableEntryLow;
                DataTableEntryLow = DataTableEntry;
                DataTableEntry = DataTableEntrySwap;

                PointerPte = MiGetPteAddress (DataTableEntryLow->DllBase);
                ASSERT (MI_IS_PTE_VALID (PointerPte));
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                PageFrameIndexLow = PageFrameIndex;
                PageFrameIndex += (DataTableEntryLow->SizeOfImage >> PAGE_SHIFT);
                VirtualAddressLow = DataTableEntryLow->DllBase;

                PointerPte = MiGetPteAddress (DataTableEntry->DllBase);
                ASSERT (MI_IS_PTE_VALID (PointerPte));
            }

            if (PageFrameIndex == MI_GET_PAGE_FRAME_FROM_PTE (PointerPte)) {

                //
                // The kernel and HAL are contiguous, note they may span
                // more than one large page and usually (but not if the
                // second image ends on a large page boundary) has slush
                // on the end.
                //

                PageFrameIndex += (DataTableEntry->SizeOfImage >> PAGE_SHIFT);
                if (PageFrameIndex & (MM_PFN_MAPPED_BY_PDE - 1)) {

                    //
                    // Locate the slush descriptor, if one is not found then
                    // do NOT use large pages.
                    //

                    ASSERT (MxSlushDescriptor1 == NULL);

                    TempSlushDescriptor = MiFindDescriptor (LoaderBlock,
                                                            PageFrameIndex);

                    if (TempSlushDescriptor != NULL) {

                        MxSlushDescriptor1 = TempSlushDescriptor;

                        *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldSlushDescriptor1 =
                                *MxSlushDescriptor1;
                    }
                }
                else {
                    TempSlushDescriptor = (PMEMORY_ALLOCATION_DESCRIPTOR) 1;

                    //
                    // The contiguous images end exactly on a large
                    // page boundary.
                    //

                }

                if (TempSlushDescriptor != NULL) {
                    EndVirtualAddress = (PVOID)((ULONG_PTR) DataTableEntry->DllBase + DataTableEntry->SizeOfImage - 1);

                    MiLargeVaRanges[MiLargeVaRangeIndex].VirtualAddress = VirtualAddressLow;
                    MiLargeVaRanges[MiLargeVaRangeIndex].EndVirtualAddress = EndVirtualAddress;
                    MiLargeVaRangeIndex += 1;
                }
            }
            else {

                //
                // The kernel and HAL are discontiguous, thus they span
                // more than one large page and usually (but not if the image
                // ends on a large page boundary) have slush after each image.
                //
                // Note there may also even be a gap between each large page.
                //
                // The HAL does not immediately follow the kernel so check
                // for a firmware temporary descriptor following each of them
                // that must consume the rest of each large page.  Anything
                // else is unexpected (ie, a downrev loader) and so don't use
                // large pages in that case.
                //

                if (PageFrameIndex & (MM_PFN_MAPPED_BY_PDE - 1)) {

                    //
                    // Locate the slush descriptor, if one is not found then
                    // do NOT use large pages.
                    //

                    ASSERT (MxSlushDescriptor1 == NULL);

                    TempSlushDescriptor = MiFindDescriptor (LoaderBlock,
                                                            PageFrameIndex);

                    if (TempSlushDescriptor != NULL) {

                        MxSlushDescriptor1 = TempSlushDescriptor;
                        *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldSlushDescriptor1 =
                            *MxSlushDescriptor1;
                    }
                }
                else {
                    TempSlushDescriptor = (PMEMORY_ALLOCATION_DESCRIPTOR) 1;
                }

                if (TempSlushDescriptor != NULL) {

                    MiLargeVaRanges[MiLargeVaRangeIndex].VirtualAddress =
                                            DataTableEntryLow->DllBase;

                    EndVirtualAddress = (PVOID)((ULONG_PTR) DataTableEntryLow->DllBase + DataTableEntryLow->SizeOfImage - 1);

                    MiLargeVaRanges[MiLargeVaRangeIndex].EndVirtualAddress = EndVirtualAddress;
                    MiLargeVaRangeIndex += 1;
                }

                //
                // Now check the slush for the second image.
                //

                ASSERT (PointerPte == MiGetPteAddress (DataTableEntry->DllBase));
                ASSERT (MI_IS_PTE_VALID (PointerPte));

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                PageFrameIndex += (DataTableEntry->SizeOfImage >> PAGE_SHIFT);

                if (PageFrameIndex & (MM_PFN_MAPPED_BY_PDE - 1)) {

                    //
                    // Locate the slush descriptor, if one is not found then
                    // do NOT use large pages.
                    //

                    ASSERT (MxSlushDescriptor2 == NULL);

                    TempSlushDescriptor = MiFindDescriptor (LoaderBlock,
                                                            PageFrameIndex);

                    if (TempSlushDescriptor != NULL) {

                        MxSlushDescriptor2 = TempSlushDescriptor;

                        *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldSlushDescriptor2 =
                            *MxSlushDescriptor2;
                    }
                }
                else {
                    TempSlushDescriptor = (PMEMORY_ALLOCATION_DESCRIPTOR) 1;
                }

                if (TempSlushDescriptor != NULL) {

                    MiLargeVaRanges[MiLargeVaRangeIndex].VirtualAddress =
                                            DataTableEntry->DllBase;

                    EndVirtualAddress = (PVOID)((ULONG_PTR) DataTableEntry->DllBase + DataTableEntry->SizeOfImage - 1);

                    MiLargeVaRanges[MiLargeVaRangeIndex].EndVirtualAddress = EndVirtualAddress;
                    MiLargeVaRangeIndex += 1;
                }
            }
        }
    }

    //
    // Allocate page directory pages for the initial large page allocation.
    // Initial nonpaged pool, the PFN database & the color arrays are placed
    // here.
    //

    TempPte = ValidKernelPte;
    TempPde = ValidKernelPde;

    PageFrameIndex = MiInitialLargePage;

    if (MiInitialLargePage != (PFN_NUMBER) -1) {

        StartPpe = MiGetPpeAddress (MmPfnDatabase);
        StartPde = MiGetPdeAddress (MmPfnDatabase);
        EndPde = MiGetPdeAddress ((PVOID)((ULONG_PTR)MmPfnDatabase +
                    (MiInitialLargePageSize << PAGE_SHIFT) - 1));

        MI_MAKE_PDE_MAP_LARGE_PAGE (&TempPde);
    }
    else {
        StartPpe = MiGetPpeAddress (MmNonPagedPoolStart);
        StartPde = MiGetPdeAddress (MmNonPagedPoolStart);
        EndPde = MiGetPdeAddress ((PVOID)((ULONG_PTR)MmNonPagedPoolStart +
                    (MmSizeOfNonPagedPoolInBytes - 1)));
    }

    First = TRUE;

    while (StartPde <= EndPde) {

        if (First == TRUE || MiIsPteOnPdeBoundary (StartPde)) {

            if (First == TRUE || MiIsPteOnPpeBoundary (StartPde)) {

                StartPxe = MiGetPdeAddress (StartPde);

                if (StartPxe->u.Hard.Valid == 0) {

                    //
                    // Map in a page directory parent page, using the
                    // slush descriptor if one exists.
                    //

                    NextPhysicalPage = MxGetNextPage (1, TRUE);
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    MI_WRITE_VALID_PTE (StartPxe, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe),
                                   PAGE_SIZE);
                }
            }

            First = FALSE;

            StartPpe = MiGetPteAddress (StartPde);

            if (StartPpe->u.Hard.Valid == 0) {

                //
                // Map in a page directory page, using the
                // slush descriptor if one exists.
                //

                NextPhysicalPage = MxGetNextPage (1, TRUE);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (StartPpe, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe),
                               PAGE_SIZE);
            }
        }

        ASSERT (StartPde->u.Hard.Valid == 0);

        if (MiInitialLargePage != (PFN_NUMBER) -1) {
            TempPde.u.Hard.PageFrameNumber = PageFrameIndex;
            PageFrameIndex += (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);
            MI_WRITE_VALID_PTE (StartPde, TempPde);
        }
        else {

            //
            // Allocate a page table page here since we're not using large
            // pages, using the slush descriptor if one exists.
            //

            NextPhysicalPage = MxGetNextPage (1, TRUE);
            TempPde.u.Hard.PageFrameNumber = NextPhysicalPage;
            MI_WRITE_VALID_PTE (StartPde, TempPde);
            RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPde),
                           PAGE_SIZE);

            //
            // Allocate data pages here since we're not using large pages.
            //

            PointerPte = MiGetVirtualAddressMappedByPte (StartPde);

            for (i = 0; i < PTE_PER_PAGE; i += 1) {
                NextPhysicalPage = MxGetNextPage (1, FALSE);
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                MI_WRITE_VALID_PTE (PointerPte, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte),
                               PAGE_SIZE);
                PointerPte += 1;
            }
        }

        StartPde += 1;
    }

    MmFreePagesByColor[0] = (PMMCOLOR_TABLES)
                              &MmPfnDatabase[MmHighestPossiblePhysicalPage + 1];

    if (MiInitialLargePage != (PFN_NUMBER) -1) {
        RtlZeroMemory (MmPfnDatabase, MiInitialLargePageSize << PAGE_SHIFT);
    }
    else {

        //
        // Large pages were not used because this machine's physical memory
        // was not contiguous enough.
        //
        // Go through the memory descriptors and for each physical page make
        // sure the PFN database has a valid PTE to map it.  This allows
        // machines with sparse physical memory to have a minimal PFN database.
        //

        NextPhysicalPage = MxFreeDescriptor->BasePage;
        FreeNumberOfPages = MxFreeDescriptor->PageCount;

        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

            MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                                 MEMORY_ALLOCATION_DESCRIPTOR,
                                                 ListEntry);

            if ((MemoryDescriptor->MemoryType == LoaderFirmwarePermanent) ||
                (MemoryDescriptor->MemoryType == LoaderBBTMemory) ||
                (MemoryDescriptor->MemoryType == LoaderSpecialMemory)) {

                //
                // Skip these ranges.
                //

                NextMd = MemoryDescriptor->ListEntry.Flink;
                continue;
            }

            //
            // Temporarily add back in the memory allocated since Phase 0
            // began so PFN entries for it will be created and mapped.
            //
            // Note actual PFN entry allocations must be done carefully as
            // memory from the descriptor itself could get used to map
            // the PFNs for the descriptor !
            //

            if (MemoryDescriptor == MxFreeDescriptor) {
                BasePage = MxOldFreeDescriptor.BasePage;
                PageCount = (PFN_COUNT) MxOldFreeDescriptor.PageCount;
            }
            else if (MemoryDescriptor == MxSlushDescriptor1) {
                BasePage = MxOldSlushDescriptor1.BasePage;
                PageCount = MxOldSlushDescriptor1.PageCount;
            }
            else if (MemoryDescriptor == MxSlushDescriptor2) {
                BasePage = MxOldSlushDescriptor2.BasePage;
                PageCount = MxOldSlushDescriptor2.PageCount;
            }
            else {
                BasePage = MemoryDescriptor->BasePage;
                PageCount = MemoryDescriptor->PageCount;
            }

            PointerPte = MiGetPteAddress (MI_PFN_ELEMENT(BasePage));

            LastPte = MiGetPteAddress (((PCHAR)(MI_PFN_ELEMENT(
                                            BasePage + PageCount))) - 1);

            while (PointerPte <= LastPte) {

                StartPxe = MiGetPpeAddress (PointerPte);

                if (StartPxe->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    NextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (StartPxe, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe),
                                   PAGE_SIZE);
                }

                StartPpe = MiGetPdeAddress (PointerPte);

                if (StartPpe->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    NextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (StartPpe, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe),
                                   PAGE_SIZE);
                }

                StartPde = MiGetPteAddress (PointerPte);

                if (StartPde->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    NextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (StartPde, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPde),
                                   PAGE_SIZE);
                }

                if (PointerPte->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    NextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      3);
                    }
                    MI_WRITE_VALID_PTE (PointerPte, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte),
                                   PAGE_SIZE);
                }
                PointerPte += 1;
            }

            NextMd = MemoryDescriptor->ListEntry.Flink;
        }

        //
        // Ensure the color arrays are mapped.
        //

        PointerPte = MiGetPteAddress (MmFreePagesByColor[0]);
        LastPte = MiGetPteAddress ((PCHAR)MmFreePagesByColor[0] + (2 * MmSecondaryColors * sizeof (MMCOLOR_TABLES)) - 1);

        StartPxe = MiGetPdeAddress (PointerPte);
        StartPpe = MiGetPdeAddress (PointerPte);
        PointerPde = MiGetPteAddress (PointerPte);

        while (PointerPte <= LastPte) {

            if (StartPxe->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                NextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MxOldFreeDescriptor.PageCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (StartPxe, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe), PAGE_SIZE);
            }

            if (StartPpe->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                NextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MxOldFreeDescriptor.PageCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (StartPpe, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe), PAGE_SIZE);
            }

            if (PointerPde->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                NextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MxOldFreeDescriptor.PageCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (PointerPde, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPde), PAGE_SIZE);
            }

            if (PointerPte->u.Hard.Valid == 0) {
                TempPte.u.Hard.PageFrameNumber = NextPhysicalPage;
                ASSERT (FreeNumberOfPages != 0);
                NextPhysicalPage += 1;
                FreeNumberOfPages -= 1;
                if (FreeNumberOfPages == 0) {
                    KeBugCheckEx (INSTALL_MORE_MEMORY,
                                  MmNumberOfPhysicalPages,
                                  FreeNumberOfPages,
                                  MxOldFreeDescriptor.PageCount,
                                  3);
                }
                MI_WRITE_VALID_PTE (PointerPte, TempPte);
                RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte), PAGE_SIZE);
            }

            PointerPte += 1;
            if (MiIsPteOnPdeBoundary (PointerPte)) {
                PointerPde += 1;
                if (MiIsPteOnPdeBoundary (PointerPde)) {
                    StartPpe += 1;
                }
            }
        }

        //
        // Adjust the free descriptor for all the pages we just took.
        //

        MxFreeDescriptor->PageCount -= (LONG)(NextPhysicalPage - MxFreeDescriptor->BasePage);

        MxFreeDescriptor->BasePage = (PFN_COUNT) NextPhysicalPage;
    }

    //
    // Set subsection base to the address to zero as the PTE format allows the
    // complete address space to be spanned.
    //

    MmSubsectionBase = 0;

    //
    // There must be at least one page of system PTEs before the expanded
    // nonpaged pool.
    //

    ASSERT (MiGetPteAddress(SystemPteStart) < MiGetPteAddress(MmNonPagedPoolExpansionStart));

    //
    // Non-paged pages now exist, build the pool structures.
    //

    MiInitializeNonPagedPool ();
    MiInitializeNonPagedPoolThresholds ();

    //
    // Before nonpaged pool can be used, the PFN database must
    // be built.  This is due to the fact that the start and end of
    // allocation bits for nonpaged pool are maintained in the
    // PFN elements for the corresponding pages.
    //

    //
    // Initialize support for colored pages.
    //

    MmFreePagesByColor[1] = &MmFreePagesByColor[0][MmSecondaryColors];

    for (i = 0; i < MmSecondaryColors; i += 1) {
        MmFreePagesByColor[ZeroedPageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByColor[ZeroedPageList][i].Blink = (PVOID) MM_EMPTY_LIST;
        MmFreePagesByColor[ZeroedPageList][i].Count = 0;
        MmFreePagesByColor[FreePageList][i].Flink = MM_EMPTY_LIST;
        MmFreePagesByColor[FreePageList][i].Blink = (PVOID) MM_EMPTY_LIST;
        MmFreePagesByColor[FreePageList][i].Count = 0;
    }

    //
    // Ensure the hyperspace and session spaces are not mapped so they don't
    // get made global by the loops below.
    //

    ASSERT (MiGetPxeAddress (HYPER_SPACE)->u.Hard.Valid == 0);
    ASSERT (MiGetPxeAddress (MM_SESSION_SPACE_DEFAULT)->u.Hard.Valid == 0);

    //
    // Go through the page table entries and for any page which is valid,
    // update the corresponding PFN database element.
    //
    // Skip anything in the 1TB physical range.
    //

    Kseg0StartPte = MiGetPteAddress (MM_KSEG0_BASE);
    Kseg0EndPte = MiGetPteAddress (MM_KSEG2_BASE);

    StartPxe = MiGetPxeAddress (NULL);
    EndPxe = StartPxe + PXE_PER_PAGE;

    for ( ; StartPxe < EndPxe; StartPxe += 1) {

        if (StartPxe->u.Hard.Valid == 0) {
            continue;
        }

        va = MiGetVirtualAddressMappedByPxe (StartPxe);
        ASSERT (va >= MM_SYSTEM_RANGE_START);
        if (MI_IS_PAGE_TABLE_ADDRESS (va)) {
            UseGlobal = 0;
        }
        else {
            UseGlobal = 1;
        }

        ASSERT (StartPxe->u.Hard.LargePage == 0);
        ASSERT (StartPxe->u.Hard.Owner == 0);
        ASSERT (StartPxe->u.Hard.Global == 0);

        PxePage = MI_GET_PAGE_FRAME_FROM_PTE(StartPxe);

        if (MiIsRegularMemory (LoaderBlock, PxePage)) {

            Pfn1 = MI_PFN_ELEMENT(PxePage);

            Pfn1->u4.PteFrame = (DirBase >> PAGE_SHIFT);
            Pfn1->PteAddress = StartPxe;
            Pfn1->u2.ShareCount += 1;
            Pfn1->u3.e2.ReferenceCount = 1;
            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            Pfn1->u3.e1.CacheAttribute = MiCached;
            MiDetermineNode (PxePage, Pfn1);
        }
        else {
            Pfn1 = NULL;
        }

        StartPpe = MiGetVirtualAddressMappedByPte (StartPxe);
        EndPpe = StartPpe + PPE_PER_PAGE;

        for ( ; StartPpe < EndPpe; StartPpe += 1) {

            if (StartPpe->u.Hard.Valid == 0) {
                continue;
            }

            ASSERT (StartPpe->u.Hard.LargePage == 0);
            ASSERT (StartPpe->u.Hard.Owner == 0);
            ASSERT (StartPpe->u.Hard.Global == 0);

            PpePage = MI_GET_PAGE_FRAME_FROM_PTE (StartPpe);

            if (MiIsRegularMemory (LoaderBlock, PpePage)) {

                Pfn2 = MI_PFN_ELEMENT (PpePage);

                Pfn2->u4.PteFrame = PxePage;
                Pfn2->PteAddress = StartPpe;
                Pfn2->u2.ShareCount += 1;
                Pfn2->u3.e2.ReferenceCount = 1;
                Pfn2->u3.e1.PageLocation = ActiveAndValid;
                Pfn2->u3.e1.CacheAttribute = MiCached;
                MiDetermineNode (PpePage, Pfn2);
            }
            else {
                Pfn2 = NULL;
            }

            ASSERT (Pfn1 != NULL);
            Pfn1->u2.ShareCount += 1;

            StartPde = MiGetVirtualAddressMappedByPte (StartPpe);
            EndPde = StartPde + PDE_PER_PAGE;

            for ( ; StartPde < EndPde; StartPde += 1) {

                if (StartPde->u.Hard.Valid == 0) {
                    continue;
                }

                ASSERT (StartPde->u.Hard.Owner == 0);
                StartPde->u.Hard.Global = UseGlobal;

                PdePage = MI_GET_PAGE_FRAME_FROM_PTE (StartPde);

                if (MiIsRegularMemory (LoaderBlock, PdePage)) {

                    Pfn3 = MI_PFN_ELEMENT (PdePage);

                    Pfn3->u4.PteFrame = PpePage;
                    Pfn3->PteAddress = StartPde;
                    Pfn3->u2.ShareCount += 1;
                    Pfn3->u3.e2.ReferenceCount = 1;
                    Pfn3->u3.e1.PageLocation = ActiveAndValid;
                    Pfn3->u3.e1.CacheAttribute = MiCached;
                    MiDetermineNode (PdePage, Pfn3);
                }
                else {
                    Pfn3 = NULL;
                }

                ASSERT (Pfn2 != NULL);
                Pfn2->u2.ShareCount += 1;

                StartPte = MiGetVirtualAddressMappedByPte (StartPde);

                if (StartPde->u.Hard.LargePage == 1) {
                    if (Pfn3 != NULL) {
                        Pfn4 = Pfn3;
                        for (i = 0; i < PDE_PER_PAGE; i += 1) {
                            Pfn4->u4.PteFrame = PpePage;
                            Pfn4->PteAddress = StartPte;
                            Pfn4->u2.ShareCount += 1;
                            Pfn4->u3.e2.ReferenceCount = 1;
                            Pfn4->u3.e1.PageLocation = ActiveAndValid;
                            Pfn4->u3.e1.CacheAttribute = MiCached;
                            MiDetermineNode (PdePage + i, Pfn4);
                            StartPte += 1;
                            Pfn4 += 1;
                        }
                    }
                }
                else {

                    EndPte = StartPte + PDE_PER_PAGE;

                    for ( ; StartPte < EndPte; StartPte += 1) {

                        if (StartPte->u.Hard.Valid == 0) {
                            continue;
                        }

                        if (StartPte->u.Hard.LargePage == 1) {
                            continue;
                        }

                        ASSERT (StartPte->u.Hard.Owner == 0);
                        StartPte->u.Hard.Global = UseGlobal;

                        PtePage = MI_GET_PAGE_FRAME_FROM_PTE (StartPte);

                        ASSERT (Pfn3 != NULL);
                        Pfn3->u2.ShareCount += 1;

                        if (!MiIsRegularMemory (LoaderBlock, PtePage)) {
                            continue;
                        }

                        Pfn4 = MI_PFN_ELEMENT (PtePage);

                        if ((MmIsAddressValid(Pfn4)) &&
                             MmIsAddressValid((PUCHAR)(Pfn4+1)-1)) {

                            if ((StartPte >= Kseg0StartPte) && (StartPte < Kseg0EndPte)) {
                                if (Pfn4->u3.e1.PageLocation == ActiveAndValid) {
                                    //
                                    // Ignore stale KSEG0 mappings - the fact
                                    // that the page is valid means we have
                                    // already used it for a lower virtual
                                    // address (like the system cache WSLEs).
                                    //

                                    continue;
                                }
                            }

                            Pfn4->u4.PteFrame = PdePage;
                            Pfn4->PteAddress = StartPte;
                            Pfn4->u2.ShareCount += 1;
                            Pfn4->u3.e2.ReferenceCount = 1;
                            Pfn4->u3.e1.PageLocation = ActiveAndValid;
                            Pfn4->u3.e1.CacheAttribute = MiCached;
                            MiDetermineNode (PtePage, Pfn4);
                        }
                    }
                }
            }
        }
    }

    //
    // If the lowest physical page is zero and the page is still unused, mark
    // it as in use. This is because we want to find bugs where a physical
    // page is specified as zero.
    //

    Pfn1 = &MmPfnDatabase[MmLowestPhysicalPage];

    if ((MmLowestPhysicalPage == 0) && (Pfn1->u3.e2.ReferenceCount == 0)) {

        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);

        //
        // Make the reference count non-zero and point it into a
        // page directory.
        //

        Pde = MiGetPxeAddress (0xFFFFFFFFB0000000);
        PdePage = MI_GET_PAGE_FRAME_FROM_PTE (Pde);
        Pfn1->u4.PteFrame = PdePage;
        Pfn1->PteAddress = Pde;
        Pfn1->u2.ShareCount += 1;
        Pfn1->u3.e2.ReferenceCount = 0xfff0;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        Pfn1->u3.e1.CacheAttribute = MiCached;
        MiDetermineNode (0, Pfn1);
    }

    //
    // Walk through the memory descriptors and add pages to the
    // free list in the PFN database.
    //
    // Since the LoaderBlock memory descriptors are ordered
    // from low physical memory address to high, walk it backwards so the
    // high physical pages go to the front of the freelists.  The thinking
    // is that pages initially allocated by the system are less likely to be
    // freed so don't waste memory below 16mb (or 4gb) that may be needed
    // by ISA drivers later.
    //

    NextMd = LoaderBlock->MemoryDescriptorListHead.Blink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        i = MemoryDescriptor->PageCount;
        PageFrameIndex = MemoryDescriptor->BasePage;

        if ((MemoryDescriptor == MxSlushDescriptor1) ||
            (MemoryDescriptor == MxSlushDescriptor2)) {

            //
            // Excess slush is given to expansion nonpaged pool here to ensure
            // that it is always mapped fully cached since the rest of the
            // large page is inserted as fully cached into the TB.
            //

            MiAddExpansionNonPagedPool (PageFrameIndex, i, FALSE);
            NextMd = MemoryDescriptor->ListEntry.Blink;
            continue;
        }

        switch (MemoryDescriptor->MemoryType) {
            case LoaderBad:

                if (PageFrameIndex > MmHighestPhysicalPage) {
                    i = 0;
                }
                else if (PageFrameIndex + i > MmHighestPhysicalPage + 1) {
                    i = MmHighestPhysicalPage + 1 - PageFrameIndex;
                }

                LOCK_PFN (OldIrql);

                while (i != 0) {
                    MiInsertPageInList (&MmBadPageListHead, PageFrameIndex);
                    i -= 1;
                    PageFrameIndex += 1;
                }

                UNLOCK_PFN (OldIrql);

                break;

            case LoaderFree:
            case LoaderLoadedProgram:
            case LoaderFirmwareTemporary:
            case LoaderOsloaderStack:

                PageFrameIndex += (i - 1);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                LOCK_PFN (OldIrql);

                while (i != 0) {
                    if (Pfn1->u3.e2.ReferenceCount == 0) {

                        //
                        // Set the PTE address to the physical page for
                        // virtual address alignment checking.
                        //

                        Pfn1->PteAddress =
                                        (PMMPTE)(PageFrameIndex << PTE_SHIFT);
                        Pfn1->u3.e1.CacheAttribute = MiCached;
                        MiDetermineNode (PageFrameIndex, Pfn1);
                        MiInsertPageInFreeList (PageFrameIndex);
                    }

                    Pfn1 -= 1;
                    i -= 1;
                    PageFrameIndex -= 1;
                }

                UNLOCK_PFN (OldIrql);

                break;

            case LoaderFirmwarePermanent:
            case LoaderSpecialMemory:
            case LoaderBBTMemory:

                //
                // Skip these ranges.
                //

                break;

            default:

                PointerPte = MiGetPteAddress (KSEG0_BASE +
                                            (PageFrameIndex << PAGE_SHIFT));

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                while (i != 0) {

                    //
                    // Set page as in use.
                    //

                    PointerPde = MiGetPdeAddress (KSEG0_BASE +
                                             (PageFrameIndex << PAGE_SHIFT));

                    if (Pfn1->u3.e2.ReferenceCount == 0) {
                        Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE(PointerPde);
                        Pfn1->PteAddress = PointerPte;
                        Pfn1->u2.ShareCount += 1;
                        Pfn1->u3.e2.ReferenceCount = 1;
                        Pfn1->u3.e1.PageLocation = ActiveAndValid;
                        Pfn1->u3.e1.CacheAttribute = MiCached;
                        MiDetermineNode (PageFrameIndex, Pfn1);

                        if (MemoryDescriptor->MemoryType == LoaderXIPRom) {
                            Pfn1->u1.Flink = 0;
                            Pfn1->u2.ShareCount = 0;
                            Pfn1->u3.e2.ReferenceCount = 0;
                            Pfn1->u3.e1.PageLocation = 0;
                            Pfn1->u3.e1.CacheAttribute = MiCached;
                            Pfn1->u3.e1.Rom = 1;
                            Pfn1->u4.InPageError = 0;
                            Pfn1->u3.e1.PrototypePte = 1;
                        }
                    }
                    Pfn1 += 1;
                    i -= 1;
                    PageFrameIndex += 1;
                    PointerPte += 1;
                }

                break;
        }

        NextMd = MemoryDescriptor->ListEntry.Blink;
    }

    //
    // If the large page chunk came from the middle of the free descriptor (due
    // to alignment requirements), then add the pages from the split bottom
    // portion of the free descriptor now.
    //

    i = MiSlushDescriptorCount;
    NextPhysicalPage = MiSlushDescriptorBase;
    Pfn1 = MI_PFN_ELEMENT (NextPhysicalPage);

    LOCK_PFN (OldIrql);

    while (i != 0) {
        if (Pfn1->u3.e2.ReferenceCount == 0) {

            //
            // Set the PTE address to the physical page for
            // virtual address alignment checking.
            //

            Pfn1->PteAddress = (PMMPTE)(NextPhysicalPage << PTE_SHIFT);
            Pfn1->u3.e1.CacheAttribute = MiCached;
            MiDetermineNode (NextPhysicalPage, Pfn1);
            MiInsertPageInFreeList (NextPhysicalPage);
        }
        Pfn1 += 1;
        i -= 1;
        NextPhysicalPage += 1;
    }

    UNLOCK_PFN (OldIrql);

    //
    // Mark all PFN entries for the PFN pages in use.
    //

    if (MiInitialLargePage != (PFN_NUMBER) -1) {

        //
        // All PFN entries for the PFN pages in use better be marked as such.
        //

        PointerPde = MiGetPdeAddress (MmPfnDatabase);
        ASSERT (PointerPde->u.Hard.LargePage == 1);
        PageFrameIndex = (PFN_NUMBER) PointerPde->u.Hard.PageFrameNumber;
        Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
        i = MxPfnAllocation;

        do {
            Pfn1->PteAddress = (PMMPTE)(PageFrameIndex << PTE_SHIFT);
            ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
            ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
            PageFrameIndex += 1;
            Pfn1 += 1;
            i -= 1;
        } while (i != 0);

        if (MmDynamicPfn == 0) {

            //
            // Scan the PFN database backward for pages that are completely
            // zero.  These pages are unused and can be added to the free list.
            //
            // This allows machines with sparse physical memory to have a
            // minimal PFN database even when mapped with large pages.
            //

            BottomPfn = MI_PFN_ELEMENT(MmHighestPhysicalPage);

            do {

                //
                // Compute the address of the start of the page that is next
                // lower in memory and scan backwards until that page address
                // is reached or just crossed.
                //

                if (((ULONG_PTR)BottomPfn & (PAGE_SIZE - 1)) != 0) {
                    BasePfn = (PMMPFN)((ULONG_PTR)BottomPfn & ~(PAGE_SIZE - 1));
                    TopPfn = BottomPfn + 1;

                }
                else {
                    BasePfn = (PMMPFN)((ULONG_PTR)BottomPfn - PAGE_SIZE);
                    TopPfn = BottomPfn;
                }

                while (BottomPfn > BasePfn) {
                    BottomPfn -= 1;
                }

                //
                // If the entire range over which the PFN entries span is
                // completely zero and the PFN entry that maps the page is
                // not in the range, then add the page to the free list.
                //

                Range = (ULONG_PTR)TopPfn - (ULONG_PTR)BottomPfn;
                if (RtlCompareMemoryUlong ((PVOID)BottomPfn, Range, 0) == Range) {

                    //
                    // Set the PTE address to the physical page for virtual
                    // address alignment checking.
                    //

                    PointerPde = MiGetPdeAddress (BasePfn);
                    ASSERT (PointerPde->u.Hard.LargePage == 1);
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde) + MiGetPteOffset (BasePfn);

                    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                    ASSERT (Pfn1->PteAddress == (PMMPTE)(PageFrameIndex << PTE_SHIFT));
                    Pfn1->u3.e2.ReferenceCount = 0;
                    Pfn1->PteAddress = (PMMPTE)(PageFrameIndex << PTE_SHIFT);
                    Pfn1->u3.e1.CacheAttribute = MiCached;
                    MiDetermineNode (PageFrameIndex, Pfn1);
                    MiAddExpansionNonPagedPool (PageFrameIndex, 1, FALSE);
                }
            } while (BottomPfn > MmPfnDatabase);
        }
    }
    else {

        //
        // The PFN database is sparsely allocated in small pages.
        //

        PointerPte = MiGetPteAddress (MmPfnDatabase);

        LastPte = MiGetPteAddress (MmPfnDatabase + MmHighestPhysicalPage + 1);
        if (LastPte != PAGE_ALIGN (LastPte)) {
            LastPte += 1;
        }

        StartPxe = MiGetPpeAddress (PointerPte);
        StartPpe = MiGetPdeAddress (PointerPte);
        PointerPde = MiGetPteAddress (PointerPte);

        while (PointerPte < LastPte) {

            if (StartPxe->u.Hard.Valid == 0) {
                StartPxe += 1;
                StartPpe = MiGetVirtualAddressMappedByPte (StartPxe);
                PointerPde = MiGetVirtualAddressMappedByPte (StartPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                continue;
            }

            if (StartPpe->u.Hard.Valid == 0) {
                StartPpe += 1;
                StartPxe = MiGetPteAddress (StartPpe);
                PointerPde = MiGetVirtualAddressMappedByPte (StartPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                continue;
            }

            if (PointerPde->u.Hard.Valid == 0) {
                PointerPde += 1;
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                if (MiIsPteOnPdeBoundary (PointerPde)) {
                    StartPpe += 1;
                    if (MiIsPteOnPdeBoundary (StartPpe)) {
                        StartPxe += 1;
                    }
                }
                continue;
            }

            if (PointerPte->u.Hard.Valid == 1) {

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                Pfn1->PteAddress = PointerPte;
                Pfn1->u3.e1.PageColor = 0;
                Pfn1->u3.e2.ReferenceCount = 1;
                Pfn1->u3.e1.PageLocation = ActiveAndValid;
                Pfn1->u3.e1.CacheAttribute = MiCached;
            }

            PointerPte += 1;
            if (MiIsPteOnPdeBoundary (PointerPte)) {
                PointerPde += 1;
                if (MiIsPteOnPdeBoundary (PointerPde)) {
                    StartPpe += 1;
                    if (MiIsPteOnPdeBoundary (StartPpe)) {
                        StartPxe += 1;
                    }
                }
            }
        }
    }

    //
    // Initialize the nonpaged pool.
    //

    InitializePool (NonPagedPool, 0);

    //
    // Adjust the memory descriptor to indicate that free pool has
    // been used for nonpaged pool creation.
    //
    // N.B.  This is required because the descriptors are walked upon
    // return from this routine to create the MmPhysicalMemoryBlock.
    //

    *MxFreeDescriptor = *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldFreeDescriptor;

    if (MxSlushDescriptor1 != NULL) {
        *MxSlushDescriptor1 = *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldSlushDescriptor1;
    }

    if (MxSlushDescriptor2 != NULL) {
        *MxSlushDescriptor2 = *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldSlushDescriptor2;
    }

    //
    //
    // Initialize the system PTE pool now that nonpaged pool exists.
    //

    PointerPte = MiGetPteAddress (SystemPteStart);
    ASSERT (((ULONG_PTR)PointerPte & (PAGE_SIZE - 1)) == 0);

    MmNumberOfSystemPtes = (ULONG)(MiGetPteAddress (MmPfnDatabase) - PointerPte - 1);

    MiInitializeSystemPtes (PointerPte, MmNumberOfSystemPtes, SystemPteSpace);

    //
    // Initialize the debugger PTE.
    //

    MmDebugPte = MiReserveSystemPtes (1, SystemPteSpace);

    MmDebugPte->u.Long = 0;

    MmDebugVa = MiGetVirtualAddressMappedByPte (MmDebugPte);

    MmCrashDumpPte = MiReserveSystemPtes (16, SystemPteSpace);

    MmCrashDumpVa = MiGetVirtualAddressMappedByPte (MmCrashDumpPte);

    //
    // Allocate a page directory and a pair of page table pages.
    // Map the hyper space page directory page into the top level parent
    // directory & the hyper space page table page into the page directory
    // and map an additional page that will eventually be used for the
    // working set list.  Page tables after the first two are set up later
    // on during individual process working set initialization.
    //
    // The working set list page will eventually be a part of hyper space.
    // It is mapped into the second level page directory page so it can be
    // zeroed and so it will be accounted for in the PFN database. Later
    // the page will be unmapped, and its page frame number captured in the
    // system process object.
    //

    TempPte = ValidKernelPte;
    TempPte.u.Hard.Global = 0;

    StartPxe = MiGetPxeAddress (HYPER_SPACE);
    StartPpe = MiGetPpeAddress (HYPER_SPACE);
    StartPde = MiGetPdeAddress (HYPER_SPACE);

    LOCK_PFN (OldIrql);

    if (StartPxe->u.Hard.Valid == 0) {
        ASSERT (StartPxe->u.Long == 0);
        TempPte.u.Hard.PageFrameNumber = MiRemoveAnyPage (0);
        *StartPxe = TempPte;
        RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPxe), PAGE_SIZE);
    }
    else {
        ASSERT (StartPxe->u.Hard.Global == 0);
    }

    if (StartPpe->u.Hard.Valid == 0) {
        ASSERT (StartPpe->u.Long == 0);
        TempPte.u.Hard.PageFrameNumber = MiRemoveAnyPage (0);
        *StartPpe = TempPte;
        RtlZeroMemory (MiGetVirtualAddressMappedByPte (StartPpe), PAGE_SIZE);
    }
    else {
        ASSERT (StartPpe->u.Hard.Global == 0);
    }

    TempPte.u.Hard.PageFrameNumber = MiRemoveAnyPage (0);
    *StartPde = TempPte;

    //
    // Zero the hyper space page table page.
    //

    StartPte = MiGetPteAddress (HYPER_SPACE);
    RtlZeroMemory (StartPte, PAGE_SIZE);

    PageFrameIndex = MiRemoveAnyPage (0);

    UNLOCK_PFN (OldIrql);

    //
    // Hyper space now exists, set the necessary variables.
    //

    MmFirstReservedMappingPte = MiGetPteAddress (FIRST_MAPPING_PTE);
    MmLastReservedMappingPte = MiGetPteAddress (LAST_MAPPING_PTE);

    //
    // Create zeroing PTEs for the zero page thread.
    //

    MiFirstReservedZeroingPte = MiReserveSystemPtes (NUMBER_OF_ZEROING_PTES + 1, SystemPteSpace);

    RtlZeroMemory (MiFirstReservedZeroingPte,
                   (NUMBER_OF_ZEROING_PTES + 1) * sizeof(MMPTE));

    //
    // Use the page frame number field of the first PTE as an
    // offset into the available zeroing PTEs.
    //

    MiFirstReservedZeroingPte->u.Hard.PageFrameNumber = NUMBER_OF_ZEROING_PTES;

    //
    // Create the VAD bitmap for this process.
    //

    PointerPte = MiGetPteAddress (VAD_BITMAP_SPACE);

    //
    // Note the global bit must be off for the bitmap data.
    //

    TempPte = ValidKernelPteLocal;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    //
    // Point to the page we just created and zero it.
    //

    RtlZeroMemory (VAD_BITMAP_SPACE, PAGE_SIZE);

    MiLastVadBit = (ULONG)((((ULONG_PTR) MI_64K_ALIGN (MM_HIGHEST_VAD_ADDRESS))) / X64K);
    if (MiLastVadBit > PAGE_SIZE * 8 - 1) {
        MiLastVadBit = PAGE_SIZE * 8 - 1;
    }

    //
    // Initialize this process's memory management structures including
    // the working set list.
    //

    CurrentProcess = PsGetCurrentProcess ();

    //
    // The PFN element for the page directory has already been initialized,
    // zero the reference count and the share count so they won't be wrong.
    //

    Pfn1 = MI_PFN_ELEMENT (PdePageNumber);

    LOCK_PFN (OldIrql);

    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e2.ReferenceCount = 0;

    //
    // Get a page for the working set list and zero it.
    //

    PageFrameIndex = MiRemoveAnyPage (0);

    UNLOCK_PFN (OldIrql);

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPte = MiGetPteAddress (MmWorkingSetList);
    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    RtlZeroMemory (MmWorkingSetList, PAGE_SIZE);

    CurrentProcess->WorkingSetPage = PageFrameIndex;

    CurrentProcess->Vm.MaximumWorkingSetSize = (ULONG)MmSystemProcessWorkingSetMax;
    CurrentProcess->Vm.MinimumWorkingSetSize = (ULONG)MmSystemProcessWorkingSetMin;

    DummyFlags = 0;
    MmInitializeProcessAddressSpace (CurrentProcess, NULL, NULL, &DummyFlags, NULL);

    return;
}

DECLSPEC_NOINLINE
VOID
MiGetStackPointer (
    OUT PULONG_PTR StackPointer
    )

/*++

Routine Description:

    This routine retrieves the stack pointer of the calling function.

Arguments:

    StackPointer - Supplies a pointer to the stack pointer return value.

Return Value:

    None.

Environment:

    Any.

--*/

{
    //
    // The AMD64 calling convention dictates that the home address of a
    // callee's first parameter is the caller's stack pointer.
    //

    *StackPointer = (ULONG_PTR)&StackPointer;
}

