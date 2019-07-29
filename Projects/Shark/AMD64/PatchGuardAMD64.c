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

#include "PatchGuard.h"

#include "Ctx.h"
#include "Detours.h"
#include "Rtx.h"
#include "Scan.h"
#include "Space.h"
#include "Stack.h"

#define __ROL64(x, n) (((x) << ((( n & 0xFF) % 64))) | ((x) >> (64 - (( n & 0xFF) % 64))))
#define __ROR64(x, n) (((x) >> ((( n & 0xFF) % 64))) | ((x) << (64 - (( n & 0xFF) % 64))))

#define GetGpBlock(pgblock) \
            ((PGPBLOCK)((PCHAR)pgblock - sizeof(GPBLOCK)))

VOID
NTAPI
PgClearCallback(
    __in PCONTEXT Context,
    __in_opt PVOID ProgramCounter,
    __in_opt PPGOBJECT Object,
    __in_opt PPGBLOCK PgBlock
)
{
    PETHREAD Thread = NULL;
    PKSTART_FRAME StartFrame = NULL;
    PKSWITCH_FRAME SwitchFrame = NULL;
    PKTRAP_FRAME TrapFrame = NULL;
    ULONG Index = 0;

    if (NULL != ProgramCounter) {
        // protected code for donate version
    }
    else {
        GetCounterBody(
            &Object->Body.Reserved,
            &PgBlock);

#ifndef PUBLIC
        GetGpBlock(PgBlock)->DbgPrint(
            PgBlock->Message[Object->Encrypted],
            Object);
#endif // !PUBLIC

        if (PgEncrypted == Object->Encrypted) {
            Context->Rip = UnsafeReadPointer(Context->Rsp + KSTART_FRAME_LENGTH);
            Context->Rsp += KSTART_FRAME_LENGTH + sizeof(PVOID);

            if (PgPoolBigPage == Object->Type) {
                GetGpBlock(PgBlock)->ExFreePoolWithTag(
                    Object->BaseAddress,
                    0);
            }
            else if (PgSystemPtes == Object->Type) {
                PgBlock->MmFreeIndependentPages(
                    Object->BaseAddress,
                    Object->RegionSize);
            }
        }
        else {
            for (Index = 0;
                Index < Object->RegionSize - PgBlock->SizeSdbpCheckDll;
                Index++) {
                if (PgBlock->SizeSdbpCheckDll == GetGpBlock(PgBlock)->RtlCompareMemory(
                    (PCHAR)Object->BaseAddress + Index,
                    PgBlock->_SdbpCheckDll,
                    PgBlock->SizeSdbpCheckDll)) {
                    Thread = (PETHREAD)__readgsqword(FIELD_OFFSET(KPCR, Prcb.CurrentThread));

                    // protected code for donate version

                    StartFrame =
                        (PKSTART_FRAME)(
                        (*(PULONG64)((ULONG64)Thread +
                            GetGpBlock(PgBlock)->DebuggerDataBlock.OffsetKThreadInitialStack)) -
                            KSTART_FRAME_LENGTH);

                    SwitchFrame = (PKSWITCH_FRAME)((ULONG64)StartFrame - KSWITCH_FRAME_LENGTH);

                    StartFrame->P1Home = (ULONG64)PgBlock->WorkerContext;
                    StartFrame->P2Home = (ULONG64)PgBlock->ExpWorkerThread;
                    StartFrame->P3Home = (ULONG64)PgBlock->PspSystemThreadStartup;
                    StartFrame->Return = 0;
                    SwitchFrame->Return = (ULONG64)PgBlock->KiStartSystemThread;
                    SwitchFrame->ApcBypass = APC_LEVEL;
                    SwitchFrame->Rbp = (ULONG64)TrapFrame + FIELD_OFFSET(KTRAP_FRAME, Xmm1);

                    Context->Rsp = (ULONG64)StartFrame;
                    Context->Rip = (ULONG64)PgBlock->KiStartSystemThread;

                    if (PgPoolBigPage == Object->Type) {
                        GetGpBlock(PgBlock)->ExFreePoolWithTag(
                            Object->BaseAddress,
                            0);
                    }
                    else if (PgSystemPtes == Object->Type) {
                        PgBlock->MmFreeIndependentPages(
                            Object->BaseAddress,
                            Object->RegionSize);
                    }

                    break;
                }
            }
        }

        GetGpBlock(PgBlock)->ExFreePoolWithTag(Object, 0);
    }

    GetGpBlock(PgBlock)->RtlRestoreContext(Context, NULL);
}

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
    PVOID EndToLock = NULL;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    LONG64 Diff = 0;
    UNICODE_STRING RoutineString = { 0 };
    PVOID RoutineAddress = NULL;

    PPOOL_BIG_PAGES * PageTable = NULL;
    PSIZE_T PageTableSize = NULL;
    PRTL_BITMAP BitMap = NULL;

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

    CHAR HeaderEx[] =
        "55 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC B0 00 00 00 8B 82";

    ULONG64 SdbpCheckDll[] = {
        0x7c8b483024748b48, 0x333824548b4c2824,
        0x08ea8349028949c0, 0x7c8948f473d43b4c,
        0xe88bf88bd88b2824, 0x8b4cd88b4cd08b4c,
        0x4cf08b4ce88b4ce0, 0xcccccccce6fff88b
    };

    CHAR MiGetSystemRegionType[] =
        "48 b8 00 00 00 00 00 80 ff ff 48 3b c8 72 1c 48 c1 e9 27 81 e1 ff 01 00 00 8d 81 00 ff ff ff";

    ULONG64 Btc64[] = {
        0xc3c18b48d1bb0f48
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

    InitializeListHead(&PgBlock->List);
    KeInitializeSpinLock(&PgBlock->Lock);

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

                if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
                    ControlPc = ScanBytes(
                        ViewBase,
                        (PCHAR)ViewBase + ViewSize,
                        HeaderEx);
                }
                else {
                    ControlPc = ScanBytes(
                        ViewBase,
                        (PCHAR)ViewBase + ViewSize,
                        Header);
                }

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
                                (ULONG64)RvaToVa(TargetPc - 4) + Diff;

