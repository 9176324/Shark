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

#ifndef _RELOAD_H_
#define _RELOAD_H_

#include <guarddefs.h>
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
        ptr CpuControlBlock; // hypervisor

        ptr NativeObject;

#ifdef _WIN64
        ptr Wx86NativeObject;
#endif // _WIN64
        LIST_ENTRY LoadedPrivateImageList;
        LIST_ENTRY ObjectList;
        KSPIN_LOCK ObjectLock;

        s8 NumberProcessors;
        s8 Linkage[3];// { 0x33, 0xc0, 0xc3 };

        s32 BuildNumber;
        u32 Flags;
        u64 * PerfGlobalGroupMask;
        s8 KiSystemServiceCopyEnd[6];
        cptr PerfInfoLogSysCallEntry;
        ptr KeUserExceptionDispatcher;

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

        void
        (NTAPI * KiDispatchException)(
            __in PEXCEPTION_RECORD ExceptionRecord,
            __in PKEXCEPTION_FRAME ExceptionFrame,
            __in PKTRAP_FRAME TrapFrame,
            __in KPROCESSOR_MODE PreviousMode,
            __in b FirstChance
            );

        void
        (NTAPI * KeContextFromKframes)(
            __in PKTRAP_FRAME TrapFrame,
#if defined(_X86_)
            __in_opt PKEXCEPTION_FRAME ExceptionFrame,
#else
            __in PKEXCEPTION_FRAME ExceptionFrame,
#endif
            __inout PCONTEXT ContextFrame
            );

        void
        (NTAPI * KeContextToKframes)(
            __inout PKTRAP_FRAME TrapFrame,

#if defined(_X86_)
            __inout_opt PKEXCEPTION_FRAME ExceptionFrame,
#else
            __inout PKEXCEPTION_FRAME ExceptionFrame,
#endif
            __in PCONTEXT ContextFrame,
            __in u32 ContextFlags,
            __in KPROCESSOR_MODE PreviousMode
            );

#ifndef _WIN64
        status
        (NTAPI * DbgkpSendApiMessageLpc)(
            __inout ptr ApiMsg,
            __in ptr Port,
            __in b SuspendProcess
            );

        status
        (FASTCALL * FastDbgkpSendApiMessageLpc)(
            __inout ptr ApiMsg,
            __in ptr Port,
            __in b SuspendProcess
            );
#else
        status
        (FASTCALL * DbgkpSendApiMessageLpc)(
            __inout ptr ApiMsg,
            __in ptr Port,
            __in b SuspendProcess
            );
