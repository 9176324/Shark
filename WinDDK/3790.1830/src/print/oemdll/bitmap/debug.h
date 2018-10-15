//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright  1996 - 2003  Microsoft Corporation.  All Rights Reserved.
//
//  FILE:   Debug.H
//    
//
//  PURPOSE:    Define common data types, and external function prototypes
//          for debugging functions. Also defines the various debug macros.
//
//  PLATFORMS:
//
//    Windows XP, Windows Server 2003
//
//
//  History: 
//          06/28/03    xxx created.
//
//

#ifndef _DEBUG_H
#define _DEBUG_H

#include "devmode.h"

// VC and Build use different debug defines.
// The following makes it so either will
// cause the inclusion of debugging code.
#if !defined(_DEBUG) && defined(DBG)
    #define _DEBUG      DBG
#elif defined(_DEBUG) && !defined(DBG)
    #define DBG         _DEBUG
#endif



/////////////////////////////////////////////////////////
//      Macros
/////////////////////////////////////////////////////////
//
// These macros are used for debugging purposes. They expand
// to white spaces on a free build. Here is a brief description
// of what they do and how they are used:
//
// giDebugLevel
// Global variable which set the current debug level to control
// the amount of debug messages emitted.
//
// VERBOSE(msg)
// Display a message if the current debug level is <= DBG_VERBOSE.
//
// TERSE(msg)
// Display a message if the current debug level is <= DBG_TERSE.
//
// WARNING(msg)
// Display a message if the current debug level is <= DBG_WARNING.
// The message format is: WRN filename (linenumber): message
//
// ERR(msg)
// Similiar to WARNING macro above - displays a message
// if the current debug level is <= DBG_ERROR.
//
// ASSERT(cond)
// Verify a condition is true. If not, force a breakpoint.
//
// ASSERTMSG(cond, msg)
// Verify a condition is true. If not, display a message and
// force a breakpoint.
//
// RIP(msg)
// Display a message and force a breakpoint.
//
// Usage:
// These macros require extra parantheses for the msg argument
// example, ASSERTMSG(x > 0, ("x is less than 0\n"));
//           WARNING(("App passed NULL pointer, ignoring...\n"));
//

#define DBG_VERBOSE     1
#define DBG_TERSE       2
#define DBG_WARNING 3
#define DBG_ERROR       4
#define DBG_RIP         5
#define DBG_NONE        6

#define DLLTEXT(s)      TEXT("BITMAP:  ") TEXT(s)
#define ERRORTEXT(s)    TEXT("ERROR ") DLLTEXT(s)

#if DBG

    #define OEMDBG(iDbgLvl, szTag)      COemUniDbg OEMDbg(iDbgLvl, szTag)

    #define DebugMsg    OEMDbg.OEMDebugMessage

    extern INT giDebugLevel;

    #define DBGMSG(level, prefix, msg) { \
        if (giDebugLevel <= (level)) { \
            DebugMsg("%s %s (%d): ", prefix, StripDirPrefixA(__FILE__), __LINE__); \
            DebugMsg(msg); \
        } \
    }

    #define VERBOSE     if(giDebugLevel <= DBG_VERBOSE) DebugMsg
    #define TERSE       if(giDebugLevel <= DBG_TERSE) DebugMsg
    #define WARNING     if(giDebugLevel <= DBG_WARNING) DebugMsg
    #define ERR         if(giDebugLevel <= DBG_ERROR) DebugMsg

    #define DBG_OEMDMPARAM(iDbgLvl, szLabel, pobj)              {OEMDbg.vDumpOemDMParam(iDbgLvl, szLabel, pobj);}
    #define DBG_OEMDEVMODE(iDbgLvl, szLabel, pobj)              {OEMDbg.vDumpOemDevMode(iDbgLvl, szLabel, pobj);}
    #define DBG_SURFOBJ(iDbgLvl, szLabel, pobj)                 {OEMDbg.vDumpSURFOBJ(iDbgLvl, szLabel, pobj);}
    #define DBG_STROBJ(iDbgLvl, szLabel, pobj)                  {OEMDbg.vDumpSTROBJ(iDbgLvl, szLabel, pobj);}
    #define DBG_FONTOBJ(iDbgLvl, szLabel, pobj)                 {OEMDbg.vDumpFONTOBJ(iDbgLvl, szLabel, pobj);}
    #define DBG_CLIPOBJ(iDbgLvl, szLabel, pobj)                 {OEMDbg.vDumpCLIPOBJ(iDbgLvl, szLabel, pobj);}
    #define DBG_BRUSHOBJ(iDbgLvl, szLabel, pobj)                    {OEMDbg.vDumpBRUSHOBJ(iDbgLvl, szLabel, pobj);}
    #define DBG_GDIINFO(iDbgLvl, szLabel, pobj)                 {OEMDbg.vDumpGDIINFO(iDbgLvl, szLabel, pobj);}
    #define DBG_DEVINFO(iDbgLvl, szLabel, pobj)                 {OEMDbg.vDumpDEVINFO(iDbgLvl, szLabel, pobj);}
    #define DBG_BMPINFO(iDbgLvl, szLabel, pobj)                 {OEMDbg.vDumpBitmapInfoHeader(iDbgLvl, szLabel, pobj);}
    #define DBG_POINTL(iDbgLvl, szLabel, pobj)                  {OEMDbg.vDumpPOINTL(iDbgLvl, szLabel, pobj);}
    #define DBG_RECTL(iDbgLvl, szLabel, pobj)                       {OEMDbg.vDumpRECTL(iDbgLvl, szLabel, pobj);}
    #define DBG_XLATEOBJ(iDbgLvl, szLabel, pobj)                    {OEMDbg.vDumpXLATEOBJ(iDbgLvl, szLabel, pobj);}
    #define DBG_COLORADJUSTMENT(iDbgLvl, szLabel, pobj)         {OEMDbg.vDumpCOLORADJUSTMENT(iDbgLvl, szLabel, pobj);}

    #define ASSERT(cond) \
    { \
        if (! (cond)) \
        { \
            RIP(("\n")); \
        } \
    }

    #define ASSERTMSG(cond, msg) \
    { \
        if (! (cond)) { \
            RIP(msg); \
        } \
    }

    #define RIP(msg) \
    { \
        DBGMSG(DBG_RIP, "RIP", msg); \
        DebugBreak(); \
    }


