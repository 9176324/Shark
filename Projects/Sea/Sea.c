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
#include <DeviceDefs.h>

#include "Sea.h"

#include "Print.h"
#include "Sysload.h"

BOOLEAN
NTAPI
_DllMainCRTStartupForGS(
    __in PVOID DllHandle,
    __in ULONG Reason,
    __in_opt PCONTEXT Context
);

VOID
NTAPI
Startup(
    VOID
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN Result = FALSE;
    HANDLE FileHandle = NULL;
    UNICODE_STRING FilePath = { 0 };
    OBJECT_ATTRIBUTES ObjectAttributes = { 0 };
    IO_STATUS_BLOCK IoStatusBlock = { 0 };

    Result = _DllMainCRTStartupForGS(
        NtCurrentPeb()->ImageBaseAddress,
        DLL_PROCESS_ATTACH,
        NULL);

    if (FALSE != Result) {
        Status = LoadSystemImage(MANAGER_IMAGE_LINK, MANAGER_SERVICE_LINK);

        if (RTL_SOFT_ASSERT(NT_SUCCESS(Status))) {
            RtlInitUnicodeString(&FilePath, MANAGER_DEVICE_LINK);

            InitializeObjectAttributes(
                &ObjectAttributes,
                &FilePath,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

            Status = NtOpenFile(
                &FileHandle,
                FILE_ALL_ACCESS,
                &ObjectAttributes,
                &IoStatusBlock,
                FILE_SHARE_VALID_FLAGS,
                FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT);

            if (NT_SUCCESS(Status)) {
                Status = NtDeviceIoControlFile(
                    FileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    API_METHOD_DISABLE_PATCHGUARD,
                    NULL,
                    0,
                    NULL,
                    0);

                RTL_SOFT_ASSERT(NT_SUCCESS(NtClose(FileHandle)));
            }

            UnloadSystemImage(MANAGER_SERVICE_LINK);
        }
    }

    NtTerminateProcess(NtCurrentProcess(), STATUS_SUCCESS);
}
