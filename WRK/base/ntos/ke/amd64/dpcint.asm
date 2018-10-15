        title  "Deferred Procedure Call Interrupt"
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
;   dpcint.asm
;
; Abstract:
;
;   This module implements the code necessary to process the Deferred
;   Procedure Call interrupt.
;
;--

include ksamd64.inc

        extern  KiDispatchInterrupt:proc
        extern  KiIdleSummary:qword
        extern  KiInitiateUserApc:proc
        extern  KiRestoreDebugRegisterState:proc
        extern  KiSaveDebugRegisterState:proc
        extern  __imp_HalRequestSoftwareInterrupt:qword

        subttl  "Deferred Procedure Call Interrupt"
;++
;
; VOID
; KiDpcInterrupt (
;     VOID
;     )
;
; Routine Description:
;
;   This routine is entered as the result of a software interrupt generated
;   at DISPATCH_LEVEL. Its function is to save the machine state and call
;   the dispatch interrupt routine.
;
;   N.B. This is a directly connected interrupt that does not use an interrupt
;        object.
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

        NESTED_ENTRY KiDpcInterrupt, _TEXT$00

        .pushframe                      ; mark machine frame

;
; Check for interrupt from the idle halt state.
;
; N.B. If an DPC interrupt occurs when idle halt is set, then the interrupt
;      occurred during the power managment halted state. The interrupt can
;      be immediately dismissed since the idle loop will provide the correct
;      processing.
;

        test    byte ptr MfSegCs[rsp], MODE_MASK ; test if previous mode user
        jnz     short KiDP10            ; if nz, previous mode is user
        cmp     byte ptr gs:[PcIdleHalt], 0 ; check for idle halt interrupt
        je      short KiDP10            ; if e, not interrupt from idle halt

        EndSystemInterrupt              ; perform EOI

        iretq                           ; return

;
; Normal DPC interrupt.
;

KiDP10: alloc_stack 8                   ; allocate dummy vector
        push_reg rbp                    ; save nonvolatile register

        GENERATE_INTERRUPT_FRAME <>, <DirectNoSListCheck> ; generate interrupt frame

        mov     ecx, DISPATCH_LEVEL     ; set new IRQL level

	ENTER_INTERRUPT	<>, <NoCount>   ; raise IRQL, do EOI, enable interrupts

        call    KiDispatchInterrupt     ; process the dispatch interrupt

        EXIT_INTERRUPT <NoEOI>, <NoCount>, <Direct> ; lower IRQL and restore state

        NESTED_END KiDpcInterrupt, _TEXT$00

        subttl  "Deferred Procedure Call Interrupt Bypass"
;++
;
; VOID
; KiDpcInterruptBypass (
;     VOID
;     )
;
; Routine Description:
;
;   This routine is entered as the result of a bypassed software interrupt at
;   dispatch level. Its function is to set the current IRQL to DISPATCH_LEVEL
;   and call the dispatch interrupt routine.
;
;   N.B. This function is entered with interrupts disabled and returns with
;        interrupts disabled.
;
; Arguments:
;
;   None.
;
; Implicit Arguments:
;
;   rbp - Supplies the address of the trap frame.
;
; Return Value:
;
;   None.
;
;--

        NESTED_ENTRY KiDpcInterruptBypass, _TEXT$00

        alloc_stack 8                   ; allocate stack frame

        END_PROLOGUE

        mov     ecx, DISPATCH_LEVEL     ; set new IRQL level

        SetIrql                         ;

        sti                             ; enable interrupts

        call    KiDispatchInterrupt     ; process the dispatch interrupt

        cli                             ; disable interrupts
        add     rsp, 8                  ; deallocate stack frame
        ret                             ; return

        NESTED_END KiDpcInterruptBypass, _TEXT$00

        end

