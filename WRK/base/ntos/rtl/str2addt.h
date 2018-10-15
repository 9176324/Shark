/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    str2addt.h

Abstract:

    Code file for IP string-to-address translation routines.

--*/

struct in6_addr {
    union {
        UCHAR Byte[16];
        USHORT Word[8];
    } u;
};
#define s6_bytes   u.Byte
#define s6_words   u.Word

struct in_addr {
        union {
                struct { UCHAR s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { USHORT s_w1,s_w2; } S_un_w;
                ULONG S_addr;
        } S_un;
};
#define s_addr  S_un.S_addr

//
// Define some versions of crt functions which are not affected by locale.
//
#define ISDIGIT(c)  (_istascii(c) && _istdigit(c))
#define ISLOWER(c)  (_istascii(c) && _istlower(c))
#define ISXDIGIT(c) (_istascii(c) && _istxdigit(c))

#define INADDR_NONE 0xffffffff

NTSTATUS
RtlIpv6StringToAddressT(
    IN LPCTSTR S,
    OUT LPCTSTR *Terminator,
    OUT struct in6_addr *Addr
    )

/*++

Routine Description:

    Parses the string S as an IPv6 address. See RFC 1884.
    The basic string representation consists of 8 hex numbers
    separated by colons, with a couple embellishments:
    - a string of zero numbers (at most one) may be replaced
    with a double-colon. Double-colons are allowed at beginning/end
    of the string.
    - the last 32 bits may be represented in IPv4-style dotted-octet notation.

    For example,
        ::
        ::1
        ::157.56.138.30
        ::ffff:156.56.136.75
        ff01::
        ff02::2
        0:1:2:3:4:5:6:7

Arguments:

    S - RFC 1884 string representation of an IPv6 address.

    Terminator - Receives a pointer to the character that terminated
        the conversion.

    Addr - Receives the IPv6 address.

Return Value:

    TRUE if parsing was successful. FALSE otherwise.

--*/

{
    enum { Start, InNumber, AfterDoubleColon } state = Start;
    const TCHAR *number = NULL;
    BOOLEAN sawHex;
    ULONG numColons = 0, numDots = 0, numDigits = 0;
    ULONG sawDoubleColon = 0;
    ULONG i = 0;
    TCHAR c;

    // There are a several difficulties here. For one, we don't know
    // when we see a double-colon how many zeroes it represents.
    // So we just remember where we saw it and insert the zeroes
    // at the end. For another, when we see the first digits
    // of a number we don't know if it is hex or decimal. So we
    // remember a pointer to the first character of the number
    // and convert it after we see the following character.

    while (c = *S) {

        switch (state) {
        case Start:
            if (c == _T(':')) {

                // this case only handles double-colon at the beginning

                if (numDots > 0)
                    goto Finish;
                if (numColons > 0)
                    goto Finish;
                if (S[1] != _T(':'))
                    goto Finish;

                sawDoubleColon = 1;
                numColons = 2;
                Addr->s6_words[i++] = 0; // pretend it was 0::
                S++;
                state = AfterDoubleColon;

            } else
        case AfterDoubleColon:
            if (ISDIGIT(c)) {

                sawHex = FALSE;
                number = S;
                state = InNumber;
                numDigits = 1;

            } else if (ISXDIGIT(c)) {

                if (numDots > 0)
                    goto Finish;

                sawHex = TRUE;
                number = S;
                state = InNumber;
                numDigits = 1;

            } else
                goto Finish;
            break;

        case InNumber:
            if (ISDIGIT(c)) {

                numDigits++;

                // remain in InNumber state

            } else if (ISXDIGIT(c)) {

                numDigits++;

                if (numDots > 0)
                    goto Finish;

                sawHex = TRUE;
                // remain in InNumber state;

            } else if (c == _T(':')) {

                if (numDots > 0)
                    goto Finish;
                if (numColons > 6)
                    goto Finish;

                if (S[1] == _T(':')) {

                    if (sawDoubleColon)
                        goto Finish;
                    if (numColons > 5)
                        goto Finish;

                    sawDoubleColon = numColons+1;
                    numColons += 2;
                    S++;
                    state = AfterDoubleColon;

                } else {
                    numColons++;
                    state = Start;
                }

            } else if (c == _T('.')) {

                if (sawHex)
                    goto Finish;
                if (numDots > 2)
                    goto Finish;
                if (numColons > 6)
                    goto Finish;
                numDots++;
                state = Start;

            } else
                goto Finish;
            break;
        }

        // If we finished a number, parse it.

        if ((state != InNumber) && (number != NULL)) {

            // Note either numDots > 0 or numColons > 0,
            // because something terminated the number.

            if (numDots == 0) {
                if (numDigits > 4)
                    return STATUS_INVALID_PARAMETER;
                Addr->s6_words[i++] =
                    RtlUshortByteSwap((USHORT) _tcstol(number, NULL, 16));
            } else {
                ULONG Temp;
                if (numDigits > 3)
                    return STATUS_INVALID_PARAMETER;
                Temp = _tcstol(number, NULL, 10);
                if (Temp > 255) 
                    return STATUS_INVALID_PARAMETER;
                Addr->s6_bytes[2*i + numDots-1] = (UCHAR) Temp;
            }
        }

        S++;
    }

Finish:
    *Terminator = S;

    // Check that we have a complete address.

    if (numDots == 0)
        ;
    else if (numDots == 3)
        numColons++;
    else
        return STATUS_INVALID_PARAMETER;

    if (sawDoubleColon)
        ;
    else if (numColons == 7)
        ;
    else
        return STATUS_INVALID_PARAMETER;

    // Parse the last number, if necessary.

    if (state == InNumber) {

        if (numDots == 0) {
            if (numDigits > 4)
                return STATUS_INVALID_PARAMETER;
            Addr->s6_words[i] =
                RtlUshortByteSwap((USHORT) _tcstol(number, NULL, 16));
        } else {
            ULONG Temp;
            if (numDigits > 3)
                return STATUS_INVALID_PARAMETER;
            Temp = _tcstol(number, NULL, 10);
            if (Temp > 255) 
                return STATUS_INVALID_PARAMETER;
            Addr->s6_bytes[2*i + numDots] = (UCHAR) Temp;
        }

    } else if (state == AfterDoubleColon) {

        Addr->s6_words[i] = 0; // pretend it was ::0

    } else
        return STATUS_INVALID_PARAMETER;

    // Insert zeroes for the double-colon, if necessary.

    if (sawDoubleColon) {

        RtlMoveMemory(&Addr->s6_words[sawDoubleColon + 8 - numColons],
                      &Addr->s6_words[sawDoubleColon],
                      (numColons - sawDoubleColon) * sizeof(USHORT));
        RtlZeroMemory(&Addr->s6_words[sawDoubleColon],
                      (8 - numColons) * sizeof(USHORT));
    }

    return STATUS_SUCCESS;
}


NTSTATUS
RtlIpv6StringToAddressExT (
    IN LPCTSTR AddressString,
    OUT struct in6_addr *Address,
    OUT PULONG ScopeId,
    OUT PUSHORT Port
    )

/*++

Routine Description:

    Parsing a human-readable string to Address, port number and scope id. 

    The syntax is address%scope-id or [address%scope-id]:port, where 
    the scope-id and port are optional.
    Note that since the IPv6 address format uses a varying number
    of ':' characters, the IPv4 convention of address:port cannot
    be supported without the braces.

Arguments:

    AddressString - Points to the zero-terminated human-readable string.

    Address - Receive address part (in6_addr) of this address.

    ScopeId - Receive scopeid of this address. If there is no scope id in
             the address string, 0 is returned. 

    Port - Receive port number of this address. If there is no port number 
          in the string, 0 is returned. Port is returned in network byte order.

Return Value:

    NT_STATUS - STATUS_SUCCESS if successful, NT error code if not.

--*/

{
    LPTSTR Terminator;
    ULONG TempScopeId;
    USHORT TempPort;
    TCHAR Ch;
    BOOLEAN ExpectBrace;

    //
    // Quick sanity checks.
    //
    if ((AddressString == NULL) ||
        (Address == NULL) ||
        (ScopeId == NULL) ||
        (Port == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    TempPort = 0;
    TempScopeId = 0;
    ExpectBrace = FALSE;
    if (*AddressString == _T('[')) {
        ExpectBrace = TRUE;
        AddressString++;
    }

    if (!NT_SUCCESS(RtlIpv6StringToAddressT(AddressString, 
                                            &Terminator, 
                                            Address))) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // We have parsed the address, check for a scope-id.
    //
    if (*Terminator == _T('%')) {
        Terminator++;
        Ch = *Terminator;
        if (!ISDIGIT(Ch)) {
            return STATUS_INVALID_PARAMETER;
        }
        while ((Ch != 0) && (Ch != _T(']'))) {
            if (!ISDIGIT(Ch)) {
                return STATUS_INVALID_PARAMETER;
            }
            //
            // first check the possibility of overflow
            //
            if (((ULONGLONG)TempScopeId * 10 + Ch - _T('0')) >  
                0xFFFFFFFF) {
                return STATUS_INVALID_PARAMETER;
            }
            TempScopeId = 10 * TempScopeId + (Ch - _T('0'));
            Terminator++;
            Ch = *Terminator;
        }
        
    }

    //
    // When we come here, the current char should either be the
    // end of the string or ']' if expectbrace is true. 
    //
    if (*Terminator == _T(']')) {
        if (!ExpectBrace) {
            return STATUS_INVALID_PARAMETER;
        }
        ExpectBrace = FALSE;
        Terminator++;
        //
        // See if we have a port to parse.
        //
        if (*Terminator == _T(':')) {
            USHORT Base;
            Terminator++;
            Base = 10;
            if (*Terminator == _T('0')) {
                Base = 8;
                Terminator++;         
                if ((*Terminator == _T('x')) || 
                    (*Terminator == _T('X'))) {
                    Base = 16;
                    Terminator++;
                }
            }
            Ch = *Terminator;
            while (Ch != 0) {
                if (ISDIGIT(Ch) && (Ch - _T('0')) < Base) {
                    //
                    // check the possibility for overflow first
                    //
                    if (((ULONG)TempPort * Base + Ch - _T('0')) >  
                         0xFFFF) {
                        return STATUS_INVALID_PARAMETER; 
                    }
                    TempPort = (TempPort * Base) + (Ch - _T('0'));
                } else if (Base == 16 && ISXDIGIT(Ch)) {
                    //
                    // check the possibility for overflow
                    //
                    if ((((ULONG)TempPort << 4) + Ch + 10 - 
                        (ISLOWER(Ch)? _T('a') : _T('A'))) > 0xFFFF) {
                        return STATUS_INVALID_PARAMETER;
                    }
                    TempPort = (TempPort << 4);
                    TempPort += Ch + 10 - (ISLOWER(Ch)? _T('a') : _T('A')); 
                } else {
                    return STATUS_INVALID_PARAMETER;
                }
                Terminator++;
                Ch = *Terminator;
            }
        }       
    }

    //
    // We finished parsing address, scope id and port number. We are expecting the
    // end of the string. 
    //
    if ((*Terminator != 0) || ExpectBrace) {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // Now construct the address.
    //
    *Port = RtlUshortByteSwap(TempPort);
    *ScopeId = TempScopeId;
    return STATUS_SUCCESS;
} 

NTSTATUS
RtlIpv4StringToAddressT(
    IN LPCTSTR String,
    IN BOOLEAN Strict,
    OUT LPCTSTR *Terminator,
    OUT struct in_addr *Addr
    )

/*++

Routine Description:

    This function interprets the character string specified by the cp
    parameter.  This string represents a numeric Internet address
    expressed in the Internet standard ".'' notation.  The value
    returned is a number suitable for use as an Internet address.  All
    Internet addresses are returned in network order (bytes ordered from
    left to right).

    Internet Addresses

    Values specified using the "." notation take one of the following
    forms:

    a.b.c.d   a.b.c     a.b  a

    When four parts are specified, each is interpreted as a byte of data
    and assigned, from left to right, to the four bytes of an Internet
    address.  Note that when an Internet address is viewed as a 32-bit
    integer quantity on the Intel architecture, the bytes referred to
    above appear as "d.c.b.a''.  That is, the bytes on an Intel
    processor are ordered from right to left.

    Note: The following notations are only used by Berkeley, and nowhere
    else on the Internet.  In the interests of compatibility with their
    software, they are supported as specified.

    When a three part address is specified, the last part is interpreted
    as a 16-bit quantity and placed in the right most two bytes of the
    network address.  This makes the three part address format
    convenient for specifying Class B network addresses as
    "128.net.host''.

    When a two part address is specified, the last part is interpreted
    as a 24-bit quantity and placed in the right most three bytes of the
    network address.  This makes the two part address format convenient
    for specifying Class A network addresses as "net.host''.

    When only one part is given, the value is stored directly in the
    network address without any byte rearrangement.

Arguments:

    String - A character string representing a number expressed in the
        Internet standard "." notation.

    Strict - If TRUE, the string must be dotted-decimal with 4 parts.
             Otherwise, any of the four forms are allowed, with decimal,
             octal, or hex.

    Terminator - Receives a pointer to the character that terminated
        the conversion.

    Addr - Receives a pointer to the structure to fill in with
        a suitable binary representation of the Internet address given. 

Return Value:

    TRUE if parsing was successful. FALSE otherwise.

--*/

{
    ULONG val, n;
    LONG base;
    TCHAR c;
    ULONG parts[4], *pp = parts;
    BOOLEAN sawDigit;

again:
    //
    // We must see at least one digit for address to be valid.
    //
    sawDigit=FALSE; 

    //
    // Collect number up to ``.''.
    // Values are specified as for C:
    // 0x=hex, 0=octal, other=decimal.
    //
    val = 0; 
    base = 10;
    if (*String == _T('0')) {
        String++;
        if (ISDIGIT(*String)) {
            base = 8;
        } else if (*String == _T('x') || *String == _T('X')) {
            base = 16;
            String++;
        } else {
            //
            // It is still decimal but we saw the digit
            // and it was 0.
            //
            sawDigit = TRUE;
        }
    }
    if (Strict && (base != 10)) {
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }

    while ((c = *String)!= 0) {
        ULONG newVal;
        if (ISDIGIT(c) && ((c - _T('0')) < base)) {
            newVal = (val * base) + (c - _T('0'));
        } else if ((base == 16) && ISXDIGIT(c)) {
            newVal = (val << 4) + (c + 10 - (ISLOWER(c) ? _T('a') : _T('A')));
        } else {
            break;
        }

        //
        // Protect from overflow
        //
        if (newVal < val) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        String++;
        sawDigit = TRUE;
        val = newVal;
    }
    if (*String == _T('.')) {
        //
        // Internet format:
        //      a.b.c.d
        //      a.b.c   (with c treated as 16-bits)
        //      a.b     (with b treated as 24 bits)
        //
        if (pp >= parts + 3) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        *pp++ = val, String++;

        //
        // Check if we saw at least one digit.
        //
        if (!sawDigit) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }

        goto again;
    }

    //
    // Check if we saw at least one digit.
    //
    if (!sawDigit) {
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }
    *pp++ = val;

    //
    // Concoct the address according to
    // the number of parts specified.
    //
    n = (ULONG)(pp - parts);
    if (Strict && (n != 4)) {
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }
    switch ((int) n) {

    case 1:                         /* a -- 32 bits */
        val = parts[0];
        break;

    case 2:                         /* a.b -- 8.24 bits */
        if ((parts[0] > 0xff) || (parts[1] > 0xffffff)) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        val = (parts[0] << 24) | (parts[1] & 0xffffff);
        break;

    case 3:                         /* a.b.c -- 8.8.16 bits */
        if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
            (parts[2] > 0xffff)) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                (parts[2] & 0xffff);
        break;

    case 4:                         /* a.b.c.d -- 8.8.8.8 bits */
        if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
            (parts[2] > 0xff) || (parts[3] > 0xff)) {
            *Terminator = String;
            return STATUS_INVALID_PARAMETER;
        }
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
              ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
        break;

    default:
        *Terminator = String;
        return STATUS_INVALID_PARAMETER;
    }

    val = RtlUlongByteSwap(val);
    *Terminator = String;
    Addr->s_addr = val;

    return STATUS_SUCCESS;
}


NTSTATUS
RtlIpv4StringToAddressExT (
    IN LPCTSTR AddressString,
    IN BOOLEAN Strict,
    OUT struct in_addr *Address,
    OUT PUSHORT Port
    )

/*++

Routine Description:

    Parsing a human-readable string to in_addr and port number.

Arguments:

    AddressString - Points to the zero-terminated human-readable string.

    Strict - If TRUE, the address portion must be dotted-decimal with 4 parts.
             Otherwise, any of the four forms are allowed, with decimal,
             octal, or hex.

    Address - Receives the address (in_addr) itself.

    Port - Receives port number. 0 is returned if there is no port number.
           Port is returned in network byte order.  

Return Value:

    NTSTATUS - STATUS_SUCCESS if successful, error code if not.

--*/

{
    LPTSTR Terminator;
    USHORT TempPort;
    
    if ((AddressString == NULL) ||
        (Address == NULL) ||
        (Port == NULL)) { 
        return STATUS_INVALID_PARAMETER;
    }

    if (!NT_SUCCESS(RtlIpv4StringToAddressT(AddressString, 
                                            Strict, 
                                            &Terminator, 
                                            Address))) {
        return STATUS_INVALID_PARAMETER;
    }
    
    if (*Terminator == _T(':')) {
        TCHAR Ch;
        USHORT Base;
        BOOLEAN ExpectPort = TRUE;
        Terminator++;
        TempPort = 0;
        Base = 10;
        if (*Terminator == _T('0')) {
            Base = 8;
            Terminator++;
            if ((*Terminator == _T('x')) || (*Terminator == _T('X'))) {
                Base = 16;
                Terminator++;
            }
        }
        if (Ch = *Terminator) {
            ExpectPort = FALSE;
        }
        while (Ch = *Terminator++) {
            if (ISDIGIT(Ch) && (USHORT)(Ch-_T('0')) < Base) {
                //
                // Check the possibility for overflow
                //
                if (((ULONG)TempPort * Base + Ch - _T('0')) > 0xFFFF) {
                    return STATUS_INVALID_PARAMETER;
                }
                TempPort = (TempPort * Base) + (Ch - _T('0'));
            } else if (Base == 16 && ISXDIGIT(Ch)) {
                //
                // Check the possibility for overflow first
                //
                if ((((ULONG)TempPort << 4) + Ch + 10 -  
                    (ISLOWER(Ch) ? _T('a') : _T('A')))
                    > 0xFFFF) {
                    return STATUS_INVALID_PARAMETER;
                }
                TempPort = TempPort << 4;
                TempPort += Ch + 10 - (ISLOWER(Ch) ? _T('a') : _T('A'));
            } else {
                return STATUS_INVALID_PARAMETER;
            }
        }
        if (ExpectPort) {
            return STATUS_INVALID_PARAMETER;
        }
    } else if (*Terminator == 0) {
        TempPort = 0;
    } else {
        return STATUS_INVALID_PARAMETER;
    }
    *Port = RtlUshortByteSwap(TempPort);
    return STATUS_SUCCESS;
}

