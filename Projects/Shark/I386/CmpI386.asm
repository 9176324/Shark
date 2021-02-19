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
; The Initial Developer of the Original e is blindtiger.
;
;

.686

        .xlist
include callconv.inc
        .list
        
_TEXT$00    SEGMENT PAGE 'CODE'

; b
;     NTAPI
;     _cmpbyte(
;         __in s8 b1,
;         __in s8 b2
;     );

    cPublicProc __cmpbyte, 2

        mov cl, [esp + 4]
        mov dl, [esp + 8]
        cmp cl, dl
        setnz al

        stdRET __cmpbyte

    stdENDP __cmpbyte
    
; b
;     NTAPI
;     _cmpword(
;         __in s16 s1,
;         __in s16 s2
;     );

    cPublicProc __cmpword, 2

        mov cx, [esp + 4]
        mov dx, [esp + 8]
        cmp cx, dx
        setnz al
        
        stdRET __cmpword

    stdENDP __cmpword

; b
;     NTAPI
;     _cmpdword(
;         __in s32 l1,
;         __in s32 l2
;     );

    cPublicProc __cmpdword, 2

        mov ecx, [esp + 4]
        mov edx, [esp + 8]
        cmp ecx, edx
        setnz al
        
        stdRET __cmpdword

    stdENDP __cmpdword
    
; b
;     NTAPI
;     _cmpqword(
;         __in s64 ll1,
;         __in s64 ll2
;     );

    cPublicProc __cmpqword, 4

        mov ecx, [esp + 4]
        mov edx, [esp + 0ch]
        cmp ecx, edx

        jnz @f
    
        mov ecx, [esp + 8h]
        mov edx, [esp + 10h]
        cmp ecx, edx

    @@ :
        setnz al
        
        stdRET __cmpqword

    stdENDP __cmpqword

_TEXT$00    ends

        end
