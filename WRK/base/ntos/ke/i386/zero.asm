        title  "Zero memory pages using fastest means available"
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
;    zero.asm
;
; Abstract:
;
;    Zero memory pages using the fastest means available.
;
;--

.386p
        .xlist
include ks386.inc
include callconv.inc
include mac386.inc
include irqli386.inc

        .list

        EXTRNP  HalRequestSoftwareInterrupt,1,IMPORT,FASTCALL

;
; Register Definitions (for instruction macros).
;

rEAX            equ     0
rECX            equ     1
rEDX            equ     2
rEBX            equ     3
rESP            equ     4
rEBP            equ     5
rESI            equ     6
rEDI            equ     7

;
; Define SIMD instructions used in this module.
;

xorps           macro   XMMReg1, XMMReg2
                db      0FH, 057H, 0C0H + (XMMReg1 * 8) + XMMReg2
                endm

movntps         macro   GeneralReg, Offset, XMMReg
                db      0FH, 02BH, 040H + (XmmReg * 8) + GeneralReg, Offset
                endm

sfence          macro
                db      0FH, 0AEH, 0F8H
                endm

movaps_load     macro   XMMReg, GeneralReg
                db      0FH, 028H, (XMMReg * 8) + 4, (4 * 8) + GeneralReg
                endm

movaps_store    macro   GeneralReg, XMMReg
                db      0FH, 029H, (XMMReg * 8) + 4, (4 * 8) + GeneralReg
                endm


;
; NPX Save and Restore
;

fxsave          macro   Register
                db      0FH, 0AEH, Register
                endm

fxrstor         macro   Register
                db      0FH, 0AEH, 8+Register
                endm


_TEXT   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

;++
;
; VOID
; KeZeroPages (
;     IN PVOID PageBase,
;     IN SIZE_T NumberOfBytes
;    )
;
; Routine Description:
;
;     KeZeroPages is really just a function pointer that points at
;     either KiZeroPages or KiXMMIZeroPages depending on whether or
;     not XMMI instructions are available.
;
; Arguments:
;
;     (ecx) PageBase    Base address of pages to be zeroed.
;
;     (edx) NumberOfBytes    Number of bytes to be zeroed.  Always a PAGE_SIZE multiple.
;
;
; Return Value:
;
;--


        page    ,132
        subttl  "KiXMMIZeroPagesNoSave - Use XMMI to zero memory (XMMI owned)"

;++
;
; VOID
; KiXMMIZeroPagesNoSave (
;     IN PVOID PageBase,
;     IN SIZE_T NumberOfBytes
;     )
;
; Routine Description:
;
;     Use XMMI to zero a page of memory 16 bytes at a time while
;     at the same time minimizing cache pollution.
;
;     Note: The XMMI register set belongs to this thread.  It is neither
;     saved nor restored by this procedure.
;
; Arguments:
;
;     (ecx) PageBase    Virtual address of the base of the page to be zeroed.
;
;     (edx) NumberOfBytes    Number of bytes to be zeroed.  Always a PAGE_SIZE multiple.
;
; Return Value:
;
;     None.
;
;--

INNER_LOOP_BYTES    equ 64
INNER_LOOP_SHIFT    equ 6

cPublicFastCall KiXMMIZeroPagesNoSave,2
cPublicFpo 0, 1

        xorps   0, 0                            ; zero xmm0 (128 bits)
        shr     edx, INNER_LOOP_SHIFT           ; Number of Iterations

inner:

        movntps rECX, 0,  0                     ; store bytes  0 - 15
        movntps rECX, 16, 0                     ;             16 - 31
        movntps rECX, 32, 0                     ;             32 - 47
        movntps rECX, 48, 0                     ;             48 - 63

        add     ecx, 64                         ; increment base
        dec     edx                             ; decrement loop count
        jnz     short inner

        ; Force all stores to complete before any other
        ; stores from this processor.

        sfence

ifndef SFENCE_IS_NOT_BUSTED

        ; ERRATA the next uncached write to this processor's APIC 
        ; may fail unless the store pipes have drained.  sfence by
        ; itself is not enough.   Force drainage now by doing an
        ; interlocked exchange.

        xchg    [esp-4], edx

endif

        fstRET  KiXMMIZeroPagesNoSave

fstENDP KiXMMIZeroPagesNoSave


        page    ,132
        subttl  "KiXMMIZeroPages - Use XMMI to zero memory"

;++
;
; VOID
; KiXMMIZeroPages (
;     IN PVOID PageBase,
;     IN SIZE_T NumberOfBytes
;     )
;
; Routine Description:
;
;     Use XMMI to zero a page of memory 16 bytes at a time.  This
;     routine is a wrapper around KiXMMIZeroPagesNoSave.  In this
;     case we don't have the luxury of not saving/restoring context.
;
; Arguments:
;
;     (ecx) PageBase    Virtual address of the base of the page to be zeroed.
;
;     (edx) NumberOfBytes    Number of bytes to be zeroed.  Always a PAGE_SIZE multiple.
;
; Return Value:
;
;     None.
;
;--

