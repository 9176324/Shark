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
* The Initial Developer of the Original e is blindtiger.
*
*/

#ifndef _RTX_H_
#define _RTX_H_

#include <devicedefs.h>

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef struct _OBJECT *POBJECT;

    typedef struct _ROUTINES32 {
        u32 KernelRoutine;
        u32 SystemRoutine;
        u32 RundownRoutine;
        u32 NormalRoutine;
        u32 Result;
    }ROUTINES32, *PROUTINES32;

    typedef struct _ROUTINES64 {
        u64 KernelRoutine;
        u64 SystemRoutine;
        u64 RundownRoutine;
        u64 NormalRoutine;
        u64 Result;
    }ROUTINES64, *PROUTINES64;

    typedef struct _ROUTINES {
        PGKERNEL_ROUTINE KernelRoutine;
        PGSYSTEM_ROUTINE SystemRoutine;
        PGRUNDOWN_ROUTINE RundownRoutine;
        PGNORMAL_ROUTINE NormalRoutine;
        u Result;
    }ROUTINES, *PROUTINES;

    typedef struct _WORKER_OBJECT {
        SINGLE_LIST_ENTRY NextEntry;
        ROUTINES Routines;
    }WORKER_OBJECT, *PWORKER_OBJECT;

    typedef struct _RTX {
        POBJECT Object;
        POBJECT Target;
        ptr ApiMessage;
        KEVENT Notify;

        u16 Platform;
        u32 Processor;

        union {
            ROUTINES Routines;
            ROUTINES32 Routines32;
            ROUTINES64 Routines64;
        };

        KPROCESSOR_MODE Mode;
    } RTX, *PRTX;

    typedef struct _ATX {
        KAPC Apc;
        RTX Rtx;
    } ATX, *PATX;

#define MAXIMUM_COMPARE_INSTRUCTION_COUNT 8

    status
        NTAPI
        AsyncCall(
            __in ptr UniqueThread,
            __in_opt PGKERNEL_ROUTINE KernelRoutine,
            __in_opt PGSYSTEM_ROUTINE SystemRoutine,
            __in_opt PGRUNDOWN_ROUTINE RundownRoutine,
            __in_opt PGNORMAL_ROUTINE NormalRoutine
        );

    u
        NTAPI
        IpiSingleCall(
            __in_opt PGKERNEL_ROUTINE KernelRoutine,
            __in_opt PGSYSTEM_ROUTINE SystemRoutine,
            __in_opt PGRUNDOWN_ROUTINE RundownRoutine,
            __in_opt PGNORMAL_ROUTINE NormalRoutine
        );

    void
        NTAPI
        IpiGenericCall(
            __in_opt PGKERNEL_ROUTINE KernelRoutine,
            __in_opt PGSYSTEM_ROUTINE SystemRoutine,
            __in_opt PGRUNDOWN_ROUTINE RundownRoutine,
            __in_opt PGNORMAL_ROUTINE NormalRoutine
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_RTX_H_