#ifndef PUBLIC
                            FindAndPrintSymbol(
                                "[Shark]",
                                (PVOID)PgBlock->Fields[0]);
#endif // !PUBLIC

                            PgBlock->Fields[1] =
                                (ULONG64)RvaToVa(TargetPc + 10) + Diff;

#ifndef PUBLIC
                            FindAndPrintSymbol(
                                "[Shark]",
                                (PVOID)PgBlock->Fields[1]);
#endif // !PUBLIC

                            PgBlock->Fields[2] =
                                (ULONG64)RvaToVa(TargetPc + 24) + Diff;

#ifndef PUBLIC
                            FindAndPrintSymbol(
                                "[Shark]",
                                (PVOID)PgBlock->Fields[2]);
#endif // !PUBLIC

                            PgBlock->Fields[3] =
                                (ULONG64)RvaToVa(TargetPc + 38) + Diff;

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

                                    TempField = (ULONG64)RvaToVa(TargetPc + 3) + Diff;

                                    if ((ULONG_PTR)TempField ==
                                        (ULONG_PTR)GetGpBlock(PgBlock)->DbgPrint) {
                                        TempField = (ULONG64)RvaToVa(TargetPc + 31) + Diff;

                                        RtlCopyMemory(
                                            &PgBlock->MmFreeIndependentPages,
                                            &TempField,
                                            sizeof(PVOID));

#ifndef PUBLIC
                                        DbgPrint(
                                            "[Shark] < %p > MmFreeIndependentPages\n",
                                            PgBlock->MmFreeIndependentPages);
#endif // !PUBLIC

                                        TempField = (ULONG64)RvaToVa(TargetPc + 45) + Diff;

                                        RtlCopyMemory(
                                            &PgBlock->MmSetPageProtection,
                                            &TempField,
                                            sizeof(PVOID));

#ifndef PUBLIC
                                        DbgPrint(
                                            "[Shark] < %p > MmSetPageProtection\n",
                                            PgBlock->MmSetPageProtection);
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

                                TempField = (ULONG64)RvaToVa(TargetPc + 3) + Diff;

                                if ((ULONG_PTR)TempField ==
                                    (ULONG_PTR)GetGpBlock(PgBlock)->PsLoadedModuleList) {
                                    TempField = (ULONG64)RvaToVa(TargetPc - 11) + Diff;

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
                    TargetPc = RvaToVa(TargetPc + 14);

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
                                    if (NULL == PgBlock->ExpLargePoolTableLock) {
                                        PgBlock->ExpLargePoolTableLock =
                                            (PEX_SPIN_LOCK)RvaToVa(TargetPc + 3);
                                    }
                                }
                            }

                            if (0 == _CmpByte(TargetPc[0], 0x0f) &&
                                0 == _CmpByte(TargetPc[1], 0x0d) &&
                                0 == _CmpByte(TargetPc[2], 0x0d)) {
                                if (NULL == PgBlock->ExpLargePoolTableLock) {
                                    PgBlock->ExpLargePoolTableLock =
                                        (PEX_SPIN_LOCK)RvaToVa(TargetPc + 3);
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
                    TargetPc = RvaToVa(ControlPc + 3);

                    // struct _MI_SYSTEM_PTE_TYPE *
                    BitMap = TargetPc;

                    PgBlock->NumberOfPtes = BitMap->SizeOfBitMap * 8;
#ifndef PUBLIC
                    DbgPrint(
                        "[Shark] < %p > NumberOfPtes\n",
                        PgBlock->NumberOfPtes);
#endif // !PUBLIC

                    if (GetGpBlock(PgBlock)->BuildNumber < 9600) {
                        PgBlock->BasePte =
                            *(PMMPTE *)((PCHAR)(BitMap + 1) + sizeof(ULONG) * 2);
                    }
                    else {
                        PgBlock->BasePte = *(PMMPTE *)(BitMap + 1);
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

    RtlInitUnicodeString(&RoutineString, L"KeBugCheckEx");

    PgBlock->KeBugCheckEx = MmGetSystemRoutineAddress(&RoutineString);

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > KeBugCheckEx\n",
        PgBlock->KeBugCheckEx);
#endif // !PUBLIC

    RtlInitUnicodeString(&RoutineString, L"RtlLookupFunctionEntry");

    PgBlock->RtlLookupFunctionEntry = MmGetSystemRoutineAddress(&RoutineString);

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > RtlLookupFunctionEntry\n",
        PgBlock->RtlLookupFunctionEntry);
#endif // !PUBLIC

    RtlInitUnicodeString(&RoutineString, L"RtlVirtualUnwind");

    PgBlock->RtlVirtualUnwind = MmGetSystemRoutineAddress(&RoutineString);

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > RtlVirtualUnwind\n",
        PgBlock->RtlVirtualUnwind);
