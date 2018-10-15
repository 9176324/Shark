        title  "Processor State Save Restore"
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
;   procstat.asm
;
; Abstract:
;
;   This module implements routines to save and restore processor control
;   state.
;
;--

include ksamd64.inc

        altentry KiBugCheckReturn

        extern  KeBugCheck2:proc
        extern  KxContextToKframes:proc
        extern  KiHardwareTrigger:dword
        extern  RtlCaptureContext:proc

        subttl  "BugCheck"
;++
;
; VOID
; KeBugCheck (
;     __in ULONG BugCheckCode
;     )
;
; Routine Description:
;
;   This routine calls extended bugcheck to crash the system in a controlled
;   manner.
;
; Arguments:
;
;   BugCheckCode (rcx) - BugCheck code.
;
; Return Value:
;
;   None - function does not return.
;
;--

BcFrame struct
        P1Home  dq ?                    ; parameter home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        P5Home  dq ?                    ;
BcFrame ends

        NESTED_ENTRY KeBugCheck, _TEXT$00

        alloc_stack (sizeof BcFrame)    ; allocate stack frame 

        END_PROLOGUE

        ALTERNATE_ENTRY KiBugCheckReturn

        call KeBugCheckEx               ; bugcheck system - not return
        nop                             ; fill - do not remove

        NESTED_END KeBugCheck, _TEXT$00

        subttl  "BugCheck 3"
;++
;
; VOID
; KeBugCheck3 (
;     __in ULONG BugCheckCode,
;     __in ULONG_PTR P1,
;     __in ULONG_PTR P2,
;     __in ULONG_PTR P3
;     )
;
; Routine Description:
;
;   This routine calls extended bugcheck to crash the system in a controlled
;   manner.
;
;   This routine differs from KeBugCheckEx in that the fourth bugcheck
;   parameter is not used and presumed zero.
;
; Arguments:
;
;   BugCheckCode (rcx) - BugCheck code.
;
;   P1, P2 and P3 - Bugcheck parameters
;
; Return Value:
;
;   None - function does not return.
;
;--

BcFrame struct
        P1Home  dq ?                    ; parameter home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        P5Home  dq ?                    ;
BcFrame ends

        NESTED_ENTRY KiBugCheck3, _TEXT$00

        alloc_stack (sizeof BcFrame)    ; allocate stack frame 

        END_PROLOGUE

        mov     BcFrame.P5Home[rsp], 0
        call    KeBugCheckEx            ; bugcheck system - not return

        nop                             ; fill - do not remove

        NESTED_END KiBugCheck3, _TEXT$00


        subttl "BugCheck Extended"
;++
;
; VOID
; KeBugCheckEx (
;     __in ULONG BugCheckCode,
;     __in ULONG_PTR P1,
;     __in ULONG_PTR P2,
;     __in ULONG_PTR P3,
;     __in ULONG_PTR P4
;     )
;
; Routine Description:
;
;   This routine restores the context and control state of the current 
;   processor and passes control to KeBugCheck2.
;
; Arguments:
;
;   BugCheckCode (rcx) - BugCheck code.
;
;   P1, P2, P3 and P4 - BugCheck parameters.
;
; Return Value:
;
;   None - function does not return.
;
;--

BeFrame struct
        P1Home  dq ?                    ; parameter home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        P5Home  dq ?                    ;
        P6Home  dq ?                    ;
        Flags   dq ?                    ;
BeFrame ends

        NESTED_ENTRY KeBugCheckEx, _TEXT$00

        mov     P1Home[rsp], rcx        ; save argument registers
        mov     P2Home[rsp], rdx        ;
        mov     P3Home[rsp], r8         ;
        mov     P4Home[rsp], r9         ;

        push_eflags                     ; save processor flags
        alloc_stack (sizeof BeFrame - 8) ; allocate stack frame 

        END_PROLOGUE

