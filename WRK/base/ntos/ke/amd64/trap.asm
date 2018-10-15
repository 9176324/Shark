     title  "Trap Processing"
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
;   trap.asm
;
; Abstract:
;
;   This module implements the code necessary to field and process AMD64
;   trap conditions.
;
;--

include ksamd64.inc

        altentry KiExceptionExit
        altentry KiSystemServiceCopyEnd
        altentry KiSystemServiceCopyStart
        altentry KiSystemServiceExit
        altentry KiSystemServiceGdiTebAccess
        altentry KiSystemServiceRepeat
        altentry KiSystemServiceStart

        extern  ExpInterlockedPopEntrySListEnd:proc
        extern  ExpInterlockedPopEntrySListFault:byte
        extern  ExpInterlockedPopEntrySListResume:proc
        extern  KdpOweBreakpoint:byte
        extern  KdSetOwedBreakpoints:proc
        extern  KeBugCheckEx:proc
        extern  KeGdiFlushUserBatch:qword
        extern  KeServiceDescriptorTableShadow:qword
        extern  KiCheckForSListAddress:proc
        extern  KiCodePatchCycle:dword
        extern  KiConvertToGuiThread:proc
        extern  KiDispatchException:proc
        extern  KiDpcInterruptBypass:proc
        extern  KiIdleSummary:qword
        extern  KiInitiateUserApc:proc
        extern  KiPrefetchRetry:byte
        extern  KiPreprocessInvalidOpcodeFault:proc
        extern  KiPreprocessKernelAccessFault:proc
        extern  KiProcessNMI:proc
        extern  KiProcessorBlock:qword
        extern  KiRestoreDebugRegisterState:proc
        extern  KiSaveDebugRegisterState:proc
        extern  KiSaveProcessorState:proc
        extern  MmAccessFault:proc
        extern  MmUserProbeAddress:qword
        extern  RtlUnwindEx:proc
        extern  PsWatchEnabled:byte
        extern  PsWatchWorkingSet:proc
        extern  __imp_HalHandleMcheck:qword
        extern  __imp_HalRequestSoftwareInterrupt:qword

;
; Define special macros to align trap entry points on cache line boundaries.
;
; N.B. This will only work if all functions in this module are declared with
;      these macros.
;

TRAP_ENTRY macro Name, Handler

_TEXT$10 segment page 'CODE'

        align   64

        public  Name

ifb <Handler>

Name    proc    frame

else

Name    proc    frame:Handler

endif

        endm

TRAP_END macro Name

Name    endp

_TEXT$10 ends

        endm

        subttl  "Divide Error Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of an attempted division by zero
;   or the result of an attempted division does not fit in the destination
;   operand (i.e., the largest negative number divided by minus one).
;
;   N.B. The two possible conditions that can cause this exception are not
;        separated and the exception is reported as a divide by zero.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiDivideErrorFault

        GENERATE_TRAP_FRAME <>, <PatchCycle> ; generate trap frame

        mov     ecx, KI_EXCEPTION_INTEGER_DIVIDE_BY_ZERO ; set exception code
        xor     edx, edx                ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiDivideErrorFault

        subttl  "Debug Trap Or Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a debug trap or fault. The
;   following conditions cause entry to this routine:
;
;   1. Instruction fetch breakpoint fault.
;   2. Data read or write breakpoint trap.
;   3. I/O read or write breakpoint trap.
;   4. General detect condition fault (in-circuit emulator).
;   5. Single step trap (TF set).
;   6. Task switch trap (not possible on this system).
;   7. Execution of an int 1 instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiDebugTrapOrFault

        GENERATE_TRAP_FRAME             ; generate trap frame

        xor     edx, edx                ; set number of parameters
        test    dword ptr TrEflags[rbp], EFLAGS_TF_MASK ; test if TF is set
        jz      short KiDT30            ; if z, TF not set
        cmp     byte ptr gs:[PcCpuVendor], CPU_AMD ; check if AMD processor
        jne     short KiDT30            ; if ne, not authentic AMD processor

;
; The host processor is an authentic AMD processor.
;
; Check if branch tracing and last branch capture is enabled.
;

        test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode user
        jnz     short KiDT10            ; if nz, previous mode user

;
; Previous mode was kernel - the debug registers have not yet been saved.
;

        mov     rax, dr7                ; get debug control register
        test    ax, DR7_TRACE_BRANCH    ; test if trace branch set
        jz      short KiDT30            ; if z, trace branch not set
        test    ax, DR7_LAST_BRANCH     ; test if last branch set
        jz      short KiDT30            ; if z, last branch not set
        mov     ecx, MSR_LAST_BRANCH_FROM ; get last branch information
        rdmsr                           ;
        mov     r9d, eax                ;
        shl     rdx, 32                 ;
        or      r9, rdx                 ;
        mov     ecx, MSR_LAST_BRANCH_TO ;
        rdmsr                           ;
        mov     r10d, eax               ;
        shl     rdx, 32                 ;
        or      r10, rdx                ;
        jmp     short KiDT20            ; finish in common code


;
; Previous mode was user - the debug registers are saved in the trap frame.
;

KiDT10: test    word ptr TrDr7[rbp], DR7_TRACE_BRANCH ; test if trace branch set
        jz      short KiDT30            ; if z, trace branch not set
        test    word ptr TrDr7[rbp], DR7_LAST_BRANCH ; test if last branch set
        jz      short KiDT30            ; if z, last branch not set
        mov     r9, TrLastBranchFromRip[rbp] ; set last RIP parameters
        mov     r10, TrLastBranchToRip[rbp] ;
KiDT20: mov     edx, 2                  ; set number of parameters
KiDT30: mov     ecx, STATUS_SINGLE_STEP ; set exception code
        and     dword ptr TrEflags[rbp], NOT EFLAGS_TF_MASK ; reset the TF bit
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiDebugTrapOrFault

        subttl  "Nonmaskable Interrupt"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a nonmaskable interrupt. A
;   switch to the panic stack occurs before the exception frame is pushed
;   on the stack.
;
;   N.B. This routine executes on the panic stack.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack and the NMI is
;   processed. If a return to this routine occurs, then the NMI was handled.
;
;--

        TRAP_ENTRY KiNmiInterrupt

        .pushframe                      ; mark machine frame

        alloc_stack 8                   ; allocate dummy vector
        push_reg rbp                    ; save nonvolatile register

        GENERATE_INTERRUPT_FRAME <>, <Direct>, <Nmi> ; generate interrupt frame

        mov     ecx, HIGH_LEVEL         ; set IRQL value

        ENTER_INTERRUPT <NoEOI>, <NoCount>, <Nmi> ; enter interrupt

;
; Check to determine if a recursive non-maskable interrupt has occured. This
; can happen when an SMI interrupts an NMI in progress, unmasking NMIs, and a
; second NMI is pending.
;

        lea     rax, KTRAP_FRAME_LENGTH[rsp] ; get base stack address
        cmp     rax, TrRsp[rbp]         ; check if within range
        jbe     KiNi10                  ; if be, old stack above base
        sub     rax, NMI_STACK_SIZE     ; compute stack limit
        cmp     rax, TrRsp[rbp]         ; check if within range
        jbe     KiNi20                  ; if be, old stack in range

KiNi10: call    KxNmiInterrupt          ; call secondary routine

        EXIT_INTERRUPT <NoEOI>, <NoCount>, <Direct>, <Nmi> ; restore trap state and exit

;
; A recursive non-maskable interrupt has occured.
;

KiNi20: xor     r10, r10                ; clear bugcheck parameters
        xor     r9, r9                  ;
        xor     r8, r8                  ;
        xor     edx, edx                ;
        mov     ecx, RECURSIVE_NMI      ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiNmiInterrupt

;
; This routine generates an exception frame, then processes the NMI.
;

        TRAP_ENTRY KxNmiInterrupt

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

        lea     rcx, (-128)[rbp]        ; set trap frame address
        mov     rdx, rsp                ; set exception frame address
        call    KiSaveProcessorState    ; save processor state
        lea     rcx, (-128)[rbp]        ; set trap frame address
        mov     rdx, rsp                ; set exception frame address
        call    KiProcessNMI            ; process NMI

        RESTORE_EXCEPTION_STATE         ; restore exception state/deallocate

        ret                             ; return

        TRAP_END KxNmiInterrupt


        subttl  "Breakpoint Trap"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an int 3
