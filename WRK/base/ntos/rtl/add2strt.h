/*++

Copyright (c) Microsoft Corporation. All rights reserved. 

You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
If you do not agree to the terms, do not use the code.


Module Name:

    add2strt.h

Abstract:

    Code for IP address-to-string translation routines.

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
#define AF_INET 2
#define AF_INET6 23
#define INET_ADDRSTRLEN  22
#define INET6_ADDRSTRLEN 65

LPTSTR
RtlIpv6AddressToStringT(
    __in const struct in6_addr *Addr,
    __out_ecount(INET6_ADDRSTRLEN) LPTSTR S
    )

/*++

Routine Description:

    Generates an IPv6 string literal corresponding to the address Addr.
    The shortened canonical forms are used (RFC 1884 etc).
    The basic string representation consists of 8 hex numbers
    separated by colons, with a couple embellishments:
    - a string of zero numbers (at most one) is replaced
    with a double-colon.
    - the last 32 bits are represented in IPv4-style dotted-octet notation
    if the address is a v4-compatible or ISATAP address.

    For example,
        ::
        ::1
        ::157.56.138.30
        ::ffff:156.56.136.75
        ff01::
        ff02::2
        0:1:2:3:4:5:6:7

Arguments:

    S - Receives a pointer to the buffer in which to place the
        string literal.

    Addr - Receives the IPv6 address.

Return Value:

    Pointer to the null byte at the end of the string inserted.
    This can be used by the caller to easily append more information.

--*/

