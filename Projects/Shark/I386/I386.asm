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

.386p

.model flat, stdcall

        .xlist
include callconv.inc
        .list
        
_TEXT$00    SEGMENT PAGE 'CODE'

    cPublicProc __FlushSingleTb, 1
        
        mov eax, [esp + 4]
        invlpg [eax]

        stdRET __FlushSingleTb
        
    stdENDP __FlushSingleTb
        
    cPublicProc __IpiDispatcher, 4

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
        
        stdRET __IpiDispatcher
        
@@ :    
        mov eax, [ebp + 0ch]

        test eax, eax
        jz @f

        push [ebp + 14h]
        push [ebp + 10h]

        call eax
        
        mov esp, ebp
        pop ebp
        
        stdRET __IpiDispatcher
        
@@ :    
        mov eax, [ebp + 10h]

        test eax, eax
        jz @f

        push [ebp + 14h]

        call eax
        
        mov esp, ebp
        pop ebp
        
        stdRET __IpiDispatcher
        
error : 
        xor eax, eax

        mov esp, ebp
        pop ebp
        
        stdRET __IpiDispatcher
        
    stdENDP __IpiDispatcher
    
    cPublicProc _IsPAEEnable, 0
    
        mov eax, cr4
        and eax, CR4_PAE
        shr eax, 5

        stdRET _IsPAEEnable
        
    stdENDP _IsPAEEnable
    
    cPublicProc _GetPteAddress, 1
    
        mov edx, cr4
        and edx, CR4_PAE
        shr edx, 5
        mov eax, [esp + 4]
        shr eax, 0ch
        mov cl, 2
        add cl, dl
        shl eax, cl
        lea eax, [eax + 0C0000000h]

        stdRET _GetPteAddress
        
    stdENDP _GetPteAddress
    
    cPublicProc _GetPdeAddress, 1
    
        mov edx, cr4
        and edx, CR4_PAE
        shr edx, 5
        mov cl, 16h
        sub cl, dl
        mov eax, [esp + 4]
        shr eax, cl
        mov cl, 2
        add cl, dl
        shl eax, cl

        test dl, dl
        jz @f

        mov ecx, 300000h

@@ :    lea eax, [eax + ecx + 0C0300000h]

        stdRET _GetPdeAddress
        
    stdENDP _GetPdeAddress

    cPublicProc _GetVirtualAddressMappedByPde, 1
    
        mov edx, cr4
        and edx, CR4_PAE
        shr edx, 5
        mov eax, [esp + 4]
        mov cl, 14h
        shl dl, 1
        sub cl, dl
        shl eax, cl
    
        stdRET _GetVirtualAddressMappedByPde
        
    stdENDP _GetVirtualAddressMappedByPde
    
    cPublicProc _GetVirtualAddressMappedByPte, 1
    
        mov edx, cr4
        and edx, CR4_PAE
        shr edx, 5
        mov eax, [esp + 4]
        mov cl, 0ah
        sub cl, dl
        shl eax, cl
    
        stdRET _GetVirtualAddressMappedByPte
        
    stdENDP _GetVirtualAddressMappedByPte
    
_TEXT$00    ends

        end
