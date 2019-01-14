/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Public License Version
* 2.0 (the "License")); you may not use this file except in compliance with
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

#include <defs.h>

#include "Stack.h"

#include "Reload.h"

#ifndef _WIN64
DECLSPEC_NOINLINE
ULONG
NTAPI
WalkFrameChain(
    __out PCALLERS Callers,
    __in ULONG Count
)
{
    ULONG Fp = 0;
    ULONG Index = 0;

    _asm mov Fp, ebp;

    while (Index < Count &&
        MmIsAddressValid((PVOID)Fp)) {
        Callers[Index].Establisher = (PVOID)(*(PULONG)(Fp + 4));
        Callers[Index].EstablisherFrame = (PVOID *)(Fp + 8);

        Index += 1;
        Fp = *(PULONG)Fp;
    }

    return Index;
}
#else
DECLSPEC_NOINLINE
ULONG
NTAPI
WalkFrameChain(
    __out PCALLERS Callers,
    __in ULONG Count
)
{
    CONTEXT ContextRecord = { 0 };
    PVOID HandlerData = NULL;
    ULONG Index = 0;
    PRUNTIME_FUNCTION FunctionEntry = NULL;
    ULONG64 ImageBase = 0;
    ULONG64 EstablisherFrame = 0;

    RtlCaptureContext(&ContextRecord);

    while (Index < Count &&
        MmIsAddressValid((PVOID)ContextRecord.Rip)) {
        FunctionEntry = RtlLookupFunctionEntry(
            ContextRecord.Rip,
            &ImageBase,
            NULL);

        if (NULL != FunctionEntry) {
            RtlVirtualUnwind(
                UNW_FLAG_NHANDLER,
                ImageBase,
                ContextRecord.Rip,
                FunctionEntry,
                &ContextRecord,
                &HandlerData,
                &EstablisherFrame,
                NULL);

            Callers[Index].Establisher = (PVOID)ContextRecord.Rip;
            Callers[Index].EstablisherFrame = (PVOID *)EstablisherFrame;

            Index += 1;
        }
        else {
            break;
        }
    }

    return Index;
}
#endif // !_WIN64

PSTR
NTAPI
FindSymbol(
    __in PVOID Address,
    __out_opt PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory = NULL;
    ULONG Size = 0;
    PULONG NameTable = NULL;
    PUSHORT OrdinalTable = NULL;
    PULONG AddressTable = NULL;
    USHORT HintIndex = 0;
    PVOID ProcedureAddress = NULL;
    PVOID NearAddress = NULL;
    PSTR ProcedureName = NULL;

    Status = FindEntryForKernelImageAddress(
        Address,
        &FoundDataTableEntry);

    if (NT_SUCCESS(Status)) {
        ExportDirectory = RtlImageDirectoryEntryToData(
            FoundDataTableEntry->DllBase,
            TRUE,
            IMAGE_DIRECTORY_ENTRY_EXPORT,
            &Size);

        if (NULL != ExportDirectory) {
            NameTable = (PCHAR)FoundDataTableEntry->DllBase + ExportDirectory->AddressOfNames;
            OrdinalTable = (PCHAR)FoundDataTableEntry->DllBase + ExportDirectory->AddressOfNameOrdinals;
            AddressTable = (PCHAR)FoundDataTableEntry->DllBase + ExportDirectory->AddressOfFunctions;

            if (NULL != NameTable &&
                NULL != OrdinalTable &&
                NULL != AddressTable) {
                for (HintIndex = 0;
                    HintIndex < ExportDirectory->NumberOfNames;
                    HintIndex++) {
                    ProcedureAddress = (PCHAR)FoundDataTableEntry->DllBase + AddressTable[OrdinalTable[HintIndex]];

                    if ((ULONG_PTR)ProcedureAddress <=
                        (ULONG_PTR)Address) {
                        if (NULL == NearAddress) {
                            NearAddress = ProcedureAddress;
                            ProcedureName = (PCHAR)FoundDataTableEntry->DllBase + NameTable[HintIndex];
                        }
                        else if ((ULONG_PTR)ProcedureAddress >(ULONG_PTR)NearAddress) {
                            NearAddress = ProcedureAddress;
                            ProcedureName = (PCHAR)FoundDataTableEntry->DllBase + NameTable[HintIndex];
                        }
                    }
                }
            }
        }

        if (NULL != DataTableEntry) {
            *DataTableEntry = FoundDataTableEntry;
        }
    }
    else {
        if (NULL != DataTableEntry) {
            *DataTableEntry = NULL;
        }
    }

    return ProcedureName;
}

VOID
NTAPI
PrintSymbol(
    __in PVOID Address
)
{
    PKLDR_DATA_TABLE_ENTRY DataTableEnry = NULL;
    PVOID ProcedureAddress = NULL;
    PSTR ProcedureName = NULL;

    ProcedureName = FindSymbol(
        (PVOID)Address,
        &DataTableEnry);

    if (NULL != ProcedureName) {
        ProcedureAddress = GetKernelProcedureAddress(
            DataTableEnry->DllBase,
            ProcedureName,
            0);

        if (0 == (ULONG64)Address - (ULONG64)ProcedureAddress) {
#ifndef PUBLIC
            DbgPrint(
                "[Shark] < %p > < %wZ!%hs >\n",
                Address,
                &DataTableEnry->BaseDllName,
                ProcedureName);
#endif // !PUBLIC
        }
        else {
#ifndef PUBLIC
            DbgPrint(
                "[Shark] < %p > < %wZ!%hs + 0x%x >\n",
                Address,
                &DataTableEnry->BaseDllName,
                ProcedureName,
                (ULONG64)Address - (ULONG64)ProcedureAddress);
#endif // !PUBLIC
        }
    }
    else if (NULL != DataTableEnry) {
#ifndef PUBLIC
        DbgPrint(
            "[Shark] < %p > < %wZ!%s + 0x%x >\n",
            Address,
            &DataTableEnry->BaseDllName,
            (ULONG64)Address - (ULONG64)DataTableEnry->DllBase);
#endif // !PUBLIC
    }
    else {
#ifndef PUBLIC
        DbgPrint(
            "[Shark] < %p >\n",
            Address);
#endif // !PUBLIC
    }
}

VOID
NTAPI
PrintFrameChain(
    __in PCALLERS Callers,
    __in_opt ULONG FramesToSkip,
    __in ULONG Count
)
{
    ULONG Index = 0;

    for (Index = FramesToSkip;
        Index < Count;
        Index++) {
        PrintSymbol(Callers[Index].Establisher);
    }
}