;   instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiBreakpointTrap

        GENERATE_TRAP_FRAME             ; generate trap frame

        mov     ecx, STATUS_BREAKPOINT  ; set exception code
        mov     edx, 1                  ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        dec     r8                      ;
        mov     r9d, BREAKPOINT_BREAK   ; set parameter 1 value
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiBreakpointTrap

        subttl  "Overflow Trap"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an into
;   instruction when the OF flag is set.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiOverflowTrap

        GENERATE_TRAP_FRAME             ; generate trap frame

        mov     ecx, STATUS_INTEGER_OVERFLOW ; set exception code
        xor     edx, edx                ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        dec     r8                      ;
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiOverflowTrap

        subttl  "Bound Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of a bound
;   instruction and when the bound range is exceeded.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiBoundFault

        GENERATE_TRAP_FRAME             ; generate trap frame

        mov     ecx, STATUS_ARRAY_BOUNDS_EXCEEDED ; set exception code
        xor     edx, edx                ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiBoundFault

        subttl  "Invalid Opcode Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an invalid
;   instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiInvalidOpcodeFault

        GENERATE_TRAP_FRAME <>, <PatchCycle> ; generate trap frame

        mov     rcx, rsp                ; set trap frame address
        call    KiPreprocessInvalidOpcodeFault ; check for opcode emulation
        or      eax, eax                ; test if opcode emulated
        jnz     short KiIO10            ; if nz, opcode emulated
        mov     ecx, KI_EXCEPTION_INVALID_OP ; set exception code
        xor     edx, edx                ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

KiIO10: RESTORE_TRAP_STATE <Volatile>   ; restore trap state and exit

        TRAP_END KiInvalidOpcodeFault

        subttl  "NPX Not Available Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the numeric coprocessor not
;   being available for one of the following conditions:
;
;   1. A floating point instruction was executed and EM is set in CR0 -
;       this condition should never happen since EM will never be set.
;
;   2. A floating point instruction was executed and the TS flag is set
;       in CR0 - this condition should never happen since TS will never
;       be set.
;
;   3. A WAIT of FWAIT instruction was executed and the MP and TS flags
;       are set in CR0 - this condition should never occur since neither
;       TS nor MP will ever be set.
;
;   N.B. The NPX state should always be available.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack and bugcheck
;   is called.
;
;--

        TRAP_ENTRY KiNpxNotAvailableFault

        GENERATE_TRAP_FRAME             ; generate trap frame

        mov     r10, TrRip[rbp]         ; set parameter 5 to exception address
        mov     r9, cr4                 ; set parameter 4 to control register 4
        mov     r8, cr0                 ; set parameter 3 to control register 0
        mov     edx, EXCEPTION_NPX_NOT_AVAILABLE ; set unexpected trap number
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiNpxNotAvailableFault

        subttl  "Double Fault Abort"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the generation of a second
;   exception while another exception is being generated. A switch to the
;   panic stack occurs before the exception frame is pushed on the stack.
;
;   N.B. This routine executes on the panic stack.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the new stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack and bugcheck
;   is called.
;
;--

        TRAP_ENTRY KiDoubleFaultAbort

        GENERATE_TRAP_FRAME <ErrorCode> ; generate trap frame

        mov     r10, TrRip[rbp]         ; set parameter 5 to exception address
        mov     r9, cr4                 ; set parameter 4 to control register 4
        mov     r8, cr0                 ; set parameter 3 to control register 0
        mov     edx, EXCEPTION_DOUBLE_FAULT ; set unexpected trap number
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiDoubleFaultAbort

        subttl  "NPX Segment Overrrun Abort"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a hardware failure since this
;   vector is obsolete.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the new stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   This trap should never occur and the system is shutdown via a call to
;   bugcheck.
;
;--

        TRAP_ENTRY KiNpxSegmentOverrunAbort

        GENERATE_TRAP_FRAME             ; generate trap frame

        mov     r10, TrRip[rbp]         ; set parameter 5 to exception address
        mov     r9, cr4                 ; set parameter 4 to control register 4
        mov     r8, cr0                 ; set parameter 3 to control register 0
        mov     edx, EXCEPTION_NPX_OVERRUN ; set unexpected trap number
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiNpxSegmentOverrunAbort

        subttl  "Invalid TSS Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a hardware or software failure
;   since there is no task switching in 64-bit mode and 32-bit code does not
;   have any task state segments.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the new stack.
;   The segment selector index for the segment descriptor that caused the
;   violation is pushed as the error code.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack and bugcheck
;   is called.
;
;--

        TRAP_ENTRY KiInvalidTssFault

        GENERATE_TRAP_FRAME <ErrorCode> ; generate trap frame

        mov     r10, TrRip[rbp]         ; set parameter 5 to exception address
        mov     r9d, TrErrorCode[rbp]   ; set parameter 4 to selector index
        mov     r8, cr0                 ; set parameter 3 to control register 0
        mov     edx, EXCEPTION_INVALID_TSS ; set unexpected trap number
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiInvalidTssFault

        subttl  "Segment Not Present Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a segment not present (P bit 0)
;   fault. This fault can only occur in legacy 32-bit code.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the new stack.
;   The segment selector index for the segment descriptor that is not
;   present is pushed as the error code.
;
; Disposition:
;
;   A standard trap frame is constructed. If the previous mode is user,
;   then the exception parameters are loaded into registers and the exception
;   is dispatched via common code. Otherwise, bugcheck is called.
;
;--

        TRAP_ENTRY KiSegmentNotPresentFault

        GENERATE_TRAP_FRAME <ErrorCode> ; generate trap frame

        mov     r8, TrRip[rbp]          ; get exception address
        test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode user
        jz      short KiSN10            ; if z, previous mode not user

;
; The previous mode was user.
;

        mov     ecx, STATUS_ACCESS_VIOLATION ; set exception code
        mov     edx, 2                  ; set number of parameters
        mov     r9d, TrErrorCode[rbp]   ; set parameter 1 value
        or      r9d, RPL_MASK           ;
        and     r9d, 0ffffh             ;
        xor     r10, r10                ; set parameter 2 value
        call    KiExceptionDispatch     ; dispatch exception - no return

;
; The previous mode was kernel.
;

KiSN10: mov     r10, r8                 ; set parameter 5 to exception address
        mov     r9d, TrErrorCode[rbp]   ; set parameter 4 to selector index
        mov     r8, cr0                 ; set parameter 3 to control register 0
        mov     edx, EXCEPTION_SEGMENT_NOT_PRESENT ; set unexpected trap number
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiSegmentNotPresentFault

        subttl  "Stack Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a stack fault. This fault can
;   only occur in legacy 32-bit code.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the new stack.
;   The segment selector index for the segment descriptor that caused the
;   exception is pushed as the error code.
;
; Disposition:
;
;   A standard trap frame is constructed. If the previous mode is user,
;   then the exception parameters are loaded into registers and the exception
;   is dispatched via common code. Otherwise, bugcheck is called.
;
;--

        TRAP_ENTRY KiStackFault

        GENERATE_TRAP_FRAME <ErrorCode> ; generate trap frame

        mov     r8, TrRip[rbp]          ; get exception address
        test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode user
        jz      short KiSF10            ; if z, previous mode not user

;
; The previous mode was user.
;

        mov     ecx, STATUS_ACCESS_VIOLATION ; set exception code
        mov     edx, 2                  ; set number of parameters
        mov     r9d, TrErrorCode[rbp]   ; set parameter 1 value
        or      r9d, RPL_MASK           ;
        and     r9d, 0ffffh             ;
        xor     r10, r10                ; set parameter 2 value
        call    KiExceptionDispatch     ; dispatch exception - no return

;
; The previous mode was kernel.
;

