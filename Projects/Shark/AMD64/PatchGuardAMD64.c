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
#include "Except.h"
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
    PPGBLOCK Block = NULL;

    GetCounterBody(
        &Object->Body.Reserved,
        &Block);

    if (PgPoolBigPage == Object->Type) {
        GetRtBlock(Block)->ExFreePoolWithTag(
            Object->BaseAddress,
            0);
    }
    else if (PgSystemPtes == Object->Type) {
        Block->MmFreeIndependentPages(
            Object->BaseAddress,
            Object->RegionSize);
    }
    else if (PgMaximumType == Object->Type) {
        NOTHING;
    }

    GetRtBlock(Block)->ExFreePoolWithTag(Object, 0);
}

void
NTAPI
PgClearCallback(
    __in PCONTEXT Context,
    __in_opt ptr ProgramCounter,
    __in_opt PPGOBJECT Object,
    __in_opt PPGBLOCK Block
)
{
    PETHREAD Thread = NULL;
    PKSTART_FRAME StartFrame = NULL;
    PKSWITCH_FRAME SwitchFrame = NULL;
    PKTRAP_FRAME TrapFrame = NULL;

    if (NULL != Object) {
        GetCounterBody(
            &Object->Body.Reserved,
            &Block);

#ifdef DEBUG
        GetRtBlock(Block)->vDbgPrint(
            Block->ClearMessage[Object->Encrypted],
            Object);
#endif // DEBUG

        GetRtBlock(Block)->ExInterlockedRemoveHeadList(
            Object->Entry.Blink,
            &Block->Lock);

        if (PgDoubleEncrypted == Object->Encrypted) {
            Context->Rip = __rduptr(Context->Rsp);
            Context->Rsp += sizeof(ptr);

            ExInitializeWorkItem(
                &Object->Worker, Block->FreeWorker, Object);

            Block->ExQueueWorkItem(
                &Object->Worker, CriticalWorkQueue);
        }
        else if (PgEncrypted == Object->Encrypted) {
            Context->Rip = __rduptr(Context->Rsp + KSTART_FRAME_LENGTH);
            Context->Rsp += KSTART_FRAME_LENGTH + sizeof(ptr);

            ExInitializeWorkItem(
                &Object->Worker, Block->FreeWorker, Object);

            Block->ExQueueWorkItem(
                &Object->Worker, CriticalWorkQueue);
        }
        else {
            Thread = (PETHREAD)__readgsqword(FIELD_OFFSET(KPCR, Prcb.CurrentThread));

            if (GetRtBlock(Block)->BuildNumber >= 18362) {
                // ETHREAD->ReservedCrossThreadFlags
                // clear SameThreadPassiveFlags Bit 0 (BugCheck 139)

                *((u8 *)Thread + Block->OffsetSameThreadPassive) &= 0xFFFFFFFE;
            }

            StartFrame =
                (PKSTART_FRAME)(
                (*(u64ptr)((u64)Thread +
                    GetRtBlock(Block)->DebuggerDataBlock.OffsetKThreadInitialStack)) -
                    KSTART_FRAME_LENGTH);

            StartFrame->P1Home = (u64)Block->WorkerContext;
            StartFrame->P2Home = (u64)Block->ExpWorkerThread;
            StartFrame->P3Home = (u64)Block->PspSystemThreadStartup;
            StartFrame->Return = 0;

            SwitchFrame = (PKSWITCH_FRAME)((u64)StartFrame - KSWITCH_FRAME_LENGTH);

            SwitchFrame->Return = (u64)Block->KiStartSystemThread;
            SwitchFrame->ApcBypass = APC_LEVEL;
            SwitchFrame->Rbp = (u64)TrapFrame + FIELD_OFFSET(KTRAP_FRAME, Xmm1);

            Context->Rsp = (u64)StartFrame;
            Context->Rip = (u64)Block->KiStartSystemThread;

            ExInitializeWorkItem(
                &Object->Worker, Block->FreeWorker, Object);

            Block->ExQueueWorkItem(
                &Object->Worker, CriticalWorkQueue);
        }

        GetRtBlock(Block)->RtlRestoreContext(Context, NULL);
    }
    else {
        __debugbreak();
    }
}

void
NTAPI
InitializePgBlock(
    __inout PPGBLOCK Block
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

    ptr Address = NULL;
    PMMPTE PointerPte = NULL;

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
        "[SHARK] < %p > declassified context cleared\n",
        "[SHARK] < %p > encrypted context cleared\n",
        "[SHARK] < %p > double encrypted context cleared\n"
    };

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > Block\n",
        Block);
