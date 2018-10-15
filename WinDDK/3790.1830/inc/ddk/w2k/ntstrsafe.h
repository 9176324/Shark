/******************************************************************
*                                                                 *
*  ntstrsafe.h -- This module defines safer C library string      *
*                 routine replacements for drivers. These are     *
*                 meant to make C a bit more safe in reference    *
*                 to security and robustness. A similar file,     *
*                 strsafe.h, is available for applications.       *
*                                                                 *
*  Copyright (c) Microsoft Corp.  All rights reserved.            *
*                                                                 *
******************************************************************/
#ifndef _NTSTRSAFE_H_INCLUDED_
#define _NTSTRSAFE_H_INCLUDED_
#pragma once

#include <stdio.h>          // for _vsnprintf, _vsnwprintf, getc, getwc
#include <string.h>         // for memset
#include <stdarg.h>         // for va_start, etc.
#include <specstrings.h>    // for __in, etc.

#ifndef NTSTRSAFE_NO_UNICODE_STRING_FUNCTIONS
#include <ntdef.h>          // for UNICODE_STRING, etc.
#endif


// compiletime asserts (failure results in error C2118: negative subscript)
#ifndef C_ASSERT
#define C_ASSERT(e) typedef char __C_ASSERT__[(e)?1:-1]
#endif

#ifdef __cplusplus
#define _NTSTRSAFE_EXTERN_C    extern "C"
#else
#define _NTSTRSAFE_EXTERN_C    extern
#endif

#ifndef _STRSAFE_USE_SECURE_CRT
#if defined(__GOT_SECURE_LIB__) && (__GOT_SECURE_LIB__ >= 200402L)
#define _STRSAFE_USE_SECURE_CRT 1
#else
#define _STRSAFE_USE_SECURE_CRT 0
#endif
#endif


//
// The following steps are *REQUIRED* if ntstrsafe.h is used for drivers on:
//     Windows 2000
//     Windows Millennium Edition
//     Windows 98 Second Edition
//     Windows 98
//
// 1. #define NTSTRSAFE_LIB before including this header file.
// 2. Add ntstrsafe.lib to the TARGET_LIBS line in SOURCES
//
// Drivers running on XP and later can skip these steps to create a smaller
// driver.
//
#if defined(NTSTRSAFE_LIB)
#define NTSTRSAFEDDI  _NTSTRSAFE_EXTERN_C NTSTATUS __stdcall
#pragma comment(lib, "ntstrsafe.lib")
#elif defined(NTSTRSAFE_LIB_IMPL)
#define NTSTRSAFEDDI  _NTSTRSAFE_EXTERN_C NTSTATUS __stdcall
#else
#define NTSTRSAFEDDI  __inline NTSTATUS __stdcall
#define NTSTRSAFE_INLINE
#endif

// Some functions we want to force inline because we want compiletime differences based on #defines, or they use 
// stdin and we want to avoid building multiple versions of ntstrsafe.lib (depending on what #define or if 
// you use msvcrt, libcmt, etc..)
#define NTSTRSAFE_INLINE_API  __inline NTSTATUS __stdcall

// The user can request no "Cb" or no "Cch" fuctions, but not both!
#if defined(NTSTRSAFE_NO_CB_FUNCTIONS) && defined(NTSTRSAFE_NO_CCH_FUNCTIONS)
#error cannot specify both NTSTRSAFE_NO_CB_FUNCTIONS and NTSTRSAFE_NO_CCH_FUNCTIONS !!
#endif

// This should only be defined when we are building ntstrsafe.lib
#ifdef NTSTRSAFE_LIB_IMPL
#define NTSTRSAFE_INLINE
#endif

#define NTSTRSAFE_MAX_CCH     2147483647 // max # of characters we support (same as INT_MAX)

#ifndef NTSTRSAFE_UNICODE_STRING_MAX_CCH
#define NTSTRSAFE_UNICODE_STRING_MAX_CCH    0x7ffe  // max # of characters supported in a UNICODE_STRING
#endif
C_ASSERT(NTSTRSAFE_UNICODE_STRING_MAX_CCH <= 0x7ffe);

// If both strsafe.h and ntstrsafe.h are included, only use definitions below from one.
#ifndef _STRSAFE_H_INCLUDED_

// Flags for controling the Ex functions
//
//      STRSAFE_FILL_BYTE(0xFF)                         0x000000FF  // bottom byte specifies fill pattern
#define STRSAFE_IGNORE_NULLS                            0x00000100  // treat null string pointers as TEXT("") -- don't fault on NULL buffers
#define STRSAFE_FILL_BEHIND_NULL                        0x00000200  // fill in extra space behind the null terminator
#define STRSAFE_FILL_ON_FAILURE                         0x00000400  // on failure, overwrite pszDest with fill pattern and null terminate it
#define STRSAFE_NULL_ON_FAILURE                         0x00000800  // on failure, set *pszDest = TEXT('\0')
#define STRSAFE_NO_TRUNCATION                           0x00001000  // instead of returning a truncated result, copy/append nothing to pszDest and null terminate it
#define STRSAFE_IGNORE_NULL_UNICODE_STRINGS             0x00010000  // don't crash on null UNICODE_STRING pointers (STRSAFE_IGNORE_NULLS implies this flag)
#define STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED     0x00020000  // we will fail if the Dest PUNICODE_STRING's Buffer cannot be null terminated 

#define STRSAFE_VALID_FLAGS                     (0x000000FF | STRSAFE_IGNORE_NULLS | STRSAFE_FILL_BEHIND_NULL | STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION)
#define STRSAFE_UNICODE_STRING_VALID_FLAGS      (STRSAFE_VALID_FLAGS | STRSAFE_IGNORE_NULL_UNICODE_STRINGS | STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)

// helper macro to set the fill character and specify buffer filling
#define STRSAFE_FILL_BYTE(x)                    ((unsigned long)((x & 0x000000FF) | STRSAFE_FILL_BEHIND_NULL))
#define STRSAFE_FAILURE_BYTE(x)                 ((unsigned long)((x & 0x000000FF) | STRSAFE_FILL_ON_FAILURE))

#define STRSAFE_GET_FILL_PATTERN(dwFlags)       ((int)(dwFlags & 0x000000FF))

#endif // _STRSAFE_H_INCLUDED_


//
// These typedefs are used in places where the string is guaranteed to
// be null terminated.
//
typedef __nullterminated char* NTSTRSAFE_PSTR;
typedef __nullterminated const char* NTSTRSAFE_PCSTR;
typedef __nullterminated wchar_t* NTSTRSAFE_PWSTR;
typedef __nullterminated const wchar_t* NTSTRSAFE_PCWSTR;


#ifdef _STRSAFE_H_INCLUDED_
#pragma warning(push)
#pragma warning(disable : 4995)
#endif // _STRSAFE_H_INCLUDED_


// prototypes for the worker functions
#ifdef NTSTRSAFE_INLINE

NTSTRSAFEDDI
RtlStringCopyWorkerA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc);

NTSTRSAFEDDI
RtlStringCopyWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc);

NTSTRSAFEDDI
RtlStringCopyExWorkerA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCopyExWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCopyNWorkerA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToCopy);

NTSTRSAFEDDI
RtlStringCopyNWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy);

NTSTRSAFEDDI
RtlStringCopyNExWorkerA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToCopy,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCopyNExWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCatWorkerA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc);

NTSTRSAFEDDI
RtlStringCatWorkerW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc);

NTSTRSAFEDDI
RtlStringCatExWorkerA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCatExWorkerW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCatNWorkerA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToAppend);

NTSTRSAFEDDI
RtlStringCatNWorkerW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend);

NTSTRSAFEDDI
RtlStringCatNExWorkerA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToAppend,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCatNExWorkerW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringVPrintfWorkerA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList);

NTSTRSAFEDDI
RtlStringVPrintfWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList);

NTSTRSAFEDDI
RtlStringVPrintfExWorkerA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList);

NTSTRSAFEDDI
RtlStringVPrintfExWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList);

NTSTRSAFEDDI
RtlStringLengthWorkerA(
    __in NTSTRSAFE_PCSTR psz,
    __in size_t cchMax,
    __out_opt size_t* pcchLength);

NTSTRSAFEDDI
RtlStringLengthWorkerW(
    __in NTSTRSAFE_PCWSTR psz,
    __in size_t cchMax,
    __out_opt size_t* pcchLength);
    
#endif  // NTSTRSAFE_INLINE
    
#ifndef NTSTRSAFE_NO_UNICODE_STRING_FUNCTIONS

