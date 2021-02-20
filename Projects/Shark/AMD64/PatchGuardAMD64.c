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

#include "PatchGuard.h"

#include "Ctx.h"
#include "Guard.h"
#include "Rtx.h"
#include "Scan.h"
#include "Space.h"
#include "Stack.h"

void
NTAPI
PgFreeWorker(
    __in PPGOBJECT Object
)
{
    PPGBLOCK PgBlock = NULL;

    GetCounterBody(
        &Object->Body.Reserved,
        &PgBlock);

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
    else if (PgMaximumType == Object->Type) {
        NOTHING;
    }

    GetGpBlock(PgBlock)->ExFreePoolWithTag(Object, 0);
}

void
NTAPI
PgClearCallback(
    __in PCONTEXT Context,
    __in_opt ptr ProgramCounter,
    __in_opt PPGOBJECT Object,
    __in_opt PPGBLOCK PgBlock
)
{
    PETHREAD Thread = NULL;
    PKSTART_FRAME StartFrame = NULL;
    PKSWITCH_FRAME SwitchFrame = NULL;
    PKTRAP_FRAME TrapFrame = NULL;
    u32 Index = 0;
    KNONVOLATILE_CONTEXT_POINTERS ContextPointers = { 0 };
    u64 ControlPc = 0;
    u64 EstablisherFrame = 0;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    ptr HandlerData = NULL;
    u64 ImageBase = 0;
    PMMPTE PointerPte = NULL;
    u32 BugCheckCode = 0;

    if (NULL != ProgramCounter) {
        BugCheckCode = __rds32(&Context->Rcx);

        if (ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY == BugCheckCode) {
            TrapFrame = (PKTRAP_FRAME)Context->R9;

            if (NULL != TrapFrame) {
                ContextPointers.Rbx = &Context->Rbx;
                ContextPointers.Rsp = &Context->Rsp;
                ContextPointers.Rbp = &Context->Rbp;
                ContextPointers.Rsi = &Context->Rsi;
                ContextPointers.Rdi = &Context->Rdi;
                ContextPointers.R12 = &Context->R12;
                ContextPointers.R13 = &Context->R13;
                ContextPointers.R14 = &Context->R14;
                ContextPointers.R15 = &Context->R15;

                ContextPointers.Xmm6 = &Context->Xmm6;
                ContextPointers.Xmm7 = &Context->Xmm7;
                ContextPointers.Xmm8 = &Context->Xmm8;
                ContextPointers.Xmm9 = &Context->Xmm9;
                ContextPointers.Xmm10 = &Context->Xmm10;
                ContextPointers.Xmm11 = &Context->Xmm11;
                ContextPointers.Xmm12 = &Context->Xmm12;
                ContextPointers.Xmm13 = &Context->Xmm13;
                ContextPointers.Xmm14 = &Context->Xmm14;
                ContextPointers.Xmm15 = &Context->Xmm15;

                do {
                    ControlPc = Context->Rip;

                    FunctionEntry = PgBlock->RtlLookupFunctionEntry(
                        ControlPc,
                        &ImageBase,
                        NULL);

                    if (FunctionEntry != NULL) {
                        PgBlock->RtlVirtualUnwind(
                            UNW_FLAG_EHANDLER,
                            ImageBase,
                            ControlPc,
                            FunctionEntry,
                            Context,
                            &HandlerData,
                            &EstablisherFrame,
                            &ContextPointers);
                    }
                    else {
                        Context->Rip = *(u64ptr)(Context->Rsp);
                        Context->Rsp += 8;
                    }
                } while (EstablisherFrame != (u64)TrapFrame);

                Context->Rax = TrapFrame->Rax;
                Context->Rcx = TrapFrame->Rcx;
                Context->Rdx = TrapFrame->Rdx;
                Context->R8 = TrapFrame->R8;
                Context->R9 = TrapFrame->R9;
                Context->R10 = TrapFrame->R10;
                Context->R11 = TrapFrame->R11;

                Context->Xmm0 = TrapFrame->Xmm0;
                Context->Xmm1 = TrapFrame->Xmm1;
                Context->Xmm2 = TrapFrame->Xmm2;
                Context->Xmm3 = TrapFrame->Xmm3;
                Context->Xmm4 = TrapFrame->Xmm4;
                Context->Xmm5 = TrapFrame->Xmm5;

                Context->MxCsr = TrapFrame->MxCsr;
            }

            if (FALSE == IsListEmpty(&PgBlock->ObjectList)) {
                Object = CONTAINING_RECORD(
                    PgBlock->ObjectList.Flink,
                    PGOBJECT,
                    Entry);

                while (&Object->Entry != &PgBlock->ObjectList) {
                    if (TrapFrame->Rip >= (u64)Object->BaseAddress &&
                        TrapFrame->Rip < (u64)Object->BaseAddress + PAGE_SIZE) {
                        GetGpBlock(PgBlock)->ExInterlockedRemoveHeadList(
                            Object->Entry.Blink,
                            &PgBlock->ObjectLock);

                        if (0x1131482e == __rds32(TrapFrame->Rip)) {
                            Context->Rip = __rdsptr(TrapFrame->Rsp);
                            Context->Rsp = TrapFrame->Rsp + sizeof(ptr);

#ifdef DEBUG
                            GetGpBlock(PgBlock)->vDbgPrint(
                                PgBlock->ClearMessage[PgEncrypted],
                                Object);

                            PgBlock->Cleared++;
#endif // DEBUG

                            ExInitializeWorkItem(
                                &Object->Worker, PgBlock->FreeWorker, Object);

                            PgBlock->ExQueueWorkItem(
                                &Object->Worker, CriticalWorkQueue);
                        }
                        else {
                            PgBlock->MmSetPageProtection(
                                PAGE_ALIGN(TrapFrame->Rip),
                                PAGE_SIZE,
                                PAGE_EXECUTE_READWRITE);

                            Object->Type = PgMaximumType;

                            ExInitializeWorkItem(
                                &Object->Worker, PgBlock->FreeWorker, Object);

                            PgBlock->ExQueueWorkItem(
                                &Object->Worker, CriticalWorkQueue);

                            Context->Rip = TrapFrame->Rip;
                            Context->Rsp = TrapFrame->Rsp;
                        }

                        break;
                    }

                    Object = CONTAINING_RECORD(
                        Object->Entry.Flink,
                        PGOBJECT,
                        Entry);
                }
            }
        }
        else {
            // __debugbreak();
        }
    }
    else {
        GetCounterBody(
            &Object->Body.Reserved,
            &PgBlock);

#ifdef DEBUG
        GetGpBlock(PgBlock)->vDbgPrint(
            PgBlock->ClearMessage[Object->Encrypted],
            Object);

        PgBlock->Cleared++;
#endif // DEBUG

        GetGpBlock(PgBlock)->ExInterlockedRemoveHeadList(
            Object->Entry.Blink,
            &PgBlock->ObjectLock);

        if (PgEncrypted == Object->Encrypted) {
            Context->Rip = __rdsptr(Context->Rsp + KSTART_FRAME_LENGTH);
            Context->Rsp += KSTART_FRAME_LENGTH + sizeof(ptr);

            ExInitializeWorkItem(
                &Object->Worker, PgBlock->FreeWorker, Object);

            PgBlock->ExQueueWorkItem(
                &Object->Worker, CriticalWorkQueue);
        }
        else {
            Thread = (PETHREAD)__readgsqword(FIELD_OFFSET(KPCR, Prcb.CurrentThread));

            if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
                // ETHREAD->ReservedCrossThreadFlags
                // clear SameThreadPassiveFlags Bit 0 (BugCheck 139)

                *((PBOOLEAN)Thread + PgBlock->OffsetSameThreadPassive) &= 0xFFFFFFFE;
            }

            StartFrame =
                (PKSTART_FRAME)(
                (*(u64ptr)((u64)Thread +
                    GetGpBlock(PgBlock)->DebuggerDataBlock.OffsetKThreadInitialStack)) -
                    KSTART_FRAME_LENGTH);

            SwitchFrame = (PKSWITCH_FRAME)((u64)StartFrame - KSWITCH_FRAME_LENGTH);

            StartFrame->P1Home = (u64)PgBlock->WorkerContext;
            StartFrame->P2Home = (u64)PgBlock->ExpWorkerThread;
            StartFrame->P3Home = (u64)PgBlock->PspSystemThreadStartup;
            StartFrame->Return = 0;
            SwitchFrame->Return = (u64)PgBlock->KiStartSystemThread;
            SwitchFrame->ApcBypass = APC_LEVEL;
            SwitchFrame->Rbp = (u64)TrapFrame + FIELD_OFFSET(KTRAP_FRAME, Xmm1);

            Context->Rsp = (u64)StartFrame;
            Context->Rip = (u64)PgBlock->KiStartSystemThread;

            ExInitializeWorkItem(
                &Object->Worker, PgBlock->FreeWorker, Object);

            PgBlock->ExQueueWorkItem(
                &Object->Worker, CriticalWorkQueue);
        }
    }

    GetGpBlock(PgBlock)->RtlRestoreContext(Context, NULL);
}

