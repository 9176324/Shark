/*
*
* Copyright (c) 2015 - 2021 by blindtiger. All rights reserved.
*
* The contents of this file are subject to the Mozilla Disk License Version
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

#ifndef _TYPESDEFS_H_
#define _TYPESDEFS_H_

#ifdef __cplusplus
/* Assume byte packing throughout */
extern "C" {
#endif	/* __cplusplus */

    typedef signed __int8 s8, *s8ptr;
    typedef signed __int16 s16, *s16ptr;
    typedef signed __int32 s32, *s32ptr;
    typedef signed __int64 s64, *s64ptr;

    typedef unsigned __int8 u8, *u8ptr;
    typedef unsigned __int16 u16, *u16ptr;
    typedef unsigned __int32 u32, *u32ptr;
    typedef unsigned __int64 u64, *u64ptr;

    typedef void * ptr;

    typedef unsigned __int8 c, *cptr;
    typedef unsigned __int16 wc, *wcptr;
    typedef unsigned __int8 b, *bptr;

    typedef * __ptr32 ptr32;
    typedef * __ptr64 ptr64;

#else
#ifndef _UNICODE
#define tc c
#define tcptr cptr
#else
#define tc wc
#define tcptr wcptr
#endif // !_UNICODE

#ifndef _WIN64
typedef __int32 s, *sptr;
typedef unsigned __int32 u, *uptr;

#define __utop(v) (ptr)(u32)(u)(v)
#define __ptou(v) (u32)(u)(v)
#else
typedef __int64 s, *sptr;
typedef unsigned __int64 u, *uptr;

#define __utop(v) (ptr)(u64)(u)(v)
#define __ptou(v) (u64)(u)(v)
#endif // !_WIN64

#define __rds8(p) (*(s8ptr)__utop(p) & 0xff)
#define __rds16(p) (*(s16ptr)__utop(p) & 0xffff)
#define __rds32(p) (*(s32ptr)__utop(p) & 0xffffffff)
#define __rds64(p) (*(s64ptr)__utop(p) & 0xffffffffffffffff)
#define __rdsptr(p) (*(sptr)__utop(p))

#define __rdu8(p) (*(u8ptr)__utop(p) & 0xff)
#define __rdu16(p) (*(u16ptr)__utop(p) & 0xffff)
#define __rdu32(p) (*(u32ptr)__utop(p) & 0xffffffff)
#define __rdu64(p) (*(u64ptr)__utop(p) & 0xffffffffffffffff)
#define __rduptr(p) (*(uptr)__utop(p))

#define __rdfloat(p) (*(float *)__utop(p)
#define __rddouble(p) (*(double *)__utop(p)

#define __wrs8(p, v) (*(s8ptr)__utop(p) = (s8)(v))
#define __wrs16(p, v) (*(s16ptr)__utop(p) = (s16)(v))
#define __wrs32(p, v) (*(s32ptr)__utop(p) = (s32)(v))
#define __wrs64(p, v) (*(s64ptr)__utop(p) = (s64)(v))
#define __wrsptr(p, v) (*(sptr)__utop(p) = (s)(v))

#define __wru8(p, v) (*(u8ptr)__utop(p) = (u8)(v))
#define __wru16(p, v) (*(u16ptr)__utop(p) = (u16)(v))
#define __wru32(p, v) (*(u32ptr)__utop(p) = (u32)(v))
#define __wru64(p, v) (*(u64ptr)__utop(p) = (u64)(v))
#define __wruptr(p, v) (*(uptr)__utop(p) = (u)(v))

#define __wrfloat(p, v) (*(float *)__utop(p) = (float)(v))
#define __wrdouble(p, v) (*(double *)__utop(p) = (double)(v))

#define __max(a, b) (((a) > (b)) ? (a) : (b))
#define __min(a, b) (((a) < (b)) ? (a) : (b))

#define __makeu16(a, b) (((u16)(a) & 0xff) | (((u16)(b) & 0xff) << 8))
#define __makeu32(a, b) (((u32)(a) & 0xffff) | (((u32)(b) & 0xffff) << 16))
#define __makeu64(a, b) (((u64)(a) & 0xffffffff) | (((u64)(b) & 0xffffffff) << 32))

#define __hiu8(w) ((u8)((u16)(w) >> 8))
#define __lou8(w) ((u8)((u16)(w) & 0xff))
#define __hiu16(l) ((u16)((u32)(l) >> 16))
#define __lou16(l) ((u16)((u32)(l) & 0xffff))
#define __hiu32(l) ((u32)((u64)(l) >> 32))
#define __lou32(l) ((u32)((u64)(l) & 0xffffffff))

#define __flagon(b, n) ((n) == ((b) & (n)))

#define ByteOffset(Va) ((u32)((s)(Va) & (PAGE_SIZE - 1)))
#define PageAlign(Va) ((ptr)((u)(Va) & ~(PAGE_SIZE - 1)))

#ifndef _WIN64
#define __gcall __stdcall
#else
#define __gcall __fastcall
#endif // !_WIN64

#ifndef GCALL
#define GCALL __gcall
#endif // !GCALL

typedef
u
(GCALL * PGKERNEL_ROUTINE) (
    ptr Argument1,
    ptr Argument2,
    ptr Argument3
    );

typedef
u
(GCALL * PGSYSTEM_ROUTINE) (
    ptr Argument1,
    ptr Argument2
    );

typedef
u
(GCALL * PGRUNDOWN_ROUTINE) (
    ptr Argument
    );

typedef
u
(GCALL * PGNORMAL_ROUTINE) (
    void
    );

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif // !_TYPESDEFS_H_
