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
    PWSTR KeyPathBuffer = NULL;
    UNICODE_STRING KeyPath = { 0 };
    PWSTR ImagePathBuffer = NULL;
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
            wcscpy(
                KeyPathBuffer,
                L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");

            wcscat(KeyPathBuffer, ServiceName);

            RtlInitUnicodeString(&KeyPath, KeyPathBuffer);

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

                Status = RtlDosPathNameToNtPathName_U_WithStatus(
                    ImageFileName,
                    &ImagePath,
                    NULL,
                    NULL);

                if (TRACE(Status)) {
                    ImagePathBuffer = RtlAllocateHeap(
                        RtlProcessHeap(),
                        0,
                        MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR));

                    if (NULL != ImagePathBuffer) {
                        RtlCopyMemory(
                            ImagePathBuffer,
                            ImagePath.Buffer,
                            ImagePath.Length);

                        RtlZeroMemory(
                            (PCHAR)ImagePathBuffer + ImagePath.Length,
                            sizeof(UNICODE_NULL));

                        Status = SetValueKey(
                            KeyHandle,
                            L"ImagePath",
                            REG_EXPAND_SZ,
                            ImagePathBuffer,
                            ImagePath.Length + sizeof(UNICODE_NULL));

                        RtlFreeHeap(
                            RtlProcessHeap(),
                            0,
                            ImagePathBuffer);
                    }
                    else {
                        Status = STATUS_NO_MEMORY;
                    }

                    RtlFreeUnicodeString(&ImagePath);
                }

                Status = NtLoadDriver(&KeyPath);

                TRACE(NtClose(KeyHandle));
            }

            if (FALSE == WasEnabled) {
                Status = RtlAdjustPrivilege(
                    SE_LOAD_DRIVER_PRIVILEGE,
                    FALSE,
                    FALSE,
                    &WasEnabled);
            }

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                KeyPathBuffer);
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
    PWSTR KeyPathBuffer = NULL;
    UNICODE_STRING KeyPath = { 0 };
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
            wcscpy(
                KeyPathBuffer,
                L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\");

            wcscat(KeyPathBuffer, ServiceName);

            RtlInitUnicodeString(&KeyPath, KeyPathBuffer);

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
                    TRACE(NtDeleteKey(KeyHandle));
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
