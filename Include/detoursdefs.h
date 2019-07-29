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

#ifndef _DETOURSDEFS_H_
#define _DETOURSDEFS_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#define UnsafeReadChar(ptr) \
            (*(PCHAR)(ULONG_PTR)(ptr) & 0xff)

#define UnsafeReadShort(ptr) \
            (*(PSHORT)(ULONG_PTR)(ptr) & 0xffff)

#define UnsafeReadLong(ptr) \
            (*(PLONG)(ULONG_PTR)(ptr) & 0xffffffff)

#define UnsafeReadPointer(ptr) \
            (*(PULONG_PTR)(ULONG_PTR)(ptr))

#define UnsafeWriteChar(ptr, value) \
            (*(PCHAR)(ULONG_PTR)(ptr) = (CHAR)(value))

#define UnsafeWriteShort(ptr, value) \
            (*(PSHORT)(ULONG_PTR)(ptr) = (SHORT)(value))

#define UnsafeWritePointer(ptr, value) \
            (*(PULONG_PTR)(ULONG_PTR)(ptr) = (ULONG_PTR)(value))

#ifndef RvaToVa
#define RvaToVa(p) \
            ((PVOID)((PCHAR)(ULONG_PTR)(p) + \
                *(PLONG)(ULONG_PTR)(p) + \
                sizeof(LONG)))
#endif // !RvaToVa

#ifndef RvaToVaEx
#define RvaToVaEx(p, l) \
            ((PVOID)((PCHAR)(ULONG_PTR)(p) + \
                *(PLONG)(ULONG_PTR)(p) + \
                sizeof(LONG) + (l)))
#endif // !RvaToVaEx

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
            *(PLONG)(body)->Address = *(PLONG)&(address), \
            (body)->Push[0] = 0x68

#define GetCounterBody(body, address) \
           *(PLONG)(address) = *(PLONG)(body)->Address
#else
#define SetCounterBody(body, address) \
            *(PLONG)(body)->Address = *(PLONG)&(address), \
            *(PLONG)(body)->AddressExtend = *((PLONG)&(address) + 1), \
            (body)->Move[0] = 0xc7, \
            (body)->Move[1] = 0x44, \
            (body)->Move[2] = 0x24, \
            (body)->Move[3] = 0x04, \
            (body)->Push[0] = 0x68

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
            __in_opt PVOID ProgramCounter,
            __in_opt PVOID Parameter,
            __in_opt PVOID Reserved
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
        COUNTER_BODY Reserved; // reserved
        COUNTER_BODY Parameter; // parameter
        COUNTER_BODY Guard; // filter
        COUNTER_BODY Detour; // detour code
        COUNTER_BODY ProgramCounter; // original address
        COUNTER_BODY CaptureContext; // capture context
        CHAR Ret[1];
    }GUARD_BODY, *PGUARD_BODY;

#ifndef _WIN64
    C_ASSERT(sizeof(GUARD_BODY) == 6 * COUNTER_BODY_CODE32_LENGTH + 1);
#else
    C_ASSERT(sizeof(GUARD_BODY) == 6 * COUNTER_BODY_CODE64_LENGTH + 1);
#endif // !_WIN64

#define SetGuardBody( \
            body, reserved, parameter, guard, detour, programcounter, capturecontext) \
                SetCounterBody(&((PGUARD_BODY)(body))->Reserved, (reserved)), \
                SetCounterBody(&((PGUARD_BODY)(body))->Parameter, (parameter)), \
                SetCounterBody(&((PGUARD_BODY)(body))->Guard, (guard)), \
                SetCounterBody(&((PGUARD_BODY)(body))->Detour, (detour)), \
                SetCounterBody(&((PGUARD_BODY)(body))->ProgramCounter, (programcounter)), \
                SetCounterBody(&((PGUARD_BODY)(body))->CaptureContext, (capturecontext)), \
                SetReturnCode(&((PGUARD_BODY)(body))->Ret)

    typedef struct  _GUARD_OBJECT {
        PATCH_HEADER Header;

        PVOID Original;
        ULONG Length;

        GUARD_BODY Body;
    }GUARD_OBJECT, *PGUARD_OBJECT;

#if defined(DETOURS_X86)
#elif defined(DETOURS_X64)
#elif defined(DETOURS_IA64)
#elif defined(DETOURS_ARM)
#else
#error Must define one of DETOURS_X86, DETOURS_X64, DETOURS_IA64, or DETOURS_ARM
#endif

    ULONG
        NTAPI
        DetourGetInstructionLength(
            __in PVOID ControlPc
        );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_DETOURSDEFS_H_