void
NTAPI
InitializePgBlock(
    __inout PPGBLOCK PgBlock
)
{
    status Status = STATUS_SUCCESS;
    ptr FileHandle = NULL;
    ptr SectionHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    ptr ViewBase = NULL;
    u ViewSize = 0;
    u64 ImageBase = 0;
    u8ptr ControlPc = NULL;
    u8ptr TargetPc = NULL;
    u32 Length = 0;
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    ptr EndToLock = NULL;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    LONG64 Diff = 0;
    UNICODE_STRING RoutineString = { 0 };
    ptr RoutineAddress = NULL;

    PPOOL_BIG_PAGES * PageTable = NULL;
    uptr PageTableSize = NULL;
    PRTL_BITMAP BitMap = NULL;

    u TempField = 0;

    s8 CmpAppendDllSection[] = "2E 48 31 11 48 31 51 08 48 31 51 10 48 31 51 18";

    // Fields
    s8 Fields[] = "FB 48 8D 05";
    s8 FirstField[] = "?? 89 ?? 00 01 00 00 48 8D 05 ?? ?? ?? ?? ?? 89 ?? 08 01 00 00";
    s8 NextField[] = "48 8D 05 ?? ?? ?? ?? ?? 89 86 ?? ?? 00 00";

    s8 KiStartSystemThread[] = "B9 01 00 00 00 44 0F 22 C1 48 8B 14 24 48 8B 4C 24 08";
    s8 PspSystemThreadStartup[] = "EB ?? B9 1E 00 00 00 E8";

    // 48 8B 35 D4 C8 0C 00                     mov rsi, cs:PoolBigPageTableSize
    // 48 8D 3C 76                              lea rdi, [rsi + rsi * 2]
    // 48 C1 E7 03                              shl rdi, 3
    // 4D 85 E4                                 test r12, r12
    // 74 08                                    jz short loc_1401498F1

    // 83 FE 01                                 cmp esi, 1
    // 75 10                                    jnz short loc_1401BDE4E
    // 48 8B 15 8B 36 0D 00                     mov rdx, cs:PoolBigPageTable
    // 48 8B 35 9C 36 0D 00                     mov rsi, cs:PoolBigPageTableSize
    // EB 29                                    jmp short loc_1401BDE77

    u8ptr ExGetBigPoolInfo[] = {
        "48 83 3D ?? ?? ?? ?? 00 75 10 4D 85 C0 74 04 41 83 20 00 33 C0",
        "83 ?? 01 75 10 48 8B 15 ?? ?? ?? ?? ?? 8B ?? ?? ?? ?? ?? EB",
    };

    // MiGetSystemRegionType
    // 48 B8 00 00 00 00 00 80 FF FF	        mov     rax, 0FFFF800000000000h
    // 48 3B C8                     	        cmp     rcx, rax
    // 72 1D                        	        jb      short loc_14020A90C
    // 48 C1 E9 27                  	        shr     rcx, 27h
    // 81 E1 FF 01 00 00            	        and     ecx, 1FFh
    // 8D 81 00 FF FF FF            	        lea     eax, [rcx-100h]
    // 48 8D 0D 02 7F A4 00         	        lea     rcx, unk_140C52808
    // 0F B6 04 08                  	        movzx   eax, byte ptr [rax+rcx]

    s8 MiGetSystemRegionType[] =
        "48 c1 e9 27 81 e1 ff 01 00 00 8d 81 00 ff ff ff 48 8d 0d ?? ?? ?? ?? 0f b6 04 08";

    // 55                                       push    rbp
    // 41 54                                    push    r12
    // 41 55                                    push    r13
    // 41 56                                    push    r14
    // 41 57                                    push    r15
    // 48 81 EC C0 02 00 00                     sub     rsp, 2C0h
    // 48 8D A8 D8 FD FF FF                     lea     rbp, [rax - 228h]
    // 48 83 E5 80 and rbp, 0FFFFFFFFFFFFFF80h

    u8ptr Header[2] = {
        "55 41 54 41 55 41 56 41 57 48 81 EC C0 02 00 00 48 8D A8 D8 FD FF FF 48 83 E5 80",
        "55 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC B0 00 00 00 8B 82"
    };

    s8 ReservedCrossThreadFlags[] =
        "89 83 ?? ?? F0 83 0C 24 00 80 3D ?? ?? ?? ?? ?? 0F";

    u64 Btc64[] = { 0xC3C18B48D1BB0F48 };
    u64 Rol64[] = { 0xC3C0D348CA869148 };
    u64 Ror64[] = { 0xC3C8D348CA869148 };

    cptr ClearMessage[2] = {
        "[Shark] [PatchGuard] < %p > declassified context cleared\n",
        "[Shark] [PatchGuard] < %p > encrypted context cleared\n"
    };

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > PgBlock\n",
        PgBlock);
