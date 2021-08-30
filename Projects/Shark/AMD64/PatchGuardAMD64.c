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
* The Initial Developer of the Original Code is blindtiger.
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
#endif // DEBUG

        GetGpBlock(PgBlock)->ExInterlockedRemoveHeadList(
            Object->Entry.Blink,
            &PgBlock->ObjectLock);

        if (PgDoubleEncrypted == Object->Encrypted) {
            Context->Rip = __rdsptr(Context->Rsp);
            Context->Rsp += sizeof(ptr);

            ExInitializeWorkItem(
                &Object->Worker, PgBlock->FreeWorker, Object);

            PgBlock->ExQueueWorkItem(
                &Object->Worker, CriticalWorkQueue);
        }
        else if (PgEncrypted == Object->Encrypted) {
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
    u Index = 0;
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    ptr EndToLock = NULL;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    LONG64 Diff = 0;
    UNICODE_STRING RoutineString = { 0 };
    ptr RoutineAddress = NULL;
    u8 Selector = 0;
    PPOOL_BIG_PAGES * PageTable = NULL;
    uptr PageTableSize = NULL;
    PRTL_BITMAP BitMap = NULL;

    u TempField = 0;

    s8 CmpAppendDllSection[] = "2E 48 31 11 48 31 51 08 48 31 51 10 48 31 51 18";

    // Fields
    s8 Fields[] = "FB 48 8D 05";
    s8 FirstField[] = "?? 89 ?? 00 01 00 00 48 8D 05 ?? ?? ?? ?? ?? 89 ?? 08 01 00 00";
    s8 NextField[] = "48 8D 05 ?? ?? ?? ?? ?? 89 86 ?? ?? 00 00";

    // BranchKey

    // 48 8D 05 B7 08 72 FF                     lea     rax, KiDispatchCallout
    // 49 89 86 E8 09 00 00                     mov     [r14 + 9E8h], rax
    // 48 8D 05 39 DD 79 FF                     lea     rax, xHalTimerWatchdogStop
    // 49 89 86 F0 09 00 00                     mov     [r14 + 9F0h], rax
    // 41 C7 86 54 09 00 00 60 68 00 59         mov     dword ptr [r14 + 954h], 59006860h
    // E9 4F 02 00 00                           jmp     loc_1409D48BD
    // 83 F8 05                                 cmp     eax, 5
    // 0F 85 3C 02 00 00                        jnz     loc_1409D48B3
    // 45 8B 86 10 08 00 00                     mov     r8d, [r14 + 810h]
    // 0F 31                                    rdtsc

    // 8B 84 24 60 24 00 00                     mov     eax, [rsp + 2458h + arg_0]
    // BA 05 00 00 00                           mov     edx, 5
    // 3B C2                                    cmp     eax, edx
    // 0F 86 C8 06 00 00                        jbe     loc_1409E5933
    // 48 8D 3D 8E 4A 98 FF                     lea     rdi, KiTimerDispatch
    // 83 F8 06                                 cmp     eax, 6
    // 0F 84 8E 0A 00 00                        jz      loc_1409E5D09
    // 83 F8 07                                 cmp     eax, 7
    // 0F 84 77 0A 00 00                        jz      loc_1409E5CFB
    // 83 F8 08                                 cmp     eax, 8
    // 0F 84 60 0A 00 00                        jz      loc_1409E5CED
    // 83 F8 09                                 cmp     eax, 9
    // 0F 84 49 0A 00 00                        jz      loc_1409E5CDF
    // 0F 31                                    rdtsc

    // 48 F7 E1                                 mul     rcx
    // 48 8B C1                                 mov     rax, rcx
    // 48 2B C2                                 sub     rax, rdx
    // 48 D1 E8                                 shr     rax, 1
    // 48 03 C2                                 add     rax, rdx
    // 48 C1 E8 02                              shr     rax, 2
    // 48 6B C0 07                              imul    rax, 7
    // 48 2B C8                                 sub     rcx, rax
    // 8B C1                                    mov     eax, ecx
    // 48 8D 15 B1 52 05 00                     lea     rdx, off_140A3EE80
    // 48 8B 14 C2                              mov     rdx, [rdx+rax*8]
    // 48 8B 86 90 06 00 00                     mov     rax, [rsi+690h]
    // 48 8B 4C 24 40                           mov     rcx, [rsp+0A8h+var_68]
    // 48 89 14 01                              mov     [rcx+rax], rdx
    // 48 8B 86 A0 06 00 00                     mov     rax, [rsi+6A0h]
    // 48 89 14 01                              mov     [rcx+rax], rdx
    // 48 89 8E 98 09 00 00                     mov     [rsi+998h], rcx
    // C7 86 54 09 00 00 72 68 80 F8            mov     dword ptr [rsi+954h], 0F8806872h
    // 0F 31                                    rdtsc
    // 48 C1 E2 20                              shl     rdx, 20h

    s8ptr BranchKey[] = {
        "48 8D 05 ?? ?? ?? ?? 49 89 86 ?? ?? 00 00 41 C7 86 ?? ?? 00 00 ?? ?? ?? ?? E9 ?? ?? ?? ?? 83 F8 05",
        "8B 84 24 ?? ?? ?? ?? ?? 05 00 00 00 3B ?? 0F 86 ?? ?? ?? ?? ?? 8D ?? ?? ?? ?? ?? 83 F8 06",
        "48 F7 E1 48 8B C1 48 2B C2 48 D1 E8 48 03 C2 48 C1 E8 02 48 6B C0 07 48 2B C8 8B C1 48 8D"
    };

    // only 9600 and later
    // KeSetTimerEx
    // 48 8B 05 18 50 22 00                     mov     rax, cs:KiWaitNever

    s8 KiWaitNever[] = "48 B8 00 00 00 00 80 F7 FF FF";

    // 48 8B 1D E9 50 22 00                     mov     rbx, cs:KiWaitAlways

    s8 KiWaitAlways[] = "FB B8 00 00 14 00 33 D2 F7";

    s8 KiStartSystemThread[] = "B9 01 00 00 00 44 0F 22 C1 48 8B 14 24 48 8B 4C 24 08";
    s8 PspSystemThreadStartup[] = "EB ?? B9 1E 00 00 00 E8";

    // 7600 ~ 7601
    // 48 83 3D F5 DF 0C 00 00                  cmp     cs:PoolBigPageTable, 0
    // 75 10                                    jnz     short loc_140150235
    // 4D 85 C0                                 test    r8, r8
    // 74 04                                    jz      short loc_14015022E
    // 41 83 20 00                              and     dword ptr [r8], 0
    // 33 C0                                    xor     eax, eax
    // E9 DB 02 00 00                           jmp     loc_140150510
    // 48 8B 35 D4 DF 0C 00                     mov     rsi, cs:PoolBigPageTableSize

    // 9200 ~ 18950
    // 83 FE 01                                 cmp     esi, 1
    // 75 10                                    jnz     short loc_1401BDE4E
    // 48 8B 15 8B 36 0D 00                     mov     rdx, cs:PoolBigPageTable
    // 48 8B 35 9C 36 0D 00                     mov     rsi, cs:PoolBigPageTableSize
    // EB 29                                    jmp     short loc_1401BDE77

    // 19041
    // 40 8A F0                                 mov     sil, al
    // 83 BC 24 B0 00 00 00 01                  cmp     dword ptr [rsp+0B0h], 1
    // 75 15                                    jnz     short loc_1405BBE06
    // 48 8B 15 60 AA 65 00                     mov     rdx, cs:PoolBigPageTable
    // 48 8B 0D 71 AA 65 00                     mov     rcx, cs:PoolBigPageTableSize
    // 48 89 4C 24 40                           mov     [rsp+40h], rcx
    // EB 1A                                    jmp     short loc_1405BBE20

    // 20279
    // 40 8A F0                                 mov     sil, al
    // 41 83 FE 01                              cmp     r14d, 1
    // 75 10                                    jnz     short loc_14061C6D5
    // 48 8B 15 2C 0D 60 00                     mov     rdx, cs:qword_140C1D3F8
    // 4C 8B 35 3D 0D 60 00                     mov     r14, cs:qword_140C1D410
    // EB 15                                    jmp     short loc_14061C6EA

    // 21327
    // 40 8A F0                                 mov     sil, al
    // 41 83 FE 01                              cmp     r14d, 1
    // 75 1A                                    jnz     short loc_140624CCE
    // 48 8B 15 BD C8 5E 00                     mov     rdx, cs:PoolBigPageTable
    // 48 89 54 24 48                           mov     [rsp+98h+Src], rdx
    // 48 8B 0D C9 C8 5E 00                     mov     rcx, cs:PoolBigPageTableSize
    // 48 89 4C 24 40                           mov     [rsp+98h+var_58], rcx
    // EB 19                                    jmp     short loc_140624CE7

    struct {
        u8 Offset[4];
        u8ptr Signature;
    } ExGetBigPoolInfo[] = {
        { 3, 1, 0x1D, 0, "48 83 3D ?? ?? ?? ?? 00 75 10 4D 85 C0 74 04 41 83 20 00 33 C0" },
        { 8, 0, 0xF, 0, "83 ?? 01 75 10 48 8B 15 ?? ?? ?? ?? ?? 8B ?? ?? ?? ?? ?? EB" },
        { 0xD, 0, 0x14, 0, "83 BC 24 ?? 00 00 00 01 75 15 48 8B 15 ?? ?? ?? ?? 48 8B" },
        { 0xC, 0, 0x13, 0, "40 8A F0 41 83 FE 01 75 ?? 48 8B 15" },
        { 0xC, 0, 0x18, 0, "40 8A F0 41 83 FE 01 75 ?? 48 8B 15" }
    };

    // 48 B8 00 00 00 00 00 80 FF FF	        mov     rax, 0FFFF800000000000h
    // 48 3B C8                     	        cmp     rcx, rax
    // 72 1D                        	        jb      short loc_14020A90C
    // 48 C1 E9 27                  	        shr     rcx, 27h
    // 81 E1 FF 01 00 00            	        and     ecx, 1FFh
    // 8D 81 00 FF FF FF            	        lea     eax, [rcx-100h]
    // 48 8D 0D 02 7F A4 00         	        lea     rcx, unk_140C52808
    // 0F B6 04 08                  	        movzx   eax, byte ptr [rax+rcx]

    s8 MiGetSystemRegionType[] =
        "48 c1 e9 27 81 e1 ff 01 00 00 8d 81 00 ff ff ff 48 8d 0d";

    // 55                                       push    rbp
    // 41 54                                    push    r12
    // 41 55                                    push    r13
    // 41 56                                    push    r14
    // 41 57                                    push    r15
    // 48 81 EC C0 02 00 00                     sub     rsp, 2C0h
    // 48 8D A8 D8 FD FF FF                     lea     rbp, [rax - 228h]
    // 48 83 E5 80                              and     rbp, 0FFFFFFFFFFFFFF80h

    // 55                                       push    rbp
    // 41 54                                    push    r12
    // 41 55                                    push    r13
    // 41 56                                    push    r14
    // 41 57                                    push    r15
    // 48 8d 68 a1                              lea     rbp, [rax - 5Fh]
    // 48 81 ec b0 00 00 00                     sub     rsp, 0B0h
    // 8b 82 58 09 00 00                        mov     eax, dword ptr [rdx + 958h]

    u8ptr Header[2] = {
        "55 41 54 41 55 41 56 41 57 48 81 EC C0 02 00 00 48 8D A8 D8 FD FF FF 48 83 E5 80",
        "55 41 54 41 55 41 56 41 57 48 8D 68 A1 48 81 EC B0 00 00 00 8B 82"
    };

    s8 ReservedCrossThreadFlags[] =
        "89 83 ?? ?? F0 83 0C 24 00 80 3D ?? ?? ?? ?? ?? 0F";

    u64 Ror64[] = { 0xC3C8D348CA869148 };
    u64 Rol64[] = { 0xC3C0D348CA869148 };
    u64 RorWithBtc64[] = { 0x48C8D348CA869148, 0xCCCCCCCCC3C0BB0F };

    // 4892            xchg    rax, rdx
    // 4801c8          add     rax, rcx
    // c3              ret

    // 4892            xchg    rax, rdx
    // 480fafc1        imul    rax, rcx
    // c3              ret

    u64 PostCache[] = { 0xCCCCC3C801489248, 0xCCC3C1AF0F489248 };

    // 48c7c0c8000000  mov     rax, 0C8h
    // 482bc1          sub     rax, rcx
    // 4833c1          xor     rax, rcx
    // 4887ca          xchg    rcx, rdx
    // 48f7d1          not     rcx
    // 80e13f          and     cl, 3Fh
    // 48d3c8          ror     rax, cl
    // c3              ret

    // 48c7c0c8000000  mov     rax, 0C8h
    // 482bc1          sub     rax, rcx
    // 480fafc1        imul    rax, rcx
    // 4887ca          xchg    rcx, rdx
    // 48f7d1          not     rcx
    // 80e13f          and     cl, 3Fh
    // 48d3c8          ror     rax, cl
    // c3              ret

    u64 PostKey[] = {
        0x48000000C8C0C748, 0xCA8748C13348C12B, 0xD3483FE180D1F748, 0xCCCCCCCCCCCCC3C8,
        0x48000000C8C0C748, 0x8748C1AF0F48C12B, 0x483FE180D1F748CA, 0xCCCCCCCCCCC3C8D3
    };

    cptr ClearMessage[3] = {
        "[Shark] < %p > declassified context cleared\n",
        "[Shark] < %p > encrypted context cleared\n",
        "[Shark] < %p > double encrypted context cleared\n"
    };

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > PgBlock\n",
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
                                        "[Shark] < %p > SizeCmpAppendDllSection\n",
                                        PgBlock->SizeCmpAppendDllSection);
