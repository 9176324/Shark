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

PMMPTE SystemPtesIndex;

PIDLE_PFN IdlePfnSlist[MAXIMUM_SLIST_COUNT];
ULONG IdlePfnSlistCount[MAXIMUM_SLIST_COUNT];

BOOLEAN
NTAPI
CollideIdlePfnSlist(
    __in PIDLE_PFN IdlePfnSlistBegin
)
{
    BOOLEAN Result = FALSE;
    ULONG Index = 0;
    PIDLE_PFN TempIdlePfnSlist = NULL;
    ULONG ZeroCount = 0;

    TempIdlePfnSlist = IdlePfnSlistBegin;

    for (Index = 0;
        Index < MINIMUM_IDLE_PFN_COUNT;
        Index++, TempIdlePfnSlist++) {
        if (FALSE == MmIsAddressValid(TempIdlePfnSlist) ||
            FALSE == MmIsAddressValid(TempIdlePfnSlist + 1)) {
            break;
        }

        if (0 != TempIdlePfnSlist->Reserved) {
            break;
        }

        if (TempIdlePfnSlist->Count >= 4) {
            break;
        }

        if (0 == TempIdlePfnSlist->Color) {
            ZeroCount++;
        }

        if (IDLE_PFN_NULL != (ULONG_PTR)TempIdlePfnSlist->Next &&
            IDLE_PFN_FREE != (ULONG_PTR)TempIdlePfnSlist->Next) {
            if ((FALSE == MmIsAddressValid(
                (PVOID)((ULONG_PTR)TempIdlePfnSlist->Next & (~(ULONG_PTR)IDLE_PFN_FREE))))) {
                break;
            }
        }

        if (Index == (MINIMUM_IDLE_PFN_COUNT - 1)) {
            if (ZeroCount >= (MINIMUM_IDLE_PFN_COUNT - 1)) {
                break;
            }

            Result = TRUE;
        }
    }

    return Result;
}

VOID
NTAPI
CollideIdlePfnSlistEnd(
    __inout PIDLE_PFN * IdlePfnSlistEnd
)
{
    BOOLEAN Result = FALSE;
    ULONG Index = 0;
    PIDLE_PFN TempIdlePfnSlist = NULL;

    TempIdlePfnSlist = *IdlePfnSlistEnd;

    for (Index = 0;
        Index < MAXIMUM_IDLE_PFN_COUNT;
        Index++, TempIdlePfnSlist++) {
        if (FALSE == MmIsAddressValid(TempIdlePfnSlist) ||
            FALSE == MmIsAddressValid(TempIdlePfnSlist + 1)) {
            break;
        }

        if (0 != TempIdlePfnSlist->Reserved) {
            break;
        }

        if (TempIdlePfnSlist->Count >= 4) {
            break;
        }

        if (IDLE_PFN_NULL != (ULONG_PTR)TempIdlePfnSlist->Next &&
            IDLE_PFN_FREE != (ULONG_PTR)TempIdlePfnSlist->Next) {
            if ((FALSE == MmIsAddressValid(
                (PVOID)((ULONG_PTR)TempIdlePfnSlist->Next & (~(ULONG_PTR)IDLE_PFN_FREE))))) {
                break;
            }
        }
    }

    *IdlePfnSlistEnd = TempIdlePfnSlist;
}

