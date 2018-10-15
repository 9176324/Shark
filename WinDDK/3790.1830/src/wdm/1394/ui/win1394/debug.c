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
4/22/98  pbinder   made into template
4/22/98  pbinder   taken for 1394test
5/04/98  pbinder   taken for win1394
--*/

#define _DEBUG_C
#include "pch.h"
#undef _DEBUG_C

void 
DbgPrt(
    IN HANDLE   hWnd,
    IN PUCHAR   lpszFormat,
    IN ... 
    )
{
    char    buf[1024]; // = "WIN1394: ";
    va_list ap;

    va_start(ap, lpszFormat);

    wvsprintf( &buf[0], lpszFormat, ap );

#if defined(DBG)
    OutputDebugStringA(buf);
#endif

    if (hWnd)
        WriteTextToEditControl(hWnd, buf);

    va_end(ap);
}