#endif // !PUBLIC

    RtlInitUnicodeString(&RoutineString, L"ExInterlockedRemoveHeadList");

    PgBlock->ExInterlockedRemoveHeadList = MmGetSystemRoutineAddress(&RoutineString);

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > ExInterlockedRemoveHeadList\n",
        PgBlock->ExInterlockedRemoveHeadList);
#endif // !PUBLIC

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
        PgClearCallback,
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
    __in PPGBLOCK PgBlock,
    __in PVOID Return,
    __in PVOID Callback,
    __in_opt PVOID ProgramCounter,
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

        SetGuardBody(
            &Object->Body,
            PgBlock,
            Object,
            Callback,
            Return,
            ProgramCounter,
            CaptureContext);

        ExInterlockedInsertTailList(
            &PgBlock->List,
            &Object->Entry,
            &PgBlock->Lock);
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

    // set temp buffer PatchGuard entry head jmp to PgClearCallback and encrypt

    Pointer = (PCHAR)FieldBuffer + (RvaOfEntry & 7);

    DetourLockedBuildJumpCode(
        &Pointer,
        &Object->Body);

    while (Index--) {
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
        LastRorKey = __ROR64(LastRorKey, FieldIndex + Index);
    }

    // copy temp buffer PatchGuard entry head to old address, 
    // when PatchGuard code decrypt self jmp PgClearCallback.

    RtlCopyMemory(
        (PCHAR)PatchGuardContext + (RvaOfEntry & ~7),
        FieldBuffer,
        sizeof(FieldBuffer));

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > set new entry for encrypted context\n",
        Object);
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

    // xor decrypt must align 8 bytes
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

    // set temp buffer PatchGuard entry head jmp to PgClearCallback and encrypt

    Pointer = (PDETOUR_BODY)((PCHAR)FieldBuffer + 8 - AlignOffset);

    DetourLockedBuildJumpCode(
        &Pointer,
        &Object->Body);

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

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > set new entry for btc encrypted context\n",
        Object);
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
    PFN_NUMBER NumberOfPages = 0;
    BOOLEAN Chance = TRUE;

    if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
        // protected code for donate version
    }
    else {
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
                                RegionSize);
