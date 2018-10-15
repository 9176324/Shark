/*++

Copyright (c) Microsoft Corporation. All rights reserved.

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    ntrtlpath.h

Abstract:

    Broken out from nturtl and rtl so I can move it between them in separate
    trees without merge madness. To be integrated into ntrtl.h

--*/

#ifndef _NTRTL_PATH_
#define _NTRTL_PATH_

#if _MSC_VER >= 1100
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4514)
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#endif
#endif

//
// not NTSYSAPI so easily statically linked to
//

//
// These are OUT Disposition values.
//
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_AMBIGUOUS   (0x00000001)
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_UNC         (0x00000002)
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_DRIVE       (0x00000003)
#define RTL_NT_PATH_NAME_TO_DOS_PATH_NAME_ALREADY_DOS (0x00000004)

NTSYSAPI
NTSTATUS
NTAPI
RtlNtPathNameToDosPathName(
    __in ULONG Flags,
    __inout PRTL_UNICODE_STRING_BUFFER Path,
    __out_opt PULONG Disposition,
    __inout_opt PWSTR* FilePart
    );

//
// If either Path or Element end in a path separator, then so does the result.
// The path separator to put on the end is chosen from the existing ones.
//
#define RTL_APPEND_PATH_ELEMENT_ONLY_BACKSLASH_IS_SEPARATOR (0x00000001)
#define RTL_APPEND_PATH_ELEMENT_BUGFIX_CHECK_FIRST_THREE_CHARS_FOR_SLASH_TAKE_FOUND_SLASH_INSTEAD_OF_FIRST_CHAR (0x00000002)
NTSTATUS
NTAPI
RtlAppendPathElement(
    IN     ULONG                      Flags,
    IN OUT PRTL_UNICODE_STRING_BUFFER Path,
    PCUNICODE_STRING                  Element
    );

//
// c:\foo => c:\
// \foo => empty
// \ => empty
// Trailing slashes are generally preserved.
//

NTSTATUS
NTAPI
RtlGetLengthWithoutLastFullDosPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );

NTSTATUS
NTAPI
RtlGetLengthWithoutLastNtPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetLengthWithoutLastFullDosOrNtPathElement(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );

