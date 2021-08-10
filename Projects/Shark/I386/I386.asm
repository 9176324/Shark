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
    
    cPublicProc __GetVaMappedByPte, 1
    
        mov eax, [esp + 4]
        shl eax, 0ah

        stdRET __GetVaMappedByPte
        
    stdENDP __GetVaMappedByPte
    
    cPublicProc __GetVaMappedByPtePae, 1
    
        mov eax, [esp + 4]
        shl eax, 9

        stdRET __GetVaMappedByPtePae
        
    stdENDP __GetVaMappedByPtePae
    
    cPublicProc __GetVaMappedByPde, 1
    
        mov eax, [esp + 4]
        shl eax, 14h

        stdRET __GetVaMappedByPde
        
    stdENDP __GetVaMappedByPde
    
    cPublicProc __GetVaMappedByPdePae, 1
    
        mov eax, [esp + 4]
        shl eax, 12h

        stdRET __GetVaMappedByPdePae
        
    stdENDP __GetVaMappedByPdePae
    
_TEXT$00    ends

        end
