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

#ifndef _DETOUR_H_
#define _DETOUR_H_

#include "Reload.h"

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#ifndef __RVA_TO_VA
#define __RVA_TO_VA(p) ((PVOID)((PCHAR)(p) + *(PLONG)(p) + sizeof(LONG)))
#endif // !__RVA_TO_VA

#define JUMP_SELF 0x0000feebUI32

    typedef struct  _PATCH_HEADER {
        PVOID Entry;
        PVOID ProgramCounter;
        PVOID Target;
    }PATCH_HEADER, *PPATCH_HEADER;

#define RETURN_CODE32 "\xc3"
#define RETURN_CODE32_LENGTH (sizeof(RETURN_CODE32) - 1)

#define RETURN_CODE64 "\xc3"
#define RETURN_CODE64_LENGTH (sizeof(RETURN_CODE64) - 1)

#ifndef _WIN64
#define RETURN_CODE RETURN_CODE32
#define RETURN_CODE_LENGTH RETURN_CODE32_LENGTH
#else
#define RETURN_CODE RETURN_CODE64
#define RETURN_CODE_LENGTH RETURN_CODE64_LENGTH
#endif // !_WIN64

#define COUNTER_BODY_CODE32 "\x68\xff\xff\xff\xff"
#define COUNTER_BODY_CODE32_LENGTH (sizeof(COUNTER_BODY_CODE32) - 1)

#define COUNTER_BODY_CODE64 "\x68\xff\xff\xff\xff\xc7\x44\x24\x04\xff\xff\xff\xff"
#define COUNTER_BODY_CODE64_LENGTH (sizeof(COUNTER_BODY_CODE64) - 1)

#ifndef _WIN64
#define COUNTER_BODY_CODE COUNTER_BODY_CODE32
#define COUNTER_BODY_CODE_LENGTH COUNTER_BODY_CODE32_LENGTH
#else
#define COUNTER_BODY_CODE COUNTER_BODY_CODE64
#define COUNTER_BODY_CODE_LENGTH COUNTER_BODY_CODE64_LENGTH
#endif // !_WIN64

    typedef struct _COUNTER_BODY {
        CHAR Push[1];
        CHAR Address[4];

#ifdef _WIN64
        CHAR Move[4];
        CHAR AddressExtend[4];
#endif // _WIN64
    }COUNTER_BODY, *PCOUNTER_BODY;

#ifndef _WIN64
#define SetCounterBody(body, address) \
            RtlCopyMemory( \
                (body), \
                COUNTER_BODY_CODE32, \
                COUNTER_BODY_CODE32_LENGTH), \
            *(PLONG)(body)->Address = *(PLONG)&(address)

#define GetCounterBody(body, address) \
           *(PLONG)(address) = *(PLONG)(body)->Address
#else
#define SetCounterBody(body, address) \
            RtlCopyMemory( \
                (body), \
                COUNTER_BODY_CODE64, \
                COUNTER_BODY_CODE64_LENGTH), \
            *(PLONG)(body)->Address = *(PLONG)&(address), \
            *(PLONG)(body)->AddressExtend = *((PLONG)&(address) + 1)

#define GetCounterBody(body, address) \
           *(PLONG)(address) = *(PLONG)(body)->Address, \
           *((PLONG)(address) + 1) = *(PLONG)(body)->AddressExtend
#endif // !_WIN64

#define SetReturnCode(ret) \
            RtlCopyMemory((ret), RETURN_CODE, RETURN_CODE_LENGTH)

#ifndef _WIN64
#define HOTPATCH_MASK "\x8b\xff"
#define HOTPATCH_MASK_LENGTH (sizeof(HOTPATCH_MASK) - 1)

#define HOTPATCH_BODY_CODE "\xe9\xff\xff\xff\xff\xeb\xf9"
#define HOTPATCH_BODY_CODE_LENGTH (sizeof(HOTPATCH_BODY_CODE) - 1)

    typedef struct _HOTPATCH_BODY {
        CHAR Jmp[1];
        CHAR Hotpatch[4];
        CHAR JmpSelf[2];
    }HOTPATCH_BODY, *PHOTPATCH_BODY;

    C_ASSERT(sizeof(HOTPATCH_BODY) == HOTPATCH_BODY_CODE_LENGTH);

    ////////////////////////////////
    //
    // hotpatch header
    //
    ////////////////////////////////

    typedef struct  _HOTPATCH_OBJECT {
        PATCH_HEADER Header;
    }HOTPATCH_OBJECT, *PHOTPATCH_OBJECT;
#endif // !_WIN64