NTSTATUS
NTAPI
RtlGetLengthWithoutTrailingPathSeparators(
    IN  ULONG            Flags,
    IN  PCUNICODE_STRING Path,
    OUT ULONG*           LengthOut
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlpApplyLengthFunction(
    IN ULONG     Flags,
    IN SIZE_T    SizeOfStruct,
    IN OUT PVOID UnicodeStringOrUnicodeStringBuffer,
    NTSTATUS (NTAPI* LengthFunction)(ULONG, PCUNICODE_STRING, ULONG*)
    );

#define RtlRemoveLastNtPathElement(Flags, Path) \
    (RtlpApplyLengthFunction((Flags), sizeof(*Path), (Path), RtlGetLengthWithoutLastNtPathElement))

#define RtlRemoveLastFullDosPathElement(Flags, Path) \
    (RtlpApplyLengthFunction((Flags), sizeof(*Path), (Path), RtlGetLengthWithoutLastFullDosPathElement))

#define RtlRemoveLastFullDosOrNtPathElement(Flags, Path) \
    (RtlpApplyLengthFunction((Flags), sizeof(*Path), (Path), RtlGetLengthWithoutLastFullDosOrNtPathElement))

#define RtlRemoveTrailingPathSeparators(Flags, Path) \
    (RtlpApplyLengthFunction((Flags), sizeof(*Path), (Path), RtlGetLengthWithoutTrailingPathSeparators))


//
// Such small pieces of data are not worth exporting.
// If you want any of this data in char form, just take the first char from the string.
//
#if !(defined(SORTPP_PASS) || defined(MIDL_PASS)) // Neither the Wow64 thunk generation tools nor midl accept this.
#if defined(RTL_CONSTANT_STRING) // Some code gets here without this defined.
#if defined(_MSC_VER) && (_MSC_VER >= 1100) // VC5 for __declspec(selectany)
__declspec(selectany) extern const UNICODE_STRING RtlNtPathSeparatorString = RTL_CONSTANT_STRING(L"\\");
__declspec(selectany) extern const UNICODE_STRING RtlDosPathSeparatorsString = RTL_CONSTANT_STRING(L"\\/");
__declspec(selectany) extern const UNICODE_STRING RtlAlternateDosPathSeparatorString = RTL_CONSTANT_STRING(L"/");
#else
static const UNICODE_STRING RtlNtPathSeparatorString = RTL_CONSTANT_STRING(L"\\");
static const UNICODE_STRING RtlDosPathSeparatorsString = RTL_CONSTANT_STRING(L"\\/");
static const UNICODE_STRING RtlAlternateDosPathSeparatorString = RTL_CONSTANT_STRING(L"/");
#endif // defined(_MSC_VER) && (_MSC_VER >= 1100)
#else
#if defined(_MSC_VER) && (_MSC_VER >= 1100) // VC5 for __declspec(selectany)
__declspec(selectany) extern const UNICODE_STRING RtlNtPathSeparatorString = { 1 * sizeof(WCHAR), 2 * sizeof(WCHAR), L"\\" };
__declspec(selectany) extern const UNICODE_STRING RtlDosPathSeparatorsString = { 2 * sizeof(WCHAR), 3 * sizeof(WCHAR), L"\\/" };
__declspec(selectany) extern const UNICODE_STRING RtlAlternateDosPathSeparatorString = { 1 * sizeof(WCHAR), 2 * sizeof(WCHAR), L"/" };
#else
static const UNICODE_STRING RtlNtPathSeparatorString = { 1 * sizeof(WCHAR), 2 * sizeof(WCHAR), L"\\" };
static const UNICODE_STRING RtlDosPathSeparatorsString = { 2 * sizeof(WCHAR), 3 * sizeof(WCHAR), L"\\/" };
static const UNICODE_STRING RtlAlternateDosPathSeparatorString = { 1 * sizeof(WCHAR), 2 * sizeof(WCHAR), L"/" };
#endif // defined(_MSC_VER) && (_MSC_VER >= 1100)
#endif // defined(RTL_CONSTANT_STRING)
#define RtlCanonicalDosPathSeparatorString RtlNtPathSeparatorString
#define RtlPathSeparatorString             RtlNtPathSeparatorString
#endif

//++
//
// WCHAR
// RTL_IS_DOS_PATH_SEPARATOR(
//     IN WCHAR Ch
//     );
//
// Routine Description:
//
// Arguments:
//
//     Ch - 
//
// Return Value:
//
//     TRUE  if ch is \\ or /.
//     FALSE otherwise.
//--
#define RTL_IS_DOS_PATH_SEPARATOR(ch_) ((ch_) == '\\' || ((ch_) == '/'))

#if defined(__cplusplus) && defined(_MSC_VER) && _MSC_VER >= 1100 // VC5 for bool
inline bool RtlIsDosPathSeparator(WCHAR ch) { return RTL_IS_DOS_PATH_SEPARATOR(ch); }
#endif

//
// compatibility
//
#define RTL_IS_PATH_SEPARATOR RTL_IS_DOS_PATH_SEPARATOR
#define RtlIsPathSeparator    RtlIsDosPathSeparator

//++
//
// WCHAR
// RTL_IS_NT_PATH_SEPARATOR(
//     IN WCHAR Ch
//     );
// 
// Routine Description:
//
// Arguments:
//
//     Ch - 
//
// Return Value:
//
//     TRUE  if ch is \\.
//     FALSE otherwise.
//--
#define RTL_IS_NT_PATH_SEPARATOR(ch_) ((ch_) == '\\')

#if defined(__cplusplus) && defined(_MSC_VER) && _MSC_VER >= 1100 // VC5 for bool
inline bool RtlIsNtPathSeparator(WCHAR ch) { return RTL_IS_NT_PATH_SEPARATOR(ch); }
#endif

#ifdef __cplusplus
}       // extern "C"
#endif

#if defined (_MSC_VER) && ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4001)
#pragma warning(default:4201)
#pragma warning(default:4214)
#endif
#endif
#endif

