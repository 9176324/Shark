;
;
; Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
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

; DECLSPEC_NORETURN
;     VOID
;     STDCALL
;     _CaptureContext(
;         __in ULONG ProgramCounter,
;         __in PVOID Detour,
;         __in PGUARD Guard,
;         __in_opt PVOID Parameter,
;         __in_opt PVOID Reserved
;     );

StackPointer EQU 28h
Reserved EQU 20h
Parameter EQU 18h
Guard EQU 10h
Detour EQU 8
ProgramCounter EQU 0

    LEAF_ENTRY _CaptureContext, _TEXT$00
        
        sub rsp, CONTEXT_FRAME_LENGTH

        push rcx

        lea rcx, [rsp]

        and rcx, not 0fh

        pop CxRcx [rcx]
        
        mov CxSegCs [rcx], cs
        mov CxSegDs [rcx], ds
        mov CxSegEs [rcx], es
        mov CxSegSs [rcx], ss
        mov CxSegFs [rcx], fs
        mov CxSegGs [rcx], gs

        mov CxRax [rcx], rax
        mov CxRdx [rcx], rdx
        mov CxRbx [rcx], rbx

        lea rax, CONTEXT_FRAME_LENGTH + StackPointer [rsp]

        mov CxRsp [rcx], rax
        mov CxRbp [rcx], rbp
        mov CxRsi [rcx], rsi
        mov CxRdi [rcx], rdi
        mov CxR8 [rcx], r8
        mov CxR9 [rcx], r9
        mov CxR10 [rcx], r10
        mov CxR11 [rcx], r11
        mov CxR12 [rcx], r12
        mov CxR13 [rcx], r13
        mov CxR14 [rcx], r14
        mov CxR15 [rcx], r15
        
        movdqa CxXmm0 [rcx], xmm0
        movdqa CxXmm1 [rcx], xmm1
        movdqa CxXmm2 [rcx], xmm2
        movdqa CxXmm3 [rcx], xmm3
        movdqa CxXmm4 [rcx], xmm4
        movdqa CxXmm5 [rcx], xmm5
        movdqa CxXmm6 [rcx], xmm6
        movdqa CxXmm7 [rcx], xmm7
        movdqa CxXmm8 [rcx], xmm8
        movdqa CxXmm9 [rcx], xmm9
        movdqa CxXmm10 [rcx], xmm10
        movdqa CxXmm11 [rcx], xmm11
        movdqa CxXmm12 [rcx], xmm12
        movdqa CxXmm13 [rcx], xmm13
        movdqa CxXmm14 [rcx], xmm14
        movdqa CxXmm15 [rcx], xmm15

        stmxcsr CxMxCsr [rcx]
        
        pushfq
        pop CxEFlags [rcx]

        mov rax, CONTEXT_FRAME_LENGTH + Detour [rsp]
        mov CxRip [rcx], rax

        mov eax, CONTEXT_FULL
        mov CxContextFlags [rcx], eax
        
        mov r9, CONTEXT_FRAME_LENGTH + Reserved [rsp]
        mov r8, CONTEXT_FRAME_LENGTH + Parameter [rsp]
        mov rdx, CONTEXT_FRAME_LENGTH + ProgramCounter [rsp]
        lea rcx, [rcx]
        mov rax, CONTEXT_FRAME_LENGTH + Guard [rsp]

        call rax

    LEAF_END _CaptureContext, _TEXT$00
   
        end
