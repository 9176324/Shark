/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    init386.c

Abstract:

    This module contains the machine dependent initialization for the
    memory management component.  It is specifically tailored to the
    INTEL x86 and PAE machines.

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
#pragma alloc_text(INIT,MiIsRegularMemory)
#pragma alloc_text(INIT,MiFindDescriptor)
#endif

#define MM_LARGE_PAGE_MINIMUM  ((255*1024*1024) >> PAGE_SHIFT)

extern ULONG MmLargeSystemCache;
extern ULONG MmLargeStackSize;
extern LOGICAL MmMakeLowMemory;
extern LOGICAL MmPagedPoolMaximumDesired;

PVOID MmHyperSpaceEnd;

//
// Local data.
//

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif

ULONG MxPfnAllocation;

PMEMORY_ALLOCATION_DESCRIPTOR MxFreeDescriptor;
PMEMORY_ALLOCATION_DESCRIPTOR MxSlushDescriptor1;
PMEMORY_ALLOCATION_DESCRIPTOR MxSlushDescriptor2;

PFN_NUMBER MiSlushDescriptorBase;
PFN_NUMBER MiSlushDescriptorCount;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

MEMORY_ALLOCATION_DESCRIPTOR MxOldFreeDescriptor;
MEMORY_ALLOCATION_DESCRIPTOR MxOldSlushDescriptor1;
MEMORY_ALLOCATION_DESCRIPTOR MxOldSlushDescriptor2;

typedef struct _MI_LARGE_VA_RANGES {
    PVOID VirtualAddress;
    PVOID EndVirtualAddress;
} MI_LARGE_VA_RANGES, *PMI_LARGE_VA_RANGES;

//
// There are potentially 3 large page ranges:
//
// 1. PFN database + initial nonpaged pool
// 2. Kernel code/data
// 3. HAL code/data
//

#define MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL     0x1
#define MI_LARGE_KERNEL_HAL                         0x2

#define MI_LARGE_ALL                                0x3

ULONG MxMapLargePages = MI_LARGE_ALL;

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

    MxFreeDescriptor->BasePage += PagesNeeded;
    MxFreeDescriptor->PageCount -= PagesNeeded;

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

    MI_SET_GLOBAL_STATE (TempPde, 1);

    LOCK_PFN (OldIrql);

    do {
        ASSERT (PointerPde->u.Hard.Valid == 1);

        if (PointerPde->u.Hard.LargePage == 1) {
            goto skip;
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
            goto skip;
        }

        TempPde.u.Hard.PageFrameNumber = LargePageBaseFrame;

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);

        MI_WRITE_VALID_PTE_NEW_PAGE (PointerPde, TempPde);

        //
        // Update the idle process to use the large page mapping also as
        // the page table page is going to be freed.
        //

        MmSystemPagePtes [((ULONG_PTR)PointerPde &
            (PD_PER_SYSTEM * (sizeof(MMPTE) * PDE_PER_PAGE) - 1)) / sizeof(MMPTE)] = TempPde;

        MI_FLUSH_ENTIRE_TB (0xF);

        //
        // If booted /3GB, then retain the original page table page because
        // there is a temporary alternate mapping at 8xxx.xxxx to the
        // kernel/HAL, registry and other boot loader stuff.
        //

        if (MmVirtualBias == 0) {

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

skip:
        PointerPde += 1;
    } while (PointerPde <= LastPde);

    UNLOCK_PFN (OldIrql);
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
    PFN_NUMBER PageFrameIndex2;

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
                "MM: Loader/HAL memory block indicates large pages cannot be used for %p->%p\n",
                MiLargeVaRanges[i].VirtualAddress,
                MiLargeVaRanges[i].EndVirtualAddress);

            MiLargeVaRanges[i].VirtualAddress = NULL;

            //
            // Don't use large pages for anything if this chunk overlaps any
            // others in the request list.  This is because 2 separate ranges
            // may share a straddling large page.  If the first range was unable
            // to use large pages, but the second one does ... then only part
            // of the first range will get large pages if we enable large
            // pages for the second range.  This would be vey bad as we use
            // the MI_IS_PHYSICAL macro everywhere and assume the entire
            // range is in or out, so disable all large pages here instead.
            //

            for (j = 0; j < MiLargeVaRangeIndex; j += 1) {

                //
                // Skip the range that is already being rejected.
                //

                if (i == j) {
                    continue;
                }

                //
                // Skip any range which has already been removed.
                //

                if (MiLargeVaRanges[j].VirtualAddress == NULL) {
                    continue;
                }

                PointerPte = MiGetPteAddress (MiLargeVaRanges[j].VirtualAddress);
                ASSERT (PointerPte->u.Hard.Valid == 1);
                PageFrameIndex2 = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                if ((PageFrameIndex2 >= PageFrameIndex) &&
                    (PageFrameIndex2 <= LastPageFrameIndex)) {

                    DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                        "MM: Disabling large pages for all ranges due to overlap\n");

                    goto RemoveAllRanges;
                }

                //
                // Since it is not possible for any request chunk to completely
                // encompass another one, checking only the start and end
                // addresses is sufficient.
                //

                PointerPte = MiGetPteAddress (MiLargeVaRanges[j].EndVirtualAddress);
                ASSERT (PointerPte->u.Hard.Valid == 1);
                PageFrameIndex2 = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

                if ((PageFrameIndex2 >= PageFrameIndex) &&
                    (PageFrameIndex2 <= LastPageFrameIndex)) {

                    DbgPrintEx (DPFLTR_MM_ID, DPFLTR_WARNING_LEVEL, 
                        "MM: Disabling large pages for all ranges due to overlap\n");

                    goto RemoveAllRanges;
                }
            }

            //
            // No other ranges overlapped with this one, it is sufficient to
            // just disable this range and continue to attempt to use large
            // pages for any others.
            //

            continue;
        }

        MiAddCachedRange (PageFrameIndex, LastPageFrameIndex);
    }

    return;

RemoveAllRanges:

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
            // Round the start down to a page directory boundary and the end to
            // the last page directory entry before the next boundary.
            //

            PageFrameIndex &= ~(MM_PFN_MAPPED_BY_PDE - 1);
            LastPageFrameIndex |= (MM_PFN_MAPPED_BY_PDE - 1);

            MiRemoveCachedRange (PageFrameIndex, LastPageFrameIndex);
        }
    }

    MiLargeVaRangeIndex = 0;

    return;
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

#define MI_IS_PTE_VALID(PTE) ((PTE)->u.Hard.Valid == 1)


