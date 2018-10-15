        title  "Context Swap"
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
;    ctxswap.asm
;
; Abstract:
;
;    This module implements the code necessary to field the dispatch
;    interrupt and to perform kernel initiated context switching.
;
;--

.586p
        .xlist
include ks386.inc
include i386\kimacro.inc
include mac386.inc
include callconv.inc
include irqli386.inc
FPOFRAME macro a, b
.FPO ( a, b, 0, 0, 0, 0 )
endm
        .list

        EXTRNP  KefAcquireSpinLockAtDpcLevel,1,,FASTCALL
        EXTRNP  KefReleaseSpinLockFromDpcLevel,1,,FASTCALL

        EXTRNP  HalClearSoftwareInterrupt,1,IMPORT,FASTCALL
        EXTRNP  HalRequestSoftwareInterrupt,1,IMPORT,FASTCALL

ifndef NT_UP

        EXTRNP  KiIdleSchedule,1,,FASTCALL

endif

        EXTRNP  KiCheckForSListAddress,1,,FASTCALL
        EXTRNP  KiQueueReadyThread,2,,FASTCALL
        EXTRNP  KiRetireDpcList,1,,FASTCALL
        EXTRNP  _KiQuantumEnd,0
        EXTRNP  _KeBugCheckEx,5

        extrn   _KiTrap13:PROC
        extrn   _KeFeatureBits:DWORD

        extrn   __imp__KeRaiseIrqlToSynchLevel@0:DWORD

        extrn   _KiIdleSummary:DWORD
        
        EXTRNP  WmiTraceContextSwap,2,,FASTCALL

if DBG
        extrn   _KdDebuggerEnabled:BYTE
        EXTRNP  _DbgBreakPoint,0
        EXTRNP  _KdPollBreakIn,0
        extrn   _DbgPrint:near
        extrn   _MsgDpcTrashedEsp:BYTE
        extrn   _MsgDpcTimeout:BYTE
        extrn   _KiDPCTimeout:DWORD
endif


_TEXT$00   SEGMENT PARA PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

cPublicFastCall KiRDTSC, 1
        rdtsc                   ; read the timestamp counter
        mov     [ecx], eax      ; return the low 32 bits
        mov     [ecx+4], edx    ; return the high 32 bits
        fstRET  KiRDTSC
fstENDP KiRDTSC

        page ,132
        subttl  "Swap Context"
;++
;
; BOOLEAN
; KiSwapContext (
;    IN PKTHREAD OldThread
;    IN PKTHREAD NewThread
;    )
;
; Routine Description:
;
;    This function is a small wrapper, callable from C code, that marshalls
;    arguments and calls the actual swap context routine.
;
; Arguments:
;
;    OldThread (ecx) - Supplies the address of the old thread
;    NewThread (edx) - Supplies the address of the new thread.
;
; Return Value:
;
;    If a kernel APC is pending, then a value of TRUE is returned. Otherwise,
;    a value of FALSE is returned.
;
;--

cPublicFastCall KiSwapContext, 2
.fpo (0, 0, 0, 4, 1, 0)

;
; N.B. The following registers MUST be saved such that ebp is saved last.
;      This is done so the debugger can find the saved ebp for a thread
;      that is not currently in the running state.
;

        sub     esp, 4*4
        mov     [esp+12], ebx           ; save registers
        mov     [esp+8], esi            ;
        mov     [esp+4], edi            ;
        mov     [esp+0], ebp            ;
        mov     ebx, PCR[PcSelfPcr]     ; set address of PCR
        mov     edi, ecx                ; set old thread address
        mov     esi, edx                ; set next thread address
        movzx   ecx, byte ptr [edi].ThWaitirql ; set APC interrupt bypass disable

        call    SwapContext             ; swap context
        mov     ebp, [esp+0]            ; restore registers
        mov     edi, [esp+4]            ;
        mov     esi, [esp+8]            ;
        mov     ebx, [esp+12]           ;
        add     esp, 4*4                ;
        fstRET  KiSwapContext           ;

fstENDP KiSwapContext

        page ,132
        subttl  "Dispatch Interrupt"
;++
;
; Routine Description:
;
;    This routine is entered as the result of a software interrupt generated
;    at DISPATCH_LEVEL. Its function is to process the Deferred Procedure Call
;    (DPC) list, and then perform a context switch if a new thread has been
;    selected for execution on the processor.
;
;    This routine is entered at IRQL DISPATCH_LEVEL with the dispatcher
;    database unlocked. When a return to the caller finally occurs, the
;    IRQL remains at DISPATCH_LEVEL, and the dispatcher database is still
;    unlocked.
;
; Arguments:
;
;    None
;
; Implicit Arguments:
;
;    ecx - Supplies the address of an optional trap frame.
;
; Return Value:
;
;    None.
;
;--

        align 16
cPublicProc _KiDispatchInterrupt ,0
cPublicFpo 0, 0

;
; Check if an SLIST pop operation is being interrupted and reset EIP as
; necessary.
;
; N.B. ecx is already loaded.
;

        test    ecx, ecx                ; check for NULL trap frame
        jz      short @f                ; if z, trap frame NULL
        fstCall KiCheckForSListAddress  ; check SLIST addresses
@@:                                     ; reference label

;
; Disable interrupts and check if there is any work in the DPC list
; of the current processor.
;

        mov     ebx, PCR[PcSelfPcr]     ; get address of PCR
kdi00:  cli                             ; disable interrupts
        mov     eax, [ebx]+PcPrcbData+PbDpcQueueDepth ; get DPC queue depth
        or      eax, [ebx]+PcPrcbData+PbTimerRequest ; merge timer request

ifndef NT_UP

        or      eax, [ebx]+PcPrcbData+PbDeferredReadyListHead ; merge deferred list head

endif

        jz      short kdi40             ; if z, no DPC's or timers to process
        push    ebp                     ; save register