#endif // DEBUG

    InitializeListHead(&Block->Object);
    KeInitializeSpinLock(&Block->Lock);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &GetRtBlock(Block)->KernelDataTableEntry->FullDllName,
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
                    (u64)GetRtBlock(Block)->DebuggerDataBlock.KernBase -
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

                        if (0 == Block->SizeCmpAppendDllSection) {
                            if (8 == Length) {
                                if (0 == _cmpbyte(TargetPc[0], 0x48) &&
                                    0 == _cmpbyte(TargetPc[1], 0x31) &&
                                    0 == _cmpbyte(TargetPc[2], 0x84) &&
                                    0 == _cmpbyte(TargetPc[3], 0xca)) {
                                    Block->SizeCmpAppendDllSection = *(u32ptr)(TargetPc + 4);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > SizeCmpAppendDllSection\n",
                                        Block->SizeCmpAppendDllSection);
#endif // DEBUG

                                    if (0 == _cmpbyte(TargetPc[11], 0x48) ||
                                        0 == _cmpbyte(TargetPc[12], 0x0f) ||
                                        0 == _cmpbyte(TargetPc[13], 0xbb) ||
                                        0 == _cmpbyte(TargetPc[14], 0xc0)) {
                                        Block->BtcEnable = TRUE;

#ifdef DEBUG
                                        vDbgPrint(
                                            "[SHARK] < %p > BtcEnable\n",
                                            Block->BtcEnable);
#endif // DEBUG
                                    }

                                    while (0 != _cmpqword(
                                        0x085131481131482E,
                                        *(uptr)TargetPc)) {
                                        TargetPc--;
                                    }

                                    RtlCopyMemory(
                                        Block->_OriginalCmpAppendDllSection,
                                        TargetPc,
                                        sizeof(Block->_OriginalCmpAppendDllSection));

                                    Block->OriginalCmpAppendDllSection =
                                        (ptr)Block->_OriginalCmpAppendDllSection;

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > OriginalCmpAppendDllSection\n",
                                        Block->OriginalCmpAppendDllSection);
#endif // DEBUG
                                }
                            }
                        }

                        if (6 == Length) {
                            if (0 == _cmpbyte(TargetPc[0], 0x8b) &&
                                0 == _cmpbyte(TargetPc[1], 0x82)) {
                                Block->OffsetEntryPoint = *(u32ptr)(TargetPc + 2);

#ifdef DEBUG
                                vDbgPrint(
                                    "[SHARK] < %p > OffsetEntryPoint\n",
                                    Block->OffsetEntryPoint);
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
                    Header[GetRtBlock(Block)->BuildNumber >= 18362 ? 1 : 0]);

                if (NULL != ControlPc) {
                    TargetPc = ControlPc + Diff;

                    FunctionEntry = RtlLookupFunctionEntry(
                        (u64)TargetPc,
                        (u64ptr)&ImageBase,
                        NULL);

                    if (NULL != FunctionEntry) {
                        TargetPc =
                            (u8ptr)GetRtBlock(Block)->DebuggerDataBlock.KernBase +
                            FunctionEntry->BeginAddress - Diff;

                        RtlCopyMemory(
                            Block->Header,
                            TargetPc,
                            sizeof(Block->Header));
                    }

                    NtSection = SectionTableFromVirtualAddress(
                        ViewBase,
                        ControlPc);

                    if (NULL != NtSection) {
                        Block->SizeINITKDBG =
                            max(NtSection->SizeOfRawData, NtSection->Misc.VirtualSize);

#ifdef DEBUG
                        vDbgPrint(
                            "[SHARK] < %p > SizeINITKDBG\n",
                            Block->SizeINITKDBG);
#endif // DEBUG

                        Block->INITKDBG = __malloc(Block->SizeINITKDBG);

                        if (NULL != Block->INITKDBG) {
                            RtlCopyMemory(
                                Block->INITKDBG,
                                (u8ptr)ViewBase + NtSection->VirtualAddress,
                                Block->SizeINITKDBG);
#ifdef DEBUG
                            vDbgPrint(
                                "[SHARK] < %p > INITKDBG\n",
                                Block->INITKDBG);
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
                            ControlPc + Block->OffsetEntryPoint,
                            FirstField);

                        if (NULL != TargetPc) {
                            Block->Fields[0] =
                                (u64)__rva_to_va(TargetPc - 4) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[SHARK]",
                                (ptr)Block->Fields[0]);
#endif // DEBUG

                            Block->Fields[1] =
                                (u64)__rva_to_va(TargetPc + 10) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[SHARK]",
                                (ptr)Block->Fields[1]);
#endif // DEBUG

                            Block->Fields[2] =
                                (u64)__rva_to_va(TargetPc + 24) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[SHARK]",
                                (ptr)Block->Fields[2]);
#endif // DEBUG

                            Block->Fields[3] =
                                (u64)__rva_to_va(TargetPc + 38) + Diff;

#ifdef DEBUG
                            FindAndPrintSymbol(
                                "[SHARK]",
                                (ptr)Block->Fields[3]);
#endif // DEBUG

                            if (GetRtBlock(Block)->BuildNumber >= 9200) {
                                while (TRUE) {
                                    TargetPc = ScanBytes(
                                        TargetPc,
                                        (u8ptr)ViewBase + ViewSize,
                                        NextField);

                                    TempField = (u64)__rva_to_va(TargetPc + 3) + Diff;

                                    if ((u)TempField ==
                                        (u)GetRtBlock(Block)->DbgPrint) {
                                        TempField = (u64)__rva_to_va(TargetPc + 17) + Diff;

                                        RtlCopyMemory(
                                            &Block->MmAllocateIndependentPages,
                                            &TempField,
                                            sizeof(ptr));

#ifdef DEBUG
                                        vDbgPrint(
                                            "[SHARK] < %p > MmAllocateIndependentPages\n",
                                            Block->MmAllocateIndependentPages);
#endif // DEBUG                                         

                                        TempField = (u64)__rva_to_va(TargetPc + 31) + Diff;

                                        RtlCopyMemory(
                                            &Block->MmFreeIndependentPages,
                                            &TempField,
                                            sizeof(ptr));

#ifdef DEBUG
                                        vDbgPrint(
                                            "[SHARK] < %p > MmFreeIndependentPages\n",
                                            Block->MmFreeIndependentPages);
#endif // DEBUG

                                        TempField = (u64)__rva_to_va(TargetPc + 45) + Diff;

                                        RtlCopyMemory(
                                            &Block->MmSetPageProtection,
                                            &TempField,
                                            sizeof(ptr));

#ifdef DEBUG
                                        vDbgPrint(
                                            "[SHARK] < %p > MmSetPageProtection\n",
                                            Block->MmSetPageProtection);
#endif // DEBUG                                

                                        if (NULL != Block->MmAllocateIndependentPages &&
                                            NULL != Block->MmFreeIndependentPages) {
                                            Address =
                                                Block->MmAllocateIndependentPages(PAGE_SIZE, 0);

                                            if (NULL != Address) {
                                                PointerPte = GetPteAddress(Address);

#ifdef DEBUG
                                                vDbgPrint(
                                                    "[SHARK] < %p > test independent page < %p - %08x >\n",
                                                    PointerPte,
                                                    Address,
                                                    PAGE_SIZE);
#endif // DEBUG

                                                PointerPte = NULL;

                                                Block->MmFreeIndependentPages(Address, PAGE_SIZE);

                                                Address = NULL;
                                            }
                                        }
                                    }

                                    if ((u)TempField ==
                                        (u)GetRtBlock(Block)->KeWaitForSingleObject) {
                                        TempField = (u64)__rva_to_va(TargetPc + 59) + Diff;

                                        FunctionEntry = RtlLookupFunctionEntry(
                                            (u64)TempField,
                                            (u64ptr)&ImageBase,
                                            NULL);

                                        if (NULL != FunctionEntry) {
                                            Block->KiScbQueueScanWorker.BeginAddress =
                                                ImageBase + FunctionEntry->BeginAddress;

#ifdef DEBUG
                                            vDbgPrint(
                                                "[SHARK] < %p > KiScbQueueScanWorker\n",
                                                Block->KiScbQueueScanWorker.BeginAddress);
#endif // DEBUG

                                            Block->KiScbQueueScanWorker.EndAddress =
                                                ImageBase + FunctionEntry->EndAddress;

#ifdef DEBUG
                                            vDbgPrint(
                                                "[SHARK] < %p > KiScbQueueScanWorker end\n",
                                                Block->KiScbQueueScanWorker.EndAddress);
#endif // DEBUG
                                        }

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
                                    (u)GetRtBlock(Block)->PsLoadedModuleList) {
                                    TempField = (u64)__rva_to_va(TargetPc - 11) + Diff;

                                    RtlCopyMemory(
                                        &GetRtBlock(Block)->PsInvertedFunctionTable,
                                        &TempField,
                                        sizeof(ptr));

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > PsInvertedFunctionTable\n",
                                        GetRtBlock(Block)->PsInvertedFunctionTable);
#endif // DEBUG

                                    break;
                                }

                                TargetPc++;
                            }

                            if (GetRtBlock(Block)->BuildNumber >= 18362) {
                                TargetPc = ScanBytes(
                                    TargetPc,
                                    (u8ptr)ViewBase + ViewSize,
                                    BranchKey[0]);

                                if (NULL != TargetPc) {
                                    Block->BranchKey[10] = __rds32(TargetPc + 0x15);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[10]\n",
                                        Block->BranchKey[10]);
#endif // DEBUG
                                }

                                TargetPc = ScanBytes(
                                    TargetPc,
                                    (u8ptr)ViewBase + ViewSize,
                                    BranchKey[1]);

                                if (NULL != TargetPc) {
                                    ControlPc = __rva_to_va(TargetPc + 0x10);

                                    Block->BranchKey[0] =
                                        __rds32((u)(__rva_to_va(ControlPc + 0xA)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[0]\n",
                                        Block->BranchKey[0]);
#endif // DEBUG

                                    Block->BranchKey[1] =
                                        __rds32((u)(__rva_to_va(ControlPc + 0x13)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[1]\n",
                                        Block->BranchKey[1]);
#endif // DEBUG

                                    Block->BranchKey[2] =
                                        __rds32((u)(__rva_to_va(ControlPc + 0x1C)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[2]\n",
                                        Block->BranchKey[2]);
#endif // DEBUG

                                    Block->BranchKey[3] =
                                        __rds32((u)(__rva_to_va(ControlPc + 0x25)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[3]\n",
                                        Block->BranchKey[3]);
#endif // DEBUG

                                    Block->BranchKey[4] = __rds32(ControlPc + 0x31);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[4]\n",
                                        Block->BranchKey[4]);
#endif // DEBUG

                                    Block->BranchKey[5] =
                                        __rds32((u)(__rva_to_va(ControlPc + 2)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[5]\n",
                                        Block->BranchKey[5]);
#endif // DEBUG

                                    ControlPc = TargetPc + 0x1B;

                                    // Block->BranchKey[6] = Block->BranchKey[5];

                                    Block->BranchKey[6] = 0; // same with 5, here use 0.

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[6]\n",
                                        Block->BranchKey[6]);
#endif // DEBUG

                                    Block->BranchKey[7] =
                                        __rds32((u)(__rva_to_va(ControlPc + 0xE)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[7]\n",
                                        Block->BranchKey[7]);
#endif // DEBUG

                                    Block->BranchKey[8] =
                                        __rds32((u)(__rva_to_va(ControlPc + 0x17)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[8]\n",
                                        Block->BranchKey[8]);
#endif // DEBUG

                                    Block->BranchKey[9] =
                                        __rds32((u)(__rva_to_va(ControlPc + 0x20)) + 8);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[9]\n",
                                        Block->BranchKey[9]);
#endif // DEBUG
                                }

                                TargetPc = ScanBytes(
                                    TargetPc,
                                    (u8ptr)ViewBase + ViewSize,
                                    BranchKey[2]);

                                if (NULL != TargetPc) {
                                    Block->BranchKey[11] = __rds32(TargetPc + 0x4A);

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > BranchKey[11]\n",
                                        Block->BranchKey[11]);
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

                    Block->KiStartSystemThread = (ptr)(TargetPc + Diff);

#ifdef DEBUG
                    vDbgPrint(
                        "[SHARK] < %p > KiStartSystemThread\n",
                        Block->KiStartSystemThread);
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
                        Block->PspSystemThreadStartup =
                            (ptr)((u8ptr)GetRtBlock(Block)->DebuggerDataBlock.KernBase +
                                FunctionEntry->BeginAddress);

#ifdef DEBUG
                        vDbgPrint(
                            "[SHARK] < %p > PspSystemThreadStartup\n",
                            Block->PspSystemThreadStartup);
#endif // DEBUG
                    }
                }

                ZwUnmapViewOfSection(ZwCurrentProcess(), ViewBase);
            }

            ZwClose(SectionHandle);
        }

        ZwClose(FileHandle);
    }

    if (GetRtBlock(Block)->BuildNumber >= 9600) {
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

                            if (NULL == Block->KiWaitNever) {
                                Block->KiWaitNever = ControlPc;

#ifdef DEBUG
                                vDbgPrint(
                                    "[SHARK] < %p > KiWaitNever\n",
                                    Block->KiWaitNever);
#endif // DEBUG
                            }
                            else {
                                Block->KiWaitAlways = ControlPc;

#ifdef DEBUG
                                vDbgPrint(
                                    "[SHARK] < %p > KiWaitAlways\n",
                                    Block->KiWaitAlways);
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

    Block->Pool.MmIsNonPagedSystemAddressValid = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > MmIsNonPagedSystemAddressValid\n",
        Block->Pool.MmIsNonPagedSystemAddressValid);
#endif // DEBUG

    NtSection = LdrFindSection(
        (ptr)GetRtBlock(Block)->DebuggerDataBlock.KernBase,
        ".text");

    if (NULL != NtSection) {
        ControlPc =
            (u8ptr)GetRtBlock(Block)->DebuggerDataBlock.KernBase +
            NtSection->VirtualAddress;

        EndToLock =
            (u8ptr)GetRtBlock(Block)->DebuggerDataBlock.KernBase +
            NtSection->VirtualAddress +
            max(NtSection->SizeOfRawData, NtSection->Misc.VirtualSize);

        if (GetRtBlock(Block)->BuildNumber >= 7600 &&
            GetRtBlock(Block)->BuildNumber < 9200) {
            Selector = 0;
        }
        else if (GetRtBlock(Block)->BuildNumber >= 9200 &&
            GetRtBlock(Block)->BuildNumber < 19041) {
            Selector = 1;
        }
        else if (GetRtBlock(Block)->BuildNumber >= 19041 &&
            GetRtBlock(Block)->BuildNumber < 20000) {
            Selector = 2;
        }
        else if (GetRtBlock(Block)->BuildNumber >= 20000 &&
            GetRtBlock(Block)->BuildNumber < 21000) {
            Selector = 3;
        }
        else if (GetRtBlock(Block)->BuildNumber >= 21000 &&
            GetRtBlock(Block)->BuildNumber < 22000) {
            Selector = 4;
        }
        else if (GetRtBlock(Block)->BuildNumber >= 22000) {
            Selector = 3;
        }

        ControlPc = ScanBytes(
            ControlPc,
            EndToLock,
            ExGetBigPoolInfo[Selector].Signature);

        if (NULL != ControlPc) {
            Block->Pool.PoolBigPageTable =
                __rva_to_va_ex(
                    ControlPc + ExGetBigPoolInfo[Selector].Offset[0],
                    ExGetBigPoolInfo[Selector].Offset[1]);

#ifdef DEBUG
            vDbgPrint(
                "[SHARK] < %p > PoolBigPageTable\n",
                Block->Pool.PoolBigPageTable);
#endif // DEBUG

            Block->Pool.PoolBigPageTableSize =
                __rva_to_va_ex(
                    ControlPc + ExGetBigPoolInfo[Selector].Offset[2],
                    +ExGetBigPoolInfo[Selector].Offset[3]);

#ifdef DEBUG
            vDbgPrint(
                "[SHARK] < %p > PoolBigPageTableSize\n",
                Block->Pool.PoolBigPageTableSize);
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

                    Block->SystemPtes.NumberOfPtes = BitMap->SizeOfBitMap * 8;
#ifdef DEBUG
                    vDbgPrint(
                        "[SHARK] < %p > NumberOfPtes\n",
                        Block->SystemPtes.NumberOfPtes);
#endif // DEBUG

                    if (GetRtBlock(Block)->BuildNumber < 9600) {
                        Block->SystemPtes.BasePte =
                            *(PMMPTE *)((u8ptr)(BitMap + 1) + sizeof(u32) * 2);
                    }
                    else {
                        Block->SystemPtes.BasePte = *(PMMPTE *)(BitMap + 1);
                    }

#ifdef DEBUG
                    vDbgPrint(
                        "[SHARK] < %p > BasePte\n",
                        Block->SystemPtes.BasePte);
#endif // DEBUG

                    break;
                }
            }

            ControlPc += Length;
        }
    }

    RtlCopyMemory(
        Block->_Ror64,
        Ror64,
        sizeof(Block->_Ror64));

    Block->Ror64 = (ptr)Block->_Ror64;

    RtlCopyMemory(
        Block->_Rol64,
        Rol64,
        sizeof(Block->_Rol64));

    Block->Rol64 = (ptr)Block->_Rol64;

    RtlCopyMemory(
        Block->_RorWithBtc64,
        RorWithBtc64,
        sizeof(Block->_RorWithBtc64));

    Block->RorWithBtc64 = (ptr)Block->_RorWithBtc64;

    Block->CmpDecode =
        Block->BtcEnable ?
        (ptr)Block->RorWithBtc64 : (ptr)Block->Ror64;

    if (GetRtBlock(Block)->BuildNumber > 20000) {
        Block->BuildKey = 0x00000009UL;
    }
    else if (GetRtBlock(Block)->BuildNumber == 18362) {
        Block->BuildKey = 0xFFFFFFE6UL;
    }

    Block->CacheCmpAppendDllSection = Block->_CacheCmpAppendDllSection;

    RtlCopyMemory(
        Block->_PostCache,
        PostCache,
        sizeof(Block->_PostCache));

    Block->PostCache =
        GetRtBlock(Block)->BuildNumber > 18362 ?
        (ptr)Block->_PostCache : (ptr)(Block->_PostCache + 8);

    RtlCopyMemory(
        Block->_PostKey,
        PostKey,
        sizeof(Block->_PostKey));

    Block->PostKey =
        GetRtBlock(Block)->BuildNumber >= 18362 ?
        (ptr)Block->_PostKey : (ptr)(Block->_PostKey + 0x20);

    RtlInitUnicodeString(&RoutineString, L"MmIsAddressValid");

    Block->MmIsAddressValid = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > MmIsAddressValid\n",
        Block->MmIsAddressValid);
