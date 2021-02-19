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

include macamd64.inc

; b
;     NTAPI
;     _cmpbyte(
;         __in s8 b1,
;         __in s8 b2
;     );

    LEAF_ENTRY _cmpbyte, _TEXT$00
        
        cmp cl, dl
        setnz al
        ret

    LEAF_END _cmpbyte, _TEXT$00
        
; b
;     NTAPI
;     _cmpword(
;         __in s16 s1,
;         __in s16 s2
;     );

    LEAF_ENTRY _cmpword, _TEXT$00
        
        cmp cx, dx
        setnz al
        ret

    LEAF_END _cmpword, _TEXT$00
    
; b
;     NTAPI
;     _cmpdword(
;         __in s32 l1,
;         __in s32 l2
;     );

    LEAF_ENTRY _cmpdword, _TEXT$00
        
        cmp ecx, edx
        setnz al
        ret

    LEAF_END _cmpdword, _TEXT$00
    
; b
;     NTAPI
;     _cmpqword(
;         __in s64 ll1,
;         __in s64 ll2
;     );

    LEAF_ENTRY _cmpqword, _TEXT$00
        
        cmp rcx, rdx
        setnz al
        ret

    LEAF_END _cmpqword, _TEXT$00

        end
