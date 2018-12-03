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

    // A1 6C AE 54 00                   mov eax, ds:_MiZeroPageSlist
    // 8B 4D 08                         mov ecx, [ebp+arg_0]
    // 8D 0C C8                         lea ecx, [eax+ecx*8]
    // 66 83 79 04 00                   cmp word ptr [ecx+4], 0
    // 74 7D                            jz short loc_4EA3F1
    // E8 4F 7A F5 FF                   call @ExInterlockedFlushSList@4
    // 8B F0                            mov esi, eax
    // 85 F6                            test esi, esi
    // 74 72                            jz short loc_4EA3F1
    // 8B C6                            mov eax, esi
    // 2B 05 00 A7 56 00                sub eax, ds:_MmPfnDatabase
    // 6A 1C                            push 1Ch

    CHAR MiDrainZeroLookasidesLegacy[] =
        "a1 ?? ?? ?? ?? 8b 4d 08 8d 0c c8 66 83 79 04 00 74 7d e8 ?? ?? ?? ?? 8b f0 85 f6 74 72 8b c6 2b 05 ?? ?? ?? ?? 6a 1c";

    // A1 1C B4 5E 00                   mov     eax, _MiZeroPageSlist
    // 8D 04 C8                         lea     eax, [eax+ecx*8]
    // 66 39 50 04                      cmp     [eax+4], dx
    // 74 68                            jz      short loc_4CBF82
    // 8B C8                            mov     ecx, eax        ; ListHead
    // E8 DF 41 0A 00                   call    @ExInterlockedFlushSList@4 ; ExInterlockedFlushSList(x)
    // 8B F0                            mov     esi, eax
    // 85 F6                            test    esi, esi
    // 74 56                            jz      short loc_4CBF7D
    // 8B 06                            mov     eax, [esi]
    // 89 45 F0                         mov     [ebp+var_10], eax
    // 8B C6                            mov     eax, esi
    // 2B 05 74 44 61 00                sub     eax, ds:_MmPfnDatabase
    // 6A 1C                            push    1Ch

    CHAR MiDrainZeroLookasides[] =
        "a1 ?? ?? ?? ?? 8d 04 c8 66 39 50 04 74 68 8b c8 e8 ?? ?? ?? ?? 8b f0 85 f6 74 56 8b 06 89 45 ?? 8b c6 2b 05 ?? ?? ?? ?? 6a 1c";

    // 6B F0 14                         imul esi, eax, 14h
    // 03 34 9D 80 24 62 00             add esi, ds:_MmFreePagesByColor[ebx*4]
    // 8B 76 08                         mov esi, [esi+8]

    CHAR MmDuplicateMemory[] =
        "6b ?? 14 03 ?? ?? ?? ?? ?? ?? 8b ?? 08";

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    RtlInitUnicodeString(&RoutineString, L"KeCapturePersistentThreadState");

    ControlPc = MmGetSystemRoutineAddress(&RoutineString);

    if (NULL != ControlPc) {
        while (0 != _CmpByte(ControlPc[0], 0xc2) &&
            0 != _CmpByte(ControlPc[0], 0xc3)) {
            Length = DetourGetInstructionLength(ControlPc);

            if (5 == Length) {
                if (0 == _CmpByte(ControlPc[0], 0xa1)) {
                    PfnDatabase = *(PVOID*)*(PULONG)(ControlPc + Length - sizeof(LONG));

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
            RtlInitUnicodeString(&RoutineString, L"MmAllocateNonCachedMemory");

            ControlPc = MmGetSystemRoutineAddress(&RoutineString);

            if (NULL != ControlPc) {
                while (0 != _CmpByte(ControlPc[0], 0xc2) &&
                    0 != _CmpByte(ControlPc[0], 0xc3)) {
                    Length = DetourGetInstructionLength(ControlPc);

                    if (5 == Length) {
                        if (0xb0 == (ControlPc[0] & 0xf0)) {
                            TargetPc = *(PVOID *)(ControlPc + Length - sizeof(LONG));

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

                if (BuildNumber < 9200) {
                    ControlPc = ScanBytes(
                        SectionBase,
                        (PCHAR)SectionBase + SizeToLock,
                        MiDrainZeroLookasidesLegacy);
                }
                else {
                    ControlPc = ScanBytes(
                        SectionBase,
                        (PCHAR)SectionBase + SizeToLock,
                        MiDrainZeroLookasides);
                }

                if (NULL != ControlPc) {
                    while (0 != _CmpByte(ControlPc[0], 0xc2) &&
                        0 != _CmpByte(ControlPc[0], 0xc3)) {
                        Length = DetourGetInstructionLength(ControlPc);

                        if ((5 == Length && 0 == _CmpByte(ControlPc[0], 0xa1)) ||
                            (6 == Length && 0 == _CmpByte(ControlPc[0], 0x8b))) {
                            if (NULL == FreePageSlist[0]) {
                                TargetPc = *(PVOID *)(ControlPc + Length - sizeof(LONG));

                                FreePageSlist[0] = *(PVOID *)TargetPc;
                            }
                            else if (NULL == FreePageSlist[1]) {
                                TargetPc = *(PVOID *)(ControlPc + Length - sizeof(LONG));

                                FreePageSlist[1] = *(PVOID *)TargetPc;

                                MaximumColor =
                                    max(FreePageSlist[0], FreePageSlist[1]) -
                                    min(FreePageSlist[0], FreePageSlist[1]);

                                break;
                            }
                        }

                        ControlPc += Length;
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
                                TargetPc = *(PVOID *)(ControlPc + 6);

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

VOID
NTAPI
MakeValidKernelPte(
    __in PMMPTE PointerPte,
    __in PFN_NUMBER PageFrameIndex,
    __in ULONG ProtectionMask
)
{
    MMPTE TempPte = { 0 };

    TempPte.u.Long = ProtectToPteMask[ProtectionMask];
    TempPte.u.Hard.Valid = 1;
    TempPte.u.Hard.Accessed = 1;
    TempPte.u.Hard.Global = 1;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPte->u.Long = TempPte.u.Long;
}

VOID
NTAPI
MakeValidKernelPteEx(
    __in PMMPTE_EX PointerPteEx,
    __in PFN_NUMBER PageFrameIndex,
    __in ULONG ProtectionMask
)
{
    MMPTE_EX TempPteEx = { 0 };

    TempPteEx.u.Long = ProtectToPteMask[ProtectionMask];
    TempPteEx.u.Hard.Valid = 1;
    TempPteEx.u.Hard.Accessed = 1;
    TempPteEx.u.Hard.Global = 1;
    TempPteEx.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPteEx->u.Long = TempPteEx.u.Long;
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

    TempPte.u.Long = ProtectToPteMask[ProtectionMask];
    TempPte.u.Hard.Valid = 1;
    TempPte.u.Hard.Owner = 1;
    TempPte.u.Hard.Accessed = 1;
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPte->u.Long = TempPte.u.Long;
}

VOID
NTAPI
MakeValidUserPteEx(
    __in PMMPTE_EX PointerPteEx,
    __in PFN_NUMBER PageFrameIndex,
    __in ULONG ProtectionMask
)
{
    MMPTE_EX TempPteEx = { 0 };

    TempPteEx.u.Long = ProtectToPteMask[ProtectionMask];
    TempPteEx.u.Hard.Valid = 1;
    TempPteEx.u.Hard.Owner = 1;
    TempPteEx.u.Hard.Accessed = 1;
    TempPteEx.u.Hard.PageFrameNumber = PageFrameIndex;

    PointerPteEx->u.Long = TempPteEx.u.Long;
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
        if (FALSE != IsPAEEnable()) {
            Pfn = &PfnDatabase->Pfn7600Ex + PageFrameIndex;
        }
        else {
            Pfn = &PfnDatabase->Pfn7600 + PageFrameIndex;
        }
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
        if (FALSE != IsPAEEnable()) {
            PageFrameIndex = &Pfn->Pfn7600Ex - &PfnDatabase->Pfn7600Ex;
        }
        else {
            PageFrameIndex = &Pfn->Pfn7600 - &PfnDatabase->Pfn7600;
        }
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
            if (FALSE != IsPAEEnable()) {
                Pfn->Pfn7600Ex.PteAddress = PrototypePfn->PointerPte;
                Pfn->Pfn7600Ex.OriginalPte.u.Soft.Protection = PrototypePfn->Protection;
                Pfn->Pfn7600Ex.u3.e1.CacheAttribute = PrototypePfn->CacheAttribute;
                Pfn->Pfn7600Ex.u3.e2.ReferenceCount = PrototypePfn->ReferenceCount;
                Pfn->Pfn7600Ex.u2.ShareCount = PrototypePfn->ShareCount;
                Pfn->Pfn7600Ex.u3.e1.PageLocation = PrototypePfn->PageLocation;
                Pfn->Pfn7600Ex.u3.e1.Modified = PrototypePfn->Modified;
                Pfn->Pfn7600Ex.u4.PteFrame = PrototypePfn->PteFrame;
            }
            else {
                Pfn->Pfn7600.PteAddress = PrototypePfn->PointerPte;
                Pfn->Pfn7600.OriginalPte.u.Soft.Protection = PrototypePfn->Protection;
                Pfn->Pfn7600.u3.e1.CacheAttribute = PrototypePfn->CacheAttribute;
                Pfn->Pfn7600.u3.e2.ReferenceCount = PrototypePfn->ReferenceCount;
                Pfn->Pfn7600.u2.ShareCount = PrototypePfn->ShareCount;
                Pfn->Pfn7600.u3.e1.PageLocation = PrototypePfn->PageLocation;
                Pfn->Pfn7600.u3.e1.Modified = PrototypePfn->Modified;
                Pfn->Pfn7600.u4.PteFrame = PrototypePfn->PteFrame;
            }
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
            if (FALSE != IsPAEEnable()) {
                PrototypePfn->PointerPte = Pfn->Pfn7600Ex.PteAddress;
                PrototypePfn->Protection = Pfn->Pfn7600Ex.OriginalPte.u.Soft.Protection;
                PrototypePfn->CacheAttribute = Pfn->Pfn7600Ex.u3.e1.CacheAttribute;
                PrototypePfn->ReferenceCount = Pfn->Pfn7600Ex.u3.e2.ReferenceCount;
                PrototypePfn->ShareCount = Pfn->Pfn7600Ex.u2.ShareCount;
                PrototypePfn->PageLocation = Pfn->Pfn7600Ex.u3.e1.PageLocation;
                PrototypePfn->Modified = Pfn->Pfn7600Ex.u3.e1.Modified;
                PrototypePfn->PteFrame = Pfn->Pfn7600Ex.u4.PteFrame;
            }
            else {
                PrototypePfn->PointerPte = Pfn->Pfn7600.PteAddress;
                PrototypePfn->Protection = Pfn->Pfn7600.OriginalPte.u.Soft.Protection;
                PrototypePfn->CacheAttribute = Pfn->Pfn7600.u3.e1.CacheAttribute;
                PrototypePfn->ReferenceCount = Pfn->Pfn7600.u3.e2.ReferenceCount;
                PrototypePfn->ShareCount = Pfn->Pfn7600.u2.ShareCount;
                PrototypePfn->PageLocation = Pfn->Pfn7600.u3.e1.PageLocation;
                PrototypePfn->Modified = Pfn->Pfn7600.u3.e1.Modified;
                PrototypePfn->PteFrame = Pfn->Pfn7600.u4.PteFrame;
            }
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
    else {
        RtlZeroMemory(PrototypePfn, sizeof(PROTOTYPE_PFN));
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
        TempPfn = InterlockedPopEntrySList(&FreeSlist);

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
                    TempPfn = InterlockedPopEntrySList(&FreePageSlist[SlistIndex][Color]);

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
    PMMPTE PointerPte = NULL;
    PMMPTE_EX PointerPteEx = NULL;
    MMPTE NewPteContents = { 0 };
    MMPTE_EX NewPteContentsEx = { 0 };
    PROTOTYPE_PFN PrototypePfn = { 0 };
    ULONG BuildNumber = 0;

    PsGetVersion(NULL, NULL, &BuildNumber, NULL);

    if (FALSE != IsPAEEnable()) {
        PointerPteEx = GetPteAddress(VirtualAddress);
    }
    else {
        PointerPte = GetPteAddress(VirtualAddress);
    }

    NumberOfBytes = BYTE_OFFSET(VirtualAddress) + NumberOfBytes;
    NumberOfPages = BYTES_TO_PAGES(NumberOfBytes);

    ASSERT(0 != NumberOfPages);

    if ((ULONG_PTR)VirtualAddress > (ULONG_PTR)MM_HIGHEST_USER_ADDRESS) {
        if (FALSE != IsPAEEnable()) {
            MakeValidKernelPteEx(&NewPteContentsEx, 0, ProtectionMask);
        }
        else {
            MakeValidKernelPte(&NewPteContents, 0, ProtectionMask);
        }
    }
    else {
        if (FALSE != IsPAEEnable()) {
            MakeValidUserPteEx(&NewPteContentsEx, 0, ProtectionMask);
        }
        else {
            MakeValidUserPte(&NewPteContents, 0, ProtectionMask);
        }
    }

    if (FALSE != IsPAEEnable()) {
        for (PageFrameIndex = 0;
            PageFrameIndex < NumberOfPages;
            PageFrameIndex += 1) {
            NewPteContentsEx.u.Hard.PageFrameNumber = PointerPteEx->u.Hard.PageFrameNumber;
            NewPteContentsEx.u.Hard.Dirty = PointerPteEx->u.Hard.Dirty;
            NewPteContentsEx.u.Hard.SoftwareWsIndex = PointerPteEx->u.Hard.SoftwareWsIndex;

            PointerPteEx->u.Long = NewPteContentsEx.u.Long;

            LocalPfn(
                PointerPteEx->u.Hard.PageFrameNumber,
                &PrototypePfn);

            PrototypePfn.Protection = ProtectionMask;

            PrototypePfn.CacheAttribute =
                MM_PTE_NOCACHE == (ProtectToPteMask[ProtectionMask] & MM_PTE_NOCACHE) ?
                MiNonCached : MiCached;

            InitializePfn(
                PointerPteEx->u.Hard.PageFrameNumber,
                &PrototypePfn);

            PointerPteEx += 1;
        }
    }
    else {
        for (PageFrameIndex = 0;
            PageFrameIndex < NumberOfPages;
            PageFrameIndex += 1) {
            NewPteContents.u.Hard.PageFrameNumber = PointerPte->u.Hard.PageFrameNumber;
            NewPteContents.u.Hard.Dirty = PointerPte->u.Hard.Dirty;

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

            PointerPte += 1;
        }
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
    PMMPTE_EX TempPteEx = NULL;
    PVOID AllocatedAddress = NULL;
    PFN_NUMBER NumberOfPtes = 0;
    PFN_NUMBER Index = 0;
    PFN_NUMBER LargePageBaseFrame = 0;
    PFN_NUMBER PageFrameIndex = 0;

    NumberOfPtes = BYTES_TO_PAGES(NumberOfBytes);
    AllocatedAddress = MmAllocateMappingAddress(NumberOfBytes, -1);

    if (NULL != AllocatedAddress) {
        if (FALSE != IsPAEEnable()) {
            TempPteEx = GetPteAddress(AllocatedAddress);

            for (Index = 0;
                Index < NumberOfPtes;
                Index++) {
                PageFrameIndex = RemoveAnyPage(&TempPteEx[Index]);

                MakeValidKernelPteEx(&TempPteEx[Index], PageFrameIndex, MM_READWRITE);

                SetPageProtection(
                    GetVirtualAddressMappedByPte(&TempPteEx[Index]),
                    PAGE_SIZE,
                    MM_READWRITE);
            }
        }
        else {
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

    InterlockedPushEntrySList(&FreeSlist, Next);
}

VOID
NTAPI
FreeIndependentPages(
    __in PVOID VirtualAddress,
    __in SIZE_T NumberOfBytes
)
{
    PMMPTE TempPte = NULL;
    PMMPTE_EX TempPteEx = NULL;
    PFN_NUMBER NumberOfPtes = 0;
    PFN_NUMBER Index = 0;
    PFN_NUMBER PageFrameIndex = 0;

    NumberOfPtes = BYTES_TO_PAGES(NumberOfBytes);

    if (FALSE != IsPAEEnable()) {
        TempPteEx = GetPteAddress(VirtualAddress);

        for (Index = 0;
            Index < NumberOfPtes;
            Index++) {
            PageFrameIndex = GET_PAGE_FRAME_FROM_PTE(&TempPteEx[Index]);

            PageFrameIndex = IpiSingleCall(
                (PPS_APC_ROUTINE)NULL,
                (PKSYSTEM_ROUTINE)NULL,
                (PUSER_THREAD_START_ROUTINE)_FreePfn,
                (PVOID)PageFrameIndex);

            TempPteEx[Index].u.Long = 0;
        }
    }
    else {
        TempPte = GetPteAddress(VirtualAddress);

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

    VadLocalNode->BeginAddress = VadNode->Legacy.StartingVpn << PAGE_SHIFT;
    VadLocalNode->BeginAddress = VadNode->Legacy.EndingVpn << PAGE_SHIFT;
}