;
; Exceptions occurring in DPCs are unrelated to any exception handlers
; in the interrupted thread.  Terminate the exception list.
;

        push    [ebx].PcExceptionList
        mov     [ebx].PcExceptionList, EXCEPTION_CHAIN_END

;
; Switch to the DPC stack for this processor.
;

        mov     edx, esp
        mov     esp, [ebx].PcPrcbData.PbDpcStack
        push    edx

.fpo (0, 0, 0, 1, 1, 0)

        mov     ecx, [ebx].PcPrcb       ; get current PRCB address
        fstCall KiRetireDpcList         ; process the current DPC list

;
; Switch back to the current thread stack, restore the exception list
; and saved EBP.
;

        pop     esp
        pop     [ebx].PcExceptionList
        pop     ebp 
.fpo (0, 0, 0, 0, 0, 0)

;
; Check to determine if quantum end is requested.
;
; N.B. If a new thread is selected as a result of processing the quantum
;      end request, then the new thread is returned with the dispatcher
;      database locked. Otherwise, NULL is returned with the dispatcher
;      database unlocked.
;

kdi40:  sti                             ; enable interrupts
        cmp     byte ptr [ebx].PcPrcbData.PbQuantumEnd, 0 ; quantum end requested
        jne     kdi90                   ; if neq, quantum end request

;
; Check to determine if a new thread has been selected for execution on this
; processor.
;

        cmp     dword ptr [ebx].PcPrcbData.PbNextThread, 0 ; check if next thread
        je      kdi70                   ; if eq, then no new thread

;
; N.B. The following registers MUST be saved such that ebp is saved last.
;      This is done so the debugger can find the saved ebp for a thread
;      that is not currently in the running state.
;

.fpo (0, 0, 0, 3, 1, 0)

        sub     esp, 3*4
        mov     [esp+8], esi            ; save registers
        mov     [esp+4], edi            ;
        mov     [esp+0], ebp            ;
        mov     edi, [ebx].PcPrcbData.PbCurrentThread ; get current thread address (as old thread)

;
; Raise IRQL to SYNCH level, set context swap busy for the old thread, and
; acquire the current PRCB lock.
;

ifndef NT_UP

        call    dword ptr [__imp__KeRaiseIrqlToSynchLevel@0] ; raise IRQL to SYNCH
        mov     byte ptr [edi].ThSwapBusy, 1 ; set context swap busy
        lea     ecx, [ebx].PcPrcbData.PbPrcbLock ; get PRCB lock address
   lock bts     dword ptr [ecx], 0      ; try to acquire PRCB lock
        jnc     short kdi50             ; if nc, PRCB lock acquired
        fstCall KefAcquireSpinLockAtDpcLevel ; acquire current PRCB lock

endif

;
; Get the next thread address, set the thread state to running, queue the old
; running thread, and swap context to the next thread.
;

kdi50:  mov     esi, [ebx].PcPrcbData.PbNextThread ; get next thread address
        and     dword ptr [ebx].PcPrcbData.PbNextThread, 0 ; clear next thread address
        mov     [ebx].PcPrcbData.PbCurrentThread, esi ; set current thread address
        mov     byte ptr [esi]+ThState, Running ; set thread state to running
        mov     byte ptr [edi].ThWaitReason, WrDispatchInt  ; set wait reason
        mov     ecx, edi                ; set address of curent thread
        lea     edx, [ebx].PcPrcbData   ; set address of PRCB
        fstCall KiQueueReadyThread      ; ready thread for execution
        mov     cl, APC_LEVEL           ; set APC interrupt bypass disable
        call    SwapContext             ; swap context
        mov     ebp, [esp+0]            ; restore registers
        mov     edi, [esp+4]            ;
        mov     esi, [esp+8]            ;
        add     esp, 3*4
kdi70:  stdRET  _KiDispatchInterrupt    ; return

;
; Process quantum end event.
;
; N.B. If the quantum end code returns a NULL value, then no next thread
;      has been selected for execution. Otherwise, a next thread has been
;      selected and the source thread lock has been acquired.
;

kdi90:  mov     byte ptr [ebx].PcPrcbData.PbQuantumEnd, 0 ; clear quantum end indicator
        stdCall _KiQuantumEnd           ; process quantum end
        stdRET  _KiDispatchInterrupt    ; return

stdENDP _KiDispatchInterrupt

        page ,132
        subttl  "Swap Context to Next Thread"
;++
;
; Routine Description:
;
;    This routine is called to swap context from one thread to the next.
;    It swaps context, flushes the data, instruction, and translation
;    buffer caches, restores nonvolatile integer registers, and returns
;    to its caller.
;
;    N.B. It is assumed that the caller (only callers are within this
;         module) saved the nonvolatile registers, ebx, esi, edi, and
;         ebp. This enables the caller to have more registers available.
;
; Arguments:
;
;    cl - APC interrupt bypass disable (zero enable, nonzero disable).
;    edi - Address of previous thread.
;    esi - Address of next thread.
;    ebx - Address of PCR.
;
; Return value:
;
;    al - Kernel APC pending.
;    ebx - Address of PCR.
;    esi - Address of current thread object.
;
;--

;
;   NOTE:   The ES: override on the move to ThState is part of the
;           lazy-segment load system.  It assures that ES has a valid
;           selector in it, thus preventing us from propagating a bad
;           ES across a context switch.
;
;           Note that if segments, other than the standard flat segments,
;           with limits above 2 gig exist, neither this nor the rest of
;           lazy segment loads are reliable.
;
; Note that ThState must be set before the dispatcher lock is released
; to prevent KiSetPriorityThread from seeing a stale value.
;

ifndef NT_UP

        public  _ScPatchFxb
        public  _ScPatchFxe

endif

        public  SwapContext

        align   16

SwapContext     proc

;
; Save the APC disable flag.
;
        push    ecx                     ; save APC bypass disable
cPublicFpo 0, 1

;
; Wait for context to be swapped for the target thread.
;

ifndef NT_UP

