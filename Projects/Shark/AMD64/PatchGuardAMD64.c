/*
*
* Copyright (c) 2019 by blindtiger. All rights reserved.
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

#include "Rtx.h"
#include "Scan.h"
#include "Space.h"
#include "Stack.h"

#define __ROL64(x, n) (((x) << ((n % 64))) | ((x) >> (64 - (n % 64))))
#define __ROR64(x, n) (((x) >> ((n % 64))) | ((x) << (64 - (n % 64))))

#define GetGpBlock(pgblock) \
            ((PGPBLOCK)((PCHAR)pgblock - sizeof(GPBLOCK)))

VOID
NTAPI
InitializePgBlock(
    __inout PPGBLOCK PgBlock
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE FileHandle = NULL;
    HANDLE SectionHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
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
    UNICODE_STRING RoutineString = { 0 };
    PVOID RoutineAddress = NULL;

    PPOOL_BIG_PAGES * PageTable = NULL;
    PSIZE_T PageTableSize = NULL;

    ULONG_PTR TempField = 0;

    CHAR CmpAppendDllSection[] = "2e 48 31 11 48 31 51 08 48 31 51 10 48 31 51 18";

    // Fields
    CHAR Fields[] = "fb 48 8d 05";
    CHAR FirstField[] = "?? 89 ?? 00 01 00 00 48 8D 05 ?? ?? ?? ?? ?? 89 ?? 08 01 00 00";
    CHAR NextField[] = "48 8D 05 ?? ?? ?? ?? ?? 89 86 ?? ?? 00 00";

    CHAR KiStartSystemThread[] = "b9 01 00 00 00 44 0f 22 c1 48 8b 14 24 48 8b 4c 24 08";
    CHAR PspSystemThreadStartup[] = "eb ?? b9 1e 00 00 00 e8";

    // MiDeterminePoolType
    CHAR MiDeterminePoolType[] = "48 8b ?? e8";
    CHAR CheckMiDeterminePoolType[] = "ff 0f 00 00 0f 85 ?? ?? ?? ?? 48 8b ?? e8";

    // 55                                       push    rbp
    // 41 54                                    push    r12
    // 41 55                                    push    r13
    // 41 56                                    push    r14
    // 41 57                                    push    r15
    // 48 81 EC C0 02 00 00                     sub     rsp, 2C0h
    // 48 8D A8 D8 FD FF FF                     lea     rbp, [rax - 228h]
    // 48 83 E5 80 and rbp, 0FFFFFFFFFFFFFF80h

    CHAR Header[] =
        "55 41 54 41 55 41 56 41 57 48 81 EC C0 02 00 00 48 8D A8 D8 FD FF FF 48 83 E5 80";

    ULONG64 SdbpCheckDll[] = {
        0x7c8b483024748b48, 0x333824548b4c2824,
        0x08ea8349028949c0, 0x7c8948f473d43b4c,
        0xe88bf88bd88b2824, 0x8b4cd88b4cd08b4c,
        0x4cf08b4ce88b4ce0, 0xcccccccce6fff88b
    };

    ULONG64 Btc64[] = {
        0xc3c18b48d1bb0f48
    };

    ULONG64 ClearCallback[] = {
        0x4c89481024548948, 0xc74858ec83480824, 0x4800000000302444, 0x00000000402444c7,
        0x000000382444c748, 0x0000282444c74800, 0x0000202444c70000, 0x486824448b480000,
        0x302444894818408b, 0xff48f03024448b48, 0xb60f6824448b4808, 0x8b4830244c8b4800,
        0x8b4830244c8b4811, 0x8b48000000c8c18c, 0xfff4c890ff302444, 0xb60f6824448b48ff,
        0x00008d840fc08500, 0x8b486024448b4800, 0x4c8b480000009880, 0x894830408b486024,
        0x448b48000000f881, 0x000098808b486024, 0x4c8b4838c0834800, 0x0000988189486024,
        0xbe0f6824448b4800, 0xd2331875c0850140, 0x488b486824448b48, 0x90ff3024448b4808,
        0x8b482bebfffff4e8, 0x830140be0f682444, 0x24448b481d7501f8, 0x448b4810508b4868,
        0x8b4808488b486824, 0x0000a890ff302444, 0x44c7000001a1e900, 0x0aeb000000002024,
        0x4489c0ff2024448b, 0x8b482024448b2024, 0x3024548b4868244c, 0x10498b4818528b48,
        0x830fc13b48ca2b48, 0x24448b480000016b, 0x8b000000d8054830, 0x6824548b4820244c,
        0x24548b48084a0348, 0xd08b4818428b4c30, 0xd090ff3024448b48, 0x30244c8b48fffff4,
        0x0128850f18413948, 0x8825048b48650000, 0x30244c8b48000001, 0x48fffffdc689b70f,
        0x4830e8834808048b, 0x24448b4840244489, 0x44894840e8834840, 0x484024448b483824,
        0x88898b4830244c8b, 0x8b48088948000000, 0x30244c8b48402444, 0x4800000090898b48,
        0x4024448b48084889, 0x898b4830244c8b48, 0x1048894800000098, 0x40c7484024448b48,
        0x448b480000000028, 0x4830244c8b483824, 0x8948000000a0898b, 0xc63824448b483848,
        0x2824448b48012840, 0x8b48000000800548, 0x483041894838244c, 0x244c8b486024448b,
        0x0000009888894840, 0x4c8b486024448b48, 0x0000a0898b483024, 0x000000f888894800,
        0x40be0f6824448b48, 0x48d2331875c08501, 0x08488b486824448b, 0xe890ff3024448b48,
        0x448b482bebfffff4, 0xf8830140be0f6824, 0x6824448b481d7501, 0x24448b4810508b48,
        0x448b4808488b4868, 0x000000a890ff3024, 0x33fffffe69e905eb, 0x8b4868244c8b48d2,
        0xfff4e890ff302444, 0x60244c8b48d233ff, 0xd890ff3024448b48, 0xc358c48348fffff4
    };

    PSTR ClearMessage[2] = {
        "[Shark] < %p > declassified context cleared\n",
        "[Shark] < %p > encrypted context cleared\n"
    };

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > PgBlock\n",
        PgBlock);
#endif // !PUBLIC

    InitializeObjectAttributes(
        &ObjectAttributes,
        &GetGpBlock(PgBlock)->KernelDataTableEntry->FullDllName,
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

    if (TRACE(Status)) {
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

        if (TRACE(Status)) {
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

            if (TRACE(Status)) {
                Diff =
                    (ULONG64)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase -
                    (ULONG64)ViewBase;

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

                        if (0 == PgBlock->SizeCmpAppendDllSection) {
                            if (8 == Length) {
                                if (0 == _CmpByte(TargetPc[0], 0x48) &&
                                    0 == _CmpByte(TargetPc[1], 0x31) &&
                                    0 == _CmpByte(TargetPc[2], 0x84) &&
                                    0 == _CmpByte(TargetPc[3], 0xca)) {
                                    PgBlock->SizeCmpAppendDllSection = *(PULONG)(TargetPc + 4);

#ifndef PUBLIC
                                    DbgPrint(
                                        "[Shark] < %p > SizeCmpAppendDllSection\n",
                                        PgBlock->SizeCmpAppendDllSection);
#endif // !PUBLIC

                                    if (0 == _CmpByte(TargetPc[11], 0x48) ||
                                        0 == _CmpByte(TargetPc[12], 0x0f) ||
                                        0 == _CmpByte(TargetPc[13], 0xbb) ||
                                        0 == _CmpByte(TargetPc[14], 0xc0)) {
                                        PgBlock->BtcEnable = TRUE;

#ifndef PUBLIC
                                        DbgPrint(
                                            "[Shark] < %p > BtcEnable\n",
                                            PgBlock->BtcEnable);
#endif // !PUBLIC
                                    }
                                }
                            }
                        }

                        if (6 == Length) {
                            if (0 == _CmpByte(TargetPc[0], 0x8b) &&
                                0 == _CmpByte(TargetPc[1], 0x82)) {
                                PgBlock->OffsetEntryPoint = *(PULONG)(TargetPc + 2);

#ifndef PUBLIC
                                DbgPrint(
                                    "[Shark] < %p > OffsetEntryPoint\n",
                                    PgBlock->OffsetEntryPoint);
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
                    Header);

                if (NULL != ControlPc) {
                    TargetPc = ControlPc + Diff;

                    FunctionEntry = RtlLookupFunctionEntry(
                        (ULONG64)TargetPc,
                        (PULONG64)&GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase,
                        NULL);

                    if (NULL != FunctionEntry) {
                        TargetPc =
                            (PCHAR)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase +
                            FunctionEntry->BeginAddress - Diff;

                        RtlCopyMemory(
                            PgBlock->Header,
                            TargetPc,
                            sizeof(PgBlock->Header));
                    }

                    NtSection = SectionTableFromVirtualAddress(
                        ViewBase,
                        ControlPc);

                    if (NULL != NtSection) {
                        PgBlock->SizeINITKDBG = max(
                            NtSection->SizeOfRawData,
                            NtSection->Misc.VirtualSize);

#ifndef PUBLIC
                        DbgPrint(
                            "[Shark] < %p > SizeINITKDBG\n",
                            PgBlock->SizeINITKDBG);
#endif // !PUBLIC
                    }
                }

                ControlPc = ViewBase;

                while (NULL != ControlPc) {
                    ControlPc = ScanBytes(
                        ControlPc,
                        (PCHAR)ViewBase + ViewSize,
                        Fields);

                    if (NULL != ControlPc) {
                        TargetPc = ScanBytes(
                            ControlPc,
                            ControlPc + PgBlock->OffsetEntryPoint,
                            FirstField);

                        if (NULL != TargetPc) {
                            PgBlock->Fields[0] =
                                (ULONG64)__RVA_TO_VA(TargetPc - 4) + Diff;

#ifndef PUBLIC
                            FindAndPrintSymbol(
                                "[Shark]",
                                (PVOID)PgBlock->Fields[0]);
#endif // !PUBLIC

                            PgBlock->Fields[1] =
                                (ULONG64)__RVA_TO_VA(TargetPc + 10) + Diff;

#ifndef PUBLIC
                            FindAndPrintSymbol(
                                "[Shark]",
                                (PVOID)PgBlock->Fields[1]);
#endif // !PUBLIC

                            PgBlock->Fields[2] =
                                (ULONG64)__RVA_TO_VA(TargetPc + 24) + Diff;

#ifndef PUBLIC
                            FindAndPrintSymbol(
                                "[Shark]",
                                (PVOID)PgBlock->Fields[2]);
#endif // !PUBLIC

                            PgBlock->Fields[3] =
                                (ULONG64)__RVA_TO_VA(TargetPc + 38) + Diff;

#ifndef PUBLIC
                            FindAndPrintSymbol(
                                "[Shark]",
                                (PVOID)PgBlock->Fields[3]);
#endif // !PUBLIC

                            if (GetGpBlock(PgBlock)->BuildNumber >= 9200) {
                                while (TRUE) {
                                    TargetPc = ScanBytes(
                                        TargetPc,
                                        (PCHAR)ViewBase + ViewSize,
                                        NextField);

                                    TempField = (ULONG64)__RVA_TO_VA(TargetPc + 3) + Diff;

                                    if ((ULONG_PTR)TempField ==
                                        (ULONG_PTR)GetGpBlock(PgBlock)->DbgPrint) {
                                        TempField = (ULONG64)__RVA_TO_VA(TargetPc + 31) + Diff;

                                        RtlCopyMemory(
                                            &PgBlock->MmFreeIndependentPages,
                                            &TempField,
                                            sizeof(PVOID));

#ifndef PUBLIC
                                        DbgPrint(
                                            "[Shark] < %p > MmFreeIndependentPages\n",
                                            PgBlock->MmFreeIndependentPages);
#endif // !PUBLIC

                                        break;
                                    }

                                    TargetPc++;
                                }
                            }

                            while (TRUE) {
                                TargetPc = ScanBytes(
                                    TargetPc,
                                    (PCHAR)ViewBase + ViewSize,
                                    NextField);

                                TempField = (ULONG64)__RVA_TO_VA(TargetPc + 3) + Diff;

                                if ((ULONG_PTR)TempField ==
                                    (ULONG_PTR)GetGpBlock(PgBlock)->PsLoadedModuleList) {
                                    TempField = (ULONG64)__RVA_TO_VA(TargetPc - 11) + Diff;

                                    RtlCopyMemory(
                                        &GetGpBlock(PgBlock)->PsInvertedFunctionTable,
                                        &TempField,
                                        sizeof(PVOID));

#ifndef PUBLIC
                                    DbgPrint(
                                        "[Shark] < %p > PsInvertedFunctionTable\n",
                                        GetGpBlock(PgBlock)->PsInvertedFunctionTable);
#endif // !PUBLIC

                                    break;
                                }

                                TargetPc++;
                            }

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
                    KiStartSystemThread);

                if (NULL != ControlPc) {
                    TargetPc = ControlPc;

                    PgBlock->KiStartSystemThread = (PVOID)(TargetPc + Diff);

#ifndef PUBLIC
                    DbgPrint(
                        "[Shark] < %p > KiStartSystemThread\n",
                        PgBlock->KiStartSystemThread);
#endif // !PUBLIC
                }

                ControlPc = ScanBytes(
                    ViewBase,
                    (PCHAR)ViewBase + ViewSize,
                    PspSystemThreadStartup);

                if (NULL != ControlPc) {
                    TargetPc = (PVOID)(ControlPc + Diff);

                    FunctionEntry = RtlLookupFunctionEntry(
                        (ULONG64)TargetPc,
                        (PULONG64)&GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase,
                        NULL);

                    if (NULL != FunctionEntry) {
                        PgBlock->PspSystemThreadStartup =
                            (PVOID)((PCHAR)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase +
                                FunctionEntry->BeginAddress);

#ifndef PUBLIC
                        DbgPrint(
                            "[Shark] < %p > PspSystemThreadStartup\n",
                            PgBlock->PspSystemThreadStartup);
#endif // !PUBLIC
                    }
                }

                ZwUnmapViewOfSection(ZwCurrentProcess(), ViewBase);
            }

            ZwClose(SectionHandle);
        }

        ZwClose(FileHandle);
    }

    NtHeaders = RtlImageNtHeader(
        (PVOID)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase);

    if (NULL != NtHeaders) {
        NtSection = IMAGE_FIRST_SECTION(NtHeaders);

        ControlPc =
            (PCHAR)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase +
            NtSection[0].VirtualAddress;

        EndToLock =
            (PCHAR)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase +
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
                    TargetPc = __RVA_TO_VA(TargetPc + 14);

                    RtlCopyMemory(
                        (PVOID)&PgBlock->MmDeterminePoolType,
                        &TargetPc,
                        sizeof(PVOID));

#ifndef PUBLIC
                    DbgPrint(
                        "[Shark] < %p > MmDeterminePoolType\n",
                        PgBlock->MmDeterminePoolType);
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
                                        PageTable = (PPOOL_BIG_PAGES *)__RVA_TO_VA(TargetPc + 3);

                                        if (0 == (ULONG64)*PageTable ||
                                            0 != ((ULONG64)(*PageTable) & 0xfff)) {
                                            PageTable = NULL;
                                        }
                                    }
                                    else if (NULL == PageTableSize) {
                                        PageTableSize = (PSIZE_T)__RVA_TO_VA(TargetPc + 3);

                                        if (0 == *PageTableSize ||
                                            0 != ((ULONG64)(*PageTableSize) & 0xfff)) {
                                            PageTableSize = NULL;
                                        }
                                    }
                                }
                                else if (0 == _CmpByte(TargetPc[1], 0x8d)) {
                                    if (NULL == PgBlock->ExpLargePoolTableLock) {
                                        PgBlock->ExpLargePoolTableLock =
                                            (PEX_SPIN_LOCK)__RVA_TO_VA(TargetPc + 3);
                                    }
                                }
                            }

                            if (0 == _CmpByte(TargetPc[0], 0x0f) &&
                                0 == _CmpByte(TargetPc[1], 0x0d) &&
                                0 == _CmpByte(TargetPc[2], 0x0d)) {
                                if (NULL == PgBlock->ExpLargePoolTableLock) {
                                    PgBlock->ExpLargePoolTableLock =
                                        (PEX_SPIN_LOCK)__RVA_TO_VA(TargetPc + 3);
                                }
                            }
                        }

                        if (NULL != PageTable &&
                            NULL != PageTableSize &&
                            NULL != PgBlock->ExpLargePoolTableLock) {
                            if ((ULONG64)*PageTable > (ULONG64)*PageTableSize) {
                                PgBlock->PoolBigPageTable = (PPOOL_BIG_PAGES)*PageTable;
                                PgBlock->PoolBigPageTableSize = (SIZE_T)*PageTableSize;
                            }
                            else {
                                // swap

                                PgBlock->PoolBigPageTable = (PPOOL_BIG_PAGES)*PageTableSize;
                                PgBlock->PoolBigPageTableSize = (SIZE_T)*PageTable;
                            }

#ifndef PUBLIC
                            DbgPrint(
                                "[Shark] < %p > PoolBigPageTable\n",
                                PgBlock->PoolBigPageTable);

                            DbgPrint(
                                "[Shark] < %p > PoolBigPageTableSize\n",
                                PgBlock->PoolBigPageTableSize);

                            DbgPrint(
                                "[Shark] < %p > ExpLargePoolTableLock\n",
                                PgBlock->ExpLargePoolTableLock);
#endif // !PUBLIC

                            break;
                        }

                        TargetPc += Length;
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

    RtlInitUnicodeString(&RoutineString, L"MmAllocateMappingAddress");

    RoutineAddress = MmGetSystemRoutineAddress(&RoutineString);

    if (NULL != RoutineAddress) {
        ControlPc = RoutineAddress;

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
                    TargetPc = __RVA_TO_VA(ControlPc + 3);

                    // struct _MI_SYSTEM_PTE_TYPE *
                    PgBlock->BitMap = TargetPc;

#ifndef PUBLIC
                    DbgPrint(
                        "[Shark] < %p > BitMap\n",
                        PgBlock->BitMap);
#endif // !PUBLIC

                    if (GetGpBlock(PgBlock)->BuildNumber < 9600) {
                        PgBlock->BasePte =
                            *(PMMPTE *)((PCHAR)(PgBlock->BitMap + 1) + sizeof(ULONG) * 2);
                    }
                    else {
                        PgBlock->BasePte =
                            *(PMMPTE *)(PgBlock->BitMap + 1);
                    }

#ifndef PUBLIC
                    DbgPrint(
                        "[Shark] < %p > BasePte\n",
                        PgBlock->BasePte);
#endif // !PUBLIC

                    break;
                }
            }

            ControlPc += Length;
        }
    }

    PgBlock->SizeSdbpCheckDll = sizeof(PgBlock->_SdbpCheckDll);

    PgBlock->SdbpCheckDll = (PVOID)PgBlock->_SdbpCheckDll;

    RtlCopyMemory(
        PgBlock->SdbpCheckDll,
        SdbpCheckDll,
        sizeof(PgBlock->_SdbpCheckDll));

    PgBlock->Btc64 = (PVOID)PgBlock->_Btc64;

    RtlCopyMemory(
        PgBlock->_Btc64,
        Btc64,
        sizeof(PgBlock->_Btc64));

    PgBlock->ClearCallback = (PVOID)PgBlock->_ClearCallback;

    RtlCopyMemory(
        PgBlock->_ClearCallback,
        ClearCallback,
        sizeof(PgBlock->_ClearCallback));

    PgBlock->Message[0] = &PgBlock->_Message[0];

    RtlCopyMemory(
        PgBlock->Message[0],
        ClearMessage[0],
        strlen(ClearMessage[0]));

    PgBlock->Message[1] = &PgBlock->_Message[strlen(ClearMessage[0]) + 1];

    RtlCopyMemory(
        PgBlock->Message[1],
        ClearMessage[1],
        strlen(ClearMessage[1]));
}

PPGOBJECT
NTAPI
PgCreateObject(
    __in_opt CCHAR Encrypted,
    __in CCHAR Type,
    __in PVOID BaseAddress,
    __in_opt SIZE_T RegionSize,
    __in_opt PVOID Context,
    __in PPGBLOCK PgBlock,
    __in PVOID Filter,
    __in PVOID Original,
    __in PVOID CaptureContext
)
{
    PPGOBJECT Object = NULL;

    Object = ExAllocatePool(
        NonPagedPool,
        sizeof(PGOBJECT));

    if (NULL != Object) {
        RtlZeroMemory(Object, sizeof(PGOBJECT));

        Object->Encrypted = Encrypted;
        Object->Type = Type;
        Object->BaseAddress = BaseAddress;
        Object->RegionSize = RegionSize;
        Object->PgBlock = PgBlock;

        SetGuardBody(
            (PGUARD_BODY)&Object->Callback,
            Filter,
            Original,
            Object,
            CaptureContext);
    }

    return Object;
}

VOID
NTAPI
PgSetNewEntry(
    __inout PPGBLOCK PgBlock,
    __in PPGOBJECT Object,
    __in PVOID PatchGuardContext,
    __in ULONG64 RorKey
)
{
    ULONG64 LastRorKey = 0;
    ULONG RvaOfEntry = 0;
    ULONG64 FieldBuffer[PG_COMPARE_FIELDS_COUNT] = { 0 };
    ULONG FieldIndex = 0;
    ULONG Index = 0;
    PVOID Pointer = NULL;

    // xor code must be align 8 byte;
    // get PatchGuard entry offset in encrypted code

    FieldIndex = (PgBlock->OffsetEntryPoint -
        PgBlock->SizeCmpAppendDllSection) / sizeof(ULONG64);

    RtlCopyMemory(
        FieldBuffer,
        (PCHAR)PatchGuardContext + (PgBlock->OffsetEntryPoint & ~7),
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

    RvaOfEntry = *(PULONG)((PCHAR)FieldBuffer + (PgBlock->OffsetEntryPoint & 7));

    // copy PatchGuard entry head code to temp bufer and decode

    FieldIndex = (RvaOfEntry - PgBlock->SizeCmpAppendDllSection) / sizeof(ULONG64);

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

    Pointer = (PCHAR)FieldBuffer + (RvaOfEntry & 7);

    DetourLockedBuildJumpCode(
        &Pointer,
        &Object->Callback);

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

    InterlockedIncrementSizeT(&PgBlock->ReferenceCount);

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > set new entry for encrypted context\n",
        PgBlock->ReferenceCount);
#endif // !PUBLIC
}

VOID
NTAPI
PgSetNewEntryWithBtc(
    __inout PPGBLOCK PgBlock,
    __in PPGOBJECT Object,
    __in PVOID Context,
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
    PVOID Pointer = NULL;

    CompareCount = (ContextSize - PgBlock->SizeCmpAppendDllSection) / 8 - 1;

    for (AlignOffset = 0;
        AlignOffset < 8;
        AlignOffset++) {
        ControlPc = (PULONG64)((PCHAR)PgBlock->Header + AlignOffset);
        TargetPc = (PULONG64)((PCHAR)Context + PgBlock->SizeCmpAppendDllSection);

        for (Index = 0;
            Index < CompareCount;
            Index++) {
            RorKey = TargetPc[Index + 1] ^ ControlPc[1];
            LastRorKey = RorKey;
            RorKey = __ROR64(RorKey, Index + 1);
            RorKey = PgBlock->Btc64(RorKey, RorKey);

            if ((TargetPc[Index] ^ RorKey) == ControlPc[0]) {
                goto found;
            }
        }
    }

found:
    FieldIndex = Index - (0 == (AlignOffset & 7) ? 0 : 1);
    LastRorKeyOffset = 2 + (0 == (AlignOffset & 7) ? 0 : 1);

    RtlCopyMemory(
        FieldBuffer,
        (PCHAR)Context + PgBlock->SizeCmpAppendDllSection + FieldIndex * 8,
        sizeof(FieldBuffer));

    RorKey = LastRorKey;
    Index = LastRorKeyOffset;

    while (Index--) {
        FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
        RorKey = __ROR64(RorKey, FieldIndex + Index);
        RorKey = PgBlock->Btc64(RorKey, RorKey);
    }

    // set temp buffer PatchGuard entry head jmp to _ClearEncryptedContext and encrypt

    Pointer = (PDETOUR_BODY)((PCHAR)FieldBuffer + 8 - AlignOffset);

    DetourLockedBuildJumpCode(
        &Pointer,
        &Object->Callback);

    RorKey = LastRorKey;
    Index = LastRorKeyOffset;

    while (Index--) {
        FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
        RorKey = __ROR64(RorKey, FieldIndex + Index);
        RorKey = PgBlock->Btc64(RorKey, RorKey);
    }

    RtlCopyMemory(
        (PCHAR)Context + PgBlock->SizeCmpAppendDllSection + FieldIndex * 8,
        FieldBuffer,
        sizeof(FieldBuffer));

    InterlockedIncrementSizeT(&PgBlock->ReferenceCount);

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > set new entry for btc encrypted context\n",
        PgBlock->ReferenceCount);
#endif // !PUBLIC
}

VOID
NTAPI
PgCompareFields(
    __inout PPGBLOCK PgBlock,
    __in CCHAR Type,
    __in PVOID BaseAddress,
    __in SIZE_T RegionSize
)
{
    PVOID EndAddress = NULL;
    PCHAR TargetPc = NULL;
    SIZE_T Index = 0;
    ULONG64 RorKey = 0;
    ULONG RorKeyIndex = 0;
    PULONG64 Fields = NULL;
    PVOID Context = NULL;
    PPGOBJECT Object = NULL;

    TargetPc = BaseAddress;
    EndAddress = (PCHAR)BaseAddress + RegionSize - sizeof(PgBlock->Fields);

    do {
        Fields = TargetPc;

        if ((ULONG64)Fields == (ULONG64)PgBlock->Fields) {
            break;
        }

        RorKey = Fields[3] ^ PgBlock->Fields[3];

        // CmpAppendDllSection + 98

        // xor [rdx + rcx * 8 + 0c0h], rax
        // ror rax, cl

        // if >= win10 17134 btc rax, rax here

        // loop CmpAppendDllSection + 98

        if (0 == RorKey) {
            if (Fields[2] == PgBlock->Fields[2] &&
                Fields[1] == PgBlock->Fields[1] &&
                Fields[0] == PgBlock->Fields[0]) {
                Context = TargetPc - PG_FIRST_FIELD_OFFSET;

#ifndef PUBLIC
                DbgPrint(
                    "[Shark] < %p > found declassified context\n",
                    Context);
#endif // !PUBLIC
                break;
            }
        }
        else {
            RorKeyIndex = 0;

            RorKey = __ROR64(RorKey, PG_FIELD_BITS);

            if (PgBlock->BtcEnable) {
                RorKey = PgBlock->Btc64(RorKey, RorKey);
            }

            RorKeyIndex++;

            if ((ULONG64)(Fields[2] ^ RorKey) == (ULONG64)PgBlock->Fields[2]) {
                RorKey = __ROR64(RorKey, PG_FIELD_BITS - RorKeyIndex);

                if (PgBlock->BtcEnable) {
                    RorKey = PgBlock->Btc64(RorKey, RorKey);
                }

                RorKeyIndex++;

                if ((ULONG64)(Fields[1] ^ RorKey) == (ULONG64)PgBlock->Fields[1]) {
                    RorKey = __ROR64(RorKey, PG_FIELD_BITS - RorKeyIndex);

                    if (PgBlock->BtcEnable) {
                        RorKey = PgBlock->Btc64(RorKey, RorKey);
                    }

                    RorKeyIndex++;

                    if ((ULONG64)(Fields[0] ^ RorKey) == (ULONG64)PgBlock->Fields[0]) {
                        Context = TargetPc - PG_FIRST_FIELD_OFFSET;

                        RorKey = Fields[0] ^ PgBlock->Fields[0];

#ifndef PUBLIC
                        DbgPrint(
                            "[Shark] < %p > found encrypted context < %p - %08x >\n",
                            Context,
                            BaseAddress,
                            (PCHAR)BaseAddress + RegionSize);
#endif // !PUBLIC

                        Object = PgCreateObject(
                            PgEncrypted,
                            Type,
                            BaseAddress,
                            RegionSize,
                            Context,
                            PgBlock,
                            PgBlock->ClearCallback,
                            NULL,
                            GetGpBlock(PgBlock)->CaptureContext);

                        if (NULL != Object) {
                            if (FALSE != PgBlock->BtcEnable) {
                                PgSetNewEntryWithBtc(
                                    PgBlock,
                                    Object,
                                    Context,
                                    (ULONG_PTR)EndAddress - (ULONG_PTR)Context);
                            }
                            else {
                                for (;
                                    RorKeyIndex < PG_FIELD_BITS;
                                    RorKeyIndex++) {
                                    RorKey = __ROR64(RorKey, PG_FIELD_BITS - RorKeyIndex);
                                }

#ifndef PUBLIC
                                DbgPrint(
                                    "[Shark] < %p > first rorkey\n",
                                    RorKey);
#endif // !PUBLIC
                                PgSetNewEntry(
                                    PgBlock,
                                    Object,
                                    Context,
                                    RorKey);
                            }
                        }

                        break;
                    }
                }
            }
        }

        TargetPc++;
    } while ((ULONG64)TargetPc < (ULONG64)EndAddress);
}

VOID
NTAPI
InitializeSystemPtesBitMap(
    __inout PPGBLOCK PgBlock,
    __in PRTL_BITMAP BitMap,
    __in PFN_NUMBER NumberOfPtes
)
{
    PMMPTE PointerPxe = NULL;
    PMMPTE PointerPpe = NULL;
    PMMPTE PointerPde = NULL;
    PMMPTE PointerPte = NULL;
    PVOID PointerAddress = NULL;
    ULONG BitNumber = 0;
    PVOID BeginAddress = NULL;
    PVOID EndAddress = NULL;

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
            ( MM_PTE_VALID_MASK | MM_PTE_DIRTY_MASK | MM_PTE_WRITE_MASK | MM_PTE_ACCESS_MASK)

#define VALID_PTE_UNSET_BITS \
            ( MM_PTE_WRITE_THROUGH_MASK | MM_PTE_CACHE_DISABLE_MASK | MM_PTE_COPY_ON_WRITE_MASK )

    NumberOfPtes = PgBlock->BitMap->SizeOfBitMap;

    BeginAddress =
        GetVirtualAddressMappedByPte(
            PgBlock->BasePte);

    EndAddress =
        GetVirtualAddressMappedByPte(
            PgBlock->BasePte + NumberOfPtes);

    PointerAddress = BeginAddress;

    do {
        PointerPxe = GetPxeAddress(PointerAddress);

        if (0 != PointerPxe->u.Hard.Valid) {
            PointerPpe = GetPpeAddress(PointerAddress);

            if (0 != PointerPpe->u.Hard.Valid) {
                PointerPde = GetPdeAddress(PointerAddress);

                if (0 != PointerPde->u.Hard.Valid) {
                    if (0 == PointerPde->u.Hard.LargePage) {
                        PointerPte = GetPteAddress(PointerAddress);

                        if (0 != PointerPte->u.Hard.Valid) {
                            if (0 == PointerPte->u.Hard.NoExecute) {
                                if (VALID_PTE_SET_BITS == (PointerPte->u.Long & VALID_PTE_SET_BITS)) {
                                    if (0 == (PointerPte->u.Long & VALID_PTE_UNSET_BITS)) {
                                        BitNumber = PointerPte - PgBlock->BasePte;

                                        RtlSetBit(BitMap, BitNumber);
                                    }
                                }
                            }
                        }

                        PointerAddress = GetVirtualAddressMappedByPte(PointerPte + 1);
                    }
                    else {
                        PointerAddress = GetVirtualAddressMappedByPde(PointerPde + 1);
                    }
                }
                else {
                    PointerAddress = GetVirtualAddressMappedByPde(PointerPde + 1);
                }
            }
            else {
                PointerAddress = GetVirtualAddressMappedByPpe(PointerPpe + 1);
            }
        }
        else {
            PointerAddress = GetVirtualAddressMappedByPxe(PointerPxe + 1);
        }
    } while ((ULONG_PTR)PointerAddress < (ULONG_PTR)EndAddress);
}

VOID
NTAPI
PgClearSystemPtesEncryptedContext(
    __inout PPGBLOCK PgBlock
)
{
    PRTL_BITMAP BitMap = NULL;
    ULONG BitMapSize = 0;
    PFN_NUMBER NumberOfPtes = 0;
    ULONG HintIndex = 0;
    ULONG StartingIndex = 0;

    NumberOfPtes = PgBlock->BitMap->SizeOfBitMap;

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > SystemPtes < %p | %p >\n",
        KeGetCurrentProcessorNumber(),
        PgBlock->BasePte,
        PgBlock->BasePte + NumberOfPtes);
#endif // !PUBLIC

    BitMapSize =
        sizeof(RTL_BITMAP) + (ULONG)((((NumberOfPtes + 1) + 31) / 32) * 4);

    BitMap = ExAllocatePool(NonPagedPool, BitMapSize);

    if (NULL != BitMap) {
        RtlInitializeBitMap(
            BitMap,
            (PULONG)(BitMap + 1),
            (ULONG)(NumberOfPtes + 1));

        RtlClearAllBits(BitMap);

        InitializeSystemPtesBitMap(
            PgBlock,
            BitMap,
            NumberOfPtes);

        do {
            HintIndex = RtlFindSetBits(
                BitMap,
                BYTES_TO_PAGES(PgBlock->SizeINITKDBG),
                HintIndex);

            if (MAXULONG != HintIndex) {
                StartingIndex = HintIndex;

                HintIndex = RtlFindClearBits(
                    BitMap,
                    1,
                    StartingIndex);

                RtlClearBits(BitMap, StartingIndex, HintIndex - StartingIndex);

                PgCompareFields(
                    PgBlock,
                    PgSystemPtes,
                    GetVirtualAddressMappedByPte(PgBlock->BasePte + StartingIndex),
                    (HintIndex - StartingIndex) * PAGE_SIZE);
            }
        } while (HintIndex < NumberOfPtes);

        ExFreePool(BitMap);
    }
}

VOID
NTAPI
PgClearPoolEncryptedContext(
    __inout PPGBLOCK PgBlock,
    __in BOOLEAN AllProcesors
)
{
    SIZE_T Index = 0;
    SIZE_T PoolBigPageTableSize = 0;
    PPOOL_BIG_PAGES PoolBigPageTable = NULL;

    if (FALSE != AllProcesors) {
        PoolBigPageTableSize =
            PgBlock->PoolBigPageTableSize / GetGpBlock(PgBlock)->NumberProcessors;

        PoolBigPageTable =
            PgBlock->PoolBigPageTable + PoolBigPageTableSize * KeGetCurrentProcessorNumber();
    }
    else {
        PoolBigPageTableSize = PgBlock->PoolBigPageTableSize;
        PoolBigPageTable = PgBlock->PoolBigPageTable;
    }

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > BigPool < %p | %p >\n",
        KeGetCurrentProcessorNumber(),
        PoolBigPageTable,
        PoolBigPageTableSize);
#endif // !PUBLIC

    for (Index = 0;
        Index < PoolBigPageTableSize;
        Index++) {
        if (POOL_BIG_TABLE_ENTRY_FREE !=
            ((ULONG64)PoolBigPageTable[Index].Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            if (NonPagedPool == PgBlock->MmDeterminePoolType(PoolBigPageTable[Index].Va)) {
                if (PoolBigPageTable[Index].NumberOfPages > PgBlock->SizeINITKDBG) {
                    PgCompareFields(
                        PgBlock,
                        PgPoolBigPage,
                        PoolBigPageTable[Index].Va,
                        PoolBigPageTable[Index].NumberOfPages);
                }
            }
        }
    }
}

VOID
NTAPI
PgLocateSystemPtesObject(
    __inout PPGBLOCK PgBlock,
    __in PPGOBJECT Object
)
{
    PRTL_BITMAP BitMap = NULL;
    ULONG BitMapSize = 0;
    PFN_NUMBER NumberOfPtes = 0;
    ULONG HintIndex = 0;
    ULONG StartingIndex = 0;
    PVOID Establisher = NULL;

    NumberOfPtes = PgBlock->BitMap->SizeOfBitMap;

    GetCounterBody(&Object->Establisher, &Establisher);

    BitMapSize =
        sizeof(RTL_BITMAP) +
        (ULONG)((((NumberOfPtes + 1) + 31) / 32) * 4);

    BitMap = ExAllocatePool(NonPagedPool, BitMapSize);

    if (NULL != BitMap) {
        RtlInitializeBitMap(
            BitMap,
            (PULONG)(BitMap + 1),
            (ULONG)(NumberOfPtes + 1));

        RtlClearAllBits(BitMap);

        InitializeSystemPtesBitMap(
            PgBlock,
            BitMap,
            NumberOfPtes);

        do {
            HintIndex = RtlFindSetBits(
                BitMap,
                BYTES_TO_PAGES(PgBlock->SizeINITKDBG),
                HintIndex);

            if (MAXULONG != HintIndex) {
                StartingIndex = HintIndex;

                HintIndex = RtlFindClearBits(
                    BitMap,
                    1,
                    StartingIndex);

                RtlClearBits(BitMap, StartingIndex, HintIndex - StartingIndex);

                if ((ULONG64)Establisher >=
                    (ULONG64)GetVirtualAddressMappedByPte(
                        PgBlock->BasePte + StartingIndex) &&
                        (ULONG64)Establisher <
                    (ULONG64)GetVirtualAddressMappedByPte(
                        PgBlock->BasePte + HintIndex) - PgBlock->SizeCmpAppendDllSection) {
                    Object->BaseAddress =
                        GetVirtualAddressMappedByPte(PgBlock->BasePte + StartingIndex);

                    Object->RegionSize =
                        (SIZE_T)(HintIndex - StartingIndex)* PAGE_SIZE;

#ifndef PUBLIC
                    DbgPrint(
                        "[Shark] < %p > found region in system ptes < %p - %08x >\n",
                        Establisher,
                        Object->BaseAddress,
                        Object->RegionSize);
#endif // !PUBLIC

                    Object->Type = PgSystemPtes;

                    break;
                }
            }
        } while (HintIndex < NumberOfPtes);

        ExFreePool(BitMap);
    }
}

VOID
NTAPI
PgLocateObject(
    __inout PPGBLOCK PgBlock,
    __out PPGOBJECT Object
)
{
    PFN_NUMBER Index = 0;
    KIRQL Irql = PASSIVE_LEVEL;
    PVOID Establisher = NULL;

    GetCounterBody(&Object->Establisher, &Establisher);

    Irql = GetGpBlock(PgBlock)->ExAcquireSpinLockShared(PgBlock->ExpLargePoolTableLock);

    for (Index = 0;
        Index < PgBlock->PoolBigPageTableSize;
        Index++) {
        if (POOL_BIG_TABLE_ENTRY_FREE !=
            ((ULONG64)PgBlock->PoolBigPageTable[Index].Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            if (NonPagedPool == PgBlock->MmDeterminePoolType(PgBlock->PoolBigPageTable[Index].Va)) {
                if (PgBlock->PoolBigPageTable[Index].NumberOfPages > PgBlock->SizeINITKDBG) {
                    if ((ULONG64)Establisher >= (ULONG64)PgBlock->PoolBigPageTable[Index].Va &&
                        (ULONG64)Establisher < (ULONG64)PgBlock->PoolBigPageTable[Index].Va +
                        PgBlock->PoolBigPageTable[Index].NumberOfPages) {
                        Object->BaseAddress = PgBlock->PoolBigPageTable[Index].Va;
                        Object->RegionSize = PgBlock->PoolBigPageTable[Index].NumberOfPages;

#ifndef PUBLIC
                        DbgPrint(
                            "[Shark] < %p > found region in pool < %p - %08x >\n",
                            Establisher,
                            Object->BaseAddress,
                            Object->RegionSize);
#endif // !PUBLIC

                        Object->Type = PgPoolBigPage;

                        break;
                    }
                }
            }
        }
    }

    GetGpBlock(PgBlock)->ExReleaseSpinLockShared(PgBlock->ExpLargePoolTableLock, Irql);

    if (-1 == Object->Type) {
        IpiSingleCall(
            (PPS_APC_ROUTINE)NULL,
            (PKSYSTEM_ROUTINE)PgLocateSystemPtesObject,
            (PUSER_THREAD_START_ROUTINE)PgBlock,
            (PVOID)Object);
    }
}

VOID
NTAPI
PgCheckWorkerThread(
    __inout PPGBLOCK PgBlock
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
    PVOID * Parameters = NULL;
    PGOBJECT TempObject = { 0 };
    PPGOBJECT Object = NULL;

    Callers = ExAllocatePool(
        NonPagedPool,
        MAX_STACK_DEPTH * sizeof(CALLERS));

    if (NULL != Callers) {
        Count = WalkFrameChain(
            Callers,
            MAX_STACK_DEPTH);

        if (0 != Count) {
            // PrintFrameChain(Callers, 0, Count);

            IoGetStackLimits(&LowLimit, &HighLimit);

            InitialStack = IoGetInitialStack();

            // all worker thread start at KiStartSystemThread and return address == 0
            // if null != last return address code is in noimage

            if (NULL != Callers[Count - 1].Establisher) {
#ifndef PUBLIC
                DbgPrint(
                    "[Shark] < %p > found noimage return address in worker thread\n",
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

                        SetCounterBody(
                            &TempObject.Establisher,
                            Callers[Count - 1].Establisher);

                        TempObject.Type = -1;

                        PgLocateObject(PgBlock, &TempObject);

                        if (-1 != TempObject.Type) {
                            Object = PgCreateObject(
                                PgDeclassified,
                                TempObject.Type,
                                TempObject.BaseAddress,
                                TempObject.RegionSize,
                                NULL,
                                PgBlock,
                                PgBlock->ClearCallback,
                                Callers[Count - 1].Establisher,
                                GetGpBlock(PgBlock)->CaptureContext);

                            if (NULL != Object) {
                                SetCounterBody(
                                    &Object->Establisher,
                                    Callers[Count - 1].Establisher);

                                *TargetPc = (ULONG64)&Object->Establisher;

                                InterlockedIncrementSizeT(&PgBlock->ReferenceCount);

#ifndef PUBLIC
                                DbgPrint(
                                    "[Shark] < %p > insert worker thread check code\n",
                                    PgBlock->ReferenceCount);
#endif // !PUBLIC
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
PgCheckAllWorkerThread(
    __inout PPGBLOCK PgBlock
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
    DISPATCHER_HEADER * Header = NULL;

    /*
    if (os build >= 9600){
        PgBlock->WorkerContext = struct _DISPATCHER_HEADER

            Header->Type = 0x15
            Header->Hand = 0xac
    }
    else {
        PgBlock->WorkerContext = enum _WORK_QUEUE_TYPE

            CriticalWorkQueue = 0
            DelayedWorkQueue = 1
    }
    */

    InitialStack = IoGetInitialStack();

    if (GetGpBlock(PgBlock)->BuildNumber >= 9600) {
        while ((ULONG64)InitialStack != (ULONG64)&PgBlock) {
            Header = *(PVOID *)InitialStack;

            if (FALSE != MmIsAddressValid(Header)) {
                if (FALSE != MmIsAddressValid((PCHAR)(Header + 1) - 1)) {
                    if (0x15 == Header->Type &&
                        0xac == Header->Hand) {
                        PgBlock->WorkerContext = Header;

                        break;
                    }
                }
            }

            InitialStack--;
        }
    }
    else {
        PgBlock->WorkerContext = UlongToPtr(CriticalWorkQueue);
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

                    if (NULL == PgBlock->ExpWorkerThread) {
                        for (Index = 0;
                            Index < ProcessInfo->NumberOfThreads;
                            Index++) {
                            if ((ULONG64)PsGetCurrentThreadId() ==
                                (ULONG64)ThreadInfo[Index].ThreadInfo.ClientId.UniqueThread) {
                                PgBlock->ExpWorkerThread = ThreadInfo[Index].Win32StartAddress;

                                break;
                            }
                        }
                    }

                    if (NULL != PgBlock->ExpWorkerThread) {
                        for (Index = 0;
                            Index < ProcessInfo->NumberOfThreads;
                            Index++) {
                            if ((ULONG64)PsGetCurrentThreadId() !=
                                (ULONG64)ThreadInfo[Index].ThreadInfo.ClientId.UniqueThread &&
                                (ULONG64)PgBlock->ExpWorkerThread ==
                                (ULONG64)ThreadInfo[Index].Win32StartAddress) {
                                AsyncCall(
                                    ThreadInfo[Index].ThreadInfo.ClientId.UniqueThread,
                                    NULL,
                                    NULL,
                                    (PUSER_THREAD_START_ROUTINE)PgCheckWorkerThread,
                                    PgBlock);
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
PgClearWorker(
    __inout PVOID Argument
)
{
    struct {
        PPGBLOCK PgBlock;
        KEVENT Notify;
        WORK_QUEUE_ITEM Worker;
    }*Context;

    Context = Argument;

    IpiSingleCall(
        (PPS_APC_ROUTINE)NULL,
        (PKSYSTEM_ROUTINE)NULL,
        (PUSER_THREAD_START_ROUTINE)PgClearSystemPtesEncryptedContext,
        (PVOID)Context->PgBlock);

    IpiGenericCall(
        (PPS_APC_ROUTINE)NULL,
        (PKSYSTEM_ROUTINE)PgClearPoolEncryptedContext,
        (PUSER_THREAD_START_ROUTINE)Context->PgBlock,
        (PVOID)TRUE);

    PgCheckAllWorkerThread(Context->PgBlock);

    KeSetEvent(&Context->Notify, LOW_PRIORITY, FALSE);
}

VOID
NTAPI
PgClear(
    __inout PPGBLOCK PgBlock
)
{
    struct {
        PPGBLOCK PgBlock;
        KEVENT Notify;
        WORK_QUEUE_ITEM Worker;
    }Context = { 0 };

    Context.PgBlock = PgBlock;

    InitializePgBlock(Context.PgBlock);

    if (RTL_SOFT_ASSERTMSG(
        "[Shark] PgBlock->OffsetEntryPoint not found",
        0 != PgBlock->OffsetEntryPoint) ||
        RTL_SOFT_ASSERTMSG(
            "[Shark] PgBlock->SizeCmpAppendDllSection not found",
            0 != PgBlock->SizeCmpAppendDllSection) ||
        RTL_SOFT_ASSERTMSG(
            "[Shark] PgBlock->SizeINITKDBG not found",
            0 != PgBlock->SizeINITKDBG) ||
        RTL_SOFT_ASSERTMSG(
            "[Shark] PgBlock->PoolBigPageTable not found",
            NULL != PgBlock->PoolBigPageTable) ||
        RTL_SOFT_ASSERTMSG(
            "[Shark] PgBlock->PoolBigPageTableSize not found",
            0 != PgBlock->PoolBigPageTableSize) ||
        RTL_SOFT_ASSERTMSG(
            "[Shark] PgBlock->ExpLargePoolTableLock not found",
            NULL != PgBlock->ExpLargePoolTableLock) ||
        RTL_SOFT_ASSERTMSG(
            "[Shark] PgBlock->BitMap not found",
            NULL != PgBlock->BitMap) ||
        RTL_SOFT_ASSERTMSG(
            "[Shark] PgBlock->BasePte not found",
            NULL != PgBlock->BasePte) ||
        RTL_SOFT_ASSERTMSG(
            "[Shark] PgBlock->MmDeterminePoolType not found",
            NULL != PgBlock->MmDeterminePoolType)) {

        KeInitializeEvent(&Context.Notify, SynchronizationEvent, FALSE);
        ExInitializeWorkItem(&Context.Worker, PgClearWorker, &Context);
        ExQueueWorkItem(&Context.Worker, CriticalWorkQueue);

        KeWaitForSingleObject(
            &Context.Notify,
            Executive,
            KernelMode,
            FALSE,
            NULL);
    }
}
