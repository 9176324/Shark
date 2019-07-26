/*
*
* Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
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

#include "Detours.h"
#include "Reload.h"
#include "Rtx.h"
#include "Scan.h"

VOID
NTAPI
InitializeSystemSpace(
    __inout PGPBLOCK Block
)
{
    Block->PteBase = (PMMPTE)PTE_BASE;
    Block->PteTop = (PMMPTE)PTE_TOP;

    if (Block->DebuggerDataBlock.PaeEnabled) {
        Block->PdeBase = (PMMPTE)PDE_BASE_X86PAE;
    }
    else {
        Block->PdeBase = (PMMPTE)PDE_BASE_X86;
    }

    Block->PdeTop = (PMMPTE)PDE_TOP;
}

PMMPTE
NTAPI
GetPdeAddress(
    __in PVOID VirtualAddress
)
{
    return (PMMPTE)(0 != GpBlock->DebuggerDataBlock.PaeEnabled ?
        _GetPdeAddressPae(VirtualAddress, GpBlock->PdeBase) :
        _GetPdeAddress(VirtualAddress, GpBlock->PdeBase));
}

PMMPTE
NTAPI
GetPteAddress(
    __in PVOID VirtualAddress
)
{
    return (PMMPTE)(0 != GpBlock->DebuggerDataBlock.PaeEnabled ?
        _GetPteAddressPae(VirtualAddress, GpBlock->PteBase) :
        _GetPteAddress(VirtualAddress, GpBlock->PteBase));
}

PVOID
NTAPI
GetVirtualAddressMappedByPte(
    __in PMMPTE Pte
)
{
    return (PVOID)(0 != GpBlock->DebuggerDataBlock.PaeEnabled ?
        _GetVirtualAddressMappedByPtePae(Pte) :
        _GetVirtualAddressMappedByPte(Pte));
}

PVOID
NTAPI
GetVirtualAddressMappedByPde(
    __in PMMPTE Pde
)
{
    return (PVOID)(0 != GpBlock->DebuggerDataBlock.PaeEnabled ?
        _GetVirtualAddressMappedByPdePae(Pde) :
        _GetVirtualAddressMappedByPde(Pde));
}
