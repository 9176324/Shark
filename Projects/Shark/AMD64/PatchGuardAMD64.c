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

#include "PatchGuard.h"

#include "Ctx.h"
#include "Jump.h"
#include "Scan.h"
#include "Stack.h"

#define __ROL64(x, n) (((x) << ((n % 64))) | ((x) >> (64 - (n % 64))))
#define __ROR64(x, n) (((x) >> ((n % 64))) | ((x) << (64 - (n % 64))))

ULONG64
NTAPI
_btc64(
    __in ULONG64 a,
    __in ULONG64 b
);

VOID
NTAPI
_MakePgFire(
    VOID
);

PVOID
NTAPI
_ClearEncryptedContext(
    __in PVOID Reserved,
    __in PVOID PatchGuardContext
);

VOID
NTAPI
_RevertWorkerToSelf(
    VOID
);

VOID
NTAPI
_GuardCall(
    VOID
);

VOID
NTAPI
_CheckPatchGuardCode(
    __in PVOID BaseAddress,
    __in SIZE_T RegionSize
);

#undef MiGetPxeAddress

PMMPTE
NTAPI
MiGetPxeAddress(
    __in PMMPTE PxeBase,
    __in PVOID VirtualAddress
)
{
    RTL_SOFT_ASSERT(NULL != PxeBase);

    return PxeBase + MiGetPxeOffset(VirtualAddress);
}

#undef MiGetPpeAddress

PMMPTE
NTAPI
MiGetPpeAddress(
    __in PMMPTE PpeBase,
    __in PVOID VirtualAddress
)
{
    RTL_SOFT_ASSERT(NULL != PpeBase);

    return (PMMPTE)
        (((((LONGLONG)VirtualAddress &
            VIRTUAL_ADDRESS_MASK) >> PPI_SHIFT) << PTE_SHIFT) + (LONGLONG)PpeBase);
}

#undef MiGetPdeAddress

PMMPTE
NTAPI
MiGetPdeAddress(
    __in PMMPTE PdeBase,
    __in PVOID VirtualAddress
)
{
    RTL_SOFT_ASSERT(NULL != PdeBase);

    return (PMMPTE)
        (((((LONGLONG)VirtualAddress &
            VIRTUAL_ADDRESS_MASK) >> PDI_SHIFT) << PTE_SHIFT) + (LONGLONG)PdeBase);
}

#undef MiGetPteAddress

PMMPTE
NTAPI
MiGetPteAddress(
    __in PMMPTE PteBase,
    __in PVOID VirtualAddress
)
{
    RTL_SOFT_ASSERT(NULL != PteBase);

    return (PMMPTE)
        (((((LONGLONG)VirtualAddress &
            VIRTUAL_ADDRESS_MASK) >> PTI_SHIFT) << PTE_SHIFT) + (LONGLONG)PteBase);
}

#undef MiGetVirtualAddressMappedByPte

PVOID
NTAPI
MiGetVirtualAddressMappedByPte(
    __in PMMPTE PteBase,
    __in PMMPTE Pte
)
{
    RTL_SOFT_ASSERT(NULL != PteBase);

    return (PVOID)((((LONGLONG)Pte - (LONGLONG)PteBase) <<
        (PAGE_SHIFT + VA_SHIFT - PTE_SHIFT)) >> VA_SHIFT);
}

#undef MiGetVirtualAddressMappedByPde

#define MiGetVirtualAddressMappedByPde(PteBase, Pde) \
    MiGetVirtualAddressMappedByPte(PteBase, MiGetVirtualAddressMappedByPte(PteBase, Pde))

#undef MiGetVirtualAddressMappedByPpe

#define MiGetVirtualAddressMappedByPpe(PteBase, PdeBase, Ppe) \
    MiGetVirtualAddressMappedByPte(PteBase, MiGetVirtualAddressMappedByPde(PdeBase, Ppe))

#undef MiGetVirtualAddressMappedByPxe

#define MiGetVirtualAddressMappedByPxe(PdeBase, Pxe) \
    MiGetVirtualAddressMappedByPde(PdeBase, MiGetVirtualAddressMappedByPde(PdeBase, Pxe))

