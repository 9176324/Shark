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

.686p

        .xlist
include ks386.inc
include callconv.inc
        .list

_TEXT$00    SEGMENT PAGE 'CODE'

    cPublicProc __FlushSingleTb, 1
        
        mov eax, [esp + 4]
        invlpg [eax]

        stdRET __FlushSingleTb
        
    stdENDP __FlushSingleTb
        
    cPublicProc __GetPdeAddress, 2
    
        mov eax, [esp + 4]
        shr eax, 15h
        shl eax, 3
        add eax, [esp + 8]

        stdRET __GetPdeAddress
        
    stdENDP __GetPdeAddress
    
    cPublicProc __GetPdeAddressPae, 2
    
        mov eax, [esp + 4]
        shr eax, 16h
        shl eax, 2
        add eax, [esp + 8]

        stdRET __GetPdeAddressPae
        
    stdENDP __GetPdeAddressPae
    
    cPublicProc __GetPteAddress, 2
    
        mov eax, [esp + 4]
        shr eax, 0ch
        shl eax, 2
        add eax, [esp + 8]

        stdRET __GetPteAddress
        
    stdENDP __GetPteAddress
    
    cPublicProc __GetPteAddressPae, 2
    
        mov eax, [esp + 4]
        shr eax, 0ch
        shl eax, 3
        add eax, [esp + 8]

        stdRET __GetPteAddressPae
        
    stdENDP __GetPteAddressPae
    
    cPublicProc __GetVirtualAddressMappedByPte, 1
    
        mov eax, [esp + 4]
        shl eax, 0ah

        stdRET __GetVirtualAddressMappedByPte
        
    stdENDP __GetVirtualAddressMappedByPte
    
    cPublicProc __GetVirtualAddressMappedByPtePae, 1
    
        mov eax, [esp + 4]
        shl eax, 9

        stdRET __GetVirtualAddressMappedByPtePae
        
    stdENDP __GetVirtualAddressMappedByPtePae
    
    cPublicProc __GetVirtualAddressMappedByPde, 1
    
        mov eax, [esp + 4]
        shl eax, 14h

        stdRET __GetVirtualAddressMappedByPde
        
    stdENDP __GetVirtualAddressMappedByPde
    
    cPublicProc __GetVirtualAddressMappedByPdePae, 1
    
        mov eax, [esp + 4]
        shl eax, 12h

        stdRET __GetVirtualAddressMappedByPdePae
        
    stdENDP __GetVirtualAddressMappedByPdePae
    
    cPublicProc __GuardCall, 4

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
        
        stdRET __GuardCall
        
@@ :    
        mov eax, [ebp + 0ch]

        test eax, eax
        jz @f

        push [ebp + 14h]
        push [ebp + 10h]

        call eax
        
        mov esp, ebp
        pop ebp
        
        stdRET __GuardCall
        
@@ :    
        mov eax, [ebp + 10h]

        test eax, eax
        jz @f

        push [ebp + 14h]

        call eax
        
        mov esp, ebp
        pop ebp
        
        stdRET __GuardCall
        
@@ :    
        mov eax, [ebp + 14h]

        test eax, eax
        jz error

        call eax
        
        mov esp, ebp
        pop ebp
        
        stdRET __GuardCall
        
error : 
        xor eax, eax

        mov esp, ebp
        pop ebp
        
        stdRET __GuardCall
        
    stdENDP __GuardCall
    
_TEXT$00    ends

        end
