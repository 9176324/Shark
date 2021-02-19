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
* The Initial Developer of the Original e is blindtiger.
*
*/

#include <defs.h>

#include "Stack.h"

DECLSPEC_NOINLINE
ULONG
NTAPI
WalkFrameChain(
    __out PCALLERS Callers,
    __in ULONG Count
)
{
    ULONG Fp = 0;
    ULONG Index = 0;
    ULONG Top = 0;
    ULONG Bottom = 0;

    IoGetStackLimits(&Bottom, &Top);

    _asm mov Fp, ebp;

    while (Index < Count &&
        Fp >= Bottom &&
        Fp < Top) {
        Callers[Index].Establisher = (PVOID)(*(PULONG)(Fp + 4));
        Callers[Index].EstablisherFrame = (PVOID *)(Fp + 8);

        Index += 1;
        Fp = *(PULONG)Fp;
    }

    return Index;
}
