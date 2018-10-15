        TITLE  "Interrupt Object Support Routines"
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
;    intsup.asm
;
; Abstract:
;
;    This module implements the platform specific code to support interrupt
;    objects. It contains the interrupt dispatch code and the code template
;    that gets copied into an interrupt object.
;
;--

include ksamd64.inc

        extern  ExpInterlockedPopEntrySListEnd:proc
        extern  ExpInterlockedPopEntrySListResume:proc
        extern  KeBugCheck:proc
        extern  KeLastBranchMSR:dword
        extern  KiBugCheckDispatch:proc
        extern  KiCheckForSListAddress:proc
        extern  KiDpcInterruptBypass:proc
        extern  KiIdleSummary:qword
        extern  KiInitiateUserApc:proc
        extern  KiRestoreDebugRegisterState:proc
        extern  KiSaveDebugRegisterState:proc
        extern  PerfInfoLogInterrupt:proc
        extern  __imp_HalRequestSoftwareInterrupt:qword

        subttl  "Synchronize Execution"
;++
;
; BOOLEAN
; KeSynchronizeExecution (
;     IN PKINTERRUPT Interrupt,
;     IN PKSYNCHRONIZE_ROUTINE SynchronizeRoutine,
;     IN PVOID SynchronizeContext
;     )
;
; Routine Description:
;
;   This function synchronizes the execution of the specified routine with
;   the execution of the service routine associated with the specified
;   interrupt object.
;
; Arguments:
;
;   Interrupt (rcx) - Supplies a pointer to an interrupt object.
;
;   SynchronizeRoutine (rdx) - Supplies a pointer to the function whose
;       execution is to be synchronized with the execution of the service
;       routine associated with the specified interrupt object.
;
;   SynchronizeContext (r8) - Supplies a context pointer which is to be
;       passed to the synchronization function as a parameter.
;
; Return Value:
;
;   The value returned by the synchronization routine is returned as the
;   function value.
;
;--

SyFrame struct
        P1Home  dq ?                    ; parameter home address
        P2Home  dq ?
        P3Home  dq ?
        P4Home  dq ?
        OldIrql dd ?                    ; saved IRQL
        Fill0   dd ?
        Fill1   dq ?                    ; stack alignment
        SavedRsi dq ?                   ; saved nonvolatile register
SyFrame ends

        NESTED_ENTRY KeSynchronizeExecution, _TEXT$00

        alloc_stack (sizeof SyFrame)    ; allocate stack frame
        save_reg rsi, SyFrame.SavedRsi  ; save nonvolatile register

        END_PROLOGUE

        mov     rsi, InActualLock[rcx]  ; save interrupt object lock
        movzx   ecx, byte ptr InSynchronizeIrql[rcx] ; get synchronization IRQL

        RaiseIrql                       ; raise IRQL to synchronization level

        mov     rcx, r8                 ; set synchronization context
        mov     SyFrame.OldIrql[rsp], eax ; save previous IRQL

        AcquireSpinLock [rsi]           ; acquire interrupt spin lock

        call    rdx                     ; call synchronization routine

        ReleaseSpinlock [rsi]           ; release interrupt spin lock

        mov     ecx, SyFrame.OldIrql[rsp] ; get previous IRQL

        LowerIrql                       ; lower IRQL to previous level

        mov     rsi, SyFrame.SavedRsi[rsp] ; restore nonvolatile register
        add     rsp, sizeof SyFrame     ; deallocate stack frame
        ret                             ;

        NESTED_END KeSynchronizeExecution, _TEXT$00

        subttl  "Interrupt Exception Handler"
;++
;
; EXCEPTION_DISPOSITION
; KiInterruptHandler (
;    IN PEXCEPTION_RECORD ExceptionRecord,
;    IN PVOID EstablisherFrame,
;    IN OUT PCONTEXT ContextRecord,
;    IN OUT PDISPATCHER_CONTEXT DispatcherContext
;    )
;
; Routine Description:
;
;   This routine is the exception handler for the interrupt dispatcher. The
;   dispatching or unwinding of an exception across an interrupt causes a
;   bugcheck.
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
;   There is no return from this routine.
;
;--