VOID
NTAPI
InitializePatchGuardBlock(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE FileHandle = NULL;
    HANDLE SectionHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING ImageFileName = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    PVOID ViewBase = NULL;
    SIZE_T ViewSize = 0;
    PCHAR ControlPc = NULL;
    PCHAR TargetPc = NULL;
    ULONG Length = 0;
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PVOID EndToLock = 0;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    LONG64 Diff = 0;

    PKLDR_DATA_TABLE_ENTRY KernelDataTableEntry = NULL;
    UNICODE_STRING KernelString = { 0 };

    PPOOL_BIG_PAGES * PageTable = NULL;
    PSIZE_T PageTableSize = NULL;

    CHAR CmpAppendDllSection[] = "2e 48 31 11 48 31 51 08 48 31 51 10 48 31 51 18";
    CHAR Field[] = "fb 48 8d 05";
    CHAR FirstField[] = "?? 89 ?? 00 01 00 00 48 8D 05 ?? ?? ?? ?? ?? 89 ?? 08 01 00 00";
    CHAR PatchGuardEntry[] = "48 81 ec c0 02 00 00 48 8d a8 d8 fd ff ff 48 83 e5 80";
    CHAR KiStartSystemThread[] = "b9 01 00 00 00 44 0f 22 c1 48 8b 14 24 48 8b 4c 24 08";
    CHAR PspSystemThreadStartup[] = "eb ?? b9 1e 00 00 00 e8";

    // if os build > win10 (10586) PteBase PdeBase PpeBase and PxeBase is random;
    CHAR MiGetPteAddress[] =
        "48 c1 e9 09 48 b8 f8 ff ff ff 7f 00 00 00 48 23 c8 48 b8 ?? ?? ?? ?? ?? ?? ?? ?? 48 03 c1 c3";

    CHAR MiDeterminePoolType[] = "48 8b ?? e8";

    // check call MiDeterminePoolType
    CHAR CheckMiDeterminePoolType[] = "ff 0f 00 00 0f 85 ?? ?? ?? ?? 48 8b ?? e8";

    // 55                                                              push    rbp
    // 41 54                                                           push    r12
    // 41 55                                                           push    r13
    // 41 56                                                           push    r14
    // 41 57                                                           push    r15
    // 48 81 EC C0 02 00 00                                            sub     rsp, 2C0h
    // 48 8D A8 D8 FD FF FF                                            lea     rbp, [rax - 228h]
    // 48 83 E5 80 and rbp, 0FFFFFFFFFFFFFF80h

    CHAR EntryPoint[] = "55 41 54 41 55 41 56 41 57 48 81 EC C0 02 00 00 48 8D A8 D8 FD FF FF 48 83 E5 80";

    CHAR SdbpCheckDll[] = {
        0x48, 0x8b, 0x74, 0x24, 0x30, 0x48, 0x8b, 0x7c,
        0x24, 0x28, 0x4c, 0x8b, 0x54, 0x24, 0x38, 0x33,
        0xc0, 0x49, 0x89, 0x02, 0x49, 0x83, 0xea, 0x08,
        0x4c, 0x3b, 0xd4, 0x73, 0xf4, 0x48, 0x89, 0x7c,
        0x24, 0x28, 0x8b, 0xd8, 0x8b, 0xf8, 0x8b, 0xe8,
        0x4c, 0x8b, 0xd0, 0x4c, 0x8b, 0xd8, 0x4c, 0x8b,
        0xe0, 0x4c, 0x8b, 0xe8, 0x4c, 0x8b, 0xf0, 0x4c,
        0x8b, 0xf8, 0xff, 0xe6
    };

    CHAR ClearEncryptedContextMessage[] =
        "Shark - < %p > PatchGuard encrypted context cleared reference count\n";

    CHAR RevertWorkerToSelfMessage[] =
        "Shark - < %p > PatchGuard declassified context cleared reference count\n";

#ifndef PUBLIC
    DbgPrint(
        "Shark - < %p > PatchGuard clear context PatchGuardBlock\n",
        PatchGuardBlock);
#endif // !PUBLIC

    PsGetVersion(
        NULL,
        NULL,
        &PatchGuardBlock->BuildNumber,
        NULL);

    RtlInitUnicodeString(&KernelString, L"ntoskrnl.exe");

    Status = FindEntryForKernelImage(
        &KernelString,
        &PatchGuardBlock->KernelDataTableEntry);

    if (NT_SUCCESS(Status)) {
        RtlInitUnicodeString(
            &ImageFileName,
            L"\\SystemRoot\\System32\\ntoskrnl.exe");

        InitializeObjectAttributes(
            &ObjectAttributes,
            &ImageFileName,
            (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
            NULL,
            NULL);

        Status = ZwOpenFile(
            &FileHandle,
            FILE_EXECUTE,
            &ObjectAttributes,
            &IoStatusBlock,
            FILE_SHARE_READ | FILE_SHARE_DELETE,
            0);

        if (NT_SUCCESS(Status)) {
            InitializeObjectAttributes(
                &ObjectAttributes,
                NULL,
                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                NULL,
                NULL);

            Status = ZwCreateSection(
                &SectionHandle,
                SECTION_MAP_READ | SECTION_MAP_EXECUTE,
                &ObjectAttributes,
                NULL,
                PAGE_EXECUTE,
                SEC_IMAGE,
                FileHandle);

            if (NT_SUCCESS(Status)) {
                Status = ZwMapViewOfSection(
                    SectionHandle,
                    ZwCurrentProcess(),
                    &ViewBase,
                    0L,
                    0L,
                    NULL,
                    &ViewSize,
                    ViewShare,
                    0L,
                    PAGE_EXECUTE);

                if (NT_SUCCESS(Status)) {
                    Diff = (ULONG64)PatchGuardBlock->KernelDataTableEntry->DllBase - (ULONG64)ViewBase;

                    ControlPc = ScanBytes(
                        ViewBase,
                        (PCHAR)ViewBase + ViewSize,
                        CmpAppendDllSection);

                    if (NULL != ControlPc) {
                        TargetPc = ControlPc;

                        while (0 != _CmpByte(TargetPc[0], 0x41) &&
                            0 != _CmpByte(TargetPc[1], 0xff) &&
                            0 != _CmpByte(TargetPc[2], 0xe0)) {
                            Length = DetourGetInstructionLength(TargetPc);

                            if (0 == PatchGuardBlock->SizeOfCmpAppendDllSection) {
                                if (8 == Length) {
                                    if (0 == _CmpByte(TargetPc[0], 0x48) &&
                                        0 == _CmpByte(TargetPc[1], 0x31) &&
                                        0 == _CmpByte(TargetPc[2], 0x84) &&
                                        0 == _CmpByte(TargetPc[3], 0xca)) {
                                        PatchGuardBlock->SizeOfCmpAppendDllSection = *(PULONG)(TargetPc + 4);

#ifndef PUBLIC
                                        DbgPrint(
                                            "Shark - < %p > PatchGuard clear context SizeOfCmpAppendDllSection\n",
                                            PatchGuardBlock->SizeOfCmpAppendDllSection);
#endif // !PUBLIC

                                        if (0 == _CmpByte(TargetPc[11], 0x48) ||
                                            0 == _CmpByte(TargetPc[12], 0x0f) ||
                                            0 == _CmpByte(TargetPc[13], 0xbb) ||
                                            0 == _CmpByte(TargetPc[14], 0xc0)) {
                                            PatchGuardBlock->IsBtcEncryptedEnable = TRUE;

#ifndef PUBLIC
                                            DbgPrint(
                                                "Shark - < %p > PatchGuard clear context IsBtcEncryptedEnable\n",
                                                PatchGuardBlock->IsBtcEncryptedEnable);
#endif // !PUBLIC
                                        }
                                    }
                                }
                            }

                            if (6 == Length) {
                                if (0 == _CmpByte(TargetPc[0], 0x8b) &&
                                    0 == _CmpByte(TargetPc[1], 0x82)) {
                                    PatchGuardBlock->RvaOffsetOfEntry = *(PULONG)(TargetPc + 2);

#ifndef PUBLIC
                                    DbgPrint(
                                        "Shark - < %p > PatchGuard clear context RvaOffsetOfEntry\n",
                                        PatchGuardBlock->RvaOffsetOfEntry);
#endif // !PUBLIC
                                    break;
                                }
                            }

                            TargetPc += Length;
                        }
                    }

                    ControlPc = ScanBytes(
                        ViewBase,
                        (PCHAR)ViewBase + ViewSize,
                        EntryPoint);

                    if (NULL != ControlPc) {
                        TargetPc = ControlPc + Diff;

                        FunctionEntry = RtlLookupFunctionEntry(
                            (ULONG64)TargetPc,
                            (PULONG64)&PatchGuardBlock->KernelDataTableEntry->DllBase,
                            NULL);

                        if (NULL != FunctionEntry) {
                            TargetPc =
                                (PCHAR)PatchGuardBlock->KernelDataTableEntry->DllBase +
                                FunctionEntry->BeginAddress -
                                Diff;

                            RtlCopyMemory(
                                PatchGuardBlock->EntryPoint,
                                TargetPc,
                                sizeof(PatchGuardBlock->EntryPoint));
                        }
                    }

                    ControlPc = ViewBase;

                    while (NULL != ControlPc) {
                        ControlPc = ScanBytes(
                            ControlPc,
                            (PCHAR)ViewBase + ViewSize,
                            Field);

                        if (NULL != ControlPc) {
                            TargetPc = ScanBytes(
                                ControlPc,
                                ControlPc + PatchGuardBlock->RvaOffsetOfEntry,
                                FirstField);

                            if (NULL != TargetPc) {
                                PatchGuardBlock->CompareFields[0] =
                                    (ULONG64)RvaToVa(TargetPc - 4) + Diff;

#ifndef PUBLIC
                                DbgPrint(
                                    "Shark - < %p > PatchGuard clear context CompareFields[0]\n",
                                    PatchGuardBlock->CompareFields[0]);
#endif // !PUBLIC

                                PatchGuardBlock->CompareFields[1] =
                                    (ULONG64)RvaToVa(TargetPc + 10) + Diff;

#ifndef PUBLIC
                                DbgPrint(
                                    "Shark - < %p > PatchGuard clear context CompareFields[1]\n",
                                    PatchGuardBlock->CompareFields[1]);
#endif // !PUBLIC

                                PatchGuardBlock->CompareFields[2] =
                                    (ULONG64)RvaToVa(TargetPc + 24) + Diff;

#ifndef PUBLIC
                                DbgPrint(
                                    "Shark - < %p > PatchGuard clear context CompareFields[2]\n",
                                    PatchGuardBlock->CompareFields[2]);
#endif // !PUBLIC

                                PatchGuardBlock->CompareFields[3] =
                                    (ULONG64)RvaToVa(TargetPc + 38) + Diff;

#ifndef PUBLIC
                                DbgPrint(
                                    "Shark - < %p > PatchGuard clear context CompareFields[3]\n",
                                    PatchGuardBlock->CompareFields[3]);
#endif // !PUBLIC

                                break;
                            }

                            ControlPc++;
                        }
                        else {
                            break;
                        }
                    }

                    ControlPc = ScanBytes(
                        ViewBase,
                        (PCHAR)ViewBase + ViewSize,
                        PatchGuardEntry);

                    if (NULL != ControlPc) {
                        NtSection = SectionTableFromVirtualAddress(
                            ViewBase,
                            ControlPc);

                        if (NULL != NtSection) {
                            PatchGuardBlock->SizeOfNtSection = max(
                                NtSection->SizeOfRawData,
                                NtSection->Misc.VirtualSize);

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context SizeOfNtSection\n",
                                PatchGuardBlock->SizeOfNtSection);
#endif // !PUBLIC
                        }
                    }

                    ControlPc = ScanBytes(
                        ViewBase,
                        (PCHAR)ViewBase + ViewSize,
                        KiStartSystemThread);

                    if (NULL != ControlPc) {
                        TargetPc = ControlPc;
                        PatchGuardBlock->KiStartSystemThread = (PVOID)(TargetPc + Diff);

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context KiStartSystemThread\n",
                            PatchGuardBlock->KiStartSystemThread);
#endif // !PUBLIC
                    }

                    ControlPc = ScanBytes(
                        ViewBase,
                        (PCHAR)ViewBase + ViewSize,
                        PspSystemThreadStartup);

                    if (NULL != ControlPc) {
                        TargetPc = (PVOID)
                            (ControlPc + Diff);

                        FunctionEntry = RtlLookupFunctionEntry(
                            (ULONG64)TargetPc,
                            (PULONG64)&PatchGuardBlock->KernelDataTableEntry->DllBase,
                            NULL);

                        if (NULL != FunctionEntry) {
                            PatchGuardBlock->PspSystemThreadStartup = (PVOID)
                                ((PCHAR)PatchGuardBlock->KernelDataTableEntry->DllBase + FunctionEntry->BeginAddress);

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context PspSystemThreadStartup\n",
                                PatchGuardBlock->PspSystemThreadStartup);
#endif // !PUBLIC
                        }
                    }

                    if (PatchGuardBlock->BuildNumber >= 10586) {
                        ControlPc = ScanBytes(
                            ViewBase,
                            (PCHAR)ViewBase + ViewSize,
                            MiGetPteAddress);

                        if (NULL != ControlPc) {
                            TargetPc = (PVOID)(ControlPc + Diff);
                            PatchGuardBlock->PteBase = *(PMMPTE *)(TargetPc + 19);

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context PteBase\n",
                                PatchGuardBlock->PteBase);
#endif // !PUBLIC

                            PatchGuardBlock->PteTop = (PMMPTE)
                                ((LONGLONG)PatchGuardBlock->PteBase |
                                (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS + 1))
                                    >> PTI_SHIFT) << PTE_SHIFT) - 1));

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context PteTop\n",
                                PatchGuardBlock->PteTop);