NTSTRSAFEDDI
RtlUnicodeStringInitWorker(
    __out PUNICODE_STRING Dest,
    __in_opt NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchMax,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringValidateWorker(
    __in_opt PCUNICODE_STRING Src,
    __in size_t cchMax,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringValidateSrcWorker(
    __in PCUNICODE_STRING Src,
    __deref_out_ecount(*pcchSrcLength) wchar_t** ppszSrc,
    __out size_t* pcchSrcLength,
    __in size_t cchMax,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringValidateDestWorker(
    __in PCUNICODE_STRING Dest,
    __deref_out_ecount(*pcchDest) wchar_t** ppszDest,
    __out size_t* pcchDest,
    __out_opt size_t* pcchDestLength,
    __in size_t cchMax,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringCopyStringWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringCopyWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in_ecount(cchSrcLength) const wchar_t* pszSrc,
    __in size_t cchSrcLength,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringExHandleFailureWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringCopyStringExWorker(  
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringCopyExWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in_ecount(cchSrcLength) const wchar_t* pszSrc,
    __in size_t cchSrcLength,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringCopyStringNWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringCopyStringNExWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlUnicodeStringVPrintfWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList);
    
NTSTRSAFEDDI
RtlUnicodeStringVPrintfExWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList);

#endif  // !NTSTRSAFE_NO_UNICODE_STRING_FUNCTIONS



#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchCopy(
    OUT LPTSTR  pszDest,
    IN  size_t  cchDest,
    IN  LPCTSTR pszSrc
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcpy'.
    The size of the destination buffer (in characters) is a parameter and
    this function will not write past the end of this buffer and it will
    ALWAYS null terminate the destination buffer (unless it is zero length).

    This routine is not a replacement for strncpy.  That function will pad the
    destination string with extra null termination characters if the count is
    greater than the length of the source string, and it will fail to null
    terminate the destination string if the source string length is greater
    than or equal to the count. You can not blindly use this instead of strncpy:
    it is common for code to use it to "patch" strings and you would introduce
    errors if the code started null terminating in the middle of the string.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was copied without truncation and null terminated,
    otherwise it will return a failure code. In failure cases as much of
    pszSrc will be copied to pszDest as possible, and pszDest will be null
    terminated.

Arguments:

    pszDest        -   destination string

    cchDest        -   size of destination buffer in characters.
                       length must be = (_tcslen(src) + 1) to hold all of the
                       source including the null terminator

    pszSrc         -   source string which must be null terminated

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL. See RtlStringCchCopyEx if you require
    the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFEDDI
RtlStringCchCopyA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc);

NTSTRSAFEDDI
RtlStringCchCopyW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchCopyA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyWorkerA(pszDest, cchDest, pszSrc);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchCopyW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyWorkerW(pszDest, cchDest, pszSrc);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbCopy(
    OUT LPTSTR pszDest,
    IN  size_t cbDest,
    IN  LPCTSTR pszSrc
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcpy'.
    The size of the destination buffer (in bytes) is a parameter and this
    function will not write past the end of this buffer and it will ALWAYS
    null terminate the destination buffer (unless it is zero length).

    This routine is not a replacement for strncpy.  That function will pad the
    destination string with extra null termination characters if the count is
    greater than the length of the source string, and it will fail to null
    terminate the destination string if the source string length is greater
    than or equal to the count. You can not blindly use this instead of strncpy:
    it is common for code to use it to "patch" strings and you would introduce
    errors if the code started null terminating in the middle of the string.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was copied without truncation and null terminated,
    otherwise it will return a failure code. In failure cases as much of pszSrc
    will be copied to pszDest as possible, and pszDest will be null terminated.

Arguments:

    pszDest        -   destination string

    cbDest         -   size of destination buffer in bytes.
                       length must be = ((_tcslen(src) + 1) * sizeof(TCHAR)) to
                       hold all of the source including the null terminator

    pszSrc         -   source string which must be null terminated

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL.  See RtlStringCbCopyEx if you require
    the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFEDDI
RtlStringCbCopyA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc);

NTSTRSAFEDDI
RtlStringCbCopyW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbCopyA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc)
{
    NTSTATUS status;
    size_t cchDest;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyWorkerA(pszDest, cchDest, pszSrc);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbCopyW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc)
{
    NTSTATUS status;
    size_t cchDest;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyWorkerW(pszDest, cchDest, pszSrc);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchCopyEx(
    OUT LPTSTR  pszDest         OPTIONAL,
    IN  size_t  cchDest,
    IN  LPCTSTR pszSrc          OPTIONAL,
    OUT LPTSTR* ppszDestEnd     OPTIONAL,
    OUT size_t* pcchRemaining   OPTIONAL,
    IN  DWORD   dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcpy' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCchCopy, this routine also returns a pointer to the end of the
    destination string and the number of characters left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string

    cchDest         -   size of destination buffer in characters.
                        length must be = (_tcslen(pszSrc) + 1) to hold all of
                        the source including the null terminator

    pszSrc          -   source string which must be null terminated

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function copied any data, the result will point to the
                        null termination character

    pcchRemaining   -   if pcchRemaining is non-null, the function will return the
                        number of characters left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcpy

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchCopyExA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCchCopyExW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchCopyExA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(char) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
        cbDest = cchDest * sizeof(char);

        status = RtlStringCopyExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchCopyExW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(wchar_t) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringCopyExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbCopyEx(
    OUT LPTSTR  pszDest         OPTIONAL,
    IN  size_t  cbDest,
    IN  LPCTSTR pszSrc          OPTIONAL,
    OUT LPTSTR* ppszDestEnd     OPTIONAL,
    OUT size_t* pcbRemaining    OPTIONAL,
    IN  DWORD   dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcpy' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCbCopy, this routine also returns a pointer to the end of the
    destination string and the number of bytes left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string

    cbDest          -   size of destination buffer in bytes.
                        length must be ((_tcslen(pszSrc) + 1) * sizeof(TCHAR)) to
                        hold all of the source including the null terminator

    pszSrc          -   source string which must be null terminated

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function copied any data, the result will point to the
                        null termination character

    pcbRemaining    -   pcbRemaining is non-null,the function will return the
                        number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcpy

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCbCopyExA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCbCopyExW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbCopyExA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(char) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbCopyExW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(wchar_t) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchCopyN(
    OUT LPTSTR  pszDest,
    IN  size_t  cchDest,
    IN  LPCTSTR pszSrc,
    IN  size_t  cchToCopy
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy'.
    The size of the destination buffer (in characters) is a parameter and
    this function will not write past the end of this buffer and it will
    ALWAYS null terminate the destination buffer (unless it is zero length).

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cchToCopy is greater than the length of pszSrc.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the entire string or the first cchToCopy characters were copied
    without truncation and the resultant destination string was null terminated,
    otherwise it will return a failure code. In failure cases as much of pszSrc
    will be copied to pszDest as possible, and pszDest will be null terminated.

Arguments:

    pszDest        -   destination string

    cchDest        -   size of destination buffer in characters.
                       length must be = (_tcslen(src) + 1) to hold all of the
                       source including the null terminator

    pszSrc         -   source string

    cchToCopy      -   maximum number of characters to copy from source string,
                       not including the null terminator.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL. See RtlStringCchCopyNEx if you require
    the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFEDDI
RtlStringCchCopyNA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToCopy);

NTSTRSAFEDDI
RtlStringCchCopyNW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchCopyNA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToCopy)
{
    NTSTATUS status;

    if ((cchDest > NTSTRSAFE_MAX_CCH) ||
        (cchToCopy > NTSTRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNWorkerA(pszDest, cchDest, pszSrc, cchToCopy);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchCopyNW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy)
{
    NTSTATUS status;

    if ((cchDest > NTSTRSAFE_MAX_CCH) ||
        (cchToCopy > NTSTRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNWorkerW(pszDest, cchDest, pszSrc, cchToCopy);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbCopyN(
    OUT LPTSTR  pszDest,
    IN  size_t  cbDest,
    IN  LPCTSTR pszSrc,
    IN  size_t  cbToCopy
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy'.
    The size of the destination buffer (in bytes) is a parameter and this
    function will not write past the end of this buffer and it will ALWAYS
    null terminate the destination buffer (unless it is zero length).

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cbToCopy is greater than the size of pszSrc.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the entire string or the first cbToCopy characters were
    copied without truncation and the resultant destination string was null
    terminated, otherwise it will return a failure code. In failure cases as
    much of pszSrc will be copied to pszDest as possible, and pszDest will be
    null terminated.

Arguments:

    pszDest        -   destination string

    cbDest         -   size of destination buffer in bytes.
                       length must be = ((_tcslen(src) + 1) * sizeof(TCHAR)) to
                       hold all of the source including the null terminator

    pszSrc         -   source string

    cbToCopy       -   maximum number of bytes to copy from source string,
                       not including the null terminator.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL.  See RtlStringCbCopyEx if you require
    the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFEDDI
RtlStringCbCopyNA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cbToCopy);

NTSTRSAFEDDI
RtlStringCbCopyNW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToCopy);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbCopyNA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cbToCopy)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchToCopy;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);
    cchToCopy = cbToCopy / sizeof(char);

    if ((cchDest > NTSTRSAFE_MAX_CCH) ||
        (cchToCopy > NTSTRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNWorkerA(pszDest, cchDest, pszSrc, cchToCopy);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbCopyNW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToCopy)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchToCopy;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);
    cchToCopy = cbToCopy / sizeof(wchar_t);

    if ((cchDest > NTSTRSAFE_MAX_CCH) ||
        (cchToCopy > NTSTRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNWorkerW(pszDest, cchDest, pszSrc, cchToCopy);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchCopyNEx(
    OUT LPTSTR  pszDest         OPTIONAL,
    IN  size_t  cchDest,
    IN  LPCTSTR pszSrc          OPTIONAL,
    IN  size_t  cchToCopy,
    OUT LPTSTR* ppszDestEnd     OPTIONAL,
    OUT size_t* pcchRemaining   OPTIONAL,
    IN  DWORD   dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCchCopyN, this routine also returns a pointer to the end of the
    destination string and the number of characters left in the destination
    string including the null terminator. The flags parameter allows
    additional controls.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cchToCopy is greater than the length of pszSrc.

Arguments:

    pszDest         -   destination string

    cchDest         -   size of destination buffer in characters.
                        length must be = (_tcslen(pszSrc) + 1) to hold all of
                        the source including the null terminator

    pszSrc          -   source string

    cchToCopy       -   maximum number of characters to copy from the source
                        string

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function copied any data, the result will point to the
                        null termination character

    pcchRemaining   -   if pcchRemaining is non-null, the function will return the
                        number of characters left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcpy

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified. If STRSAFE_IGNORE_NULLS is passed, both pszDest and pszSrc
    may be NULL. An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchCopyNExA(
    __in_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToCopy,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCchCopyNExW(
    __in_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchCopyNExA(
    __in_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToCopy,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(char) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
        cbDest = cchDest * sizeof(char);

        status = RtlStringCopyNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchToCopy, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchCopyNExW(
    __in_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(wchar_t) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringCopyNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchToCopy, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbCopyNEx(
    OUT LPTSTR  pszDest         OPTIONAL,
    IN  size_t  cbDest,
    IN  LPCTSTR pszSrc          OPTIONAL,
    IN  size_t  cbToCopy,
    OUT LPTSTR* ppszDestEnd     OPTIONAL,
    OUT size_t* pcbRemaining    OPTIONAL,
    IN  DWORD   dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCbCopyN, this routine also returns a pointer to the end of the
    destination string and the number of bytes left in the destination string
    including the null terminator. The flags parameter allows additional controls.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cbToCopy is greater than the size of pszSrc.

Arguments:

    pszDest         -   destination string

    cbDest          -   size of destination buffer in bytes.
                        length must be ((_tcslen(pszSrc) + 1) * sizeof(TCHAR)) to
                        hold all of the source including the null terminator

    pszSrc          -   source string

    cbToCopy        -   maximum number of bytes to copy from source string

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function copied any data, the result will point to the
                        null termination character

    pcbRemaining    -   pcbRemaining is non-null,the function will return the
                        number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcpy

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCbCopyNExA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cbToCopy,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCbCopyNExW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToCopy,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbCopyNExA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cbToCopy,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchToCopy;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);
    cchToCopy = cbToCopy / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchToCopy, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(char) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbCopyNExW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToCopy,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchToCopy;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);
    cchToCopy = cbToCopy / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCopyNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchToCopy, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(wchar_t) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchCat(
    IN OUT LPTSTR  pszDest,
    IN     size_t  cchDest,
    IN     LPCTSTR pszSrc
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcat'.
    The size of the destination buffer (in characters) is a parameter and this
    function will not write past the end of this buffer and it will ALWAYS
    null terminate the destination buffer (unless it is zero length).

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was concatenated without truncation and null terminated,
    otherwise it will return a failure code. In failure cases as much of pszSrc
    will be appended to pszDest as possible, and pszDest will be null
    terminated.

Arguments:

    pszDest     -  destination string which must be null terminated

    cchDest     -  size of destination buffer in characters.
                   length must be = (_tcslen(pszDest) + _tcslen(pszSrc) + 1)
                   to hold all of the combine string plus the null
                   terminator

    pszSrc      -  source string which must be null terminated

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL.  See RtlStringCchCatEx if you require
    the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated and
                       the resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error occurs,
                       the destination buffer is modified to contain a truncated
                       version of the ideal result and is null terminated. This
                       is useful for situations where truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchCatA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc);

NTSTRSAFEDDI
RtlStringCchCatW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchCatA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatWorkerA(pszDest, cchDest, pszSrc);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchCatW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatWorkerW(pszDest, cchDest, pszSrc);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbCat(
    IN OUT LPTSTR  pszDest,
    IN     size_t  cbDest,
    IN     LPCTSTR pszSrc
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcat'.
    The size of the destination buffer (in bytes) is a parameter and this
    function will not write past the end of this buffer and it will ALWAYS
    null terminate the destination buffer (unless it is zero length).

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was concatenated without truncation and null terminated,
    otherwise it will return a failure code. In failure cases as much of pszSrc
    will be appended to pszDest as possible, and pszDest will be null
    terminated.

Arguments:

    pszDest     -  destination string which must be null terminated

    cbDest      -  size of destination buffer in bytes.
                   length must be = ((_tcslen(pszDest) + _tcslen(pszSrc) + 1) * sizeof(TCHAR)
                   to hold all of the combine string plus the null
                   terminator

    pszSrc      -  source string which must be null terminated

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL.  See RtlStringCbCatEx if you require
    the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated and
                       the resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error occurs,
                       the destination buffer is modified to contain a truncated
                       version of the ideal result and is null terminated. This
                       is useful for situations where truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCbCatA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc);

NTSTRSAFEDDI
RtlStringCbCatW(
    __inout_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbCatA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc)
{
    NTSTATUS status;
    size_t cchDest;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatWorkerA(pszDest, cchDest, pszSrc);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbCatW(
    __inout_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc)
{
    NTSTATUS status;
    size_t cchDest;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatWorkerW(pszDest, cchDest, pszSrc);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchCatEx(
    IN OUT LPTSTR  pszDest         OPTIONAL,
    IN     size_t  cchDest,
    IN     LPCTSTR pszSrc          OPTIONAL,
    OUT    LPTSTR* ppszDestEnd     OPTIONAL,
    OUT    size_t* pcchRemaining   OPTIONAL,
    IN     DWORD   dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcat' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCchCat, this routine also returns a pointer to the end of the
    destination string and the number of characters left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string which must be null terminated

    cchDest         -   size of destination buffer in characters
                        length must be (_tcslen(pszDest) + _tcslen(pszSrc) + 1)
                        to hold all of the combine string plus the null
                        terminator.

    pszSrc          -   source string which must be null terminated

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character

    pcchRemaining   -   if pcchRemaining is non-null, the function will return the
                        number of characters left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcat

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, pszDest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated and
                       the resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchCatExA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCchCatExW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchCatExA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(char) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
        cbDest = cchDest * sizeof(char);

        status = RtlStringCatExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchCatExW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(wchar_t) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringCatExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbCatEx(
    IN OUT LPTSTR  pszDest         OPTIONAL,
    IN     size_t  cbDest,
    IN     LPCTSTR pszSrc          OPTIONAL,
    OUT    LPTSTR* ppszDestEnd     OPTIONAL,
    OUT    size_t* pcbRemaining    OPTIONAL,
    IN     DWORD   dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcat' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCbCat, this routine also returns a pointer to the end of the
    destination string and the number of bytes left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string which must be null terminated

    cbDest          -   size of destination buffer in bytes.
                        length must be ((_tcslen(pszDest) + _tcslen(pszSrc) + 1) * sizeof(TCHAR)
                        to hold all of the combine string plus the null
                        terminator.

    pszSrc          -   source string which must be null terminated

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character

    pcbRemaining    -   if pcbRemaining is non-null, the function will return
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcat

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, pszDest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated
                       and the resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCbCatExA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCbCatExW(
    __inout_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbCatExA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatExWorkerA(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(char) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbCatExW(
    __inout_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatExWorkerW(pszDest, cchDest, cbDest, pszSrc, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(wchar_t) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchCatN(
    IN OUT LPTSTR  pszDest,
    IN     size_t  cchDest,
    IN     LPCTSTR pszSrc,
    IN     size_t  cchToAppend
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat'.
    The size of the destination buffer (in characters) is a parameter as well as
    the maximum number of characters to append, excluding the null terminator.
    This function will not write past the end of the destination buffer and it will
    ALWAYS null terminate pszDest (unless it is zero length).

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if all of pszSrc or the first cchToAppend characters were appended
    to the destination string and it was null terminated, otherwise it will
    return a failure code. In failure cases as much of pszSrc will be appended
    to pszDest as possible, and pszDest will be null terminated.

Arguments:

    pszDest         -   destination string which must be null terminated

    cchDest         -   size of destination buffer in characters.
                        length must be (_tcslen(pszDest) + min(cchToAppend, _tcslen(pszSrc)) + 1)
                        to hold all of the combine string plus the null
                        terminator.

    pszSrc          -   source string

    cchToAppend     -   maximum number of characters to append

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL. See RtlStringCchCatNEx if you require
    the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if all of pszSrc or the first cchToAppend characters
                       were concatenated to pszDest and the resultant dest
                       string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchCatNA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToAppend);

NTSTRSAFEDDI
RtlStringCchCatNW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchCatNA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToAppend)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatNWorkerA(pszDest, cchDest, pszSrc, cchToAppend);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchCatNW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatNWorkerW(pszDest, cchDest, pszSrc, cchToAppend);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbCatN(
    IN OUT LPTSTR  pszDest,
    IN     size_t  cbDest,
    IN     LPCTSTR pszSrc,
    IN     size_t  cbToAppend
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat'.
    The size of the destination buffer (in bytes) is a parameter as well as
    the maximum number of bytes to append, excluding the null terminator.
    This function will not write past the end of the destination buffer and it will
    ALWAYS null terminate pszDest (unless it is zero length).

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if all of pszSrc or the first cbToAppend bytes were appended
    to the destination string and it was null terminated, otherwise it will
    return a failure code. In failure cases as much of pszSrc will be appended
    to pszDest as possible, and pszDest will be null terminated.

Arguments:

    pszDest         -   destination string which must be null terminated

    cbDest          -   size of destination buffer in bytes.
                        length must be ((_tcslen(pszDest) + min(cbToAppend / sizeof(TCHAR), _tcslen(pszSrc)) + 1) * sizeof(TCHAR)
                        to hold all of the combine string plus the null
                        terminator.

    pszSrc          -   source string

    cbToAppend      -   maximum number of bytes to append

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL. See RtlStringCbCatNEx if you require
    the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if all of pszSrc or the first cbToAppend bytes were
                       concatenated to pszDest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCbCatNA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cbToAppend);

NTSTRSAFEDDI
RtlStringCbCatNW(
    __inout_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToAppend);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbCatNA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cbToAppend)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchToAppend;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);
    cchToAppend = cbToAppend / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatNWorkerA(pszDest, cchDest, pszSrc, cchToAppend);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbCatNW(
    __inout_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToAppend)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchToAppend;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);
    cchToAppend = cbToAppend / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatNWorkerW(pszDest, cchDest, pszSrc, cchToAppend);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchCatNEx(
    IN OUT LPTSTR  pszDest         OPTIONAL,
    IN     size_t  cchDest,
    IN     LPCTSTR pszSrc          OPTIONAL,
    IN     size_t  cchToAppend,
    OUT    LPTSTR* ppszDestEnd     OPTIONAL,
    OUT    size_t* pcchRemaining   OPTIONAL,
    IN     DWORD   dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat', with
    some additional parameters.  In addition to functionality provided by
    RtlStringCchCatN, this routine also returns a pointer to the end of the
    destination string and the number of characters left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string which must be null terminated

    cchDest         -   size of destination buffer in characters.
                        length must be (_tcslen(pszDest) + min(cchToAppend, _tcslen(pszSrc)) + 1)
                        to hold all of the combine string plus the null
                        terminator.

    pszSrc          -   source string

    cchToAppend     -   maximum number of characters to append

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character

    pcchRemaining   -   if pcchRemaining is non-null, the function will return the
                        number of characters left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, pszDest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if all of pszSrc or the first cchToAppend characters
                       were concatenated to pszDest and the resultant dest
                       string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchCatNExA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToAppend,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCchCatNExW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchCatNExA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToAppend,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(char) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
        cbDest = cchDest * sizeof(char);

        status = RtlStringCatNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchToAppend, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchCatNExW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(wchar_t) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringCatNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchToAppend, ppszDestEnd, pcchRemaining, dwFlags);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbCatNEx(
    IN OUT LPTSTR  pszDest         OPTIONAL,
    IN     size_t  cbDest,
    IN     LPCTSTR pszSrc          OPTIONAL,
    IN     size_t  cbToAppend,
    OUT    LPTSTR* ppszDestEnd     OPTIONAL,
    OUT    size_t* pcchRemaining   OPTIONAL,
    IN     DWORD   dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat', with
    some additional parameters.  In addition to functionality provided by
    RtlStringCbCatN, this routine also returns a pointer to the end of the
    destination string and the number of bytes left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string which must be null terminated

    cbDest          -   size of destination buffer in bytes.
                        length must be ((_tcslen(pszDest) + min(cbToAppend / sizeof(TCHAR), _tcslen(pszSrc)) + 1) * sizeof(TCHAR)
                        to hold all of the combine string plus the null
                        terminator.

    pszSrc          -   source string

    cbToAppend      -   maximum number of bytes to append

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character

    pcbRemaining    -   if pcbRemaining is non-null, the function will return the
                        number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, pszDest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if all of pszSrc or the first cbToAppend bytes were
                       concatenated to pszDest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCbCatNExA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cbToAppend,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags);

NTSTRSAFEDDI
RtlStringCbCatNExW(
    __inout_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToAppend,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbCatNExA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cbToAppend,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchToAppend;
    size_t cchRemaining = 0;
     
    // convert to count of characters
    cchDest = cbDest / sizeof(char);
    cchToAppend = cbToAppend / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatNExWorkerA(pszDest, cchDest, cbDest, pszSrc, cchToAppend, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(char) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbCatNExW(
    __inout_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in_bcount(cbToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToAppend,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchToAppend;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);
    cchToAppend = cbToAppend / sizeof(wchar_t);
    
    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringCatNExWorkerW(pszDest, cchDest, cbDest, pszSrc, cchToAppend, ppszDestEnd, &cchRemaining, dwFlags);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(wchar_t) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchVPrintf(
    OUT LPTSTR  pszDest,
    IN  size_t  cchDest,
    IN  LPCTSTR pszFormat,
    IN  va_list argList
    );

Routine Description:

    This routine is a safer version of the C built-in function 'vsprintf'.
    The size of the destination buffer (in characters) is a parameter and
    this function will not write past the end of this buffer and it will
    ALWAYS null terminate the destination buffer (unless it is zero length).

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was printed without truncation and null terminated,
    otherwise it will return a failure code. In failure cases it will return
    a truncated version of the ideal result.

Arguments:

    pszDest     -  destination string

    cchDest     -  size of destination buffer in characters
                   length must be sufficient to hold the resulting formatted
                   string, including the null terminator.

    pszFormat   -  format string which must be null terminated

    argList     -  va_list from the variable arguments according to the
                   stdarg.h convention

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    pszDest and pszFormat should not be NULL.  See RtlStringCchVPrintfEx if you
    require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was sufficient space in the dest buffer for
                       the resultant string and it was null terminated.

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchVPrintfA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    va_list argList);

NTSTRSAFEDDI
RtlStringCchVPrintfW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchVPrintfA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchVPrintfW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbVPrintf(
    OUT LPTSTR  pszDest,
    IN  size_t  cbDest,
    IN  LPCTSTR pszFormat,
    IN  va_list argList
    );

Routine Description:

    This routine is a safer version of the C built-in function 'vsprintf'.
    The size of the destination buffer (in bytes) is a parameter and
    this function will not write past the end of this buffer and it will
    ALWAYS null terminate the destination buffer (unless it is zero length).

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was printed without truncation and null terminated,
    otherwise it will return a failure code. In failure cases it will return
    a truncated version of the ideal result.

Arguments:

    pszDest     -  destination string

    cbDest      -  size of destination buffer in bytes
                   length must be sufficient to hold the resulting formatted
                   string, including the null terminator.

    pszFormat   -  format string which must be null terminated

    argList     -  va_list from the variable arguments according to the
                   stdarg.h convention

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    pszDest and pszFormat should not be NULL.  See RtlStringCbVPrintfEx if you
    require the handling of NULL values.


Return Value:

    STATUS_SUCCESS -   if there was sufficient space in the dest buffer for
                       the resultant string and it was null terminated.

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCbVPrintfA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList);

NTSTRSAFEDDI
RtlStringCbVPrintfW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbVPrintfA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status;
    size_t cchDest;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbVPrintfW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status;
    size_t cchDest;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS

#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchPrintf(
    OUT LPTSTR  pszDest,
    IN  size_t  cchDest,
    IN  LPCTSTR pszFormat,
    ...
    );

Routine Description:

    This routine is a safer version of the C built-in function 'sprintf'.
    The size of the destination buffer (in characters) is a parameter and
    this function will not write past the end of this buffer and it will
    ALWAYS null terminate the destination buffer (unless it is zero length).

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was printed without truncation and null terminated,
    otherwise it will return a failure code. In failure cases it will return
    a truncated version of the ideal result.

Arguments:

    pszDest     -  destination string

    cchDest     -  size of destination buffer in characters
                   length must be sufficient to hold the resulting formatted
                   string, including the null terminator.

    pszFormat   -  format string which must be null terminated

    ...         -  additional parameters to be formatted according to
                   the format string

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    pszDest and pszFormat should not be NULL.  See RtlStringCchPrintfEx if you
    require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was sufficient space in the dest buffer for
                       the resultant string and it was null terminated.

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchPrintfA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    ...);

NTSTRSAFEDDI
RtlStringCchPrintfW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...);

#ifdef NTSTRSAFE_INLINE
#pragma warning( push )
#pragma warning( disable : 4793 )
NTSTRSAFEDDI
RtlStringCchPrintfA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    ...)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchPrintfW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return status;
}
#pragma warning( pop )
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbPrintf(
    OUT LPTSTR  pszDest,
    IN  size_t  cbDest,
    IN  LPCTSTR pszFormat,
    ...
    );

Routine Description:

    This routine is a safer version of the C built-in function 'sprintf'.
    The size of the destination buffer (in bytes) is a parameter and
    this function will not write past the end of this buffer and it will
    ALWAYS null terminate the destination buffer (unless it is zero length).

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was printed without truncation and null terminated,
    otherwise it will return a failure code. In failure cases it will return
    a truncated version of the ideal result.

Arguments:

    pszDest     -  destination string

    cbDest      -  size of destination buffer in bytes
                   length must be sufficient to hold the resulting formatted
                   string, including the null terminator.

    pszFormat   -  format string which must be null terminated

    ...         -  additional parameters to be formatted according to
                   the format string

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    pszDest and pszFormat should not be NULL.  See RtlStringCbPrintfEx if you
    require the handling of NULL values.


Return Value:

    STATUS_SUCCESS -   if there was sufficient space in the dest buffer for
                       the resultant string and it was null terminated.

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/
    
NTSTRSAFEDDI
RtlStringCbPrintfA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    ...);

NTSTRSAFEDDI
RtlStringCbPrintfW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...);

#ifdef NTSTRSAFE_INLINE
#pragma warning( push )
#pragma warning( disable : 4793 )
NTSTRSAFEDDI
RtlStringCbPrintfA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    ...)
{
    NTSTATUS status;
    size_t cchDest;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfWorkerA(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbPrintfW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...)
{
    NTSTATUS status;
    size_t cchDest;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfWorkerW(pszDest, cchDest, pszFormat, argList);

        va_end(argList);
    }

    return status;
}
#pragma warning ( pop )
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchPrintfEx(
    OUT LPTSTR  pszDest         OPTIONAL,
    IN  size_t  cchDest,
    OUT LPTSTR* ppszDestEnd     OPTIONAL,
    OUT size_t* pcchRemaining   OPTIONAL,
    IN  DWORD   dwFlags,
    IN  LPCTSTR pszFormat       OPTIONAL,
    ...
    );

Routine Description:

    This routine is a safer version of the C built-in function 'sprintf' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCchPrintf, this routine also returns a pointer to the end of the
    destination string and the number of characters left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string

    cchDest         -   size of destination buffer in characters.
                        length must be sufficient to contain the resulting
                        formatted string plus the null terminator.

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function printed any data, the result will point to the
                        null termination character

    pcchRemaining   -   if pcchRemaining is non-null, the function will return
                        the number of characters left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

    pszFormat       -   format string which must be null terminated

    ...             -   additional parameters to be formatted according to
                        the format string

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    pszDest and pszFormat should not be NULL unless the STRSAFE_IGNORE_NULLS
    flag is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and
    pszFormat may be NULL.  An error may still be returned even though NULLS
    are ignored due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated and
                       the resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchPrintfExA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    ...);

NTSTRSAFEDDI
RtlStringCchPrintfExW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...);

#ifdef NTSTRSAFE_INLINE
#pragma warning( push )
#pragma warning( disable : 4793 )
NTSTRSAFEDDI
RtlStringCchPrintfExA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    ...)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;
        va_list argList;

        // safe to multiply cchDest * sizeof(char) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
        cbDest = cchDest * sizeof(char);
        va_start(argList, pszFormat);

        status = RtlStringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchPrintfExW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;
        va_list argList;

        // safe to multiply cchDest * sizeof(wchar_t) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
        cbDest = cchDest * sizeof(wchar_t);
        va_start(argList, pszFormat);

        status = RtlStringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    return status;
}
#pragma warning( pop )
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbPrintfEx(
    OUT LPTSTR  pszDest         OPTIONAL,
    IN  size_t  cbDest,
    OUT LPTSTR* ppszDestEnd     OPTIONAL,
    OUT size_t* pcbRemaining    OPTIONAL,
    IN  DWORD   dwFlags,
    IN  LPCTSTR pszFormat       OPTIONAL,
    ...
    );

Routine Description:

    This routine is a safer version of the C built-in function 'sprintf' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCbPrintf, this routine also returns a pointer to the end of the
    destination string and the number of bytes left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string

    cbDest          -   size of destination buffer in bytes.
                        length must be sufficient to contain the resulting
                        formatted string plus the null terminator.

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function printed any data, the result will point to the
                        null termination character

    pcbRemaining    -   if pcbRemaining is non-null, the function will return
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

    pszFormat       -   format string which must be null terminated

    ...             -   additional parameters to be formatted according to
                        the format string

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    pszDest and pszFormat should not be NULL unless the STRSAFE_IGNORE_NULLS
    flag is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and
    pszFormat may be NULL.  An error may still be returned even though NULLS
    are ignored due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated and
                       the resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCbPrintfExA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    ...);

NTSTRSAFEDDI
RtlStringCbPrintfExW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...);

#ifdef NTSTRSAFE_INLINE
#pragma warning( push )
#pragma warning( disable : 4793 )
NTSTRSAFEDDI
RtlStringCbPrintfExA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    ...)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(char) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbPrintfExW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        va_list argList;

        va_start(argList, pszFormat);

        status = RtlStringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);

        va_end(argList);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(wchar_t) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#pragma warning( pop )
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS

#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchVPrintfEx(
    OUT LPTSTR  pszDest         OPTIONAL,
    IN  size_t  cchDest,
    OUT LPTSTR* ppszDestEnd     OPTIONAL,
    OUT size_t* pcchRemaining   OPTIONAL,
    IN  DWORD   dwFlags,
    IN  LPCTSTR pszFormat       OPTIONAL,
    IN  va_list argList
    );

Routine Description:

    This routine is a safer version of the C built-in function 'vsprintf' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCchVPrintf, this routine also returns a pointer to the end of the
    destination string and the number of characters left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string

    cchDest         -   size of destination buffer in characters.
                        length must be sufficient to contain the resulting
                        formatted string plus the null terminator.

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function printed any data, the result will point to the
                        null termination character

    pcchRemaining   -   if pcchRemaining is non-null, the function will return
                        the number of characters left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

    pszFormat       -   format string which must be null terminated

    argList         -   va_list from the variable arguments according to the
                        stdarg.h convention

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    pszDest and pszFormat should not be NULL unless the STRSAFE_IGNORE_NULLS
    flag is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and
    pszFormat may be NULL.  An error may still be returned even though NULLS
    are ignored due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated and
                       the resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCchVPrintfExA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList);

NTSTRSAFEDDI
RtlStringCchVPrintfExW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchVPrintfExA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(char) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
        cbDest = cchDest * sizeof(char);

        status = RtlStringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchVPrintfExW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status;

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cbDest;

        // safe to multiply cchDest * sizeof(wchar_t) since cchDest < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
        cbDest = cchDest * sizeof(wchar_t);

        status = RtlStringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, pcchRemaining, dwFlags, pszFormat, argList);
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbVPrintfEx(
    OUT LPTSTR  pszDest         OPTIONAL,
    IN  size_t  cbDest,
    OUT LPTSTR* ppszDestEnd     OPTIONAL,
    OUT size_t* pcbRemaining    OPTIONAL,
    IN  DWORD   dwFlags,
    IN  LPCTSTR pszFormat       OPTIONAL,
    IN  va_list argList
    );

Routine Description:

    This routine is a safer version of the C built-in function 'vsprintf' with
    some additional parameters.  In addition to functionality provided by
    RtlStringCbVPrintf, this routine also returns a pointer to the end of the
    destination string and the number of characters left in the destination string
    including the null terminator. The flags parameter allows additional controls.

Arguments:

    pszDest         -   destination string

    cbDest          -   size of destination buffer in bytes.
                        length must be sufficient to contain the resulting
                        formatted string plus the null terminator.

    ppszDestEnd     -   if ppszDestEnd is non-null, the function will return
                        a pointer to the end of the destination string.  If the
                        function printed any data, the result will point to the
                        null termination character

    pcbRemaining    -   if pcbRemaining is non-null, the function will return
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

    pszFormat       -   format string which must be null terminated

    argList         -   va_list from the variable arguments according to the
                        stdarg.h convention

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    pszDest and pszFormat should not be NULL unless the STRSAFE_IGNORE_NULLS
    flag is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and
    pszFormat may be NULL.  An error may still be returned even though NULLS
    are ignored due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated and
                       the resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFEDDI
RtlStringCbVPrintfExA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList);