IhFrame struct
        P1Home  dq ?                    ; parameter home address
        P2Home  dq ?
        P3Home  dq ?
        P4Home  dq ?
        Fill    dq ?
IhFrame ends

        NESTED_ENTRY KiInterruptHandler, _TEXT$00

        alloc_stack (sizeof IhFrame)    ; allocate stack frame

        END_PROLOGUE

        mov     r10, ErExceptionAddress[rcx] ; set exception address
        xor     r9, r9                  ; zero remaining arguments
        xor     r8, r8                  ;
        xor     edx, edx                ;
        test    dword ptr ErExceptionFlags[rcx], EXCEPTION_UNWIND ; test for unwind
        mov     ecx, INTERRUPT_UNWIND_ATTEMPTED ; set bugcheck code
        jnz     short KiIH10            ; if nz, unwind in progress
        mov     ecx, INTERRUPT_EXCEPTION_NOT_HANDLED ; set bugcheck code
KiIH10: call    KiBugCheckDispatch      ; bugcheck system - no return
        nop                             ; fill - do not remove

        NESTED_END KiInterruptHandler, _TEXT$00

        subttl  "Chained Dispatch"
;++
;
; VOID
; KiChainedDispatch (
;     VOID
;     );
;
; Routine Description:
;
;   This routine is entered as the result of an interrupt being generated
;   via a vector that is connected to more than one interrupt object.
;
; Arguments:
;
;   rbp - Supplies a pointer to the interrupt object.
;
; Return Value:
;
;   None.
;
;--

        NESTED_ENTRY KiChainedDispatch, _TEXT$00, KiInterruptHandler

        .pushframe code                 ; mark machine frame
        .pushreg rbp                    ; mark nonvolatile register push

        GENERATE_INTERRUPT_FRAME        ; generate interrupt frame

        movzx   ecx, byte ptr InIrql[rsi] ; set interrupt IRQL

	ENTER_INTERRUPT	<NoEOI>         ; raise IRQL and enable interrupts

        call    KiScanInterruptObjectList ; scan interrupt object list

        EXIT_INTERRUPT                  ; do EOI, lower IRQL, and restore state

        NESTED_END KiChainedDispatch, _TEXT$00

        subttl  "Scan Interrupt Object List"
;++
;
; Routine Description:
;
;   This routine scans the list of interrupt objects for chained interrupt
;   dispatch. If the mode of the interrupt is latched, then a complete scan
;   of the list must be performed. Otherwise, the scan can be cut short as
;   soon as an interrupt routine returns
;
; Arguments:
;
;   rsi - Supplies a pointer to the interrupt object.
;
; Implicit Arguments:
;
;   rbp - Supplies the address of the interrupt trap frame.
;
; Return Value:
;
;   None.
;
;--

SiFrame struct
        P1Home  dq ?                    ; interrupt object parameter
        P2Home  dq ?                    ; service context parameter
        P3Home  dq ?                    ; Per calling standard
        P4Home  dq ?                    ;  "
        Return  db ?                    ; service routine return value
        Fill    db 15 dup (?)           ; fill
        SavedRbx dq ?                   ; saved register RBX
        SavedRdi dq ?                   ; saved register RDI
        SavedR12 dq ?                   ; saved register RSI
SiFrame ends

        NESTED_ENTRY KiScanInterruptObjectList, _TEXT$00

        alloc_stack (sizeof SiFrame)    ; allocate stack frame
        save_reg rbx, SiFrame.SavedRbx  ; save nonvolatile registers
        save_reg rdi, SiFrame.SavedRdi  ;
        save_reg r12, SiFrame.SavedR12  ;

        END_PROLOGUE

        lea     rbx, InInterruptListEntry[rsi] ; get list head address
        mov     r12, rbx                ; set address of first list entry

