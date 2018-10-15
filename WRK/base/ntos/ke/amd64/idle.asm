        title  "Idle Loop"
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
;   idle.asm
;
; Abstract:
;
;   This module implements the platform specified idle loop.
;
;--

include ksamd64.inc

        extern  KeAcquireQueuedSpinLockAtDpcLevel:proc
        extern  KeAcquireQueuedSpinLockRaiseToSynch:proc
        extern  KeReleaseQueuedSpinLock:proc
        extern  KeReleaseQueuedSpinLockFromDpcLevel:proc

ifndef NT_UP

        extern  KiIdleSchedule:proc

endif

        extern  KiRetireDpcList:proc
        extern  SwapContext:proc

        subttl  "Idle Loop"
;++
; VOID
; KiIdleLoop (
;     VOID
;     )
;
; Routine Description:
;
;    This routine continuously executes the idle loop and never returns.
;
; Arguments:
;
;    None.
;
; Return value:
;
;    This routine never returns.
;
;--

IlFrame struct
        P1Home  dq ?                    ;
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        Fill    dq ?                    ; fill to 8 mod 16
IlFrame ends

        NESTED_ENTRY KiIdleLoop, _TEXT$00

        alloc_stack (sizeof IlFrame)    ; allocate stack frame

        END_PROLOGUE

        mov     rbx, gs:[PcCurrentPrcb] ; get current processor block address
        jmp     short KiIL20            ; skip idle processor on first iteration

;
; There are no entries in the DPC list and a thread has not been selected
; for execution on this processor. Call the HAL so power managment can be
; performed.
;
; N.B. The HAL is called with interrupts disabled. The HAL will return
;      with interrupts enabled.
;

KiIL10: xor     ecx, ecx                ; lower IRQL to passive level

        SetIrql                         ;

        lea     rcx, PbPowerState[rbx]  ; set address of power state
        call    qword ptr PpIdleFunction[rcx] ; call idle function
        sti                             ; enable interrupts/avoid spurious interrupt
        mov     ecx, DISPATCH_LEVEL     ; set IRQL to dispatch level

        SetIrql                         ;

        and     byte ptr PbIdleHalt[rbx], 0 ; clear idle halt

KiIL20:                                 ; reference label

;
; Disable interrupts and check if there is any work in the DPC list of the
; current processor or a target processor.
;
; N.B. The following code enables interrupts for a few cycles, then disables
;      them again for the subsequent DPC and next thread checks.
;

CheckDpcList:                           ; reference label

        Yield                           ; yield processor execution

        sti                             ; enable interrupts
        nop                             ;
        nop                             ;
        cli                             ; disable interrupts

;
; Process the deferred procedure call list for the current processor.
;

        mov     eax, PbDpcQueueDepth[rbx] ; get DPC queue depth
        or      rax, PbTimerRequest[rbx] ; merge timer request value

ifndef NT_UP

        or      rax, PbDeferredReadyListHead[rbx] ; merge ready list head

endif

        jz      short CheckNextThread   ; if z, no DPCs to process
        mov     rcx, rbx                ; set current PRCB address
        call    KiRetireDpcList         ; process the current DPC list

;
; Check if a thread has been selected to run on the current processor.
;
; N.B. The variable idle halt is only written on the owning processor.
;      It is only read on other processors. This variable is set when
;      the respective processor may enter a sleep state. The following
;      code sets the variable under interlocked which provides a memory
;      barrier, then checks to determine if a thread has been schedule.
;      Code elsewhere in the system that reads this variable, set next
;      thread, executes a memory barrier, then reads the variable.
;

CheckNextThread:                        ;

   lock or      byte ptr PbIdleHalt[rbx], 1 ; set idle halt
        cmp     qword ptr PbNextThread[rbx], 0 ; check if thread selected

ifdef NT_UP

        je      KiIL10                  ; if e, no thread selected

else

        je      KiIL50                  ; if e, no thread selected

endif

        and     byte ptr PbIdleHalt[rbx], 0 ; clear idle halt
        sti                             ; enable interrupts

        mov     ecx, SYNCH_LEVEL        ; set IRQL to synchronization level

        RaiseIrql                       ;

;
; set context swap busy for the idle thread and acquire the PRCB Lock.
;

        mov     rdi, PbIdleThread[rbx]  ; get idle thread address

ifndef NT_UP

        mov     byte ptr ThSwapBusy[rdi], 1 ; set context swap busy

        AcquireSpinLock PbPrcbLock[rbx] ; acquire current PRCB Lock

endif

        mov     rsi, PbNextThread[rbx]  ; set next thread address

;
; If a thread had been scheduled for this processor, but was removed from
; eligibility (e.g., an affinity change), then the new thread could be the
; idle thread.
;

ifndef NT_UP

        cmp     rsi, rdi                ; check if swap from idle to idle
        je      short KiIL40            ; if eq, idle to idle

endif

        and     qword ptr PbNextThread[rbx], 0 ; clear next thread address
        mov     PbCurrentThread[rbx], rsi ; set current thread address
        mov     byte ptr ThState[rsi], Running ; set new thread state

;
; Clear idle schedule since a new thread has been selected for execution on
; this processor and release the PRCB lock.
;

ifndef NT_UP

        and     byte ptr PbIdleSchedule[rbx], 0 ; clear idle schedule
        and     qword ptr PbPrcbLock[rbx], 0 ; release current PRCB lock

endif

;
; Switch context to new thread.
;

KiIL30: mov     ecx, APC_LEVEL          ; set APC bypass disable
        call    SwapContext             ; swap context to next thread

ifndef NT_UP

        mov     ecx, DISPATCH_LEVEL     ; set IRQL to dispatch level

        SetIrql                         ;

endif

        jmp     KiIL20                  ; loop

;
; The new thread is the Idle thread (same as old thread).  This can happen
; rarely when a thread scheduled for this processor is made unable to run
; on this processor. As this processor has again been marked idle, other
; processors may unconditionally assign new threads to this processor.
;

ifndef NT_UP

KiIL40: and     qword ptr PbNextThread[rbx], 0 ; clear next thread
        and     qword ptr PbPrcbLock[rbx], 0 ; release current PRCB lock
        and     byte ptr ThSwapBusy[rdi], 0 ; set context swap idle 
        jmp     KiIL20                  ;

;
; Call idle schedule if requested.
;

KiIL50: cmp     byte ptr PbIdleSchedule[rbx], 0 ; check if idle schedule
        je      KiIL10                  ; if e, idle schedule not requested
        and     byte ptr PbIdleHalt[rbx], 0 ; clear idle halt
        sti                             ; enable interrupts
        mov     rcx, rbx                ; pass current PRCB address
        call    KiIdleSchedule          ; attempt to schedule thread
        test    rax, rax                ; test if new thread schedule
        mov     rsi, rax                ; set new thread address
        mov     rdi, PbIdleThread[rbx]  ; get idle thread address
        jnz     KiIL30                  ; if nz, new thread scheduled
        jmp     KiIL20                  ;

endif

        NESTED_END KiIdleLoop, _TEXT$00

        end