KiSF10: mov     r10, r8                 ; set parameter 5 to exception address
        mov     r9d, TrErrorCode[rbp]   ; set parameter 4 to selector index
        mov     r8, cr0                 ; set parameter 3 to control register 0
        mov     edx, EXCEPTION_STACK_FAULT ; set unexpected trap number
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiStackFault

        subttl  "General Protection Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a general protection violation.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   The segment selector index for the segment descriptor that caused the
;   exception, the IDT vector number for the descriptor that caused the
;   exception, or zero is pushed as the error code.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiGeneralProtectionFault

        GENERATE_TRAP_FRAME <ErrorCode>, <PatchCycle> ; generate trap frame

        mov     ecx, KI_EXCEPTION_GP_FAULT ; set GP fault internal code
        mov     edx, 2                  ; set number of parameters
        mov     r9d, TrErrorCode[rbp]   ; set parameter 1 to error code
        and     r9d, 0ffffh             ;
        xor     r10, r10                ; set parameter 2 value
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiGeneralProtectionFault       

        subttl  "Page Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a page fault which can occur
;   because of the following reasons:
;
;   1. The referenced page is not present.
;
;   2. The referenced page does not allow the requested access.
;
; Arguments:
;
;   A standard exception frame is pushed by hardware on the kernel stack.
;   A special format error code is pushed which specifies the cause of the
;   page fault as not present, read/write access denied, from user/kernel
;   mode, and attempting to set reserved bits.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack and memory
;   management is called to resolve the page fault. If memory management
;   successfully resolves the page fault, then working set information is
;   recorded, owed breakpoints are inserted, and execution is continued.
;
;   If the page fault occurred at an IRQL greater than APC_LEVEL, then the
;   system is shut down via a call to bugcheck. Otherwise, an appropriate
;   exception is raised.
;
;--

        TRAP_ENTRY KiPageFault

        GENERATE_TRAP_FRAME <Virtual>, <PatchCycle>, <SaveGSSwap> ; generate trap frame

;
; The registers eax and rcx are loaded with the error code and the virtual
; address of the fault respectively when the trap frame is generated.
;

        shr     eax, 1                  ; isolate load/store and i/d indicators
        and     eax, 09h                ;

;
; Save the load/store indicator and the faulting virtual address in the
; exception record in case an exception is raised.
;

        mov     TrFaultIndicator[rbp], al ; save load/store indicator
        mov     TrFaultAddress[rbp], rcx ; save fault address
        lea     r9, (-128)[rbp]         ; set trap frame address
        mov     r8b, TrSegCs[rbp]       ; isolate previous mode
        and     r8b, MODE_MASK          ;
        mov     rdx, rcx                ; set faulting virtual address
        movzx   ecx, al                 ; set load/store indicator
        jnz     short KiPF05            ; if nz, previous mode user
        cmp     KiPrefetchRetry, 0      ; check if prefetch retry required
        je      short KiPF05            ; if e, prefetch retry not required
        test    al, EXCEPTION_EXECUTE_FAULT ; test is execution fault
        jnz     short KiPF05            ; if nz, execution fault
        call    KiPreprocessKernelAccessFault ; preprocess fault
        test    eax, eax                ; check for instruction retry
        jge     KiPF60                  ; if ge, retry instruction
        lea     r9, (-128)[rbp]         ; set trap frame address
        mov     r8b, TrSegCs[rbp]       ; isolate previous mode
        and     r8b, MODE_MASK          ;
        mov     rdx, TrFaultAddress[rbp]; set faulting virtual address
        movzx   ecx, BYTE PTR TrFaultIndicator[rbp] ; set load/store indicator

KiPF05: call    MmAccessFault           ; attempt to resolve page fault
        test    eax, eax                ; test for successful completion
        jl      short KiPF20            ; if l, not successful completion

;
; If watch working set is enabled, then record working set information.
;

        cmp     PsWatchEnabled, 0       ; check if working set watch enabled
        je      short KiPF10            ; if e, working set watch not enabled
        mov     r8, TrFaultAddress[rbp] ; set fault address
        mov     rdx, TrRip[rbp]         ; set exception address
        mov     ecx, eax                ; set completion status
        call    PsWatchWorkingSet       ; record working set information

;
; If the debugger has any breakpoints that should be inserted, then attempt
; to insert them now.
;

KiPF10: cmp     KdpOweBreakPoint, 0     ; check if breakpoints are owed
        je      KiPF60                  ; if e, no owed breakpoints
        call    KdSetOwedBreakpoints    ; notify the debugger of new page
        jmp     KiPF60                  ; finish in common code

;
; Check if a 32-bit user mode program reloaded the segment register GS and
; wiped out the GS base address.
;

KiPF20: test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode user
        jz      short KiPF25            ; if z, previous mode not user
        cmp     word ptr TrSegCs[rbp], (KGDT64_R3_CODE or RPL_MASK) ; check for 64-bit mode
        jne     short KiPF23            ; if ne, not running in 64-bit mode
        mov     r8, gs:[PcTeb]          ; get current TEB address
        cmp     r8, TrGsSwap[rbp]       ; check for user TEB address match
        je      short KiPF25            ; if e, user TEB address match
        mov     ecx, MSR_GS_SWAP        ; set GS swap MSR number
        mov     eax, r8d                ; set low user TEB address
        shr     r8, 32                  ; set high user TEB address
        mov     edx, r8d                ; 
        wrmsr                           ; write user TEB base address
        jmp     KiPF60                  ; finish is common code

;
; Check if the 32-bit program attempted a reference outside the 32-bit address
; space.
;

KiPF23: mov     rcx, TrFaultAddress[rbp] ; get fault address
        shr     rcx, 32                 ; isolate upper address bits
        jnz     KiPF60                  ; if nz, high address bits set

;
; Memory management failed to resolve the fault.
;
; STATUS_IN_PAGE_ERROR | 0x10000000 is a special status that indicates a
;       page fault at IRQL greater than APC level. This status causes a
;       bugcheck.
;
; The following status values can be raised:
;
; STATUS_ACCESS_VIOLATION
; STATUS_GUARD_PAGE_VIOLATION
; STATUS_STACK_OVERFLOW
;
; All other status values are sconverted to:
;
; STATUS_IN_PAGE_ERROR
;

KiPF25: mov     ecx, eax                ; set status code
        mov     edx, 2                  ; set number of parameters
        cmp     ecx, STATUS_IN_PAGE_ERROR or 10000000h ; check for bugcheck code
        je      short KiPF40            ; if e, bugcheck code returned
        cmp     ecx, STATUS_ACCESS_VIOLATION ; check for status values
        je      short KiPF28            ; if e, raise exception with internal code
        cmp     ecx, STATUS_GUARD_PAGE_VIOLATION ; check for status code
        je      short KiPF30            ; if e, raise exception with code
        cmp     ecx, STATUS_STACK_OVERFLOW ; check for status code
        je      short KiPF30            ; if e, raise exception with code
        mov     ecx, STATUS_IN_PAGE_ERROR ; convert all other status codes
        mov     edx, 3                  ; set number of parameters
        mov     r11d, eax               ; set parameter 3 to real status value
        jmp     KiPF30

KiPF28: mov     ecx, KI_EXCEPTION_ACCESS_VIOLATION ; set internal code

;
; Set virtual address, load/store and i/d indicators, exception address, and
; dispatch the exception.
;

KiPF30: mov     r10, TrFaultAddress[rbp] ; set fault address
        movzx   r9, byte ptr TrFaultIndicator[rbp] ; set load/store indicator
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return

;
; A page fault occurred at an IRQL that was greater than APC_LEVEL. Set bugcheck
; parameters and join common code.
;

KiPF40: CurrentIrql                     ; get current IRQL

        mov     r10, TrRip[rbp]         ; set parameter 5 to exception address
        movzx   r9, byte ptr TrFaultIndicator[rbp] ; set load/store indicator
        and     eax, 0ffh               ; isolate current IRQL
        mov     r8, rax                 ;
        mov     rdx, TrFaultAddress[rbp] ; set fault address
        mov     ecx, IRQL_NOT_LESS_OR_EQUAL ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return

;
; If the page fault occured within the kernel pop entry SLIST code, then reset
; RIP if necessary to avoid an SLIST sequence wrap attack.
;
; Make sure that IRQL is greater than passive level to block a set context
; operations.
;