#endif // !_WIN64

        OB_PREOP_CALLBACK_STATUS
        (NTAPI * GlobalObjectPreCallback)(
            __in ptr RegistrationContext,
            __in POB_PRE_OPERATION_INFORMATION OperationInformation
            );

        void
        (NTAPI * GlobalObjectPostCallback)(
            __in ptr RegistrationContext,
            __in POB_POST_OPERATION_INFORMATION OperationInformation
            );

        void
        (NTAPI * GlobalProcessNotify)(
            __in ptr ParentId,
            __in ptr ProcessId,
            __in b Create
            );

        void
        (NTAPI * GlobalThreadNotify)(
            __in ptr ProcessId,
            __in ptr ThreadId,
            __in b Create
            );

        void
        (NTAPI * GlobalImageNotify)(
            __in PUNICODE_STRING FullImageName,
            __in ptr ProcessId,
            __in PIMAGE_INFO ImageInfo
            );

        void
        (NTAPI * KeEnterCriticalRegion)(
            void
            );

        void
        (NTAPI * KeLeaveCriticalRegion)(
            void
            );

        status
        (NTAPI * PspCreateThread)(
            __out ptr * ThreadHandle,
            __in ACCESS_MASK DesiredAccess,
            __in_opt POBJECT_ATTRIBUTES ObjectAttributes,
            __in ptr ProcessHandle,
            __in PEPROCESS ProcessPointer,
            __in_opt ptr Reserved,
            __in_opt PLARGE_INTEGER Cookie,
            __out_opt PCLIENT_ID ClientId,
            __in_opt PCONTEXT ThreadContext,
            __in_opt PINITIAL_TEB InitialTeb,
            __in b CreateSuspended,
            __in_opt PKSTART_ROUTINE StartRoutine,
            __in ptr StartContext
            );

        void
        (NTAPI * PspInitializeThunkContext)(
            void
            );

        b
        (FASTCALL * ExAcquireRundownProtection)(
            __inout PEX_RUNDOWN_REF RunRef
            );

        void
        (FASTCALL * ExReleaseRundownProtection)(
            __inout PEX_RUNDOWN_REF RunRef
            );

        void
        (FASTCALL * ExWaitForRundownProtectionRelease)(
            __inout PEX_RUNDOWN_REF RunRef
            );

        KIRQL
        (NTAPI * ExAcquireSpinLockShared)(
            __inout PEX_SPIN_LOCK SpinLock
            );

        void
        (NTAPI * ExReleaseSpinLockShared)(
            __inout PEX_SPIN_LOCK SpinLock,
            __in KIRQL OldIrql
            );

        u32
        (NTAPI * DbgPrint)(
            __in PCH Format,
            ...
            );

        u
        (NTAPI * RtlCompareMemory)(
            const void * Destination,
            const void * Source,
            u Length
            );

        void
        (NTAPI * RtlRestoreContext)(
            __in PCONTEXT ContextRecord,
            __in_opt struct _EXCEPTION_RECORD *ExceptionRecord
            );

        void
        (NTAPI * ExQueueWorkItem)(
            __inout PWORK_QUEUE_ITEM WorkItem,
            __in WORK_QUEUE_TYPE QueueType
            );

        void
        (NTAPI * ExFreePoolWithTag)(
            __in ptr P,
            __in u32 Tag
            );

        PSAFEGUARD_OBJECT BugCheckHandle;

        void
        (NTAPI * KeBugCheckEx)(
            __in u32 BugCheckCode,
            __in u P1,
            __in u P2,
            __in u P3,
            __in u P4
            );

        PLIST_ENTRY
        (FASTCALL * ExInterlockedRemoveHeadList)(
            __inout PLIST_ENTRY ListHead,
            __inout PKSPIN_LOCK Lock
            );

        ptr ObjectCallback;

        KDDEBUGGER_DATA64 DebuggerDataBlock;
        KDDEBUGGER_DATA_ADDITION64 DebuggerDataAdditionBlock;

        u16 OffsetKProcessThreadListHead;
        u16 OffsetKThreadThreadListEntry;
        u16 OffsetKThreadWin32StartAddress;
        u32 OffsetKThreadProcessId;

        struct _PGBLOCK * PgBlock;
    } GPBLOCK, *PGPBLOCK;

    NTKERNELAPI
        status
        NTAPI
        PsAcquireProcessExitSynchronization(
            __in PEPROCESS Process
        );

    NTKERNELAPI
        void
        NTAPI
        PsReleaseProcessExitSynchronization(
            __in PEPROCESS Process
        );

#define FastAcquireRundownProtection(ref) \
            GpBlock.ExAcquireRundownProtection((ref))

#define FastReleaseRundownProtection(ref) \
            GpBlock.ExReleaseRundownProtection((ref))

#define FastWaitForRundownProtectionRelease(ref) \
            GpBlock.ExWaitForRundownProtectionRelease((ref))

#define FastAcquireObjectLock(irql) \
            *(irql) = GpBlock.ExAcquireSpinLockShared(&GpBlock.ObjectLock)

#define FastReleaseObjectLock(irql) \
            GpBlock.ExReleaseSpinLockShared(&GpBlock.ObjectLock, (irql))

    VOID
        NTAPI
        InitializeGpBlock(
            __in PGPBLOCK Block
        );

    ULONG
        NTAPI
        GetPlatform(
            __in PVOID ImageBase
        );

    ULONG
        NTAPI
        GetTimeStamp(
            __in PVOID ImageBase
        );

    USHORT
        NTAPI
        GetSubsystem(
            __in PVOID ImageBase
        );

    ULONG
        NTAPI
        GetSizeOfImage(
            __in PVOID ImageBase
        );

    PVOID
        NTAPI
        GetAddressOfEntryPoint(
            __in PVOID ImageBase
        );

    PIMAGE_SECTION_HEADER
        NTAPI
        SectionTableFromVirtualAddress(
            __in PVOID ImageBase,
            __in PVOID Address
        );

    PIMAGE_SECTION_HEADER
        NTAPI
        FindSection(
            __in PVOID ImageBase,
            __in PCSTR SectionName
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