NTSTRSAFEDDI
RtlStringCbVPrintfExW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbVPrintfExA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cbDest,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(char);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfExWorkerA(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(char) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
            *pcbRemaining = (cchRemaining * sizeof(char)) + (cbDest % sizeof(char));
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbVPrintfExW(
    __out_bcount(cbDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cbDest,
    __deref_opt_out_bcount(*pcbRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcbRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status;
    size_t cchDest;
    size_t cchRemaining = 0;

    // convert to count of characters
    cchDest = cbDest / sizeof(wchar_t);

    if (cchDest > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringVPrintfExWorkerW(pszDest, cchDest, cbDest, ppszDestEnd, &cchRemaining, dwFlags, pszFormat, argList);
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (pcbRemaining)
        {
            // safe to multiply cchRemaining * sizeof(wchar_t) since cchRemaining < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
            *pcbRemaining = (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t));
        }
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS



#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlStringCchLength(
    IN  LPCTSTR psz,
    IN  size_t  cchMax,
    OUT size_t* pcchLength  OPTIONAL
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strlen'.
    It is used to make sure a string is not larger than a given length, and
    it optionally returns the current length in characters not including
    the null terminator.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string is non-null and the length including the null
    terminator is less than or equal to cchMax characters.

Arguments:

    psz         -   string to check the length of

    cchMax      -   maximum number of characters including the null terminator
                    that psz is allowed to contain

    pcch        -   if the function succeeds and pcch is non-null, the current length
                    in characters of psz excluding the null terminator will be returned.
                    This out parameter is equivalent to the return value of strlen(psz)

Notes:
    psz can be null but the function will fail

    cchMax should be greater than zero or the function will fail

Return Value:

    STATUS_SUCCESS -   psz is non-null and the length including the null
                       terminator is less than or equal to cchMax characters

    failure        -   the operation did not succeed


    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFEDDI
RtlStringCchLengthA(
    __in NTSTRSAFE_PCSTR psz,
    __in size_t cchMax,
    __out_opt size_t* pcchLength);

NTSTRSAFEDDI
RtlStringCchLengthW(
    __in NTSTRSAFE_PCWSTR psz,
    __in size_t cchMax,
    __out_opt size_t* pcchLength);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCchLengthA(
    __in NTSTRSAFE_PCSTR psz,
    __in size_t cchMax,
    __out_opt size_t* pcchLength)
{
    NTSTATUS status;

    if ((psz == NULL) || (cchMax > NTSTRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringLengthWorkerA(psz, cchMax, pcchLength);
    }
    
    if (!NT_SUCCESS(status) && pcchLength)
    {
        *pcchLength = 0;
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCchLengthW(
    __in NTSTRSAFE_PCWSTR psz,
    __in size_t cchMax,
    __out_opt size_t* pcchLength)
{
    NTSTATUS status;

    if ((psz == NULL) || (cchMax > NTSTRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringLengthWorkerW(psz, cchMax, pcchLength);
    }
    
    if (!NT_SUCCESS(status) && pcchLength)
    {
        *pcchLength = 0;
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlStringCbLength(
    IN  LPCTSTR psz,
    IN  size_t  cbMax,
    OUT size_t* pcbLength   OPTIONAL
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strlen'.
    It is used to make sure a string is not larger than a given length, and
    it optionally returns the current length in bytes not including
    the null terminator.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string is non-null and the length including the null
    terminator is less than or equal to cbMax bytes.

Arguments:

    psz         -   string to check the length of

    cbMax       -   maximum number of bytes including the null terminator
                    that psz is allowed to contain

    pcb         -   if the function succeeds and pcb is non-null, the current length
                    in bytes of psz excluding the null terminator will be returned.
                    This out parameter is equivalent to the return value of strlen(psz) * sizeof(TCHAR)

Notes:
    psz can be null but the function will fail

    cbMax should be greater than or equal to sizeof(TCHAR) or the function will fail

Return Value:

    STATUS_SUCCESS -   psz is non-null and the length including the null
                       terminator is less than or equal to cbMax bytes

    failure        -   the operation did not succeed


    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFEDDI
RtlStringCbLengthA(
    __in NTSTRSAFE_PCSTR psz,
    __in size_t cbMax,
    __out_opt size_t* pcbLength);

NTSTRSAFEDDI
RtlStringCbLengthW(
    __in NTSTRSAFE_PCWSTR psz,
    __in size_t cbMax,
    __out_opt size_t* pcbLength);

#ifdef NTSTRSAFE_INLINE
NTSTRSAFEDDI
RtlStringCbLengthA(
    __in NTSTRSAFE_PCSTR psz,
    __in size_t cbMax,
    __out_opt size_t* pcbLength)
{
    NTSTATUS status;
    size_t cchMax;
    size_t cchLength = 0;

    // convert to count of characters
    cchMax = cbMax / sizeof(char);

    if ((psz == NULL) || (cchMax > NTSTRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringLengthWorkerA(psz, cchMax, &cchLength);
    }

    if (pcbLength)
    {
        if (NT_SUCCESS(status))
        {
            // safe to multiply cch * sizeof(char) since cch < NTSTRSAFE_MAX_CCH and sizeof(char) is 1
            *pcbLength = cchLength * sizeof(char);
        }
        else
        {
            *pcbLength = 0;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCbLengthW(
    __in NTSTRSAFE_PCWSTR psz,
    __in size_t cbMax,
    __out_opt size_t* pcbLength)
{
    NTSTATUS status;
    size_t cchMax;
    size_t cchLength = 0;

    // convert to count of characters
    cchMax = cbMax / sizeof(wchar_t);

    if ((psz == NULL) || (cchMax > NTSTRSAFE_MAX_CCH))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlStringLengthWorkerW(psz, cchMax, &cchLength);
    }

    if (pcbLength)
    {
        if (NT_SUCCESS(status))
        {
            // safe to multiply cch * sizeof(wchar_t) since cch < NTSTRSAFE_MAX_CCH and sizeof(wchar_t) is 2
            *pcbLength = cchLength * sizeof(wchar_t);
        }
        else
        {
            *pcbLength = 0;
        }
    }

    return status;
}
#endif  // NTSTRSAFE_INLINE
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS

#ifndef NTSTRSAFE_NO_UNICODE_STRING_FUNCTIONS
#ifndef NTSTRSAFE_LIB_IMPL

/*++

NTSTATUS
RtlUnicodeStringInit(
    OUT PUNICODE_STRING Dest,
    IN  NTSTRSAFE_PCWSTR pszSrc  OPTIONAL
    );

Routine Description:

    The RtlUnicodeStringInit function initializes a counted unicode string from
    pszSrc.

    This function returns an NTSTATUS value.  It returns STATUS_SUCCESS if the 
    counted unicode string was sucessfully initialized from pszSrc. In failure
    cases the unicode string buffer will be set to NULL, and the Length and 
    MaximumLength members will be set to zero.

Arguments:

    Dest        - pointer to the counted unicode string to be initialized

    pszSrc      - source string which must be null or null terminated


Return Value:

    STATUS_SUCCESS -   

    failure        -   the operation did not succeed

      STATUS_INVALID_PARAMETER
                   -   this return value is an indication that the source string
                       was too large and Dest could not be initialized

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringInit(
    __out PUNICODE_STRING Dest,
    __in_opt NTSTRSAFE_PCWSTR pszSrc)
{
    return RtlUnicodeStringInitWorker(Dest, pszSrc, NTSTRSAFE_UNICODE_STRING_MAX_CCH, 0);
}


/*++

NTSTATUS
RtlUnicodeStringInitEx(
    OUT PUNICODE_STRING Dest,
    IN  NTSTRSAFE_PCWSTR pszSrc  OPTIONAL,
    IN  DWORD           dwFlags
    );

Routine Description:

    In addition to functionality provided by RtlUnicodeStringInit, this routine
    includes the flags parameter allows additional controls.

    This function returns an NTSTATUS value.  It returns STATUS_SUCCESS if the 
    counted unicode string was sucessfully initialized from pszSrc. In failure
    cases the unicode string buffer will be set to NULL, and the Length and 
    MaximumLength members will be set to zero.

Arguments:

    Dest        - pointer to the counted unicode string to be initialized

    pszSrc      - source string which must be null terminated

    dwFlags     - controls some details of the initialization:

        STRSAFE_IGNORE_NULLS
        STRSAFE_IGNORE_NULL_UNICODE_STRINGS
                    do not fault on a NULL Dest pointer

        STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED
                    if pszSrc is non-null, then make sure that strlen(pszSrc)
                    is less than NTSTRSAFE_MAX_CCH.

Return Value:

    STATUS_SUCCESS -   

    failure        -   the operation did not succeed

      STATUS_INVALID_PARAMETER
                   -   this return value is an indication that the source string
                       was too large and Dest could not be initialized

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringInitEx(
    __out PUNICODE_STRING Dest,
    __in_opt NTSTRSAFE_PCWSTR pszSrc,
    __in unsigned long dwFlags)
{
    return RtlUnicodeStringInitWorker(Dest, pszSrc, NTSTRSAFE_UNICODE_STRING_MAX_CCH, dwFlags);
}


/*++

NTSTATUS
RtlUnicodeStringValidate(
    IN  PCUNICODE_STRING    Src
    );

Routine Description:

    The RtlUnicodeStringValidate function checks the counted unicode string to make
    sure that is is valid.

    This function returns an NTSTATUS value.  It returns STATUS_SUCCESS if the 
    counted unicode string is valid.

Arguments:

    Src         - pointer to the counted unicode string to be checked

Return Value:

    STATUS_SUCCESS -   

    failure        -   the operation did not succeed

      STATUS_INVALID_PARAMETER
                   -   this return value is an indication that the source string
                       is not a valide counted unicode string

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringValidate(
    __in PCUNICODE_STRING Src)
{
    return RtlUnicodeStringValidateWorker(Src, NTSTRSAFE_UNICODE_STRING_MAX_CCH, 0);
}


/*++

NTSTATUS
RtlUnicodeStringValidateEx(
    IN  PCUNICODE_STRING    Src     OPTIONAL,
    IN  DWORD               dwFlags
    );

Routine Description:

    In addition to functionality provided by RtlUnicodeStringValidate, this routine
    includes the flags parameter allows additional controls.

    This function returns an NTSTATUS value.  It returns STATUS_SUCCESS if the 
    counted unicode string is valid.

Arguments:

    Src         - pointer to the counted unicode string to be checked

    dwFlags     - controls some details of the validation:

        STRSAFE_IGNORE_NULLS
        STRSAFE_IGNORE_NULL_UNICODE_STRINGS
                    allows Src to be NULL (will return STATUS_SUCCESS for this case).

        STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED
                    makes sure that the Buffer member of Src is either null or null terminated.

Return Value:

    STATUS_SUCCESS -   

    failure        -   the operation did not succeed

      STATUS_INVALID_PARAMETER
                   -   this return value is an indication that the source string
                       is not a valide counted unicode string given the flags passed.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringValidateEx(
    __in PCUNICODE_STRING Src,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    
    if (dwFlags & (~STRSAFE_UNICODE_STRING_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        status = RtlUnicodeStringValidateWorker(Src, NTSTRSAFE_UNICODE_STRING_MAX_CCH, dwFlags);
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringCopyString(
    OUT PUNICODE_STRING Dest,
    IN  LPCTSTR         pszSrc
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcpy' for
    UNICODE_STRINGs.

    This routine is not a replacement for strncpy.  That function will pad the
    destination string with extra null termination characters if the count is
    greater than the length of the source string, and it will fail to null
    terminate the destination string if the source string length is greater
    than or equal to the count. You can not blindly use this instead of strncpy:
    it is common for code to use it to "patch" strings and you would introduce
    errors if the code started null terminating in the middle of the string.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was copied without truncation and null terminated,
    otherwise it will return a failure code. In failure cases as much of pszSrc
    will be copied to pszDest as possible, and pszDest will be null terminated.

Arguments:

    Dest            -  destination PUNICODE_STRING

    pszSrc         -   source string which must be null terminated

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and pszSrc should not be NULL.  See RtlUnicodeStringCbCopyEx
    or RtlUnicodeStringCchCopyEx if you require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this 
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyString(
    __out PUNICODE_STRING Dest,
    __in NTSTRSAFE_PCWSTR pszSrc)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        size_t cchNewDestLength = 0;
        
        status = RtlUnicodeStringCopyStringWorker(pszDest,
                                                  cchDest,
                                                  &cchNewDestLength,
                                                  pszSrc,
                                                  0);

        Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringCopyUnicodeString(
    OUT PUNICODE_STRING     Dst,
    IN  PCUNICODE_STRING    Src
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcpy' for
    UNICODE_STRINGs.

    This routine is not a replacement for strncpy.  That function will pad the
    destination string with extra null termination characters if the count is
    greater than the length of the source string, and it will fail to null
    terminate the destination string if the source string length is greater
    than or equal to the count. You can not blindly use this instead of strncpy:
    it is common for code to use it to "patch" strings and you would introduce
    errors if the code started null terminating in the middle of the string.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was copied without truncation and null terminated,
    otherwise it will return a failure code. In failure cases as much of pszSrc
    will be copied to pszDest as possible, and pszDest will be null terminated.

Arguments:

    Dest            -  destination PUNICODE_STRING

    Src             -  source PUNICODE_STRING

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL.  See RtlUnicodeStringCbCopyEx
    or RtlUnicodeStringCchCopyEx if you require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyUnicodeString(
    __in PUNICODE_STRING Dest, 
    __out PCUNICODE_STRING Src)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszSrc;
        size_t cchSrcLength;
        size_t cchNewDestLength = 0;
        
        status = RtlUnicodeStringValidateSrcWorker(Src,
                                                   &pszSrc,
                                                   &cchSrcLength,
                                                   NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                   0);

        if (NT_SUCCESS(status))
        {
            status = RtlUnicodeStringCopyWorker(pszDest,
                                                cchDest,
                                                &cchNewDestLength,
                                                pszSrc,
                                                cchSrcLength,
                                                0);
        }

        Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringCopyStringEx(
    OUT PUNICODE_STRING Dest            OPTIONAL,
    IN  LPCTSTR         pszSrc          OPTIONAL,
    OUT PUNICODE_STRING RemainingString OPTIONAL,
    IN  DWORD           dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcpy' for
    PUNICODE_STRINGs with some additional parameters.  In addition to
    functionality provided by RtlUnicodeStringCopy, this routine also returns a
    PUNICODE_STRING to the end of the destination string and the number of bytes
    left in the destination string including the null terminator.  The flags
    parameter allows additional controls.

Arguments:

    Dest            -   destination PUNICODE_STRING

    pszSrc          -   source string which must be null terminated

    RemainingString -   if RemainingString is non-null,the function will format
                        the pointer with the remaining buffer and number of
                        bytes left in the destination string, including the null
                        terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    do not fault if Dest is null and treat NULL pszSrc like
                    empty strings (TEXT("")). this flag is useful for emulating
                    functions like lstrcpy 

        STRSAFE_IGNORE_NULL_UNICODE_STRINGS
                    do not fault if Dest is null

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

    Behavior is undefined if Dest and RemainingString are the same pointer.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyStringEx(
    __out PUNICODE_STRING Dest, 
    __in NTSTRSAFE_PCWSTR pszSrc,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszDestEnd;
        size_t cchRemaining; 
        size_t cchNewDestLength = 0;
        
        status = RtlUnicodeStringCopyStringExWorker(pszDest,
                                                    cchDest,
                                                    &cchNewDestLength,
                                                    pszSrc,
                                                    &pszDestEnd,
                                                    &cchRemaining,
                                                    dwFlags);

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE, STRSAFE_NULL_ON_FAILURE, and STRSAFE_NO_TRUNCATION flags
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags);
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringCopyUnicodeStringEx(
    OUT PUNICODE_STRING     Dest            OPTIONAL,
    IN  PCUNICODE_STRING    Src             OPTIONAL,
    OUT PUNICODE_STRING     RemainingString OPTIONAL,
    IN  DWORD               dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcpy' for
    PUNICODE_STRINGs with some additional parameters.  In addition to
    functionality provided by RtlUnicodeStringCopy, this routine also returns a
    PUNICODE_STRING to the end of the destination string and the number of bytes
    left in the destination string including the null terminator.  The flags
    parameter allows additional controls.

Arguments:

    Dest            -   destination PUNICODE_STRING

    Src             -   source PCUNICODE_STRING

    RemainingString -   if RemainingString is non-null,the function will format
                        the pointer with the remaining buffer and number of
                        bytes left in the destination string, including the null
                        terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcpy

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and Src
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

    Behavior is undefined if Dest and RemainingString are the same pointer.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyUnicodeStringEx(
    __out PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszSrc;
        size_t cchSrcLength;
        wchar_t* pszDestEnd = pszDest;
        size_t cchRemaining = cchDest; 
        size_t cchNewDestLength = 0;
        
        status = RtlUnicodeStringValidateSrcWorker(Src,
                                                   &pszSrc,
                                                   &cchSrcLength,
                                                   NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                   dwFlags);

        if (NT_SUCCESS(status))
        {
            status = RtlUnicodeStringCopyExWorker(pszDest,
                                                  cchDest,
                                                  &cchNewDestLength,
                                                  pszSrc,
                                                  cchSrcLength,
                                                  &pszDestEnd,
                                                  &cchRemaining,
                                                  dwFlags);
        }

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE, STRSAFE_NULL_ON_FAILURE, and STRSAFE_NO_TRUNCATION flags
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags);
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCopyCchStringN(
    OUT PUNICODE_STRING Dest,
    IN  LPCTSTR         pszSrc,
    IN  size_t          cchToCopy
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' for
    PUNICODE_STRINGs.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cchToCopy is greater than the length of pszSrc.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the entire string or the first cchToCopy characters were copied
    without truncation and the resultant destination string was null terminated,
    otherwise it will return a failure code. In failure cases as much of pszSrc
    will be copied to pszDest as possible, and pszDest will be null terminated.

Arguments:

    Dest           -   destination PUNICODE_STRING

    pszSrc         -   source string

    cchToCopy      -   maximum number of characters to copy from source string,
                       not including the null terminator.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and pszSrc should not be NULL. See RtlUnicodeStringCopyCchStringNEx if
    you require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyCchStringN(
    __out PUNICODE_STRING Dest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        size_t cchNewDestLength = 0;

        if (cchToCopy > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            status = RtlUnicodeStringCopyStringNWorker(pszDest,
                                                       cchDest,
                                                       &cchNewDestLength,
                                                       pszSrc,
                                                       cchToCopy,
                                                       0);
        }

        Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCopyCbStringN(
    OUT PUNICODE_STRING Dest,
    IN  LPCTSTR         pszSrc,
    IN  size_t          cbToCopy
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' for
    PUNICODE_STRINGs.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cbToCopy is greater than the size of pszSrc.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the entire string or the first cbToCopy characters were
    copied without truncation and the resultant destination string was null
    terminated, otherwise it will return a failure code. In failure cases as
    much of pszSrc will be copied to Dest as possible, and Dest will be
    null terminated if there is room.

Arguments:

    Dest           -   destination PUNICODE_STRING

    pszSrc         -   source string

    cbToCopy       -   maximum number of bytes to copy from source string,
                       not including the null terminator.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and pszSrc should not be NULL.  See RtlUnicodeStringCopyCbStringEx if you require
    the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyCbStringN(
    __out PUNICODE_STRING Dest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToCopy)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        size_t cchNewDestLength = 0;
        size_t cchToCopy;

        // convert to count of characters
        cchToCopy = cbToCopy / sizeof(wchar_t);

        if (cchToCopy > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            status = RtlUnicodeStringCopyStringNWorker(pszDest,
                                                       cchDest,
                                                       &cchNewDestLength,
                                                       pszSrc,
                                                       cchToCopy,
                                                       0);
        }

        Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCopyCchUnicodeStringN(
    OUT PUNICODE_STRING Dest,
    IN  PCUNICODE_STRING Src,
    IN  size_t cchToCopy
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' for
    PUNICODE_STRINGs.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cchToCopy is greater than the length of Src.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the entire string or the first cchToCopy characters were copied
    without truncation and the resultant destination string was null terminated,
    otherwise it will return a failure code. In failure cases as much of Src
    will be copied to Dest as possible, and Dest will be null terminated if
    there is room in the buffer.

Arguments:

    Dest           -   destination PUNICODE_STRING

    Src            -   source PCUNICODE_STRING

    cchToCopy      -   maximum number of characters to copy from source string,
                       not including the null terminator.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL. See RtlUnicodeStringCopyCchUnicodeStringNEx
    if you require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyCchUnicodeStringN(
    __out PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src,
    __in size_t cchToCopy)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        size_t cchNewDestLength = 0;

        if (cchToCopy > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            wchar_t* pszSrc;
            size_t cchSrcLength;

            status = RtlUnicodeStringValidateSrcWorker(Src,
                                                       &pszSrc,
                                                       &cchSrcLength,
                                                       NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                       0);

            if (NT_SUCCESS(status))
            {
                if (cchSrcLength < cchToCopy)
                {
                    cchToCopy = cchSrcLength;
                }

                status = RtlUnicodeStringCopyWorker(pszDest,
                                                    cchDest,
                                                    &cchNewDestLength,
                                                    pszSrc,
                                                    cchToCopy,
                                                    0);
            }
        }

        Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCopyCbUnicodeStringN(
    OUT PUNICODE_STRING     Dest,
    IN  PCUNICODE_STRING    Src,
    IN  size_t              cbToCopy
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' for
    PUNICODE_STRINGs.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cbToCopy is greater than the size of Src.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the entire string or the first cbToCopy characters were
    copied without truncation and the resultant destination string was null
    terminated, otherwise it will return a failure code. In failure cases as
    much of Src will be copied to Dest as possible, and Dest will be
    null terminated if there is room.

Arguments:

    Dest           -   destination PUNICODE_STRING

    Src            -   source PUNICODE_STRING

    cbToCopy       -   maximum number of bytes to copy from source string,
                       not including the null terminator.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL.  See RtlUnicodeStringCopyCbUnicodeStringEx
    if you require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function.

--*/


NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyCbUnicodeStringN(
    __out PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src,
    __in size_t cbToCopy)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        size_t cchNewDestLength = 0;
        size_t cchToCopy;

        // convert to count of characters
        cchToCopy = cbToCopy / sizeof(wchar_t);
        
        if (cchToCopy > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            wchar_t* pszSrc;
            size_t cchSrcLength;

            status = RtlUnicodeStringValidateSrcWorker(Src,
                                                       &pszSrc,
                                                       &cchSrcLength,
                                                       NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                       0);

            if (NT_SUCCESS(status))
            {
                if (cchSrcLength < cchToCopy)
                {
                    cchToCopy = cchSrcLength;
                }

                status = RtlUnicodeStringCopyWorker(pszDest,
                                                    cchDest,
                                                    &cchNewDestLength,
                                                    pszSrc,
                                                    cchToCopy,
                                                    0);
            }
        }

        Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCopyCchStringNEx(
    OUT PUNICODE_STRING Dest            OPTIONAL,
    IN  LPCTSTR         pszSrc          OPTIONAL,
    IN  size_t          cchToCopy,
    OUT PUNICODE_STRING RemainingString OPTIONAL,
    IN  DWORD           dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' with
    some additional parameters and for PUNICODE_STRINGs.  In addition to
    functionality provided by RtlUnicodeStringCopyCbStringN, this routine also
    returns a PUNICODE_STRING which points to the end of the
    destination string and the number of bytes left in the destination string
    including the null terminator. The flags parameter allows additional controls.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cbSrc is greater than the size of pszSrc.

Arguments:

    Dest            -   destination PUNICODE_STRING

    pszSrc          -   source string

    cchToCopy       -   maximum number of characters to copy from source string

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function copied any data, the result will point to the
                        null termination character.  MaximumLength will be the
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcpy

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    pszDest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both pszDest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyCchStringNEx(
    __out PUNICODE_STRING Dest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszDestEnd;
        size_t cchRemaining; 
        size_t cchNewDestLength = 0;

        status = RtlUnicodeStringCopyStringNExWorker(pszDest,
                                                     cchDest,
                                                     &cchNewDestLength,
                                                     pszSrc,
                                                     cchToCopy,
                                                     &pszDestEnd,
                                                     &cchRemaining,
                                                     dwFlags);

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE, STRSAFE_NULL_ON_FAILURE, and STRSAFE_NO_TRUNCATION flags
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                    cchDest,
                                                    &cchNewDestLength,
                                                    &pszDestEnd,
                                                    &cchRemaining,
                                                    dwFlags);
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCopyCbStringNEx(
    OUT PUNICODE_STRING Dest            OPTIONAL,
    IN  LPCTSTR         pszSrc          OPTIONAL,
    IN  size_t          cbToCopy,
    OUT PUNICODE_STRING RemainingString OPTIONAL,
    IN  DWORD           dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' with
    some additional parameters and for PUNICODE_STRINGs.  In addition to
    functionality provided by RtlUnicodeStringCopyCbStringN, this routine also
    returns a PUNICODE_STRING which points to the end of the
    destination string and the number of bytes left in the destination string
    including the null terminator. The flags parameter allows additional controls.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cbToCopy is greater than the size of pszSrc.

Arguments:

    Dest            -   destination PUNICODE_STRING

    pszSrc          -   source string

    cbToCopy        -   maximum number of bytes to copy from source string

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function copied any data, the result will point to the
                        null termination character.  MaximumLength will be the
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcpy

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyCbStringNEx(
    __out PUNICODE_STRING Dest,
    __in_bcount(cbToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToCopy,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszDestEnd;
        size_t cchRemaining; 
        size_t cchNewDestLength = 0;
        size_t cchToCopy;

        // convert to count of characters
        cchToCopy = cbToCopy / sizeof(wchar_t);

        status = RtlUnicodeStringCopyStringNExWorker(pszDest,
                                                     cchDest,
                                                     &cchNewDestLength,
                                                     pszSrc,
                                                     cchToCopy,
                                                     &pszDestEnd,
                                                     &cchRemaining,
                                                     dwFlags);

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE, STRSAFE_NULL_ON_FAILURE, and STRSAFE_NO_TRUNCATION flags
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                    cchDest,
                                                    &cchNewDestLength,
                                                    &pszDestEnd,
                                                    &cchRemaining,
                                                    dwFlags);
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCopyCchUnicodeStringNEx(
    OUT PUNICODE_STRING     Dest            OPTIONAL,
    IN  PCUNICODE_STRING    Src             OPTIONAL,
    IN  size_t              cchToCopy,
    OUT PUNICODE_STRING     RemainingString OPTIONAL,
    IN  DWORD               dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' with
    some additional parameters and for PUNICODE_STRINGs.  In addition to
    functionality provided by RtlUnicodeStringCopyCbStringN, this routine also
    returns a PUNICODE_STRING which points to the end of the
    destination string and the number of bytes left in the destination string
    including the null terminator. The flags parameter allows additional controls.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cbSrc is greater than the size of Src.

Arguments:

    Dest            -   destination PUNICODE_STRING

    Src             -   source string PUNICODE_STRING

    cchToCopy       -   maximum number of characters to copy from source string

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function copied any data, the result will point to the
                        null termination character.  MaximumLength will be the
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcpy

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and Src
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyCchUnicodeStringNEx(
    __out PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src,
    __in size_t cchToCopy,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszSrc;
        size_t cchSrcLength;
        wchar_t* pszDestEnd = pszDest;
        size_t cchRemaining = cchDest;
        size_t cchNewDestLength = 0;

        status = RtlUnicodeStringValidateSrcWorker(Src,
                                                   &pszSrc,
                                                   &cchSrcLength,
                                                   NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                   dwFlags);

        if (NT_SUCCESS(status))
        {
            if (cchSrcLength < cchToCopy)
            {
                cchToCopy = cchSrcLength;
            }

            status = RtlUnicodeStringCopyExWorker(pszDest,
                                                  cchDest,
                                                  &cchNewDestLength,
                                                  pszSrc,
                                                  cchToCopy,
                                                  &pszDestEnd,
                                                  &cchRemaining,
                                                  dwFlags);
        }

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE, STRSAFE_NULL_ON_FAILURE, and STRSAFE_NO_TRUNCATION flags
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags);
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCopyCbUnicodeStringNEx(
    OUT PUNICODE_STRING     Dest            OPTIONAL,
    IN  PCUNICODE_STRING    Src             OPTIONAL,
    IN  size_t              cbToCopy,
    OUT PUNICODE_STRING     RemainingString OPTIONAL,
    IN  DWORD               dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncpy' with
    some additional parameters and for PUNICODE_STRINGs.  In addition to
    functionality provided by RtlUnicodeStringCopyCbStringN, this routine also
    returns a PUNICODE_STRING which points to the end of the
    destination string and the number of bytes left in the destination string
    including the null terminator. The flags parameter allows additional controls.

    This routine is meant as a replacement for strncpy, but it does behave
    differently. This function will not pad the destination buffer with extra
    null termination characters if cbToCopy is greater than the size of Src.

Arguments:

    Dest            -   destination PUNICODE_STRING

    Src             -   source PUNICODE_STRING

    cbToCopy        -   maximum number of bytes to copy from source string

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function copied any data, the result will point to the
                        null termination character.  MaximumLength will be the
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcpy

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and Src
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all copied and the
                       resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the copy
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCopyCbUnicodeStringNEx(
    __out PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src,
    __in size_t cbToCopy,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszSrc;
        size_t cchSrcLength;
        wchar_t* pszDestEnd = pszDest;
        size_t cchRemaining = cchDest;
        size_t cchNewDestLength = 0;
        size_t cchToCopy;

        // convert to count of characters
        cchToCopy = cbToCopy / sizeof(wchar_t);

        status = RtlUnicodeStringValidateSrcWorker(Src,
                                                   &pszSrc,
                                                   &cchSrcLength,
                                                   NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                   dwFlags);

        if (NT_SUCCESS(status))
        {
            if (cchSrcLength < cchToCopy)
            {
                cchToCopy = cchSrcLength;
            }

            status = RtlUnicodeStringCopyExWorker(pszDest,
                                                  cchDest,
                                                  &cchNewDestLength,
                                                  pszSrc,
                                                  cchToCopy,
                                                  &pszDestEnd,
                                                  &cchRemaining,
                                                  dwFlags);
        }

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE, STRSAFE_NULL_ON_FAILURE, and STRSAFE_NO_TRUNCATION flags
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags);
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }


        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


/*++

NTSTATUS
RtlUnicodeStringCatString(
    IN OUT  PUNICODE_STRING Dest,
    IN      LPCTSTR         pszSrc
    );

    // BUGBUG (reinerf) - need the full comment here!

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatString(
    __inout PUNICODE_STRING Dest,
    __in NTSTRSAFE_PCWSTR pszSrc)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        size_t cchCopied = 0;

        status = RtlUnicodeStringCopyStringWorker(pszDest + cchDestLength,
                                                  cchDest - cchDestLength,
                                                  &cchCopied,
                                                  pszSrc,
                                                  0);

        Dest->Length = (USHORT)((cchDestLength + cchCopied) * sizeof(wchar_t));
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringCatUnicodeString(
    IN OUT  PUNICODE_STRING Dest,
    IN      PCUNICODE_STRING Src
    );

    // BUGBUG (reinerf) - need the full comment here!

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatUnicodeString(
    __inout PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszSrc;
        size_t cchSrcLength;
        
        status = RtlUnicodeStringValidateSrcWorker(Src,
                                                   &pszSrc,
                                                   &cchSrcLength,
                                                   NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                   0);

        if (NT_SUCCESS(status))
        {
            size_t cchCopied = 0;

            status = RtlUnicodeStringCopyWorker(pszDest + cchDestLength,
                                                cchDest - cchDestLength,
                                                &cchCopied,
                                                pszSrc,
                                                cchSrcLength,
                                                0);

            Dest->Length = (USHORT)((cchDestLength + cchCopied) * sizeof(wchar_t));
        }
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringCatStringEx(
    IN OUT  PUNICODE_STRING Dest            OPTTONAL,
    IN      LPCTSTR         pszSrc          OPTIONAL,
    OUT     PUNICODE_STRING RemainingString OPTIONAL,
    IN      DWORD           dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcat' for
    PUNICODE_STRINGs with some additional parameters.  In addition to functionality
    provided by RtlUnicodeStringCat, this routine also returns a pointer to the
    end of the destination string and the number of bytes left in the destination
    string including the null terminator in the form of a PUNICODE_STRING.  The
    flags parameter allows additional controls.

Arguments:

    Dest            -   destination PUNICODE_STRING

    Src             -   source string which must be null terminated

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character.  MaximumLength will be set to
                        the number of bytes left in the destination string  .

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcat

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, pszDest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap or if
    Dest and RemainingString are the same pointer.

    Dest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated
                       and the resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatStringEx(
    __inout PUNICODE_STRING Dest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszDestEnd = pszDest + cchDestLength;
        size_t cchRemaining = cchDest - cchDestLength;
        size_t cchNewDestLength = cchDestLength;
        size_t cchCopied = 0;

        status = RtlUnicodeStringCopyStringExWorker(pszDestEnd,
                                                    cchRemaining,
                                                    &cchCopied,
                                                    pszSrc,
                                                    &pszDestEnd,
                                                    &cchRemaining,
                                                    dwFlags);

        // handle the STRSAFE_NO_TRUNCATION flag
        if ((dwFlags & STRSAFE_NO_TRUNCATION)   &&
            !NT_SUCCESS(status)                 &&
            pszDest)
        {
            pszDestEnd = pszDest + cchDestLength;
            cchRemaining = cchDest - cchDestLength;

            cchNewDestLength = cchDestLength;

            // null terminate the original end of the string
            *pszDestEnd = L'\0';
        }
        else
        {
            cchNewDestLength = cchDestLength + cchCopied;
        }

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE flags,
                // we already handled the STRSAFE_NO_TRUNCATION so do not pass it through
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags & (~STRSAFE_NO_TRUNCATION));
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringCatUnicodeStringEx(
    IN OUT  PUNICODE_STRING     Dest            OPTIONAL,
    IN      PCUNICODE_STRING    Src             OPTIONAL,
    OUT     PUNICODE_STRING     RemainingString OPTIONAL
    IN      DWORD               dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strcat' for
    PUNICODE_STRINGs with some additional parameters.  In addition to functionality
    provided by RtlUnicodeStringCat, this routine also returns a pointer to the
    end of the destination string and the number of bytes left in the destination
    string including the null terminator in the form of a PUNICODE_STRING.  The
    flags parameter allows additional controls.

Arguments:

    Dest            -   destination PUNICODE_STRING

    Src             -   source PUNICODE_STRING

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character.  MaximumLength will be set to
                        the number of bytes left in the destination string  .

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT("")).
                    this flag is useful for emulating functions like lstrcat

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, Dest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap or if
    Dest and RemainingString are the same pointer.

    Dest and Src should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and Src
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated
                       and the resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatUnicodeStringEx(
    __inout PUNICODE_STRING Dest, 
    __in PCUNICODE_STRING Src,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszSrc;
        size_t cchSrcLength;
        wchar_t* pszDestEnd = pszDest + cchDestLength;
        size_t cchRemaining = cchDest - cchDestLength;
        size_t cchNewDestLength = cchDestLength;

        status = RtlUnicodeStringValidateSrcWorker(Src,
                                                   &pszSrc,
                                                   &cchSrcLength,
                                                   NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                   dwFlags);

        if (NT_SUCCESS(status))
        {
            size_t cchCopied = 0;

            status = RtlUnicodeStringCopyExWorker(pszDestEnd,
                                                  cchRemaining,
                                                  &cchCopied,
                                                  pszSrc,
                                                  cchSrcLength,
                                                  &pszDestEnd,
                                                  &cchRemaining,
                                                  dwFlags);

            // handle the STRSAFE_NO_TRUNCATION flag
            if ((dwFlags & STRSAFE_NO_TRUNCATION)   &&
                !NT_SUCCESS(status)                 &&
                pszDest)
            {
                pszDestEnd = pszDest + cchDestLength;
                cchRemaining = cchDest - cchDestLength;

                cchNewDestLength = cchDestLength;

                // null terminate the original end of the string
                *pszDestEnd = L'\0';
            }
            else
            {
                cchNewDestLength = cchDestLength + cchCopied;
            }
        }

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE flags,
                // we already handled the STRSAFE_NO_TRUNCATION so do not pass it through
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags & (~STRSAFE_NO_TRUNCATION));
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCatCchStringN(
    IN OUT  PUNICODE_STRING Dest,
    IN      LPCTSTR         pszSrc,
    IN      size_t          cchToAppend
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat' for
    PUNICODE_STRINGs.  This function will not write past the end of the
    destination buffer.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if all of pszSrc or the first cchToAppend characters were appended
    to the destination string and it was null terminated, otherwise it will
    return a failure code. In failure cases as much of pszSrc will be appended
    to Dest as possible, and Dest will be null terminated if there is room.

Arguments:

    Dest            -   destination PUNICODE_STRING

    pszSrc          -   source string

    cchToAppend     -   maximum number of characters to append

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and pszSrc should not be NULL. See RtlUnicodeStringCatCchStringNEx if
    you require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if all of pszSrc or the first cchToAppend characters were
                       concatenated to Dest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this status
            do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatCchStringN(
    __inout PUNICODE_STRING Dest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        if (cchToAppend > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            size_t cchCopied = 0;

            status = RtlUnicodeStringCopyStringNWorker(pszDest + cchDestLength,
                                                       cchDest - cchDestLength,
                                                       &cchCopied,
                                                       pszSrc,
                                                       cchToAppend,
                                                       0);

            Dest->Length = (USHORT)((cchDestLength + cchCopied) * sizeof(wchar_t));
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCatCbStringN(
    IN OUT  PUNICODE_STRING Dest,
    IN      LPCTSTR         pszSrc,
    IN      size_t          cbToAppend
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat' for
    PUNICODE_STRINGs.  This function will not write past the end of the
    destination buffer.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if all of pszSrc or the first cbToAppend bytes were appended
    to the destination string and it was null terminated, otherwise it will
    return a failure code. In failure cases as much of pszSrc will be appended
    to Dest as possible, and Dest will be null terminated if there is room.

Arguments:

    Dest            -   destination PUNICODE_STRING

    pszSrc          -   source string

    cbToAppend      -   maximum number of bytes to append

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and pszSrc should not be NULL. See RtlUnicodeStringCatCbStringNEx if
    you require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if all of pszSrc or the first cbToAppend bytes were
                       concatenated to pszDest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatCbStringN(
    __inout PUNICODE_STRING Dest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToAppend)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        size_t cchToAppend;

        // convert to count of characters
        cchToAppend = cbToAppend / sizeof(wchar_t);

        if (cchToAppend > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            size_t cchCopied = 0;

            status = RtlUnicodeStringCopyStringNWorker(pszDest + cchDestLength,
                                                       cchDest - cchDestLength,
                                                       &cchCopied,
                                                       pszSrc,
                                                       cchToAppend,
                                                       0);

            Dest->Length = (USHORT)((cchDestLength + cchCopied) * sizeof(wchar_t));
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCatCchUnicodeStringN(
    IN OUT  PUNICODE_STRING     Dest,
    IN      PCUNICODE_STRING    Src,
    IN      size_t              cchToAppend
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat' for
    PUNICODE_STRINGs.  This function will not write past the end of the
    destination buffer.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if all of pszSrc or the first cchToAppend characters were appended
    to the destination string and it was null terminated, otherwise it will
    return a failure code. In failure cases as much of pszSrc will be appended
    to Dest as possible, and Dest will be null terminated if there is room.

Arguments:

    Dest            -   destination PUNICODE_STRING

    Src             -   source PUNICODE_STRING

    cchToAppend     -   maximum number of characters to append

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL. See RtlUnicodeStringCatCchUnicodeStringNEx if
    you require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if all of Src or the first cchToAppend characters were
                       concatenated to Dest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatCchUnicodeStringN(
    __inout PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src,
    __in size_t cchToAppend)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        if (cchToAppend > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            wchar_t* pszSrc;
            size_t cchSrcLength;

            status = RtlUnicodeStringValidateSrcWorker(Src,
                                                       &pszSrc,
                                                       &cchSrcLength,
                                                       NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                       0);

            if (NT_SUCCESS(status))
            {
                size_t cchCopied = 0;

                if (cchSrcLength < cchToAppend)
                {
                    cchToAppend = cchSrcLength;
                }

                status = RtlUnicodeStringCopyWorker(pszDest + cchDestLength,
                                                    cchDest - cchDestLength,
                                                    &cchCopied,
                                                    pszSrc,
                                                    cchToAppend,
                                                    0);

                Dest->Length = (USHORT)((cchDestLength + cchCopied) * sizeof(wchar_t));
            }
        }
    }

    return status;
}
#endif // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCatCbUnicodeStringN(
    IN OUT  PUNICODE_STRING     Dest,
    IN      PCUNICODE_STRING    Src,
    IN      size_t              cbToAppend
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat' for
    PUNICODE_STRINGs.  This function will not write past the end of the
    destination buffer.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if all of pszSrc or the first cbToAppend bytes were appended
    to the destination string and it was null terminated, otherwise it will
    return a failure code. In failure cases as much of pszSrc will be appended
    to Dest as possible, and Dest will be null terminated if there is room.

Arguments:

    Dest            -   destination PUNICODE_STRING

    Src             -   source PUNICODE_STRING

    cbToAppend      -   maximum number of bytes to append

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL. See RtlUnicodeStringCatCbUnicodeStringNEx if
    you require the handling of NULL values.

Return Value:

    STATUS_SUCCESS -   if all of Src or the first cbToAppend bytes were
                       concatenated to pszDest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatCbUnicodeStringN(
    __inout PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src,
    __in size_t cbToAppend)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        size_t cchToAppend;

        // convert to count of characters
        cchToAppend = cbToAppend / sizeof(wchar_t);

        if (cchToAppend > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            wchar_t* pszSrc;
            size_t cchSrcLength;
            
            status = RtlUnicodeStringValidateSrcWorker(Src,
                                                       &pszSrc,
                                                       &cchSrcLength,
                                                       NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                       0);

            if (NT_SUCCESS(status))
            {
                size_t cchCopied = 0;

                if (cchSrcLength < cchToAppend)
                {
                    cchToAppend = cchSrcLength;
                }

                status = RtlUnicodeStringCopyWorker(pszDest + cchDestLength,
                                                    cchDest - cchDestLength,
                                                    &cchCopied,
                                                    pszSrc,
                                                    cchToAppend,
                                                    0);

                Dest->Length = (USHORT)((cchDestLength + cchCopied) * sizeof(wchar_t));
            }
        }
    }

    return status;
}
#endif // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCatCchStringNEx(
    IN OUT  PUNICODE_STRING Dest            OPTIONAL,
    IN      LPCTSTR         pszSrc          OPTIONAL,
    IN      size_t          cchToAppend,
    OUT     PUNICODE_STRING RemainingString OPTIONAL,
    IN      DWORD           dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat', with
    some additional parameters and for PUNICODE_STRINGs.  In addition to
    functionality provided by RtlUnicodeStringCatCchStringN, this routine also
    returns a pointer to the end of the destination string and the number of
    bytes left in the destination string including the null terminator in
    RemainingString . The flags parameter allows additional controls.

Arguments:

    Dest            -   destination PUNICODE_STRING

    pszSrc          -   source string

    cchToAppend     -   maximum number of characters to append

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character.   MaximumLength will contain
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, pszDest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if all of pszSrc or the first cchToAppend characters were
                       concatenated to Dest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatCchStringNEx(
    __inout PUNICODE_STRING Dest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszDestEnd = pszDest + cchDestLength;
        size_t cchRemaining = cchDest - cchDestLength;
        size_t cchNewDestLength = cchDestLength;

        if (cchToAppend > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            size_t cchCopied = 0;

            status = RtlUnicodeStringCopyStringNExWorker(pszDestEnd,
                                                         cchRemaining,
                                                         &cchCopied,
                                                         pszSrc,
                                                         cchToAppend,
                                                         &pszDestEnd,
                                                         &cchRemaining,
                                                         dwFlags);

            // handle the STRSAFE_NO_TRUNCATION flag
            if ((dwFlags & STRSAFE_NO_TRUNCATION)   &&
                !NT_SUCCESS(status)                 &&
                pszDest)
            {
                pszDestEnd = pszDest + cchDestLength;
                cchRemaining = cchDest - cchDestLength;

                cchNewDestLength = cchDestLength;

                // null terminate the original end of the string
                *pszDestEnd = L'\0';
            }
            else
            {
                cchNewDestLength = cchDestLength + cchCopied;
            }
        }

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE flags,
                // we already handled the STRSAFE_NO_TRUNCATION so do not pass it through
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags & (~STRSAFE_NO_TRUNCATION));
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCatCbStringNEx(
    IN OUT  PUNICODE_STRING Dest            OPTIONAL,
    IN      LPCTSTR         pszSrc          OPTIONAL,
    IN      size_t          cbToAppend,
    OUT     PUNICODE_STRING RemainingString OPTIONAL,
    IN      DWORD           dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat', with
    some additional parameters and for PUNICODE_STRINGs.  In addition to
    functionality provided by RtlUnicodeStringCatCbStringN, this routine also
    returns a pointer to the end of the destination string and the number of
    bytes left in the destination string including the null terminator in
    RemainingString . The flags parameter allows additional controls.

Arguments:

    Dest            -   destination PUNICODE_STRING

    pszSrc          -   source string

    cbToAppend      -   maximum number of bytes to append

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character.   MaximumLength will contain
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, pszDest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and pszSrc should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and pszSrc
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if all of pszSrc or the first cbToAppend bytes were
                       concatenated to pszDest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatCbStringNEx(
    __inout PUNICODE_STRING Dest,
    __in_bcount(cbToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cbToAppend,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszDestEnd = pszDest + cchDestLength;
        size_t cchRemaining = cchDest - cchDestLength;
        size_t cchNewDestLength = cchDestLength;
        size_t cchToAppend;

        // convert to count of characters
        cchToAppend = cbToAppend / sizeof(wchar_t);

        if (cchToAppend > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            size_t cchCopied = 0;

            status = RtlUnicodeStringCopyStringNExWorker(pszDestEnd,
                                                         cchRemaining,
                                                         &cchCopied,
                                                         pszSrc,
                                                         cchToAppend,
                                                         &pszDestEnd,
                                                         &cchRemaining,
                                                         dwFlags);

            // handle the STRSAFE_NO_TRUNCATION flag
            if ((dwFlags & STRSAFE_NO_TRUNCATION)   &&
                !NT_SUCCESS(status)                 &&
                pszDest)
            {
                pszDestEnd = pszDest + cchDestLength;
                cchRemaining = cchDest - cchDestLength;

                cchNewDestLength = cchDestLength;

                // null terminate the original end of the string
                *pszDestEnd = L'\0';
            }
            else
            {
                cchNewDestLength = cchDestLength + cchCopied;
            }
        }

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE flags,
                // we already handled the STRSAFE_NO_TRUNCATION so do not pass it through
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags & (~STRSAFE_NO_TRUNCATION));
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


#ifndef NTSTRSAFE_NO_CCH_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCatCchUnicodeStringNEx(
    IN OUT  PUNICODE_STRING     Dest            OPTIONAL,
    IN      PCUNICODE_STRING    Src             OPTIONAL,
    IN      size_t              cchToAppend,
    OUT     PUNICODE_STRING     RemainingString OPTIONAL,
    IN      DWORD               dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat', with
    some additional parameters and for PUNICODE_STRINGs.  In addition to
    functionality provided by RtlUnicodeStringCatCchUnicodeStringN, this routine also
    returns a pointer to the end of the destination string and the number of
    bytes left in the destination string including the null terminator in
    RemainingString . The flags parameter allows additional controls.

Arguments:

    Dest            -   destination PUNICODE_STRING

    Src             -   source const PUNICODE_STRING

    cchToAppend     -   maximum number of characters to append

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character.   MaximumLength will contain
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, Dest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and Src
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if all of Src or the first cchToAppend characters were
                       concatenated to Dest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatCchUnicodeStringNEx(
    __inout PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src,
    __in size_t cchToAppend,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszDestEnd = pszDest + cchDestLength;
        size_t cchRemaining = cchDest - cchDestLength;
        size_t cchNewDestLength = cchDestLength;
        
        if (cchToAppend > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            wchar_t* pszSrc;
            size_t cchSrcLength;

            status = RtlUnicodeStringValidateSrcWorker(Src,
                                                       &pszSrc,
                                                       &cchSrcLength,
                                                       NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                       dwFlags);

            if (NT_SUCCESS(status))
            {
                size_t cchCopied = 0;

                if (cchSrcLength < cchToAppend)
                {
                    cchToAppend = cchSrcLength;
                }

                status = RtlUnicodeStringCopyStringNExWorker(pszDestEnd,
                                                             cchRemaining,
                                                             &cchCopied,
                                                             pszSrc,
                                                             cchToAppend,
                                                             &pszDestEnd,
                                                             &cchRemaining,
                                                             dwFlags);

                // handle the STRSAFE_NO_TRUNCATION flag
                if ((dwFlags & STRSAFE_NO_TRUNCATION)   &&
                    !NT_SUCCESS(status)                 &&
                    pszDest)
                {
                    pszDestEnd = pszDest + cchDestLength;
                    cchRemaining = cchDest - cchDestLength;

                    cchNewDestLength = cchDestLength;

                    // null terminate the original end of the string
                    *pszDestEnd = L'\0';
                }
                else
                {
                    cchNewDestLength = cchDestLength + cchCopied;
                }
            }
        }

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE flags,
                // we already handled the STRSAFE_NO_TRUNCATION so do not pass it through
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags & (~STRSAFE_NO_TRUNCATION));
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CCH_FUNCTIONS


#ifndef NTSTRSAFE_NO_CB_FUNCTIONS
/*++

NTSTATUS
RtlUnicodeStringCatCbUnicodeStringNEx(
    IN OUT  PUNICODE_STRING     Dest            OPTIONAL,
    IN      PCUNICODE_STRING    Src             OPTIONAL,
    IN      size_t              cbToAppend,
    OUT     PUNICODE_STRING     RemainingString OPTIONAL,
    IN      DWORD               dwFlags
    );

Routine Description:

    This routine is a safer version of the C built-in function 'strncat', with
    some additional parameters and for PUNICODE_STRINGs.  In addition to
    functionality provided by RtlUnicodeStringCatCbStringN, this routine also
    returns a pointer to the end of the destination string and the number of
    bytes left in the destination string including the null terminator in
    RemainingString . The flags parameter allows additional controls.

Arguments:

    Dest            -   destination PUNICODE_STRING

    Src             -   source const PUNICODE_STRING

    cbToAppend      -   maximum number of bytes to append

    RemainingString -   if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function appended any data, the result will point to the
                        null termination character.   MaximumLength will contain
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any pre-existing
                    or truncated string

        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any pre-existing or
                    truncated string

        STRSAFE_NO_TRUNCATION
                    if the function returns STATUS_BUFFER_OVERFLOW, Dest
                    will not contain a truncated string, it will remain unchanged.

Notes:
    Behavior is undefined if source and destination strings overlap.

    Dest and Src should not be NULL unless the STRSAFE_IGNORE_NULLS flag
    is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and Src
    may be NULL.  An error may still be returned even though NULLS are ignored
    due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if all of Src or the first cbToAppend bytes were
                       concatenated to Dest and the resultant dest string
                       was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the operation
                       failed due to insufficient space. When this error
                       occurs, the destination buffer is modified to contain
                       a truncated version of the ideal result and is null
                       terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringCatCbUnicodeStringNEx(
    __inout PUNICODE_STRING Dest,
    __in PCUNICODE_STRING Src,
    __in size_t cbToAppend,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    size_t cchDestLength;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                &cchDestLength,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszDestEnd = pszDest + cchDestLength;
        size_t cchRemaining = cchDest - cchDestLength;
        size_t cchNewDestLength = cchDestLength;
        size_t cchToAppend;

        // convert to count of characters
        cchToAppend = cbToAppend / sizeof(wchar_t);

        if (cchToAppend > NTSTRSAFE_UNICODE_STRING_MAX_CCH)
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            wchar_t* pszSrc;
            size_t cchSrcLength;

            status = RtlUnicodeStringValidateSrcWorker(Src,
                                                       &pszSrc,
                                                       &cchSrcLength,
                                                       NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                       dwFlags);

            if (NT_SUCCESS(status))
            {
                size_t cchCopied = 0;

                if (cchSrcLength < cchToAppend)
                {
                    cchToAppend = cchSrcLength;
                }

                status = RtlUnicodeStringCopyStringNExWorker(pszDestEnd,
                                                             cchRemaining,
                                                             &cchCopied,
                                                             pszSrc,
                                                             cchToAppend,
                                                             &pszDestEnd,
                                                             &cchRemaining,
                                                             dwFlags);

                // handle the STRSAFE_NO_TRUNCATION flag
                if ((dwFlags & STRSAFE_NO_TRUNCATION)   &&
                    !NT_SUCCESS(status)                 &&
                    pszDest)
                {
                    pszDestEnd = pszDest + cchDestLength;
                    cchRemaining = cchDest - cchDestLength;

                    cchNewDestLength = cchDestLength;

                    // null terminate the original end of the string
                    *pszDestEnd = L'\0';
                }
                else
                {
                    cchNewDestLength = cchDestLength + cchCopied;
                }
            }
        }

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE flags,
                // we already handled the STRSAFE_NO_TRUNCATION so do not pass it through
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags & (~STRSAFE_NO_TRUNCATION));
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}
#endif  // !NTSTRSAFE_NO_CB_FUNCTIONS


/*++

NTSTATUS
RtlUnicodeStringVPrintf(
    OUT PUNICODE_STRING Dest,
    IN  LPCTSTR         pszFormat,
    IN  va_list         argList
    );

Routine Description:

    This routine is a safer version of the C built-in function 'vsprintf' for
    PUNICODE_STRINGs.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was printed without truncation,
    otherwise it will return a failure code. In failure cases it will return
    a truncated version of the ideal result.

Arguments:

    Dest        -  destination PUNICODE_STRING

    pszFormat   -  format string which must be null terminated

    argList     -  va_list from the variable arguments according to the
                   stdarg.h convention

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    Dest and pszFormat should not be NULL.  See RtlUnicodeStringVPrintfEx if you
    require the handling of NULL values.


Return Value:

    STATUS_SUCCESS -   if there was sufficient space in the dest buffer for
                       the resultant string and it was null terminated.

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringVPrintf(
    __out PUNICODE_STRING Dest, 
    __in __format_string NTSTRSAFE_PCWSTR pszFormat, 
    __in va_list argList)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        size_t cchNewDestLength = 0;

        status = RtlUnicodeStringVPrintfWorker(pszDest,
                                               cchDest,
                                               &cchNewDestLength,
                                               0,
                                               pszFormat,
                                               argList);

        Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringPrintf(
    OUT PUNICODE_STRING Dest,
    IN  LPCTSTR pszFormat,
    ...
    );

Routine Description:

    This routine is a safer version of the C built-in function 'sprintf' for
    PUNICODE_STRINGs.

    This function returns an NTSTATUS value, and not a pointer.  It returns
    STATUS_SUCCESS if the string was printed without truncation,
    otherwise it will return a failure code. In failure cases it will return
    a truncated version of the ideal result.

Arguments:

    Dest        -  destination PUNICODE_STRING

    pszFormat   -  format string which must be null terminated

    ...         -  additional parameters to be formatted according to
                   the format string

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    Dest and pszFormat should not be NULL.  See RtlUnicodeStringPrintfEx if you
    require the handling of NULL values.


Return Value:

    STATUS_SUCCESS -   if there was sufficient space in the dest buffer for
                       the resultant string and it was null terminated.

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringPrintf(
    __out PUNICODE_STRING Dest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                0);

    if (NT_SUCCESS(status))
    {
        va_list argList;
        size_t cchNewDestLength = 0;

        va_start(argList, pszFormat);

        status = RtlUnicodeStringVPrintfWorker(pszDest,
                                               cchDest,
                                               &cchNewDestLength,
                                               0,
                                               pszFormat,
                                               argList);

        va_end(argList);

        Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringVPrintfEx(
    OUT PUNICODE_STRING Dest            OPTIONAL,
    OUT PUNICODE_STRING RemainingString OPTIONAL,
    IN  DWORD   dwFlags,
    IN  LPCTSTR pszFormat               OPTIONAL,
    IN  va_list argList
    );

Routine Description:

    This routine is a safer version of the C built-in function 'vsprintf' with
    some additional parameters for PUNICODE_STRING.  In addition to functionality
    provided by RtlUnicodeStringVPrintf, this routine also returns a pointer to
    the end of the destination string and the number of characters left in the
    destination string including the null terminator in RemainingString.
     The flags parameter allows additional controls.

Arguments:

    Dest            -   destination PUNICODE_STRING

    RemainingString -   if RemainingString is non-null, the function will return
                        a pointer to the end of the destination string.  If the
                        function printed any data, the result will point to the
                        null termination character.  MaximumLength will contain
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

    pszFormat       -   format string which must be null terminated

    argList         -   va_list from the variable arguments according to the
                        stdarg.h convention

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    Dest and pszFormat should not be NULL unless the STRSAFE_IGNORE_NULLS
    flag is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and
    pszFormat may be NULL.  An error may still be returned even though NULLS
    are ignored due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated and
                       the resultant dest string was null terminated

    failure        -   the operation did not succeed

      STATUS_BUFFER_OVERFLOW
      Note: This status has the severity class Warning - IRPs completed with this
            status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringVPrintfEx(
    __out PUNICODE_STRING Dest, 
    __out_opt PUNICODE_STRING RemainingString, 
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat, 
    __in va_list argList)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;

    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        wchar_t* pszDestEnd = pszDest;
        size_t cchRemaining = cchDest;
        size_t cchNewDestLength = 0;

        status = RtlUnicodeStringVPrintfExWorker(pszDest,
                                                 cchDest,
                                                 &cchNewDestLength,
                                                 &pszDestEnd,
                                                 &cchRemaining,
                                                 dwFlags,
                                                 pszFormat,
                                                 argList);

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE, STRSAFE_NULL_ON_FAILURE, and STRSAFE_NO_TRUNCATION flags
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags);
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}


/*++

NTSTATUS
RtlUnicodeStringPrintfEx(
    OUT PUNICODE_STRING Dest            OPTIONAL,
    OUT PUNICODE_STRING RemainingString OPTIONAL,
    IN  DWORD           dwFlags,
    IN  LPCTSTR         pszFormat       OPTIONAL,
    ...
    );

Routine Description:

    This routine is a safer version of the C built-in function 'sprintf' with
    some additional parameters for PUNICODE_STRINGs.  In addition to
    functionality provided by RtlUnicodeStringPrintf, this routine also returns
    a pointer to the end of the destination string and the number of bytes left
    in the destination string including the null terminator in RemainingString.
    The flags parameter allows additional controls.

Arguments:

    Dest             -   destination PUNICODE_STRING

    RemainingString  -  if RemainingString is non-null, the function will return a
                        pointer to the end of the destination string.  If the
                        function printed any data, the result will point to the
                        null termination character.  MaximumLength will contain
                        the number of bytes left in the destination string,
                        including the null terminator

    dwFlags         -   controls some details of the string copy:

        STRSAFE_FILL_BEHIND_NULL
                    if the function succeeds, the low byte of dwFlags will be
                    used to fill the uninitialize part of destination buffer
                    behind the null terminator

        STRSAFE_IGNORE_NULLS
                    treat NULL string pointers like empty strings (TEXT(""))

        STRSAFE_FILL_ON_FAILURE
                    if the function fails, the low byte of dwFlags will be
                    used to fill all of the destination buffer, and it will
                    be null terminated. This will overwrite any truncated
                    string returned when the failure is
                    STATUS_BUFFER_OVERFLOW

        STRSAFE_NO_TRUNCATION /
        STRSAFE_NULL_ON_FAILURE
                    if the function fails, the destination buffer will be set
                    to the empty string. This will overwrite any truncated string
                    returned when the failure is STATUS_BUFFER_OVERFLOW.

    pszFormat       -   format string which must be null terminated

    ...             -   additional parameters to be formatted according to
                        the format string

Notes:
    Behavior is undefined if destination, format strings or any arguments
    strings overlap.

    Dest and pszFormat should not be NULL unless the STRSAFE_IGNORE_NULLS
    flag is specified.  If STRSAFE_IGNORE_NULLS is passed, both Dest and
    pszFormat may be NULL.  An error may still be returned even though NULLS
    are ignored due to insufficient space.

Return Value:

    STATUS_SUCCESS -   if there was source data and it was all concatenated and
                       the resultant dest string was null terminated

    failure        -   the operation did not succeed


      STATUS_BUFFER_OVERFLOW (STATUS_BUFFER_OVERFLOW/ERROR_INSUFFICIENT_BUFFER to user mode apps)
      Note: This status has the severity class Warning - IRPs completed with this status do have their data copied back to user mode
                   -   this return value is an indication that the print
                       operation failed due to insufficient space. When this
                       error occurs, the destination buffer is modified to
                       contain a truncated version of the ideal result and is
                       null terminated. This is useful for situations where
                       truncation is ok.

    It is strongly recommended to use the NT_SUCCESS() macro to test the
    return value of this function

--*/

NTSTRSAFE_INLINE_API
RtlUnicodeStringPrintfEx(
    __out PUNICODE_STRING Dest,
    __out_opt PUNICODE_STRING RemainingString,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    ...)
{
    NTSTATUS status;
    wchar_t* pszDest;
    size_t cchDest;
    
    status = RtlUnicodeStringValidateDestWorker(Dest,
                                                &pszDest,
                                                &cchDest,
                                                NULL,
                                                NTSTRSAFE_UNICODE_STRING_MAX_CCH,
                                                dwFlags);

    if (NT_SUCCESS(status))
    {
        va_list argList;
        wchar_t* pszDestEnd;
        size_t cchRemaining;
        size_t cchNewDestLength = 0;

        va_start(argList, pszFormat);

        status = RtlUnicodeStringVPrintfExWorker(pszDest,
                                                 cchDest,
                                                 &cchNewDestLength,
                                                 &pszDestEnd,
                                                 &cchRemaining,
                                                 dwFlags,
                                                 pszFormat,
                                                 argList);

        va_end(argList);

        if (pszDest)
        {
            if (!NT_SUCCESS(status))
            {
                // handle the STRSAFE_FILL_ON_FAILURE, STRSAFE_NULL_ON_FAILURE, and STRSAFE_NO_TRUNCATION flags
                RtlUnicodeStringExHandleFailureWorker(pszDest,
                                                      cchDest,
                                                      &cchNewDestLength,
                                                      &pszDestEnd,
                                                      &cchRemaining,
                                                      dwFlags);
            }

            Dest->Length = (USHORT)(cchNewDestLength * sizeof(wchar_t));
        }

        if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
        {   
            if (RemainingString)
            {
                RemainingString->Length = 0;
                RemainingString->MaximumLength = (USHORT)(cchRemaining * sizeof(wchar_t));
                RemainingString->Buffer = pszDestEnd;
            }
        }
    }

    return status;
}

#endif  // !NTSTRSAFE_LIB_IMPL
#endif  // !NTSTRSAFE_NO_UNICODE_STRING_FUNCTIONS

// Below here are the worker functions that actually do the work
#ifdef NTSTRSAFE_INLINE

NTSTRSAFEDDI
RtlStringCopyWorkerA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
        // can not null terminate a zero-byte dest buffer
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        while (cchDest && (*pszSrc != '\0'))
        {
            *pszDest++ = *pszSrc++;
            cchDest--;
        }

        if (cchDest == 0)
        {
            // we are going to truncate pszDest
            pszDest--;
            status = STATUS_BUFFER_OVERFLOW;
        }

        *pszDest= '\0';
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCopyWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
        // can not null terminate a zero-byte dest buffer
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        while (cchDest && (*pszSrc != L'\0'))
        {
            *pszDest++ = *pszSrc++;
            cchDest--;
        }

        if (cchDest == 0)
        {
            // we are going to truncate pszDest
            pszDest--;
            status = STATUS_BUFFER_OVERFLOW;
        }

        *pszDest= L'\0';
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCopyExWorkerA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;

    // ASSERT(cbDest == (cchDest * sizeof(char))    ||
    //        cbDest == (cchDest * sizeof(char)) + (cbDest % sizeof(char)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                // only fail if there was actually src data to copy
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && (*pszSrc != '\0'))
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(char)) + (cbDest % sizeof(char)));
                    }
                }
                else
                {
                    // we are going to truncate pszDest
                    pszDestEnd--;
                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = '\0';
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCopyExWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PWSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;

    // ASSERT(cbDest == (cchDest * sizeof(wchar_t)) ||
    //        cbDest == (cchDest * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                // only fail if there was actually src data to copy
                if (*pszSrc != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && (*pszSrc != L'\0'))
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));
                    }
                }
                else
                {
                    // we are going to truncate pszDest
                    pszDestEnd--;
                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = L'\0';
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCopyNWorkerA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchSrc) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchSrc)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
        // can not null terminate a zero-byte dest buffer
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        while (cchDest && cchSrc && (*pszSrc != '\0'))
        {
            *pszDest++ = *pszSrc++;
            cchDest--;
            cchSrc--;
        }

        if (cchDest == 0)
        {
            // we are going to truncate pszDest
            pszDest--;
            status = STATUS_BUFFER_OVERFLOW;
        }

        *pszDest= '\0';
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCopyNWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
        // can not null terminate a zero-byte dest buffer
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        while (cchDest && cchToCopy && (*pszSrc != L'\0'))
        {
            *pszDest++ = *pszSrc++;
            cchDest--;
            cchToCopy--;
        }

        if (cchDest == 0)
        {
            // we are going to truncate pszDest
            pszDest--;
            status = STATUS_BUFFER_OVERFLOW;
        }

        *pszDest= L'\0';
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCopyNExWorkerA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToCopy,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;

    // ASSERT(cbDest == (cchDest * sizeof(char))    ||
    //        cbDest == (cchDest * sizeof(char)) + (cbDest % sizeof(char)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else if (cchToCopy > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                // only fail if there was actually src data to copy
                if ((cchToCopy != 0) && (*pszSrc != '\0'))
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && cchToCopy && (*pszSrc != '\0'))
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;
                    cchToCopy--;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(char)) + (cbDest % sizeof(char)));
                    }
                }
                else
                {
                    // we are going to truncate pszDest
                    pszDestEnd--;
                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = '\0';
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCopyNExWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PWSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;

    // ASSERT(cbDest == (cchDest * sizeof(wchar_t)) ||
    //        cbDest == (cchDest * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else if (cchToCopy > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                // only fail if there was actually src data to copy
                if ((cchToCopy != 0) && (*pszSrc != L'\0'))
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && cchToCopy && (*pszSrc != L'\0'))
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;
                    cchToCopy--;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));
                    }
                }
                else
                {
                    // we are going to truncate pszDest
                    pszDestEnd--;
                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = L'\0';
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCatWorkerA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCSTR pszSrc)
{
   NTSTATUS status;
   size_t cchDestLength;

   status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestLength);

   if (NT_SUCCESS(status))
   {
       status = RtlStringCopyWorkerA(pszDest + cchDestLength,
                              cchDest - cchDestLength,
                              pszSrc);
   }

   return status;
}

