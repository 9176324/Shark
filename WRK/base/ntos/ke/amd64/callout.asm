        title  "Call Out to User Mode"
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
;   callout.asm
;
; Abstract:
;
;   This module implements the code necessary to call out from kernel
;   mode to user mode.
;
;--

include ksamd64.inc

        altentry KiSwitchKernelStackContinue
        altentry KxSwitchKernelStackCallout

        extern  KeBugCheck:proc
        extern  KeBugCheckEx:proc
        extern  KeUserCallbackDispatcher:qword
        extern  KiSystemServiceExit:proc
        extern  KiRestoreDebugRegisterState:proc
        extern  MmGrowKernelStack:proc
        extern  MmGrowKernelStackEx:proc
        extern  PsConvertToGuiThread:proc

        subttl  "Call User Mode Function"
;++
;
; NTSTATUS
; KiCallUserMode (
;     IN PVOID *Outputbuffer,
;     IN PULONG OutputLength
;     )
;
; Routine Description:
;
;   This function calls a user mode function from kernel mode.
;
;   N.B. This function calls out to user mode and the callback return
;        function returns back to the caller of this function. Therefore,
;        the stack layout must be consistent between the two routines.
;
; Arguments:
;
;   OutputBuffer (rcx) - Supplies a pointer to the variable that receives
;       the address of the output buffer.
;
;   OutputLength (rdx) - Supplies a pointer to a variable that receives
;       the length of the output buffer.
;
; Return Value:
;
;   The final status of the call out function is returned as the status
;   of the function.
;
;   N.B. This function does not return to its caller. A return to the
;        caller is executed when a callback return system service is
;        executed.
;
;   N.B. This function does return to its caller if a kernel stack
;        expansion is required and the attempted expansion fails.
;
;--

        NESTED_ENTRY KiCallUserMode, _TEXT$00

        GENERATE_EXCEPTION_FRAME <Rbp>  ; generate exception frame

;
; Save argument registers in frame and allocate a legacy floating point
; save area.
;

        mov     CuOutputBuffer[rbp], rcx ; save output buffer address
        mov     CuOutputLength[rbp], rdx ; save output length address

;
; Check if the current IRQL is above passive level.
;

        mov     rbx, gs:[PcCurrentThread] ; get current thread address

if DBG

        xor     r9, r9                  ; clear parameter value
        mov     r8, cr8                 ; get current IRQL
        or      r8, r8                  ; check if IRQL is passive level
        mov     ecx, IRQL_GT_ZERO_AT_SYSTEM_SERVICE ; set bugcheck code
        jnz     short KiCU05            ; if nz, IRQL not passive level

;
; Check if kernel APCs are disabled or a process is attached.
;

        movzx   r8d, byte ptr ThApcStateIndex[rbx] ; get APC state index
        mov     r9d, ThCombinedApcDisable[rbx] ; get kernel APC disable
        or      r9d, r9d                ; check if kernel APCs disabled
        jnz     short KiCU03            ; if nz, Kernel APCs disabled
        or      r8d, r8d                ; check if process attached
        jz      short KiCU07            ; if z, process not attached
KiCU03: mov     ecx, APC_INDEX_MISMATCH ; set bugcheck code
KiCU05: mov     rdx, ExReturn[rbp]      ; set call out return address
        xor     r10, r10                ; clear trap frame address
        mov     ExP5[rbp], r10          ; set bugcheck parameter
        call    KeBugCheckEx            ; bugcheck system - no return

endif

;
; Check if sufficient room is available on the kernel stack for another
; system call.
;

KiCU07: sub     rsp, KERNEL_STACK_CONTROL_LENGTH ; allocate stack control save area
        lea     r15, (- KERNEL_LARGE_STACK_COMMIT)[rsp] ; compute low address
        cmp     r15, ThStackLimit[rbx]  ; check if limit exceeded
        jae     short KiCU10            ; if ae, limit not exceeded
        mov     rcx, rsp                ; set current stack address
        call    MmGrowKernelStack       ; attempt to grow kernel stack
        or      eax, eax                ; check for successful completion
        jne     KiUC30                  ; if ne, attempt to grow failed