#endif // !PUBLIC

                            Object = PgCreateObject(
                                PgEncrypted,
                                Type,
                                BaseAddress,
                                RegionSize,
                                PgBlock,
                                NULL,
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
}

VOID
NTAPI
InitializeSystemPtesBitMap(
    __inout PMMPTE BasePte,
    __in PFN_NUMBER NumberOfPtes,
    __out PRTL_BITMAP BitMap
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

    BeginAddress = GetVirtualAddressMappedByPte(BasePte);
    EndAddress = GetVirtualAddressMappedByPte(BasePte + NumberOfPtes);

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
                                        BitNumber = PointerPte - BasePte;

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
    ULONG StartingRunIndex = 0;

    NumberOfPtes = PgBlock->NumberOfPtes;

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > SystemPtes < %p - %p >\n",
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
            PgBlock->BasePte,
            NumberOfPtes,
            BitMap);

        do {
            HintIndex = RtlFindSetBits(
                BitMap,
                1,
                HintIndex);

            if (MAXULONG != HintIndex) {
                RtlFindNextForwardRunClear(
                    BitMap,
                    HintIndex,
                    &StartingRunIndex);

                RtlClearBits(BitMap, HintIndex, StartingRunIndex - HintIndex);

                if (StartingRunIndex -
                    HintIndex >= BYTES_TO_PAGES(PgBlock->SizeINITKDBG)) {
                    PgCompareFields(
                        PgBlock,
                        PgSystemPtes,
                        GetVirtualAddressMappedByPte(PgBlock->BasePte + HintIndex),
                        (StartingRunIndex - HintIndex) * PAGE_SIZE);
                }

                HintIndex = StartingRunIndex;
            }
        } while (HintIndex < NumberOfPtes);

        ExFreePool(BitMap);
    }
}

