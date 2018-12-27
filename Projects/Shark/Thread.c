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

#include "Thread.h"

#include "Jump.h"
#include "Reload.h"
#include "Scan.h"

VOID
NTAPI
InitializeSystemThread(
    __inout PRELOADER_PARAMETER_BLOCK Block
)
{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    PCHAR SectionBase = NULL;
    ULONG SizeToLock = 0;
    PCHAR ControlPc = NULL;
    UNICODE_STRING RoutineString = { 0 };

#ifndef _WIN64
    ULONG Length = 0;
#else
    PVOID ImageBase = NULL;
    PRUNTIME_FUNCTION FunctionEntry = NULL;

    // 4D 8B CD                         mov     r9, r13         ; __int64
    // 4D 8B C6                         mov     r8, r14         ; __int64
    // 41 8B D7                         mov     edx, r15d       ; __int64
    // 49 8B CC                         mov     rcx, r12        ; PVOID
    // E8 AE D4 F0 FF                   call    PspCreateThread
    // EB 00                            jmp     short $+2

    CHAR NtCreateThread[] =
        "4d 8b ?? 4d 8b ?? 41 8b ?? 49 8b ?? e8 ?? ?? ?? ?? eb ??";
#endif // !_WIN64

    ControlPc = NameToAddress("NtCreateThread");

#ifndef _WIN64
    while (0 != _CmpByte(ControlPc[0], 0xc2) &&
        0 != _CmpByte(ControlPc[0], 0xc3)) {
        Length = DetourGetInstructionLength(ControlPc);

        if (5 == Length) {
            if (0 == _CmpByte(ControlPc[0], 0xe8) &&
                0 == _CmpByte(ControlPc[5], 0xeb)) {
                ReloaderBlock->PsCreateThread =
                    __RVA_TO_VA(ControlPc + Length - sizeof(LONG));

                break;
            }
        }

        ControlPc += Length;
    }
#else
    FunctionEntry = RtlLookupFunctionEntry(
        (ULONG64)ControlPc,
        (PULONG64)&ImageBase,
        NULL);

    if (NULL != FunctionEntry) {
        ControlPc = ScanBytes(
            (PCHAR)ImageBase + FunctionEntry->BeginAddress,
            (PCHAR)ImageBase + FunctionEntry->EndAddress,
            NtCreateThread);

        if (NULL != ControlPc) {
            ReloaderBlock->PsCreateThread = __RVA_TO_VA(ControlPc + 13);
        }
    }
#endif // !_WIN64
}
