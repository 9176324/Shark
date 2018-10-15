        title  "Raise Exception"
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
;    raise.asm
;
; Abstract:
;
;    This module implements the function to raise a software exception.
;
;--
.386p
        .xlist
include ks386.inc
include callconv.inc            ; calling convention macros
        .list

        EXTRNP  _ZwRaiseException,3
        EXTRNP  _RtlDispatchException,2
        EXTRNP  _ZwContinue,2
        EXTRNP  _RtlCaptureContext,1

_TEXT$01 SEGMENT DWORD PUBLIC 'CODE'
         ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

;
; Context flags definition.
;

CONTEXT_SETTING EQU CONTEXT_INTEGER OR CONTEXT_CONTROL OR CONTEXT_SEGMENTS

        page
        subttl  "Raise Software Exception"
;++
;
; VOID
; RtlRaiseException (
;    IN PEXCEPTION_RECORD ExceptionRecord
;    )
;
; Routine Description:
;
;    This function raises a software exception by building a context record,
;    and calling the exception dispatcher. If the exception dispatcher finds
;    a handler to process the exception the handler is invoked and execution 
;    is resumed. Otherwise the ZwRaiseException system service is invoked to 
;    provide default handling.
;
;    N.B. On the 386, floating point state is not defined for non-fp exceptions.
;      Therefore, this routine does not attempt to capture it.
;
;      This means this routine cannot be used to report fp exceptions.
;
; Arguments:
;
;    ExceptionRecord (ebp+8) - Supplies a pointer to an exception record.
;
; Return Value:
;
;    None.
;
;--

cPublicProc _RtlRaiseException, 1

        push    ebp
        mov     ebp, esp
        lea     esp, dword ptr [esp-ContextFrameLength]  ; Allocate a context record

;
; Invoke the helper routine to capture the caller's context. This 
; will capture the segment registers, volatile/nonvolatile registers,
; and the control registers. Note that the status flags have not been
; perturbed from their state prior to this call.
;

        stdCall _RtlCaptureContext, <esp> ; Assumes esp is pushed prior to decrement

;
; Get a pointer to exception report record, and set the exception address
; field to be the return address of this function.
;
; The captured stack pointer does not take into account the arguments
; passed to this function. Make sure to adjust it here (assuming esp is 
; properly restored).
;


        mov     edx, [ebp+4]
        mov     eax, [ebp+8]                    ; (eax) -> ExceptionReportRecord
                
        add     dword ptr [esp].CsEsp, 4        ; Adjust captured esp
        
        mov     [eax].ErExceptionAddress, edx
        
;
; Set Context flags, note that FLOATING_POINT is NOT set.
;

        mov     dword ptr [esp].CsContextFlags, CONTEXT_SETTING

;
; Perform a direct dispatch of the exception (assuming esp is pushed
; prior to any modification).
;

        stdCall _RtlDispatchException, <[ebp+8], esp>
        test    al,al
        jz      short rre02             

;
; In this case, the handler indicated that execution will be resumed. Continue
; execution via the ZwContinue system service.
;

        mov     ecx, esp                ; Restore context record address
        stdCall  _ZwContinue, <ecx, 0>
        jmp     rre03                   ; Raise a status exception if we ever get here.

;
; The exception was not handled. Raise a second chance exception. Take
; care to restore the address of the context record (assuming esp is
; restored correctly by RtlDispatchException above).
;

rre02:  mov     ecx, esp
        stdCall _ZwRaiseException, <[ebp+8], ecx, 0>

;
; There should never be a return from either exception dispatch or the
; system service unless there is a problem with the argument list itself.
; Raise another exception specifying the status value returned.
;

rre03:  stdCall _RtlRaiseStatus, <eax>
     
;
; The code should not return here as RtlRaiseStatus raises a
; non-continuable exception.
;
                
stdENDP _RtlRaiseException

        subttl  "Raise Non-Continuable Software Exception"
;++
;
; VOID
; RtlRaiseStatus (
;     IN NTSTATUS Status
;     )
;
; Routine Description:
;
;    This function raises an exception with the specified status value. The
;    exception is marked as noncontinuable with no parameters.
;
;    N.B. On the 386, floating point state is not defined for non-fp exceptions.  
;      Therefore, this routine does not attempt to capture it.
;
;      This means this routine cannot be used to report fp exceptions.
;
;    N.B. There is no return from this routine.
;
; Arguments:
;
;     Status - Supplies the status value to be used as the exception code
;        for the exception that is to be raised.
;
; Return Value:
;
;     None.
;--

cPublicProc _RtlRaiseStatus, 1
        push    ebp
        mov     ebp,esp
        lea     esp, dword ptr [esp-ContextFrameLength-ExceptionRecordLength]

;
; Invoke the helper routine to capture the caller's context. This 
; will capture the segment registers, volatile/nonvolatile registers,
; and the control registers. Note that the status flags have not been
; perturbed from their state prior to this call.
;

        stdCall _RtlCaptureContext, <esp> ; Assumes esp is pushed prior to decrement

;
; The captured stack pointer does not take into account the arguments
; passed to this function. Adjust it here.
;

        add dword ptr [esp].CsEsp, 4

;
; Get pointer to exception report record, and set the exception address
; field to be this routine's return address. Additionally, set the
; appropriate context flags in the context record.
;

        lea     ecx, dword ptr [esp+ContextFrameLength] ; Exception record
        mov     eax, [ebp+4]                            ; Return address
        mov     dword ptr [esp].CsContextFlags, CONTEXT_SETTING
        
        mov     dword ptr [ecx].ErExceptionAddress, eax   
        and     dword ptr [ecx].ErNumberParameters, 0
        mov     eax, [ebp+8]                            ; Exception code
        and     dword ptr [ecx].ErExceptionRecord, 0
        mov     dword ptr [ecx].ErExceptionCode, eax
        mov     dword ptr [ecx].ErExceptionFlags, EXCEPTION_NONCONTINUABLE

;
; ecx - Exception record
; esp - Context record
;

        stdCall _RtlDispatchException, <ecx, esp> ; Assumes esp is pushed prior to decrement
        
;
; If execution returns here, raise a second chance exception. Take care
; to reload the volatile registers, assuming esp is restored properly
; by RtlDispatchException above.
;

        lea     ecx, dword ptr [esp+ContextFrameLength]
        mov     edx, esp
        stdCall _ZwRaiseException, <ecx, edx, 0>

;
; There should never be a return from either exception dispatch or the
; system service unless there is a problem with the argument list itself.
; Recursively raise another exception specifying the status value returned.
;

        stdCall _RtlRaiseStatus, <eax>
     
;
; The code should not return here as RtlRaiseStatus raises a
; non-continuable exception.
;
        
stdENDP _RtlRaiseStatus

_TEXT$01 ends
         end

