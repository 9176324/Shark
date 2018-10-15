//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright  1997 - 2003  Microsoft Corporation.  All Rights Reserved.
//
//  FILE:	WMarkUI.H
//    
//
//  PURPOSE:	Define common data types, and external function prototypes
//				for WaterMark UI.
//
//  PLATFORMS:
//    Windows 2000, Windows XP, Windows Server 2003
//
//
#ifndef _WMARKUI_H
#define _WMARKUI_H

#include <OEM.H>
#include <DEVMODE.H>
#include "globals.h"


////////////////////////////////////////////////////////
//      OEM UI Defines
////////////////////////////////////////////////////////


// OEM Signature and version.
#define PROP_TITLE      L"WaterMark UI Page"
#define DLLTEXT(s)      TEXT("WMARKUI:  ") TEXT(s)

// OEM UI Misc defines.
#define ERRORTEXT(s)    TEXT("ERROR ") DLLTEXT(s)


////////////////////////////////////////////////////////
//      Prototypes
////////////////////////////////////////////////////////

HRESULT hrOEMPropertyPage(DWORD dwMode, POEMCUIPPARAM pOEMUIParam);


#endif