;
; Capture Processor context and control state
;

        cli                             ; disable interrupts
        mov     rcx, gs:[PcCurrentPrcb] ; get current PRCB address
        add     rcx, PbProcessorState + PsContextFrame ; set context address
        call    RtlCaptureContext       ; capture processor context
        mov     rcx, gs:[PcCurrentPrcb] ; get current PRCB address
        add     rcx, PbProcessorState   ; set address of processor state
        call    KiSaveProcessorControlState; save processor control state

;
; Update register values in captured context to their state at the entry 
; of BugCheck function.
;

        mov     r10, gs:[PcCurrentPrcb] ; get current PRCB address
        add     r10, PbProcessorState + PsContextFrame ; point to context frame
        mov     rax, P1Home + (sizeof BeFrame)[rsp] ; get saved rcx
        mov     CxRcx[r10], rax         ; update rcx in context frame
        mov     rax, BeFrame.Flags[rsp] ; get saved flags
        mov     CxEFlags[r10], rax      ; update rflag in context frame
        lea     rax, KiBugCheckReturn + 5 ; get return address of KeBugCheck
        cmp     rax, (sizeof BeFrame)[rsp] ; identify caller by return address
        jnz     short KeBC10            ; if nz, caller is not KeBugCheck
        lea     r8, (sizeof BeFrame) + (sizeof BcFrame) + 8[rsp] 
                                        ; calculate rsp at entry of KeBugCheck
        lea     r9, KeBugCheck          ; get the entry point of KeBugCheck
        jmp     short KeBC20            ; 
KeBC10: lea     r8, (sizeof BeFrame)[rsp] ; calculate rsp at entry of KeBugCheckEx
        lea     r9, KeBugCheckEx        ; get the entry point of KeBugCheckEx
KeBC20: mov     CxRsp[r10], r8          ; update rsp in context frame
        mov     CxRip[r10], r9          ; update rip in context frame
        
;
; Raise IRQL and enable interrupt as appropriate.
;

        CurrentIrql                     ; get current IRQL 

        mov     gs:[PcDebuggerSavedIRQL], al ; save current IRQL
        cmp     al, DISPATCH_LEVEL      ; check if IRQL is less than dispatch
        jge     short KeBC30            ; if ge, don't bother to raise
        mov     ecx, DISPATCH_LEVEL     ; raise to DISPATCH_LEVEL

        SetIrql                         ; set IRQL

KeBC30: mov     rax, BeFrame.Flags[rsp] ; get saved flags
        and     rax, EFLAGS_IF_MASK     ; check previous interrupt state 
        jz      short KeBC40            ; if z, interrupt was disabled
        sti                             ; enable interrupt
KeBC40:
   lock inc KiHardwareTrigger           ; assert lock to avoid speculative read

;
; Pass control to KeBugCheck2
;

        mov     rcx, P1Home + (sizeof BeFrame)[rsp] ; get saved bugcheck code
        mov     qword ptr BeFrame.P6Home[rsp], 0  ; set parameter 6 to NULL
        lea     rax, KiBugCheckReturn + 5; get return address of KeBugCheck
        cmp     rax, (sizeof BeFrame)[rsp] ; identify caller by return address
        jz      short KeBC50            ; if z, caller is KeBugCheck
        mov     rax, (5 * 8) + (sizeof BeFrame)[rsp] ; get parameter 5
        mov     BeFrame.P5Home[rsp], rax; set parameter 5
        mov     r9,  P4Home + (sizeof BeFrame)[rsp] ; restore parameter 4
        mov     r8,  P3Home + (sizeof BeFrame)[rsp] ; restore parameter 3
        mov     rdx, P2Home + (sizeof BeFrame)[rsp] ; restore parameter 2
        call    KeBugCheck2             ; bugcheck system - not return
        nop                             ; fill - do not remove
                                        ;
