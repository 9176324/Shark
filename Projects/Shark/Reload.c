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

#include "Reload.h"

#include "Except.h"

extern POBJECT_TYPE * IoDriverObjectType;

static PLIST_ENTRY LoadedModuleList;
static LIST_ENTRY LoadedPrivateImageList;

ULONG
NTAPI
GetPlatform(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG Platform = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        Platform = NtHeaders->OptionalHeader.Magic;
    }

    return Platform;
}

PVOID
NTAPI
GetAddressOfEntryPoint(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG Offset = 0;
    PVOID EntryPoint = NULL;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Offset = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.AddressOfEntryPoint;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Offset = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.AddressOfEntryPoint;
        }

        if (0 != Offset) {
            EntryPoint = (PCHAR)ImageBase + Offset;
        }
    }

    return EntryPoint;
}

ULONG
NTAPI
GetTimeStamp(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG TimeStamp = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        TimeStamp = NtHeaders->FileHeader.TimeDateStamp;
    }

    return TimeStamp;
}

USHORT
NTAPI
GetSubsystem(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    USHORT Subsystem = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Subsystem = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.Subsystem;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Subsystem = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.Subsystem;
        }
    }

    return Subsystem;
}

ULONG
NTAPI
GetSizeOfImage(
    __in PVOID ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG SizeOfImage = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            SizeOfImage = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.SizeOfImage;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            SizeOfImage = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.SizeOfImage;
        }
    }

    return SizeOfImage;
}

PIMAGE_SECTION_HEADER
NTAPI
SectionTableFromVirtualAddress(
    __in PVOID ImageBase,
    __in PVOID Address
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    ULONG Index = 0;
    ULONG Offset = 0;
    PIMAGE_SECTION_HEADER FountSection = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    ULONG SizeToLock = 0;

    NtHeaders = RtlImageNtHeader(ImageBase);

    if (NULL != NtHeaders) {
        FountSection = IMAGE_FIRST_SECTION(NtHeaders);
        Offset = (ULONG)((ULONG_PTR)Address - (ULONG_PTR)ImageBase);

        for (Index = 0;
            Index < NtHeaders->FileHeader.NumberOfSections;
            Index++) {
            SizeToLock = max(
                FountSection[Index].SizeOfRawData,
                FountSection[Index].Misc.VirtualSize);

            if (Offset >= FountSection[Index].VirtualAddress &&
                Offset < FountSection[Index].VirtualAddress + SizeToLock) {
                NtSection = &FountSection[Index];
                break;
            }
        }
    }

    return NtSection;
}

FORCEINLINE
ULONG
NTAPI
GetRelocCount(
    __in ULONG SizeOfBlock
)
{
    ULONG Count = 0;

    Count = (SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(USHORT);

    return Count;
}

PIMAGE_BASE_RELOCATION
NTAPI
RelocationBlock(
    __in PVOID VA,
    __in ULONG Count,
    __in PUSHORT NextOffset,
    __in LONG_PTR Diff
)
{
    PUSHORT FixupVA = NULL;
    USHORT Offset = 0;
    USHORT Type = 0;

    while (Count--) {
        Offset = *NextOffset & 0xfff;
        FixupVA = (PCHAR)VA + Offset;
        Type = (*NextOffset >> 12) & 0xf;

        switch (Type) {
        case IMAGE_REL_BASED_ABSOLUTE: {
            break;
        }

        case IMAGE_REL_BASED_HIGH: {
            FixupVA[1] += (USHORT)((Diff >> 16) & 0xffff);
            break;
        }

        case IMAGE_REL_BASED_LOW: {
            FixupVA[0] += (USHORT)(Diff & 0xffff);
            break;
        }

        case IMAGE_REL_BASED_HIGHLOW: {
            *(PULONG)FixupVA += (ULONG)Diff;
            break;
        }

        case IMAGE_REL_BASED_HIGHADJ: {
            FixupVA[0] += NextOffset[1] & 0xffff;
            FixupVA[1] += (USHORT)((Diff >> 16) & 0xffff);

            ++NextOffset;
            --Count;
            break;
        }

        case IMAGE_REL_BASED_MIPS_JMPADDR:
        case IMAGE_REL_BASED_SECTION:
        case IMAGE_REL_BASED_REL32:
            // case IMAGE_REL_BASED_VXD_RELATIVE:
            // case IMAGE_REL_BASED_MIPS_JMPADDR16: 

        case IMAGE_REL_BASED_IA64_IMM64: {
            break;
        }

        case IMAGE_REL_BASED_DIR64: {
            *(PULONG_PTR)FixupVA += Diff;
            break;
        }

        default: {
            return NULL;
        }
        }

        ++NextOffset;
    }

    return (PIMAGE_BASE_RELOCATION)NextOffset;
}

VOID
NTAPI
RelocateImage(
    __in PVOID ImageBase,
    __in LONG_PTR Diff
)
{
    PIMAGE_BASE_RELOCATION RelocDirectory = NULL;
    ULONG Size = 0;
    PVOID VA = 0;

    RelocDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_BASERELOC,
        &Size);

    if (0 != Size) {
        if (0 != Diff) {
            while (0 != Size) {
                VA = (PCHAR)ImageBase + RelocDirectory->VirtualAddress;
                Size -= RelocDirectory->SizeOfBlock;

                RelocDirectory = RelocationBlock(
                    VA,
                    GetRelocCount(RelocDirectory->SizeOfBlock),
                    (PUSHORT)(RelocDirectory + 1),
                    Diff);
            }
        }
    }
}