#endif // DEBUG

    RtlInitUnicodeString(&RoutineString, L"RtlLookupFunctionEntry");

    Block->RtlLookupFunctionEntry = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > RtlLookupFunctionEntry\n",
        Block->RtlLookupFunctionEntry);
#endif // DEBUG

    RtlInitUnicodeString(&RoutineString, L"RtlVirtualUnwind");

    Block->RtlVirtualUnwind = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > RtlVirtualUnwind\n",
        Block->RtlVirtualUnwind);
#endif // DEBUG

    RtlInitUnicodeString(&RoutineString, L"ExQueueWorkItem");

    Block->ExQueueWorkItem = MmGetSystemRoutineAddress(&RoutineString);

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > ExQueueWorkItem\n",
        Block->ExQueueWorkItem);
#endif // DEBUG

    if (1 != Block->IsDebug) {
        RtlCopyMemory(
            &Block->_CaptureContext[0],
            _CaptureContext,
            RTL_NUMBER_OF(Block->_CaptureContext));

        Block->CaptureContext = (ptr)(u)&Block->_CaptureContext[0];
    }
    else {
        Block->CaptureContext = _CaptureContext;
    }

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > CaptureContext\n",
        Block->CaptureContext);
#endif // DEBUG

    if (1 != Block->IsDebug) {
        RtlCopyMemory(
            &Block->_ClearMessage[0],
            ClearMessage[0],
            strlen(ClearMessage[0]));

        Block->ClearMessage[0] = &Block->_ClearMessage[0];

        RtlCopyMemory(
            &Block->_ClearMessage[0x40],
            ClearMessage[1],
            strlen(ClearMessage[1]));

        Block->ClearMessage[1] = &Block->_ClearMessage[0x40];

        RtlCopyMemory(
            &Block->_ClearMessage[0x80],
            ClearMessage[2],
            strlen(ClearMessage[2]));

        Block->ClearMessage[2] = &Block->_ClearMessage[0x80];
    }
    else {
        Block->ClearMessage[0] = ClearMessage[0];
        Block->ClearMessage[1] = ClearMessage[1];
        Block->ClearMessage[2] = ClearMessage[2];
    }

    if (1 != Block->IsDebug) {
        RtlCopyMemory(
            &Block->_FreeWorker[0],
            PgFreeWorker,
            RTL_NUMBER_OF(Block->_FreeWorker));

        Block->FreeWorker = (ptr)(u)&Block->_FreeWorker[0];
    }
    else {
        Block->FreeWorker = PgFreeWorker;
    }

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > FreeWorker\n",
        Block->FreeWorker);