KeBC50: mov     qword ptr Beframe.P5Home[rsp], 0  ; set parameter 5 to 0
        xor     r9d, r9d                ; set parameter 4 to 0
        xor     r8d, r8d                ; set parameter 3 to 0
        xor     edx, edx                ; set parameter 2 to 0
        call    KeBugCheck2             ; bugcheck system - not return
        nop                             ; fill - do not remove

        NESTED_END KeBugCheckEx, _TEXT$00

        subttl "Context To Kernel Frame"
;++
;
; VOID
; KeContextToKframes (
;     IN OUT PKTRAP_FRAME TrapFrame,
;     IN OUT PKEXCEPTION_FRAME ExceptionFrame,
;     IN PCONTEXT ContextRecord,
;     IN ULONG ContextFlags,
;     IN KPROCESSOR_MODE PreviousMode
;     )
;
; Routine Description:
;
;    This function saves the current non-volatile XMM state, performs a
;    context to kernel frames operation, then restores the non-volatile
;    XMM state.
;
; Arguments:
;
;    TrapFrame (rcx) - Supplies a pointer to a trap frame that receives the
;        volatile context from the context record.
;
;    ExceptionFrame (rdx) - Supplies a pointer to an exception frame that
;        receives the nonvolatile context from the context record.
;
;    ContextRecord (r8) - Supplies a pointer to a context frame that contains
;        the context that is to be copied into the trap and exception frames.
;
;    ContextFlags (r9) - Supplies the set of flags that specify which parts
;        of the context frame are to be copied into the trap and exception
;        frames.
;
;    PreviousMode (32[rsp]) - Supplies the processor mode for which the
;        exception and trap frames are being built.
;
; Return Value:
;
;    None.
;
;--

KfFrame struct
        P1Home  dq ?                    ; parameter home addresses
        P2Home  dq ?                    ;
        P3Home  dq ?                    ;
        P4Home  dq ?                    ;
        P5Home  dq ?                    ;
        OldIrql dd ?                    ; previous IRQL
        Fill1   dd ?                    ; fill
        SavedXmm6 db 16 dup (?)         ; saved nonvolatile floating registers
        SavedXmm7 db 16 dup (?)         ;
        SavedXmm8 db 16 dup (?)         ;
        SavedXmm9 db 16 dup (?)         ;
        SavedXmm10 db 16 dup (?)        ;
        SavedXmm11 db 16 dup (?)        ;
        SavedXmm12 db 16 dup (?)        ;
        SavedXmm13 db 16 dup (?)        ;
        SavedXmm14 db 16 dup (?)        ;
        SavedXmm15 db 16 dup (?)        ;
        Fill2   dq ?                    ; fill
KfFrame ends

        NESTED_ENTRY KeContextToKframes, _TEXT$00

        alloc_stack (sizeof KfFrame)    ; allocate stack frame
        save_xmm128 xmm6, KfFrame.SavedXmm6 ; save nonvolatile floating registers
        save_xmm128 xmm7, KfFrame.SavedXmm7 ;
        save_xmm128 xmm8, KfFrame.SavedXmm8 ;
        save_xmm128 xmm9, KfFrame.SavedXmm9 ;
        save_xmm128 xmm10, KfFrame.SavedXmm10 ;
        save_xmm128 xmm11, KfFrame.SavedXmm11 ;
        save_xmm128 xmm12, KfFrame.SavedXmm12 ;
        save_xmm128 xmm13, KfFrame.SavedXmm13 ;
        save_xmm128 xmm14, KfFrame.SavedXmm14 ;
        save_xmm128 xmm15, KfFrame.SavedXmm15 ;

        END_PROLOGUE

        mov     rax, cr8                ; get current IRQL
        mov     KfFrame.OldIrql[rsp], eax ; save current IRQL
        cmp     eax, APC_LEVEL          ; check if above or equal to APC level
        jae     short KfCS10            ; if ae, above or equal APC level
        mov     eax, APC_LEVEL          ; raise IRQL to APC level
        mov     cr8, rax                ;