#endif // DEBUG

                                    if (0 == _cmpbyte(TargetPc[11], 0x48) ||
                                        0 == _cmpbyte(TargetPc[12], 0x0f) ||
                                        0 == _cmpbyte(TargetPc[13], 0xbb) ||
                                        0 == _cmpbyte(TargetPc[14], 0xc0)) {
                                        PgBlock->BtcEnable = TRUE;

#ifdef DEBUG
                                        vDbgPrint(
                                            "[Shark] < %p > BtcEnable\n",
                                            PgBlock->BtcEnable);
#endif // DEBUG
                                    }

                                    while (0 != _cmpqword(
                                        0x085131481131482E,
                                        *(uptr)TargetPc)) {
                                        TargetPc--;
                                    }

                                    RtlCopyMemory(
                                        PgBlock->_OriginalCmpAppendDllSection,
                                        TargetPc,
                                        sizeof(PgBlock->_OriginalCmpAppendDllSection));

                                    PgBlock->OriginalCmpAppendDllSection =
                                        (ptr)PgBlock->_OriginalCmpAppendDllSection;

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > OriginalCmpAppendDllSection\n",
                                        PgBlock->OriginalCmpAppendDllSection);
#endif // DEBUG
                                }
                            }
                        }

                        if (6 == Length) {
                            if (0 == _cmpbyte(TargetPc[0], 0x8b) &&
                                0 == _cmpbyte(TargetPc[1], 0x82)) {
                                PgBlock->OffsetEntryPoint = *(u32ptr)(TargetPc + 2);

#ifdef DEBUG
                                vDbgPrint(
                                    "[Shark] < %p > OffsetEntryPoint\n",
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
                            "[Shark] < %p > SizeINITKDBG\n",
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
                                "[Shark] < %p > INITKDBG\n",
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
                                "[Shark]",
                                (ptr)PgBlock->Fields[0]);
#endif // DEBUG

                            PgBlock->Fields[1] =
                                (u64)__rva_to_va(TargetPc + 10) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[Shark]",
                                (ptr)PgBlock->Fields[1]);
#endif // DEBUG

                            PgBlock->Fields[2] =
                                (u64)__rva_to_va(TargetPc + 24) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[Shark]",
                                (ptr)PgBlock->Fields[2]);