;
; If interrupt logging is enabled, then store the initial time stamp value.
;

        mov     rax, gs:[PcPerfGlobalGroupMask]; get global mask address
        test    rax, rax                ; test if logging enabled
        mov     qword ptr TrTimeStamp[rbp], 0 ; clear time stamp value
        je      short KiSI05            ; if e, logging not enabled
        test    qword ptr PERF_INTERRUPT_OFFSET[rax], PERF_INTERRUPT_FLAG ; check flag
        jz      short KiSI05            ; if z, interrupt logging not enabled
        rdtsc                           ; read time stamp counter
        shl     rdx, 32                 ; combine low and high parts
        or      rax, rdx                ;
        mov     TrTimeStamp[rbp], rax   ; save starting time stamp value

;
; Scan the list of connected interrupt objects and call the service routine.
;

KiSI05: xor     edi, edi                ; clear interrupt handled flag
KiSI10: sub     r12, InInterruptListEntry ; compute interrupt object address
        movzx   ecx, byte ptr InSynchronizeIrql[r12] ; get synchronization IRQL
        mov     r11, InActualLock[r12]  ; get actual spin lock address
        cmp     cl, InIrql[rsi]         ; check if equal interrupt IRQL
        je      short KiSI20            ; if e, IRQL levels equal

        SetIrql                         ; set IRQL to synchronization level

KiSI20: AcquireSpinLock [r11]           ; acquire interrupt spin lock

        mov     rcx, r12                ; set interrupt object parameter
        mov     rdx, InServiceContext[r12] ; set context parameter
        call    qword ptr InServiceRoutine[r12] ; call interrupt service routine
        mov     r11, InActualLock[r12]  ; get actual spin lock address
        mov     SiFrame.Return[rsp], al ; save return value
        movzx   ecx, byte ptr InIrql[rsi] ; get interrupt IRQL

        ReleaseSpinLock [r11]           ; release interrupt spin lock

        cmp     cl, InSynchronizeIrql[r12] ; check if equal synchronization IRQL
        je      short KiSI25            ; if e, IRQL levels equal

        SetIrql                         ; set IRQL to interrupt level

;
; If interrupt logging is enabled, then log the interrupt.
;

KiSI25: cmp     qword ptr TrTimeStamp[rbp], 0 ; check if interrupt logging enabled
        je      short KiSI30            ; if e, interrupt logging not enabled
        mov     r8, TrTimeStamp[rbp]    ; set initial time stamp value
        movzx   edx, SiFrame.Return[rsp] ; set interrupt return value
        mov     rcx, InServiceRoutine[r12] ; set interrupt service routine address
        call    PerfInfoLogInterrupt    ; log interrupt
        rdtsc                           ; read time stamp counter
        shl     rdx, 32                 ; combine low and high parts
        or      rax, rdx                ;
        mov     TrTimeStamp[rbp], rax   ; save starting time stamp value
KiSI30: test    byte ptr SiFrame.Return[rsp], 0ffh ; test if interrupt handled
        jz      short KiSI40            ; if z, interrupt not handled
        cmp     word ptr InMode[r12], InLatched ; check if latched interrupt
        jne     short KiSI50            ; if ne, not latched interrupt
        inc     edi                     ; indicate latched interrupt handled
KiSI40: mov     r12, InInterruptListEntry[r12] ; get next interrupt list entry
        cmp     r12, rbx                ; check if end of list
        jne     KiSI10                  ; if ne, not end of list

;
; The complete interrupt object list has been scanned. This can only happen
; if the interrupt is a level sensitive interrupt and no interrupt was handled
; or the interrupt is a latched interrupt. Therefore, if any interrupt was
; handled it was a latched interrupt and the list needs to be scanned again
; to ensure that no interrupts are lost.
;

        test    edi, edi                ; test if any interrupts handled
        jnz     KiSI05                  ; if nz, latched interrupt handled
