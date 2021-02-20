/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
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

#include "Guard.h"
#include "Reload.h"
#include "Rtx.h"
#include "Scan.h"

void
NTAPI
InitializeSpace(
    __inout PGPBLOCK Block
)
{
    if (Block->BuildNumber >= 10586) {
        Block->PteBase = (PMMPTE)Block->DebuggerDataAdditionBlock.PteBase;

        Block->PteTop = (PMMPTE)
            ((s64)Block->PteBase |
            (((((s64)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PTI_SHIFT) << PTE_SHIFT) - 1));

        Block->PdeBase = (PMMPTE)
            (((s64)Block->PteBase & ~(((s64)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)) |
            (((s64)Block->PteBase >> 9) & (((s64)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)));

        Block->PdeTop = (PMMPTE)
            ((s64)Block->PdeBase |
            (((((s64)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PDI_SHIFT) << PTE_SHIFT) - 1));

        Block->PpeBase = (PMMPTE)
            (((s64)Block->PdeBase & ~(((s64)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)) |
            (((s64)Block->PdeBase >> 9) & (((s64)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)));

        Block->PpeTop = (PMMPTE)
            ((s64)Block->PpeBase |
            (((((s64)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PPI_SHIFT) << PTE_SHIFT) - 1));

        Block->PxeBase = (PMMPTE)
            (((s64)Block->PpeBase & ~(((s64)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)) |
            (((s64)Block->PpeBase >> 9) & (((s64)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)));

        Block->PxeTop = (PMMPTE)
            ((s64)Block->PxeBase |
            (((((s64)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PXI_SHIFT) << PTE_SHIFT) - 1));
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
    __in ptr VirtualAddress
)
{
    return GpBlock->PxeBase + MiGetPxeOffset(VirtualAddress);
}

PMMPTE
NTAPI
GetPpeAddress(
    __in ptr VirtualAddress
)
{
    return (PMMPTE)
        (((((s64)VirtualAddress & VIRTUAL_ADDRESS_MASK)
            >> PPI_SHIFT)
            << PTE_SHIFT) + (s64)GpBlock->PpeBase);
}

PMMPTE
NTAPI
GetPdeAddress(
    __in ptr VirtualAddress
)
{
    return (PMMPTE)
        (((((s64)VirtualAddress & VIRTUAL_ADDRESS_MASK)
            >> PDI_SHIFT)
            << PTE_SHIFT) + (s64)GpBlock->PdeBase);
}

PMMPTE
NTAPI
GetPteAddress(
    __in ptr VirtualAddress
)
{
    return (PMMPTE)
        (((((s64)VirtualAddress & VIRTUAL_ADDRESS_MASK)
            >> PTI_SHIFT)
            << PTE_SHIFT) + (s64)GpBlock->PteBase);
}

ptr
NTAPI
GetVaMappedByPte(
    __in PMMPTE Pte
)
{
    return (ptr)((((s64)Pte - (s64)GpBlock->PteBase) <<
        (PAGE_SHIFT + VA_SHIFT - PTE_SHIFT)) >> VA_SHIFT);
}

ptr
NTAPI
GetVaMappedByPde(
    __in PMMPTE Pde
)
{
    return GetVaMappedByPte(GetVaMappedByPte(Pde));
}

ptr
NTAPI
GetVaMappedByPpe(
    __in PMMPTE Ppe
)
{
    return GetVaMappedByPte(GetVaMappedByPde(Ppe));
}

ptr
NTAPI
GetVaMappedByPxe(
    __in PMMPTE Pxe
)
{
    return GetVaMappedByPde(GetVaMappedByPde(Pxe));
}
