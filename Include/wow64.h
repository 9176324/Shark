/*
*
* Copyright (c) 2015-2018 by blindtiger ( blindtiger@foxmail.com )
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

#ifndef _WOW64_H_
#define _WOW64_H_

#ifdef __cplusplus
/* Assume C declarations for C++ */
extern "C" {
#endif	/* __cplusplus */

#ifdef _WIN64

#if !defined(RC_INVOKED)

#define WOW64_CONTEXT_i386      0x00010000    // this assumes that i386 and
#define WOW64_CONTEXT_i486      0x00010000    // i486 have identical context records

#define WOW64_CONTEXT_CONTROL               (WOW64_CONTEXT_i386 | 0x00000001L) // SS:SP, CS:IP, FLAGS, BP
#define WOW64_CONTEXT_INTEGER               (WOW64_CONTEXT_i386 | 0x00000002L) // AX, BX, CX, DX, SI, DI
#define WOW64_CONTEXT_SEGMENTS              (WOW64_CONTEXT_i386 | 0x00000004L) // DS, ES, FS, GS
#define WOW64_CONTEXT_FLOATING_POINT        (WOW64_CONTEXT_i386 | 0x00000008L) // 387 state
#define WOW64_CONTEXT_DEBUG_REGISTERS       (WOW64_CONTEXT_i386 | 0x00000010L) // DB 0-3,6,7
#define WOW64_CONTEXT_EXTENDED_REGISTERS    (WOW64_CONTEXT_i386 | 0x00000020L) // cpu specific extensions

#define WOW64_CONTEXT_FULL      (WOW64_CONTEXT_CONTROL | WOW64_CONTEXT_INTEGER | WOW64_CONTEXT_SEGMENTS)

#define WOW64_CONTEXT_ALL       (WOW64_CONTEXT_CONTROL | WOW64_CONTEXT_INTEGER | WOW64_CONTEXT_SEGMENTS | \
                                 WOW64_CONTEXT_FLOATING_POINT | WOW64_CONTEXT_DEBUG_REGISTERS | \
                                 WOW64_CONTEXT_EXTENDED_REGISTERS)

#define WOW64_CONTEXT_XSTATE                (WOW64_CONTEXT_i386 | 0x00000040L)

#endif // !defined(RC_INVOKED)

    //
    //  Define the size of the 80387 save area, which is in the context frame.
    //

#define WOW64_SIZE_OF_80387_REGISTERS      80

#define WOW64_MAXIMUM_SUPPORTED_EXTENSION     512

    typedef struct _WOW64_FLOATING_SAVE_AREA {
        ULONG ControlWord;
        ULONG StatusWord;
        ULONG TagWord;
        ULONG ErrorOffset;
        ULONG ErrorSelector;
        ULONG DataOffset;
        ULONG DataSelector;
        BOOLEAN RegisterArea[WOW64_SIZE_OF_80387_REGISTERS];
        ULONG Cr0NpxState;
    } WOW64_FLOATING_SAVE_AREA;

    typedef WOW64_FLOATING_SAVE_AREA *PWOW64_FLOATING_SAVE_AREA;