KiSI50: mov     rbx, SiFrame.SavedRbx[rsp] ; restore nonvolatile register
        mov     rdi, SiFrame.SavedRdi[rsp] ;
        mov     r12, SiFrame.SavedR12[rsp] ;
        add     rsp, (sizeof SiFrame)   ; deallocate stack frame
        ret                             ;

        NESTED_END KiscanInterruptObjectList, _TEXT$00

        subttl  "Interrupt Dispatch"
;++
;
; Routine Description:
;
;   This routine is entered as the result of an interrupt being generated
;   via a vector that is connected to an interrupt object. Its function is
;   to directly call the specified interrupt service routine.
;
;   This routine is identical to interrupt dispatch no lock except that
;   the interrupt spinlock is taken.
;
;   N.B. On entry rbp and rsi have been saved on the stack.
;
; Arguments:
;
;   rbp - Supplies a pointer to the interrupt object.
;
; Return Value:
;
;   None.
;
;--

        NESTED_ENTRY KiInterruptDispatch, _TEXT$00, KiInterruptHandler

        .pushframe code                 ; mark machine frame
        .pushreg rbp                    ; mark nonvolatile register push

        GENERATE_INTERRUPT_FRAME        ; generate interrupt frame

;
; N.B. It is possible for a interrupt to occur at an IRQL that is lower
;      than the current IRQL. This happens when the IRQL raised and at
;      the same time an interrupt request is granted.
;

        movzx   ecx, byte ptr InIrql[rsi] ; set interrupt IRQL

	ENTER_INTERRUPT <NoEOI>         ; raise IRQL and enable interrupts

        lea     rax, (-128)[rbp]        ; set trap frame address
        mov     InTrapFrame[rsi], rax   ;

;
; If interrupt logging is enabled, then store the initial time stamp value.
;

        mov     rax, gs:[PcPerfGlobalGroupMask]; get global mask address
        test    rax, rax                ; test if logging enabled
        mov     qword ptr TrTimeStamp[rbp], 0 ; clear time stamp value
        je      short KiID10            ; if e, logging not enabled
        test    qword ptr PERF_INTERRUPT_OFFSET[rax], PERF_INTERRUPT_FLAG ; check flag
        jz      short KiID10            ; if z, interrupt logging not enabled
        rdtsc                           ; read time stamp counter
        shl     rdx, 32                 ; combine low and high parts
        or      rax, rdx                ;
        mov     TrTimeStamp[rbp], rax   ; save starting time stamp value
        mov     rax, InServiceRoutine[rsi] ; save interrupt service routine address
        mov     TrP5[rbp], rax          ;

;
; Dispatch interrupt.
;

KiID10: mov     rcx, rsi                ; set address of interrupt object
        mov     rsi, InActualLock[rsi]  ; get address of interrupt lock
        mov     rdx, InServiceContext[rcx] ; set service context

        AcquireSpinLock [rsi]           ; acquire interrupt spin lock

        call    qword ptr InServiceRoutine[rcx] ; call interrupt service routine

        ReleaseSpinLock [rsi]           ; release interrupt spin lock

;
; If interrupt logging is enabled, then log the interrupt.
;

        cmp     qword ptr TrTimeStamp[rbp], 0 ; check if interrupt logging enabled
        je      short KiID20            ; if e, interrupt logging not enabled
        mov     r8, TrTimeStamp[rbp]    ; set initial time stamp value
        mov     edx, TRUE               ; set interrupt return value
        mov     rcx, TrP5[rbp]          ; set interrupt service routine address
        call    PerfInfoLogInterrupt    ; log interrupt

KiID20: EXIT_INTERRUPT                  ; do EOI, lower IRQL, and restore state

        NESTED_END KiInterruptDispatch, _TEXT$00

        subttl  "Interrupt Dispatch, With Last Branch Control"
