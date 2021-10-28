/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
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
* The Initial Developer of the Original Code is blindtiger.
*
*/

#include <defs.h>

#include "Stack.h"

#include "Reload.h"

void
NTAPI
PrintSymbol(
    __in u8ptr Prefix,
    __in PSYMBOL Symbol
)
{
    if (NULL != Symbol->String) {
        if (0 == Symbol->Offset) {
#ifdef DEBUG
            vDbgPrint(
                "%s < %p > %wZ!%hs\n",
                Prefix,
                Symbol->Address,
                &Symbol->DataTableEntry->BaseDllName,
                Symbol->String);
#endif // DEBUG
        }
        else {
#ifdef DEBUG
            vDbgPrint(
                "%s < %p > %wZ!%hs + %x\n",
                Prefix,
                Symbol->Address,
                &Symbol->DataTableEntry->BaseDllName,
                Symbol->String,
                Symbol->Offset);
#endif // DEBUG
        }
    }
    else if (0 != Symbol->Ordinal) {
        if (0 == Symbol->Offset) {
#ifdef DEBUG
            vDbgPrint(
                "%s < %p > %wZ!@%d\n",
                Prefix,
                Symbol->Address,
                &Symbol->DataTableEntry->BaseDllName,
                Symbol->Ordinal);
#endif // DEBUG
        }
        else {
#ifdef DEBUG
            vDbgPrint(
                "%s < %p > %wZ!@%d + %x\n",
                Prefix,
                Symbol->Address,
                &Symbol->DataTableEntry->BaseDllName,
                Symbol->Ordinal,
                Symbol->Offset);
#endif // DEBUG
        }
    }
    else if (NULL != Symbol->DataTableEntry) {
#ifdef DEBUG
        vDbgPrint(
            "%s < %p > %wZ + %x\n",
            Prefix,
            Symbol->Address,
            &Symbol->DataTableEntry->BaseDllName,
            Symbol->Offset);
#endif // DEBUG
    }
    else {
#ifdef DEBUG
        vDbgPrint(
            "%s < %p > symbol not found\n",
            Prefix,
            Symbol->Address);
#endif // DEBUG
    }
}

void
NTAPI
WalkImageSymbol(
    __in ptr Address,
    __inout PSYMBOL Symbol
)
{
    status Status = STATUS_SUCCESS;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory = NULL;
    u32 Size = 0;
    u32ptr NameTable = NULL;
    u16ptr OrdinalTable = NULL;
    u32ptr AddressTable = NULL;
    cptr NameTableName = NULL;
    u16 HintIndex = 0;
    u16 NameIndex = 0;
    ptr ProcedureAddress = NULL;
    ptr NearAddress = NULL;

    Symbol->Address = Address;

    Symbol->Offset =
        (u)Address - (u)Symbol->DataTableEntry->DllBase;

    ExportDirectory = RtlImageDirectoryEntryToData(
        Symbol->DataTableEntry->DllBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &Size);

    if (NULL != ExportDirectory) {
        NameTable =
            (u8ptr)Symbol->DataTableEntry->DllBase + ExportDirectory->AddressOfNames;

        OrdinalTable =
            (u8ptr)Symbol->DataTableEntry->DllBase + ExportDirectory->AddressOfNameOrdinals;

        AddressTable =
            (u8ptr)Symbol->DataTableEntry->DllBase + ExportDirectory->AddressOfFunctions;

        if (NULL != NameTable &&
            NULL != OrdinalTable &&
            NULL != AddressTable) {
            for (HintIndex = 0;
                HintIndex < ExportDirectory->NumberOfFunctions;
                HintIndex++) {
                ProcedureAddress =
                    (u8ptr)Symbol->DataTableEntry->DllBase + AddressTable[HintIndex];

                if ((u)ProcedureAddress <= (u)Symbol->Address &&
                    (u)ProcedureAddress > (u)NearAddress) {
                    NearAddress = ProcedureAddress;

                    for (NameIndex = 0;
                        NameIndex < ExportDirectory->NumberOfNames;
                        NameIndex++) {
                        if (HintIndex == OrdinalTable[NameIndex]) {
                            Symbol->String =
                                (u8ptr)Symbol->DataTableEntry->DllBase + NameTable[HintIndex];
                        }
                    }

                    Symbol->Ordinal =
                        HintIndex + ExportDirectory->Base;

                    Symbol->Offset =
                        (u)Symbol->Address - (u)ProcedureAddress;
                }
            }
        }
    }
}

void
NTAPI
FindSymbol(
    __in ptr Address,
    __inout PSYMBOL Symbol
)
{
    status Status = STATUS_SUCCESS;

    Status = FindEntryForImageAddress(
        Address,
        &Symbol->DataTableEntry);

    if (NT_SUCCESS(Status)) {
        WalkImageSymbol(Address, Symbol);
    }
}

void
NTAPI
FindAndPrintSymbol(
    __in u8ptr Prefix,
    __in ptr Address
)
{
    SYMBOL Symbol = { 0 };

    FindSymbol(Address, &Symbol);
    PrintSymbol(Prefix, &Symbol);
}

void
NTAPI
PrintFrameChain(
    __in u8ptr Prefix,
    __in PCALLERS Callers,
    __in_opt u32 FramesToSkip,
    __in u32 Count
)
{
    u32 Index = 0;

    for (Index = FramesToSkip;
        Index < Count;
        Index++) {
        FindAndPrintSymbol(
            Prefix,
            Callers[Index].Establisher);
    }
}
