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

        u64 P1Home;
        u64 P2Home;
        u64 P3Home;
        u64 P4Home;
        u64 P5;

        //
        // Kernel callout initial stack value.
        //

        u64 InitialStack;

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

        u64 TrapFrame;
        u64 CallbackStack;
        u64 OutputBuffer;
        u64 OutputLength;

        //
        // Saved MXCSR when a thread is interrupted in kernel mode via a dispatch
        // interrupt.
        //

        u64 MxCsr;

        //
        // Saved nonvolatile register - not always saved.
        //

        u64 Rbp;

        //
        // Saved nonvolatile registers.
        //

        u64 Rbx;
        u64 Rdi;
        u64 Rsi;
        u64 R12;
        u64 R13;
        u64 R14;
        u64 R15;

        //
        // EFLAGS and return address.
        //

        u64 Return;
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
            (u)Thread +
            RtBlock.DebuggerDataBlock.OffsetKThreadApcProcess,
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
        return *(PCCHAR)((u)Thread +
            RtBlock.DebuggerDataBlock.OffsetKThreadState);
    }

    FORCEINLINE
        PLIST_ENTRY
        NTAPI
        GetProcessThreadListHead(
            __inout PRTB RtBlock,
            __inout PEPROCESS Process
        )
    {
        return (PLIST_ENTRY)((u8ptr)Process +
            RtBlock->OffsetKProcessThreadListHead);
    }

    FORCEINLINE
        PETHREAD
        NTAPI
        GetProcessFirstThread(
            __inout PRTB RtBlock,
            __inout PEPROCESS Process
        )
    {
        return (PETHREAD)
            ((u8ptr)GetProcessThreadListHead(
                RtBlock, Process)->Flink -
                RtBlock->OffsetKThreadThreadListEntry);
    }

    FORCEINLINE
        PLIST_ENTRY
        NTAPI
        GetThreadListEntry(
            __inout PRTB RtBlock,
            __inout PETHREAD Thread
        )
    {
        return (PLIST_ENTRY)((u8ptr)Thread +
            RtBlock->OffsetKThreadThreadListEntry);
    }

    FORCEINLINE
        PETHREAD
        NTAPI
        GetNexThread(
            __inout PRTB RtBlock,
            __inout PETHREAD Thread
        )
    {
        return (PETHREAD)
            ((u8ptr)(((PLIST_ENTRY)((u8ptr)Thread +
                RtBlock->OffsetKThreadThreadListEntry))->Flink) -
                RtBlock->OffsetKThreadThreadListEntry);
    }

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_CTX_H_
