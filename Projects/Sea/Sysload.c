/*
*
* Copyright (c) 2019 by blindtiger. All rights reserved.
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

#include <defs.h>

#include "Sysload.h"

NTSTATUS
NTAPI
CreateKey(
    __out PHANDLE KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in PCWSTR KeyList,
    __in ULONG CreateOptions
)
{
    NTSTATUS Status = STATUS_SUCCESS;
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

NTSTATUS
NTAPI
OpenKey(
    __out PHANDLE KeyHandle,
    __in ACCESS_MASK DesiredAccess,
    __in PCWSTR KeyList
)
{
    NTSTATUS Status = STATUS_SUCCESS;
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

NTSTATUS
NTAPI
SetValueKey(
    __in HANDLE KeyHandle,
    __in PCWSTR ValueName,
    __in ULONG Type,
    __in_bcount_opt(DataSize) PVOID Data,
    __in ULONG DataSize
)
{
    NTSTATUS Status = STATUS_SUCCESS;
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

VOID
NTAPI
DeleteKey(
    __in PUNICODE_STRING KeyPath
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    PKEY_BASIC_INFORMATION BasicInformation = NULL;
    PKEY_VALUE_BASIC_INFORMATION ValueBasicInformation = NULL;
    UNICODE_STRING KeyName = { 0 };
    UNICODE_STRING Separator = { 0 };
    UNICODE_STRING SubKeyName = { 0 };
    PWSTR KeyNameBuffer = NULL;
    ULONG Length = 0;
    ULONG ResultLength = 0;

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
            MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR) +
            FIELD_OFFSET(KEY_BASIC_INFORMATION, Name);

        BasicInformation = RtlAllocateHeap(
            RtlProcessHeap(),
            0,
            Length +
            MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR) +
            MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR) +
            FIELD_OFFSET(KEY_VALUE_BASIC_INFORMATION, Name));

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
                    SubKeyName.MaximumLength = (USHORT)MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

                    KeyName.Buffer = BasicInformation->Name;
                    KeyName.Length = (USHORT)BasicInformation->NameLength;
                    KeyName.MaximumLength = (USHORT)MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

                    RtlInitUnicodeString(&Separator, L"\\");

                    RtlAppendStringToString(&SubKeyName, KeyPath);
                    RtlAppendStringToString(&SubKeyName, &Separator);
                    RtlAppendStringToString(&SubKeyName, &KeyName);

                    DeleteKey(&SubKeyName);
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
                    KeyName.Length = (USHORT)ValueBasicInformation->NameLength;
                    KeyName.MaximumLength = (USHORT)MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

                    TRACE(NtDeleteValueKey(
                        KeyHandle,
                        &KeyName));
                }
            } while (NT_SUCCESS(Status));

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                BasicInformation);
        }

        TRACE(NtDeleteKey(KeyHandle));
        TRACE(NtClose(KeyHandle));
    }
}

NTSTATUS
NTAPI
LoadKernelImage(
    __in PWSTR ImageFileName,
    __in PWSTR ServiceName
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING KeyPath = { 0 };
    PWSTR KeyPathBuffer = NULL;
    UNICODE_STRING KeyName = { 0 };
    UNICODE_STRING ImagePath = { 0 };
    ULONG Type = 1;
    ULONG Start = 3;
    ULONG ErrorControl = 1;
    BOOLEAN WasEnabled = FALSE;

    Status = RtlAdjustPrivilege(
        SE_LOAD_DRIVER_PRIVILEGE,
        TRUE,
        FALSE,
        &WasEnabled);

    if (TRACE(Status)) {
        KeyPathBuffer = RtlAllocateHeap(
            RtlProcessHeap(),
            0,
            MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR));

        if (NULL != KeyPathBuffer) {
            KeyPath.Buffer = KeyPathBuffer;
            KeyPath.Length = 0;
            KeyPath.MaximumLength = MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

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
                TRACE(SetValueKey(
                    KeyHandle,
                    L"Type",
                    REG_DWORD,
                    &Type,
                    sizeof(Type)));

                TRACE(SetValueKey(
                    KeyHandle,
                    L"ErrorControl",
                    REG_DWORD,
                    &ErrorControl,
                    sizeof(ErrorControl)));

                TRACE(SetValueKey(
                    KeyHandle,
                    L"Start",
                    REG_DWORD,
                    &Start,
                    sizeof(Start)));

                TRACE(SetValueKey(
                    KeyHandle,
                    L"DisplayName",
                    REG_SZ,
                    ServiceName,
                    wcslen(ServiceName) * sizeof(WCHAR) + sizeof(UNICODE_NULL)));

                RtlInitUnicodeString(&ImagePath, ImageFileName);

                TRACE(SetValueKey(
                    KeyHandle,
                    L"ImagePath",
                    REG_EXPAND_SZ,
                    ImagePath.Buffer,
                    ImagePath.Length + sizeof(UNICODE_NULL)));

                Status = NtLoadDriver(&KeyPath);

                TRACE(NtClose(KeyHandle));
            }

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                KeyPathBuffer);
        }

        if (FALSE == WasEnabled) {
            Status = RtlAdjustPrivilege(
                SE_LOAD_DRIVER_PRIVILEGE,
                FALSE,
                FALSE,
                &WasEnabled);
        }
    }

    return Status;
}

NTSTATUS
NTAPI
UnloadKernelImage(
    __in PWSTR ServiceName
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE KeyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    UNICODE_STRING KeyPath = { 0 };
    PWSTR KeyPathBuffer = NULL;
    UNICODE_STRING KeyName = { 0 };
    BOOLEAN WasEnabled = FALSE;

    Status = RtlAdjustPrivilege(
        SE_LOAD_DRIVER_PRIVILEGE,
        TRUE,
        FALSE,
        &WasEnabled);

    if (TRACE(Status)) {
        KeyPathBuffer = RtlAllocateHeap(
            RtlProcessHeap(),
            0,
            MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR));

        if (NULL != KeyPathBuffer) {
            KeyPath.Buffer = KeyPathBuffer;
            KeyPath.Length = 0;
            KeyPath.MaximumLength = MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);

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
                    DeleteKey(&KeyPath);
                }

                TRACE(NtClose(KeyHandle));
            }

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                KeyPathBuffer);
        }

        if (FALSE == WasEnabled) {
            Status = RtlAdjustPrivilege(
                SE_LOAD_DRIVER_PRIVILEGE,
                FALSE,
                FALSE,
                &WasEnabled);
        }
    }

    return Status;
}