VOID
NTAPI
PgClearPoolEncryptedContext(
    __inout PPGBLOCK PgBlock
)
{
    POOL_TYPE CheckType = NonPagedPool;
    POOL_TYPE PoolType = NonPagedPool;
    SIZE_T Index = 0;
    SIZE_T PoolBigPageTableSize = 0;
    PPOOL_BIG_PAGES PoolBigPageTable = NULL;

    PoolBigPageTableSize = PgBlock->PoolBigPageTableSize;
    PoolBigPageTable = PgBlock->PoolBigPageTable;

#ifndef PUBLIC
    DbgPrint(
        "[Shark] < %p > BigPool < %p - %08x >\n",
        KeGetCurrentProcessorNumber(),
        PoolBigPageTable,
        PoolBigPageTableSize);
#endif // !PUBLIC

    for (Index = 0;
        Index < PoolBigPageTableSize;
        Index++) {
        if (POOL_BIG_TABLE_ENTRY_FREE !=
            ((ULONG64)PoolBigPageTable[Index].Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            PoolType = PgBlock->MmDeterminePoolType(PoolBigPageTable[Index].Va);
            CheckType = PoolType & BASE_POOL_TYPE_MASK;

            if (NonPagedPool == CheckType) {
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
PgLocatePoolObject(
    __inout PPGBLOCK PgBlock,
    __in PVOID Establisher,
    __in PPGOBJECT Object
)
{
    PFN_NUMBER Index = 0;

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
                        GetGpBlock(PgBlock)->DbgPrint(
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
}

VOID
NTAPI
PgLocateSystemPtesObject(
    __inout PPGBLOCK PgBlock,
    __in PVOID Establisher,
    __in PPGOBJECT Object
)
{
    PRTL_BITMAP BitMap = NULL;
    ULONG BitMapSize = 0;
    PFN_NUMBER NumberOfPtes = 0;
    ULONG HintIndex = 0;
    ULONG StartingRunIndex = 0;

    NumberOfPtes = PgBlock->NumberOfPtes;

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
            PgBlock->BasePte,
            NumberOfPtes,
            BitMap);

        do {
            HintIndex = RtlFindSetBits(
                BitMap,
                1,
                HintIndex);

            if (MAXULONG != HintIndex) {
                RtlFindNextForwardRunClear(
                    BitMap,
                    HintIndex,
                    &StartingRunIndex);

                RtlClearBits(BitMap, HintIndex, StartingRunIndex - HintIndex);

                if ((ULONG64)Establisher >=
                    (ULONG64)GetVirtualAddressMappedByPte(
                        PgBlock->BasePte + HintIndex) &&
                        (ULONG64)Establisher <
                    (ULONG64)GetVirtualAddressMappedByPte(
                        PgBlock->BasePte + StartingRunIndex) - PgBlock->SizeCmpAppendDllSection) {
                    Object->BaseAddress =
                        GetVirtualAddressMappedByPte(PgBlock->BasePte + HintIndex);

                    Object->RegionSize =
                        (SIZE_T)(StartingRunIndex - HintIndex) * PAGE_SIZE;

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

                HintIndex = StartingRunIndex;
            }
        } while (HintIndex < NumberOfPtes);

        ExFreePool(BitMap);
    }
}

VOID
NTAPI
PgLocateAllObject(
    __inout PPGBLOCK PgBlock,
    __in PVOID Establisher,
    __out PPGOBJECT Object
)
{
    PgLocatePoolObject(
        PgBlock,
        Establisher,
        Object);

    if (-1 == Object->Type) {
        PgLocateSystemPtesObject(
            PgBlock,
            Establisher,
            Object);
    }
}

VOID
NTAPI
PgCheckAllWorkerThread(
    __inout PPGBLOCK PgBlock
)
{
    PETHREAD Thread = NULL;
    PEXCEPTION_FRAME ExceptionFrame = NULL;
    ULONG64 ControlPc = 0;
    ULONG64 EstablisherFrame = 0;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    PVOID HandlerData = NULL;
    ULONG64 ImageBase = 0;
    PPGOBJECT Object = NULL;
    PMMPTE PointerPte = NULL;

    struct {
        CONTEXT ContextRecord;
        KNONVOLATILE_CONTEXT_POINTERS ContextPointers;
        PGOBJECT Object;
    }*Context;

    Context = ExAllocatePool(
        NonPagedPool,
        sizeof(CONTEXT) + sizeof(KNONVOLATILE_CONTEXT_POINTERS) + sizeof(PGOBJECT));

    if (NULL != Context) {
        Thread = GetProcessFirstThread(
            GetGpBlock(PgBlock),
            PsGetCurrentThreadProcess());

        while (GetThreadListEntry(
            GetGpBlock(PgBlock),
            Thread) != GetProcessThreadListHead(
                GetGpBlock(PgBlock),
                PsGetCurrentThreadProcess())) {
            if (PsGetCurrentThreadId() != PsGetThreadId(Thread) &&
                (ULONG64)PgBlock->ExpWorkerThread == UnsafeReadPointer(
                (PCHAR)Thread +
                    GetGpBlock(PgBlock)->OffsetKThreadWin32StartAddress)) {
                // SwapContext save ExceptionFrame on Thread->KernelStack
                ExceptionFrame = (PEXCEPTION_FRAME)UnsafeReadPointer(
                    (PCHAR)Thread +
                    GetGpBlock(PgBlock)->DebuggerDataBlock.OffsetKThreadKernelStack);

                PointerPte = GetPteAddress(ExceptionFrame);

                if (0 != PointerPte->u.Hard.Valid) {
                    EstablisherFrame =
                        (ULONG64)(ExceptionFrame + 1) +
                        KSTART_FRAME_LENGTH + sizeof(PVOID);

                    Context->ContextRecord.Rip = UnsafeReadPointer(EstablisherFrame);
                    Context->ContextRecord.Rsp = EstablisherFrame + sizeof(PVOID);

                    Context->ContextRecord.Rbx = ExceptionFrame->Rbx;
                    Context->ContextRecord.Rbp = ExceptionFrame->Rbp;
                    Context->ContextRecord.Rsi = ExceptionFrame->Rsi;
                    Context->ContextRecord.Rdi = ExceptionFrame->Rdi;
                    Context->ContextRecord.R12 = ExceptionFrame->R12;
                    Context->ContextRecord.R13 = ExceptionFrame->R13;
                    Context->ContextRecord.R14 = ExceptionFrame->R14;
                    Context->ContextRecord.R15 = ExceptionFrame->R15;

                    Context->ContextRecord.Xmm6 = ExceptionFrame->Xmm6;
                    Context->ContextRecord.Xmm7 = ExceptionFrame->Xmm7;
                    Context->ContextRecord.Xmm8 = ExceptionFrame->Xmm8;
                    Context->ContextRecord.Xmm9 = ExceptionFrame->Xmm9;
                    Context->ContextRecord.Xmm10 = ExceptionFrame->Xmm10;
                    Context->ContextRecord.Xmm11 = ExceptionFrame->Xmm11;
                    Context->ContextRecord.Xmm12 = ExceptionFrame->Xmm12;
                    Context->ContextRecord.Xmm13 = ExceptionFrame->Xmm13;
                    Context->ContextRecord.Xmm14 = ExceptionFrame->Xmm14;
                    Context->ContextRecord.Xmm15 = ExceptionFrame->Xmm15;

                    Context->ContextPointers.Rbx = &Context->ContextRecord.Rbx;
                    Context->ContextPointers.Rsp = &Context->ContextRecord.Rsp;
                    Context->ContextPointers.Rbp = &Context->ContextRecord.Rbp;
                    Context->ContextPointers.Rsi = &Context->ContextRecord.Rsi;
                    Context->ContextPointers.Rdi = &Context->ContextRecord.Rdi;
                    Context->ContextPointers.R12 = &Context->ContextRecord.R12;
                    Context->ContextPointers.R13 = &Context->ContextRecord.R13;
                    Context->ContextPointers.R14 = &Context->ContextRecord.R14;
                    Context->ContextPointers.R15 = &Context->ContextRecord.R15;

                    Context->ContextPointers.Xmm6 = &Context->ContextRecord.Xmm6;
                    Context->ContextPointers.Xmm7 = &Context->ContextRecord.Xmm7;
                    Context->ContextPointers.Xmm8 = &Context->ContextRecord.Xmm8;
                    Context->ContextPointers.Xmm9 = &Context->ContextRecord.Xmm9;
                    Context->ContextPointers.Xmm10 = &Context->ContextRecord.Xmm10;
                    Context->ContextPointers.Xmm11 = &Context->ContextRecord.Xmm11;
                    Context->ContextPointers.Xmm12 = &Context->ContextRecord.Xmm12;
                    Context->ContextPointers.Xmm13 = &Context->ContextRecord.Xmm13;
                    Context->ContextPointers.Xmm14 = &Context->ContextRecord.Xmm14;
                    Context->ContextPointers.Xmm15 = &Context->ContextRecord.Xmm15;

                    while (EstablisherFrame >= UnsafeReadPointer(
                        (PCHAR)Thread +
                        GetGpBlock(PgBlock)->DebuggerDataBlock.OffsetKThreadKernelStack) &&
                        EstablisherFrame < UnsafeReadPointer(
                        (PCHAR)Thread +
                            GetGpBlock(PgBlock)->DebuggerDataBlock.OffsetKThreadInitialStack)) {
                        FunctionEntry = RtlLookupFunctionEntry(
                            Context->ContextRecord.Rip,
                            &ImageBase,
                            NULL);

                        if (NULL != FunctionEntry) {
                            RtlVirtualUnwind(
                                UNW_FLAG_NHANDLER,
                                ImageBase,
                                Context->ContextRecord.Rip,
                                FunctionEntry,
                                &Context->ContextRecord,
                                &HandlerData,
                                &EstablisherFrame,
                                &Context->ContextPointers);
                        }
                        else {
                            if (0 != Context->ContextRecord.Rip) {
#ifndef PUBLIC
                                DbgPrint(
                                    "[Shark] < %p > found noimage return address in worker thread stack\n",
                                    Context->ContextRecord.Rip);
#endif // !PUBLIC

                                Context->Object.Type = -1;

                                PgLocateAllObject(
                                    PgBlock,
                                    (PVOID)Context->ContextRecord.Rip,
                                    &Context->Object);

                                if (-1 != Context->Object.Type) {
                                    Object = PgCreateObject(
                                        PgDeclassified,
                                        Context->Object.Type,
                                        Context->Object.BaseAddress,
                                        Context->Object.RegionSize,
                                        PgBlock,
                                        NULL,
                                        PgBlock->ClearCallback,
                                        NULL,
                                        GetGpBlock(PgBlock)->CaptureContext);

                                    if (NULL != Object) {
                                        // align stack, must start at Body.Parameter
                                        UnsafeWritePointer(
                                            Context->ContextRecord.Rsp - 8,
                                            &Object->Body.Parameter);

#ifndef PUBLIC
                                        DbgPrint(
                                            "[Shark] < %p > insert worker thread check code\n",
                                            Object);
#endif // !PUBLIC
                                    }
                                }
                            }

                            break;
                        }
                    }
                }
            }

            Thread = GetNexThread(GetGpBlock(PgBlock), Thread);
        }

        ExFreePool(Context);
    }
}

VOID
NTAPI
PgClearAll(
    __inout PPGBLOCK PgBlock
)
{
    // protected code for donate version

    PgClearPoolEncryptedContext(PgBlock);

    if (9200 <= GetGpBlock(PgBlock)->BuildNumber) {
        PgClearSystemPtesEncryptedContext(PgBlock);
    }

    PgCheckAllWorkerThread(PgBlock);
}

VOID
NTAPI
PgClearWorker(
    __inout PVOID Argument
)
{
    ULONG Index = 0;
    PULONG64 InitialStack = 0;
    DISPATCHER_HEADER * Header = NULL;
    ULONG ReturnLength = 0;

    struct {
        PPGBLOCK PgBlock;
        KEVENT Notify;
        WORK_QUEUE_ITEM Worker;
    }*Context;

    Context = Argument;

    /*
    if (os build >= 9600){
        PgBlock->WorkerContext = struct _DISPATCHER_HEADER

        // Header->Type = 0x15
        // Header->Hand = 0xac
    }
    else {
        PgBlock->WorkerContext = enum _WORK_QUEUE_TYPE

        // CriticalWorkQueue = 0
        // DelayedWorkQueue = 1
    }
    */

    InitialStack = IoGetInitialStack();

    if (GetGpBlock(Context->PgBlock)->BuildNumber >= 9600) {
        while ((ULONG64)InitialStack != (ULONG64)&Argument) {
            Header = *(PVOID *)InitialStack;

            if (FALSE != MmIsAddressValid(Header)) {
                if (FALSE != MmIsAddressValid((PCHAR)(Header + 1) - 1)) {
                    if (0x15 == Header->Type &&
                        0xac == Header->Hand) {
                        Context->PgBlock->WorkerContext = Header;

                        break;
                    }
                }
            }

            InitialStack--;
        }
    }
    else {
        Context->PgBlock->WorkerContext = UlongToPtr(CriticalWorkQueue);
    }

    if (NT_SUCCESS(ZwQueryInformationThread(
        ZwCurrentThread(),
        ThreadQuerySetWin32StartAddress,
        &Context->PgBlock->ExpWorkerThread,
        sizeof(PVOID),
        &ReturnLength))) {
        for (Index = 0;
            Index < GetGpBlock(Context->PgBlock)->DebuggerDataBlock.SizeEThread;
            Index += 8) {
            if ((ULONG_PTR)Context->PgBlock->ExpWorkerThread ==
                UnsafeReadPointer((PCHAR)KeGetCurrentThread() + Index)) {
                GetGpBlock(Context->PgBlock)->OffsetKThreadWin32StartAddress = Index;

                break;
            }
        }

        IpiSingleCall(
            (PPS_APC_ROUTINE)NULL,
            (PKSYSTEM_ROUTINE)NULL,
            (PUSER_THREAD_START_ROUTINE)PgClearAll,
            (PVOID)Context->PgBlock);
    }

    KeSetEvent(&Context->Notify, LOW_PRIORITY, FALSE);
}

VOID
NTAPI
PgClear(
    __inout PPGBLOCK PgBlock
)
{
    BOOLEAN Chance = TRUE;

    struct {
        PPGBLOCK PgBlock;
        KEVENT Notify;
        WORK_QUEUE_ITEM Worker;
    }Context = { 0 };

    Context.PgBlock = PgBlock;

    if (GetGpBlock(PgBlock)->BuildNumber >= 7600 &&
        GetGpBlock(PgBlock)->BuildNumber < 18362) {
        InitializePgBlock(Context.PgBlock);

        if (0 != PgBlock->OffsetEntryPoint ||
            0 != PgBlock->SizeCmpAppendDllSection ||
            0 != PgBlock->SizeINITKDBG ||
            NULL != PgBlock->PoolBigPageTable ||
            0 != PgBlock->PoolBigPageTableSize ||
            NULL != PgBlock->ExpLargePoolTableLock ||
            NULL != PgBlock->MmDeterminePoolType) {

            if (GetGpBlock(PgBlock)->BuildNumber >= 9200) {
                if (0 == PgBlock->NumberOfPtes || NULL == PgBlock->BasePte) {
                    Chance = FALSE;
                }
            }

            if (FALSE != Chance) {
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
    }
    else {
#ifndef PUBLIC
        DbgPrint(
            "[Shark] < %p > free version only support windows version 7600 ~ 17763\n",
            PgBlock);
#endif // !PUBLIC
    }
}