#endif // DEBUG

                            PgBlock->Fields[3] =
                                (u64)__rva_to_va(TargetPc + 38) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[Shark]",
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
                                                "[Shark] < %p > MmAllocateIndependentPages\n",
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
                                            "[Shark] < %p > MmFreeIndependentPages\n",
                                            PgBlock->MmFreeIndependentPages);
#endif // DEBUG

                                        TempField = (u64)__rva_to_va(TargetPc + 45) + Diff;

                                        RtlCopyMemory(
                                            &PgBlock->MmSetPageProtection,
                                            &TempField,
                                            sizeof(ptr));

#ifdef DEBUG
                                        vDbgPrint(
                                            "[Shark] < %p > MmSetPageProtection\n",
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
                                        "[Shark] < %p > PsInvertedFunctionTable\n",
                                        GetGpBlock(PgBlock)->PsInvertedFunctionTable);
#endif // DEBUG

                                    break;
                                }

                                TargetPc++;
                            }

                            if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
                                TargetPc = ScanBytes(
                                    TargetPc,
                                    (u8ptr)ViewBase + ViewSize,
                                    BranchKey[0]);

                                if (NULL != TargetPc) {
                                    PgBlock->BranchKey[10] = __rds32(TargetPc + 0x15);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[10]\n",
                                        PgBlock->BranchKey[10]);