KfCS10: mov     r10, (5 * 8) + (sizeof KfFrame)[rsp] ; get parameter 5
        mov     KfFrame.P5Home[rsp], r10 ; set parameter 5
        mov     rax, dr7                ; access debug register
        call    KxContextToKframes      ; perform a context to kernel frames
        test    rax, rax                ; test if legacy floating switched
        jz      short KfCS20            ; if z, legacy floating not switched

;
; N.B. The following legacy restore also restores the nonvolatile floating
;      registers xmm6-xmm15 with potentially incorrect values. Fortunately,
;      these registers are restored to their proper value shortly thereafter.
;

        fxrstor [rax]                   ; restore legacy floating state
KfCS20: cmp     KfFrame.OldIrql[rsp], APC_LEVEL ; check if lower IRQL required
        jae     short KfCS30            ; if ae, lower IRQL not necessary
        mov     eax, KfFrame.OldIrql[rsp] ; lower IRQL to previous level
        mov     cr8, rax                ;
KfCS30: movdqa  xmm6, KfFrame.SavedXmm6[rsp] ; restore nonvolatile floating registers
        movdqa  xmm7, KfFrame.SavedXmm7[rsp] ;
        movdqa  xmm8, KfFrame.SavedXmm8[rsp] ;
        movdqa  xmm9, KfFrame.SavedXmm9[rsp] ;
        movdqa  xmm10, KfFrame.SavedXmm10[rsp] ;
        movdqa  xmm11, KfFrame.SavedXmm11[rsp] ;
        movdqa  xmm12, KfFrame.SavedXmm12[rsp] ;
        movdqa  xmm13, KfFrame.SavedXmm13[rsp] ;
        movdqa  xmm14, KfFrame.SavedXmm14[rsp] ;
        movdqa  xmm15, KfFrame.SavedXmm15[rsp] ;
        add     rsp, (sizeof KfFrame)   ; deallocate stack frame
        ret                             ; return

        NESTED_END KeContextToKframes, _TEXT$00

        subttl  "Save Initial Processor Control State"
;++
;
; KiSaveInitialProcessorControlState (
;     PKPROCESSOR_STATE ProcessorState
;     );
;
; Routine Description:
;
;   This routine saves the initial control state of the current processor.
;
;   N.B. The debug register state is not saved.
;
; Arguments:
;
;   ProcessorState (rcx) - Supplies a pointer to a processor state structure.
;
; Return Value:
;
;    None.
;
;--

        LEAF_ENTRY KiSaveInitialProcessorControlState, _TEXT$00

        mov     rax, cr0                ; save processor control state
        mov     PsCr0[rcx], rax         ;
        mov     rax, cr2                ;
        mov     PsCr2[rcx], rax         ;
        mov     rax, cr3                ;
        mov     PsCr3[rcx], rax         ;
        mov     rax, cr4                ;
        mov     PsCr4[rcx], rax         ;
        mov     rax, cr8                ;
        mov     PsCr8[rcx], rax         ;

        sgdt    fword ptr PsGdtr[rcx]   ; save GDTR
        sidt    fword ptr PsIdtr[rcx]   ; save IDTR

        str     word ptr PsTr[rcx]      ; save TR
        sldt    word ptr PsLdtr[rcx]    ; save LDTR

        stmxcsr dword ptr PsMxCsr[rcx]  ; save XMM control/status
        ret                             ; return

        LEAF_END KiSaveInitialProcessorControlState, _TEXT$00

        subttl  "Restore Processor Control State"
