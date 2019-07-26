/*
*
* Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
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
        ULONG ApcRoutine;
        ULONG SystemRoutine;
        ULONG StartRoutine;
        ULONG StartContext;
        ULONG Result;
    }ROUTINES32, *PROUTINES32;

    typedef struct _ROUTINES64 {
        ULONG64 ApcRoutine;
        ULONG64 SystemRoutine;
        ULONG64 StartRoutine;
        ULONG64 StartContext;
        ULONG64 Result;
    }ROUTINES64, *PROUTINES64;

    typedef struct _ROUTINES {
        PPS_APC_ROUTINE ApcRoutine;
        PKSYSTEM_ROUTINE SystemRoutine;
        PUSER_THREAD_START_ROUTINE StartRoutine;
        PVOID StartContext;
        ULONG_PTR Result;
    }ROUTINES, *PROUTINES;

    typedef struct _WORKER_OBJECT {
        SINGLE_LIST_ENTRY NextEntry;
        ROUTINES Routines;
    }WORKER_OBJECT, *PWORKER_OBJECT;

    typedef struct _RTX {
        POBJECT Object;
        POBJECT Target;
        PVOID ApiMessage;
        KEVENT Notify;

        USHORT Platform;
        ULONG Processor;

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

    ULONG_PTR
        NTAPI
        _GuardCall(
            __in_opt PPS_APC_ROUTINE ApcRoutine,
            __in_opt PKSYSTEM_ROUTINE SystemRoutine,
            __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
            __in_opt PVOID StartContext
        );

    FORCEINLINE
    ULONG_PTR
        NTAPI
        GuardCall(
            __in_opt PPS_APC_ROUTINE ApcRoutine,
            __in_opt PKSYSTEM_ROUTINE SystemRoutine,
            __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
            __in_opt PVOID StartContext
        )
    {
        ULONG_PTR Result = 0;

        __try {
            Result = _GuardCall(
                ApcRoutine,
                SystemRoutine,
                StartRoutine,
                StartContext);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            NOTHING;
        }

        return Result;
    }

    NTSTATUS
        NTAPI
        AsyncCall(
            __in HANDLE UniqueThread,
            __in_opt PPS_APC_ROUTINE ApcRoutine,
            __in_opt PKSYSTEM_ROUTINE SystemRoutine,
            __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
            __in_opt PVOID StartContext
        );

    ULONG_PTR
        NTAPI
        IpiSingleCall(
            __in_opt PPS_APC_ROUTINE ApcRoutine,
            __in_opt PKSYSTEM_ROUTINE SystemRoutine,
            __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
            __in_opt PVOID StartContext
        );

    VOID
        NTAPI
        IpiGenericCall(
            __in_opt PPS_APC_ROUTINE ApcRoutine,
            __in_opt PKSYSTEM_ROUTINE SystemRoutine,
            __in_opt PUSER_THREAD_START_ROUTINE StartRoutine,
            __in_opt PVOID StartContext
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_RTX_H_
