//===========================================================================
// DICPUTIL.H
//
//===========================================================================

//===========================================================================
// (C) Copyright 1998 Microsoft Corp.  All rights reserved.
//
// You have a royalty-free right to use, modify, reproduce and
// distribute the Sample Files (and/or any modified version) in
// any way you find useful, provided that you agree that
// Microsoft has no warranty obligations or liability for any
// Sample Application Files which are modified.
//===========================================================================

#ifndef _DICPUTIL_H
#define _DICPUTIL_H

//---------------------------------------------------------------------------

// prototypes
HRESULT DIUtilGetJoystickTypeName(short nJoystickId, 
                                    LPDIRECTINPUTJOYCONFIG pdiJoyCfg, 
                                    LPTSTR tszTypeName, int nBufSize);
HRESULT DIUtilGetJoystickDisplayName(short nJoystickId, 
                                    LPDIRECTINPUTJOYCONFIG pdiJoyCfg, 
                                    LPTSTR tszDispName, int nBufSize);
HRESULT DIUtilGetJoystickConfigCLSID(short nJoystickId, 
                                    LPDIRECTINPUTJOYCONFIG pdiJoyCfg,
                                    CLSID* pclsid);
HRESULT DIUtilGetJoystickCallout(short nJoystickId, 
                                    LPDIRECTINPUTJOYCONFIG pdiJoyCfg,
                                    LPTSTR tszCallout, int nBufSize);
HRESULT DIUtilPollJoystick(LPDIRECTINPUTDEVICE2 pdiDevice2, DIJOYSTATE *pdijs);


// helper functions
HRESULT DIUtilCreateDevice2FromJoyConfig(short nJoystickId, HWND hWnd,
                            LPDIRECTINPUT pdi,
                            LPDIRECTINPUTJOYCONFIG pdiJoyCfg,
                            LPDIRECTINPUTDEVICE2 *ppdiDevice2);


//---------------------------------------------------------------------------
#endif _DICPUTIL_H





















