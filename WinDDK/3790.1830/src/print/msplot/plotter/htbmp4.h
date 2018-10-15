/*++

Copyright (c) 1990-2003  Microsoft Corporation


Module Name:

    htbmp4.h


Abstract:

    This module contains prototypes and definitions for htbmp4.c


Author:

    21-Dec-1993 Tue 21:33:43 created  


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/


#ifndef _HTBMP4BPP_
#define _HTBMP4BPP_


BOOL
Output4bppHTBmp(
    PHTBMPINFO  pHTBmpInfo
    );

BOOL
Output4bppRotateHTBmp(
    PHTBMPINFO  pHTBmpInfo
    );



#endif  // _HTBMP4BPP_