;
; Save the callback stack address and the initial stack address in the current
; frame.
;

KiCU10: mov     rax, ThCallbackStack[rbx] ; save current callback stack address
        mov     CuCallbackStack[rbp], rax ;
        mov     rdx, ThInitialStack[rbx] ; save initial stack address
        mov     CuInitialStack[rbp], rdx ;
        mov     ThCallbackstack[rbx], rbp ; set new callback stack address

;
; Initialize the current and previous kernel stack segment descriptors in the
; kernel stack control area. These descriptors are used to control kernel
; stack expansion from drivers.
;

        mov     qword ptr KcPreviousBase[rsp], 0 ; clear previous base
        mov     rax, KcCurrentBase[rdx] ; set current stack base
        mov     KcCurrentBase[rsp], rax ;
        mov     rax, KcActualLimit[rdx] ; set current stack limit
        mov     KcActualLimit[rsp], rax ;

;
; Save the current trap frame address and establish a new initial kernel stack
; address;
;

        mov     rsi, ThTrapFrame[rbx]   ; save current trap frame address
        mov     CuTrapFrame[rbp], rsi   ;
        cli                             ; disable interrupts
        mov     rdi, gs:[PcTss]         ; get processor TSS address
        mov     ThInitialStack[rbx], rsp ; set new initial stack address
        mov     TssRsp0[rdi], rsp       ; set initial stack address in TSS
        mov     gs:[PcRspBase], rsp     ; set initial stack address in PRCB

;
; Check to determine if a user mode APC is pending.
;
; N.B. Interrupts are not enabled throughout the remainder of the system
;      service exit.
;

        cmp     byte ptr ThApcState + AsUserApcPending[rbx], 0 ; APC pending?
        jne     short KiUC20            ; if ne, user APC pending

;
; A user APC is not pending.
;
; Exit directly back to user mode without building a trap frame.
;

        lea     rbp, 128[rsi]           ; get previous trap frame address
        ldmxcsr TrMxCsr[rbp]            ; restore previous MXCSR
        test    word ptr TrDr7[rbp], DR7_ACTIVE ; test if debug active
        jz      short @f                ; if z, debug not active
        call    KiRestoreDebugRegisterState ; restore user debug register state
@@:     mov     r8, TrRsp[rbp]          ; get previous RSP value
        mov     r9, TrRbp[rbp]          ; get previous RBP value
        xor     r10, r10                ; scrub volatile integer registers
        pxor    xmm0, xmm0              ; scrub volatile floating registers
        pxor    xmm1, xmm1              ;
        pxor    xmm2, xmm2              ;
        pxor    xmm3, xmm3              ;
        pxor    xmm4, xmm4              ;
        pxor    xmm5, xmm5              ;
        mov     rcx, KeUserCallbackDispatcher ; set user return address
        mov     r11, TrEFlags[rbp]      ; get previous EFLAGS
        mov     rbp, r9                 ; restore RBP
        mov     rsp, r8                 ; restore RSP
        swapgs                          ; swap GS base to user mode TEB
        sysretq                         ; return from system call to user mode

;
; A user APC is pending.
;
; Construct a trap frame to facilitate the transfer into user mode via
; the standard system call exit.
;
; N.B. The below code uses an unusual sequence to transfer control. This
;      instruction sequence is required to avoid detection as an epilogue.
;

KiUC20: sub     rsp, KTRAP_FRAME_LENGTH ; allocate a trap frame
        mov     rdi, rsp                ; set destination address
        mov     ecx, (KTRAP_FRAME_LENGTH / 8) ; set length of copy
    rep movsq                           ; copy trap frame
        lea     rbp, 128[rsp]           ; set frame pointer address
        mov     rax, KeUserCallbackDispatcher ; set user return address
        mov     TrRip[rbp], rax         ;
        lea     rcx, KiSystemServiceExit ; get address of service exit
        jmp     rcx                     ; finish in common code

