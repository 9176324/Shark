//===========================================================================
// CPLSVR1.H
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

#pragma pack(8)

//---------------------------------------------------------------------------

#ifndef _CPLSVR1_H
#define _CPLSVR1_H

//---------------------------------------------------------------------------
#define INC_OLE2

#ifndef PPVOID
#define PPVOID  LPVOID*
#endif

// included headers
#define STRICT
#include <windows.h>
#include <objbase.h>
#define DIRECTINPUT_VERSION 0x700
#include <dinput.h>
#include <dinputd.h>
#include <dicpl.h>
#include <assert.h>
#include "resource.h"

// symbolic constants
#define ID_POLLTIMER    50
#define NUMPAGES        4

#define STR_LEN_32		32
#define STR_LEN_64		64
#define STR_LEN_128		128

// data structures
typedef struct _CPLPAGEINFO
{
    LPWSTR  lpwszDlgTemplate;
    DLGPROC fpPageProc;
} CPLPAGEINFO;

// global variables
extern HINSTANCE            ghInst;
extern CRITICAL_SECTION     gcritsect;


// prototypes
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv);
STDAPI DllCanUnloadNow(void);
extern BOOL RegisterCustomButtonClass();
void SetButtonState(HWND hDlg, BYTE nButton, BOOL bState );

// GUIDs used by our CPL
//* CLSID_CplSvr1 {13BF4C10-E102-11D0-B3E3-0020AFD4290C}
const GUID CLSID_CplSvr1 = 
{ 0x13bf4c10, 0xe102, 0x11d0, { 0xb3, 0xe3, 0x0, 0x20, 0xaf, 0xd4, 0x29, 0xc } };
//* NULL_GUID {00000000-0000-0000-0000-000000000000}
const GUID NULL_GUID = 
{ 0x0, 0x0, 0x0, { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 } };

// derive a new class from CDIGameCntrlPropSheet
//
// we want to store some additional data here
class CDIGameCntrlPropSheet_X : public IDIGameCntrlPropSheet
{
    private:
    short				    m_cProperty_refcount;
	short				    m_nID;

    public:
    CDIGameCntrlPropSheet_X(void);
    ~CDIGameCntrlPropSheet_X(void);
    // IUnknown methods
	virtual STDMETHODIMP            QueryInterface(REFIID, PPVOID);
	virtual STDMETHODIMP_(ULONG)    AddRef(void);
	virtual STDMETHODIMP_(ULONG)    Release(void);
    // IDIGameCntrlPropSheet methods		
	virtual STDMETHODIMP			GetSheetInfo(LPDIGCSHEETINFO *ppSheetInfo);
	virtual STDMETHODIMP			GetPageInfo (LPDIGCPAGEINFO  *ppPageInfo );
	virtual STDMETHODIMP			SetID(USHORT nID);
	virtual STDMETHODIMP_(USHORT)	GetID(void)			{return m_nID;}
    virtual STDMETHODIMP            Initialize(void);
    virtual STDMETHODIMP            SetDevice(LPDIRECTINPUTDEVICE2 pdiDevice2);
    virtual STDMETHODIMP            GetDevice(LPDIRECTINPUTDEVICE2 *ppdiDevice2);
    virtual STDMETHODIMP            SetJoyConfig(LPDIRECTINPUTJOYCONFIG pdiJoyCfg);
    virtual STDMETHODIMP            GetJoyConfig(LPDIRECTINPUTJOYCONFIG *ppdiJoyCfg);

    protected:
    DIGCSHEETINFO           m_digcSheetInfo;
    DIGCPAGEINFO            *m_pdigcPageInfo;
    LPDIRECTINPUTDEVICE2    m_pdiDevice2;
    LPDIRECTINPUTJOYCONFIG  m_pdiJoyCfg;
    WCHAR                   *m_wszSheetCaption;

};

//---------------------------------------------------------------------------
#endif // _CPLSVR1_H

























