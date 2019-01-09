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

    C_ASSERT(sizeof(FUNCTION_TABLE_ENTRY32) == 0x10);

    typedef struct _FUNCTION_TABLE_ENTRY64 {
        ULONG64 FunctionTable;
        ULONG64 ImageBase;
        ULONG SizeOfImage;
        ULONG SizeOfTable;
    } FUNCTION_TABLE_ENTRY64, *PFUNCTION_TABLE_ENTRY64;

    C_ASSERT(sizeof(FUNCTION_TABLE_ENTRY64) == 0x18);

    typedef struct _FUNCTION_TABLE {
        ULONG CurrentSize;
        ULONG MaximumSize;
        ULONG Epoch;
        BOOLEAN Overflow;
        ULONG TableEntry[1];
    } FUNCTION_TABLE, *PFUNCTION_TABLE;

    C_ASSERT(FIELD_OFFSET(FUNCTION_TABLE, TableEntry) == 0x10);

    typedef struct _FUNCTION_TABLE_LEGACY {
        ULONG CurrentSize;
        ULONG MaximumSize;
        BOOLEAN Overflow;
        ULONG TableEntry[1];
    } FUNCTION_TABLE_LEGACY, *PFUNCTION_TABLE_LEGACY;

    C_ASSERT(FIELD_OFFSET(FUNCTION_TABLE_LEGACY, TableEntry) == 0xc);

    ULONG
        NTAPI
        EncodeSystemPointer(
            __in ULONG Pointer
        );

    ULONG
        NTAPI
        RtlDecodeSystemPointer(
            __in ULONG Pointer
        );

    VOID
        NTAPI
        CaptureImageExceptionValues(
            __in PVOID Base,
            __out PVOID * FunctionTable,
            __out PULONG TableSize
        );

    VOID
        NTAPI
        InsertInvertedFunctionTable(
            __in PVOID ImageBase,
            __in ULONG SizeOfImage
        );

    VOID
        NTAPI
        RemoveInvertedFunctionTable(
            __in PVOID ImageBase
        );

#ifdef _WIN64
    VOID
        NTAPI
        Wx86InsertInvertedFunctionTable(
            __in PVOID ImageBase,
            __in ULONG SizeOfImage
        );

    VOID
        NTAPI
        Wx86RemoveInvertedFunctionTable(
            __in PVOID ImageBase
        );
#endif // _WIN64

    PVOID
        NTAPI
        LookupFunctionTableEx(
            __in PVOID ControlPc,
            __inout PVOID * ImageBase,
            __out PULONG SizeOfTable
        );

    PVOID
        NTAPI
        LookupFunctionTable(
            __in PVOID ControlPc,
            __out PVOID * ImageBase,
            __out PULONG SizeOfTable
        );

#ifdef _WIN64
    PRUNTIME_FUNCTION
        NTAPI
        LookupFunctionEntry(
            __in ULONG64 ControlPc,
            __out PULONG64 ImageBase,
            __inout_opt PUNWIND_HISTORY_TABLE HistoryTable
        );

    PEXCEPTION_ROUTINE
        NTAPI
        VirtualUnwind(
            __in ULONG HandlerType,
            __in ULONG64 ImageBase,
            __in ULONG64 ControlPc,
            __in PRUNTIME_FUNCTION FunctionEntry,
            __inout PCONTEXT ContextRecord,
            __out PVOID * HandlerData,
            __out PULONG64 EstablisherFrame,
            __inout_opt PKNONVOLATILE_CONTEXT_POINTERS ContextPointers
        );
#endif // _WIN64

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_EXCEPT_H_