NTSTRSAFEDDI
RtlStringCatWorkerW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in NTSTRSAFE_PCWSTR pszSrc)
{
   NTSTATUS status;
   size_t cchDestLength;

   status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestLength);

   if (NT_SUCCESS(status))
   {
       status = RtlStringCopyWorkerW(pszDest + cchDestLength,
                              cchDest - cchDestLength,
                              pszSrc);
   }

   return status;
}

NTSTRSAFEDDI
RtlStringCatExWorkerA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;

    // ASSERT(cbDest == (cchDest * sizeof(char))    ||
    //        cbDest == (cchDest * sizeof(char)) + (cbDest % sizeof(char)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cchDestLength;

        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest == 0) && (cbDest == 0))
                {
                    cchDestLength = 0;
                }
                else
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestLength);

                if (NT_SUCCESS(status))
                {
                    pszDestEnd = pszDest + cchDestLength;
                    cchRemaining = cchDest - cchDestLength;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }
        else
        {
            status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestLength);

            if (NT_SUCCESS(status))
            {
                pszDestEnd = pszDest + cchDestLength;
                cchRemaining = cchDest - cchDestLength;
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                // only fail if there was actually src data to append
                if (*pszSrc != '\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                // we handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE cases below, so do not pass
                // those flags through
                status = RtlStringCopyExWorkerA(pszDestEnd,
                                         cchRemaining,
                                         (cchRemaining * sizeof(char)) + (cbDest % sizeof(char)),
                                         pszSrc,
                                         &pszDestEnd,
                                         &cchRemaining,
                                         dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            // STRSAFE_NO_TRUNCATION is taken care of by RtlStringCopyExWorkerA()

            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & STRSAFE_NULL_ON_FAILURE)
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCatExWorkerW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PWSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;

    // ASSERT(cbDest == (cchDest * sizeof(wchar_t)) ||
    //        cbDest == (cchDest * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        size_t cchDestLength;

        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest == 0) && (cbDest == 0))
                {
                    cchDestLength = 0;
                }
                else
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestLength);

                if (NT_SUCCESS(status))
                {
                    pszDestEnd = pszDest + cchDestLength;
                    cchRemaining = cchDest - cchDestLength;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }
        else
        {
            status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestLength);

            if (NT_SUCCESS(status))
            {
                pszDestEnd = pszDest + cchDestLength;
                cchRemaining = cchDest - cchDestLength;
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                // only fail if there was actually src data to append
                if (*pszSrc != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                // we handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE cases below, so do not pass
                // those flags through
                status = RtlStringCopyExWorkerW(pszDestEnd,
                                         cchRemaining,
                                         (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)),
                                         pszSrc,
                                         &pszDestEnd,
                                         &cchRemaining,
                                         dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            // STRSAFE_NO_TRUNCATION is taken care of by RtlStringCopyExWorkerW()

            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & STRSAFE_NULL_ON_FAILURE)
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCatNWorkerA(
    __inout_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToAppend)
{
    NTSTATUS status;
    size_t cchDestLength;

    status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestLength);

    if (NT_SUCCESS(status))
    {
        status = RtlStringCopyNWorkerA(pszDest + cchDestLength,
                                cchDest - cchDestLength,
                                pszSrc,
                                cchToAppend);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCatNWorkerW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend)
{
    NTSTATUS status;
    size_t cchDestLength;

    status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestLength);

    if (NT_SUCCESS(status))
    {
        status = RtlStringCopyNWorkerW(pszDest + cchDestLength,
                                cchDest - cchDestLength,
                                pszSrc,
                                cchToAppend);
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCatNExWorkerA(
    __inout_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCSTR pszSrc,
    __in size_t cchToAppend,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchDestLength = 0;

    // ASSERT(cbDest == (cchDest * sizeof(char))    ||
    //        cbDest == (cchDest * sizeof(char)) + (cbDest % sizeof(char)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else if (cchToAppend > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest == 0) && (cbDest == 0))
                {
                    cchDestLength = 0;
                }
                else
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestLength);

                if (NT_SUCCESS(status))
                {
                    pszDestEnd = pszDest + cchDestLength;
                    cchRemaining = cchDest - cchDestLength;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = "";
            }
        }
        else
        {
            status = RtlStringLengthWorkerA(pszDest, cchDest, &cchDestLength);

            if (NT_SUCCESS(status))
            {
                pszDestEnd = pszDest + cchDestLength;
                cchRemaining = cchDest - cchDestLength;
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                // only fail if there was actually src data to append
                if ((cchToAppend != 0) && (*pszSrc != '\0'))
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                // we handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE cases below, so do not pass
                // those flags through
                status = RtlStringCopyNExWorkerA(pszDestEnd,
                                          cchRemaining,
                                          (cchRemaining * sizeof(char)) + (cbDest % sizeof(char)),
                                          pszSrc,
                                          cchToAppend,
                                          &pszDestEnd,
                                          &cchRemaining,
                                          dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            // STRSAFE_NO_TRUNCATION is taken care of by RtlStringCopyNExWorkerA()

            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringCatNExWorkerW(
    __inout_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __in_ecount(cchToAppend) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToAppend,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PWSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchDestLength = 0;


    // ASSERT(cbDest == (cchDest * sizeof(wchar_t)) ||
    //        cbDest == (cchDest * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else if (cchToAppend > NTSTRSAFE_MAX_CCH)
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest == 0) && (cbDest == 0))
                {
                    cchDestLength = 0;
                }
                else
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestLength);

                if (NT_SUCCESS(status))
                {
                    pszDestEnd = pszDest + cchDestLength;
                    cchRemaining = cchDest - cchDestLength;
                }
            }

            if (pszSrc == NULL)
            {
                pszSrc = L"";
            }
        }
        else
        {
            status = RtlStringLengthWorkerW(pszDest, cchDest, &cchDestLength);

            if (NT_SUCCESS(status))
            {
                pszDestEnd = pszDest + cchDestLength;
                cchRemaining = cchDest - cchDestLength;
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                // only fail if there was actually src data to append
                if ((cchToAppend != 0) && (*pszSrc != L'\0'))
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                // we handle the STRSAFE_FILL_ON_FAILURE and STRSAFE_NULL_ON_FAILURE cases below, so do not pass
                // those flags through
                status = RtlStringCopyNExWorkerW(pszDestEnd,
                                          cchRemaining,
                                          (cchRemaining * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)),
                                          pszSrc,
                                          cchToAppend,
                                          &pszDestEnd,
                                          &cchRemaining,
                                          dwFlags & (~(STRSAFE_FILL_ON_FAILURE | STRSAFE_NULL_ON_FAILURE)));
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            // STRSAFE_NO_TRUNCATION is taken care of by RtlStringCopyNExWorkerW()

            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringVPrintfWorkerA(
    __out_ecount(cchDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
        // can not null terminate a zero-byte dest buffer
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        int iRet;
        size_t cchMax;

        // leave the last space for the null terminator
        cchMax = cchDest - 1;

#if (_STRSAFE_USE_SECURE_CRT == 1) && !defined(NTSTRSAFE_LIB_IMPL)
        iRet = _vsnprintf_s(pszDest, cchDest, cchMax, pszFormat, argList);
#else
        iRet = _vsnprintf(pszDest, cchMax, pszFormat, argList);
#endif
        // ASSERT((iRet < 0) || (((size_t)iRet) <= cchMax));

        if ((iRet < 0) || (((size_t)iRet) > cchMax))
        {
            // need to null terminate the string
            pszDest += cchMax;
            *pszDest = '\0';

            // we have truncated pszDest
            status = STATUS_BUFFER_OVERFLOW;
        }
        else if (((size_t)iRet) == cchMax)
        {
            // need to null terminate the string
            pszDest += cchMax;
            *pszDest = '\0';
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringVPrintfWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (cchDest == 0)
    {
        // can not null terminate a zero-byte dest buffer
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        int iRet;
        size_t cchMax;

        // leave the last space for the null terminator
        cchMax = cchDest - 1;

#if (_STRSAFE_USE_SECURE_CRT == 1) && !defined(NTSTRSAFE_LIB_IMPL)
        iRet = _vsnwprintf_s(pszDest, cchDest, cchMax, pszFormat, argList);
#else
        iRet = _vsnwprintf(pszDest, cchMax, pszFormat, argList);
#endif
        // ASSERT((iRet < 0) || (((size_t)iRet) <= cchMax));

        if ((iRet < 0) || (((size_t)iRet) > cchMax))
        {
            // need to null terminate the string
            pszDest += cchMax;
            *pszDest = L'\0';

            // we have truncated pszDest
            status = STATUS_BUFFER_OVERFLOW;
        }
        else if (((size_t)iRet) == cchMax)
        {
            // need to null terminate the string
            pszDest += cchMax;
            *pszDest = L'\0';
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringVPrintfExWorkerA(
    __out_bcount(cbDest) NTSTRSAFE_PSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;

    // ASSERT(cbDest == (cchDest * sizeof(char))    ||
    //        cbDest == (cchDest * sizeof(char)) + (cbDest % sizeof(char)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszFormat == NULL)
            {
                pszFormat = "";
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                // only fail if there was actually a non-empty format string
                if (*pszFormat != '\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                int iRet;
                size_t cchMax;

                // leave the last space for the null terminator
                cchMax = cchDest - 1;

#if (_STRSAFE_USE_SECURE_CRT == 1) && !defined(NTSTRSAFE_LIB_IMPL)
                iRet = _vsnprintf_s(pszDest, cchDest, cchMax, pszFormat, argList);
#else
                iRet = _vsnprintf(pszDest, cchMax, pszFormat, argList);
#endif
                // ASSERT((iRet < 0) || (((size_t)iRet) <= cchMax));

                if ((iRet < 0) || (((size_t)iRet) > cchMax))
                {
                    // we have truncated pszDest
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                    // need to null terminate the string
                    *pszDestEnd = '\0';

                    status = STATUS_BUFFER_OVERFLOW;
                }
                else if (((size_t)iRet) == cchMax)
                {
                    // string fit perfectly
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                    // need to null terminate the string
                    *pszDestEnd = '\0';
                }
                else if (((size_t)iRet) < cchMax)
                {
                    // there is extra room
                    pszDestEnd = pszDest + iRet;
                    cchRemaining = cchDest - iRet;

                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(char)) + (cbDest % sizeof(char)));
                    }
                }
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = '\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = '\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringVPrintfExWorkerW(
    __out_ecount(cchDest) NTSTRSAFE_PWSTR pszDest,
    __in size_t cchDest,
    __in size_t cbDest,
    __deref_opt_out_ecount(*pcchRemaining) NTSTRSAFE_PWSTR* ppszDestEnd,
    __out_opt size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;
    NTSTRSAFE_PWSTR pszDestEnd = pszDest;
    size_t cchRemaining = 0;

    // ASSERT(cbDest == (cchDest * sizeof(wchar_t)) ||
    //        cbDest == (cchDest * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));

    // only accept valid flags
    if (dwFlags & (~STRSAFE_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (dwFlags & STRSAFE_IGNORE_NULLS)
        {
            if (pszDest == NULL)
            {
                if ((cchDest != 0) || (cbDest != 0))
                {
                    // NULL pszDest and non-zero cchDest/cbDest is invalid
                    status = STATUS_INVALID_PARAMETER;
                }
            }

            if (pszFormat == NULL)
            {
                pszFormat = L"";
            }
        }

        if (NT_SUCCESS(status))
        {
            if (cchDest == 0)
            {
                pszDestEnd = pszDest;
                cchRemaining = 0;

                // only fail if there was actually a non-empty format string
                if (*pszFormat != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                int iRet;
                size_t cchMax;

                // leave the last space for the null terminator
                cchMax = cchDest - 1;

#if (_STRSAFE_USE_SECURE_CRT == 1) && !defined(NTSTRSAFE_LIB_IMPL)
                iRet = _vsnwprintf_s(pszDest, cchDest, cchMax, pszFormat, argList);
#else
                iRet = _vsnwprintf(pszDest, cchMax, pszFormat, argList);
#endif
                // ASSERT((iRet < 0) || (((size_t)iRet) <= cchMax));

                if ((iRet < 0) || (((size_t)iRet) > cchMax))
                {
                    // we have truncated pszDest
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                    // need to null terminate the string
                    *pszDestEnd = L'\0';

                    status = STATUS_BUFFER_OVERFLOW;
                }
                else if (((size_t)iRet) == cchMax)
                {
                    // string fit perfectly
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                    // need to null terminate the string
                    *pszDestEnd = L'\0';
                }
                else if (((size_t)iRet) < cchMax)
                {
                    // there is extra room
                    pszDestEnd = pszDest + iRet;
                    cchRemaining = cchDest - iRet;

                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), ((cchRemaining - 1) * sizeof(wchar_t)) + (cbDest % sizeof(wchar_t)));
                    }
                }
            }
        }
    }

    if (!NT_SUCCESS(status))
    {
        if (pszDest)
        {
            if (dwFlags & STRSAFE_FILL_ON_FAILURE)
            {
                memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cbDest);

                if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;
                }
                else if (cchDest > 0)
                {
                    pszDestEnd = pszDest + cchDest - 1;
                    cchRemaining = 1;

                    // null terminate the end of the string
                    *pszDestEnd = L'\0';
                }
            }

            if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
            {
                if (cchDest > 0)
                {
                    pszDestEnd = pszDest;
                    cchRemaining = cchDest;

                    // null terminate the beginning of the string
                    *pszDestEnd = L'\0';
                }
            }
        }
    }

    if (NT_SUCCESS(status) || (status == STATUS_BUFFER_OVERFLOW))
    {
        if (ppszDestEnd)
        {
            *ppszDestEnd = pszDestEnd;
        }

        if (pcchRemaining)
        {
            *pcchRemaining = cchRemaining;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringLengthWorkerA(
    __in NTSTRSAFE_PCSTR psz,
    __in size_t cchMax,
    __out_opt size_t* pcchLength)
{
    NTSTATUS status = STATUS_SUCCESS;
    size_t cchMaxPrev = cchMax;

    while (cchMax && (*psz != '\0'))
    {
        psz++;
        cchMax--;
    }

    if (cchMax == 0)
    {
        // the string is longer than cchMax
        status = STATUS_INVALID_PARAMETER;
    }

    if (pcchLength)
    {
        if (NT_SUCCESS(status))
        {
            *pcchLength = cchMaxPrev - cchMax;
        }
        else
        {
            *pcchLength = 0;
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlStringLengthWorkerW(
    __in NTSTRSAFE_PCWSTR psz,
    __in size_t cchMax,
    __out_opt size_t* pcchLength)
{
    NTSTATUS status = STATUS_SUCCESS;
    size_t cchMaxPrev = cchMax;

    while (cchMax && (*psz != L'\0'))
    {
        psz++;
        cchMax--;
    }

    if (cchMax == 0)
    {
        // the string is longer than cchMax
        status = STATUS_INVALID_PARAMETER;
    }

    if (pcchLength)
    {
        if (NT_SUCCESS(status))
        {
            *pcchLength = cchMaxPrev - cchMax;
        }
        else
        {
            *pcchLength = 0;
        }
    }

    return status;
}

#ifndef NTSTRSAFE_NO_UNICODE_STRING_FUNCTIONS

NTSTRSAFEDDI
RtlUnicodeStringLengthHelper(
    __in NTSTRSAFE_PCWSTR psz,
    __in size_t cchMax,
    __out size_t* pcchLength,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    size_t cchMaxPrev = cchMax;

    if ((dwFlags & STRSAFE_IGNORE_NULLS) && (psz == NULL))
    {
        psz = L"";
    }

    while (cchMax && (*psz != L'\0'))
    {
        psz++;
        cchMax--;
    }

    if (cchMax == 0)
    {
        if ((dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED) ||
            (*psz != L'\0'))
        {
            // the string is longer than cchMax
            status = STATUS_INVALID_PARAMETER;
        }
    }

    if (NT_SUCCESS(status))
    {
        *pcchLength = cchMaxPrev - cchMax;
    }
    else
    {
        *pcchLength = 0;
    }

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringInitWorker(
    __out PUNICODE_STRING Dest,
    __in_opt NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchMax,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (Dest || !(dwFlags & (STRSAFE_IGNORE_NULL_UNICODE_STRINGS | STRSAFE_IGNORE_NULLS)))
    {
        Dest->Length = 0;
        Dest->MaximumLength = 0;
        Dest->Buffer = NULL;
    }

    if (dwFlags & (~STRSAFE_UNICODE_STRING_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (pszSrc)
        {
            size_t cchSrcLength;

            status = RtlUnicodeStringLengthHelper(pszSrc,
                                                  cchMax,
                                                  &cchSrcLength,
                                                  dwFlags);

            if (NT_SUCCESS(status))
            {
                if (Dest)
                {
                    size_t cbLength = cchSrcLength * sizeof(wchar_t);

                    Dest->Length = (USHORT)cbLength;
                    Dest->MaximumLength = (USHORT)(cbLength + sizeof(wchar_t));
                    Dest->Buffer = (PWSTR)pszSrc;
                }
                else
                {
                    status = STATUS_INVALID_PARAMETER;
                }
            }
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringValidateWorker(
    __in_opt PCUNICODE_STRING Src,
    __in size_t cchMax,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;

    if (Src || !(dwFlags & (STRSAFE_IGNORE_NULL_UNICODE_STRINGS | STRSAFE_IGNORE_NULLS)))
    {
        if (((Src->Length % sizeof(wchar_t)) != 0)          ||
            ((Src->MaximumLength % sizeof(wchar_t)) != 0)   ||
            (Src->Length > Src->MaximumLength)              ||
            (Src->MaximumLength > cchMax))
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else if ((Src->Buffer == NULL)  &&
                 ((Src->Length != 0) || (Src->MaximumLength != 0)))
        {
            status = STATUS_INVALID_PARAMETER;
        }
        else if (dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)
        {
            if (Src->Buffer != NULL)
            {
                if ((Src->MaximumLength == 0)           ||
                    (Src->Length >= Src->MaximumLength) ||
                    (Src->Buffer[Src->Length] != L'\0'))
                {
                    status = STATUS_INVALID_PARAMETER;
                }
            }
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringValidateSrcWorker(
    __in PCUNICODE_STRING Src,
    __deref_out_ecount(*pcchSrcLength) wchar_t** ppszSrc,
    __out size_t* pcchSrcLength,
    __in size_t cchMax,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    *ppszSrc = NULL;
    *pcchSrcLength = 0;

    status = RtlUnicodeStringValidateWorker(Src, cchMax, (dwFlags & (~STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)));

    if (NT_SUCCESS(status))
    {
        if (Src)
        {
            *ppszSrc = Src->Buffer;
            *pcchSrcLength = Src->Length / sizeof(wchar_t);
        }

        if ((*ppszSrc == NULL) && (dwFlags & STRSAFE_IGNORE_NULLS))
        {
            *ppszSrc = L"";
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringValidateDestWorker(
    __in PCUNICODE_STRING Dest,
    __deref_out_ecount(*pcchDest) wchar_t** ppszDest,
    __out size_t* pcchDest,
    __out_opt size_t* pcchDestLength,
    __in size_t cchMax,
    __in unsigned long dwFlags)
{
    NTSTATUS status;

    *ppszDest = NULL;
    *pcchDest = 0;

    if (pcchDestLength)
    {
        *pcchDestLength = 0;
    }

    status = RtlUnicodeStringValidateWorker(Dest, cchMax, (dwFlags & (~STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)));

    if (NT_SUCCESS(status) && Dest)
    {
        *ppszDest = Dest->Buffer;
        *pcchDest = Dest->MaximumLength / sizeof(wchar_t);

        if (pcchDestLength)
        {
            *pcchDestLength = Dest->Length / sizeof(wchar_t);
        }
    }

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringCopyStringWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    size_t cchNewDestLength = 0;

    if (dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)
    {
        if (cchDest == 0)
        {
            cchNewDestLength = 0;

            // can not null terminate a zero-byte dest buffer
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            while (cchDest && (*pszSrc != L'\0'))
            {
                *pszDest++ = *pszSrc++;
                cchDest--;

                cchNewDestLength++;
            }

            if (cchDest == 0)
            {
                // we are going to truncate pszDest
                pszDest--;
                cchNewDestLength--;

                status = STATUS_BUFFER_OVERFLOW;
            }

            *pszDest= L'\0';
        }
    }
    else
    {
        while (cchDest && (*pszSrc != L'\0'))
        {
            *pszDest++ = *pszSrc++;
            cchDest--;

            cchNewDestLength++;
        }
        
        if (*pszSrc != L'\0')
        {
            // we are going to truncate pszDest
            status = STATUS_BUFFER_OVERFLOW;
        }

        // always null terminate when possible
        if (cchDest)
        {
            *pszDest = L'\0';
        }
    }

    *pcchNewDestLength = cchNewDestLength;

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringCopyWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in_ecount(cchSrcLength) const wchar_t* pszSrc,
    __in size_t cchSrcLength,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    size_t cchNewDestLength = 0;

    if (dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)
    {
        if (cchDest == 0)
        {
            cchNewDestLength = 0;

            // can not null terminate a zero-byte dest buffer
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            while (cchDest && cchSrcLength)
            {
                *pszDest++ = *pszSrc++;
                cchDest--;
                cchSrcLength--;

                cchNewDestLength++;
            }

            if (cchDest == 0)
            {
                // we are going to truncate pszDest
                pszDest--;
                cchNewDestLength--;

                status = STATUS_BUFFER_OVERFLOW;
            }

            *pszDest= L'\0';
        }
    }
    else
    {
        while (cchDest && cchSrcLength)
        {
            *pszDest++ = *pszSrc++;
            cchDest--;
            cchSrcLength--;

            cchNewDestLength++;
        }
        
        if (cchSrcLength)
        {
            // we are going to truncate pszDest
            status = STATUS_BUFFER_OVERFLOW;
        }

        // always null terminate when possible
        if (cchDest)
        {
            *pszDest = L'\0';
        }
    }

    *pcchNewDestLength = cchNewDestLength;

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringExHandleFailureWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    if (dwFlags & STRSAFE_FILL_ON_FAILURE)
    {
        memset(pszDest, STRSAFE_GET_FILL_PATTERN(dwFlags), cchDest * sizeof(wchar_t));

        if (STRSAFE_GET_FILL_PATTERN(dwFlags) == 0)
        {
            *ppszDestEnd = pszDest;
            *pcchRemaining = cchDest;

            *pcchNewDestLength = 0;
        }
        else if (cchDest > 0)
        {
            wchar_t* pszDestEnd;
            
            pszDestEnd = pszDest + cchDest - 1;

            *ppszDestEnd = pszDestEnd;
            *pcchRemaining = 1;

            *pcchNewDestLength = cchDest - 1;

            // null terminate the end of the string
            *pszDestEnd = L'\0';
        }
    }

    if (dwFlags & (STRSAFE_NULL_ON_FAILURE | STRSAFE_NO_TRUNCATION))
    {
        if (cchDest > 0)
        {
            *ppszDestEnd = pszDest;
            *pcchRemaining = cchDest;

            *pcchNewDestLength = 0;

            // null terminate the beginning of the string
            *pszDest = L'\0';
        }
    }

    return STATUS_SUCCESS;
}

NTSTRSAFEDDI
RtlUnicodeStringCopyStringExWorker(  
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in NTSTRSAFE_PCWSTR pszSrc,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchNewDestLength = 0;

    if (dwFlags & (~STRSAFE_UNICODE_STRING_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if ((dwFlags & STRSAFE_IGNORE_NULLS) && (pszSrc == NULL))
        {
            pszSrc = L"";
        }

        if (cchDest == 0)
        {
            // only fail if there was actually src data to copy
            if (*pszSrc != L'\0')
            {
                if (pszDest == NULL)
                {
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    status = STATUS_BUFFER_OVERFLOW;
                }
            }
        }
        else
        {
            if (dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && (*pszSrc != L'\0'))
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;

                    cchNewDestLength++;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), (cchRemaining - 1) * sizeof(wchar_t));
                    }
                }
                else
                {
                    // we are going to truncate pszDest
                    pszDestEnd--;
                    cchNewDestLength--;

                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = L'\0';
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && (*pszSrc != L'\0'))
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;

                    cchNewDestLength++;
                }
            
                if (cchRemaining > 0)
                {
                    // always null terminate when possible
                    *pszDestEnd = L'\0';

                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), (cchRemaining - 1) * sizeof(wchar_t));
                    }
                }
                else if (*pszSrc != L'\0')
                {
                    // we are going to truncate pszDest
                    status = STATUS_BUFFER_OVERFLOW;
                }
            }
        }
    }

    *ppszDestEnd = pszDestEnd;
    *pcchRemaining = cchRemaining;

    *pcchNewDestLength = cchNewDestLength;

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringCopyExWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in_ecount(cchSrcLength) const wchar_t* pszSrc,
    __in size_t cchSrcLength,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchNewDestLength = 0;

    if (dwFlags & (~STRSAFE_UNICODE_STRING_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if (cchDest == 0)
        {
            // only fail if there was actually src data to copy
            if (cchSrcLength)
            {
                if (pszDest == NULL)
                {
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    status = STATUS_BUFFER_OVERFLOW;
                }
            }
        }
        else
        {
            if (dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && cchSrcLength)
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;
                    cchSrcLength--;

                    cchNewDestLength++;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), (cchRemaining - 1) * sizeof(wchar_t));
                    }
                }
                else
                {
                    // we are going to truncate pszDest
                    pszDestEnd--;
                    cchNewDestLength--;

                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = L'\0';
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && cchSrcLength)
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;
                    cchSrcLength--;

                    cchNewDestLength++;
                }
            
                if (cchRemaining > 0)
                {
                    // always null terminate when possible
                    *pszDestEnd = L'\0';

                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), (cchRemaining - 1) * sizeof(wchar_t));
                    }
                }
                else if (cchSrcLength != 0)
                {
                    // we are going to truncate pszDest
                    status = STATUS_BUFFER_OVERFLOW;
                }
            }
        }
    }

    *ppszDestEnd = pszDestEnd;
    *pcchRemaining = cchRemaining;

    *pcchNewDestLength = cchNewDestLength;

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringCopyStringNWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    size_t cchNewDestLength = 0;

    if (dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)
    {
        if (cchDest == 0)
        {
            cchNewDestLength = 0;

            // can not null terminate a zero-byte dest buffer
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            while (cchDest && cchToCopy && (*pszSrc != L'\0'))
            {
                *pszDest++ = *pszSrc++;
                cchDest--;
                cchToCopy--;

                cchNewDestLength++;
            }

            if (cchDest == 0)
            {
                // we are going to truncate pszDest
                pszDest--;
                cchNewDestLength--;

                status = STATUS_BUFFER_OVERFLOW;
            }

            *pszDest= L'\0';
        }
    }
    else
    {
        while (cchDest && cchToCopy && (*pszSrc != L'\0'))
        {
            *pszDest++ = *pszSrc++;
            cchDest--;
            cchToCopy--;

            cchNewDestLength++;
        }

        if (cchDest == 0)
        {
            if ((cchToCopy != 0) && (*pszSrc != L'\0'))
            {
                // we are going to truncate pszDest
                status = STATUS_BUFFER_OVERFLOW;
            }
        }
        else
        {
            // always null terminate when possible
            *pszDest = L'\0';
        }
    }

    *pcchNewDestLength = cchNewDestLength;

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringCopyStringNExWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in_ecount(cchToCopy) NTSTRSAFE_PCWSTR pszSrc,
    __in size_t cchToCopy,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags)
{
    NTSTATUS status = STATUS_SUCCESS;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchNewDestLength = 0;

    if (dwFlags & (~STRSAFE_UNICODE_STRING_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        if ((dwFlags & STRSAFE_IGNORE_NULLS) && (pszSrc == NULL))
        {
            pszSrc = L"";
        }

        if (cchDest == 0)
        {
            // only fail if there was actually src data to copy
            if ((cchToCopy != 0) && (*pszSrc != L'\0'))
            {
                if (pszDest == NULL)
                {
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    status = STATUS_BUFFER_OVERFLOW;
                }
            }
        }
        else
        {
            if (dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && cchToCopy && (*pszSrc != L'\0'))
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;
                    cchToCopy--;

                    cchNewDestLength++;
                }

                if (cchRemaining > 0)
                {
                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), (cchRemaining - 1) * sizeof(wchar_t));
                    }
                }
                else
                {
                    // we are going to truncate pszDest
                    pszDestEnd--;
                    cchNewDestLength--;

                    cchRemaining++;

                    status = STATUS_BUFFER_OVERFLOW;
                }

                *pszDestEnd = L'\0';
            }
            else
            {
                pszDestEnd = pszDest;
                cchRemaining = cchDest;

                while (cchRemaining && cchToCopy && (*pszSrc != L'\0'))
                {
                    *pszDestEnd++ = *pszSrc++;
                    cchRemaining--;
                    cchToCopy--;

                    cchNewDestLength++;
                }
            
                if (cchRemaining > 0)
                {
                    // always null terminate when possible
                    *pszDestEnd = L'\0';

                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), (cchRemaining - 1) * sizeof(wchar_t));
                    }
                }
                else if ((cchToCopy != 0) && (*pszSrc != L'\0'))
                {
                    // we are going to truncate pszDest
                    status = STATUS_BUFFER_OVERFLOW;
                }
            }
        }
    }

    *ppszDestEnd = pszDestEnd;
    *pcchRemaining = cchRemaining;

    *pcchNewDestLength = cchNewDestLength;

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringVPrintfWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;
    int iRet;
    size_t cchMax;
    size_t cchNewDestLength = 0;
    
    if (dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)
    {
        if (cchDest == 0)
        {
            // can not null terminate a zero-byte dest buffer
            status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            // leave the last space for the null terminator
            cchMax = cchDest - 1;

#if (_STRSAFE_USE_SECURE_CRT == 1) && !defined(NTSTRSAFE_LIB_IMPL)
            iRet = _vsnwprintf_s(pszDest, cchDest, cchMax, pszFormat, argList);
#else
            iRet = _vsnwprintf(pszDest, cchMax, pszFormat, argList);
#endif
            // ASSERT((iRet < 0) || (((size_t)iRet) <= cchMax));

            if ((iRet < 0) || (((size_t)iRet) > cchMax))
            {
                // need to null terminate the string
                pszDest += cchMax;
                *pszDest = L'\0';

                cchNewDestLength = cchMax;

                // we have truncated pszDest
                status = STATUS_BUFFER_OVERFLOW;
            }
            else
            {
                cchNewDestLength = (size_t)iRet;

                if (cchNewDestLength == cchMax)
                {
                    // need to null terminate the string
                    pszDest += cchMax;
                    *pszDest = L'\0';
                }
            }
        }
    }
    else
    {
        if (cchDest == 0)
        {
            // only fail if there was actually a non-empty format string
            if (*pszFormat != L'\0')
            {
                if (pszDest == NULL)
                {
                    status = STATUS_INVALID_PARAMETER;
                }
                else
                {
                    status = STATUS_BUFFER_OVERFLOW;
                }
            }
        }
        else
        {
            cchMax = cchDest;

            iRet = _vsnwprintf(pszDest, cchMax, pszFormat, argList);
            // ASSERT((iRet < 0) || (((size_t)iRet) <= cchMax));

            if ((iRet < 0) || (((size_t)iRet) > cchMax))
            {
                cchNewDestLength = cchMax;

                // we have truncated pszDest
                status = STATUS_BUFFER_OVERFLOW;
            }
            else
            {
                cchNewDestLength = (size_t)iRet;
            }
        }
    }

    *pcchNewDestLength = cchNewDestLength;

    return status;
}

