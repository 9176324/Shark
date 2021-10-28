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
#include <devicedefs.h>

#include "Support.h"

u32 Cookie;
u32 SessionCookie;
ptr Session;
ptr SUPHandle;

static
u16
NTAPI
LdrGetOsPlatform(
    void
)
{
    status Status = STATUS_SUCCESS;
    SYSTEM_PROCESSOR_INFORMATION ProcessorInformation = { 0 };
    u ProcessInformation = 0;
    u32 ReturnLength = 0;
    u16 Platform = 0;

    Status = NtQuerySystemInformation(
        SystemProcessorInformation,
        &ProcessorInformation,
        sizeof(SYSTEM_PROCESSOR_INFORMATION),
        &ReturnLength);

    if (TRACE(Status)) {
        if (PROCESSOR_ARCHITECTURE_AMD64 ==
            ProcessorInformation.ProcessorArchitecture) {
            Platform = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
        }
        else if (PROCESSOR_ARCHITECTURE_INTEL ==
            ProcessorInformation.ProcessorArchitecture) {
            Status = NtQueryInformationProcess(
                NtCurrentProcess(),
                ProcessWow64Information,
                &ProcessInformation,
                sizeof(ProcessInformation),
                &ReturnLength);

            if (TRACE(Status)) {
                if (ProcessInformation) {
                    Platform = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
                }
                else {
                    Platform = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
                }
            }
        }
    }

    return Platform;
}

status
NTAPI
RegistryCreateKey(
    __out ptr * KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in wcptr KeyList,
    __in u32 CreateOptions
)
{
    status Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING KeyPath = { 0 };

    RtlInitUnicodeString(&KeyPath, KeyList);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtCreateKey(
        KeyHandle,
        DesiredAccess,
        &ObjectAttributes,
        0,
        NULL,
        CreateOptions,
        NULL);

    return Status;
}

status
NTAPI
RegistryOpenKey(
    __out ptr * KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in wcptr KeyList
)
{
    status Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING KeyPath = { 0 };

    RtlInitUnicodeString(&KeyPath, KeyList);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenKey(
        KeyHandle,
        DesiredAccess,
        &ObjectAttributes);

    return Status;
}

status
NTAPI
RegistrySetValueKey(
    __in ptr KeyHandle,
    __in wcptr ValueName,
    __in u32 Type,
    __in_bcount_opt(DataSize) ptr Data,
    __in u32 DataSize
)
{
    status Status = STATUS_SUCCESS;
    UNICODE_STRING KeyValueName = { 0 };

    RtlInitUnicodeString(&KeyValueName, ValueName);

    Status = NtSetValueKey(
        KeyHandle,
        &KeyValueName,
        0,
        Type,
        Data,
        DataSize);

    return Status;
}

void
NTAPI
RegistryDeleteKey(
    __in PUNICODE_STRING KeyPath
)
{
    status Status = STATUS_SUCCESS;
    ptr KeyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    PKEY_BASIC_INFORMATION BasicInformation = NULL;
    PKEY_VALUE_BASIC_INFORMATION ValueBasicInformation = NULL;
    UNICODE_STRING KeyName = { 0 };
    UNICODE_STRING Separator = { 0 };
    UNICODE_STRING SubKeyName = { 0 };
    wcptr KeyNameBuffer = NULL;
    u32 Length = 0;
    u32 ResultLength = 0;

    InitializeObjectAttributes(
        &ObjectAttributes,
        KeyPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenKey(
        &KeyHandle,
        KEY_ALL_ACCESS,
        &ObjectAttributes);

    if (NT_SUCCESS(Status)) {
        Length =
            MAXIMUM_FILENAME_LENGTH * sizeof(wc) +
            FIELD_OFFSET(KEY_BASIC_INFORMATION, Name);

        BasicInformation =
            __malloc(Length
                + MAXIMUM_FILENAME_LENGTH * sizeof(wc)
                + MAXIMUM_FILENAME_LENGTH * sizeof(wc)
                + FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name));

        if (NULL != BasicInformation) {
            KeyNameBuffer = (PCHAR)BasicInformation + Length;
            ValueBasicInformation = KeyNameBuffer + MAXIMUM_FILENAME_LENGTH;

            do {
                Status = NtEnumerateKey(
                    KeyHandle,
                    0,
                    KeyBasicInformation,
                    BasicInformation,
                    Length,
                    &ResultLength);

                if (NT_SUCCESS(Status)) {
                    SubKeyName.Buffer = KeyNameBuffer;
                    SubKeyName.Length = 0;
                    SubKeyName.MaximumLength = (u16)MAXIMUM_FILENAME_LENGTH * sizeof(wc);

                    KeyName.Buffer = BasicInformation->Name;
                    KeyName.Length = (u16)BasicInformation->NameLength;
                    KeyName.MaximumLength = (u16)MAXIMUM_FILENAME_LENGTH * sizeof(wc);

                    RtlInitUnicodeString(&Separator, L"\\");

                    RtlAppendStringToString(&SubKeyName, KeyPath);
                    RtlAppendStringToString(&SubKeyName, &Separator);
                    RtlAppendStringToString(&SubKeyName, &KeyName);

                    RegistryDeleteKey(&SubKeyName);
                }
            } while (NT_SUCCESS(Status));

            do {
                Status = NtEnumerateValueKey(
                    KeyHandle,
                    0,
                    KeyValueBasicInformation,
                    ValueBasicInformation,
                    Length,
                    &ResultLength);

                if (NT_SUCCESS(Status)) {
                    KeyName.Buffer = ValueBasicInformation->Name;
                    KeyName.Length = (u16)ValueBasicInformation->NameLength;
                    KeyName.MaximumLength = (u16)MAXIMUM_FILENAME_LENGTH * sizeof(wc);

                    TRACE(NtDeleteValueKey(
                        KeyHandle,
                        &KeyName));
                }
            } while (NT_SUCCESS(Status));

            __free(BasicInformation);
        }

        TRACE(NtDeleteKey(KeyHandle));
        TRACE(NtClose(KeyHandle));
    }
}