#endif // !PUBLIC

                            PatchGuardBlock->PdeBase = (PMMPTE)
                                (((LONGLONG)PatchGuardBlock->PteBase & ~(
                                ((LONGLONG)1 << PHYSICAL_ADDRESS_BITS) - 1)) |
                                    (((LONGLONG)PatchGuardBlock->PteBase >> 9) & (
                                    ((LONGLONG)1 << PHYSICAL_ADDRESS_BITS) - 1)));

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context PdeBase\n",
                                PatchGuardBlock->PdeBase);
#endif // !PUBLIC

                            PatchGuardBlock->PdeTop = (PMMPTE)
                                ((LONGLONG)PatchGuardBlock->PdeBase |
                                (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS +
                                    1)) >> PDI_SHIFT) << PTE_SHIFT) - 1));

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context PdeTop\n",
                                PatchGuardBlock->PdeTop);
#endif // !PUBLIC

                            PatchGuardBlock->PpeBase = (PMMPTE)
                                (((LONGLONG)PatchGuardBlock->PdeBase & ~(
                                ((LONGLONG)1 << PHYSICAL_ADDRESS_BITS) - 1)) |
                                    (((LONGLONG)PatchGuardBlock->PdeBase >> 9) & (
                                    ((LONGLONG)1 << PHYSICAL_ADDRESS_BITS) - 1)));

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context PpeBase\n",
                                PatchGuardBlock->PpeBase);
#endif // !PUBLIC

                            PatchGuardBlock->PpeTop = (PMMPTE)
                                ((LONGLONG)PatchGuardBlock->PpeBase |
                                (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS +
                                    1)) >> PPI_SHIFT) << PTE_SHIFT) - 1));

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context PpeTop\n",
                                PatchGuardBlock->PpeTop);
#endif // !PUBLIC

                            PatchGuardBlock->PxeBase = (PMMPTE)
                                (((LONGLONG)PatchGuardBlock->PpeBase & ~(
                                ((LONGLONG)1 << PHYSICAL_ADDRESS_BITS) - 1)) |
                                    (((LONGLONG)PatchGuardBlock->PpeBase >> 9) & (
                                    ((LONGLONG)1 << PHYSICAL_ADDRESS_BITS) - 1)));

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context PxeBase\n",
                                PatchGuardBlock->PxeBase);
#endif // !PUBLIC

                            PatchGuardBlock->PxeTop = (PMMPTE)
                                ((LONGLONG)PatchGuardBlock->PxeBase |
                                (((((LONGLONG)1 << (VIRTUAL_ADDRESS_BITS +
                                    1)) >> PXI_SHIFT) << PTE_SHIFT) - 1));

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > PatchGuard clear context PxeTop\n",
                                PatchGuardBlock->PxeTop);
#endif // !PUBLIC
                        }
                    }
                    else {
                        PatchGuardBlock->PteBase = (PMMPTE)PTE_BASE;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context PteBase\n",
                            PatchGuardBlock->PteBase);
#endif // !PUBLIC

                        PatchGuardBlock->PteTop = (PMMPTE)PTE_TOP;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context PteTop\n",
                            PatchGuardBlock->PteTop);
#endif // !PUBLIC

                        PatchGuardBlock->PdeBase = (PMMPTE)PDE_BASE;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context PdeBase\n",
                            PatchGuardBlock->PdeBase);
#endif // !PUBLIC

                        PatchGuardBlock->PdeTop = (PMMPTE)PDE_TOP;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context PdeTop\n",
                            PatchGuardBlock->PdeTop);
#endif // !PUBLIC

                        PatchGuardBlock->PpeBase = (PMMPTE)PPE_BASE;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context PpeBase\n",
                            PatchGuardBlock->PpeBase);
#endif // !PUBLIC

                        PatchGuardBlock->PpeTop = (PMMPTE)PPE_TOP;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context PpeTop\n",
                            PatchGuardBlock->PpeTop);
#endif // !PUBLIC

                        PatchGuardBlock->PxeBase = (PMMPTE)PXE_BASE;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context PxeBase\n",
                            PatchGuardBlock->PxeBase);
#endif // !PUBLIC

                        PatchGuardBlock->PxeTop = (PMMPTE)PXE_TOP;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context PxeTop\n",
                            PatchGuardBlock->PxeTop);
