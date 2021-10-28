/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
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
* The Initial Developer of the Original Code is blindtiger.
*
*/

#ifndef _STACK_H_
#define _STACK_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef struct _CALLERS {
        ptr * EstablisherFrame;
        ptr Establisher;
    }CALLERS, *PCALLERS;

    DECLSPEC_NOINLINE
        u32
        NTAPI
        WalkFrameChain(
            __out PCALLERS Callers,
            __in u32 Count
        );

    typedef struct _SYMBOL {
        PKLDR_DATA_TABLE_ENTRY DataTableEntry;
        ptr Address;
        cptr String;
        u16 Ordinal;
        s32 Offset;
    }SYMBOL, *PSYMBOL;

    void
        NTAPI
        PrintSymbol(
            __in u8ptr Prefix,
            __in PSYMBOL Symbol
        );

    void
        NTAPI
        WalkImageSymbol(
            __in ptr Address,
            __inout PSYMBOL Symbol
        );

    void
        NTAPI
        FindSymbol(
            __in ptr Address,
            __inout PSYMBOL Symbol
        );

    void
        NTAPI
        FindAndPrintSymbol(
            __in u8ptr Prefix,
            __in ptr Address
        );

    void
        NTAPI
        PrintFrameChain(
            __in u8ptr Prefix,
            __in PCALLERS Callers,
            __in_opt u32 FramesToSkip,
            __in u32 Count
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_STACK_H_
