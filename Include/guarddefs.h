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

#ifndef _GUARDDEFS_H_
#define _GUARDDEFS_H_

#include <typesdefs.h>

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#ifndef __rva_to_va_ex
#define __rva_to_va_ex(p, offset) \
            ((ptr)((s8ptr)(p) + *(s32ptr)(p) + sizeof(s32) + (offset)))
#endif // !__rva_to_va_ex

#ifndef __rva_to_va
#define __rva_to_va(p) __rva_to_va_ex((p), 0)
#endif // !__rva_to_va

#define JUMP_SELF 0x0000feebUI32

    typedef struct  _PATCH_HEADER {
        u Length;
        ptr Entry;
        ptr ProgramCounter;
        ptr Target;
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
        u8 Push[1];
        u8 Address[4];

#ifdef _WIN64
        u8 Move[4];
        u8 AddressExtend[4];
#endif // _WIN64
    }COUNTER_BODY, *PCOUNTER_BODY;

#ifndef _WIN64
#define SetCounterBody(body, address) \
            *(s32ptr)(body)->Address = *(s32ptr)&(address), \
            (body)->Push[0] = 0x68

#define GetCounterBody(body, address) \
           *(s32ptr)(address) = *(s32ptr)(body)->Address
#else
#define SetCounterBody(body, address) \
            *(s32ptr)(body)->Address = *(s32ptr)&(address), \
            *(s32ptr)(body)->AddressExtend = *((s32ptr)&(address) + 1), \
            (body)->Move[0] = 0xc7, \
            (body)->Move[1] = 0x44, \
            (body)->Move[2] = 0x24, \
            (body)->Move[3] = 0x04, \
            (body)->Push[0] = 0x68

#define GetCounterBody(body, address) \
           *(s32ptr)(address) = *(s32ptr)(body)->Address, \
           *((s32ptr)(address) + 1) = *(s32ptr)(body)->AddressExtend
#endif // !_WIN64

#define SetReturnCode(ret) \
            RtlCopyMemory((ret), RETURN_CODE, RETURN_CODE_LENGTH)

#ifndef _WIN64
#define HOTPATCH_MASK "\x8b\xff"
#define HOTPATCH_MASK_LENGTH (sizeof(HOTPATCH_MASK) - 1)