NTSTRSAFEDDI
RtlUnicodeStringVPrintfExWorker(
    __out_ecount(cchDest) wchar_t* pszDest,
    __in size_t cchDest,
    __out size_t* pcchNewDestLength,
    __deref_out_ecount(*pcchRemaining) wchar_t** ppszDestEnd,
    __out size_t* pcchRemaining,
    __in unsigned long dwFlags,
    __in __format_string NTSTRSAFE_PCWSTR pszFormat,
    __in va_list argList)
{
    NTSTATUS status = STATUS_SUCCESS;
    wchar_t* pszDestEnd = pszDest;
    size_t cchRemaining = 0;
    size_t cchNewDestLength = 0;

    if (dwFlags & (~STRSAFE_UNICODE_STRING_VALID_FLAGS))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        int iRet;
        size_t cchMax;

        if ((dwFlags & STRSAFE_IGNORE_NULLS) && (pszFormat == NULL))
        {
            pszFormat = L"";
        }

        if (dwFlags & STRSAFE_UNICODE_STRING_DEST_NULL_TERMINATED)
        {
            if (cchDest == 0)
            {
                // can not null terminate a zero-byte dest buffer
                status = STATUS_INVALID_PARAMETER;
            }
            else
            {
                // leave the last space for the null terminator
                cchMax = cchDest - 1;

#if (_STRSAFE_USE_SECURE_CRT == 1) && !defined(NTSTRSAFE_LIB_IMPL)
                iRet = _vsnwprintf_s(pszDest, cchDest, cchMax, pszFormat, argList);
#else
                iRet = _vsnwprintf(pszDest, cchMax, pszFormat, argList);
#endif
                // ASSERT((iRet < 0) || (((size_t)iRet) <= cchMax));

                if ((iRet < 0) || (((size_t)iRet) > cchMax))
                {
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                    // need to null terminate the string
                    *pszDestEnd = L'\0';

                    cchNewDestLength = cchMax;

                    // we have truncated pszDest
                    status = STATUS_BUFFER_OVERFLOW;
                }
                else if (((size_t)iRet) == cchMax)
                {
                    // string fit perfectly
                    pszDestEnd = pszDest + cchMax;
                    cchRemaining = 1;

                    // need to null terminate the string
                    *pszDestEnd = L'\0';

                    cchNewDestLength = cchMax;
                }
                else if (((size_t)iRet) < cchMax)
                {
                    // there is extra room
                    cchNewDestLength = (size_t)iRet;
                    
                    pszDestEnd = pszDest + cchNewDestLength;
                    cchRemaining = cchDest - cchNewDestLength;

                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), (cchRemaining - 1) * sizeof(wchar_t));
                    }
                }
            }
        }
        else
        {
            if (cchDest == 0)
            {
                // only fail if there was actually a non-empty format string
                if (*pszFormat != L'\0')
                {
                    if (pszDest == NULL)
                    {
                        status = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        status = STATUS_BUFFER_OVERFLOW;
                    }
                }
            }
            else
            {
                cchMax = cchDest;

                iRet = _vsnwprintf(pszDest, cchMax, pszFormat, argList);
                // ASSERT((iRet < 0) || (((size_t)iRet) <= cchMax));

                if ((iRet < 0) || (((size_t)iRet) > cchMax))
                {
                    if (cchMax > 0)
                    {
                        pszDestEnd = pszDest + cchMax - 1;
                    }

                    cchRemaining = 0;

                    cchNewDestLength = cchMax;

                    // we have truncated pszDest
                    status = STATUS_BUFFER_OVERFLOW;
                }
                else if (((size_t)iRet) == cchMax)
                {
                    // string fit perfectly
                    if (cchMax > 0)
                    {
                        pszDestEnd = pszDest + cchMax - 1;
                    }

                    cchRemaining = 0;

                    cchNewDestLength = cchMax;
                }
                else if (((size_t)iRet) < cchMax)
                {
                    // there is extra room
                    cchNewDestLength = (size_t)iRet;
                    
                    pszDestEnd = pszDest + cchNewDestLength;
                    cchRemaining = cchDest - cchNewDestLength;

                    if (dwFlags & STRSAFE_FILL_BEHIND_NULL)
                    {
                        memset(pszDestEnd + 1, STRSAFE_GET_FILL_PATTERN(dwFlags), (cchRemaining - 1) * sizeof(wchar_t));
                    }
                }
            }
        }
    }

    *ppszDestEnd = pszDestEnd;
    *pcchRemaining = cchRemaining;

    *pcchNewDestLength = cchNewDestLength;

    return status;
}