;++
;
; KiRestoreProcessorControlState (
;     VOID
;     );
;
; Routine Description:
;
;   This routine restores the control state of the current processor.
;
; Arguments:
;
;   ProcessorState (rcx) - Supplies a pointer to a processor state structure.
;
; Return Value:
;
;   None.
;
;--

        LEAF_ENTRY KiRestoreProcessorControlState, _TEXT$00

        mov     rax, PsCr0[rcx]         ; restore processor control registers
        mov     cr0, rax                ;
        mov     rax, PsCr3[rcx]         ;
        mov     cr3, rax                ;
        mov     rax, PsCr4[rcx]         ;
        mov     cr4, rax                ;
        mov     rax, PsCr8[rcx]         ;
        mov     cr8, rax                ;

        lgdt    fword ptr PsGdtr[rcx]   ; restore GDTR
        lidt    fword ptr PsIdtr[rcx]   ; restore IDTR

;
; Force the TSS descriptor into a non-busy state so no fault will occur when
; TR is loaded.
;

	movzx	eax, word ptr PsTr[rcx] ; get TSS selector
	add	rax, PsGdtr + 2[rcx]	; compute TSS GDT entry address
	and	byte ptr 5[rax], NOT 2  ; clear busy bit
        ltr     word ptr PsTr[rcx]      ; restore TR

	xor     eax, eax                ; load a NULL selector into the ldt
	lldt	ax                      ;

        ldmxcsr dword ptr PsMxCsr[rcx]  ; restore XMM control/status

;
; Restore debug control state.
;

        xor     edx, edx                ; restore debug registers
        mov     dr7, rdx                ;
        mov     rax, PsKernelDr0[rcx]   ;
        mov     rdx, PsKernelDr1[rcx]   ;
        mov     dr0, rax                ;
        mov     dr1, rdx                ;
        mov     rax, PsKernelDr2[rcx]   ;
        mov     rdx, PsKernelDr3[rcx]   ;
        mov     dr2, rax                ;
        mov     dr3, rdx                ;
        mov     rdx, PsKernelDr7[rcx]   ;
        xor     eax, eax                ;
        mov     dr6, rax                ;
        mov     dr7, rdx                ;
        cmp     byte ptr gs:[PcCpuVendor], CPU_AMD ; check if AMD processor
        jne     short KiRC30            ; if ne, not authentic AMD processor

;
; The host processor is an authentic AMD processor.
;
; Check if branch tracing or last branch capture is enabled.
;

        test    dx, DR7_TRACE_BRANCH    ; test for trace branch enable
        jz      short KiRC10            ; if z, trace branch not enabled
        or      eax, MSR_DEBUG_CRL_BTF  ; set trace branch enable
KiRC10: test    dx, DR7_LAST_BRANCH     ; test for last branch enable
        jz      short KiRC20            ; if z, last branch not enabled
        or      eax, MSR_DEBUG_CTL_LBR  ; set last branch enable
KiRC20: test    eax, eax                ; test for extended debug enables
        jz      short KiRC30            ; if z, no extended debug enables
        mov     r8d, eax                ; save extended debug enables
        mov     ecx, MSR_DEGUG_CTL      ; set debug control MSR number
        rdmsr                           ; set extended debug control
        and     eax, not (MSR_DEBUG_CTL_LBR or MSR_DEBUG_CRL_BTF) ;
        or      eax, r8d                ;
        wrmsr                           ;
KiRC30: ret                             ; return

        LEAF_END KiRestoreProcessorControlState, _TEXT$00

        subttl  "Save Processor Control State"
;++
;
; KiSaveProcessorControlState (
;     PKPROCESSOR_STATE ProcessorState
;     );
;
; Routine Description:
;
;   This routine saves the control state of the current processor.
;
; Arguments:
;
;   ProcessorState (rcx) - Supplies a pointer to a processor state structure.
;
; Return Value:
;
;    None.
;
;--

        LEAF_ENTRY KiSaveProcessorControlState, _TEXT$00

        mov     rax, cr0                ; save processor control state
        mov     PsCr0[rcx], rax         ;
        mov     rax, cr2                ;
        mov     PsCr2[rcx], rax         ;
        mov     rax, cr3                ;
        mov     PsCr3[rcx], rax         ;
        mov     rax, cr4                ;
        mov     PsCr4[rcx], rax         ;
        mov     rax, cr8                ;
        mov     PsCr8[rcx], rax         ;

        sgdt    fword ptr PsGdtr[rcx]   ; save GDTR
        sidt    fword ptr PsIdtr[rcx]   ; save IDTR

        str     word ptr PsTr[rcx]      ; save TR
        sldt    word ptr PsLdtr[rcx]    ; save LDTR

        stmxcsr dword ptr PsMxCsr[rcx]  ; save XMM control/status

