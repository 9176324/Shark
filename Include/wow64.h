/*
*
* Copyright (c) 2015 - 2021 by blindtiger ( blindtiger@foxmail.com )
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
        u32 ControlWord;
        u32 StatusWord;
        u32 TagWord;
        u32 ErrorOffset;
        u32 ErrorSelector;
        u32 DataOffset;
        u32 DataSelector;
        b RegisterArea[WOW64_SIZE_OF_80387_REGISTERS];
        u32 Cr0NpxState;
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

        u32 ContextFlags;

        //
        // This section is specified/returned if CONTEXT_DEBUG_REGISTERS is
        // set in ContextFlags.  Note that CONTEXT_DEBUG_REGISTERS is NOT
        // included in CONTEXT_FULL.
        //

        u32 Dr0;
        u32 Dr1;
        u32 Dr2;
        u32 Dr3;
        u32 Dr6;
        u32 Dr7;

        //
        // This section is specified/returned if the
        // ContextFlags word contians the flag CONTEXT_FLOATING_POINT.
        //

        WOW64_FLOATING_SAVE_AREA FloatSave;

        //
        // This section is specified/returned if the
        // ContextFlags word contians the flag CONTEXT_SEGMENTS.
        //

        u32 SegGs;
        u32 SegFs;
        u32 SegEs;
        u32 SegDs;

        //
        // This section is specified/returned if the
        // ContextFlags word contians the flag CONTEXT_INTEGER.
        //

        u32 Edi;
        u32 Esi;
        u32 Ebx;
        u32 Edx;
        u32 Ecx;
        u32 Eax;

        //
        // This section is specified/returned if the
        // ContextFlags word contians the flag CONTEXT_CONTROL.
        //

        u32 Ebp;
        u32 Eip;
        u32 SegCs;              // MUST BE SANITIZED
        u32 EFlags;             // MUST BE SANITIZED
        u32 Esp;
        u32 SegSs;

        //
        // This section is specified/returned if the ContextFlags word
        // contains the flag CONTEXT_EXTENDED_REGISTERS.
        // The format and contexts are processor specific
        //

        b ExtendedRegisters[WOW64_MAXIMUM_SUPPORTED_EXTENSION];

    } WOW64_CONTEXT;

    typedef WOW64_CONTEXT *PWOW64_CONTEXT;

    C_ASSERT(sizeof(WOW64_CONTEXT) == 0x2cc);

#define WOW64_CONTEXT_TO_PROGRAM_COUNTER(Context) ((Context)->Eip)
#define WOW64_PROGRAM_COUNTER_TO_CONTEXT(Context, ProgramCounter) ((Context)->Eip = (ProgramCounter))

#define WOW64_CONTEXT_LENGTH  (sizeof(WOW64_CONTEXT))
#define WOW64_CONTEXT_ALIGN   (sizeof(u32))
#define WOW64_CONTEXT_ROUND   (WOW64_CONTEXT_ALIGN - 1)

#define ThreadWow64Context 29

#include "poppack.h"

    typedef struct _WOW64_LDT_ENTRY {
        u16 LimitLow;
        u16 BaseLow;

        union {
            struct {
                b BaseMid;
                b Flags1;     // Declare as bytes to avoid alignment
                b Flags2;     // Problems.
                b BaseHi;
            } Bytes;

            struct {
                u32 BaseMid : 8;
                u32 Type : 5;
                u32 Dpl : 2;
                u32 Pres : 1;
                u32 LimitHi : 4;
                u32 Sys : 1;
                u32 Reserved_0 : 1;
                u32 Default_Big : 1;
                u32 Granularity : 1;
                u32 BaseHi : 8;
            } Bits;
        } HighWord;
    } WOW64_LDT_ENTRY, *PWOW64_LDT_ENTRY;

    typedef struct _WOW64_DESCRIPTOR_TABLE_ENTRY {
        u32 Selector;
        WOW64_LDT_ENTRY Descriptor;
    } WOW64_DESCRIPTOR_TABLE_ENTRY, *PWOW64_DESCRIPTOR_TABLE_ENTRY;

    NTSYSCALLAPI
        NTSTATUS
        NTAPI
        RtlWow64SuspendThread(
            __in ptr ThreadHandle,
            __out_opt u32ptr PreviousSuspendCount
        );

    NTSYSCALLAPI
        NTSTATUS
        NTAPI
        RtlWow64GetThreadContext(
            __in ptr ThreadHandle,
            __inout PWOW64_CONTEXT ThreadContext
        );

    NTSYSCALLAPI
        NTSTATUS
        NTAPI
        RtlWow64SetThreadContext(
            __in ptr ThreadHandle,
            __in PWOW64_CONTEXT ThreadContext
        );

#endif // _WIN64

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_WOW64_H_