#endif  // !NTSTRSAFE_NO_UNICODE_STRING_FUNCTIONS
#endif  // NTSTRSAFE_INLINE



// Do not call these functions, they are worker functions for internal use within this file
#ifdef DEPRECATE_SUPPORTED
#pragma deprecated(RtlStringCopyWorkerA)
#pragma deprecated(RtlStringCopyWorkerW)
#pragma deprecated(RtlStringCopyExWorkerA)
#pragma deprecated(RtlStringCopyExWorkerW)
#pragma deprecated(RtlStringCatWorkerA)
#pragma deprecated(RtlStringCatWorkerW)
#pragma deprecated(RtlStringCatExWorkerA)
#pragma deprecated(RtlStringCatExWorkerW)
#pragma deprecated(RtlStringCatNWorkerA)
#pragma deprecated(RtlStringCatNWorkerW)
#pragma deprecated(RtlStringCatNExWorkerA)
#pragma deprecated(RtlStringCatNExWorkerW)
#pragma deprecated(RtlStringVPrintfWorkerA)
#pragma deprecated(RtlStringVPrintfWorkerW)
#pragma deprecated(RtlStringVPrintfExWorkerA)
#pragma deprecated(RtlStringVPrintfExWorkerW)
#pragma deprecated(RtlStringLengthWorkerA)
#pragma deprecated(RtlStringLengthWorkerW)
#pragma deprecated(RtlStringGetsExWorkerA)
#pragma deprecated(RtlStringGetsExWorkerW)
#pragma deprecated(RtlUnicodeStringLengthHelper)
#pragma deprecated(RtlUnicodeStringInitWorker)
#pragma deprecated(RtlUnicodeStringValidateWorker)
#pragma deprecated(RtlUnicodeStringValidateSrcWorker)
#pragma deprecated(RtlUnicodeStringValidateDestWorker)
#pragma deprecated(RtlUnicodeStringCopyStringWorker)
#pragma deprecated(RtlUnicodeStringCopyWorker)
#pragma deprecated(RtlUnicodeStringExHandleFailureWorker)
#pragma deprecated(RtlUnicodeStringCopyStringExWorker)
#pragma deprecated(RtlUnicodeStringCopyExWorker)
#pragma deprecated(RtlUnicodeStringCopyStringNWorker)
#pragma deprecated(RtlUnicodeStringCopyStringNExWorker)
#pragma deprecated(RtlUnicodeStringVPrintfWorker)
#pragma deprecated(RtlUnicodeStringVPrintfExWorker)
#else
#define RtlStringCopyWorkerA       RtlStringCopyWorkerA_instead_use_RtlStringCchCopyA_or_RtlStringCchCopyExA;
#define RtlStringCopyWorkerW       RtlStringCopyWorkerW_instead_use_RtlStringCchCopyW_or_RtlStringCchCopyExW;
#define RtlStringCopyExWorkerA     RtlStringCopyExWorkerA_instead_use_RtlStringCchCopyA_or_RtlStringCchCopyExA;
#define RtlStringCopyExWorkerW     RtlStringCopyExWorkerW_instead_use_RtlStringCchCopyW_or_RtlStringCchCopyExW;
#define RtlStringCatWorkerA        RtlStringCatWorkerA_instead_use_RtlStringCchCatA_or_RtlStringCchCatExA;
#define RtlStringCatWorkerW        RtlStringCatWorkerW_instead_use_RtlStringCchCatW_or_RtlStringCchCatExW;
#define RtlStringCatExWorkerA      RtlStringCatExWorkerA_instead_use_RtlStringCchCatA_or_RtlStringCchCatExA;
#define RtlStringCatExWorkerW      RtlStringCatExWorkerW_instead_use_RtlStringCchCatW_or_RtlStringCchCatExW;
#define RtlStringCatNWorkerA       RtlStringCatNWorkerA_instead_use_RtlStringCchCatNA_or_StrincCbCatNA;
#define RtlStringCatNWorkerW       RtlStringCatNWorkerW_instead_use_RtlStringCchCatNW_or_RtlStringCbCatNW;
#define RtlStringCatNExWorkerA     RtlStringCatNExWorkerA_instead_use_RtlStringCchCatNExA_or_RtlStringCbCatNExA;
#define RtlStringCatNExWorkerW     RtlStringCatNExWorkerW_instead_use_RtlStringCchCatNExW_or_RtlStringCbCatNExW;
#define RtlStringVPrintfWorkerA    RtlStringVPrintfWorkerA_instead_use_RtlStringCchVPrintfA_or_RtlStringCchVPrintfExA;
#define RtlStringVPrintfWorkerW    RtlStringVPrintfWorkerW_instead_use_RtlStringCchVPrintfW_or_RtlStringCchVPrintfExW;
#define RtlStringVPrintfExWorkerA  RtlStringVPrintfExWorkerA_instead_use_RtlStringCchVPrintfA_or_RtlStringCchVPrintfExA;
#define RtlStringVPrintfExWorkerW  RtlStringVPrintfExWorkerW_instead_use_RtlStringCchVPrintfW_or_RtlStringCchVPrintfExW;
#define RtlStringLengthWorkerA     RtlStringLengthWorkerA_instead_use_RtlStringCchLengthA_or_RtlStringCbLengthA;
#define RtlStringLengthWorkerW     RtlStringLengthWorkerW_instead_use_RtlStringCchLengthW_or_RtlStringCbLengthW;
#define RtlStringGetsExWorkerA     RtlStringGetsExWorkerA_instead_use_RtlStringCchGetsA_or_RtlStringCbGetsA
#define RtlStringGetsExWorkerW     RtlStringGetsExWorkerW_instead_use_RtlStringCchGetsW_or_RtlStringCbGetsW
#define RtlUnicodeStringLengthHelper            RtlUnicodeStringLengthHelper_do_not_call_this_function
#define RtlUnicodeStringInitWorker              RtlUnicodeStringInitWorker_instead_use_RtlUnicodeStringInit_or_RtlUnicodeStringInitEx
#define RtlUnicodeStringValidateWorker          RtlUnicodeStringValidateWorker_instead_use_RtlUnicodeStringValidate_or_RtlUnicodeStringValidateEx
#define RtlUnicodeStringValidateSrcWorker       RtlUnicodeStringValidateSrcWorker_do_not_call_this_function
#define RtlUnicodeStringValidateDestWorker      RtlUnicodeStringValidateDestWorker_do_not_call_this_function
#define RtlUnicodeStringCopyStringWorker        RtlUnicodeStringCopyStringWorker_instead_use_RtlUnicodeStringCopyString
#define RtlUnicodeStringCopyWorker              RtlUnicodeStringCopyWorker_instead_use_RtlUnicodeStringCopyString
#define RtlUnicodeStringExHandleFailureWorker   RtlUnicodeStringExHandleFailureWorker_do_not_call_this_function
#define RtlUnicodeStringCopyStringExWorker      RtlUnicodeStringCopyStringExWorker_instead_use_RtlUnicodeStringCopyStringEx
#define RtlUnicodeStringCopyExWorker            RtlUnicodeStringCopyExWorker_instead_use_RtlUnicodeStringCopyStringEx
#define RtlUnicodeStringCopyStringNWorker       RtlUnicodeStringCopyStringNWorker_instead_use_RtlUnicodeStringCopyCchStringN_orRtlUnicodeStringCopyCbStringN
#define RtlUnicodeStringCopyStringNExWorker     RtlUnicodeStringCopyStringNExWorker_instead_use_RtlUnicodeStringCopyCchStringNEx_or_RtlUnicodeStringCopyCbStringNEx
#define RtlUnicodeStringVPrintfWorker           RtlUnicodeStringVPrintfWorker_instead_use_RtlUnicodeStringVPrintf
#define RtlUnicodeStringVPrintfExWorker         RtlUnicodeStringVPrintfExWorker_instead_use_RtlUnicodeStringVPrintfEx
#endif // !DEPRECATE_SUPPORTED