sc00:   cmp     byte ptr [esi].ThSwapBusy, 0 ; check if context swap busy
        je      short sc01              ; if e, context swap idle
        YIELD                           ; yield execution for SMT system
        jmp     short sc00              ;

endif

;
; Increment the number of context switches on this processor.
;
; N.B. This increment is done here is force the cache block containing the
;      context switch count into the cache as write exclusive. There are
;      several other references to this cache block in the following code.
;

sc01:   inc     es:dword ptr [ebx]+PcContextSwitches ; processor count

;
; Save the thread exception list head.
;

        push    [ebx]+PcExceptionList   ; save thread exception list head

cPublicFpo 0, 2

;
; Check for context swap logging.
;

        cmp     [ebx]+PcPerfGlobalGroupMask, 0 ; check if logging enable
        jne     sc92                    ; If not, then check if we are enabled
sc03:

ifndef NT_UP

if DBG

        mov     cl, [esi]+ThNextProcessor ; get current processor number
        cmp     cl, [ebx]+PcPrcbData+PbNumber ; same as running processor?
        jne     sc_error2               ; if ne, processor number mismatch

endif

endif

;
; On a uniprocessor system the NPX state is swapped in a lazy manner.
; If a thread whose state is not in the coprocessor attempts to perform
; a coprocessor operation, the current NPX state is swapped out (if needed),
; and the new state is swapped in during the fault.  (KiTrap07)
;
; On a multiprocessor system we still fault in the NPX state on demand, but
; we save the state when the thread switches out (assuming the NPX state
; was loaded).  This is because it could be difficult to obtain the thread's
; NPX in the trap handler if it was loaded into a different processor's
; coprocessor.
;

        mov     ebp, cr0                ; get current CR0
        mov     edx, ebp                ;

ifndef NT_UP

        cmp     byte ptr [edi]+ThNpxState, NPX_STATE_LOADED ; check if NPX state
        je      sc_save_npx_state       ; if e, NPX state not loaded

endif

;
; Save the old stack pointer and compute the new stack limits.
;

sc05:   mov     [edi]+ThKernelStack, esp ; save old kernel stack pointer
        mov     eax, [esi]+ThInitialStack ; get new initial stack pointer


;
; (eax) = Initial Stack
; (ebx) = PCR
; (edi) = OldThread
; (esi) = NewThread
; (ebp) = Current CR0
; (edx) = Current CR0
;

.errnz (NPX_STATE_NOT_LOADED - CR0_TS - CR0_MP)
.errnz (NPX_STATE_LOADED - 0)

ifdef NT_UP
;
; On UP systems floating point state might be being changed by an ISR so we
; block interrupts.
;
        cli
endif
        movzx   ecx, byte ptr [esi]+ThNpxState ; new NPX state is (or is not) loaded
        and     edx, NOT (CR0_MP+CR0_EM+CR0_TS) ; clear thread settable NPX bits
        or      ecx, edx                ; or in new thread's cr0
        or      ecx, [eax]+FpCr0NpxState-NPX_FRAME_LENGTH ; merge new thread settable state
        cmp     ebp, ecx                ; check if old and new CR0 match
        jne     sc_reload_cr0           ; if ne, change in CR0
sc06:

ifdef NT_UP
        sti
endif

if DBG
        mov     eax, [esi]+ThKernelStack ; set new stack pointer
        cmp     esi, dword ptr [eax-4]
        je      @f
        int     3
@@:
        xchg    esp, eax
        mov     [eax-4], edi             ; Save thread address on stack below stack pointer
else
        mov     esp, [esi]+ThKernelStack ; set new stack pointer
endif


;
; Check if the old process is the same as the new process.
;

        mov     ebp, [esi].ThApcState.AsProcess ; get old process address
        mov     eax, [edi].ThApcState.AsProcess ; get old process address
        cmp     ebp, eax                        ; check if process match
        jz      short sc23                      ; if z, process match

;
; Set the processor bit in the new process and clear the old.
;

ifndef NT_UP

        mov     ecx, [ebx]+PcSetMemberCopy ; get processor set member
   lock xor     [ebp]+PrActiveProcessors, ecx ; set bit in new processor set
   lock xor     [eax]+PrActiveProcessors, ecx ; clear bit in old processor set

if DBG

        test    [ebp]+PrActiveProcessors, ecx ; test if bit set in new set
        jz      sc_error5               ; if z, bit not set in new set
        test    [eax]+PrActiveProcessors, ecx ; test if bit clear in old set
        jnz     sc_error4               ; if nz, bit not clear in old set
endif

endif

;
; LDT switch, If either the target or source process have an LDT we need to
; load the ldt
;

        mov     ecx, [ebp]+PrLdtDescriptor
        or      ecx, [eax]+PrLdtDescriptor
        jnz     sc_load_ldt             ; if nz, LDT limit
sc_load_ldt_ret:

;
; Load the new CR3 and as a side effect flush non-global TB entries.
;

        mov     eax, [ebp]+PrDirectoryTableBase ; get new directory base
        mov     cr3, eax                ; and flush TB


;
; Set context swap idle for the old thread.
;

sc23:                                   ;

ifndef NT_UP

        and     byte ptr [edi].ThSwapBusy, 0 ; clear old thread swap busy

endif

        xor     eax, eax
        mov     gs, eax

;
; Set the TEB descriptor to point to the thread TEB and set the TEB address
; in the PCR. The es override here is to force lazy segment loading to occur.
;
        mov     eax, [esi]+ThTeb        ; get user TEB address
        mov     [ebx]+PcTeb, eax        ; set user TEB address
        mov     ecx, [ebx]+PcGdt        ; get GDT address
        mov     [ecx]+(KGDT_R3_TEB+KgdtBaseLow), ax ;
        shr     eax, 16                 ;
        mov     [ecx]+(KGDT_R3_TEB+KgdtBaseMid), al ;
        mov     [ecx]+(KGDT_R3_TEB+KgdtBaseHi), ah ;