#endif // DEBUG

    InitializeListHead(&PgBlock->ObjectList);
    KeInitializeSpinLock(&PgBlock->ObjectLock);

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
                    (u64)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase -
                    (u64)ViewBase;

                ControlPc = ScanBytes(
                    ViewBase,
                    (u8ptr)ViewBase + ViewSize,
                    CmpAppendDllSection);

                if (NULL != ControlPc) {
                    TargetPc = ControlPc;

                    while (0 != _cmpbyte(TargetPc[0], 0x41) &&
                        0 != _cmpbyte(TargetPc[1], 0xff) &&
                        0 != _cmpbyte(TargetPc[2], 0xe0)) {
                        Length = DetourGetInstructionLength(TargetPc);

                        if (0 == PgBlock->SizeCmpAppendDllSection) {
                            if (8 == Length) {
                                if (0 == _cmpbyte(TargetPc[0], 0x48) &&
                                    0 == _cmpbyte(TargetPc[1], 0x31) &&
                                    0 == _cmpbyte(TargetPc[2], 0x84) &&
                                    0 == _cmpbyte(TargetPc[3], 0xca)) {
                                    PgBlock->SizeCmpAppendDllSection = *(u32ptr)(TargetPc + 4);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] [PatchGuard] < %p > SizeCmpAppendDllSection\n",
                                        PgBlock->SizeCmpAppendDllSection);
#endif // DEBUG

                                    if (0 == _cmpbyte(TargetPc[11], 0x48) ||
                                        0 == _cmpbyte(TargetPc[12], 0x0f) ||
                                        0 == _cmpbyte(TargetPc[13], 0xbb) ||
                                        0 == _cmpbyte(TargetPc[14], 0xc0)) {
                                        PgBlock->BtcEnable = TRUE;

#ifdef DEBUG
                                        vDbgPrint(
                                            "[Shark] [PatchGuard] < %p > BtcEnable\n",
                                            PgBlock->BtcEnable);
#endif // DEBUG
                                    }
                                }
                            }
                        }

                        if (6 == Length) {
                            if (0 == _cmpbyte(TargetPc[0], 0x8b) &&
                                0 == _cmpbyte(TargetPc[1], 0x82)) {
                                PgBlock->OffsetEntryPoint = *(u32ptr)(TargetPc + 2);

#ifdef DEBUG
                                vDbgPrint(
                                    "[Shark] [PatchGuard] < %p > OffsetEntryPoint\n",
                                    PgBlock->OffsetEntryPoint);
#endif // DEBUG
                                break;
                            }
                        }

                        TargetPc += Length;
                    }
                }

                ControlPc = ScanBytes(
                    ViewBase,
                    (u8ptr)ViewBase + ViewSize,
                    Header[GetGpBlock(PgBlock)->BuildNumber >= 18362 ? 1 : 0]);

                if (NULL != ControlPc) {
                    TargetPc = ControlPc + Diff;

                    FunctionEntry = RtlLookupFunctionEntry(
                        (u64)TargetPc,
                        (u64ptr)&ImageBase,
                        NULL);

                    if (NULL != FunctionEntry) {
                        TargetPc =
                            (u8ptr)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase +
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
                        PgBlock->SizeINITKDBG =
                            max(NtSection->SizeOfRawData, NtSection->Misc.VirtualSize);

#ifdef DEBUG
                        vDbgPrint(
                            "[Shark] [PatchGuard] < %p > SizeINITKDBG\n",
                            PgBlock->SizeINITKDBG);
#endif // DEBUG

                        PgBlock->INITKDBG = __malloc(PgBlock->SizeINITKDBG);

                        if (NULL != PgBlock->INITKDBG) {
                            RtlCopyMemory(
                                PgBlock->INITKDBG,
                                (u8ptr)ViewBase + NtSection->VirtualAddress,
                                PgBlock->SizeINITKDBG);
#ifdef DEBUG
                            vDbgPrint(
                                "[Shark] [PatchGuard] < %p > INITKDBG\n",
                                PgBlock->INITKDBG);
#endif // DEBUG
                        }
                    }
                }

                ControlPc = ViewBase;

                while (NULL != ControlPc) {
                    ControlPc = ScanBytes(
                        ControlPc,
                        (u8ptr)ViewBase + ViewSize,
                        Fields);

                    if (NULL != ControlPc) {
                        TargetPc = ScanBytes(
                            ControlPc,
                            ControlPc + PgBlock->OffsetEntryPoint,
                            FirstField);

                        if (NULL != TargetPc) {
                            PgBlock->Fields[0] =
                                (u64)__rva_to_va(TargetPc - 4) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[Shark] [PatchGuard]",
                                (ptr)PgBlock->Fields[0]);
#endif // DEBUG

                            PgBlock->Fields[1] =
                                (u64)__rva_to_va(TargetPc + 10) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[Shark] [PatchGuard]",
                                (ptr)PgBlock->Fields[1]);
#endif // DEBUG

                            PgBlock->Fields[2] =
                                (u64)__rva_to_va(TargetPc + 24) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[Shark] [PatchGuard]",
                                (ptr)PgBlock->Fields[2]);
#endif // DEBUG

                            PgBlock->Fields[3] =
                                (u64)__rva_to_va(TargetPc + 38) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[Shark] [PatchGuard]",
                                (ptr)PgBlock->Fields[3]);
#endif // DEBUG

                            if (GetGpBlock(PgBlock)->BuildNumber >= 9200) {
                                while (TRUE) {
                                    TargetPc = ScanBytes(
                                        TargetPc,
                                        (u8ptr)ViewBase + ViewSize,
                                        NextField);

                                    TempField = (u64)__rva_to_va(TargetPc + 3) + Diff;

                                    if ((u)TempField ==
                                        (u)GetGpBlock(PgBlock)->DbgPrint) {
                                        // if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
                                        if (GetGpBlock(PgBlock)->BuildNumber >= 9200) {
                                            TempField = (u64)__rva_to_va(TargetPc + 17) + Diff;

                                            RtlCopyMemory(
                                                &PgBlock->MmAllocateIndependentPages,
                                                &TempField,
                                                sizeof(ptr));

#ifdef DEBUG
                                            vDbgPrint(
                                                "[Shark] [PatchGuard] < %p > MmAllocateIndependentPages\n",
                                                PgBlock->MmAllocateIndependentPages);
#endif // DEBUG
                                        }

                                        TempField = (u64)__rva_to_va(TargetPc + 31) + Diff;

                                        RtlCopyMemory(
                                            &PgBlock->MmFreeIndependentPages,
                                            &TempField,
                                            sizeof(ptr));

#ifdef DEBUG
                                        vDbgPrint(
                                            "[Shark] [PatchGuard] < %p > MmFreeIndependentPages\n",
                                            PgBlock->MmFreeIndependentPages);
#endif // DEBUG

                                        TempField = (u64)__rva_to_va(TargetPc + 45) + Diff;

                                        RtlCopyMemory(
                                            &PgBlock->MmSetPageProtection,
                                            &TempField,
                                            sizeof(ptr));

#ifdef DEBUG
                                        vDbgPrint(
                                            "[Shark] [PatchGuard] < %p > MmSetPageProtection\n",
                                            PgBlock->MmSetPageProtection);
#endif // DEBUG

                                        break;
                                    }

                                    TargetPc++;
                                }
                            }

                            while (TRUE) {
                                TargetPc = ScanBytes(
                                    TargetPc,
                                    (u8ptr)ViewBase + ViewSize,
                                    NextField);

                                TempField = (u64)__rva_to_va(TargetPc + 3) + Diff;

                                if ((u)TempField ==
                                    (u)GetGpBlock(PgBlock)->PsLoadedModuleList) {
                                    TempField = (u64)__rva_to_va(TargetPc - 11) + Diff;

                                    RtlCopyMemory(
                                        &GetGpBlock(PgBlock)->PsInvertedFunctionTable,
                                        &TempField,
                                        sizeof(ptr));

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] [PatchGuard] < %p > PsInvertedFunctionTable\n",
                                        GetGpBlock(PgBlock)->PsInvertedFunctionTable);
#endif // DEBUG

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
                    (u8ptr)ViewBase + ViewSize,
                    KiStartSystemThread);

                if (NULL != ControlPc) {
                    TargetPc = ControlPc;

                    PgBlock->KiStartSystemThread = (ptr)(TargetPc + Diff);

#ifdef DEBUG
                    vDbgPrint(
                        "[Shark] [PatchGuard] < %p > KiStartSystemThread\n",
                        PgBlock->KiStartSystemThread);
#endif // DEBUG
                }

                ControlPc = ScanBytes(
                    ViewBase,
                    (u8ptr)ViewBase + ViewSize,
                    PspSystemThreadStartup);

                if (NULL != ControlPc) {
                    TargetPc = (ptr)(ControlPc + Diff);

                    FunctionEntry = RtlLookupFunctionEntry(
                        (u64)TargetPc,
                        (u64ptr)&ImageBase,
                        NULL);

                    if (NULL != FunctionEntry) {
                        PgBlock->PspSystemThreadStartup =
                            (ptr)((u8ptr)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase +
                                FunctionEntry->BeginAddress);

#ifdef DEBUG
                        vDbgPrint(
                            "[Shark] [PatchGuard] < %p > PspSystemThreadStartup\n",
                            PgBlock->PspSystemThreadStartup);
#endif // DEBUG
                    }
                }

                ZwUnmapViewOfSection(ZwCurrentProcess(), ViewBase);
            }

            ZwClose(SectionHandle);
        }

        ZwClose(FileHandle);
    }

    RtlInitUnicodeString(&RoutineString, L"MmIsNonPagedSystemAddressValid");

    PgBlock->Pool.MmIsNonPagedSystemAddressValid = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > MmIsNonPagedSystemAddressValid\n",
        PgBlock->Pool.MmIsNonPagedSystemAddressValid);
