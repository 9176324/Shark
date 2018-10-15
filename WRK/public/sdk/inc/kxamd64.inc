;++
;
; Copyright (c) Microsoft Corporation. All rights reserved.
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
; Module:
;
;   kxamd64.w
;
; Astract:
;
;   Contains AMD64 architecture constants and assembly macros.
;
;--

include macamd64.inc

;
; Define macro to clear legacy floating exceptions.
;

clfpex  macro

        db      0dbh, 0e2h

        endm

;                             
; Define macro to acquire spin lock.
;
; Arguments:
;
;   None.
;
; N.B. This macro uses no registers.
;

AcquireSpinLock macro Address

        local exit, spin

ifndef NT_UP

   lock bts     qword ptr Address, 0    ; attempt to acquire spin lock
        jnc     short exit              ; if nc, spin lock acquired

spin:   Yield                           ; yield execution

        test    qword ptr Address, 1    ; check if lock currently owned
        jnz     short spin              ; if nz, spin lock owned
   lock bts     qword ptr Address, 0    ; attempt to acquire spin lock
        jc      short spin              ; if c, spin lock owned

exit:                                   ; continue

endif

        endm

;                             
; Define macro to acquire spin lock and mask interrupts.
;
; Arguments:
;
;   None.
;
; Note:
;
;   rsp is assumed to point to pushed EFLAGS
;
; N.B. This macro uses no registers.
;

AcquireSpinLockDisable macro Address

        local exit, spin, spin1

        cli                             ; disable interrupts

ifndef NT_UP

   lock bts     qword ptr Address, 0    ; attempt to acquire spin lock
        jnc     short exit              ; if nc, spin lock acquired
spin:   test    dword ptr [rsp], EFLAGS_IF_MASK ; test if interrupts enabled
        jz      short spin1             ; if z, interrupts disabled
        sti                             ; enable interrupts

spin1:  Yield                           ; yield execution

        test    qword ptr Address, 1    ; check if lock currently owned
        jnz     short spin1             ; if nz, spin lock owned
        cli                             ; lock is (was) clear, disable ints
   lock bts     qword ptr Address, 0    ; attempt to acquire spin lock
        jc      short spin              ; if c, spin lock owned
exit:                                   ; continue

endif

        endm

;
; Define macro to release spin lock.
;
; Arguments:
;
;   None.
;
; N.B. This macro uses no registers.
;

ReleaseSpinLock macro Address

ifndef NT_UP

   lock and     qword ptr Address, 0    ; release spin lock

endif

        endm

;
; Define macro to release spin lock and restore the interrupt flag.
;
; Arguments:
;
;   None.
;
; Note:
;
;   rsp is assumed to point to pushd EFLAGS
;
; N.B. This macro uses no registers.
;

ReleaseSpinLockEnable macro Address

        local exit

ifndef NT_UP

   lock and     qword ptr Address, 0    ; release spin lock

endif

        test    dword ptr [rsp], EFLAGS_IF_MASK ; test if interrupts enabled
        jz      short exit              ; if z, interrupts not enabled
        sti                             ; enable interrupts
exit:                                   ; continue

        endm

;
; Define macro to try to acquire spin lock.
;
; Arguments:
;
;   None.
;
; N.B. This macro uses no registers.
;

TryToAcquireSpinLock macro Address

ifndef NT_UP

        lock bts qword ptr Address, 0   ; attempt to acquire spin lock

endif

        endm

;
; Define macro to perform the equivalent of reading cr8.
;
; Arguments:
;
;   None
;
; The equivalent of the contents of cr8 is returned in rax
;
; N.B. This macro is restricted to using only rax.
;

ReadCr8 macro

        mov     rax, cr8                ; read IRQL

	endm

;
; Define macro to perform the equivalent of writing cr8.
;
; Arguments:
;
;   rcx - The desired value of cr8.
;

WriteCr8 macro

        mov     cr8, rcx                ; write IRQL			

	endm

;
; Define macro to get current IRQL.
;
; Arguments:
;
;   None.
;
; The previous IRQL is returned in rax.
;

