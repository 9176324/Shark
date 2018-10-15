//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright  1996 - 2003  Microsoft Corporation.  All Rights Reserved.
//
//  FILE:   Debug.cpp
//    
//
//  PURPOSE:  Implementation of the COemUniDbg debug class and 
//          its associated debug functions.
//
//
//  Functions:
//          COemUniDbg constructor and destructor
//          COemUniDbg::OEMDebugMessage
//          COemUniDbg::OEMRealDebugMessage
//          COemUniDbg::StripDirPrefixA
//          COemUniDbg::EnsureLabel
//          COemUniDbg::vDumpFlags
//          COemUniDbg::vDumpOemDevMode
//          COemUniDbg::vDumpOemDMParam
//          COemUniDbg::vDumpSURFOBJ
//          COemUniDbg::vDumpSTROBJ
//          COemUniDbg::vDumpFONTOBJ
//          COemUniDbg::vDumpCLIPOBJ
//          COemUniDbg::vDumpBRUSHOBJ
//          COemUniDbg::vDumpGDIINFO
//          COemUniDbg::vDumpDEVINFO
//          COemUniDbg::vDumpBitmapInfoHeader
//          COemUniDbg::vDumpPOINTL
//          COemUniDbg::vDumpRECTL
//          COemUniDbg::vDumpXLATEOBJ
//          COemUniDbg::vDumpCOLORADJUSTMENT
//
//
//
//  PLATFORMS:  Windows XP, Windows Server 2003
//
//
//  History: 
//          06/28/03    xxx created.
//
//

#include "precomp.h"
#include "bitmap.h"
#include "devmode.h"
#include "debug.h"

// StrSafe.h needs to be included last
// to disallow bad string functions.
#include <STRSAFE.H>



////////////////////////////////////////////////////////
//      INTERNAL DEFINES
////////////////////////////////////////////////////////

#define DEBUG_BUFFER_SIZE       1024
#define PATH_SEPARATOR          '\\'
#define MAX_LOOP                10

// Determine what level of debugging messages to eject. 
#ifdef VERBOSE_MSG
    #define DEBUG_LEVEL     DBG_VERBOSE
#elif TERSE_MSG
    #define DEBUG_LEVEL     DBG_TERSE
#elif WARNING_MSG
    #define DEBUG_LEVEL     DBG_WARNING
#elif ERROR_MSG
    #define DEBUG_LEVEL     DBG_ERROR
#elif RIP_MSG
    #define DEBUG_LEVEL     DBG_RIP
#elif NO_DBG_MSG
    #define DEBUG_LEVEL     DBG_NONE
#else
    #define DEBUG_LEVEL     DBG_WARNING
#endif



////////////////////////////////////////////////////////
//      EXTERNAL GLOBALS
////////////////////////////////////////////////////////

INT giDebugLevel = DEBUG_LEVEL;

////////////////////////////////////////////////////////////////////////////////
//
// COemUniDbg body
//

__stdcall COemUniDbg::COemUniDbg(
    INT     iDebugLevel,
    PWSTR   pszInTag
    )
{
    // Do any init stuff here.
    //

    // Check if the debug level is appropriate to output the tag
    //
    if (iDebugLevel >= giDebugLevel)
    {
        PCWSTR pszTag = EnsureLabel(pszInTag, L"??? function entry.");
        OEMDebugMessage(DLLTEXT("%s\r\n"), pszTag);
    }
}

__stdcall COemUniDbg::~COemUniDbg(
    VOID
    )
{
    // Do any cleanup stuff here.
    //
}

BOOL __stdcall
COemUniDbg::
OEMDebugMessage(
    LPCWSTR lpszMessage, 
    ...
    )

/*++

Routine Description:

    Outputs variable argument debug string.
    
Arguments:

    lpszMessage - format string

Return Value:

    TRUE if successful, FALSE if there is an error

--*/

{
    BOOL    bResult;
    va_list VAList;

    // Pass the variable parameters to OEMRealDebugMessage to be processed.
    va_start(VAList, lpszMessage);
    bResult = OEMRealDebugMessage(MAX_PATH, lpszMessage, VAList);
    va_end(VAList);

    return bResult;
}


BOOL __stdcall
COemUniDbg::
OEMRealDebugMessage(
    DWORD       dwSize, 
    LPCWSTR lpszMessage, 
    va_list     arglist
    )

/*++

Routine Description:

    Outputs variable argument debug string.
    
Arguments:

    dwSize - size of temp buffer to hold formatted string
    lpszMessage - format string
    arglist - Variable argument list...

Return Value:

    TRUE if successful, FALSE if there is an error

--*/

{
    LPWSTR      lpszMsgBuf;
    HRESULT     hResult;

    // Parameter checking.
    if( (NULL == lpszMessage)
        ||
        (0 == dwSize) )
    {
        return FALSE;
    }

    // Allocate memory for message buffer.
    lpszMsgBuf = new WCHAR[dwSize + 1];    
    if(NULL == lpszMsgBuf)
        return FALSE;

    // Pass the variable parameters to wvsprintf to be formated.
    hResult = StringCbVPrintfW(lpszMsgBuf, (dwSize + 1) * sizeof(WCHAR), lpszMessage, arglist);

    // Dump string to debug output.
    OutputDebugStringW(lpszMsgBuf);

    // Clean up.
    delete[] lpszMsgBuf;

    return SUCCEEDED(hResult);
}

PCSTR __stdcall
COemUniDbg::
StripDirPrefixA(
    IN PCSTR    pstrFilename
    )

/*++

Routine Description:

    Strip the directory prefix off a filename (ANSI version)

Arguments:

    pstrFilename - Pointer to filename string

Return Value:

    Pointer to the last component of a filename (without directory prefix)

--*/

{
    PCSTR   pstr;

    if (pstr = strrchr(pstrFilename, PATH_SEPARATOR))
        return pstr + 1;

    return pstrFilename;
}

PCWSTR __stdcall
COemUniDbg::
EnsureLabel(
    PCWSTR      pszInLabel, 
    PCWSTR      pszDefLabel
    )

/*++

Routine Description:

    This function checks if pszInLabel is valid. If not, it returns
    pszDefLabel else pszInLabel is returned.
    
Arguments:

    pszInLabel - custom label string passed in with the call to the dump function
    pszDefLabel - default label string in case custom label string is not valid

Return Value:

    pszInLabel if it is valid, else pszDefLabel

--*/

{
    // By design, pszDefLabel is assumed to be a valid, non-empty
    // string (since it is supplied internally).
    //
    if (!pszInLabel || !*pszInLabel)
    {
        // The caller supplied a NULL string or empty string;
        // supply the internal default.
        //
        return pszDefLabel;
    }
    return pszInLabel;
}

void __stdcall
COemUniDbg::
vDumpFlags(
    DWORD           dwFlags,
    PDBG_FLAGS      pDebugFlags
    )

/*++

Routine Description:

    Dumps the combination of flags in members such as 
    pso->fjBitmap, pstro->flAccel, pfo->flFontType etc.
    
Arguments:

    dwFlags - combined value of the relevant member
        Example values are pso->fjBitmap, pstro->flAccel
    pDebugFlags - structure containing the different possible values
        that can be combined in dwFlags

Return Value:

    NONE

--*/