VOID
MiInitMachineDependent (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

/*++

Routine Description:

    This routine performs the necessary operations to enable virtual
    memory.  This includes building the page directory page, building
    page table pages to map the code section, the data section, the
    stack section and the trap handler.

    It also initializes the PFN database and populates the free list.

Arguments:

    LoaderBlock  - Supplies a pointer to the firmware setup loader block.

Return Value:

    None.

Environment:

    Kernel mode.

    N.B.  This routine uses memory from the loader block descriptors, but
    the descriptors themselves must be restored prior to return as our caller
    walks them to create the MmPhysicalMemoryBlock.

--*/

{
    ULONG ExtraPtes1;
    PMMPTE ExtraPtes1Pointer;
    ULONG ExtraPtes2;
    PFN_NUMBER TotalFreePages;
    LOGICAL InitialNonPagedPoolSetViaRegistry;
    PMMPFN AlignedPfnDatabase;
    PHYSICAL_ADDRESS MaxHotPlugMemoryAddress;
    ULONG Bias;
    PMMPFN BasePfn;
    PMMPFN BottomPfn;
    PMMPFN TopPfn;
    PFN_NUMBER FirstPfnDatabasePage;
    ULONG BasePage;
    PFN_NUMBER PagesLeft;
    ULONG Range;
    PFN_NUMBER PageCount;
    ULONG i, j;
    PFN_NUMBER PdePageNumber;
    PFN_NUMBER PdePage;
    PFN_NUMBER PageFrameIndex;
    ULONG MaxPool;
    PEPROCESS CurrentProcess;
    ULONG DirBase;
    ULONG MostFreePage;
    PFN_NUMBER PagesNeeded;
    PLIST_ENTRY NextMd;
    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR TempSlushDescriptor;
    MMPTE TempPde;
    MMPTE TempPte;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE Pde;
    PMMPTE StartPde;
    PMMPTE EndPde;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    ULONG PdeCount;
    ULONG va;
    KIRQL OldIrql;
    PVOID VirtualAddress;
    PVOID EndVirtualAddress;
    PVOID NonPagedPoolStartVirtual;
    PVOID NonPagedPoolStartLow;
    LOGICAL ExtraSystemCacheViews;
    SIZE_T MaximumNonPagedPoolInBytesLimit;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    ULONG ReturnedLength;
    NTSTATUS status;
    UCHAR Associativity;
    ULONG NonPagedSystemStart;
    LOGICAL PagedPoolMaximumDesired;
    SIZE_T NumberOfBytes;
    ULONG DummyFlags;

    if (InitializationPhase == 1) {

        //
        // If *ALL* the booted processors support large pages, and the
        // number of physical pages is greater than the threshold, then map
        // the kernel image, HAL, PFN database and initial nonpaged pool
        // with large pages.
        //

        if ((KeFeatureBits & KF_LARGE_PAGE) && (MxMapLargePages != 0)) {
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
    ASSERT (MxMapLargePages == MI_LARGE_ALL);

    ExtraPtes1 = 0;
    ExtraPtes2 = 0;
    ExtraPtes1Pointer = NULL;
    ExtraSystemCacheViews = FALSE;
    MostFreePage = 0;
    NonPagedPoolStartLow = NULL;
    PagedPoolMaximumDesired = FALSE;

    //
    // Initializing these is not needed for correctness, but without it
    // the compiler cannot compile this code W4 to check for use of
    // uninitialized variables.
    //

    FirstPfnDatabasePage = 0;
    MaximumNonPagedPoolInBytesLimit = 0;

    //
    // If the chip doesn't support large pages then disable large page support.
    //

    if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
        MxMapLargePages = 0;
    }

    //
    // This flag is registry-settable so check before overriding.
    //

    if (MmProtectFreedNonPagedPool == TRUE) {
        MxMapLargePages &= ~MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL;
    }

    //
    // Sanitize this registry-specifiable large stack size.  Note the registry
    // size is in 1K chunks, ie: 32 means 32k.  Note also that the registry
    // setting does not include the guard page and we don't want to burden
    // administrators with knowing about it so we automatically subtract one
    // page from their request.
    //

    if (MmLargeStackSize > (KERNEL_LARGE_STACK_SIZE / 1024)) {

        //
        // No registry override or the override is too high.
        // Set it to the default.
        //

        MmLargeStackSize = KERNEL_LARGE_STACK_SIZE;
    }
    else {

        //
        // Convert registry override from 1K units to bytes.  Note intelligent
        // choices are 16k or 32k because we bin those sizes in sysptes.
        //

        MmLargeStackSize *= 1024;
        MmLargeStackSize = MI_ROUND_TO_SIZE (MmLargeStackSize, PAGE_SIZE);
        MmLargeStackSize -= PAGE_SIZE;
        ASSERT (MmLargeStackSize <= KERNEL_LARGE_STACK_SIZE);
        ASSERT ((MmLargeStackSize & (PAGE_SIZE-1)) == 0);

        //
        // Don't allow a value that is too low either.
        //

        if (MmLargeStackSize < KERNEL_STACK_SIZE) {
            MmLargeStackSize = KERNEL_STACK_SIZE;
        }
    }

    //
    // If the host processor supports global bits, then set the global
    // bit in the template kernel PTE and PDE entries.
    //

    if (KeFeatureBits & KF_GLOBAL_PAGE) {
        ValidKernelPte.u.Long |= MM_PTE_GLOBAL_MASK;

#if !defined(_X86PAE_)

        //
        // Note that the PAE mode of the processor does not support the
        // global bit in PDEs which map 4K page table pages.
        //

        ValidKernelPde.u.Long |= MM_PTE_GLOBAL_MASK;
#endif
        MmPteGlobal.u.Long = MM_PTE_GLOBAL_MASK;
    }

    TempPte = ValidKernelPte;
    TempPde = ValidKernelPde;

    //
    // Set the directory base for the system process.
    //

    PointerPte = MiGetPdeAddress (PDE_BASE);
    PdePageNumber = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    CurrentProcess = PsGetCurrentProcess ();

#if defined(_X86PAE_)

    PrototypePte.u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;

    _asm {
        mov     eax, cr3
        mov     DirBase, eax
    }

    //
    // Note cr3 must be 32-byte aligned.
    //

    ASSERT ((DirBase & 0x1f) == 0);

    //
    // Initialize the PaeTop for this process right away.
    //

    RtlCopyMemory ((PVOID) &MiSystemPaeVa,
                   (PVOID) (KSEG0_BASE | DirBase),
                   sizeof (MiSystemPaeVa));

    CurrentProcess->PaeTop = &MiSystemPaeVa;

#else

    DirBase = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) << PAGE_SHIFT;

#endif

    CurrentProcess->Pcb.DirectoryTableBase[0] = DirBase;
    KeSweepDcache (FALSE);

    //
    // Unmap the low 2Gb of memory.
    //

    PointerPde = MiGetPdeAddress (0);
    LastPte = MiGetPdeAddress (KSEG0_BASE);

    MiZeroMemoryPte (PointerPde, LastPte - PointerPde);

    //
    // Get the lower bound of the free physical memory and the
    // number of physical pages by walking the memory descriptor lists.
    //

    MxFreeDescriptor = NULL;
    TotalFreePages = 0;

    NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

    while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {

        MemoryDescriptor = CONTAINING_RECORD (NextMd,
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

            if ((MemoryDescriptor->BasePage + MemoryDescriptor->PageCount) > MmHighestPhysicalPage) {

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

                if (MemoryDescriptor->PageCount > MostFreePage) {
                    MostFreePage = MemoryDescriptor->PageCount;
                    MxFreeDescriptor = MemoryDescriptor;
                }

                TotalFreePages += MemoryDescriptor->PageCount;
            }
        }

        NextMd = MemoryDescriptor->ListEntry.Flink;
    }

    if (MmLargeSystemCache != 0) {
        ExtraSystemCacheViews = TRUE;
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

    //
    // Capture the registry-specified initial nonpaged pool setting as we
    // will modify the variable later.
    //

    if ((MmSizeOfNonPagedPoolInBytes != 0) ||
        (MmMaximumNonPagedPoolPercent != 0)) {

        InitialNonPagedPoolSetViaRegistry = TRUE;
    }
    else {
        InitialNonPagedPoolSetViaRegistry = FALSE;
    }

    if (MmNumberOfPhysicalPages <= MmLargePageMinimum) {

        MxMapLargePages = 0;

        //
        // Reduce the size of the initial nonpaged pool on small configurations
        // as RAM is precious (unless the registry has overridden it).
        //

        if ((MmNumberOfPhysicalPages <= MM_LARGE_PAGE_MINIMUM) &&
            (MmSizeOfNonPagedPoolInBytes == 0)) {

            MmSizeOfNonPagedPoolInBytes = 2 * 1024 * 1024;
        }
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

        status = HalQuerySystemInformation (HalQueryMaxHotPlugMemoryAddress,
                                            sizeof(PHYSICAL_ADDRESS),
                                            (PPHYSICAL_ADDRESS) &MaxHotPlugMemoryAddress,
                                            &ReturnedLength);

        if (NT_SUCCESS (status)) {
            ASSERT (ReturnedLength == sizeof(PHYSICAL_ADDRESS));

            MmDynamicPfn = (PFN_NUMBER) (MaxHotPlugMemoryAddress.QuadPart / PAGE_SIZE);
        }
    }

    if (MmDynamicPfn != 0) {
#if defined(_X86PAE_)
        MmHighestPossiblePhysicalPage = MI_DTC_MAX_PAGES - 1;
        if (MmVirtualBias != 0) {
            MmHighestPossiblePhysicalPage = MI_DTC_BOOTED_3GB_MAX_PAGES - 1;
            if (MmDynamicPfn > MmHighestPossiblePhysicalPage + 1) {
                MmDynamicPfn = MmHighestPossiblePhysicalPage + 1;
            }
        }
#else
        MmHighestPossiblePhysicalPage = MI_DEFAULT_MAX_PAGES - 1;
#endif
        if (MmDynamicPfn - 1 < MmHighestPossiblePhysicalPage) {
            if (MmDynamicPfn - 1 < MmHighestPhysicalPage) {
                MmDynamicPfn = MmHighestPhysicalPage + 1;
            }
            MmHighestPossiblePhysicalPage = MmDynamicPfn - 1;
        }

        if (TotalFreePages < MmDynamicPfn / 16) {

            //
            // Don't use large pages for the PFN database as the number of
            // physical pages at boot is much smaller than would be needed
            // for the maximum configuration.  Using large pages for the
            // mostly sparse (at this time) PFN database can result in us
            // not booting due to nonpaged pool expansion reservations that
            // occur during initial pool creation.
            //

            MxMapLargePages &= ~MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL;
        }
    }
    else {
        MmHighestPossiblePhysicalPage = MmHighestPhysicalPage;
    }

    if (MmHighestPossiblePhysicalPage > 0x400000 - 1) {

        //
        // The PFN database is more than 112mb.  Force it to come from the
        // 2GB->3GB virtual address range.  Note the administrator cannot be
        // booting /3GB as when he does, the loader throws away memory
        // above the physical 16GB line, so this must be a hot-add
        // configuration.  Since the loader has already put the system at
        // 3GB, the highest possible hot add page must be reduced now.
        //

        if (MmVirtualBias != 0) {
            MmHighestPossiblePhysicalPage = 0x400000 - 1;

            if (MmHighestPhysicalPage > MmHighestPossiblePhysicalPage) {
                MmHighestPhysicalPage = MmHighestPossiblePhysicalPage;
            }
        }
        else {

            //
            // The virtual space between 2 and 3GB virtual is best used
            // for system PTEs when this much physical memory is present.
            //

            ExtraSystemCacheViews = FALSE;
        }
    }

    //
    // Don't enable extra system cache views as virtual addresses are limited.
    // Only a kernel-verifier special case can trigger this.
    //

    if ((KernelVerifier == TRUE) &&
        (MmVirtualBias == 0) &&
        (MmNumberOfPhysicalPages <= MmLargePageMinimum) &&
        (MmHighestPossiblePhysicalPage > 0x100000)) {

        ExtraSystemCacheViews = FALSE;
    }

#if defined(_X86PAE_)

    if (MmVirtualBias != 0) {

        //
        // User space is larger than 2GB, make extra room for the user space
        // working set list & associated hash tables.
        //

        MmSystemCacheWorkingSetList = (PMMWSL) ((ULONG_PTR) 
            MmSystemCacheWorkingSetList + MM_SYSTEM_CACHE_WORKING_SET_3GB_DELTA);
    }

    MmSystemCacheWorkingSetListPte = MiGetPteAddress (MmSystemCacheWorkingSetList);

    //
    // Only PAE machines with at least 5GB of physical memory get to use this.
    //

    if (strstr(LoaderBlock->LoadOptions, "NOLOWMEM")) {
        if (MmNumberOfPhysicalPages >= 5 * 1024 * 1024 / 4) {
            MiNoLowMemory = (PFN_NUMBER)((ULONGLONG)_4gb / PAGE_SIZE);
        }
    }

    if (MiNoLowMemory != 0) {
        MmMakeLowMemory = TRUE;
    }

#endif

    MmHyperSpaceEnd = (PVOID)((ULONG_PTR)MmSystemCacheWorkingSetList - 1);

    //
    // Build non-paged pool using the physical pages following the
    // data page in which to build the pool from.  Non-paged pool grows
    // from the high range of the virtual address space and expands
    // downward.
    //
    // At this time non-paged pool is constructed so virtual addresses
    // are also physically contiguous.
    //

    if ((MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT) >
                        (7 * (TotalFreePages >> 3))) {

        //
        // More than 7/8 of memory is allocated to nonpagedpool, reset to 0.
        //

        MmSizeOfNonPagedPoolInBytes = 0;
        if (MmMaximumNonPagedPoolPercent == 0) {
            InitialNonPagedPoolSetViaRegistry = FALSE;
        }
    }

    if (MmSizeOfNonPagedPoolInBytes < MmMinimumNonPagedPoolSize) {

        //
        // Calculate the size of nonpaged pool.
        // Use the minimum size, then for every MB above 4mb add extra
        // pages.
        //

        MmSizeOfNonPagedPoolInBytes = MmMinimumNonPagedPoolSize;

        MmSizeOfNonPagedPoolInBytes +=
                            ((TotalFreePages - 1024)/256) *
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
            ((TotalFreePages * MmMaximumNonPagedPoolPercent) / 100);

        //
        // Carefully set the maximum keeping in mind that maximum PAE
        // machines can have 16*1024*1024 pages so care must be taken
        // that multiplying by PAGE_SIZE doesn't overflow here.
        //

        if (MaximumNonPagedPoolInBytesLimit > ((MM_MAX_INITIAL_NONPAGED_POOL + MM_MAX_ADDITIONAL_NONPAGED_POOL) / PAGE_SIZE)) {
            MaximumNonPagedPoolInBytesLimit = MM_MAX_INITIAL_NONPAGED_POOL + MM_MAX_ADDITIONAL_NONPAGED_POOL;
        }
        else {
            MaximumNonPagedPoolInBytesLimit *= PAGE_SIZE;
        }

        if (MaximumNonPagedPoolInBytesLimit < 6 * 1024 * 1024) {
            MaximumNonPagedPoolInBytesLimit = 6 * 1024 * 1024;
        }

        if (MmSizeOfNonPagedPoolInBytes > MaximumNonPagedPoolInBytesLimit) {
            MmSizeOfNonPagedPoolInBytes = MaximumNonPagedPoolInBytesLimit;
        }
    }
    
    //
    // Align to page size boundary.
    //

    MmSizeOfNonPagedPoolInBytes &= ~(PAGE_SIZE - 1);

    //
    // Calculate the maximum size of pool.
    //

    if (MmMaximumNonPagedPoolInBytes == 0) {

        //
        // Calculate the size of nonpaged pool.  If 4mb or less use
        // the minimum size, then for every MB above 4mb add extra
        // pages.
        //

        MmMaximumNonPagedPoolInBytes = MmDefaultMaximumNonPagedPool;

        //
        // Make sure enough expansion for the PFN database exists.
        //

        MmMaximumNonPagedPoolInBytes += (ULONG)PAGE_ALIGN (
                                      (MmHighestPossiblePhysicalPage + 1) * sizeof(MMPFN));

        //
        // Only use the new formula for autosizing nonpaged pool on machines
        // with at least 512MB.  The new formula allocates 1/2 as much nonpaged
        // pool per MB but scales much higher - machines with ~1.2GB or more
        // get 256MB of nonpaged pool.  Note that the old formula gave machines
        // with 512MB of RAM 128MB of nonpaged pool so this behavior is
        // preserved with the new formula as well.
        //

        if (TotalFreePages >= 0x1f000) {
            MmMaximumNonPagedPoolInBytes +=
                            ((TotalFreePages - 1024)/256) *
                            (MmMaxAdditionNonPagedPoolPerMb / 2);

            if (MmMaximumNonPagedPoolInBytes < MM_MAX_ADDITIONAL_NONPAGED_POOL) {
                MmMaximumNonPagedPoolInBytes = MM_MAX_ADDITIONAL_NONPAGED_POOL;
            }
        }
        else {
            MmMaximumNonPagedPoolInBytes +=
                            ((TotalFreePages - 1024)/256) *
                            MmMaxAdditionNonPagedPoolPerMb;
        }
        if ((MmMaximumNonPagedPoolPercent != 0) &&
            (MmMaximumNonPagedPoolInBytes > MaximumNonPagedPoolInBytesLimit)) {
                MmMaximumNonPagedPoolInBytes = MaximumNonPagedPoolInBytesLimit;
        }
    }

    MaxPool = MmSizeOfNonPagedPoolInBytes + PAGE_SIZE * 16 +
                                   (ULONG)PAGE_ALIGN (
                                        (MmHighestPossiblePhysicalPage + 1) * sizeof(MMPFN));

    if (MmMaximumNonPagedPoolInBytes < MaxPool) {
        MmMaximumNonPagedPoolInBytes = MaxPool;
    }

    //
    // Systems that are booted /3GB have a 128MB nonpaged pool maximum,
    //
    // Systems that have a full 2GB system virtual address space can enjoy an
    // extra 128MB of nonpaged pool in the upper GB of the address space.
    //

    MaxPool = MM_MAX_INITIAL_NONPAGED_POOL;

    if (MmVirtualBias == 0) {
        MaxPool += MM_MAX_ADDITIONAL_NONPAGED_POOL;
    }

    if (InitialNonPagedPoolSetViaRegistry == TRUE) {

        //
        // Make sure handling this initial setting won't push a /3GB
        // configuration past the maximum - note this cannot happen in
        // the normal non /3GB case.  The intent is for this setting to
        // allow configuration of *less* than the maximum amount of
        // nonpaged pool (to leave this VA space and/or physical pages
        // available for other resources).
        //

        if (MmSizeOfNonPagedPoolInBytes + MM_MAX_ADDITIONAL_NONPAGED_POOL < MaxPool) {
            MaxPool = MmSizeOfNonPagedPoolInBytes + MM_MAX_ADDITIONAL_NONPAGED_POOL;
        }
    }

    if (MmMaximumNonPagedPoolInBytes > MaxPool) {
        MmMaximumNonPagedPoolInBytes = MaxPool;
    }

    //
    // Grow the initial nonpaged pool if necessary so that the overall pool
    // will aggregate to the right size.
    //

    if ((MmMaximumNonPagedPoolInBytes > MM_MAX_INITIAL_NONPAGED_POOL) &&
        (InitialNonPagedPoolSetViaRegistry == FALSE)) {

        if (MmSizeOfNonPagedPoolInBytes < MmMaximumNonPagedPoolInBytes - MM_MAX_ADDITIONAL_NONPAGED_POOL) {

            //
            // Note the initial nonpaged pool can only be grown if there
            // is a sufficient contiguous physical memory chunk it can
            // be carved from immediately.
            //

            PagesLeft = MxPagesAvailable ();

            if (((MmMaximumNonPagedPoolInBytes - MM_MAX_ADDITIONAL_NONPAGED_POOL) >> PAGE_SHIFT) + ((32 * 1024 * 1024) >> PAGE_SHIFT) < PagesLeft) {

                MmSizeOfNonPagedPoolInBytes = MmMaximumNonPagedPoolInBytes - MM_MAX_ADDITIONAL_NONPAGED_POOL;
            }
            else {

                //
                // Since the initial nonpaged pool could not be grown, don't
                // leave any excess in the expansion nonpaged pool as we
                // cannot encode it into subsection format on non-pae
                // machines.
                //

                if (MmMaximumNonPagedPoolInBytes > MmSizeOfNonPagedPoolInBytes + MM_MAX_ADDITIONAL_NONPAGED_POOL) {
                    MmMaximumNonPagedPoolInBytes = MmSizeOfNonPagedPoolInBytes + MM_MAX_ADDITIONAL_NONPAGED_POOL;
                }
            }
        }
    }

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

#if defined(MI_MULTINODE)

    //
    // Determine the number of bits in MmSecondaryColorMask. This
    // is the number of bits the Node color must be shifted
    // by before it is included in colors.
    //

    i = MmSecondaryColorMask;
    MmSecondaryColorNodeShift = 0;
    while (i != 0) {
        i >>= 1;
        MmSecondaryColorNodeShift += 1;
    }

    //
    // Adjust the number of secondary colors by the number of nodes
    // in the machine.   The secondary color mask is NOT adjusted
    // as it is used to control coloring within a node.  The node
    // color is added to the color AFTER normal color calculations
    // are performed.
    //

    MmSecondaryColors *= KeNumberNodes;

    for (i = 0; i < KeNumberNodes; i += 1) {
        KeNodeBlock[i]->Color = (UCHAR)i;
        KeNodeBlock[i]->MmShiftedColor = i << MmSecondaryColorNodeShift;
        InitializeSListHead(&KeNodeBlock[i]->DeadStackList);
    }

#endif

    MiMaximumSystemCacheSizeExtra = 0;

    //
    // Add in the PFN database size (based on the number of pages required
    // from page zero to the highest page).
    //
    // Get the number of secondary colors and add the array for tracking
    // secondary colors to the end of the PFN database.
    //

    MxPfnAllocation = 1 + ((((MmHighestPossiblePhysicalPage + 1) * sizeof(MMPFN)) +
                        (MmSecondaryColors * sizeof(MMCOLOR_TABLES)*2))
                            >> PAGE_SHIFT);

    if (MmVirtualBias == 0) {

        MmNonPagedPoolStart = (PVOID)((ULONG)MmNonPagedPoolEnd
                                      - MmMaximumNonPagedPoolInBytes
                                      + MmSizeOfNonPagedPoolInBytes);
    }
    else {

        ULONG Reduction;
        ULONG MaxExpansion;
        ULONG ExpansionBytes;

        MmNonPagedPoolStart = (PVOID)((ULONG) MmNonPagedPoolEnd -
                                      (MmMaximumNonPagedPoolInBytes +
                                       (MxPfnAllocation << PAGE_SHIFT)));

        //
        // Align the nonpaged pool start to a PDE boundary as both it and
        // the PFN database will generally be allocated from a large page.
        //

        Reduction = (ULONG)MmNonPagedPoolStart & (MM_VA_MAPPED_BY_PDE - 1);

        MmNonPagedPoolStart = (PVOID)((ULONG_PTR)MmNonPagedPoolStart - Reduction);
        ExpansionBytes = MmMaximumNonPagedPoolInBytes;
        ASSERT (ExpansionBytes <= MM_MAX_ADDITIONAL_NONPAGED_POOL);

        //
        // Grow the expansion pool to use any slush VA being created.
        //

        MaxExpansion = MM_MAX_ADDITIONAL_NONPAGED_POOL - ExpansionBytes;
        if (Reduction < MaxExpansion) {
            MaxExpansion = Reduction;
        }

        MmNonPagedPoolEnd = (PVOID)((ULONG_PTR)MmNonPagedPoolEnd - Reduction);
        MmNonPagedPoolEnd = (PVOID)((ULONG_PTR)MmNonPagedPoolEnd + MaxExpansion);
        MmMaximumNonPagedPoolInBytes += MaxExpansion;
    }

    MmNonPagedPoolStart = (PVOID) PAGE_ALIGN (MmNonPagedPoolStart);

    NonPagedPoolStartVirtual = MmNonPagedPoolStart;

    //
    // Sanitize the paged pool size if specified badly via the registry.
    //

    if ((MmSizeOfPagedPoolInBytes != 0) &&
        (MmSizeOfPagedPoolInBytes != (SIZE_T)-1)) {

        if (MI_ROUND_TO_SIZE (MmSizeOfPagedPoolInBytes, MM_VA_MAPPED_BY_PDE) <=
            MmSizeOfPagedPoolInBytes) {

            //
            // The size wrapped so the user must want the maximum.
            //

            MmSizeOfPagedPoolInBytes = (SIZE_T)-1;
        }
    }

    //
    // Allocate additional paged pool provided it can fit and either the
    // user asked for it or we decide 460MB of PTE space is sufficient.
    //
    // Note at 64GB of RAM, the PFN database spans 464mb.  Given that plus
    // initial nonpaged pool at 128mb and space for the loader's highest
    // page and session space, there may not be any room left to guarantee
    // we will be able to allocate system PTEs out of the virtual address
    // space below 3gb.  So don't crimp for more than 64GB.
    //

    if ((MmVirtualBias == 0) &&
        (MmHighestPossiblePhysicalPage <= 0x1000000)) {

        if (((MmLargeStackSize <= (32 * 1024 - PAGE_SIZE)) && (MiUseMaximumSystemSpace != 0)) ||
        ((MmSizeOfPagedPoolInBytes == (SIZE_T)-1) ||
         ((MmSizeOfPagedPoolInBytes == 0) &&
         (MmNumberOfPhysicalPages >= (1 * 1024 * 1024 * 1024 / PAGE_SIZE)) &&
         (MiRequestedSystemPtes != (ULONG)-1)))) {

            if ((ExpMultiUserTS == FALSE) || (MmSizeOfPagedPoolInBytes != 0)) {

                PagedPoolMaximumDesired = TRUE;
                MmPagedPoolMaximumDesired = TRUE;
            }
            else {

                //
                // This is a multiuser TS machine defaulting to
                // autoconfiguration.  These machines use approximately
                // 3.25x PTEs compared to paged pool per session.
                // If the stack size is halved, then 1.6x becomes the ratio.
                //
                // Estimate how many PTEs and paged pool virtual space
                // will be available and divide it up now.
                //

                ULONG LowVa;
                ULONG TotalVirtualSpace;
                ULONG PagedPoolPortion;
                ULONG PtePortion;

                TotalVirtualSpace = (ULONG) NonPagedPoolStartVirtual - (ULONG) MM_PAGED_POOL_START;
                LowVa = (MM_KSEG0_BASE | MmBootImageSize) + MxPfnAllocation * PAGE_SIZE + MmSizeOfNonPagedPoolInBytes;

                if (LowVa < MiSystemViewStart) {
                    TotalVirtualSpace += (MiSystemViewStart - LowVa);
                }

                PtePortion = 77;
                PagedPoolPortion = 100 - PtePortion;

                //
                // If the large stack size has been reduced, then adjust the
                // ratio automatically as well.
                //

                if (MmLargeStackSize != KERNEL_LARGE_STACK_SIZE) {
                    PtePortion = (PtePortion * MmLargeStackSize) / KERNEL_LARGE_STACK_SIZE;
                }

                MmSizeOfPagedPoolInBytes = (TotalVirtualSpace / (PagedPoolPortion + PtePortion)) * PagedPoolPortion;
            }
    
            //
            // Make sure we always allocate extra PTEs later as we have crimped
            // the initial allocation here.
            //
    
            ExtraSystemCacheViews = FALSE;
            MmNumberOfSystemPtes = 3000;
            MiRequestedSystemPtes = (ULONG)-1;
        }
    }

    //
    // Calculate the starting PDE for the system PTE pool which is
    // right below the nonpaged pool.
    //

    MmNonPagedSystemStart = (PVOID)(((ULONG)NonPagedPoolStartVirtual -
                                ((MmNumberOfSystemPtes + 1) * PAGE_SIZE)) &
                                 (~PAGE_DIRECTORY_MASK));

    if (MmNonPagedSystemStart < MM_LOWEST_NONPAGED_SYSTEM_START) {

        MmNonPagedSystemStart = MM_LOWEST_NONPAGED_SYSTEM_START;

        MmNumberOfSystemPtes = (((ULONG)NonPagedPoolStartVirtual -
                                 (ULONG)MmNonPagedSystemStart) >> PAGE_SHIFT)-1;

        ASSERT (MmNumberOfSystemPtes > 1000);
    }

    if ((MmVirtualBias == 0) &&
        (MmSizeOfPagedPoolInBytes > ((ULONG) MmNonPagedSystemStart - (ULONG) MM_PAGED_POOL_START)) &&
        (MmPagedPoolMaximumDesired == FALSE)) {
    
        ULONG OldNonPagedSystemStart;
        ULONG ExtraPtesNeeded;
        ULONG InitialPagedPoolSize;

        MmSizeOfPagedPoolInBytes = MI_ROUND_TO_SIZE (MmSizeOfPagedPoolInBytes, MM_VA_MAPPED_BY_PDE);

        //
        // Recalculate the starting PDE for the system PTE pool which is
        // right below the nonpaged pool.  Leave at least 3000 high
        // system PTEs.
        //

        OldNonPagedSystemStart = (ULONG) MmNonPagedSystemStart;

        NonPagedSystemStart = ((ULONG)NonPagedPoolStartVirtual -
                                    ((3000 + 1) * PAGE_SIZE)) &
                                     ~PAGE_DIRECTORY_MASK;

        if (NonPagedSystemStart < (ULONG) MM_LOWEST_NONPAGED_SYSTEM_START) {
            NonPagedSystemStart = (ULONG) MM_LOWEST_NONPAGED_SYSTEM_START;
        }

        InitialPagedPoolSize = NonPagedSystemStart - (ULONG) MM_PAGED_POOL_START;

        if (MmSizeOfPagedPoolInBytes > InitialPagedPoolSize) {
            MmSizeOfPagedPoolInBytes = InitialPagedPoolSize;
        }
        else {
            NonPagedSystemStart = ((ULONG) MM_PAGED_POOL_START +
                                        MmSizeOfPagedPoolInBytes);

            ASSERT ((NonPagedSystemStart & PAGE_DIRECTORY_MASK) == 0);

            ASSERT (NonPagedSystemStart >= (ULONG) MM_LOWEST_NONPAGED_SYSTEM_START);
        }
        
        ASSERT (NonPagedSystemStart >= OldNonPagedSystemStart);
        ExtraPtesNeeded = (NonPagedSystemStart - OldNonPagedSystemStart) >> PAGE_SHIFT;

        //
        // Note the PagedPoolMaximumDesired local is deliberately not set
        // because we don't want or need to delete PDEs later in this
        // routine.  The exact amount has been allocated here.
        // The global MmPagedPoolMaximumDesired is set because other parts
        // of memory management use it to finish sizing properly.
        //

        MmPagedPoolMaximumDesired = TRUE;

        MmNonPagedSystemStart = (PVOID) NonPagedSystemStart;
        MmNumberOfSystemPtes = (((ULONG)NonPagedPoolStartVirtual -
                                 (ULONG)NonPagedSystemStart) >> PAGE_SHIFT)-1;
    }
    
    //
    // If the host processor supports large pages, and the number of
    // physical pages is greater than the threshold, then map the kernel
    // image and HAL into a large page.
    //

    if (MxMapLargePages & MI_LARGE_KERNEL_HAL) {

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
    // Save the original descriptor value as everything must be restored
    // prior to this function returning.
    //

    *(PMEMORY_ALLOCATION_DESCRIPTOR)&MxOldFreeDescriptor = *MxFreeDescriptor;

    //
    // Ensure there are enough pages in the descriptor to satisfy the sizes.
    //

    PagesLeft = MxPagesAvailable ();

    PagesNeeded = MxPfnAllocation + (MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT);

    //
    // If we're going to try to use large pages to map it, then align pages
    // needed to a PDE multiple so that the large page can be completely filled.
    //

    if ((MxMapLargePages & MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL) &&
        ((PagesNeeded << PAGE_SHIFT) & (MM_MINIMUM_VA_FOR_LARGE_PAGE - 1))) {

        ULONG ReductionBytes;

        ReductionBytes =
            ((PagesNeeded << PAGE_SHIFT) & (MM_MINIMUM_VA_FOR_LARGE_PAGE - 1));

        ASSERT (MmSizeOfNonPagedPoolInBytes > ReductionBytes);
        MmSizeOfNonPagedPoolInBytes -= ReductionBytes;
        MmMaximumNonPagedPoolInBytes -= ReductionBytes;
        PagesNeeded -= (ReductionBytes >> PAGE_SHIFT);
    }

    if (PagesNeeded + ((32 * 1024 * 1024) >> PAGE_SHIFT) > PagesLeft) {
        MxMapLargePages &= ~MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL;
    }

    if (MmVirtualBias == 0) {

        //
        // Put the PFN database in low virtual memory just
        // above the loaded images.
        //

        MmPfnDatabase = (PMMPFN)(MM_KSEG0_BASE | MmBootImageSize);

        ASSERT (((ULONG_PTR)MmPfnDatabase & (MM_VA_MAPPED_BY_PDE - 1)) == 0);

        //
        // Systems with extremely large PFN databases (ie: spanning 64GB of RAM)
        // and bumped session space sizes may require a reduction in the initial
        // nonpaged pool size in order to fit.
        //

        NumberOfBytes = MiSystemViewStart - (ULONG_PTR) MmPfnDatabase;

        if (PagesNeeded > (NumberOfBytes >> PAGE_SHIFT)) {

            ULONG ReductionBytes;

            ReductionBytes = (PagesNeeded << PAGE_SHIFT) - NumberOfBytes;

            MmSizeOfNonPagedPoolInBytes -= ReductionBytes;
            MmMaximumNonPagedPoolInBytes -= ReductionBytes;
            PagesNeeded -= (ReductionBytes >> PAGE_SHIFT);
        }
    }
    else {

        //
        // Put the PFN database at the top of the system PTE range (where non
        // paged pool would have started) and slide the nonpaged pool start
        // upwards.
        //

        MmPfnDatabase = (PMMPFN) MmNonPagedPoolStart;

        MmNonPagedPoolStart = (PVOID)((PCHAR)MmPfnDatabase + (MxPfnAllocation << PAGE_SHIFT));

        NonPagedPoolStartVirtual = MmNonPagedPoolStart;
    }

    if (MxMapLargePages & MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL) {

        PFN_NUMBER LastPage;

        //
        // The PFN database and nonpaged pool must be allocated from a
        // large-page aligned chunk of memory in the free descriptor.
        // This is because all the pages in the large page range must
        // be mapped fully cached (to avoid TB attribute conflicts) so
        // it's easiest to ensure they are all consumed now.
        //
        // See if the free descriptor has enough pages of large page alignment
        // to satisfy our calculation.
        //

        ASSERT (((PagesNeeded << PAGE_SHIFT) & (MM_MINIMUM_VA_FOR_LARGE_PAGE - 1)) == 0);

        BasePage = MI_ROUND_TO_SIZE (MxFreeDescriptor->BasePage,
                                     MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);

        LastPage = MxFreeDescriptor->BasePage + MxFreeDescriptor->PageCount;

        //
        // We can safely round up here because we checked earlier that the
        // free descriptor has at least 32mb (much more than a large page's
        // worth) more than we need.
        //

        if (BasePage < MxFreeDescriptor->BasePage) {
            BasePage += (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);
            ASSERT (BasePage >= MxFreeDescriptor->BasePage);
            ASSERT (BasePage < LastPage);
        }

        ASSERT (BasePage + PagesNeeded <= LastPage);

        //
        // Get the physical pages for the PFN database & initial nonpaged pool.
        //

        if (BasePage == MxFreeDescriptor->BasePage) {

            //
            // The descriptor starts on a large page aligned boundary so remove
            // the large page span from the bottom of the free descriptor.
            //

            FirstPfnDatabasePage = BasePage;
            ASSERT ((FirstPfnDatabasePage & (MM_PFN_MAPPED_BY_PDE - 1)) == 0);

            MxFreeDescriptor->BasePage += PagesNeeded;
            MxFreeDescriptor->PageCount -= (ULONG) PagesNeeded;
        }
        else {

            if ((LastPage & ((MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT) - 1)) == 0) {
                //
                // The descriptor ends on a large page aligned boundary so
                // remove the large page span from the top of the free
                // descriptor.
                //

                FirstPfnDatabasePage = LastPage - PagesNeeded;
                ASSERT ((FirstPfnDatabasePage & (MM_PFN_MAPPED_BY_PDE - 1)) == 0);

                MxFreeDescriptor->PageCount -= PagesNeeded;
            }
            else {

                //
                // The descriptor does not start or end on a large page aligned
                // address so chop the descriptor.  The excess slush is added to
                // the freelist later.
                //

                MiSlushDescriptorBase = MxFreeDescriptor->BasePage;
                MiSlushDescriptorCount = BasePage - MxFreeDescriptor->BasePage;

                FirstPfnDatabasePage = BasePage;
                ASSERT ((FirstPfnDatabasePage & (MM_PFN_MAPPED_BY_PDE - 1)) == 0);

                MxFreeDescriptor->PageCount -= (ULONG) (PagesNeeded + MiSlushDescriptorCount);

                MxFreeDescriptor->BasePage = (ULONG) (BasePage + PagesNeeded);
            }
        }
    
        ASSERT (((ULONG_PTR)MmPfnDatabase & (MM_VA_MAPPED_BY_PDE - 1)) == 0);

        //
        // Adjust the system PTEs top so no future PTE allocations get
        // to share the large page mapping.
        //

        if (MmVirtualBias != 0) {
            MmNumberOfSystemPtes = ((PCHAR) MmPfnDatabase - (PCHAR) MmNonPagedSystemStart) / PAGE_SIZE;
        }

        //
        // Add the PFN database and initial nonpaged pool combined range
        // to the large page ranges.
        //

        MiLargeVaRanges[MiLargeVaRangeIndex].VirtualAddress = MmPfnDatabase;
        MiLargeVaRanges[MiLargeVaRangeIndex].EndVirtualAddress =
                              (PVOID) ((ULONG_PTR)MmPfnDatabase + (MxPfnAllocation << PAGE_SHIFT) + MmSizeOfNonPagedPoolInBytes - 1);
        MiLargeVaRangeIndex += 1;
    }

    //
    // Initial nonpaged pool immediately follows the PFN database.
    //
    // Since the PFN database and the initial nonpaged pool are physically
    // adjacent, a single PDE is shared, thus reducing the number of pages
    // that otherwise might need to be marked as must-be-cachable.
    //

    MmNonPagedPoolStart = (PVOID)((ULONG_PTR)MmPfnDatabase + (MxPfnAllocation << PAGE_SHIFT));

    if ((ULONG_PTR)MmNonPagedPoolEnd - (ULONG_PTR)MmNonPagedPoolStart < MmMaximumNonPagedPoolInBytes) {
        ASSERT (MmVirtualBias != 0);
        MmMaximumNonPagedPoolInBytes = (ULONG_PTR)MmNonPagedPoolEnd - (ULONG_PTR)MmNonPagedPoolStart;
    }

    if (MxMapLargePages & MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL) {

        if (MmVirtualBias != 0) {
            NonPagedPoolStartVirtual = MmNonPagedPoolStart;
        }
    }

    if (PagedPoolMaximumDesired == TRUE) {
    
        ASSERT (MmVirtualBias == 0);

        //
        // Maximum paged pool was requested.  This means slice away most of
        // the system PTEs being used at the high end of the virtual address
        // space and use that address range for more paged pool instead.
        //

        ASSERT (MiIsVirtualAddressOnPdeBoundary (MmNonPagedSystemStart));

        PointerPde = MiGetPdeAddress ((PCHAR) MmNonPagedSystemStart + (MmNumberOfSystemPtes << PAGE_SHIFT));

        PointerPde -= 2;
#if defined (_X86PAE_)

        //
        // Since PAE needs twice as many PDEs to cover the same VA range ...
        //

        PointerPde -= 2;
#endif

        if (MiGetVirtualAddressMappedByPde (PointerPde) >= MM_LOWEST_NONPAGED_SYSTEM_START) {
            MmNonPagedSystemStart = MiGetVirtualAddressMappedByPde (PointerPde);
            MmNumberOfSystemPtes = (((ULONG)NonPagedPoolStartVirtual -
                                 (ULONG)MmNonPagedSystemStart) >> PAGE_SHIFT)-1;
        }
    }

    //
    // Allocate pages and fill in the PTEs for the initial nonpaged pool.
    //

    if ((MxMapLargePages & MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL) == 0) {

        //
        // The physical memory for the initial nonpaged pool was not allocated
        // above so do it now.
        //

        PagesNeeded = MmSizeOfNonPagedPoolInBytes >> PAGE_SHIFT;

        //
        // If needed, reduce our request to accommodate
        // physical memory fragmention.
        //

        PagesLeft = MxPagesAvailable ();

        if (PagesNeeded > PagesLeft) {
            MmSizeOfNonPagedPoolInBytes -= (PagesNeeded - PagesLeft) << PAGE_SHIFT;
            MmMaximumNonPagedPoolInBytes -= (MmSizeOfNonPagedPoolInBytes - ((PagesNeeded - PagesLeft) << PAGE_SHIFT));
            PagesNeeded = PagesLeft;
        }

        PageFrameIndex = MxGetNextPage (PagesNeeded, FALSE);
    }
    else {
        PageFrameIndex = FirstPfnDatabasePage + MxPfnAllocation;
    }

    MmMaximumNonPagedPoolInPages = (MmMaximumNonPagedPoolInBytes >> PAGE_SHIFT);

    //
    // Set up page table pages to map system PTEs and the expansion nonpaged
    // pool.  If the system was booted /3GB, then the initial nonpaged pool
    // is mapped here as well.
    //

    StartPde = MiGetPdeAddress (MmNonPagedSystemStart);
    EndPde = MiGetPdeAddress ((PVOID)((PCHAR)MmNonPagedPoolEnd - 1));

    while (StartPde <= EndPde) {

        ASSERT (StartPde->u.Hard.Valid == 0);

        //
        // Map in a page table page, using the
        // slush descriptor if one exists.
        //

        TempPde.u.Hard.PageFrameNumber = MxGetNextPage (1, TRUE);
        *StartPde = TempPde;
        PointerPte = MiGetVirtualAddressMappedByPte (StartPde);
        RtlZeroMemory (PointerPte, PAGE_SIZE);
        StartPde += 1;
    }

    MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

    if (MmVirtualBias == 0) {

        //
        // Allocate the page table pages to map the PFN database and the
        // initial nonpaged pool now.  If the system switches to large
        // pages in Phase 1, these pages will be discarded then.
        //

        StartPde = MiGetPdeAddress (MmPfnDatabase);

        VirtualAddress = (PVOID) ((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes - 1);

        EndPde = MiGetPdeAddress (VirtualAddress);

        //
        // Use any extra virtual address space between the top of initial
        // nonpaged pool and session space for additional system PTEs or
        // caching.
        //

        PointerPde = EndPde + 1;
        EndPde = MiGetPdeAddress (MiSystemViewStart - 1);

        if (PointerPde <= EndPde) {

            //
            // There is available virtual space - consume everything up
            // to the system view area (which is always rounded to a page
            // directory boundary to avoid wasting valuable virtual
            // address space.
            //

            ULONG NumberOfExtraSystemPdes;

            VirtualAddress = MiGetVirtualAddressMappedByPde (PointerPde);
            NumberOfExtraSystemPdes = EndPde - PointerPde + 1;

            //
            // Mark the new range as PTEs iff maximum PTEs are requested,
            // TS in app server mode is selected or special pooling is
            // enabled.  Otherwise if large system caching was selected
            // then use it for that.  Finally default to PTEs if neither
            // of the above.
            //

            if ((MiRequestedSystemPtes == (ULONG)-1) ||
                (ExpMultiUserTS == TRUE) ||
                (MmVerifyDriverBufferLength != (ULONG)-1) ||
                ((MmSpecialPoolTag != 0) && (MmSpecialPoolTag != (ULONG)-1))) {

                ExtraSystemCacheViews = FALSE;
            }

            if (ExtraSystemCacheViews == TRUE) {

                //
                // The system is configured to favor large system caching,
                // so share the remaining virtual address space between the
                // system cache and system PTEs.
                //

                MiMaximumSystemCacheSizeExtra =
                                    (NumberOfExtraSystemPdes * 5) / 6;

                ExtraPtes1 = NumberOfExtraSystemPdes -
                                    MiMaximumSystemCacheSizeExtra;

                ExtraPtes1 *= (MM_VA_MAPPED_BY_PDE / PAGE_SIZE);

                MiMaximumSystemCacheSizeExtra *= MM_VA_MAPPED_BY_PDE;

                ExtraPtes1Pointer = MiGetPteAddress ((ULONG)VirtualAddress + 
                                            MiMaximumSystemCacheSizeExtra);

                MiMaximumSystemCacheSizeExtra >>= PAGE_SHIFT;

                MiSystemCacheStartExtra = VirtualAddress;
            }
            else {
                ExtraPtes1 = BYTES_TO_PAGES(MiSystemViewStart - (ULONG)VirtualAddress);
                ExtraPtes1Pointer = MiGetPteAddress (VirtualAddress);
            }
        }

        //
        // Allocate and initialize the page table pages.
        //

        while (StartPde <= EndPde) {

            ASSERT (StartPde->u.Hard.Valid == 0);
            if (StartPde->u.Hard.Valid == 0) {

                //
                // Map in a page directory page, using the
                // slush descriptor if one exists.
                //

                TempPde.u.Hard.PageFrameNumber = MxGetNextPage (1, TRUE);
                *StartPde = TempPde;
                PointerPte = MiGetVirtualAddressMappedByPte (StartPde);
                RtlZeroMemory (PointerPte, PAGE_SIZE);
            }
            StartPde += 1;
        }

        if (MiUseMaximumSystemSpace != 0) {

            //
            // Use the 1GB->2GB virtual range for even more system PTEs.
            // Note the shared user data PTE (and PDE) must be left user
            // accessible, but everything else is kernelmode only.
            //

            ExtraPtes2 = BYTES_TO_PAGES(MiUseMaximumSystemSpaceEnd - MiUseMaximumSystemSpace);

            StartPde = MiGetPdeAddress (MiUseMaximumSystemSpace);
            EndPde = MiGetPdeAddress (MiUseMaximumSystemSpaceEnd);

            while (StartPde < EndPde) {

                ASSERT (StartPde->u.Hard.Valid == 0);

                //
                // Map in a page directory page, using the
                // slush descriptor if one exists.
                //

                TempPde.u.Hard.PageFrameNumber = MxGetNextPage (1, TRUE);
                *StartPde = TempPde;
                PointerPte = MiGetVirtualAddressMappedByPte (StartPde);
                RtlZeroMemory (PointerPte, PAGE_SIZE);
                StartPde += 1;
            }
        }

        //
        // The virtual address, length and page tables to map the initial
        // nonpaged pool are already allocated - just fill in the mappings.
        //

        MmSubsectionBase = (ULONG)MmNonPagedPoolStart;

    }

    MmNonPagedPoolExpansionStart = NonPagedPoolStartVirtual;

    //
    // Make sure the scratch PTEs for the crash dump and debugger code
    // exist.  They may not if booted /3GB and alignment moved the PFN
    // database down and rippled the end of nonpaged pool down with it.
    //

    PointerPde = MiGetPdeAddress (MM_CRASH_DUMP_VA);
    ASSERT (PointerPde == MiGetPdeAddress (MM_DEBUG_VA));

    if (PointerPde->u.Hard.Valid == 0) {

        //
        // Map in a page table page, using the
        // slush descriptor if one exists.
        //

        TempPde.u.Hard.PageFrameNumber = MxGetNextPage (1, TRUE);

        *PointerPde = TempPde;
        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
        RtlZeroMemory (PointerPte, PAGE_SIZE);
    }

    //
    // Map the initial nonpaged pool.
    //

    PointerPte = MiGetPteAddress (MmNonPagedPoolStart);

    LastPte = MiGetPteAddress ((ULONG)MmNonPagedPoolStart +
                                        MmSizeOfNonPagedPoolInBytes - 1);

    if (MxMapLargePages & MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL) {
        ASSERT (MiIsPteOnPdeBoundary (LastPte + 1));
    }

    while (PointerPte <= LastPte) {
        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
        MI_WRITE_VALID_PTE (PointerPte, TempPte);
        PointerPte += 1;
        PageFrameIndex += 1;
    }

    if (MmVirtualBias != 0) {

        MmNonPagedPoolExpansionStart = (PVOID)((PCHAR)NonPagedPoolStartVirtual +
                    MmSizeOfNonPagedPoolInBytes);

        //
        // When booted /3GB, if /USERVA was specified then use any leftover
        // virtual space between 2 and 3gb for extra system PTEs.
        //

        if (MiUseMaximumSystemSpace != 0) {

            ExtraPtes2 = BYTES_TO_PAGES(MiUseMaximumSystemSpaceEnd - MiUseMaximumSystemSpace);

            StartPde = MiGetPdeAddress (MiUseMaximumSystemSpace);
            EndPde = MiGetPdeAddress (MiUseMaximumSystemSpaceEnd);

            while (StartPde < EndPde) {

                ASSERT (StartPde->u.Hard.Valid == 0);

                //
                // Map in a page directory page, using the
                // slush descriptor if one exists.
                //

                TempPde.u.Hard.PageFrameNumber = MxGetNextPage (1, TRUE);
                *StartPde = TempPde;
                PointerPte = MiGetVirtualAddressMappedByPte (StartPde);
                RtlZeroMemory (PointerPte, PAGE_SIZE);
                StartPde += 1;
            }
        }
    }

    TempPte = ValidKernelPte;

    //
    // There must be at least one page of system PTEs before the expanded
    // nonpaged pool.
    //

    ASSERT (MiGetPteAddress (MmNonPagedSystemStart) < MiGetPteAddress (MmNonPagedPoolExpansionStart));

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

    if (MxMapLargePages & MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL) {

        //
        // The physical pages to be used for the PFN database have already
        // been allocated.  Initialize the page table mappings (the
        // directory mappings are already initialized) for the PFN
        // database until the switch to large pages occurs in Phase 1.
        //

        PointerPte = MiGetPteAddress (MmPfnDatabase);

        LastPte = MiGetPteAddress ((ULONG_PTR)MmPfnDatabase + (MxPfnAllocation << PAGE_SHIFT));

        PageFrameIndex = FirstPfnDatabasePage;

        while (PointerPte < LastPte) {
            ASSERT ((PointerPte->u.Hard.Valid == 0) ||
                    (PointerPte->u.Hard.PageFrameNumber == PageFrameIndex));
            if (MiIsPteOnPdeBoundary (PointerPte)) {
                ASSERT ((PageFrameIndex & (MM_PFN_MAPPED_BY_PDE - 1)) == 0);
            }
            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

            if (PointerPte->u.Hard.Valid == 0) {
                MI_WRITE_VALID_PTE (PointerPte, TempPte);
            }
            else {
                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);
            }

            PointerPte += 1;
            PageFrameIndex += 1;
        }

        RtlZeroMemory (MmPfnDatabase, MxPfnAllocation << PAGE_SHIFT);
    }
    else {

        ULONG FreeNextPhysicalPage;
        ULONG FreeNumberOfPages;

        //
        // Go through the memory descriptors and for each physical page make
        // sure the PFN database has a valid PTE to map it.  This allows
        // machines with sparse physical memory to have a minimal PFN database.
        //

        FreeNextPhysicalPage = MxFreeDescriptor->BasePage;
        FreeNumberOfPages = MxFreeDescriptor->PageCount;

        PagesLeft = 0;
        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;
        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
            MemoryDescriptor = CONTAINING_RECORD (NextMd,
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
                PageCount = MxOldFreeDescriptor.PageCount;
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
                if (PointerPte->u.Hard.Valid == 0) {
                    TempPte.u.Hard.PageFrameNumber = FreeNextPhysicalPage;
                    ASSERT (FreeNumberOfPages != 0);
                    FreeNextPhysicalPage += 1;
                    FreeNumberOfPages -= 1;
                    if (FreeNumberOfPages == 0) {
                        KeBugCheckEx (INSTALL_MORE_MEMORY,
                                      MmNumberOfPhysicalPages,
                                      FreeNumberOfPages,
                                      MxOldFreeDescriptor.PageCount,
                                      1);
                    }
                    PagesLeft += 1;
                    MI_WRITE_VALID_PTE (PointerPte, TempPte);
                    RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte),
                                   PAGE_SIZE);
                }
                PointerPte += 1;
            }

            NextMd = MemoryDescriptor->ListEntry.Flink;
        }

        //
        // Update the global counts - this would have been tricky to do while
        // removing pages from them as we looped above.
        //
        // Later we will walk the memory descriptors and add pages to the free
        // list in the PFN database.
        //
        // To do this correctly:
        //
        // The FreeDescriptor fields must be updated so the PFN database
        // consumption isn't added to the freelist.
        //

        MxFreeDescriptor->BasePage = FreeNextPhysicalPage;
        MxFreeDescriptor->PageCount = FreeNumberOfPages;
    }

#if defined (_X86PAE_)

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

#endif

    //
    // Initialize support for colored pages.
    //

    MmFreePagesByColor[0] = (PMMCOLOR_TABLES)
                              &MmPfnDatabase[MmHighestPossiblePhysicalPage + 1];

    //
    // Make sure the PTEs are mapped.
    //

    PointerPte = MiGetPteAddress (&MmFreePagesByColor[0][0]);

    LastPte = MiGetPteAddress ((PCHAR)MmFreePagesByColor[0] + (2 * MmSecondaryColors * sizeof (MMCOLOR_TABLES)) - 1);

    while (PointerPte <= LastPte) {
        if (PointerPte->u.Hard.Valid == 0) {
            TempPte.u.Hard.PageFrameNumber = MxGetNextPage (1, TRUE);
            MI_WRITE_VALID_PTE (PointerPte, TempPte);
            RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte),
                           PAGE_SIZE);
        }

        PointerPte += 1;
    }

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
    // Go through the page table entries and for any page which is valid,
    // update the corresponding PFN database element.
    //

    Pde = MiGetPdeAddress (NULL);
    va = 0;
    PdeCount = PD_PER_SYSTEM * PDE_PER_PAGE;

    for (i = 0; i < PdeCount; i += 1) {

        //
        // If the kernel image has been biased to allow for 3gb of user
        // address space, then the first several mb of memory is
        // double mapped to KSEG0_BASE and to ALTERNATE_BASE. Therefore,
        // the KSEG0_BASE entries must be skipped.
        //

        if (MmVirtualBias != 0) {
            if ((Pde >= MiGetPdeAddress(KSEG0_BASE)) &&
                (Pde < MiGetPdeAddress(KSEG0_BASE + MmBootImageSize))) {
                Pde += 1;
                va += (ULONG)PDE_PER_PAGE * (ULONG)PAGE_SIZE;
                continue;
            }
        }

        if ((Pde->u.Hard.Valid == 1) && (Pde->u.Hard.LargePage == 0)) {

            PdePage = MI_GET_PAGE_FRAME_FROM_PTE (Pde);

            if (MiIsRegularMemory (LoaderBlock, PdePage)) {

                Pfn1 = MI_PFN_ELEMENT(PdePage);
                Pfn1->u4.PteFrame = PdePageNumber;
                Pfn1->PteAddress = Pde;
                Pfn1->u2.ShareCount += 1;
                Pfn1->u3.e2.ReferenceCount = 1;
                Pfn1->u3.e1.PageLocation = ActiveAndValid;
                Pfn1->u3.e1.CacheAttribute = MiCached;
                MiDetermineNode (PdePage, Pfn1);
            }
            else {
                Pfn1 = NULL;
            }

            PointerPte = MiGetPteAddress (va);

            //
            // Set global bit.
            //

            TempPde.u.Long = MiDetermineUserGlobalPteMask (PointerPte) &
                                                           ~MM_PTE_ACCESS_MASK;

#if defined(_X86PAE_)

            //
            // Note that the PAE mode of the processor does not support the
            // global bit in PDEs which map 4K page table pages.
            //

            TempPde.u.Hard.Global = 0;
#endif

            Pde->u.Long |= TempPde.u.Long;

            for (j = 0 ; j < PTE_PER_PAGE; j += 1) {
                if (PointerPte->u.Hard.Valid == 1) {

                    PointerPte->u.Long |= MiDetermineUserGlobalPteMask (PointerPte) &
                                                            ~MM_PTE_ACCESS_MASK;

                    ASSERT (Pfn1 != NULL);
                    Pfn1->u2.ShareCount += 1;

                    if ((MiIsRegularMemory (LoaderBlock, (PFN_NUMBER) PointerPte->u.Hard.PageFrameNumber)) &&

                        ((va >= MM_KSEG2_BASE) &&
                         ((va < KSEG0_BASE + MmVirtualBias) ||
                          (va >= (KSEG0_BASE + MmVirtualBias + MmBootImageSize)))) ||
                        ((MmVirtualBias == 0) &&
                         (va >= (ULONG)MmNonPagedPoolStart) &&
                         (va < (ULONG)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes))) {

                        Pfn2 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);

                        if (MmIsAddressValid (Pfn2) &&
                             MmIsAddressValid ((PUCHAR)(Pfn2+1)-1)) {

                            Pfn2->u4.PteFrame = PdePage;
                            Pfn2->PteAddress = PointerPte;
                            Pfn2->u2.ShareCount += 1;
                            Pfn2->u3.e2.ReferenceCount = 1;
                            Pfn2->u3.e1.PageLocation = ActiveAndValid;
                            Pfn2->u3.e1.CacheAttribute = MiCached;
                            MiDetermineNode (
                                (PFN_NUMBER)PointerPte->u.Hard.PageFrameNumber,
                                Pfn2);
                        }
                    }
                }

                va += PAGE_SIZE;
                PointerPte += 1;
            }

        }
        else {
            va += (ULONG)PDE_PER_PAGE * (ULONG)PAGE_SIZE;
        }

        Pde += 1;
    }

    MI_FLUSH_CURRENT_TB ();

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

        Pde = MiGetPdeAddress (0xffffffff);
        PdePage = MI_GET_PAGE_FRAME_FROM_PTE (Pde);
        Pfn1->u4.PteFrame = PdePageNumber;
        Pfn1->PteAddress = Pde;
        Pfn1->u2.ShareCount += 1;
        Pfn1->u3.e2.ReferenceCount = 0xfff0;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        Pfn1->u3.e1.CacheAttribute = MiCached;
        MiDetermineNode (0, Pfn1);
    }

    Bias = 0;

    if (MmVirtualBias != 0) {

        //
        // This is nasty.  You don't want to know.  Cleanup needed.
        //

        Bias = ALTERNATE_BASE - KSEG0_BASE;
    }

    //
    // Walk through the memory descriptors and add pages to the
    // free list in the PFN database.  Before doing this, adjust the
    // two descriptors we used so they only contain memory that can be
    // freed now (ie: any memory we removed from them earlier in this routine
    // without updating the descriptor for must be updated now).
    //

    //
    // We may have taken memory out of the MxFreeDescriptor - but
    // that's ok because we wouldn't want to free that memory right now
    // (or ever) anyway.
    //

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

        MemoryDescriptor = CONTAINING_RECORD (NextMd,
                                              MEMORY_ALLOCATION_DESCRIPTOR,
                                              ListEntry);

        i = MemoryDescriptor->PageCount;
        PageFrameIndex = MemoryDescriptor->BasePage;

        //
        // Ensure no frames are inserted beyond the end of the PFN
        // database.  This can happen for example if the system
        // has > 16GB of RAM and is booted /3GB - the top of this
        // routine reduces the highest physical page and then
        // creates the PFN database.  But the loader block still
        // contains descriptions of the pages above 16GB.
        //

        if (PageFrameIndex > MmHighestPhysicalPage) {
            NextMd = MemoryDescriptor->ListEntry.Blink;
            continue;
        }

        if (PageFrameIndex + i > MmHighestPhysicalPage + 1) {
            i = MmHighestPhysicalPage + 1 - PageFrameIndex;
            MemoryDescriptor->PageCount = i;
            if (i == 0) {
                NextMd = MemoryDescriptor->ListEntry.Blink;
                continue;
            }
        }

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
                while (i != 0) {
                    MiInsertPageInList (&MmBadPageListHead, PageFrameIndex);
                    i -= 1;
                    PageFrameIndex += 1;
                }
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

                PointerPte = MiGetPteAddress (KSEG0_BASE + Bias +
                                            (PageFrameIndex << PAGE_SHIFT));

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                while (i != 0) {

                    //
                    // Set page as in use.
                    //

                    PointerPde = MiGetPdeAddress (KSEG0_BASE + Bias +
                                             (PageFrameIndex << PAGE_SHIFT));

                    if (Pfn1->u3.e2.ReferenceCount == 0) {
                        Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
                        Pfn1->PteAddress = PointerPte;
                        Pfn1->u2.ShareCount += 1;
                        Pfn1->u3.e2.ReferenceCount = 1;
                        Pfn1->u3.e1.PageLocation = ActiveAndValid;
                        MiDetermineNode (PageFrameIndex, Pfn1);

                        if (MemoryDescriptor->MemoryType == LoaderXIPRom) {
                            Pfn1->u1.Flink = 0;
                            Pfn1->u2.ShareCount = 0;
                            Pfn1->u3.e2.ReferenceCount = 0;
                            Pfn1->u3.e1.PageLocation = 0;
                            Pfn1->u3.e1.Rom = 1;
                            Pfn1->u4.InPageError = 0;
                            Pfn1->u3.e1.PrototypePte = 1;
                        }
                        Pfn1->u3.e1.CacheAttribute = MiCached;
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

    if ((MxMapLargePages & MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL) == 0) {

        //
        // When booted /3GB, the PTEs mapping the PFN database were already
        // scanned above as part of scanning the entire VA space, and the
        // PFNs updated then.  For non-3GB, the PFNs must be updated now.
        //

        if (MmVirtualBias == 0) {
            PointerPte = MiGetPteAddress (&MmPfnDatabase[MmLowestPhysicalPage]);
            LastPte = MiGetPteAddress (&MmPfnDatabase[MmHighestPossiblePhysicalPage]);
            while (PointerPte <= LastPte) {
                if (PointerPte->u.Hard.Valid == 1) {
                    Pfn1 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);
                    Pfn1->u2.ShareCount = 1;
                    Pfn1->u3.e2.ReferenceCount = 1;
                }
                PointerPte += 1;
            }
        }
    }
    else {

        //
        // The PFN database is allocated using large pages.
        //
        // If the large page chunk came from the middle of the free descriptor
        // (due to alignment requirements), then add the pages from the split
        // bottom portion of the free descriptor now.
        //
    
        i = MiSlushDescriptorCount;
        PageFrameIndex = MiSlushDescriptorBase;
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
        PointerPte = MiGetPteAddress (KSEG0_BASE + Bias +
                                    (PageFrameIndex << PAGE_SHIFT));

        while (i != 0) {
            if (Pfn1->u3.e2.ReferenceCount == 0) {
    
                //
                // Set the PTE address to the physical page for
                // virtual address alignment checking.
                //
    
                LOCK_PFN (OldIrql);
                Pfn1->PteAddress = PointerPte;
                Pfn1->u3.e1.CacheAttribute = MiCached;
                MiDetermineNode (PageFrameIndex, Pfn1);
                MiInsertPageInFreeList (PageFrameIndex);
                UNLOCK_PFN (OldIrql);
            }
            Pfn1 += 1;
            i -= 1;
            PageFrameIndex += 1;
            PointerPte += 1;
        }

        //
        // Mark all PFN entries for the PFN database pages as in use.
        //

        PointerPte = MiGetPteAddress (MmPfnDatabase);
        PageFrameIndex = (PFN_NUMBER) PointerPte->u.Hard.PageFrameNumber;
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        i = MxPfnAllocation;

        do {
            Pfn1->u3.e1.CacheAttribute = MiCached;
            MiDetermineNode (PageFrameIndex, Pfn1);
            Pfn1->u3.e2.ReferenceCount = 1;
            PageFrameIndex += 1;
            Pfn1 += 1;
            i -= 1;
        } while (i != 0);

        if (MmDynamicPfn == 0) {

            //
            // Scan the PFN database backward for pages that are completely
            // zero.  These pages are unused and can be added to the free list.
            //

            BottomPfn = MI_PFN_ELEMENT (MmHighestPhysicalPage);
            do {

                //
                // Compute the address of the start of the page that is next
                // lower in memory and scan backwards until that page address
                // is reached or just crossed.
                //

                if (BYTE_OFFSET (BottomPfn)) {
                    BasePfn = (PMMPFN) PAGE_ALIGN (BottomPfn);
                    TopPfn = BottomPfn + 1;
                }
                else {
                    BasePfn = (PMMPFN) PAGE_ALIGN (BottomPfn - 1);
                    TopPfn = BottomPfn;
                }

                while (BottomPfn > BasePfn) {
                    BottomPfn -= 1;
                }

                //
                // If the entire range over which the PFN entries span is
                // completely zero and the PFN entry that maps the page is
                // not in the range, then add the page to the appropriate
                // free list.
                //

                Range = (ULONG)TopPfn - (ULONG)BottomPfn;
                if (RtlCompareMemoryUlong((PVOID)BottomPfn, Range, 0) == Range) {

                    //
                    // Set the PTE address to the physical page for virtual
                    // address alignment checking.
                    //

                    PointerPte = MiGetPteAddress (BasePfn);
                    PageFrameIndex = (PFN_NUMBER)PointerPte->u.Hard.PageFrameNumber;
                    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

                    ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                    Pfn1->u3.e2.ReferenceCount = 0;
                    Pfn1->u3.e1.CacheAttribute = MiCached;

                    MiDetermineNode (PageFrameIndex, Pfn1);
                    MiAddExpansionNonPagedPool (PageFrameIndex, 1, FALSE);
                }
            } while (BottomPfn > MmPfnDatabase);
        }
    }

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
    // Initialize the nonpaged pool.
    //

    InitializePool (NonPagedPool, 0);

    //
    // Initialize the system PTE pool now that nonpaged pool exists.
    // This is used for mapping I/O space, driver images and kernel stacks.
    // Note this expands the initial PTE allocation to use all possible
    // available virtual space by reclaiming the initial nonpaged
    // pool range (in non /3GB systems) because that range has already been
    // moved into the 2GB virtual range.
    //

    PointerPte = MiGetPteAddress (MmNonPagedSystemStart);
    ASSERT (((ULONG)PointerPte & (PAGE_SIZE - 1)) == 0);

    if (MmVirtualBias == 0) {
        MmNumberOfSystemPtes = MiGetPteAddress (NonPagedPoolStartVirtual) - PointerPte - 1;
    }
    else {
        if (MxMapLargePages & MI_LARGE_PFN_DATABASE_AND_NONPAGED_POOL) {
            AlignedPfnDatabase = (PMMPFN) ((ULONG) MmPfnDatabase & ~(MM_VA_MAPPED_BY_PDE - 1));
            MmNumberOfSystemPtes = MiGetPteAddress (AlignedPfnDatabase) - PointerPte;
            //
            // Reduce the system PTE space by one PTE to create a guard page.
            //

            MmNumberOfSystemPtes -= 1;
        }
        else {
            MmNumberOfSystemPtes = MiGetPteAddress (MmPfnDatabase) - PointerPte - 1;
        }
    }

    MiInitializeSystemPtes (PointerPte, MmNumberOfSystemPtes, SystemPteSpace);

    //
    // Now that the system PTE chain has been initialized, add any additional
    // ranges to the reserve.
    //

    if (ExtraPtes1 != 0) {

        //
        // Increment the system PTEs (for autoconfiguration purposes) and put
        // the PTE chain into reserve so it isn't used until needed (to
        // prevent fragmentation).
        //

        MiIncrementSystemPtes (ExtraPtes1);

        MiAddExtraSystemPteRanges (ExtraPtes1Pointer, ExtraPtes1);
    }

    if (ExtraPtes2 != 0) {

        //
        // Increment the system PTEs (for autoconfiguration purposes) and put
        // the PTE chain into reserve so it isn't used until needed (to
        // prevent fragmentation).
        //

        if (MM_SHARED_USER_DATA_VA > MiUseMaximumSystemSpace) {
            if (MiUseMaximumSystemSpaceEnd > MM_SHARED_USER_DATA_VA) {
                ExtraPtes2 = BYTES_TO_PAGES(MM_SHARED_USER_DATA_VA - MiUseMaximumSystemSpace);
            }
        }
        else {
            ASSERT (MmVirtualBias != 0);
        }

        if (ExtraPtes2 != 0) {

            //
            // Increment the system PTEs (for autoconfiguration purposes) but
            // don't actually add the PTEs till later (to prevent
            // fragmentation).
            //

            PointerPte = MiGetPteAddress (MiUseMaximumSystemSpace);

            MiAddExtraSystemPteRanges (PointerPte, ExtraPtes2);

            MiIncrementSystemPtes (ExtraPtes2);
        }
    }

    //
    // Initialize memory management structures for this process.
    //
    // Build the working set list.  This requires the creation of a PDE
    // to map hyperspace and the page table page pointed to
    // by the PDE must be initialized.
    //
    // Note we can't remove a zeroed page as hyperspace does not
    // exist and we map non-zeroed pages into hyperspace to zero them.
    //

    TempPde = ValidKernelPdeLocal;

    PointerPde = MiGetPdeAddress (HYPER_SPACE);

    LOCK_PFN (OldIrql);

    PageFrameIndex = MiRemoveAnyPage (0);
    TempPde.u.Hard.PageFrameNumber = PageFrameIndex;

    MI_WRITE_VALID_PTE (PointerPde, TempPde);

    MI_FLUSH_CURRENT_TB ();

    UNLOCK_PFN (OldIrql);

    //
    // Point to the page table page we just created and zero it.
    //

    PointerPte = MiGetPteAddress (HYPER_SPACE);
    RtlZeroMemory ((PVOID)PointerPte, PAGE_SIZE);

    //
    // Hyper space now exists, set the necessary variables.
    //

    MmFirstReservedMappingPte = MiGetPteAddress (FIRST_MAPPING_PTE);
    MmLastReservedMappingPte = MiGetPteAddress (LAST_MAPPING_PTE);

    MmFirstReservedMappingPte->u.Hard.PageFrameNumber = NUMBER_OF_MAPPING_PTES;

    MmWorkingSetList = (PMMWSL) ((ULONG_PTR)VAD_BITMAP_SPACE + PAGE_SIZE);

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

    LOCK_PFN (OldIrql);
    PageFrameIndex = MiRemoveAnyPage (0);
    UNLOCK_PFN (OldIrql);

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

    //
    // If booted /3GB, then the bitmap needs to be 2K bigger, shift
    // the working set accordingly as well.
    //
    // Note the 2K expansion portion of the bitmap is automatically
    // carved out of the working set page allocated below.
    //

    if (MmVirtualBias != 0) {
        MmWorkingSetList = (PMMWSL) ((ULONG_PTR)MmWorkingSetList + PAGE_SIZE / 2);
    }

    MiLastVadBit = (((ULONG_PTR) MI_64K_ALIGN (MM_HIGHEST_VAD_ADDRESS))) / X64K;

#if defined (_X86PAE_)

    //
    // Only bitmap the first 2GB of the PAE address space when booted /3GB.
    // This is because PAE has twice as many pagetable pages as non-PAE which
    // causes the MMWSL structure to be larger than 2K.  If we bitmapped the
    // entire user address space in this configuration then we'd need a 6K
    // bitmap and this would cause the initial MMWSL structure to overflow
    // into a second page.  This would require a bunch of extra code throughout
    // process support and other areas so just limit the bitmap for now.
    //

    if (MiLastVadBit > PAGE_SIZE * 8 - 1) {
        ASSERT (MmVirtualBias != 0);
        MiLastVadBit = PAGE_SIZE * 8 - 1;
        MmWorkingSetList = (PMMWSL) ((ULONG_PTR)VAD_BITMAP_SPACE + PAGE_SIZE);
    }

#endif

    //
    // Initialize this process's memory management structures including
    // the working set list.
    //
    // The PFN element for the page directory has already been initialized,
    // zero the reference count and the share count so they won't be
    // wrong.
    //

    Pfn1 = MI_PFN_ELEMENT (PdePageNumber);

    LOCK_PFN (OldIrql);

    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e2.ReferenceCount = 0;

#if defined (_X86PAE_)
    PointerPte = MiGetPteAddress (PDE_BASE);
    for (i = 0; i < PD_PER_SYSTEM; i += 1) {

        PdePageNumber = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        Pfn1 = MI_PFN_ELEMENT (PdePageNumber);
        Pfn1->u2.ShareCount = 0;
        Pfn1->u3.e2.ReferenceCount = 0;

        PointerPte += 1;
    }
#endif

    //
    // Get a page for the working set list and zero it.
    //

    TempPte = ValidKernelPteLocal;
    PageFrameIndex = MiRemoveAnyPage (0);
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPte = MiGetPteAddress (MmWorkingSetList);
    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    //
    // Note that when booted /3GB, MmWorkingSetList is not page aligned, so
    // always start zeroing from the start of the page regardless.
    //

    RtlZeroMemory (MiGetVirtualAddressMappedByPte (PointerPte), PAGE_SIZE);

    CurrentProcess->WorkingSetPage = PageFrameIndex;

#if defined (_X86PAE_)
    MiPaeInitialize ();
#endif

    MI_FLUSH_CURRENT_TB ();

    UNLOCK_PFN (OldIrql);

    CurrentProcess->Vm.MaximumWorkingSetSize = MmSystemProcessWorkingSetMax;
    CurrentProcess->Vm.MinimumWorkingSetSize = MmSystemProcessWorkingSetMin;

    DummyFlags = 0;
    MmInitializeProcessAddressSpace (CurrentProcess, NULL, NULL, &DummyFlags, NULL);

    //
    // Ensure the secondary page structures are marked as in use.
    //

    if (MmVirtualBias == 0) {

        ASSERT (MmFreePagesByColor[0] < (PMMCOLOR_TABLES)MM_SYSTEM_CACHE_END_EXTRA);

        PointerPde = MiGetPdeAddress(MmFreePagesByColor[0]);
        ASSERT (PointerPde->u.Hard.Valid == 1);

        PointerPte = MiGetPteAddress (MmFreePagesByColor[0]);
        ASSERT (PointerPte->u.Hard.Valid == 1);

        LastPte = MiGetPteAddress ((ULONG)&MmFreePagesByColor[1][MmSecondaryColors] - 1);
        ASSERT (LastPte->u.Hard.Valid == 1);

        LOCK_PFN (OldIrql);

        while (PointerPte <= LastPte) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            if (Pfn1->u3.e2.ReferenceCount == 0) {
                Pfn1->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
                Pfn1->PteAddress = PointerPte;
                Pfn1->u2.ShareCount += 1;
                Pfn1->u3.e2.ReferenceCount = 1;
                Pfn1->u3.e1.PageLocation = ActiveAndValid;
                Pfn1->u3.e1.CacheAttribute = MiCached;
                MiDetermineNode (PageFrameIndex, Pfn1);
            }
            PointerPte += 1;
        }
        UNLOCK_PFN (OldIrql);
    }

    return;
}