#endif // DEBUG

    NtSection = FindSection(
        (ptr)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase,
        ".text");

    if (NULL != NtSection) {
        ControlPc =
            (u8ptr)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase +
            NtSection->VirtualAddress;

        EndToLock =
            (u8ptr)GetGpBlock(PgBlock)->DebuggerDataBlock.KernBase +
            NtSection->VirtualAddress +
            max(NtSection->SizeOfRawData, NtSection->Misc.VirtualSize);

        if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
            TargetPc = ScanBytes(
                ControlPc,
                EndToLock,
                MiGetSystemRegionType);

            if (NULL != TargetPc) {
                TargetPc -= 0xf;

                RtlCopyMemory(
                    (ptr)&PgBlock->MiGetSystemRegionType,
                    &TargetPc,
                    sizeof(ptr));

#ifdef DEBUG
                vDbgPrint(
                    "[Shark] [PatchGuard] < %p > MiGetSystemRegionType\n",
                    PgBlock->MiGetSystemRegionType);
#endif // DEBUG
            }
        }

        ControlPc = ScanBytes(
            ControlPc,
            EndToLock,
            ExGetBigPoolInfo[GetGpBlock(PgBlock)->BuildNumber >= 9200 ? 1 : 0]);

        if (NULL != ControlPc) {
            PgBlock->Pool.PoolBigPageTable =
                __rva_to_va(ControlPc
                    + (GetGpBlock(PgBlock)->BuildNumber >= 9200 ? 8 : 3));

#ifdef DEBUG
            vDbgPrint(
                "[Shark] [PatchGuard] < %p > PoolBigPageTable\n",
                PgBlock->Pool.PoolBigPageTable);
#endif // DEBUG

            PgBlock->Pool.PoolBigPageTableSize =
                __rva_to_va(ControlPc
                    + (GetGpBlock(PgBlock)->BuildNumber >= 9200 ? 0xF : 0x1D));

#ifdef DEBUG
            vDbgPrint(
                "[Shark] [PatchGuard] < %p > PoolBigPageTableSize\n",
                PgBlock->Pool.PoolBigPageTableSize);
#endif // DEBUG
        }
    }

    RtlInitUnicodeString(&RoutineString, L"MmAllocateMappingAddressEx");

    RoutineAddress = MmGetSystemRoutineAddress(&RoutineString);

    if (NULL == RoutineAddress) {
        RtlInitUnicodeString(&RoutineString, L"MmAllocateMappingAddress");

        RoutineAddress = MmGetSystemRoutineAddress(&RoutineString);
    }

    if (NULL != RoutineAddress) {
        ControlPc = RoutineAddress;

        while (TRUE) {
            Length = DetourGetInstructionLength(ControlPc);

            if (1 == Length) {
                if (0 == _cmpbyte(ControlPc[0], 0xc3)) {
                    break;
                }
            }

            if (7 == Length) {
                if (0 == _cmpbyte(ControlPc[0], 0x48) &&
                    0 == _cmpbyte(ControlPc[1], 0x8d) &&
                    0 == _cmpbyte(ControlPc[2], 0x0d)) {
                    TargetPc = __rva_to_va(ControlPc + 3);

                    // struct _MI_SYSTEM_PTE_TYPE *
                    BitMap = TargetPc;

                    PgBlock->SystemPtes.NumberOfPtes = BitMap->SizeOfBitMap * 8;
#ifdef DEBUG
                    vDbgPrint(
                        "[Shark] [PatchGuard] < %p > NumberOfPtes\n",
                        PgBlock->SystemPtes.NumberOfPtes);
#endif // DEBUG

                    if (GetGpBlock(PgBlock)->BuildNumber < 9600) {
                        PgBlock->SystemPtes.BasePte =
                            *(PMMPTE *)((u8ptr)(BitMap + 1) + sizeof(u32) * 2);
                    }
                    else {
                        PgBlock->SystemPtes.BasePte = *(PMMPTE *)(BitMap + 1);
                    }

#ifdef DEBUG
                    vDbgPrint(
                        "[Shark] [PatchGuard] < %p > BasePte\n",
                        PgBlock->SystemPtes.BasePte);
#endif // DEBUG

                    break;
                }
            }

            ControlPc += Length;
        }
    }

    RtlCopyMemory(
        PgBlock->_Btc64,
        Btc64,
        sizeof(PgBlock->_Btc64));

    PgBlock->Btc64 = (ptr)PgBlock->_Btc64;

    RtlCopyMemory(
        PgBlock->_Rol64,
        Rol64,
        sizeof(PgBlock->_Rol64));

    PgBlock->Rol64 = (ptr)PgBlock->_Rol64;

    RtlCopyMemory(
        PgBlock->_Ror64,
        Ror64,
        sizeof(PgBlock->_Ror64));

    PgBlock->Ror64 = (ptr)PgBlock->_Ror64;

    RtlInitUnicodeString(&RoutineString, L"RtlLookupFunctionEntry");

    PgBlock->RtlLookupFunctionEntry = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > RtlLookupFunctionEntry\n",
        PgBlock->RtlLookupFunctionEntry);
#endif // DEBUG

    RtlInitUnicodeString(&RoutineString, L"RtlVirtualUnwind");

    PgBlock->RtlVirtualUnwind = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > RtlVirtualUnwind\n",
        PgBlock->RtlVirtualUnwind);
#endif // DEBUG

    RtlInitUnicodeString(&RoutineString, L"ExQueueWorkItem");

    PgBlock->ExQueueWorkItem = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ExQueueWorkItem\n",
        PgBlock->ExQueueWorkItem);
#endif // DEBUG

    RtlCopyMemory(
        &PgBlock->_CaptureContext[0],
        _CaptureContext,
        RTL_NUMBER_OF(PgBlock->_CaptureContext));

    PgBlock->CaptureContext = __utop(&PgBlock->_CaptureContext[0]);

    // for debug
    // PgBlock->CaptureContext = _CaptureContext;

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > CaptureContext\n",
        PgBlock->CaptureContext);
#endif // DEBUG

    RtlCopyMemory(
        &PgBlock->_ClearMessage[0],
        ClearMessage[0],
        strlen(ClearMessage[0]));

    PgBlock->ClearMessage[0] = &PgBlock->_ClearMessage[0];

    // for debug
    // PgBlock->ClearMessage[0] = ClearMessage[0];

    RtlCopyMemory(
        &PgBlock->_ClearMessage[0x40],
        ClearMessage[1],
        strlen(ClearMessage[1]));

    PgBlock->ClearMessage[1] = &PgBlock->_ClearMessage[0x40];

    // for debug
    // PgBlock->ClearMessage[1] = ClearMessage[1];

    RtlCopyMemory(
        &PgBlock->_FreeWorker[0],
        PgFreeWorker,
        RTL_NUMBER_OF(PgBlock->_FreeWorker));

    PgBlock->FreeWorker = __utop(&PgBlock->_FreeWorker[0]);

    // for debug
    // PgBlock->FreeWorker = PgFreeWorker;

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > FreeWorker\n",
        PgBlock->FreeWorker);
#endif // DEBUG

    RtlCopyMemory(
        &PgBlock->_ClearCallback[0],
        PgClearCallback,
        RTL_NUMBER_OF(PgBlock->_ClearCallback));

    PgBlock->ClearCallback = __utop(&PgBlock->_ClearCallback[0]);

    // for debug
    // PgBlock->ClearCallback = PgClearCallback;

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > ClearCallback\n",
        PgBlock->ClearCallback);
