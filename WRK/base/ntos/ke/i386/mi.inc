;++
;
; Copyright (c) Microsoft Corporation. All rights reserved. 
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
;   Module Name:
;
;       MI.INC
;
;   Abstract:
;
;       This module contains equates for x86 machine instructions
;
;   Revision History:
;       
;       This information used to reside in Trap.asm, but is now needed in
;       multiple source files.
;
;--


MAX_INSTRUCTION_LENGTH		EQU	15
MAX_INSTRUCTION_PREFIX_LENGTH	EQU	4
MI_LOCK_PREFIX			EQU	0F0H
MI_ADDR_PREFIX			EQU	067H
MI_TWO_BYTE			EQU	0FH
MI_HLT				EQU	0F4H
MI_LTR_LLDT			EQU	0
MI_LGDT_LIDT_LMSW		EQU	01H
MI_MODRM_MASK			EQU	38H
MI_LLDT_MASK			EQU	10h
MI_LTR_MASK			EQU	18H
MI_LGDT_MASK			EQU	10H
MI_LIDT_MASK			EQU	18H
MI_LMSW_MASK			EQU	30H
MI_SPECIAL_MOV_MASK		EQU	20H
MI_REP_INS_OUTS 		EQU	0F3H
MI_MIN_INS_OUTS 		EQU	06CH
MI_MAX_INS_OUTS 		EQU	06FH

MI_CLTS                         EQU     006H
MI_INVD                         EQU     008H
MI_WBINVD                       EQU     009H

MI_MOV_FROM_CR                  EQU     020H
MI_MOV_FROM_DR                  EQU     021H
MI_MOV_TO_CR                    EQU     022H
MI_MOV_TO_DR                    EQU     023H
MI_MOV_FROM_TR                  EQU     024H
MI_MOV_TO_TR                    EQU     026H

MI_WRMSR                        EQU     030H
MI_RDTSC                        EQU     031H
MI_RDMSR                        EQU     032H
MI_RDPMC                        EQU     033H
MI_SYSEXIT                      EQU     035H