CurrentIrql macro

        ReadCr8                         ; get current IRQL

        endm

;
; Define macro to lower IRQL.
;
; Arguments:
;
;   rcx - Supplies the new IRQL.
;
; N.B. The register rax is destroyed.
;
; N.B. This macro is restricted to using only rcx and rdx.
;

LowerIrql macro

        local   exit

if DBG

        mov     rdx, rax                ; preserve rax
        ReadCr8                         ; get current IRQL
        cmp     eax, ecx                ; check new IRQL
        jge     short exit              ; if ge, new IRQL okay
        int     3                       ; break into debugger
exit:   mov     rax, rdx

endif

        WriteCr8                        ; set new IRQL

        endm

;
; Define macro to raise IRQL.
;
; Arguments:
;
;   rcx - Supplies the new IRQL.
;
; The previous IRQL is returned in rax.
;
; N.B. This macro is restricted to using only rax and rcx.
;

RaiseIrql macro

        local   exit

        ReadCr8                         ; get current IRQL

if DBG

        cmp     eax, ecx                ; check new IRQL
        jle     short exit              ; if le, new IRQL okay
        int     3                       ; break into debugger

endif

exit:   WriteCr8                        ; set new IRQL

        endm

;
; Define macro to set IRQL.
;
; Arguments:
;
;   rcx - Supplies the new IRQL.
;
; N.B. This macro is restricted to using only rcx.
;

SetIrql macro

	WriteCr8			; set new IRQL

        endm

;
; Define macro to swap IRQL.
;
; Arguments:
;
;   rcx - Supplies the new IRQL.
;
; The previous IRQL is returned in rax.
;
; N.B. This macro is restricted to using only rax and rcx.
;

SwapIrql macro

	ReadCr8				; get current IRQL
	WriteCr8			; set new IRQL

        endm

;
; Define end system interrupt macro.
;
; Arguments:
;
;   None.
;
; N.B. The following code generates:
;
;      mov     dword ptr ds:[0fffffffffffe00b0h], 0
;
;      which is:
;
;      LOCAL_APIC(LU_EOI) = 0;
;

EndSystemInterrupt macro

        db      0c7h, 004h, 025h, 0b0h  ;
        db      000h, 0feh, 0ffh, 000h  ;
        db      000h, 000h, 000h        ;

        endm

;
; Define restore exception state macro.
;
;   This macro restores the nonvolatile state.
;
; Arguments:
;
;   Flag - If blank, then nonvolatile floating and integer registers are
;       restored. If nonblank and identical to "Rbp", then rbp is restored
;       in addition to the nonvolatile floating and integer registers. If
;       nonblank and identical to "NoFp", then only the nonvolatile integer
;       registers are restored.
;
; Implicit arguments:
;
;   rsp - Supplies the address of the exception frame.
;

RESTORE_EXCEPTION_STATE macro Flag

        lea     rcx, 100h[rsp]          ; set frame display pointer

ifdif <Flag>, <NoFp>

        movdqa  xmm6, ExXmm6[rsp]       ; restore nonvolatile xmm registers
        movdqa  xmm7, ExXmm7[rsp]       ;
        movdqa  xmm8, ExXmm8[rsp]       ;
        movdqa  xmm9, ExXmm9[rsp]       ;
        movdqa  xmm10, ExXmm10[rsp]     ;
        movdqa  xmm11, (ExXmm11 - 100h)[rcx] ;
        movdqa  xmm12, (ExXmm12 - 100h)[rcx] ;
        movdqa  xmm13, (ExXmm13 - 100h)[rcx] ;
        movdqa  xmm14, (ExXmm14 - 100h)[rcx] ;
        movdqa  xmm15, (ExXmm15 - 100h)[rcx] ;

endif

        mov     rbx, (ExRbx - 100h)[rcx]  ; restore nonvolatile integer registers
        mov     rdi, (ExRdi - 100h)[rcx]  ;
        mov     rsi, (ExRsi - 100h)[rcx]  ;
        mov     r12, (ExR12 - 100h)[rcx]  ;
        mov     r13, (ExR13 - 100h)[rcx]  ;
        mov     r14, (ExR14 - 100h)[rcx]  ;
        mov     r15, (ExR15 - 100h)[rcx]  ;

