;
;
; Copyright (c) 2018 by blindtiger. All rights reserved.
;
; The contents of this file are subject to the Mozilla Public License Version
; 2.0 (the "License"); you may not use this file except in compliance with
; the License. You may obtain a copy of the License at
; http://www.mozilla.org/MPL/
;
; Software distributed under the License is distributed on an "AS IS" basis,
; WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
; for the specific language governing rights and limitations under the
; License.
;
; The Initial Developer of the Original e is blindtiger.
;
;

include ksamd64.inc
include macamd64.inc

PgbIoGetInitialStack                    equ 00000000h
PgbWorkerContext                        equ 00000008h
PgbExpWorkerThread                      equ 00000010h
PgbPspSystemThreadStartup               equ 00000018h
PgbKiStartSystemThread                  equ 00000020h
PgbDbgPrint                             equ 00000028h
PgbClearEncryptedContextMessage         equ 00000030h
PgbRevertWorkerToSelfMessage            equ 00000038h
PgbRtlCompareMemory                     equ 00000040h
PgbSdbpCheckDll                         equ 00000048h
PgbSizeOfSdbpCheckDll                   equ 00000050h
PgbCheckPatchGuardCode                  equ 00000058h
PgbClearEncryptedContext                equ 00000060h
PgbRevertWorkerToSelf                   equ 00000068h
PgbRtlRestoreContext                    equ 00000070h
PgbExQueueWorkItem                      equ 00000078h
PgbExFreePool                           equ 00000080h
PgbReferenceCount                       equ 00000088h

; ULONG64
; NTAPI
; _btc64(
;     __in ULONG64 a,
;     __in ULONG64 b
; );

    LEAF_ENTRY _btc64, _TEXT$00

        btc rcx, rdx
        mov rax, rcx
        ret
    
    LEAF_END _btc64, _TEXT$00
        
; VOID
;     NTAPI
;     _MakePgFire(
;         VOID
; );

    LEAF_ENTRY _MakePgFire, _TEXT$00
        
        sub rsp, 10h

        lea rcx, [rsp + 2]

        sidt fword ptr [rcx]

        mov ax, 0ffffh
        mov [rcx], ax

        lidt fword ptr [rcx]
        sidt fword ptr [rcx]

        add rsp, 10h

        ret

    LEAF_END _MakePgFire, _TEXT$00
        
; PVOID
; NTAPI
; _ClearEncryptedContext(
;     __in PVOID Reserved,
;     __in PVOID PatchGuardContext
; );

    LEAF_ENTRY _ClearEncryptedContext, _TEXT$00
    
@@:
        dq 1 dup (0)                                ; PgbPatchGuardBlock

        push rbx
        sub rsp, KSTART_FRAME_LENGTH - 10h
        
        lea rbx, @b
        mov rbx, [rbx]
        
        lea rcx, PgbReferenceCount [rbx]
        lock dec qword ptr [rcx]

        mov rdx, [rcx]
        mov rcx, PgbClearEncryptedContextMessage [rbx]
        mov rax, PgbDbgPrint [rbx]
        call rax
        
        add rsp, KSTART_FRAME_LENGTH - 10h
        pop rbx

        add rsp, 30h

        ret

    LEAF_END _ClearEncryptedContext, _TEXT$00
        
; VOID
; NTAPI
; _RevertWorkerToSelf(
;     VOID
; );

    LEAF_ENTRY _RevertWorkerToSelf, _TEXT$00
    
@@:
        dq 1 dup (0)                                ; PgbPatchGuardBlock

        and rsp, not 0fh
        
        lea rbx, @b
        mov rbx, [rbx]
        
        mov r15, PgbWorkerContext [rbx]
        mov r14, PgbExpWorkerThread [rbx]
        mov r13, PgbPspSystemThreadStartup [rbx]
        mov r12, PgbKiStartSystemThread [rbx]
        
        lea rcx, PgbReferenceCount [rbx]
        lock dec qword ptr [rcx]

        mov rdx, [rcx]
        mov rcx, PgbRevertWorkerToSelfMessage [rbx]
        mov rax, PgbDbgPrint [rbx]
        call rax
        
        mov rax, PgbIoGetInitialStack [rbx]
        call rax
        
        mov rsp, rax
        sub rsp, KSTART_FRAME_LENGTH
        
        mov SfP1Home [rsp], r15
        mov SfP2Home [rsp], r14
        mov SfP3Home [rsp], r13
        mov qword ptr SfReturn [rsp], 0
        jmp r12

    LEAF_END _RevertWorkerToSelf, _TEXT$00
    