cPublicFastCall KiXMMIZeroPages,2
cPublicFpo 0, 2

        push    ebp
        push    ebx
        mov     ebx, PCR[PcPrcbData+PbCurrentThread]
        mov     eax, [ebx].ThInitialStack
        sub     eax, NPX_FRAME_LENGTH
        mov     ebp, esp                        ; save stack pointer
        sub     esp, 16                         ; reserve space for xmm0
        and     esp, 0FFFFFFF0H                 ; 16 byte aligned
        cli                                     ; don't context switch
        test    [eax].FpCr0NpxState, CR0_EM     ; if FP explicitly disabled
        jnz     short kxzp90                    ; do it the old way
        dec     word ptr [ebx].ThSpecialApcDisable ; Disable APCs now we know we are zeroing with SSE
        cmp     byte ptr [ebx].ThNpxState, NPX_STATE_LOADED
        je      short kxzp80                    ; jiff, NPX stated loaded

        ; NPX state is not loaded on this thread, it will be by
        ; the time we reenable context switching.

        mov     byte ptr [ebx].ThNpxState, NPX_STATE_LOADED

        ; enable use of FP instructions

        mov     ebx, cr0
        and     ebx, NOT (CR0_MP+CR0_TS+CR0_EM)
        mov     cr0, ebx                        ; enable NPX

ifdef NT_UP

        ; if this is a UP machine, the state might be loaded for
        ; another thread in which case it needs to be saved.

        mov     ebx, PCR[PcPrcbData+PbNpxThread]; Owner of NPX state
        or      ebx, ebx                        ; NULL?
        jz      short @f                        ; yes, skip save.

        mov     byte ptr [ebx].ThNpxState, NPX_STATE_NOT_LOADED
        mov     ebx, [ebx].ThInitialStack       ; get address of save
        sub     ebx, NPX_FRAME_LENGTH           ; area.
        fxsave  rEBX                            ; save NPX
@@:

endif

        ; Now load the NPX context for this thread.  This is because
        ; if we switch away from this thread it will get saved again
        ; in this save area and destroying it would be bad.

        fxrstor rEAX

        mov     eax, PCR[PcPrcbData+PbCurrentThread]
        mov     PCR[PcPrcbData+PbNpxThread], eax

kxzp80:
        sti                                     ; reenable context switching
        movaps_store rESP, 0                    ; save xmm0
        fstCall KiXMMIZeroPagesNoSave           ; zero the page
        movaps_load  0, rESP                    ; restore xmm

        mov     eax, PCR[PcPrcbData+PbCurrentThread]
        inc     word ptr [eax].ThSpecialApcDisable
        jne     @f
        lea     eax, [eax].ThApcState.AsApcListHead
        cmp     eax, [eax]
        je      @f
        mov     cl, APC_LEVEL           ; request software interrupt level
        fstCall HalRequestSoftwareInterrupt ;
@@:
        ; restore stack pointer, non-volatiles and return

        mov     esp, ebp
        pop     ebx
        pop     ebp
        fstRET  KiXMMIZeroPages


        ; FP is explicitly disabled for this thread (probably a VDM
        ; thread).  Restore stack pointer, non-volatiles and jump into
        ; KiZeroPage to do the work the old fashioned way.

kxzp90:
        sti
        mov     esp, ebp
        pop     ebx
        pop     ebp
        jmp     short @KiZeroPages@8

fstENDP KiXMMIZeroPages


        page    ,132
        subttl  "KiZeroPages - Available to all X86 processors"

;++
;
; KiZeroPages(
;     PVOID PageBase,
;     IN SIZE_T NumberOfBytes
;     )
;
; Routine Description:
;
;     Generic Zero Page routine, used on processors that don't have
;     a more efficient way to zero large blocks of memory.
;     (Same as RtlZeroMemory).
;
; Arguments:
;
;     (ecx) PageBase    Base address of page to be zeroed.
;
;     (edx) NumberOfBytes    Number of bytes to be zeroed.  Always a PAGE_SIZE multiple.
;
; Return Value:
;
;     None.
;
;--

cPublicFastCall KiZeroPages,2
cPublicFpo 0, 0

        push    edi                             ; save EDI (non-volatile)
        xor     eax, eax                        ; 32 bit zero
        mov     edi, ecx                        ; setup for repsto
        mov     ecx, edx                        ; number of bytes
        shr     ecx, 2                          ; iteration count

        ; store eax, ecx times starting at edi

        rep stosd

        pop     edi                             ; restore edi and return
        fstRET  KiZeroPages

fstENDP KiZeroPages


_TEXT   ends
        end

