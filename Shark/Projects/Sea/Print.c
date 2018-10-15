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

#include "Print.h"

VOID
(NTAPI * Kernel32OutputDebugString)(
    __in PCTSTR OutputString
    );

VOID
__cdecl
Print(
    __in __format_string PCTSTR Format,
    ...
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PVOID ImageBase = NULL;
    UNICODE_STRING ImageName = { 0 };
    ANSI_STRING ProcedureName = { 0 };

    PTSTR Buffer = NULL;
    SIZE_T BufferSize = 0;
    va_list ArgvList = NULL;

    if (NULL == Kernel32OutputDebugString) {
        RtlInitUnicodeString(&ImageName, L"Kernel32.dll");

        Status = LdrGetDllHandle(
            NULL,
            NULL,
            &ImageName,
            &ImageBase);

        if ((RTL_SOFT_ASSERT(NT_SUCCESS(Status)))) {
#ifndef _UNICODE
#define ProcedureString "OutputDebugStringA"
#else
#define ProcedureString "OutputDebugStringW"
#endif // !_UNICODE

            RtlInitAnsiString(&ProcedureName, ProcedureString);

#undef ProcedureString

            LdrGetProcedureAddress(
                ImageBase,
                &ProcedureName,
                0,
                (PVOID *)&Kernel32OutputDebugString);
        }
    }

    if (NULL != Kernel32OutputDebugString) {
        va_start(
            ArgvList,
            Format);

        BufferSize = _vsctprintf(
            Format,
            ArgvList);

        if (-1 != BufferSize) {
            BufferSize += sizeof(TCHAR);

            Buffer = RtlAllocateHeap(
                RtlProcessHeap(),
                0,
                BufferSize * sizeof(TCHAR));

            if (Buffer) {
                if (-1 != _vsntprintf(
                    Buffer,
                    BufferSize,
                    Format,
                    ArgvList)) {
                    Kernel32OutputDebugString(Buffer);
                }

                RtlFreeHeap(
                    RtlProcessHeap(),
                    0,
                    Buffer);
            }
        }
    }
}
