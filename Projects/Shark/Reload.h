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

#ifndef _RELOAD_H_
#define _RELOAD_H_

#include <devicedefs.h>
#include <dump.h>

#include "Space.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#define BinDirectory "H:\\Labs\\Sefirot\\Build\\Bins\\"

#define SystemDirectory L"\\SystemRoot\\System32\\"
#define Wow64SystemDirectory L"\\SystemRoot\\SysWOW64\\"

    typedef struct _RELOADER_PARAMETER_BLOCK {
        KDDEBUGGER_DATA64 DebuggerDataBlock;
        KDDEBUGGER_DATA_ADDITION64 DebuggerDataAdditionBlock;

        PKLDR_DATA_TABLE_ENTRY CoreDataTableEntry;

        PVOID PrivateHeader;

        PVOID CpuControlBlock;
        PKLDR_DATA_TABLE_ENTRY RootDataTableEntry;
        BOOLEAN DeployRoot;

        ULONG BuildNumber;
        CCHAR NumberProcessors;

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

        LIST_ENTRY LoadedPrivateImageList;

        KSERVICE_TABLE_DESCRIPTOR * ServiceDescriptorTable;
        KSERVICE_TABLE_DESCRIPTOR * ServiceDescriptorTableShadow;

        USHORT OffsetKThreadTrapFrame;

        NTSTATUS
        (NTAPI * PsCreateThread)(
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

        BOOLEAN DeployPatchGuard;

        // PATCHGUARD_BLOCK PatchGuardBlock;
    } RELOADER_PARAMETER_BLOCK, *PRELOADER_PARAMETER_BLOCK;

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

    VOID
        NTAPI
        RelocateImage(
            __in PVOID ImageBase,
            __in LONG_PTR Diff
        );

    VOID
        NTAPI
        InitializeLoadedModuleList(
            __in PRELOADER_PARAMETER_BLOCK Block
        );

    NTSTATUS
        NTAPI
        FindEntryForKernelPrivateImage(
            __in PUNICODE_STRING ImageFileName,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    NTSTATUS
        NTAPI
        FindEntryForKernelImage(
            __in PUNICODE_STRING ImageFileName,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    NTSTATUS
        NTAPI
        FindEntryForKernelPrivateImageAddress(
            __in PVOID Address,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    NTSTATUS
        NTAPI
        FindEntryForKernelImageAddress(
            __in PVOID Address,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    PVOID
        NTAPI
        GetKernelProcedureAddress(
            __in PVOID ImageBase,
            __in_opt PSTR ProcedureName,
            __in_opt ULONG ProcedureNumber
        );

    PVOID
        NTAPI
        NameToAddress(
            __in PSTR String
        );

    PKLDR_DATA_TABLE_ENTRY
        NTAPI
        LoadKernelPrivateImage(
            __in PVOID ViewBase,
            __in PCWSTR ImageName,
            __in BOOLEAN Insert
        );

    VOID
        NTAPI
        UnloadKernelPrivateImage(
            __in PKLDR_DATA_TABLE_ENTRY DataTableEntry
        );

    extern PRELOADER_PARAMETER_BLOCK ReloaderBlock;

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_RELOAD_H_