KiPF60: lea     rax, ExpInterlockedPopEntrySListResume ; get SLIST resume address
        cmp     rax, TrRip[rbp]         ; check resume address is above RIP
        jae     KiPF70                  ; if ae, resume address above RIP
        lea     rax, ExpInterlockedPopEntrySListEnd ; get SLIST end address
        cmp     rax, TrRip[rbp]         ; check end address is below RIP
        jb      short KiPF70            ; if b, end address below RIP

        CurrentIrql                     ; get Current IRQL

        or      eax, eax                ; test is IRQL is passive level
        mov     TrP5[rbp], eax          ; save current IRQL
        jne     short KiPF65            ; if ne, IRQL is above passive level
        mov     ecx, APC_LEVEL          ; get APC level

        SetIrql                         ; set IRQL to APC level

KiPF65: lea     rcx, (-128)[rbp]        ; set trap frame address
        call    KiCheckForSListAddress  ; check RIP and reset if necessary
        mov     ecx, TrP5[rbp]          ; get previous IRQL value
        or      ecx, ecx                ; test if IRQL was raised
        jne     short KiPF70            ; if nz, IRQL was not raised

        SetIrql                         ; set IRQL to previous value

;
; Test if a user APC should be delivered and exit exception.
;

KiPF70: RESTORE_TRAP_STATE <Volatile>   ; restore trap state and exit

        TRAP_END KiPageFault

        subttl  "Legacy Floating Error"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a legacy floating point fault.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack. If the previous
;   mode is user, then reason for the exception is determine, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code. Otherwise, bugcheck is called.
;
;--

        TRAP_ENTRY KiFloatingErrorFault

        GENERATE_TRAP_FRAME             ; generate trap frame

        test    byte ptr TrSegCs[rbp], MODE_MASK ; check if previous mode user
        jz      KiFE30                  ; if z,  previous mode not user

;
; The previous mode was user mode.
;

        fnstcw  TrErrorCode[rbp]        ; store floating control word
        fnstsw  ax                      ; store floating status word
        mov     cx, TrErrorCode[rbp]    ; get control word
        and     cx, FSW_ERROR_MASK      ; isolate masked exceptions
        not     cx                      ; compute enabled exceptions
        and     ax, cx                  ; isolate exceptions
        mov     ecx, STATUS_FLOAT_INVALID_OPERATION ; set exception code
        xor     r9, r9                  ; set first exception parameter
        mov     edx, 1                  ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        test    al, FSW_INVALID_OPERATION ; test for invalid operation
        jz      short KiFE10            ; if z, non invalid operation
        test    al, FSW_STACK_FAULT     ; test is caused by stack fault
        jz      short KiFE20            ; if z, not caused by stack fault
        mov     ecx, STATUS_FLOAT_STACK_CHECK ; set exception code
        jmp     short KiFE20            ; finish in common code

KiFE10: mov     ecx, STATUS_FLOAT_DIVIDE_BY_ZERO ; set exception code
        test    al, FSW_ZERO_DIVIDE     ; test for divide by zero
        jnz     short KiFE20            ; if nz, divide by zero
        mov     ecx, STATUS_FLOAT_INVALID_OPERATION ; set exception code
        test    al, FSW_DENORMAL        ; test if denormal operand
        jnz     short KiFE20            ; if nz, denormal operand
        mov     ecx, STATUS_FLOAT_OVERFLOW ; set exception code
        test    al, FSW_OVERFLOW        ; test if overflow
        jnz     short KiFE20            ; if nz, overflow
        mov     ecx, STATUS_FLOAT_UNDERFLOW ; set exception code
        test    al, FSW_UNDERFLOW       ; test if underflow
        jnz     short KiFE20            ; if nz, underflow
        mov     ecx, STATUS_FLOAT_INEXACT_RESULT ; set exception code
        test    al, FSW_PRECISION       ; test for inexact result
        jz      short KiFE30            ; if z, not inexact result

KiFE20: call    KiExceptionDispatch     ; dispatch exception - no return

;
; The previous mode was kernel mode or the cause of the exception is unknown.
;

KiFE30: mov     edx, EXCEPTION_NPX_ERROR; set unexpected trap number
        mov     r10, TrRip[rbp]         ; set parameter 5 to exception address
        mov     r9, cr4                 ; set parameter 4 to control register 4
        mov     r8, cr0                 ; set parameter 3 to control register 0
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiFloatingErrorFault

        subttl  "Alignment Fault"
;++
;
; Routine Description:
;
;   This routine is entered as the result of an attempted access to unaligned
;   data.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   An error error code of zero is pushed on the stack.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiAlignmentFault

        GENERATE_TRAP_FRAME <ErrorCode> ; generate trap frame

        mov     ecx, STATUS_DATATYPE_MISALIGNMENT ; set exception code
        xor     edx, edx                ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiAlignmentFault

        subttl  "Machine Check Abort"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a machine check. A switch to
;   the machine check stack occurs before the exception frame is pushed on
;   the stack.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap and exception frame are constructed on the kernel stack
;   and the HAL is called to determine if the machine check abort is fatal.
;   If the HAL call returns, then system operation is continued.
;
;--

        TRAP_ENTRY KiMcheckAbort

	.pushframe			; mark machine frame

	alloc_stack 8			; allocate dummy vector
	push_reg rbp			; save nonvolatile register

	GENERATE_INTERRUPT_FRAME <>, <Direct> ; generate interrupt frame

        mov     ecx, HIGH_LEVEL         ; set IRQL value

        ENTER_INTERRUPT <NoEoi>, <NoCount> ; raise IRQL and enable interrupts

;
; Check to determine if a recursive machine check has occurred. This can
; happen when machine check in progress is cleared and another machine
; check exception occurs before a complete exit from the below code can
; be performed.
;

        lea     rax, KTRAP_FRAME_LENGTH[rsp] ; get base stack address
        cmp     rax, TrRsp[rbp]         ; check if with range
        jbe     short KiMC10            ; if be, old stack above base
        sub     rax, KERNEL_MCA_EXCEPTION_STACK_SIZE ; compute limit stack address
        cmp     rax, TrRsp[rbp]         ; check if with range
        jbe     KiMC20                  ; if be, old stack in range

KiMC10: call    KxMcheckAbort           ; call secondary routine

;
; Clear machine check in progress.
;
; N.B. This is done very late to ensure that the window whereby a recursive
;      machine can occur is as small as possible. A recursive machine check
;      reloads the machine stack pointer from the TSS and overwrites any
;      information previously on the stack.
;

        xor     eax, eax                ; clear machine check in progress
        xor     edx, edx                ;
        mov     ecx, MSR_MCG_STATUS     ;
        wrmsr                           ;

        EXIT_INTERRUPT <NoEoi>, <NoCount>, <Direct> ; lower IRQL and restore state

;
; A recursive machine check exception has occurred.
;
;

KiMC20: xor     r10,r10                 ; clear bugcheck parameters
        xor     r9, r9                  ;
        xor     r8, r8                  ;
        xor     edx, edx                ;
        mov     ecx, RECURSIVE_MACHINE_CHECK ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiMcheckAbort

;
; This routine generates an exception frame, then calls the HAL to process
; the machine check.
;

        TRAP_ENTRY KxMcheckAbort

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

        lea     rcx, (-128)[rbp]        ; set trap frame address
        mov     rdx, rsp                ; set exception frame address
        call    __imp_HalHandleMcheck   ; give HAL a chance to handle mcheck

        RESTORE_EXCEPTION_STATE         ; restore exception state/deallocate

        ret                             ; return

        TRAP_END KxMcheckAbort

        subttl  "XMM Floating Error"
