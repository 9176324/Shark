        TITLE   "String support routines"
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
;    stringsup.asm
;
; Abstract:
;
;    This module implements supplementary routines for performing string
;    operations.
;
;--

.386p

include ks386.inc
include callconv.inc            ; calling convention macros

if DBG
_DATA   SEGMENT  DWORD PUBLIC 'DATA'

_MsgStringTooLong   db  'RTL: RtlInit*String called with string length: (%x)\n',0

_DATA ENDS

    extrn   _KdDebuggerEnabled:BYTE
    EXTRNP  _DbgBreakPoint,0
    extrn   _DbgPrint:near
endif


_TEXT   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

        page ,132
        subttl  "RtlInitAnsiString"
;++
;
; VOID
; RtlInitAnsiString(
;     OUT PANSI_STRING DestinationString,
;     IN PSZ SourceString OPTIONAL
;     )
;
;
; Routine Description:
;
;    The RtlInitAnsiString function initializes an NT counted string.
;    The DestinationString is initialized to point to the SourceString
;    and the Length and MaximumLength fields of DestinationString are
;    initialized to the length of the SourceString, which is zero if
;    SourceString is not specified.
;
; Arguments:
;
;    (TOS+4) = DestinationString - Pointer to the counted string to initialize
;
;    (TOS+8) = SourceString - Optional pointer to a null terminated string that
;        the counted string is to point to.
;
;
; Return Value:
;
;    None.
;
; NOTE:
;       This routine assumes that the string is less than 64K in size.
;
;--

cPublicProc _RtlInitString ,2
cPublicFpo 2,2
        push    edi
        mov     edi,[esp]+8+4           ; (edi)= SourceString
        mov     edx,[esp]+4+4           ; (esi)= DestinationString
        mov     DWORD PTR [edx], 0      ; (Destination).Length = (Destination).MaximumLength = 0
        mov     DWORD PTR [edx]+4, edi  ; (Destination).Buffer = 0
        or      edi, edi
        jz      @f
        or      ecx, -1
        xor     eax, eax
        repne   scasb
        not     ecx
        cmp     ecx,0ffffh
        jbe     rtis

;
; spew length and bp if debugger attached.
;
if DBG
        push    edx
        push    ecx
        push    offset FLAT:_MsgStringTooLong
        call    _DbgPrint
        add     esp, 2 * 4
        cmp     _KdDebuggerEnabled,0
        je      nobp1
        call    _DbgBreakPoint@0
nobp1:  pop     edx
endif

        mov     ecx,0ffffh              ; overflow, truncate at MAXUSHORT
rtis:   mov     [edx]+2, cx             ; Save maximum length
        dec     ecx
        mov     [edx], cx               ; Save length
@@:     pop     edi
        stdRET    _RtlInitString

stdENDP _RtlInitString


cPublicProc _RtlInitAnsiString ,2
cPublicFpo 2,2
        push    edi
        mov     edi,[esp]+8+4           ; (edi)= SourceString
        mov     edx,[esp]+4+4           ; (esi)= DestinationString
        mov     DWORD PTR [edx], 0      ; (Destination).Length = (Destination).MaximumLength = 0
        mov     DWORD PTR [edx]+4, edi  ; (Destination).Buffer = 0
        or      edi, edi
        jz      @f
        or      ecx, -1
        xor     eax, eax
        repne   scasb
        not     ecx
        cmp     ecx,0ffffh
        jbe     rtias
;
; spew length and bp if debugger attached.
;
if DBG
        push    edx
        push    ecx
        push    offset FLAT:_MsgStringTooLong
        call    _DbgPrint
        add     esp, 2 * 4
        cmp     _KdDebuggerEnabled,0
        je      nobp2
        call    _DbgBreakPoint@0
nobp2:  pop    edx
endif

        mov     ecx,0ffffh              ; overflow, truncate at MAXUSHORT
rtias:  mov     [edx]+2, cx             ; Save maximum length
        dec     ecx
        mov     [edx], cx               ; Save length
@@:     pop     edi
        stdRET    _RtlInitAnsiString

stdENDP _RtlInitAnsiString


        page
        subttl  "RtlInitUnicodeString"
;++
;
; VOID
; RtlInitUnicodeString(
;     OUT PUNICODE_STRING DestinationString,
;     IN PWSTR SourceString OPTIONAL
;     )
;
;
; Routine Description:
;
;    The RtlInitUnicodeString function initializes an NT counted string.
;    The DestinationString is initialized to point to the SourceString
;    and the Length and MaximumLength fields of DestinationString are
;    initialized to the length of the SourceString, which is zero if
;    SourceString is not specified.
;
; Arguments:
;
;    (TOS+4) = DestinationString - Pointer to the counted string to initialize
;
;    (TOS+8) = SourceString - Optional pointer to a null terminated string that
;        the counted string is to point to.
;
;
; Return Value:
;
;    None.
;
; NOTE:
;       This routine assumes that the string is less than 64K in size.
;
;--

cPublicProc _RtlInitUnicodeString ,2
cPublicFpo 2,2
        push    edi
        mov     edi,[esp]+8+4           ; (edi)= SourceString
        mov     edx,[esp]+4+4           ; (esi)= DestinationString
        mov     DWORD PTR [edx], 0      ; (Destination).Length = (Destination).MaximumLength = 0
        mov     DWORD PTR [edx]+4, edi  ; (Destination).Buffer = 0
        or      edi, edi
        jz      @f
        or      ecx, -1
        xor     eax, eax
        repne   scasw
        not     ecx
        shl     ecx,1
        cmp     ecx,0fffeh
        jbe     rtius

;
; spew length and bp if debugger attached.
;
if DBG
        push    edx
        push    ecx
        push    offset FLAT:_MsgStringTooLong
        call    _DbgPrint
        add     esp, 2 * 4
        cmp     _KdDebuggerEnabled,0
        je      nobp3
        call    _DbgBreakPoint@0
nobp3:  pop     edx
endif

        mov     ecx,0fffeh              ; overflow, truncate at MAX_USTRING
rtius:  mov     [edx]+2, cx             ; Save maximum length
        dec     ecx
        dec     ecx
        mov     [edx], cx               ; Save length
@@:     pop     edi
        stdRET    _RtlInitUnicodeString

stdENDP _RtlInitUnicodeString

_TEXT   ends
        end