#endif // DEBUG
                                }

                                TargetPc = ScanBytes(
                                    TargetPc,
                                    (u8ptr)ViewBase + ViewSize,
                                    BranchKey[1]);

                                if (NULL != TargetPc) {
                                    ControlPc = __rva_to_va(TargetPc + 0x10);

                                    PgBlock->BranchKey[0] = __rds32(
                                        __ptou(__rva_to_va(ControlPc + 0xA)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[0]\n",
                                        PgBlock->BranchKey[0]);
#endif // DEBUG

                                    PgBlock->BranchKey[1] = __rds32(
                                        __ptou(__rva_to_va(ControlPc + 0x13)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[1]\n",
                                        PgBlock->BranchKey[1]);
#endif // DEBUG

                                    PgBlock->BranchKey[2] = __rds32(
                                        __ptou(__rva_to_va(ControlPc + 0x1C)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[2]\n",
                                        PgBlock->BranchKey[2]);
#endif // DEBUG

                                    PgBlock->BranchKey[3] = __rds32(
                                        __ptou(__rva_to_va(ControlPc + 0x25)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[3]\n",
                                        PgBlock->BranchKey[3]);
#endif // DEBUG

                                    PgBlock->BranchKey[4] = __rds32(ControlPc + 0x31);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[4]\n",
                                        PgBlock->BranchKey[4]);
#endif // DEBUG

                                    PgBlock->BranchKey[5] = __rds32(
                                        __ptou(__rva_to_va(ControlPc + 2)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[5]\n",
                                        PgBlock->BranchKey[5]);
#endif // DEBUG

                                    ControlPc = TargetPc + 0x1B;

                                    // PgBlock->BranchKey[6] = PgBlock->BranchKey[5];

                                    PgBlock->BranchKey[6] = 0; // same with 5, here use 0.

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[6]\n",
                                        PgBlock->BranchKey[6]);
#endif // DEBUG

                                    PgBlock->BranchKey[7] = __rds32(
                                        __ptou(__rva_to_va(ControlPc + 0xE)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[7]\n",
                                        PgBlock->BranchKey[7]);
#endif // DEBUG

                                    PgBlock->BranchKey[8] = __rds32(
                                        __ptou(__rva_to_va(ControlPc + 0x17)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[8]\n",
                                        PgBlock->BranchKey[8]);
#endif // DEBUG

                                    PgBlock->BranchKey[9] = __rds32(
                                        __ptou(__rva_to_va(ControlPc + 0x20)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[9]\n",
                                        PgBlock->BranchKey[9]);
#endif // DEBUG
                                }

                                TargetPc = ScanBytes(
                                    TargetPc,
                                    (u8ptr)ViewBase + ViewSize,
                                    BranchKey[2]);

                                if (NULL != TargetPc) {
                                    PgBlock->BranchKey[11] = __rds32(TargetPc + 0x4A);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > BranchKey[11]\n",
                                        PgBlock->BranchKey[11]);
#endif // DEBUG
                                }
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
                        "[Shark] < %p > KiStartSystemThread\n",
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
                            "[Shark] < %p > PspSystemThreadStartup\n",
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

    if (GetGpBlock(PgBlock)->BuildNumber >= 9600) {
        RtlInitUnicodeString(&RoutineString, L"KeSetTimerEx");

        TargetPc = MmGetSystemRoutineAddress(&RoutineString);

        if (NULL != TargetPc) {
            FunctionEntry = RtlLookupFunctionEntry(
                (u64)TargetPc,
                (u64ptr)&ImageBase,
                NULL);

            if (NULL != FunctionEntry) {
                do {
                    Length = DetourGetInstructionLength(TargetPc);

                    if (7 == Length) {
                        if (0 == _cmpbyte(TargetPc[0], 0x48) &&
                            0 == _cmpbyte(TargetPc[1], 0x8B)) {
                            ControlPc = __rva_to_va(TargetPc + 3);

                            if (NULL == PgBlock->KiWaitNever) {
                                PgBlock->KiWaitNever = ControlPc;

#ifdef DEBUG
                                vDbgPrint(
                                    "[Shark] < %p > KiWaitNever\n",
                                    PgBlock->KiWaitNever);
#endif // DEBUG
                            }
                            else {
                                PgBlock->KiWaitAlways = ControlPc;

#ifdef DEBUG
                                vDbgPrint(
                                    "[Shark] < %p > KiWaitAlways\n",
                                    PgBlock->KiWaitAlways);
#endif // DEBUG

                                break;
                            }
                        }
                    }

                    TargetPc += Length;
                } while (TargetPc < ((u8ptr)ImageBase + FunctionEntry->EndAddress));
            }
        }
    }

    RtlInitUnicodeString(&RoutineString, L"MmIsNonPagedSystemAddressValid");

    PgBlock->Pool.MmIsNonPagedSystemAddressValid = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > MmIsNonPagedSystemAddressValid\n",
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

        if (GetGpBlock(PgBlock)->BuildNumber >= 15063) {
            TargetPc = ScanBytes(
                ControlPc,
                EndToLock,
                MiGetSystemRegionType);

            if (NULL != TargetPc) {
                PgBlock->SystemRegionTypeArray = __rva_to_va(TargetPc + 0x13);

#ifdef DEBUG
                vDbgPrint(
                    "[Shark] < %p > SystemRegionTypeArray\n",
                    PgBlock->SystemRegionTypeArray);
#endif // DEBUG
            }
        }

        if (GetGpBlock(PgBlock)->BuildNumber >= 7600 &&
            GetGpBlock(PgBlock)->BuildNumber < 9200) {
            Selector = 0;
        }
        else if (GetGpBlock(PgBlock)->BuildNumber >= 9200 &&
            GetGpBlock(PgBlock)->BuildNumber < 19041) {
            Selector = 1;
        }
        else if (GetGpBlock(PgBlock)->BuildNumber >= 19041 &&
            GetGpBlock(PgBlock)->BuildNumber < 20000) {
            Selector = 2;
        }
        else if (GetGpBlock(PgBlock)->BuildNumber >= 20000 &&
            GetGpBlock(PgBlock)->BuildNumber < 21000) {
            Selector = 3;
        }
        else if (GetGpBlock(PgBlock)->BuildNumber >= 21000 &&
            GetGpBlock(PgBlock)->BuildNumber < 22000) {
            Selector = 4;
        }
        else if (GetGpBlock(PgBlock)->BuildNumber >= 22000) {
            Selector = 3;
        }

        ControlPc = ScanBytes(
            ControlPc,
            EndToLock,
            ExGetBigPoolInfo[Selector].Signature);

        if (NULL != ControlPc) {
            PgBlock->Pool.PoolBigPageTable =
                __rva_to_va_ex(
                    ControlPc + ExGetBigPoolInfo[Selector].Offset[0],
                    ExGetBigPoolInfo[Selector].Offset[1]);

#ifdef DEBUG
            vDbgPrint(
                "[Shark] < %p > PoolBigPageTable\n",
                PgBlock->Pool.PoolBigPageTable);
#endif // DEBUG

            PgBlock->Pool.PoolBigPageTableSize =
                __rva_to_va_ex(
                    ControlPc + ExGetBigPoolInfo[Selector].Offset[2],
                    +ExGetBigPoolInfo[Selector].Offset[3]);

#ifdef DEBUG
            vDbgPrint(
                "[Shark] < %p > PoolBigPageTableSize\n",
                PgBlock->Pool.PoolBigPageTableSize);
#endif // DEBUG
        }
    }

    if (GetGpBlock(PgBlock)->BuildNumber < 21000) {
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
                            "[Shark] < %p > NumberOfPtes\n",
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
                            "[Shark] < %p > BasePte\n",
                            PgBlock->SystemPtes.BasePte);
#endif // DEBUG

                        break;
                    }
                }

                ControlPc += Length;
            }
        }
    }
    else {
        for (Index = 0;
            Index < SYSTEM_REGION_TYPE_ARRAY_COUNT;
            Index++) {
            if (MiVaKernelStacks ==
                PgBlock->SystemRegionTypeArray[Index]) {
                if (NULL == PgBlock->SystemPtes.BasePte) {
                    PgBlock->SystemPtes.BasePte =
                        GetPteAddress(__utop((Index | 0xFFFFFFE0) << 0x27));
                }

                PgBlock->SystemPtes.NumberOfPtes += 0x40000000;
            }
            else {
                if (NULL != PgBlock->SystemPtes.BasePte) {
                    break;
                }
            }
        }

#ifdef DEBUG
        vDbgPrint(
            "[Shark] < %p > BasePte\n",
            PgBlock->SystemPtes.BasePte);

        vDbgPrint(
            "[Shark] < %p > NumberOfPtes\n",
            PgBlock->SystemPtes.NumberOfPtes);
#endif // DEBUG
    }

    RtlCopyMemory(
        PgBlock->_Ror64,
        Ror64,
        sizeof(PgBlock->_Ror64));

    PgBlock->Ror64 = __utop(PgBlock->_Ror64);

    RtlCopyMemory(
        PgBlock->_Rol64,
        Rol64,
        sizeof(PgBlock->_Rol64));

    PgBlock->Rol64 = __utop(PgBlock->_Rol64);

    RtlCopyMemory(
        PgBlock->_RorWithBtc64,
        RorWithBtc64,
        sizeof(PgBlock->_RorWithBtc64));

    PgBlock->RorWithBtc64 = __utop(PgBlock->_RorWithBtc64);

    PgBlock->CmpDecode =
        PgBlock->BtcEnable ?
        __utop(PgBlock->RorWithBtc64) : __utop(PgBlock->Ror64);

    if (GetGpBlock(PgBlock)->BuildNumber > 20000) {
        PgBlock->BuildKey = 0x00000009UL;
    }
    else if (GetGpBlock(PgBlock)->BuildNumber == 18362) {
        PgBlock->BuildKey = 0xFFFFFFE6UL;
    }

    PgBlock->CacheCmpAppendDllSection = PgBlock->_CacheCmpAppendDllSection;

    RtlCopyMemory(
        PgBlock->_PostCache,
        PostCache,
        sizeof(PgBlock->_PostCache));

    PgBlock->PostCache =
        GetGpBlock(PgBlock)->BuildNumber > 18362 ?
        __utop(PgBlock->_PostCache) : __utop(PgBlock->_PostCache + 8);

    RtlCopyMemory(
        PgBlock->_PostKey,
        PostKey,
        sizeof(PgBlock->_PostKey));

    PgBlock->PostKey =
        GetGpBlock(PgBlock)->BuildNumber >= 18362 ?
        __utop(PgBlock->_PostKey) : __utop(PgBlock->_PostKey + 0x20);

    RtlInitUnicodeString(&RoutineString, L"RtlLookupFunctionEntry");

    PgBlock->RtlLookupFunctionEntry = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > RtlLookupFunctionEntry\n",
        PgBlock->RtlLookupFunctionEntry);
#endif // DEBUG

    RtlInitUnicodeString(&RoutineString, L"RtlVirtualUnwind");

    PgBlock->RtlVirtualUnwind = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > RtlVirtualUnwind\n",
        PgBlock->RtlVirtualUnwind);