VOID
NTAPI
InitializeSpace(
    __in PVOID Parameter
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PCHAR SectionBase = NULL;
    ULONG SizeToLock = 0;
    UNICODE_STRING ImageFileName = { 0 };
    PLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ULONG BuildNumber = 0;
    PCHAR ControlPc = NULL;
    UNICODE_STRING RoutineString = { 0 };
    ULONG Length = 0;
    USHORT Index = 0;
    PIDLE_PFN * IdlePfnSlistPointer = NULL;
    PIDLE_PFN TempIdlePfnSlist = NULL;
    PPHYSICAL_MEMORY_RANGE PhysicalMemoryRange = NULL;

    // 48 C1 E9 09                      shr     rcx, 9
    // 48 B8 F8 FF FF FF 7F 00 00 00    mov     rax, 7FFFFFFFF8h
    // 48 23 C8                         and     rcx, rax
    // 48 B8 00 00 00 00 80 F6 FF FF    mov     rax, 0FFFFF68000000000h
    // 48 03 C1                         add     rax, rcx
    // C3                               retn

    CHAR MiGetPteAddress[] =
        "48 c1 e9 09 48 b8 f8 ff ff ff 7f 00 00 00 48 23 c8 48 b8 ?? ?? ?? ?? ?? ?? ?? ?? 48 03 c1 c3";

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
                    PfnDatabase = *(PVOID *)RvaToVa(ControlPc + 3);

                    break;
                }
            }

            ControlPc += Length;
        }
    }

    PhysicalMemoryRange = MmGetPhysicalMemoryRanges();

    if (NULL != PhysicalMemoryRange) {
        while (0 != PhysicalMemoryRange->NumberOfBytes.QuadPart) {
            PfnAllocation += PhysicalMemoryRange->NumberOfBytes.QuadPart / PAGE_SIZE;

            PhysicalMemoryRange++;
        }
    }

    RtlInitUnicodeString(&ImageFileName, L"ntoskrnl.exe");

    if (RTL_SOFT_ASSERT(NT_SUCCESS(FindEntryForKernelImage(
        &ImageFileName,
        &DataTableEntry)))) {
        SystemPtesIndex = GetPteAddress(
            (PVOID)((ULONG_PTR)DataTableEntry->DllBase + DataTableEntry->SizeOfImage));

        NtHeaders = RtlImageNtHeader(DataTableEntry->DllBase);

        if (NULL != NtHeaders) {
            NtSection = IMAGE_FIRST_SECTION(NtHeaders);
            for (Index = 0;
                Index < NtHeaders->FileHeader.NumberOfSections;
                Index++) {
                if (0 == _CmpByte(NtSection[Index].Name[0], '.') &&
                    (0 == _CmpByte(NtSection[Index].Name[1], 'd') || 0 == _CmpByte(NtSection[Index].Name[1], 'D')) &&
                    (0 == _CmpByte(NtSection[Index].Name[2], 'a') || 0 == _CmpByte(NtSection[Index].Name[2], 'A')) &&
                    (0 == _CmpByte(NtSection[Index].Name[3], 't') || 0 == _CmpByte(NtSection[Index].Name[3], 'T')) &&
                    (0 == _CmpByte(NtSection[Index].Name[4], 'a') || 0 == _CmpByte(NtSection[Index].Name[4], 'A'))) {
                    if (0 != NtSection[Index].VirtualAddress) {
                        SectionBase = (PCHAR)DataTableEntry->DllBase + NtSection[Index].VirtualAddress;

                        SizeToLock = max(
                            NtSection[Index].SizeOfRawData,
                            NtSection[Index].Misc.VirtualSize);

                        for (IdlePfnSlistPointer = (PIDLE_PFN *)SectionBase;
                            (ULONG_PTR)IdlePfnSlistPointer < (ULONG_PTR)SectionBase + SizeToLock;
                            IdlePfnSlistPointer++) {
                            if (FALSE != CollideIdlePfnSlist(*IdlePfnSlistPointer)) {
                                if (NULL == IdlePfnSlist[0]) {
                                    IdlePfnSlist[0] = *IdlePfnSlistPointer;

                                    TempIdlePfnSlist = IdlePfnSlist[0];

                                    CollideIdlePfnSlistEnd(&TempIdlePfnSlist);

                                    IdlePfnSlistCount[0] = (TempIdlePfnSlist - IdlePfnSlist[0]) & ~0xf;
                                }
                                else if (NULL == IdlePfnSlist[1]) {
                                    IdlePfnSlist[1] = *IdlePfnSlistPointer;

                                    TempIdlePfnSlist = IdlePfnSlist[1];

                                    CollideIdlePfnSlistEnd(&TempIdlePfnSlist);

                                    IdlePfnSlistCount[1] = (TempIdlePfnSlist - IdlePfnSlist[1]) & ~0xf;

                                    break;
                                }
                            }
                        }

                        break;
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
    __in PMMPTE PointerPte,
    __in PMMPTE NewPteContents
)
{
    PPFN Pfn = NULL;
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    Pfn = PfnElement(PageFrameIndex);

    if (BuildNumber < 9200) {
        RtlZeroMemory(&Pfn->Pfn7600, sizeof(Pfn->Pfn7600));

        Pfn->Pfn7600.PteAddress = PointerPte;

        if (NewPteContents->u.Hard.NoExecute == 0) {
            Pfn->Pfn7600.OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
        }
        else {
            Pfn->Pfn7600.OriginalPte.u.Soft.Protection = MM_READWRITE;
        }

        Pfn->Pfn7600.u3.e1.CacheAttribute = MiCached;
        Pfn->Pfn7600.u3.e2.ReferenceCount = 1;
        Pfn->Pfn7600.u2.ShareCount = 1;
        Pfn->Pfn7600.u3.e1.PageLocation = ActiveAndValid;
        Pfn->Pfn7600.u3.e1.Modified = 1;

        Pfn->Pfn7600.u4.PteFrame = GetPteAddress(Pfn)->u.Hard.PageFrameNumber;
    }
    else if (BuildNumber >= 9200 && BuildNumber < 9600) {
        RtlZeroMemory(&Pfn->Pfn9200, sizeof(Pfn->Pfn9200));

        Pfn->Pfn9200.PteAddress = PointerPte;

        if (NewPteContents->u.Hard.NoExecute == 0) {
            Pfn->Pfn9200.OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
        }
        else {
            Pfn->Pfn9200.OriginalPte.u.Soft.Protection = MM_READWRITE;
        }

        Pfn->Pfn9200.u3.e1.CacheAttribute = MiCached;
        Pfn->Pfn9200.u3.e2.ReferenceCount = 1;
        Pfn->Pfn9200.u2.ShareCount = 1;
        Pfn->Pfn9200.u3.e1.PageLocation = ActiveAndValid;
        Pfn->Pfn9200.u3.e1.Modified = 1;

        Pfn->Pfn9200.u4.PteFrame = GetPteAddress(Pfn)->u.Hard.PageFrameNumber;
    }
    else if (BuildNumber >= 9600 && BuildNumber < 10240) {
        RtlZeroMemory(&Pfn->Pfn9600, sizeof(Pfn->Pfn9600));

        Pfn->Pfn9600.PteAddress = PointerPte;

        if (NewPteContents->u.Hard.NoExecute == 0) {
            Pfn->Pfn9600.OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
        }
        else {
            Pfn->Pfn9600.OriginalPte.u.Soft.Protection = MM_READWRITE;
        }

        Pfn->Pfn9600.u3.e1.CacheAttribute = MiCached;
        Pfn->Pfn9600.u3.e2.ReferenceCount = 1;
        Pfn->Pfn9600.u2.ShareCount = 1;
        Pfn->Pfn9600.u3.e1.PageLocation = ActiveAndValid;
        Pfn->Pfn9600.u3.e1.Modified = 1;

        Pfn->Pfn9600.u4.PteFrame = GetPteAddress(Pfn)->u.Hard.PageFrameNumber;
    }
    else {
        RtlZeroMemory(&Pfn->Pfn10240, sizeof(Pfn->Pfn10240));

        Pfn->Pfn10240.PteAddress = PointerPte;

        if (NewPteContents->u.Hard.NoExecute == 0) {
            Pfn->Pfn10240.OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
        }
        else {
            Pfn->Pfn10240.OriginalPte.u.Soft.Protection = MM_READWRITE;
        }

        Pfn->Pfn10240.u3.e1.CacheAttribute = MiCached;
        Pfn->Pfn10240.u3.e2.ReferenceCount = 1;
        Pfn->Pfn10240.u2.ShareCount = 1;
        Pfn->Pfn10240.u3.e1.PageLocation = ActiveAndValid;
        Pfn->Pfn10240.u3.e1.Modified = 1;

        Pfn->Pfn10240.u4.PteFrame = GetPteAddress(Pfn)->u.Hard.PageFrameNumber;
    }
}

PFN_NUMBER
NTAPI
_RemoveAnyPage(
    __in PMMPTE PointerPte
)
{
    PPFN Pfn = NULL;
    ULONG StorageIndex = 0;
    ULONG SlistIndex = 0;
    ULONG Index = 0;
    PFN_NUMBER PageFrameIndex = 0;
    MMPTE TempPte = { 0 };
    PIDLE_PFN TempIdlePfnSlist = NULL;
    ULONG_PTR TempAddress = 0;

    if (NULL == IdlePfnSlist[0] &&
        NULL == IdlePfnSlist[1]) {
        __debugbreak();
    }

    for (StorageIndex = 0;
        StorageIndex < MINIMUM_STORAGE_COUNT;
        StorageIndex++) {
        for (SlistIndex = 0;
            SlistIndex < MAXIMUM_SLIST_COUNT;
            SlistIndex++) {
            TempIdlePfnSlist = IdlePfnSlist[SlistIndex];

            for (Index = 0;
                Index < IdlePfnSlistCount[SlistIndex];
                Index++) {
                if (TempIdlePfnSlist[Index].Count >(MINIMUM_STORAGE_COUNT - StorageIndex - 1)) {
                    TempAddress = (ULONG_PTR)TempIdlePfnSlist[Index].Next;
                    Pfn = (PPFN)(TempAddress & (~(ULONG_PTR)IDLE_PFN_FREE));
                    TempAddress = *(PULONG_PTR)Pfn | (TempAddress & IDLE_PFN_FREE);
                    TempIdlePfnSlist[Index].Next = (PSINGLE_LIST_ENTRY)TempAddress;
                    TempIdlePfnSlist[Index].Count--;

                    goto found;
                }
            }
        }
    }

found:
    if (NULL != Pfn) {
        PageFrameIndex = PfnElementToIndex(Pfn);

        MakeValidKernelPte(&TempPte, PageFrameIndex, MM_READWRITE);
        InitializePfn(PageFrameIndex, PointerPte, &TempPte);
    }

    return PageFrameIndex;
}

PFN_NUMBER
NTAPI
RemoveAnyPage(
    __in PMMPTE PointerPte
)
{
    PFN_NUMBER PageFrameIndex = 0;
    PVOID VirtualAddress = NULL;
    LARGE_INTEGER Interval = { 0 };

    do {
        PageFrameIndex = IpiSingleCall(
            (PPS_APC_ROUTINE)NULL,
            (PKSYSTEM_ROUTINE)NULL,
            (PUSER_THREAD_START_ROUTINE)_RemoveAnyPage,
            (PVOID)PointerPte);

        Interval.QuadPart = Int32x32To64(10, -10 * 1000);

        KeDelayExecutionThread(KernelMode, FALSE, &Interval);
    } while (0 == PageFrameIndex);

    if (RTL_SOFT_ASSERT(0 != PageFrameIndex)) {
        VirtualAddress = GetVirtualAddressMappedByPte(PointerPte);

        if ((ULONG_PTR)VirtualAddress > (ULONG_PTR)MM_HIGHEST_USER_ADDRESS) {
            MakeValidKernelPte(PointerPte, PageFrameIndex, MM_READWRITE);
        }
        else {
            MakeValidUserPte(PointerPte, PageFrameIndex, MM_READWRITE);
        }

        FlushMultipleTb(VirtualAddress, PAGE_SIZE, TRUE);

        RtlZeroMemory(VirtualAddress, PAGE_SIZE);
    }

    return PageFrameIndex;
}

VOID
NTAPI
MakePdeExistAndMakeValid(
    __in PMMPTE PointerPde
)
{
    PFN_NUMBER PfnIndex = 0;
    PMMPTE PointerPxe = NULL;
    PMMPTE PointerPpe = NULL;
    PVOID VirtualAddress = NULL;

    VirtualAddress = GetVirtualAddressMappedByPde(PointerPde);

    PointerPxe = GetPxeAddress(VirtualAddress);

    if (MM_PTE_VALID_MASK != PointerPxe->u.Hard.Valid) {
        PfnIndex = RemoveAnyPage(PointerPxe);

        MakeValidKernelPte(PointerPxe, PfnIndex, MM_READWRITE);
    }

    PointerPpe = GetPpeAddress(VirtualAddress);

    if (MM_PTE_VALID_MASK != PointerPpe->u.Hard.Valid) {
        PfnIndex = RemoveAnyPage(PointerPpe);

        MakeValidKernelPte(PointerPpe, PfnIndex, MM_READWRITE);
    }

    if (MM_PTE_VALID_MASK != PointerPde->u.Hard.Valid) {
        PfnIndex = RemoveAnyPage(PointerPde);

        MakeValidKernelPte(PointerPde, PfnIndex, MM_READWRITE);
    }
}

VOID
NTAPI
ReservePtes(
    __in ULONG NumberOfPtes,
    __out PMMPTE * ReservePte
)
{
    PMMPTE PointerPde = NULL;
    PMMPTE PointerPte = NULL;
    PMMPTE BeginPte = NULL;
    ULONG Index = 0;
    PVOID VirtualAddress = NULL;
    PFN_NUMBER PfnIndex = 0;
    PFN_NUMBER TotalFreePages = 0;

    BeginPte = NULL == *ReservePte ? SystemPtesIndex : *ReservePte;

    TotalFreePages = 0;

    do {
        VirtualAddress = GetVirtualAddressMappedByPte(BeginPte);

        TotalFreePages = 0;

        for (Index = 0;
            Index < NumberOfPtes;
            Index++) {
            if (FALSE == MmIsAddressValid((PCHAR)VirtualAddress + PAGE_SIZE * Index)) {
                TotalFreePages++;
            }
            else {
                BeginPte += Index + 1;
                break;
            }
        }
    } while (TotalFreePages != NumberOfPtes);

    PointerPte = BeginPte;
    SystemPtesIndex += BeginPte == SystemPtesIndex ? NumberOfPtes : 0;

    for (Index = 0;
        Index < NumberOfPtes;
        Index++) {
        VirtualAddress = GetVirtualAddressMappedByPte(&PointerPte[Index]);
        PointerPde = GetPdeAddress(VirtualAddress);

        MakePdeExistAndMakeValid(PointerPde);

        PfnIndex = RemoveAnyPage(&PointerPte[Index]);

        if ((ULONG_PTR)VirtualAddress > (ULONG_PTR)MM_HIGHEST_USER_ADDRESS) {
            MakeValidKernelPte(&PointerPte[Index], PfnIndex, MM_READWRITE);
        }
        else {
            MakeValidUserPte(&PointerPte[Index], PfnIndex, MM_READWRITE);
        }

        SET_PTE_DIRTY(&PointerPte[Index]);
    }

    *ReservePte = PointerPte;
}

PVOID
NTAPI
AllocateIndependentPages(
    __in PVOID VirtualAddress,
    __in SIZE_T NumberOfBytes
)
{
    PMMPTE PointerPte = NULL;
    PVOID AllocatedAddress = NULL;
    PFN_NUMBER NumberOfPages = 0;

    if (NULL != VirtualAddress) {
        PointerPte = GetPteAddress(VirtualAddress);
    }

    NumberOfPages = BYTES_TO_PAGES(NumberOfBytes);

    ReservePtes(NumberOfPages, &PointerPte);

    AllocatedAddress = GetVirtualAddressMappedByPte(PointerPte);

    FlushMultipleTb(VirtualAddress, NumberOfBytes, TRUE);

    return AllocatedAddress;
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
    MMPTE TempPte = { 0 };
    MMPTE NewPteContents = { 0 };
    PPFN Pfn = NULL;
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    PointerPte = GetPteAddress(VirtualAddress);
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
        TempPte = *PointerPte;

        NewPteContents.u.Hard.PageFrameNumber = TempPte.u.Hard.PageFrameNumber;
        NewPteContents.u.Hard.Dirty = TempPte.u.Hard.Dirty;

        Pfn = PfnElement(TempPte.u.Hard.PageFrameNumber);

        if (BuildNumber < 9200) {
            Pfn->Pfn7600.OriginalPte.u.Soft.Protection = ProtectionMask;
        }
        else if (BuildNumber >= 9200 && BuildNumber < 9600) {
            Pfn->Pfn9200.OriginalPte.u.Soft.Protection = ProtectionMask;
        }
        else if (BuildNumber >= 9600 && BuildNumber < 10240) {
            Pfn->Pfn9600.OriginalPte.u.Soft.Protection = ProtectionMask;
        }
        else {
            Pfn->Pfn10240.OriginalPte.u.Soft.Protection = ProtectionMask;
        }

        PointerPte->u.Long = NewPteContents.u.Long;
        PointerPte++;
    }

    FlushMultipleTb(VirtualAddress, NumberOfBytes, TRUE);
}

VOID
NTAPI
DeletePfn(
    __in PFN_NUMBER Page
)
{
    PPFN Pfn = NULL;
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    Pfn = PfnElement(Page);

    if (BuildNumber < 9200) {
        RtlZeroMemory(&Pfn->Pfn7600, sizeof(Pfn->Pfn7600));
    }
    else if (BuildNumber >= 9200 && BuildNumber < 9600) {
        RtlZeroMemory(&Pfn->Pfn9200, sizeof(Pfn->Pfn9200));
    }
    else if (BuildNumber >= 9600 && BuildNumber < 10240) {
        RtlZeroMemory(&Pfn->Pfn9600, sizeof(Pfn->Pfn9600));
    }
    else {
        RtlZeroMemory(&Pfn->Pfn10240, sizeof(Pfn->Pfn10240));
    }
}

VOID
NTAPI
ReleasePtes(
    __in PMMPTE PointerPte,
    __in ULONG NumberOfPtes
)
{
    PMMPTE TempPte = NULL;
    PFN_NUMBER Index = 0;
    PFN_NUMBER PageFrameIndex = 0;
    PVOID VirtualAddress = NULL;

    VirtualAddress = GetVirtualAddressMappedByPte(PointerPte);

    for (Index = 0;
        Index < NumberOfPtes;
        Index++) {
        if (FALSE != MmIsAddressValid((PCHAR)VirtualAddress + PAGE_SIZE *Index)) {
            TempPte = GetPteAddress((PCHAR)VirtualAddress + PAGE_SIZE *Index);

            PageFrameIndex = GET_PAGE_FRAME_FROM_PTE(TempPte);

            RtlZeroMemory(TempPte, sizeof(MMPTE));
        }

        IpiSingleCall(
            (PPS_APC_ROUTINE)NULL,
            (PKSYSTEM_ROUTINE)NULL,
            (PUSER_THREAD_START_ROUTINE)DeletePfn,
            (PVOID)PageFrameIndex);
    }
}

VOID
NTAPI
FreeIndependentPages(
    __in PVOID VirtualAddress,
    __in SIZE_T NumberOfBytes
)
{
    PMMPTE PointerPte = NULL;
    PFN_NUMBER NumberOfPages = 0;

    PointerPte = GetPteAddress(VirtualAddress);
    NumberOfPages = BYTES_TO_PAGES(NumberOfBytes);

    ReleasePtes(PointerPte, NumberOfPages);

    FlushMultipleTb(VirtualAddress, NumberOfBytes, TRUE);
}
