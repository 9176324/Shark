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

#ifndef _CTX_H_
#define _CTX_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#ifndef _WIN64
#define GetTrapFrame(Thread) GetTrapFrameThread(Thread)
#define GetBaseTrapFrame(Thread) GetBaseTrapFrameThread(Thread)
#else
    typedef struct _KSTACK_SEGMENT_LEGACY {
        ULONG_PTR StackBase;
        ULONG_PTR StackLimit;
        ULONG_PTR KernelStack;
        ULONG_PTR InitialStack;
        ULONG_PTR ActualLimit;
    } KSTACK_SEGMENT_LEGACY, *PKSTACK_SEGMENT_LEGACY;

    typedef struct _KSTACK_CONTROL_LEGACY {
        KSTACK_SEGMENT_LEGACY Current;
        KSTACK_SEGMENT_LEGACY Previous;
    } KSTACK_CONTROL_LEGACY, *PKSTACK_CONTROL_LEGACY;

    typedef struct _KSTACK_SEGMENT {
        ULONG_PTR StackBase;
        ULONG_PTR StackLimit;
        ULONG_PTR KernelStack;
        ULONG_PTR InitialStack;
    } KSTACK_SEGMENT, *PKSTACK_SEGMENT;

    typedef struct _KSTACK_CONTROL {
        ULONG_PTR StackBase;

        union {
            ULONG_PTR ActualLimit;
            BOOLEAN StackExpansion : 1;
        };

        KSTACK_SEGMENT Previous;
    }KSTACK_CONTROL, *PKSTACK_CONTROL;

#define GetTrapFrame(Thread) GetTrapFrameThread(Thread)
#define GetBaseTrapFrame(Thread) GetBaseTrapFrameThread(Thread)
#endif // !_WIN64

    PKTRAP_FRAME
        NTAPI
        GetBaseTrapFrameThread(
            __in PETHREAD Thread
        );

    PKTRAP_FRAME
        NTAPI
        GetTrapFrameThread(
            __in PKTHREAD Thread
        );

    VOID
        NTAPI
        SetTrapFrameThread(
            __in PKTHREAD Thread,
            __in PKTRAP_FRAME TrapFrame
        );

    PKAPC_STATE
        NTAPI
        GetApcStateThread(
            __in PKTHREAD Thread
        );

#ifdef _WIN64
    NTSTATUS
        NTAPI
        PsWow64GetContextThread(
            __in PETHREAD Thread,
            __inout PWOW64_CONTEXT ThreadContext,
            __in KPROCESSOR_MODE Mode
        );

    NTSTATUS
        NTAPI
        PsWow64SetContextThread(
            __in PETHREAD Thread,
            __in PWOW64_CONTEXT ThreadContext,
            __in KPROCESSOR_MODE Mode
        );
#endif // _WIN64

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_CTX_H_