VOID
NTAPI
InitializeLoadedModuleList(
    __in PKLDR_DATA_TABLE_ENTRY DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY TempDataTableEntry = NULL;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;
    PDRIVER_OBJECT DriverObject = NULL;
    UNICODE_STRING DriverPath = { 0 };
    PKLDR_DATA_TABLE_ENTRY KernelDataTableEntry = NULL;
    UNICODE_STRING KernelString = { 0 };

    RtlInitUnicodeString(&KernelString, L"ntoskrnl.exe");

    if (NULL == LoadedModuleList) {
        RtlInitUnicodeString(&DriverPath, L"\\Driver\\disk");

        Status = ObReferenceObjectByName(
            &DriverPath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            FILE_ALL_ACCESS,
            *IoDriverObjectType,
            KernelMode,
            NULL,
            &DriverObject);

        if (NT_SUCCESS(Status)) {
            Status = STATUS_NO_MORE_ENTRIES;

            TempDataTableEntry = DriverObject->DriverSection;

            if (NULL != TempDataTableEntry) {
                FoundDataTableEntry = CONTAINING_RECORD(
                    TempDataTableEntry->InLoadOrderLinks.Flink,
                    KLDR_DATA_TABLE_ENTRY,
                    InLoadOrderLinks);

                while (FoundDataTableEntry != TempDataTableEntry) {
                    if (NULL != FoundDataTableEntry->DllBase) {
                        if (FALSE != RtlEqualUnicodeString(
                            &KernelString,
                            &FoundDataTableEntry->BaseDllName,
                            TRUE)) {
                            LoadedModuleList = FoundDataTableEntry->InLoadOrderLinks.Blink;
                            break;
                        }
                    }

                    FoundDataTableEntry = CONTAINING_RECORD(
                        FoundDataTableEntry->InLoadOrderLinks.Flink,
                        KLDR_DATA_TABLE_ENTRY,
                        InLoadOrderLinks);
                }
            }

            ObDereferenceObject(DriverObject);
        }
    }

    InitializeListHead(&LoadedPrivateImageList);

    if (NULL != DataTableEntry) {
        DataTableEntry->LoadCount++;

        InsertTailList(
            &LoadedPrivateImageList,
            &DataTableEntry->InLoadOrderLinks);

        CaptureImageExceptionValues(
            DataTableEntry->DllBase,
            &DataTableEntry->ExceptionTable,
            &DataTableEntry->ExceptionTableSize);
    }
}

NTSTATUS
NTAPI
FindEntryForKernelPrivateImage(
    __in PUNICODE_STRING ImageFileName,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if (FALSE == IsListEmpty(&LoadedPrivateImageList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            LoadedPrivateImageList.Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry != (ULONG_PTR)&LoadedPrivateImageList) {
            if (FALSE != RtlEqualUnicodeString(
                ImageFileName,
                &FoundDataTableEntry->BaseDllName,
                TRUE)) {
                *DataTableEntry = FoundDataTableEntry;
                Status = STATUS_SUCCESS;
                break;
            }

            FoundDataTableEntry = CONTAINING_RECORD(
                FoundDataTableEntry->InLoadOrderLinks.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);
        }
    }

    return Status;
}