;
; An attempt to grow the kernel stack failed.
;

KiUC30: mov     rsp, rbp                ; deallocate legacy save area

        RESTORE_EXCEPTION_STATE <Rbp>   ; restore exception state/deallocate

        ret                             ;

        NESTED_END KiCallUserMode, _TEXT$00

        subttl  "Convert To Gui Thread"
;++
;
; NTSTATUS
; KiConvertToGuiThread (
;     VOID
;     );
;
; Routine Description:
;
;   This routine is a stub routine which is called by the system service
;   dispatcher to convert the current thread to a GUI thread. The process
;   of converting to a GUI mode thread involves allocating a large stack,
;   switching to the large stack, and then calling the win32k subsystem
;   to record state. In order to switch the kernel stack the frame pointer
;   used in the system service dispatch must be relocated. 
;
;   N.B. The address of the pushed rbp in this routine is located from the
;        trap frame address in switch kernel stack.
;
;   N.B. This routine diverges from the calling standard in that the caller
;        (KiSystemCall64) expects the parameter home addresses to be preserved.
;
; Arguments:
;
;   None.
;
; Implicit arguments:
;
;   rbp - Supplies a pointer to the trap frame.
;
; Return Value:
;
;   The status returned by the real convert to GUI thread is returned as the
;   function status.
;
;--

CgFrame struct
        P1Home  dq ?                    ; parameter home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        SavedRbp dq ?                   ; saved nonvolatile register
CgFrame ends

        NESTED_ENTRY KiConvertToGuiThread, _TEXT$00

        push_reg rbp                    ; save frame pointer
        alloc_stack (sizeof CgFrame - 8) ; allocate stack frame

        END_PROLOGUE

        call    PsConvertToGuiThread    ; convert to GUI thread
        add     rsp, (sizeof CgFrame - 8) ; deallocate stack frame
        pop     rbp                     ; restore frame pointer
        ret                             ;

        NESTED_END KiConvertToGuiThread, _TEXT$00

        subttl  "Switch Kernel Stack"
;++
;
; PVOID
; KeSwitchKernelStack (
;     IN PVOID StackBase,
;     IN PVOID StackLimit
;     )
;
; Routine Description:
;
;   This function switches to the specified large kernel stack.
;
;   N.B. This function can ONLY be called when there are no variables
;        in the stack that refer to other variables in the stack, i.e.,
;        there are no pointers into the stack.
;
;   N.B. The address of the frame pointer used in the system service
;        dispatcher is located using the trap frame.
;
; Arguments:
;
;   StackBase (rcx) - Supplies a pointer to the base of the new kernel
;       stack.
;
;   StackLimit (rdx) - Supplies a pointer to the limit of the new kernel
;       stack.
;
; Return Value:
;
;   The previous stack base is returned as the function value.
;
;--

SkFrame struct
        Fill    dq ?                    ; fill to 8 mod 16
        SavedRdi dq ?                   ; saved register RDI
        SavedRsi dq ?                   ; saved register RSI
SkFrame ends

        NESTED_ENTRY KeSwitchKernelStack, _TEXT$00

        alloc_stack (sizeof SkFrame)    ; allocate stack frame
        save_reg rdi, SkFrame.SavedRdi  ; save nonvolatile registers
        save_reg rsi, SkFrame.savedRsi  ;

        END_PROLOGUE

