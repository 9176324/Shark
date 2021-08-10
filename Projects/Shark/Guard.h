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

#ifndef _GUARD_H_
#define _GUARD_H_

#include <guarddefs.h>

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    //////////////////////////////////////////////////////////////////////////////
    //
    //  Function:
    //      DetourCopyInstruction(ptr pDst,
    //                            ptr *ppDstPool
    //                            ptr pSrc,
    //                            ptr *ppTarget,
    //                            s32 *plExtra)
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

    ptr
        NTAPI
        DetourCopyInstruction(
            __in_opt ptr pDst,
            __in_opt ptr * ppDstPool,
            __in ptr pSrc,
            __out_opt ptr * ppTarget,
            __out s32 * plExtra
        );

    DECLSPEC_NORETURN
        void
        NTAPI
        _CaptureContext(
            __in u32 ProgramCounter,
            __in ptr Guard,
            __in PGUARD_CALLBACK Callback,
            __in_opt ptr Parameter,
            __in_opt ptr Reserved
        );

    void
        NTAPI
        MapLockedCopyInstruction(
            __in ptr Destination,
            __in ptr Source,
            __in u32 Length
        );

    void
        NTAPI
        MapLockedBuildJumpCode(
            __inout ptr * Pointer,
            __in ptr Guard
        );

    void
        NTAPI
        LockedCopyInstruction(
            __in ptr Destination,
            __in ptr Source,
            __in u32 Length
        );

    void
        NTAPI
        LockedBuildJumpCode(
            __inout ptr * Pointer,
            __in ptr Guard
        );

    void
        NTAPI
        BuildJumpCode(
            __inout ptr * Pointer,
            __in ptr Guard
        );

#ifndef _WIN64
    PPATCH_HEADER
        NTAPI
        HotpatchAttach(
            __inout ptr * Pointer,
            __in ptr Hotpatch
        );

    void
        NTAPI
        HotpatchDetach(
            __inout ptr * Pointer,
            __in PPATCH_HEADER PatchHeader,
            __in ptr Hotpatch
        );
#endif // !_WIN64

    PPATCH_HEADER
        NTAPI
        GuardAttach(
            __inout ptr * Pointer,
            __in ptr Guard
        );

    void
        NTAPI
        GuardDetach(
            __inout ptr * Pointer,
            __in PPATCH_HEADER PatchHeader,
            __in ptr Guard
        );

    PPATCH_HEADER
        NTAPI
        SafeGuardAttach(
            __inout ptr * Pointer,
            __in PGUARD_CALLBACK Callback,
            __in_opt ptr CaptureContext,
            __in_opt ptr Parameter,
            __in_opt ptr Reserved
        );

    void
        NTAPI
        SafeGuardDetach(
            __inout ptr * Pointer,
            __in PPATCH_HEADER PatchHeader,
            __in PGUARD_CALLBACK Callback
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_GUARD_H_
