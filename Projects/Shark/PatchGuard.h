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

#ifndef _PATCHGUARD_H_
#define _PATCHGUARD_H_

#include "..\..\WRK\base\ntos\mm\mi.h"

#include "Reload.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef LONG EX_SPIN_LOCK, *PEX_SPIN_LOCK;

#define POOL_BIG_TABLE_ENTRY_FREE 0x1

    typedef struct _POOL_BIG_PAGES {
        PVOID Va;
        ULONG Key;
        ULONG PoolType;
        SIZE_T NumberOfPages;
    } POOL_BIG_PAGES, *PPOOL_BIG_PAGES;

    typedef struct _KPRIQUEUE {
        DISPATCHER_HEADER Header;
        LIST_ENTRY EntryListHead[32];
        LONG CurrentCount[32];
        ULONG MaximumCount;
        LIST_ENTRY ThreadListHead;
    }KPRIQUEUE, *PKPRIQUEUE;

    typedef struct _PATCHGUARD_BLOCK {
#define PG_MAXIMUM_CONTEXT_COUNT            0x00000002UI32 // Worker 中可能存在的 Context 最大数量
#define PG_FIRST_FIELD_OFFSET               0x00000100UI32 // 搜索使用的第一个 Context 成员偏移
#define PG_CMP_APPEND_DLL_SECTION_END       0x000000c0UI32 // CmpAppendDllSection 长度
#define PG_COMPARE_FIELDS_COUNT             0x00000004UI32 // 搜索时比较的 Context 成员数量
#define PG_MAXIMUM_EP_BUFFER_COUNT          PG_COMPARE_FIELDS_COUNT // EntryPoint 缓存大小 用来搜索头部的代码片段
        // ( 最小长度 =  max(2 * 8 + 7, sizeof(JMP_CODE)) )

#define PG_FIELD_BITS \
            ((ULONG)((((PG_FIRST_FIELD_OFFSET + PG_COMPARE_FIELDS_COUNT * sizeof(PVOID)) \
                - PG_CMP_APPEND_DLL_SECTION_END) / sizeof(PVOID)) - 1))

        PVOID
        (NTAPI * IoGetInitialStack)(
            VOID
            );

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

        ULONG
        (NTAPI * DbgPrint)(
            __in PCH Format,
            ...
            );

        // pointer to _Message[0]
        PSTR ClearEncryptedContextMessage;

        // pointer to _Message[1]
        PSTR RevertWorkerToSelfMessage;

        SIZE_T
        (NTAPI * RtlCompareMemory)(
            const VOID * Destination,
            const VOID * Source,
            SIZE_T Length
            );

        // pointer to _SdbpCheckDll
        PVOID SdbpCheckDll;
        SIZE_T SizeOfSdbpCheckDll;

        // pointer to _CheckPatchGuardCode
        BOOLEAN
        (NTAPI * CheckPatchGuardCode)(
            __in PVOID BaseAddress,
            __in SIZE_T RegionSize
            );

        // pointer to _ClearEncryptedContext
        VOID
        (NTAPI * ClearEncryptedContext)(
            VOID
            );

        // pointer to _RevertWorkerToSelf
        VOID
        (NTAPI * RevertWorkerToSelf)(
            VOID
            );

        VOID
        (NTAPI * RtlRestoreContext)(
            __in PCONTEXT ContextRecord,
            __in_opt struct _EXCEPTION_RECORD *ExceptionRecord
            );

        VOID
        (NTAPI * ExQueueWorkItem)(
            __inout PWORK_QUEUE_ITEM WorkItem,
            __in WORK_QUEUE_TYPE QueueType
            );

        VOID
        (NTAPI * ExFreePool)(
            __in PVOID P
            );

        LONG_PTR ReferenceCount;

        KEVENT Notify;

        PVOID KernelBase;

        ULONG BuildNumber;
        CCHAR NumberProcessors;

        PPOOL_BIG_PAGES PoolBigPageTable;
        SIZE_T PoolBigPageTableSize;
        PEX_SPIN_LOCK ExpLargePoolTableLock;

        struct {
            PRTL_BITMAP BitMap;
            PMMPTE BasePte;
        }SystemPtes;

        BOOLEAN IsBtcEncryptedEnable;
        ULONG RvaOffsetOfEntry;
        ULONG SizeOfCmpAppendDllSection;
        ULONG SizeOfNtSection;
        ULONG_PTR CompareFields[PG_COMPARE_FIELDS_COUNT];
        ULONG_PTR EntryPoint[PG_COMPARE_FIELDS_COUNT];
        WORK_QUEUE_ITEM ClearWorkerItem;

        KIRQL
        (NTAPI * ExAcquireSpinLockShared)(
            __inout PEX_SPIN_LOCK SpinLock
            );

        VOID
        (NTAPI * ExReleaseSpinLockShared)(
            __inout PEX_SPIN_LOCK SpinLock,
            __in KIRQL OldIrql
            );

        POOL_TYPE
        (NTAPI * MmDeterminePoolType)(
            __in PVOID VirtualAddress
            );

        CHAR _SdbpCheckDll[0x40];
        CHAR _Message[2][0x60];

        struct {
            struct PATCHGUARD_BLOCK * PatchGuardBlock;

            // shellcode clone _ClearEncryptedContext
            CHAR _ShellCode[0x40];
        }_ClearEncryptedContext;

        struct {
            struct PATCHGUARD_BLOCK * PatchGuardBlock;

            // shellcode clone _RevertWorkerToSelf
            CHAR _ShellCode[0x70];
        }_RevertWorkerToSelf;

        struct {
            // shellcode clone _CheckPatchGuardCode
            CHAR _ShellCode[0x50];
        }_CheckPatchGuardCode;

        struct {
            BOOLEAN Usable;

            struct PATCHGUARD_BLOCK * PatchGuardBlock;

            PVOID Parameters[4];

            // shellcode clone _GuardCall
            CHAR _ShellCode[0x1c0];
        }_GuardCall[PG_MAXIMUM_CONTEXT_COUNT];
    }PATCHGUARD_BLOCK, *PPATCHGUARD_BLOCK;

#ifndef _WIN64
#else
    ULONG64
        NTAPI
        _btc64(
            __in ULONG64 a,
            __in ULONG64 b
        );

    VOID
        NTAPI
        _MakePgFire(
            VOID
        );

    PVOID
        NTAPI
        _ClearEncryptedContext(
            __in PVOID Reserved,
            __in PVOID PatchGuardContext
        );

    VOID
        NTAPI
        _RevertWorkerToSelf(
            VOID
        );

    VOID
        NTAPI
        _PgGuardCall(
            VOID
        );

    VOID
        NTAPI
        _CheckPatchGuardCode(
            __in PVOID BaseAddress,
            __in SIZE_T RegionSize
        );
#endif // !_WIN64

    VOID
        NTAPI
        DisablePatchGuard(
            __inout PPATCHGUARD_BLOCK PatchGuardBlock
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_PATCHGUARD_H_