ifdif <Flag>, <NoPop>

ifidn <Flag>, <Rbp>

        mov     rbp, (ExRbp - 100h)[rcx]  ; restore nonvolatile integer register

endif

        add     rsp, KEXCEPTION_FRAME_LENGTH - (1 * 8) ; deallocate frame

endif

        endm

;
; Define generate exception frame macro.
;
;   This macro allocates an exception frame and saves the nonvolatile state.
;
; Arguments:
;
;   Flag - If blank, then nonvolatile floating and integer registers are
;       saved. If nonblank and identical to "Rbp", then rbp is saved in
;       addition to the nonvolatile floating and integer registers. If
;       nonblank and identical to "NoFp", then only the nonvolatile integer
;       registers are saved.
;
; Implicit arguments:
;
;   The top of the stack is assumed to contain a return address.
;

GENERATE_EXCEPTION_FRAME macro Flag

        alloc_stack (KEXCEPTION_FRAME_LENGTH - (1 * 8)) ; allocate frame
        lea     rax, 100h[rsp]          ; set frame display pointer

ifdif <Flag>, <NoFp>

        save_xmm128 xmm6, ExXmm6        ; save xmm nonvolatile registers
        save_xmm128 xmm7, ExXmm7        ;
        save_xmm128 xmm8, ExXmm8        ;
        save_xmm128 xmm9, ExXmm9        ;
        save_xmm128 xmm10, ExXmm10      ;

        movdqa  (ExXmm11 - 100h)[rax], xmm11 ;
        .savexmm128 xmm11, ExXmm11      ;

        movdqa  (ExXmm12 - 100h)[rax], xmm12 ;
        .savexmm128 xmm12, ExXmm12      ;

        movdqa  (ExXmm13 - 100h)[rax], xmm13 ;
        .savexmm128 xmm13, ExXmm13      ;

        movdqa  (ExXmm14 - 100h)[rax], xmm14 ;
        .savexmm128 xmm14, ExXmm14      ;

        movdqa  (ExXmm15 - 100h)[rax], xmm15 ;
        .savexmm128 xmm15, ExXmm15      ;

endif

ifidn <Flag>, <Rbp>

        mov     (ExRbp - 100h)[rax], rbp  ; save nonvolatile integer register
        .savereg rbp, ExRbp             ;
        set_frame rbp, 0                ; set frame pointer

endif

        mov     (ExRbx - 100h)[rax], rbx  ;
        .savereg rbx, ExRbx             ;

        mov     (ExRdi - 100h)[rax], rdi  ;
        .savereg rdi, ExRdi             ;

        mov     (ExRsi - 100h)[rax], rsi  ;
        .savereg rsi, ExRsi             ;

        mov     (ExR12 - 100h)[rax], r12  ;
        .savereg r12, ExR12             ;

        mov     (ExR13 - 100h)[rax], r13  ;
        .savereg r13, ExR13             ;

        mov     (ExR14 - 100h)[rax], r14  ;
        .savereg r14, ExR14             ;

        mov     (ExR15 - 100h)[rax], r15  ;
        .savereg r15, ExR15             ;

        END_PROLOGUE

        endm

;
; Define restore trap state macro.
;
;   This macro restores the volatile state, and if necessary, restores the
;   user debug state, deallocats the trap frame, and exits the trap.
;
;   N.B. This macro must preserve eax in case it is not reloaded from the
;        trap frame.
;
; Arguments:
;
;   State - Determines what state is restored and what tests are made. Valid
;       values are:
;
;           Service - restore state for a service executed from user mode.
;           Kernel - restore state for a service executed from kernel mode.
;           Volatile - restore state for a trap or interrupt.
;
;   Disable - If blank, then disable interrupts.
;
; Implicit arguments:
;
;   rbp - Supplies the address of the trap frame.
;