;++
;
; Routine Description:
;
;   This routine is entered as the result of a XMM floating point fault.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, mode is user,
;   then reason for the exception is determine, the exception parameters are
;   loaded into registers, and the exception is dispatched via common code.
;   If no reason can be determined for the exception, then bugcheck is called.
;
;--

        TRAP_ENTRY KiXmmException

        GENERATE_TRAP_FRAME <MxCsr>     ; generate trap frame

        mov     cx, ax                  ; shift enables into position
        shr     cx, XSW_ERROR_SHIFT     ;
        and     cx, XSW_ERROR_MASK      ; isolate masked exceptions
        not     cx                      ; compute enabled exceptions
        movzx   r10d, ax                ; set second exception parameter
        and     ax, cx                  ; isolate exceptions
        mov     edx, 2                  ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        xor     r9, r9                  ; set first exception parameter
        cmp     word ptr TrSegCs[rbp], KGDT64_R3_CMCODE or RPL_MASK ; legacy code?
        je      short KiXE15            ; if e, legacy code

;
; The XMM exception occurred in 64-bit code.
;

        mov     ecx, STATUS_FLOAT_INVALID_OPERATION ; set exception code
        test    al, XSW_INVALID_OPERATION ; test for invalid operation
        jnz     short KiXE10            ; if z, invalid operation
        mov     ecx, STATUS_FLOAT_DIVIDE_BY_ZERO ; set exception code
        test    al, XSW_ZERO_DIVIDE     ; test for divide by zero
        jnz     short KiXE10            ; if nz, divide by zero
        mov     ecx, STATUS_FLOAT_INVALID_OPERATION ; set exception code
        test    al, XSW_DENORMAL        ; test if denormal operand
        jnz     short KiXE10            ; if nz, denormal operand
        mov     ecx, STATUS_FLOAT_OVERFLOW ; set exception code
        test    al, XSW_OVERFLOW        ; test if overflow
        jnz     short KiXE10            ; if nz, overflow
        mov     ecx, STATUS_FLOAT_UNDERFLOW ; set exception code
        test    al, XSW_UNDERFLOW       ; test if underflow
        jnz     short KiXE10            ; if nz, underflow
        mov     ecx, STATUS_FLOAT_INEXACT_RESULT ; set exception code
        test    al, XSW_PRECISION       ; test for inexact result
        jz      short KiXE20            ; if z, not inexact result

KiXE10: call    KiExceptionDispatch     ; dispatch exception - no return

;
; The XMM exception occurred in legacy 32-bit code
;

KiXE15: mov     ecx, STATUS_FLOAT_MULTIPLE_TRAPS ; set exception code
        test    al, XSW_INVALID_OPERATION ; test for invalid operation
        jnz     short KiXE10            ; if z, invalid operation
        test    al, XSW_ZERO_DIVIDE     ; test for divide by zero
        jnz     short KiXE10            ; if nz, divide by zero
        test    al, XSW_DENORMAL        ; test if denormal operand
        jnz     short KiXE10            ; if nz, denormal operand
        mov     ecx, STATUS_FLOAT_MULTIPLE_FAULTS ; set exception code
        test    al, XSW_OVERFLOW        ; test if overflow
        jnz     short KiXE10            ; if nz, overflow
        test    al, XSW_UNDERFLOW       ; test if underflow
        jnz     short KiXE10            ; if nz, underflow
        test    al, XSW_PRECISION       ; test for inexact result
        jnz     short KiXE10            ; if nz, inexact result
        
;
; The cause of the exception is unknown.
;

KiXE20: mov     r10, TrRip[rbp]         ; set parameter 5 to exception address
        mov     r9, cr4                 ; set parameter 4 to control register 4
        mov     r8, cr0                 ; set parameter 3 to control register 0
        mov     edx, EXCEPTION_NPX_OVERRUN  ; set unexpected trap number
        mov     ecx, UNEXPECTED_KERNEL_MODE_TRAP ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        TRAP_END KiXmmException

        subttl  "Raise Assertion Trap"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an int 2c
;   instruction.
;
; Arguments:
;
;   None.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   arguments are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiRaiseAssertion

        sub     qword ptr MfRip[rsp], 2 ; convert trap to fault 

        GENERATE_TRAP_FRAME             ; generate trap frame

        mov     ecx, STATUS_ASSERTION_FAILURE ; set exception code
        xor     edx, edx                ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiRaiseAssertion

        subttl  "Debug Service Trap"
;++
;
; Routine Description:
;
;   This routine is entered as the result of the execution of an int 2d
;   instruction.
;
; Arguments:
;
;   The standard exception frame is pushed by hardware on the kernel stack.
;   There is no error code for this exception.
;
; Disposition:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   arguments are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiDebugServiceTrap

        inc     qword ptr MfRip[rsp]    ; increment past int 3 instruction

        GENERATE_TRAP_FRAME             ; generate trap frame

        mov     ecx, STATUS_BREAKPOINT  ; set exception code
        mov     edx, 1                  ; set number of parameters
        mov     r9, TrRax[rbp]          ; set parameter 1 value
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiDebugServiceTrap

        subttl  "System Service Call 32-bit"
;++
;
; Routine Description:
;
;   This routine gains control when a system call instruction is executed
;   from 32-bit mode. System service calls from 32-bit code are not supported
;   and this exception is turned into an invalid opcode fault.
;
;   N.B. This routine is never entered from kernel mode and it executed with
;        interrupts disabled.
;
; Arguments:
;
;   The standard exception frame is pushed on the stack.
;
; Return Value:
;
;   A standard trap frame is constructed on the kernel stack, the exception
;   parameters are loaded into registers, and the exception is dispatched via
;   common code.
;
;--

        TRAP_ENTRY KiSystemCall32

        swapgs                          ; swap GS base to kernel PCR
        mov     gs:[PcUserRsp], rsp     ; save user stack pointer
        mov     rsp, gs:[PcRspBase]     ; set kernel stack pointer
        push    KGDT64_R3_DATA or RPL_MASK ; push 32-bit SS selector
        push    gs:[PcUserRsp]          ; push user stack pointer
        push    r11                     ; push previous EFLAGS
        push    KGDT64_R3_CMCODE or RPL_MASK ; push dummy 32-bit CS selector
        push    rcx                     ; push return address
        swapgs                          ; swap GS base to user TEB

        GENERATE_TRAP_FRAME             ; generate trap frame

        mov     ecx, STATUS_ILLEGAL_INSTRUCTION ; set exception code
        xor     edx, edx                ; set number of parameters
        mov     r8, TrRip[rbp]          ; set exception address
        call    KiExceptionDispatch     ; dispatch exception - no return
        nop                             ; fill - do not remove

        TRAP_END KiSystemCall32

        subttl  "System Service Exception Handler"
;++
;
; EXCEPTION_DISPOSITION
; KiSystemServiceHandler (
;    IN PEXCEPTION_RECORD ExceptionRecord,
;    IN PVOID EstablisherFrame,
;    IN OUT PCONTEXT ContextRecord,
;    IN OUT PDISPATCHER_CONTEXT DispatcherContext
;    )
;
; Routine Description:
;
;   This routine is the exception handler for the system service dispatcher.
;
;   If an unwind is being performed and the system service dispatcher is
;   the target of the unwind, then an exception occured while attempting
;   to copy the user's in-memory argument list. Control is transferred to
;   the system service exit by return a continue execution disposition
;   value.
;
;   If an unwind is being performed and the previous mode is user, then
;   bugcheck is called to crash the system. It is not valid to unwind
;   out of a system service into user mode.
;
;   If an unwind is being performed and the previous mode is kernel, then
;   the previous mode field from the trap frame is restored to the thread
;   object.
;
;   If an exception is being raised and the exception PC is the address
;   of the system service dispatcher in-memory argument copy code, then an
;   unwind to the system service exit code is initiated.
;
;   If an exception is being raised and the exception PC is not within
;   the range of the system service dispatcher, and the previous mode is
;   not user, then a continue search disposition value is returned. Otherwise,
;   a system service has failed to handle an exception and bugcheck is
;   called. It is invalid for a system service not to handle all exceptions
;   that can be raised in the service.
;
; Arguments:
;
;   ExceptionRecord (rcx) - Supplies a pointer to an exception record.
;
;   EstablisherFrame (rdx) - Supplies the frame pointer of the establisher
;       of this exception handler.
;
;   ContextRecord (r8) - Supplies a pointer to a context record.
;
;   DispatcherContext (r9) - Supplies a pointer to  the dispatcher context
;       record.
;
; Return Value:
;
;   If bugcheck is called, there is no return from this routine and the
;   system is crashed. If an exception occured while attempting to copy
;   the user in-memory argument list, then there is no return from this
;   routine, and unwind is called. Otherwise, ExceptionContinueSearch is
;   returned as the function value.
;
;--