#endif // DEBUG

    RtlInitUnicodeString(&RoutineString, L"ExQueueWorkItem");

    PgBlock->ExQueueWorkItem = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > ExQueueWorkItem\n",
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
        "[Shark] < %p > CaptureContext\n",
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
        &PgBlock->_ClearMessage[0x80],
        ClearMessage[2],
        strlen(ClearMessage[2]));

    PgBlock->ClearMessage[2] = &PgBlock->_ClearMessage[0x80];

    // for debug
    // PgBlock->ClearMessage[2] = ClearMessage[2];

    RtlCopyMemory(
        &PgBlock->_FreeWorker[0],
        PgFreeWorker,
        RTL_NUMBER_OF(PgBlock->_FreeWorker));

    PgBlock->FreeWorker = __utop(&PgBlock->_FreeWorker[0]);

    // for debug
    // PgBlock->FreeWorker = PgFreeWorker;

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > FreeWorker\n",
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
        "[Shark] < %p > ClearCallback\n",
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
        LastRorKey = PgBlock->Rol64(LastRorKey, Index);
    }

    for (Index = 0;
        Index < RTL_NUMBER_OF(FieldBuffer);
        Index++) {
        LastRorKey = PgBlock->Rol64(LastRorKey, FieldIndex + Index);
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
        LastRorKey = PgBlock->Rol64(LastRorKey, Index);
    }

    for (Index = 0;
        Index < RTL_NUMBER_OF(FieldBuffer);
        Index++) {
        LastRorKey = PgBlock->Rol64(LastRorKey, FieldIndex + Index);
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
    }

    // set temp buffer PatchGuard entry head jmp to PgClearCallback and encrypt

    Pointer = (u8ptr)FieldBuffer + (RvaOfEntry & 7);

    LockedBuildJumpCode(&Pointer, &Object->Body);

    while (Index--) {
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
        LastRorKey = PgBlock->Ror64(LastRorKey, FieldIndex + Index);
    }

    // copy temp buffer PatchGuard entry head to old address, 
    // when PatchGuard code decrypt self jmp PgClearCallback.

    RtlCopyMemory(
        (u8ptr)PatchGuardContext + (RvaOfEntry & ~7),
        FieldBuffer,
        sizeof(FieldBuffer));

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > set new entry for encrypted context\n",
        Object);
#endif // DEBUG
}

u
NTAPI
PgScanEntryWithBtc(
    __inout PPGBLOCK PgBlock,
    __in PPGOBJECT Object,
    __out u32ptr AlignOffset,
    __out uptr LastKey
)
{
    u Result = 0;
    u64 RorKey = 0;
    u32 Index = 0;
    u32 Align = 0;
    u64ptr ControlPc = NULL;
    u64ptr TargetPc = NULL;
    u32 CompareCount = 0;

    // xor decrypt must align 8 bytes
    CompareCount = (Object->ContextSize - PgBlock->SizeCmpAppendDllSection) / 8 - 1;
    TargetPc = __utop(__ptou(Object->Context) + PgBlock->SizeCmpAppendDllSection);

    do {
        ControlPc = __utop(PgBlock->Header + Align);

        for (Index = 0;
            Index < CompareCount;
            Index++) {
            RorKey = TargetPc[Index + 1] ^ ControlPc[1];

            *LastKey = RorKey;

            RorKey = PgBlock->CmpDecode(RorKey, Index + 1);

            if ((TargetPc[Index] ^ RorKey) == ControlPc[0]) {
                Result = Index;
                *AlignOffset = Align;

                break;
            }
        }

        Align++;
    } while (Align < 8 && Index == CompareCount);

    return Result;
}

void
NTAPI
PgSetNewEntryWithBtc(
    __inout PPGBLOCK PgBlock,
    __in PPGOBJECT Object
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

    Index = PgScanEntryWithBtc(
        PgBlock,
        Object,
        &AlignOffset,
        &LastRorKey);

    if (0 == Index) {
#ifdef DEBUG
        vDbgPrint(
            "[Shark] < %p > entrypoint not found!\n",
            Object);
#endif // DEBUG
    }
    else {
        FieldIndex = Index - (0 == (AlignOffset & 7) ? 0 : 1);
        LastRorKeyOffset = 2 + (0 == (AlignOffset & 7) ? 0 : 1);

        RtlCopyMemory(
            FieldBuffer,
            (u8ptr)Object->Context + PgBlock->SizeCmpAppendDllSection + FieldIndex * sizeof(u),
            sizeof(FieldBuffer));

        RorKey = LastRorKey;
        Index = LastRorKeyOffset;

        while (Index--) {
            FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
            RorKey = PgBlock->CmpDecode(RorKey, FieldIndex + Index);
        }

        // set temp buffer PatchGuard entry head jmp to PgClearCallback and encrypt

        Pointer = (PGUARD_BODY)((u8ptr)FieldBuffer + sizeof(u) - AlignOffset);

        LockedBuildJumpCode(&Pointer, &Object->Body);

        RorKey = LastRorKey;
        Index = LastRorKeyOffset;

        while (Index--) {
            FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
            RorKey = PgBlock->CmpDecode(RorKey, FieldIndex + Index);
        }

        RtlCopyMemory(
            (u8ptr)Object->Context + PgBlock->SizeCmpAppendDllSection + FieldIndex * sizeof(u),
            FieldBuffer,
            sizeof(FieldBuffer));

#ifdef DEBUG
        vDbgPrint(
            "[Shark] < %p > set new entry for btc encrypted context\n",
            Object);
#endif // DEBUG
    }
}

b
NTAPI
PgFastScanTimer(
    __inout PPGBLOCK PgBlock,
    __in ptr Context
)
{
    b Result = 0;
    u ControlPc = 0;
    u Original = 0;
    u Key = 0;
    u ControlKey = 0;
    u Index = 0;
    u KeyIndex = 0;
    u Count = 3;
    u XorKey = 0;
    u Cache = 0;

    u8 KeyTable[] = {
        0x00, 0x03, 0x05, 0x08, 0x06, 0x09, 0x0C, 0x07, 0x0D, 0x0A, 0x0E, 0x04, 0x01, 0x0F, 0x0B, 0x02
    };

    ControlKey = __ptou(Context);
    ControlKey = PgBlock->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        ControlPc = __ptou(Context) + Index * 8;
        Cache = Original = __rdu64(ControlPc);
        Cache = Original ^ ControlKey;
        Cache += PgBlock->PostCache(Index, Context);

        ControlKey ^= PgBlock->PostKey(Index, Original);

        ControlKey = PgBlock->Rol64(
            ControlKey,
            (0 == PgBlock->BuildKey ? ~Original : Original ^ PgBlock->BuildKey) & 0x3F);

        ControlKey += __ptou(Context);

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Cache = PgBlock->Rol64(Cache, 4);

            Key = KeyTable[Cache & 0xF];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;
        }

        __wruptr(PgBlock->CacheCmpAppendDllSection + Index, Cache);

        Index++;
    } while (Index < Count);

    XorKey =
        PgBlock->OriginalCmpAppendDllSection[1] ^ PgBlock->CacheCmpAppendDllSection[1];

    if ((PgBlock->OriginalCmpAppendDllSection[2] ^ PgBlock->CacheCmpAppendDllSection[2]) == XorKey) {
        Result = TRUE;
    }

    return Result;
}

