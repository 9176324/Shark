        title  "Hard Error Support"
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
;   hrderror.asm
;
; Abstract:
;
;   This module implements code necessary to save processor context and 
;   state at hard error.
;
;--

include ksamd64.inc

        extern  ExpSystemErrorHandler2:proc
        extern  KiSaveProcessorControlState:proc
        extern  RtlCaptureContext:proc

        subttl  "System Error Handler"
;++
;
; VOID
; ExpSystemErrorHandler (
;     IN NTSTATUS ErrorStatus,
;     IN ULONG NumberOfParameters,
;     IN ULONG UnicodeStringParameterMask,
;     IN PULONG_PTR Parameters,
;     IN BOOLEAN CallShutdown
;     )
;
; Routine Description:
;
;   This function saves the processor context and control state at hard error. 
;
; Return Value:
;
;   None.
;
;--

EhFrame struct
        P1Home  dq ?                    ; parameter home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        P5Home  dq ?                    ;
        OldIrql dq ?                    ; saved IRQL
        Flags   dq ?                    ; saved Rflags
EhFrame ends

        NESTED_ENTRY ExpSystemErrorHandler, _TEXT$00

        mov     P1Home[rsp], rcx        ; save argument registers
        mov     P2Home[rsp], rdx        ;
        mov     P3Home[rsp], r8         ;
        mov     P4Home[rsp], r9         ;

        push_eflags                     ; push processor flags
        alloc_stack (sizeof EhFrame - 8); allocate stack frame

        END_PROLOGUE

;
; Capture processor context and control state.
;

        mov     EhFrame.OldIrql[rsp], rax ; keep a copy of rax 
        mov     rcx, DISPATCH_LEVEL     ; raise to DISPATCH_LEVEL

        RaiseIrql                       ; raise IRQL

        xchg    EhFrame.OldIrql[rsp], rax ; save previous IRQL and restore rax
        mov     rcx, gs:[PcCurrentPrcb] ; get current PRCB address
        add     rcx, PbProcessorState + PsContextFrame ; set context address
        call    RtlCaptureContext       ; capture processor context
        mov     rcx, gs:[PcCurrentPrcb] ; get current PRCB address
        add     rcx, PbProcessorState   ; set address of processor state
        call    KiSaveProcessorControlState; save processor control state     

;
; Update captured context to the state at the entry of this function.
;

        mov     r10, gs:[PcCurrentPrcb] ; get current PRCB address
        add     r10, PbProcessorState   ; point to processor state
        mov     rax, EhFrame.OldIrql[rsp] ; get saved cr8 
        mov     PsCr8[r10], rax         ; update cr8 in processor state
        add     r10, PsContextFrame     ; point to context frame
        mov     rax, P1Home + sizeof EhFrame[rsp] ; get saved rcx
        mov     CxRcx[r10], rax         ; update rcx in context frame
        mov     rax, EhFrame.Flags[rsp] ; get saved flags
        mov     CxEFlags[r10], rax      ; update rflag in context frame
        lea     rax, sizeof EhFrame[rsp]; calculate rsp at function entry
        mov     CxRsp[r10], rax;        ; update rsp in context frame
        lea     rax, ExpSystemErrorHandler ; get the Rip at function entry
        mov     CxRip[r10], rax         ; update rip in context frame

;
; Call the system error handler.
;
        
        mov     rcx, EhFrame.OldIrql[rsp] ; get previous IRQL

        LowerIrql                       ; lower IRQL to previous level

        mov     rax, (5 * 8) + sizeof EhFrame[rsp] ; get parameter 5
        mov     EhFrame.P5Home[rsp], rax; set parameter 5
        mov     r9,  P4Home + sizeof EhFrame[rsp] ; restore parameter 4
        mov     r8,  P3Home + sizeof EhFrame[rsp] ; restore parameter 3
        mov     rdx, P2Home + sizeof EhFrame[rsp] ; restore parameter 2
        mov     rcx, P1Home + sizeof EhFrame[rsp] ; restore parameter 1
        call    ExpSystemErrorHandler2  ; call the error handler
        add     rsp, (sizeof EhFrame)   ; deallocate stack frame
        ret                             ; return

        NESTED_END ExpSystemErrorHandler, _TEXT$00

        end

