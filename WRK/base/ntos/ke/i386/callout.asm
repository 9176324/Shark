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
;    callout.asm
;
; Abstract:
;
;    This module implements the code necessary to call out from kernel
;    mode to user mode.
;
;--

.386p
        .xlist
include ks386.inc
include i386\kimacro.inc
include callconv.inc
include irqli386.inc
        .list

        extrn   _KiServiceExit:PROC
        extrn   _KeUserCallbackDispatcher:DWORD

        EXTRNP  _KeBugCheck2,6
        EXTRNP  _MmGrowKernelStack,1

_TEXT   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:FLAT, FS:NOTHING, GS:NOTHING

        page ,132
        subttl  "Call User Mode Function"
;++
;
; NTSTATUS
; KiCallUserMode (
;    IN PVOID *Outputbuffer,
;    IN PULONG OutputLength
;    )
;
; Routine Description:
;
;    This function calls a user mode function from kernel mode.
;
;    N.B. This function calls out to user mode and the NtCallbackReturn
;        function returns back to the caller of this function. Therefore,
;        the stack layout must be consistent between the two routines.
;
; Arguments:
;
;    OutputBuffer - Supplies a pointer to the variable that receives
;        the address of the output buffer.
;
;    OutputLength - Supplies a pointer to a variable that receives
;        the length of the output buffer.
;
; Return Value:
;
;    The final status of the call out function is returned as the status
;    of the function.
;
;    N.B. This function does not return to its caller. A return to the
;        caller is executed when a NtCallbackReturn system service is
;        executed.
;
;    N.B. This function does return to its caller if a kernel stack
;         expansion is required and the attempted expansion fails.
;
;--

;
; To support the debugger, the callback stack frame is now defined in i386.h.
; If the stack frame is changed, i386.h must be updated and geni386
; rebuilt and run, then rebuild this file and ntos\kd.
;
; The FPO record below must also be updated to correctly represent
; the stack frame.
;

cPublicProc _KiCallUserMode, 2

.FPO (3, 2, 4, 4, 0, 0)

;
; Save nonvolatile registers.
;

        push    ebp                     ; save nonvolatile registers
        push    ebx                     ;
        push    esi                     ;
        push    edi                     ;

        mov     ebx,PCR[PcPrcbData + PbCurrentThread] ; get current thread address

;
; Check if the current IRQL is above passive level.
;

if DBG

        CurrentIrql                     ; get current IRQL
        or      al,al                   ; check if IRQL is passive level
        jz      short Kcb00             ; if z, IRQL at passive level
        stdCall _KeBugCheck2,<IRQL_GT_ZERO_AT_SYSTEM_SERVICE,0,eax,0,0,0>

;
; Check if kernel APCs are disabled or a process is attached.
;

Kcb00:  movzx   eax,byte ptr[ebx]+ThApcStateIndex ; get APC state index
        mov     edx,[ebx]+ThCombinedApcDisable ; get kernel APC disable
        or      eax,eax                ; check if process attached
        jne     short Kcb05            ; if ne, process is attached
        or      edx,edx                ; check if kernel APCs disabled
        jz      short Kcb07            ; if z, kernel APCs enabled
Kcb05:  stdCall _KeBugCheck2,<APC_INDEX_MISMATCH,0,eax,edx,0,0>

endif

;
; Check if sufficient room is available on the kernel stack for another
; system call.
;

Kcb07:  lea     eax,[esp]-KERNEL_LARGE_STACK_COMMIT ; compute bottom address
        cmp     eax,[ebx]+ThStackLimit  ; check if limit exceeded
        jae     short Kcb10             ; if ae, limit not exceeded
        stdCall _MmGrowKernelStack,<esp> ; attempt to grow kernel stack
        or      eax, eax                ; check for successful completion
        jne     Kcb20                   ; if ne, attempt to grow failed

;
; Get the address of the current thread and save the previous trap frame
; and callback stack addresses in the current frame. Also save the new
; callback stack address in the thread object.
;

Kcb10:  push    [ebx].ThCallbackStack   ; save callback stack address
        mov     edx,[ebx].ThTrapFrame   ; get current trap frame address
        push    edx                     ; save trap frame address
        mov     esi,[ebx].ThInitialStack ; get initial stack address
        push    esi                     ; save initial stack address
        mov     [ebx].ThCallbackStack,esp ; save callback stack address

KcbPrologEnd: ; help for the debugger