u
NTAPI
PgFastScanBranch(
    __inout PPGBLOCK PgBlock,
    __in ptr Context,
    __inout u32 BranchKey
)
{
    u Result = 0;
    u ControlPc = 0;
    u Original = 0;
    u Key = 0;
    u ControlKey = 0;
    u Index = 0;
    u KeyIndex = 0;
    u Count = 3;
    u XorKey = 0;
    u Cache = 0;

    u8 KeyTable[] = {
        0x00, 0x03, 0x05, 0x08, 0x06, 0x09, 0x0C, 0x07, 0x0D, 0x0A, 0x0E, 0x04, 0x01, 0x0F, 0x0B, 0x02
    };

    ControlKey = __ptou(Context);
    ControlKey = PgBlock->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        ControlPc = __ptou(Context) + Index * 8;
        Cache = Original = __rdu64(ControlPc);

        Cache ^= *PgBlock->KiWaitNever;

        Cache =
            PgBlock->Rol64(Cache, *PgBlock->KiWaitNever) ^ ControlKey;

        Cache =
            _byteswap_uint64(Cache);

        Cache ^= *PgBlock->KiWaitAlways;
        Cache += PgBlock->PostCache(Index, Context);

        ControlKey ^= PgBlock->PostKey(Index, Original);

        ControlKey = PgBlock->Rol64(
            ControlKey,
            (0 == PgBlock->BuildKey ? ~Original : Original ^ PgBlock->BuildKey) & 0x3F);

        ControlKey += __ptou(Context);
        ControlKey ^= BranchKey;

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Cache = PgBlock->Rol64(Cache, 4);

            Key = KeyTable[Cache & 0xF];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;
        }

        __wruptr(PgBlock->CacheCmpAppendDllSection + Index, Cache);

        Index++;
    } while (Index < Count);

    XorKey =
        PgBlock->OriginalCmpAppendDllSection[1] ^ PgBlock->CacheCmpAppendDllSection[1];

    if ((PgBlock->OriginalCmpAppendDllSection[2] ^ PgBlock->CacheCmpAppendDllSection[2]) == XorKey) {
        Result = TRUE;
    }

    return Result;
}

void
NTAPI
PgSetTimerNewEntry(
    __inout PPGBLOCK PgBlock,
    __in PPGOBJECT Object
)
{
    u Original = 0;
    u Key = 0;
    u ControlKey = 0;
    u Index = 0;
    u KeyIndex = 0;
    u Count = 0;
    u Cache = 0;
    ptr Pointer = NULL;
    b Pass = FALSE;

    u8 KeyTable[] = {
        0x00, 0x03, 0x05, 0x08, 0x06, 0x09, 0x0C, 0x07, 0x0D, 0x0A, 0x0E, 0x04, 0x01, 0x0F, 0x0B, 0x02,
        0x00, 0x0C, 0x0F, 0x01, 0x0B, 0x02, 0x04, 0x07, 0x03, 0x05, 0x09, 0x0E, 0x06, 0x08, 0x0A, 0x0D
    };

    Count = 0xC8 / 8;

    ControlKey = __ptou(Object->Context);
    ControlKey = PgBlock->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        Cache = Original = __rdu64(__ptou(Object->Context) + Index * 8);
        Cache = Original ^ ControlKey;
        Cache += PgBlock->PostCache(Index, Object->Context);

        ControlKey ^= PgBlock->PostKey(Index, Original);

        ControlKey = PgBlock->Rol64(
            ControlKey,
            (0 == PgBlock->BuildKey ? ~Original : Original ^ PgBlock->BuildKey) & 0x3F);

        ControlKey += __ptou(Object->Context);

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Cache = PgBlock->Rol64(Cache, 4);

            Key = KeyTable[Cache & 0xF];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;

            if (FALSE != Pass) break;
        }

        __wru64(__ptou(Object->Context) + Index * 8, Cache);

        Index++;

        if (Index == (0xC8 / 8)) {
            Object->Key =
                PgBlock->OriginalCmpAppendDllSection[1] ^ PgBlock->CacheCmpAppendDllSection[1];

            Count = Cache ^ Object->Key;
            Count >>= 0x20;
            Count &= 0xFFFFFFFFUL;
            Count += 0xC8 / 8;

            Object->ContextSize = sizeof(u) * Count;

            Pass = TRUE;
        }
    } while (Index < Count);

    Object->Context[2] ^= Object->Key;
    Object->Context[3] ^= Object->Key;

    Pointer = &Object->Context[2];

    LockedBuildJumpCode(&Pointer, &Object->Body);

    Object->Context[2] ^= Object->Key;
    Object->Context[3] ^= Object->Key;

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > set new entry for double encrypted context\n",
        Object);
#endif // DEBUG

    Index = 0;
    Pass = FALSE;
    ControlKey = __ptou(Object->Context);
    ControlKey = PgBlock->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        Cache = __rdu64(__ptou(Object->Context) + Index * 8);

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Key = KeyTable[0x10 + (Cache & 0xF)];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;

            Cache = PgBlock->Ror64(Cache, 4);

            if (FALSE != Pass) break;
        }

        Cache -= PgBlock->PostCache(Index, Object->Context);

        Cache ^= ControlKey;

        ControlKey ^= PgBlock->PostKey(Index, Cache);

        ControlKey = PgBlock->Rol64(
            ControlKey,
            (0 == PgBlock->BuildKey ? ~Cache : Cache ^ PgBlock->BuildKey) & 0x3F);

        ControlKey += __ptou(Object->Context);

        __wru64(__ptou(Object->Context) + Index * 8, Cache);

        Index++;

        if (Index == (0xC8 / 8)) {
            Pass = TRUE;
        }
    } while (Index < Count);
}

