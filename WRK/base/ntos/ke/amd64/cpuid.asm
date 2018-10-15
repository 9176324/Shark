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
;    cpu.asm
;
; Abstract:
;
;    This module implements the code necessary to determine cpu information.
;
;--

include ksamd64.inc

        altentry KiCpuIdFault

;++
;
; VOID
; KiCpuId (
;     ULONG Function,
;     ULONG Index,
;     PCPU_INFO CpuInfo
;     );
;
; Routine Description:
;
;   Executes the cpuid instruction and returns the resultant register
;   values.
;
; Arguments:
;
;   ecx - Supplies the cpuid function value.
;
;   edx - Supplies a index of cache descriptor. 
; 
;   r8  - Supplies the address a cpu information structure.
;
; Return Value:
;
;   The return values from the cpuid instruction are stored in the specified
;   cpu information structure.
;
;--

        NESTED_ENTRY KiCpuId, _TEXT$00

        push_reg rbx                    ; save nonvolatile register

        END_PROLOGUE

        mov     eax, ecx                ; set cpuid function
        mov     ecx, edx                ; set index (only used by function 4)
        cpuid                           ; get cpu information

        ALTERNATE_ENTRY KiCpuIdFault

        mov     CpuEax[r8], eax         ; save cpu information in structure
        mov     CpuEbx[r8], ebx         ;
        mov     CpuEcx[r8], ecx         ;
        mov     CpuEdx[r8], edx         ;
        pop     rbx                     ; restore nonvolatile register
        ret                             ; return

        NESTED_END KiCpuId, _TEXT$00

        end