RESTORE_TRAP_STATE macro State, Disable, NmiFlag, LBranch

        local   first, second, third

ifb <Disable>

        cli                             ; disable interrupts

endif

ifdif <State>, <Kernel>

;
; State is either <Volatile> or <Service>
;

ifidn <State>, <Volatile>

        test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode user
        jz      third                   ; if z, previous mode not user

endif

ifdif <NmiFlag>, <Nmi>

        mov     rcx, gs:[PcCurrentThread] ; get current thread address
        cmp     byte ptr ThApcState + AsUserApcPending[rcx], 0 ; APC pending?
        je      short first             ; if e, no user APC pending

endif

ifidn <State>, <Service>

        mov     TrRax[rbp], eax         ; save service status
        xor     eax, eax                ; scrub volatile integer registers
        mov     TrRcx[rbp], rax         ;
        mov     TrRdx[rbp], rax         ;
        mov     TrR8[rbp], rax          ;
        mov     TrR9[rbp], rax          ;
        mov     TrR10[rbp], rax         ;
        mov     TrR11[rbp], rax         ;
        pxor    xmm0, xmm0              ; scrub volatile floating registers
        movdqa  TrXmm0[rbp], xmm0       ;
        movdqa  TrXmm1[rbp], xmm0       ;
        movdqa  TrXmm2[rbp], xmm0       ;
        movdqa  TrXmm3[rbp], xmm0       ;
        movdqa  TrXmm4[rbp], xmm0       ;
        movdqa  TrXmm5[rbp], xmm0       ;

endif

ifdif <NmiFlag>, <Nmi>

        mov     ecx, APC_LEVEL          ; get APC level

        SetIrql                         ; set IRQL to APC level

        sti                             ; allow interrupts
        call    KiInitiateUserApc       ; initiate APC execution
        cli                             ; disable interrupts
        mov     ecx, PASSIVE_LEVEL      ; get PASSIVE level

        SetIrql                         ; set IRQL to PASSIVE level

endif

ifidn <State>, <Service>

        mov     eax, TrRax[rbp]         ; restore service status

endif

first:  ldmxcsr TrMxCsr[rbp]            ; restore XMM control/status
        test    word ptr TrDr7[rbp], DR7_ACTIVE ; test if debug active
        jz      short second            ; if z, debug not active

ifidn <State>, <Service>

        mov     TrRax[rbp], eax         ; save service status

endif

        call    KiRestoreDebugRegisterState ; restore user debug register state

ifidn <State>, <Service>

        mov     eax, TrRax[rbp]         ; restore service status

endif

second:                                 ;

;
; At this point it is known that the return will be to user mode.
;

ifidn <State>, <Volatile>

        movdqa  xmm0, TrXmm0[rbp]       ; restore volatile XMM registers
        movdqa  xmm1, TrXmm1[rbp]       ;
        movdqa  xmm2, TrXmm2[rbp]       ;
        movdqa  xmm3, TrXmm3[rbp]       ;
        movdqa  xmm4, TrXmm4[rbp]       ;
        movdqa  xmm5, TrXmm5[rbp]       ;

        mov     r11, TrR11[rbp]         ; restore volatile integer state
        mov     r10, TrR10[rbp]         ;
        mov     r9, TrR9[rbp]           ;
        mov     r8, TrR8[rbp]           ;

ifnb <LBranch>

        mov     ecx, TrLastBranchMSR[rbp] ; get last branch MSR number
        or      ecx, ecx                ; test if last branch MSR defined
        jz      short @f                ; if z, last branch MSR not defined
        mov     eax, TrLastBranchControl[rbp] ; write last branch control register
        mov     edx, TrLastBranchControl + 4[rbp] ;
        wrmsr                           ;
@@:                                     ;

endif

        mov     rdx, TrRdx[rbp]         ;
        mov     rcx, TrRcx[rbp]         ;
        mov     rax, TrRax[rbp]         ;
        mov     rsp, rbp                ; trim stack to frame offset
        mov     rbp, TrRbp[rbp]         ; restore RBP
        add     rsp, (KTRAP_FRAME_LENGTH - (5 * 8) - 128) ; deallocate stack
        swapgs                          ; swap GS base to user mode TEB
        iretq                           ;