void
NTAPI
PgSetBranchNewEntry(
    __inout PPGBLOCK PgBlock,
    __inout PPGOBJECT Object,
    __in u32 BranchKey
)
{
    u Original = 0;
    u Key = 0;
    u ControlKey = 0;
    u Index = 0;
    u KeyIndex = 0;
    u Count = 0;
    u Cache = 0;
    ptr Pointer = NULL;
    b Pass = FALSE;

    u8 KeyTable[] = {
        0x00, 0x03, 0x05, 0x08, 0x06, 0x09, 0x0C, 0x07, 0x0D, 0x0A, 0x0E, 0x04, 0x01, 0x0F, 0x0B, 0x02,
        0x00, 0x0C, 0x0F, 0x01, 0x0B, 0x02, 0x04, 0x07, 0x03, 0x05, 0x09, 0x0E, 0x06, 0x08, 0x0A, 0x0D
    };

    Count = 0xC8 / 8;

    ControlKey = __ptou(Object->Context);
    ControlKey = PgBlock->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        Cache = Original = __rdu64(__ptou(Object->Context) + Index * 8);

        Cache ^= *PgBlock->KiWaitNever;
        Cache = PgBlock->Rol64(Cache, *PgBlock->KiWaitNever) ^ ControlKey;
        Cache = _byteswap_uint64(Cache);
        Cache ^= *PgBlock->KiWaitAlways;
        Cache += PgBlock->PostCache(Index, Object->Context);

        ControlKey ^= PgBlock->PostKey(Index, Original);

        ControlKey = PgBlock->Rol64(
            ControlKey,
            (0 == PgBlock->BuildKey ? ~Original : Original ^ PgBlock->BuildKey) & 0x3F);

        ControlKey += __ptou(Object->Context);
        ControlKey ^= BranchKey;

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Cache = PgBlock->Rol64(Cache, 4);

            Key = KeyTable[Cache & 0xF];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;

            if (FALSE != Pass) break;
        }

        __wru64(__ptou(Object->Context) + Index * 8, Cache);

        Index++;

        if (Index == (0xC8 / 8)) {
            Object->Key =
                PgBlock->OriginalCmpAppendDllSection[1] ^ PgBlock->CacheCmpAppendDllSection[1];

            Count = Cache ^ Object->Key;
            Count >>= 0x20;
            Count &= 0xFFFFFFFFUL;
            Count += 0xC8 / 8;

            Object->ContextSize = sizeof(u) * Count;

            Pass = TRUE;
        }
    } while (Index < Count);

    Object->Context[2] ^= Object->Key;
    Object->Context[3] ^= Object->Key;

    Pointer = &Object->Context[2];

    LockedBuildJumpCode(&Pointer, &Object->Body);

    Object->Context[2] ^= Object->Key;
    Object->Context[3] ^= Object->Key;

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > set new entry for double encrypted context\n",
        Object);
#endif // DEBUG

    Index = 0;
    Pass = FALSE;
    ControlKey = __ptou(Object->Context);
    ControlKey = PgBlock->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        Cache = __rdu64(__ptou(Object->Context) + Index * 8);

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Key = KeyTable[0x10 + (Cache & 0xF)];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;

            Cache = PgBlock->Ror64(Cache, 4);

            if (FALSE != Pass) break;
        }

        Cache -= PgBlock->PostCache(Index, Object->Context);
        Cache ^= *PgBlock->KiWaitAlways;
        Cache = _byteswap_uint64(Cache);
        Cache ^= ControlKey;
        Cache = PgBlock->Ror64(Cache, *PgBlock->KiWaitNever);
        Cache ^= *PgBlock->KiWaitNever;

        ControlKey ^= PgBlock->PostKey(Index, Cache);

        ControlKey = PgBlock->Rol64(
            ControlKey,
            (0 == PgBlock->BuildKey ? ~Cache : Cache ^ PgBlock->BuildKey) & 0x3F);

        ControlKey += __ptou(Object->Context);
        ControlKey ^= BranchKey;

        __wru64(__ptou(Object->Context) + Index * 8, Cache);

        Index++;

        if (Index == (0xC8 / 8)) {
            Pass = TRUE;
        }
    } while (Index < Count);
}

