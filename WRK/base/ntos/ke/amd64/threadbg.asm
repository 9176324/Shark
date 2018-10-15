        title  "Thread Startup"
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
;    threadbg.asm
;
; Abstract:
;
;    This module implements the code necessary to startup a thread in kernel
;    mode.
;
; Environment:
;
;    IRQL APC_LEVEL.
;
;--

include ksamd64.inc

        altentry KiStartSystemThread
        altentry KiStartUserThread
        altentry KiStartUserThreadReturn

        extern  KeBugCheck:proc
        extern  KiExceptionExit:proc
        extern  KiSaveDebugRegisterState:proc

        subttl  "System Thread Startup"
;++
;
; Routine Description:
;
;   This routine is called to start a system thread. This function calls the
;   initial thread procedure after having extracted the startup parameters
;   from the specified start frame. If control returns from the initial
;   thread procedure, then a bugcheck will occur.
;
; Implicit Arguments:
;
;   N.B. This function begins execution at its alternate entry point with
;        a start frame on the stack. This frame contains the start context,
;        the start routine, and the system routine.
;
; Return Value:
;
;    None - no return.
;
;--

        NESTED_ENTRY KxStartSystemThread, _TEXT$00

        .allocstack (KSTART_FRAME_LENGTH - 8) ; allocate stack frame

        END_PROLOGUE

        ALTERNATE_ENTRY KiStartSystemThread

        mov     ecx, APC_LEVEL          ; set IRQL to APC level

        SetIrql                         ; 

        mov     rdx, SfP1Home[rsp]      ; get startup context parameter
        mov     rcx, SfP2Home[rsp]      ; get startup routine address
        call    qword ptr SfP3Home[rsp] ; call system routine
        mov     rcx, NO_USER_MODE_CONTEXT ; set bugcheck parameter
        call    KeBugCheck              ; call bugcheck - no return
        nop                             ; do not remove

        NESTED_END KxStartSystemThread, _TEXT$00

        subttl  "User Thread Startup"
;++
;
; Routine Description:
;
;   This routine is called to start a user thread. This function calls the
;   initial thread procedure after having extracted the startup parameters
;   from the specified exception frame. If control returns from the initial
;   thread routine, then the user mode context is restored and control is
;   transferred to the exception exit code.
;
; Implicit Arguments:
;
;   N.B. This function begins execution with a trap frame and an exception
;        frame on the stack that represents the user mode context. The start
;        context, start routine, and the system routine parameters are stored
;        in the exception record.
;
; Return Value:
;
;    None.
;
;--

        NESTED_ENTRY KyStartUserThread, _TEXT$00

        GENERATE_TRAP_FRAME             ; generate trap frame

        call    KxStartUserThread       ; call dummy startup routine

        ALTERNATE_ENTRY KiStartUserThreadReturn

        nop                             ; do not remove

        NESTED_END KyStartUserThread, _TEXT$00


        NESTED_ENTRY KxStartUserThread, _TEXT$00

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

        ALTERNATE_ENTRY KiStartUserThread

        mov     ecx, APC_LEVEL          ; set IRQL to APC level

        SetIrql                         ; 

        mov     rdx, ExP1Home[rsp]      ; get startup context parameter
        mov     rcx, ExP2Home[rsp]      ; get startup  routine address
        call    qword ptr ExP3Home[rsp] ; call system routine

;
; N.B. The below code uses an unusual sequence to transfer control. This
;      instruction sequence is required to avoid detection as an epilogue.
;

        lea     rcx, KiExceptionExit    ; get address of exception exit
        jmp     rcx                     ; finish in common code

        NESTED_END KxStartUserThread, _TEXT$00

        end