;++
;
; Routine Description:
;
;   This routine is entered as the result of an interrupt being generated
;   via a vector that is connected to an interrupt object. Its function is
;   to save the last branch control MSR, disable last branch recording, and
;   directly call the specified interrupt service routine.
;
;   This routine is identical to interrupt dispatch except that no spinlock
;   is taken.
;
;   N.B. On entry rbp and rsi have been saved on the stack.
;
;   N.B. This routine is only executed on EM64T based systems.
;
; Arguments:
;
;   rbp - Supplies a pointer to the interrupt object.
;
; Return Value:
;
;   None.
;
;--

        NESTED_ENTRY KiInterruptDispatchLBControl, _TEXT$00, KiInterruptHandler

        .pushframe code                 ; mark machine frame
        .pushreg rbp                    ; mark nonvolatile register push

        GENERATE_INTERRUPT_FRAME <>, <>, <>, <LBranch> ; generate interrupt frame

;
; N.B. It is possible for a interrupt to occur at an IRQL that is lower
;      than the current IRQL. This happens when the IRQL raised and at
;      the same time an interrupt request is granted.
;

        movzx   ecx, byte ptr InIrql[rsi] ; set interrupt IRQL

        ENTER_INTERRUPT <NoEOI>         ; raise IRQL and enable interrupts

        lea     rax, (-128)[rbp]        ; set trap frame address
        mov     InTrapFrame[rsi], rax   ;

;
; If interrupt logging is enabled, then store the initial time stamp value.
;

        mov     rax, gs:[PcPerfGlobalGroupMask]; get global mask address
        test    rax, rax                ; test if logging enabled
        mov     qword ptr TrTimeStamp[rbp], 0 ; clear time stamp value
        je      short KiLB10            ; if e, logging not enabled
        test    qword ptr PERF_INTERRUPT_OFFSET[rax], PERF_INTERRUPT_FLAG ; check flag
        jz      short KiLB10            ; if z, interrupt logging not enabled
        rdtsc                           ; read time stamp counter
        shl     rdx, 32                 ; combine low and high parts
        or      rax, rdx                ;
        mov     TrTimeStamp[rbp], rax   ; save starting time stamp value
        mov     rax, InServiceRoutine[rsi] ; save interrupt service routine address
        mov     TrP5[rbp], rax          ;

;
; Dispatch interrupt.
;

KiLB10: mov     rcx, rsi                ; set address of interrupt object
        mov     rdx, InServiceContext[rsi] ; set service context
        call    qword ptr InServiceRoutine[rsi] ; call interrupt service routine

;
; If interrupt logging is enabled, then log the interrupt.
;

        cmp     qword ptr TrTimeStamp[rbp], 0 ; check if interrupt logging enabled
        je      short KiLB20            ; if e, interrupt logging not enabled
        mov     r8, TrTimeStamp[rbp]    ; set initial time stamp value
        mov     edx, TRUE               ; set interrupt return value
        mov     rcx, TrP5[rbp]          ; set interrupt service routine address
        call    PerfInfoLogInterrupt    ; log interrupt

KiLB20: EXIT_INTERRUPT <>, <>, <>, <>, <LBranch> ; do EOI, lower IRQL, and restore state

        NESTED_END KiInterruptDispatchLBControl, _TEXT$00

        subttl  "Interrupt Dispatch, No Lock"
;++
;
; Routine Description:
;
;   This routine is entered as the result of an interrupt being generated
;   via a vector that is connected to an interrupt object. Its function is
;   to directly call the specified interrupt service routine.
;
;   This routine is identical to interrupt dispatch except that no spinlock
;   is taken.
;
;   N.B. On entry rbp and rsi have been saved on the stack.
;
; Arguments:
;
;   rbp - Supplies a pointer to the interrupt object.
;
; Return Value:
;
;   None.
;
;--

        NESTED_ENTRY KiInterruptDispatchNoLock, _TEXT$00, KiInterruptHandler

        .pushframe code                 ; mark machine frame
        .pushreg rbp                    ; mark nonvolatile register push

        GENERATE_INTERRUPT_FRAME        ; generate interrupt frame