#endif // !PUBLIC
                    }

                    ZwUnmapViewOfSection(NtCurrentProcess(), ViewBase);
                }

                ZwClose(SectionHandle);
            }

            ZwClose(FileHandle);
        }

        NtHeaders = RtlImageNtHeader(PatchGuardBlock->KernelDataTableEntry->DllBase);

        if (NULL != NtHeaders) {
            NtSection = IMAGE_FIRST_SECTION(NtHeaders);

            ControlPc = (PCHAR)PatchGuardBlock->KernelDataTableEntry->DllBase + NtSection[0].VirtualAddress;

            EndToLock = (PCHAR)PatchGuardBlock->KernelDataTableEntry->DllBase +
                NtSection[0].VirtualAddress +
                max(NtSection[0].SizeOfRawData, NtSection[0].Misc.VirtualSize);

            while (TRUE) {
                ControlPc = ScanBytes(
                    ControlPc,
                    EndToLock,
                    MiDeterminePoolType);

                if (NULL != ControlPc) {
                    TargetPc = ScanBytes(
                        ControlPc,
                        ControlPc + 0x100,
                        CheckMiDeterminePoolType);

                    if (NULL != TargetPc) {
                        ControlPc = TargetPc;
                        TargetPc = RvaToVa(TargetPc + 14);

                        RtlCopyMemory(
                            (PVOID)&PatchGuardBlock->MmDeterminePoolType,
                            &TargetPc,
                            sizeof(PVOID));

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context MmDeterminePoolType\n",
                            PatchGuardBlock->MmDeterminePoolType);
#endif // !PUBLIC

                        TargetPc = ControlPc;

                        while (TRUE) {
                            Length = DetourGetInstructionLength(TargetPc);

                            if (1 == Length) {
                                if (0 == _CmpByte(TargetPc[0], 0xc3)) {
                                    break;
                                }
                            }

                            if (7 == Length) {
                                if (0x40 == (TargetPc[0] & 0xf0)) {
                                    if (0 == _CmpByte(TargetPc[1], 0x8b)) {
                                        if (NULL == PageTable) {
                                            PageTable = (PPOOL_BIG_PAGES *)RvaToVa(TargetPc + 3);

                                            if (0 == (ULONG64)*PageTable ||
                                                0 != ((ULONG64)(*PageTable) & 0xfff)) {
                                                PageTable = NULL;
                                            }
                                        }
                                        else if (NULL == PageTableSize) {
                                            PageTableSize = (PSIZE_T)RvaToVa(TargetPc + 3);

                                            if (0 == *PageTableSize ||
                                                0 != ((ULONG64)(*PageTableSize) & 0xfff)) {
                                                PageTableSize = NULL;
                                            }
                                        }
                                    }
                                    else if (0 == _CmpByte(TargetPc[1], 0x8d)) {
                                        if (NULL == PatchGuardBlock->ExpLargePoolTableLock) {
                                            PatchGuardBlock->ExpLargePoolTableLock = (PEX_SPIN_LOCK)RvaToVa(TargetPc + 3);
                                        }
                                    }
                                }

                                if (0 == _CmpByte(TargetPc[0], 0x0f) &&
                                    0 == _CmpByte(TargetPc[1], 0x0d) &&
                                    0 == _CmpByte(TargetPc[2], 0x0d)) {
                                    if (NULL == PatchGuardBlock->ExpLargePoolTableLock) {
                                        PatchGuardBlock->ExpLargePoolTableLock = (PEX_SPIN_LOCK)RvaToVa(TargetPc + 3);
                                    }
                                }
                            }

                            if (NULL != PageTable &&
                                NULL != PageTableSize &&
                                NULL != PatchGuardBlock->ExpLargePoolTableLock) {
                                if ((ULONG64)*PageTable > (ULONG64)*PageTableSize) {
                                    PatchGuardBlock->PoolBigPageTable = (PPOOL_BIG_PAGES)*PageTable;
                                    PatchGuardBlock->PoolBigPageTableSize = (SIZE_T)*PageTableSize;
                                }
                                else {
                                    // swap

                                    PatchGuardBlock->PoolBigPageTable = (PPOOL_BIG_PAGES)*PageTableSize;
                                    PatchGuardBlock->PoolBigPageTableSize = (SIZE_T)*PageTable;
                                }

#ifndef PUBLIC
                                DbgPrint(
                                    "Shark - < %p > PatchGuard clear context PoolBigPageTable\n",
                                    PatchGuardBlock->PoolBigPageTable);

                                DbgPrint(
                                    "Shark - < %p > PatchGuard clear context PoolBigPageTableSize\n",
                                    PatchGuardBlock->PoolBigPageTableSize);

                                DbgPrint(
                                    "Shark - < %p > PatchGuard clear context ExpLargePoolTableLock\n",
                                    PatchGuardBlock->ExpLargePoolTableLock);
#endif // !PUBLIC

                                break;
                            }

                            TargetPc +=
                                DetourGetInstructionLength(TargetPc);
                        }

                        break;
                    }
                }
                else {
                    break;
                }

                ControlPc++;
            }
        }

        ControlPc = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "MmAllocateMappingAddress",
            0);

        if (NULL != ControlPc) {
            while (TRUE) {
                Length = DetourGetInstructionLength(ControlPc);

                if (1 == Length) {
                    if (0 == _CmpByte(ControlPc[0], 0xc3)) {
                        break;
                    }
                }

                if (7 == Length) {
                    if (0 == _CmpByte(ControlPc[0], 0x48) &&
                        0 == _CmpByte(ControlPc[1], 0x8d) &&
                        0 == _CmpByte(ControlPc[2], 0x0d)) {
                        TargetPc = RvaToVa(ControlPc + 3);

                        // struct _MI_SYSTEM_PTE_TYPE *
                        PatchGuardBlock->SystemPtes.BitMap = TargetPc;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context SystemPtes.BitMap\n",
                            PatchGuardBlock->SystemPtes.BitMap);
#endif // !PUBLIC

                        if (PatchGuardBlock->BuildNumber < 9600) {
                            PatchGuardBlock->SystemPtes.BasePte = *(PMMPTE *)
                                ((PCHAR)(PatchGuardBlock->SystemPtes.BitMap + 1) + sizeof(ULONG) * 2);
                        }
                        else {
                            PatchGuardBlock->SystemPtes.BasePte = *(PMMPTE *)
                                (PatchGuardBlock->SystemPtes.BitMap + 1);
                        }

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context SystemPtes.BasePte\n",
                            PatchGuardBlock->SystemPtes.BasePte);
#endif // !PUBLIC

                        break;
                    }
                }

                ControlPc += Length;
            }
        }

        ControlPc = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "KeCapturePersistentThreadState",
            0);

        if (NULL != ControlPc) {
            while (TRUE) {
                Length = DetourGetInstructionLength(ControlPc);

                if (1 == Length) {
                    if (0 == _CmpByte(ControlPc[0], 0xc3)) {
                        break;
                    }
                }

                if (7 == Length) {
                    if (0 == _CmpByte(ControlPc[0], 0x48) &&
                        0 == _CmpByte(ControlPc[1], 0x8b) &&
                        0 == _CmpByte(ControlPc[2], 0x05)) {
                        TargetPc = RvaToVa(ControlPc + 3);

                        PatchGuardBlock->MmPfnDatabase = *(PMMPFN *)TargetPc;

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > PatchGuard clear context MmPfnDatabase\n",
                            PatchGuardBlock->MmPfnDatabase);
#endif // !PUBLIC

                        break;
                    }
                }

                ControlPc += Length;
            }
        }

        PatchGuardBlock->MmHighestUserAddress = *(PVOID *)GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "MmHighestUserAddress",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context MmHighestUserAddress\n",
            PatchGuardBlock->MmHighestUserAddress);
#endif // !PUBLIC

        PatchGuardBlock->MmSystemRangeStart = *(PVOID *)GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "MmSystemRangeStart",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context MmSystemRangeStart\n",
            PatchGuardBlock->MmSystemRangeStart);
#endif // !PUBLIC

        PatchGuardBlock->IoGetInitialStack = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "IoGetInitialStack",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context IoGetInitialStack\n",
            PatchGuardBlock->IoGetInitialStack);
#endif // !PUBLIC

        PatchGuardBlock->ExAcquireSpinLockShared = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "ExAcquireSpinLockShared",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context ExAcquireSpinLockShared\n",
            PatchGuardBlock->ExAcquireSpinLockShared);
#endif // !PUBLIC

        PatchGuardBlock->ExReleaseSpinLockShared = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "ExReleaseSpinLockShared",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context ExReleaseSpinLockShared\n",
            PatchGuardBlock->ExReleaseSpinLockShared);
#endif // !PUBLIC

        RtlCopyMemory(
            &PatchGuardBlock->_ClearEncryptedContext,
            _ClearEncryptedContext,
            sizeof(PatchGuardBlock->_ClearEncryptedContext));

        // mov rbx, 0fffff80000000000h ; relocate PatchGuardBlock

        PatchGuardBlock->_ClearEncryptedContext.PatchGuardBlock = PatchGuardBlock;

        PatchGuardBlock->ClearEncryptedContext = (PVOID)PatchGuardBlock->_ClearEncryptedContext._ShellCode;

        RtlCopyMemory(
            &PatchGuardBlock->_RevertWorkerToSelf,
            _RevertWorkerToSelf,
            sizeof(PatchGuardBlock->_RevertWorkerToSelf));

        PatchGuardBlock->_RevertWorkerToSelf.PatchGuardBlock = PatchGuardBlock;

        PatchGuardBlock->RevertWorkerToSelf = (PVOID)PatchGuardBlock->_RevertWorkerToSelf._ShellCode;

        PatchGuardBlock->DbgPrint = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "DbgPrint",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context DbgPrint\n",
            PatchGuardBlock->DbgPrint);
#endif // !PUBLIC

        PatchGuardBlock->ClearEncryptedContextMessage = (PVOID)PatchGuardBlock->_Message[0];

        RtlCopyMemory(
            PatchGuardBlock->ClearEncryptedContextMessage,
            ClearEncryptedContextMessage,
            sizeof(ClearEncryptedContextMessage));

        PatchGuardBlock->RevertWorkerToSelfMessage = (PVOID)PatchGuardBlock->_Message[1];

        RtlCopyMemory(
            PatchGuardBlock->RevertWorkerToSelfMessage,
            RevertWorkerToSelfMessage,
            sizeof(RevertWorkerToSelfMessage));

        PatchGuardBlock->RtlCompareMemory = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "RtlCompareMemory",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context RtlCompareMemory\n",
            PatchGuardBlock->RtlCompareMemory);
#endif // !PUBLIC

        PatchGuardBlock->RtlRestoreContext = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "RtlRestoreContext",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context RtlRestoreContext\n",
            PatchGuardBlock->RtlRestoreContext);
#endif // !PUBLIC

        PatchGuardBlock->ExQueueWorkItem = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "ExQueueWorkItem",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context ExQueueWorkItem\n",
            PatchGuardBlock->ExQueueWorkItem);
#endif // !PUBLIC

        PatchGuardBlock->ExFreePool = GetKernelProcedureAddress(
            PatchGuardBlock->KernelDataTableEntry->DllBase,
            "ExFreePool",
            0);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > PatchGuard clear context ExFreePool\n",
            PatchGuardBlock->ExFreePool);
