/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    cnvint.c

Abstract:

    Text to integer and integer to text conversion routines.

--*/

#include <ntrtlp.h>

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,RtlIntegerToChar)
#pragma alloc_text(PAGE,RtlCharToInteger)
#pragma alloc_text(PAGE,RtlUnicodeStringToInteger)
#pragma alloc_text(PAGE,RtlIntegerToUnicode)
#pragma alloc_text(PAGE,RtlIntegerToUnicodeString)
#pragma alloc_text(PAGE,RtlLargeIntegerToChar)
#pragma alloc_text(PAGE,RtlLargeIntegerToUnicode)
#pragma alloc_text(PAGE,RtlInt64ToUnicodeString)
#endif

#if defined(ALLOC_DATA_PRAGMA)
#pragma const_seg("PAGECONST")
#endif
const CHAR RtlpIntegerChars[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

const WCHAR RtlpIntegerWChars[] = { L'0', L'1', L'2', L'3', L'4', L'5',
                              L'6', L'7', L'8', L'9', L'A', L'B',
                              L'C', L'D', L'E', L'F' };

NTSTATUS
RtlIntegerToChar (
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN LONG OutputLength,
    OUT PSZ String
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    CHAR Result[33], *s;
    ULONG Shift, Mask, Digit, Length;

    RTL_PAGED_CODE();

    Shift = 0;
    switch( Base ) {
        case 16:    Shift = 4;  break;
        case  8:    Shift = 3;  break;
        case  2:    Shift = 1;  break;

        case  0:    Base = 10;
        case 10:    Shift = 0;  break;
        default:    return( STATUS_INVALID_PARAMETER );
        }

    if (Shift != 0) {
        Mask = (1 << Shift) - 1;
    }

    s = &Result[33];
    do {
        if (Shift != 0) {
            Digit = Value & Mask;
            Value >>= Shift;
        } else {
            Digit = Value % Base;
            Value = Value / Base;
        }

        *--s = RtlpIntegerChars[Digit];
    } while (Value != 0);

    Length = (ULONG) (&Result[33] - s);
    if (OutputLength < 0) {
        OutputLength = -OutputLength;
        while ((LONG)Length < OutputLength) {
            *String++ = '0';
            OutputLength--;
        }
    }

    if ((LONG)Length > OutputLength) {
        return (STATUS_BUFFER_OVERFLOW);
    } else {
        try {
            RtlCopyMemory (String, s, Length);

            if ((LONG)Length < OutputLength) {
                String[Length] = '\0';
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return (GetExceptionCode ());
        }

        return (STATUS_SUCCESS);
    }
}


NTSTATUS
RtlCharToInteger (
    IN PCSZ String,
    IN ULONG Base OPTIONAL,
    OUT PULONG Value
    )
{
    CHAR c, Sign;
    ULONG Result, Digit, Shift;

    RTL_PAGED_CODE();

    while ((Sign = *String++) <= ' ') {
        if (!*String) {
            String--;
            break;
        }
    }

    c = Sign;
    if (c == '-' || c == '+') {
        c = *String++;
    }

    if (!ARGUMENT_PRESENT( Base )) {
        Base = 10;
        Shift = 0;
        if (c == '0') {
            c = *String++;
            if (c == 'x') {
                Base = 16;
                Shift = 4;
            } else if (c == 'o') {
                Base = 8;
                Shift = 3;
            } else if (c == 'b') {
                Base = 2;
                Shift = 1;
            } else {
                String--;
            }

            c = *String++;
        }
    } else {
        switch( Base ) {
            case 16:    Shift = 4;  break;
            case  8:    Shift = 3;  break;
            case  2:    Shift = 1;  break;
            case 10:    Shift = 0;  break;
            default:    return( STATUS_INVALID_PARAMETER );
        }
    }

    Result = 0;
    while (c) {
        if (c >= '0' && c <= '9') {
            Digit = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            Digit = c - 'A' + 10;
        } else if (c >= 'a' && c <= 'f') {
            Digit = c - 'a' + 10;
        } else {
            break;
        }

        if (Digit >= Base) {
            break;
        }

        if (Shift == 0) {
            Result = (Base * Result) + Digit;
        } else {
            Result = (Result << Shift) | Digit;
        }

        c = *String++;
    }

    if (Sign == '-') {
        Result = (ULONG)(-(LONG)Result);
    }

    try {
        *Value = Result;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return( GetExceptionCode() );
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
RtlUnicodeStringToInteger (
    IN PCUNICODE_STRING String,
    IN ULONG Base OPTIONAL,
    OUT PULONG Value
    )
{
    PCWSTR s;
    WCHAR c, Sign;
    ULONG nChars, Result, Digit, Shift;

    RTL_PAGED_CODE();

    s = String->Buffer;
    nChars = String->Length / sizeof( WCHAR );
    while (nChars-- && (Sign = *s++) <= ' ') {
        if (!nChars) {
            Sign = UNICODE_NULL;
            break;
            }
        }

    c = Sign;
    if (c == L'-' || c == L'+') {
        if (nChars) {
            nChars--;
            c = *s++;
            }
        else {
            c = UNICODE_NULL;
            }
        }

    if (!ARGUMENT_PRESENT( Base )) {
        Base = 10;
        Shift = 0;
        if (c == L'0') {
            if (nChars) {
                nChars--;
                c = *s++;
                if (c == L'x') {
                    Base = 16;
                    Shift = 4;
                    }
                else
                if (c == L'o') {
                    Base = 8;
                    Shift = 3;
                    }
                else
                if (c == L'b') {
                    Base = 2;
                    Shift = 1;
                    }
                else {
                    nChars++;
                    s--;
                    }
                }

            if (nChars) {
                nChars--;
                c = *s++;
                }
            else {
                c = UNICODE_NULL;
                }
            }
        }
    else {
        switch( Base ) {
            case 16:    Shift = 4;  break;
            case  8:    Shift = 3;  break;
            case  2:    Shift = 1;  break;
            case 10:    Shift = 0;  break;
            default:    return( STATUS_INVALID_PARAMETER );
            }
        }

    Result = 0;
    while (c != UNICODE_NULL) {
        if (c >= L'0' && c <= L'9') {
            Digit = c - L'0';
            }
        else
        if (c >= L'A' && c <= L'F') {
            Digit = c - L'A' + 10;
            }
        else
        if (c >= L'a' && c <= L'f') {
            Digit = c - L'a' + 10;
            }
        else {
            break;
            }

        if (Digit >= Base) {
            break;
            }

        if (Shift == 0) {
            Result = (Base * Result) + Digit;
            }
        else {
            Result = (Result << Shift) | Digit;
            }

        if (!nChars) {
            break;
            }
        nChars--;
        c = *s++;
        }

    if (Sign == L'-') {
        Result = (ULONG)(-(LONG)Result);
        }

    try {
        *Value = Result;
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        return( GetExceptionCode() );
        }

    return( STATUS_SUCCESS );
}


NTSTATUS
RtlIntegerToUnicode (
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN LONG OutputLength,
    OUT PWSTR String
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    WCHAR Result[33], *s;
    ULONG Shift, Mask, Digit, Length;

    RTL_PAGED_CODE();

    Shift = 0;
    switch( Base ) {
        case 16:    Shift = 4;  break;
        case  8:    Shift = 3;  break;
        case  2:    Shift = 1;  break;

        case  0:    Base = 10;
        case 10:    Shift = 0;  break;
        default:    return (STATUS_INVALID_PARAMETER);
    }

    if (Shift != 0) {
        Mask = (1 << Shift) - 1;
    }

    s = &Result[33];
    do {
        if (Shift != 0) {
            Digit = Value & Mask;
            Value >>= Shift;
        } else {
            Digit = Value % Base;
            Value = Value / Base;
        }

        *--s = RtlpIntegerWChars[ Digit ];
    } while (Value != 0);

    Length = (ULONG) (&Result[33] - s);
    if (OutputLength < 0) {
        OutputLength = -OutputLength;
        while ((LONG)Length < OutputLength) {
            *String++ = L'0';
            OutputLength--;
        }
    }

    if ((LONG)Length > OutputLength) {
        return (STATUS_BUFFER_OVERFLOW);
    } else {
        try {
            RtlCopyMemory (String, s, Length * sizeof (WCHAR));

            if ((LONG)Length < OutputLength) {
                String[Length] = L'\0';
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return (GetExceptionCode ());
        }

        return (STATUS_SUCCESS);
    }
}


NTSTATUS
RtlIntegerToUnicodeString (
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN OUT PUNICODE_STRING String
    )
{
    NTSTATUS Status;
    UCHAR ResultBuffer[16];
    ANSI_STRING AnsiString;

    RTL_PAGED_CODE();

    Status = RtlIntegerToChar (Value, Base, sizeof (ResultBuffer), ResultBuffer);
    if (NT_SUCCESS (Status)) {
        AnsiString.Buffer = ResultBuffer;
        AnsiString.MaximumLength = sizeof (ResultBuffer);
        AnsiString.Length = (USHORT)strlen (ResultBuffer);
        Status = RtlAnsiStringToUnicodeString (String, &AnsiString, FALSE);
     }

    return( Status );
}


#define MAX_DIGITS 65

NTSTATUS
RtlLargeIntegerToChar (
    IN PLARGE_INTEGER Value,
    IN ULONG Base OPTIONAL,
    IN LONG OutputLength,
    OUT PSZ String
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    CHAR Result[MAX_DIGITS], *s;
    ULONG Shift, Mask, Digit, Length;

    RTL_PAGED_CODE();

    Shift = 0;
    switch (Base) {
        case 16:    Shift = 4;  break;
        case  8:    Shift = 3;  break;
        case  2:    Shift = 1;  break;

        case  0:
        case 10:    Shift = 0;  break;
        default:    return (STATUS_INVALID_PARAMETER);
    }

    if (Shift != 0) {
        Mask = (1 << Shift) - 1;
    }

    s = &Result[MAX_DIGITS];

    if (Shift != 0) {
        ULONGLONG tValue;

        tValue = (ULONGLONG) Value->QuadPart;
        do {
            Digit  = (ULONG) (tValue & Mask);
            tValue = tValue >> Shift;
            *--s = RtlpIntegerChars[Digit];
        } while (tValue != 0);

    } else {
        LARGE_INTEGER TempValue = *Value;
        do {
            TempValue = RtlExtendedLargeIntegerDivide (TempValue, Base, &Digit);
            *--s = RtlpIntegerChars [Digit];
        } while (TempValue.QuadPart != 0);
    }

    Length = (ULONG)(&Result[MAX_DIGITS] - s);
    if (OutputLength < 0) {
        OutputLength = -OutputLength;
        while ((LONG)Length < OutputLength) {
            *String++ = '0';
            OutputLength--;
        }
    }

    if ((LONG)Length > OutputLength) {
        return (STATUS_BUFFER_OVERFLOW);
    } else {
        try {
            RtlCopyMemory (String, s, Length);

            if ((LONG)Length < OutputLength) {
                String[Length] = '\0';
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return (GetExceptionCode ());
        }

        return (STATUS_SUCCESS);
    }
}

NTSTATUS
RtlLargeIntegerToUnicode (
    IN PLARGE_INTEGER Value,
    IN ULONG Base OPTIONAL,
    IN LONG OutputLength,
    OUT PWSTR String
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    WCHAR Result [MAX_DIGITS], *s;
    ULONG Shift, Mask, Digit, Length;

    RTL_PAGED_CODE();

    Shift = 0;
    switch (Base) {
        case 16:    Shift = 4;  break;
        case  8:    Shift = 3;  break;
        case  2:    Shift = 1;  break;

        case  0:
        case 10:    Shift = 0;  break;
        default:    return (STATUS_INVALID_PARAMETER);
    }

    if (Shift != 0) {
        Mask = (1 << Shift) - 1;
    }

    s = &Result [MAX_DIGITS];
    if (Shift != 0) {
        ULONGLONG tValue;

        tValue = (ULONGLONG) Value->QuadPart;
        do {
            Digit  = (ULONG) (tValue & Mask);
            tValue = tValue >> Shift;
            *--s = RtlpIntegerWChars[Digit];
        } while (tValue != 0);
    } else {
        LARGE_INTEGER TempValue = *Value;
        do {
            TempValue = RtlExtendedLargeIntegerDivide (TempValue, Base, &Digit);
            *--s = RtlpIntegerWChars[Digit];
        } while (TempValue.QuadPart != 0);
    }

    Length = (ULONG)(&Result[MAX_DIGITS] - s);

    ASSERT (Length <= MAX_DIGITS);

    if (OutputLength < 0) {
        OutputLength = -OutputLength;
        while ((LONG)Length < OutputLength) {
            *String++ = L'0';
            OutputLength--;
        }
    }

    if ((LONG)Length > OutputLength) {
        return (STATUS_BUFFER_OVERFLOW);
    } else {
        try {
            RtlCopyMemory (String, s, Length * sizeof(WCHAR));

            if ((LONG)Length < OutputLength) {
                String [Length] = L'\0';
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return (GetExceptionCode ());
        }

        return (STATUS_SUCCESS);
    }
}

NTSTATUS
RtlInt64ToUnicodeString (
    IN ULONGLONG Value,
    IN ULONG Base OPTIONAL,
    IN OUT PUNICODE_STRING String
    )

{

    NTSTATUS Status;
    UCHAR ResultBuffer[32];
    ANSI_STRING AnsiString;
    LARGE_INTEGER Temp;

    RTL_PAGED_CODE();

    Temp.QuadPart = Value;
    Status = RtlLargeIntegerToChar(&Temp,
                                   Base,
                                   sizeof(ResultBuffer),
                                   ResultBuffer);

    if (NT_SUCCESS(Status)) {
        AnsiString.Buffer = ResultBuffer;
        AnsiString.MaximumLength = sizeof(ResultBuffer);
        AnsiString.Length = (USHORT)strlen(ResultBuffer);
        Status = RtlAnsiStringToUnicodeString(String, &AnsiString, FALSE);
    }

    return Status;
}

#if defined(ALLOC_DATA_PRAGMA)
#pragma const_seg()
#endif

