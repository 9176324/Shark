     title  "Miscellaneous Functions"
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
;   miscs.asm
;
; Abstract:
;
;   This module implements machine dependent miscellaneous kernel functions.
;
;--

include ksamd64.inc

        extern  KiContinueEx:proc
        extern  KiExceptionExit:proc
        extern  KiRaiseException:proc

        subttl  "Continue Execution System Service"
;++
;
; NTSTATUS
; NtContinue (
;     IN PCONTEXT ContextRecord,
;     IN BOOLEAN TestAlert
;     )
;
; Routine Description:
;
;   This routine is called as a system service to continue execution after
;   an exception has occurred. Its function is to transfer information from
;   the specified context record into the trap frame that was built when the
;   system service was executed, and then exit the system as if an exception
;   had occurred.
;
; Arguments:
;
;   ContextRecord (rcx) - Supplies a pointer to a context record.
;
;   TestAlert (dl) - Supplies a boolean value that specifies whether alert
;       should be tested for the previous processor mode.
;
; Implicit Arguments:
;
;   rbp - Supplies the address of a trap frame.
;
; Return Value:
;
;   Normally there is no return from this routine. However, if the specified
;   context record is misaligned or is not accessible, then the appropriate
;   status code is returned.
;
;--

        NESTED_ENTRY NtContinue, _TEXT$00

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

;
; Copy context to kernel frames or bypass for user APC.
;
; N.B. The context record and test alert arguments are in the same
;      registers.
;

        mov     r8, rsp                 ; set exception frame address
        lea     r9, (-128)[rbp]         ; set trap frame address
        call    KiContinueEx            ; transfer context to kernel frames

;
; If the return status is less than or equal to zero, then return via the
; system service dispatcher. Otherwise, return via exception exit.
;

        test    eax, eax                ; test return status value
        jle     short KiCO10            ; if le, return via service dispatcher

;
; Return via exception exit.
;
; If the legacy stated is switched, then restore the legacy floating state.
;
; N.B. If the legacy state is restored, then xmm6-xmm15 are also restored
;      with potentially incorrect values. Fortunately, exception exit will
;      restore these registers with their proper value before returning to
;      the user.
;
; N.B. The below code uses an unusual sequence to transfer control. This
;      instruction sequence is required to avoid detection as an epilogue.
;
                                                                          
        mov     rcx, gs:[PcCurrentThread] ; get current thread address
        test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode kernel
        jnz     short KiCO05            ; if nz, previous mode user
        mov     rdx, TrTrapFrame[rbp]   ; restore previous trap frame address
        mov     ThTrapFrame[rcx], rdx   ;
        mov     dl, TrPreviousMode[rbp] ; restore thread previous mode
        mov     ThPreviousMode[rcx], dl ;
KiCO05: lea     rcx, KiExceptionExit    ; get address of exception exit
        jmp     rcx                     ; finish in common code

;
; Return via the system service dispatcher.
;
; N.B. The nonvolatile state in the exception frame is not restored since
;      the state is not used in this function.
;

KiCO10: add     rsp, (KEXCEPTION_FRAME_LENGTH - (1 * 8)) ; deallocate stack frame
        ret                             ; return

        NESTED_END NtContinue, _TEXT$00

        subttl  "Raise Exception System Service"
;++
;
; NTSTATUS
; NtRaiseException (
;     IN PEXCEPTION_RECORD ExceptionRecord,
;     IN PCONTEXT ContextRecord,
;     IN BOOLEAN FirstChance
;     )
;
; Routine Description:
;
;   This routine is called as a system service to raise an exception. Its
;   function is to transfer information from the specified context record
;   into the trap frame that was built when the system service was executed.
;   The exception may be raised as a first or second chance exception.
;
; Arguments:
;
;   ExceptionRecord (rcx) - Supplies a pointer to an exception record.
;
;   ContextRecord (rdx) - Supplies a pointer to a context record.
;
;   FirstChance (r8b) - Supplies a boolean value that specifies whether
;       this is the first (TRUE) or second chance (FALSE) for dispatching
;       the exception.
;
; Implicit Arguments:
;
;   rbp - Supplies a pointer to a trap frame.
;
; Return Value:
;
;   Normally there is no return from this routine. However, if the specified
;   context record or exception record is misaligned or is not accessible,
;   then the appropriate status code is returned.
;
;--

        NESTED_ENTRY NtRaiseException, _TEXT$00

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

;
; Record the address of the caller in a place that won't be overwritten
; by the following context to k-frames.
;

        mov     rax, TrRip[rbp]         ; copy fault address to preserved entry
        mov     TrFaultAddress[rbp], rax ;

;
; Call the raise exception kernel routine which will marshall the arguments
; and then call the exception dispatcher.
;

        mov     ExP5[rsp], r8b          ; set first chance parameter
        mov     r8, rsp                 ; set exception frame address
        lea     r9, (-128)[rbp]         ; set trap frame address
        call    KiRaiseException        ; call raise exception routine

;
; If the kernel raise exception routine returns success, then exit via the
; exception exit code. Otherwise, return to the system service dispatcher.
;

        test    eax, eax                ; test if service failed
        jnz     short KiRE20            ; if nz, service failed

;
; Exit via the exception exit code which will restore the machine state.
;
; N.B. The below code uses an unusual sequence to transfer control. This
;      instruction sequence is required to avoid detection as an epilogue.
;

        test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode kernel
        jnz     short KiRE10            ; if nz, previous mode user
        mov     rbx, gs:[PcCurrentThread] ; get current thread address
        mov     rdx, TrTrapFrame[rbp]   ; restore previous trap frame address
        mov     ThTrapFrame[rbx], rdx   ;
        mov     dl, TrPreviousMode[rbp] ; restore thread previous mode
        mov     ThPreviousMode[rbx], dl ;
KiRE10: lea     rcx, KiExceptionExit    ; get address of exception exit
        jmp     rcx                     ; finish in common code

;
; The context or exception record is misaligned or not accessible, or the
; exception was not handled.
;

KiRE20: RESTORE_EXCEPTION_STATE         ; restore exception state/deallocate

        ret                             ; return

        NESTED_END NtRaiseException, _TEXT$00

;++
;
; ULONG
; KiDivide6432 (
;     IN ULONG64 Dividend,
;     IN ULONG Divisor
;     );
;
; Routine Description:
;
;   This routine performs a divide operation with a dividend of exactly
;   64 bits, a divisor of exactly 32 bits, and a quotient of exactly 32
;   bits.
;
; Arguments:
;
;   Dividend - Supplies a 64-bit unsigned dividend.
;
;   Divisor - Supplies a 32-bit unsigned divisor.
;
; Return Value:
;
;   Returns the 32-bit unsigned quotient.
;
;-- 

        LEAF_ENTRY KiDivide6432, INIT

        mov     eax, ecx                ; set low 32-bits of dividend
        mov     r8, rdx                 ; save 32-bit divisor
        shr     rcx, 32                 ; set high 32-bits of dividend
        mov     edx, ecx                ;
        div     r8d                     ; perform 64/32 to 32 divide
        ret                             ;

        LEAF_END KiDivide6432, INIT

        end

