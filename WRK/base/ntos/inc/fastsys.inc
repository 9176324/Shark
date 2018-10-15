; /*
;
; Copyright (c) Microsoft Corporation. All rights reserved. 
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
; Module Name:
;
;    fastsys.inc
;
; Abstract:
;
;    This module implements the header for Fast System Calls for Intel
;    x86 family processors.
;
;--

if 0  ; Begin C only code   */

// CPUID Feature bit in EDX indicating that fast system calls are supported

#define KI_FAST_SYSCALL_SUPPORTED   0x0800

#define MSR_SYSENTER_CS             0x00000174
#define MSR_SYSENTER_ESP            0x00000175
#define MSR_SYSENTER_EIP            0x00000176

#define MSR_EXT_FEATURE_ENABLE      0xC0000080
#define MSR_SYSCALL_TARGET_ADDR     0xC0000081

//
// Define bits in the Extended Feature Enable Register (EFER) (MSR).
//

#define MSR_EFER_SCE                0x1

/*
endif
;
;  Begin Assembly definitions
;


iSYSENTER macro
    db 0fH,34H
endm ;; SYSENTER

iSYSEXIT macro
    db 0fH,35H
endm ;; SYSEXIT

iSYSCALL macro
    db 0fH,05H
endm ;; SYSCALL

iSYSRET macro
    db 0fH,07H
endm ;; SYSRET

; */
