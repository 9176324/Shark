        title  "Interprocessor Interrupts"
;++
;
; Copyright (c) Microsoft Corporation. All rights reserved. 
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
; Module Name:
;
;   intipi.asm
;
; Abstract:
;
;   This module implements the code necessary to process interprocessor
;   interrupt requests.
;
;--

include ksamd64.inc

        extern  ExpInterlockedPopEntrySListEnd:proc
        extern  ExpInterlockedPopEntrySListResume:proc
        extern  KiCheckForSListAddress:proc
        extern  KiDpcInterruptBypass:proc
        extern  KiInitiateUserApc:proc
        extern  KiIdleSummary:qword
        extern  KiIpiProcessRequests:proc
        extern  KiRestoreDebugRegisterState:proc
        extern  KiSaveDebugRegisterState:proc
        extern  __imp_HalRequestSoftwareInterrupt:qword

        subttl  "Interprocess Interrupt Service Routine"
;++
;
; VOID
; KiIpiInterrupt (
;     VOID
;     )
;
; Routine Description:
;
;   This routine is entered as the result of an interprocessor interrupt at
;   IPI level. Its function is to process all interprocessor requests.
;
; Arguments:
;
;   None.
;
; Return Value:
;
;   None.
;
;--

        NESTED_ENTRY KiIpiInterrupt, _TEXT$00

        .pushframe                      ; mark machine frame

        alloc_stack 8                   ; allocate dummy vector
        push_reg rbp                    ; save nonvolatile register

        GENERATE_INTERRUPT_FRAME <>, <Direct> ; generate interrupt frame

        mov     ecx, IPI_LEVEL          ; set new IRQL level

	ENTER_INTERRUPT <NoEoi>         ; raise IRQL and enable interrupts

;
; Process all interprocessor requests.
;

        call    KiIpiProcessRequests    ; process interprocessor requests

        EXIT_INTERRUPT <>, <>, <Direct> ; do EOI, lower IRQL and restore state

        NESTED_END KiIpiInterrupt, _TEXT$00

        end