;
; Save debug control state.
;

        mov     rax, dr0                ; save debug registers
        mov     rdx, dr1                ;
        mov     PsKernelDr0[rcx], rax   ;
        mov     PsKernelDr1[rcx], rdx   ;
        mov     rax, dr2                ;
        mov     rdx, dr3                ;
        mov     PsKernelDr2[rcx], rax   ;
        mov     PsKernelDr3[rcx], rdx   ;
        mov     rax, dr6                ;
        mov     rdx, dr7                ;
        mov     PsKernelDr6[rcx], rax   ;
        mov     PsKernelDr7[rcx], rdx   ;
        xor     eax, eax                ;
        mov     dr7, rax                ;
        cmp     byte ptr gs:[PcCpuVendor], CPU_AMD ; check if AMD processor
        jne     short KiSC10            ; if ne, not authentic AMD processor

;
; The host processor is an authentic AMD processor.
;
; Check if branch tracing or last branch capture is enabled.
;

        test    dx, DR7_TRACE_BRANCH or DR7_LAST_BRANCH ; test for extended enables
        jz      short KiSC10            ; if z, extended debugging not enabled
        mov     r8, rcx                 ; save processor state address
        mov     ecx, MSR_LAST_BRANCH_FROM ; save last branch information
        rdmsr                           ;
        mov     PsLastBranchFromRip[r8], eax ;
        mov     PsLastBranchFromRip + 4[r8], edx ;
        mov     ecx, MSR_LAST_BRANCH_TO ;
        rdmsr                           ;
        mov     PsLastBranchToRip[r8], eax ;
        mov     PsLastBranchToRip + 4[r8], edx ;
        mov     ecx, MSR_LAST_EXCEPTION_FROM ;
        rdmsr                           ;
        mov     PsLastExceptionFromRip[r8], eax ;
        mov     PsLastExceptionFromRip + 4[r8], edx ;
        mov     ecx, MSR_LAST_EXCEPTION_TO ;
        rdmsr                           ;
        mov     PsLastExceptionToRip[r8], eax ;
        mov     PsLastExceptionToRip + 4[r8], edx ;
        mov     ecx, MSR_DEGUG_CTL      ; clear extended debug control
        rdmsr                           ;
        and     eax, not (MSR_DEBUG_CTL_LBR or MSR_DEBUG_CRL_BTF) ; 
        wrmsr                           ; 
KiSC10: ret                             ; return

        LEAF_END KiSaveProcessorControlState, _TEXT$00

        subttl  "Restore Debug Register State"
;++
;
; VOID
; KiRestoreDebugRegisterState (
;     VOID
;     );
;
; Routine Description:
;
;    This routine is executed on a transition from kernel mode to user mode
;    and restores the debug register state.
;
;    N.B. This routine is used for both trap/interrupt and system service
;         exit.
;
; Arguments:
;
;    None.
;
; Implicit Arguments:
;
;    rbp  - Supplies a pointer to a trap frame.
;
; Return Value:
;
;    None.
;
;--

        LEAF_ENTRY KiRestoreDebugRegisterState, _TEXT$00

        xor     edx, edx                ; clear register
        mov     dr7, rdx                ; clear control before loading
        mov     rax, TrDr0[rbp]         ; restore user debug registers
        mov     rdx, TrDr1[rbp]         ;
        mov     dr0, rax                ;
        mov     dr1, rdx                ;
        mov     rax, TrDr2[rbp]         ;
        mov     rdx, TrDr3[rbp]         ;
        mov     dr2, rax                ;
        mov     dr3, rdx                ;
        mov     rdx, TrDr7[rbp]         ;
        xor     eax, eax                ;
        mov     dr6, rax                ;
        mov     dr7, rdx                ;
        cmp     byte ptr gs:[PcCpuVendor], CPU_AMD ; check if AMD processor
        jne     short KiRD30            ; if ne, not authentic AMD processor

