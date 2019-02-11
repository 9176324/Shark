/*
*
* Copyright (c) 2019 by blindtiger. All rights reserved.
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

#ifndef __RVA_TO_VA
#define __RVA_TO_VA(p) ((PVOID)((PCHAR)(p) + *(PLONG)(p) + sizeof(LONG)))
#endif // !__RVA_TO_VA

#define JUMP_SELF 0x0000feebUI32

#define HOT_PATCH_HEADER "\x8b\xff"
#define HOT_PATCH_HEADER_LENGTH (sizeof(HOT_PATCH_HEADER) - 1)

#define JUMP_CODE32 "\x68\xff\xff\xff\xff\xc3"
#define JUMP_CODE64 "\xff\x25\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff"

#define JUMP_CODE32_LENGTH (sizeof(JUMP_CODE32) - 1)
#define JUMP_CODE64_LENGTH (sizeof(JUMP_CODE64) - 1)

    typedef struct _JMPCODE32 {
#pragma pack(push, 1)
        UCHAR Push[1];

        union {
            LONG JumpAddress;
        }u1;

        UCHAR Ret[1];
        UCHAR Filler[2];
#pragma pack(pop)
    }JMPCODE32, *PJMPCODE32;

    C_ASSERT(FIELD_OFFSET(JMPCODE32, u1.JumpAddress) == 1);
    C_ASSERT(sizeof(JMPCODE32) == 8);

    typedef struct _JMPCODE64 {
#pragma pack(push, 1)
        UCHAR Jmp[6];

        union {
            LONGLONG JumpAddress;
        }u1;

        UCHAR Filler[2];
#pragma pack(pop)
    }JMPCODE64, *PJMPCODE64;

    C_ASSERT(FIELD_OFFSET(JMPCODE64, u1.JumpAddress) == 6);
    C_ASSERT(sizeof(JMPCODE64) == 0x10);

#ifndef _WIN64
#define JUMP_CODE JUMP_CODE32 
#define JUMP_CODE_LENGTH JUMP_CODE32_LENGTH
    typedef JMPCODE32 JMPCODE;
    typedef JMPCODE32 *PJMPCODE;
#else
#define JUMP_CODE JUMP_CODE64
#define JUMP_CODE_LENGTH JUMP_CODE64_LENGTH
    typedef JMPCODE64 JMPCODE;
    typedef JMPCODE64 *PJMPCODE;
#endif // !_WIN64

    //////////////////////////////////////////////////////////////////////////////
    //
    //  Function:
    //      DetourCopyInstruction(PVOID pDst,
    //                            PVOID *ppDstPool
    //                            PVOID pSrc,
    //                            PVOID *ppTarget,
    //                            LONG *plExtra)
    //  Purpose:
    //      Copy a single instruction from pSrc to pDst.
    //
    //  Arguments:
    //      pDst:
    //          Destination address for the instruction.  May be NULL in which
    //          case DetourCopyInstruction is used to measure an instruction.
    //          If not NULL then the source instruction is copied to the
    //          destination instruction and any relative arguments are adjusted.
    //      ppDstPool:
    //          Destination address for the end of the constant pool.  The
    //          constant pool works backwards toward pDst.  All memory between
    //          pDst and *ppDstPool must be available for use by this function.
    //          ppDstPool may be NULL if pDst is NULL.
    //      pSrc:
    //          Source address of the instruction.
    //      ppTarget:
    //          Out parameter for any target instruction address pointed to by
    //          the instruction.  For example, a branch or a jump insruction has
    //          a target, but a load or store instruction doesn't.  A target is
    //          another instruction that may be executed as a result of this
    //          instruction.  ppTarget may be NULL.
    //      plExtra:
    //          Out parameter for the number of extra bytes needed by the
    //          instruction to reach the target.  For example, lExtra = 3 if the
    //          instruction had an 8-bit relative offset, but needs a 32-bit
    //          relative offset.
    //
    //  Returns:
    //      Returns the address of the next instruction (following in the source)
    //      instruction.  By subtracting pSrc from the return value, the caller
    //      can determinte the size of the instruction copied.
    //
    //  Comments:
    //      By following the pTarget, the caller can follow alternate
    //      instruction streams.  However, it is not always possible to determine
    //      the target based on static analysis.  For example, the destination of
    //      a jump relative to a register cannot be determined from just the
    //      instruction stream.  The output value, pTarget, can have any of the
    //      following outputs:
    //          DETOUR_INSTRUCTION_TARGET_NONE:
    //              The instruction has no targets.
    //          DETOUR_INSTRUCTION_TARGET_DYNAMIC:
    //              The instruction has a non-deterministic (dynamic) target.
    //              (i.e. the jump is to an address held in a register.)
    //          Address:   The instruction has the specified target.
    //
    //      When copying instructions, DetourCopyInstruction insures that any
    //      targets remain constant.  It does so by adjusting any IP relative
    //      offsets.
    //
    //////////////////////////////////////////////////////////////////////////////

    PVOID
        NTAPI
        DetourCopyInstruction(
            __in_opt PVOID Dst,
            __in_opt PVOID * DstPool,
            __in PVOID Src,
            __out_opt PVOID * Target,
            __out LONG * Extra
        );

    ULONG
        NTAPI
        DetourGetInstructionLength(
            __in PVOID ControlPc
        );

    VOID
        NTAPI
        DetourLockCopyInstruction(
            __in PVOID Destination,
            __in PVOID Source,
            __in ULONG Length
        );

    VOID
        NTAPI
        DetourBuildJumpCode32(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

    VOID
        NTAPI
        DetourBuildJumpCode64(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

    VOID
        NTAPI
        DetourBuildJumpCode(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

    VOID
        NTAPI
        DetourAttach(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

    VOID
        NTAPI
        DetourDetach(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

    VOID
        NTAPI
        DetourAttachHotPatch(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

    VOID
        NTAPI
        DetourDetachHotPatch(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_JUMP_H_