#ifndef NTSTRSAFE_NO_DEPRECATE
// Deprecate all of the unsafe functions to generate compiletime errors. If you do not want
// this then you can #define NTSTRSAFE_NO_DEPRECATE before including this file.
#ifdef DEPRECATE_SUPPORTED

#pragma deprecated(strcpy)
#pragma deprecated(wcscpy)
#pragma deprecated(strcat)
#pragma deprecated(wcscat)
#pragma deprecated(sprintf)
#pragma deprecated(swprintf)
#pragma deprecated(vsprintf)
#pragma deprecated(vswprintf)
#pragma deprecated(_snprintf)
#pragma deprecated(_snwprintf)
#pragma deprecated(_vsnprintf)
#pragma deprecated(_vsnwprintf)

#else // DEPRECATE_SUPPORTED

#undef strcpy
#define strcpy      strcpy_instead_use_RtlStringCbCopyA_or_RtlStringCchCopyA;

#undef wcscpy
#define wcscpy      wcscpy_instead_use_RtlStringCbCopyW_or_RtlStringCchCopyW;

#undef strcat
#define strcat      strcat_instead_use_RtlStringCbCatA_or_RtlStringCchCatA;

#undef wcscat
#define wcscat      wcscat_instead_use_RtlStringCbCatW_or_RtlStringCchCatW;

