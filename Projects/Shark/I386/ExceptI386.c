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

#include "Except.h"

#include "Guard.h"
#include "Scan.h"
#include "Space.h"

void
NTAPI
InitializeExcept(
    __inout PRTB Block
)
{
    ptr ImageBase = NULL;
    PIMAGE_NT_HEADERS NtHeaders = NULL;
    PIMAGE_SECTION_HEADER NtSection = NULL;
    u32 SizeToLock = 0;
    u8ptr ControlPc = NULL;
    u8ptr TargetPc = NULL;
    u32 Length = 0;
    ptr RoutineAddress = NULL;
    u32 Count = 0;
    UNICODE_STRING String = { 0 };

    s8 RtlDispatchException[] =
        { 0xc7, 0x41, 0x04, 0x01, 0x00, 0x00, 0x00, 0x54, 0x51 };

    s8 Hotpatch[] =
        { 0xb8, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90 , 0x90 };

    s8 RtlIsValidHandler[] = "8B ?? ?? E8 ?? ?? ?? ?? 84 C0";
    s8 RtlpIsValidExceptionChain[] = "8B ?? E8 ?? ?? ?? ?? 84 C0";

    RtlInitUnicodeString(&String, L"ExRaiseStatus");

    RoutineAddress = MmGetSystemRoutineAddress(&String);

    if (NULL != RoutineAddress) {
        ControlPc = RoutineAddress;

        while (TRUE) {
            Length = DetourGetInstructionLength(ControlPc);

            if (7 == Length) {
                if (sizeof(RtlDispatchException) == RtlCompareMemory(
                    ControlPc,
                    RtlDispatchException,
                    sizeof(RtlDispatchException))) {
                    RoutineAddress =
                        __rva_to_va((ControlPc
                            + sizeof(RtlDispatchException)) + 1);

#ifdef DEBUG
                    vDbgPrint(
                        "[SHARK] < %p > RtlDispatchException\n",
                        RoutineAddress);
#endif // DEBUG

                    break;
                }
            }

            ControlPc += Length;
        }

        ControlPc = ScanBytes(
            RoutineAddress,
            (u8ptr)RoutineAddress + PAGE_SIZE,
            RtlIsValidHandler);

        if (NULL != ControlPc) {
            TargetPc = __rva_to_va(ControlPc + 4);

#ifdef DEBUG
            vDbgPrint(
                "[SHARK] < %p > RtlIsValidHandler\n",
                TargetPc);
#endif // DEBUG

            ControlPc = TargetPc;

            while (TRUE) {
                Length = DetourGetInstructionLength(ControlPc);

                if (0 == _cmpbyte(ControlPc[0], 0xc3) ||
                    0 == _cmpbyte(ControlPc[0], 0xc2)) {
                    RtlCopyMemory(
                        Hotpatch + 5,
                        ControlPc,
                        Length);

                    MapLockedCopyInstruction(
                        TargetPc,
                        Hotpatch,
                        RTL_NUMBER_OF(Hotpatch));

                    break;
                }

                ControlPc += Length;
            }
        }

        if (RtBlock.BuildNumber >= 9600) {
            ControlPc = ScanBytes(
                RoutineAddress,
                (u8ptr)RoutineAddress + PAGE_SIZE,
                RtlpIsValidExceptionChain);

            if (NULL != ControlPc) {
                TargetPc = __rva_to_va(ControlPc + 3);

#ifdef DEBUG
                vDbgPrint(
                    "[SHARK] < %p > RtlpIsValidExceptionChain\n",
                    TargetPc);
#endif // DEBUG

                ControlPc = TargetPc;

                while (TRUE) {
                    Length = DetourGetInstructionLength(ControlPc);

                    if (0 == _cmpbyte(ControlPc[0], 0xc3) ||
                        0 == _cmpbyte(ControlPc[0], 0xc2)) {
                        RtlCopyMemory(
                            Hotpatch + 5,
                            ControlPc,
                            Length);

                        MapLockedCopyInstruction(
                            TargetPc,
                            Hotpatch,
                            RTL_NUMBER_OF(Hotpatch));

                        break;
                    }

                    ControlPc += Length;
                }
            }
        }
    }
}
