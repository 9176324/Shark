        title  "Processor Type and Stepping Detection"
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
;    psctxwrap.asm
;
; Abstract:
;
;    This module implements the code necessary to wrap the get/set thread
;    context functions so register state is correctly saved and restored.
;
;--

include ksamd64.inc

        extern  KeSetEvent:Proc
        extern  PspGetSetContextInternal:proc

;++
;
; VOID
; PspGetSetContextSpecialApc (
;     IN PKAPC Apc,
;     IN OUT PKNORMAL_ROUTINE *NormalRoutine,
;     IN OUT PVOID *NormalContext,
;     IN OUT PVOID *SystemArgument1,
;     IN OUT PVOID *SystemArgument2
;     );
;
; Routine Description:
;
;    This function saves and restore non-volatile state for the get/set
;    thread context functions.
;
; Arguments:
;
;    
;   rcx - Supplies a pointer to an APC object.
;
;   rdx - Supplies a pointer to the normal APC routine (not used).
;
;   r8 - Supplies a pointer to the normal Context (not used).
;
;   r9 - Supplies a pointer to the operation type.
;
;   40[rsp] - Supplies a pointer to the current thread (not used). 
;
; Return Value:
;
;    None.
;
;--

SaFrame struct
        P1Home  dq ?                    ; parameter home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        SavedXmm6 db 16 dup (?)         ; saved nonvolatile floating registers
        SavedXmm7 db 16 dup (?)         ;
        SavedXmm8 db 16 dup (?)         ;
        SavedXmm9 db 16 dup (?)         ;
        SavedXmm10 db 16 dup (?)        ;
        SavedXmm11 db 16 dup (?)        ;
        SavedXmm12 db 16 dup (?)        ;
        SavedXmm13 db 16 dup (?)        ;
        SavedXmm14 db 16 dup (?)        ;
        SavedXmm15 db 16 dup (?)        ;
        SavedRbx dq ?                   ; saved nonvolatile integer registers
        SavedRbp dq ?                   ;
        SavedRsi dq ?                   ;
        SavedRdi dq ?                   ;
        SavedR12 dq ?                   ;
        SavedR13 dq ?                   ;
        SavedR14 dq ?                   ;
        SavedR15 dq ?                   ;
        Event    dq ?                   ; address of event to set
SaFrame ends

        NESTED_ENTRY PspGetSetContextSpecialApc, _TEXT$00

        alloc_stack (sizeof SaFrame)    ; allocate stack frame
        save_reg rbx, SaFrame.SavedRbx  ; save nonvolatile integer registers
        save_reg rbp, SaFrame.SavedRbp  ;
        save_reg rsi, SaFrame.SavedRsi  ;
        save_reg rdi, SaFrame.SavedRdi  ;
        save_reg r12, SaFrame.SavedR12  ;
        save_reg r13, SaFrame.SavedR13  ;
        save_reg r14, SaFrame.SavedR14  ;
        save_reg r15, SaFrame.SavedR15  ;
        save_xmm128 xmm6, SaFrame.SavedXmm6 ; save nonvolatile floating registers
        save_xmm128 xmm7, SaFrame.SavedXmm7 ;
        save_xmm128 xmm8, SaFrame.SavedXmm8 ;
        save_xmm128 xmm9, SaFrame.SavedXmm9 ;
        save_xmm128 xmm10, SaFrame.SavedXmm10 ;
        save_xmm128 xmm11, SaFrame.SavedXmm11 ;
        save_xmm128 xmm12, SaFrame.SavedXmm12 ;
        save_xmm128 xmm13, SaFrame.SavedXmm13 ;
        save_xmm128 xmm14, SaFrame.SavedXmm14 ;
        save_xmm128 xmm15, SaFrame.SavedXmm15 ;

        END_PROLOGUE

        mov     rdx, [r9]               ; get operation type
        lea     r8, SaFrame.Event[rsp]  ; get address to store event address
        mov     r9, dr7                 ; access debug register
        call    PspGetSetContextInternal ; get/set thread context
        test    rax, rax                ; test for legacy state restore
        jz      short GsCS10            ; if z, do not restore legacy state

;
; N.B. The following legacy restore also restores the nonvolatile floating
;      register xmm6-xmm15 with potentially incorrect values. Fortunately,
;      these registers are restored to their proper values chortly thereafter.
;

        fxrstor [rax]                   ; restore legacy floating state
GsCS10: mov     rcx, SaFrame.Event[rsp] ; set event address
        xor     edx, edx                ; set priority increment
        xor     r8d, r8d                ; set wait next
        call    KeSetEvent              ; set completion event
        mov     rbx, SaFrame.SavedRbx[rsp] ; restore nonvolatile integer registers
        mov     rbp, SaFrame.SavedRbp[rsp] ;
        mov     rsi, SaFrame.SavedRsi[rsp] ;
        mov     rdi, SaFrame.SavedRdi[rsp] ;
        mov     r12, SaFrame.SavedR12[rsp] ;
        mov     r13, SaFrame.SavedR13[rsp] ;
        mov     r14, SaFrame.SavedR14[rsp] ;
        mov     r15, SaFrame.SavedR15[rsp] ;
        movdqa  xmm6, SaFrame.SavedXmm6[rsp] ; restore nonvolatile floating registers
        movdqa  xmm7, SaFrame.SavedXmm7[rsp] ;
        movdqa  xmm8, SaFrame.SavedXmm8[rsp] ;
        movdqa  xmm9, SaFrame.SavedXmm9[rsp] ;
        movdqa  xmm10, SaFrame.SavedXmm10[rsp] ;
        movdqa  xmm11, SaFrame.SavedXmm11[rsp] ;
        movdqa  xmm12, SaFrame.SavedXmm12[rsp] ;
        movdqa  xmm13, SaFrame.SavedXmm13[rsp] ;
        movdqa  xmm14, SaFrame.SavedXmm14[rsp] ;
        movdqa  xmm15, SaFrame.SavedXmm15[rsp] ;
        add     rsp, (sizeof SaFrame)   ; deallocate stack frame
        ret                             ; return

        NESTED_END PspGetSetContextSpecialApc, _TEXT$00

        end

