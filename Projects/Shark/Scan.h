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

#ifndef _SCAN_H_
#define _SCAN_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    b
        NTAPI
        _cmpbyte(
            __in s8 b1,
            __in s8 b2
        );

    b
        NTAPI
        _cmpword(
            __in s16 s1,
            __in s16 s2
        );

    b
        NTAPI
        _cmpdword(
            __in s32 l1,
            __in s32 l2
        );

    b
        NTAPI
        _cmpqword(
            __in s64 ll1,
            __in s64 ll2
        );

    ptr
        NTAPI
        ScanBytes(
            __in u8ptr Begin,
            __in u8ptr End,
            __in u8ptr Sig
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_SCAN_H_