{
    DWORD dwFound = 0;
    BOOL bFirstFlag = FALSE;

    OEMDebugMessage(TEXT( "%#x ["), dwFlags);

    // Traverse through the list of flags to see if any match
    //
    for ( ; pDebugFlags->dwFlag; ++pDebugFlags)
    {
        if(dwFlags & pDebugFlags->dwFlag)
        {
            if (!bFirstFlag)
            {
                OEMDebugMessage(TEXT("%s"), pDebugFlags->pszFlag);
                bFirstFlag = TRUE;
            }
            else
            {
                OEMDebugMessage(TEXT(" | %s"), pDebugFlags->pszFlag);
            }
            dwFound |= pDebugFlags->dwFlag;
        }
    }

    OEMDebugMessage(TEXT("]"));

    //
    // Check if there are extra bits set that we don't understand.
    //
    if(dwFound != dwFlags)
    {
        OEMDebugMessage(TEXT("  <ExtraBits: %x>"), dwFlags & ~dwFound);
    }

    OEMDebugMessage(TEXT("\r\n"));
}

void __stdcall
COemUniDbg::
vDumpOemDevMode(
    INT             iDebugLevel,
    PWSTR           pszInLabel,
    PCOEMDEV        pOemDevmode
    )

/*++

Routine Description:

    Dumps the members of a private OEM devmode (OEMDEV) structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pOemDevmode - pointer to the OEMDEV strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }
    
    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pOemDevMode");
    
    // Return if strct to be dumped is invalid
    //
    if (!pOemDevmode)
    {
        OEMDebugMessage(TEXT("\npOemDevmode [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }

    // Format the data
    //
    OEMDebugMessage(TEXT("\npOemDevmode [%s]: %#x\r\n"), pszLabel, pOemDevmode);

    if ((pOemDevmode->dmOEMExtra.dwSize >= sizeof(OEMDEV))
        &&
        (OEM_SIGNATURE == pOemDevmode->dmOEMExtra.dwSignature))
    {
        OEMDebugMessage(TEXT("\tdmOEMExtra.dwSize: %d\r\n"), pOemDevmode->dmOEMExtra.dwSize);
        OEMDebugMessage(TEXT("\tdmOEMExtra.dwSignature: %#x\r\n"), pOemDevmode->dmOEMExtra.dwSignature);
        OEMDebugMessage(TEXT("\tdmOEMExtra.dwVersion: %#x\r\n"), pOemDevmode->dmOEMExtra.dwVersion);
        OEMDebugMessage(TEXT("\tdwDriverData: %#x\r\n"), pOemDevmode->dwDriverData);
    }
    else
    {
        OEMDebugMessage(TEXT("\tUnknown private OEM DEVMODE.\r\n"));
    }

    OEMDebugMessage(TEXT("\n"));
}


void __stdcall
COemUniDbg::
vDumpOemDMParam(
    INT             iDebugLevel,
    PWSTR           pszInLabel,
    POEMDMPARAM pOemDMParam
    )

/*++

Routine Description:

    Dumps the members of a OEMDMPARAM structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pOemDMParam - pointer to the OEMDMPARAM strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }
    
    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pOemDMParam");
    
    // Return if strct to be dumped is invalid
    //
    if (!pOemDMParam)
    {
        OEMDebugMessage(TEXT("\npOemDMParam [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }

    // Format the data
    //
    OEMDebugMessage(TEXT("\npOemDMParam [%s]: %#x\r\n"), pszLabel, pOemDMParam);
    OEMDebugMessage(TEXT("\tcbSize = %d\r\n"), pOemDMParam->cbSize);
    OEMDebugMessage(TEXT("\tpdriverobj = %#x\r\n"), pOemDMParam->pdriverobj);
    OEMDebugMessage(TEXT("\thPrinter = %#x\r\n"), pOemDMParam->hPrinter);
    OEMDebugMessage(TEXT("\thModule = %#x\r\n"), pOemDMParam->hModule);
    OEMDebugMessage(TEXT("\tpPublicDMIn = %#x\r\n"), pOemDMParam->pPublicDMIn);
    OEMDebugMessage(TEXT("\tpPublicDMOut = %#x\r\n"), pOemDMParam->pPublicDMOut);
    OEMDebugMessage(TEXT("\tpOEMDMIn = %#x\r\n"), pOemDMParam->pOEMDMIn);
    OEMDebugMessage(TEXT("\tpOEMDMOut = %#x\r\n"), pOemDMParam->pOEMDMOut);
    OEMDebugMessage(TEXT("\tcbBufSize = %d\r\n"), pOemDMParam->cbBufSize);

    OEMDebugMessage(TEXT("\n"));

}

#if DBG
    DBG_FLAGS gafdSURFOBJ_fjBitmap[] = {
        { L"BMF_TOPDOWN",       BMF_TOPDOWN},
        { L"BMF_NOZEROINIT",        BMF_NOZEROINIT},
        { L"BMF_DONTCACHE",     BMF_DONTCACHE},
        { L"BMF_USERMEM",       BMF_USERMEM},
        { L"BMF_KMSECTION",     BMF_KMSECTION},
        { L"BMF_NOTSYSMEM",     BMF_NOTSYSMEM},
        { L"BMF_WINDOW_BLT",    BMF_WINDOW_BLT},
        {NULL, 0}               // The NULL entry is important
//      { L"BMF_UMPDMEM",       BMF_UMPDMEM},
//      { L"BMF_RESERVED",      BMF_RESERVED},
    };
#else
    DBG_FLAGS gafdSURFOBJ_fjBitmap[] = {
        {NULL, 0}
    };
#endif

void __stdcall
COemUniDbg::
vDumpSURFOBJ(
    INT         iDebugLevel,
    PWSTR       pszInLabel,
    SURFOBJ     *pso
    )

/*++

Routine Description:

    Dumps the members of a SURFOBJ structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pso - pointer to the SURFOBJ strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pso");

    // Return if strct to be dumped is invalid
    //
    if (!pso)
    {
        OEMDebugMessage(TEXT("\nSURFOBJ [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nSURFOBJ [%s]: %#x\r\n"), pszLabel, pso);
    OEMDebugMessage(TEXT("\tdhSurf: %#x\r\n"), pso->dhsurf);
    OEMDebugMessage(TEXT("\thSurf: %#x\r\n"), pso->hsurf);
    OEMDebugMessage(TEXT("\tdhpdev: %#x\r\n"), pso->dhpdev);
    OEMDebugMessage(TEXT("\thdev: %#x\r\n"), pso->hdev);
    OEMDebugMessage(TEXT("\tsizlBitmap: [%ld x %ld]\r\n"), pso->sizlBitmap.cx, pso->sizlBitmap.cy);
    OEMDebugMessage(TEXT("\tcjBits: %ld\r\n"), pso->cjBits);
    OEMDebugMessage(TEXT("\tpvBits: %#x\r\n"), pso->pvBits);
    OEMDebugMessage(TEXT("\tpvScan0: %#x\r\n"), pso->pvScan0);
    OEMDebugMessage(TEXT("\tlDelta: %ld\r\n"), pso->lDelta);
    OEMDebugMessage(TEXT("\tiUniq: %ld\r\n"), pso->iUniq);

    PWSTR psziBitmapFormat = L"0";
    switch (pso->iBitmapFormat)
    {
        case BMF_1BPP : psziBitmapFormat = L"BMF_1BPP" ; break;
        case BMF_4BPP : psziBitmapFormat = L"BMF_4BPP" ; break;
        case BMF_8BPP : psziBitmapFormat = L"BMF_8BPP" ; break;
        case BMF_16BPP: psziBitmapFormat = L"BMF_16BPP"; break;
        case BMF_24BPP: psziBitmapFormat = L"BMF_24BPP"; break;
        case BMF_32BPP: psziBitmapFormat = L"BMF_32BPP"; break;
        case BMF_4RLE : psziBitmapFormat = L"BMF_4RLE" ; break;
        case BMF_8RLE : psziBitmapFormat = L"BMF_8RLE" ; break;
        case BMF_JPEG : psziBitmapFormat = L"BMF_JPEG" ; break;
        case BMF_PNG  : psziBitmapFormat = L"BMF_PNG " ; break;
    }
    OEMDebugMessage(TEXT("\tiBitmapFormat: %s\r\n"), psziBitmapFormat);

    PWSTR psziType = L"0";
    switch (pso->iType)
    {
        case STYPE_BITMAP   : psziType = L"STYPE_BITMAP"   ; break;
        case STYPE_DEVBITMAP: psziType = L"STYPE_DEVBITMAP"; break;
        case STYPE_DEVICE   : psziType = L"STYPE_DEVICE"   ; break;
    }
    OEMDebugMessage(TEXT("\tiType: %s\r\n"), psziType);

    OEMDebugMessage(TEXT("\tfjBitmap: "));
    if (STYPE_BITMAP == pso->iType)
        vDumpFlags(pso->fjBitmap, gafdSURFOBJ_fjBitmap);
    else
        OEMDebugMessage(TEXT("IGNORE\r\n"));

    OEMDebugMessage(TEXT("\n"));

}

#if DBG
    DBG_FLAGS gafdSTROBJ_flAccel[] = {
        { L"SO_FLAG_DEFAULT_PLACEMENT",             SO_FLAG_DEFAULT_PLACEMENT},
        { L"SO_HORIZONTAL",                         SO_HORIZONTAL},
        { L"SO_VERTICAL",                           SO_VERTICAL},
        { L"SO_REVERSED",                           SO_REVERSED},
        { L"SO_ZERO_BEARINGS",                      SO_ZERO_BEARINGS},
        { L"SO_CHAR_INC_EQUAL_BM_BASE",         SO_CHAR_INC_EQUAL_BM_BASE},
        { L"SO_MAXEXT_EQUAL_BM_SIDE",               SO_MAXEXT_EQUAL_BM_SIDE},
        { L"SO_DO_NOT_SUBSTITUTE_DEVICE_FONT",      SO_DO_NOT_SUBSTITUTE_DEVICE_FONT},
        { L"SO_GLYPHINDEX_TEXTOUT",                 SO_GLYPHINDEX_TEXTOUT},
        { L"SO_ESC_NOT_ORIENT",                     SO_ESC_NOT_ORIENT},
        { L"SO_DXDY",                               SO_DXDY},
        { L"SO_CHARACTER_EXTRA",                    SO_CHARACTER_EXTRA},
        { L"SO_BREAK_EXTRA",                            SO_BREAK_EXTRA},
        {NULL, 0}                                   // The NULL entry is important
    };
#else
    DBG_FLAGS gafdSTROBJ_flAccel[] = {
        {NULL, 0}
    };
#endif

void __stdcall
COemUniDbg::
vDumpSTROBJ(
    INT         iDebugLevel,
    PWSTR       pszInLabel,
    STROBJ      *pstro
    )

/*++

Routine Description:

    Dumps the members of a STROBJ structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pstro - pointer to the STROBJ strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pstro");

    // Return if strct to be dumped is invalid
    //
    if (!pstro)
    {
        OEMDebugMessage(TEXT("\nSTROBJ [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nSTROBJ [%s]: %#x\r\n"), pszLabel, pstro);
    OEMDebugMessage(TEXT("\tcGlyphs: %ld\r\n"), pstro->cGlyphs);
    OEMDebugMessage(TEXT("\tflAccel: "));
    vDumpFlags(pstro->flAccel, gafdSTROBJ_flAccel);
    OEMDebugMessage(TEXT("\tulCharInc: %ld\r\n"), pstro->ulCharInc);
    OEMDebugMessage(TEXT("\trclBkGround: (%ld, %ld) (%ld, %ld)\r\n"), pstro->rclBkGround.left, pstro->rclBkGround.top, pstro->rclBkGround.right, pstro->rclBkGround.bottom);
    if (!pstro->pgp)
        OEMDebugMessage(TEXT("\tpgp: NULL\r\n"));
    else
        OEMDebugMessage(TEXT("\tpgp: %#x\r\n"), pstro->pgp);
    OEMDebugMessage(TEXT("\tpwszOrg: \"%s\"\r\n"), pstro->pwszOrg);

    OEMDebugMessage(TEXT("\n"));

}

#if DBG
    DBG_FLAGS gafdFONTOBJ_flFontType[] = {
        { L"FO_TYPE_RASTER",        FO_TYPE_RASTER},
        { L"FO_TYPE_DEVICE",        FO_TYPE_DEVICE},
        { L"FO_TYPE_TRUETYPE",  FO_TYPE_TRUETYPE},
        { L"FO_TYPE_OPENTYPE",  0x8},
        { L"FO_SIM_BOLD",       FO_SIM_BOLD},
        { L"FO_SIM_ITALIC",     FO_SIM_ITALIC},
        { L"FO_EM_HEIGHT",      FO_EM_HEIGHT},
        { L"FO_GRAY16",         FO_GRAY16},
        { L"FO_NOGRAY16",       FO_NOGRAY16},
        { L"FO_CFF",                FO_CFF},
        { L"FO_POSTSCRIPT",     FO_POSTSCRIPT},
        { L"FO_MULTIPLEMASTER", FO_MULTIPLEMASTER},
        { L"FO_VERT_FACE",      FO_VERT_FACE},
        { L"FO_DBCS_FONT",      FO_DBCS_FONT},
        {NULL, 0}               // The NULL entry is important
    };
#else
    DBG_FLAGS gafdFONTOBJ_flFontType[] = {
        {NULL, 0}
    };
#endif

void __stdcall
COemUniDbg::
vDumpFONTOBJ(
    INT         iDebugLevel,
    PWSTR       pszInLabel,
    FONTOBJ     *pfo
    )

/*++

Routine Description:

    Dumps the members of a FONTOBJ structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pfo - pointer to the FONTOBJ strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pfo");

    // Return if strct to be dumped is invalid
    //
    if (!pfo)
    {
        OEMDebugMessage(TEXT("\nFONTOBJ [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nFONTOBJ [%s]: %#x\r\n"), pszLabel, pfo);
    OEMDebugMessage(TEXT("\tiUniq: %ld\r\n"), pfo->iUniq);
    OEMDebugMessage(TEXT("\tiFace: %ld\r\n"), pfo->iFace);
    OEMDebugMessage(TEXT("\tcxMax: %ld\r\n"), pfo->cxMax);
    OEMDebugMessage(TEXT("\tflFontType: "));
    vDumpFlags(pfo->flFontType, gafdFONTOBJ_flFontType);
    OEMDebugMessage(TEXT("\tiTTUniq: %#x\r\n"), pfo->iTTUniq);
    OEMDebugMessage(TEXT("\tiFile: %#x\r\n"), pfo->iFile);
    OEMDebugMessage(TEXT("\tsizLogResPpi: [%ld x %ld]\r\n"), pfo->sizLogResPpi.cx, pfo->sizLogResPpi.cy);
    OEMDebugMessage(TEXT("\tulStyleSize: %ld\r\n"), pfo->ulStyleSize);
    if (!pfo->pvConsumer)
        OEMDebugMessage(TEXT("\tpvConsumer: NULL\r\n"));
    else
        OEMDebugMessage(TEXT("\tpvConsumer: %#x\r\n"), pfo->pvConsumer);
    if (!pfo->pvProducer)
        OEMDebugMessage(TEXT("\tpvProducer: NULL\r\n"));
    else
        OEMDebugMessage(TEXT("\tpvProducer: %#x\r\n"), pfo->pvProducer);

    OEMDebugMessage(TEXT("\n"));

}

void __stdcall
COemUniDbg::
vDumpCLIPOBJ(
    INT         iDebugLevel,
    PWSTR       pszInLabel,
    CLIPOBJ     *pco
    )

/*++

Routine Description:

    Dumps the members of a CLIPOBJ structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pco - pointer to the CLIPOBJ strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pco");

    // Return if strct to be dumped is invalid
    //
    if (!pco)
    {
        OEMDebugMessage(TEXT("\nCLIPOBJ [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nCLIPOBJ [%s]: %#x\r\n"), pszLabel, pco);
    OEMDebugMessage(TEXT("\tiUniq: %ld\r\n"), pco->iUniq);
    OEMDebugMessage(TEXT("\trclBounds: (%ld, %ld) (%ld, %ld)\r\n"), pco->rclBounds.left, pco->rclBounds.top, pco->rclBounds.right, pco->rclBounds.bottom);

    PWSTR psziDComplexity = L"Unknown iDComplexity.";
    switch (pco->iDComplexity )
    {
        case DC_COMPLEX: psziDComplexity = L"DC_COMPLEX"; break;
        case DC_RECT: psziDComplexity = L"DC_RECT"; break;
        case DC_TRIVIAL: psziDComplexity = L"DC_TRIVIAL"; break;
    }
    OEMDebugMessage(TEXT("\tiDComplexity: %s\r\n"), psziDComplexity);

    PWSTR psziFComplexity = L"0";
    switch (pco->iFComplexity)
    {
        case FC_COMPLEX: psziFComplexity = L"FC_COMPLEX"; break;
        case FC_RECT: psziFComplexity = L"FC_RECT"; break;
        case FC_RECT4: psziFComplexity = L"FC_RECT4"; break;
    }
    OEMDebugMessage(TEXT("\tiFComplexity: %s\r\n"), psziFComplexity);

    PWSTR psziMode = L"0";
    switch (pco->iMode)
    {
        case TC_PATHOBJ: psziMode = L"TC_PATHOBJ"; break;
        case TC_RECTANGLES: psziMode = L"TC_RECTANGLES"; break;
    }
    OEMDebugMessage(TEXT("\tiMode: %s\r\n"), psziMode);

    PWSTR pszfjOptions = L"0";
    switch (pco->fjOptions)
    {
        case OC_BANK_CLIP: pszfjOptions = L"TC_PATHOBJ"; break;
    }
    OEMDebugMessage(TEXT("\tfjOptions: %s\r\n"), pszfjOptions);

    OEMDebugMessage(TEXT("\n"));
}

#if DBG
    DBG_FLAGS gafdBRUSHOBJ_flColorType[] = {
        { L"BR_CMYKCOLOR",      BR_CMYKCOLOR},
        { L"BR_DEVICE_ICM",     BR_DEVICE_ICM},
        { L"BR_HOST_ICM",       BR_HOST_ICM},
        {NULL, 0}               // The NULL entry is important
    };
#else
    DBG_FLAGS gafdBRUSHOBJ_flColorType[] = {
        {NULL, 0}
    };
#endif

void __stdcall
COemUniDbg::
vDumpBRUSHOBJ(
    INT         iDebugLevel,
    PWSTR       pszInLabel,
    BRUSHOBJ    *pbo
    )

/*++

Routine Description:

    Dumps the members of a BRUSHOBJ structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pbo - pointer to the BRUSHOBJ strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pbo");

    // Return if strct to be dumped is invalid
    //
    if (!pbo)
    {
        OEMDebugMessage(TEXT("\nBRUSHOBJ [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nBRUSHOBJ [%s]: %#x\r\n"), pszLabel, pbo);
    OEMDebugMessage(TEXT("\tiSolidColor: %#x\r\n"), pbo->iSolidColor);
    OEMDebugMessage(TEXT("\tpvRbrush: %#x\r\n"), pbo->pvRbrush);
    OEMDebugMessage(TEXT("\tflColorType: "));
    vDumpFlags(pbo->flColorType, gafdBRUSHOBJ_flColorType);

    OEMDebugMessage(TEXT("\n"));
}

#if DBG
    DBG_FLAGS gafdGDIINFO_flHTFlags[] = {
        { L"HT_FLAG_8BPP_CMY332_MASK",          HT_FLAG_8BPP_CMY332_MASK},
        { L"HT_FLAG_ADDITIVE_PRIMS",                HT_FLAG_ADDITIVE_PRIMS},
        { L"HT_FLAG_DO_DEVCLR_XFORM",           HT_FLAG_DO_DEVCLR_XFORM},
        { L"HT_FLAG_HAS_BLACK_DYE",             HT_FLAG_HAS_BLACK_DYE},
        { L"HT_FLAG_HIGH_INK_ABSORPTION",       HT_FLAG_HIGH_INK_ABSORPTION},
        { L"HT_FLAG_HIGHER_INK_ABSORPTION",     HT_FLAG_HIGHER_INK_ABSORPTION},
        { L"HT_FLAG_HIGHEST_INK_ABSORPTION",    HT_FLAG_HIGHEST_INK_ABSORPTION},
        { L"HT_FLAG_INK_ABSORPTION_IDX0",       HT_FLAG_INK_ABSORPTION_IDX0},
        { L"HT_FLAG_INK_ABSORPTION_IDX1",       HT_FLAG_INK_ABSORPTION_IDX1},
        { L"HT_FLAG_INK_ABSORPTION_IDX2",       HT_FLAG_INK_ABSORPTION_IDX2},
        { L"HT_FLAG_INK_ABSORPTION_IDX3",       HT_FLAG_INK_ABSORPTION_IDX3},
        { L"HT_FLAG_INK_HIGH_ABSORPTION",       HT_FLAG_INK_HIGH_ABSORPTION},
        { L"HT_FLAG_LOW_INK_ABSORPTION",        HT_FLAG_LOW_INK_ABSORPTION},
        { L"HT_FLAG_LOWER_INK_ABSORPTION",      HT_FLAG_LOWER_INK_ABSORPTION},
        { L"HT_FLAG_LOWEST_INK_ABSORPTION",     HT_FLAG_LOWEST_INK_ABSORPTION},
        { L"HT_FLAG_OUTPUT_CMY",                    HT_FLAG_OUTPUT_CMY},
        { L"HT_FLAG_PRINT_DRAFT_MODE",          HT_FLAG_PRINT_DRAFT_MODE},
        { L"HT_FLAG_SQUARE_DEVICE_PEL",         HT_FLAG_SQUARE_DEVICE_PEL},
        { L"HT_FLAG_USE_8BPP_BITMASK",          HT_FLAG_USE_8BPP_BITMASK},
        { L"HT_FLAG_NORMAL_INK_ABSORPTION",     HT_FLAG_NORMAL_INK_ABSORPTION},
//      { L"HT_FLAG_INVERT_8BPP_BITMASK_IDX",   HT_FLAG_INVERT_8BPP_BITMASK_IDX},
        {NULL, 0}                               // The NULL entry is important
    };
#else
    DBG_FLAGS gafdGDIINFO_flHTFlags[] = {
        {NULL, 0}
    };
#endif

void __stdcall
COemUniDbg::
vDumpGDIINFO(
    INT         iDebugLevel,
    PWSTR       pszInLabel,
    GDIINFO     *pGdiInfo
    )

/*++

Routine Description:

    Dumps the members of a GDIINFO structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pGdiInfo - pointer to the GDIINFO strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pGdiInfo");

    // Return if strct to be dumped is invalid
    //
    if (!pGdiInfo)
    {
        OEMDebugMessage(TEXT("\nGDIINFO [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nGDIINFO [%s]: %#x\r\n"), pszLabel, pGdiInfo);
    OEMDebugMessage(TEXT("\tulVersion: %#x\r\n"), pGdiInfo->ulVersion);

    PWSTR pszulTechnology = L"???";
    switch (pGdiInfo->ulTechnology)
    {
        case DT_PLOTTER: pszulTechnology = L"DT_PLOTTER"; break;
        case DT_RASDISPLAY: pszulTechnology = L"DT_RASDISPLAY"; break;
        case DT_RASPRINTER: pszulTechnology = L"DT_RASPRINTER"; break;
        case DT_RASCAMERA: pszulTechnology = L"DT_RASCAMERA"; break;
        case DT_CHARSTREAM: pszulTechnology = L"DT_CHARSTREAM"; break;
    }
    OEMDebugMessage(TEXT("\tulTechnology: %ld [%s]\r\n"), pGdiInfo->ulTechnology, pszulTechnology);

    OEMDebugMessage(TEXT("\tulHorzSize: %ld\r\n"), pGdiInfo->ulHorzSize);
    OEMDebugMessage(TEXT("\tulVertSize: %ld\r\n"), pGdiInfo->ulVertSize);
    OEMDebugMessage(TEXT("\tulHorzRes: %ld\r\n"), pGdiInfo->ulHorzRes);
    OEMDebugMessage(TEXT("\tulVertRes: %ld\r\n"), pGdiInfo->ulVertRes);
    OEMDebugMessage(TEXT("\tcBitsPixel: %ld\r\n"), pGdiInfo->cBitsPixel);
    OEMDebugMessage(TEXT("\tcPlanes: %ld\r\n"), pGdiInfo->cPlanes);
    OEMDebugMessage(TEXT("\tulNumColors: %ld\r\n"), pGdiInfo->ulNumColors);
    OEMDebugMessage(TEXT("\tflRaster: %#x\r\n"), pGdiInfo->flRaster);
    OEMDebugMessage(TEXT("\tulLogPixelsX: %ld\r\n"), pGdiInfo->ulLogPixelsX);
    OEMDebugMessage(TEXT("\tulLogPixelsY: %ld\r\n"), pGdiInfo->ulLogPixelsY);

    PWSTR pszTextCaps = L"Text scrolling through DrvBitBlt/DrvCopyBits";
    if (TC_SCROLLBLT == (pGdiInfo->flTextCaps & TC_SCROLLBLT))
    {
        pszTextCaps = L"Text scrolling through DrvTextOut";
    }
    OEMDebugMessage(TEXT("\tflTextCaps: %s\n"), pszTextCaps);

    OEMDebugMessage(TEXT("\tulDACRed: %#x\r\n"), pGdiInfo->ulDACRed);
    OEMDebugMessage(TEXT("\tulDACGreen: %#x\r\n"), pGdiInfo->ulDACGreen);
    OEMDebugMessage(TEXT("\tulDACBlue: %#x\r\n"), pGdiInfo->ulDACBlue);
    OEMDebugMessage(TEXT("\tulAspectX: %ld\r\n"), pGdiInfo->ulAspectX);
    OEMDebugMessage(TEXT("\tulAspectY: %ld\r\n"), pGdiInfo->ulAspectY);
    OEMDebugMessage(TEXT("\tulAspectXY: %ld\r\n"), pGdiInfo->ulAspectXY);
    OEMDebugMessage(TEXT("\txStyleStep: %ld\r\n"), pGdiInfo->xStyleStep);
    OEMDebugMessage(TEXT("\tyStyleStep: %ld\r\n"), pGdiInfo->yStyleStep);
    OEMDebugMessage(TEXT("\tdenStyleStep: %ld\r\n"), pGdiInfo->denStyleStep);
    OEMDebugMessage(TEXT("\tptlPhysOffset: (%ld, %ld)\r\n"), pGdiInfo->ptlPhysOffset.x, pGdiInfo->ptlPhysOffset.y);
    OEMDebugMessage(TEXT("\tszlPhysSize: (%ld, %ld)\r\n"), pGdiInfo->szlPhysSize.cx, pGdiInfo->szlPhysSize.cy);
    OEMDebugMessage(TEXT("\tulNumPalReg: %ld\r\n"), pGdiInfo->ulNumPalReg);
    OEMDebugMessage(TEXT("\tulDevicePelsDPI: %ld\r\n"), pGdiInfo->ulDevicePelsDPI);

    PWSTR pszPrimaryOrder = L"???";
    switch(pGdiInfo->ulPrimaryOrder)
    {
        case PRIMARY_ORDER_ABC: pszPrimaryOrder = L"PRIMARY_ORDER_ABC [RGB/CMY]"; break;
        case PRIMARY_ORDER_ACB: pszPrimaryOrder = L"PRIMARY_ORDER_ACB [RBG/CYM]"; break;
        case PRIMARY_ORDER_BAC: pszPrimaryOrder = L"PRIMARY_ORDER_BAC [GRB/MCY]"; break;
        case PRIMARY_ORDER_BCA: pszPrimaryOrder = L"PRIMARY_ORDER_BCA [GBR/MYC]"; break;
        case PRIMARY_ORDER_CBA: pszPrimaryOrder = L"PRIMARY_ORDER_CBA [BGR/YMC]"; break;
        case PRIMARY_ORDER_CAB: pszPrimaryOrder = L"PRIMARY_ORDER_CAB [BRG/YCM]"; break;
    }
    OEMDebugMessage(TEXT("\tulPrimaryOrder: %s\n"), pszPrimaryOrder);

    PWSTR pszHTPat = L"???";
    switch(pGdiInfo->ulHTPatternSize)
    {
        case HT_PATSIZE_2x2         : pszHTPat = L"HT_PATSIZE_2x2 ";            break;
        case HT_PATSIZE_2x2_M       : pszHTPat = L"HT_PATSIZE_2x2_M";           break;
        case HT_PATSIZE_4x4         : pszHTPat = L"HT_PATSIZE_4x4";         break;
        case HT_PATSIZE_4x4_M       : pszHTPat = L"HT_PATSIZE_4x4_M";           break;
        case HT_PATSIZE_6x6         : pszHTPat = L"HT_PATSIZE_6x6";         break;
        case HT_PATSIZE_6x6_M       : pszHTPat = L"HT_PATSIZE_6x6_M";           break;
        case HT_PATSIZE_8x8         : pszHTPat = L"HT_PATSIZE_8x8";         break;
        case HT_PATSIZE_8x8_M       : pszHTPat = L"HT_PATSIZE_8x8_M";           break;
        case HT_PATSIZE_10x10       : pszHTPat = L"HT_PATSIZE_10x10";           break;
        case HT_PATSIZE_10x10_M     : pszHTPat = L"HT_PATSIZE_10x10_M";     break;
        case HT_PATSIZE_12x12       : pszHTPat = L"HT_PATSIZE_12x12";           break;
        case HT_PATSIZE_12x12_M     : pszHTPat = L"HT_PATSIZE_12x12_M";     break;
        case HT_PATSIZE_14x14       : pszHTPat = L"HT_PATSIZE_14x14";           break;
        case HT_PATSIZE_14x14_M     : pszHTPat = L"HT_PATSIZE_14x14_M";     break;
        case HT_PATSIZE_16x16       : pszHTPat = L"HT_PATSIZE_16x16";           break;
        case HT_PATSIZE_16x16_M     : pszHTPat = L"HT_PATSIZE_16x16_M";     break;
        case HT_PATSIZE_SUPERCELL   : pszHTPat = L"HT_PATSIZE_SUPERCELL";       break;
        case HT_PATSIZE_SUPERCELL_M : pszHTPat = L"HT_PATSIZE_SUPERCELL_M"; break;
        case HT_PATSIZE_USER        : pszHTPat = L"HT_PATSIZE_USER";            break;
//      case HT_PATSIZE_MAX_INDEX   : pszHTPat = L"HT_PATSIZE_MAX_INDEX";       break;
//      case HT_PATSIZE_DEFAULT     : pszHTPat = L"HT_PATSIZE_DEFAULT";     break;
    }
    OEMDebugMessage(TEXT("\tulHTPatternSize: %s\n"), pszHTPat);

    PWSTR pszHTOutputFormat = L"???";
    switch(pGdiInfo->ulHTOutputFormat)
    {
        case HT_FORMAT_1BPP         : pszHTOutputFormat = L"HT_FORMAT_1BPP";        break;
        case HT_FORMAT_4BPP         : pszHTOutputFormat = L"HT_FORMAT_4BPP";        break;
        case HT_FORMAT_4BPP_IRGB    : pszHTOutputFormat = L"HT_FORMAT_4BPP_IRGB";   break;
        case HT_FORMAT_8BPP         : pszHTOutputFormat = L"HT_FORMAT_8BPP";        break;
        case HT_FORMAT_16BPP        : pszHTOutputFormat = L"HT_FORMAT_16BPP";       break;
        case HT_FORMAT_24BPP        : pszHTOutputFormat = L"HT_FORMAT_24BPP";       break;
        case HT_FORMAT_32BPP        : pszHTOutputFormat = L"HT_FORMAT_32BPP";       break;
    }
    OEMDebugMessage(TEXT("\tulHTOutputFormat: %s\n"), pszHTOutputFormat);

    OEMDebugMessage(TEXT("\tflHTFlags: "));
    vDumpFlags(pGdiInfo->flHTFlags, gafdGDIINFO_flHTFlags);
    OEMDebugMessage(TEXT("\tulVRefresh: %ld\r\n"), pGdiInfo->ulVRefresh);
    OEMDebugMessage(TEXT("\tulBltAlignment: %ld\r\n"), pGdiInfo->ulBltAlignment);
    OEMDebugMessage(TEXT("\tulPanningHorzRes: %ld\r\n"), pGdiInfo->ulPanningHorzRes);
    OEMDebugMessage(TEXT("\tulPanningVertRes: %ld\r\n"), pGdiInfo->ulPanningVertRes);
    OEMDebugMessage(TEXT("\txPanningAlignment: %ld\r\n"), pGdiInfo->xPanningAlignment);
    OEMDebugMessage(TEXT("\tyPanningAlignment: %ld\r\n"), pGdiInfo->yPanningAlignment);
    OEMDebugMessage(TEXT("\tcxHTPat: %ld\r\n"), pGdiInfo->cxHTPat);
    OEMDebugMessage(TEXT("\tcyHTPat: %ld\r\n"), pGdiInfo->cyHTPat);
    OEMDebugMessage(TEXT("\tpHTPatA: %#x\r\n"), pGdiInfo->pHTPatA);
    OEMDebugMessage(TEXT("\tpHTPatB: %#x\r\n"), pGdiInfo->pHTPatB);
    OEMDebugMessage(TEXT("\tpHTPatC: %#x\r\n"), pGdiInfo->pHTPatC);
    OEMDebugMessage(TEXT("\tflShadeBlend: %#x\r\n"), pGdiInfo->flShadeBlend);

    PWSTR pszPhysPixChars = L"???";
    switch(pGdiInfo->ulPhysicalPixelCharacteristics)
    {
        case PPC_DEFAULT: pszPhysPixChars = L"PPC_DEFAULT"; break;
        case PPC_BGR_ORDER_HORIZONTAL_STRIPES: pszPhysPixChars = L"PPC_BGR_ORDER_HORIZONTAL_STRIPES"; break;
        case PPC_BGR_ORDER_VERTICAL_STRIPES: pszPhysPixChars = L"PPC_BGR_ORDER_VERTICAL_STRIPES"; break;
        case PPC_RGB_ORDER_HORIZONTAL_STRIPES: pszPhysPixChars = L"PPC_RGB_ORDER_HORIZONTAL_STRIPES"; break;
        case PPC_RGB_ORDER_VERTICAL_STRIPES: pszPhysPixChars = L"PPC_RGB_ORDER_VERTICAL_STRIPES"; break;
        case PPC_UNDEFINED: pszPhysPixChars = L"PPC_UNDEFINED"; break;
    }
    OEMDebugMessage(TEXT("\tulPhysicalPixelCharacteristics: %s\n"), pszPhysPixChars);

    PWSTR pszPhysPixGamma = L"???";
    switch(pGdiInfo->ulPhysicalPixelGamma)
    {
        case PPG_DEFAULT: pszPhysPixGamma = L"PPG_DEFAULT"; break;
        case PPG_SRGB   : pszPhysPixGamma = L"PPG_SRGB"; break;
    }
    OEMDebugMessage(TEXT("\tulPhysicalPixelGamma: %s\n"), pszPhysPixGamma);

    OEMDebugMessage(TEXT("\n"));
}

#if DBG
    DBG_FLAGS gafdDEVINFO_flGraphicsCaps[] = {
        { L"GCAPS_BEZIERS",             GCAPS_BEZIERS},
        { L"GCAPS_GEOMETRICWIDE",       GCAPS_GEOMETRICWIDE},
        { L"GCAPS_ALTERNATEFILL",       GCAPS_ALTERNATEFILL},
        { L"GCAPS_WINDINGFILL",         GCAPS_WINDINGFILL},
        { L"GCAPS_HALFTONE",                GCAPS_HALFTONE},
        { L"GCAPS_COLOR_DITHER",        GCAPS_COLOR_DITHER},
        { L"GCAPS_HORIZSTRIKE",         GCAPS_HORIZSTRIKE},
        { L"GCAPS_VERTSTRIKE",          GCAPS_VERTSTRIKE},
        { L"GCAPS_OPAQUERECT",          GCAPS_OPAQUERECT},
        { L"GCAPS_VECTORFONT",          GCAPS_VECTORFONT},
        { L"GCAPS_MONO_DITHER",         GCAPS_MONO_DITHER},
        { L"GCAPS_ASYNCCHANGE",     GCAPS_ASYNCCHANGE},
        { L"GCAPS_ASYNCMOVE",           GCAPS_ASYNCMOVE},
        { L"GCAPS_DONTJOURNAL",         GCAPS_DONTJOURNAL},
        { L"GCAPS_DIRECTDRAW",          GCAPS_DIRECTDRAW},
        { L"GCAPS_ARBRUSHOPAQUE",       GCAPS_ARBRUSHOPAQUE},
        { L"GCAPS_PANNING",             GCAPS_PANNING},
        { L"GCAPS_HIGHRESTEXT",         GCAPS_HIGHRESTEXT},
        { L"GCAPS_PALMANAGED",          GCAPS_PALMANAGED},
        { L"GCAPS_DITHERONREALIZE",     GCAPS_DITHERONREALIZE},
        { L"GCAPS_NO64BITMEMACCESS",    GCAPS_NO64BITMEMACCESS},
        { L"GCAPS_FORCEDITHER",         GCAPS_FORCEDITHER},
        { L"GCAPS_GRAY16",              GCAPS_GRAY16},
        { L"GCAPS_ICM",                 GCAPS_ICM},
        { L"GCAPS_CMYKCOLOR",           GCAPS_CMYKCOLOR},
        { L"GCAPS_LAYERED",             GCAPS_LAYERED},
        { L"GCAPS_ARBRUSHTEXT",         GCAPS_ARBRUSHTEXT},
        { L"GCAPS_SCREENPRECISION",     GCAPS_SCREENPRECISION},
        { L"GCAPS_FONT_RASTERIZER",     GCAPS_FONT_RASTERIZER},
        { L"GCAPS_NUP",                 GCAPS_NUP},
        {NULL, 0}                       // The NULL entry is important
    };

    DBG_FLAGS gafdDEVINFO_flGraphicsCaps2[] = {
        { L"GCAPS2_JPEGSRC",                GCAPS2_JPEGSRC},
        { L"GCAPS2_xxxx",               GCAPS2_xxxx},
        { L"GCAPS2_PNGSRC",             GCAPS2_PNGSRC},
        { L"GCAPS2_CHANGEGAMMARAMP",    GCAPS2_CHANGEGAMMARAMP},
        { L"GCAPS2_ALPHACURSOR",        GCAPS2_ALPHACURSOR},
        { L"GCAPS2_SYNCFLUSH",          GCAPS2_SYNCFLUSH},
        { L"GCAPS2_SYNCTIMER",          GCAPS2_SYNCTIMER},
        { L"GCAPS2_ICD_MULTIMON",       GCAPS2_ICD_MULTIMON},
        { L"GCAPS2_MOUSETRAILS",        GCAPS2_MOUSETRAILS},
        { L"GCAPS2_RESERVED1",          GCAPS2_RESERVED1},
        {NULL, 0}                       // The NULL entry is important
    };
#else
    DBG_FLAGS gafdDEVINFO_flGraphicsCaps[] = {
        {NULL, 0}
    };

    DBG_FLAGS gafdDEVINFO_flGraphicsCaps2[] = {
        {NULL, 0}
    };
#endif

void __stdcall
COemUniDbg::
vDumpDEVINFO(
    INT         iDebugLevel,
    PWSTR       pszInLabel,
    DEVINFO     *pDevInfo
    )

/*++

Routine Description:

    Dumps the members of a DEVINFO structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pDevInfo - pointer to the DEVINFO strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pDevInfo");

    // Return if strct to be dumped is invalid
    //
    if (!pDevInfo)
    {
        OEMDebugMessage(TEXT("\nDEVINFO [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nDEVINFO [%s]: %#x\r\n"), pszLabel, pDevInfo);
    OEMDebugMessage(TEXT("\tflGraphicsCaps: "));
    vDumpFlags(pDevInfo->flGraphicsCaps, gafdDEVINFO_flGraphicsCaps);
    OEMDebugMessage(TEXT("\tcFonts: %ld\r\n"), pDevInfo->cFonts);

    PWSTR psziDitherFormat = L"0";
    switch (pDevInfo->iDitherFormat)
    {
        case BMF_1BPP : psziDitherFormat = L"BMF_1BPP" ; break;
        case BMF_4BPP : psziDitherFormat = L"BMF_4BPP" ; break;
        case BMF_8BPP : psziDitherFormat = L"BMF_8BPP" ; break;
        case BMF_16BPP: psziDitherFormat = L"BMF_16BPP"; break;
        case BMF_24BPP: psziDitherFormat = L"BMF_24BPP"; break;
        case BMF_32BPP: psziDitherFormat = L"BMF_32BPP"; break;
        case BMF_4RLE : psziDitherFormat = L"BMF_4RLE" ; break;
        case BMF_8RLE : psziDitherFormat = L"BMF_8RLE" ; break;
        case BMF_JPEG : psziDitherFormat = L"BMF_JPEG" ; break;
        case BMF_PNG  : psziDitherFormat = L"BMF_PNG " ; break;
    }
    OEMDebugMessage(TEXT("\tiDitherFormat: %s\r\n"), psziDitherFormat);

    OEMDebugMessage(TEXT("\tcxDither: %ld\r\n"), pDevInfo->cxDither);
    OEMDebugMessage(TEXT("\tcyDither: %ld\r\n"), pDevInfo->cyDither);
    OEMDebugMessage(TEXT("\thpalDefault: %#x\r\n"), pDevInfo->hpalDefault);
    OEMDebugMessage(TEXT("\tflGraphicsCaps2: "));
    vDumpFlags(pDevInfo->flGraphicsCaps2, gafdDEVINFO_flGraphicsCaps2);

    OEMDebugMessage(TEXT("\n"));
}

void __stdcall
COemUniDbg::
vDumpBitmapInfoHeader(
    INT                 iDebugLevel,
    PWSTR               pszInLabel,
    BITMAPINFOHEADER    *pBitmapInfoHeader
    )

/*++

Routine Description:

    Dumps the members of a BITMAPINFOHEADER structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pBitmapInfoHeader - pointer to the BITMAPINFOHEADER strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pBitmapInfoHeader");

    // Return if strct to be dumped is invalid
    //
    if (!pBitmapInfoHeader)
    {
        OEMDebugMessage(TEXT("\nBITMAPINFOHEADER [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nBITMAPINFOHEADER [%s]: %#x\r\n"), pszLabel, pBitmapInfoHeader);
    OEMDebugMessage(TEXT("\tbiSize: %ld\r\n"), pBitmapInfoHeader->biSize);
    OEMDebugMessage(TEXT("\tbiWidth: %ld\r\n"), pBitmapInfoHeader->biWidth);
    OEMDebugMessage(TEXT("\tbiHeight: %ld\r\n"), pBitmapInfoHeader->biHeight);
    OEMDebugMessage(TEXT("\tbiPlanes: %ld\r\n"), pBitmapInfoHeader->biPlanes);
    OEMDebugMessage(TEXT("\tbiBitCount: %ld\r\n"), pBitmapInfoHeader->biBitCount);

    PWSTR pszbiCompression = L"0";
    switch (pBitmapInfoHeader->biCompression)
    {
        case BI_RGB : pszbiCompression = L"BI_RGB" ; break;
        case BI_RLE8 : pszbiCompression = L"BI_RLE8" ; break;
        case BI_RLE4 : pszbiCompression = L"BI_RLE4" ; break;
        case BI_BITFIELDS: pszbiCompression = L"BI_BITFIELDS"; break;
        case BI_JPEG : pszbiCompression = L"BI_JPEG" ; break;
        case BI_PNG  : pszbiCompression = L"BI_PNG " ; break;
    }
    OEMDebugMessage(TEXT("\tbiCompression: %s\r\n"), pszbiCompression);
    
    OEMDebugMessage(TEXT("\tbiSizeImage: %ld\r\n"), pBitmapInfoHeader->biSizeImage);
    OEMDebugMessage(TEXT("\tbiXPelsPerMeter: %ld\r\n"), pBitmapInfoHeader->biXPelsPerMeter);
    OEMDebugMessage(TEXT("\tbiYPelsPerMeter: %ld\r\n"), pBitmapInfoHeader->biYPelsPerMeter);
    OEMDebugMessage(TEXT("\tbiClrUsed: %ld\r\n"), pBitmapInfoHeader->biClrUsed);
    OEMDebugMessage(TEXT("\tbiClrImportant: %ld\r\n"), pBitmapInfoHeader->biClrImportant);

    OEMDebugMessage(TEXT("\n"));
}

void __stdcall
COemUniDbg::
vDumpPOINTL(
    INT     iDebugLevel,
    PWSTR   pszInLabel,
    POINTL  *pptl
    )

/*++

Routine Description:

    Dumps the members of a POINTL structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pptl - pointer to the POINTL strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pptl");

    // Return if strct to be dumped is invalid
    //
    if (!pptl)
    {
        OEMDebugMessage(TEXT("\nPOINTL [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nPOINTL [%s]: %#x (%ld, %ld)\r\n"), pszLabel, pptl, pptl->x, pptl->y);

    OEMDebugMessage(TEXT("\n"));
}

void __stdcall
COemUniDbg::
vDumpRECTL(
    INT     iDebugLevel,
    PWSTR   pszInLabel,
    RECTL   *prcl
    )

/*++

Routine Description:

    Dumps the members of a RECTL structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    prcl - pointer to the RECTL strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"prcl");

    // Return if strct to be dumped is invalid
    //
    if (!prcl)
    {
        OEMDebugMessage(TEXT("\nRECTL [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nRECTL [%s]: %#x (%ld, %ld) (%ld, %ld)\r\n"), pszLabel, prcl, prcl->left, prcl->top, prcl->right, prcl->bottom);

    OEMDebugMessage(TEXT("\n"));
}

#if DBG
    DBG_FLAGS gafdXLATEOBJ_flXlate[] = {
        { L"XO_DEVICE_ICM",     XO_DEVICE_ICM},
        { L"XO_FROM_CMYK",      XO_FROM_CMYK},
        { L"XO_HOST_ICM",       XO_HOST_ICM},
        { L"XO_TABLE",          XO_TABLE},
        { L"XO_TO_MONO",        XO_TO_MONO},
        { L"XO_TRIVIAL",            XO_TRIVIAL},
        {NULL, 0}               // The NULL entry is important
    };
#else
    DBG_FLAGS gafdXLATEOBJ_flXlate[] = {
        {NULL, 0}
    };
#endif

void __stdcall
COemUniDbg::
vDumpXLATEOBJ(
    INT         iDebugLevel,
    PWSTR       pszInLabel,
    XLATEOBJ    *pxlo
    )

/*++

Routine Description:

    Dumps the members of a XLATEOBJ structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pxlo - pointer to the XLATEOBJ strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pxlo");

    // Return if strct to be dumped is invalid
    //
    if (!pxlo)
    {
        OEMDebugMessage(TEXT("\nXLATEOBJ [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nXLATEOBJ [%s]: %#x\r\n"), pszLabel, pxlo);
    OEMDebugMessage(TEXT("\tiUniq: %ld\r\n"), pxlo->iUniq);
    OEMDebugMessage(TEXT("\tflXlate: "));
    vDumpFlags(pxlo->flXlate, gafdXLATEOBJ_flXlate);
    OEMDebugMessage(TEXT("\tiSrcType: %ld [obsolete]\r\n"), pxlo->iSrcType);
    OEMDebugMessage(TEXT("\tiDstType: %ld [obsolete]\r\n"), pxlo->iDstType);
    OEMDebugMessage(TEXT("\tcEntries: %ld\r\n"), pxlo->cEntries);
    if (pxlo->pulXlate)
        OEMDebugMessage(TEXT("\tpulXlate: %#x [%ld]\r\n"), pxlo->pulXlate, *pxlo->pulXlate);
    else
        OEMDebugMessage(TEXT("\tpulXlate: %#x\r\n"), pxlo->pulXlate);

    OEMDebugMessage(TEXT("\n"));
}

#if DBG
    DBG_FLAGS gafdCOLORADJUSTMENT_caFlags[] = {
        { L"CA_NEGATIVE",       CA_NEGATIVE},
        { L"CA_LOG_FILTER",     CA_LOG_FILTER},
        {NULL, 0}               // The NULL entry is important
    };
#else
    DBG_FLAGS gafdCOLORADJUSTMENT_caFlags[] = {
        {NULL, 0}
    };
#endif

void __stdcall
COemUniDbg::
vDumpCOLORADJUSTMENT(
    INT                 iDebugLevel,
    PWSTR               pszInLabel,
    COLORADJUSTMENT *pca
    )

/*++

Routine Description:

    Dumps the members of a COLORADJUSTMENT structure.
    
Arguments:

    iDebugLevel - desired output debug level
    pszInLabel - output label string
    pca - pointer to the COLORADJUSTMENT strct to be dumped

Return Value:

    NONE

--*/

