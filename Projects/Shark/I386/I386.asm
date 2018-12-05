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

.686p

        .xlist
include ks386.inc
include callconv.inc
        .list
        
_TEXT$00    SEGMENT PAGE 'CODE'

    cPublicProc _IsPAEEnable, 0
    
        mov eax, cr4
        and eax, CR4_PAE
        shr eax, 5

        stdRET _IsPAEEnable
        
    stdENDP _IsPAEEnable
    
    cPublicProc __FlushSingleTb, 1
        
        mov eax, [esp + 4]
        invlpg [eax]

        stdRET __FlushSingleTb
        
    stdENDP __FlushSingleTb
        
    cPublicProc __MultipleDispatcher, 4

        mov edi, edi

        push ebp
        mov ebp, esp

        mov eax, [ebp + 8]

        test eax, eax
        jz @f

        push [ebp + 14h]
        push [ebp + 10h]
        push [ebp + 0ch]

        call eax
        
        mov esp, ebp
        pop ebp
        
        stdRET __MultipleDispatcher
        
@@ :    
        mov eax, [ebp + 0ch]

        test eax, eax
        jz @f

        push [ebp + 14h]
        push [ebp + 10h]

        call eax
        
        mov esp, ebp
        pop ebp
        
        stdRET __MultipleDispatcher
        
@@ :    
        mov eax, [ebp + 10h]

        test eax, eax
        jz nothing

        push [ebp + 14h]

        call eax
        
        mov esp, ebp
        pop ebp
        
        stdRET __MultipleDispatcher
        
nothing : 
        xor eax, eax

        mov esp, ebp
        pop ebp
        
        stdRET __MultipleDispatcher
        
    stdENDP __MultipleDispatcher
    
_TEXT$00    ends

        end
