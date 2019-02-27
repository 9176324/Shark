/*
*
* Copyright (c) 2019 by blindtiger. All rights reserved.
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

VOID
NTAPI
PrintSymbol(
    __in PCSTR Prefix,
    __in PSYMBOL Symbol
)
{
    if (NULL != Symbol->String) {
        if (0 == Symbol->Offset) {
#ifndef PUBLIC
            DbgPrint(
                "%s< %p > %wZ!%hs\n",
                Prefix,
                Symbol->Address,
                &Symbol->DataTableEntry->BaseDllName,
                Symbol->String);
#endif // !PUBLIC
        }
        else {
#ifndef PUBLIC
            DbgPrint(
                "%s< %p > %wZ!%hs + %x\n",
                Prefix,
                Symbol->Address,
                &Symbol->DataTableEntry->BaseDllName,
                Symbol->String,
                Symbol->Offset);
#endif // !PUBLIC
        }
    }
    else if (0 != Symbol->Ordinal) {
        if (0 == Symbol->Offset) {
#ifndef PUBLIC
            DbgPrint(
                "%s< %p > %wZ!@%d\n",
                Prefix,
                Symbol->Address,
                &Symbol->DataTableEntry->BaseDllName,
                Symbol->Ordinal);
#endif // !PUBLIC
        }
        else {
#ifndef PUBLIC
            DbgPrint(
                "%s< %p > %wZ!@%d + %x\n",
                Prefix,
                Symbol->Address,
                &Symbol->DataTableEntry->BaseDllName,
                Symbol->Ordinal,
                Symbol->Offset);
#endif // !PUBLIC
        }
    }
    else if (NULL != Symbol->DataTableEntry) {
#ifndef PUBLIC
        DbgPrint(
            "%s< %p > %wZ + %x\n",
            Prefix,
            Symbol->Address,
            &Symbol->DataTableEntry->BaseDllName,
            Symbol->Offset);
#endif // !PUBLIC
    }
    else {
#ifndef PUBLIC
        DbgPrint(
            "%s< %p > symbol not found\n",
            Prefix,
            Symbol->Address);
#endif // !PUBLIC
    }
}

VOID
NTAPI
WalkImageSymbol(
    __in PVOID Address,
    __inout PSYMBOL Symbol
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory = NULL;
    ULONG Size = 0;
    PULONG NameTable = NULL;
    PUSHORT OrdinalTable = NULL;
    PULONG AddressTable = NULL;
    PSTR NameTableName = NULL;
    USHORT HintIndex = 0;
    USHORT NameIndex = 0;
    PVOID ProcedureAddress = NULL;
    PVOID NearAddress = NULL;

    Symbol->Address = Address;

    Symbol->Offset =
        (ULONG_PTR)Address - (ULONG_PTR)Symbol->DataTableEntry->DllBase;

    ExportDirectory = RtlImageDirectoryEntryToData(
        Symbol->DataTableEntry->DllBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &Size);

    if (NULL != ExportDirectory) {
        NameTable =
            (PCHAR)Symbol->DataTableEntry->DllBase + ExportDirectory->AddressOfNames;

        OrdinalTable =
            (PCHAR)Symbol->DataTableEntry->DllBase + ExportDirectory->AddressOfNameOrdinals;

        AddressTable =
            (PCHAR)Symbol->DataTableEntry->DllBase + ExportDirectory->AddressOfFunctions;

        if (NULL != NameTable &&
            NULL != OrdinalTable &&
            NULL != AddressTable) {
            for (HintIndex = 0;
                HintIndex < ExportDirectory->NumberOfFunctions;
                HintIndex++) {
                ProcedureAddress =
                    (PCHAR)Symbol->DataTableEntry->DllBase + AddressTable[HintIndex];

                if ((ULONG_PTR)ProcedureAddress <= (ULONG_PTR)Symbol->Address &&
                    (ULONG_PTR)ProcedureAddress > (ULONG_PTR)NearAddress) {
                    NearAddress = ProcedureAddress;

                    for (NameIndex = 0;
                        NameIndex < ExportDirectory->NumberOfNames;
                        NameIndex++) {
                        if (HintIndex == OrdinalTable[NameIndex]) {
                            Symbol->String =
                                (PCHAR)Symbol->DataTableEntry->DllBase + NameTable[HintIndex];
                        }
                    }

                    Symbol->Ordinal =
                        HintIndex + ExportDirectory->Base;

                    Symbol->Offset =
                        (ULONG_PTR)Symbol->Address - (ULONG_PTR)ProcedureAddress;
                }
            }
        }
    }
}

VOID
NTAPI
FindSymbol(
    __in PVOID Address,
    __inout PSYMBOL Symbol
)
{
    if (NT_SUCCESS(FindEntryForKernelImageAddress(
        Address,
        &Symbol->DataTableEntry))) {
        WalkImageSymbol(Address, Symbol);
    }
}

VOID
NTAPI
FindAndPrintSymbol(
    __in PCSTR Prefix,
    __in PVOID Address
)
{
    SYMBOL Symbol = { 0 };

    FindSymbol(Address, &Symbol);
    PrintSymbol(Prefix, &Symbol);
}

VOID
NTAPI
PrintFrameChain(
    __in PCSTR Prefix,
    __in PCALLERS Callers,
    __in_opt ULONG FramesToSkip,
    __in ULONG Count
)
{
    ULONG Index = 0;

    for (Index = FramesToSkip;
        Index < Count;
        Index++) {
        FindAndPrintSymbol(
            Prefix,
            Callers[Index].Establisher);
    }
}
