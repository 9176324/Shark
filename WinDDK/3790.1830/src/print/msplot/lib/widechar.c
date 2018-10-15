/*++

Copyright (c) 1990-2003  Microsoft Corporation


Module Name:

    widechar.c


Abstract:

    This module contains all NLS unicode / ansi translation code


Author:

    18-Nov-1993 Thu 08:21:37 created  


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/

#include "precomp.h"
#pragma hdrstop



LPWSTR
str2Wstr(
    LPWSTR  pwStr,
    size_t  cchDest,
    LPSTR   pbStr
    )

/*++

Routine Description:

    This function copy a ansi string to the equvlent of unicode string which
    also include the NULL teiminator

Arguments:

    pwStr   - Point to the unicode string location, it must have the size of
              (strlen(pstr) + 1) * sizeof(WCHAR)

    pbStr   - a null teiminated string

Return Value:

    pwcs

Author:

    18-Nov-1993 Thu 08:36:00 created  


Revision History:


--*/

{
    size_t    cch;

    if (NULL == pbStr || NULL == pwStr)
    {
        return NULL;
    }

    //
    // Make sure that the size of dest buffer is large enough.
    //
    if (SUCCEEDED(StringCchLengthA(pbStr, cchDest, &cch)))
    {
        //
        // cch returned above does not include the null terminator.
        // So we need to add 1 to cch to make sure destination string is
        // null terminated.
        //
        AnsiToUniCode(pbStr, pwStr, cch+1);
        return pwStr;
    }
    else
    {
        return NULL;
    }
}






LPSTR
WStr2Str(
    LPSTR   pbStr,
    size_t  cchDest,
    LPWSTR  pwStr
    )

/*++

Routine Description:

    This function convert a UNICODE string to the ANSI string, assume that
    pbStr has same character count memory as pwStr

Arguments:

    pbStr   - Point to the ANSI string which has size of wcslen(pwStr) + 1

    pwStr   - Point to the UNICODE string


Return Value:


    pbStr


Author:

    06-Dec-1993 Mon 13:06:12 created  


Revision History:


--*/

{

    size_t    cch;

    if (NULL == pbStr || NULL == pwStr)
    {
        return NULL;
    }

    if (SUCCEEDED(StringCchLengthW(pwStr, cchDest, &cch)))
    {
        //
        // cch returned above does not include the null terminator.
        // So we need to add 1 to cch to make sure destination string is 
        // null terminated.
        //
        UniCodeToAnsi(pbStr, pwStr, cch+1);
        return pbStr;
    }
    else
    {
        return NULL;
    }
}



