/*
*
* Copyright (c) 2019 by blindtiger. All rights reserved.
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

#include "Reload.h"

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


    FORCEINLINE
        PKAPC_STATE
        NTAPI
        GetApcStateThread(
            __in PKTHREAD Thread
        )
    {
        return CONTAINING_RECORD(
            (ULONG_PTR)Thread +
            GpBlock->DebuggerDataBlock.OffsetKThreadApcProcess,
            KAPC_STATE,
            Process);
    }

    FORCEINLINE
        KTHREAD_STATE
        NTAPI
        GetThreadState(
            __in PKTHREAD Thread
        )
    {
        return *(PCCHAR)((ULONG_PTR)Thread +
            GpBlock->DebuggerDataBlock.OffsetKThreadState);
    }

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_CTX_H_
