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

#ifndef _CTX_H_
#define _CTX_H_

#include "Reload.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#ifdef _WIN64
    typedef struct _EXCEPTION_FRAME {
        //
        // Home address for the parameter registers.
        //

        ULONG64 P1Home;
        ULONG64 P2Home;
        ULONG64 P3Home;
        ULONG64 P4Home;
        ULONG64 P5;

        //
        // Kernel callout initial stack value.
        //

        ULONG64 InitialStack;

        //
        // Saved nonvolatile floating registers.
        //

        M128A Xmm6;
        M128A Xmm7;
        M128A Xmm8;
        M128A Xmm9;
        M128A Xmm10;
        M128A Xmm11;
        M128A Xmm12;
        M128A Xmm13;
        M128A Xmm14;
        M128A Xmm15;

        //
        // Kernel callout frame variables.
        //

        ULONG64 TrapFrame;
        ULONG64 CallbackStack;
        ULONG64 OutputBuffer;
        ULONG64 OutputLength;

        //
        // Saved MXCSR when a thread is interrupted in kernel mode via a dispatch
        // interrupt.
        //

        ULONG64 MxCsr;

        //
        // Saved nonvolatile register - not always saved.
        //

        ULONG64 Rbp;

        //
        // Saved nonvolatile registers.
        //

        ULONG64 Rbx;
        ULONG64 Rdi;
        ULONG64 Rsi;
        ULONG64 R12;
        ULONG64 R13;
        ULONG64 R14;
        ULONG64 R15;

        //
        // EFLAGS and return address.
        //

        ULONG64 Return;
    }EXCEPTION_FRAME, *PEXCEPTION_FRAME;

#define EXCEPTION_FRAME_LENGTH sizeof(EXCEPTION_FRAME)

    C_ASSERT((sizeof(EXCEPTION_FRAME) & STACK_ROUND) == 0);

#endif // _WIN64

#define GetBaseTrapFrame(Thread) GetBaseTrapFrameThread(Thread)

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
            GpBlock.DebuggerDataBlock.OffsetKThreadApcProcess,
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
            GpBlock.DebuggerDataBlock.OffsetKThreadState);
    }

    FORCEINLINE
        PLIST_ENTRY
        NTAPI
        GetProcessThreadListHead(
            __inout PGPBLOCK GpBlock,
            __inout PEPROCESS Process
        )
    {
        return (PLIST_ENTRY)((PCHAR)Process +
            GpBlock->OffsetKProcessThreadListHead);
    }

    FORCEINLINE
        PETHREAD
        NTAPI
        GetProcessFirstThread(
            __inout PGPBLOCK GpBlock,
            __inout PEPROCESS Process
        )
    {
        return (PETHREAD)
            ((PCHAR)GetProcessThreadListHead(
                GpBlock, Process)->Flink -
                GpBlock->OffsetKThreadThreadListEntry);
    }

    FORCEINLINE
        PLIST_ENTRY
        NTAPI
        GetThreadListEntry(
            __inout PGPBLOCK GpBlock,
            __inout PETHREAD Thread
        )
    {
        return (PLIST_ENTRY)((PCHAR)Thread +
            GpBlock->OffsetKThreadThreadListEntry);
    }

    FORCEINLINE
        PETHREAD
        NTAPI
        GetNexThread(
            __inout PGPBLOCK GpBlock,
            __inout PETHREAD Thread
        )
    {
        return (PETHREAD)
            ((PCHAR)(((PLIST_ENTRY)((PCHAR)Thread +
                GpBlock->OffsetKThreadThreadListEntry))->Flink) -
                GpBlock->OffsetKThreadThreadListEntry);
    }

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_CTX_H_
