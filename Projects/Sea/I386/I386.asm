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

.686

        .xlist
include callconv.inc
        .list

_TEXT   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:FLAT, FS:NOTHING, GS:NOTHING
        
    __ValidateEH3RN :
 
        mov edi, edi
 
        push ebp
        mov ebp, esp
 
        mov eax, [ebp + 8]
        test byte ptr [eax + 8], 3
        mov eax, 0
        setz al
 
        mov esp, ebp
        pop ebp
 
        ret
    
    PUBLIC __ValidateEH3RN
    
_TEXT   ends

        end