else

        mov     r8, TrRsp[rbp]          ; get previous RSP value
        mov     r9, TrRbp[rbp]          ; get previous RBP value
        xor     edx, edx                ; scrub volatile integer registers
        xor     r10, r10                ;
        pxor    xmm0, xmm0              ; scrub volatile floating registers
        pxor    xmm1, xmm1              ;
        pxor    xmm2, xmm2              ;
        pxor    xmm3, xmm3              ;
        pxor    xmm4, xmm4              ;
        pxor    xmm5, xmm5              ;
        mov     rcx, TrRip[rbp]         ; get return address
        mov     r11, TrEFlags[rbp]      ; get previous EFLAGS
        mov     rbp, r9                 ; restore RBP
        mov     rsp, r8                 ; restore RSP
        swapgs                          ; swap GS base to user mode TEB
        sysretq                         ; return from system call to user mode

endif

ifidn <State>, <Volatile>

third:  ldmxcsr TrMxCsr[rbp]            ; restore XMM control/status
        movdqa  xmm0, TrXmm0[rbp]       ; restore volatile XMM registers
        movdqa  xmm1, TrXmm1[rbp]       ;
        movdqa  xmm2, TrXmm2[rbp]       ;
        movdqa  xmm3, TrXmm3[rbp]       ;
        movdqa  xmm4, TrXmm4[rbp]       ;
        movdqa  xmm5, TrXmm5[rbp]       ;

ifidn <NmiFlag>, <Nmi>

        mov     eax, TrGsBase[rbp]      ; restore GS base MSR
        mov     edx, TrGsBase + 4[rbp]  ;
        mov     ecx, MSR_GS_BASE        ;
        wrmsr                           ;
        mov     rax, TrFaultAddress[rbp] ; restore CR2
        mov     cr2, rax                ;

endif

        mov     r11, TrR11[rbp]         ; restore volatile integer state
        mov     r10, TrR10[rbp]         ;
        mov     r9, TrR9[rbp]           ;
        mov     r8, TrR8[rbp]           ;

ifnb <LBranch>

        mov     ecx, TrLastBranchMSR[rbp] ; get last branch MSR number
        or      ecx, ecx                ; test if last branch MSR defined
        jz      short @f                ; if z, last branch MSR not defined
        mov     eax, TrLastBranchControl[rbp] ; write last branch control register
        mov     edx, TrLastBranchControl + 4[rbp] ;
        wrmsr                           ;
@@:                                     ;

endif

        mov     rdx, TrRdx[rbp]         ;
        mov     rcx, TrRcx[rbp]         ;
        mov     rax, TrRax[rbp]         ;
        mov     rsp, rbp                ; trim stack to frame offset
        mov     rbp, TrRbp[rbp]         ; restore RBP
        add     rsp, (KTRAP_FRAME_LENGTH - (5 * 8) - 128) ; deallocate stack
        iretq                           ;

endif

;
; State is kernel mode.
;

else

        mov     rsp, rbp                ; trim stack to frame offset
        mov     rbp, TrRbp[rbp]         ; restore RBP
        mov     rsp, TrRsp[rsp]         ; restore RSP
        sti                             ; enable interrupts
        ret                             ; return from system call to kernel mode

endif

        endm

;
; Define save trap state macro.
;
;   This macro saves the volatile state, and if necessary, saves the user
;   debug state and loads the kernel debug state.
;
; Arguments:
;
;   SaveGSSwap - If non-blank, then save the GS swap register if the previous
;       mode is user.
;
; Implicit arguments:
;
;    rbp - Supplies the address of the trap frame.
;

SAVE_TRAP_STATE macro Service, SaveGSSwap, NmiFlag, LBranch

        local   first, second

        mov     TrRax[rbp], rax         ; save volatile integer registers
        mov     TrRcx[rbp], rcx         ;
        mov     TrRdx[rbp], rdx         ;