#endif // !PUBLIC

        RtlCopyMemory(
            PatchGuardBlock->_SdbpCheckDll,
            SdbpCheckDll,
            sizeof(SdbpCheckDll));

        PatchGuardBlock->SdbpCheckDll = (PVOID)PatchGuardBlock->_SdbpCheckDll;
        PatchGuardBlock->SizeOfSdbpCheckDll = sizeof(SdbpCheckDll);

        RtlCopyMemory(
            PatchGuardBlock->_CheckPatchGuardCode._ShellCode,
            _CheckPatchGuardCode,
            sizeof(PatchGuardBlock->_CheckPatchGuardCode._ShellCode));

        PatchGuardBlock->CheckPatchGuardCode = (PVOID)PatchGuardBlock->_CheckPatchGuardCode._ShellCode;
    }
}

VOID
NTAPI
SetNewEntryForEncryptedContext(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock,
    __in PVOID PatchGuardContext,
    __in ULONG64 RorKey
)
{
    ULONG64 LastRorKey = 0;
    ULONG RvaOfEntry = 0;
    ULONG64 FieldBuffer[PG_COMPARE_FIELDS_COUNT] = { 0 };
    ULONG FieldIndex = 0;
    ULONG Index = 0;
    PCHAR ControlPc = NULL;

    // xor code must be align 8 byte;
    // get PatchGuard entry offset in encrypted code

    FieldIndex = (PatchGuardBlock->RvaOffsetOfEntry -
        PatchGuardBlock->SizeOfCmpAppendDllSection) / sizeof(ULONG64);

    RtlCopyMemory(
        FieldBuffer,
        (PCHAR)PatchGuardContext + (PatchGuardBlock->RvaOffsetOfEntry & ~7),
        sizeof(FieldBuffer));

    LastRorKey = RorKey;

    for (Index = 0;
        Index < FieldIndex;
        Index++) {
        LastRorKey = __ROL64(LastRorKey, Index);
    }

    for (Index = 0;
        Index < RTL_NUMBER_OF(FieldBuffer);
        Index++) {
        LastRorKey = __ROL64(LastRorKey, FieldIndex + Index);
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
    }

    RvaOfEntry = *(PULONG)((PCHAR)FieldBuffer + (PatchGuardBlock->RvaOffsetOfEntry & 7));

    // copy PatchGuard entry head code to temp bufer and decode

    FieldIndex = (RvaOfEntry - PatchGuardBlock->SizeOfCmpAppendDllSection) / sizeof(ULONG64);

    RtlCopyMemory(
        FieldBuffer,
        (PCHAR)PatchGuardContext + (RvaOfEntry & ~7),
        sizeof(FieldBuffer));

    LastRorKey = RorKey;

    for (Index = 0;
        Index < FieldIndex;
        Index++) {
        LastRorKey = __ROL64(LastRorKey, Index);
    }

    for (Index = 0;
        Index < RTL_NUMBER_OF(FieldBuffer);
        Index++) {
        LastRorKey = __ROL64(LastRorKey, FieldIndex + Index);
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
    }

    // set temp buffer PatchGuard entry head jmp to _ClearEncryptedContext and encrypt

    ControlPc = (PCHAR)FieldBuffer + (RvaOfEntry & 7);

    BuildJumpCode(
        PatchGuardBlock->_ClearEncryptedContext._ShellCode,
        &ControlPc);

    while (Index--) {
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
        LastRorKey = __ROR64(LastRorKey, FieldIndex + Index);
    }

    // copy temp buffer PatchGuard entry head to old address, 
    // when PatchGuard code decrypt self jmp _ClearEncryptedContext.

    RtlCopyMemory(
        (PCHAR)PatchGuardContext + (RvaOfEntry & ~7),
        FieldBuffer,
        sizeof(FieldBuffer));

#ifndef PUBLIC
    DbgPrint(
        "Shark - < %p > set new entry for PatchGuard encrypted context\n",
        PatchGuardBlock->_ClearEncryptedContext._ShellCode);
#endif // !PUBLIC

    InterlockedIncrementSizeT(&PatchGuardBlock->ReferenceCount);

#ifndef PUBLIC
    DbgPrint(
        "Shark - < %p > reference count\n",
        PatchGuardBlock->ReferenceCount);
#endif // !PUBLIC
}

VOID
NTAPI
SetNewEntryForEncryptedWithBtcContext(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock,
    __in PVOID PatchGuardContext,
    __in SIZE_T ContextSize
)
{
    ULONG64 RorKey = 0;
    ULONG64 LastRorKey = 0;
    ULONG LastRorKeyOffset = 0;
    ULONG64 FieldBuffer[PG_COMPARE_FIELDS_COUNT] = { 0 };
    ULONG FieldIndex = 0;
    ULONG AlignOffset = 0;
    ULONG Index = 0;
    PULONG64 ControlPc = NULL;
    PULONG64 TargetPc = NULL;
    ULONG CompareCount = 0;

    CompareCount = (ULONG)(ContextSize - PatchGuardBlock->SizeOfCmpAppendDllSection) / 8 - 1;

    for (AlignOffset = 0;
        AlignOffset < 8;
        AlignOffset++) {
        ControlPc = (PULONG64)((PCHAR)PatchGuardBlock->EntryPoint + AlignOffset);
        TargetPc = (PULONG64)((PCHAR)PatchGuardContext + PatchGuardBlock->SizeOfCmpAppendDllSection);

        for (Index = 0;
            Index < CompareCount;
            Index++) {
            RorKey = TargetPc[Index + 1] ^ ControlPc[1];
            LastRorKey = RorKey;
            RorKey = __ROR64(RorKey, Index + 1);
            RorKey = _btc64(RorKey, RorKey);

            if ((TargetPc[Index] ^ RorKey) == ControlPc[0]) {
                goto found;
            }
        }
    }

found:
    if (FALSE != RTL_SOFT_ASSERTMSG(
        "Shark - PatchGuard context entrypoint not found",
        Index != CompareCount)) {
        FieldIndex = Index - (0 == (AlignOffset & 7) ? 0 : 1);
        LastRorKeyOffset = 2 + (0 == (AlignOffset & 7) ? 0 : 1);

        RtlCopyMemory(
            FieldBuffer,
            (PCHAR)PatchGuardContext + PatchGuardBlock->SizeOfCmpAppendDllSection + FieldIndex * 8,
            sizeof(FieldBuffer));

        RorKey = LastRorKey;
        Index = LastRorKeyOffset;

        while (Index--) {
            FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
            RorKey = __ROR64(RorKey, FieldIndex + Index);
            RorKey = _btc64(RorKey, RorKey);
        }

        // set temp buffer PatchGuard entry head jmp to _ClearEncryptedContext and encrypt

        ControlPc = (PCHAR)FieldBuffer + 8 - AlignOffset;

        BuildJumpCode(
            PatchGuardBlock->_ClearEncryptedContext._ShellCode,
            &ControlPc);

        RorKey = LastRorKey;
        Index = LastRorKeyOffset;

        while (Index--) {
            FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
            RorKey = __ROR64(RorKey, FieldIndex + Index);
            RorKey = _btc64(RorKey, RorKey);
        }

        RtlCopyMemory(
            (PCHAR)PatchGuardContext + PatchGuardBlock->SizeOfCmpAppendDllSection + FieldIndex * 8,
            FieldBuffer,
            sizeof(FieldBuffer));

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > set new entry for PatchGuard encrypted with btc context\n",
            PatchGuardBlock->_ClearEncryptedContext._ShellCode);
#endif // !PUBLIC

        InterlockedIncrementSizeT(&PatchGuardBlock->ReferenceCount);

#ifndef PUBLIC
        DbgPrint(
            "Shark - < %p > reference count\n",
            PatchGuardBlock->ReferenceCount);
#endif // !PUBLIC
    }
}

