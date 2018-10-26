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

#include "KernelReload.h"

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
#define PG_MAXIMUM_CONTEXT_COUNT 0x00000002UI32
#define PG_KEY_INTERVAL 0x00000080UI32
#define PG_FIRST_FIELD_OFFSET 0x00000100UI32
#define PG_CMP_APPEND_DLL_SECTION_END 0x000000c0UI32
#define PG_COMPARE_FIELDS_COUNT  0x00000004UI32

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
        PVOID ClearEncryptedContextMessage;

        // pointer to _Message[1]
        PVOID RevertWorkerToSelfMessage;

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

        KEVENT Notify;

        PKLDR_DATA_TABLE_ENTRY KernelDataTableEntry;

        ULONG BuildNumber;

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
        WORK_QUEUE_ITEM ClearWorkerItem;

        PVOID MmHighestUserAddress;
        PVOID MmSystemRangeStart;

        PMMPTE PxeBase;
        PMMPTE PpeBase;
        PMMPTE PdeBase;
        PMMPTE PteBase;

        PMMPTE PxeTop;
        PMMPTE PpeTop;
        PMMPTE PdeTop;
        PMMPTE PteTop;

        PMMPFN MmPfnDatabase;

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
        CHAR _Message[2][0x40];

        struct {
            struct PATCHGUARD_BLOCK * PatchGuardBlock;

            // shellcode clone _ClearEncryptedContext
            CHAR _ShellCode[0x38];
        }_ClearEncryptedContext;

        struct {
            struct PATCHGUARD_BLOCK * PatchGuardBlock;

            // shellcode clone _RevertWorkerToSelf
            CHAR _ShellCode[0x58];
        }_RevertWorkerToSelf;

        // shellcode clone _CheckPatchGuardCode
        CHAR _CheckPatchGuardCode[0x50];

        struct {
            LONG_PTR ReferenceCount;

            struct PATCHGUARD_BLOCK * PatchGuardBlock;

            PVOID Parameters[4];

            // shellcode clone _GuardCall
            CHAR _ShellCode[0x1a0];
        }_GuardCall[PG_MAXIMUM_CONTEXT_COUNT];
    }PATCHGUARD_BLOCK, *PPATCHGUARD_BLOCK;

#ifdef _WIN64
    VOID
        NTAPI
        DisablePatchGuard(
            __inout PPATCHGUARD_BLOCK PatchGuardBlock
        );
#endif // _WIN64

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_PATCHGUARD_H_