NTSTATUS
NTAPI
FindEntryForKernelImage(
    __in PUNICODE_STRING ImageFileName,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if (NULL != LoadedModuleList) {
        FoundDataTableEntry = CONTAINING_RECORD(
            LoadedModuleList->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while (FoundDataTableEntry != LoadedModuleList) {
            if (FALSE != RtlEqualUnicodeString(
                ImageFileName,
                &FoundDataTableEntry->BaseDllName,
                TRUE)) {
                *DataTableEntry = FoundDataTableEntry;
                Status = STATUS_SUCCESS;
                break;
            }

            FoundDataTableEntry = CONTAINING_RECORD(
                FoundDataTableEntry->InLoadOrderLinks.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);
        }
    }

    return Status;
}

NTSTATUS
NTAPI
FindEntryForKernelPrivateImageAddress(
    __in PVOID Address,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if (FALSE == IsListEmpty(&LoadedPrivateImageList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            LoadedPrivateImageList.Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry !=
            (ULONG_PTR)&LoadedPrivateImageList) {
            if ((ULONG_PTR)Address >= (ULONG_PTR)FoundDataTableEntry->DllBase &&
                (ULONG_PTR)Address < (ULONG_PTR)FoundDataTableEntry->DllBase +
                FoundDataTableEntry->SizeOfImage) {
                *DataTableEntry = FoundDataTableEntry;
                Status = STATUS_SUCCESS;
                break;
            }

            FoundDataTableEntry = CONTAINING_RECORD(
                FoundDataTableEntry->InLoadOrderLinks.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);
        }
    }

    return Status;
}

NTSTATUS
NTAPI
FindEntryForKernelImageAddress(
    __in PVOID Address,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if (NULL != LoadedModuleList) {
        FoundDataTableEntry = CONTAINING_RECORD(
            LoadedModuleList->Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while (FoundDataTableEntry != LoadedModuleList) {
            if ((ULONG_PTR)Address >= (ULONG_PTR)FoundDataTableEntry->DllBase &&
                (ULONG_PTR)Address < (ULONG_PTR)FoundDataTableEntry->DllBase +
                FoundDataTableEntry->SizeOfImage) {
                *DataTableEntry = FoundDataTableEntry;
                Status = STATUS_SUCCESS;
                break;
            }

            FoundDataTableEntry = CONTAINING_RECORD(
                FoundDataTableEntry->InLoadOrderLinks.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);
        }
    }

    return Status;
}

PVOID
NTAPI
GetKernelForwarder(
    __in PSTR ForwarderData
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSTR Separator = NULL;
    PSTR ImageName = NULL;
    PSTR ProcedureName = NULL;
    ULONG ProcedureNumber = 0;
    PVOID ProcedureAddress = NULL;
    PLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING TempImageFileName = { 0 };
    UNICODE_STRING ImageFileName = { 0 };

    Separator = strchr(
        ForwarderData,
        '.');

    if (Separator) {
        ImageName = ExAllocatePool(
            NonPagedPool,
            Separator - ForwarderData);

        if (ImageName) {
            RtlCopyMemory(
                ImageName,
                ForwarderData,
                Separator - ForwarderData);

            RtlInitAnsiString(
                &TempImageFileName,
                ImageName);

            Status = RtlAnsiStringToUnicodeString(
                &ImageFileName,
                &TempImageFileName,
                TRUE);

            if (NT_SUCCESS(Status)) {
                Status = FindEntryForKernelPrivateImage(
                    &ImageFileName,
                    &DataTableEntry);

                if (NT_SUCCESS(Status)) {
                    Separator += 1;
                    ProcedureName = Separator;

                    if (Separator[0] != '#') {
                        ProcedureAddress = GetKernelProcedureAddress(
                            DataTableEntry->DllBase,
                            ProcedureName,
                            0);
                    }
                    else {
                        Separator += 1;

                        if (RtlCharToInteger(
                            Separator,
                            0,
                            &ProcedureNumber) >= 0) {
                            ProcedureAddress = GetKernelProcedureAddress(
                                DataTableEntry->DllBase,
                                NULL,
                                ProcedureNumber);
                        }
                    }
                }
                else {
                    Status = FindEntryForKernelImage(
                        &ImageFileName,
                        &DataTableEntry);

                    if (NT_SUCCESS(Status)) {
                        Separator += 1;
                        ProcedureName = Separator;

                        if (Separator[0] != '#') {
                            ProcedureAddress = GetKernelProcedureAddress(
                                DataTableEntry->DllBase,
                                ProcedureName,
                                0);
                        }
                        else {
                            Separator += 1;

                            if (RtlCharToInteger(
                                Separator,
                                0,
                                &ProcedureNumber) >= 0) {
                                ProcedureAddress = GetKernelProcedureAddress(
                                    DataTableEntry->DllBase,
                                    NULL,
                                    ProcedureNumber);
                            }
                        }
                    }
                }

                RtlFreeUnicodeString(&ImageFileName);
            }

            ExFreePool(ImageName);
        }
    }

    return ProcedureAddress;
}

PVOID
NTAPI
GetKernelProcedureAddress(
    __in PVOID ImageBase,
    __in_opt PSTR ProcedureName,
    __in_opt ULONG ProcedureNumber
)
{
    PIMAGE_EXPORT_DIRECTORY ExportDirectory = NULL;
    ULONG Size = 0;
    PULONG NameTable = NULL;
    PUSHORT OrdinalTable = NULL;
    PULONG AddressTable = NULL;
    PSTR NameTableName = NULL;
    USHORT HintIndex = 0;
    PVOID ProcedureAddress = NULL;

    ExportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &Size);

    if (NULL != ExportDirectory) {
        NameTable = (PCHAR)ImageBase + ExportDirectory->AddressOfNames;
        OrdinalTable = (PCHAR)ImageBase + ExportDirectory->AddressOfNameOrdinals;
        AddressTable = (PCHAR)ImageBase + ExportDirectory->AddressOfFunctions;

        if (NULL != NameTable &&
            NULL != OrdinalTable &&
            NULL != AddressTable) {
            if (ProcedureNumber >= ExportDirectory->Base &&
                ProcedureNumber < MAXSHORT) {
                ProcedureAddress = (PCHAR)ImageBase +
                    AddressTable[ProcedureNumber - ExportDirectory->Base];
            }
            else {
                for (HintIndex = 0;
                    HintIndex < ExportDirectory->NumberOfNames;
                    HintIndex++) {
                    NameTableName = (PCHAR)ImageBase + NameTable[HintIndex];

                    if (0 == _stricmp(
                        ProcedureName,
                        NameTableName)) {
                        ProcedureAddress = (PCHAR)ImageBase +
                            AddressTable[OrdinalTable[HintIndex]];
                    }
                }
            }
        }

        if ((ULONG_PTR)ProcedureAddress >= (ULONG_PTR)ExportDirectory &&
            (ULONG_PTR)ProcedureAddress < (ULONG_PTR)ExportDirectory + Size) {
            ProcedureAddress = GetKernelForwarder(ProcedureAddress);
        }
    }

    return ProcedureAddress;
}

VOID
NTAPI
DereferenceKernelImage(
    __in PSTR ImageName
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING TempImageFileName = { 0 };
    UNICODE_STRING ImageFileName = { 0 };

    RtlInitAnsiString(
        &TempImageFileName,
        ImageName);

    Status = RtlAnsiStringToUnicodeString(
        &ImageFileName,
        &TempImageFileName,
        TRUE);


    if (NT_SUCCESS(Status)) {
        Status = FindEntryForKernelPrivateImage(
            &ImageFileName,
            &DataTableEntry);

        if (NT_SUCCESS(Status)) {
            DataTableEntry->LoadCount--;
        }
        else {
            Status = FindEntryForKernelImage(
                &ImageFileName,
                &DataTableEntry);

            if (NT_SUCCESS(Status)) {
                DataTableEntry->LoadCount--;
            }
        }

        RtlFreeUnicodeString(&ImageFileName);
    }
}

VOID
NTAPI
DereferenceKernelImageImports(
    __in PVOID ImageBase
)
{
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory = NULL;
    ULONG Size = 0;
    PSTR ImportImageName = NULL;

    ImportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_IMPORT,
        &Size);

    if (0 != Size) {
        do {
            ImportImageName = (PCHAR)ImageBase + ImportDirectory->Name;

            DereferenceKernelImage(ImportImageName);

            ImportDirectory++;
        } while (0 != ImportDirectory->Characteristics);
    }
}