#endif // DEBUG

    if (1 != Block->IsDebug) {
        RtlCopyMemory(
            &Block->_ClearCallback[0],
            PgClearCallback,
            RTL_NUMBER_OF(Block->_ClearCallback));

        Block->ClearCallback = (ptr)(u)&Block->_ClearCallback[0];
    }
    else {
        Block->ClearCallback = PgClearCallback;
    }

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > ClearCallback\n",
        Block->ClearCallback);
#endif // DEBUG
}

PPGOBJECT
NTAPI
PgCreateObject(
    __in_opt u8 Encrypted,
    __in u8 Type,
    __in ptr BaseAddress,
    __in_opt u RegionSize,
    __in PPGBLOCK Block,
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
            Block,
            Object,
            Callback,
            Return,
            ProgramCounter,
            CaptureContext);

        ExInterlockedInsertTailList(
            &Block->Object, &Object->Entry, &Block->Lock);
    }

    return Object;
}

void
NTAPI
PgSetNewEntry(
    __inout PPGBLOCK Block,
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

    FieldIndex = (Block->OffsetEntryPoint -
        Block->SizeCmpAppendDllSection) / sizeof(u64);

    RtlCopyMemory(
        FieldBuffer,
        (u8ptr)PatchGuardContext + (Block->OffsetEntryPoint & ~7),
        sizeof(FieldBuffer));

    LastRorKey = RorKey;

    for (Index = 0;
        Index < FieldIndex;
        Index++) {
        LastRorKey = Block->Rol64(LastRorKey, Index);
    }

    for (Index = 0;
        Index < RTL_NUMBER_OF(FieldBuffer);
        Index++) {
        LastRorKey = Block->Rol64(LastRorKey, FieldIndex + Index);
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
    }

    RvaOfEntry = *(u32ptr)((u8ptr)FieldBuffer + (Block->OffsetEntryPoint & 7));

    // copy PatchGuard entry head code to temp bufer and decode

    FieldIndex = (RvaOfEntry - Block->SizeCmpAppendDllSection) / sizeof(u64);

    RtlCopyMemory(
        FieldBuffer,
        (u8ptr)PatchGuardContext + (RvaOfEntry & ~7),
        sizeof(FieldBuffer));

    LastRorKey = RorKey;

    for (Index = 0;
        Index < FieldIndex;
        Index++) {
        LastRorKey = Block->Rol64(LastRorKey, Index);
    }

    for (Index = 0;
        Index < RTL_NUMBER_OF(FieldBuffer);
        Index++) {
        LastRorKey = Block->Rol64(LastRorKey, FieldIndex + Index);
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
    }

    // set temp buffer PatchGuard entry head jmp to PgClearCallback and encrypt

    Pointer = (u8ptr)FieldBuffer + (RvaOfEntry & 7);

    LockedBuildJumpCode(&Pointer, &Object->Body);

    while (Index--) {
        FieldBuffer[Index] = FieldBuffer[Index] ^ LastRorKey;
        LastRorKey = Block->Ror64(LastRorKey, FieldIndex + Index);
    }

    // copy temp buffer PatchGuard entry head to old address, 
    // when PatchGuard code decrypt self jmp PgClearCallback.

    RtlCopyMemory(
        (u8ptr)PatchGuardContext + (RvaOfEntry & ~7),
        FieldBuffer,
        sizeof(FieldBuffer));

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > set new entry for encrypted context\n",
        Object);
#endif // DEBUG
}

u
NTAPI
PgScanEntryWithBtc(
    __inout PPGBLOCK Block,
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
    CompareCount = (Object->ContextSize - Block->SizeCmpAppendDllSection) / 8 - 1;
    TargetPc = (ptr)((u)Object->Context + Block->SizeCmpAppendDllSection);

    do {
        ControlPc = (ptr)(Block->Header + Align);

        for (Index = 0;
            Index < CompareCount;
            Index++) {
            RorKey = TargetPc[Index + 1] ^ ControlPc[1];

            *LastKey = RorKey;

            RorKey = Block->CmpDecode(RorKey, Index + 1);

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
    __inout PPGBLOCK Block,
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
        Block,
        Object,
        &AlignOffset,
        &LastRorKey);

    if (0 == Index) {
#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > entrypoint not found!\n",
            Object);
#endif // DEBUG
    }
    else {
        FieldIndex = Index - (0 == (AlignOffset & 7) ? 0 : 1);
        LastRorKeyOffset = 2 + (0 == (AlignOffset & 7) ? 0 : 1);

        RtlCopyMemory(
            FieldBuffer,
            (u8ptr)Object->Context + Block->SizeCmpAppendDllSection + FieldIndex * sizeof(u),
            sizeof(FieldBuffer));

        RorKey = LastRorKey;
        Index = LastRorKeyOffset;

        while (Index--) {
            FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
            RorKey = Block->CmpDecode(RorKey, FieldIndex + Index);
        }

        // set temp buffer PatchGuard entry head jmp to PgClearCallback and encrypt

        Pointer = (PGUARD_BODY)((u8ptr)FieldBuffer + sizeof(u) - AlignOffset);

        LockedBuildJumpCode(&Pointer, &Object->Body);

        RorKey = LastRorKey;
        Index = LastRorKeyOffset;

        while (Index--) {
            FieldBuffer[Index] = FieldBuffer[Index] ^ RorKey;
            RorKey = Block->CmpDecode(RorKey, FieldIndex + Index);
        }

        RtlCopyMemory(
            (u8ptr)Object->Context + Block->SizeCmpAppendDllSection + FieldIndex * sizeof(u),
            FieldBuffer,
            sizeof(FieldBuffer));

#ifdef DEBUG
        vDbgPrint(
            "[SHARK] < %p > set new entry for btc encrypted context\n",
            Object);
#endif // DEBUG
    }
}

b
NTAPI
PgFastScanTimer(
    __inout PPGBLOCK Block,
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

    ControlKey = (u)Context;
    ControlKey = Block->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        ControlPc = (u)Context + Index * 8;
        Cache = Original = __rdu64(ControlPc);
        Cache = Original ^ ControlKey;
        Cache += Block->PostCache(Index, Context);

        ControlKey ^= Block->PostKey(Index, Original);

        ControlKey = Block->Rol64(
            ControlKey,
            (0 == Block->BuildKey ? ~Original : Original ^ Block->BuildKey) & 0x3F);

        ControlKey += (u)Context;

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Cache = Block->Rol64(Cache, 4);

            Key = KeyTable[Cache & 0xF];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;
        }

        __wruptr(Block->CacheCmpAppendDllSection + Index, Cache);

        Index++;
    } while (Index < Count);

    XorKey =
        Block->OriginalCmpAppendDllSection[1] ^ Block->CacheCmpAppendDllSection[1];

    if ((Block->OriginalCmpAppendDllSection[2] ^ Block->CacheCmpAppendDllSection[2]) == XorKey) {
        Result = TRUE;
    }

    return Result;
}

