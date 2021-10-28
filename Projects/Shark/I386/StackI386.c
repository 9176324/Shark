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

#include "Stack.h"

DECLSPEC_NOINLINE
u32
NTAPI
WalkFrameChain(
    __out PCALLERS Callers,
    __in u32 Count
)
{
    u32 Fp = 0;
    u32 Index = 0;
    u32 Top = 0;
    u32 Bottom = 0;

    IoGetStackLimits(&Bottom, &Top);

    _asm mov Fp, ebp;

    while (Index < Count &&
        Fp >= Bottom &&
        Fp < Top) {
        Callers[Index].Establisher = (ptr)(*(u32ptr)(Fp + 4));
        Callers[Index].EstablisherFrame = (ptr *)(Fp + 8);

        Index += 1;
        Fp = *(u32ptr)Fp;
    }

    return Index;
}