;
; Save the address of the new stack and copy the current stack to the new
; stack.
;

        mov     r8, rcx                 ; save new stack base address
        mov     r10, gs:[PcCurrentThread] ; get current thread address
        mov     rcx, ThStackBase[r10]   ; get current stack base address
        mov     r9, ThTrapFrame[r10]    ; get current trap frame address
        sub     r9, rcx                 ; relocate trap frame address
        add     r9, r8                  ;
        mov     ThTrapFrame[r10], r9    ; set new trap frame address
        sub     rcx, rsp                ; compute length of copy in bytes
        mov     rdi, r8                 ; compute destination address of copy
        sub     rdi, rcx                ;
        mov     r9, rdi                 ; save new stack pointer address
        mov     rsi, rsp                ; set source address of copy
        shr     rcx, 3                  ; compute length of copy on quadwords
    rep movsq                           ; copy old stack to new stack
        mov     rcx, ThTrapFrame[r10]   ; get new trap frame address
        lea     rax, 128[rcx]           ; compute new frame address
        mov     (-2 * 8)[rcx], rax      ; set relocated frame pointer 

;
; Switch to the new kernel stack, initialize the current and previous kernel
; stack segment descriptors in the kernel stack control area, and return the
; address of the old kernel stack.
;

        mov     rax, ThStackBase[r10]   ; get current stack base address
        cli                             ; disable interrupts
        mov     byte ptr ThLargeStack[r10], 1 ; set large stack TRUE
        mov     ThStackBase[r10], r8    ; set new stack base address
        sub     r8, KERNEL_STACK_CONTROL_LENGTH ; compute initial stack address
        mov     ThInitialStack[r10], r8 ; set new initial stack address
        mov     ThStackLimit[r10], rdx  ; set new stack limit address
        mov     r10, gs:[PcTss]         ; get processor TSS address
        mov     TssRsp0[r10], r8        ; set initial stack address in TSS
        mov     gs:[PcRspBase], r8      ; set initial stack address in PRCB
        mov     rsp, r9                 ; set new stack pointer address
        mov     qword ptr KcPreviousBase[r8], 0 ; clear previous base
        lea     r9, KERNEL_STACK_CONTROL_LENGTH[r8] ; set current stack base
        mov     KcCurrentBase[r8], r9   ;
        sub     r9, KERNEL_LARGE_STACK_SIZE ; set actual stack limit
        mov     KcActualLimit[r8], r9  ;
        sti                             ; enable interrupts
        mov     rdi, SkFrame.SavedRdi[rsp] ; restore nonvolatile registers
        mov     rsi, SkFrame.SavedRsi[rsp] ;
        add     rsp, (sizeof SkFrame)   ; deallocate stack frame
        ret                             ; return

        NESTED_END KeSwitchKernelStack, _TEXT$00

        subttl  "Switch Kernel Stack and Callout Handler Exception"
;++
;
; EXCEPTION_DISPOSITION
; KiSwitchKernelStackAndCalloutHandler (
;     IN PEXCEPTION_RECORD ExceptionRecord,
;     IN UINT_PTR EstablisherFrame,
;     IN OUT PCONTEXT ContextRecord,
;     IN OUT PDISPATCHER_CONTEXT DispatcherContext
;     )
;
; Routine Description:
;
;   This function is called when an unhandled exception occurs while the
;   kernel stack has been expanded and a callout is active. This is a fatal
;   condition and causes a bugcheck.
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
;   DispatcherContext (r9) - Supplies a pointer to the dispatcher context
;       record.
;
; Return Value:
;
;   None.
;
;--

KhFrame struct
        P1Home  dq ?                    ; argument home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        Fill    dq ?                    ; fill to 0 mod 8
KhFrame ends

        NESTED_ENTRY KiSwitchKernelStackAndCalloutHandler, _TEXT$00

        alloc_stack (sizeof KhFrame)    ; allocate stack frame

        END_PROLOGUE

        mov     ecx, KMODE_EXCEPTION_NOT_HANDLED ; set bugcheck code
        call    KeBugCheck              ; bugcheck the system
        nop                             ; fill - do not remove

        NESTED_END KiSwitchKernelStackAndCalloutHandler, _TEXT$00

        subttl  "Switch Kernel Stack and Callout"
