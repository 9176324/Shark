        TITLE   "Compute Timer Table Index"
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
;    timindex.asm
;
; Abstract:
;
;    This module implements the code necessary to compute the timer table
;    index for a timer.
;
;--

.386p
        .xlist
include ks386.inc
include callconv.inc            ; calling convention macros
        .list

        extrn  _KiTimeIncrementReciprocal:dword
        extrn  _KiTimeIncrementShiftCount:BYTE

_TEXT$00   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

        subttl  "Compute 64-bit Timer Table Index"
;++
;
; ULONG
; KiComputeTimerTableIndex (
;    IN ULONG64 DueTime
;    )
;
; Routine Description:
;
;    This function computes the timer table index for the specified due time.
;
;    The formula for the index calculation is:
;
;    Index = (Due Time / Maximum time) & (Table Size - 1)
;
;    The time increment division is performed using reciprocal multiplication.
;
; Arguments:
;
;    DueTime  - Supplies the absolute due time.
;
; Return Value:
;
;    The time table index is returned as the function value.
;
;--

DueTimeLow equ 8[esp]
DueTimeHigh equ 12[esp]

cPublicProc _KiComputeTimerTableIndex, 2

        push    ebx                     ; save nonvolatile register

;
; Compute low 32-bits of dividend times high 32-bits of divisor.
;

        mov     eax, DueTimeLow         ; get low 32-bits of dividend
        mul     [_KiTimeIncrementReciprocal + 4] ; multiply by high 32-bits of divisor
        mov     ebx, eax                ; save full 64-bit product
        mov     ecx, edx                ;

;
; Compute high 32-bits of dividend times low 32-bits of divisor.
;

        mov     eax, DueTimeHigh        ; get high 32-bits of dividend
        mul     [_KiTimeIncrementReciprocal] ; multiply by low 32-bits of divisor
        add     ebx, eax                ; compute middle 64-bits of product
        adc     ecx, edx                ;

;
; Compute low 32-bits of dividend times low 32-bits of divisor.
;

        mov     eax, DueTimeLow         ; get low 32-bits of dividend
        mul     [_KiTimeIncrementReciprocal] ; multiply by low 32-bits of divisor

;
; Compute high 32-bits of dividend time high 32-bits of divisor
;

        mov     eax, DueTimeHigh        ; get high 32-bits of dividend
        push    edx                     ; save high 32-bits of product
        mul     [_KiTimeIncrementReciprocal + 4] ; multiply by high 32-bits of divisor
        pop     edx                     ; retrieve high 32-bits of prodcut
        add     edx, ebx                ; compute full middle 64-bits of product
        adc     eax, ecx                ;

;
; Right shift the result by the specified shift count and mask off extra
; bits.
;

        mov     cl, [_KiTimeIncrementShiftCount] ; get shift count value
        shr     eax, cl                 ; extract appropriate product bits
        and     eax, (TIMER_TABLE_SIZE - 1); reduce to size of table
        pop     ebx                     ; restore nonvolatile register

        stdRET  _KicomputeTimerTableIndex

stdENDP _KiComputeTimerTableIndex

_TEXT$00   ends
        end

