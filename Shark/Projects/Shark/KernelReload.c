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

#include <Defs.h>

#include "KernelReload.h"

#include "Except.h"

extern POBJECT_TYPE * IoDriverObjectType;

static PLIST_ENTRY LoadedModuleList;
static LIST_ENTRY LoadedPrivateModuleList;

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

#define KERNEL_NAME L"ntoskrnl.exe"

    KernelString.Buffer = (const PUSHORT)KERNEL_NAME;
    KernelString.Length = sizeof(KERNEL_NAME) - sizeof(WCHAR);
    KernelString.MaximumLength = sizeof KERNEL_NAME;

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

    InitializeListHead(&LoadedPrivateModuleList);

    if (NULL != DataTableEntry) {
        DataTableEntry->LoadCount++;

        InsertTailList(
            &LoadedPrivateModuleList,
            &DataTableEntry->InLoadOrderLinks);

        
    }
}

NTSTATUS
NTAPI
FindEntryForPrivateSystemImage(
    __in PUNICODE_STRING ImageFileName,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if (FALSE == IsListEmpty(&LoadedPrivateModuleList)) {
        FoundDataTableEntry = CONTAINING_RECORD(
            LoadedPrivateModuleList.Flink,
            KLDR_DATA_TABLE_ENTRY,
            InLoadOrderLinks);

        while ((ULONG_PTR)FoundDataTableEntry != (ULONG_PTR)&LoadedPrivateModuleList) {
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
FindEntryForSystemImage(
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
FindEntryForPrivateSystemImageAddress(
    __in PVOID Address,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if ((ULONG_PTR)Address > *(PULONG_PTR)MM_HIGHEST_USER_ADDRESS) {
        if (FALSE == IsListEmpty(&LoadedPrivateModuleList)) {
            FoundDataTableEntry = CONTAINING_RECORD(
                LoadedPrivateModuleList.Flink,
                KLDR_DATA_TABLE_ENTRY,
                InLoadOrderLinks);

            while ((ULONG_PTR)FoundDataTableEntry !=
                (ULONG_PTR)&LoadedPrivateModuleList) {
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
    }

    return Status;
}

NTSTATUS
NTAPI
FindEntryForSystemImageAddress(
    __in PVOID Address,
    __out PKLDR_DATA_TABLE_ENTRY * DataTableEntry
)
{
    NTSTATUS Status = STATUS_NO_MORE_ENTRIES;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry = NULL;

    if ((ULONG_PTR)Address > *(PULONG_PTR)MM_HIGHEST_USER_ADDRESS) {
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
    }

    return Status;
}

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

PVOID
NTAPI
GetForwarder(
    __in PSTR ForwarderData
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PSTR Separator = NULL;
    PSTR ImageName = NULL;
    PVOID ImageBase = NULL;
    PSTR ProcedureName = NULL;
    ULONG ProcedureNumber = 0;
    PVOID ProcedureAddress = NULL;
    PLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING TempFileName = { 0 };
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
                &TempFileName,
                ImageName);

            Status = RtlAnsiStringToUnicodeString(
                &ImageFileName,
                &TempFileName,
                TRUE);

            if (NT_SUCCESS(Status)) {
                Status = FindEntryForPrivateSystemImage(
                    &ImageFileName,
                    &DataTableEntry);

                if (NT_SUCCESS(Status)) {
                    ImageBase = DataTableEntry->DllBase;
                }
                else {
                    Status = FindEntryForSystemImage(
                        &ImageFileName,
                        &DataTableEntry);

                    if (NT_SUCCESS(Status)) {
                        ImageBase = DataTableEntry->DllBase;
                    }
                }

                RtlFreeUnicodeString(&ImageFileName);
            }

            if (NULL != ImageBase) {
                Separator += 1;
                ProcedureName = Separator;

                if (Separator[0] != '#') {
                    ProcedureAddress = GetProcedureAddress(
                        ImageBase,
                        ProcedureName,
                        0);
                }
                else {
                    Separator += 1;

                    if (RtlCharToInteger(
                        Separator,
                        0,
                        &ProcedureNumber) >= 0) {
                        ProcedureAddress = GetProcedureAddress(
                            ImageBase,
                            NULL,
                            ProcedureNumber);
                    }
                }
            }

            ExFreePool(ImageName);
        }
    }

    return ProcedureAddress;
}

PVOID
NTAPI
GetProcedureAddress(
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
            ProcedureAddress = GetForwarder(ProcedureAddress);
        }
    }

    return ProcedureAddress;
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
DereferenceKernelImage(
    __in PSTR ImageName
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    ANSI_STRING TempFileName = { 0 };
    UNICODE_STRING ImageFileName = { 0 };

    RtlInitAnsiString(
        &TempFileName,
        ImageName);

    Status = RtlAnsiStringToUnicodeString(
        &ImageFileName,
        &TempFileName,
        TRUE);

    if (NT_SUCCESS(Status)) {
        Status = FindEntryForSystemImage(
            &ImageFileName,
            &DataTableEntry);

        if (NT_SUCCESS(Status)) {
            DataTableEntry->LoadCount--;
        }

        RtlFreeUnicodeString(&ImageFileName);
    }
}

VOID
NTAPI
DereferenceKernelImports(
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
    ANSI_STRING TempFileName = { 0 };
    UNICODE_STRING ImageFileName = { 0 };
    PVOID ImageBase = NULL;

    RtlInitAnsiString(&TempFileName, ImageName);

    Status = RtlAnsiStringToUnicodeString(
        &ImageFileName,
        &TempFileName,
        TRUE);

    if (NT_SUCCESS(Status)) {
        Status = FindEntryForSystemImage(
            &ImageFileName,
            &DataTableEntry);

        if (NT_SUCCESS(Status)) {
            DataTableEntry->LoadCount++;
            ImageBase = DataTableEntry->DllBase;
        }

        RtlFreeUnicodeString(&ImageFileName);
    }

    return ImageBase;
}

VOID
NTAPI
SnapThunk(
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

                        FunctionAddress = GetProcedureAddress(
                            ImportImageBase,
                            NULL,
                            Ordinal);

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (ULONG_PTR)FunctionAddress;
                        }
                        else {
                            DbgPrint(
                                "Shark - import procedure ordinal@%d not found\n",
                                Ordinal);
                        }
                    }
                    else {
                        ImportByName = (PCHAR)ImageBase + OriginalThunk->u1.AddressOfData;

                        FunctionAddress = GetProcedureAddress(
                            ImportImageBase,
                            ImportByName->Name,
                            0);

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (ULONG_PTR)FunctionAddress;
                        }
                        else {
                            DbgPrint(
                                "Shark - import procedure %hs not found\n",
                                ImportByName->Name);
                        }
                    }

                    OriginalThunk++;
                    Thunk++;
                } while (OriginalThunk->u1.Function);
            }
            else {
                DbgPrint(
                    "Shark - import dll %hs not found\n",
                    ImportImageName);
            }

            ImportDirectory++;
        } while (0 != ImportDirectory->Characteristics);
    }
}