ShFrame struct
        P1Home  dq ?                    ; parameter home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        P5Home  dq ?                    ;
        P6Home  dq ?                    ;
        Fill    dq ?                    ;
ShFrame ends

        TRAP_ENTRY KiSystemServiceHandler

        alloc_stack (sizeof ShFrame)    ; allocate stack frame

        END_PROLOGUE

        test    dword ptr ErExceptionFlags[rcx], EXCEPTION_UNWIND ; test for unwind
        jnz     KiSH30                  ; if nz, unwind in progress

;
; An exception is in progress.
;
; If the exception PC is the address of the GDI TEB access, then call unwind
; to transfer control to the system service exit code. Otherwise, check if
; the exception PC is the address of the in memory argument copy code for
; the system service dispatcher. If the exception PC is within the range of
; the in memory copy code, then call unwind to transfer control to the
; system service exit code. Otherwise, check if the previous mode is user
; or kernel mode.
;

        lea     rax, KiSystemServiceGdiTebAccess ; get GDI TEB access address
        cmp     rax, ErExceptionAddress[rcx] ; check if address match
        je      short KiSH05            ; if e, address match
        lea     rax, KiSystemServiceCopyStart ; get copy code start address
        cmp     rax, ErExceptionAddress[rcx] ; check if address in range
        ja      short KiSH10            ; if a, address not is range
        lea     rax, KiSystemServiceCopyEnd ; get copy code end address
        cmp     rax, ErExceptionAddress[rcx] ; check if address match
        jbe     short KiSH10            ; if be, address not in range

;
; The exception was raised by the system service dispatcher GDI TEB access
; code or the argument copy code. Unwind to the system service exit with the
; exception status code as the return value.
;

KiSH05: and     ShFrame.P6Home[rsp], 0  ; clear address of history table
        mov     ShFrame.P5Home[rsp], r8 ; set address of context record
        mov     r9d, ErExceptionCode[rcx] ; set return value
        mov     r8, rcx                 ; set address of exception record
        mov     rcx, rdx                ; set target frame address
        lea     rdx, KiSystemServiceExit ; set target IP address
        call    RtlUnwindEx             ; unwind - no return

;
; If the previous mode was kernel mode, then the continue the search for an
; exception handler. Otherwise, bugcheck the system.
;

KiSH10: mov     rax, gs:[PcCurrentThread] ; get current thread address
        cmp     byte ptr ThPreviousMode[rax], KernelMode ; check for kernel mode
        je      short KiSH20            ; if e, previous mode kernel

;
; Previous mode is user mode - bugcheck the system.
;

        xor     r10, r10                ; zero parameter 5
        mov     r9, r8                  ; set context record address
        mov     r8, ErExceptionAddress[rcx] ; set exception address
        mov     edx, ErExceptionCode[rcx] ; set exception code 
        mov     ecx, SYSTEM_SERVICE_EXCEPTION ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return

;
; Previous mode is kernel mode - continue search for a handler.
;

KiSH20: mov     eax, ExceptionContinueSearch ; set return value
        add     rsp, sizeof ShFrame     ; deallocate stack frame
        ret                             ; return

;
; An unwind is in progress.
;
; If a target unwind is being performed, then continue the unwind operation.
; Otherwise, check if the previous mode is user or kernel mode.
;

KiSH30: test    dword ptr ErExceptionFlags[rcx], EXCEPTION_TARGET_UNWIND ; test for target unwind
        jnz     short KiSH20            ; if nz, target unwind in progress

;
; If the previous mode was kernel mode, then restore the previous mode and
; continue the unwind operation. Otherwise, bugcheck the system.
;

        mov     rax, gs:[PcCurrentThread] ; get current thread address
        cmp     byte ptr ThPreviousMode[rax], KernelMode ; check for kernel mode
        je      short KiSH40            ; if e, previous mode kernel

;
; Previous mode was user mode - bugcheck the system.
;

        mov     ecx, SYSTEM_UNWIND_PREVIOUS_USER ; set bugcheck code
        call    KiBugCheckDispatch      ; bugcheck system - no return

;
; Previous mode is kernel mode - restore previous mode and continue unwind
; operation.
;

KiSH40: mov     rcx, ThTrapFrame[rax]   ; get current trap frame address
        mov     rdx, TrTrapFrame + 128[rcx] ; restore previous trap frame address
        mov     ThTrapFrame[rax], rdx   ;
        mov     dl, TrPreviousMode + 128[rcx] ; restore previous mode
        mov     ThPreviousMode[rax], dl ; 
        jmp     short KiSH20            ; finish in common code

        TRAP_END KiSystemServiceHandler

        subttl  "System Service Internal"
;++
;
; VOID
; KiServiceInternal (
;     VOID
;     )
;
; Routine Description:
;
;   This function is called to provide the linkage between an internally
;   called system service and the system service dispatcher.
;
;   N.B. It is known that the previous mode was kernel and interrupts are
;        disabled.
;
; Arguments:
;
;   eax - Supplies the system service number.
;
;   rcx, rdx, r8, and r9 supply the service register arguments.
;
; Return value:
;
;   None.
;
;--

        TRAP_ENTRY KiServiceInternal

        push_frame                      ; mark machine frame
        alloc_stack 8                   ; allocate dummy error code
        push_reg rbp                    ; save standard register
        alloc_stack (KTRAP_FRAME_LENGTH - (7 * 8)) ; allocate fixed frame
        set_frame rbp, 128              ; set frame pointer
        mov     TrRbx[rbp], rbx         ; save nonvolatile registers
        .savereg rbx, (TrRbx + 128)     ;
        mov     TrRdi[rbp], rdi         ;
        .savereg rdi, (TrRdi + 128)     ;
        mov     TrRsi[rbp], rsi         ;
        .savereg rsi, (TrRsi + 128)     ;

        END_PROLOGUE

        sti                             ; enable interrupts
        mov     rbx, gs:[PcCurrentThread] ; get current thread address
        prefetchw ThTrapFrame[rbx]      ; prefetch with write intent
        movzx   edi, byte ptr ThPreviousMode[rbx] ; save previous mode in trap frame
        mov     TrPreviousMode[rbp], dil ;
        mov     byte ptr ThPreviousMode[rbx], KernelMode ; set thread previous mode
        mov     r10, ThTrapFrame[rbx]   ; save previous frame pointer address
        mov     TrTrapFrame[rbp], r10   ;

;        
; N.B. The below code uses an unusual sequence to transfer control. This
;      instruction sequence is required to avoid detection as an epilogue.
;

        lea     r11, KiSystemServiceStart ; get address of service start
        jmp     r11                     ; finish in common code

        TRAP_END  KiServiceInternal

        subttl  "System Service Call 64-bit"