status
NTAPI
RegistryCreateSevice(
    __in wcptr ImageFileName,
    __in wcptr ServiceName
)
{
    status Status = STATUS_SUCCESS;
    ptr KeyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING KeyPath = { 0 };
    wcptr KeyPathBuffer = NULL;
    UNICODE_STRING KeyName = { 0 };
    UNICODE_STRING ImagePath = { 0 };
    u32 Type = 1;
    u32 Start = 3;
    u32 ErrorControl = 1;
    b WasEnabled = FALSE;

    Status = RtlAdjustPrivilege(
        SE_LOAD_DRIVER_PRIVILEGE,
        TRUE,
        FALSE,
        &WasEnabled);

    if (TRACE(Status)) {
        KeyPathBuffer =
            __malloc(MAXIMUM_FILENAME_LENGTH * sizeof(wc));

        if (NULL != KeyPathBuffer) {
            KeyPath.Buffer = KeyPathBuffer;
            KeyPath.Length = 0;
            KeyPath.MaximumLength = MAXIMUM_FILENAME_LENGTH * sizeof(wc);

            RtlInitUnicodeString(&KeyName, ServicesDirectory);
            RtlAppendStringToString(&KeyPath, &KeyName);
            RtlInitUnicodeString(&KeyName, ServiceName);
            RtlAppendStringToString(&KeyPath, &KeyName);

            InitializeObjectAttributes(
                &ObjectAttributes,
                &KeyPath,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

            Status = NtCreateKey(
                &KeyHandle,
                KEY_ALL_ACCESS,
                &ObjectAttributes,
                0,
                NULL,
                REG_OPTION_VOLATILE,
                NULL);

            if (TRACE(Status)) {
                TRACE(RegistrySetValueKey(
                    KeyHandle,
                    L"Type",
                    REG_DWORD,
                    &Type,
                    sizeof(Type)));

                TRACE(RegistrySetValueKey(
                    KeyHandle,
                    L"ErrorControl",
                    REG_DWORD,
                    &ErrorControl,
                    sizeof(ErrorControl)));

                TRACE(RegistrySetValueKey(
                    KeyHandle,
                    L"Start",
                    REG_DWORD,
                    &Start,
                    sizeof(Start)));

                TRACE(RegistrySetValueKey(
                    KeyHandle,
                    L"DisplayName",
                    REG_SZ,
                    ServiceName,
                    wcslen(ServiceName) * sizeof(wc) + sizeof(UNICODE_NULL)));

                RtlInitUnicodeString(&ImagePath, ImageFileName);

                TRACE(RegistrySetValueKey(
                    KeyHandle,
                    L"ImagePath",
                    REG_EXPAND_SZ,
                    ImagePath.Buffer,
                    ImagePath.Length + sizeof(UNICODE_NULL)));

                Status = NtLoadDriver(&KeyPath);

                TRACE(NtClose(KeyHandle));
            }

            __free(KeyPathBuffer);
        }

        if (FALSE == WasEnabled) {
            TRACE(RtlAdjustPrivilege(
                SE_LOAD_DRIVER_PRIVILEGE,
                FALSE,
                FALSE,
                &WasEnabled));
        }
    }

    return Status;
}

status
NTAPI
RegistryDeleteSevice(
    __in wcptr ServiceName
)
{
    status Status = STATUS_SUCCESS;
    ptr KeyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING KeyPath = { 0 };
    wcptr KeyPathBuffer = NULL;
    UNICODE_STRING KeyName = { 0 };
    b WasEnabled = FALSE;

    Status = RtlAdjustPrivilege(
        SE_LOAD_DRIVER_PRIVILEGE,
        TRUE,
        FALSE,
        &WasEnabled);

    if (TRACE(Status)) {
        KeyPathBuffer =
            __malloc(MAXIMUM_FILENAME_LENGTH * sizeof(wc));

        if (NULL != KeyPathBuffer) {
            KeyPath.Buffer = KeyPathBuffer;
            KeyPath.Length = 0;
            KeyPath.MaximumLength = MAXIMUM_FILENAME_LENGTH * sizeof(wc);

            RtlInitUnicodeString(&KeyName, ServicesDirectory);
            RtlAppendStringToString(&KeyPath, &KeyName);
            RtlInitUnicodeString(&KeyName, ServiceName);
            RtlAppendStringToString(&KeyPath, &KeyName);

            InitializeObjectAttributes(
                &ObjectAttributes,
                &KeyPath,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

            Status = NtOpenKey(
                &KeyHandle,
                KEY_ALL_ACCESS,
                &ObjectAttributes);

            if (TRACE(Status)) {
                Status = NtUnloadDriver(&KeyPath);

                if (TRACE(Status)) {
                    RegistryDeleteKey(&KeyPath);
                }

                TRACE(NtClose(KeyHandle));
            }

            __free(KeyPathBuffer);
        }

        if (FALSE == WasEnabled) {
            TRACE(RtlAdjustPrivilege(
                SE_LOAD_DRIVER_PRIVILEGE,
                FALSE,
                FALSE,
                &WasEnabled));
        }
    }

    return Status;
}

FORCEINLINE
u32
NTAPI
LdrGetRelocCount(
    __in u32 SizeOfBlock
)
{
    u32 Count = 0;

    Count = (SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(u16);

    return Count;
}

PIMAGE_BASE_RELOCATION
NTAPI
LdrRelocBlock(
    __in ptr VA,
    __in u32 Count,
    __in u16ptr NextOffset,
    __in s Diff
)
{
    u16ptr FixupVA = NULL;
    u16 Offset = 0;
    u16 Type = 0;

    while (Count--) {
        Offset = *NextOffset & 0xfff;
        FixupVA = (u8ptr)VA + Offset;
        Type = (*NextOffset >> 12) & 0xf;

        switch (Type) {
        case IMAGE_REL_BASED_ABSOLUTE: {
            break;
        }

        case IMAGE_REL_BASED_HIGH: {
            FixupVA[1] += (u16)((Diff >> 16) & 0xffff);
            break;
        }

        case IMAGE_REL_BASED_LOW: {
            FixupVA[0] += (u16)(Diff & 0xffff);
            break;
        }

        case IMAGE_REL_BASED_HIGHLOW: {
            *(u32ptr)FixupVA += (u32)Diff;
            break;
        }

        case IMAGE_REL_BASED_HIGHADJ: {
            FixupVA[0] += NextOffset[1] & 0xffff;
            FixupVA[1] += (u16)((Diff >> 16) & 0xffff);

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
            *(uptr)FixupVA += Diff;
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

void
NTAPI
LdrRelocImage(
    __in ptr ImageBase,
    __in s Diff
)
{
    PIMAGE_BASE_RELOCATION RelocDirectory = NULL;
    u32 Size = 0;
    ptr VA = 0;

    RelocDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_BASERELOC,
        &Size);

    if (0 != Size) {
        if (0 != Diff) {
            while (0 != Size) {
                VA = (u8ptr)ImageBase + RelocDirectory->VirtualAddress;
                Size -= RelocDirectory->SizeOfBlock;

                RelocDirectory = LdrRelocBlock(
                    VA,
                    LdrGetRelocCount(RelocDirectory->SizeOfBlock),
                    (u16ptr)(RelocDirectory + 1),
                    Diff);
            }
        }
    }
}

status
NTAPI
LdrMapSectionOffline(
    __in cptr ImageFileName,
    __out ptr * ImageViewBase
)
{
    status Status = STATUS_SUCCESS;
    ptr Handle = NULL;
    ptr Section = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    ptr ViewBase = NULL;
    u ViewSize = 0;
    STRING String = { 0 };
    UNICODE_STRING FullPath = { 0 };

    RtlInitString(&String, ImageFileName);

    Status = RtlAnsiStringToUnicodeString(
        &FullPath,
        &String,
        TRUE);

    if (NT_SUCCESS(Status)) {
        InitializeObjectAttributes(
            &ObjectAttributes,
            &FullPath,
            (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
            NULL,
            NULL);

        Status = NtOpenFile(
            &Handle,
            FILE_EXECUTE,
            &ObjectAttributes,
            &IoStatusBlock,
            FILE_SHARE_READ | FILE_SHARE_DELETE,
            0);

        if (NT_SUCCESS(Status)) {
            InitializeObjectAttributes(
                &ObjectAttributes,
                NULL,
                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                NULL,
                NULL);

            Status = NtCreateSection(
                &Section,
                SECTION_MAP_READ | SECTION_MAP_EXECUTE,
                &ObjectAttributes,
                NULL,
                PAGE_EXECUTE,
                SEC_IMAGE,
                Handle);

            if (NT_SUCCESS(Status)) {
                Status = NtMapViewOfSection(
                    Section,
                    NtCurrentProcess(),
                    &ViewBase,
                    0L,
                    0L,
                    NULL,
                    &ViewSize,
                    ViewShare,
                    0L,
                    PAGE_EXECUTE);

                if (NT_SUCCESS(Status)) {
                    *ImageViewBase = ViewBase;
                }

                TRACE(NtClose(Section));
            }

            TRACE(NtClose(Handle));
        }

        RtlFreeUnicodeString(&FullPath);
    }

    return Status;
}

status
NTAPI
LdrLoadImportOffline(
    __in PSTRING ImageFileName,
    __out ptr * ImageBase,
    __out ptr * ImportViewBase
)
{
    status Status = STATUS_SUCCESS;
    PRTL_PROCESS_MODULES Modules = NULL;
    u32 BufferSize = PAGE_SIZE;
    u32 Index = 0;
    u32 ReturnLength = 0;
    STRING String = { 0 };
    c FullName[MAXIMUM_FILENAME_LENGTH] = { 0 };

retry:
    Modules = __malloc(BufferSize);

    if (NULL != Modules) {
        RtlZeroMemory(Modules, BufferSize);

        Status = NtQuerySystemInformation(
            SystemModuleInformation,
            Modules,
            BufferSize,
            &ReturnLength);

        if (NT_SUCCESS(Status)) {
            Status = STATUS_NOT_FOUND;

            for (Index = 0;
                Index < Modules->NumberOfModules;
                Index++) {
                _splitpath(
                    Modules->Modules[Index].FullPathName,
                    NULL,
                    NULL,
                    FullName,
                    NULL);

                _splitpath(
                    Modules->Modules[Index].FullPathName,
                    NULL,
                    NULL,
                    NULL,
                    FullName + strlen(FullName));

                RtlInitString(&String, FullName);

                if (FALSE != RtlPrefixString(
                    ImageFileName,
                    &String,
                    TRUE)) {
                    *ImageBase = Modules->Modules[Index].ImageBase;

                    Status = LdrMapSectionOffline(
                        Modules->Modules[Index].FullPathName,
                        ImportViewBase);

                    break;
                }
            }
        }

        __free(Modules);

        Modules = NULL;

        if (STATUS_INFO_LENGTH_MISMATCH == Status) {
            BufferSize = ReturnLength;

            goto retry;
        }
    }
    else {
        Status = STATUS_NO_MEMORY;
    }

    return Status;
}

ptr
NTAPI
LdrForwardOffline(
    __in cptr ForwarderData
)
{
    status Status = STATUS_SUCCESS;
    ptr ImageBase = NULL;
    ptr ImageViewBase = NULL;
    cptr Separator = NULL;
    cptr ImageName = NULL;
    cptr ProcedureName = NULL;
    u32 ProcedureNumber = 0;
    ptr ProcedureAddress = NULL;
    STRING String = { 0 };

    Separator = strchr(ForwarderData, '.');

    if (NULL != Separator) {
        ImageName = __malloc(Separator - ForwarderData);

        if (NULL != ImageName) {
            RtlCopyMemory(
                ImageName,
                ForwarderData,
                Separator - ForwarderData);

            String.Buffer = ImageName;
            String.Length = Separator - ForwarderData;
            String.MaximumLength = Separator - ForwarderData;

            Status = LdrLoadImportOffline(
                &String,
                &ImageBase,
                &ImageViewBase);

            if (TRACE(Status)) {
                Status = STATUS_NO_MORE_ENTRIES;

                Separator += 1;
                ProcedureName = Separator;

                if (Separator[0] != '@') {
                    ProcedureAddress = LdrGetSymbolOffline(
                        ImageBase,
                        ImageViewBase,
                        ProcedureName,
                        0);
                }
                else {
                    Separator += 1;

                    if (RtlCharToInteger(
                        Separator,
                        0,
                        &ProcedureNumber) >= 0) {
                        ProcedureAddress = LdrGetSymbolOffline(
                            ImageBase,
                            ImageViewBase,
                            NULL,
                            ProcedureNumber);
                    }
                }

                TRACE(NtUnmapViewOfSection(
                    NtCurrentProcess(),
                    ImageBase));
            }

            __free(ImageName);
        }
    }

    return ProcedureAddress;
}

ptr
NTAPI
LdrGetSymbolOffline(
    __in ptr ImageBase,
    __in ptr ImageViewBase,
    __in_opt cptr ProcedureName,
    __in_opt u32 ProcedureNumber
)
{
    PIMAGE_EXPORT_DIRECTORY ExportDirectory = NULL;
    u32 Size = 0;
    u32ptr NameTable = NULL;
    u16ptr OrdinalTable = NULL;
    u32ptr AddressTable = NULL;
    cptr NameTableName = NULL;
    u16 HintIndex = 0;
    ptr ProcedureAddress = NULL;

    ExportDirectory = RtlImageDirectoryEntryToData(
        ImageViewBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &Size);

    if (NULL != ExportDirectory) {
        NameTable = (u8ptr)ImageViewBase + ExportDirectory->AddressOfNames;
        OrdinalTable = (u8ptr)ImageViewBase + ExportDirectory->AddressOfNameOrdinals;
        AddressTable = (u8ptr)ImageViewBase + ExportDirectory->AddressOfFunctions;

        if (NULL != NameTable &&
            NULL != OrdinalTable &&
            NULL != AddressTable) {
            if (ProcedureNumber >= ExportDirectory->Base &&
                ProcedureNumber < MAXSHORT) {
                ProcedureAddress = (u8ptr)ImageBase
                    + AddressTable[ProcedureNumber - ExportDirectory->Base];
            }
            else {
                for (HintIndex = 0;
                    HintIndex < ExportDirectory->NumberOfNames;
                    HintIndex++) {
                    NameTableName = (u8ptr)ImageViewBase + NameTable[HintIndex];

                    if (0 == _stricmp(
                        ProcedureName,
                        NameTableName)) {
                        ProcedureAddress = (u8ptr)ImageBase
                            + AddressTable[OrdinalTable[HintIndex]];
                    }
                }
            }
        }

        if ((u)ProcedureAddress >= (u)ExportDirectory &&
            (u)ProcedureAddress < (u)ExportDirectory + Size) {
            ProcedureAddress = LdrForwardOffline(ProcedureAddress);
        }
    }

    return ProcedureAddress;
}

void
NTAPI
LdrSnapThunkOffline(
    __in ptr ImageBase
)
{
    status Status = STATUS_SUCCESS;
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory = NULL;
    u32 Size = 0;
    PIMAGE_THUNK_DATA OriginalThunk = NULL;
    PIMAGE_THUNK_DATA Thunk = NULL;
    PIMAGE_IMPORT_BY_NAME ImportByName = NULL;
    u16 Ordinal = 0;
    cptr ImportName = NULL;
    ptr ImportBase = NULL;
    ptr ImportViewBase = NULL;
    ptr FunctionAddress = NULL;
    u32 Index = 0;
    STRING String = { 0 };

    ImportDirectory = RtlImageDirectoryEntryToData(
        ImageBase,
        TRUE,
        IMAGE_DIRECTORY_ENTRY_IMPORT,
        &Size);

    if (0 != Size) {
        do {
            OriginalThunk = (u8ptr)ImageBase + ImportDirectory->OriginalFirstThunk;
            Thunk = (u8ptr)ImageBase + ImportDirectory->FirstThunk;
            ImportName = (u8ptr)ImageBase + ImportDirectory->Name;

            RtlInitString(&String, ImportName);

            Status = LdrLoadImportOffline(&String, &ImportBase, &ImportViewBase);

            if (TRACE(Status)) {
                do {
                    if (IMAGE_SNAP_BY_ORDINAL(OriginalThunk->u1.Ordinal)) {
                        Ordinal = (u16)IMAGE_ORDINAL(OriginalThunk->u1.Ordinal);

                        FunctionAddress = LdrGetSymbolOffline(
                            ImportBase,
                            ImportViewBase,
                            NULL,
                            Ordinal);

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (u)FunctionAddress;
                        }
                        else {
                            DbgPrint(
                                "[SHARK] import procedure ordinal@%d not found\n",
                                Ordinal);
                        }
                    }
                    else {
                        ImportByName = (u8ptr)ImageBase + OriginalThunk->u1.AddressOfData;

                        FunctionAddress = LdrGetSymbolOffline(
                            ImportBase,
                            ImportViewBase,
                            ImportByName->Name,
                            0);

                        if (NULL != FunctionAddress) {
                            Thunk->u1.Function = (u)FunctionAddress;
                        }
                        else {
                            DbgPrint(
                                "[SHARK] import procedure %hs not found\n",
                                ImportByName->Name);
                        }
                    }

                    OriginalThunk++;
                    Thunk++;
                } while (OriginalThunk->u1.Function);

                TRACE(NtUnmapViewOfSection(
                    NtCurrentProcess(),
                    ImportViewBase));
            }
            else {
                DbgPrint(
                    "[SHARK] import dll %hs not found\n",
                    ImportName);
            }

            ImportDirectory++;
        } while (0 != ImportDirectory->Characteristics);
    }
}

ptr64
NTAPI
LdrGetEntryPointOffline(
    __in ptr ImageBase,
    __in ptr ViewBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    u32 Offset = 0;
    ptr64 EntryPoint = NULL;

    NtHeaders = RtlImageNtHeader(ViewBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Offset = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.AddressOfEntryPoint;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Offset = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.AddressOfEntryPoint;
        }

        if (0 != Offset) {
            EntryPoint = (u8ptr)ImageBase + Offset;
        }
    }

    return EntryPoint;
}

u32
NTAPI
LdrGetSize(
    __in ptr ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    u32 SizeOfImage = 0;

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

s
NTAPI
LdrSetImageBase(
    __in ptr ViewBase,
    __in ptr ImageBase
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    s Diff = 0;

    NtHeaders = RtlImageNtHeader(ViewBase);

    if (NULL != NtHeaders) {
        if (IMAGE_NT_OPTIONAL_HDR32_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Diff = (s)ImageBase
                - (s)((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase;

            ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase =
                (u)ImageBase;
        }

        if (IMAGE_NT_OPTIONAL_HDR64_MAGIC == NtHeaders->OptionalHeader.Magic) {
            Diff = (s64)ImageBase
                - (s64)((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase;

            ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase =
                (u64)ImageBase;
        }
    }

    return Diff;
}

status
NTAPI
SupLdrMapSection(
    __in wcptr ImageFileName,
    __out ptr * ViewBase,
    __out u * ViewSize
)
{
    status Status = STATUS_SUCCESS;
    ptr Handle = NULL;
    ptr Section = NULL;
    UNICODE_STRING FilePath = { 0 };
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    FILE_STANDARD_INFORMATION StandardInformation = { 0 };
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    LARGE_INTEGER ByteOffset = { 0 };
    ptr FileCache = NULL;
    u Index = 0;

    Status = RtlDosPathNameToNtPathName_U_WithStatus(
        ImageFileName,
        &FilePath,
        NULL,
        NULL);

    if (NT_SUCCESS(Status)) {
        InitializeObjectAttributes(
            &ObjectAttributes,
            &FilePath,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL);

        Status = NtOpenFile(
            &Handle,
            FILE_READ_DATA,
            &ObjectAttributes,
            &IoStatusBlock,
            FILE_SHARE_READ | FILE_SHARE_DELETE,
            0);

        if (NT_SUCCESS(Status)) {
            Status = NtQueryInformationFile(
                Handle,
                &IoStatusBlock,
                &StandardInformation,
                sizeof(FILE_STANDARD_INFORMATION),
                FileStandardInformation);

            if (TRACE(Status)) {
                FileCache = __malloc(StandardInformation.EndOfFile.LowPart);

                if (NULL != FileCache) {
                    Status = NtReadFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        FileCache,
                        StandardInformation.EndOfFile.LowPart,
                        &ByteOffset,
                        NULL);

                    if (NT_SUCCESS(Status)) {
                        *ViewSize = LdrGetSize(FileCache);

                        *ViewBase = __malloc(*ViewSize);

                        if (NULL != *ViewBase) {
                            NtHeaders = RtlImageNtHeader(FileCache);
                            NtSection = IMAGE_FIRST_SECTION(NtHeaders);

                            RtlCopyMemory(
                                *ViewBase,
                                FileCache,
                                NtSection->VirtualAddress);

                            for (Index = 0;
                                Index < NtHeaders->FileHeader.NumberOfSections;
                                Index++) {
                                if (0 != NtSection[Index].VirtualAddress) {
                                    RtlCopyMemory(
                                        (ptr)((u)*ViewBase + NtSection[Index].VirtualAddress),
                                        (ptr)((u)FileCache + NtSection[Index].PointerToRawData),
                                        NtSection[Index].SizeOfRawData);
                                }
                            }
                        }
                        else {
                            Status = STATUS_INSUFFICIENT_RESOURCES;
                        }
                    }

                    __free(FileCache);
                }
                else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            TRACE(NtClose(Handle));
        }

        RtlFreeUnicodeString(&FilePath);
    }

    return Status;
}

status
NTAPI
SupInstall(
    void
)
{
    status Status = STATUS_SUCCESS;
    ptr Handle = NULL;
    u DiskSize = 0;
    UNICODE_STRING ImagePath = { 0 };
    wc ImagePathBuffer[MAXIMUM_FILENAME_LENGTH] = { 0 };

    Status = RegistryOpenKey(
        &Handle,
        KEY_READ,
        RegistryString);

    if (Status < 0) {
        Status = RtlDosPathNameToNtPathName_U_WithStatus(
            SupportString,
            &ImagePath,
            NULL,
            NULL);

        if (TRACE(Status)) {
            RtlCopyMemory(
                ImagePathBuffer, ImagePath.Buffer, ImagePath.Length);

            Status = RegistryCreateSevice(
                ImagePathBuffer, ServiceString);

            RtlFreeUnicodeString(&ImagePath);
        }
    }
    else {
        Status = STATUS_NOT_SUPPORTED;
    }

    return Status;
}

void
NTAPI
SupUninstall(
    void
)
{
    TRACE(RegistryDeleteSevice(ServiceString));
}

status
NTAPI
SupInit(
    void
)
{
    status Status = STATUS_SUCCESS;
    ptr Handle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    UNICODE_STRING String = { 0 };
    SUPCOOKIE Req = { 0 };

    RtlInitUnicodeString(&String, DeviceString);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &String,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenFile(
        &Handle,
        FILE_ALL_ACCESS,
        &ObjectAttributes,
        &IoStatusBlock,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

    if (TRACE(Status)) {
        RtlFillMemory(&Req, sizeof(Req), 0xff);

        Req.Hdr.u32Cookie = SUPCOOKIE_INITIAL_COOKIE;
        Req.Hdr.u32SessionCookie = __rdtsc();
        Req.Hdr.cbIn = SUP_IOCTL_COOKIE_SIZE_IN;
        Req.Hdr.cbOut = SUP_IOCTL_COOKIE_SIZE_OUT;
        Req.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
        Req.Hdr.rc = ERR_INTERNAL_ERROR;
        Req.u.In.u32MinVersion =
            (SUPDRVIOC_VERSION & 0xffff0000) == 0x00070000 ?
            0x00070002 :
            SUPDRVIOC_VERSION & 0xffff0000;

        strcpy(Req.u.In.szMagic, SUPCOOKIE_MAGIC);

        Status = NtDeviceIoControlFile(
            Handle,
            NULL,
            NULL,
            NULL,
            &IoStatusBlock,
            SUP_IOCTL_COOKIE,
            &Req,
            SUP_IOCTL_COOKIE_SIZE_IN,
            &Req,
            SUP_IOCTL_COOKIE_SIZE_OUT);

        if (TRACE(Status)) {
            Cookie = Req.u.Out.u32Cookie;
            SessionCookie = Req.u.Out.u32SessionCookie;
            Session = Req.u.Out.pSession;
            SUPHandle = Handle;
        }
    }

    return Status;
}

void
NTAPI
SupTerm(
    void
)
{
    TRACE(NtClose(SUPHandle));

    Cookie = 0;
    SessionCookie = 0;
    Session = SUPHandle;
    SUPHandle = NULL;
}

status
NTAPI
SupLdrUnload(
    __in ptr ImageBase
)
{
    status Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    SUPLDRFREE Req = { 0 };

    Req.Hdr.u32Cookie = Cookie;
    Req.Hdr.u32SessionCookie = SessionCookie;
    Req.Hdr.cbIn = SUP_IOCTL_LDR_FREE_SIZE_IN;
    Req.Hdr.cbOut = SUP_IOCTL_LDR_FREE_SIZE_OUT;
    Req.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
    Req.Hdr.rc = ERR_INTERNAL_ERROR;
    Req.u.In.pvImageBase = ImageBase;

    Status = NtDeviceIoControlFile(
        SUPHandle,
        NULL,
        NULL,
        NULL,
        &IoStatusBlock,
        SUP_IOCTL_LDR_FREE,
        &Req,
        SUP_IOCTL_LDR_FREE_SIZE_IN,
        &Req,
        SUP_IOCTL_LDR_FREE_SIZE_OUT);

    return Status;
}

status
NTAPI
SupLdrLoad(
    __in wcptr ImageFileName,
    __in cptr BaseName,
    __in u32 Operation
)
{
    status Status = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock = { 0 };
    ptr ViewBase = NULL;
    u ViewSize = 0;
    PSUPLDRLOAD LoadReq = NULL;
    SUPLDROPEN OpenReq = { 0 };
    SUPCALLVMMR0 CallReq = { 0 };
    s Diff = 0;

    Status = SupLdrMapSection(
        ImageFileName,
        &ViewBase,
        &ViewSize);

    if (TRACE(Status)) {
        OpenReq.Hdr.u32Cookie = Cookie;
        OpenReq.Hdr.u32SessionCookie = SessionCookie;
        OpenReq.Hdr.cbIn = SUP_IOCTL_LDR_OPEN_SIZE_IN;
        OpenReq.Hdr.cbOut = SUP_IOCTL_LDR_OPEN_SIZE_OUT;
        OpenReq.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
        OpenReq.Hdr.rc = ERR_INTERNAL_ERROR;
        OpenReq.u.In.cbImage = ViewSize;

        strcpy(OpenReq.u.In.szName, BaseName);

        Status = NtDeviceIoControlFile(
            SUPHandle,
            NULL,
            NULL,
            NULL,
            &IoStatusBlock,
            SUP_IOCTL_LDR_OPEN,
            &OpenReq,
            OpenReq.Hdr.cbIn,
            &OpenReq,
            OpenReq.Hdr.cbOut);

        if (TRACE(Status)) {
            LoadReq = __malloc(SUP_IOCTL_LDR_LOAD_SIZE(ViewSize));

            if (NULL != LoadReq) {
                RtlZeroMemory(LoadReq, SUP_IOCTL_LDR_LOAD_SIZE(ViewSize));

                LoadReq->Hdr.u32Cookie = Cookie;
                LoadReq->Hdr.u32SessionCookie = SessionCookie;
                LoadReq->Hdr.cbIn = SUP_IOCTL_LDR_LOAD_SIZE_IN(ViewSize);
                LoadReq->Hdr.cbOut = SUP_IOCTL_LDR_LOAD_SIZE_OUT;
                LoadReq->Hdr.fFlags = SUPREQHDR_FLAGS_MAGIC | SUPREQHDR_FLAGS_EXTRA_IN;
                LoadReq->Hdr.rc = ERR_INTERNAL_ERROR;
                LoadReq->u.In.eEPType = SUPLDRLOADEP_VMMR0;
                LoadReq->u.In.pvImageBase = OpenReq.u.Out.pvImageBase;
                LoadReq->u.In.cbImage = ViewSize;

                RtlCopyMemory(
                    LoadReq->u.In.achImage,
                    ViewBase,
                    ViewSize);

                Diff = LdrSetImageBase(
                    LoadReq->u.In.achImage,
                    LoadReq->u.In.pvImageBase);

                LdrRelocImage(LoadReq->u.In.achImage, Diff);
                LdrSnapThunkOffline(LoadReq->u.In.achImage);

                LoadReq->u.In.pfnModuleTerm =
                    LoadReq->u.In.pfnModuleInit = LdrGetEntryPointOffline(
                        LoadReq->u.In.pvImageBase,
                        LoadReq->u.In.achImage);

                // pass unset
                LoadReq->u.In.EP.VMMR0.pvVMMR0 = (ptr)((u)OpenReq.u.Out.pvImageBase + 1);

                LoadReq->u.In.EP.VMMR0.pvVMMR0EntryInt = LdrGetEntryPointOffline(
                    LoadReq->u.In.pvImageBase,
                    LoadReq->u.In.achImage);;

                LoadReq->u.In.EP.VMMR0.pvVMMR0EntryFast = LdrGetSymbolOffline(
                    LoadReq->u.In.pvImageBase,
                    LoadReq->u.In.achImage,
                    "KernelFastCall",
                    0);

                LoadReq->u.In.EP.VMMR0.pvVMMR0EntryEx = LdrGetSymbolOffline(
                    LoadReq->u.In.pvImageBase,
                    LoadReq->u.In.achImage,
                    "KernelEntry",
                    0);

                Status = NtDeviceIoControlFile(
                    SUPHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    SUP_IOCTL_LDR_LOAD,
                    LoadReq,
                    LoadReq->Hdr.cbIn,
                    LoadReq,
                    LoadReq->Hdr.cbOut);

                if (NT_SUCCESS(Status)) {
                    CallReq.Hdr.u32Cookie = Cookie;
                    CallReq.Hdr.u32SessionCookie = SessionCookie;
                    CallReq.Hdr.cbIn = SUP_IOCTL_CALL_VMMR0_SIZE_IN(0);
                    CallReq.Hdr.cbOut = SUP_IOCTL_CALL_VMMR0_SIZE_OUT(0);
                    CallReq.Hdr.fFlags = SUPREQHDR_FLAGS_DEFAULT;
                    CallReq.Hdr.rc = ERR_INTERNAL_ERROR;
                    CallReq.u.In.pVMR0 = (ptr)((u)OpenReq.u.Out.pvImageBase
                        - MEM_FENCE_EXTRA - sizeof(SUPDRVLDRIMAGE)/* - sizeof(MEMHDR) */);
                    CallReq.u.In.uOperation = Operation;
                    CallReq.u.In.u64Arg = 0;

                    DbgPrint(
                        ".reload /i %s=%p < %p - %08x >\n",
                        BaseName,
                        OpenReq.u.Out.pvImageBase,
                        OpenReq.u.Out.pvImageBase,
                        ViewSize);

                    Status = NtDeviceIoControlFile(
                        SUPHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        SUP_IOCTL_CALL_VMMR0(0),
                        &CallReq,
                        CallReq.Hdr.cbIn,
                        &CallReq,
                        CallReq.Hdr.cbOut);

                    SupLdrUnload(OpenReq.u.Out.pvImageBase);
                }

                __free(LoadReq);
            }
            else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        __free(ViewBase);
    }

    return Status;
}