;++
;
; VOID
; KiSwitchKernelStackAndCallout (
;     IN PVOID Parameter,
;     IN PEXPAND_STACK_CALLOUT Callout,
;     IN PVOID LargeStack,
;     IN SIZE_T CommitSize
;     )
;
; Routine Description:
;
;   This function initializes the kernel stack control region of the new
;   stack, sets the new stack parameters for the current thread, calls
;   the specified function with the specified argument, and reverses the
;   process on return from the called function.
;
; Arguments:
;
;   Parameter (rcx) - Supplies a the call out routine parameter.
;
;   Callout (rdx) - Supplies a pointer to the call out function.
;
;   LargeStack (r8) - Supplies the base address of a large kernel stack.
;
;   CommitSize (r9) - Supplies the stack commit size in bytes page aligned.
;   
; Return Value:
;
;   The previous stack base is returned as the function value.
;
;--

KoFrame struct
        Fill    dq ?                    ; fill to 8 mod 16
        SavedRdi dq ?                   ; saved register RDI
        SavedRsi dq ?                   ; saved register RSI
KoFrame ends

KbFrame struct
        P1home  dq ?                    ; parameter home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        MfCode  dq ?                    ; dummy machine frame
        MfRip   dq ?                    ;
        MfSegCs dq ?                    ;
        MfEFlags dq ?                   ;
        MfRsp   dq ?                    ;
        MfSeqSs dq ?                    ;
KbFrame ends

        NESTED_ENTRY KiSwitchKernelStackAndCallout, _TEXT$00, KiSwitchKernelStackAndCalloutHandler

        alloc_stack (sizeof KoFrame)    ; allocate stack frame
        save_reg rdi, KoFrame.SavedRdi  ; save nonvolatile registers
        save_reg rsi, KoFrame.savedRsi  ;

        END_PROLOGUE

;
; Save the current thread stack parameters in the previous section of the
; kernel stack control area.
;

        mov     rsi, gs:[PcCurrentThread] ; get current thread address
        mov     rdi, rsp                ; save current stack address
        lea     rax, (-KERNEL_STACK_CONTROL_LENGTH)[r8] ; get initial stack address
        mov     r10, ThStackBase[rsi]   ; save current stack base
        mov     KcPreviousBase[rax], r10 ;
        mov     r10, ThStackLimit[rsi]  ; save current stack limit
        mov     KcPreviousLimit[rax], r10 ;
        mov     KcPreviousKernel[rax], rdi ; save current kernel stack
        mov     r10, ThInitialStack[rsi] ; save initial stack address
        mov     KcPreviousInitial[rax], r10 ;

;
; Initialize the current thread parameters in the current section of the
; kernel stack control area.
;

        mov     KcCurrentBase[rax], r8  ; set current stack base
        lea     r11, (-KERNEL_LARGE_STACK_SIZE)[r8] ; set actual stack limit
        mov     KcActualLimit[rax], r11 ;

;
; Initialize a dummy machine frame and parameter home address area on the
; new stack.
;
; N.B. Only the return address and previous stack pointer are filled into
;      the dummy machine frame.
;

        lea     r11, (-(sizeof KbFrame))[rax] ; get new frame address
        lea     r10, KiSwitchKernelStackContinue ; set RIP in machine frame
        mov     KbFrame.MfRip[r11], r10 ;
        mov     KbFrame.MfRsp[r11], rdi ; set RSP in machine frame

;
; Initialize new thread stack parameters and jump dispatch routine.
;

        cli                             ; disable interupts
        mov     ThStackBase[rsi], r8    ; set new stack base
        sub     r8, KERNEL_LARGE_STACK_COMMIT ; set new stack limit
        mov     ThStackLimit[rsi], r8   ;
        mov     ThInitialStack[rsi], rax ; set new initial stack
        mov     rsp, r11                ; set new stack pointer
        sti                             ; enable interrupt
        jmp     KxSwitchKernelStackCallout ; call specified routine