;++
;
; Routine Description:
;
;   This routine gains control when a system call instruction is executed
;   from 64-bit mode. The specified system service is executed by locating
;   its routine address in system service dispatch table and calling the
;   specified function.
;
;   N.B. This routine is never entered from kernel mode and it executed with
;        interrupts disabled.
;
; Arguments:
;
;   eax - Supplies the system service number.
;
; Return Value:
;
;   eax - System service status code.
;
;   r10, rdx, r8, and r9 - Supply the first four system call arguments.
;
;   rcx - Supplies the RIP of the system call.
;
;   r11 - Supplies the previous EFLAGS.
;
;--

        TRAP_ENTRY KiSystemCall64, KiSystemServiceHandler

        swapgs                          ; swap GS base to kernel PCR
        mov     gs:[PcUserRsp], rsp     ; save user stack pointer
        mov     rsp, gs:[PcRspBase]     ; set kernel stack pointer
        push    KGDT64_R3_DATA or RPL_MASK ; push dummy SS selector
        push    gs:[PcUserRsp]          ; push user stack pointer
        push    r11                     ; push previous EFLAGS
        push    KGDT64_R3_CODE or RPL_MASK ; push dummy 64-bit CS selector
        push    rcx                     ; push return address
        mov     rcx, r10                ; set first argument value

        push_frame                      ; mark machine frame
        alloc_stack 8                   ; allocate dummy error code
        push_reg rbp                    ; save standard register
        alloc_stack (KTRAP_FRAME_LENGTH - (7 * 8)) ; allocate fixed frame
        set_frame rbp, 128              ; set frame pointer
        mov     TrRbx[rbp], rbx         ; save nonvolatile registers
        .savereg rbx, (TrRbx + 128)     ;
        mov     TrRdi[rbp], rdi         ;
        .savereg rdi, (TrRdi + 128)     ;
        mov     TrRsi[rbp], rsi         ;
        .savereg rsi, (TrRsi + 128)     ;

        END_PROLOGUE

        mov     byte ptr TrExceptionActive[rbp], 2 ; set service active
        mov     rbx, gs:[PcCurrentThread] ; get current thread address
        prefetchw ThTrapFrame[rbx]      ; prefetch with write intent
        stmxcsr TrMxCsr[rbp]            ; save current MXCSR
        ldmxcsr gs:[PcMxCsr]            ; set default MXCSR
        test    byte ptr ThDebugActive[rbx], TRUE ; test if debug enabled
        mov     word ptr TrDr7[rbp], 0  ; assume debug not enabled
        jz      short KiSS05            ; if z, debug not enabled
        mov     TrRax[rbp], rax         ; save service argument registers
        mov     TrRcx[rbp], rcx         ;
        mov     TrRdx[rbp], rdx         ;
        mov     TrR8[rbp], r8           ;
        mov     TrR9[rbp], r9           ;
        call    KiSaveDebugRegisterState ; save user debug registers
        mov     rax, TrRax[rbp]         ; restore service argument registers
        mov     rcx, TrRcx[rbp]         ;
        mov     rdx, TrRdx[rbp]         ;
        mov     r8, TrR8[rbp]           ;
        mov     r9, TrR9[rbp]           ;

        align   16

KiSS05: sti                             ; enable interrupts

if DBG

        cmp     byte ptr ThPreviousMode[rbx], UserMode ; check previous mode
        je      short @f                ; if e, previous mode set to user
        int     3                       ;
@@:                                     ;

endif

;
; Dispatch system service.
;
;   eax - Supplies the system service number.
;   rbx - Supplies the current thread address.
;   rcx - Supplies the first argument if present.
;   rdx - Supplies the second argument if present.
;   r8 - Supplies the third argument if present.
;   r9 - Supplies the fourth argument if present.
;

        ALTERNATE_ENTRY KiSystemServiceStart

        mov     ThTrapFrame[rbx], rsp   ; set current frame pointer address
        mov     edi, eax                ; copy system service number
        shr     edi, SERVICE_TABLE_SHIFT ; isolate service table number
        and     edi, SERVICE_TABLE_MASK ;
        and     eax, SERVICE_NUMBER_MASK ; isolate service table offset

;
; Repeat system service after attempt to convert to GUI thread.
;

        ALTERNATE_ENTRY KiSystemServiceRepeat

;
; If the specified system service number is not within range, then attempt
; to convert the thread to a GUI thread and retry the service dispatch.
;

        mov     r10, ThServiceTable + ThBase[rbx + rdi] ; get table base address
        cmp     eax, ThServiceTable + ThLimit[rbx + rdi] ; check if valid service
        jae     KiSS50                  ; if ae, not valid service
        movsxd  r11, dword ptr [r10 + rax * 4] ; get system service offset
        add     r10, r11                ; add table base to 

;
; If the service is a GUI service and the GDI user batch queue is not empty,
; then flush the GDI user batch queue.
;

        cmp     edi, SERVICE_TABLE_TEST ; check if GUI service
        jne     short KiSS10            ; if ne, not GUI service
        mov     r11, ThTeb[rbx]         ; get user TEB address

        ALTERNATE_ENTRY KiSystemServiceGdiTebAccess

        cmp     dword ptr TeGdiBatchCount[r11], 0 ; check batch queue depth
        je      short KiSS10            ; if e, batch queue empty
        mov     TrRcx[rbp], rcx         ; save system service arguments
        mov     TrRdx[rbp], rdx         ;
        mov     rbx, r8                 ;
        mov     rdi, r9                 ;
        mov     rsi, r10                ; save system service address
        call    KeGdiFlushUserBatch     ; call flush GDI user batch routine
        mov     rcx, TrRcx[rbp]         ; restore system service arguments
        mov     rdx, TrRdx[rbp]         ;
        mov     r8, rbx                 ;
        mov     r9, rdi                 ;
        mov     r10, rsi                ; restore system service address

;
; Check if system service has any in memory arguments.
;

        align   16

KiSS10: mov     eax, r10d               ; isolate number of in memory arguments
        and     eax, 15                 ;
        jz      KiSS30                  ; if z, no in memory arguments
        sub     r10, rax                ; compute actual function address
        shl     eax, 3                  ; compute argument bytes for dispatch
        lea     rsp, (-14 * 8)[rsp]     ; allocate stack argument area
        lea     rdi, (3 * 8)[rsp]       ; compute copy destination address
        mov     rsi, TrRsp[rbp]         ; get previous stack address
        lea     rsi, (4 * 8)[rsi]       ; compute copy source address
        test    byte ptr TrSegCs[rbp], MODE_MASK ; check if previous mode user
        jz      short KiSS20            ; if z, previous mode kernel
        cmp     rsi, MmUserProbeAddress ; check if source address in range
        cmovae  rsi, MmUserProbeAddress ; if ae, reset copy source address

;
; The following code is very carefully optimized so there is exactly 8 bytes
; of code for each argument move.
;
; N.B. The source and destination registers are biased by 8 bytes.
;
; N.B. Four additional arguments are specified in registers.
;

        align   16

KiSS20: lea     r11, KiSystemServiceCopyEnd ; get copy ending address
        sub     r11, rax                ; substract number of bytes to copy
        jmp     r11                     ; 

        align   16

        ALTERNATE_ENTRY KiSystemServiceCopyStart

        mov     rax, 112[rsi]           ; copy fourteenth argument
        mov     112[rdi], rax           ;
        mov     rax, 104[rsi]           ; copy thirteenth argument
        mov     104[rdi], rax           ;
        mov     rax, 96[rsi]            ; copy twelfth argument
        mov     96[rdi], rax            ;
        mov     rax, 88[rsi]            ; copy eleventh argument
        mov     88[rdi], rax            ;
        mov     rax, 80[rsi]            ; copy tenth argument
        mov     80[rdi], rax            ;
        mov     rax, 72[rsi]            ; copy nineth argument
        mov     72[rdi], rax            ;
        mov     rax, 64[rsi]            ; copy eighth argument
        mov     64[rdi], rax            ;
        mov     rax, 56[rsi]            ; copy seventh argument
        mov     56[rdi], rax            ;
        mov     rax, 48[rsi]            ; copy sixth argument
        mov     48[rdi], rax            ;
        mov     rax, 40[rsi]            ; copy fifth argument
        mov     40[rdi], rax            ;
        mov     rax, 32[rsi]            ; copy fourth argument
        mov     32[rdi], rax            ;
        mov     rax, 24[rsi]            ; copy third argument
        mov     24[rdi], rax            ;
        mov     rax, 16[rsi]            ; copy second argument
        mov     16[rdi], rax            ;
        mov     rax, 8[rsi]             ; copy first argument
        mov     8[rdi], rax             ;

        ALTERNATE_ENTRY KiSystemServiceCopyEnd

;
; Call system service.
;

KiSS30:                                 ;

        call    r10                     ; call system service
        inc     dword ptr gs:[PcSystemCalls] ; increment number of system calls

;
; System service exit.
;
;   eax - Supplies the system service status.
;
;   rbp - Supplies the address of the trap frame.
;
; N.B. It is possible that the values of rsi, rdi, and rbx have been destroyed
;      and, therefore, they cannot be used in the system service exit sequence.
;      This can happen on a failed attempt to raise an exception via a system
;      service.
;

        ALTERNATE_ENTRY KiSystemServiceExit

        mov     rbx, TrRbx[rbp]         ; restore extra registers
        mov     rdi, TrRdi[rbp]         ;
        mov     rsi, TrRsi[rbp]         ;
        test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode user
        jz      KiSS40                  ; if z, previous mode not user

