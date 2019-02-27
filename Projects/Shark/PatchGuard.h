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

#ifndef _PATCHGUARD_H_
#define _PATCHGUARD_H_

#include "..\..\WRK\base\ntos\mm\mi.h"

#include "Detour.h"
#include "Reload.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#define POOL_BIG_TABLE_ENTRY_FREE 0x1

    typedef struct _POOL_BIG_PAGES {
        PVOID Va;
        ULONG Key;
        ULONG PoolType;
        SIZE_T NumberOfPages;
    } POOL_BIG_PAGES, *PPOOL_BIG_PAGES;

    enum {
        PgPoolBigPage,
        PgSystemPtes
    };

    enum {
        PgDeclassified,
        PgEncrypted
    };

    typedef struct _PGOBJECT {
        BOOLEAN Encrypted;
        CCHAR Type;
        PVOID BaseAddress;
        SIZE_T RegionSize;
        struct _PGBLOCK * PgBlock;

        struct {
            COUNTER_BODY Establisher; // establisher
        }; // only worker use

        COUNTER_BODY Callback; // callback
        COUNTER_BODY Original; // original address
        COUNTER_BODY Self; // self address
        COUNTER_BODY CaptureContext; // capture context
        CHAR Ret[1];
    }PGOBJECT, *PPGOBJECT;

    typedef struct _PGBLOCK {
        // struct _GPBLOCK Block;

#define PG_MAXIMUM_CONTEXT_COUNT 0x00000002UI32 // Worker 中可能存在的 Context 最大数量
#define PG_FIRST_FIELD_OFFSET 0x00000100UI32 // 搜索使用的第一个 Context 成员偏移
#define PG_CMP_APPEND_DLL_SECTION_END 0x000000c0UI32 // CmpAppendDllSection 长度
#define PG_COMPARE_FIELDS_COUNT 0x00000004UI32 // 搜索时比较的 Context 成员数量

        // EntryPoint 缓存大小 用来搜索头部的代码片段 ( 最小长度 =  max(2 * 8 + 7, sizeof(DETOUR_BODY)) )
#define PG_MAXIMUM_EP_BUFFER_COUNT ((max(2 * 8 + 7, sizeof(DETOUR_BODY)) + 7) & ~7)

#define PG_FIELD_BITS \
            ((ULONG)((((PG_FIRST_FIELD_OFFSET + PG_COMPARE_FIELDS_COUNT * sizeof(PVOID)) \
                - PG_CMP_APPEND_DLL_SECTION_END) / sizeof(PVOID)) - 1))

        LONG_PTR ReferenceCount;

        BOOLEAN BtcEnable;
        ULONG OffsetEntryPoint;
        ULONG SizeCmpAppendDllSection;
        ULONG SizeINITKDBG;
        SIZE_T SizeSdbpCheckDll;
        ULONG_PTR Fields[PG_COMPARE_FIELDS_COUNT];
        CHAR Header[PG_MAXIMUM_EP_BUFFER_COUNT];

        struct {
            PPOOL_BIG_PAGES PoolBigPageTable;
            SIZE_T PoolBigPageTableSize;
            PEX_SPIN_LOCK ExpLargePoolTableLock;

            POOL_TYPE
            (NTAPI * MmDeterminePoolType)(
                __in PVOID VirtualAddress
                );
        }; // pool big page

        struct {
            PRTL_BITMAP BitMap;
            PMMPTE BasePte;
        }; // system ptes

        struct {
            PVOID WorkerContext;

            VOID
            (NTAPI * ExpWorkerThread)(
                __in PVOID StartContext
                );

            VOID
            (NTAPI * PspSystemThreadStartup)(
                __in PKSTART_ROUTINE StartRoutine,
                __in PVOID StartContext
                );

            VOID
            (NTAPI * KiStartSystemThread)(
                VOID
                );
        }; // restart ExpWorkerThread

        VOID
        (NTAPI * MmFreeIndependentPages)(
            __in PVOID VirtualAddress,
            __in SIZE_T NumberOfBytes
            );

        VOID
        (NTAPI * SdbpCheckDll)(
            __in ULONG BugCheckCode,
            __in ULONG_PTR P1,
            __in ULONG_PTR P2,
            __in ULONG_PTR P3,
            __in ULONG_PTR P4,
            __in VOID
            (NTAPI * KeBugCheckEx)(
                __in ULONG BugCheckCode,
                __in ULONG_PTR P1,
                __in ULONG_PTR P2,
                __in ULONG_PTR P3,
                __in ULONG_PTR P4
                ),
            __in ULONG64 InitialStack
            );

        ULONG64
        (NTAPI *  Btc64)(
            __in ULONG64 a,
            __in ULONG64 b
            );

        VOID
        (NTAPI * ClearCallback)(
            __in PCONTEXT Context,
            __in PPGOBJECT Object
            );

        PSTR Message[2];

        CHAR _SdbpCheckDll[0x3c];
        CHAR _Btc64[8];
        CHAR _ClearCallback[0x2e0];
        CHAR _Message[0x55];
    }PGBLOCK, *PPGBLOCK;

    VOID
        NTAPI
        PgClear(
            __inout PPGBLOCK PgBlock
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_PATCHGUARD_H_
