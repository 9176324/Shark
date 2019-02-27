;
;
; Copyright (c) 2019 by blindtiger. All rights reserved.
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

include ksamd64.inc
include macamd64.inc

    LEAF_ENTRY _FlushSingleTb, _TEXT$00
        
        mov rax, rcx
        invlpg [rax]
        ret

    LEAF_END _FlushSingleTb, _TEXT$00
        
    NESTED_ENTRY _MultipleDispatcher, _TEXT$00

        alloc_stack ( KSTART_FRAME_LENGTH - 8 )
        
        END_PROLOGUE
        
        mov rax, rcx

        test rax, rax
        jz @f

        mov rcx, rdx
        mov rdx, r8
        mov r8, r9

        call rax

        add rsp,  ( KSTART_FRAME_LENGTH - 8 )

        ret

@@ :    
        mov rax, rdx

        test rax, rax
        jz @f
        
        mov rcx, r8
        mov rdx, r9

        call rax

        add rsp,  ( KSTART_FRAME_LENGTH - 8 )

        ret

@@ :    
        mov rax, r8

        test rax, rax
        jz @f
        
        mov rcx, r9

        call rax

        add rsp,  ( KSTART_FRAME_LENGTH - 8 )

        ret
        
@@ :    
        mov rax, r9

        test rax, rax
        jz error
        
        call rax

        add rsp,  ( KSTART_FRAME_LENGTH - 8 )

        ret

error : 
        add rsp,  ( KSTART_FRAME_LENGTH - 8 )

        ret

    NESTED_END _MultipleDispatcher, _TEXT$00
        
        end
