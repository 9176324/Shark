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

static PMMPTE PxeBase;
static PMMPTE PpeBase;
static PMMPTE PdeBase;
static PMMPTE PteBase;

static PMMPTE PxeTop;
static PMMPTE PpeTop;
static PMMPTE PdeTop;
static PMMPTE PteTop;

VOID
NTAPI
InitializeSystemSpace(
    __in PVOID Parameter
)
{
    PVOID ImageBase = NULL;
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PCHAR SectionBase = NULL;
    ULONG SizeToLock = 0;
    UNICODE_STRING ImageFileName = { 0 };
    PLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ULONG BuildNumber = 0;
    PCHAR ControlPc = NULL;
    PCHAR TargetPc = NULL;
    UNICODE_STRING RoutineString = { 0 };
    ULONG Length = 0;
    USHORT Index = 0;
    PRUNTIME_FUNCTION FunctionEntry = NULL;

    // 48 C1 E9 09                      shr     rcx, 9
    // 48 B8 F8 FF FF FF 7F 00 00 00    mov     rax, 7FFFFFFFF8h
    // 48 23 C8                         and     rcx, rax
    // 48 B8 00 00 00 00 80 F6 FF FF    mov     rax, 0FFFFF68000000000h
    // 48 03 C1                         add     rax, rcx
    // C3                               retn

    CHAR MiGetPteAddress[] =
        "48 c1 e9 09 48 b8 f8 ff ff ff 7f 00 00 00 48 23 c8 48 b8 ?? ?? ?? ?? ?? ?? ?? ?? 48 03 c1 c3";

    // 48 F7 E9                         imul    rcx
    // 4C 8B E2                         mov     r12, rdx
    // 49 C1 FC 03                      sar     r12, 3
    // 49 8B CC                         mov     rcx, r12
    // 48 C1 E9 3F                      shr     rcx, 3Fh
    // 4C 03 E1                         add     r12, rcx
    // 44 0F 20 C0                      mov     rax, cr8
    // 48 89 44 24 68                   mov     [rsp+58h+arg_8], rax

    CHAR MiDrainZeroLookasides[] =
        "48 f7 e9 4c 8b ?? 49 c1 ?? 03 49 8b ?? 48 c1 e9 3f 4c 03 ?? 44 0f 20 c0 48 89 44 24 ??";

    // 8B C5                            mov     eax, ebp
    // 48 8D 0C 80                      lea     rcx, [rax+rax*4]
    // 49 8B 84 F0 C0 F1 34 00          mov     rax, ds:rva MmFreePagesByColor[r8+rsi*8]
    // 48 8D 1C C8                      lea     rbx, [rax+rcx*8]

    CHAR MmDuplicateMemory[] =
        "8b ?? 48 8d 0c 80 49 8b 84 ?? ?? ?? ?? ?? 48 8d ?? c8";

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    if (BuildNumber >= 10586) {
        RtlInitUnicodeString(&ImageFileName, L"ntoskrnl.exe");

        if (RTL_SOFT_ASSERT(NT_SUCCESS(FindEntryForKernelImage(
            &ImageFileName,
            &DataTableEntry)))) {
            NtHeaders = RtlImageNtHeader(DataTableEntry->DllBase);

            if (NULL != NtHeaders) {
                NtSection = IMAGE_FIRST_SECTION(NtHeaders);

                SectionBase = (PCHAR)DataTableEntry->DllBase + NtSection[0].VirtualAddress;

                SizeToLock = max(
                    NtSection[0].SizeOfRawData,
                    NtSection[0].Misc.VirtualSize);

                ControlPc = ScanBytes(
                    SectionBase,
                    (PCHAR)SectionBase + SizeToLock,
                    MiGetPteAddress);

                if (NULL != ControlPc) {
                    PteBase = *(PMMPTE *)(ControlPc + 19);

                    PteTop = (PMMPTE)
                        ((LONGLONG)PteBase |
                        (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PTI_SHIFT) << PTE_SHIFT) - 1));

                    PdeBase = (PMMPTE)
                        (((LONGLONG)PteBase & ~(((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)) |
                        (((LONGLONG)PteBase >> 9) & (((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)));

                    PdeTop = (PMMPTE)
                        ((LONGLONG)PdeBase |
                        (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PDI_SHIFT) << PTE_SHIFT) - 1));

                    PpeBase = (PMMPTE)
                        (((LONGLONG)PdeBase & ~(((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)) |
                        (((LONGLONG)PdeBase >> 9) & (((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)));

                    PpeTop = (PMMPTE)
                        ((LONGLONG)PpeBase |
                        (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PPI_SHIFT) << PTE_SHIFT) - 1));

                    PxeBase = (PMMPTE)
                        (((LONGLONG)PpeBase & ~(((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)) |
                        (((LONGLONG)PpeBase >> 9) & (((LONGLONG)1 << (PHYSICAL_ADDRESS_BITS - 1)) - 1)));

                    PxeTop = (PMMPTE)
                        ((LONGLONG)PxeBase |
                        (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS + 1)) >> PXI_SHIFT) << PTE_SHIFT) - 1));
                }
            }
        }
    }
    else {
        PteBase = (PMMPTE)PTE_BASE;
        PteTop = (PMMPTE)PTE_TOP;
        PdeBase = (PMMPTE)PDE_BASE;
        PdeTop = (PMMPTE)PDE_TOP;
        PpeBase = (PMMPTE)PPE_BASE;
        PpeTop = (PMMPTE)PPE_TOP;
        PxeBase = (PMMPTE)PXE_BASE;
        PxeTop = (PMMPTE)PXE_TOP;
    }

    RtlInitUnicodeString(&RoutineString, L"KeCapturePersistentThreadState");

    ControlPc = MmGetSystemRoutineAddress(&RoutineString);

    if (NULL != ControlPc) {
        while (0 != _CmpByte(ControlPc[0], 0xc2) &&
            0 != _CmpByte(ControlPc[0], 0xc3)) {
            Length = DetourGetInstructionLength(ControlPc);

            if (7 == Length) {
                if (0 == _CmpByte(ControlPc[0], 0x48) &&
                    0 == _CmpByte(ControlPc[1], 0x8b) &&
                    0 == _CmpByte(ControlPc[2], 0x05)) {
                    PfnDatabase = *(PVOID *)RvaToVa(ControlPc + Length - sizeof(LONG));

                    break;
                }
            }

            ControlPc += Length;
        }
    }

    RtlInitUnicodeString(&ImageFileName, L"ntoskrnl.exe");

    if (RTL_SOFT_ASSERT(NT_SUCCESS(FindEntryForKernelImage(
        &ImageFileName,
        &DataTableEntry)))) {
        if (BuildNumber > 9600) {
            RtlInitUnicodeString(&RoutineString, L"MmAddPhysicalMemory");

            ControlPc = MmGetSystemRoutineAddress(&RoutineString);

            if (NULL != ControlPc) {
                while (0 != _CmpByte(ControlPc[0], 0xc2) &&
                    0 != _CmpByte(ControlPc[0], 0xc3)) {
                    Length = DetourGetInstructionLength(ControlPc);

                    if (7 == Length) {
                        if (0 == _CmpByte(ControlPc[0], 0x48) &&
                            0 == _CmpByte(ControlPc[1], 0x8d) &&
                            0 == _CmpByte(ControlPc[2], 0x0d)) {
                            TargetPc = RvaToVa(ControlPc + Length - sizeof(LONG));

                            for (ControlPc = TargetPc;
                                ;
                                ControlPc += 0x10) {
                                if (FreePageList == ((PPARTITION_PAGE_LISTS)ControlPc)->FreePageListHead.ListName &&
                                    ZeroedPageList == ((PPARTITION_PAGE_LISTS)ControlPc)->ZeroedPageListHead.ListName &&
                                    StandbyPageList == ((PPARTITION_PAGE_LISTS)ControlPc)->StandbyPageListHead.ListName) {
                                    TargetPc = ControlPc;

                                    RtlCopyMemory(
                                        &FreePagesByColor,
                                        &TargetPc,
                                        sizeof(PVOID));

                                    FreePageSlist[0] = ((PPARTITION_PAGE_LISTS)TargetPc)->FreePageSlist[0];
                                    FreePageSlist[1] = ((PPARTITION_PAGE_LISTS)TargetPc)->FreePageSlist[1];

                                    MaximumColor =
                                        max(FreePageSlist[0], FreePageSlist[1]) -
                                        min(FreePageSlist[0], FreePageSlist[1]);

                                    break;
                                }
                            }

                            break;
                        }
                    }

                    ControlPc += Length;
                }
            }
        }
        else {
            NtHeaders = RtlImageNtHeader(DataTableEntry->DllBase);

            if (NULL != NtHeaders) {
                NtSection = IMAGE_FIRST_SECTION(NtHeaders);

                SectionBase = (PCHAR)DataTableEntry->DllBase + NtSection[0].VirtualAddress;

                SizeToLock = max(
                    NtSection[0].SizeOfRawData,
                    NtSection[0].Misc.VirtualSize);

                ControlPc = ScanBytes(
                    SectionBase,
                    (PCHAR)SectionBase + SizeToLock,
                    MiDrainZeroLookasides);

                if (NULL != ControlPc) {
                    FunctionEntry = RtlLookupFunctionEntry(
                        (ULONG64)ControlPc,
                        (PULONG64)&ImageBase,
                        NULL);

                    if (NULL != FunctionEntry) {
                        ControlPc = (PCHAR)ImageBase + FunctionEntry->BeginAddress;

                        while ((ULONG_PTR)ControlPc <
                            ((ULONG_PTR)ImageBase + FunctionEntry->EndAddress)) {
                            if (0 == _CmpByte(ControlPc[0], 0x48) &&
                                0 == _CmpByte(ControlPc[1], 0x8b)) {
                                Length = DetourGetInstructionLength(ControlPc);

                                if (7 == Length) {
                                    if (NULL == FreePageSlist[0]) {
                                        FreePageSlist[0] = *(PVOID *)RvaToVa(ControlPc + Length - sizeof(LONG));
                                    }
                                    else if (NULL == FreePageSlist[1]) {
                                        FreePageSlist[1] = *(PVOID *)RvaToVa(ControlPc + Length - sizeof(LONG));

                                        MaximumColor =
                                            max(FreePageSlist[0], FreePageSlist[1]) -
                                            min(FreePageSlist[0], FreePageSlist[1]);

                                        break;
                                    }
                                }
                            }

                            ControlPc++;
                        }
                    }
                }

                for (Index = 0;
                    Index < NtHeaders->FileHeader.NumberOfSections;
                    Index++) {
                    if ((0 == _CmpByte(NtSection[Index].Name[0], 'p') || 0 == _CmpByte(NtSection[Index].Name[0], 'P')) &&
                        (0 == _CmpByte(NtSection[Index].Name[1], 'a') || 0 == _CmpByte(NtSection[Index].Name[1], 'A')) &&
                        (0 == _CmpByte(NtSection[Index].Name[2], 'g') || 0 == _CmpByte(NtSection[Index].Name[2], 'G')) &&
                        (0 == _CmpByte(NtSection[Index].Name[3], 'e') || 0 == _CmpByte(NtSection[Index].Name[3], 'E')) &&
                        (0 == _CmpByte(NtSection[Index].Name[4], 'l') || 0 == _CmpByte(NtSection[Index].Name[4], 'L')) &&
                        (0 == _CmpByte(NtSection[Index].Name[5], 'k') || 0 == _CmpByte(NtSection[Index].Name[5], 'K'))) {
                        if (0 != NtSection[Index].VirtualAddress) {
                            SectionBase = (PCHAR)DataTableEntry->DllBase + NtSection[Index].VirtualAddress;

                            SizeToLock = max(
                                NtSection[Index].SizeOfRawData,
                                NtSection[Index].Misc.VirtualSize);

                            ControlPc = ScanBytes(
                                SectionBase,
                                (PCHAR)SectionBase + SizeToLock,
                                MmDuplicateMemory);

                            if (NULL != ControlPc) {
                                TargetPc =
                                    (PCHAR)DataTableEntry->DllBase + *(PLONG)(ControlPc + 10);

                                RtlCopyMemory(
                                    &FreePagesByColor,
                                    &TargetPc,
                                    sizeof(PVOID));
                            }
                        }
                    }
                }
            }
        }
    }
}

