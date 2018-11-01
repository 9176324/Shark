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

#ifndef _KERNELRELOAD_H_
#define _KERNELRELOAD_H_

#include "..\..\WRK\base\ntos\mm\mi.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#define SystemDirectory L"\\SystemRoot\\System32"

    VOID
        NTAPI
        InitializeLoadedModuleList(
            __in PKLDR_DATA_TABLE_ENTRY DataTableEntry
        );

    NTSTATUS
        NTAPI
        FindEntryForPrivateSystemImage(
            __in PUNICODE_STRING ImageFileName,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    NTSTATUS
        NTAPI
        FindEntryForSystemImage(
            __in PUNICODE_STRING ImageFileName,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    NTSTATUS
        NTAPI
        FindEntryForPrivateSystemImageAddress(
            __in PVOID Address,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    NTSTATUS
        NTAPI
        FindEntryForSystemImageAddress(
            __in PVOID Address,
            __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
        );

    ULONG
        NTAPI
        GetPlatform(
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

    PVOID
        NTAPI
        GetProcedureAddress(
            __in PVOID ImageBase,
            __in_opt PSTR ProcedureName,
            __in_opt ULONG ProcedureNumber
        );

    PKLDR_DATA_TABLE_ENTRY
        NTAPI
        LoadPrivateSystemImage(
            __in PVOID ViewBase,
            __in PCWSTR ImageName,
            __in BOOLEAN Insert
        );

    VOID
        NTAPI
        UnloadPrivateSystemImage(
            __in PKLDR_DATA_TABLE_ENTRY DataTableEntry
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_KERNELRELOAD_H_
