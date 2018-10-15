/*++

Copyright (c) 1998  Microsoft Corporation

Module Name: 

    debug.c

Abstract


Author:

    Peter Binder (pbinder) 4/08/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/08/98  pbinder   birth
--*/

#define _DEBUG_C
#include "pch.h"
#undef _DEBUG_C

void
WriteToEditWindow(
    IN HANDLE   hWnd,
    IN PUCHAR   lpszFormat, 
    IN ... 
    )
{
    char    buf[1024] = "1394API: ";
    va_list ap;

    va_start(ap, lpszFormat);

    wvsprintf( &buf[9], lpszFormat, ap );

    if (hWnd)
        WriteTextToEditControl(hWnd, buf);

    va_end(ap);
}

void 
DbgPrt(
    IN HANDLE   hWnd,
    IN PUCHAR   lpszFormat,
    IN ... 
    )
{
    char    buf[1024] = "1394API: ";
    va_list ap;

    va_start(ap, lpszFormat);

    wvsprintf( &buf[9], lpszFormat, ap );

#if defined(DBG)
    OutputDebugStringA(buf);
#endif

    if (hWnd)
        WriteTextToEditControl(hWnd, buf);

    va_end(ap);
}


