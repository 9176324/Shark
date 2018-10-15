        TITLE   "Capture Stack Back Trace"
;++
;
; Copyright (c) Microsoft Corporation. All rights reserved. 
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
; Module Name:
;
;    stkwalk.asm
;
; Abstract:
;
;    This module implements the routine RtlCaptureStackBackTrace.  It will
;    walker the stack back trace and capture a portion of it.
;
;--

.386p
        .xlist
include ks386.inc
include callconv.inc            ; calling convention macros
        .list

        EXTRNP  _MmIsAddressValid,1

_TEXT   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

        page ,132
        subttl  "RtlGetCallersAddress"
;++
;
; VOID
; RtlGetCallersAddress(
;    OUT PVOID *CallersAddress,
;    OUT PVOID *CallersCaller
;    )
;
; Routine Description:
;
;    This routine walks up the stack frames, capturing the first two return
;    addresses
;
;
; Arguments:
;
;    OUT PVOID CallersAddress - returns callers return address
;    OUT PVOID CallersCaller - returns callers caller return address
;
; Return Value:
;
;     None.
;
;
;--
RgcaCallersAddress      EQU [ebp+08h]
RgcaCallersCaller       EQU [ebp+0Ch]
cPublicProc _RtlGetCallersAddress ,2
        push    ebp
        mov     ebp, esp
        push    ebx                     ; Save EBX
        push    esi                     ; Save ESI
        push    edi                     ; Save EDI
        mov     eax, PCR[PcPrcbData+PbCurrentThread] ; (eax)->current thread
        push    ThInitialStack[eax]     ;  RcbtInitialStack = base of kernel stack
        push    esp                     ; Save current esp for handler
        push    offset RgcaFault        ; Address of handler
        push    PCR[PcExceptionList]    ; Save current list head
        mov     PCR[PcExceptionList],esp; Put us on list
        xor     esi,esi                 ; (ESI) will contain callers return address
        xor     edi,edi                 ; (EDI) will contain callers caller return address

        mov     edx,ebp                 ; (EDX) = current frame pointer
        mov     edx,[edx]               ; (EDX) = callers frame pointer
        cmp     edx,ebp                 ; If outside stack limits,
        jbe     short RgcaExit          ; ...then exit
        cmp     edx,RcbtInitialStack
        jae     short RgcaCheckForDpcStack
        cmp     edx,ThStackLimit[eax]
        jb      short RgcaCheckForDpcStack

Rgca20:
        mov     esi,[edx].4             ; Get callers return address

        mov     edx,[edx]               ; (EDX) = callers caller frame pointer
        cmp     edx,ebp                 ; If outside stack limits,
        jbe     short RgcaExit          ; ...then exit
        cmp     edx,RcbtInitialStack
        jae     short RgcaExit

        mov     edi,[edx].4             ; Get callers caller return address
RgcaExit:
        mov     ecx,RgcaCallersAddress
        jecxz   @F
        mov     [ecx],esi
@@:
        mov     ecx,RgcaCallersCaller
        jecxz   @F
        mov     [ecx],edi
@@:
        pop     PCR[PcExceptionList]    ; Restore Exception list
        pop     edi                     ; discard handler address
        pop     edi                     ; discard saved esp
        pop     edi                     ; discard RcbtInitialStack
        pop     edi                     ; Restore EDI
        pop     esi                     ; Restore ESI
        pop     ebx                     ; Restore EBX
        pop     ebp
        stdRET  _RtlGetCallersAddress

;
; We may be executing on the DPC stack for this processor which is ok.
;

RgcaCheckForDpcStack:

        ; Check if DPC active.

        cmp     dword ptr PCR[PcPrcbData+PbDpcRoutineActive], 0
        mov     eax, PCR[PcPrcbData+PbDpcStack]
        je      short RgcaExit          ; if no DPC active, must be bad stack.

        ; Check if address if below DPC stack upper bound
        ;
        ; Note: If we ARE on the DPC stack, we need to adjust this function's
        ; idea of the initial stack pointer so it will succeed the check at
        ; the next level.   We do not support transitioning across stacks in
        ; this function.

        cmp     edx, eax
        mov     RcbtInitialStack, eax
        jae     short RgcaExit          ; exit if above upper bound

        ; Check if below DPC stack lower bound.

        sub     eax, KERNEL_STACK_SIZE
        cmp     edx, eax
        ja      short Rgca20            ; jif on DPC stack
        jmp     short RgcaExit          ; not on DPC stack.


RgcaFault:
;
; Quick and dirty exception handler.  This will not unwind properly, which
; is ok because this function is a leaf except for calling KeGetCurrentIrql,
; which has no exception handler.
;
        mov     eax,[esp+0Ch]           ; (esp)->Context
        mov     edi,CsEdi[eax]          ; restore buffer pointer
        mov     esp,[esp+8]             ; (esp)->ExceptionList
        jmp     RgcaExit                ;
stdENDP _RtlGetCallersAddress

RcbtInitialStack        EQU [ebp-10h]

_TEXT   ends
        end