ifnb <LBranch>

        mov     ecx, KeLastBranchMSR    ; get last branch MSR number
        or      ecx, ecx                ; test if last branch MSR defined
        jz      short @f                ; if z, last branch MSR not defined
        rdmsr                           ; read last branch control register
        mov     TrLastBranchControl[rbp], eax ; save last branch control
        mov     TrLastBranchControl + 4[rbp], edx ;
        btr     eax, 0                  ; clear bit 0 in last branch control 
        wrmsr                           ; disable last branch recording
@@:     mov     TrLastBranchMSR[rbp], ecx ; save last branch MSR number

endif

        mov     TrR8[rbp], r8           ;
        mov     TrR9[rbp], r9           ;
        mov     TrR10[rbp], r10         ;
        mov     TrR11[rbp], r11         ;

ifidn <NmiFlag>, <Nmi>

        test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode user
        jnz     short second            ; if nz, previous mode user

;
; Preserve the current GS base in the trap frame.
;

        mov     ecx, MSR_GS_BASE        ; save GS base MSR in trap frame
        rdmsr                           ;
        mov     TrGsBase[rbp], eax      ;
        mov     TrGsBase + 4[rbp], edx  ;

;
; Load the correct kernel GS base.
;

        lea     rcx, KiProcessorBlock   ; get processor block array address
        mov     eax, KGDT64_R3_CMTEB    ; set selector number
        lsl     eax, eax                ; load segment limit
        shr     eax, 14                 ; extract processor number
        mov     rdx, [rcx + rax * 8]    ; get current PRCB address
        sub     rdx, PcPrcb             ; compute current PCR address
        mov     eax, edx                ; set current GS base MSR
        shr     rdx, 32                 ;
        mov     ecx, MSR_GS_BASE        ;
        wrmsr                           ;

;
; Preserve CR2 in the trap frame.
;

        mov     rax, cr2                ; save CR2 in trap frame
        mov     TrFaultAddress[rbp], rax ;
        jmp     short first             ;

second:

else

        test    byte ptr TrSegCs[rbp], MODE_MASK ; test if previous mode user
        jz      short first             ; if z, previous mode kernel

endif

        swapgs                          ; swap GS base to kernel mode PCR
        mov     r10, gs:[PcCurrentThread] ; get current thread address

ifnb <SaveGSSwap>

        cmp     word ptr TrSegCs[rbp], (KGDT64_R3_CODE or RPL_MASK) ; check for 64-bit mode
        jne     short @f                ; if ne, not running in 64-bit mode
        mov     ecx, MSR_GS_SWAP        ; set GS swap MSR number
        rdmsr                           ; read GS swap MSR
        mov     TrGsSwap[rbp], eax      ; save GS swap MSR
        mov     TrGsSwap + 4[rbp], edx  ;
@@:                                     ;

endif

        test    byte ptr ThDebugActive[r10], TRUE ; test if debug enabled
        mov     word ptr TrDr7[rbp], 0  ; assume debug not enabled
        jz      short first             ; if z, debug not enabled
        call    KiSaveDebugRegisterState ; save debug register state
first:  cld                             ; clear direction flag
        stmxcsr TrMxCsr[rbp]            ; save XMM control/status
        ldmxcsr dword ptr gs:[PcMxCsr]  ; set default XMM control/status
        movdqa  TrXmm0[rbp], xmm0       ; save volatile xmm registers
        movdqa  TrXmm1[rbp], xmm1       ;
        movdqa  TrXmm2[rbp], xmm2       ;
        movdqa  TrXmm3[rbp], xmm3       ;
        movdqa  TrXmm4[rbp], xmm4       ;
        movdqa  TrXmm5[rbp], xmm5       ;

        endm

;
; Define interrupt frame generation macro.
;
;   This macro generates an interrupt frame.
;
; Arguments:
;
;   Vector - If non-blank, then the vector number is on the stack.
;
;   Direct - If non-blank, then the interrupt is directly connected.
;
; Return value:
;
;   If Vector is non-blank, then the value of the vector is returned in eax.
;
; Note: Trap and interrupt frames are exempt from the "first instruction must
;       be two bytes" rule.
;