; VOID
; NTAPI
; _CheckPatchGuardCode(
;     __in PVOID BaseAddress,
;     __in SIZE_T RegionSize
; );

    NESTED_ENTRY _CheckPatchGuardCode, _TEXT$00

        alloc_stack ( KSTART_FRAME_LENGTH - 8 )
        
        END_PROLOGUE
        
        mov rsi, rcx
        mov rdi, PgbSdbpCheckDll [rbx]

        mov r12, rdx
        mov r13, PgbSizeOfSdbpCheckDll [rbx]
        sub r12, r13

        xor r14, r14

        mov r15, PgbRtlCompareMemory [rbx]

@@:
        lea rcx, [rsi + r14]
        mov rdx, rdi
        mov r8, r13
        
        call r15

        cmp rax, r13
        setnz al

        jz @f

        inc r14

        cmp r14, r12
        jnz @b

@@:
        add rsp,  ( KSTART_FRAME_LENGTH - 8 )
        
        ret

    NESTED_END _CheckPatchGuardCode, _TEXT$00

; VOID
; NTAPI
; _PgGuardCall(
;     VOID
; );

    NESTED_ENTRY _PgGuardCall, _TEXT$00
        
@@:
        dq 1 dup (0)                                ; _GuardCall->Usable
        dq 1 dup (0)                                ; PgbPatchGuardBlock
        dq 4 dup (0)                                ; _GuardCall->Parameters

        alloc_stack CONTEXT_FRAME_LENGTH

        END_PROLOGUE
        
        mov CxSegCs [rsp], cs
        mov CxSegDs [rsp], ds
        mov CxSegEs [rsp], es
        mov CxSegSs [rsp], ss
        mov CxSegFs [rsp], fs
        mov CxSegGs [rsp], gs

        mov CxRax [rsp], rax
        mov CxRcx [rsp], rcx
        mov CxRdx [rsp], rdx
        mov CxRbx [rsp], rbx

        lea rax, CONTEXT_FRAME_LENGTH [rsp]

        mov CxRsp [rsp], rax
        mov CxRbp [rsp], rbp
        mov CxRsi [rsp], rsi
        mov CxRdi [rsp], rdi
        mov CxR8 [rsp], r8
        mov CxR9 [rsp], r9
        mov CxR10 [rsp], r10
        mov CxR11 [rsp], r11
        mov CxR12 [rsp], r12
        mov CxR13 [rsp], r13
        mov CxR14 [rsp], r14
        mov CxR15 [rsp], r15

        movdqa CxXmm0 [rsp], xmm0
        movdqa CxXmm1 [rsp], xmm1
        movdqa CxXmm2 [rsp], xmm2
        movdqa CxXmm3 [rsp], xmm3
        movdqa CxXmm4 [rsp], xmm4
        movdqa CxXmm5 [rsp], xmm5
        movdqa CxXmm6 [rsp], xmm6
        movdqa CxXmm7 [rsp], xmm7
        movdqa CxXmm8 [rsp], xmm8
        movdqa CxXmm9 [rsp], xmm9
        movdqa CxXmm10 [rsp], xmm10
        movdqa CxXmm11 [rsp], xmm11
        movdqa CxXmm12 [rsp], xmm12
        movdqa CxXmm13 [rsp], xmm13
        movdqa CxXmm14 [rsp], xmm14
        movdqa CxXmm15 [rsp], xmm15

        stmxcsr CxMxCsr[rsp]

        pushfq
        pop rax
        mov CxEFlags[rsp], eax

        mov eax, CONTEXT_FULL or CONTEXT_SEGMENTS
        mov CxContextFlags [rsp], eax
        
        lea rbx, @b

        mov rax, [rbx + 10h]
        mov CxRip [rsp], rax

        mov rcx, [rbx + 18h]                        ; BaseAddress
        mov rdx, [rbx + 20h]                        ; RegionSize
        
        mov rbx, [rbx + 8]
        
        mov rax, PgbCheckPatchGuardCode [rbx]

        call rax
        
        test al, al
        jnz @f

        mov rax, PgbRevertWorkerToSelf [rbx]

        call rax
        
        int 3

@@:
        mov rax, PgbRtlRestoreContext [rbx]

        lea rcx, [rsp]
        xor rdx, rdx

        call rax

        int 3

    NESTED_END _PgGuardCall, _TEXT$00

        end
