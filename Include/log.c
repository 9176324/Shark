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

#include "log.h"

VOID
CDECL
vDbgPrint(
    __in PCTSTR Format,
    ...
)
{
    PTSTR Buffer = NULL;
    SIZE_T BufferSize = 0;
    va_list ArgvList = NULL;

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

        if (NULL != Buffer) {
            if (-1 != _vsntprintf(
                Buffer,
                BufferSize,
                Format,
                ArgvList)) {
                OutputDebugString(Buffer);
            }

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                Buffer);
        }
    }
}

VOID
NTAPI
PrintHexadecimal(
    __in PCTSTR Prefix,
    __in PCSTR Hexadecimal,
    __in ULONG Length
)
{
#define MAXIMUM_LINE_CHARACTERS 16
#define SINGLE_CHARACTER 3

    ULONG Index = 0;
    ULONG SubIndex = 0;
    TCHAR Line[SINGLE_CHARACTER * (MAXIMUM_LINE_CHARACTERS + 1)] = { 0 };

    for (Index = 0;
        Index < Length;
        Index++) {
        if ((MAXIMUM_LINE_CHARACTERS - 1) == SubIndex) {
            _stprintf(
                &Line[SubIndex * SINGLE_CHARACTER],
                TEXT("%02x"),
                Hexadecimal[Index] & 0xff);

            Line[SubIndex * SINGLE_CHARACTER + (SINGLE_CHARACTER - 1)] = 0;

            SubIndex = 0;

            vDbgPrint(
                TEXT("%s %s\n"),
                Prefix,
                Line);
        }
        else if ((MAXIMUM_LINE_CHARACTERS / 2 - 1) == SubIndex) {
            _stprintf(
                &Line[SubIndex * SINGLE_CHARACTER],
                TEXT("%02x-"),
                Hexadecimal[Index] & 0xff);

            SubIndex++;
        }
        else {
            _stprintf(
                &Line[SubIndex * SINGLE_CHARACTER],
                TEXT("%02x "),
                Hexadecimal[Index] & 0xff);

            SubIndex++;
        }
    }
}
