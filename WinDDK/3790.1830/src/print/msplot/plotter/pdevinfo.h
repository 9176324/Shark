/*++

Copyright (c) 1990-2003  Microsoft Corporation


Module Name:

    pdevinfo.h


Abstract:

    This module contains prototypes for pdevinfo.c


Author:

    30-Nov-1993 Tue 20:37:51 created  

    07-Dec-1993 Tue 00:21:25 updated  
        change dhsurf to dhpdev in SURFOBJ_GETPDEV

[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:

   Dec 06 1993,       Fixed to grab pdev from dhpdev instead of dhsurf

--*/


#ifndef _PDEVINFO_
#define _PDEVINFO_


PPDEV
ValidatePDEVFromSurfObj(
    SURFOBJ *
    );

#define SURFOBJ_GETPDEV(pso)    ValidatePDEVFromSurfObj(pso)


#endif  // _PDEVINFO_