;
; Adjust the initial stack address, if necessary, and store in the TSS so V86
; mode threads and 32 bit threads can share a common trapframe structure and
; the NPX save area will be accessible in the same manner on all threads.
;

        mov     eax, [esi].ThInitialStack ; get initial stack address
        sub     eax, NPX_FRAME_LENGTH
.errnz (EFLAGS_V86_MASK AND 0FF00FFFFh)
        test    byte ptr [eax] - KTRAP_FRAME_LENGTH + TsEFlags + 2, EFLAGS_V86_MASK / 10000h
        jnz     short sc24              ; if nz, V86 frame, no adjustment
        sub     eax, TsV86Gs - TsHardwareSegSs ; bias for missing fields
sc24:   mov     ecx, [ebx]+PcTssCopy    ; get TSS address
        mov     [ecx]+TssEsp0, eax      ; set initial kernel stack address

;
; Set the IOPM map offset value.
;
; N.B. This may be a redundant load of this value if the process did not
;      change during the context switch. However, always reloading this
;      value saves several instructions under the context swap lock.
;

        mov     ax, [ebp]+PrIopmOffset  ; set IOPM offset
        mov     [ecx]+TssIoMapBase, ax  ;
;
; Update context switch counters.
;

        inc     dword ptr [esi]+ThContextSwitches ; thread count

;
; Restore thread exception list head and get APC bypass disable.
;

        pop     [ebx].PcExceptionList   ; restore thread exception list head
        pop     ecx                     ; get APC bypass disable

;
; Check if an attempt is being made to context switch while in a DPC routine.
;

        cmp     byte ptr [ebx]+PcPrcbData+PbDpcRoutineActive, 0 ; check if DPC active
        jne     sc91                    ; bugcheck if DPC active.

;
; If the new thread has a kernel mode APC pending, then request an APC
; interrupt.
;

        cmp     byte ptr [esi].ThApcState.AsKernelApcPending, 0 ; APC pending?
        jne     short sc80              ; if ne, kernel APC pending
        xor     eax, eax                ; set return value
        ret                             ; return

;
; The new thread has an APC interrupt pending.
;
; If the the special APC disable count is nonzero, then return no kernel APC
; pending. An APC will be requested when the special APC disable count reaches
; zero. 
;
; If APC interrupt bypass is not enabled, then request a software interrupt
; at APC_LEVEL and return no kernel APC pending. Otherwise, return kernel APC
; pending.
;

sc80:   cmp     word ptr [esi].ThSpecialApcDisable, 0 ; check if special APC disable
        jne     short sc90              ; if ne, special APC disable
        test    cl, cl                  ; test for APC bypass disable
        jz      short sc90              ; if z, APC bypass enabled
        mov     cl, APC_LEVEL           ; request software interrupt level
        fstCall HalRequestSoftwareInterrupt ;
        or      eax, esp                ; clear ZF flag
sc90:   setz    al                      ; set return value
        ret                             ; return


;
; Set for new LDT value
;

sc_load_ldt:
        mov     eax, [ebp+PrLdtDescriptor] ;
        test    eax, eax
        je      @f
        mov     ecx, [ebx]+PcGdt        ; get GDT address
        mov     [ecx+KGDT_LDT], eax     ;
        mov     eax, [ebp+PrLdtDescriptor+4] ;
        mov     [ecx+KGDT_LDT+4], eax   ;

;
; Set up int 21 descriptor of IDT.  If the process does not have an Ldt, it
; should never make any int 21 calls.  If it does, an exception is generated. If
; the process has an Ldt, we need to update int21 entry of LDT for the process.
; Note the Int21Descriptor of the process may simply indicate an invalid
; entry.  In which case, the int 21 will be trapped to the kernel.
;

        mov     ecx, [ebx]+PcIdt        ;
        mov     eax, [ebp+PrInt21Descriptor] ;
        mov     [ecx+21h*8], eax        ;
        mov     eax, [ebp+PrInt21Descriptor+4] ;
        mov     [ecx+21h*8+4], eax      ;
        mov     eax, KGDT_LDT
@@:     lldt    ax
        jmp     sc_load_ldt_ret

;
; Cr0 has changed (ie, floating point processor present), load the new value.
;

sc_reload_cr0:

if DBG

        test    byte ptr [esi]+ThNpxState, NOT (CR0_TS+CR0_MP)
        jnz     sc_error                ;
        test    dword ptr [eax]+FpCr0NpxState-NPX_FRAME_LENGTH, NOT (CR0_PE+CR0_MP+CR0_EM+CR0_TS)
        jnz     sc_error3               ;

endif

        mov     cr0,ecx                 ; set new CR0 NPX state
        jmp     sc06

;
; Save coprocessor's current context.  FpCr0NpxState is the current thread's
; CR0 state.  The following bits are valid: CR0_MP, CR0_EM, CR0_TS.  MVDMs
; may set and clear MP & EM as they please and the settings will be reloaded
; on a context switch (but they will not be saved from CR0 to Cr0NpxState).
; The kernel sets and clears TS as required.
;
; (ebp) = Current CR0
; (edx) = Current CR0
;

ifndef NT_UP

sc_save_npx_state:
        and     edx, NOT (CR0_MP+CR0_EM+CR0_TS) ; we need access to the NPX state

        mov     ecx, [edi].ThInitialStack        ; get NPX save save area address
        sub     ecx, NPX_FRAME_LENGTH

        cmp     ebp, edx                        ; Does CR0 need reloading?
        je      short sc_npx10

        mov     cr0, edx                        ; set new cr0
        mov     ebp, edx                        ; (ebp) = (edx) = current cr0 state

sc_npx10:

;
; The fwait following the fnsave is to make sure that the fnsave has stored the
; data into the save area before this coprocessor state could possibly be
; context switched in and used on a different (co)processor.  I've added the
; clocks from when the dispatcher lock is released and don't believe it's a
; possibility.  I've also timed the impact this fwait seems to have on a 486
; when performing lots of numeric calculations.  It appears as if there is
; nothing to wait for after the fnsave (although the 486 manual says there is)
; and therefore the calculation time far outweighed the 3clk fwait and it
; didn't make a noticable difference.
;

