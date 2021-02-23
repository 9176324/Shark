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
* The Initial Developer of the Original e is blindtiger.
*
*/

#ifndef _DEFS_H_
#define _DEFS_H_

#ifndef PUBLIC
// #define PUBLIC
#endif // !PUBLIC

#define _WIN32_WINNT 0x0500

#include <typesdefs.h>
#include <statusdefs.h>
#include <listdefs.h>

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

#ifndef DEBUG
#define DEBUG
#endif // !DEBUG

#ifndef NTOS_KERNEL_RUNTIME
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#define PAGE_SIZE 0x1000

    void
        CDECL
        vDbgPrint(
            __in tcptr Format,
            ...
        );

#ifndef _DebugBreak
#define _DebugBreak \
    if (FALSE != NtCurrentPeb()->BeingDebugged) __debugbreak
#endif // !_DebugBreak

#ifndef __malloc
#define __malloc(size) RtlAllocateHeap(RtlProcessHeap(), 0, size);
#endif // !__malloc

#ifndef __free
#define __free(pointer) RtlFreeHeap(RtlProcessHeap(), 0, pointer)
#endif // !__free
#else
#include <ntos.h>
#include <zwapi.h>

#define vDbgPrint DbgPrint

#ifndef _DebugBreak
#define _DebugBreak \
    if (FALSE == KdDebuggerNotPresent) __debugbreak
#endif // !_DebugBreak

#ifndef __malloc
#define __malloc(size) ExAllocatePool(NonPagedPool, size);
#endif // !__malloc

#ifndef __free
#define __free(pointer) ExFreePool(pointer);
#endif // !__free
#endif // !NTOS_KERNEL_RUNTIME

#ifdef _WIN64
#include <wow64t.h>
#include <wow64.h>
#endif // _WIN64

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#include <arccodes.h>
#include <ntddkbd.h>
#include <ntddmou.h>

    char *
        __cdecl
        strcpy_s(
            char * strDestination,
            size_t numberOfElements,
            const char * strSource
        );

    wchar_t *
        __cdecl
        wcscpy_s(
            wchar_t * strDestination,
            size_t numberOfElements,
            const wchar_t * strSource
        );

#ifndef DEBUG
#define TRACE(exp) (((status)exp) >= 0)
#else
#ifndef NTOS_KERNEL_RUNTIME
#define TRACE(exp) \
            (((status)exp) >= 0) ? \
                TRUE : \
                (vDbgPrint( \
                    _T("[FRK] %hs[%d] %hs failed < %08x >\n"), \
                    __FILE__, \
                    __LINE__, \
                    __FUNCDNAME__, \
                    exp),/* _DebugBreak(),*/ FALSE)
#else
#define TRACE(exp) \
            (((status)exp) >= 0) ? \
                TRUE : \
                (vDbgPrint( \
                    "[FRK] %hs[%d] %hs failed < %08x >\n", \
                    __FILE__, \
                    __LINE__, \
                    __FUNCDNAME__, \
                    exp),/* _DebugBreak(),*/ FALSE)
#endif // !NTOS_KERNEL_RUNTIME
#endif // !DEBUG

#define SystemRootDirectory L"\\SystemRoot\\System32\\"
#define Wow64SystemRootDirectory L"\\SystemRoot\\SysWOW64\\"

#define ServicesDirectory L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\"

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_DEFS_H_
