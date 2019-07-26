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

.686

        .xlist
include callconv.inc
        .list
        
_TEXT$00    SEGMENT PAGE 'CODE'

; BOOLEAN
;     NTAPI
;     _CmpByte(
;         __in CHAR b1,
;         __in CHAR b2
;     );

    cPublicProc __CmpByte, 2

        mov cl, [esp + 4]
        mov dl, [esp + 8]
        cmp cl, dl
        setnz al

        stdRET __CmpByte

    stdENDP __CmpByte
    
; BOOLEAN
;     NTAPI
;     _CmpShort(
;         __in SHORT s1,
;         __in SHORT s2
;     );

    cPublicProc __CmpShort, 2

        mov cx, [esp + 4]
        mov dx, [esp + 8]
        cmp cx, dx
        setnz al
        
        stdRET __CmpShort

    stdENDP __CmpShort

; BOOLEAN
;     NTAPI
;     _CmpLong(
;         __in LONG l1,
;         __in LONG l2
;     );

    cPublicProc __CmpLong, 2

        mov ecx, [esp + 4]
        mov edx, [esp + 8]
        cmp ecx, edx
        setnz al
        
        stdRET __CmpLong

    stdENDP __CmpLong
    
; BOOLEAN
;     NTAPI
;     _CmpLongLong(
;         __in LONGLONG ll1,
;         __in LONGLONG ll2
;     );

    cPublicProc __CmpLongLong, 4

        mov ecx, [esp + 4]
        mov edx, [esp + 0ch]
        cmp ecx, edx

        jnz @f
    
        mov ecx, [esp + 8h]
        mov edx, [esp + 10h]
        cmp ecx, edx

    @@ :
        setnz al
        
        stdRET __CmpLongLong

    stdENDP __CmpLongLong

_TEXT$00    ends

        end
