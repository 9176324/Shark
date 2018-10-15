        title  "Asynchronous Procedure Call Interrupt"
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
;   apcint.asm
;
; Abstract:
;
;   This module implements the code necessary to process the  Asynchronous
;   Procedure Call interrupt request.
;
;--

include ksamd64.inc

        extern  KiDeliverApc:proc
        extern  KiIdleSummary:qword
        extern  KiRestoreDebugRegisterState:proc
        extern  KiSaveDebugRegisterState:proc

        subttl  "Asynchronous Procedure Call Interrupt"
;++
;
; VOID
; KiApcInterrupt (
;     VOID
;     )
;
; Routine Description:
;
;   This routine is entered as the result of a software interrupt generated
;   at APC_LEVEL. Its function is to save the machine state and call the APC
;   delivery routine.
;
;   N.B. This is a directly connected interrupt that does not use an interrupt
;        object.
;
;   N.B. APC interrupts are never requested for user mode APCs.
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

        NESTED_ENTRY KiApcInterrupt, _TEXT$00

        .pushframe                      ; mark machine frame

;
; Check for interrupt from the idle halt state.
;
; N.B. If an APC interrupt occurs when idle halt is set, then the interrupt
;      occurred during the power managment halted state. The interrupt can
;      be immediately dismissed since the idle loop will provide the correct
;      processing.
;

        test    byte ptr MfSegCs[rsp], MODE_MASK ; test if previous mode user
        jnz     short KiAP10            ; if nz, previous mode is user
        cmp     byte ptr gs:[PcIdleHalt], 0 ; check for idle halt interrupt
        je      short KiAP10            ; if e, not interrupt from halt

        EndSystemInterrupt              ; Perform EOI

        iretq                           ; return

;
; Normal APC interrupt.
;

KiAP10: alloc_stack 8                   ; allocate dummy vector
        push_reg rbp                    ; save nonvolatile register

        GENERATE_INTERRUPT_FRAME <>, <DirectNoSlistCheck> ; generate interrupt frame

        mov     ecx, APC_LEVEL          ; set new IRQL level

	ENTER_INTERRUPT	<>, <NoCount>   ; raise IRQL, do EOI, enable interrupts

        mov     ecx, KernelMode         ; set APC processor mode
        xor     edx, edx                ; set exception frame address
        lea     r8, (-128)[rbp]         ; set trap frame address
        call    KiDeliverApc            ; initiate APC execution

        EXIT_INTERRUPT <NoEOI>, <NoCount>, <Direct> ; lower IRQL and restore state

        NESTED_END KiApcInterrupt, _TEXT$00

        subttl  "Initiate User APC Execution"
;++
;
; Routine Description:
;
;   This routine generates an exception frame and attempts to deliver a user
;   APC.
;
; Arguments:
;
;   rbp - Supplies the address of the trap frame.
;
;   rsp - Supplies the address of the trap frame.
;
; Return value:
;
;   None.
;
;--

        NESTED_ENTRY KiInitiateUserApc, _TEXT$00

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

        mov     ecx, UserMode           ; set APC processor mode
        mov     rdx, rsp                ; set exception frame address
        lea     r8, (-128)[rbp]         ; set trap frame address
        call    KiDeliverApc            ; deliver APC

        RESTORE_EXCEPTION_STATE         ; restore exception state/deallocate

        ret                             ; return

        NESTED_END KiInitiateUserApc, _TEXT$00

        end

