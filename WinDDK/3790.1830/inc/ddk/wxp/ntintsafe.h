/******************************************************************
*                                                                 *
*  ntintsafe.h -- This module defines helper functions to prevent *
*                 integer overflow bugs for drivers. A similar    *
*                 file, intsafe.h, is available for applications. *
*                                                                 *
*  Copyright (c) Microsoft Corp.  All rights reserved.            *
*                                                                 *
******************************************************************/
#ifndef _NTINTSAFE_H_INCLUDED_
#define _NTINTSAFE_H_INCLUDED_

#if (_MSC_VER > 1000)
#pragma once
#endif

#include <specstrings.h>    // for __in, etc.

// compiletime asserts (failure results in error C2118: negative subscript)
#ifndef C_ASSERT
#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]
#endif

#if !defined(_W64)
#if !defined(__midl) && (defined(_X86_) || defined(_M_IX86)) && (_MSC_VER >= 1300)
#define _W64 __w64
#else
#define _W64
#endif
#endif

//
// typedefs
//
typedef          char       CHAR;
typedef unsigned char       UCHAR;
typedef unsigned char       BYTE;
typedef          short      SHORT;
typedef unsigned short      USHORT;
typedef unsigned short      WORD;
typedef          int        INT;
typedef unsigned int        UINT;
typedef          long       LONG;
typedef unsigned long       ULONG;
typedef unsigned long       DWORD;
typedef          __int64    RtlINT64;
typedef unsigned __int64    ULONGLONG;

#if (__midl > 501)
typedef [public]          __int3264 INT_PTR;
typedef [public] unsigned __int3264 UINT_PTR;
typedef [public]          __int3264 LONG_PTR;
typedef [public] unsigned __int3264 ULONG_PTR;
#else
#ifdef _WIN64
typedef          __int64    INT_PTR;
typedef unsigned __int64    UINT_PTR;
typedef          __int64    LONG_PTR;
typedef unsigned __int64    ULONG_PTR;
#else
typedef _W64 int            INT_PTR;
typedef _W64 unsigned int   UINT_PTR;
typedef _W64 long           LONG_PTR;
typedef _W64 unsigned long  ULONG_PTR;
#endif // WIN64
#endif // (__midl > 501)

#ifdef _WIN64
typedef          __int64    ptrdiff_t;
typedef unsigned __int64    size_t;
#else
typedef _W64 int            ptrdiff_t;
typedef _W64 unsigned int   size_t;
#endif

typedef ULONG_PTR   DWORD_PTR;
typedef LONG_PTR    SSIZE_T;
typedef ULONG_PTR   SIZE_T;



#ifndef LOWORD
#define LOWORD(_dw)     ((WORD)((DWORD_PTR)(_dw) & 0xffff))
#endif
#ifndef HIWORD
#define HIWORD(_dw)     ((WORD)(((DWORD_PTR)(_dw)) >> 16))
#endif
#ifndef LODWORD
#define LODWORD(_qw)    ((DWORD)((unsigned __int64)(_qw)))
#endif
#ifndef HIDWORD
#define HIDWORD(_qw)    ((DWORD)(((unsigned __int64)(_qw)) >> 32))
#endif

//
// AMD64 intrinsic UnsignedMultiply128 definition
//
#if defined(_AMD64_)
#ifdef __cplusplus
extern "C" {
#endif
#define UnsignedMultiply128 _umul128
ULONG64
UnsignedMultiply128 (
    __in ULONG64  Multiplier,
    __in ULONG64  Multiplicand,
    __out ULONG64 *HighProduct
    );
#pragma intrinsic(_umul128)
#ifdef __cplusplus
}
#endif
#endif // _AMD64_

//
// UInt32x32To64 macro
//
#ifndef UInt32x32To64
#if defined(MIDL_PASS) || defined(RC_INVOKED) || defined(_M_CEE_PURE) || defined(_68K_) || defined(_MPPC_) || defined(_M_IA64) || defined(_M_AMD64)
#define UInt32x32To64(a, b) (((unsigned __int64)((unsigned int)(a))) * ((unsigned __int64)((unsigned int)(b))))
#elif defined(_M_IX86)
#define UInt32x32To64(a, b) ((unsigned __int64)(((unsigned __int64)((unsigned int)(a))) * ((unsigned int)(b))))
#else
#error Must define a target architecture.
#endif
#endif // !UInt32x32To64


//
// Min/Max type values
//
#define INTSAFE_SHORT_MIN       (-32768)
#define INTSAFE_INT_MIN         (-2147483647 - 1)
#define INTSAFE_LONG_MIN        (-2147483647L - 1)
#define INTSAFE_INT64_MIN       (-9223372036854775807i64 - 1)

#ifdef _WIN64
#define INTSAFE_INT_PTR_MIN     INTSAFE_INT64_MIN
#define INTSAFE_LONG_PTR_MIN    INTSAFE_INT64_MIN
#define INTSAFE_ptrdiff_t_MIN   INTSAFE_INT64_MIN
#define INTSAFE_SSIZE_T_MIN     INTSAFE_INT64_MIN
#else
#define INTSAFE_INT_PTR_MIN     INTSAFE_INT_MIN
#define INTSAFE_LONG_PTR_MIN    INTSAFE_LONG_MIN
#define INTSAFE_ptrdiff_t_MIN   INTSAFE_INT_MIN
#define INTSAFE_SSIZE_T_MIN     INTSAFE_LONG_MIN
#endif

#define INTSAFE_BYTE_MAX        0xff
#define INTSAFE_SHORT_MAX       32767
#define INTSAFE_USHORT_MAX      0xffff
#define INTSAFE_WORD_MAX        0xffff
#define INTSAFE_INT_MAX         2147483647
#define INTSAFE_UINT_MAX        0xffffffff
#define INTSAFE_LONG_MAX        2147483647L
#define INTSAFE_ULONG_MAX       0xffffffffUL
#define INTSAFE_DWORD_MAX       0xffffffffUL
#define INTSAFE_INT64_MAX       9223372036854775807i64
#define INTSAFE_ULONGLONG_MAX   0xffffffffffffffffui64

#ifdef _WIN64
#define INTSAFE_INT_PTR_MAX     INTSAFE_INT64_MAX
#define INTSAFE_UINT_PTR_MAX    INTSAFE_ULONGLONG_MAX
#define INTSAFE_LONG_PTR_MAX    INTSAFE_INT64_MAX
#define INTSAFE_ULONG_PTR_MAX   INTSAFE_ULONGLONG_MAX
#define INTSAFE_DWORD_PTR_MAX   INTSAFE_ULONGLONG_MAX
#define INTSAFE_ptrdiff_t_MAX   INTSAFE_INT64_MAX
#define INTSAFE_size_t_MAX      INTSAFE_ULONGLONG_MAX
#define INTSAFE_SSIZE_T_MAX     INTSAFE_INT64_MAX
#define INTSAFE_SIZE_T_MAX      INTSAFE_ULONGLONG_MAX
#else
#define INTSAFE_INT_PTR_MAX     INTSAFE_INT_MAX 
#define INTSAFE_UINT_PTR_MAX    INTSAFE_UINT_MAX
#define INTSAFE_LONG_PTR_MAX    INTSAFE_LONG_MAX
#define INTSAFE_ULONG_PTR_MAX   INTSAFE_ULONG_MAX
#define INTSAFE_DWORD_PTR_MAX   INTSAFE_DWORD_MAX
#define INTSAFE_ptrdiff_t_MAX   INTSAFE_INT_MAX
#define INTSAFE_size_t_MAX      INTSAFE_UINT_MAX
#define INTSAFE_SSIZE_T_MAX     INTSAFE_LONG_MAX
#define INTSAFE_SIZE_T_MAX      INTSAFE_ULONG_MAX
#endif


//
// It is common for -1 to be used as an error value
//
#define BYTE_ERROR      0xff
#define SHORT_ERROR     (-1)
#define USHORT_ERROR    0xffff
#define WORD_ERROR      USHORT_ERROR
#define INT_ERROR       (-1)
#define UINT_ERROR      0xffffffff
#define LONG_ERROR      (-1L)
#define ULONG_ERROR     0xffffffffUL
#define DWORD_ERROR     ULONG_ERROR
#define INT64_ERROR     (-1i64)
#define ULONGLONG_ERROR (0xffffffffffffffffui64)

