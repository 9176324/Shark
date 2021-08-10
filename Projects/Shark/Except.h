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

    enum {
        KiDivideErrorFault,
        KiDebugTrapOrFault,
        KiNmiInterrupt,
        KiBreakpointTrap,
        KiOverflowTrap,
        KiBoundFault,
        KiInvalidOpcodeFault,
        KiNpxNotAvailableFault,
        KiDoubleFaultAbort,
        KiNpxSegmentOverrunAbort,
        KiInvalidTssFault,
        KiSegmentNotPresentFault,
        KiStackFault,
        KiGeneralProtectionFault,
        KiPageFault,
        KiFloatingErrorFault,
        KiAlignmentFault,
        KiMcheckAbort,
        KiXmmException,
        KiApcInterrupt,
        KiRaiseAssertion,
        KiDebugServiceTrap,
        KiDpcInterrupt,
        KiIpiInterrupt,
        KiMaxInterrupt
    };

    typedef
        VOID
        (NTAPI * PINTERRUPT_HANDLER) (
            VOID
            );

    typedef struct _INTERRUPTION_FRAME {
        ULONG_PTR ProgramCounter;
        ULONG_PTR SegCs;
        ULONG_PTR Eflags;
    }INTERRUPTION_FRAME, *PINTERRUPTION_FRAME;

    typedef union _INTERRUPT_ADDRESS {
        struct {
#ifndef _WIN64
            USHORT Offset;
            USHORT ExtendedOffset;
#else
            USHORT OffsetLow;
            USHORT OffsetMiddle;
            ULONG OffsetHigh;
#endif // !_WIN64
        };

        ULONG_PTR Address;
    } INTERRUPT_ADDRESS, *PINTERRUPT_ADDRESS;

    typedef struct _OBJECT OBJECT, *POBJECT;

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

    VOID
        NTAPI
        CaptureImageExceptionValues(
            __in PVOID Base,
            __out PVOID * FunctionTable,
            __out PULONG TableSize
        );

    VOID
        NTAPI
        InitializeExcept(
            __inout PGPBLOCK Block
        );

#ifndef _WIN64
    typedef struct _DISPATCHER_CONTEXT {
        PEXCEPTION_REGISTRATION_RECORD RegistrationPointer;
    } DISPATCHER_CONTEXT;
#else
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
#endif // !_WIN64

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_EXCEPT_H_