u
NTAPI
PgFastScanBranch(
    __inout PPGBLOCK Block,
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

    ControlKey = (u)Context;
    ControlKey = Block->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        ControlPc = (u)Context + Index * 8;
        Cache = Original = __rdu64(ControlPc);

        Cache ^= *Block->KiWaitNever;

        Cache =
            Block->Rol64(Cache, *Block->KiWaitNever) ^ ControlKey;

        Cache =
            _byteswap_uint64(Cache);

        Cache ^= *Block->KiWaitAlways;
        Cache += Block->PostCache(Index, Context);

        ControlKey ^= Block->PostKey(Index, Original);

        ControlKey = Block->Rol64(
            ControlKey,
            (0 == Block->BuildKey ? ~Original : Original ^ Block->BuildKey) & 0x3F);

        ControlKey += (u)Context;
        ControlKey ^= BranchKey;

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Cache = Block->Rol64(Cache, 4);

            Key = KeyTable[Cache & 0xF];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;
        }

        __wruptr(Block->CacheCmpAppendDllSection + Index, Cache);

        Index++;
    } while (Index < Count);

    XorKey =
        Block->OriginalCmpAppendDllSection[1] ^ Block->CacheCmpAppendDllSection[1];

    if ((Block->OriginalCmpAppendDllSection[2] ^ Block->CacheCmpAppendDllSection[2]) == XorKey) {
        Result = TRUE;
    }

    return Result;
}

void
NTAPI
PgSetTimerNewEntry(
    __inout PPGBLOCK Block,
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

    ControlKey = (u)(Object->Context);
    ControlKey = Block->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        Cache = Original = __rdu64((u)(Object->Context) + Index * 8);
        Cache = Original ^ ControlKey;
        Cache += Block->PostCache(Index, Object->Context);

        ControlKey ^= Block->PostKey(Index, Original);

        ControlKey = Block->Rol64(
            ControlKey,
            (0 == Block->BuildKey ? ~Original : Original ^ Block->BuildKey) & 0x3F);

        ControlKey += (u)(Object->Context);

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Cache = Block->Rol64(Cache, 4);

            Key = KeyTable[Cache & 0xF];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;

            if (FALSE != Pass) break;
        }

        __wru64((u)(Object->Context) + Index * 8, Cache);

        Index++;

        if (Index == (0xC8 / 8)) {
            Object->Key =
                Block->OriginalCmpAppendDllSection[1] ^ Block->CacheCmpAppendDllSection[1];

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
        "[SHARK] < %p > set new entry for double encrypted context\n",
        Object);
#endif // DEBUG

    Index = 0;
    Pass = FALSE;
    ControlKey = (u)(Object->Context);
    ControlKey = Block->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        Cache = __rdu64((u)(Object->Context) + Index * 8);

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Key = KeyTable[0x10 + (Cache & 0xF)];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;

            Cache = Block->Ror64(Cache, 4);

            if (FALSE != Pass) break;
        }

        Cache -= Block->PostCache(Index, Object->Context);

        Cache ^= ControlKey;

        ControlKey ^= Block->PostKey(Index, Cache);

        ControlKey = Block->Rol64(
            ControlKey,
            (0 == Block->BuildKey ? ~Cache : Cache ^ Block->BuildKey) & 0x3F);

        ControlKey += (u)(Object->Context);

        __wru64((u)(Object->Context) + Index * 8, Cache);

        Index++;

        if (Index == (0xC8 / 8)) {
            Pass = TRUE;
        }
    } while (Index < Count);
}

void
NTAPI
PgSetBranchNewEntry(
    __inout PPGBLOCK Block,
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

    ControlKey = (u)(Object->Context);
    ControlKey = Block->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        Cache = Original = __rdu64((u)(Object->Context) + Index * 8);

        Cache ^= *Block->KiWaitNever;
        Cache = Block->Rol64(Cache, *Block->KiWaitNever) ^ ControlKey;
        Cache = _byteswap_uint64(Cache);
        Cache ^= *Block->KiWaitAlways;
        Cache += Block->PostCache(Index, Object->Context);

        ControlKey ^= Block->PostKey(Index, Original);

        ControlKey = Block->Rol64(
            ControlKey,
            (0 == Block->BuildKey ? ~Original : Original ^ Block->BuildKey) & 0x3F);

        ControlKey += (u)(Object->Context);
        ControlKey ^= BranchKey;

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Cache = Block->Rol64(Cache, 4);

            Key = KeyTable[Cache & 0xF];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;

            if (FALSE != Pass) break;
        }

        __wru64((u)(Object->Context) + Index * 8, Cache);

        Index++;

        if (Index == (0xC8 / 8)) {
            Object->Key =
                Block->OriginalCmpAppendDllSection[1] ^ Block->CacheCmpAppendDllSection[1];

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
        "[SHARK] < %p > set new entry for double encrypted context\n",
        Object);
#endif // DEBUG

    Index = 0;
    Pass = FALSE;
    ControlKey = (u)(Object->Context);
    ControlKey = Block->Ror64(ControlKey, ControlKey & 0x3F);

    do {
        Cache = __rdu64((u)(Object->Context) + Index * 8);

        for (KeyIndex = 0;
            KeyIndex < 0x10;
            KeyIndex++) {
            Key = KeyTable[0x10 + (Cache & 0xF)];

            Cache &= 0xFFFFFFFFFFFFFFF0ULL;
            Cache |= Key;

            Cache = Block->Ror64(Cache, 4);

            if (FALSE != Pass) break;
        }

        Cache -= Block->PostCache(Index, Object->Context);
        Cache ^= *Block->KiWaitAlways;
        Cache = _byteswap_uint64(Cache);
        Cache ^= ControlKey;
        Cache = Block->Ror64(Cache, *Block->KiWaitNever);
        Cache ^= *Block->KiWaitNever;

        ControlKey ^= Block->PostKey(Index, Cache);

        ControlKey = Block->Rol64(
            ControlKey,
            (0 == Block->BuildKey ? ~Cache : Cache ^ Block->BuildKey) & 0x3F);

        ControlKey += (u)(Object->Context);
        ControlKey ^= BranchKey;

        __wru64((u)(Object->Context) + Index * 8, Cache);

        Index++;

        if (Index == (0xC8 / 8)) {
            Pass = TRUE;
        }
    } while (Index < Count);
}

PPGOBJECT
NTAPI
PgCompareDoubleEncryptedFields(
    __inout PPGBLOCK Block,
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
        + PAGE_SIZE + Block->SizeCmpAppendDllSection;

    for (Index = 0;
        Index < RTL_NUMBER_OF(Block->BranchKey) && 0 == Result;
        Index++) {
        TargetPc = BaseAddress;

        do {
            Result = PgFastScanBranch(
                Block,
                TargetPc,
                Block->BranchKey[Index]);

            if (0 != Result) {
                Object = PgCreateObject(
                    PgDoubleEncrypted,
                    Type,
                    BaseAddress,
                    RegionSize,
                    Block,
                    NULL,
                    Block->ClearCallback,
                    NULL,
                    Block->CaptureContext);

                if (NULL != Object) {
                    Object->Context = TargetPc;

                    PgSetBranchNewEntry(
                        Block,
                        Object,
                        Block->BranchKey[Index]);
                }

                break;
            }
            else {
                Result = PgFastScanTimer(Block, TargetPc);

                if (0 != Result) {
                    Object = PgCreateObject(
                        PgDoubleEncrypted,
                        Type,
                        BaseAddress,
                        RegionSize,
                        Block,
                        NULL,
                        Block->ClearCallback,
                        NULL,
                        Block->CaptureContext);

                    if (NULL != Object) {
                        Object->Context = TargetPc;

                        PgSetTimerNewEntry(Block, Object);
                    }

                    break;
                }
            }
        } while ((u)TargetPc++ < (u)EndAddress);
    }

    return Object;
}