#endif // DEBUG
}

PPGOBJECT
NTAPI
PgCreateObject(
    __in_opt u8 Encrypted,
    __in u8 Type,
    __in ptr BaseAddress,
    __in_opt u RegionSize,
    __in PPGBLOCK PgBlock,
    __in ptr Return,
    __in ptr Callback,
    __in_opt ptr ProgramCounter,
    __in ptr CaptureContext
)
{
    PPGOBJECT Object = NULL;

    Object = __malloc(sizeof(PGOBJECT));

    if (NULL != Object) {
        RtlZeroMemory(Object, sizeof(PGOBJECT));

        Object->Encrypted = Encrypted;
        Object->Type = Type;
        Object->BaseAddress = BaseAddress;
        Object->RegionSize = RegionSize;

        SetStackBody(
            &Object->Body,
            PgBlock,
            Object,
            Callback,
            Return,
            ProgramCounter,
            CaptureContext);

        ExInterlockedInsertTailList(
            &PgBlock->ObjectList, &Object->Entry, &PgBlock->ObjectLock);
    }

    return Object;
}

void
NTAPI
PgSetNewEntry(
    __inout PPGBLOCK PgBlock,
    __in PPGOBJECT Object,
    __in ptr PatchGuardContext,
    __in u64 RorKey
)
{
    u64 LastRorKey = 0;
    u32 RvaOfEntry = 0;
    u64 FieldBuffer[PG_COMPARE_FIELDS_COUNT] = { 0 };
    u32 FieldIndex = 0;
    u32 Index = 0;
    ptr Pointer = NULL;

    // xor code must be align 8 byte;
    // get PatchGuard entry offset in encrypted code

    FieldIndex = (PgBlock->OffsetEntryPoint -
        PgBlock->SizeCmpAppendDllSection) / sizeof(u64);

    RtlCopyMemory(
        FieldBuffer,
        (u8ptr)PatchGuardContext + (PgBlock->OffsetEntryPoint & ~7),
        sizeof(FieldBuffer));

    LastRorKey = RorKey;

    for (Index = 0;
        Index < FieldIndex;
        Index++) {
        LastRorKey = ROL64(PgBlock, LastRorKey, Index);
    }

    for (Index = 0;
        Index < RTL_NUMBER_OF(FieldBuffer);
        Index++) {
        LastRorKey = ROL64(PgBlock, LastRorKey, FieldIndex + Index);
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
    }

    RvaOfEntry = *(u32ptr)((u8ptr)FieldBuffer + (PgBlock->OffsetEntryPoint & 7));

    // copy PatchGuard entry head code to temp bufer and decode

    FieldIndex = (RvaOfEntry - PgBlock->SizeCmpAppendDllSection) / sizeof(u64);

    RtlCopyMemory(
        FieldBuffer,
        (u8ptr)PatchGuardContext + (RvaOfEntry & ~7),
        sizeof(FieldBuffer));

    LastRorKey = RorKey;

    for (Index = 0;
        Index < FieldIndex;
        Index++) {
        LastRorKey = ROL64(PgBlock, LastRorKey, Index);
    }

    for (Index = 0;
        Index < RTL_NUMBER_OF(FieldBuffer);
        Index++) {
        LastRorKey = ROL64(PgBlock, LastRorKey, FieldIndex + Index);
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
    }

    // set temp buffer PatchGuard entry head jmp to PgClearCallback and encrypt

    Pointer = (u8ptr)FieldBuffer + (RvaOfEntry & 7);

    LockedBuildJumpCode(&Pointer, &Object->Body);

    while (Index--) {
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
        LastRorKey = ROR64(PgBlock, LastRorKey, FieldIndex + Index);
    }

    // copy temp buffer PatchGuard entry head to old address, 
    // when PatchGuard code decrypt self jmp PgClearCallback.

    RtlCopyMemory(
        (u8ptr)PatchGuardContext + (RvaOfEntry & ~7),
        FieldBuffer,
        sizeof(FieldBuffer));

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > set new entry for encrypted context\n",
        Object);
#endif // DEBUG
}

