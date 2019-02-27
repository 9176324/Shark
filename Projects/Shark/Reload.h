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

#ifndef _RELOAD_H_
#define _RELOAD_H_

#include <devicedefs.h>
#include <dump.h>

#include "Space.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef LONG EX_SPIN_LOCK, *PEX_SPIN_LOCK;

    typedef enum _OB_PREOP_CALLBACK_STATUS OB_PREOP_CALLBACK_STATUS;
    typedef struct _OB_PRE_OPERATION_INFORMATION *POB_PRE_OPERATION_INFORMATION;
    typedef struct _OB_POST_OPERATION_INFORMATION *POB_POST_OPERATION_INFORMATION;

    typedef struct _GPBLOCK {
        PLIST_ENTRY PsLoadedModuleList;
        PEPROCESS PsInitialSystemProcess;
        PERESOURCE PsLoadedModuleResource;
        struct _FUNCTION_TABLE * PsInvertedFunctionTable;
        KSERVICE_TABLE_DESCRIPTOR * KeServiceDescriptorTable;
        KSERVICE_TABLE_DESCRIPTOR * KeServiceDescriptorTableShadow;

        PKLDR_DATA_TABLE_ENTRY KernelDataTableEntry; // ntoskrnl.exe
        PKLDR_DATA_TABLE_ENTRY CoreDataTableEntry; // self
        PVOID CpuControlBlock; // hypervisor
        struct _PRIVATE_HEADER * PrivateHeader; // private data

        struct _PRIVATE_OBJECT * NativeObject;

#ifdef _WIN64
        struct _PRIVATE_OBJECT * Wx86NativeObject;
#endif // _WIN64
        LIST_ENTRY LoadedPrivateImageList;
        LIST_ENTRY ObjectList;
        KSPIN_LOCK ObjectLock;

        struct {
            BOOLEAN Hypervisor : 1; // deploy hypervisor
            BOOLEAN PatchGuard : 1; // deploy bypass patchguard
            BOOLEAN Callback : 1; // deploy callback
        }Features;

        CCHAR NumberProcessors;
        CCHAR Reserved[2];

        ULONG BuildNumber;

#ifdef _WIN64
        PMMPTE PxeBase;
        PMMPTE PxeTop;

        PMMPTE PpeBase;
        PMMPTE PpeTop;
#endif // _WIN64

        PMMPTE PdeBase;
        PMMPTE PdeTop;

        PMMPTE PteBase;
        PMMPTE PteTop;

        VOID
        (NTAPI * CaptureContext)(
            __in ULONG ProgramCounter,
            __in PVOID Detour,
            __in struct _GUARD * Guard
            );

        VOID
        (NTAPI * KeEnterCriticalRegion)(
            VOID
            );

        VOID
        (NTAPI * KeLeaveCriticalRegion)(
            VOID
            );

        NTSTATUS
        (NTAPI * PspCreateThread)(
            __out PHANDLE ThreadHandle,
            __in ACCESS_MASK DesiredAccess,
            __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
            __in HANDLE ProcessHandle,
            __in PEPROCESS ProcessPointer,
            __in_opt PVOID Reserved,
            __in_opt PLARGE_INTEGER Cookie,
            __out_opt PCLIENT_ID ClientId,
            __in_opt PCONTEXT ThreadContext,
            __in_opt PINITIAL_TEB InitialTeb,
            __in BOOLEAN CreateSuspended,
            __in_opt PKSTART_ROUTINE StartRoutine,
            __in PVOID StartContext
            );

        BOOLEAN
        (FASTCALL * ExAcquireRundownProtection)(
            __inout PEX_RUNDOWN_REF RunRef
            );

        VOID
        (FASTCALL * ExReleaseRundownProtection)(
            __inout PEX_RUNDOWN_REF RunRef
            );

        VOID
        (FASTCALL * ExWaitForRundownProtectionRelease)(
            __inout PEX_RUNDOWN_REF RunRef
            );

        PEX_CALLBACK_ROUTINE_BLOCK
        (NTAPI * ExAllocateCallBack)(
            __in PEX_CALLBACK_FUNCTION Function,
            __in PVOID Context
            );

        VOID
        (NTAPI * ExFreeCallBack)(
            __in PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
            );

        VOID
        (NTAPI * ExWaitForCallBacks)(
            __in PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
            );

        PEX_CALLBACK_ROUTINE_BLOCK
        (NTAPI * ExReferenceCallBackBlock)(
            __inout PEX_CALLBACK CallBack
            );

        PEX_CALLBACK_FUNCTION
        (NTAPI * ExGetCallBackBlockRoutine)(
            __in PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
            );

        BOOLEAN
        (NTAPI * ExCompareExchangeCallBack)(
            __inout PEX_CALLBACK CallBack,
            __in PEX_CALLBACK_ROUTINE_BLOCK NewBlock,
            __in PEX_CALLBACK_ROUTINE_BLOCK OldBlock
            );

        VOID
        (NTAPI * ExDereferenceCallBackBlock)(
            __inout PEX_CALLBACK CallBack,
            __in PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
            );

        KIRQL
        (NTAPI * ExAcquireSpinLockShared)(
            __inout PEX_SPIN_LOCK SpinLock
            );

        VOID
        (NTAPI * ExReleaseSpinLockShared)(
            __inout PEX_SPIN_LOCK SpinLock,
            __in KIRQL OldIrql
            );

        ULONG
        (NTAPI * DbgPrint)(
            __in PCH Format,
            ...
            );

        SIZE_T
        (NTAPI * RtlCompareMemory)(
            const VOID * Destination,
            const VOID * Source,
            SIZE_T Length
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
        (NTAPI * ExFreePoolWithTag)(
            __in PVOID P,
            __in ULONG Tag
            );

        VOID
        (NTAPI * KeBugCheckEx)(
            __in ULONG BugCheckCode,
            __in ULONG_PTR P1,
            __in ULONG_PTR P2,
            __in ULONG_PTR P3,
            __in ULONG_PTR P4
            );

        HANDLE ObjectCallback;

#define MAXIMUM_NOTIFY 64

#define PROCESS_NOTIFY 0
#define THREAD_NOTIFY 1
#define IMAGE_NOTIFY 2

        ULONG ProcessNotifyRoutineCount;
        EX_CALLBACK ProcessNotifyRoutine[MAXIMUM_NOTIFY];

        ULONG ThreadNotifyRoutineCount;
        EX_CALLBACK ThreadNotifyRoutine[MAXIMUM_NOTIFY];

        ULONG ImageNotifyRoutineCount;
        EX_CALLBACK ImageNotifyRoutine[MAXIMUM_NOTIFY];

        OB_PREOP_CALLBACK_STATUS
        (NTAPI * ObjectPreCallback)(
            __in PVOID RegistrationContext,
            __in POB_PRE_OPERATION_INFORMATION OperationInformation
            );

        VOID
        (NTAPI * ObjectPostCallback)(
            __in PVOID RegistrationContext,
            __in POB_POST_OPERATION_INFORMATION OperationInformation
            );

        KDDEBUGGER_DATA64 DebuggerDataBlock;
        KDDEBUGGER_DATA_ADDITION64 DebuggerDataAdditionBlock;

#ifndef _WIN64
        CHAR _CaptureContext[0x91];
#else
        CHAR _CaptureContext[0x152];
#endif // !_WIN64

        // struct _PGBLOCK PgBlock;
    } GPBLOCK, *PGPBLOCK;

    PIMAGE_SECTION_HEADER
        NTAPI
        SectionTableFromVirtualAddress(
            __in PVOID ImageBase,
            __in PVOID Address
        );

    VOID
        NTAPI
        InitializeGpBlock(
            __in PGPBLOCK Block
        );

    NTSTATUS
        NTAPI
        FindEntryForKernelImage(
            __in PUNICODE_STRING ImageFileName,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    NTSTATUS
        NTAPI
        FindEntryForKernelImageAddress(
            __in PVOID Address,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    extern PGPBLOCK GpBlock;

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_RELOAD_H_
