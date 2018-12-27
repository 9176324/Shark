/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License")); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original e is blindtiger.
*
*/

#include <defs.h>

#include "Space.h"

#include "Jump.h"
#include "Reload.h"
#include "Rtx.h"
#include "Scan.h"

VOID
NTAPI
InitializeSystemSpace(
    __inout PRELOADER_PARAMETER_BLOCK Block
)
{
    if (Block->BuildNumber >= 10586) {
        Block->PteBase = (PMMPTE)Block->DebuggerDataAdditionBlock.PteBase;

        Block->PteTop = (PMMPTE)
            ((LONGLONG)Block->PteBase |
            (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PTI_SHIFT) << PTE_SHIFT) - 1));

        Block->PdeBase = (PMMPTE)
            (((LONGLONG)Block->PteBase & ~(((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)) |
            (((LONGLONG)Block->PteBase >> 9) & (((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)));

        Block->PdeTop = (PMMPTE)
            ((LONGLONG)Block->PdeBase |
            (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PDI_SHIFT) << PTE_SHIFT) - 1));

        Block->PpeBase = (PMMPTE)
            (((LONGLONG)Block->PdeBase & ~(((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)) |
            (((LONGLONG)Block->PdeBase >> 9) & (((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)));

        Block->PpeTop = (PMMPTE)
            ((LONGLONG)Block->PpeBase |
            (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PPI_SHIFT) << PTE_SHIFT) - 1));

        Block->PxeBase = (PMMPTE)
            (((LONGLONG)Block->PpeBase & ~(((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)) |
            (((LONGLONG)Block->PpeBase >> 9) & (((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)));

        Block->PxeTop = (PMMPTE)
            ((LONGLONG)Block->PxeBase |
            (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PXI_SHIFT) << PTE_SHIFT) - 1));
    }
    else {
        Block->PteBase = (PMMPTE)PTE_BASE;
        Block->PteTop = (PMMPTE)PTE_TOP;
        Block->PdeBase = (PMMPTE)PDE_BASE;
        Block->PdeTop = (PMMPTE)PDE_TOP;
        Block->PpeBase = (PMMPTE)PPE_BASE;
        Block->PpeTop = (PMMPTE)PPE_TOP;
        Block->PxeBase = (PMMPTE)PXE_BASE;
        Block->PxeTop = (PMMPTE)PXE_TOP;
    }
}

PMMPTE
NTAPI
GetPxeAddress(
    __in PVOID VirtualAddress
)
{
    return ReloaderBlock->PxeBase + MiGetPxeOffset(VirtualAddress);
}

PMMPTE
NTAPI
GetPpeAddress(
    __in PVOID VirtualAddress
)
{
    return (PMMPTE)
        (((((LONGLONG)VirtualAddress &
            VIRTUAL_ADDRESS_MASK) >> PPI_SHIFT) << PTE_SHIFT) + (LONGLONG)ReloaderBlock->PpeBase);
}

PMMPTE
NTAPI
GetPdeAddress(
    __in PVOID VirtualAddress
)
{
    return (PMMPTE)
        (((((LONGLONG)VirtualAddress &
            VIRTUAL_ADDRESS_MASK) >> PDI_SHIFT) << PTE_SHIFT) + (LONGLONG)ReloaderBlock->PdeBase);
}

PMMPTE
NTAPI
GetPteAddress(
    __in PVOID VirtualAddress
)
{
    return (PMMPTE)
        (((((LONGLONG)VirtualAddress &
            VIRTUAL_ADDRESS_MASK) >> PTI_SHIFT) << PTE_SHIFT) + (LONGLONG)ReloaderBlock->PteBase);
}

PVOID
NTAPI
GetVirtualAddressMappedByPte(
    __in PMMPTE Pte
)
{
    return (PVOID)((((LONGLONG)Pte - (LONGLONG)ReloaderBlock->PteBase) <<
        (PAGE_SHIFT + VA_SHIFT - PTE_SHIFT)) >> VA_SHIFT);
}

PVOID
NTAPI
GetVirtualAddressMappedByPde(
    __in PMMPTE Pde
)
{
    return GetVirtualAddressMappedByPte(GetVirtualAddressMappedByPte(Pde));
}

PVOID
NTAPI
GetVirtualAddressMappedByPpe(
    __in PMMPTE Ppe
)
{
    return GetVirtualAddressMappedByPte(GetVirtualAddressMappedByPde(Ppe));
}

PVOID
NTAPI
GetVirtualAddressMappedByPxe(
    __in PMMPTE Pxe
)
{
    return GetVirtualAddressMappedByPde(GetVirtualAddressMappedByPde(Pxe));
}

VOID
NTAPI
MakeValidKernelPte(
    __in PMMPTE PointerPte,
    __in PFN_NUMBER PageFrameIndex,
    __in ULONG ProtectionMask
)
{
    MMPTE TempPte = { 0 };

    TempPte.u.Long = ProtectToPteMask[ProtectionMask] | MM_PTE_VALID_MASK;
    TempPte.u.Hard.Accessed = 1;
    TempPte.u.Hard.Global = 1;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPte->u.Long = TempPte.u.Long;
}

VOID
NTAPI
MakeValidUserPte(
    __in PMMPTE PointerPte,
    __in PFN_NUMBER PageFrameIndex,
    __in ULONG ProtectionMask
)
{
    MMPTE TempPte = { 0 };

    TempPte.u.Long = ProtectToPteMask[ProtectionMask] | MM_PTE_VALID_MASK;
    TempPte.u.Hard.Owner = 1;
    TempPte.u.Hard.Accessed = 1;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPte->u.Long = TempPte.u.Long;
}

PPFN
NTAPI
PfnElement(
    __in PFN_NUMBER PageFrameIndex
)
{
    PPFN Pfn = NULL;

    if (ReloaderBlock->BuildNumber < 9200) {
        Pfn = (PPFN_7600)&ReloaderBlock->DebuggerDataBlock.MmPfnDatabase + PageFrameIndex;
    }
    else if (ReloaderBlock->BuildNumber >= 9200 && ReloaderBlock->BuildNumber < 9600) {
        Pfn = (PPFN_9200)&ReloaderBlock->DebuggerDataBlock.MmPfnDatabase + PageFrameIndex;
    }
    else if (ReloaderBlock->BuildNumber >= 9600 && ReloaderBlock->BuildNumber < 10240) {
        Pfn = (PPFN_9600)&ReloaderBlock->DebuggerDataBlock.MmPfnDatabase + PageFrameIndex;
    }
    else {
        Pfn = (PPFN_10240)&ReloaderBlock->DebuggerDataBlock.MmPfnDatabase + PageFrameIndex;
    }

    return Pfn;
}

PFN_NUMBER
NTAPI
PfnElementToIndex(
    __in PPFN Pfn
)
{
    PFN_NUMBER PageFrameIndex = 0;

    if (ReloaderBlock->BuildNumber < 9200) {
        PageFrameIndex = &Pfn->Pfn7600 - (PPFN_7600)&ReloaderBlock->DebuggerDataBlock.MmPfnDatabase;
    }
    else if (ReloaderBlock->BuildNumber >= 9200 && ReloaderBlock->BuildNumber < 9600) {
        PageFrameIndex = &Pfn->Pfn9200 - (PPFN_9200)&ReloaderBlock->DebuggerDataBlock.MmPfnDatabase;
    }
    else if (ReloaderBlock->BuildNumber >= 9600 && ReloaderBlock->BuildNumber < 10240) {
        PageFrameIndex = &Pfn->Pfn9600 - (PPFN_9600)&ReloaderBlock->DebuggerDataBlock.MmPfnDatabase;
    }
    else {
        PageFrameIndex = &Pfn->Pfn10240 - (PPFN_10240)&ReloaderBlock->DebuggerDataBlock.MmPfnDatabase;
    }

    return PageFrameIndex;
}

VOID
NTAPI
InitializePfn(
    __in PFN_NUMBER PageFrameIndex,
    __in PPROTOTYPE_PFN PrototypePfn
)
{
    PPFN Pfn = NULL;

    Pfn = PfnElement(PageFrameIndex);

    if (MmIsAddressValid(Pfn)) {
        if (ReloaderBlock->BuildNumber < 9200) {
            Pfn->Pfn7600.PteAddress = PrototypePfn->PointerPte;
            Pfn->Pfn7600.OriginalPte.u.Soft.Protection = PrototypePfn->Protection;
            Pfn->Pfn7600.u3.e1.CacheAttribute = PrototypePfn->CacheAttribute;
            Pfn->Pfn7600.u3.e2.ReferenceCount = PrototypePfn->ReferenceCount;
            Pfn->Pfn7600.u2.ShareCount = PrototypePfn->ShareCount;
            Pfn->Pfn7600.u3.e1.PageLocation = PrototypePfn->PageLocation;
            Pfn->Pfn7600.u3.e1.Modified = PrototypePfn->Modified;
            Pfn->Pfn7600.u4.PteFrame = PrototypePfn->PteFrame;
        }
        else if (ReloaderBlock->BuildNumber >= 9200 && ReloaderBlock->BuildNumber < 9600) {
            Pfn->Pfn9200.PteAddress = PrototypePfn->PointerPte;
            Pfn->Pfn9200.OriginalPte.u.Soft.Protection = PrototypePfn->Protection;
            Pfn->Pfn9200.u3.e1.CacheAttribute = PrototypePfn->CacheAttribute;
            Pfn->Pfn9200.u3.e2.ReferenceCount = PrototypePfn->ReferenceCount;
            Pfn->Pfn9200.u2.ShareCount = PrototypePfn->ShareCount;
            Pfn->Pfn9200.u3.e1.PageLocation = PrototypePfn->PageLocation;
            Pfn->Pfn9200.u3.e1.Modified = PrototypePfn->Modified;
            Pfn->Pfn9200.u4.PteFrame = PrototypePfn->PteFrame;
        }
        else if (ReloaderBlock->BuildNumber >= 9600 && ReloaderBlock->BuildNumber < 10240) {
            Pfn->Pfn9600.PteAddress = PrototypePfn->PointerPte;
            Pfn->Pfn9600.OriginalPte.u.Soft.Protection = PrototypePfn->Protection;
            Pfn->Pfn9600.u3.e1.CacheAttribute = PrototypePfn->CacheAttribute;
            Pfn->Pfn9600.u3.e2.ReferenceCount = PrototypePfn->ReferenceCount;
            Pfn->Pfn9600.u2.ShareCount = PrototypePfn->ShareCount;
            Pfn->Pfn9600.u3.e1.PageLocation = PrototypePfn->PageLocation;
            Pfn->Pfn9600.u3.e1.Modified = PrototypePfn->Modified;
            Pfn->Pfn9600.u4.PteFrame = PrototypePfn->PteFrame;
        }
        else {
            Pfn->Pfn10240.PteAddress = PrototypePfn->PointerPte;
            Pfn->Pfn10240.OriginalPte.u.Soft.Protection = PrototypePfn->Protection;
            Pfn->Pfn10240.u3.e1.CacheAttribute = PrototypePfn->CacheAttribute;
            Pfn->Pfn10240.u3.e2.ReferenceCount = PrototypePfn->ReferenceCount;
            Pfn->Pfn10240.u2.ShareCount = PrototypePfn->ShareCount;
            Pfn->Pfn10240.u3.e1.PageLocation = PrototypePfn->PageLocation;
            Pfn->Pfn10240.u3.e1.Modified = PrototypePfn->Modified;
            Pfn->Pfn10240.u4.PteFrame = PrototypePfn->PteFrame;
        }
    }
}

VOID
NTAPI
LocalPfn(
    __in PFN_NUMBER PageFrameIndex,
    __out PPROTOTYPE_PFN PrototypePfn
)
{
    PPFN Pfn = NULL;

    Pfn = PfnElement(PageFrameIndex);

    if (MmIsAddressValid(Pfn)) {
        if (ReloaderBlock->BuildNumber < 9200) {
            PrototypePfn->PointerPte = Pfn->Pfn7600.PteAddress;
            PrototypePfn->Protection = Pfn->Pfn7600.OriginalPte.u.Soft.Protection;
            PrototypePfn->CacheAttribute = Pfn->Pfn7600.u3.e1.CacheAttribute;
            PrototypePfn->ReferenceCount = Pfn->Pfn7600.u3.e2.ReferenceCount;
            PrototypePfn->ShareCount = Pfn->Pfn7600.u2.ShareCount;
            PrototypePfn->PageLocation = Pfn->Pfn7600.u3.e1.PageLocation;
            PrototypePfn->Modified = Pfn->Pfn7600.u3.e1.Modified;
            PrototypePfn->PteFrame = Pfn->Pfn7600.u4.PteFrame;
        }
        else if (ReloaderBlock->BuildNumber >= 9200 && ReloaderBlock->BuildNumber < 9600) {
            PrototypePfn->PointerPte = Pfn->Pfn9200.PteAddress;
            PrototypePfn->Protection = Pfn->Pfn9200.OriginalPte.u.Soft.Protection;
            PrototypePfn->CacheAttribute = Pfn->Pfn9200.u3.e1.CacheAttribute;
            PrototypePfn->ReferenceCount = Pfn->Pfn9200.u3.e2.ReferenceCount;
            PrototypePfn->ShareCount = Pfn->Pfn9200.u2.ShareCount;
            PrototypePfn->PageLocation = Pfn->Pfn9200.u3.e1.PageLocation;
            PrototypePfn->Modified = Pfn->Pfn9200.u3.e1.Modified;
            PrototypePfn->PteFrame = Pfn->Pfn9200.u4.PteFrame;
        }
        else if (ReloaderBlock->BuildNumber >= 9600 && ReloaderBlock->BuildNumber < 10240) {
            PrototypePfn->PointerPte = Pfn->Pfn9600.PteAddress;
            PrototypePfn->Protection = Pfn->Pfn9600.OriginalPte.u.Soft.Protection;
            PrototypePfn->CacheAttribute = Pfn->Pfn9600.u3.e1.CacheAttribute;
            PrototypePfn->ReferenceCount = Pfn->Pfn9600.u3.e2.ReferenceCount;
            PrototypePfn->ShareCount = Pfn->Pfn9600.u2.ShareCount;
            PrototypePfn->PageLocation = Pfn->Pfn9600.u3.e1.PageLocation;
            PrototypePfn->Modified = Pfn->Pfn9600.u3.e1.Modified;
            PrototypePfn->PteFrame = Pfn->Pfn9600.u4.PteFrame;
        }
        else {
            PrototypePfn->PointerPte = Pfn->Pfn10240.PteAddress;
            PrototypePfn->Protection = Pfn->Pfn10240.OriginalPte.u.Soft.Protection;
            PrototypePfn->CacheAttribute = Pfn->Pfn10240.u3.e1.CacheAttribute;
            PrototypePfn->ReferenceCount = Pfn->Pfn10240.u3.e2.ReferenceCount;
            PrototypePfn->ShareCount = Pfn->Pfn10240.u2.ShareCount;
            PrototypePfn->PageLocation = Pfn->Pfn10240.u3.e1.PageLocation;
            PrototypePfn->Modified = Pfn->Pfn10240.u3.e1.Modified;
            PrototypePfn->PteFrame = Pfn->Pfn10240.u4.PteFrame;
        }
    }
}

VOID
NTAPI
SetPageProtection(
    __in_bcount(NumberOfBytes) PVOID VirtualAddress,
    __in SIZE_T NumberOfBytes,
    __in ULONG ProtectionMask
)
{
    PFN_NUMBER PageFrameIndex = 0;
    PFN_NUMBER NumberOfPages = 0;
    PMMPTE PointerPte = NULL;
    MMPTE NewPteContents = { 0 };
    MMPTE TempPte = { 0 };
    PROTOTYPE_PFN PrototypePfn = { 0 };

    NumberOfBytes = BYTE_OFFSET(VirtualAddress) + NumberOfBytes;
    NumberOfPages = BYTES_TO_PAGES(NumberOfBytes);

    ASSERT(0 != NumberOfPages);

    if ((ULONG_PTR)VirtualAddress > (ULONG_PTR)MM_HIGHEST_USER_ADDRESS) {
        MakeValidKernelPte(&NewPteContents, 0, ProtectionMask);
    }
    else {
        MakeValidUserPte(&NewPteContents, 0, ProtectionMask);
    }

    for (PageFrameIndex = 0;
        PageFrameIndex < NumberOfPages;
        PageFrameIndex += 1) {
        PointerPte = GetPteAddress((PCHAR)VirtualAddress + PageFrameIndex * PAGE_SIZE);

        TempPte.u.Long = PointerPte->u.Long;

        NewPteContents.u.Hard.PageFrameNumber = TempPte.u.Hard.PageFrameNumber;
        NewPteContents.u.Hard.Dirty = TempPte.u.Hard.Dirty;

        PointerPte->u.Long = NewPteContents.u.Long;

        LocalPfn(
            PointerPte->u.Hard.PageFrameNumber,
            &PrototypePfn);

        PrototypePfn.Protection = ProtectionMask;

        PrototypePfn.CacheAttribute =
            MM_PTE_NOCACHE == (ProtectToPteMask[ProtectionMask] & MM_PTE_NOCACHE) ?
            MiNonCached : MiCached;

        InitializePfn(
            PointerPte->u.Hard.PageFrameNumber,
            &PrototypePfn);
    }

    FlushMultipleTb(VirtualAddress, NumberOfBytes, TRUE);
}

PFN_NUMBER
NTAPI
GetPageFrame(
    __in PVOID Page
)
{
    PMMPTE PointerPde = NULL;
    PMMPTE PointerPte = NULL;
    PFN_NUMBER PageFrameIndex = 0;

    PointerPte = GetPteAddress(Page);
    PointerPde = GetPdeAddress(Page);

    if (0 != PointerPde->u.Hard.LargePage) {
        PageFrameIndex = GET_PAGE_FRAME_FROM_PTE(PointerPde) +
            (BYTE_OFFSET(PointerPte) / sizeof(MMPTE));
    }
    else {
        PageFrameIndex = GET_PAGE_FRAME_FROM_PTE(PointerPte);
    }

    return PageFrameIndex;
}

PVOID
NTAPI
AllocateDriverPages(
    __in SIZE_T NumberOfBytes
)
{
    PMMPTE TempPte = NULL;
    PVOID TempAddress = NULL;
    PVOID AllocatedAddress = NULL;
    PFN_NUMBER NumberOfPtes = 0;
    PFN_NUMBER Index = 0;
    PFN_NUMBER PageFrameIndex = 0;

    NumberOfPtes = BYTES_TO_PAGES(NumberOfBytes);

    TempAddress = ExAllocatePool(NonPagedPool, NumberOfPtes * PAGE_SIZE);

    if (NULL != TempAddress) {
        AllocatedAddress = MmAllocateMappingAddress(NumberOfBytes, -1);

        if (NULL != AllocatedAddress) {
            for (Index = 0;
                Index < NumberOfPtes;
                Index++) {
                TempPte = GetPteAddress((PCHAR)AllocatedAddress + Index * PAGE_SIZE);

                PageFrameIndex = GetPageFrame((PCHAR)TempAddress + Index * PAGE_SIZE);

                MakeValidKernelPte(TempPte, PageFrameIndex, MM_READWRITE);

                SetPageProtection(
                    GetVirtualAddressMappedByPte(TempPte),
                    PAGE_SIZE,
                    MM_READWRITE);
            }
        }
    }

    FlushMultipleTb(AllocatedAddress, NumberOfBytes, TRUE);

    return AllocatedAddress;
}

VOID
NTAPI
FreeDriverPages(
    __in PVOID VirtualAddress
)
{
    PMMPTE TempPte = NULL;
    PVOID TempAddress = NULL;
    PROTOTYPE_PFN PrototypePfn = { 0 };

    TempPte = GetPteAddress(VirtualAddress);

    LocalPfn(
        GET_PAGE_FRAME_FROM_PTE(TempPte),
        &PrototypePfn);

    MmFreeMappingAddress(VirtualAddress, -1);

    TempAddress = GetVirtualAddressMappedByPte(PrototypePfn.PointerPte);

    ExFreePool(TempAddress);
}