void
NTAPI
PgSetNewEntryWithBtc(
    __inout PPGBLOCK PgBlock,
    __in PPGOBJECT Object,
    __in ptr Context,
    __in u ContextSize
)
{
    u64 RorKey = 0;
    u64 LastRorKey = 0;
    u32 LastRorKeyOffset = 0;
    u64 FieldBuffer[PG_COMPARE_FIELDS_COUNT] = { 0 };
    u32 FieldIndex = 0;
    u32 AlignOffset = 0;
    u32 Index = 0;
    u64ptr ControlPc = NULL;
    u64ptr TargetPc = NULL;
    u32 CompareCount = 0;
    ptr Pointer = NULL;

    // xor decrypt must align 8 bytes
    CompareCount = (ContextSize - PgBlock->SizeCmpAppendDllSection) / 8 - 1;

    for (AlignOffset = 0;
        AlignOffset < 8;
        AlignOffset++) {
        ControlPc = (u64ptr)((u8ptr)PgBlock->Header + AlignOffset);
        TargetPc = (u64ptr)((u8ptr)Context + PgBlock->SizeCmpAppendDllSection);

        for (Index = 0;
            Index < CompareCount;
            Index++) {
            RorKey = TargetPc[Index + 1] ^ ControlPc[1];
            LastRorKey = RorKey;
            RorKey = ROR64(PgBlock, RorKey, Index + 1);
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
        (u8ptr)Context + PgBlock->SizeCmpAppendDllSection + FieldIndex * 8,
        sizeof(FieldBuffer));

    RorKey = LastRorKey;
    Index = LastRorKeyOffset;

    while (Index--) {
        FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
        RorKey = ROR64(PgBlock, RorKey, FieldIndex + Index);
        RorKey = PgBlock->Btc64(RorKey, RorKey);
    }

    // set temp buffer PatchGuard entry head jmp to PgClearCallback and encrypt

    Pointer = (PGUARD_BODY)((u8ptr)FieldBuffer + 8 - AlignOffset);

    LockedBuildJumpCode(&Pointer, &Object->Body);

    RorKey = LastRorKey;
    Index = LastRorKeyOffset;

    while (Index--) {
        FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
        RorKey = ROR64(PgBlock, RorKey, FieldIndex + Index);
        RorKey = PgBlock->Btc64(RorKey, RorKey);
    }

    RtlCopyMemory(
        (u8ptr)Context + PgBlock->SizeCmpAppendDllSection + FieldIndex * 8,
        FieldBuffer,
        sizeof(FieldBuffer));

#ifdef DEBUG
    vDbgPrint(
        "[Shark] [PatchGuard] < %p > set new entry for btc encrypted context\n",
        Object);
#endif // DEBUG
}

void
NTAPI
PgCompareFields(
    __inout PPGBLOCK PgBlock,
    __in u8 Type,
    __in ptr BaseAddress,
    __in u RegionSize
)
{
    ptr EndAddress = NULL;
    u8ptr TargetPc = NULL;
    u Index = 0;
    u64 RorKey = 0;
    u32 RorKeyIndex = 0;
    u64ptr Fields = NULL;
    ptr Context = NULL;
    PPGOBJECT Object = NULL;
    PMMPTE PointerPde = NULL;
    PMMPTE PointerPte = NULL;
    PFN_NUMBER NumberOfPages = 0;
    b Chance = TRUE;
    u8 VaType = 0;

    if (0 != PgBlock->Repeat) {
        if (FALSE == IsListEmpty(&PgBlock->ObjectList)) {
            Object = CONTAINING_RECORD(
                PgBlock->ObjectList.Flink,
                PGOBJECT,
                Entry);

            while (&Object->Entry != &PgBlock->ObjectList) {
                if (BaseAddress == Object->BaseAddress)return;

                Object = CONTAINING_RECORD(
                    Object->Entry.Flink,
                    PGOBJECT,
                    Entry);
            }
        }
    }

    if (FALSE != MmIsAddressValid(BaseAddress)) {
        if ((__ptou(PgBlock) >= __ptou(BaseAddress)) &&
            (__ptou(PgBlock) < (__ptou(BaseAddress) + RegionSize))) {
            // pass self
        }
        else {
            if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
                if (MiVaSystemPtes == PgBlock->MiGetSystemRegionType(BaseAddress)) {
                    VaType = PgSystemPtes;
                }
                else if (MiVaNonPagedPool == PgBlock->MiGetSystemRegionType(BaseAddress)) {
                    VaType = PgPoolBigPage;
                }

                NumberOfPages = BYTES_TO_PAGES(RegionSize);

                for (Index = 0;
                    Index < NumberOfPages;
                    Index++) {
                    if (0 == _cmpword(
                        *(PSHORT)((u8ptr)BaseAddress + PAGE_SIZE * Index),
                        IMAGE_DOS_SIGNATURE)) {
                        Chance = FALSE;

                        // pass memory pe

                        break;
                    }
                }

                if (FALSE != Chance) {
                    if (PgPoolBigPage == VaType) {
                        PointerPde = GetPdeAddress(BaseAddress);

                        if (0 == PointerPde->u.Hard.LargePage) {
                            PointerPte = GetPteAddress(BaseAddress);

                            if (1 == PointerPte->u.Hard.NoExecute) {
                                Chance = FALSE;
                            }
                        }
                        else if (1 == PointerPde->u.Hard.NoExecute) {
                            Chance = FALSE;
                        }
                        else {
                            __debugbreak();
                        }
                    }
                    else {
                        if (ROUND_TO_PAGES(*(u64ptr)BaseAddress) != RegionSize) {
                            Chance = FALSE;
                        }
                    }
                }

                if (FALSE != Chance) {
                    if (FALSE != PgBlock->MmSetPageProtection(
                        BaseAddress,
                        PAGE_SIZE,
                        PAGE_READWRITE)) {
                        Object = PgCreateObject(
                            TRUE,
                            Type,
                            BaseAddress,
                            RegionSize,
                            PgBlock,
                            NULL,
                            PgBlock->ClearCallback,
                            NULL,
                            PgBlock->CaptureContext);

#ifdef DEBUG
                        vDbgPrint(
                            "[Shark] [PatchGuard] < %p > clear region execute mask < %p - %08x >\n",
                            Object,
                            BaseAddress,
                            RegionSize);
#endif // DEBUG
                    }
                }
            }
            else {
                TargetPc = BaseAddress;
                // EndAddress = (u8ptr)BaseAddress + RegionSize - sizeof(PgBlock->Fields);
                EndAddress = (u8ptr)BaseAddress + PAGE_SIZE - sizeof(PgBlock->Fields);

                do {
                    Fields = TargetPc;

                    if ((u64)Fields == (u64)PgBlock->Fields) {
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

#ifdef DEBUG
                            vDbgPrint(
                                "[Shark] [PatchGuard] < %p > found declassified context\n",
                                Context);
#endif // DEBUG
                            break;
                        }
                    }
                    else {
                        RorKeyIndex = 0;

                        RorKey = ROR64(PgBlock, RorKey, PG_FIELD_BITS);

                        if (PgBlock->BtcEnable) {
                            RorKey = PgBlock->Btc64(RorKey, RorKey);
                        }

                        RorKeyIndex++;

                        if ((u64)(Fields[2] ^ RorKey) == (u64)PgBlock->Fields[2]) {
                            RorKey = ROR64(PgBlock, RorKey, PG_FIELD_BITS - RorKeyIndex);

                            if (PgBlock->BtcEnable) {
                                RorKey = PgBlock->Btc64(RorKey, RorKey);
                            }

                            RorKeyIndex++;

                            if ((u64)(Fields[1] ^ RorKey) == (u64)PgBlock->Fields[1]) {
                                RorKey = ROR64(PgBlock, RorKey, PG_FIELD_BITS - RorKeyIndex);

                                if (PgBlock->BtcEnable) {
                                    RorKey = PgBlock->Btc64(RorKey, RorKey);
                                }

                                RorKeyIndex++;

                                if ((u64)(Fields[0] ^ RorKey) == (u64)PgBlock->Fields[0]) {
                                    Context = TargetPc - PG_FIRST_FIELD_OFFSET;

                                    RorKey = Fields[0] ^ PgBlock->Fields[0];

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] [PatchGuard] < %p > found encrypted context < %p - %08x >\n",
                                        Context,
                                        BaseAddress,
                                        RegionSize);
#endif // DEBUG

                                    Object = PgCreateObject(
                                        PgEncrypted,
                                        Type,
                                        BaseAddress,
                                        RegionSize,
                                        PgBlock,
                                        NULL,
                                        PgBlock->ClearCallback,
                                        NULL,
                                        PgBlock->CaptureContext);

                                    if (NULL != Object) {
                                        PgBlock->Count++;

                                        if (FALSE != PgBlock->BtcEnable) {
                                            PgSetNewEntryWithBtc(
                                                PgBlock,
                                                Object,
                                                Context,
                                                (u)EndAddress - (u)Context);
                                        }
                                        else {
                                            for (;
                                                RorKeyIndex < PG_FIELD_BITS;
                                                RorKeyIndex++) {
                                                RorKey = ROR64(PgBlock, RorKey, PG_FIELD_BITS - RorKeyIndex);
                                            }

#ifdef DEBUG
                                            vDbgPrint(
                                                "[Shark] [PatchGuard] < %p > first rorkey\n",
                                                RorKey);
#endif // DEBUG
                                            PgSetNewEntry(PgBlock, Object, Context, RorKey);
                                        }
                                    }

                                    break;
                                }
                            }
                        }
                    }

                    TargetPc++;
                } while ((u64)TargetPc < (u64)EndAddress);
            }
        }
    }
}

void
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
    ptr PointerAddress = NULL;
    u32 BitNumber = 0;
    ptr BeginAddress = NULL;
    ptr EndAddress = NULL;

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

    BeginAddress = GetVaMappedByPte(BasePte);
    EndAddress = GetVaMappedByPte(BasePte + NumberOfPtes);

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

                        PointerAddress = GetVaMappedByPte(PointerPte + 1);
                    }
                    else {
                        PointerAddress = GetVaMappedByPde(PointerPde + 1);
                    }
                }
                else {
                    PointerAddress = GetVaMappedByPde(PointerPde + 1);
                }
            }
            else {
                PointerAddress = GetVaMappedByPpe(PointerPpe + 1);
            }
        }
        else {
            PointerAddress = GetVaMappedByPxe(PointerPxe + 1);
        }
    } while ((u)PointerAddress < (u)EndAddress);
}

void
NTAPI
PgClearSystemPtesEncryptedContext(
    __inout PPGBLOCK PgBlock
)
{
    PRTL_BITMAP BitMap = NULL;
    u32 BitMapSize = 0;
    PFN_NUMBER NumberOfPtes = 0;
    u32 HintIndex = 0;
    u32 StartingRunIndex = 0;

    NumberOfPtes = PgBlock->SystemPtes.NumberOfPtes;

#ifdef DEBUG
    if (0 == PgBlock->Repeat) {
        vDbgPrint(
            "[Shark] [PatchGuard] < %p > SystemPtes < %p - %p >\n",
            KeGetCurrentProcessorNumber(),
            PgBlock->SystemPtes.BasePte,
            PgBlock->SystemPtes.BasePte + NumberOfPtes);
    }
#endif // DEBUG

    BitMapSize =
        sizeof(RTL_BITMAP) + (u32)((((NumberOfPtes + 1) + 31) / 32) * 4);

    BitMap = __malloc(BitMapSize);

    if (NULL != BitMap) {
        RtlInitializeBitMap(
            BitMap,
            (u32ptr)(BitMap + 1),
            (u32)(NumberOfPtes + 1));

        RtlClearAllBits(BitMap);

        InitializeSystemPtesBitMap(
            PgBlock->SystemPtes.BasePte,
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
                        GetVaMappedByPte(PgBlock->SystemPtes.BasePte + HintIndex),
                        (StartingRunIndex - HintIndex) * PAGE_SIZE);
                }

                HintIndex = StartingRunIndex;
            }
        } while (HintIndex < NumberOfPtes);

        __free(BitMap);
    }
}

