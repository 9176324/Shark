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

#ifndef _DETOURS_H_
#define _DETOURS_H_

#include <detoursdefs.h>

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

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
            __in_opt PVOID pDst,
            __in_opt PVOID * ppDstPool,
            __in PVOID pSrc,
            __out_opt PVOID * ppTarget,
            __out LONG * plExtra
        );

    VOID
        NTAPI
        DetourMapLockedCopyInstruction(
            __in PVOID Destination,
            __in PVOID Source,
            __in ULONG Length
        );

    VOID
        NTAPI
        DetourMapLockedBuildJumpCode(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

    VOID
        NTAPI
        DetourLockedCopyInstruction(
            __in PVOID Destination,
            __in PVOID Source,
            __in ULONG Length
        );

    VOID
        NTAPI
        DetourLockedBuildJumpCode(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

#ifndef _WIN64
    PPATCH_HEADER
        NTAPI
        HotpatchAttach(
            __inout PVOID * Pointer,
            __in PVOID Hotpatch
        );

    VOID
        NTAPI
        HotpatchDetach(
            __inout PVOID * Pointer,
            __in PPATCH_HEADER PatchHeader,
            __in PVOID Hotpatch
        );
#endif // !_WIN64

    PPATCH_HEADER
        NTAPI
        DetourAttach(
            __inout PVOID * Pointer,
            __in PVOID Detour
        );

    VOID
        NTAPI
        DetourDetach(
            __inout PVOID * Pointer,
            __in PPATCH_HEADER PatchHeader,
            __in PVOID Detour
        );

    PPATCH_HEADER
        NTAPI
        DetourGuardAttach(
            __inout PVOID * Pointer,
            __in PGUARD Guard,
            __in_opt PVOID Parameter,
            __in_opt PVOID Reserved
        );

    VOID
        NTAPI
        DetourGuardDetach(
            __inout PVOID * Pointer,
            __in PPATCH_HEADER PatchHeader,
            __in PGUARD Guard
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_DETOURS_H_