#ifdef _WIN64
#define INT_PTR_ERROR   INT64_ERROR
#define UINT_PTR_ERROR  ULONGLONG_ERROR
#define LONG_PTR_ERROR  INT64_ERROR
#define ULONG_PTR_ERROR ULONGLONG_ERROR
#define DWORD_PTR_ERROR ULONGLONG_ERROR
#define ptrdiff_t_ERROR INT64_ERROR
#define size_t_ERROR    ULONGLONG_ERROR
#define SSIZE_T_ERROR   INT64_ERROR
#define SIZE_T_ERROR    ULONGLONG_ERROR
#else
#define INT_PTR_ERROR   INT_ERROR 
#define UINT_PTR_ERROR  UINT_ERROR
#define LONG_PTR_ERROR  LONG_ERROR
#define ULONG_PTR_ERROR ULONG_ERROR
#define DWORD_PTR_ERROR DWORD_ERROR
#define ptrdiff_t_ERROR INT_ERROR
#define size_t_ERROR    UINT_ERROR
#define SSIZE_T_ERROR   LONG_ERROR
#define SIZE_T_ERROR    ULONG_ERROR
#endif


//
// We make some assumptions about the sizes of various types. Let's be
// explicit about those assumptions and check them.
//
C_ASSERT(sizeof(USHORT) == 2);
C_ASSERT(sizeof(INT) == 4);
C_ASSERT(sizeof(UINT) == 4);
C_ASSERT(sizeof(LONG) == 4);
C_ASSERT(sizeof(ULONG) == 4);
C_ASSERT(sizeof(UINT_PTR) == sizeof(ULONG_PTR));


// ============================================================================
// Conversion functions
//
// There are three reasons for having conversion functions:
//
// 1. We are converting from a signed type to an unsigned type of the same
//    size, or vice-versa
//
//    Since we only have unsigned math functions, this allows people to convert
//    to unsigned, do the math, and then convert back to signed
//
// 2. We are converting to a smaller type, and we could therefore possibly
//    overflow.
//
//    However, it makes no sense to have functions that convert from a
//    ULONGLONG -> BYTE (too big of a transition). If you want this then write
//    your own wrapper function that calls RtlULongLongToLong() and RtlULongToByte().
//
// 3. We are converting to a bigger type, and we are signed and the type we are
//    converting to is unsigned.
//
//=============================================================================