PVOID
NTAPI
ReferenceKernelImage(
    __in PVOID ImageName
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING TempImageFileName = { 0 };
    UNICODE_STRING ImageFileName = { 0 };
    PVOID ImageBase = NULL;

    RtlInitAnsiString(&TempImageFileName, ImageName);

    Status = RtlAnsiStringToUnicodeString(
        &ImageFileName,
        &TempImageFileName,
        TRUE);

    if (NT_SUCCESS(Status)) {
        Status = FindEntryForKernelPrivateImage(
            &ImageFileName,
            &DataTableEntry);

        if (NT_SUCCESS(Status)) {
            DataTableEntry->LoadCount++;
            ImageBase = DataTableEntry->DllBase;
        }
        else {
            Status = FindEntryForKernelImage(
                &ImageFileName,
                &DataTableEntry);

            if (NT_SUCCESS(Status)) {
                DataTableEntry->LoadCount++;
                ImageBase = DataTableEntry->DllBase;
            }
        }

        RtlFreeUnicodeString(&ImageFileName);
    }

    return ImageBase;
}

VOID
NTAPI
SnapKernelThunk(
    __in PVOID ImageBase
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory = NULL;
    ULONG Size = 0;
    PIMAGE_THUNK_DATA OriginalThunk = NULL;
    PIMAGE_THUNK_DATA Thunk = NULL;
    PIMAGE_IMPORT_BY_NAME ImportByName = NULL;
    PSTR ImportImageName = NULL;
    PVOID ImportImageBase = NULL;
    USHORT Ordinal = 0;
    PVOID FunctionAddress = NULL;

    ImportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_IMPORT,
        &Size);

    if (0 != Size) {
        do {
            OriginalThunk = (PCHAR)ImageBase + ImportDirectory->OriginalFirstThunk;
            Thunk = (PCHAR)ImageBase + ImportDirectory->FirstThunk;
            ImportImageName = (PCHAR)ImageBase + ImportDirectory->Name;

            ImportImageBase = ReferenceKernelImage(ImportImageName);

            if (NULL != ImportImageBase) {
                do {
                    if (IMAGE_SNAP_BY_ORDINAL(OriginalThunk->u1.Ordinal)) {
                        Ordinal = (USHORT)IMAGE_ORDINAL(OriginalThunk->u1.Ordinal);

                        FunctionAddress = GetKernelProcedureAddress(
                            ImportImageBase,
                            NULL,
                            Ordinal);

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (ULONG_PTR)FunctionAddress;
                        }
                        else {
                            DbgPrint(
                                "[Sefirot] [Tiferet] import procedure ordinal@%d not found\n",
                                Ordinal);
                        }
                    }
                    else {
                        ImportByName = (PCHAR)ImageBase + OriginalThunk->u1.AddressOfData;

                        FunctionAddress = GetKernelProcedureAddress(
                            ImportImageBase,
                            ImportByName->Name,
                            0);

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (ULONG_PTR)FunctionAddress;
                        }
                        else {
                            DbgPrint(
                                "[Sefirot] [Tiferet] import procedure %hs not found\n",
                                ImportByName->Name);
                        }
                    }

                    OriginalThunk++;
                    Thunk++;
                } while (OriginalThunk->u1.Function);
            }
            else {
                DbgPrint(
                    "[Sefirot] [Tiferet] import dll %hs not found\n",
                    ImportImageName);
            }

            ImportDirectory++;
        } while (0 != ImportDirectory->Characteristics);
    }
}