PMMPTE
NTAPI
GetPxeAddress(
    __in PVOID VirtualAddress
)
{
    return PxeBase + MiGetPxeOffset(VirtualAddress);
}

PMMPTE
NTAPI
GetPpeAddress(
    __in PVOID VirtualAddress
)
{
    return (PMMPTE)
        (((((LONGLONG)VirtualAddress &
            VIRTUAL_ADDRESS_MASK) >> PPI_SHIFT) << PTE_SHIFT) + (LONGLONG)PpeBase);
}

PMMPTE
NTAPI
GetPdeAddress(
    __in PVOID VirtualAddress
)
{
    return (PMMPTE)
        (((((LONGLONG)VirtualAddress &
            VIRTUAL_ADDRESS_MASK) >> PDI_SHIFT) << PTE_SHIFT) + (LONGLONG)PdeBase);
}

PMMPTE
NTAPI
GetPteAddress(
    __in PVOID VirtualAddress
)
{
    return (PMMPTE)
        (((((LONGLONG)VirtualAddress &
            VIRTUAL_ADDRESS_MASK) >> PTI_SHIFT) << PTE_SHIFT) + (LONGLONG)PteBase);
}

PVOID
NTAPI
GetVirtualAddressMappedByPte(
    __in PMMPTE Pte
)
{
    return (PVOID)((((LONGLONG)Pte - (LONGLONG)PteBase) <<
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
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    if (BuildNumber < 9200) {
        Pfn = &PfnDatabase->Pfn7600 + PageFrameIndex;
    }
    else if (BuildNumber >= 9200 && BuildNumber < 9600) {
        Pfn = &PfnDatabase->Pfn9200 + PageFrameIndex;
    }
    else if (BuildNumber >= 9600 && BuildNumber < 10240) {
        Pfn = &PfnDatabase->Pfn9600 + PageFrameIndex;
    }
    else {
        Pfn = &PfnDatabase->Pfn10240 + PageFrameIndex;
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
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    if (BuildNumber < 9200) {
        PageFrameIndex = &Pfn->Pfn7600 - &PfnDatabase->Pfn7600;
    }
    else if (BuildNumber >= 9200 && BuildNumber < 9600) {
        PageFrameIndex = &Pfn->Pfn9200 - &PfnDatabase->Pfn9200;
    }
    else if (BuildNumber >= 9600 && BuildNumber < 10240) {
        PageFrameIndex = &Pfn->Pfn9600 - &PfnDatabase->Pfn9600;
    }
    else {
        PageFrameIndex = &Pfn->Pfn10240 - &PfnDatabase->Pfn10240;
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
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    Pfn = PfnElement(PageFrameIndex);

    if (FALSE != MmIsAddressValid(Pfn)) {
        if (BuildNumber < 9200) {
            Pfn->Pfn7600.PteAddress = PrototypePfn->PointerPte;
            Pfn->Pfn7600.OriginalPte.u.Soft.Protection = PrototypePfn->Protection;
            Pfn->Pfn7600.u3.e1.CacheAttribute = PrototypePfn->CacheAttribute;
            Pfn->Pfn7600.u3.e2.ReferenceCount = PrototypePfn->ReferenceCount;
            Pfn->Pfn7600.u2.ShareCount = PrototypePfn->ShareCount;
            Pfn->Pfn7600.u3.e1.PageLocation = PrototypePfn->PageLocation;
            Pfn->Pfn7600.u3.e1.Modified = PrototypePfn->Modified;
            Pfn->Pfn7600.u4.PteFrame = PrototypePfn->PteFrame;
        }
        else if (BuildNumber >= 9200 && BuildNumber < 9600) {
            Pfn->Pfn9200.PteAddress = PrototypePfn->PointerPte;
            Pfn->Pfn9200.OriginalPte.u.Soft.Protection = PrototypePfn->Protection;
            Pfn->Pfn9200.u3.e1.CacheAttribute = PrototypePfn->CacheAttribute;
            Pfn->Pfn9200.u3.e2.ReferenceCount = PrototypePfn->ReferenceCount;
            Pfn->Pfn9200.u2.ShareCount = PrototypePfn->ShareCount;
            Pfn->Pfn9200.u3.e1.PageLocation = PrototypePfn->PageLocation;
            Pfn->Pfn9200.u3.e1.Modified = PrototypePfn->Modified;
            Pfn->Pfn9200.u4.PteFrame = PrototypePfn->PteFrame;
        }
        else if (BuildNumber >= 9600 && BuildNumber < 10240) {
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
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    Pfn = PfnElement(PageFrameIndex);

    if (FALSE != MmIsAddressValid(Pfn)) {
        if (BuildNumber < 9200) {
            PrototypePfn->PointerPte = Pfn->Pfn7600.PteAddress;
            PrototypePfn->Protection = Pfn->Pfn7600.OriginalPte.u.Soft.Protection;
            PrototypePfn->CacheAttribute = Pfn->Pfn7600.u3.e1.CacheAttribute;
            PrototypePfn->ReferenceCount = Pfn->Pfn7600.u3.e2.ReferenceCount;
            PrototypePfn->ShareCount = Pfn->Pfn7600.u2.ShareCount;
            PrototypePfn->PageLocation = Pfn->Pfn7600.u3.e1.PageLocation;
            PrototypePfn->Modified = Pfn->Pfn7600.u3.e1.Modified;
            PrototypePfn->PteFrame = Pfn->Pfn7600.u4.PteFrame;
        }
        else if (BuildNumber >= 9200 && BuildNumber < 9600) {
            PrototypePfn->PointerPte = Pfn->Pfn9200.PteAddress;
            PrototypePfn->Protection = Pfn->Pfn9200.OriginalPte.u.Soft.Protection;
            PrototypePfn->CacheAttribute = Pfn->Pfn9200.u3.e1.CacheAttribute;
            PrototypePfn->ReferenceCount = Pfn->Pfn9200.u3.e2.ReferenceCount;
            PrototypePfn->ShareCount = Pfn->Pfn9200.u2.ShareCount;
            PrototypePfn->PageLocation = Pfn->Pfn9200.u3.e1.PageLocation;
            PrototypePfn->Modified = Pfn->Pfn9200.u3.e1.Modified;
            PrototypePfn->PteFrame = Pfn->Pfn9200.u4.PteFrame;
        }
        else if (BuildNumber >= 9600 && BuildNumber < 10240) {
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

PFN_NUMBER
NTAPI
_RemoveAnyPage(
    __in PVOID PointerPte
)
{
    PFN_NUMBER PageFrameIndex = 0;
    PPFN TempPfn = NULL;
    USHORT DepthIndex = 0;
    USHORT SlistIndex = 0;
    USHORT Color = 0;

    if (0 != FreeSlist.Depth) {
        TempPfn = ExpInterlockedPopEntrySList(&FreeSlist);

        PageFrameIndex = PfnElementToIndex(TempPfn);

        return PageFrameIndex;
    }

    for (DepthIndex = 0;
        DepthIndex < MAXIMUM_PFNSLIST_DEPTH;
        DepthIndex++) {
        for (SlistIndex = 0;
            SlistIndex < MAXIMUM_PFNSLIST_COUNT;
            SlistIndex++) {
            for (Color = 0;
                Color < MaximumColor;
                Color++) {
                if ((MAXIMUM_PFNSLIST_DEPTH - DepthIndex) == FreePageSlist[SlistIndex][Color].Depth) {
                    TempPfn = ExpInterlockedPopEntrySList(&FreePageSlist[SlistIndex][Color]);

                    PageFrameIndex = PfnElementToIndex(TempPfn);

                    return PageFrameIndex;
                }
            }
        }
    }

    return PageFrameIndex;
}

PFN_NUMBER
NTAPI
RemoveAnyPage(
    __in PVOID PointerPte
)
{
    PFN_NUMBER PageFrameIndex = 0;
    PROTOTYPE_PFN PrototypePfn = { 0 };
    PROTOTYPE_PFN PreviousPfn = { 0 };
    PROTOTYPE_PFN NextPfn = { 0 };
    PSINGLE_LIST_ENTRY TempEntry = NULL;
    PVOID VirtualAddress = NULL;

    PageFrameIndex = IpiSingleCall(
        (PPS_APC_ROUTINE)NULL,
        (PKSYSTEM_ROUTINE)NULL,
        (PUSER_THREAD_START_ROUTINE)_RemoveAnyPage,
        (PVOID)PointerPte);

    ASSERT(0 != PageFrameIndex);

    PrototypePfn.PointerPte = PointerPte;
    PrototypePfn.Protection = MM_READWRITE;
    PrototypePfn.CacheAttribute = MiCached;
    PrototypePfn.ReferenceCount = 1;
    PrototypePfn.ShareCount = 1;
    PrototypePfn.PageLocation = ActiveAndValid;
    PrototypePfn.Modified = 1;
    PrototypePfn.PteFrame = GET_PAGE_FRAME_FROM_PTE(GetPteAddress(PointerPte));

    InitializePfn(PageFrameIndex, &PrototypePfn);

    VirtualAddress = GetVirtualAddressMappedByPte(PointerPte);

    if ((ULONG_PTR)VirtualAddress > (ULONG_PTR)MM_HIGHEST_USER_ADDRESS) {
        MakeValidKernelPte(PointerPte, PageFrameIndex, MM_READWRITE);
    }
    else {
        MakeValidUserPte(PointerPte, PageFrameIndex, MM_READWRITE);
    }

    FlushMultipleTb(VirtualAddress, PAGE_SIZE, TRUE);

    RtlZeroMemory(VirtualAddress, PAGE_SIZE);

    return PageFrameIndex;
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
    PMMPTE PointerPde = NULL;
    PMMPTE PointerPte = NULL;
    MMPTE TempPte = { 0 };
    MMPTE NewPteContents = { 0 };
    PROTOTYPE_PFN PrototypePfn = { 0 };

    PointerPte = GetPteAddress(VirtualAddress);

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
        TempPte.u.Long = PointerPte->u.Long;

        NewPteContents.u.Hard.PageFrameNumber = TempPte.u.Hard.PageFrameNumber;
        NewPteContents.u.Hard.Dirty = TempPte.u.Hard.Dirty;
        NewPteContents.u.Hard.SoftwareWsIndex = TempPte.u.Hard.SoftwareWsIndex;

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

        PointerPte++;
    }

    FlushMultipleTb(VirtualAddress, NumberOfBytes, TRUE);
}

PVOID
NTAPI
AllocateIndependentPages(
    __in SIZE_T NumberOfBytes
)
{
    PMMPTE TempPte = NULL;
    PVOID AllocatedAddress = NULL;
    PFN_NUMBER NumberOfPtes = 0;
    PFN_NUMBER Index = 0;
    PFN_NUMBER LargePageBaseFrame = 0;
    PFN_NUMBER PageFrameIndex = 0;

    NumberOfPtes = BYTES_TO_PAGES(NumberOfBytes);
    AllocatedAddress = MmAllocateMappingAddress(NumberOfBytes, -1);

    if (NULL != AllocatedAddress) {
        TempPte = GetPteAddress(AllocatedAddress);

        for (Index = 0;
            Index < NumberOfPtes;
            Index++) {
            PageFrameIndex = RemoveAnyPage(&TempPte[Index]);

            MakeValidKernelPte(&TempPte[Index], PageFrameIndex, MM_READWRITE);

            SetPageProtection(
                GetVirtualAddressMappedByPte(&TempPte[Index]),
                PAGE_SIZE,
                MM_READWRITE);
        }
    }

    FlushMultipleTb(AllocatedAddress, NumberOfBytes, TRUE);

    return AllocatedAddress;
}

VOID
NTAPI
_FreePfn(
    __in PFN_NUMBER PageFrameIndex
)
{
    PSLIST_ENTRY Next = NULL;

    Next = PfnElement(PageFrameIndex);

    ExpInterlockedPushEntrySList(&FreeSlist, Next);
}

VOID
NTAPI
FreeIndependentPages(
    __in PVOID VirtualAddress,
    __in SIZE_T NumberOfBytes
)
{
    PMMPTE TempPte = NULL;
    PFN_NUMBER NumberOfPtes = 0;
    PFN_NUMBER Index = 0;
    PFN_NUMBER PageFrameIndex = 0;

    TempPte = GetPteAddress(VirtualAddress);
    NumberOfPtes = BYTES_TO_PAGES(NumberOfBytes);

    for (Index = 0;
        Index < NumberOfPtes;
        Index++) {
        PageFrameIndex = GET_PAGE_FRAME_FROM_PTE(&TempPte[Index]);

        PageFrameIndex = IpiSingleCall(
            (PPS_APC_ROUTINE)NULL,
            (PKSYSTEM_ROUTINE)NULL,
            (PUSER_THREAD_START_ROUTINE)_FreePfn,
            (PVOID)PageFrameIndex);

        TempPte[Index].u.Long = 0;
    }

    MmFreeMappingAddress(VirtualAddress, -1);
    FlushMultipleTb(VirtualAddress, NumberOfBytes, TRUE);
}

VOID
NTAPI
LocalVpn(
    __in PVAD_NODE VadNode,
    __out PVAD_LOCAL_NODE VadLocalNode
)
{
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    if (BuildNumber < 9600) {
        VadLocalNode->BalancedRoot.LeftChild = VadNode->AvlRoot.LeftChild;
        VadLocalNode->BalancedRoot.RightChild = VadNode->AvlRoot.RightChild;
        VadLocalNode->BalancedRoot.Parent = VadNode->AvlRoot.Parent;
    }
    else {
        VadLocalNode->BalancedRoot.LeftChild = VadNode->BalancedRoot.LeftChild;
        VadLocalNode->BalancedRoot.RightChild = VadNode->BalancedRoot.RightChild;
        VadLocalNode->BalancedRoot.Parent = VadNode->BalancedRoot.Parent;
    }

    if (BuildNumber < 9200) {
        VadLocalNode->BeginAddress = VadNode->Legacy.StartingVpn << PAGE_SHIFT;
        VadLocalNode->BeginAddress = VadNode->Legacy.EndingVpn << PAGE_SHIFT;
    }
    else if (BuildNumber >= 9200 && BuildNumber < 9600) {
        VadLocalNode->BeginAddress = VadNode->StartingVpn << PAGE_SHIFT;
        VadLocalNode->BeginAddress = VadNode->EndingVpn << PAGE_SHIFT;
    }
    else {
        VadLocalNode->BeginAddress =
            ((((ULONG_PTR)VadNode->StartingVpnHigh) << 32) + VadNode->StartingVpn) << PAGE_SHIFT;

        VadLocalNode->BeginAddress =
            ((((ULONG_PTR)VadNode->EndingVpnHigh) << 32) + VadNode->EndingVpn) << PAGE_SHIFT;
    }
}