void
NTAPI
PgCompareFields(
    __inout PPGBLOCK Block,
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
        if (((u)Block >= (u)BaseAddress) &&
            ((u)Block < ((u)BaseAddress + RegionSize))) {
            // pass self
        }
        else {
            if (GetRtBlock(Block)->BuildNumber >= 18362) {
                Object = PgCompareDoubleEncryptedFields(
                    Block,
                    Type,
                    BaseAddress,
                    RegionSize);

                if (NULL != Object) {
#ifdef DEBUG
                    vDbgPrint(
                        "[SHARK] < %p > found encrypted context < %p - %08x >\n",
                        Object->Context,
                        BaseAddress,
                        RegionSize);
#endif // DEBUG
                }
            }
            else {
                TargetPc = BaseAddress;

                // only search one page
                EndAddress = (u8ptr)BaseAddress + PAGE_SIZE - sizeof(Block->Fields);

                do {
                    Fields = TargetPc;

                    if ((u64)Fields == (u64)Block->Fields) {
                        break;
                    }

                    RorKey = Fields[3] ^ Block->Fields[3];

                    // CmpAppendDllSection + 98

                    // xor [rdx + rcx * 8 + 0c0h], rax
                    // ror rax, cl

                    // if >= win10 17134 btc rax, rax here

                    // loop CmpAppendDllSection + 98

                    if (0 == RorKey) {
                        if (Fields[2] == Block->Fields[2] &&
                            Fields[1] == Block->Fields[1] &&
                            Fields[0] == Block->Fields[0]) {
                            Context = TargetPc - PG_FIRST_FIELD_OFFSET;

#ifdef DEBUG
                            vDbgPrint(
                                "[SHARK] < %p > found declassified context\n",
                                Context);
#endif // DEBUG
                            break;
                        }
                    }
                    else {
                        RorKeyIndex = 0;

                        RorKey = Block->CmpDecode(RorKey, PG_FIELD_BITS);

                        RorKeyIndex++;

                        if ((u64)(Fields[2] ^ RorKey) == (u64)Block->Fields[2]) {
                            RorKey = Block->CmpDecode(RorKey, PG_FIELD_BITS - RorKeyIndex);

                            RorKeyIndex++;

                            if ((u64)(Fields[1] ^ RorKey) == (u64)Block->Fields[1]) {
                                RorKey = Block->CmpDecode(RorKey, PG_FIELD_BITS - RorKeyIndex);

                                RorKeyIndex++;

                                if ((u64)(Fields[0] ^ RorKey) == (u64)Block->Fields[0]) {
                                    Context = TargetPc - PG_FIRST_FIELD_OFFSET;

                                    RorKey = Fields[0] ^ Block->Fields[0];

#ifdef DEBUG
                                    vDbgPrint(
                                        "[SHARK] < %p > found encrypted context < %p - %08x >\n",
                                        Context,
                                        BaseAddress,
                                        RegionSize);
#endif // DEBUG

                                    Object = PgCreateObject(
                                        PgEncrypted,
                                        Type,
                                        BaseAddress,
                                        RegionSize,
                                        Block,
                                        NULL,
                                        Block->ClearCallback,
                                        NULL,
                                        Block->CaptureContext);

                                    if (NULL != Object) {
                                        Block->Count++;

                                        Object->Context = Context;

                                        Object->ContextSize =
                                            RegionSize - ((u)Context - (u)BaseAddress);

                                        if (FALSE != Block->BtcEnable) {
                                            PgSetNewEntryWithBtc(Block, Object);
                                        }
                                        else {
                                            for (;
                                                RorKeyIndex < PG_FIELD_BITS;
                                                RorKeyIndex++) {
                                                RorKey = Block->Ror64(RorKey, PG_FIELD_BITS - RorKeyIndex);
                                            }

                                            Object->Key = RorKey;

#ifdef DEBUG
                                            vDbgPrint(
                                                "[SHARK] < %p > first rorkey\n",
                                                RorKey);
#endif // DEBUG
                                            PgSetNewEntry(Block, Object, Context, RorKey);
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
    __inout PPGBLOCK Block
)
{
    PRTL_BITMAP BitMap = NULL;
    u32 BitMapSize = 0;
    PFN_NUMBER NumberOfPtes = 0;
    u32 HintIndex = 0;
    u32 StartingRunIndex = 0;

    NumberOfPtes = Block->SystemPtes.NumberOfPtes;

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > SystemPtes < %p - %p >\n",
        KeGetCurrentProcessorNumber(),
        Block->SystemPtes.BasePte,
        Block->SystemPtes.BasePte + NumberOfPtes);
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
            Block->SystemPtes.BasePte,
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

                if (((StartingRunIndex - HintIndex)
                    >= BYTES_TO_PAGES(Block->SizeINITKDBG)) &&
                    (StartingRunIndex - HintIndex)
                    == BYTES_TO_PAGES(
                        __rduptr(GetVaMappedByPte(Block->SystemPtes.BasePte + HintIndex)))) {
#ifdef DEBUG
                    vDbgPrint(
                        "[SHARK] < %p > scan < %p - %08x > < %p, %p, %p, %p...>\n",
                        KeGetCurrentProcessorNumber(),
                        GetVaMappedByPte(Block->SystemPtes.BasePte + HintIndex),
                        (StartingRunIndex - HintIndex) * PAGE_SIZE,
                        __rduptr(GetVaMappedByPte(Block->SystemPtes.BasePte + HintIndex)),
                        __rduptr((u)GetVaMappedByPte(Block->SystemPtes.BasePte + HintIndex) + 8),
                        __rduptr((u)GetVaMappedByPte(Block->SystemPtes.BasePte + HintIndex) + 0x10),
                        __rduptr((u)GetVaMappedByPte(Block->SystemPtes.BasePte + HintIndex) + 0x18));
#endif // DEBUG

                    PgCompareFields(
                        Block,
                        PgSystemPtes,
                        GetVaMappedByPte(Block->SystemPtes.BasePte + HintIndex),
                        (StartingRunIndex - HintIndex) * PAGE_SIZE);
                }

                HintIndex = StartingRunIndex;
            }
        } while (HintIndex < NumberOfPtes);

        __free(BitMap);
    }
}

b
NTAPI
FastScanPool(
    __inout PPGBLOCK Block,
    __in PPOOL_BIG_PAGES PoolBigPage
)
{
    b Result = FALSE;
    u Index = 0;

    if (POOL_BIG_TABLE_ENTRY_FREE !=
        ((u64)PoolBigPage->Va & POOL_BIG_TABLE_ENTRY_FREE)) {
        if (0 == BYTE_OFFSET(PoolBigPage->NumberOfPages) &&
            PoolBigPage->NumberOfPages >= Block->SizeINITKDBG) {
            if (0 !=
                Block->Pool.MmIsNonPagedSystemAddressValid(PoolBigPage->Va)) {
                Result = TRUE;

                for (Index = 0;
                    Index < PG_COMPARE_FIELDS_COUNT;
                    Index++) {
                    if (0 == __rduptr((u)PoolBigPage->Va + Index * 8) ||
                        -1 == __rdsptr((u)PoolBigPage->Va + Index * 8) ||
                        -2 == __rdsptr((u)PoolBigPage->Va + Index * 8) ||
                        0 == ((__rduptr((u)PoolBigPage->Va + Index * 8) >> VIRTUAL_ADDRESS_BITS) & 0xffff) ||
                        0xffff == ((__rduptr((u)PoolBigPage->Va + Index * 8) >> VIRTUAL_ADDRESS_BITS) & 0xffff)) {
                        Result = FALSE;

                        break;
                    }
                }
            }
        }
    }

    return Result;
}

void
NTAPI
PgClearPoolEncryptedContext(
    __inout PPGBLOCK Block
)
{
    PPOOL_BIG_PAGES PoolBigPage = NULL;
    u Index = 0;
    u Offset = 0;

    Offset =
        GetRtBlock(Block)->BuildNumber > 20000 ?
        sizeof(POOL_BIG_PAGESEX) : sizeof(POOL_BIG_PAGES);

#ifdef DEBUG
    vDbgPrint(
        "[SHARK] < %p > BigPool < %p - %08x >\n",
        KeGetCurrentProcessorNumber(),
        *Block->Pool.PoolBigPageTable,
        *Block->Pool.PoolBigPageTableSize);
#endif // DEBUG

    for (Index = 0;
        Index < *Block->Pool.PoolBigPageTableSize;
        Index++) {
        PoolBigPage =
            (ptr)((u)*Block->Pool.PoolBigPageTable
                + Index * Offset);

        if (FALSE != FastScanPool(Block, PoolBigPage)) {
#ifdef DEBUG
            vDbgPrint(
                "[SHARK] < %p > scan < %p - %08x > < %p, %p, %p, %p...>\n",
                KeGetCurrentProcessorNumber(),
                PoolBigPage->Va,
                PoolBigPage->NumberOfPages,
                __rduptr((u)PoolBigPage->Va),
                __rduptr((u)PoolBigPage->Va + 8),
                __rduptr((u)PoolBigPage->Va + 0x10),
                __rduptr((u)PoolBigPage->Va + 0x18));
#endif // DEBUG

            PgCompareFields(
                Block,
                PgPoolBigPage,
                PoolBigPage->Va,
                PoolBigPage->NumberOfPages);
        }
    }
}

void
NTAPI
PgLocatePoolObject(
    __inout PPGBLOCK Block,
    __in ptr Establisher,
    __in PPGOBJECT Object
)
{
    PPOOL_BIG_PAGES PoolBigPage = NULL;
    u Index = 0;
    u Offset = 0;

    Offset =
        GetRtBlock(Block)->BuildNumber > 20000 ?
        sizeof(POOL_BIG_PAGESEX) : sizeof(POOL_BIG_PAGES);

    for (Index = 0;
        Index < *Block->Pool.PoolBigPageTableSize;
        Index++) {
        PoolBigPage =
            (ptr)((u)*Block->Pool.PoolBigPageTable
                + Index * Offset);

        if (POOL_BIG_TABLE_ENTRY_FREE !=
            ((u64)PoolBigPage->Va & POOL_BIG_TABLE_ENTRY_FREE)) {
            if (FALSE !=
                Block->Pool.MmIsNonPagedSystemAddressValid(
                    PoolBigPage->Va)) {
                if (PoolBigPage->NumberOfPages > Block->SizeINITKDBG) {
                    if ((u64)Establisher >= (u64)PoolBigPage->Va &&
                        (u64)Establisher < (u64)PoolBigPage->Va +
                        PoolBigPage->NumberOfPages) {
                        Object->BaseAddress = PoolBigPage->Va;
                        Object->RegionSize = PoolBigPage->NumberOfPages;

#ifdef DEBUG
                        GetRtBlock(Block)->vDbgPrint(
                            "[SHARK] < %p > found region in pool < %p - %08x >\n",
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
    __inout PPGBLOCK Block,
    __in ptr ProgramCounter,
    __in PPGOBJECT Object
)
{
    PRTL_BITMAP BitMap = NULL;
    u32 BitMapSize = 0;
    PFN_NUMBER NumberOfPtes = 0;
    u32 HintIndex = 0;
    u32 StartingRunIndex = 0;

    NumberOfPtes = Block->SystemPtes.NumberOfPtes;

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
            Block->SystemPtes.BasePte,
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

                if ((u64)ProgramCounter >=
                    (u64)GetVaMappedByPte(
                        Block->SystemPtes.BasePte + HintIndex) &&
                        (u64)ProgramCounter <
                    (u64)GetVaMappedByPte(
                        Block->SystemPtes.BasePte + StartingRunIndex) - Block->SizeCmpAppendDllSection) {
                    // align clear execute mask region

                    if (GetRtBlock(Block)->BuildNumber >= 18362) {
                        Object->BaseAddress =
                            GetVaMappedByPte(Block->SystemPtes.BasePte + HintIndex - 1);

                        Object->RegionSize =
                            (u)(StartingRunIndex - HintIndex + 1) * PAGE_SIZE;
                    }
                    else {
                        Object->BaseAddress =
                            GetVaMappedByPte(Block->SystemPtes.BasePte + HintIndex);

                        Object->RegionSize =
                            (u)(StartingRunIndex - HintIndex) * PAGE_SIZE;
                    }
#ifdef DEBUG
                    vDbgPrint(
                        "[SHARK] < %p > found region in system ptes < %p - %08x >\n",
                        ProgramCounter,
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
    __inout PPGBLOCK Block,
    __in ptr Establisher,
    __out PPGOBJECT Object
)
{
    PgLocatePoolObject(Block, Establisher, Object);

    if (-1 == Object->Type) {
        PgLocateSystemPtesObject(Block, Establisher, Object);
    }
}

void
NTAPI
PgCheckAllWorkerThread(
    __inout PPGBLOCK Block
)
{
    PETHREAD Thread = NULL;
    PEXCEPTION_FRAME ExceptionFrame = NULL;
    u64 EstablisherFrame = 0;
    u64 ReturnAddress = 0;
    u64 Stack = 0;
    u Index = 0;
    PPGOBJECT Object = NULL;
    b Chance = FALSE;
    PGOBJECT ObjectRecord = { 0 };

    if (NULL != Block->INITKDBG) {
        Thread = GetProcessFirstThread(
            GetRtBlock(Block),
            PsGetCurrentThreadProcess());

        while (GetThreadListEntry(GetRtBlock(Block),
            Thread) != GetProcessThreadListHead(
                GetRtBlock(Block),
                PsGetCurrentThreadProcess())) {
            if (PsGetCurrentThreadId() != PsGetThreadId(Thread) &&
                (u64)Block->ExpWorkerThread == __rduptr(
                (u8ptr)Thread +
                    GetRtBlock(Block)->OffsetKThreadWin32StartAddress)) {
                Chance = FALSE;

                Stack = __rduptr((u8ptr)Thread
                    + GetRtBlock(Block)->DebuggerDataBlock.OffsetKThreadInitialStack);

                // protect: 1f - outswapped kernel stack
                if (FALSE != MmIsAddressValid((ptr)Stack)) {
                    // KiScbQueueScanWorker 
                    // this function decrypt pg worker context
                    // only pg code use it

                    // 7600 1 level                                                
                    // ExpWorkerThread -> FsRtlUninitializeSmallMcb -> FsRtlMdlReadCompleteDevEx

                    // 9200 ~ 15063 1 level                     
                    // ExpWorkerThread -> KiScbQueueScanWorker(jmp) 
                    //     -> FsRtlUninitializeSmallMcb -> FsRtlMdlReadCompleteDevEx

                    // 16299 ~ later 2 level                    
                    // ExpWorkerThread -> KiScbQueueScanWorker(call) 
                    //     -> FsRtlUninitializeSmallMcb -> FsRtlMdlReadCompleteDevEx

                    if (GetRtBlock(Block)->BuildNumber >= 16299) {
                        // check KiScbQueueScanWorker

                        if (Block->ExpWorkerThreadReturn ==
                            (ptr)__rduptr(Stack - Block->OffsetExpWorkerThreadReturn)) {

                            ReturnAddress =
                                __rduptr(Stack
                                    - Block->OffsetExpWorkerThreadReturn
                                    - KSTART_FRAME_LENGTH);

                            if (FALSE != MmIsAddressValid((ptr)ReturnAddress)) {
                                if (ReturnAddress >= Block->KiScbQueueScanWorker.BeginAddress &&
                                    ReturnAddress < Block->KiScbQueueScanWorker.EndAddress) {
                                    Chance = TRUE;
                                }
                            }
                        }
                    }
                    else {
                        // check FsRtlUninitializeSmallMcb     

                        if (Block->ExpWorkerThreadReturn ==
                            (ptr)__rduptr(Stack - Block->OffsetExpWorkerThreadReturn)) {

                            // 48 83 EC 48      sub     rsp, 48h
                            // E8 5B CB FF FF   call    FsRtlMdlReadCompleteDevEx

                            ReturnAddress =
                                __rduptr(Stack
                                    - Block->OffsetExpWorkerThreadReturn - 0x50);

                            if (FALSE != MmIsAddressValid((ptr)ReturnAddress) &&
                                FALSE != MmIsAddressValid((ptr)(ReturnAddress + PG_COMPARE_BYTE_COUNT))) {
                                for (Index = 0;
                                    Index < Block->SizeINITKDBG - PG_COMPARE_BYTE_COUNT;
                                    Index++) {
                                    if (PG_COMPARE_BYTE_COUNT == RtlCompareMemory(
                                        (u8ptr)Block->INITKDBG + Index,
                                        (ptr)ReturnAddress,
                                        PG_COMPARE_BYTE_COUNT)) {
                                        Chance = TRUE;

                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                if (FALSE != Chance) {
                    // SwapContext
                    ExceptionFrame = (PEXCEPTION_FRAME)__rduptr(
                        (u8ptr)Thread +
                        GetRtBlock(Block)->DebuggerDataBlock.OffsetKThreadKernelStack);

                    EstablisherFrame =
                        ((u64)(ExceptionFrame + 1) + KSTART_FRAME_LENGTH + sizeof(ptr));

#ifdef DEBUG
                    vDbgPrint(
                        "[SHARK] < %p > found patchguard address in worker thread stack\n",
                        __rduptr(EstablisherFrame));
#endif // DEBUG

                    ObjectRecord.Type = -1;

                    PgLocateAllObject(
                        Block,
                        (ptr)__rduptr(EstablisherFrame),
                        &ObjectRecord);

                    if (-1 != ObjectRecord.Type) {
                        Object = PgCreateObject(
                            PgDeclassified,
                            ObjectRecord.Type,
                            ObjectRecord.BaseAddress,
                            ObjectRecord.RegionSize,
                            Block,
                            NULL,
                            Block->ClearCallback,
                            NULL,
                            Block->CaptureContext);

                        if (NULL != Object) {
                            Block->Count++;

                            // align stack 
                            // must start at Body.Parameter
                            __wruptr(
                                EstablisherFrame,
                                &Object->Body.Parameter);

#ifdef DEBUG
                            vDbgPrint(
                                "[SHARK] < %p > insert worker restart code\n",
                                Object);
#endif // DEBUG
                        }
                    }
                }
            }

            Thread = GetNexThread(GetRtBlock(Block), Thread);
        }
    }
}

void
NTAPI
PgClearAll(
    __inout PPGBLOCK Block
)
{

    // init self ldr
    CaptureImageExceptionValues(
        GetRtBlock(Block)->Self->DataTableEntry.DllBase,
        &GetRtBlock(Block)->Self->DataTableEntry.ExceptionTable,
        &GetRtBlock(Block)->Self->DataTableEntry.ExceptionTableSize);

    InsertTailList(
        &GetRtBlock(Block)->LoadedModuleList,
        &GetRtBlock(Block)->Self->DataTableEntry.InLoadOrderLinks);

    // init self exception
    InsertInvertedFunctionTable(
        GetRtBlock(Block)->Self->DataTableEntry.DllBase,
        GetRtBlock(Block)->Self->DataTableEntry.SizeOfImage);

    PgClearPoolEncryptedContext(Block);

    if (GetRtBlock(Block)->BuildNumber >= 9200) {
        PgClearSystemPtesEncryptedContext(Block);
    }

    if (NULL != Block->INITKDBG) {
        PgCheckAllWorkerThread(Block);

        __free(Block->INITKDBG);

        Block->INITKDBG = NULL;
    }
}

void
NTAPI
PgClearWorker(
    __inout ptr Argument
)
{
    u32 Index = 0;
    u64ptr Stack = 0;
    DISPATCHER_HEADER * Header = NULL;
    u32 ReturnLength = 0;
    u8ptr TargetPc = NULL;
    u32 Length = 0;
    b Chance = TRUE;

    struct {
        PPGBLOCK Block;
        KEVENT Notify;
        WORK_QUEUE_ITEM Worker;
    }*Context = Argument;

    Context->Block->ExpWorkerThreadReturn = _ReturnAddress();

    InitializePgBlock(Context->Block);

    Stack = IoGetInitialStack();

    while (*Stack != (u64)_ReturnAddress()) {
        Stack--;
    }

    Context->Block->OffsetExpWorkerThreadReturn =
        (u64)IoGetInitialStack() - (u64)Stack;

    if (GetRtBlock(Context->Block)->BuildNumber >= 9600) {
        // Header->Type = 0x15
        // Header->Hand = 0xac

        // WorkerContext = struct _DISPATCHER_HEADER

        while ((u64)Stack != (u64)&Argument) {
            Header = *(ptr *)Stack;

            if (FALSE != MmIsAddressValid(Header)) {
                if (FALSE != MmIsAddressValid((u8ptr)(Header + 1) - 1)) {
                    if (0x15 == Header->Type &&
                        0xac == Header->Hand) {
                        Context->Block->WorkerContext = Header;

                        break;
                    }
                }
            }

            Stack--;
        }
    }
    else {
        // CriticalWorkQueue = 0
        // DelayedWorkQueue = 1

        // WorkerContext = enum _WORK_QUEUE_TYPE

        Context->Block->WorkerContext = UlongToPtr(CriticalWorkQueue);
    }

    if (NT_SUCCESS(ZwQueryInformationThread(
        ZwCurrentThread(),
        ThreadQuerySetWin32StartAddress,
        &Context->Block->ExpWorkerThread,
        sizeof(ptr),
        &ReturnLength))) {
        for (Index = 0;
            Index < GetRtBlock(Context->Block)->DebuggerDataBlock.SizeEThread;
            Index += 8) {
            if ((u)Context->Block->ExpWorkerThread ==
                __rduptr((u8ptr)KeGetCurrentThread() + Index)) {
                GetRtBlock(Context->Block)->OffsetKThreadWin32StartAddress = Index;

                break;
            }
        }

        if (GetRtBlock(Context->Block)->BuildNumber >= 18362) {
            TargetPc = (u8ptr)Context->Block->ExpWorkerThread;

            while (TRUE) {
                Length = DetourGetInstructionLength(TargetPc);

                if (6 == Length) {
                    if (0 == _cmpbyte(TargetPc[0], 0x8b) &&
                        0 == _cmpbyte(TargetPc[1], 0x83)) {
                        Context->Block->OffsetSameThreadPassive = *(u32ptr)(TargetPc + 2);

#ifdef DEBUG
                        vDbgPrint(
                            "[SHARK] < %p > OffsetSameThreadPassive\n",
                            Context->Block->OffsetSameThreadPassive);
#endif // DEBUG

                        break;
                    }
                }

                TargetPc += Length;
            }
        }
    }

    if (0 == Context->Block->SizeCmpAppendDllSection ||
        0 == Context->Block->OffsetEntryPoint ||
        0 == Context->Block->SizeINITKDBG ||
        NULL == GetRtBlock(Context->Block)->PsInvertedFunctionTable ||
        NULL == Context->Block->KiStartSystemThread ||
        NULL == Context->Block->PspSystemThreadStartup ||
        NULL == Context->Block->Pool.PoolBigPageTable ||
        NULL == Context->Block->Pool.PoolBigPageTableSize ||
        NULL == Context->Block->ExpWorkerThread) {
        Chance = FALSE;
    }

    if (GetRtBlock(Context->Block)->BuildNumber >= 9200) {
        if (0 == Context->Block->SystemPtes.NumberOfPtes ||
            NULL == Context->Block->SystemPtes.BasePte ||
            NULL == Context->Block->MmAllocateIndependentPages ||
            NULL == Context->Block->MmFreeIndependentPages ||
            NULL == Context->Block->MmSetPageProtection ||
            0 == Context->Block->KiScbQueueScanWorker.BeginAddress ||
            0 == Context->Block->KiScbQueueScanWorker.BeginAddress) {
            Chance = FALSE;
        }
    }

    if (GetRtBlock(Context->Block)->BuildNumber >= 9600) {
        if (0 == Context->Block->WorkerContext ||
            NULL == Context->Block->KiWaitNever ||
            NULL == Context->Block->KiWaitAlways) {
            Chance = FALSE;
        }
    }

    if (GetRtBlock(Context->Block)->BuildNumber >= 18362) {
        if (0 == Context->Block->OffsetSameThreadPassive) {
            Chance = FALSE;
        }
    }

    if (FALSE != Chance) {
        IpiSingleCall(
            (PGKERNEL_ROUTINE)NULL,
            (PGSYSTEM_ROUTINE)NULL,
            (PGRUNDOWN_ROUTINE)PgClearAll,
            (PGNORMAL_ROUTINE)Context->Block);
    }

    KeSetEvent(&Context->Notify, LOW_PRIORITY, FALSE);
}

void
NTAPI
PgClear(
    __inout PPGBLOCK Block
)
{
    struct {
        PPGBLOCK Block;
        KEVENT Notify;
        WORK_QUEUE_ITEM Worker;
    }Context = { Block, {0}, {0} };

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