;
; Continue from callout on original stack with
;

        ALTERNATE_ENTRY KiSwitchKernelStackContinue

        mov     rdi, KoFrame.SavedRdi[rsp] ; restore nonvolatile registers
        mov     rsi, KoFrame.SavedRsi[rsp] ;
        add     rsp, (sizeof KoFrame)   ; deallocate stack frame
        ret                             ; return

        NESTED_END KiSwitchKernelStackAndCallout, _TEXT$00

;++
;
; This is a dummy function to effect a call out on an expanded stack with
; the proper unwind information.
;
; N.B. The prologue is not executed and the stack frame is set up before
;      transferring control to the alternate entry.
;
;
; At entry to this dummy function:
;
;    rcx - Supplies the call out routine parameter.
;    rdx - Supplies the address of the call out routine.
;    rsi - Supplies the current thread address.
;    rdi - Supplies the old kernel stack address.
;    rsp - Supplies the new stack address.
;    r9 - Supplies the required commit limit.
;       
;--

        NESTED_ENTRY KySwitchKernelStackCallout, _TEXT$00

        push_frame code                 ;
        .allocstack 32                  ; 

        END_PROLOGUE

        ALTERNATE_ENTRY KxSwitchKernelStackCallout

        mov     KbFrame.MfCode[rsp], rcx ; save call out parameter
        mov     KbFrame.MfEFlags[rsp], rdx ; save call out routine address
        mov     rcx, rsp                ; set current kernel stack
        mov     rdx, r9                 ; set require commit size
        call    MmGrowKernelStackEx     ; grow stack to required commitment
        test    eax, eax                ; test if stack committed
        jnz     short KxKO10            ; if nz, stack not committed
        mov     rcx, KbFrame.MfCode[rsp] ; set call out parameter
        call    KbFrame.MfEFlags[rsp]   ; call specified routine
        xor     eax, eax                ; set success completion status

;
; Restore the thread stack parameters and finish in previous code.
;
; N.B. The TSS initial stack and the PRCB initial stack only needs to be
;      reloaded after the call out and just before switching back to the
;      previous stack. This is in contrast to the user mode call code
;      which reloads these two values in both directions. The reason this
;      is possible is because the thread can not return to user mode
;      until after the call out is complete and any exceptions or interrupts
;      will nest on the current kernel stack.
;

KxKO10: mov     rcx, ThInitialStack[rsi] ; get address of initial frame
        cli                             ; disable interrupts
        mov     r10, KcPreviousBase[rcx] ; set stack base
        mov     ThStackBase[rsi], r10   ;
        mov     r10, KcPreviouslimit[rcx] ; set stack limit
        mov     ThStackLimit[rsi], r10  ;
        mov     r10, KcPreviousInitial[rcx] ; set initial stack
        mov     ThInitialStack[rsi], r10 ;
        mov     rdx, gs:[PcTss]         ; get processor TSS address
        mov     TssRsp0[rdx], r10       ; set initial stack address in TSS
        mov     gs:[PcRspBase], r10     ; set initial stack address in PRCB
        mov     rsp, rdi                ; set previous stack address
        sti                             ; enable interrupts
        jmp     KiSwitchKernelStackContinue ; finish in common code

        NESTED_END KySwitchKernelStackCallout, _TEXT$00

        subttl  "Return from User Mode Callback"
;++
;
; NTSTATUS
; NtCallbackReturn (
;     __in __opt PVOID OutputBuffer,
;     __in ULONG OutputLength,
;     __in NTSTATUS Status
;     )
;
; Routine Description:
;
;   This function returns from a user mode callout to the kernel
;   mode caller of the user mode callback function.
;
;   N.B. This function returns to the function that called out to user mode
;        using the call user mode function. Therefore, the stack layout must
;        be consistent between the two routines.
;
; Arguments:
;
;   OutputBuffer (rcx) - Supplies an optional pointer to an output buffer.
;
;   OutputLength (edx) - Supplies the length of the output buffer.
;
;   Status (r8d) - Supplies the status value returned to the caller of the
;       callback function.
;
; Return Value:
;
;   If the callback return cannot be executed, then an error status is
;   returned. Otherwise, the specified callback status is returned to
;   the caller of the callback function.
;
;   N.B. This function returns to the function that called out to user
;        mode is a callout is currently active.
;
;--

