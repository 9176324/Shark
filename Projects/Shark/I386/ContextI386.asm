;
;
; Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
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
; The Initial Developer of the Original Code is blindtiger.
;
;

.686p

        .xlist
include ks386.inc
include callconv.inc
        .list
        
_TEXT$00    SEGMENT PAGE 'CODE'

; DECLSPEC_NORETURN
;     void
;     STDCALL
;     _CaptureContext(
;         __in u32 ProgramCounter,
;         __in ptr Guard,
;         __in PGUARD_CALLBACK Callback,
;         __in_opt ptr Parameter,
;         __in_opt ptr Reserved
;     );

StackPointer EQU 14h
Reserved EQU 10h
Parameter EQU 0ch
Callback EQU 8
Guard EQU 4
ProgramCounter EQU 0

    cPublicProc __CaptureContext, 5
    
        pushfd

        sub esp, ContextFrameLength - 4
        
        push ebx
        
        lea ebx, [esp]

        and ebx, not 7

        pop [ebx].CsEbx

        mov [ebx].CsSegEs, es
        mov [ebx].CsSegCs, cs
        mov [ebx].CsSegSs, ss
        mov [ebx].CsSegDs, ds
        mov [ebx].CsSegFs, fs
        mov [ebx].CsSegGs, gs
        
        mov [ebx].CsEax, eax
        mov [ebx].CsEcx, ecx
        mov [ebx].CsEdx, edx

        mov [ebx].CsEbp, ebp
        mov [ebx].CsEsi, esi
        mov [ebx].CsEdi, edi
        
        mov eax, [esp - 4].ContextFrameLength
        mov [ebx].CsEFlags, eax

        lea eax, [esp].ContextFrameLength.StackPointer
        mov [ebx].CsEsp, eax

        mov eax, [esp].ContextFrameLength.Guard
        mov [ebx].CsEip, eax
        
        mov eax, CONTEXT_FULL
        mov [ebx].CsContextFlags, eax
        
        mov edx, [esp].ContextFrameLength.ProgramCounter
        lea ecx, [ebx]
        mov edi, [esp].ContextFrameLength.Reserved
        mov esi, [esp].ContextFrameLength.Parameter
        mov eax, [esp].ContextFrameLength.Callback
        
        push edi
        push esi
        push edx
        push ecx

        call eax

    stdENDP __CaptureContext
    
_TEXT$00    ends

        end