#define HOTPATCH_BODY_CODE "\xe9\xff\xff\xff\xff\xeb\xf9"
#define HOTPATCH_BODY_CODE_LENGTH (sizeof(HOTPATCH_BODY_CODE) - 1)

    typedef struct _HOTPATCH_BODY {
        u8 Jmp[1];
        u8 Hotpatch[4];
        u8 JmpSelf[2];
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

#define GUARD_BODY_CODE32 "\x68\xff\xff\xff\xff\xc3"
#define GUARD_BODY_CODE32_LENGTH (sizeof(GUARD_BODY_CODE32) - 1)

#define GUARD_BODY_CODE64 "\x68\xff\xff\xff\xff\xc7\x44\x24\x04\xff\xff\xff\xff\xc3"
#define GUARD_BODY_CODE64_LENGTH (sizeof(GUARD_BODY_CODE64) - 1)

#ifndef _WIN64
#define GUARD_BODY_CODE GUARD_BODY_CODE32
#define GUARD_BODY_CODE_LENGTH GUARD_BODY_CODE32_LENGTH
#else
#define GUARD_BODY_CODE GUARD_BODY_CODE64
#define GUARD_BODY_CODE_LENGTH GUARD_BODY_CODE64_LENGTH
#endif // !_WIN64

    typedef struct _GUARD_BODY {
        COUNTER_BODY Guard;
        u8 Ret[1];
    }GUARD_BODY, *PGUARD_BODY;

#ifndef _WIN64
    C_ASSERT(sizeof(GUARD_BODY) == GUARD_BODY_CODE32_LENGTH);
#else
    C_ASSERT(sizeof(GUARD_BODY) == GUARD_BODY_CODE64_LENGTH);
#endif // !_WIN64

#define SetGuardBody(body, address) \
            SetCounterBody(&(body)->Guard,(address)), \
            SetReturnCode(&(body)->Ret)

    ////////////////////////////////
    //
    // patch header
    // original
    // code
    // import
    //
    ////////////////////////////////

    typedef struct  _GUARD_OBJECT {
        PATCH_HEADER Header;

        ptr Original;
        u32 Length;
    }GUARD_OBJECT, *PGUARD_OBJECT;

    ////////////////////////////////
    //
    // stack header
    // stack code
    // original
    // code
    //
    ////////////////////////////////

    typedef
        void
        (NTAPI * PGUARD_CALLBACK)(
            __in PCONTEXT Context,
            __in_opt ptr ProgramCounter,
            __in_opt ptr Parameter,
            __in_opt ptr Reserved
            );

#ifndef _WIN64
#define GuardReturn(context) (*(uptr)__utop((context)->Esp))
#define GuardArgv(context, index) (*(uptr)__utop((context)->Esp + 4 + 4 * (index)))
#else
#define GuardReturn(context) (*(uptr)(context)->Rsp)
#define GuardArgv(context, index) \
            0 == (index) ? (context)->Rcx : \
            (1 == (index) ? (context)->Rdx : \
            (2 == (index) ? (context)->R8 : \
            (3 == (index) ? (context)->R9 : \
            (*(uptr)((context)->Rsp + 8 + 8 * (index))))))
#endif // !_WIN64

    typedef struct  _SAFEGUARD_BODY {
        COUNTER_BODY Reserved; // reserved
        COUNTER_BODY Parameter; // parameter
        COUNTER_BODY Callback; // filter
        COUNTER_BODY Guard; // guard code
        COUNTER_BODY ProgramCounter; // original address
        COUNTER_BODY CaptureContext; // capture context
        u8 Ret[1];
    }SAFEGUARD_BODY, *PSAFEGUARD_BODY;

#ifndef _WIN64
    C_ASSERT(sizeof(SAFEGUARD_BODY) == 6 * COUNTER_BODY_CODE32_LENGTH + 1);
#else
    C_ASSERT(sizeof(SAFEGUARD_BODY) == 6 * COUNTER_BODY_CODE64_LENGTH + 1);
#endif // !_WIN64

#define SetStackBody( \
            body, reserved, parameter, callback, guard, programcounter, capturecontext) \
                SetCounterBody(&((PSAFEGUARD_BODY)(body))->Reserved, (reserved)), \
                SetCounterBody(&((PSAFEGUARD_BODY)(body))->Parameter, (parameter)), \
                SetCounterBody(&((PSAFEGUARD_BODY)(body))->Callback, (callback)), \
                SetCounterBody(&((PSAFEGUARD_BODY)(body))->Guard, (guard)), \
                SetCounterBody(&((PSAFEGUARD_BODY)(body))->ProgramCounter, (programcounter)), \
                SetCounterBody(&((PSAFEGUARD_BODY)(body))->CaptureContext, (capturecontext)), \
                SetReturnCode(&((PSAFEGUARD_BODY)(body))->Ret)

    typedef struct  _SAFEGUARD_OBJECT {
        PATCH_HEADER Header;

        ptr Original;
        u32 Length;

        SAFEGUARD_BODY Body;
    }SAFEGUARD_OBJECT, *PSAFEGUARD_OBJECT;

#if defined(DETOURS_X86)
#elif defined(DETOURS_X64)
#elif defined(DETOURS_IA64)
#elif defined(DETOURS_ARM)
#else
#error Must define one of DETOURS_X86, DETOURS_X64, DETOURS_IA64, or DETOURS_ARM
#endif

    u32
        NTAPI
        DetourGetInstructionLength(
            __in ptr ControlPc
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_GUARDDEFS_H_