CbFrame struct
        Fill    dq ?                    ; fill to 8 mod 16
        SavedRdi dq ?                   ; saved register RDI
        SavedRsi dq ?                   ; saved register RSI
CbFrame ends

        NESTED_ENTRY NtCallbackReturn, _TEXT$00

        alloc_stack (sizeof CbFrame)    ; allocate stack frame
        save_reg rdi, CbFrame.SavedRdi  ; save nonvolatile registers
        save_reg rsi, CbFrame.savedRsi  ;

        END_PROLOGUE

        mov     r11, gs:[PcCurrentThread] ; get current thread address
        mov     r10, ThCallbackStack[r11] ; get callback stack address
        cmp     r10, 0                  ; check if callback active
        je      KiCb10                  ; if zero, callback not active
        mov     eax, r8d                ; save completion status

;
; Store the output buffer address and length.
;

        mov     r9, CuOutputBuffer[r10] ; get address to store output buffer
        mov     [r9], rcx               ; store output buffer address
        mov     r9, CuOutputLength[r10] ; get address to store output length
        mov     [r9], edx               ; store output buffer length

;
; Restore the previous callback stack address and trap frame address.
;

        cli                             ; disable interrupts
        mov     r8, CuTrapFrame[r10]    ; get previous trap frame address
        mov     r9, ThTrapFrame[r11]    ; get current trap frame
        mov     ThTrapFrame[r11], r8    ; restore previous trap frame address

;
; If debug registers are active, then copy the debug registers from the
; current trap frame to the previous trap frame.
;
; N.B. Trap frame offsets are biased offsets.
;

        test    byte ptr ThDebugActive[r11], TRUE ; test if debug enabled
        mov     word ptr TrDr7 + 128[r8], 0 ; assume debug not enabled
        jz      short KiCB05            ; if z, debug not enabled
        mov     rcx, TrDr0 + 128[r9]    ; move current debug registers to
        mov     rdx, TrDr1 + 128[r9]    ;   previous trap frame
        mov     TrDr0 + 128[r8], rcx    ;
        mov     TrDr1 + 128[r8], rdx    ;
        mov     rcx, TrDr2 + 128[r9]    ;
        mov     rdx, TrDr3 + 128[r9]    ;
        mov     TrDr2 + 128[r8], rcx    ;
        mov     TrDr3 + 128[r8], rdx    ;
        mov     rcx, TrDr7 + 128[r9]    ;
        mov     TrDr7 + 128[r8], rcx    ;
KiCB05: mov     r8, CuCallbackStack[r10] ; get previous callback stack address
        mov     ThCallbackStack[r11], r8 ; restore previous callback stack address

;
; Restore initial stack address and restore exception state.
;

        mov     r9, CuInitialStack[r10] ; get previous initial stack address
        mov     ThInitialStack[r11], r9 ; restore initial stack address
        mov     r8, gs:[PcTss]          ; get processor TSS address
        mov     TssRsp0[r8], r9         ; set initial stack address in TSS
        mov     gs:[PcRspBase], r9      ; set initial stack address in PRCB
        mov     rsp, r10                ; trim stack back to callback frame

        RESTORE_EXCEPTION_STATE <Rbp>   ; restore exception state/deallocate

        sti                             ; enable interrupts
        ret                             ; return

;
; No callback is currently active.
;

KiCB10: mov     eax, STATUS_NO_CALLBACK_ACTIVE ; set service status
        mov     rdi, CbFrame.SavedRdi[rsp] ; restore nonvolatile registers
        mov     rsi, CbFrame.savedRsi[rsp] ;
        add     rsp, sizeof CbFrame     ; deallocate stack frame
        ret                             ; return

        NESTED_END NtCallbackReturn, _TEXT$00

        end