#define DETOUR_BODY_CODE32 "\x68\xff\xff\xff\xff\xc3"
#define DETOUR_BODY_CODE32_LENGTH (sizeof(DETOUR_BODY_CODE32) - 1)

#define DETOUR_BODY_CODE64 "\x68\xff\xff\xff\xff\xc7\x44\x24\x04\xff\xff\xff\xff\xc3"
#define DETOUR_BODY_CODE64_LENGTH (sizeof(DETOUR_BODY_CODE64) - 1)

#ifndef _WIN64
#define DETOUR_BODY_CODE DETOUR_BODY_CODE32
#define DETOUR_BODY_CODE_LENGTH DETOUR_BODY_CODE32_LENGTH
#else
#define DETOUR_BODY_CODE DETOUR_BODY_CODE64
#define DETOUR_BODY_CODE_LENGTH DETOUR_BODY_CODE64_LENGTH
#endif // !_WIN64

    typedef struct _DETOUR_BODY {
        COUNTER_BODY Detour;
        UCHAR Ret[1];
    }DETOUR_BODY, *PDETOUR_BODY;

#ifndef _WIN64
    C_ASSERT(sizeof(DETOUR_BODY) == DETOUR_BODY_CODE32_LENGTH);
#else
    C_ASSERT(sizeof(DETOUR_BODY) == DETOUR_BODY_CODE64_LENGTH);
#endif // !_WIN64

#define SetDetourBody(body, address) \
            SetCounterBody(&(body)->Detour,(address)), \
            SetReturnCode(&(body)->Ret)

    ////////////////////////////////
    //
    // detour header
    // original
    // code
    // import
    //
    ////////////////////////////////

    typedef struct  _DETOUR_OBJECT {
        PATCH_HEADER Header;

        PVOID Original;
        ULONG Length;
    }DETOUR_OBJECT, *PDETOUR_OBJECT;

    ////////////////////////////////
    //
    // guard header
    // guard code
    // original
    // code
    //
    ////////////////////////////////

    typedef
        VOID
        (NTAPI * PGUARD)(
            __in PCONTEXT Context,
            __in PVOID ProgramCounter
            );

#ifndef _WIN64
#define GuardReturn(context) (*(PULONG_PTR)UlongToPtr((context)->Esp))
#define GuardArgv(context, index) (*(PULONG_PTR)UlongToPtr((context)->Esp + 4 + 4 * (index)))
#else
#define GuardReturn(context) (*(PULONG_PTR)(context)->Rsp)
#define GuardArgv(context, index) \
            0 == (index) ? (context)->Rcx : \
            (1 == (index) ? (context)->Rdx : \
            (2 == (index) ? (context)->R8 : \
            (3 == (index) ? (context)->R9 : \
            (*(PULONG_PTR)((context)->Rsp + 8 + 8 * (index))))))
#endif // !_WIN64

    typedef struct  _GUARD_BODY {
        COUNTER_BODY Guard; // filter
        COUNTER_BODY Detour; // detour code
        COUNTER_BODY ProgramCounter; // original address

        COUNTER_BODY CaptureContext; // capture context
        CHAR Ret[1];
    }GUARD_BODY, *PGUARD_BODY;

#ifndef _WIN64
    C_ASSERT(sizeof(GUARD_BODY) == 4 * COUNTER_BODY_CODE32_LENGTH + 1);
#else
    C_ASSERT(sizeof(GUARD_BODY) == 4 * COUNTER_BODY_CODE64_LENGTH + 1);
#endif // !_WIN64

#define SetGuardBody(body, guard, detour, programcounter, capturecontext) \
            SetCounterBody(&(body)->Guard,(guard)), \
            SetCounterBody(&(body)->Detour,(detour)), \
            SetCounterBody(&(body)->ProgramCounter,(programcounter)), \
            SetCounterBody(&(body)->CaptureContext,(capturecontext)), \
            SetReturnCode(&(body)->Ret)

    typedef struct  _GUARD_OBJECT {
        PATCH_HEADER Header;

        PVOID Original;
        ULONG Length;

        GUARD_BODY Body;
    }GUARD_OBJECT, *PGUARD_OBJECT;

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

    DECLSPEC_NORETURN
        VOID
        __stdcall
        _CaptureContext(
            __in ULONG ProgramCounter,
            __in PVOID Detour,
            __in PGUARD Guard
        );

    ULONG
        NTAPI
        DetourGetInstructionLength(
            __in PVOID ControlPc
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
            __in PGUARD Guard
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

#endif // !_DETOUR_H_
