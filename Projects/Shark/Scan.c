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

#include "Scan.h"

u
NTAPI
TrimBytes(
    __in u8ptr Sig,
    __in_opt u8ptr Coll,
    __in_bcount(Coll) u CollSize,
    __out bptr Selector
)
{
    status Status = STATUS_SUCCESS;
    u Result = 0;
    u8ptr Buffer = NULL;
    u BufferSize = 0;
    u8 Single[3] = { 0 };
    u32 Digit = 0;
    u Index = 0;
    u32 Length = 0;

    Length = strlen(Sig);

    for (Index = 0;
        Index < Length;
        Index++) {
        if (0 != isxdigit(*(Sig + Index)) ||
            0 == _cmpbyte(*(Sig + Index), '?')) {
            BufferSize++;
        }
    }

    if (0 != BufferSize) {
        Buffer = __malloc(BufferSize);

        if (NULL != Buffer) {
            RtlZeroMemory(
                Buffer,
                BufferSize);

            for (Index = 0;
                Index < Length;
                Index++) {
                if (0 != isxdigit(*(Sig + Index)) ||
                    0 == _cmpbyte(*(Sig + Index), '?')) {
                    RtlCopyMemory(
                        Buffer + strlen(Buffer),
                        Sig + Index,
                        2);

                    Index++;
                }
            }

            if (0 != (BufferSize & 1)) {
                Result = -1;
            }
            else {
                if (NULL == Coll) {
                    Result = BufferSize / 2;
                }
                else {
                    Result = BufferSize / 2;

                    if (CollSize >= BufferSize / 2) {
                        for (Index = 0;
                            Index < BufferSize;
                            Index += 2) {
                            if (0 == _cmpbyte(*(Buffer + Index), '?') &&
                                0 == _cmpbyte(*(Buffer + Index + 1), '?')) {
                                *(Coll + Index / 2) = '?';

                                *Selector = TRUE;
                            }
                            else if (0 != isxdigit(*(Buffer + Index)) &&
                                0 != isxdigit(*(Buffer + Index + 1))) {
                                RtlCopyMemory(
                                    Single,
                                    Buffer + Index,
                                    sizeof(u8) * 2);

                                Status = RtlCharToInteger(
                                    Single,
                                    16,
                                    &Digit);

                                if (TRACE(Status)) {
                                    *(Coll + Index / 2) = (u8)Digit;

                                    *Selector =
                                        FALSE != *Selector ? TRUE : FALSE;
                                }
                                else {
                                    Result = -1;

                                    break;
                                }
                            }
                            else {
                                Result = -1;

                                break;
                            }
                        }
                    }
                    else {
                        Result = -1;
                    }
                }
            }

            __free(Buffer);
        }
    }

    return Result;
}

u
NTAPI
CompareBytes(
    __in u8ptr Destination,
    __in u8ptr Source,
    __in u Length,
    __in b Selector
)
{
    u Count = 0;

    if (FALSE == Selector) {
        Count = RtlCompareMemory(
            Destination,
            Source,
            Length);
    }
    else {
        for (Count = 0;
            Count < Length;
            Count++) {
            if (0 != _cmpbyte(*(Destination + Count), *(Source + Count)) &&
                0 != _cmpbyte(*(Source + Count), '?')) {
                break;
            }
        }
    }

    return Count;
}

ptr
NTAPI
ScanBytes(
    __in u8ptr Begin,
    __in u8ptr End,
    __in u8ptr Sig
)
{
    b Selector = FALSE;
    u8ptr Coll = NULL;
    u CollSize = 0;
    ptr Result = NULL;
    u Index = 0;

    CollSize = TrimBytes(
        Sig,
        NULL,
        CollSize,
        &Selector);

    if (-1 != CollSize) {
        if ((s)(End - Begin - CollSize) >= 0) {
            Coll = __malloc(CollSize);

            if (NULL != Coll) {
                CollSize = TrimBytes(
                    Sig,
                    Coll,
                    CollSize,
                    &Selector);

                if (-1 != CollSize) {
                    for (Index = 0;
                        Index < End - Begin - CollSize;
                        Index++) {
                        if (CollSize == CompareBytes(
                            Begin + Index,
                            Coll,
                            CollSize,
                            Selector)) {
                            Result = Begin + Index;
                            break;
                        }
                    }
                }

                __free(Coll);
            }
        }
    }

    return Result;
}