#else // !DBG

    #define OEMDBG(iDbgLvl, szTag)

    #define DebugMsg    NOP_FUNCTION

    #define VERBOSE     NOP_FUNCTION
    #define TERSE       NOP_FUNCTION
    #define WARNING     NOP_FUNCTION
    #define ERR         NOP_FUNCTION

    #define DBG_OEMDMPARAM(iDbgLvl, szLabel, pobj)          NOP_FUNCTION
    #define DBG_OEMDEVMODE(iDbgLvl, szLabel, pobj)          NOP_FUNCTION
    #define DBG_SURFOBJ(iDbgLvl, szLabel, pobj)             NOP_FUNCTION
    #define DBG_STROBJ(iDbgLvl, szLabel, pobj)              NOP_FUNCTION
    #define DBG_FONTOBJ(iDbgLvl, szLabel, pobj)             NOP_FUNCTION
    #define DBG_CLIPOBJ(iDbgLvl, szLabel, pobj)             NOP_FUNCTION
    #define DBG_BRUSHOBJ(iDbgLvl, szLabel, pobj)                NOP_FUNCTION
    #define DBG_GDIINFO(iDbgLvl, szLabel, pobj)             NOP_FUNCTION
    #define DBG_DEVINFO(iDbgLvl, szLabel, pobj)             NOP_FUNCTION
    #define DBG_BMPINFO(iDbgLvl, szLabel, pobj)             NOP_FUNCTION
    #define DBG_POINTL(iDbgLvl, szLabel, pobj)              NOP_FUNCTION
    #define DBG_RECTL(iDbgLvl, szLabel, pobj)                   NOP_FUNCTION
    #define DBG_XLATEOBJ(iDbgLvl, szLabel, pobj)                NOP_FUNCTION
    #define DBG_COLORADJUSTMENT(iDbgLvl, szLabel, pobj)     NOP_FUNCTION

    #define ASSERT(cond)

    #define ASSERTMSG(cond, msg)
    #define RIP(msg)

#endif

typedef struct DBG_FLAGS {
    LPWSTR pszFlag;
    DWORD dwFlag;
} *PDBG_FLAGS;

class COemUniDbg
{
public:
    COemUniDbg(INT iDebugLevel, PWSTR pszTag);
    ~COemUniDbg();

    BOOL __stdcall OEMDebugMessage(LPCWSTR, ...);
    void __stdcall vDumpOemDMParam(INT iDebugLevel, PWSTR pszInLabel, POEMDMPARAM pOemDMParam);
    void __stdcall vDumpOemDevMode(INT iDebugLevel, PWSTR pszInLabel, PCOEMDEV pOemDevmode);
    void __stdcall vDumpSURFOBJ(INT iDebugLevel, PWSTR pszInLabel, SURFOBJ *pso);
    void __stdcall vDumpSTROBJ(INT iDebugLevel, PWSTR pszInLabel, STROBJ *pstro);
    void __stdcall vDumpFONTOBJ(INT iDebugLevel, PWSTR pszInLabel, FONTOBJ *pfo);
    void __stdcall vDumpCLIPOBJ(INT iDebugLevel, PWSTR pszInLabel, CLIPOBJ *pco);
    void __stdcall vDumpBRUSHOBJ(INT iDebugLevel, PWSTR pszInLabel, BRUSHOBJ *pbo);
    void __stdcall vDumpGDIINFO(INT iDebugLevel, PWSTR pszInLabel, GDIINFO *pGdiInfo);
    void __stdcall vDumpDEVINFO(INT iDebugLevel, PWSTR pszInLabel, DEVINFO *pDevInfo);
    void __stdcall vDumpBitmapInfoHeader(INT iDebugLevel, PWSTR pszInLabel, PBITMAPINFOHEADER pBitmapInfoHeader);
    void __stdcall vDumpPOINTL(INT iDebugLevel, PWSTR pszInLabel, POINTL *pptl);
    void __stdcall vDumpRECTL(INT iDebugLevel, PWSTR pszInLabel, RECTL *prcl);
    void __stdcall vDumpXLATEOBJ(INT iDebugLevel, PWSTR pszInLabel, XLATEOBJ *pxlo);
    void __stdcall vDumpCOLORADJUSTMENT(INT iDebugLevel, PWSTR pszInLabel, COLORADJUSTMENT *pca);

private:
    BOOL __stdcall OEMRealDebugMessage(DWORD dwSize, LPCWSTR lpszMessage, va_list arglist);
    PCSTR __stdcall StripDirPrefixA(IN PCSTR    pstrFilename);
    PCWSTR __stdcall EnsureLabel(PCWSTR pszInLabel, PCWSTR pszDefLabel);
    void __stdcall vDumpFlags(DWORD dwFlags, PDBG_FLAGS pDebugFlags);
    

};

#endif






