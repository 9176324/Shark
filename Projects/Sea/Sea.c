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

#include "Sea.h"

#include "Support.h"

NTSTATUS
NTAPI
NtProcessStartup(
    __in PPEB PebBase
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    TCHAR ErrorString[MAXIMUM_FILENAME_LENGTH] = { 0 };

    Status = SupInstall();

    if (ST_SUCCESS(Status)) {
        Status = SupInit();

        if (ST_SUCCESS(Status)) {
            Status = SupLdrLoad(KernelString, "Shark", CmdReload | CmdPgClear);

            SupTerm();
        }
        else {
            _stprintf(
                ErrorString,
                TEXT("load driver error code < %08x >\n"),
                Status);

            MessageBox(NULL, ErrorString, TEXT("error"), MB_OK);
        }

        SupUninstall();
    }
    else {
        _stprintf(
            ErrorString,
            TEXT("load driver error code < %08x >\n"),
            Status);

        MessageBox(NULL, ErrorString, TEXT("error"), MB_OK);
    }

    return  NtTerminateProcess(
        NtCurrentProcess(),
        STATUS_SUCCESS);
}