;
; The host processor is an authentic AMD processor.
;
; Check if branch tracing or last branch capture is enabled.
;

        test    dx, DR7_TRACE_BRANCH    ; test for trace branch enable
        jz      short KiRD10            ; if z, trace branch not enabled
        or      eax, MSR_DEBUG_CRL_BTF  ; set trace branch enable
KiRD10: test    dx, DR7_LAST_BRANCH     ; test for last branch enable
        jz      short KiRD20            ; if z, last branch not enabled
        or      eax, MSR_DEBUG_CTL_LBR  ; set last branch enable
KiRD20: test    eax, eax                ; test for extended debug enables
        jz      short KiRD30            ; if z, no extended debug enables
        mov     r8d, eax                ; save extended debug enables
        mov     ecx, MSR_DEGUG_CTL      ; set extended debug control
        rdmsr                           ;
        and     eax, not (MSR_DEBUG_CTL_LBR or MSR_DEBUG_CRL_BTF) ;
        or      eax, r8d                ;
        wrmsr                           ; 
KiRD30: ret                             ; return

        LEAF_END KiRestoreDebugRegisterState, _TEXT$00

        subttl  "Save Debug Register State"
;++
;
; VOID
; KiSaveDebugRegisterState (
;     VOID
;     );
;
; Routine Description:
;
;    This routine is called on a transition from user mode to kernel mode
;    when user debug registers are active. It saves the user debug registers
;    and loads the kernel debug register state.
;
; Arguments:
;
;    None.
;
; Implicit Arguments:
;
;    rbp - Supplies a pointer to a trap frame.
;   
; Return Value:
;
;    None.
;
;--

        LEAF_ENTRY KiSaveDebugRegisterState, _TEXT$00

        mov     r9, gs:[PcSelf]         ; get PCR address
        mov     rax, dr0                ; save user debug registers
        mov     rdx, dr1                ;
        mov     TrDr0[rbp], rax         ;
        mov     TrDr1[rbp], rdx         ;
        mov     rax, dr2                ;
        mov     rdx, dr3                ;
        mov     TrDr2[rbp], rax         ;
        mov     TrDr3[rbp], rdx         ;
        mov     rax, dr6                ;
        mov     rdx, dr7                ;
        mov     TrDr6[rbp], rax         ;
        mov     TrDr7[rbp], rdx         ;
        xor     eax, eax                ;
        mov     dr7, rax                ;
        cmp     byte ptr gs:[PcCpuVendor], CPU_AMD ; check if AMD processor
        jne     short KiSD10            ; if ne, not authentic AMD processor

;
; The host processor is an authentic AMD processor.
;
; Check if branch tracing or last branch capture is enabled.
;

        test    dx, DR7_TRACE_BRANCH or DR7_LAST_BRANCH ; test for extended enables
        jz      short KiSD10            ; if z, not extended debug enables
        mov     ecx, MSR_LAST_BRANCH_FROM ; save last branch information
        rdmsr                           ;
        mov     TrLastBranchFromRip[rbp], eax ;
        mov     TrLastBranchFromRip + 4[rbp], edx ;
        mov     ecx, MSR_LAST_BRANCH_TO ;
        rdmsr                           ;
        mov     TrLastBranchToRip[rbp], eax ;
        mov     TrLastBranchToRip + 4[rbp], edx ;
        mov     ecx, MSR_LAST_EXCEPTION_FROM ;
        rdmsr                           ;
        mov     TrLastExceptionFromRip[rbp], eax ;
        mov     TrLastExceptionFromRip + 4[rbp], edx ;
        mov     ecx, MSR_LAST_EXCEPTION_TO ;
        rdmsr                           ;
        mov     TrLastExceptionToRip[rbp], eax ;
        mov     TrLastExceptionToRip + 4[rbp], edx ;
        mov     ecx, MSR_DEGUG_CTL      ; Clear extended debug control
        rdmsr                           ;
        and     eax, not (MSR_DEBUG_CTL_LBR or MSR_DEBUG_CRL_BTF) ; 
        wrmsr                           ; 