{
    int maxFirst, maxLast;
    int curFirst, curLast;
    int i;
    int endHex = 8;

    // Check for IPv6-compatible, IPv4-mapped, and IPv4-translated
    // addresses
    if ((Addr->s6_words[0] == 0) && (Addr->s6_words[1] == 0) &&
        (Addr->s6_words[2] == 0) && (Addr->s6_words[3] == 0) &&
        (Addr->s6_words[6] != 0)) {
        if ((Addr->s6_words[4] == 0) &&
             ((Addr->s6_words[5] == 0) || (Addr->s6_words[5] == 0xffff)))
        {
            // compatible or mapped
            S += _stprintf(S, _T("::%hs%u.%u.%u.%u"),
                           Addr->s6_words[5] == 0 ? "" : "ffff:",
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            return S;
        }
        else if ((Addr->s6_words[4] == 0xffff) && (Addr->s6_words[5] == 0)) {
            // translated
            S += _stprintf(S, _T("::ffff:0:%u.%u.%u.%u"),
                           Addr->s6_bytes[12], Addr->s6_bytes[13],
                           Addr->s6_bytes[14], Addr->s6_bytes[15]);
            return S;
        }
    }


    // Find largest contiguous substring of zeroes
    // A substring is [First, Last), so it's empty if First == Last.

    maxFirst = maxLast = 0;
    curFirst = curLast = 0;

    // ISATAP EUI64 starts with 00005EFE (or 02005EFE)...
    if (((Addr->s6_words[4] & 0xfffd) == 0) && (Addr->s6_words[5] == 0xfe5e)) {
        endHex = 6;
    }

    for (i = 0; i < endHex; i++) {

        if (Addr->s6_words[i] == 0) {
            // Extend current substring
            curLast = i+1;

            // Check if current is now largest
            if (curLast - curFirst > maxLast - maxFirst) {

                maxFirst = curFirst;
                maxLast = curLast;
            }
        }
        else {
            // Start a new substring
            curFirst = curLast = i+1;
        }
    }

    // Ignore a substring of length 1.
    if (maxLast - maxFirst <= 1)
        maxFirst = maxLast = 0;

        // Write colon-separated words.
        // A double-colon takes the place of the longest string of zeroes.
        // All zeroes is just "::".

    for (i = 0; i < endHex; i++) {

        // Skip over string of zeroes
        if ((maxFirst <= i) && (i < maxLast)) {

            S += _stprintf(S, _T("::"));
            i = maxLast-1;
            continue;
        }

        // Need colon separator if not at beginning
        if ((i != 0) && (i != maxLast))
            S += _stprintf(S, _T(":"));

        S += _stprintf(S, _T("%x"), RtlUshortByteSwap(Addr->s6_words[i]));
    }

    if (endHex < 8) {
        S += _stprintf(S, _T(":%u.%u.%u.%u"),
                       Addr->s6_bytes[12], Addr->s6_bytes[13],
                       Addr->s6_bytes[14], Addr->s6_bytes[15]);
    }

    return S;
}

NTSTATUS
RtlIpv6AddressToStringExT(
    __in const struct in6_addr *Address,
    __in ULONG ScopeId,
    __in USHORT Port,
    __out_ecount_part(*AddressStringLength, *AddressStringLength) LPTSTR AddressString,
    __inout PULONG AddressStringLength
    )

/*++

Routine Description:

    This is the extension routine which handles a full address conversion
    including address, scopeid and port (scopeid and port are optional).

Arguments:

    Address - The address part to be translated.

    ScopeId - The Scope ID of the address (optional).

    Port - The port number of the address (optional). 
           Port is in network byte order.

    AddressString - Pointer to output buffer where we will fill in address string.

    AddressStringLength - For input, it is the length of the input buffer; for 
                          output it is the length we actual returned.
Return Value:

    STATUS_SUCCESS if the operation is successful, error code otherwise.

--*/
{
    TCHAR String[INET6_ADDRSTRLEN];
    LPTSTR S;
    ULONG Length;
    
    if ((Address == NULL) ||
        (AddressString == NULL) ||
        (AddressStringLength == NULL)) {

        return STATUS_INVALID_PARAMETER;
    }
    S = String;
    if (Port) {
        S += _stprintf(S, _T("["));
    }

    //
    // Now translate this address.
    //
    S = RtlIpv6AddressToStringT(Address, S);
    if (ScopeId != 0) {
        S += _stprintf(S, _T("%%%u"), ScopeId);
    }
    if (Port != 0) {
        S += _stprintf(S, _T("]:%u"), RtlUshortByteSwap(Port));
    }
    Length = (ULONG)(S - String + 1);
    if (*AddressStringLength < Length) {
        //
        // Before return, tell the caller how big 
        // the buffer we need.
        //
        *AddressStringLength = Length;
        return STATUS_INVALID_PARAMETER;
    }
    *AddressStringLength = Length;
    RtlCopyMemory(AddressString, String, Length * sizeof(TCHAR));
    return STATUS_SUCCESS;

}
    

LPTSTR
RtlIpv4AddressToStringT(
    __in const struct in_addr *Addr,
    __out_ecount(16) LPTSTR S
    )

/*++

Routine Description:

    Generates an IPv4 string literal corresponding to the address Addr.

Arguments:

    S - Receives a pointer to the buffer in which to place the
        string literal.

    Addr - Receives the IPv4 address.

Return Value:

    Pointer to the null byte at the end of the string inserted.
    This can be used by the caller to easily append more information.

--*/

{
    S += _stprintf(S, _T("%u.%u.%u.%u"),
                  ( Addr->s_addr >>  0 ) & 0xFF,
                  ( Addr->s_addr >>  8 ) & 0xFF,
                  ( Addr->s_addr >> 16 ) & 0xFF,
                  ( Addr->s_addr >> 24 ) & 0xFF );

    return S;
}


NTSTATUS
RtlIpv4AddressToStringExT(
    __in const struct in_addr *Address,
    __in USHORT Port,
    __out_ecount_part(*AddressStringLength, *AddressStringLength) LPTSTR AddressString,
    __inout PULONG AddressStringLength
    )

/*++

Routine Description:

    This is the extension routine which handles a full address conversion
    including address and port (port is optional).
    
Arguments:

    Address - The address part to translate.

    Port - Port number if there is any, otherwise 0. Port is in network 
           byte order. 

    AddressString - Receives the formatted address string.
    
    AddressStringLength - On input, contains the length of AddressString.
        On output, contains the number of characters actually written
        to AddressString.

Return Value:

    STATUS_SUCCESS if the operation is successful, error code otherwise.

--*/

{

    TCHAR String[INET_ADDRSTRLEN];
    LPTSTR S;
    ULONG Length;

    //
    // Quick sanity checks.
    //
    if ((Address == NULL) ||
        (AddressString == NULL) ||
        (AddressStringLength == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }
    S = String;

    //
    // Now translate this address.
    //
    S = RtlIpv4AddressToStringT(Address, S);
    if (Port != 0) {
        S += _stprintf(S, _T(":%u"), RtlUshortByteSwap(Port));
    }
    Length = (ULONG)(S - String + 1);
    if (*AddressStringLength < Length) {
        //
        // Before return, tell the caller how big
        // the buffer we need. 
        //
        *AddressStringLength = Length;
        return STATUS_INVALID_PARAMETER;
    }
    RtlCopyMemory(AddressString, String, Length * sizeof(TCHAR));
    *AddressStringLength = Length;
    return STATUS_SUCCESS;
} 