#include "pshpack4.h"

    //
    // Context Frame
    //
    //  This frame has a several purposes: 1) it is used as an argument to
    //  NtContinue, 2) is is used to constuct a call frame for APC delivery,
    //  and 3) it is used in the user level thread creation routines.
    //
    //  The layout of the record conforms to a standard call frame.
    //

    typedef struct _WOW64_CONTEXT {

        //
        // The flags values within this flag control the contents of
        // a CONTEXT record.
        //
        // If the context record is used as an input parameter, then
        // for each portion of the context record controlled by a flag
        // whose value is set, it is assumed that that portion of the
        // context record contains valid context. If the context record
        // is being used to modify a threads context, then only that
        // portion of the threads context will be modified.
        //
        // If the context record is used as an IN OUT parameter to capture
        // the context of a thread, then only those portions of the thread's
        // context corresponding to set flags will be returned.
        //
        // The context record is never used as an OUT only parameter.
        //

        ULONG ContextFlags;

        //
        // This section is specified/returned if CONTEXT_DEBUG_REGISTERS is
        // set in ContextFlags.  Note that CONTEXT_DEBUG_REGISTERS is NOT
        // included in CONTEXT_FULL.
        //

        ULONG Dr0;
        ULONG Dr1;
        ULONG Dr2;
        ULONG Dr3;
        ULONG Dr6;
        ULONG Dr7;

        //
        // This section is specified/returned if the
        // ContextFlags word contians the flag CONTEXT_FLOATING_POINT.
        //

        WOW64_FLOATING_SAVE_AREA FloatSave;

        //
        // This section is specified/returned if the
        // ContextFlags word contians the flag CONTEXT_SEGMENTS.
        //

        ULONG SegGs;
        ULONG SegFs;
        ULONG SegEs;
        ULONG SegDs;

        //
        // This section is specified/returned if the
        // ContextFlags word contians the flag CONTEXT_INTEGER.
        //

        ULONG Edi;
        ULONG Esi;
        ULONG Ebx;
        ULONG Edx;
        ULONG Ecx;
        ULONG Eax;

        //
        // This section is specified/returned if the
        // ContextFlags word contians the flag CONTEXT_CONTROL.
        //

        ULONG Ebp;
        ULONG Eip;
        ULONG SegCs;              // MUST BE SANITIZED
        ULONG EFlags;             // MUST BE SANITIZED
        ULONG Esp;
        ULONG SegSs;

        //
        // This section is specified/returned if the ContextFlags word
        // contains the flag CONTEXT_EXTENDED_REGISTERS.
        // The format and contexts are processor specific
        //

        BOOLEAN ExtendedRegisters[WOW64_MAXIMUM_SUPPORTED_EXTENSION];

    } WOW64_CONTEXT;

    typedef WOW64_CONTEXT *PWOW64_CONTEXT;

    C_ASSERT(sizeof(WOW64_CONTEXT) == 0x2cc);

#define WOW64_CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Eip)
#define WOW64_PROGRAM_COUNTER_TO_CONTEXT(Context, ProgramCounter) ((Context)->Eip = (ProgramCounter))

#define WOW64_CONTEXT_LENGTH  (sizeof(WOW64_CONTEXT))
#define WOW64_CONTEXT_ALIGN   (sizeof(ULONG))
#define WOW64_CONTEXT_ROUND   (CONTEXT_ALIGN - 1)

#include "poppack.h"

    typedef struct _WOW64_LDT_ENTRY {
        USHORT LimitLow;
        USHORT BaseLow;

        union {
            struct {
                BOOLEAN BaseMid;
                BOOLEAN Flags1;     // Declare as bytes to avoid alignment
                BOOLEAN Flags2;     // Problems.
                BOOLEAN BaseHi;
            } Bytes;

            struct {
                ULONG BaseMid : 8;
                ULONG Type : 5;
                ULONG Dpl : 2;
                ULONG Pres : 1;
                ULONG LimitHi : 4;
                ULONG Sys : 1;
                ULONG Reserved_0 : 1;
                ULONG Default_Big : 1;
                ULONG Granularity : 1;
                ULONG BaseHi : 8;
            } Bits;
        } HighWord;
    } WOW64_LDT_ENTRY, *PWOW64_LDT_ENTRY;

    typedef struct _WOW64_DESCRIPTOR_TABLE_ENTRY {
        ULONG Selector;
        WOW64_LDT_ENTRY Descriptor;
    } WOW64_DESCRIPTOR_TABLE_ENTRY, *PWOW64_DESCRIPTOR_TABLE_ENTRY;

    NTSYSCALLAPI
        NTSTATUS
        NTAPI
        RtlWow64SuspendThread(
            __in HANDLE ThreadHandle,
            __out_opt PULONG PreviousSuspendCount
        );

    NTSYSCALLAPI
        NTSTATUS
        NTAPI
        RtlWow64GetThreadContext(
            __in HANDLE ThreadHandle,
            __inout PWOW64_CONTEXT ThreadContext
        );

    NTSYSCALLAPI
        NTSTATUS
        NTAPI
        RtlWow64SetThreadContext(
            __in HANDLE ThreadHandle,
            __in PWOW64_CONTEXT ThreadContext
        );

#endif // _WIN64

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_WOW64_H_