PVOID
NTAPI
AllocateImage(
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
MapImageNtSection(
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
        ImageBase = AllocateImage(ViewBase);

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
            SnapThunk(ImageBase);
        }
    }

    return ImageBase;
}

PKLDR_DATA_TABLE_ENTRY
NTAPI
LoadPrivateSystemImage(
    __in PVOID ViewBase,
    __in PCWSTR ImageName,
    __in BOOLEAN Insert
)
{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry = NULL;
    PVOID ImageBase = NULL;
    PCWSTR BaseName = NULL;
    ULONG Platform = 0;

    ImageBase = MapImageNtSection(ViewBase);

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

        CaptureImageExceptionValues(
            ImageBase,
            &DataTableEntry->ExceptionTable,
            &DataTableEntry->ExceptionTableSize);

        DataTableEntry->FullDllName.Buffer = DataTableEntry + 1;

        DataTableEntry->FullDllName.MaximumLength =
            MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

        BaseName = wcsrchr(
            ImageName,
            L'\\');

        if (NULL == BaseName) {
            BaseName = ImageName;
        }
        else {
            BaseName++;
        }

        Platform = GetPlatform(ImageBase);

        wcscpy(
            DataTableEntry->FullDllName.Buffer,
            SystemDirectory);

        wcscat(DataTableEntry->FullDllName.Buffer, BaseName);

        DataTableEntry->FullDllName.Length =
            wcslen(DataTableEntry->FullDllName.Buffer) * sizeof(WCHAR);

        DataTableEntry->BaseDllName.Buffer =
            DataTableEntry->FullDllName.Buffer + MAXIMUM_FILENAME_LENGTH;

        DataTableEntry->BaseDllName.MaximumLength =
            MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

        wcscpy(
            DataTableEntry->BaseDllName.Buffer,
            BaseName);

        DataTableEntry->BaseDllName.Length =
            wcslen(DataTableEntry->BaseDllName.Buffer) * sizeof(WCHAR);

        if (FALSE != Insert) {
            InsertTailList(
                &LoadedPrivateModuleList,
                &DataTableEntry->InLoadOrderLinks);
        }
    }

    return DataTableEntry;
}

VOID
NTAPI
UnloadPrivateSystemImage(
    __in PKLDR_DATA_TABLE_ENTRY DataTableEntry
)
{
    RTL_SOFT_ASSERT(NULL != DataTableEntry);

    DereferenceKernelImports(DataTableEntry->DllBase);
    RemoveEntryList(&DataTableEntry->InLoadOrderLinks);
    MmFreeContiguousMemory(DataTableEntry);
}