PVOID
NTAPI
AllocateKernelPrivateImage(
    __in PVOID ViewBase
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID ImageBase = NULL;
    PHYSICAL_ADDRESS HighestAcceptableAddress = { 0 };
    ULONG SizeOfImage = 0;

    SizeOfImage = GetSizeOfImage(ViewBase);

    if (0 != SizeOfImage) {
        SizeOfImage += PAGE_SIZE;

        HighestAcceptableAddress.QuadPart = -1;

        ImageBase = MmAllocateContiguousMemory(
            SizeOfImage,
            HighestAcceptableAddress);

        if (NULL != ImageBase) {
            ImageBase = (PCHAR)ImageBase + PAGE_SIZE;
        }
    }

    return ImageBase;
}

PVOID
NTAPI
MapKernelPrivateImage(
    __in PVOID ViewBase
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID ImageBase = NULL;
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    LONG_PTR Diff = 0;
    SIZE_T Index = 0;

    NtHeaders = RtlImageNtHeader(ViewBase);

    if (NULL != NtHeaders) {
        ImageBase = AllocateKernelPrivateImage(ViewBase);

        if (NULL != ImageBase) {
            RtlZeroMemory(
                ImageBase,
                NtHeaders->OptionalHeader.SizeOfImage);

            NtSection = IMAGE_FIRST_SECTION(NtHeaders);

            RtlCopyMemory(
                ImageBase,
                ViewBase,
                NtSection->VirtualAddress);

            for (Index = 0;
                Index < NtHeaders->FileHeader.NumberOfSections;
                Index++) {
                if (0 != NtSection[Index].VirtualAddress) {
                    RtlCopyMemory(
                        (PCHAR)ImageBase + NtSection[Index].VirtualAddress,
                        (PCHAR)ViewBase + NtSection[Index].PointerToRawData,
                        NtSection[Index].SizeOfRawData);
                }
            }

            NtHeaders = RtlImageNtHeader(ImageBase);

            Diff = (LONG_PTR)ImageBase - NtHeaders->OptionalHeader.ImageBase;
            NtHeaders->OptionalHeader.ImageBase = (ULONG_PTR)ImageBase;

            RelocateImage(ImageBase, Diff);
            SnapKernelThunk(ImageBase);
        }
    }

    return ImageBase;
}