void
NTAPI
PgClearPoolEncryptedContext(
    __inout PPGBLOCK PgBlock
)
{
    u Index = 0;

#ifdef DEBUG
    if (0 == PgBlock->Repeat) {
        vDbgPrint(
            "[Shark] [PatchGuard] < %p > BigPool < %p - %08x >\n",
            KeGetCurrentProcessorNumber(),
            POOL_TABLE,
            POOL_TABLE_SIZE);
    }
#endif // DEBUG

    for (Index = 0;
        Index < POOL_TABLE_SIZE;
        Index++) {
        if (POOL_BIG_TABLE_ENTRY_FREE !=
            ((u64)POOL_TABLE[Index].Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            if (FALSE !=
                PgBlock->Pool.MmIsNonPagedSystemAddressValid(POOL_TABLE[Index].Va)) {
                if (POOL_TABLE[Index].NumberOfPages > PgBlock->SizeINITKDBG) {
                    PgCompareFields(
                        PgBlock,
                        PgPoolBigPage,
                        POOL_TABLE[Index].Va,
                        POOL_TABLE[Index].NumberOfPages);
                }
            }
        }
    }
}

void
NTAPI
PgLocatePoolObject(
    __inout PPGBLOCK PgBlock,
    __in ptr Establisher,
    __in PPGOBJECT Object
)
{
    PFN_NUMBER Index = 0;

    for (Index = 0;
        Index < POOL_TABLE_SIZE;
        Index++) {
        if (POOL_BIG_TABLE_ENTRY_FREE !=
            ((u64)POOL_TABLE[Index].Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            if (FALSE !=
                PgBlock->Pool.MmIsNonPagedSystemAddressValid(
                    POOL_TABLE[Index].Va)) {
                if (POOL_TABLE[Index].NumberOfPages > PgBlock->SizeINITKDBG) {
                    if ((u64)Establisher >= (u64)POOL_TABLE[Index].Va &&
                        (u64)Establisher < (u64)POOL_TABLE[Index].Va +
                        POOL_TABLE[Index].NumberOfPages) {
                        Object->BaseAddress = POOL_TABLE[Index].Va;
                        Object->RegionSize = POOL_TABLE[Index].NumberOfPages;

#ifdef DEBUG
                        GetGpBlock(PgBlock)->vDbgPrint(
                            "[Shark] [PatchGuard] < %p > found region in pool < %p - %08x >\n",
                            Establisher,
                            Object->BaseAddress,
                            Object->RegionSize);
#endif // DEBUG

                        Object->Type = PgPoolBigPage;

                        break;
                    }
                }
            }
        }
    }
}

void
NTAPI
PgLocateSystemPtesObject(
    __inout PPGBLOCK PgBlock,
    __in ptr Establisher,
    __in PPGOBJECT Object
)
{
    PRTL_BITMAP BitMap = NULL;
    u32 BitMapSize = 0;
    PFN_NUMBER NumberOfPtes = 0;
    u32 HintIndex = 0;
    u32 StartingRunIndex = 0;

    NumberOfPtes = PgBlock->SystemPtes.NumberOfPtes;

    BitMapSize =
        sizeof(RTL_BITMAP) +
        (u32)((((NumberOfPtes + 1) + 31) / 32) * 4);

    BitMap = __malloc(BitMapSize);

    if (NULL != BitMap) {
        RtlInitializeBitMap(
            BitMap,
            (u32ptr)(BitMap + 1),
            (u32)(NumberOfPtes + 1));

        RtlClearAllBits(BitMap);

        InitializeSystemPtesBitMap(
            PgBlock->SystemPtes.BasePte,
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

                if ((u64)Establisher >=
                    (u64)GetVaMappedByPte(
                        PgBlock->SystemPtes.BasePte + HintIndex) &&
                        (u64)Establisher <
                    (u64)GetVaMappedByPte(
                        PgBlock->SystemPtes.BasePte + StartingRunIndex) - PgBlock->SizeCmpAppendDllSection) {
                    // align clear execute mask region

                    if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
                        Object->BaseAddress =
                            GetVaMappedByPte(PgBlock->SystemPtes.BasePte + HintIndex - 1);

                        Object->RegionSize =
                            (u)(StartingRunIndex - HintIndex + 1) * PAGE_SIZE;
                    }
                    else {
                        Object->BaseAddress =
                            GetVaMappedByPte(PgBlock->SystemPtes.BasePte + HintIndex);

                        Object->RegionSize =
                            (u)(StartingRunIndex - HintIndex) * PAGE_SIZE;
                    }
#ifdef DEBUG
                    vDbgPrint(
                        "[Shark] [PatchGuard] < %p > found region in system ptes < %p - %08x >\n",
                        Establisher,
                        Object->BaseAddress,
                        Object->RegionSize);
#endif // DEBUG

                    Object->Type = PgSystemPtes;

                    break;
                }

                HintIndex = StartingRunIndex;
            }
        } while (HintIndex < NumberOfPtes);

        __free(BitMap);
    }
}

