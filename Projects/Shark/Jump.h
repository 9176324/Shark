/*
*
* Copyright (c) 2018 by blindtiger. All rights reserved.
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

#ifndef _JUMP_H_
#define _JUMP_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#ifndef RvaToVa
#define RvaToVa(p) ((PVOID)((PCHAR)(p) + *(PLONG)(p) + sizeof(LONG)))
#endif // !RvaToVa

#define JUMP_SELF 0x0000feebUI32

#define JUMP_CODE32 "\x68\xff\xff\xff\xff\xc3"
#define JUMP_CODE64 "\xff\x25\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff"

#define JUMP_CODE32_LENGTH (sizeof(JUMP_CODE32) - 1)
#define JUMP_CODE64_LENGTH (sizeof(JUMP_CODE64) - 1)

    typedef struct _JMPCODE32 {
#pragma pack(push, 1)
        UCHAR Reserved1[1];

        union {
            UCHAR Reserved2[4];
            LONG JumpAddress;
        }u1;

        UCHAR Reserved3[1];
        UCHAR Filler[2];
#pragma pack(pop)
    }JMPCODE32, *PJMPCODE32;

    C_ASSERT(FIELD_OFFSET(JMPCODE32, u1.JumpAddress) == 1);
    C_ASSERT(sizeof(JMPCODE32) == 8);

    typedef struct _JMPCODE64 {
#pragma pack(push, 1)
        UCHAR Reserved1[6];

        union {
            UCHAR Reserved2[8];
            LONGLONG JumpAddress;
        }u1;

        UCHAR Filler[2];
#pragma pack(pop)
    }JMPCODE64, *PJMPCODE64;

    C_ASSERT(FIELD_OFFSET(JMPCODE64, u1.JumpAddress) == 6);
    C_ASSERT(sizeof(JMPCODE64) == 0x10);

#ifndef _WIN64
#define JUMP_CODE JUMP_CODE32 
    typedef JMPCODE32 JMPCODE;
    typedef JMPCODE32 *PJMPCODE;
#else
#define JUMP_CODE JUMP_CODE64
    typedef JMPCODE64 JMPCODE;
    typedef JMPCODE64 *PJMPCODE;
#endif // !_WIN64

    PVOID
        NTAPI
        DetourCopyInstruction(
            __in_opt PVOID Dst,
            __in_opt PVOID * DstPool,
            __in PVOID Src,
            __in_opt PVOID * Target,
            __in LONG * Extra
        );

    ULONG
        NTAPI
        DetourGetInstructionLength(
            __in PVOID ControlPc
        );

    NTSTATUS
        NTAPI
        BuildJumpCode32(
            __in PVOID Function,
            __inout PVOID * NewAddress
        );

    NTSTATUS
        NTAPI
        BuildJumpCode64(
            __in PVOID Function,
            __inout PVOID * NewAddress
        );

    NTSTATUS
        NTAPI
        BuildJumpCode(
            __in PVOID Function,
            __inout PVOID * NewAddress
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_JUMP_H_
