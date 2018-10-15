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

#include "Scan.h"

SIZE_T
NTAPI
TrimBytes(
    __in PSTR Sig,
    __in_opt PSTR Coll,
    __in_bcount(Coll) SIZE_T CollSize,
    __out PBOOLEAN Selector
)
{
    NTSTATUS Status = STATUS_SUCCESS;
    SIZE_T Result = 0;
    PSTR Buffer = NULL;
    SIZE_T BufferSize = 0;
    CHAR Single[3] = { 0 };
    ULONG Digit = 0;
    SIZE_T Index = 0;
    ULONG Length = 0;

    Length = strlen(Sig);

    for (Index = 0;
        Index < Length;
        Index++) {
        if (0 != isxdigit(*(Sig + Index)) ||
            0 == _CmpByte(*(Sig + Index), '?')) {
            BufferSize++;
        }
    }

    if (0 != BufferSize) {
#ifndef NTOS_KERNEL_RUNTIME
        Buffer = RtlAllocateHeap(
            RtlProcessHeap(),
            0,
            BufferSize);
#else
        Buffer = ExAllocatePool(
            NonPagedPool,
            BufferSize);
#endif // !NTOS_KERNEL_RUNTIME

        if (NULL != Buffer) {
            RtlZeroMemory(
                Buffer,
                BufferSize);

            for (Index = 0;
                Index < Length;
                Index++) {
                if (0 != isxdigit(*(Sig + Index)) ||
                    0 == _CmpByte(*(Sig + Index), '?')) {
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
                            if (0 == _CmpByte(*(Buffer + Index), '?') &&
                                0 == _CmpByte(*(Buffer + Index + 1), '?')) {
                                *(Coll + Index / 2) = '?';

                                *Selector = TRUE;
                            }
                            else if (0 != isxdigit(*(Buffer + Index)) &&
                                0 != isxdigit(*(Buffer + Index + 1))) {
                                RtlCopyMemory(
                                    Single,
                                    Buffer + Index,
                                    sizeof(CHAR) * 2);

                                Status = RtlCharToInteger(
                                    Single,
                                    16,
                                    &Digit);

                                if (RTL_SOFT_ASSERT(NT_SUCCESS(Status))) {
                                    *(Coll + Index / 2) = (CHAR)Digit;

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

#ifndef NTOS_KERNEL_RUNTIME
            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                Buffer);
#else
            ExFreePool(Buffer);
#endif // !NTOS_KERNEL_RUNTIME
        }
    }

    return Result;
        }

SIZE_T
NTAPI
CompareBytes(
    __in PSTR Destination,
    __in PSTR Source,
    __in SIZE_T Length,
    __in BOOLEAN Selector
)
{
    SIZE_T Count = 0;

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
            if (0 != _CmpByte(*(Destination + Count), *(Source + Count)) &&
                0 != _CmpByte(*(Source + Count), '?')) {
                break;
            }
        }
    }

    return Count;
}

PVOID
NTAPI
ScanBytes(
    __in PSTR Begin,
    __in PSTR End,
    __in PSTR Sig
)
{
    BOOLEAN Selector = FALSE;
    PSTR Coll = NULL;
    SIZE_T CollSize = 0;
    PVOID Result = NULL;
    SIZE_T Index = 0;

    CollSize = TrimBytes(
        Sig,
        NULL,
        CollSize,
        &Selector);

    if (-1 != CollSize) {
        if ((LONG_PTR)(End - Begin - CollSize) >= 0) {
#ifndef NTOS_KERNEL_RUNTIME
            Coll = RtlAllocateHeap(
                RtlProcessHeap(),
                0,
                CollSize);
#else
            Coll = ExAllocatePool(
                NonPagedPool,
                CollSize);
#endif // !NTOS_KERNEL_RUNTIME

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

#ifndef NTOS_KERNEL_RUNTIME
                RtlFreeHeap(
                    RtlProcessHeap(),
                    0,
                    Coll);
#else
                ExFreePool(Coll);
#endif // !NTOS_KERNEL_RUNTIME
            }
        }
    }

    return Result;
        }