;
; Copy the numeric save area from the previous save area to the new save
; area and establish a new initial kernel stack.
;
; Make sure that the destination NPX Save area is 16-byte aligned
; as required by fxsave\fxrstor
;

        and     esp, 0fffffff0h         ;
        mov     edi,esp                 ; set new initial stack address
        sub     esp,NPX_FRAME_LENGTH    ; compute destination NPX save area
        sub     esi,NPX_FRAME_LENGTH    ; compute source NPX save area
        cli                             ; disable interrupts
        mov     ecx,[esi].FpControlWord ; copy NPX state to new frame
        mov     [esp].FpControlWord,ecx ;
        mov     ecx,[esi].FpStatusWord  ;
        mov     [esp].FpStatusWord,ecx  ;
        mov     ecx,[esi].FpTagWord     ;
        mov     [esp].FpTagWord,ecx     ;
        mov     ecx,[esi].FxMXCsr       ;
        mov     [esp].FxMXCsr,ecx       ;
        mov     ecx,[esi].FpCr0NpxState ;
        mov     [esp].FpCr0NpxState,ecx ;
        mov     esi,PCR[PcTss]          ; get address of task switch segment
        mov     [ebx].ThInitialStack,edi ; reset initial stack address
        mov     ecx, esp                ;
        sub     esp,TsV86Gs - TsHardwareSegSs ; bias for missing V86 fields

.errnz (EFLAGS_V86_MASK AND 0FF00FFFFh)

        test    byte ptr [edx]+TsEFlags+2,EFLAGS_V86_MASK/010000h  ; is this a V86 frame?
        jne     @f
        mov     ecx, esp                ; Use adjusted esp (normal case)
@@:     mov     [esi].TssEsp0,ecx       ; set kernel entry stack address

;
; Construct a trap frame to facilitate the transfer into user mode via
; the standard system call exit.
;

        sub     esp,TsHardwareSegSs + 4 ; allocate trap frame
        mov     ebp,esp                 ; set address of trap frame
        mov     ecx,(TsHardwareSegSs - TsSegFs + 4) / 4; set repeat count
        lea     edi,[esp].TsSegFs       ; set destination address
        lea     esi,[edx].TsSegFs       ; set source address
        rep     movsd                   ; copy trap information

        mov     edi, [edx].TsDr7                ; Fetch control register
        test    edi, (NOT DR7_RESERVED_MASK)    ; Do we need to restore Debug reg?
        mov     [esp].TsDr7, edi                ; Save away Dr7
        jnz     short Kcb18                     ; Yes, go save them.

Kcb15:  mov     eax,_KeUserCallbackDispatcher ; st address of callback dispatcher
        mov     [esp].TsEip,eax         ;
        mov     eax,PCR[PcExceptionList] ; get current exception list
        mov     [esp].TsExceptionList,eax ; set previous exception list
        mov     eax,[edx].TsPreviousPreviousMode ; get previous mode
        mov     [esp].TsPreviousPreviousMode,eax ; set previous mode
        sti                             ; enable interrupts

        SET_DEBUG_DATA                  ; set system call debug data for exit

        jmp     _KiServiceExit          ; exit through service dispatch

Kcb18:
        mov     ecx,(TsDr6 - TsDr0 + 4) / 4; set repeat count
        lea     edi,[esp].TsDr0         ; set destination address
        lea     esi,[edx].TsDr0         ; set source address
        rep     movsd                   ; copy trap information
        jmp     short Kcb15

;
; An attempt to grow the kernel stack failed.
;

Kcb20:  pop     edi                     ; restore nonvolitile register
        pop     esi                     ;
        pop     ebx                     ;
        pop     ebp                     ;
        stdRET  _KiCallUserMode

stdENDP _KiCallUserMode

        page ,132
        subttl  "Switch Kernel Stack"
;++
;
; PVOID
; KeSwitchKernelStack (
;    IN PVOID StackBase,
;    IN PVOID StackLimit
;    )
;
; Routine Description:
;
;    This function switches to the specified large kernel stack.
;
;    N.B. This function can ONLY be called when there are no variables
;        in the stack that refer to other variables in the stack, i.e.,
;        there are no pointers into the stack.
;
; Arguments:
;
;    StackBase (esp + 4) - Supplies a pointer to the base of the new kernel
;        stack.
;
;    StackLimit (esp + 8) - Supplies a pointer to the limit of the new kernel
;        stack.
;
; Return Value:
;
;    The old kernel stack is returned as the function value.
;
;--

SsStkBs equ     4                       ; new kernel stack base address
SsStkLm equ     8                       ; new kernel stack limit address

cPublicProc _KeSwitchKernelStack, 2

;
; Save the address of the new stack and copy the old stack to the new
; stack.
;

        push    esi                     ; save string move registers
        push    edi                     ;
        mov     edx,PCR[PcPrcbData + PbCurrentThread] ; get current thread address
        mov     edi,[esp]+SsStkBs + 8   ; get new kernel stack base address
        mov     ecx,[edx].ThStackBase   ; get current stack base address
        sub     ebp,ecx                 ; relocate the callers frame pointer
        add     ebp,edi                 ;
        mov     eax,[edx].ThTrapFrame   ; relocate the current trap frame address
        sub     eax,ecx                 ;
        add     eax,edi                 ;
        mov     [edx].ThTrapFrame,eax   ;
        sub     ecx,esp                 ; compute length of copy
        sub     edi,ecx                 ; set destination address of copy
        mov     esi,esp                 ; set source address of copy
        push    edi                     ; save new stack pointer address
        rep     movsb                   ; copy old stack to new stack
        pop     edi                     ; restore new stack pointer address