VOID
NTAPI
CompareEncryptedContextFields(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock,
    __in PVOID CompareBegin,
    __in PVOID CompareEnd
)
{
    PCHAR TargetPc = NULL;
    SIZE_T Index = 0;
    ULONG64 RorKey = 0;
    ULONG RorKeyIndex = 0;
    PULONG64 CompareFields = NULL;
    PVOID PatchGuardContext = NULL;

    TargetPc = CompareBegin;

    do {
        CompareFields = TargetPc;

        if ((ULONG64)CompareFields == (ULONG64)PatchGuardBlock->CompareFields) {
            break;
        }

        RorKey = CompareFields[3] ^ PatchGuardBlock->CompareFields[3];

        // CmpAppendDllSection + 98

        // xor [rdx + rcx * 8 + 0c0h], rax
        // ror rax, cl

        // if >= win10 17134 btc rax, rax here

        // loop CmpAppendDllSection + 98

        if (0 == RorKey) {
            if (CompareFields[2] == PatchGuardBlock->CompareFields[2] &&
                CompareFields[1] == PatchGuardBlock->CompareFields[1] &&
                CompareFields[0] == PatchGuardBlock->CompareFields[0]) {
                PatchGuardContext = TargetPc - PG_FIRST_FIELD_OFFSET;

#ifndef PUBLIC
                DbgPrint(
                    "Shark - < %p > found PatchGuard declassified context\n",
                    PatchGuardContext);
#endif // !PUBLIC
                break;
            }
        }
        else {
            RorKeyIndex = 0;

            RorKey = __ROR64(RorKey, PG_FIELD_BITS);

            if (FALSE != PatchGuardBlock->IsBtcEncryptedEnable) {
                RorKey = _btc64(RorKey, RorKey);
            }

            RorKeyIndex++;

            if ((ULONG64)(CompareFields[2] ^ RorKey) == (ULONG64)PatchGuardBlock->CompareFields[2]) {
                RorKey = __ROR64(RorKey, PG_FIELD_BITS - RorKeyIndex);

                if (FALSE != PatchGuardBlock->IsBtcEncryptedEnable) {
                    RorKey = _btc64(RorKey, RorKey);
                }

                RorKeyIndex++;

                if ((ULONG64)(CompareFields[1] ^ RorKey) == (ULONG64)PatchGuardBlock->CompareFields[1]) {
                    RorKey = __ROR64(RorKey, PG_FIELD_BITS - RorKeyIndex);

                    if (FALSE != PatchGuardBlock->IsBtcEncryptedEnable) {
                        RorKey = _btc64(RorKey, RorKey);
                    }

                    RorKeyIndex++;

                    if ((ULONG64)(CompareFields[0] ^ RorKey) == (ULONG64)PatchGuardBlock->CompareFields[0]) {
                        PatchGuardContext = TargetPc - PG_FIRST_FIELD_OFFSET;

                        RorKey = CompareFields[0] ^ PatchGuardBlock->CompareFields[0];

#ifndef PUBLIC
                        DbgPrint(
                            "Shark - < %p > found PatchGuard encrypted context\n",
                            PatchGuardContext);
#endif // !PUBLIC

                        if (FALSE != PatchGuardBlock->IsBtcEncryptedEnable) {
                            SetNewEntryForEncryptedWithBtcContext(
                                PatchGuardBlock,
                                PatchGuardContext,
                                (ULONG_PTR)CompareEnd - (ULONG_PTR)PatchGuardContext);
                        }
                        else {
                            for (;
                                RorKeyIndex < PG_FIELD_BITS;
                                RorKeyIndex++) {
                                RorKey = __ROR64(RorKey, PG_FIELD_BITS - RorKeyIndex);
                            }

#ifndef PUBLIC
                            DbgPrint(
                                "Shark - < %p > first rorkey\n",
                                RorKey);
#endif // !PUBLIC

                            SetNewEntryForEncryptedContext(
                                PatchGuardBlock,
                                PatchGuardContext,
                                RorKey);
                        }

                        break;
                    }
                }
            }
        }

        TargetPc++;
    } while ((ULONG64)TargetPc < (ULONG64)CompareEnd);
}

VOID
NTAPI
ClearSystemPtesEncryptedContext(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock
)
{
    PFN_NUMBER Index = 0;
    PFN_NUMBER NumberOfPtes = 0;
    PMMPTE BasePte = NULL;
    PMMPTE PointerPte = NULL;
    PVOID BaseVa = NULL;
    PVOID PointerVa = NULL;

    PMMPTE LastPte = NULL;
    PVOID LastVa = NULL;
    KIRQL Irql = 0;

    /*
    PatchGuard Context pages allocate by MmAllocateIndependentPages

    PTE field like this

    nt!_MMPTE
        [+0x000] Long             : 0x2da963 [Type: unsigned __int64]
        [+0x000] VolatileLong     : 0x2da963 [Type: unsigned __int64]
        [+0x000] Hard             [Type: _MMPTE_HARDWARE]

            [+0x000 ( 0: 0)] Valid            : 0x1     [Type: unsigned __int64] <- MM_PTE_VALID_MASK
            [+0x000 ( 1: 1)] Dirty1           : 0x1     [Type: unsigned __int64] <- MM_PTE_DIRTY_MASK
            [+0x000 ( 2: 2)] Owner            : 0x0     [Type: unsigned __int64]
            [+0x000 ( 3: 3)] WriteThrough     : 0x0     [Type: unsigned __int64]
            [+0x000 ( 4: 4)] CacheDisable     : 0x0     [Type: unsigned __int64]
            [+0x000 ( 5: 5)] Accessed         : 0x1     [Type: unsigned __int64] <- MM_PTE_ACCESS_MASK
            [+0x000 ( 6: 6)] Dirty            : 0x1     [Type: unsigned __int64] <- MM_PTE_DIRTY_MASK
            [+0x000 ( 7: 7)] LargePage        : 0x0     [Type: unsigned __int64]
            [+0x000 ( 8: 8)] Global           : 0x1     [Type: unsigned __int64] <- MM_PTE_GLOBAL_MASK
            [+0x000 ( 9: 9)] CopyOnWrite      : 0x0     [Type: unsigned __int64]
            [+0x000 (10:10)] Unused           : 0x0     [Type: unsigned __int64]
            [+0x000 (11:11)] Write            : 0x1     [Type: unsigned __int64] <- MM_PTE_WRITE_MASK
            [+0x000 (47:12)] PageFrameNumber  : 0x2da   [Type: unsigned __int64] <- pfndata index
            [+0x000 (51:48)] reserved1        : 0x0     [Type: unsigned __int64]
            [+0x000 (62:52)] SoftwareWsIndex  : 0x0     [Type: unsigned __int64]
            [+0x000 (63:63)] NoExecute        : 0x0     [Type: unsigned __int64] <- page can executable

        [+0x000] Flush            [Type: _HARDWARE_PTE]
        [+0x000] Proto            [Type: _MMPTE_PROTOTYPE]
        [+0x000] Soft             [Type: _MMPTE_SOFTWARE]
        [+0x000] TimeStamp        [Type: _MMPTE_TIMESTAMP]
        [+0x000] Trans            [Type: _MMPTE_TRANSITION]
        [+0x000] Subsect          [Type: _MMPTE_SUBSECTION]
        [+0x000] List             [Type: _MMPTE_LIST]
    */

#define VALID_PTE_SET_BITS \
            ( MM_PTE_VALID_MASK | MM_PTE_WRITE_MASK | MM_PTE_ACCESS_MASK | \
                MM_PTE_DIRTY_MASK | MM_PTE_GLOBAL_MASK)

#define VALID_PTE_UNSET_BITS \
            ( MM_PTE_OWNER_MASK | MM_PTE_WRITE_THROUGH_MASK | MM_PTE_CACHE_DISABLE_MASK | \
                MM_PTE_LARGE_PAGE_MASK | MM_PTE_COPY_ON_WRITE_MASK | MM_PTE_PROTOTYPE_MASK )

    BaseVa = MiGetVirtualAddressMappedByPte(
        PatchGuardBlock->PteBase,
        PatchGuardBlock->SystemPtes.BasePte);

    BasePte = PatchGuardBlock->SystemPtes.BasePte;

    NumberOfPtes =
        PatchGuardBlock->SystemPtes.BitMap->SizeOfBitMap * 8;

    for (Index = 0;
        Index < NumberOfPtes;
        Index++) {
        PointerPte = BasePte + Index;
        PointerVa = (PCHAR)BaseVa + PAGE_SIZE * Index;

        if (FALSE != MmIsAddressValid(PointerVa)) {
            if (FALSE != MI_IS_PTE_EXECUTABLE(PointerPte)) {
                if (VALID_PTE_SET_BITS == (PointerPte->u.Long & VALID_PTE_SET_BITS)) {
                    if (0 == (PointerPte->u.Long & VALID_PTE_UNSET_BITS)) {
                        LastPte = PointerPte;
                        LastVa = PointerVa;

                        while (Index++) {
                            PointerPte = BasePte + Index;
                            PointerVa = (PCHAR)BaseVa + PAGE_SIZE * Index;

                            if (FALSE == MmIsAddressValid(PointerVa)) {
                                break;
                            }

                            if (FALSE == MI_IS_PTE_EXECUTABLE(PointerPte)) {
                                break;
                            }

                            if (VALID_PTE_SET_BITS != (PointerPte->u.Long & VALID_PTE_SET_BITS)) {
                                break;
                            }

                            if (0 != (PointerPte->u.Long & VALID_PTE_UNSET_BITS)) {
                                break;
                            }
                        }

                        if ((PointerPte - LastPte) * PAGE_SIZE > PatchGuardBlock->SizeOfNtSection) {
                            CompareEncryptedContextFields(
                                PatchGuardBlock,
                                LastVa,
                                (PCHAR)PointerVa - PatchGuardBlock->SizeOfCmpAppendDllSection);
                        }
                    }
                }
            }
        }
    }
}

