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
;    irqli386.inc - 386 machine specific IRQL manipulation routines
;
; Abstract:
;
;   This module provides macros to raise, lower and get current IRQL.
;   These routines are abstracted through macros in order to facilitate
;   generation of a kernel that manipulates IRQL directly.
;
;   If _APIC_TPR_ is defined, IRQL is manipulated directly via the PIC.
;   Otherwise, the appropriate HAL routine is called.
;
; This module exposes four public macros:
;
; CurrentIrql
; RaiseIrql
; LowerIrql
; LowerIrqlRet
;


ifdef _APIC_TPR_

        APIC_TPR    equ dword ptr ds:0FFFE0080h
        extrn       _HalpIRQLToTPR:DWORD
        extrn       _HalpVectorToIRQL:DWORD

;++
;
;   CurrentIrql
;
;   Macro Description:
;
;       This macro retrieves the current IRQL in eax
;
;   Arguments:
;
;       None
;
;   Return Value:
;
;       The current IRQL is returned in EAX.
;
;--

CurrentIrql macro
        mov     eax, APIC_TPR
        shr     eax, 4
        add     eax, _HalpVectorToIRQL
        movzx   eax, BYTE PTR [eax]
endm

;++
;
;   SetIrql Irql
;
;   Macro Description:
;
;       This macro sets IRQL to the supplied value.  This macro is an
;       internal routine only.
;
;   Arguments:
;
;       Irql - provides the value for the desired new IRQL.  It is supplied in
;           one of three forms:
;
;           cl    - the value is zero-extended into ecx before use
;           ecx   - cl has already been zero-extended
;           const - Constant expression is used directly
;
;   Return Value:
;
;       None.
;
;--

SetIrql macro Irql
ifidn <Irql>, <ecx>
        add     ecx, _HalpIRQLToTPR
        movzx   ecx, BYTE PTR [ecx]
else
ifidn <Irql>, <cl>
        movzx   ecx, cl
        add     ecx, _HalpIRQLToTPR
        movzx   ecx, BYTE PTR [ecx]
else
ifnb <Irql>
        mov     ecx, _HalpIRQLToTPR
        movzx   ecx, BYTE PTR [ecx+Irql]
else
.err Irqlparm not supplied to SetIrql
endif
endif
endif
        mov     APIC_TPR, ecx
endm

;++
;
;   LowerIrql Irql
;
;   Macro Description:
;
;       This macro lowers IRQL to the supplied value.
;
;   Arguments:
;
;       Irql - provides the value for the desired new IRQL.  It is supplied in
;           one of three forms:
;
;           cl    - the value is zero-extended into ecx before use
;           ecx   - cl has already been zero-extended
;           const - Constant expression is used directly
;
;   Return Value:
;
;       None.
;
;--

LowerIrql macro Irql
        SetIrql Irql
        mov     ecx, APIC_TPR
endm

;++
;
;   LowerIrqlRet Irql
;
;   Macro Description:
;
;       This macro lowers IRQL to the supplied value and returns to the calling
;       routine.
;
;   Arguments:
;
;       Irql - provides the value for the desired new IRQL.  It is supplied in
;           one of three forms:
;
;           cl    - the value is zero-extended into ecx before use
;           ecx   - cl has already been zero-extended
;           const - Constant expression is used directly
;
;   Return Value:
;
;       None.
;
;   Comments:
;
;       This macro should be used only in cases where the function epilogue is
;       known to be identical to that of KfLowerIrql().
;
;--

LowerIrqlRet macro Irql
        LowerIrql Irql
        ret
endm

;++
;
;   RaiseIrql Irql [,NoOld]
;
;   Macro Description:
;
;       This macro raises IRQL to the supplied value.
;
;   Arguments:
;
;       Irql - provides the value for the desired new IRQL.  It is supplied in
;           one of three forms:
;
;           cl    - the value is zero-extended into ecx before use
;           ecx   - cl has already been zero-extended
;           const - Constant expression is used directly
;
;       NoOld - Optional.  If supplied, no value is returned in eax.
;
;   Return Value:
;
;       Unless the NoOld flag is supplied, eax contains the zero-extended
;       value of the IRQL before raising.
;
;--

RaiseIrql macro Irql, nooldirql
ifidn <nooldirql>, <NoOld>
else
ifnb <nooldirql>
.err RaiseIrql used with optional parm other than NoOld
else
        CurrentIrql
endif
endif
        SetIrql Irql
endm

else

        extrn __imp_@KfRaiseIrql@4:DWORD
        extrn __imp_@KfLowerIrql@4:DWORD
        extrn __imp__KeGetCurrentIrql@0:DWORD

;++
;
;   CurrentIrql
;
;   Macro Description:
;
;       This macro retrieves the current IRQL in eax
;
;   Arguments:
;
;       None
;
;   Return Value:
;
;       The current IRQL is returned in EAX.
;
;--

CurrentIrql macro
        call    __imp__KeGetCurrentIrql@0
endm

;++
;
;   LowerIrql Irql
;
;   Macro Description:
;
;       This macro lowers IRQL to the supplied value.
;
;   Arguments:
;
;       Irql - provides the value for the desired new IRQL.  It is supplied in
;           one of three forms:
;
;           cl    - the value is zero-extended into ecx before use
;           ecx   - cl has already been zero-extended
;           const - Constant expression is used directly
;
;   Return Value:
;
;       None.
;
;--

LowerIrql macro Irql
ifidn <Irql>, <cl>
else
ifidn <Irql>, <ecx>
else
        mov     ecx, Irql
endif
endif
        call    __imp_@KfLowerIrql@4
endm

;++
;
;   LowerIrqlRet Irql
;
;   Macro Description:
;
;       This macro lowers IRQL to the supplied value and returns to the calling
;       routine.
;
;   Arguments:
;
;       Irql - provides the value for the desired new IRQL.  It is supplied in
;           one of three forms:
;
;           cl    - the value is zero-extended into ecx before use
;           ecx   - cl has already been zero-extended
;           const - Constant expression is used directly
;
;   Return Value:
;
;       None.
;
;   Comments:
;
;       This macro should be used only in cases where the function epilogue is
;       known to be identical to that of KfLowerIrql().
;
;--

LowerIrqlRet macro Irql
ifidn <Irql>, <cl>
else
ifidn <Irql>, <ecx>
else
        mov     ecx, Irql
endif
endif
        jmp     __imp_@KfLowerIrql@4
endm

;++
;
;   RaiseIrql Irql [,NoOld]
;
;   Macro Description:
;
;       This macro raises IRQL to the supplied value.
;
;   Arguments:
;
;       Irql - provides the value for the desired new IRQL.  It is supplied in
;           one of three forms:
;
;           cl    - the value is zero-extended into ecx before use
;           ecx   - cl has already been zero-extended
;           const - Constant expression is used directly
;
;       NoOld - Optional.  If supplied, no value is returned in eax.
;
;   Return Value:
;
;       Unless the NoOld flag is supplied, eax contains the zero-extended
;       value of the IRQL before raising.
;
;--

RaiseIrql macro Irql
ifidn <Irql>, <cl>
else
ifidn <Irql>, <ecx>
else
        mov     ecx, Irql
endif
endif
        call    __imp_@KfRaiseIrql@4
endm

endif