GENERATE_INTERRUPT_FRAME macro Vector, Direct, NmiFlag, LBranch

;
; At this point the hardware frame has been pushed onto an aligned stack. The
; vector number or a dummy vector number and rbp have also been pushed on the
; stack.
;

ifb <Direct>

        push_reg rsi                    ; save nonvolatile register
        alloc_stack (KTRAP_FRAME_LENGTH - (8 * 8)) ; allocate fixed frame
        mov     rsi, rbp                ; set address of interrupt object

else

        alloc_stack (KTRAP_FRAME_LENGTH - (8 * 7)) ; allocate fixed frame

endif

        set_frame rbp, 128              ; set frame pointer

        END_PROLOGUE
                  
        mov     byte ptr TrExceptionActive[rbp], 0 ; set interrupt active

        SAVE_TRAP_STATE <>, <>, <NmiFlag>, <LBranch> ; save trap state

;
; Check if a kernel-mode SLIST pop operation is being interrupted and reset
; RIP as necessary.
; 
                  
ifdifi <Direct>, <DirectNoSListCheck>

        lea     rax, ExpInterlockedPopEntrySListResume ; get SLIST resume address
        cmp     rax, TrRip[rbp]         ; check resume address is above RIP
        jae     short not_slist         ; if ae, resume address above RIP
        lea     rax, ExpInterlockedPopEntrySListEnd ; get SLIST end address
        cmp     rax, TrRip[rbp]         ; check end address is below RIP
        jb      short not_slist         ; if b, end address below RIP
        lea     rcx, (-128)[rbp]        ; set trap frame address
        call    KiCheckForSListAddress  ; check RIP and reset if necessary
not_slist:                              ;

endif

ifnb <Vector>

        mov     eax, TrErrorCode[rbp]   ; return vector number

endif

        inc     dword ptr gs:[PcInterruptCount] ; increment interrupt count

        endm

;
; Define enter interrupt macro.
;
;   This macro raises IRQL, sets the interrupt flag, records the previous
;   IRQL in the trap frame, and invokes the HAL to perform an EOI.
;
; Arguments:
;
;   NoEOI - If blank, then generate end of interrupt.
;
;   NoCount - If blank, then increment nesting level.
;
; Implicit arguments:
;
;   rcx - Supplies the interrupt IRQL.
;
;   rbp - Supplies the address of the trap frame.
;
;   Interrupt flag is clear.
;
; Return Value:
;
;   None.
;

ENTER_INTERRUPT macro NoEOI, NoCount, NmiFlag

;
; N.B. It is possible for a interrupt to occur at an IRQL that is lower
;      than the current IRQL. This happens when the IRQL raised and at
;      the same time an interrupt request is granted.
;
;
; N.B. Raise IRQL cannot be used below since this macro is used in the NMI
;      handler and would trigger a false assert.
;

        SwapIrql                        ; raise IRQL to interrupt level

        mov     TrPreviousIrql[rbp], al ; save previous IRQL

ifb <NoCount>

        inc     byte ptr gs:[PcNestingLevel] ; increment nesting level

endif

ifb <NoEOI>

        EndSystemInterrupt              ; perform EOI

endif

ifdif <NmiFlag>, <Nmi>

        sti                             ; enable interrupts

endif

        endm

;
; Define exit interrupt macro.
;
;   This macro exits an interrupt.
;
; Arguments:
;
;   NoEOI - If blank, then generate end of interrupt.
;
;   NoCount - If blank, then decrement nesting level.
;
;   Direct - If non-blank, then the interrupt is directly connected.
;   
; Implicit arguments:
;
;   rbp - Supplies the address of the trap frame.
;
; Return Value:
;
;   None.
;

EXIT_INTERRUPT macro NoEOI, NoCount, Direct, NmiFlag, LBranch

        local   exit, request

        cli                             ; disable interrupts

ifb <NoEOI>

        EndSystemInterrupt              ; perform EOI

endif