;
; Switch to the new kernel stack and return the address of the old kernel
; stack.
;

        mov     eax,[edx].ThStackBase   ; get old kernel stack base address
        mov     ecx,[esp]+SsStkBs + 8   ; get new kernel stack base address
        mov     esi,[esp]+SsStkLm + 8   ; get new kernel stack limit address
        cli                             ; disable interrupts
        mov     [edx].ThStackBase,ecx   ; set new kernel stack base address
        mov     [edx].ThStackLimit,esi  ; set new kernel stack limit address
        mov     byte ptr [edx].ThLargeStack, 1 ; set large stack TRUE
        mov     [edx].ThInitialStack,ecx ; set new initial stack address
        mov     esi,[edx].ThTrapFrame   ; Get current trap frame address
        mov     edx,PCR[PcTss]          ; get address of task switch segment
.errnz (EFLAGS_V86_MASK AND 0FF00FFFFh)
        test    byte ptr [esi]+TsEFlags+2,EFLAGS_V86_MASK/010000h  ; is this a V86 frame?
        lea     ecx, dword ptr [ecx-NPX_FRAME_lENGTH]    ; compute NPX save area address
        jne     @f
        sub     ecx,TsV86Gs - TsHardwareSegSs ; bias for missing V86 fields
@@:     mov     [edx].TssEsp0,ecx       ; set kernel entry stack address
        mov     esp,edi                 ; set new stack pointer address
        sti                             ;
        pop     edi                     ; restore string move registers
        pop     esi                     ;
        stdRET  _KeSwitchKernelStack

stdENDP _KeSwitchKernelStack

        page ,132
        subttl  "Get User Mode Stack Address"
;++
;
; PULONG
; KiGetUserModeStackAddress (
;    VOID
;    )
;
; Routine Description:
;
;    This function returns the address of the user stack address in the
;    current trap frame.
;
; Arguments:
;
;    None.
;
; Return Value:
;
;    The address of the user stack address.
;
;--

cPublicProc _KiGetUserModeStackAddress, 0

        mov     eax,PCR[PcPrcbData + PbCurrentThread] ; get current thread address
        mov     eax,[eax].ThTrapFrame   ; get current trap frame address
        lea     eax,[eax].TsHardwareEsp ; get address of stack address
        stdRET  _KiGetUserModeStackAddress

stdENDP _KiGetUserModeStackAddress

        page ,132
        subttl  "Return from User Mode Callback"
;++
;
; NTSTATUS
; NtCallbackReturn (
;    IN PVOID OutputBuffer OPTIONAL,
;    IN ULONG OutputLength,
;    IN NTSTATUS Status
;    )
;
; Routine Description:
;
;    This function returns from a user mode callout to the kernel
;    mode caller of the user mode callback function.
;
;    N.B. This function returns to the function that called out to user
;        mode and the KiCallUserMode function calls out to user mode.
;        Therefore, the stack layout must be consistent between the
;        two routines.
;
; Arguments:
;
;    OutputBuffer - Supplies an optional pointer to an output buffer.
;
;    OutputLength - Supplies the length of the output buffer.
;
;    Status - Supplies the status value returned to the caller of the
;        callback function.
;
; Return Value:
;
;    If the callback return cannot be executed, then an error status is
;    returned. Otherwise, the specified callback status is returned to
;    the caller of the callback function.
;
;    N.B. This function returns to the function that called out to user
;         mode is a callout is currently active.
;
;--

cPublicProc _NtCallbackReturn, 3

        mov     eax,PCR[PcPrcbData + PbCurrentThread] ; get current thread address
        mov     ecx,[eax].ThCallbackStack ; get callback stack address
        test    ecx, ecx
        je      CbExit                    ; if zero, no callback stack present

