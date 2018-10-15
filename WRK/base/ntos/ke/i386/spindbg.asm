        TITLE   "Spin Locks"
;++
;
; Copyright (c) Microsoft Corporation. All rights reserved. 
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
;  Module Name:
;
;     spindbg.asm
;
;  Abstract:
;
;--

        PAGE

.386p

include ks386.inc
include callconv.inc                    ; calling convention macros
include mac386.inc
include irqli386.inc


if DBG
        EXTRNP  _KeBugCheckEx,5
ifdef DBGMP
        EXTRNP  _KiPollDebugger,0
endif
        extrn   _KeTickCount:DWORD
        extrn   _KiSpinlockTimeout:DWORD
endif


_TEXT$00   SEGMENT  DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

;++
;
;  VOID
;  Kii386SpinOnSpinLock (
;     IN PKSPIN_LOCK SpinLock
;     IN ULONG       Flag
;     )
;
;  Routine Description:
;
;     This function is called on a debug build to spin on a spinlock.
;     It is invoked by the DEBUG version of SPIN_ON_SPINLOCK macro.
;
;  Warning:
;
;     Not called with C calling conventions
;     Does not destroy any register
;
;--

cPublicProc Kii386SpinOnSpinLock,2

if DBG
cPublicFpo 2,2
        push    eax
        push    ebx

        mov     eax, [esp+12]           ; (eax) = LockAddress

        mov     ebx, PCR[PcPrcbData.PbCurrentThread]
        or      ebx, 1                  ; or on busy bit
        cmp     ebx, [eax]              ; current thread the owner?
        je      short ssl_sameid        ; Yes, go abort

ssl_10:
        mov     ebx, _KeTickCount       ; Current time
        add     ebx, _KiSpinlockTimeout ; wait n ticks

ifdef DBGMP
        test    byte ptr [esp+16], 2    ; poll debugger while waiting?
        jnz     short ssl_30
endif

;
; Spin while watching KeTickCount
;

ssl_20: YIELD
        cmp     _KeTickCount, ebx       ; check current time
        jnc     short ssl_timeout       ; NC, too many ticks have gone by

        test    dword ptr [eax], 1
        jnz     short ssl_20

ssl_exit:
        pop     ebx                     ; Spinlock is not busy, return
        pop     eax
        stdRET  Kii386SpinOnSpinLock

ifdef DBGMP
;
; Spin while watching KeTickCount & poll debugger
;

ssl_30: YIELD
        cmp     _KeTickCount, ebx       ; check current time
        jnc     short ssl_timeout       ; overflowed

        stdCall _KiPollDebugger

        test    dword ptr [eax], 1
        jnz     short ssl_30

        pop     ebx                     ; Spinlock is not busy, return
        pop     eax
        stdRET  Kii386SpinOnSpinLock
endif

;
; Out of line exception conditions
;

ssl_sameid:
        test    byte ptr [esp+16], 1    ; ID check enabled?
        jz      short ssl_10            ; no, continue
        push    eax
        CurrentIrql                     ; Get current IRQL
        cmp     al, SYNCH_LEVEL         ; Above synch_level we may seen the same
        pop     eax                     ;  thread on two different processors
        jg      ssl_10

        ; recursed on lock, abort

        stdCall _KeBugCheckEx,<SPIN_LOCK_ALREADY_OWNED,eax,0,0,0>

ssl_timeout:
        test    byte ptr [esp+16], 4    ; Timeout check enabled?
        jz      short ssl_10            ; no, continue

        CurrentIrql                     ; Check to see what level we're spinning at
        cmp     al, DISPATCH_LEVEL
        mov     eax, [esp+12]           ; restore eax
        jc      short ssl_10            ; if < dispatch_level, don't timeout

        test    dword ptr [eax], 1      ; Check to see if spinlock was freed
        jz      short ssl_exit

        public SpinLockSpinningForTooLong
SpinLockSpinningForTooLong:

        int 3                           ; Stop here
        jmp     short ssl_10            ; re-wait

else    ; DBG
        stdRET  Kii386SpinOnSpinLock
endif
stdENDP Kii386SpinOnSpinLock,2

_TEXT$00   ends

        end