void
NTAPI
PgLocateAllObject(
    __inout PPGBLOCK PgBlock,
    __in ptr Establisher,
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

void
NTAPI
PgCheckAllWorkerThread(
    __inout PPGBLOCK PgBlock
)
{
    PETHREAD Thread = NULL;
    PEXCEPTION_FRAME ExceptionFrame = NULL;
    u64 ControlPc = 0;
    u64 EstablisherFrame = 0;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    ptr HandlerData = NULL;
    u64 ImageBase = 0;
    PPGOBJECT Object = NULL;
    u32 Index = 0;

    struct {
        CONTEXT ContextRecord;
        KNONVOLATILE_CONTEXT_POINTERS ContextPointers;
        PGOBJECT Object;
    }*Context;

    if (NULL != PgBlock->INITKDBG) {
        Context =
            __malloc(sizeof(CONTEXT)
                + sizeof(KNONVOLATILE_CONTEXT_POINTERS)
                + sizeof(PGOBJECT));

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
                    (u64)PgBlock->ExpWorkerThread == __rdsptr(
                    (u8ptr)Thread +
                        GetGpBlock(PgBlock)->OffsetKThreadWin32StartAddress)) {
                    // SwapContext
                    ExceptionFrame = (PEXCEPTION_FRAME)__rdsptr(
                        (u8ptr)Thread +
                        GetGpBlock(PgBlock)->DebuggerDataBlock.OffsetKThreadKernelStack);

                    if (FALSE != MmIsAddressValid(ExceptionFrame)) {
                        EstablisherFrame =
                            (u64)(ExceptionFrame + 1) +
                            KSTART_FRAME_LENGTH + sizeof(ptr);

                        Context->ContextRecord.Rip = __rdsptr(EstablisherFrame);
                        Context->ContextRecord.Rsp = EstablisherFrame + sizeof(ptr);

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

                        while (EstablisherFrame >= __rdu64(
                            (u8ptr)Thread +
                            GetGpBlock(PgBlock)->DebuggerDataBlock.OffsetKThreadKernelStack) &&
                            EstablisherFrame < __rdu64(
                            (u8ptr)Thread +
                                GetGpBlock(PgBlock)->DebuggerDataBlock.OffsetKThreadInitialStack)) {
                            if (FALSE !=
                                MmIsAddressValid((ptr)Context->ContextRecord.Rip)) {
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
                                    break;
                                }
                            }
                            else {
                                break;
                            }
                        }

                        if (FALSE !=
                            MmIsAddressValid((ptr)Context->ContextRecord.Rip)) {
                            if (0 != Context->ContextRecord.Rip) {
#ifdef DEBUG
                                vDbgPrint(
                                    "[Shark] [PatchGuard] < %p > found noimage return address in worker thread stack\n",
                                    Context->ContextRecord.Rip);
#endif // DEBUG

                                for (Index = 0;
                                    Index < PgBlock->SizeINITKDBG - PG_COMPARE_BYTE_COUNT;
                                    Index++) {
                                    if (PG_COMPARE_BYTE_COUNT == RtlCompareMemory(
                                        (u8ptr)PgBlock->INITKDBG + Index,
                                        (ptr)Context->ContextRecord.Rip,
                                        PG_COMPARE_BYTE_COUNT)) {
                                        Context->Object.Type = -1;

                                        PgLocateAllObject(
                                            PgBlock,
                                            (ptr)Context->ContextRecord.Rip,
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
                                                PgBlock->CaptureContext);

                                            if (NULL != Object) {
                                                PgBlock->Count++;

                                                // align stack must start at Body.Parameter
                                                __wruptr(
                                                    Context->ContextRecord.Rsp - 8,
                                                    &Object->Body.Parameter);

#ifdef DEBUG
                                                vDbgPrint(
                                                    "[Shark] [PatchGuard] < %p > insert worker thread check code\n",
                                                    Object);
#endif // DEBUG
                                            }
                                        }

                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                Thread = GetNexThread(GetGpBlock(PgBlock), Thread);
            }

            __free(Context);
        }

        __free(PgBlock->INITKDBG);

        PgBlock->INITKDBG = NULL;
    }
}

void
NTAPI
PgClearAll(
    __inout PPGBLOCK PgBlock
)
{

    if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
        GetGpBlock(PgBlock)->BugCheckHandle = SafeGuardAttach(
            (ptr *)&GetGpBlock(PgBlock)->KeBugCheckEx,
            PgBlock->ClearCallback,
            NULL,
            NULL,
            PgBlock);
    }

    PgClearPoolEncryptedContext(PgBlock);

    if (GetGpBlock(PgBlock)->BuildNumber >= 9200) {
        PgClearSystemPtesEncryptedContext(PgBlock);
    }

    PgCheckAllWorkerThread(PgBlock);
}

void
NTAPI
PgClearWorker(
    __inout ptr Argument
)
{
    u32 Index = 0;
    u64ptr InitialStack = 0;
    DISPATCHER_HEADER * Header = NULL;
    u32 ReturnLength = 0;
    u8ptr TargetPc = NULL;
    u32 Length = 0;
    b Chance = TRUE;

    struct {
        PPGBLOCK PgBlock;
        KEVENT Notify;
        WORK_QUEUE_ITEM Worker;
    }*Context;

    Context = Argument;

    if (0 == Context->PgBlock->Repeat) {
        InitializePgBlock(Context->PgBlock);

        /*
        if (os build >= 9600){
            // Header->Type = 0x15
            // Header->Hand = 0xac

            PgBlock->WorkerContext = struct _DISPATCHER_HEADER
        }
        else {
            // CriticalWorkQueue = 0
            // DelayedWorkQueue = 1

            PgBlock->WorkerContext = enum _WORK_QUEUE_TYPE
        }
        */

        InitialStack = IoGetInitialStack();

        if (GetGpBlock(Context->PgBlock)->BuildNumber >= 9600) {
            while ((u64)InitialStack != (u64)&Argument) {
                Header = *(ptr *)InitialStack;

                if (FALSE != MmIsAddressValid(Header)) {
                    if (FALSE != MmIsAddressValid((u8ptr)(Header + 1) - 1)) {
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
            sizeof(ptr),
            &ReturnLength))) {
            for (Index = 0;
                Index < GetGpBlock(Context->PgBlock)->DebuggerDataBlock.SizeEThread;
                Index += 8) {
                if ((u)Context->PgBlock->ExpWorkerThread ==
                    __rdsptr((u8ptr)KeGetCurrentThread() + Index)) {
                    GetGpBlock(Context->PgBlock)->OffsetKThreadWin32StartAddress = Index;

                    break;
                }
            }

            if (GetGpBlock(Context->PgBlock)->BuildNumber >= 18362) {
                TargetPc = (u8ptr)Context->PgBlock->ExpWorkerThread;

                while (TRUE) {
                    Length = DetourGetInstructionLength(TargetPc);

                    if (6 == Length) {
                        if (0 == _cmpbyte(TargetPc[0], 0x8b) &&
                            0 == _cmpbyte(TargetPc[1], 0x83)) {
                            Context->PgBlock->OffsetSameThreadPassive = *(u32ptr)(TargetPc + 2);

#ifdef DEBUG
                            vDbgPrint(
                                "[Shark] [PatchGuard] < %p > OffsetSameThreadPassive\n",
                                Context->PgBlock->OffsetSameThreadPassive);
#endif // DEBUG

                            break;
                        }
                    }

                    TargetPc += Length;
                }
            }
        }
    }

    if (0 == Context->PgBlock->SizeCmpAppendDllSection ||
        0 == Context->PgBlock->OffsetEntryPoint ||
        0 == Context->PgBlock->SizeINITKDBG ||
        NULL == GetGpBlock(Context->PgBlock)->PsInvertedFunctionTable ||
        NULL == Context->PgBlock->KiStartSystemThread ||
        NULL == Context->PgBlock->PspSystemThreadStartup ||
        NULL == Context->PgBlock->Pool.PoolBigPageTable ||
        NULL == Context->PgBlock->Pool.PoolBigPageTableSize) {
        Chance = FALSE;
    }

    if (GetGpBlock(Context->PgBlock)->BuildNumber >= 9200) {
        if (0 == Context->PgBlock->SystemPtes.NumberOfPtes ||
            NULL == Context->PgBlock->SystemPtes.BasePte ||
            NULL == Context->PgBlock->MmAllocateIndependentPages ||
            NULL == Context->PgBlock->MmFreeIndependentPages ||
            NULL == Context->PgBlock->MmSetPageProtection) {
            Chance = FALSE;
        }
    }

    if (GetGpBlock(Context->PgBlock)->BuildNumber >= 9600) {
        if (0 == Context->PgBlock->WorkerContext) {
            Chance = FALSE;
        }
    }

    if (GetGpBlock(Context->PgBlock)->BuildNumber >= 18362) {
        if (0 == Context->PgBlock->OffsetSameThreadPassive) {
            Chance = FALSE;
        }
    }

    if (FALSE != Chance) {
        IpiSingleCall(
            (PGKERNEL_ROUTINE)NULL,
            (PGSYSTEM_ROUTINE)NULL,
            (PGRUNDOWN_ROUTINE)PgClearAll,
            (PGNORMAL_ROUTINE)Context->PgBlock);
    }

    KeSetEvent(&Context->Notify, LOW_PRIORITY, FALSE);
}

void
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

    if (PgBlock->Cleared < PG_MAXIMUM_CONTEXT_COUNT) {
        KeInitializeEvent(&Context.Notify, SynchronizationEvent, FALSE);
        ExInitializeWorkItem(&Context.Worker, PgClearWorker, &Context);
        ExQueueWorkItem(&Context.Worker, CriticalWorkQueue);

        KeWaitForSingleObject(
            &Context.Notify,
            Executive,
            KernelMode,
            FALSE,
            NULL);

        PgBlock->Repeat++;
    }
}
