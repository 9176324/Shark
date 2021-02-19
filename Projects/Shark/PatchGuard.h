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

#ifndef _PATCHGUARD_H_
#define _PATCHGUARD_H_

#include "..\..\WRK\base\ntos\mm\mi.h"

#include "Guard.h"
#include "Reload.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#define POOL_BIG_TABLE_ENTRY_FREE 0x1

    typedef struct _POOL_BIG_PAGES {
        ptr Va;
        u32 Key;
        u32 PoolType;
        u NumberOfPages;
    } POOL_BIG_PAGES, *PPOOL_BIG_PAGES;

    enum {
        PgPoolBigPage,
        PgSystemPtes,
        PgMaximumType
    };

    enum {
        PgDeclassified,
        PgEncrypted
    };

    typedef struct _PGOBJECT {
        LIST_ENTRY Entry;
        WORK_QUEUE_ITEM Worker;
        b Encrypted;
        u64 XorKey;
        u8 Type;
        ptr BaseAddress;
        u RegionSize;
        SAFEGUARD_BODY Body;
    }PGOBJECT, *PPGOBJECT;

    typedef enum _MI_SYSTEM_VA_TYPE {
        MiVaUnused,
        MiVaSessionSpace,
        MiVaProcessSpace,
        MiVaBootLoaded,
        MiVaPfnDatabase,
        MiVaNonPagedPool,
        MiVaPagedPool,
        MiVaSpecialPoolPaged,
        MiVaSystemCache,
        MiVaSystemPtes,
        MiVaHal,
        MiVaSessionGlobalSpace,
        MiVaDriverImages,
        MiVaSystemPtesLarge,
        MiVaKernelStacks,
        MiVaSecureNonPagedPool,
        MiVaMaximumType
    }MI_SYSTEM_VA_TYPE, *PMI_SYSTEM_VA_TYPE;

    typedef struct _PGBLOCK {
        struct _GPBLOCK * GpBlock;

#define GetGpBlock(pgb) (pgb->GpBlock)

#define PG_MAXIMUM_CONTEXT_COUNT 0x00000003UI32 // 可能存在的 Context 最大数量
#define PG_FIRST_FIELD_OFFSET 0x00000100UI32 // 搜索使用的第一个 Context 成员偏移
#define PG_CMP_APPEND_DLL_SECTION_END 0x000000c0UI32 // CmpAppendDllSection 长度
#define PG_COMPARE_FIELDS_COUNT 0x00000004UI32 // 搜索时比较的 Context 成员数量
#define PG_COMPARE_BYTE_COUNT 0x00000010UI32 // 搜索 Worker 时比较的字节数量

        // EntryPoint 缓存大小 用来搜索头部的代码片段 ( 最小长度 =  max(2 * 8 + 7, sizeof(GUARD_BODY)) )
#define PG_MAXIMUM_EP_BUFFER_COUNT ((max(2 * 8 + 7, sizeof(GUARD_BODY)) + 7) & ~7)

#define PG_FIELD_BITS \
            ((u32)((((PG_FIRST_FIELD_OFFSET + PG_COMPARE_FIELDS_COUNT * sizeof(ptr)) \
                - PG_CMP_APPEND_DLL_SECTION_END) / sizeof(ptr)) - 1))

        u8 Count;
        u8 Cleared;
        b Repeat;
        b BtcEnable;
        u32 OffsetEntryPoint;
        u32 SizeCmpAppendDllSection;
        u32 SizeINITKDBG;
        ptr INITKDBG;

        struct {
            PPOOL_BIG_PAGES * PoolBigPageTable;
            uptr PoolBigPageTableSize;

#define POOL_TABLE (*PgBlock->Pool.PoolBigPageTable)
#define POOL_TABLE_SIZE (*PgBlock->Pool.PoolBigPageTableSize)

            b
            (NTAPI * MmIsNonPagedSystemAddressValid)(
                __in ptr VirtualAddress
                );
        }Pool; // pool big page

        struct {
            u NumberOfPtes;
            PMMPTE BasePte;
        }SystemPtes; // system ptes

        struct {
            ptr WorkerContext;

            MI_SYSTEM_VA_TYPE
            (NTAPI * MiGetSystemRegionType)(
                __in ptr VirtualAddress
                );

            void
            (NTAPI * ExpWorkerThread)(
                __in ptr StartContext
                );

            void
            (NTAPI * PspSystemThreadStartup)(
                __in PKSTART_ROUTINE StartRoutine,
                __in ptr StartContext
                );

            void
            (NTAPI * KiStartSystemThread)(
                void
                );

            u32 OffsetSameThreadPassive;
        }; // restart ExpWorkerThread

        ptr
        (NTAPI * MmAllocateIndependentPages)(
            __in u NumberOfBytes,
            __in u32 Node
            );

        void
        (NTAPI * MmFreeIndependentPages)(
            __in ptr VirtualAddress,
            __in u NumberOfBytes
            );

        b
        (NTAPI * MmSetPageProtection)(
            __in_bcount(NumberOfBytes) ptr VirtualAddress,
            __in u NumberOfBytes,
            __in u32 NewProtect
            );

        ptr
        (NTAPI * RtlLookupFunctionEntry)(
            __in u64 ControlPc,
            __out u64ptr ImageBase,
            __inout_opt ptr HistoryTable
            );

        PEXCEPTION_ROUTINE
        (NTAPI * RtlVirtualUnwind)(
            __in u32 HandlerType,
            __in u64 ImageBase,
            __in u64 ControlPc,
            __in ptr FunctionEntry,
            __inout PCONTEXT ContextRecord,
            __out ptr * HandlerData,
            __out u64ptr EstablisherFrame,
            __inout_opt ptr ContextPointers
            );

        void
        (NTAPI * ExQueueWorkItem)(
            __inout PWORK_QUEUE_ITEM WorkItem,
            __in WORK_QUEUE_TYPE QueueType
            );

        void
        (NTAPI * FreeWorker)(
            __in struct _PGOBJECT * Object
            );

        void
        (NTAPI * ClearCallback)(
            __in PCONTEXT Context,
            __in_opt ptr ProgramCounter,
            __in_opt ptr Parameter,
            __in_opt ptr Reserved
            );

        u64
        (NTAPI *  Btc64)(
            __in u64 a,
            __in u64 b
            );

        u
        (NTAPI * Ror64)(
            __in u Value,
            __in u8 Count
            );

#define ROR64(p, x, n) (p)->Ror64((x), (n))

        u
        (NTAPI * Rol64)(
            __in u Value,
            __in u8 Count
            );

#define ROL64(p, x, n) (p)->Rol64((x), (n))

        void
        (NTAPI * CaptureContext)(
            __in u32 ProgramCounter,
            __in ptr Guard,
            __in ptr Stack,
            __in_opt ptr Parameter,
            __in_opt ptr Reserved
            );

        cptr ClearMessage[2];

        LIST_ENTRY ObjectList;
        KSPIN_LOCK ObjectLock;

        u64 Fields[PG_COMPARE_FIELDS_COUNT];
        u8 Header[PG_MAXIMUM_EP_BUFFER_COUNT];
        u8 _Btc64[8];
        u8 _Ror64[8];
        u8 _Rol64[8];

#ifndef _WIN64
        u8 _CaptureContext[0x100];
#else
        u8 _CaptureContext[0x200];
#endif // !_WIN64

        u8 _ClearMessage[0x80];
        u8 _FreeWorker[0xB0];
        u8 _ClearCallback[0xC00];
    }PGBLOCK, *PPGBLOCK;

    void
        NTAPI
        PgClear(
            __inout PPGBLOCK PgBlock
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_PATCHGUARD_H_
