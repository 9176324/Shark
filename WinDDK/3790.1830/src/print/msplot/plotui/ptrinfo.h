/*++

Copyright (c) 1990-2003  Microsoft Corporation


Module Name:

    uituils.h


Abstract:

    This module contains the defines for UIUtils.c


Author:

    03-Dec-1993 Fri 21:35:50 created  


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/



#ifndef _PRINTER_INFO_
#define _PRINTER_INFO_


#define MPF_DEVICEDATA      0x00000001
#define MPF_HELPFILE        0x00000002
#define MPF_PCPSUI          0x00000004


PPRINTERINFO
MapPrinter(
    HANDLE          hPrinter,
    PPLOTDEVMODE    pPlotDMIn,
    LPDWORD         pdwErrIDS,
    DWORD           MPFlags
    );

VOID
UnMapPrinter(
    PPRINTERINFO    pPI
    );

LPBYTE
GetPrinterInfo(
    HANDLE  hPrinter,
    UINT    PrinterInfoLevel
    );

DWORD
GetPlotterIconID(
    PPRINTERINFO    pPI
    );


#endif  // _PRINTER_INFO_