;
; Check if the current IRQL is above passive level.
;

if DBG

        xor     r9, r9                  ; clear parameter value
        mov     r8, cr8                 ; get current IRQL
        or      r8, r8                  ; check if IRQL is passive level
        mov     ecx, IRQL_GT_ZERO_AT_SYSTEM_SERVICE ; set bugcheck code
        jnz     short KiSS33            ; if nz, IRQL not passive level

;
; Check if kernel APCs are disabled or a process is attached.
;

        mov     rcx, gs:[PcCurrentThread] ; get current thread address
        movzx   r8d, byte ptr ThApcStateIndex[rcx] ; get APC state index
        mov     r9d, ThCombinedApcDisable[rcx] ; get kernel APC disable
        or      r9d, r9d                ; check if kernel APCs disabled
        jnz     short KiSS32            ; if nz, Kernel APCs disabled
        or      r8d, r8d                ; check if process attached
        jz      short KiSS37            ; if z, process not attached
KiSS32: mov     ecx, APC_INDEX_MISMATCH ; set bugcheck code
KiSS33: mov     rdx, TrRip[rbp]         ; set system call address
        mov     r10, rbp                ; set trap frame address
        call    KiBugCheckDispatch      ; bugcheck system - no return

endif

KiSS37: RESTORE_TRAP_STATE <Service>    ; restore trap state/exit to user mode

KiSS40: mov     rcx, gs:[PcCurrentThread] ; get current thread address
        mov     rdx, TrTrapFrame[rbp]   ; restore previous trap frame address
        mov     ThTrapFrame[rcx], rdx   ;
        mov     dl, TrPreviousMode[rbp] ; restore previous mode
        mov     ThPreviousMode[rcx], dl ;

        RESTORE_TRAP_STATE <Kernel>     ; restore trap state/exit to kernel mode

;
; The specified system service number is not within range. Attempt to convert
; the thread to a GUI thread if the specified system service is a GUI service
; and the thread has not already been converted to a GUI thread.
;
; N.B. Convert to GUI thread will not overwrite the parameter home area.
;

KiSS50: cmp     edi, SERVICE_TABLE_TEST ; check if GUI service
        jne     KiSS60                  ; if ne, not GUI service
        mov     TrP1Home[rbp], eax      ; save system service number
        mov     TrP2Home[rbp], rcx      ; save system service arguments
        mov     TrP3Home[rbp], rdx      ;
        mov     TrP4Home[rbp], r8       ;
        mov     TrP5[rbp], r9           ;
        call    KiConvertToGuiThread    ; attempt to convert to GUI thread
        or      eax, eax                ; check if service was successful
        mov     eax, TrP1Home[rbp]      ; restore system service number
        mov     rcx, TrP2Home[rbp]      ; restore system service arguments
        mov     rdx, TrP3Home[rbp]      ;
        mov     r8, TrP4Home[rbp]       ;
        mov     r9, TrP5[rbp]           ;
        mov     ThTrapFrame[rbx], rsp   ; set current frame pointer address
        jz      KiSystemServiceRepeat   ; if z, successful conversion to GUI

;
; The conversion to a GUI thread failed. The correct return value is encoded
; in a byte table indexed by the service number that is at the end of the
; service address table. The encoding is as follows:
;
;   0 - return 0.
;   -1 - return -1.
;   1 - return status code.
;

        lea     rdi, KeServiceDescriptorTableShadow + SdLength ;
        mov     esi, SdLimit[rdi]       ; get service table limit
        mov     rdi, SdBase[rdi]        ; get service table base
        lea     rdi, [rdi + rsi * 4]    ; get ending service table address
        movsx   eax, byte ptr [rdi + rax] ; get status byte value
        or      eax, eax                ; check for 0 or - 1
        jle     KiSystemServiceExit     ; if le, return status byte value
KiSS60: mov     eax, STATUS_INVALID_SYSTEM_SERVICE ; set return status
        jmp     KiSystemServiceExit     ; finish in common code

        TRAP_END KiSystemCall64

        subttl  "Common Bugcheck Dispatch"
;++
;
; Routine Description:
;
;   This routine allocates an exception frame on stack, saves nonvolatile
;   machine state, and calls the system bugcheck code.
;
;   N.B. It is the responsibility of the caller to initialize the exception
;        record.
;
; Arguments:
;
;   ecx - Supplies the bugcheck code.
;
;   rdx to r10 - Supplies the bugcheck parameters.
;
; Return Value:
;
;    There is no return from this function.
;
;--

        TRAP_ENTRY KiBugCheckDispatch

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

        mov     ExP5[rsp], r10          ; save parameter 5
        call    KeBugCheckEx            ; bugcheck system - not return
        nop                             ; fill - do not remove

        TRAP_END KiBugCheckDispatch

        subttl  "Common Exception Dispatch"
;++
;
; Routine Description:
;
;   This routine allocates an exception frame on stack, saves nonvolatile
;   machine state, and calls the system exception dispatcher.
;
;   N.B. It is the responsibility of the caller to initialize the exception
;        record.
;
; Arguments:
;
;   ecx - Supplies the exception code.
;
;   edx - Supplies the number of parameters.
;
;   r8 - Supplies the exception address.
;
;   r9 - r11 - Supply the exception  parameters.
;
;   rbp - Supplies a pointer to the trap frame.
;
;   rsp - Supplies a pointer to the trap frame.
;
; Return Value:
;
;    There is no return from this function.
;
;--

        TRAP_ENTRY KiExceptionDispatch

        GENERATE_EXCEPTION_FRAME        ; generate exception frame

        lea     rax, ExExceptionRecord[rsp] ; get exception record address
        mov     ErExceptionCode[rax], ecx ; set exception code
        xor     ecx, ecx                ;
        mov     dword ptr ErExceptionFlags[rax], ecx ; clear exception flags
        mov     ErExceptionRecord[rax], rcx ; clear exception record address
        mov     ErExceptionAddress[rax], r8 ; set exception address
        mov     ErNumberParameters[rax], edx ; set number of parameters
        mov     ErExceptionInformation[rax], r9 ; set exception parameters
        mov     ErExceptionInformation + 8[rax], r10 ;
        mov     ErExceptionInformation + 16[rax], r11 ;
        mov     r9b, TrSegCs[rbp]       ; isolate previous mode
        and     r9b, MODE_MASK          ;
        mov     byte ptr ExP5[rsp], TRUE ; set first chance parameter
        lea     r8, (-128)[rbp]         ; set trap frame address
        mov     rdx, rsp                ; set exception frame address
        mov     rcx, rax                ; set exception record address
        call    KiDispatchException     ; dispatch exception

        subttl  "Common Exception Exit"
;++
;
; Routine Description:
;
;   This routine is called to exit an exception.
;
;   N.B. This transfer of control occurs from:
;
;        1. a fall through from above.
;        2. the exit from a continue system service.
;        3. the exit form a raise exception system service.
;        4. the exit into user mode from thread startup.
;
;   N.B. Control is transferred to this code via a jump.
;
; Arguments:
;
;   rbp - Supplies the address of the trap frame.
;
;   rsp - Supplies the address of the exception frame.
;
; Return Value:
;
;   Function does not return.
;
;--

        ALTERNATE_ENTRY KiExceptionExit

        RESTORE_EXCEPTION_STATE <NoPop> ; restore exception state/deallocate

        RESTORE_TRAP_STATE <Volatile>   ; restore trap state and exit

        TRAP_END KiExceptionDispatch

        subttl  "System Service Linkage"
;++
;
; VOID
; KiServiceLinkage (
;     VOID
;     )
;
; Routine Description:
;
;   This is a dummay function that only exists to make trace back through
;   a kernel mode to kernel mode system call work.
; Arguments:
;
;   None.
;
; Return value:
;
;   None.
;
;--

        TRAP_ENTRY KiServiceLinkage

        .allocstack 0

        END_PROLOGUE

        ret                             ;

        TRAP_END  KiServiceLinkage

        end

