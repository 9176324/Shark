/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
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
* The Initial Developer of the Original e is blindtiger.
*
*/

#ifndef _EXCEPT_H_
#define _EXCEPT_H_

#include "Reload.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#define MAXIMUM_USER_FUNCTION_TABLE_SIZE 512
#define MAXIMUM_KERNEL_FUNCTION_TABLE_SIZE 256

    typedef struct _FUNCTION_TABLE_ENTRY32 {
        ULONG FunctionTable;
        ULONG ImageBase;
        ULONG SizeOfImage;
        ULONG SizeOfTable;
    } FUNCTION_TABLE_ENTRY32, *PFUNCTION_TABLE_ENTRY32;

    typedef struct _FUNCTION_TABLE_ENTRY64 {
        ULONG64 FunctionTable;
        ULONG64 ImageBase;
        ULONG SizeOfImage;
        ULONG SizeOfTable;
    } FUNCTION_TABLE_ENTRY64, *PFUNCTION_TABLE_ENTRY64;

    typedef struct _FUNCTION_TABLE {
        ULONG CurrentSize;
        ULONG MaximumSize;
        ULONG Epoch;
        BOOLEAN Overflow;
        ULONG TableEntry[1];
    } FUNCTION_TABLE, *PFUNCTION_TABLE;

    typedef struct _FUNCTION_TABLE_LEGACY {
        ULONG CurrentSize;
        ULONG MaximumSize;
        BOOLEAN Overflow;
        ULONG TableEntry[1];
    } FUNCTION_TABLE_LEGACY, *PFUNCTION_TABLE_LEGACY;

    ULONG
        NTAPI
        EncodeSystemPointer32(
            __in ULONG Pointer
        );

    ULONG
        NTAPI
        DecodeSystemPointer32(
            __in ULONG Pointer
        );

    VOID
        NTAPI
        CaptureImageExceptionValues(
            __in PVOID Base,
            __out PVOID * FunctionTable,
            __out PULONG TableSize
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_EXCEPT_H_
