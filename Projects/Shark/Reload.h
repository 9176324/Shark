/*
*
* Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
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

#include <detoursdefs.h>
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

        PVOID NativeObject;

#ifdef _WIN64
        PVOID Wx86NativeObject;
#endif // _WIN64

        LIST_ENTRY LoadedPrivateImageList;
        LIST_ENTRY ObjectList;
        KSPIN_LOCK ObjectLock;

        CHAR NumberProcessors;
        CHAR Linkage[3];// { 0x33, 0xc0, 0xc3 };

        LONG BuildNumber;

        PBOOLEAN KdDebuggerEnabled;
        PBOOLEAN KdDebuggerNotPresent;
        PBOOLEAN KdEnteredDebugger;

        PBOOLEAN ShadowKdDebuggerEnabled;
        PBOOLEAN ShadowKdDebuggerNotPresent;
        PBOOLEAN ShadowKdEnteredDebugger;

        NTSTATUS
        (NTAPI * NtQuerySystemInformation)(
            __in SYSTEM_INFORMATION_CLASS SystemInformationClass,
            __out_bcount_opt(SystemInformationLength) PVOID SystemInformation,
            __in ULONG SystemInformationLength,
            __out_opt PULONG ReturnLength
            );

        VOID
        (NTAPI * KiDispatchException)(
            __in PEXCEPTION_RECORD ExceptionRecord,
            __in PKEXCEPTION_FRAME ExceptionFrame,
            __in PKTRAP_FRAME TrapFrame,
            __in KPROCESSOR_MODE PreviousMode,
            __in BOOLEAN FirstChance
            );

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

        PGUARD_OBJECT BugCheckHandle;

        VOID
        (NTAPI * KeBugCheckEx)(
            __in ULONG BugCheckCode,
            __in ULONG_PTR P1,
            __in ULONG_PTR P2,
            __in ULONG_PTR P3,
            __in ULONG_PTR P4
            );

        PLIST_ENTRY
        (FASTCALL * ExInterlockedRemoveHeadList)(
            __inout PLIST_ENTRY ListHead,
            __inout PKSPIN_LOCK Lock
            );

        HANDLE ObjectCallback;

        KDDEBUGGER_DATA64 DebuggerDataBlock;
        KDDEBUGGER_DATA_ADDITION64 DebuggerDataAdditionBlock;

        USHORT OffsetKProcessThreadListHead;
        USHORT OffsetKThreadThreadListEntry;
        USHORT OffsetKThreadWin32StartAddress;

#ifndef _WIN64
        CHAR _CaptureContext[0x100];
#else
        CHAR _CaptureContext[0x200];
#endif // !_WIN64

        // struct _PGBLOCK PgBlock;
    } GPBLOCK, *PGPBLOCK;

    NTKERNELAPI
        NTSTATUS
        NTAPI
        PsAcquireProcessExitSynchronization(
            __in PEPROCESS Process
        );

    NTKERNELAPI
        VOID
        NTAPI
        PsReleaseProcessExitSynchronization(
            __in PEPROCESS Process
        );

#define FastAcquireRundownProtection(ref) \
            GpBlock->ExAcquireRundownProtection((ref))

#define FastReleaseRundownProtection(ref) \
            GpBlock->ExReleaseRundownProtection((ref))

#define FastWaitForRundownProtectionRelease(ref) \
            GpBlock->ExWaitForRundownProtectionRelease((ref))

#define FastAcquireObjectLock(irql) \
            *(irql) = GpBlock->ExAcquireSpinLockShared(&GpBlock->ObjectLock)

#define FastReleaseObjectLock(irql) \
            GpBlock->ExReleaseSpinLockShared(&GpBlock->ObjectLock, (irql))

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