;
; N.B. It is possible for a interrupt to occur at an IRQL that is lower
;      than the current IRQL. This happens when the IRQL raised and at
;      the same time an interrupt request is granted.
;

        movzx   ecx, byte ptr InIrql[rsi] ; set interrupt IRQL

        ENTER_INTERRUPT <NoEOI>         ; raise IRQL and enable interrupts

        lea     rax, (-128)[rbp]        ; set trap frame address
        mov     InTrapFrame[rsi], rax   ;

;
; If interrupt logging is enabled, then store the initial time stamp value.
;

        mov     rax, gs:[PcPerfGlobalGroupMask]; get global mask address
        test    rax, rax                ; test if logging enabled
        mov     qword ptr TrTimeStamp[rbp], 0 ; clear time stamp value
        je      short KiNL10            ; if e, logging not enabled
        test    qword ptr PERF_INTERRUPT_OFFSET[rax], PERF_INTERRUPT_FLAG ; check flag
        jz      short KiNL10            ; if z, interrupt logging not enabled
        rdtsc                           ; read time stamp counter
        shl     rdx, 32                 ; combine low and high parts
        or      rax, rdx                ;
        mov     TrTimeStamp[rbp], rax   ; save starting time stamp value
        mov     rax, InServiceRoutine[rsi] ; save interrupt service routine address
        mov     TrP5[rbp], rax          ;

;
; Dispatch interrupt.
;

KiNL10: mov     rcx, rsi                ; set address of interrupt object
        mov     rdx, InServiceContext[rsi] ; set service context
        call    qword ptr InServiceRoutine[rsi] ; call interrupt service routine

;
; If interrupt logging is enabled, then log the interrupt.
;

        cmp     qword ptr TrTimeStamp[rbp], 0 ; check if interrupt logging enabled
        je      short KiNL20            ; if e, interrupt logging not enabled
        mov     r8, TrTimeStamp[rbp]    ; set initial time stamp value
        mov     edx, TRUE               ; set interrupt return value
        mov     rcx, TrP5[rbp]          ; set interrupt service routine address
        call    PerfInfoLogInterrupt    ; log interrupt

KiNL20: EXIT_INTERRUPT                  ; do EOI, lower IRQL, and restore state

        NESTED_END KiInterruptDispatchNoLock, _TEXT$00

        subttl  "Interrupt Dispatch, No EOI"
;++
;
; Routine Description:
;
;   This routine is entered as the result of an interrupt being generated
;   via a vector that is connected to an interrupt object. Its function is
;   to directly call the specified interrupt service routine.
;
;   This routine is identical to KiInterruptDispatchNolock except that no 
;   EOI is performed.
;
;   N.B. On entry rbp and rsi have been saved on the stack.
;
; Arguments:
;
;   rbp - Supplies a pointer to the interrupt object.
;
; Return Value:
;
;   None.
;
;--

        NESTED_ENTRY KiInterruptDispatchNoEOI, _TEXT$00, KiInterruptHandler

        .pushframe code                 ; mark machine frame
        .pushreg rbp                    ; mark nonvolatile register push

        GENERATE_INTERRUPT_FRAME        ; generate interrupt frame

;
; N.B. It is possible for a interrupt to occur at an IRQL that is lower
;      than the current IRQL. This happens when the IRQL raised and at
;      the same time an interrupt request is granted.
;

        movzx   ecx, byte ptr InIrql[rsi] ; set interrupt IRQL

        ENTER_INTERRUPT <NoEOI>         ; raise IRQL and enable interrupts

        lea     rax, (-128)[rbp]        ; set trap frame address
        mov     InTrapFrame[rsi], rax   ;

;
; If interrupt logging is enabled, then store the initial time stamp value.
;

        mov     rax, gs:[PcPerfGlobalGroupMask]; get global mask address
        test    rax, rax                ; test if logging enabled
        mov     qword ptr TrTimeStamp[rbp], 0 ; clear time stamp value
        je      short KiNE10            ; if e, logging not enabled
        test    qword ptr PERF_INTERRUPT_OFFSET[rax], PERF_INTERRUPT_FLAG ; check flag
        jz      short KiNE10            ; if z, interrupt logging not enabled
        rdtsc                           ; read time stamp counter
        shl     rdx, 32                 ; combine low and high parts
        or      rax, rdx                ;
        mov     TrTimeStamp[rbp], rax   ; save starting time stamp value
        mov     rax, InServiceRoutine[rsi] ; save interrupt service routine address
        mov     TrP5[rbp], rax          ;