KiSD10: test    word ptr PcKernelDr7[r9], DR7_ACTIVE ; test if debug enabled
        jz      short KiSD40            ; if z, debug not enabled
        mov     rax, PcKernelDr0[r9]    ; set debug registers
        mov     rdx, PcKernelDr1[r9]    ;
        mov     dr0, rax                ;
        mov     dr1, rdx                ;
        mov     rax, PcKernelDr2[r9]    ;
        mov     rdx, PcKernelDr3[r9]    ;
        mov     dr2, rax                ;
        mov     dr3, rdx                ;
        mov     rdx, PcKernelDr7[r9]    ;
        xor     eax, eax                ;
        mov     dr6, rax                ;
        mov     dr7, rdx                ;
        cmp     byte ptr gs:[PcCpuVendor], CPU_AMD ; check if AMD processor
        jne     short KiSD40            ; if ne, not authentic AMD processor

;
; The host processor is an authentic AMD processor.
;
; Check if branch tracing or last branch capture is enabled.
;

        test    dx, DR7_TRACE_BRANCH    ; test for trace branch enable
        jz      short KiSD20            ; if z, trace branch not enabled
        or      eax, MSR_DEBUG_CRL_BTF  ; set trace branch enable
KiSD20: test    dx, DR7_LAST_BRANCH     ; test for last branch enable
        jz      short KiSD30            ; if z, last branch not enabled
        or      eax, MSR_DEBUG_CTL_LBR  ; set last branch enable
KiSD30: test    eax, eax                ; test for extended debug enables
        jz      short KiSD40            ; if z, no extended debug enables
        mov     r8d, eax                ; save extended debug enables
        mov     ecx, MSR_DEGUG_CTL      ; set extended debug control
        rdmsr                           ;
        and     eax, not (MSR_DEBUG_CTL_LBR or MSR_DEBUG_CRL_BTF) ;
        or      eax, r8d                ; 
        wrmsr                           ; 
KiSD40: ret                             ; return

        LEAF_END KiSaveDebugRegisterState, _TEXT$00

        subttl  "Get Current Stack Pointer"
;++
;
; ULONG64
; KeGetCurrentStackPointer (
;     VOID
;     );
;
; Routine Description:
;
;   This function returns the caller's stack pointer.
;
; Arguments:
;
;   None.
;
; Return Value:
;
;   The callers stack pointer is returned as the function value.
;
;--

        LEAF_ENTRY KeGetCurrentStackPointer, _TEXT$00

        lea     rax, 8[rsp]             ; get callers stack pointer
        ret                             ;

        LEAF_END KeGetCurrentStackPointer, _TEXT$00
        
        subttl  "Save Legacy Floating Point State"
;++
;
; VOID
; KeSaveLegacyFloatingPointState (
;     PXMM_SAVE_AREA32 NpxFrame
;     );
;
; Routine Description:
;
;   This routine saves the legacy floating state for the current thread.
;
; Arguments:
;
;   NpxFrame (rcx) - Supplies the address of the legacy floating save area.
;
; Return Value:
;
;   None.
;
;--

        LEAF_ENTRY KeSaveLegacyFloatingPointState, _TEXT$00

        fxsave  [rcx]                   ; save legacy floating state
        ret                             ;

        LEAF_END KeSaveLegacyFloatingPointState, _TEXT$00
        
        end