VOID
NTAPI
ClearPoolEncryptedContext(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock
)
{
    PCHAR TargetPc = NULL;
    SIZE_T Index = 0;
    ULONG64 RorKey = 0;
    PULONG64 Field = NULL;
    PVOID PatchGuardContext = NULL;
    KIRQL Irql = 0;

    PVOID CompareBegin = NULL;
    PVOID CompareEnd = NULL;

    Irql = PatchGuardBlock->ExAcquireSpinLockShared(PatchGuardBlock->ExpLargePoolTableLock);

    for (Index = 0;
        Index < PatchGuardBlock->PoolBigPageTableSize;
        Index++) {
        if (POOL_BIG_TABLE_ENTRY_FREE != (
            (ULONG64)PatchGuardBlock->PoolBigPageTable[Index].Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            if (NonPagedPool == PatchGuardBlock->MmDeterminePoolType(
                PatchGuardBlock->PoolBigPageTable[Index].Va)) {
                if (PatchGuardBlock->PoolBigPageTable[Index].NumberOfPages > PatchGuardBlock->SizeOfNtSection) {
                    CompareBegin = PatchGuardBlock->PoolBigPageTable[Index].Va;
                    CompareEnd = (PCHAR)PatchGuardBlock->PoolBigPageTable[Index].Va +
                        PatchGuardBlock->PoolBigPageTable[Index].NumberOfPages -
                        PatchGuardBlock->SizeOfCmpAppendDllSection;

                    CompareEncryptedContextFields(
                        PatchGuardBlock,
                        CompareBegin,
                        CompareEnd);
                }
            }
        }
    }

    PatchGuardBlock->ExReleaseSpinLockShared(PatchGuardBlock->ExpLargePoolTableLock, Irql);
}

SIZE_T
NTAPI
GetVirtualAddressRegion(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock,
    __in PVOID VirtualAddress,
    __out PVOID * BaseAddress,
    __out PSIZE_T RegionSize
)
{
    BOOLEAN Result = FALSE;
    KIRQL Irql = 0;
    PFN_NUMBER Index = 0;
    PFN_NUMBER NumberOfPtes = 0;
    PMMPTE BasePte = NULL;
    PMMPTE PointerPte = NULL;
    PVOID BaseVa = NULL;
    PVOID PointerVa = NULL;

    PMMPTE LastPte = NULL;
    PVOID LastVa = NULL;

    *BaseAddress = NULL;
    *RegionSize = 0;

    Irql = PatchGuardBlock->ExAcquireSpinLockShared(PatchGuardBlock->ExpLargePoolTableLock);

    for (Index = 0;
        Index < PatchGuardBlock->PoolBigPageTableSize;
        Index++) {
        if (POOL_BIG_TABLE_ENTRY_FREE !=
            ((ULONG64)PatchGuardBlock->PoolBigPageTable[Index].Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            if (NonPagedPool == PatchGuardBlock->MmDeterminePoolType(PatchGuardBlock->PoolBigPageTable[Index].Va)) {
                if (PatchGuardBlock->PoolBigPageTable[Index].NumberOfPages > PatchGuardBlock->SizeOfNtSection) {
                    if ((ULONG64)VirtualAddress >= (ULONG64)PatchGuardBlock->PoolBigPageTable[Index].Va &&
                        (ULONG64)VirtualAddress < (ULONG64)PatchGuardBlock->PoolBigPageTable[Index].Va +
                        PatchGuardBlock->PoolBigPageTable[Index].NumberOfPages) {
                        *BaseAddress = PatchGuardBlock->PoolBigPageTable[Index].Va;
                        *RegionSize = PatchGuardBlock->PoolBigPageTable[Index].NumberOfPages;

                        Result = TRUE;
                        break;
                    }
                }
            }
        }
    }

    if (FALSE == Result) {
        PatchGuardBlock->ExReleaseSpinLockShared(PatchGuardBlock->ExpLargePoolTableLock, Irql);

        BaseAddress = MiGetVirtualAddressMappedByPte(
            PatchGuardBlock->PteBase,
            PatchGuardBlock->SystemPtes.BasePte);

        BasePte = PatchGuardBlock->SystemPtes.BasePte;

        NumberOfPtes =
            PatchGuardBlock->SystemPtes.BitMap->SizeOfBitMap * 8;

        for (Index = 0;
            Index < NumberOfPtes;
            Index++) {
            PointerPte = BasePte + Index;
            PointerVa = (PCHAR)BaseVa + PAGE_SIZE * Index;

            if (FALSE != MmIsAddressValid(PointerVa)) {
                if (FALSE != MI_IS_PTE_EXECUTABLE(PointerPte)) {
                    if (VALID_PTE_SET_BITS == (PointerPte->u.Long & VALID_PTE_SET_BITS)) {
                        if (0 == (PointerPte->u.Long & VALID_PTE_UNSET_BITS)) {
                            LastPte = PointerPte;
                            LastVa = PointerVa;

                            while (Index++) {
                                PointerPte = BasePte + Index;
                                PointerVa = (PCHAR)BaseAddress + PAGE_SIZE * Index;

                                if (FALSE == MmIsAddressValid(PointerVa)) {
                                    break;
                                }

                                if (FALSE == MI_IS_PTE_EXECUTABLE(PointerPte)) {
                                    break;
                                }

                                if (VALID_PTE_SET_BITS != (PointerPte->u.Long & VALID_PTE_SET_BITS)) {
                                    break;
                                }

                                if (0 != (PointerPte->u.Long & VALID_PTE_UNSET_BITS)) {
                                    break;
                                }
                            }

                            if ((ULONG64)VirtualAddress >= (ULONG64)LastVa &&
                                (ULONG64)VirtualAddress < (ULONG64)PointerVa) {
                                *BaseAddress = LastVa;
                                *RegionSize = (SIZE_T)((PCHAR)PointerVa - (PCHAR)LastVa);

                                Result = TRUE;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return Result;
}

VOID
NTAPI
CheckWorkerThread(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock
)
{
    ULONG64 LowLimit = 0;
    ULONG64 HighLimit = 0;
    PULONG64 InitialStack = 0;
    PULONG64 TargetPc = NULL;
    ULONG Count = 0;
    PCALLERS Callers = NULL;
    ULONG Index = 0;
    LONG_PTR Reference = 0;
    PVOID BaseAddress = NULL;
    SIZE_T RegionSize = 0;
    PVOID * Parameters = NULL;

    Callers = ExAllocatePool(
        NonPagedPool,
        MAX_STACK_DEPTH * sizeof(CALLERS));

    if (NULL != Callers) {
        Count = WalkFrameChain(
            Callers,
            MAX_STACK_DEPTH);

        if (0 != Count) {
            IoGetStackLimits(&LowLimit, &HighLimit);

            InitialStack = IoGetInitialStack();

            // all worker thread start at KiStartSystemThread and return address == 0
            // if null != last return address code is in noimage

            if (NULL != Callers[Count - 1].Establisher) {
#ifndef PUBLIC
                DbgPrint(
                    "Shark - < %p > found noimage return address in worker thread\n",
                    Callers[Count - 1].Establisher);
#endif // !PUBLIC

                for (TargetPc = (PULONG64)Callers[Count - 1].EstablisherFrame;
                    (ULONG64)TargetPc < (ULONG64)InitialStack;
                    TargetPc++) {
                    // In most cases, PatchGuard code will wait for a random time.
                    // set return address in current thread stack to _RevertWorkerToSelf
                    // than PatchGuard code was not continue

                    if ((ULONG64)*TargetPc == (ULONG64)Callers[Count - 1].Establisher) {
                        // restart ExpWorkerThread

                        // ExFrame->P1Home = WorkerContext;
                        // ExFrame->P2Home = ExpWorkerThread;
                        // ExFrame->P3Home = PspSystemThreadStartup;
                        // ExFrame->Return = KiStartSystemThread; <- jmp this function return address == 0

                        if (FALSE != GetVirtualAddressRegion(
                            PatchGuardBlock,
                            Callers[Count - 1].Establisher,
                            &BaseAddress,
                            &RegionSize)) {
                            for (Index = 0;
                                Index < PG_MAXIMUM_CONTEXT_COUNT;
                                Index++) {
                                if (FALSE == PatchGuardBlock->_GuardCall[Index].Usable) {
                                    PatchGuardBlock->_GuardCall[Index].Usable = TRUE;

                                    RtlCopyMemory(
                                        &PatchGuardBlock->_GuardCall[Index],
                                        _GuardCall,
                                        sizeof(PatchGuardBlock->_GuardCall[Index]));
                                    PatchGuardBlock->_GuardCall[Index].PatchGuardBlock = PatchGuardBlock;

                                    Parameters = PatchGuardBlock->_GuardCall[Index].Parameters;

                                    Parameters[0] = (PVOID)Callers[Count - 1].Establisher;
                                    Parameters[1] = (PVOID)BaseAddress;
                                    Parameters[2] = (PVOID)RegionSize;

                                    *TargetPc = (ULONG64)PatchGuardBlock->_GuardCall[Index]._ShellCode;

#ifndef PUBLIC
                                    DbgPrint(
                                        "Shark - < %p > insert worker thread check function\n",
                                        PatchGuardBlock->_GuardCall[Index]._ShellCode);
#endif // !PUBLIC

                                    InterlockedIncrementSizeT(&PatchGuardBlock->ReferenceCount);

#ifndef PUBLIC
                                    DbgPrint(
                                        "Shark - < %p > reference count\n",
                                        PatchGuardBlock->ReferenceCount);
#endif // !PUBLIC

                                    break;
                                }
                            }
                        }

                        break;
                    }
                }
            }
        }

        ExFreePool(Callers);
    }
}

VOID
NTAPI
CheckAllWorkerThread(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo = NULL;
    PSYSTEM_EXTENDED_THREAD_INFORMATION ThreadInfo = NULL;
    PVOID Buffer = NULL;
    ULONG BufferSize = PAGE_SIZE;
    ULONG ReturnLength = 0;
    ULONG Index = 0;
    PULONG64 InitialStack = 0;
    PKPRIQUEUE WorkPriQueue = NULL;
    ULONG OsBuildNumber = 0;

    /*
    if (os build >= 9600){
        PatchGuardBlock->WorkerContext = struct _KPRIQUEUE

            WorkPriQueue->Header.Type = 0x15
            WorkPriQueue->Header.Hand = 0xac
    }
    else {
        PatchGuardBlock->WorkerContext = enum _WORK_QUEUE_TYPE

            CriticalWorkQueue = 0
            DelayedWorkQueue = 1
    }
    */

    InitialStack = IoGetInitialStack();

    PsGetVersion(
        NULL,
        NULL,
        &OsBuildNumber,
        NULL);

    if (OsBuildNumber >= 9600) {
        while ((ULONG64)InitialStack != (ULONG64)&PatchGuardBlock) {
            WorkPriQueue = *(PVOID *)InitialStack;

            if (FALSE != MmIsAddressValid(WorkPriQueue)) {
                if (FALSE != MmIsAddressValid((PCHAR)(WorkPriQueue + 1) - 1)) {
                    if (0x15 == WorkPriQueue->Header.Type &&
                        0xac == WorkPriQueue->Header.Hand) {
                        PatchGuardBlock->WorkerContext = WorkPriQueue;

                        break;
                    }
                }
            }

            InitialStack--;
        }
    }
    else {
        PatchGuardBlock->WorkerContext = UlongToPtr(CriticalWorkQueue);
    }

retry:
    Buffer = ExAllocatePool(
        NonPagedPool,
        BufferSize);

    if (NULL != Buffer) {
        RtlZeroMemory(
            Buffer,
            BufferSize);

        Status = ZwQuerySystemInformation(
            SystemExtendedProcessInformation,
            Buffer,
            BufferSize,
            &ReturnLength);

        if (NT_SUCCESS(Status)) {
            ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)Buffer;

            while (TRUE) {
                if (PsGetCurrentProcessId() == ProcessInfo->UniqueProcessId) {
                    ThreadInfo = (PSYSTEM_EXTENDED_THREAD_INFORMATION)
                        (ProcessInfo + 1);

                    if (NULL == PatchGuardBlock->ExpWorkerThread) {
                        for (Index = 0;
                            Index < ProcessInfo->NumberOfThreads;
                            Index++) {
                            if ((ULONG64)PsGetCurrentThreadId() ==
                                (ULONG64)ThreadInfo[Index].ThreadInfo.ClientId.UniqueThread) {
                                PatchGuardBlock->ExpWorkerThread = ThreadInfo[Index].Win32StartAddress;

                                break;
                            }
                        }
                    }

                    if (NULL != PatchGuardBlock->ExpWorkerThread) {
                        for (Index = 0;
                            Index < ProcessInfo->NumberOfThreads;
                            Index++) {
                            if ((ULONG64)PsGetCurrentThreadId() !=
                                (ULONG64)ThreadInfo[Index].ThreadInfo.ClientId.UniqueThread &&
                                (ULONG64)PatchGuardBlock->ExpWorkerThread ==
                                (ULONG64)ThreadInfo[Index].Win32StartAddress) {
                                RemoteCall(
                                    ThreadInfo[Index].ThreadInfo.ClientId.UniqueThread,
                                    IMAGE_NT_OPTIONAL_HDR_MAGIC,
                                    (PUSER_THREAD_START_ROUTINE)CheckWorkerThread,
                                    PatchGuardBlock,
                                    KernelMode);
                            }
                        }
                    }

                    break;
                }

                if (0 == ProcessInfo->NextEntryOffset) {
                    break;
                }
                else {
                    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
                        ((PCHAR)ProcessInfo + ProcessInfo->NextEntryOffset);
                }
            }
        }

        ExFreePool(Buffer);
        Buffer = NULL;

        if (STATUS_INFO_LENGTH_MISMATCH == Status) {
            BufferSize = ReturnLength;
            goto retry;
        }
    }
}

VOID
NTAPI
ClearPatchGuardWorker(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock
)
{
    ClearSystemPtesEncryptedContext(PatchGuardBlock);
    ClearPoolEncryptedContext(PatchGuardBlock);
    CheckAllWorkerThread(PatchGuardBlock);

    KeSetEvent(&PatchGuardBlock->Notify, LOW_PRIORITY, FALSE);
}

VOID
NTAPI
DisablePatchGuard(
    __inout PPATCHGUARD_BLOCK PatchGuardBlock
)
{
    // after PatchGuard logic is interrupted not trigger again.
    // so no need to continue running.

    InitializePatchGuardBlock(PatchGuardBlock);

    if (RTL_SOFT_ASSERTMSG(
        "Shark - PatchGuardBlock->IoGetInitialStack not found",
        NULL != PatchGuardBlock->IoGetInitialStack) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->AcquireSpinLockShared not found",
            NULL != PatchGuardBlock->ExAcquireSpinLockShared) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->ReleaseSpinLockShared not found",
            NULL != PatchGuardBlock->ExReleaseSpinLockShared) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->RvaOffsetOfEntry not found",
            0 != PatchGuardBlock->RvaOffsetOfEntry) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->SizeOfCmpAppendDllSection not found",
            0 != PatchGuardBlock->SizeOfCmpAppendDllSection) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->SizeOfNtSection not found",
            0 != PatchGuardBlock->SizeOfNtSection) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->PoolBigPageTable not found",
            NULL != PatchGuardBlock->PoolBigPageTable) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->PoolBigPageTableSize not found",
            0 != PatchGuardBlock->PoolBigPageTableSize) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->ExpLargePoolTableLock not found",
            NULL != PatchGuardBlock->ExpLargePoolTableLock) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->SystemPtesState not found",
            NULL != PatchGuardBlock->SystemPtes.BitMap) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->SystemPtesState not found",
            NULL != PatchGuardBlock->SystemPtes.BasePte) ||
        RTL_SOFT_ASSERTMSG(
            "Shark - PatchGuardBlock->MmDeterminePoolType not found",
            NULL != PatchGuardBlock->MmDeterminePoolType)) {
        KeInitializeEvent(
            &PatchGuardBlock->Notify,
            SynchronizationEvent,
            FALSE);

        ExInitializeWorkItem(
            &PatchGuardBlock->ClearWorkerItem,
            ClearPatchGuardWorker,
            PatchGuardBlock);

        ExQueueWorkItem(
            &PatchGuardBlock->ClearWorkerItem,
            CriticalWorkQueue);

        KeWaitForSingleObject(
            &PatchGuardBlock->Notify,
            Executive,
            KernelMode,
            FALSE,
            NULL);
    }
}