//
// SHORT -> UCHAR conversion
//
__inline
__checkReturn
NTSTATUS
RtlShortToUChar(
    __in SHORT sOperand,
    __out UCHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if ((sOperand >= 0) && (sOperand <= 255))
    {
        *pch = (UCHAR)sOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// SHORT -> CHAR conversion
//
#ifdef _CHAR_UNSIGNED
__forceinline
__checkReturn
NTSTATUS
RtlShortToChar(
    __in SHORT sOperand,
    __out CHAR* pch)
{
    return RtlShortToUChar(sOperand, (UCHAR*)pch);
}
#else
__inline
__checkReturn
NTSTATUS
RtlShortToChar(
    __in SHORT sOperand,
    __out CHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if ((sOperand >= -128) && (sOperand <= 127))
    {
        *pch = (CHAR)sOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}
#endif // _CHAR_UNSIGNED

//
// SHORT -> BYTE conversion
//
#define RtlShortToByte  RtlShortToUChar

//
// SHORT -> USHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlShortToUShort(
    __in SHORT sOperand,
    __out USHORT* pusResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pusResult = USHORT_ERROR;

    if (sOperand >= 0)
    {
        *pusResult = (USHORT)sOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// SHORT -> WORD conversion
//
#define RtlShortToWord RtlShortToUShort

//
// USHORT -> UCHAR conversion
//
__inline
__checkReturn
NTSTATUS
RtlUShortToUChar(
    __in USHORT usOperand,
    __out UCHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if (usOperand <= 255)
    {
        *pch = (UCHAR)usOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// USHORT -> CHAR conversion
//
#ifdef _CHAR_UNSIGNED
__forceinline
__checkReturn
NTSTATUS
RtlUShortToChar(
    __in USHORT usOperand,
    __out CHAR* pch)
{
    return RtlUShortToUChar(usOperand, (UCHAR*)pch);
}
#else
__inline
__checkReturn
NTSTATUS
RtlUShortToChar(
    __in USHORT usOperand,
    __out CHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if (usOperand <= 127)
    {
        *pch = (CHAR)usOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}
#endif // _CHAR_UNSIGNED

//
// USHORT -> BYTE conversion
//
#define RtlUShortToByte    RtlUShortToUChar

//
// USHORT -> SHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlUShortToShort(
    __in USHORT usOperand,
    __out SHORT* psResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *psResult = SHORT_ERROR;

    if (usOperand <= INTSAFE_SHORT_MAX)
    {
        *psResult = (SHORT)usOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// WORD -> CHAR conversion
//
#define RtlWordToChar  RtlUShortToChar

//
// WORD -> UCHAR conversion
//
#define RtlWordToUChar RtlUShortToUChar

//
// WORD -> BYTE conversion
//
#define RtlWordToByte  RtlUShortToByte

//
// WORD -> SHORT conversion
//
#define RtlWordToShort RtlUShortToShort

//
// INT -> UCHAR conversion
//
__inline
__checkReturn
NTSTATUS
RtlIntToUChar(
    __in INT iOperand,
    __out UCHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if ((iOperand >= 0) && (iOperand <= 255))
    {
        *pch = (UCHAR)iOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// INT -> CHAR conversion
//
#ifdef _CHAR_UNSIGNED
__forceinline
__checkReturn
NTSTATUS
RtlIntToChar(
    __in INT iOperand,
    __out CHAR* pch)
{
    return RtlIntToUChar(iOperand, (UCHAR*)pch);
}
#else
__inline
__checkReturn
NTSTATUS
RtlIntToChar(
    __in INT iOperand,
    __out CHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if ((iOperand >= -128) && (iOperand <= 127))
    {
        *pch = (CHAR)iOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}
#endif // _CHAR_UNSIGNED

//
// INT -> BYTE conversion
//
#define RtlIntToByte   RtlIntToUChar

//
// INT -> SHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlIntToShort(
    __in INT iOperand,
    __out SHORT* psResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *psResult = SHORT_ERROR;

    if ((iOperand >= INTSAFE_SHORT_MIN) && (iOperand <= INTSAFE_SHORT_MAX))
    {
        *psResult = (SHORT)iOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// INT -> USHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlIntToUShort(
    __in INT iOperand,
    __out USHORT* pusResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pusResult = USHORT_ERROR;

    if ((iOperand >= 0) && (iOperand <= INTSAFE_USHORT_MAX))
    {
        *pusResult = (USHORT)iOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// INT -> WORD conversion
//
#define RtlIntToWord   RtlIntToUShort

//
// INT -> UINT conversion
//
__inline
__checkReturn
NTSTATUS
RtlIntToUInt(
    __in INT iOperand,
    __out UINT* puResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *puResult = UINT_ERROR;

    if (iOperand >= 0)
    {
        *puResult = (UINT)iOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// INT -> UINT_PTR conversion
//
#ifdef _WIN64
#define RtlIntToUIntPtr    RtlIntToULongLong
#else
#define RtlIntToUIntPtr    RtlIntToUInt
#endif

//
// INT -> ULONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlIntToULong(
    __in INT iOperand,
    __out ULONG* pulResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pulResult = ULONG_ERROR;

    if (iOperand >= 0)
    {
        *pulResult = (ULONG)iOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// INT -> ULONG_PTR conversion
//
#ifdef _WIN64
#define RtlIntToULongPtr   RtlIntToULongLong
#else
#define RtlIntToULongPtr   RtlIntToULong
#endif

//
// INT -> DWORD conversion
//
#define RtlIntToDWord  RtlIntToULong

//
// INT -> DWORD_PTR conversion
//
#define RtlIntToDWordPtr   RtlIntToULongPtr

//
// INT -> ULONGLONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlIntToULongLong(
    __in INT iOperand,
    __out ULONGLONG* pullResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pullResult = ULONGLONG_ERROR;

    if (iOperand >= 0)
    {
        *pullResult = (ULONGLONG)iOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// INT -> size_t conversion
//
#define RtlIntToSizeT  RtlIntToUIntPtr

//
// INT -> SIZE_T conversion
//
#define RtlIntToSIZET  RtlIntToULongPtr

//
// INT_PTR -> INT conversion
//
#ifdef _WIN64
#define RtlIntPtrToInt RtlInt64ToInt
#else
__inline
__checkReturn
NTSTATUS
RtlIntPtrToInt(
    __in INT_PTR iOperand,
    __out INT* piResult)
{
    *piResult = (INT)iOperand;
    return STATUS_SUCCESS;
}
#endif

//
// INT_PTR -> UINT conversion
//
#ifdef _WIN64
#define RtlIntPtrToUInt    RtlInt64ToUInt
#else
#define RtlIntPtrToUInt    RtlIntToUInt
#endif

//
// INT_PTR -> UINT_PTR conversion
//
#ifdef _WIN64
#define RtlIntPtrToUIntPtr RtlInt64ToULongLong
#else
#define RtlIntPtrToUIntPtr RtlIntToUInt
#endif

//
// INT_PTR -> LONG conversion
//
#ifdef _WIN64
#define RtlIntPtrToLong    RtlInt64ToLong
#else
__inline
__checkReturn
NTSTATUS
RtlIntPtrToLong(
    __in INT_PTR iOperand,
    __out LONG* plResult)
{
    *plResult = (LONG)iOperand;
    return STATUS_SUCCESS;
}    
#endif

//
// INT_PTR -> ULONG conversion
//
#ifdef _WIN64
#define RtlIntPtrToULong   RtlInt64ToULong
#else
#define RtlIntPtrToULong   RtlIntToULong
#endif

//
// INT_PTR -> ULONG_PTR conversion
//
#ifdef _WIN64
#define RtlIntPtrToULongPtr    RtlInt64ToULongLong
#else
#define RtlIntPtrToULongPtr    RtlIntToULong
#endif

//
// INT_PTR -> DWORD conversion
//
#define RtlIntPtrToDWord   RtlIntPtrToULong

//    
// INT_PTR -> DWORD_PTR conversion
//
#define RtlIntPtrToDWordPtr    RtlIntPtrToULongPtr

//
// INT_PTR -> ULONGLONG conversion
//
#ifdef _WIN64
#define RtlIntPtrToULongLong   RtlInt64ToULongLong
#else
#define RtlIntPtrToULongLong   RtlIntToULongLong
#endif

//
// INT_PTR -> size_t conversion
//
#define RtlIntPtrToSizeT   RtlIntPtrToUIntPtr

//
// INT_PTR -> SIZE_T conversion
//
#define RtlIntPtrToSIZET   RtlIntPtrToULongPtr

//
// UINT -> UCHAR conversion
//
__inline
__checkReturn
NTSTATUS
RtlUIntToUChar(
    __in UINT uOperand,
    __out UCHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if (uOperand <= 255)
    {
        *pch = (UCHAR)uOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// UINT -> CHAR conversion
//
#ifdef _CHAR_UNSIGNED
__forceinline
__checkReturn
NTSTATUS
RtlUIntToChar(
    __in UINT uOperand,
    __out CHAR* pch)
{
    return RtlUIntToUChar(uOperand, (UCHAR*)pch);
}
#else
__inline
__checkReturn
NTSTATUS
RtlUIntToChar(
    __in UINT uOperand,
    __out CHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if (uOperand <= 127)
    {
        *pch = (CHAR)uOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}
#endif // _CHAR_UNSIGNED

//
// UINT -> BYTE conversion
//
#define RtlUIntToByte   RtlUIntToUChar

//
// UINT -> SHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlUIntToShort(
    __in UINT uOperand,
    __out SHORT* psResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *psResult = SHORT_ERROR;

    if (uOperand <= INTSAFE_SHORT_MAX)
    {
        *psResult = (SHORT)uOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// UINT -> USHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlUIntToUShort(
    __in UINT uOperand,
    __out USHORT* pusResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pusResult = USHORT_ERROR;

    if (uOperand <= INTSAFE_USHORT_MAX)
    {
        *pusResult = (USHORT)uOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// UINT -> WORD conversion
//
#define RtlUIntToWord  RtlUIntToUShort

//
// UINT -> INT conversion
//
__inline
__checkReturn
NTSTATUS
RtlUIntToInt(
    __in UINT uOperand,
    __out INT* piResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *piResult = INT_ERROR;

    if (uOperand <= INTSAFE_INT_MAX)
    {
        *piResult = (INT)uOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// UINT -> INT_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlUIntToIntPtr(
    __in UINT uOperand,
    __out INT_PTR* piResult)
{
    *piResult = (INT_PTR)uOperand;
    return STATUS_SUCCESS;
}
#else
#define RtlUIntToIntPtr    RtlUIntToInt
#endif

//
// UINT -> LONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlUIntToLong(
    __in UINT uOperand,
    __out LONG* plResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *plResult = LONG_ERROR;

    if (uOperand <= INTSAFE_LONG_MAX)
    {
        *plResult = (LONG)uOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// UINT -> LONG_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlUIntToLongPtr(
    __in UINT uOperand,
    __out LONG_PTR* plResult)
{
    *plResult = (LONG_PTR)uOperand;
    return STATUS_SUCCESS;
}
#else
#define RtlUIntToLongPtr   RtlUIntToLong
#endif

//
// UINT -> ptrdiff_t conversion
//
#define RtlUIntToPtrdiffT  RtlUIntToIntPtr

//
// UINT -> SSIZE_T conversion
//
#define RtlUIntToSSIZET    RtlUIntToLongPtr

//
// UINT_PTR -> INT conversion
//
#ifdef _WIN64
#define RtlUIntPtrToInt    RtlULongLongToInt
#else
#define RtlUIntPtrToInt    RtlUIntToInt
#endif

//
// UINT_PTR -> INT_PTR conversion
//
#ifdef _WIN64
#define RtlUIntPtrToIntPtr RtlULongLongToInt64
#else
#define RtlUIntPtrToIntPtr RtlUIntToInt
#endif

//
// UINT_PTR -> UINT conversion
//
#ifdef _WIN64
#define RtlUIntPtrToUInt   RtlULongLongToUInt
#else
__inline
__checkReturn
NTSTATUS
RtlUIntPtrToUInt(
    __in UINT_PTR uOperand,
    __out UINT* puResult)
{
    *puResult = (UINT)uOperand;
    return STATUS_SUCCESS;
}
#endif

//
// UINT_PTR -> LONG conversion
//
#ifdef _WIN64
#define RtlUIntPtrToLong   RtlULongLongToLong
#else
#define RtlUIntPtrToLong   RtlUIntToLong
#endif

//
// UINT_PTR -> LONG_PTR conversion
//
#ifdef _WIN64
#define RtlUIntPtrToLongPtr    RtlULongLongToInt64
#else
#define RtlUIntPtrToLongPtr    RtlUIntToLong
#endif

//
// UINT_PTR -> ULONG conversion
//
#ifdef _WIN64
#define RtlUIntPtrToULong  RtlULongLongToULong
#else
__inline
__checkReturn
NTSTATUS
RtlUIntPtrToULong(
    __in UINT_PTR uOperand,
    __out ULONG* pulResult)
{
    *pulResult = (ULONG)uOperand;
    return STATUS_SUCCESS;
}
#endif

//
// UINT_PTR -> DWORD conversion
//
#define RtlUIntPtrToDWord  RtlUIntPtrToULong

//
// UINT_PTR -> RtlINT64 conversion
//
__inline
__checkReturn
NTSTATUS
RtlUIntPtrToInt64(
    __in UINT_PTR uOperand,
    __out RtlINT64* pi64Result)
{
#ifdef _WIN64
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pi64Result = INT64_ERROR;
    
    if (uOperand <= INTSAFE_INT64_MAX)
    {
        *pi64Result = (INT64)uOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
#else
    *pi64Result = (INT64)uOperand;
    return STATUS_SUCCESS;
#endif
}

//
// UINT_PTR -> ptrdiff_t conversion
//
#define RtlUIntPtrToPtrdiffT   RtlUIntPtrToIntPtr

//
// UINT_PTR -> SSIZE_T conversion
//
#define RtlUIntPtrToSSIZET RtlUIntPtrToLongPtr

//
// LONG -> UCHAR conversion
//
__inline
__checkReturn
NTSTATUS
RtlLongToUChar(
    __in LONG lOperand,
    __out UCHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if ((lOperand >= 0) && (lOperand <= 255))
    {
        *pch = (UCHAR)lOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// LONG -> CHAR conversion
//
#ifdef _CHAR_UNSIGNED
__forceinline
__checkReturn
NTSTATUS
RtlLongToChar(
    __in LONG lOperand,
    __out CHAR* pch)
{
    return RtlLongToUChar(lOperand, (UCHAR*)pch);
}
#else
__inline
__checkReturn
NTSTATUS
RtlLongToChar(
    __in LONG lOperand,
    __out CHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if ((lOperand >= -128) && (lOperand <= 127))
    {
        *pch = (CHAR)lOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}
#endif // _CHAR_UNSIGNED

//
// LONG -> BYTE conversion
//
#define RtlLongToByte  RtlLongToUChar

//
// LONG -> SHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlLongToShort(
    __in LONG lOperand,
    __out SHORT* psResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *psResult = SHORT_ERROR;
     
    if ((lOperand >= INTSAFE_SHORT_MIN) && (lOperand <= INTSAFE_SHORT_MAX))
    {
       *psResult = (SHORT)lOperand;
       status = STATUS_SUCCESS;
    }
     
    return status;
}

//
// LONG -> USHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlLongToUShort(
    __in LONG lOperand,
    __out USHORT* pusResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pusResult = USHORT_ERROR;
    
    if ((lOperand >= 0) && (lOperand <= INTSAFE_USHORT_MAX))
    {
        *pusResult = (USHORT)lOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//   
// LONG -> WORD conversion
//
#define RtlLongToWord  RtlLongToUShort

//
// LONG -> INT conversion
//
__inline
__checkReturn
NTSTATUS
RtlLongToInt(
    __in LONG lOperand,
    __out INT* piResult)
{
    // we have C_ASSERT's above that ensures that this assignment is ok
    *piResult = (INT)lOperand;
    return STATUS_SUCCESS;
}

//
// LONG -> INT_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlLongToIntPtr(
    __in LONG lOperand,
    __out INT_PTR* piResult)
{
    *piResult = (INT_PTR)lOperand;
    return STATUS_SUCCESS;
}
#else
#define RtlLongToIntPtr    RtlLongToInt
#endif

//
// LONG -> UINT conversion
//
__inline
__checkReturn
NTSTATUS
RtlLongToUInt(
    __in LONG lOperand,
    __out UINT* puResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *puResult = UINT_ERROR;
    
    if (lOperand >= 0)
    {
        *puResult = (UINT)lOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}   

//
// LONG -> UINT_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlLongToUIntPtr(
    __in LONG lOperand,
    __out UINT_PTR* puResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *puResult = UINT_PTR_ERROR;
    
    if (lOperand >= 0)
    {
        *puResult = (UINT_PTR)lOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}
#else
#define RtlLongToUIntPtr   RtlLongToUInt
#endif

//
// LONG -> ULONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlLongToULong(
    __in LONG lOperand,
    __out ULONG* pulResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pulResult = ULONG_ERROR;
    
    if (lOperand >= 0)
    {
        *pulResult = (ULONG)lOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// LONG -> ULONG_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlLongToULongPtr(
    __in LONG lOperand,
    __out ULONG_PTR* pulResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pulResult = ULONG_PTR_ERROR;
    
    if (lOperand >= 0)
    {
        *pulResult = (ULONG_PTR)lOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}
#else
#define RtlLongToULongPtr  RtlLongToULong
#endif

//
// LONG -> DWORD conversion
//
#define RtlLongToDWord RtlLongToULong

//
// LONG -> DWORD_PTR conversion
//
#define RtlLongToDWordPtr  RtlLongToULongPtr

//
// LONG -> ULONGLONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlLongToULongLong(
    __in LONG lOperand,
    __out ULONGLONG* pullResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pullResult = ULONGLONG_ERROR;
    
    if (lOperand >= 0)
    {
        *pullResult = (ULONGLONG)lOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// LONG -> ptrdiff_t conversion
//
#define RtlLongToPtrdiffT  RtlLongToIntPtr

//
// LONG -> size_t conversion
//
#define RtlLongToSizeT RtlLongToUIntPtr

//
// LONG -> SIZE_T conversion
//
#define RtlLongToSIZET RtlLongToULongPtr

//
// LONG_PTR -> INT conversion
//
#ifdef _WIN64
#define RtlLongPtrToInt    RtlInt64ToInt
#else
#define RtlLongPtrToInt    RtlLongToInt
#endif

//
// LONG_PTR -> INT_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlLongPtrToIntPtr(
    __in LONG_PTR lOperand,
    __out INT_PTR* piResult)
{
    *piResult = (INT_PTR)lOperand;
    return STATUS_SUCCESS;
}
#else
#define RtlLongPtrToIntPtr RtlLongToInt
#endif

//
// LONG_PTR -> UINT conversion
//
#ifdef _WIN64
#define RtlLongPtrToUInt   RtlInt64ToUInt
#else
#define RtlLongPtrToUInt   RtlLongToUInt
#endif

//
// LONG_PTR -> UINT_PTR conversion
//
#ifdef _WIN64
#define RtlLongPtrToUIntPtr    RtlInt64ToULongLong
#else
#define RtlLongPtrToUIntPtr    RtlLongToUInt
#endif

//
// LONG_PTR -> LONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlLongPtrToLong(
    __in LONG_PTR lOperand,
    __out LONG* plResult)
{
#ifdef _WIN64
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *plResult = LONG_ERROR;
    
    if ((lOperand >= INTSAFE_LONG_MIN) && (lOperand <= INTSAFE_LONG_MAX))
    {
        *plResult = (LONG)lOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
#else
    *plResult = (LONG)lOperand;
    return STATUS_SUCCESS;
#endif
}

//    
// LONG_PTR -> ULONG conversion
//
#ifdef _WIN64
#define RtlLongPtrToULong  RtlInt64ToULong
#else
#define RtlLongPtrToULong  RtlLongToULong
#endif

//
// LONG_PTR -> ULONG_PTR conversion
//
#ifdef _WIN64
#define RtlLongPtrToULongPtr   RtlInt64ToULongLong
#else
#define RtlLongPtrToULongPtr   RtlLongToULong
#endif

//
// LONG_PTR -> DWORD conversion
//
#define RtlLongPtrToDWord  RtlLongPtrToULong

//
// LONG_PTR -> DWORD_PTR conversion
//
#define RtlLongPtrToDWordPtr   RtlLongPtrToULongPtr 

//
// LONG_PTR -> ULONGLONG conversion
//
#ifdef _WIN64
#define RtlLongPtrToULongLong  RtlInt64ToULongLong
#else
#define RtlLongPtrToULongLong  RtlLongToULongLong
#endif

//
// LONG_PTR -> size_t conversion
//
#define RtlLongPtrToSizeT  RtlLongPtrToUIntPtr

//
// LONG_PTR -> SIZE_T conversion
//
#define RtlLongPtrToSIZET  RtlLongPtrToULongPtr

//
// UINT -> UCHAR conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongToUChar(
    __in ULONG ulOperand,
    __out UCHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if (ulOperand <= 255)
    {
        *pch = (UCHAR)ulOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// ULONG -> CHAR conversion
//
#ifdef _CHAR_UNSIGNED
__forceinline
__checkReturn
NTSTATUS
RtlULongToChar(
    __in ULONG ulOperand,
    __out CHAR* pch)
{
    return RtlULongToUChar(ulOperand, (UCHAR*)pch);
}
#else
__inline
__checkReturn
NTSTATUS
RtlULongToChar(
    __in ULONG ulOperand,
    __out CHAR* pch)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pch = '\0';

    if (ulOperand <= 127)
    {
        *pch = (CHAR)ulOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}
#endif // _CHAR_UNSIGNED

//
// ULONG -> BYTE conversion
//
#define RtlULongToByte RtlULongToUChar

//
// ULONG -> SHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongToShort(
    __in ULONG ulOperand,
    __out SHORT* psResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *psResult = SHORT_ERROR;

    if (ulOperand <= INTSAFE_SHORT_MAX)
    {
        *psResult = (SHORT)ulOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// ULONG -> USHORT conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongToUShort(
    __in ULONG ulOperand,
    __out USHORT* pusResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pusResult = USHORT_ERROR;

    if (ulOperand <= INTSAFE_USHORT_MAX)
    {
        *pusResult = (USHORT)ulOperand;
        status = STATUS_SUCCESS;
    }

    return status;
}

//
// ULONG -> WORD conversion
//
#define RtlULongToWord RtlULongToUShort

//
// ULONG -> INT conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongToInt(
    __in ULONG ulOperand,
    __out INT* piResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *piResult = INT_ERROR;
    
    if (ulOperand <= INTSAFE_INT_MAX)
    {
        *piResult = (INT)ulOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// ULONG -> INT_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlULongToIntPtr(
    __in ULONG ulOperand,
    __out INT_PTR* piResult)
{
    *piResult = (INT_PTR)ulOperand;
    return STATUS_SUCCESS;
}
#else
#define RtlULongToIntPtr   RtlULongToInt
#endif

//
// ULONG -> UINT conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongToUInt(
    __in ULONG ulOperand,
    __out UINT* puResult)
{
    // we have C_ASSERT's above that ensures that this assignment is ok    
    *puResult = (UINT)ulOperand;    
    return STATUS_SUCCESS;
}

//
// ULONG -> UINT_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlULongToUIntPtr(
    __in LONG ulOperand,
    __out UINT_PTR* puiResult)
{
    *puiResult = (UINT_PTR)ulOperand;
    return STATUS_SUCCESS;
}
#else
#define RtlULongToUIntPtr  RtlULongToUInt
#endif

//
// ULONG -> LONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongToLong(
    __in ULONG ulOperand,
    __out LONG* plResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *plResult = LONG_ERROR;
    
    if (ulOperand <= INTSAFE_LONG_MAX)
    {
        *plResult = (LONG)ulOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// ULONG -> LONG_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlULongToLongPtr(
    __in ULONG ulOperand,
    __out LONG_PTR* plResult)
{
    *plResult = (LONG_PTR)ulOperand;
    return STATUS_SUCCESS;
}
#else
#define RtlULongToLongPtr  RtlULongToLong
#endif

//
// ULONG -> ptrdiff_t conversion
//
#define RtlULongToPtrdiffT RtlULongToIntPtr

//
// ULONG -> SSIZE_T conversion
//
#define RtlULongToSSIZET   RtlULongToLongPtr

//
// ULONG_PTR -> INT conversion
//
#ifdef _WIN64
#define RtlULongPtrToInt   RtlULongLongToInt
#else
#define RtlULongPtrToInt   RtlULongToInt
#endif

//
// ULONG_PTR -> INT_PTR conversion
//
#ifdef _WIN64
#define RtlULongPtrToIntPtr    RtlULongLongToInt64
#else
#define RtlULongPtrToIntPtr    RtlULongToInt
#endif

//
// ULONG_PTR -> UINT conversion
//
#ifdef _WIN64
#define RtlULongPtrToUInt  RtlULongLongToUInt
#else
#define RtlULongPtrToUInt  RtlULongToUInt
#endif

//
// ULONG_PTR -> UINT_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlULongPtrToUIntPtr(
    __in ULONG_PTR ulOperand,
    __out UINT_PTR* puResult)
{
    *puResult = (UINT_PTR)ulOperand;
    return STATUS_SUCCESS;
}
#else
#define RtlULongPtrToUIntPtr   RtlULongToUInt
#endif

//
// ULONG_PTR -> LONG conversion
//
#ifdef _WIN64
#define RtlULongPtrToLong  RtlULongLongToLong
#else
#define RtlULongPtrToLong  RtlULongToLong
#endif

//        
// ULONG_PTR -> LONG_PTR conversion
//
#ifdef _WIN64
#define RtlULongPtrToLongPtr   RtlULongLongToInt64
#else
#define RtlULongPtrToLongPtr   RtlULongToLong
#endif

//
// ULONG_PTR -> ULONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongPtrToULong(
    __in ULONG_PTR ulOperand,
    __out ULONG* pulResult)
{
#ifdef _WIN64
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pulResult = ULONG_ERROR;

    if (ulOperand <= INTSAFE_ULONG_MAX)
    {
        *pulResult = (ULONG)ulOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
#else
    *pulResult = (ULONG)ulOperand;
    return STATUS_SUCCESS;
#endif    
}

//
// ULONG_PTR -> DWORD conversion
//
#define RtlULongPtrToDWord RtlULongPtrToULong

//
// ULONG_PTR -> RtlINT64
//
__inline
__checkReturn
NTSTATUS
RtlULongPtrToInt64(
    __in ULONG_PTR ulOperand,
    __out RtlINT64* pi64Result)
{
#ifdef _WIN64
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pi64Result = INT64_ERROR;
    
    if (ulOperand <= INTSAFE_INT64_MAX)
    {
        *pi64Result = (INT64)ulOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
#else
    *pi64Result = (INT64)ulOperand;
    return STATUS_SUCCESS;
#endif
}

//
// ULONG_PTR -> ptrdiff_t conversion
//
#define RtlULongPtrToPtrdiffT  RtlULongPtrToIntPtr

//
// ULONG_PTR -> SSIZE_T conversion
//
#define RtlULongPtrToSSIZET    RtlULongPtrToLongPtr

//
// DWORD -> CHAR conversion
//
#define RtlDWordToChar RtlULongToChar

//
// DWORD -> UCHAR conversion
//
#define RtlDWordToUChar    RtlULongToUChar

//
// DWORD -> BYTE conversion
//
#define RtlDWordToByte RtlULongToUChar

//
// DWORD -> SHORT conversion
//
#define RtlDWordToShort    RtlULongToShort

//
// DWORD -> USHORT conversion
//
#define RtlDWordToUShort   RtlULongToUShort

//
// DWORD -> WORD conversion
//
#define RtlDWordToWord RtlULongToUShort

//
// DWORD -> INT conversion
//
#define RtlDWordToInt  RtlULongToInt

//
// DWORD -> INT_PTR conversion
//
#define RtlDWordToIntPtr   RtlULongToIntPtr

//
// DWORD -> UINT conversion
//
#define RtlDWordToUInt RtlULongToUInt

//
// DWORD -> UINT_PTR conversion
//
#define RtlDWordToUIntPtr  RtlULongToUIntPtr

//
// DWORD -> LONG conversion
//
#define RtlDWordToLong RtlULongToLong

//
// DWORD -> LONG_PTR conversion
//
#define RtlDWordToLongPtr  RtlULongToLongPtr

//
// DWORD -> ptrdiff_t conversion
//
#define RtlDWordToPtrdiffT RtlULongToIntPtr

//
// DWORD -> SSIZE_T conversion
//
#define RtlDWordToSSIZET   RtlULongToLongPtr

//
// DWORD_PTR -> INT conversion
//
#define RtlDWordPtrToInt   RtlULongPtrToInt

//
// DWORD_PTR -> INT_PTR conversion
//
#define RtlDWordPtrToIntPtr    RtlULongPtrToIntPtr

//
// DWORD_PTR -> UINT conversion
//
#define RtlDWordPtrToUInt  RtlULongPtrToUInt

//
// DWODR_PTR -> UINT_PTR conversion
//
#define RtlDWordPtrToUIntPtr   RtlULongPtrToUIntPtr

//
// DWORD_PTR -> LONG conversion
//
#define RtlDWordPtrToLong  RtlULongPtrToLong

//
// DWORD_PTR -> LONG_PTR conversion
//
#define RtlDWordPtrToLongPtr   RtlULongPtrToLongPtr

//
// DWORD_PTR -> ULONG conversion
//
#define RtlDWordPtrToULong RtlULongPtrToULong

//
// DWORD_PTR -> DWORD conversion
//
#define RtlDWordPtrToDWord RtlULongPtrToULong

//
// DWORD_PTR -> RtlINT64 conversion
//
#define RtlDWordPtrToInt64 RtlULongPtrToInt64

//
// DWORD_PTR -> ptrdiff_t conversion
//
#define RtlDWordPtrToPtrdiffT  RtlULongPtrToIntPtr

//
// DWORD_PTR -> SSIZE_T conversion
//
#define RtlDWordPtrToSSIZET    RtlULongPtrToLongPtr

//
// RtlINT64 -> INT conversion
//
__inline
__checkReturn
NTSTATUS
RtlInt64ToInt(
    __in RtlINT64 i64Operand,
    __out INT* piResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *piResult = INT_ERROR;
    
    if ((i64Operand >= INTSAFE_INT_MIN) && (i64Operand <= INTSAFE_INT_MAX))
    {
        *piResult = (INT)i64Operand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// RtlINT64 -> INT_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlInt64ToIntPtr(
    __in RtlINT64 i64Operand,
    __out INT_PTR* piResult)
{
    *piResult = (INT_PTR)i64Operand;
    return STATUS_SUCCESS;
}
#else
#define RtlInt64ToIntPtr   RtlInt64ToInt
#endif

//
// RtlINT64 -> UINT conversion
//
__inline
__checkReturn
NTSTATUS
RtlInt64ToUInt(
    __in RtlINT64 i64Operand,
    __out UINT* puResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *puResult = UINT_ERROR;
    
    if ((i64Operand >= 0) && (i64Operand <= INTSAFE_UINT_MAX))
    {
        *puResult = (UINT)i64Operand;
        status = STATUS_SUCCESS;
    }
    
    return status;    
}

//
// RtlINT64 -> UINT_PTR conversion
//
#ifdef _WIN64
#define RtlInt64ToUIntPtr  RtlInt64ToULongLong
#else
#define RtlInt64ToUIntPtr  RtlInt64ToUInt
#endif

//
// RtlINT64 -> LONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlInt64ToLong(
    __in RtlINT64 i64Operand,
    __out LONG* plResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *plResult = LONG_ERROR;
    
    if ((i64Operand >= INTSAFE_LONG_MIN) && (i64Operand <= INTSAFE_LONG_MAX))
    {
        *plResult = (LONG)i64Operand;
        status = STATUS_SUCCESS;
    }
    
    return status;    
}

//
// RtlINT64 -> LONG_PTR conversion
//
#ifdef _WIN64
#define RtlInt64ToLongPtr  RtlInt64ToIntPtr
#else
#define RtlInt64ToLongPtr  RtlInt64ToLong
#endif

//
// RtlINT64 -> ULONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlInt64ToULong(
    __in RtlINT64 i64Operand,
    __out ULONG* pulResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pulResult = ULONG_ERROR;
    
    if ((i64Operand >= 0) && (i64Operand <= INTSAFE_ULONG_MAX))
    {
        *pulResult = (ULONG)i64Operand;
        status = STATUS_SUCCESS;
    }
    
    return status;    
}

//
// RtlINT64 -> ULONG_PTR conversion
//
#ifdef _WIN64
#define RtlInt64ToULongPtr RtlInt64ToULongLong
#else
#define RtlInt64ToULongPtr RtlInt64ToULong
#endif

//
// RtlINT64 -> DWORD conversion
//
#define RtlInt64ToDWord    RtlInt64ToULong

//
// RtlINT64 -> DWORD_PTR conversion
//
#define RtlInt64ToDWordPtr RtlInt64ToULongPtr

//
// RtlINT64 -> ULONGLONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlInt64ToULongLong(
    __in RtlINT64 i64Operand,
    __out ULONGLONG* pullResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pullResult = ULONGLONG_ERROR;
    
    if (i64Operand >= 0)
    {
        *pullResult = (ULONGLONG)i64Operand;
        status = STATUS_SUCCESS;
    }
    
    return status; 
}

//
// RtlINT64 -> ptrdiff_t conversion
//
#define RtlInt64ToPtrdiffT RtlInt64ToIntPtr

//
// RtlINT64 -> size_t conversion
//
#define RtlInt64ToSizeT    RtlInt64ToUIntPtr

//
// RtlINT64 -> SSIZE_T conversion
//
#define RtlInt64ToSSIZET   RtlInt64ToLongPtr

//
// RtlINT64 -> SIZE_T conversion
//
#define RtlInt64ToSIZET    RtlInt64ToULongPtr

//
// ULONGLONG -> INT conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongLongToInt(
    __in ULONGLONG ullOperand,
    __out INT* piResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *piResult = INT_ERROR;
    
    if (ullOperand <= INTSAFE_INT_MAX)
    {
        *piResult = (INT)ullOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// ULONGLONG -> INT_PTR conversion
//
#ifdef _WIN64
#define RtlULongLongToIntPtr   RtlULongLongToInt64
#else
#define RtlULongLongToIntPtr   RtlULongLongToInt
#endif

//
// ULONGLONG -> UINT conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongLongToUInt(
    __in ULONGLONG ullOperand,
    __out UINT* puResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *puResult = UINT_ERROR;
    
    if (ullOperand <= INTSAFE_UINT_MAX)
    {
        *puResult = (UINT)ullOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// ULONGLONG -> UINT_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlULongLongToUIntPtr(
    __in ULONGLONG ullOperand,
    __out UINT_PTR* puResult)
{
    *puResult = (UINT_PTR)ullOperand;
    return STATUS_SUCCESS;
}
#else    
#define RtlULongLongToUIntPtr  RtlULongLongToUInt
#endif

//
// ULONGLONG -> LONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongLongToLong(
    __in ULONGLONG ullOperand,
    __out LONG* plResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *plResult = LONG_ERROR;
    
    if (ullOperand <= INTSAFE_LONG_MAX)
    {
        *plResult = (LONG)ullOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// ULONGLONG -> LONG_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlULongLongToLongPtr(
    __in ULONGLONG ullOperand,
    __out LONG_PTR* plResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *plResult = LONG_PTR_ERROR;
    
    if (ullOperand <= INTSAFE_LONG_PTR_MAX)
    {
        *plResult = (LONG_PTR)ullOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}
#else
#define RtlULongLongToLongPtr  RtlULongLongToLong
#endif

//
// ULONGLONG -> ULONG conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongLongToULong(
    __in ULONGLONG ullOperand,
    __out ULONG* pulResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pulResult = ULONG_ERROR;
    
    if (ullOperand <= INTSAFE_ULONG_MAX)
    {
        *pulResult = (ULONG)ullOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// ULONGLONG -> ULONG_PTR conversion
//
#ifdef _WIN64
__inline
__checkReturn
NTSTATUS
RtlULongLongToULongPtr(
    __in ULONGLONG ullOperand,
    __out ULONG_PTR* pulResult)
{
    *pulResult = (ULONG_PTR)ullOperand;
    return STATUS_SUCCESS;
}
#else
#define RtlULongLongToULongPtr RtlULongLongToULong
#endif

//
// ULONGLONG -> DWORD conversion
//
#define RtlULongLongToDWord    RtlULongLongToULong

//
// ULONGLONG -> DWORD_PTR conversion
//
#define RtlULongLongToDWordPtr RtlULongLongToULongPtr

//
// ULONGLONG -> RtlINT64 conversion
//
__inline
__checkReturn
NTSTATUS
RtlULongLongToInt64(
    __in ULONGLONG ullOperand,
    __out RtlINT64* pi64Result)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pi64Result = INT64_ERROR;
    
    if (ullOperand <= INTSAFE_INT64_MAX)
    {
        *pi64Result = (INT64)ullOperand;
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// ULONGLONG -> ptrdiff_t conversion
//
#define RtlULongLongToPtrdiffT RtlULongLongToIntPtr

//
// ULONGLONG -> size_t conversion
//
#define RtlULongLongToSizeT    RtlULongLongToUIntPtr

//
// ULONGLONG -> SSIZE_T conversion
//
#define RtlULongLongToSSIZET   RtlULongLongToLongPtr

//
// ULONGLONG -> SIZE_T conversion
//
#define RtlULongLongToSIZET    RtlULongLongToULongPtr

//
// ptrdiff_t -> INT conversion
//
#define RtlPtrdiffTToInt   RtlIntPtrToInt

//
// ptrdiff_t -> UINT conversion
//
#define RtlPtrdiffTToUInt  RtlIntPtrToUInt

//
// ptrdiff_t -> UINT_PTR conversion
//
#define RtlPtrdiffTToUIntPtr   RtlIntPtrToUIntPtr

//
// ptrdiff_t -> LONG conversion
//
#define RtlPtrdiffTToLong  RtlIntPtrToLong

//
// ptrdiff_t -> ULONG conversion
//
#define RtlPtrdiffTToULong RtlIntPtrToULong

//
// ptrdiff_t -> ULONG_PTR conversion
//
#define RtlPtrdiffTToULongPtr  RtlIntPtrToULongPtr

//
// ptrdiff_t -> DWORD conversion
//
#define RtlPtrdiffTToDWord RtlIntPtrToULong

//
// ptrdiff_t -> DWORD_PTR conversion
//
#define RtlPtrdiffTToDWordPtr  RtlIntPtrToULongPtr

//
// ptrdiff_t -> size_t conversion
//
#define RtlPtrdiffTToSizeT RtlIntPtrToUIntPtr

//
// ptrdiff_t -> SIZE_T conversion
//
#define RtlPtrdiffTToSIZET RtlIntPtrToULongPtr

//
// size_t -> INT conversion
//
#define RtlSizeTToInt  RtlUIntPtrToInt

//
// size_t -> INT_PTR conversion
//
#define RtlSizeTToIntPtr   RtlUIntPtrToIntPtr

//
// size_t -> UINT conversion
//
#define RtlSizeTToUInt RtlUIntPtrToUInt

//
// size_t -> LONG conversion
//
#define RtlSizeTToLong RtlUIntPtrToLong

//
// size_t -> LONG_PTR conversion
//
#define RtlSizeTToLongPtr  RtlUIntPtrToLongPtr

//
// size_t -> ULONG conversion
//
#define RtlSizeTToULong    RtlUIntPtrToULong

//
// size_t -> DWORD conversion
//
#define RtlSizeTToDWord    RtlUIntPtrToULong

//
// size_t -> RtlINT64
//
__inline
__checkReturn
NTSTATUS
RtlSizeTToInt64(
    __in size_t Operand,
    __out RtlINT64* pi64Result)
{
#ifdef _WIN64
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pi64Result = INT64_ERROR;
    
    if (Operand <= INTSAFE_INT64_MAX)
    {
        *pi64Result = (INT64)Operand;
        status = STATUS_SUCCESS;
    }
    
    return status; 
#else
    *pi64Result = (INT64)Operand;
    return STATUS_SUCCESS;
#endif
}

//   
// size_t -> ptrdiff_t conversion
//
#define RtlSizeTToPtrdiffT RtlUIntPtrToIntPtr

//
// size_t -> SSIZE_T conversion
//
#define RtlSizeTToSSIZET   RtlUIntPtrToLongPtr

//
// SSIZE_T -> INT conversion
//
#define RtlSSIZETToInt RtlLongPtrToInt

//
// SSIZE_T -> INT_PTR conversion
//
#define RtlSSIZETToIntPtr  RtlLongPtrToIntPtr

//
// SSIZE_T -> UINT conversion
//
#define RtlSSIZETToUInt    RtlLongPtrToUInt

//
// SSIZE_T -> UINT_PTR conversion
//
#define RtlSSIZETToUIntPtr RtlLongPtrToUIntPtr

//
// SSIZE_T -> LONG conversion
//
#define RtlSSIZETToLong    RtlLongPtrToLong

//
// SSIZE_T -> ULONG conversion
//
#define RtlSSIZETToULong   RtlLongPtrToULong

//
// SSIZE_T -> ULONG_PTR conversion
//
#define RtlSSIZETToULongPtr    RtlLongPtrToULongPtr

//
// SSIZE_T -> DWORD conversion
//
#define RtlSSIZETToDWord   RtlLongPtrToULong

//
// SSIZE_T -> DWORD_PTR conversion
//
#define RtlSSIZETToDWordPtr    RtlLongPtrToULongPtr

//
// SSIZE_T -> size_t conversion
//
#define RtlSSIZETToSizeT   RtlLongPtrToUIntPtr

//
// SSIZE_T -> SIZE_T conversion
//
#define RtlSSIZETToSIZET   RtlLongPtrToULongPtr

//
// SIZE_T -> INT conversion
//
#define RtlSIZETToInt  RtlULongPtrToInt

//
// SIZE_T -> INT_PTR conversion
//
#define RtlSIZETToIntPtr   RtlULongPtrToIntPtr

//
// SIZE_T -> UINT conversion
//
#define RtlSIZETToUInt RtlULongPtrToUInt

//
// SIZE_T -> LONG conversion
//
#define RtlSIZETToLong RtlULongPtrToLong

//
// SIZE_T -> LONG_PTR conversion
//
#define RtlSIZETToLongPtr  RtlULongPtrToLongPtr

//
// SIZE_T -> ULONG conversion
//
#define RtlSIZETToULong    RtlULongPtrToULong

//
// SIZE_T -> DWORD conversion
//
#define RtlSIZETToDWord    RtlULongPtrToULong

//
// SIZE_T -> RtlINT64
//
#define RtlSIZETToInt64    RtlULongPtrToInt64

//
// SIZE_T -> ptrdiff_t conversion
//
#define RtlSIZETToPtrdiffT RtlULongPtrToIntPtr

//
// SIZE_T -> SSIZE_T conversion
//
#define RtlSIZETToSSIZET   RtlULongPtrToLongPtr


// ============================================================================
// Addition functions
//=============================================================================

//
// USHORT addition
//
__inline
__checkReturn
NTSTATUS
RtlUShortAdd(
    __in USHORT usAugend,
    __in USHORT usAddend,
    __out USHORT* pusResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pusResult = USHORT_ERROR;

    if (((USHORT)(usAugend + usAddend)) >= usAugend)
    {
        *pusResult = (usAugend + usAddend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// WORD addtition
//
#define RtlWordAdd     RtlUShortAdd

//
// UINT addition
//
__inline
__checkReturn
NTSTATUS
RtlUIntAdd(
    __in UINT uAugend,
    __in UINT uAddend,
    __out UINT* puResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *puResult = UINT_ERROR;

    if ((uAugend + uAddend) >= uAugend)
    {
        *puResult = (uAugend + uAddend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// UINT_PTR addition
//
#ifdef _WIN64
#define RtlUIntPtrAdd      RtlULongLongAdd
#else
#define RtlUIntPtrAdd      RtlUIntAdd
#endif // _WIN64

//
// ULONG addition
//
__inline
__checkReturn
NTSTATUS
RtlULongAdd(
    __in ULONG ulAugend,
    __in ULONG ulAddend,
    __out ULONG* pulResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pulResult = ULONG_ERROR;

    if ((ulAugend + ulAddend) >= ulAugend)
    {
        *pulResult = (ulAugend + ulAddend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// ULONG_PTR addition
//
#ifdef _WIN64
#define RtlULongPtrAdd     RtlULongLongAdd
#else
#define RtlULongPtrAdd     RtlULongAdd
#endif // _WIN64

//
// DWORD addition
//
#define RtlDWordAdd        RtlULongAdd

//
// DWORD_PTR addition
//
#ifdef _WIN64
#define RtlDWordPtrAdd     RtlULongLongAdd
#else
#define RtlDWordPtrAdd     RtlULongAdd
#endif // _WIN64

//
// size_t addition
//
__inline
__checkReturn
NTSTATUS
RtlSizeTAdd(
    __in size_t Augend,
    __in size_t Addend,
    __out size_t* pResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pResult = size_t_ERROR;

    if ((Augend + Addend) >= Augend)
    {
        *pResult = (Augend + Addend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// SIZE_T addition
//
#ifdef _WIN64
#define RtlSIZETAdd      RtlULongLongAdd
#else
#define RtlSIZETAdd      RtlULongAdd
#endif // _WIN64

//
// ULONGLONG addition
//
__inline
__checkReturn
NTSTATUS
RtlULongLongAdd(
    __in ULONGLONG ullAugend,
    __in ULONGLONG ullAddend,
    __out ULONGLONG* pullResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pullResult = ULONGLONG_ERROR;

    if ((ullAugend + ullAddend) >= ullAugend)
    {
        *pullResult = (ullAugend + ullAddend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}


// ============================================================================
// Subtraction functions
//=============================================================================

//
// USHORT subtraction
//
__inline
__checkReturn
NTSTATUS
RtlUShortSub(
    __in USHORT usMinuend,
    __in USHORT usSubtrahend,
    __out USHORT* pusResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pusResult = USHORT_ERROR;

    if (usMinuend >= usSubtrahend)
    {
        *pusResult = (usMinuend - usSubtrahend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// WORD subtraction
//
#define RtlWordSub     RtlUShortSub


//
// UINT subtraction
//
__inline
__checkReturn
NTSTATUS
RtlUIntSub(
    __in UINT uMinuend,
    __in UINT uSubtrahend,
    __out UINT* puResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *puResult = UINT_ERROR;

    if (uMinuend >= uSubtrahend)
    {
        *puResult = (uMinuend - uSubtrahend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// UINT_PTR subtraction
//
#ifdef _WIN64
#define RtlUIntPtrSub  RtlULongLongSub
#else
#define RtlUIntPtrSub  RtlUIntSub
#endif // _WIN64

//
// ULONG subtraction
//
__inline
__checkReturn
NTSTATUS
RtlULongSub(
    __in ULONG ulMinuend,
    __in ULONG ulSubtrahend,
    __out ULONG* pulResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pulResult = ULONG_ERROR;

    if (ulMinuend >= ulSubtrahend)
    {
        *pulResult = (ulMinuend - ulSubtrahend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// ULONG_PTR subtraction
//
#ifdef _WIN64
#define RtlULongPtrSub RtlULongLongSub
#else
#define RtlULongPtrSub RtlULongSub
#endif // _WIN64


//
// DWORD subtraction
//
#define RtlDWordSub        RtlULongSub

//
// DWORD_PTR subtraction
//
#ifdef _WIN64
#define RtlDWordPtrSub     RtlULongLongSub
#else
#define RtlDWordPtrSub     RtlULongSub
#endif // _WIN64

//
// size_t subtraction
//
__inline
__checkReturn
NTSTATUS
RtlSizeTSub(
    __in size_t Minuend,
    __in size_t Subtrahend,
    __out size_t* pResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pResult = size_t_ERROR;

    if (Minuend >= Subtrahend)
    {
        *pResult = (Minuend - Subtrahend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}

//
// SIZE_T subtraction
//
#ifdef _WIN64
#define RtlSIZETSub    RtlULongLongSub
#else
#define RtlSIZETSub    RtlULongSub
#endif // _WIN64

//
// ULONGLONG subtraction
//
__inline
__checkReturn
NTSTATUS
RtlULongLongSub(
    __in ULONGLONG ullMinuend,
    __in ULONGLONG ullSubtrahend,
    __out ULONGLONG* pullResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
    *pullResult = ULONGLONG_ERROR;

    if (ullMinuend >= ullSubtrahend)
    {
        *pullResult = (ullMinuend - ullSubtrahend);
        status = STATUS_SUCCESS;
    }
    
    return status;
}


// ============================================================================
// Multiplication functions
//=============================================================================

//
// USHORT multiplication
//
__inline
__checkReturn
NTSTATUS
RtlUShortMult(
    __in USHORT usMultiplicand,
    __in USHORT usMultiplier,
    __out USHORT* pusResult)
{
    ULONG ulResult = ((ULONG)usMultiplicand) * (ULONG)usMultiplier;
    
    return RtlULongToUShort(ulResult, pusResult);
}

//
// WORD multiplication
//
#define RtlWordMult      RtlUShortMult

//
// UINT multiplication
//
__inline
__checkReturn
NTSTATUS
RtlUIntMult(
    __in UINT uMultiplicand,
    __in UINT uMultiplier,
    __out UINT* puResult)
{
    ULONGLONG ull64Result = UInt32x32To64(uMultiplicand, uMultiplier);

    return RtlULongLongToUInt(ull64Result, puResult);
}

//
// UINT_PTR multiplication
//
#ifdef _WIN64
#define RtlUIntPtrMult     RtlULongLongMult
#else
#define RtlUIntPtrMult     RtlUIntMult
#endif // _WIN64

//
// ULONG multiplication
//
__inline
__checkReturn
NTSTATUS
RtlULongMult(
    __in ULONG ulMultiplicand,
    __in ULONG ulMultiplier,
    __out ULONG* pulResult)
{
    ULONGLONG ull64Result = UInt32x32To64(ulMultiplicand, ulMultiplier);
    
    return RtlULongLongToULong(ull64Result, pulResult);
}

//
// ULONG_PTR multiplication
//
#ifdef _WIN64
#define RtlULongPtrMult    RtlULongLongMult
#else
#define RtlULongPtrMult    RtlULongMult
#endif // _WIN64


//
// DWORD multiplication
//
#define RtlDWordMult       RtlULongMult

//
// DWORD_PTR multiplication
//
#ifdef _WIN64
#define RtlDWordPtrMult    RtlULongLongMult
#else
#define RtlDWordPtrMult    RtlULongMult
#endif // _WIN64

//
// size_t multiplication
//

#ifdef _WIN64
#define RtlSizeTMult       RtlULongLongMult
#else
#define RtlSizeTMult       RtlUIntMult
#endif // _WIN64

//
// SIZE_T multiplication
//
#ifdef _WIN64
#define RtlSIZETMult       RtlULongLongMult
#else
#define RtlSIZETMult       RtlULongMult
#endif // _WIN64

//
// ULONGLONG multiplication
//
__inline
__checkReturn
NTSTATUS
RtlULongLongMult(
    __in ULONGLONG ullMultiplicand,
    __in ULONGLONG ullMultiplier,
    __out ULONGLONG* pullResult)
{
    NTSTATUS status = STATUS_INTEGER_OVERFLOW;
#ifdef _AMD64_
    ULONGLONG u64ResultHigh;
    ULONGLONG u64ResultLow;
    
    *pullResult = ULONGLONG_ERROR;
    
    u64ResultLow = UnsignedMultiply128(ullMultiplicand, ullMultiplier, &u64ResultHigh);
    if (u64ResultHigh == 0)
    {
        *pullResult = u64ResultLow;
        status = STATUS_SUCCESS;
    }
#else
    // 64x64 into 128 is like 32.32 x 32.32.
    //
    // a.b * c.d = a*(c.d) + .b*(c.d) = a*c + a*.d + .b*c + .b*.d
    // back in non-decimal notation where A=a*2^32 and C=c*2^32:  
    // A*C + A*d + b*C + b*d
    // So there are four components to add together.
    //   result = (a*c*2^64) + (a*d*2^32) + (b*c*2^32) + (b*d)
    //
    // a * c must be 0 or there would be bits in the high 64-bits
    // a * d must be less than 2^32 or there would be bits in the high 64-bits
    // b * c must be less than 2^32 or there would be bits in the high 64-bits
    // then there must be no overflow of the resulting values summed up.
    
    ULONG dw_a;
    ULONG dw_b;
    ULONG dw_c;
    ULONG dw_d;
    ULONGLONG ad = 0;
    ULONGLONG bc = 0;
    ULONGLONG bd = 0;
    ULONGLONG ullResult = 0;
    
    *pullResult = ULONGLONG_ERROR;

    dw_a = (ULONG)(ullMultiplicand >> 32);
    dw_c = (ULONG)(ullMultiplier >> 32);

    // common case -- if high dwords are both zero, no chance for overflow
    if ((dw_a == 0) && (dw_c == 0))
    {
        dw_b = (DWORD)ullMultiplicand;
        dw_d = (DWORD)ullMultiplier;

        *pullResult = (((ULONGLONG)dw_b) * (ULONGLONG)dw_d);
        status = STATUS_SUCCESS;
    }
    else
    {
        // a * c must be 0 or there would be bits set in the high 64-bits
        if ((dw_a == 0) ||
            (dw_c == 0))
        {
            dw_d = (DWORD)ullMultiplier;

            // a * d must be less than 2^32 or there would be bits set in the high 64-bits
            ad = (((ULONGLONG)dw_a) * (ULONGLONG)dw_d);
            if ((ad & 0xffffffff00000000) == 0)
            {
                dw_b = (DWORD)ullMultiplicand;

                // b * c must be less than 2^32 or there would be bits set in the high 64-bits
                bc = (((ULONGLONG)dw_b) * (ULONGLONG)dw_c);
                if ((bc & 0xffffffff00000000) == 0)
                {
                    // now sum them all up checking for overflow.
                    // shifting is safe because we already checked for overflow above
                    if (NT_SUCCESS(RtlULongLongAdd(bc << 32, ad << 32, &ullResult)))                        
                    {
                        // b * d
                        bd = (((ULONGLONG)dw_b) * (ULONGLONG)dw_d);
                    
                        if (NT_SUCCESS(RtlULongLongAdd(ullResult, bd, &ullResult)))
                        {
                            *pullResult = ullResult;
                            status = STATUS_SUCCESS;
                        }
                    }
                }
            }
        }
    }
#endif // _AMD64_  
    
    return status;
}

#endif // _NTINTSAFE_H_INCLUDED_