;
; Dispatch interrupt.
;

KiNE10: mov     rcx, rsi                ; set address of interrupt object
        mov     rdx, InServiceContext[rsi] ; set service context
        call    qword ptr InServiceRoutine[rsi] ; call interrupt service routine

;
; If interrupt logging is enabled, then log the interrupt.
;

        cmp     qword ptr TrTimeStamp[rbp], 0 ; check if interrupt logging enabled
        je      short KiNE20            ; if e, interrupt logging not enabled
        mov     r8, TrTimeStamp[rbp]    ; set initial time stamp value
        mov     edx, TRUE               ; set interrupt return value
        mov     rcx, TrP5[rbp]          ; set interrupt service routine address
        call    PerfInfoLogInterrupt    ; log interrupt

KiNE20: EXIT_INTERRUPT <NoEOI>          ; lower IRQL, and restore state

        NESTED_END KiInterruptDispatchNoEOI, _TEXT$00

        subttl  "Interrupt Template"
;++
;
; Routine Description:
;
;   This routine is a template that is copied into each interrupt object.
;   Its function is to save volatile machine state, compute the interrupt
;   object address, and transfer control to the appropriate interrupt
;   dispatcher.
;
;   N.B. Interrupts are disabled on entry to this routine.
;
; Arguments:
;
;    None.
;
; Return Value:
;
;    N.B. Control does not return to this routine. The respective interrupt
;         dispatcher dismisses the interrupt directly.
;
;--

        LEAF_ENTRY KiInterruptTemplate, _TEXT$00

        push    rax                     ; push dummy vector number
        push    rbp                     ; save nonvolatile register
        lea     rbp, KiInterruptTemplate - InDispatchCode ; get interrupt object address
        jmp     qword ptr InDispatchAddress[rbp] ; finish in common code

        LEAF_END KiInterruptTemplate, _TEXT$00

        subttl  "Spurious Interrupt Template"
;++
;
; Routine Description:
;
;   This routine is a template that is copied into the spurious interrupt
;   objects.  Its function is to immediately iret.
;
;   N.B. Interrupts are disabled on entry to this routine.
;
; Arguments:
;
;    None.
;
; Return Value:
;
;    None.
;
;--

        LEAF_ENTRY KiSpuriousInterruptTemplate, _TEXT$00

;
; N.B. KINTERRUPT.ServiceCount and KINTERRUPT.DispatchCount are treated as a
; single quadword field and a 64-bit increment is performed.
;

        db      048h, 0FFh, 005h
        dd      InServiceCount - (InDispatchCode + 7)

        iretq

        LEAF_END KiSpuriousInterruptTemplate, _TEXT$00


        subttl  "Unexpected Interrupt Code"
;++
;
; RoutineDescription:
;
;   An entry in the following table is generated for each vector that can
;   receive an unexpected interrupt. Each entry in the table contains code
;   to push the vector number on the stack and then jump to common code to
;   process the unexpected interrupt.
;
; Arguments:
;
;    None.
;
;--

        NESTED_ENTRY KiUnexpectedInterrupt, _TEXT$00

        .pushframe code                 ; mark machine frame
        .pushreg rbp                    ; mark nonvolatile register push

        GENERATE_INTERRUPT_FRAME <Vector>  ; generate interrupt frame

        mov     ecx, eax                ; compute interrupt IRQL
        shr     ecx, 4                  ;

        ENTER_INTERRUPT <NoEOI>         ; raise IRQL and enable interrupts

        EXIT_INTERRUPT                  ; do EOI, lower IRQL, and restore state

        NESTED_END KiUnexpectedInterrupt, _TEXT$00

        end