;
; If FXSR feature is NOT present on the processor, the fxsave instruction is
; patched at boot time to start using fnsave instead
;

_ScPatchFxb:
;       fxsave  [ecx]                   ; save NPX state
        db      0FH, 0AEH, 01
_ScPatchFxe:

        mov     byte ptr [edi]+ThNpxState, NPX_STATE_NOT_LOADED ; set no NPX state
        mov     dword ptr [ebx].PcPrcbData+PbNpxThread, 0  ; clear npx owner
        jmp     sc05
endif

;
; This code is out of line to optimize the normal case with tracing is off.
;

sc92:   mov     eax, [ebx]+PcPerfGlobalGroupMask ; Load the ptr into eax
        cmp     eax, 0                  ; catch race condition on pointer here
        jz      sc03                    ; instead of above in mainline code
        mov     edx, esi                ; pass the new ETHREAD object
        mov     ecx, edi                ; pass the old ETHREAD object
        test    dword ptr [eax+PERF_CONTEXTSWAP_OFFSET], PERF_CONTEXTSWAP_FLAG
        jz      sc03                    ; return if our flag is not set

        fstCall WmiTraceContextSwap     ; call the Wmi context swap trace
        jmp     sc03                    ;

;
; A context switch was attempted while executing a DPC - bugcheck.
;

.fpo (2, 0, 0, 0, 0, 0)
sc91:
        mov     eax, [edi]+ThInitialStack ; get the old stack so it can
                                          ; be saved in the minidump
        stdCall _KeBugCheckEx <ATTEMPTED_SWITCH_FROM_DPC, edi, esi, eax, 0>
        ret                             ; return

if DBG
sc_error5:  int 3
sc_error4:  int 3
sc_error3:  int 3
sc_error2:  int 3
sc_error:   int 3
endif

SwapContext     endp

        page , 132
        subttl "Flush EntireTranslation Buffer"
;++
;
; VOID
; KeFlushCurrentTb (
;     )
;
; Routine Description:
;
;     This function flushes the entire translation buffer (TB) on the current
;     processor and also flushes the data cache if an entry in the translation
;     buffer has become invalid.
;
; Arguments:
;
; Return Value:
;
;     None.
;
;--

cPublicProc _KeFlushCurrentTb ,0

ktb00:  mov     eax, cr3                ; (eax) = directory table base
        mov     cr3, eax                ; flush TLB
        stdRET    _KeFlushCurrentTb

ktb_gb: mov     eax, cr4                ; *** see Ki386EnableGlobalPage ***
        and     eax, not CR4_PGE        ; This FlushCurrentTb version gets copied into
        mov     cr4, eax                ; ktb00 at initialization time if needed.
        or      eax, CR4_PGE
        mov     cr4, eax
ktb_eb: stdRET    _KeFlushCurrentTb

stdENDP _KeFlushCurrentTb
        ;;
        ;; moved KiFlushDcache below KeFlushCurrentTb for BBT purposes.  BBT
        ;; needs an end label to treat KeFlushCurrentTb as data and to keep together.
        ;;
        page , 132
        subttl "Flush Data Cache"
;++
;
; VOID
; KiFlushDcache (
;     )
;
; VOID
; KiFlushIcache (
;     )
;
; Routine Description:
;
;   This routine does nothing on i386 and i486 systems.   Why?  Because
;   (a) their caches are completely transparent,  (b) they don't have
;   instructions to flush their caches.
;
; Arguments:
;
;     None.
;
; Return Value:
;
;     None.
;
;--

cPublicProc _KiFlushDcache  ,0
cPublicProc _KiFlushIcache  ,0

        stdRET    _KiFlushIcache

stdENDP _KiFlushIcache
stdENDP _KiFlushDcache


_TEXT$00   ends

INIT    SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