PPGOBJECT
NTAPI
PgCompareDoubleEncryptedFields(
    __inout PPGBLOCK PgBlock,
    __in u8 Type,
    __in ptr BaseAddress,
    __in u RegionSize
)
{
    ptr EndAddress = NULL;
    u8ptr TargetPc = NULL;
    u Index = 0;
    PPGOBJECT Object = NULL;
    u Result = 0;

    // search first page
    EndAddress = (u8ptr)BaseAddress
        + PAGE_SIZE + PgBlock->SizeCmpAppendDllSection;

    for (Index = 0;
        Index < RTL_NUMBER_OF(PgBlock->BranchKey) && 0 == Result;
        Index++) {
        TargetPc = BaseAddress;

        do {
            Result = PgFastScanBranch(
                PgBlock,
                TargetPc,
                PgBlock->BranchKey[Index]);

            if (0 != Result) {
                Object = PgCreateObject(
                    PgDoubleEncrypted,
                    Type,
                    BaseAddress,
                    RegionSize,
                    PgBlock,
                    NULL,
                    PgBlock->ClearCallback,
                    NULL,
                    PgBlock->CaptureContext);

                if (NULL != Object) {
                    Object->Context = TargetPc;

                    PgSetBranchNewEntry(
                        PgBlock,
                        Object,
                        PgBlock->BranchKey[Index]);
                }

                break;
            }
            else {
                Result = PgFastScanTimer(PgBlock, TargetPc);

                if (0 != Result) {
                    Object = PgCreateObject(
                        PgDoubleEncrypted,
                        Type,
                        BaseAddress,
                        RegionSize,
                        PgBlock,
                        NULL,
                        PgBlock->ClearCallback,
                        NULL,
                        PgBlock->CaptureContext);

                    if (NULL != Object) {
                        Object->Context = TargetPc;

                        PgSetTimerNewEntry(PgBlock, Object);
                    }

                    break;
                }
            }
        } while (__ptou(TargetPc++) < __ptou(EndAddress));
    }

    return Object;
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
    b Result = 0;

    if (FALSE != MmIsAddressValid(BaseAddress)) {
        if ((__ptou(PgBlock) >= __ptou(BaseAddress)) &&
            (__ptou(PgBlock) < (__ptou(BaseAddress) + RegionSize))) {
            // pass self
        }
        else {
            if (GetGpBlock(PgBlock)->BuildNumber >= 18362) {
                Object = PgCompareDoubleEncryptedFields(
                    PgBlock,
                    Type,
                    BaseAddress,
                    RegionSize);

                if (NULL != Object) {
#ifdef DEBUG
                    vDbgPrint(
                        "[Shark] < %p > found encrypted context < %p - %08x >\n",
                        Object->Context,
                        BaseAddress,
                        RegionSize);
#endif // DEBUG
                }
            }
            else {
                TargetPc = BaseAddress;

                // only search one page
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
                                "[Shark] < %p > found declassified context\n",
                                Context);
#endif // DEBUG
                            break;
                        }
                    }
                    else {
                        RorKeyIndex = 0;

                        RorKey = PgBlock->CmpDecode(RorKey, PG_FIELD_BITS);

                        RorKeyIndex++;

                        if ((u64)(Fields[2] ^ RorKey) == (u64)PgBlock->Fields[2]) {
                            RorKey = PgBlock->CmpDecode(RorKey, PG_FIELD_BITS - RorKeyIndex);

                            RorKeyIndex++;

                            if ((u64)(Fields[1] ^ RorKey) == (u64)PgBlock->Fields[1]) {
                                RorKey = PgBlock->CmpDecode(RorKey, PG_FIELD_BITS - RorKeyIndex);

                                RorKeyIndex++;

                                if ((u64)(Fields[0] ^ RorKey) == (u64)PgBlock->Fields[0]) {
                                    Context = TargetPc - PG_FIRST_FIELD_OFFSET;

                                    RorKey = Fields[0] ^ PgBlock->Fields[0];

#ifdef DEBUG
                                    vDbgPrint(
                                        "[Shark] < %p > found encrypted context < %p - %08x >\n",
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

                                        Object->Context = Context;

                                        Object->ContextSize =
                                            RegionSize - (__ptou(Context) - __ptou(BaseAddress));

                                        if (FALSE != PgBlock->BtcEnable) {
                                            PgSetNewEntryWithBtc(PgBlock, Object);
                                        }
                                        else {
                                            for (;
                                                RorKeyIndex < PG_FIELD_BITS;
                                                RorKeyIndex++) {
                                                RorKey = PgBlock->Ror64(RorKey, PG_FIELD_BITS - RorKeyIndex);
                                            }

                                            Object->Key = RorKey;

#ifdef DEBUG
                                            vDbgPrint(
                                                "[Shark] < %p > first rorkey\n",
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
    vDbgPrint(
        "[Shark] < %p > SystemPtes < %p - %p >\n",
        KeGetCurrentProcessorNumber(),
        PgBlock->SystemPtes.BasePte,
        PgBlock->SystemPtes.BasePte + NumberOfPtes);
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
                    /*
#ifdef DEBUG
                    vDbgPrint(
                        "[Shark] < %p > scan < %p - %08x > < %p, %p, %p, %p...>\n",
                        KeGetCurrentProcessorNumber(),
                        GetVaMappedByPte(PgBlock->SystemPtes.BasePte + HintIndex),
                        (StartingRunIndex - HintIndex) * PAGE_SIZE,
                        __rduptr(GetVaMappedByPte(PgBlock->SystemPtes.BasePte + HintIndex)),
                        __rduptr(__ptou(GetVaMappedByPte(PgBlock->SystemPtes.BasePte + HintIndex)) + 8),
                        __rduptr(__ptou(GetVaMappedByPte(PgBlock->SystemPtes.BasePte + HintIndex)) + 0x10),
                        __rduptr(__ptou(GetVaMappedByPte(PgBlock->SystemPtes.BasePte + HintIndex)) + 0x18));
#endif // DEBUG
                    */

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
    PPOOL_BIG_PAGES PoolBigPage = NULL;
    u Index = 0;
    u Offset = 0;
    PMMPTE PointerPde = NULL;
    PMMPTE PointerPte = NULL;
    b Pass = FALSE;

    Offset =
        GetGpBlock(PgBlock)->BuildNumber > 20000 ?
        sizeof(POOL_BIG_PAGESEX) : sizeof(POOL_BIG_PAGES);

#ifdef DEBUG
    vDbgPrint(
        "[Shark] < %p > BigPool < %p - %08x >\n",
        KeGetCurrentProcessorNumber(),
        *PgBlock->Pool.PoolBigPageTable,
        *PgBlock->Pool.PoolBigPageTableSize);
#endif // DEBUG

    for (Index = 0;
        Index < *PgBlock->Pool.PoolBigPageTableSize;
        Index++) {
        PoolBigPage =
            __utop(
                __ptou(*PgBlock->Pool.PoolBigPageTable)
                + Index * Offset);

        if (POOL_BIG_TABLE_ENTRY_FREE !=
            ((u64)PoolBigPage->Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            if (PoolBigPage->NumberOfPages >= PgBlock->SizeINITKDBG) {
                if (0 !=
                    PgBlock->Pool.MmIsNonPagedSystemAddressValid(
                        PoolBigPage->Va)) {

                    PointerPde = GetPdeAddress(PoolBigPage->Va);

                    if (1 == PointerPde->u.HardLarge.LargePage) {
                        if (1 == PointerPde->u.Hard.NoExecute) {
                            continue;
                        }
                    }
                    else {
                        PointerPte = GetPteAddress(PoolBigPage->Va);

                        if (1 == PointerPte->u.Hard.NoExecute) {
                            continue;
                        }
                    }

                    /*
#ifdef DEBUG
                    vDbgPrint(
                        "[Shark] < %p > scan < %p - %08x > < %p, %p, %p, %p...>\n",
                        KeGetCurrentProcessorNumber(),
                        PAGE_ALIGN(PoolBigPage->Va),
                        PoolBigPage->NumberOfPages,
                        __rduptr(PAGE_ALIGN(PoolBigPage->Va)),
                        __rduptr(__ptou(PAGE_ALIGN(PoolBigPage->Va)) + 8),
                        __rduptr(__ptou(PAGE_ALIGN(PoolBigPage->Va)) + 0x10),
                        __rduptr(__ptou(PAGE_ALIGN(PoolBigPage->Va)) + 0x18));
#endif // DEBUG
                    */

                    PgCompareFields(
                        PgBlock,
                        PgPoolBigPage,
                        PoolBigPage->Va,
                        PoolBigPage->NumberOfPages);
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
    PPOOL_BIG_PAGES PoolBigPage = NULL;
    u Index = 0;
    u Offset = 0;

    Offset =
        GetGpBlock(PgBlock)->BuildNumber > 20000 ?
        sizeof(POOL_BIG_PAGESEX) : sizeof(POOL_BIG_PAGES);

    for (Index = 0;
        Index < *PgBlock->Pool.PoolBigPageTableSize;
        Index++) {
        PoolBigPage =
            __utop(
                __ptou(*PgBlock->Pool.PoolBigPageTable)
                + Index * Offset);

        if (POOL_BIG_TABLE_ENTRY_FREE !=
            ((u64)PoolBigPage->Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            if (FALSE !=
                PgBlock->Pool.MmIsNonPagedSystemAddressValid(
                    PoolBigPage->Va)) {
                if (PoolBigPage->NumberOfPages > PgBlock->SizeINITKDBG) {
                    if ((u64)Establisher >= (u64)PoolBigPage->Va &&
                        (u64)Establisher < (u64)PoolBigPage->Va +
                        PoolBigPage->NumberOfPages) {
                        Object->BaseAddress = PoolBigPage->Va;
                        Object->RegionSize = PoolBigPage->NumberOfPages;

#ifdef DEBUG
                        GetGpBlock(PgBlock)->vDbgPrint(
                            "[Shark] < %p > found region in pool < %p - %08x >\n",
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
                        "[Shark] < %p > found region in system ptes < %p - %08x >\n",
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
                                    "[Shark] < %p > found noimage return address in worker thread stack\n",
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
                                                    "[Shark] < %p > insert worker thread check code\n",
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
    }
}

void
NTAPI
PgClearAll(
    __inout PPGBLOCK PgBlock
)
{
    PgClearPoolEncryptedContext(PgBlock);

    if (GetGpBlock(PgBlock)->BuildNumber >= 9200) {
        PgClearSystemPtesEncryptedContext(PgBlock);
    }

    if (NULL != PgBlock->INITKDBG) {
        PgCheckAllWorkerThread(PgBlock);

        __free(PgBlock->INITKDBG);

        PgBlock->INITKDBG = NULL;
    }
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
                            "[Shark] < %p > OffsetSameThreadPassive\n",
                            Context->PgBlock->OffsetSameThreadPassive);
#endif // DEBUG

                        break;
                    }
                }

                TargetPc += Length;
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
        NULL == Context->PgBlock->Pool.PoolBigPageTableSize ||
        NULL == Context->PgBlock->ExpWorkerThread) {
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
        if (0 == Context->PgBlock->WorkerContext ||
            NULL == Context->PgBlock->KiWaitNever ||
            NULL == Context->PgBlock->KiWaitAlways) {
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
