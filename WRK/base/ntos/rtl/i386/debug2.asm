        title  "Debug Support Functions"
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
;    debug.s
;
; Abstract:
;
;    This module implements functions to support debugging NT.
;
;--
.386p


        .xlist
include ks386.inc
include callconv.inc            ; calling convention macros
        .list

_TEXT	SEGMENT PUBLIC DWORD 'CODE'
ASSUME  DS:FLAT, ES:FLAT, FS:NOTHING, GS:NOTHING, SS:NOTHING

cPublicProc _DbgBreakPoint        ,0
cPublicFpo 0,0
        int 3
        stdRET    _DbgBreakPoint
stdENDP _DbgBreakPoint

cPublicProc _DbgUserBreakPoint        ,0
cPublicFpo 0,0
        int 3
        stdRET    _DbgUserBreakPoint
stdENDP _DbgUserBreakPoint

cPublicProc _DbgBreakPointWithStatus,1
cPublicFpo 1,0
        mov eax,[esp+4]
        public _RtlpBreakWithStatusInstruction@0
_RtlpBreakWithStatusInstruction@0:
        int 3
        stdRET  _DbgBreakPointWithStatus
stdENDP _DbgBreakPointWithStatus


_TEXT   ends
        end