{
    // Check if the debug level is appropriate
    //
    if (iDebugLevel < giDebugLevel)
    {
        // Nothing to output here
        //
        return;
    }

    // Prepare the label string
    //
    PCWSTR pszLabel = EnsureLabel(pszInLabel, L"pca");

    // Return if strct to be dumped is invalid
    //
    if (!pca)
    {
        OEMDebugMessage(TEXT("\nCOLORADJUSTMENT [%s]: NULL\r\n"), pszLabel);

        // Nothing else to output
        //
        return;
    }
    
    // Format the data
    //
    OEMDebugMessage(TEXT("\nCOLORADJUSTMENT [%s]: %#x\r\n"), pszLabel, pca);
    OEMDebugMessage(TEXT("\tcaSize: %#x\r\n"), pca->caSize);
    OEMDebugMessage(TEXT("\tcaFlags: "));
    if (pca->caFlags)
        vDumpFlags(pca->caFlags, gafdCOLORADJUSTMENT_caFlags);
    else
        OEMDebugMessage(TEXT("NULL\r\n"));

    PWSTR pszcaIlluminantIndex = L"0";
    switch (pca->caIlluminantIndex)
    {
        case ILLUMINANT_DEVICE_DEFAULT: pszcaIlluminantIndex = L"ILLUMINANT_DEVICE_DEFAULT" ; break;
        case ILLUMINANT_A: pszcaIlluminantIndex = L"ILLUMINANT_A [Tungsten lamp]" ; break;
        case ILLUMINANT_B: pszcaIlluminantIndex = L"ILLUMINANT_B [Noon sunlight]" ; break;
        case ILLUMINANT_C: pszcaIlluminantIndex = L"ILLUMINANT_C [NTSC daylight]" ; break;
        case ILLUMINANT_D50: pszcaIlluminantIndex = L"ILLUMINANT_D50 [Normal print]" ; break;
        case ILLUMINANT_D55: pszcaIlluminantIndex = L"ILLUMINANT_D55 [Bond paper print]" ; break;
        case ILLUMINANT_D65: pszcaIlluminantIndex = L"ILLUMINANT_D65 [Standard daylight]" ; break;
        case ILLUMINANT_D75: pszcaIlluminantIndex = L"ILLUMINANT_D75 [Northern daylight]" ; break;
        case ILLUMINANT_F2: pszcaIlluminantIndex = L"ILLUMINANT_F2 [Cool white lamp]" ; break;
    }
    OEMDebugMessage(TEXT("\tcaIlluminantIndex: %s\r\n"), pszcaIlluminantIndex);

    OEMDebugMessage(TEXT("\tcaRedGamma: %d\r\n"), (int)pca->caRedGamma);
    OEMDebugMessage(TEXT("\tcaGreenGamma: %d\r\n"), (int)pca->caGreenGamma);
    OEMDebugMessage(TEXT("\tcaBlueGamma: %d\r\n"), (int)pca->caBlueGamma);
    OEMDebugMessage(TEXT("\tcaReferenceBlack: %d\r\n"), (int)pca->caReferenceBlack);
    OEMDebugMessage(TEXT("\tcaReferenceWhite: %d\r\n"), (int)pca->caReferenceWhite);
    OEMDebugMessage(TEXT("\tcaContrast: %d\r\n"), (int)pca->caContrast);
    OEMDebugMessage(TEXT("\tcaBrightness: %d\r\n"), (int)pca->caBrightness);
    OEMDebugMessage(TEXT("\tcaColorfulness: %d\r\n"), (int)pca->caColorfulness);
    OEMDebugMessage(TEXT("\tcaRedGreenTint: %d\r\n"), (int)pca->caRedGreenTint);

    OEMDebugMessage(TEXT("\n"));
}


