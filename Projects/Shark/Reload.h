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

#include <devicedefs.h>
#include <guarddefs.h>
#include <dump.h>

#include "Space.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef s32 EX_SPIN_LOCK, *PEX_SPIN_LOCK;

    typedef enum _OB_PREOP_CALLBACK_STATUS OB_PREOP_CALLBACK_STATUS;
    typedef struct _OB_PRE_OPERATION_INFORMATION *POB_PRE_OPERATION_INFORMATION;
    typedef struct _OB_POST_OPERATION_INFORMATION *POB_POST_OPERATION_INFORMATION;

    // runtime state block
    typedef struct _RTB {
        PLIST_ENTRY PsLoadedModuleList;
        PEPROCESS PsInitialSystemProcess;
        PERESOURCE PsLoadedModuleResource;
        struct _FUNCTION_TABLE * PsInvertedFunctionTable;
        KSERVICE_TABLE_DESCRIPTOR * KeServiceDescriptorTable;
        KSERVICE_TABLE_DESCRIPTOR * KeServiceDescriptorTableShadow;
        PKLDR_DATA_TABLE_ENTRY KernelDataTableEntry; // ntoskrnl.exe

        union {
            SUPDRVLDRIMAGE SupImage;

            struct {
                KLDR_DATA_TABLE_ENTRY DataTableEntry;
                wc FullDllName[MAXIMUM_FILENAME_LENGTH];
                wc BaseDllName[MAXIMUM_FILENAME_LENGTH];
            };
        }*Self;

        ptr CpuControlBlock; // hypervisor

        ptr NativeObject;

#ifdef _WIN64
        ptr Wx86NativeObject;
#endif // _WIN64

        LIST_ENTRY LoadedModuleList;

        LIST_ENTRY Object;
        KSPIN_LOCK Lock;

        s8 NumberProcessors;
        s8 Linkage[3];// { 0x33, 0xc0, 0xc3 };

        u32 BuildNumber;
        u32 Operation;
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

        status
        (NTAPI * ProcessOpen)(
            __in OB_OPEN_REASON OpenReason,
            __in KPROCESSOR_MODE PreviousMode,
            __in_opt PEPROCESS Process,
            __in ptr ProcessObject,
            __in ACCESS_MASK * GrantedAccess,
            __in u32 HandleCount
            );

        void
        (NTAPI * ProcessDelete)(
            __in ptr ProcessObject
            );

        status
        (NTAPI * ThreadOpen)(
            __in OB_OPEN_REASON OpenReason,
            __in KPROCESSOR_MODE PreviousMode,
            __in_opt PEPROCESS Process,
            __in ptr ThreadObject,
            __in ACCESS_MASK * GrantedAccess,
            __in u32 HandleCount
            );

        void
        (NTAPI * ThreadDelete)(
            __in ptr ThreadObject
            );

        status
        (NTAPI * FileOpen)(
            __in OB_OPEN_REASON OpenReason,
            __in KPROCESSOR_MODE PreviousMode,
            __in_opt PEPROCESS Process,
            __in ptr FileObject,
            __in ACCESS_MASK * GrantedAccess,
            __in u32 HandleCount
            );

        status
        (NTAPI * FileParse)(
            __in ptr ParseObject,
            __in ptr ObjectType,
            __in PACCESS_STATE AccessState,
            __in KPROCESSOR_MODE AccessMode,
            __in u32 Attributes,
            __inout PUNICODE_STRING CompleteName,
            __inout PUNICODE_STRING RemainingName,
            __inout_opt ptr Context,
            __in_opt PSECURITY_QUALITY_OF_SERVICE SecurityQos,
            __out ptr * FileObject
            );

        void
        (NTAPI * FileDelete)(
            __in ptr FileObject
            );

        status
        (NTAPI * DriverOpen)(
            __in OB_OPEN_REASON OpenReason,
            __in KPROCESSOR_MODE PreviousMode,
            __in_opt PEPROCESS Process,
            __in ptr DriverObject,
            __in ACCESS_MASK * GrantedAccess,
            __in u32 HandleCount
            );

        void
        (NTAPI * DriverDelete)(
            __in ptr DriverObject
            );

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

        NTSTATUS
        (NTAPI *  KeWaitForSingleObject)(
            __in PVOID Object,
            __in KWAIT_REASON WaitReason,
            __in KPROCESSOR_MODE WaitMode,
            __in BOOLEAN Alertable,
            __in_opt PLARGE_INTEGER Timeout
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

        u32 NameInterval;

        u16 OffsetKProcessThreadListHead;
        u16 OffsetKThreadThreadListEntry;
        u16 OffsetKThreadWin32StartAddress;
        u32 OffsetKThreadProcessId;
        u16 OffsetEProcessActiveProcessLinks;

        struct _PGBLOCK * PgBlock;
    } RTB, *PRTB;

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
            RtBlock.ExAcquireRundownProtection((ref))

#define FastReleaseRundownProtection(ref) \
            RtBlock.ExReleaseRundownProtection((ref))

#define FastWaitForRundownProtectionRelease(ref) \
            RtBlock.ExWaitForRundownProtectionRelease((ref))

#define FastAcquireObjectLock(irql) \
            *(irql) = RtBlock.ExAcquireSpinLockShared(&RtBlock.Lock)

#define FastReleaseObjectLock(irql) \
            RtBlock.ExReleaseSpinLockShared(&RtBlock.Lock, (irql))

    void
        NTAPI
        InitializeGpBlock(
            __in PRTB Block
        );

    u32
        NTAPI
        LdrMakeProtection(
            __in PIMAGE_SECTION_HEADER NtSection
        );

    u32
        NTAPI
        LdrGetPlatform(
            __in ptr ImageBase
        );

    u32
        NTAPI
        LdrGetTimeStamp(
            __in ptr ImageBase
        );

    u16
        NTAPI
        LdrGetSubsystem(
            __in ptr ImageBase
        );

    u32
        NTAPI
        LdrGetSize(
            __in ptr ImageBase
        );

    ptr
        NTAPI
        LdrGetEntryPoint(
            __in ptr ImageBase
        );

    PIMAGE_SECTION_HEADER
        NTAPI
        SectionTableFromVirtualAddress(
            __in ptr ImageBase,
            __in ptr Address
        );

    PIMAGE_SECTION_HEADER
        NTAPI
        LdrFindSection(
            __in ptr ImageBase,
            __in u8ptr SectionName
        );

    void
        NTAPI
        LdrRelocImage(
            __in ptr ImageBase,
            __in s Diff
        );

    status
        NTAPI
        FindEntryForImage(
            __in PUNICODE_STRING ImageFileName,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    status
        NTAPI
        FindEntryForImageAddress(
            __in ptr Address,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    ptr
        NTAPI
        LdrGetSymbol(
            __in ptr ImageBase,
            __in_opt cptr ProcedureName,
            __in_opt u32 ProcedureNumber
        );

    ptr
        NTAPI
        LdrNameToAddress(
            __in cptr String
        );

    void
        NTAPI
        LdrSnapThunk(
            __in ptr ImageBase
        );

    void
        NTAPI
        LdrEnumerateThunk(
            __in ptr ImageBase
        );

    void
        NTAPI
        LdrReplaceThunk(
            __in ptr ImageBase,
            __in_opt cptr ImageName,
            __in_opt cptr ProcedureName,
            __in_opt u32 ProcedureNumber,
            __in ptr ProcedureAddress
        );

    s
        NTAPI
        LdrSetImageBase(
            __in ptr ImageBase
        );

    PKLDR_DATA_TABLE_ENTRY
        NTAPI
        LdrLoad(
            __in ptr ViewBase,
            __in wcptr ImageName,
            __in u32 Flags
        );

    void
        NTAPI
        LdrUnload(
            __in PKLDR_DATA_TABLE_ENTRY DataTableEntry
        );

    void
        NTAPI
        DumpImageWorker(
            __in ptr ImageBase,
            __in u32 SizeOfImage,
            __in PUNICODE_STRING ImageFIleName
        );

    status
        NTAPI
        DumpFileWorker(
            __in PUNICODE_STRING ImageFIleName
        );

    extern RTB RtBlock;

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_RELOAD_H_
