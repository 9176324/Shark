/*++

Copyright (c) 1990-2003  Microsoft Corporation


Module Name:

    plotdbg.c


Abstract:

    This module contains all plotter's debugging functions


Author:

    15-Nov-1993 Mon 17:57:24 created  


[Environment:]

    GDI Device Driver - Plotter.


[Notes:]


Revision History:


--*/


#include "precomp.h"
#pragma hdrstop

#if DBG

BOOL    DoPlotWarn = FALSE;



VOID
cdecl
PlotDbgPrint(
    LPSTR   pszFormat,
    ...
    )

/*++

Routine Description:

    This fucntion output the debug informat to the debugger


Arguments:

    pszFormat   - format string

    ...         - variable data


Return Value:


    VOID

Author:

    15-Nov-1993 Mon 17:57:59 created  


Revision History:


--*/

{
    va_list         vaList;

#if defined(UMODE) || defined(USERMODE_DRIVER)

    static WCHAR    wOutBuf[768];
    static WCHAR    wFormatBuf[256];
    size_t          cch;

    //
    // We assume that UNICODE flag is turn on for the compilation, bug the
    // format string passed to here is ASCII version, so we need to convert
    // it to LPWSTR before the wvsprintf()
    //

    if (!SUCCEEDED(StringCchLengthA(pszFormat, CCHOF(wFormatBuf), &cch)))
    {
        return;
    }

    va_start(vaList, pszFormat);
    MultiByteToWideChar(CP_ACP, 0, pszFormat, -1, wFormatBuf, CCHOF(wFormatBuf));
    if (!SUCCEEDED(StringCchVPrintfW(wOutBuf, CCHOF(wOutBuf), wFormatBuf, vaList)))
    {
        return;
    }
    va_end(vaList);

    OutputDebugString((LPCTSTR)wOutBuf);
    OutputDebugString(TEXT("\n"));

#else

    va_start(vaList, pszFormat);
    EngDebugPrint("PLOT",pszFormat,vaList);
    va_end(vaList);

#endif
}




VOID
PlotDbgType(
    INT    Type
    )

/*++

Routine Description:

    this function output the ERROR/WARNING message


Arguments:

    Type

Return Value:


Author:

    15-Nov-1993 Mon 22:53:01 created  


Revision History:


--*/

{
    extern  TCHAR   DebugDLLName[];

#if defined(UMODE) || defined(USERMODE_DRIVER)

    if (Type) {

        OutputDebugString((Type < 0) ? TEXT("ERROR: ") : TEXT("WARNING: "));
    }

    OutputDebugString(DebugDLLName);
    OutputDebugString(TEXT("!"));

#else

    PlotDbgPrint("%s: %ws!\n",
                 (Type < 0) ? "ERROR" : "WARNING",
                 DebugDLLName);
#endif
}




VOID
_PlotAssert(
    LPSTR   pMsg,
    LPSTR   pFalseExp,
    LPSTR   pFilename,
    UINT    LineNo,
    DWORD_PTR   Exp,
    BOOL    Stop
    )

/*++

Routine Description:

    This function output assertion message and false expression to the debugger
    then break into the debugger


Arguments:

    pMsg        - Message to displayed

    pFlaseExp   - false expression

    pFilename   - plotter source filename

    LineNo      - line number of the flase expression

Return Value:

    VOID


Author:

    15-Nov-1993 Mon 18:47:30 created  


Revision History:


--*/

{
    PlotDbgPrint("\n");

    if ((pMsg) && (*pMsg)) {

        PlotDbgPrint(pMsg, Exp);
    }

    if (pFalseExp && pFilename) {
        PlotDbgPrint("Assertion failed (%hs) in %hs line %u",
	            pFalseExp, pFilename, LineNo);
    }

    if (Stop) {

        DebugBreak();
    }
}



#endif  // DBG