PKLDR_DATA_TABLE_ENTRY
NTAPI
LoadKernelPrivateImage(
    __in PVOID ViewBase,
    __in PCWSTR ImageName,
    __in BOOLEAN Insert
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    UNICODE_STRING ImageFileName = { 0 };
    PVOID ImageBase = NULL;
    PCWSTR BaseName = NULL;

    BaseName = wcsrchr(ImageName, L'\\');

    if (NULL == BaseName) {
        BaseName = ImageName;
    }
    else {
        BaseName++;
    }

    RtlInitUnicodeString(&ImageFileName, BaseName);

    Status = FindEntryForKernelPrivateImage(
        &ImageFileName,
        &DataTableEntry);

    if (NT_SUCCESS(Status)) {
        DataTableEntry->LoadCount++;
    }
    else {
        ImageBase = MapKernelPrivateImage(ViewBase);

        if (NULL != ImageBase) {
            DataTableEntry = (PKLDR_DATA_TABLE_ENTRY)
                ((PCHAR)ImageBase - PAGE_SIZE);

            RtlZeroMemory(
                DataTableEntry,
                sizeof(KLDR_DATA_TABLE_ENTRY) +
                MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR) * 2);

            DataTableEntry->DllBase = ImageBase;
            DataTableEntry->SizeOfImage = GetSizeOfImage(ImageBase);
            DataTableEntry->EntryPoint = GetAddressOfEntryPoint(ImageBase);
            DataTableEntry->Flags = LDRP_STATIC_LINK;
            DataTableEntry->LoadCount = 0;

            DataTableEntry->FullDllName.Buffer = DataTableEntry + 1;

            DataTableEntry->FullDllName.MaximumLength =
                MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

            wcscpy(DataTableEntry->FullDllName.Buffer, SystemDirectory);
            wcscat(DataTableEntry->FullDllName.Buffer, BaseName);

            DataTableEntry->FullDllName.Length =
                wcslen(DataTableEntry->FullDllName.Buffer) * sizeof(WCHAR);

            DataTableEntry->BaseDllName.Buffer =
                DataTableEntry->FullDllName.Buffer + MAXIMUM_FILENAME_LENGTH;

            DataTableEntry->BaseDllName.MaximumLength =
                MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

            wcscpy(DataTableEntry->BaseDllName.Buffer, BaseName);

            DataTableEntry->BaseDllName.Length =
                wcslen(DataTableEntry->BaseDllName.Buffer) * sizeof(WCHAR);

            if (FALSE != Insert) {
                DataTableEntry->LoadCount++;

                InsertTailList(
                    &LoadedPrivateImageList,
                    &DataTableEntry->InLoadOrderLinks);

                CaptureImageExceptionValues(
                    DataTableEntry->DllBase,
                    &DataTableEntry->ExceptionTable,
                    &DataTableEntry->ExceptionTableSize);
            }
        }
    }

    return DataTableEntry;
}

VOID
NTAPI
UnloadKernelPrivateImage(
    __in PKLDR_DATA_TABLE_ENTRY DataTableEntry
)
{
    RTL_SOFT_ASSERT(NULL != DataTableEntry);

    DataTableEntry->LoadCount--;

    if (0 == DataTableEntry->LoadCount) {
        DereferenceKernelImageImports(DataTableEntry->DllBase);
        RemoveEntryList(&DataTableEntry->InLoadOrderLinks);
        MmFreeContiguousMemory(DataTableEntry);
    }
}