;
; Restore the current exception list from the saved exception list in the
; current trap frame, restore the trap frame and callback stack addresses,
; store the output buffer address and length, and set the service status.
;

        mov     ebx, [eax].ThTrapFrame   ; get current trap frame address
        mov     edx, [ebx].TsExceptionList ; get saved exception list address
        mov     PCR[PcExceptionList], edx ; restore exception list address
        mov     edi, [esp] + 4           ; get output buffer address
        mov     esi, [esp] + 8           ; get output buffer length
        mov     ebp, [esp] + 12          ; get callout service status
        mov     ebx, [ecx].CuOutBf       ; get address to store output buffer
        mov     [ebx], edi               ; store output buffer address
        mov     ebx, [ecx].CuOutLn       ; get address to store output length
        mov     [ebx], esi               ; store output buffer length
        mov     ebx, [ecx]               ; get previous initial stack address
        cli                             ; disable interrupt
        mov     esi, [eax].ThInitialStack ; get source NPX save area address
        mov     [eax].ThInitialStack, ebx ; restore initial stack address
        sub     esi, NPX_FRAME_LENGTH    ; compute source NPX save area
        sub     ebx, NPX_FRAME_LENGTH    ; compute destination NPX save area
        mov     edx,[esi].FpControlWord ; copy NPX state to previous frame
        mov     [ebx].FpControlWord, edx ;
        mov     edx, [esi].FpStatusWord  ;
        mov     [ebx].FpStatusWord, edx  ;
        mov     edx, [esi].FpTagWord     ;
        mov     [ebx].FpTagWord, edx     ;
        mov     edx, [esi].FxMXCsr       ;
        mov     [ebx].FxMXCsr, edx       ;
        mov     edx, [esi].FpCr0NpxState ;
        mov     [ebx].FpCr0NpxState, edx ;
        cmp     ebp, STATUS_CALLBACK_POP_STACK
        mov     edi, [ecx].CuTrFr        ; Fetch address of previous trap frame
        jz      short _NtCbCopyTrapFrame
_NtCbCopyTrapFrameRet:
        and     dword ptr [edi].TsDr7, 0 ; Assume original trap frame has no debug registers active
        test    byte ptr [eax].ThDebugActive, 0ffh 
        jne     short _NtCbGetDebugRegs
_NtCbGetDebugRet:        
        mov     edx, PCR[PcTss]          ; Get address of task switch segment
        lea     esp, [ecx].CuCbStk      ; Trim stack back to callback frame
.errnz (EFLAGS_V86_MASK AND 0FF00FFFFh)
        test    byte ptr [edi]+TsEFlags+2, EFLAGS_V86_MASK/010000h  ; is this a V86 frame?
        jne     @f
        sub     ebx, TsV86Gs - TsHardwareSegSs ; bias for missing V86 fields
@@:
        mov     [edx].TssEsp0, ebx       ; restore kernel entry stack address
        mov     [eax].ThTrapFrame, edi  ; restore current trap frame address
        sti                             ; enable interrupts
        pop     [eax].ThCallbackStack   ; restore callback stack address
        mov     eax, ebp                 ; set callback service status
        
;
; Restore nonvolatile registers, clean call parameters from stack, and
; return to callback caller.
;

        pop     edi                     ; restore nonvolatile registers
        pop     esi                     ;
        pop     ebx                     ;
        pop     ebp                     ;
        pop     edx                     ; save return address
        add     esp, 8                   ; remove parameters from stack
        jmp     edx                     ; return to callback caller

;
; edi - previous trap frame [in/out]
; ecx - callback stack [in/out]
; eax - current thread [in/out]
; esi/edx: available
;

_NtCbCopyTrapFrame:
        mov     ecx, (TsV86Es-TsSegFs)/4 
        mov     esi, [eax].ThTrapFrame   ; trap frame for this system call
.errnz (EFLAGS_V86_MASK AND 0FF00FFFFh)
        test    byte ptr [esi]+TsEFlags+2, EFLAGS_V86_MASK/010000h
        mov     edx, edi
        lea     edi, [edi].TsSegFs
        jz      short @f
        add     ecx, (KTRAP_FRAME_LENGTH-TsV86Es)/4
@@:     lea     esi, [esi].TsSegFs
        rep     movsd                    ; copy the relevant parts of the frame
        mov     ecx, [eax].ThCallbackStack
        mov     edi, edx                 ; restores ecx, edi
        jmp     _NtCbCopyTrapFrameRet

_NtCbGetDebugRegs:

;
; Copy debug registers from current frame to original frame.
;
        mov     esi, [eax].ThTrapFrame  ; Get current trap frame
        mov     edx, [esi].TsDr0
        mov     [edi].TsDr0, edx
        mov     edx, [esi].TsDr1
        mov     [edi].TsDr1, edx
        mov     edx, [esi].TsDr2
        mov     [edi].TsDr2, edx
        mov     edx, [esi].TsDr3
        mov     [edi].TsDr3, edx
        mov     edx, [esi].TsDr6
        mov     [edi].TsDr6, edx
        mov     edx, [esi].TsDr7
        mov     [edi].TsDr7, edx
        jmp     _NtCbGetDebugRet

;
; No callback is currently active.
;

CbExit: mov     eax, STATUS_NO_CALLBACK_ACTIVE ; set service status
        stdRET  _NtCallBackReturn

stdENDP _NtCallbackReturn

_TEXT   ends
        end