ifb <NoCount>

        dec     byte ptr gs:[PcNestingLevel] ; decrement nesting level
        jnz     short exit              ; if nz, more interrupts nested
        mov     cl, gs:[PcInterruptRequest] ; get interrupt request value
        and     byte ptr gs:[PcInterruptRequest], 0 ; clear interrupt request
        cmp     byte ptr gs:[PcIdleHalt], 0 ; check for idle halt interrupt
        jne     short exit              ; if ne, interrupt from idle halt
        test    cl, cl                  ; test if dispatch interrupt request
        jz      short exit              ; if z, no dispatch interrupt request
        cmp     byte ptr TrPreviousIrql[rbp], DISPATCH_LEVEL ; check for bypass
        jae     short request           ; if ae, bypass not possible
        call    KiDpcInterruptBypass    ; bypass dispatch interrupt
        jmp     short exit              ; finish in common code

request:                                ;
        mov     rcx, gs:[PcSetMember]   ; get current processor set member
        test    KiIdleSummary, rcx      ; test if current processor idle
        jnz     short exit              ; if nz, current processor idle
        mov     ecx, DISPATCH_LEVEL     ; request dispatch interrupt
        call    __imp_HalRequestSoftwareInterrupt ;
exit:                                   ;

endif

        movzx   ecx, byte ptr TrPreviousIrql[rbp] ; get previous IRQL

        SetIrql                         ; set IRQL to previous level

ifb <Direct>

        mov     rsi, TrRsi[rbp]         ; restore extra register

endif

        RESTORE_TRAP_STATE <Volatile>, <NoDisable>, <NmiFlag>, <LBranch> ; restore trap state

        endm

;
; Define trap frame generation macro.
;
;   This macro generates a trap frame.
;
; Arguments:
;
;   ErrorCode - If non-blank, then an error code is on the stack.
;
;   PatchCycle - If non-blank, then store the global patch cycle count in
;       the trap frame.
;
;   SaveGSSwap - If non-blank, then save the GS swap register if the previous
;       mode is user.
;
; Return value:
;
;   If ErrorCode is non-blank, then the value of the error code is returned
;   in eax.
;
; Note: Trap and interrupt frames are exempt from the "first instruction must
;       be two bytes" rule.
;

GENERATE_TRAP_FRAME macro ErrorCode, PatchCycle, SaveGSSwap

        local   exit

ifb <ErrorCode>

        push_frame                      ; mark machine frame without error code
        alloc_stack 8                   ; allocate dummy error code

else

ifidn <ErrorCode>, <MxCsr>

        push_frame                      ; mark machine frame without error code
        alloc_stack 8                   ; allocate dummy error code

else

        push_frame code                 ; mark machine frame with error code

endif

endif

        push_reg rbp                    ; save nonvolatile register
        alloc_stack (KTRAP_FRAME_LENGTH - (7 * 8)) ; allocate fixed frame
        set_frame rbp, 128              ; set frame pointer

        END_PROLOGUE

        mov     byte ptr TrExceptionActive[rbp], 1 ; set exception active

        SAVE_TRAP_STATE <>, <SaveGSSwap> ; save trap state

ifnb <PatchCycle>

        mov     eax, KiCodePatchCycle   ; get current patch cycle count
        mov     TrCodePatchCycle[rbp], eax ; save patch cycle count

endif

ifnb <ErrorCode>

ifidn <ErrorCode>, <MxCsr>

        mov     ax, TrMxCsr[rbp]        ; return saved MXCSR

else

        mov     eax, TrErrorCode[rbp]   ; return error code

ifidn <ErrorCode>, <Virtual>

        mov     rcx, cr2                ; return virtual address

endif

endif

endif

;
; Enable interrupts if and only if they were enabled before the trap occurred.
; If the exception is not handled by the kernel debugger and interrupts were
; previously disabled, then a bugcheck will occur.
;

        test    qword ptr TrEFlags[rbp], EFLAGS_IF_MASK ; test if interrupt enabled
        jz      short exit              ; if z, interrupts not enabled
        sti                             ; enable interrupts
exit:                                   ; reference label

        endm