#undef sprintf
#define sprintf     sprintf_instead_use_RtlStringCbPrintfA_or_RtlStringCchPrintfA;

#undef swprintf
#define swprintf    swprintf_instead_use_RtlStringCbPrintfW_or_RtlStringCchPrintfW;

#undef vsprintf
#define vsprintf    vsprintf_instead_use_RtlStringCbVPrintfA_or_RtlStringCchVPrintfA;

#undef vswprintf
#define vswprintf   vswprintf_instead_use_RtlStringCbVPrintfW_or_RtlStringCchVPrintfW;

#undef _snprintf
#define _snprintf   _snprintf_instead_use_RtlStringCbPrintfA_or_RtlStringCchPrintfA;

#undef _snwprintf
#define _snwprintf  _snwprintf_instead_use_RtlStringCbPrintfW_or_RtlStringCchPrintfW;

#undef _vsnprintf
#define _vsnprintf  _vsnprintf_instead_use_RtlStringCbVPrintfA_or_RtlStringCchVPrintfA;

#undef _vsnwprintf
#define _vsnwprintf _vsnwprintf_instead_use_RtlStringCbVPrintfW_or_RtlStringCchVPrintfW;

#endif  // !DEPRECATE_SUPPORTED
#endif  // !NTSTRSAFE_NO_DEPRECATE

#ifdef _STRSAFE_H_INCLUDED_
#pragma warning(pop)
#endif // _STRSAFE_H_INCLUDED_

#endif  // _NTSTRSAFE_H_INCLUDED_