;++
;
; VOID
; Ki386EnableGlobalPage (
;     IN volatile PLONG Number
;     )
;
; /*++
;
; Routine Description:
;
;     This routine enables the global page PDE/PTE support in the system,
;     and stalls until complete and them sets the current processor's cr4
;     register to enable global page support.
;
; Arguments:
;
;     Number - Supplies a pointer to the count of the number of processors in
;     the configuration.
;
; Return Value:
;
;     None.
;--

cPublicProc _Ki386EnableGlobalPage,1
        push    esi
        push    edi
        push    ebx

        mov     edx, [esp+16]           ; pointer to Number
        pushfd
        cli

;
; Wait for all processors
;
        lock dec dword ptr [edx]        ; count down
egp10:  YIELD
        cmp     dword ptr [edx], 0      ; wait for all processors to signal
        jnz     short egp10

        cmp     byte ptr PCR[PcNumber], 0 ; processor 0?
        jne     short egp20

;
; Install proper KeFlushCurrentTb function.
;

        mov     edi, ktb00
        mov     esi, ktb_gb
        mov     ecx, ktb_eb - ktb_gb + 1
        rep movsb

        mov     byte ptr [ktb_eb], 0

;
; Wait for P0 to signal that proper flush TB handlers have been installed
;
egp20:  cmp     byte ptr [ktb_eb], 0
        jnz     short egp20

;
; Flush TB, and enable global page support
; (note load of CR4 is explicitly done before the load of CR3
; to work around P6 step B0 errata 11)
;
        mov     eax, cr4
        and     eax, not CR4_PGE        ; should not be set, but let's be safe
        mov     ecx, cr3
        mov     cr4, eax

        mov     cr3, ecx                ; Flush TB

        or      eax, CR4_PGE            ; enable global TBs
        mov     cr4, eax
        popfd
        pop     ebx
        pop     edi
        pop     esi

        stdRET  _Ki386EnableGlobalPage
stdENDP _Ki386EnableGlobalPage

;++
;
; VOID
; Ki386EnableDE (
;     IN volatile PLONG Number
;     )
;
; /*++
;
; Routine Description:
;
;     This routine sets DE bit in CR4 to enable IO breakpoints
;
; Arguments:
;
;     Number - Supplies a pointer to the count of the number of processors in
;     the configuration.
;
; Return Value:
;
;     None.
;--

cPublicProc _Ki386EnableDE,1

        mov     eax, cr4
        or      eax, CR4_DE
        mov     cr4, eax

        stdRET  _Ki386EnableDE
stdENDP _Ki386EnableDE


;++
;
; VOID
; Ki386EnableFxsr (
;     IN volatile PLONG Number
;     )
;
; /*++
;
; Routine Description:
;
;     This routine sets OSFXSR bit in CR4 to indicate that OS supports
;     FXSAVE/FXRSTOR for use during context switches
;
; Arguments:
;
;     Number - Supplies a pointer to the count of the number of processors in
;     the configuration.
;
; Return Value:
;
;     None.
;--

cPublicProc _Ki386EnableFxsr,1

        mov     eax, cr4
        or      eax, CR4_FXSR
        mov     cr4, eax

        stdRET  _Ki386EnableFxsr
stdENDP _Ki386EnableFxsr


;++
;
; VOID
; Ki386EnableXMMIExceptions (
;     IN volatile PLONG Number
;     )
;
; /*++
;
; Routine Description:
;
;     This routine installs int 19 XMMI unmasked Numeric Exception handler
;     and sets OSXMMEXCPT bit in CR4 to indicate that OS supports
;     unmasked Katmai New Instruction technology exceptions.
;
; Arguments:
;
;     Number - Supplies a pointer to count of the number of processors in
;     the configuration.
;
; Return Value:
;
;     None.
;--

cPublicProc _Ki386EnableXMMIExceptions,1


        ;Set up IDT for INT19
        mov     ecx,PCR[PcIdt]              ;Get IDT address
        lea     eax, [ecx] + 098h           ;XMMI exception is int 19
        mov     byte ptr [eax + 5], 08eh    ;P=1,DPL=0,Type=e
        mov     word ptr [eax + 2], KGDT_R0_CODE ;Kernel code selector
        mov     edx, offset FLAT:_KiTrap13  ;Address of int 19 handler
        mov     ecx,edx
        mov     word ptr [eax],cx           ;addr moves into low byte
        shr     ecx,16
        mov     word ptr [eax + 6],cx       ;addr moves into high byte
        ;Enable XMMI exception handling
        mov     eax, cr4
        or      eax, CR4_XMMEXCPT
        mov     cr4, eax

        stdRET  _Ki386EnableXMMIExceptions
stdENDP _Ki386EnableXMMIExceptions


;++
;
; VOID
; Ki386EnableCurrentLargePage (
;     IN ULONG IdentityAddr,
;     IN ULONG IdentityCr3
;     )
;
; /*++
;
; Routine Description:
;
;     This routine enables the large page PDE support in the processor.
;
; Arguments:
;
;     IdentityAddr - Supplies the linear address of the beginning of this
;     function where (linear == physical).
;
;     IdentityCr3 - Supplies a pointer to the temporary page directory and
;     page tables that provide both the kernel (virtual ->physical) and
;     identity (linear->physical) mappings needed for this function.
;
; Return Value:
;
;     None.
;--

public _Ki386EnableCurrentLargePageEnd
cPublicProc _Ki386EnableCurrentLargePage,2
        mov     ecx,[esp]+4             ; (ecx)-> IdentityAddr
        mov     edx,[esp]+8             ; (edx)-> IdentityCr3
        pushfd                          ; save current IF state
        cli                             ; disable interrupts

        mov     eax, cr3                ; (eax)-> original Cr3
        mov     cr3, edx                ; load Cr3 with Identity mapping

        sub     ecx, offset _Ki386EnableCurrentLargePage
        add     ecx, offset _Ki386LargePageIdentityLabel
        jmp     ecx                     ; jump to (linear == physical)

_Ki386LargePageIdentityLabel:
        mov    ecx, cr0
        and    ecx, NOT CR0_PG          ; clear PG bit to disable paging
        mov    cr0, ecx                 ; disable paging
        jmp    $+2
        mov     edx, cr4
        or      edx, CR4_PSE            ; enable Page Size Extensions
        mov     cr4, edx
        mov     edx, offset OriginalMapping
        or      ecx, CR0_PG             ; set PG bit to enable paging
        mov     cr0, ecx                ; enable paging
        jmp     edx                     ; Return to original mapping.

OriginalMapping:
        mov     cr3, eax                ; restore original Cr3
        popfd                           ; restore interrupts to previous

        stdRET  _Ki386EnableCurrentLargePage

_Ki386EnableCurrentLargePageEnd:

stdENDP _Ki386EnableCurrentLargePage

INIT    ends

_TEXT$00   SEGMENT PARA PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

        page , 132
        subttl "Swap Process"
;++
;
; VOID
; KiSwapProcess (
;     IN PKPROCESS NewProcess,
;     IN PKPROCESS OldProcess
;     )
;
; Routine Description:
;
;     This function swaps the address space to another process by flushing
;     the data cache, the instruction cache, the translation buffer, and
;     establishes a new directory table base.
;
;     It also swaps in the LDT and IOPM of the new process.  This is necessary
;     to avoid bogus mismatches in SwapContext.
;
;     NOTE: keep in sync with process switch part of SwapContext
;
; Arguments:
;
;     Process - Supplies a pointer to a control object of type process.
;
; Return Value:
;
;     None.
;
;--

cPublicProc _KiSwapProcess  ,2
cPublicFpo 2, 0

        mov     edx,[esp]+4             ; (edx)-> New process
        mov     eax,[esp]+8             ; (eax)-> Old Process

;
; Set the processor number in the new process and clear it in the old.
;

ifndef NT_UP

        mov     ecx, PCR[PcSetMember]
   lock xor     [edx]+PrActiveProcessors,ecx ; set bit in new processor set
   lock xor     [eax]+PrActiveProcessors,ecx ; clear bit in old processor set

if DBG

        test    [edx]+PrActiveProcessors,ecx ; test if bit set in new set
        jz      kisp_error1             ; if z, bit not set in new set
        test    [eax]+PrActiveProcessors,ecx ; test if bit clear in old set
        jnz     kisp_error              ; if nz, bit not clear in old set

endif

endif

;
;   Change LDT, If either the source or target process has an LDT we need to
;   load the new one.
;

        mov     ecx, [edx]+PrLdtDescriptor
        or      ecx, [eax]+PrLdtDescriptor
        jnz     kisp_load_ldt           ; if nz, LDT limit
kisp_load_ldt_ret:                      ; if nz, LDT limit

;
; Load the new CR3 and as a side effect flush non-global TB entries.
;

        mov     eax,[edx]+PrDirectoryTableBase
        mov     cr3,eax

        mov     ecx,PCR[PcTssCopy]      ; (ecx)-> TSS

;
;   Clear gs so it can't leak across processes
;

        xor     eax,eax                         ; assume ldtr is to be NULL
        mov     gs,ax                           ; Clear gs.  (also workarounds

;
;   Change IOPM
;

        mov     ax,[edx]+PrIopmOffset
        mov     [ecx]+TssIoMapBase,ax

        stdRET    _KiSwapProcess

kisp_load_ldt:

;
;   Edit LDT descriptor
;

        mov     eax,[edx+PrLdtDescriptor]
        test    eax, eax
        je      @f
        mov     ecx,PCR[PcGdt]
        mov     [ecx+KGDT_LDT],eax
        mov     eax,[edx+PrLdtDescriptor+4]
        mov     [ecx+KGDT_LDT+4],eax

;
;   Set up int 21 descriptor of IDT.  If the process does not have Ldt, it
;   should never make any int 21 call.  If it does, an exception is generated.
;   If the process has Ldt, we need to update int21 entry of LDT for the process.
;   Note the Int21Descriptor of the process may simply indicate an invalid
;   entry.  In which case, the int 21 will be trapped to the kernel.
;

        mov     ecx, PCR[PcIdt]
        mov     eax, [edx+PrInt21Descriptor]
        mov     [ecx+21h*8], eax
        mov     eax, [edx+PrInt21Descriptor+4]
        mov     [ecx+21h*8+4], eax

        mov     eax,KGDT_LDT                    ;@@32-bit op to avoid prefix
@@:     lldt    ax
        jmp     kisp_load_ldt_ret

if DBG
kisp_error1: int 3
kisp_error:  int 3
endif

stdENDP _KiSwapProcess

        page ,132
        subttl  "Idle Loop"
;++
;
; VOID
; KiIdleLoop(
;     VOID
;     )
;
; Routine Description:
;
;    This routine continuously executes the idle loop and never returns.
;
; Arguments:
;
;    ebx - Address of the current processor's PCR.
;
; Return value:
;
;    None - routine never returns.
;
;--

cPublicFastCall KiIdleLoop  ,0
cPublicFpo 0, 0

if DBG

        xor     edi, edi                ; reset poll breakin counter

endif

        jmp     short kid20             ; Skip HalIdleProcessor on first iteration

;
; There are no entries in the DPC list and a thread has not been selected
; for execution on this processor. Call the HAL so power managment can be
; performed.
;
; N.B. The HAL is called with interrupts disabled. The HAL will return
;      with interrupts enabled.
;
; N.B. Use a call instruction instead of a push-jmp, as the call instruction
;      executes faster and won't invalidate the processor's call-return stack
;      cache.
;

kid10:  lea     ecx, [ebx].PcPrcbData.PbPowerState
        call    dword ptr [ecx].PpIdleFunction      ; (ecx) = Arg0

;
; Give the debugger an opportunity to gain control on debug systems.
;
; N.B. On an MP system the lowest numbered idle processor is the only
;      processor that polls for a breakin request.
;

kid20:

if DBG
ifndef NT_UP

        mov     eax, _KiIdleSummary     ; get idle summary
        mov     ecx, [ebx].PcSetMember  ; get set member
        dec     ecx                     ; compute right bit mask
        and     eax, ecx                ; check if any lower bits set
        jnz     short CheckDpcList      ; if nz, not lowest numbered

endif

        dec     edi                     ; decrement poll counter
        jg      short CheckDpcList      ; if g, not time to poll

        POLL_DEBUGGER                   ; check if break in requested
endif

kid30:

if DBG

ifndef NT_UP

        mov     edi, 20 * 1000          ; set breakin poll interval

else

        mov     edi, 100                ; UP idle loop has a HLT in it

endif

endif

CheckDpcList0:                          ;
        YIELD

;
; Disable interrupts and check if there is any work in the DPC list of the
; current processor or a target processor.
;

CheckDpcList:

;
; N.B. The following code enables interrupts for a few cycles, then
;      disables them again for the subsequent DPC and next thread
;      checks.
;

        sti                             ; enable interrupts
        nop                             ;
        nop                             ;
        cli                             ; disable interrupts

;
; Process the deferred procedure call list for the current processor.
;

        mov     eax, [ebx]+PcPrcbData+PbDpcQueueDepth ; get DPC queue depth
        or      eax, [ebx]+PcPrcbData+PbTimerRequest ; merge timer request

ifndef NT_UP

        or      eax, [ebx]+PcPrcbData+PbDeferredReadyListHead ; merge deferred list head

endif

        jz      short CheckNextThread   ; if z, no DPC's or timers to process
        mov     cl, DISPATCH_LEVEL      ; set interrupt level
        fstCall HalClearSoftwareInterrupt ; clear software interrupt
        lea     ecx, [ebx].PcPrcbData   ; set current PRCB address
        fstCall KiRetireDpcList         ; process the current DPC list

if DBG

        xor     edi, edi                ; clear breakin poll interval

endif

;
; Check if a thread has been selected to run on the current processor.
;

CheckNextThread:                        ;
        cmp     dword ptr [ebx].PcPrcbData.PbNextThread, 0 ; thread selected?

ifdef NT_UP

        je      short kid10             ; if eq, no thread selected

else

        je      kid40                   ; if eq, no thread selected.

endif

;
; Raise IRQL to synchronization level and enable interrupts.
;

ifndef NT_UP

        RaiseIrql SYNCH_LEVEL, NoOld    ; raise IRQL to synchronization level

endif

        sti                             ; enable interrupts
        mov     edi, [ebx].PcPrcbData.PbCurrentThread ; get idle thread address

;
; Set context swap busy for idle thread and acquire the PRCB lock.
;

ifndef NT_UP

        mov     byte ptr [edi].ThSwapBusy, 1 ; set context swap busy
   lock bts     dword ptr [ebx].PcPrcbData.PbPrcbLock, 0 ; try to acquire PRCB Lock
        jnc     short kid33             ; if nc, PRCB lock acquired
        lea     ecx, [ebx].PcPrcbData.PbPrcbLock ; get PRCB lock address
        fstCall KefAcquireSpinLockAtDpcLevel ; acquire current PRCB lock

endif

;
; If a thread had been scheduled for this processor but was removed from
; eligibility (e.g., an affinity change), then the new thread could be the
; idle thread.
;

kid33:  mov     esi, [ebx].PcPrcbData.PbNextThread ; get next thread address

ifndef NT_UP
                                        
        cmp     esi, edi                ; check if idle thread
        je      short kisame            ; if e, processor idle again

endif

        and     dword ptr [ebx].PcPrcbData.PbNextThread, 0 ; clear next thread
        mov     [ebx].PcPrcbData.PbCurrentThread, esi ; set new thread address
        mov     byte  ptr [esi]+ThState, Running ; set thread state running

;
; Clear idle schedule since a new thread has been selected for execution on
; this processor and release the PRCB lock.
;

ifndef NT_UP

        and     byte ptr [ebx].PcPrcbData.PbIdleSchedule, 0 ; clear idle schedule
        and     dword ptr [ebx].PcPrcbData.PbPrcbLock, 0 ; release current PRCB lock

endif

kid35:                                  ;

        mov     ecx, APC_LEVEL          ; set APC bypass disable
        call    SwapContext             ; swap context

ifndef NT_UP

        LowerIrql DISPATCH_LEVEL        ; lower IRQL to dispatch level

endif

        jmp     kid30                   ;

;
; The new thread is the Idle thread (same as old thread).  This can happen
; rarely when a thread scheduled for this processor is made unable to run
; on this processor. As this processor has again been marked idle, other
; processors may unconditionally assign new threads to this processor.
;

ifndef NT_UP

kisame: and     dword ptr [ebx].PcPrcbData.PbNextThread, 0 ; clear next thread
        and     dword ptr [ebx].PcPrcbData.PbPrcbLock, 0 ; release current PRCB lock
        and     byte ptr [edi].ThSwapBusy, 0 ; set idle thread context swap idle
        jmp     kid30                   ;

;
; Call idle schedule if requested.
;

kid40:  cmp     byte ptr [ebx].PcPrcbData.PbIdleSchedule, 0 ; check if idle schedule
        je      kid10                   ; if e, idle schedule not requested
        sti                             ; enable interrupts
        lea     ecx, [ebx].PcPrcbData   ; get current PRCB address
        fstCall KiIdleSchedule          ; attempt to schedule thread
        test    eax, eax                ; test if new thread schedule
        mov     esi, eax                ; set new thread address
        mov     edi, [ebx].PcPrcbData.PbIdleThread ; get idle thread address
        jnz     short kid35             ; if nz, new thread scheduled
        jmp     kid30                   ;

endif

fstENDP KiIdleLoop

ifdef DBGMP
cPublicProc _KiPollDebugger,0
cPublicFpo 0,3
        push    eax
        push    ecx
        push    edx
        POLL_DEBUGGER
        pop     edx
        pop     ecx
        pop     eax
        stdRET    _KiPollDebugger
stdENDP _KiPollDebugger

endif

        page , 132
        subttl "Adjust TSS ESP0 value"
;++
;
; VOID
; KiAdjustEsp0 (
;     IN PKTRAP_FRAME TrapFrame
;     )
;
; Routine Description:
;
;     This routine puts the appropriate ESP0 value in the esp0 field of the
;     TSS.  This allows protect mode and V86 mode to use the same stack
;     frame.  The ESP0 value for protected mode is 16 bytes lower than
;     for V86 mode to compensate for the missing segment registers.
;
; Arguments:
;
;     TrapFrame - Supplies a pointer to the TrapFrame.
;
; Return Value:
;
;     None.
;
;--
cPublicProc _Ki386AdjustEsp0 ,1

if DBG

        ;
        ; Make sure we are not called when the trap frame can be
        ; edited by a SetContextThread.
        ;

        CurrentIrql
        cmp     al, APC_LEVEL
        jge     @f
        int     3
@@:

endif

        mov     eax, PCR[PcPrcbData.PbCurrentThread] ; get current thread address
        mov     edx, [esp + 4]          ; edx -> trap frame
        mov     eax, [eax]+ThInitialStack ; eax = base of stack
        test    dword ptr [edx]+TsEFlags, EFLAGS_V86_MASK ; is this a V86 frame?
        jnz     short ae10              ; if nz, V86 frame
        sub     eax, TsV86Gs - TsHardwareSegSS ; compensate for missing regs
ae10:   sub     eax, NPX_FRAME_LENGTH   ; 
        pushfd                          ; Make sure we don't move
        cli                             ; processors while we do this
        mov     edx, PCR[PcTssCopy]     ;
        mov     [edx]+TssEsp0, eax      ; set Esp0 value
        popfd                           ;

        stdRET    _Ki386AdjustEsp0

stdENDP _Ki386AdjustEsp0


_TEXT$00   ends

        end

