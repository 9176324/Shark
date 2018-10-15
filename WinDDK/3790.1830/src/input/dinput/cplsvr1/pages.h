//===========================================================================
// PAGES.H
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

#ifndef _PAGES_H
#define _PAGES_H

//---------------------------------------------------------------------------

// symbolic constants
#define MAX_OBJECTS 1024

// structures
typedef struct _EFFECTLIST
{
    int     nCount;
    GUID    rgGuid[MAX_OBJECTS];
    DWORD   rgdwEffType[MAX_OBJECTS];
    DWORD   rgdwStaticParams[MAX_OBJECTS];
    DWORD   rgdwDynamicParams[MAX_OBJECTS];
    TCHAR   rgtszName[MAX_OBJECTS][MAX_PATH];
} EFFECTLIST;

typedef struct _OBJECTINFO
{
    int     nCount;
    GUID    rgGuidType[MAX_OBJECTS];
    DWORD   rgdwOfs[MAX_OBJECTS];
    DWORD   rgdwType[MAX_OBJECTS];
    TCHAR   rgtszName[MAX_OBJECTS][MAX_PATH];

} OBJECTINFO;

// prototypes
// dialog callback functions
INT_PTR CALLBACK CPLSVR1_Page1_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam);
INT_PTR CALLBACK CPLSVR1_Page2_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam);
INT_PTR CALLBACK CPLSVR1_Page3_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam);
INT_PTR CALLBACK CPLSVR1_Page4_DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                    LPARAM lParam);
// helper functions
HRESULT InitDInput(HWND hWnd, CDIGameCntrlPropSheet_X *pdiCpl);
BOOL CALLBACK EnumDeviceObjectsProc(LPCDIDEVICEOBJECTINSTANCE pdidoi, 
                                    LPVOID pv);
// info display functions
HRESULT DisplayAxisList(HWND hWnd, UINT uTextCtrlId, UINT uListCtrlId,
                        LPDIRECTINPUTDEVICE2 pdiDevice2);
HRESULT DisplayButtonList(HWND hWnd, UINT uTextCtrlId, UINT uListCtrlId,
                          LPDIRECTINPUTDEVICE2 pdiDevice2);
HRESULT DisplayPOVList(HWND hWnd, UINT uTextCtrlId, UINT uListCtrlId,
                        LPDIRECTINPUTDEVICE2 pdiDevice2);
HRESULT DisplayEffectList(HWND hWnd, UINT uListCtrlId,
                          LPDIRECTINPUTDEVICE2 pdiDevice2);
HRESULT DisplayJoystickState(HWND hWnd, DIJOYSTATE *pdijs);


//---------------------------------------------------------------------------
#endif // _PAGES_H









