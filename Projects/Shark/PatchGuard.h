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

#ifdef _WIN64                                           
    C_ASSERT(sizeof(POOL_BIG_PAGES) == sizeof(u) * 3);
#endif // _WIN64

    typedef struct _POOL_BIG_PAGESEX {
        ptr Va;
        u32 Key;
        u32 PoolType;
        u NumberOfPages;
        u Unuse;
    } POOL_BIG_PAGESEX, *PPOOL_BIG_PAGESEX;

#ifdef _WIN64                                       
    C_ASSERT(sizeof(POOL_BIG_PAGES) == sizeof(u) * 3);
#endif // _WIN64

    enum {
        PgPoolBigPage,
        PgSystemPtes,
        PgMaximumType
    };

    enum {
        PgDeclassified,
        PgEncrypted,
        PgDoubleEncrypted
    };

    typedef struct _PGOBJECT {
        LIST_ENTRY Entry;
        WORK_QUEUE_ITEM Worker;
        b Encrypted;
        u64 Key;
        uptr Context;
        u ContextSize;
        u8 Type;
        ptr BaseAddress;
        u RegionSize;
        SAFEGUARD_BODY Body;
    }PGOBJECT, *PPGOBJECT;

    // build number > 20000
    typedef enum _MI_SYSTEM_VA_TYPE {
        MiVaUnused = 0,
        MiVaSessionSpace = 1,
        MiVaProcessSpace = 2,
        MiVaBootLoaded = 3,
        MiVaPfnDatabase = 4,
        MiVaNonPagedPool = 5,
        MiVaPagedPool = 6,
        MiVaSpecialPoolPaged = 7,
        MiVaSystemCache = 8,
        MiVaSystemPtes = 9,
        MiVaHal = 10,
        MiVaSessionGlobalSpace = 11,
        MiVaDriverImages = 12,
        MiVaSystemPtesLarge = 13,
        MiVaKernelStacks = 14,
        MiVaSecureNonPagedPool = 15,
        MiVaKernelShadowStacks = 16,
        MiVaMaximumType
    }MI_SYSTEM_VA_TYPE, *PMI_SYSTEM_VA_TYPE;

    typedef struct _PGBLOCK {
        struct _GPBLOCK * GpBlock;

#define GetGpBlock(pgb) (pgb->GpBlock)

#define PG_MAXIMUM_CONTEXT_COUNT 0x00000003UI32 // The maximum number of context that may exist
#define PG_FIRST_FIELD_OFFSET 0x00000100UI32 // offset of the first context member used by the search
#define PG_CMP_APPEND_DLL_SECTION_END 0x000000c0UI32 // CmpAppendDllSection length
#define PG_COMPARE_FIELDS_COUNT 0x00000004UI32 // number of context members compared during search
#define PG_COMPARE_BYTE_COUNT 0x00000010UI32 // number of bytes compared when searching for workers

        // EntryPoint cache size used to search the code fragment of the header 
        // (minimum length = max(2 * 8 + 7, sizeof(GUARD_BODY)))
#define PG_MAXIMUM_EP_BUFFER_COUNT ((max(2 * 8 + 7, sizeof(GUARD_BODY)) + 7) & ~7)

#define PG_FIELD_BITS \
            ((u32)((((PG_FIRST_FIELD_OFFSET + PG_COMPARE_FIELDS_COUNT * sizeof(ptr)) \
                - PG_CMP_APPEND_DLL_SECTION_END) / sizeof(ptr)) - 1))

        u8 Count;
        b BtcEnable;
        u32 OffsetEntryPoint;
        u32 SizeCmpAppendDllSection;
        u32 SizeINITKDBG;
        ptr INITKDBG;
        s32 BuildKey;
        s32 BranchKey[12];

        uptr OriginalCmpAppendDllSection;
        uptr CacheCmpAppendDllSection;

        uptr KiWaitNever;
        uptr KiWaitAlways;

        struct {
            union {
                PPOOL_BIG_PAGES * PoolBigPageTable;
                PPOOL_BIG_PAGESEX * PoolBigPageTableEx;
            };

            uptr PoolBigPageTableSize;

            b
            (NTAPI * MmIsNonPagedSystemAddressValid)(
                __in ptr VirtualAddress
                );
        }Pool; // pool big page

        struct {
            u NumberOfPtes;
            PMMPTE BasePte;
        }SystemPtes; // system ptes       

#define SYSTEM_REGION_TYPE_ARRAY_COUNT 0x100

        s8 * SystemRegionTypeArray;

        struct {
            ptr WorkerContext;

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

        b
        (NTAPI * MmIsAddressValid)(
            __in ptr VirtualAddress
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

        u
        (FASTCALL * CmpDecode)(
            __in u Value,
            __in u8 Count
            );

        u
        (FASTCALL * Ror64)(
            __in u Value,
            __in u8 Count
            );

        u
        (FASTCALL * Rol64)(
            __in u Value,
            __in u8 Count
            );

        u64
        (FASTCALL *  RorWithBtc64)(
            __in u64 Value,
            __in u64 Count
            );

        void
        (NTAPI * CaptureContext)(
            __in u32 ProgramCounter,
            __in ptr Guard,
            __in ptr Stack,
            __in_opt ptr Parameter,
            __in_opt ptr Reserved
            );

        u
        (FASTCALL * PostCache)(
            __in u Index,
            __in ptr Context
            );

        u
        (FASTCALL * PostKey)(
            __in u Index,
            __in u Original
            );

        cptr ClearMessage[3];

        LIST_ENTRY ObjectList;
        KSPIN_LOCK ObjectLock;

        u64 Fields[PG_COMPARE_FIELDS_COUNT];
        u8 Header[PG_MAXIMUM_EP_BUFFER_COUNT];
        u8 _Ror64[8];
        u8 _Rol64[8];
        u8 _RorWithBtc64[16];
        u8 _PostCache[16];
        u8 _PostKey[64];

#ifndef _WIN64
        u8 _CaptureContext[0x100];
#else
        u8 _CaptureContext[0x200];
#endif // !_WIN64

        u8 _ClearMessage[0x120];
        u8 _FreeWorker[0xB0];
        u8 _ClearCallback[0xE00];

        u8 _OriginalCmpAppendDllSection[0xC8];
        u8 _CacheCmpAppendDllSection[0xC8];
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
