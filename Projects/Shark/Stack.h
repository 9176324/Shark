/*
*
* Copyright (c) 2015 - 2019 by blindtiger. All rights reserved.
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

#ifndef _STACK_H_
#define _STACK_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef struct _CALLERS {
        PVOID * EstablisherFrame;
        PVOID Establisher;
    }CALLERS, *PCALLERS;

    DECLSPEC_NOINLINE
        ULONG
        NTAPI
        WalkFrameChain(
            __out PCALLERS Callers,
            __in ULONG Count
        );

    typedef struct _SYMBOL {
        PKLDR_DATA_TABLE_ENTRY DataTableEntry;
        PVOID Address;
        PCHAR String;
        USHORT Ordinal;
        LONG Offset;
    }SYMBOL, *PSYMBOL;

    VOID
        NTAPI
        PrintSymbol(
            __in PCSTR Prefix,
            __in PSYMBOL Symbol
        );

    VOID
        NTAPI
        WalkImageSymbol(
            __in PVOID Address,
            __inout PSYMBOL Symbol
        );

    VOID
        NTAPI
        FindSymbol(
            __in PVOID Address,
            __inout PSYMBOL Symbol
        );

    VOID
        NTAPI
        FindAndPrintSymbol(
            __in PCSTR Prefix,
            __in PVOID Address
        );

    VOID
        NTAPI
        PrintFrameChain(
            __in PCSTR Prefix,
            __in PCALLERS Callers,
            __in_opt ULONG FramesToSkip,
            __in ULONG Count
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_STACK_H_
