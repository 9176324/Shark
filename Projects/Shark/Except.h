/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
* http://www.mozilla.org/MPL/
*
* Software distributed under the License is distributed on an "AS IS" basis,
* WITHOUT WARRANTY OF ANY KIND, either express or implied. SEe the License
* for the specific language governing rights and limitations under the
* License.
*
* The Initial Developer of the Original Code is blindtiger.
*
*/

#ifndef _EXCEPT_H_
#define _EXCEPT_H_

#include "Reload.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    void
        NTAPI
        CaptureImageExceptionValues(
            __in ptr Base,
            __out ptr * FunctionTable,
            __out u32ptr TableSize
        );

    void
        NTAPI
        InitializeExcept(
            __inout PRTB Block
        );

#ifndef _WIN64
    typedef struct _DISPATCHER_CONTEXT {
        PEXCEPTION_REGISTRATION_RECORD RegistrationPointer;
    } DISPATCHER_CONTEXT;
#else
    void
        NTAPI
        InsertInvertedFunctionTable(
            __in ptr ImageBase,
            __in u32 SizeOfImage
        );

    void
        NTAPI
        RemoveInvertedFunctionTable(
            __in ptr ImageBase
        );

    PRUNTIME_FUNCTION
        NTAPI
        UnwindPrologue(
            __in u64 ImageBase,
            __in u64 ControlPc,
            __in u64 FrameBase,
            __in PRUNTIME_FUNCTION FunctionEntry,
            __inout PCONTEXT ContextRecord,
            __inout_opt PKNONVOLATILE_CONTEXT_POINTERS ContextPointers
        );

    PEXCEPTION_ROUTINE
        NTAPI
        VirtualUnwind(
            __in u32 HandlerType,
            __in u64 ImageBase,
            __in u64 ControlPc,
            __in PRUNTIME_FUNCTION FunctionEntry,
            __inout PCONTEXT ContextRecord,
            __out ptr * HandlerData,
            __out u64ptr EstablisherFrame,
            __inout_opt PKNONVOLATILE_CONTEXT_POINTERS ContextPointers
        );
#endif // !_WIN64

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_EXCEPT_H_
