////////////////////////////////////
// (C) COPYRIGHT MICROSOFT CORP., 1998-1999
//
// FILE: EXTEND.CPP
//
// DESCRIPTION: Implements core DLL routines as well as web view extensions.
//
#include "precomp.h"
#pragma hdrstop
#include <string.h>
#include <tchar.h>
#include "resource.h"

#include "extidl_i.c"

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_TestShellExt,  CShellExt)
OBJECT_ENTRY(CLSID_TestUIExtension,  CWiaUIExtension)
END_OBJECT_MAP()


STDAPI DllRegisterServer(void)
{

    // registers object, typelib and all interfaces in typelib
    return _Module.RegisterServer(TRUE);
}


STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer();
}


EXTERN_C
BOOL
DllMain(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:

            _Module.Init (ObjectMap, hinst);
            DisableThreadLibraryCalls(hinst);

            break;

        case DLL_PROCESS_DETACH:
            _Module.Term();
            break;
    }
    return TRUE;
}


extern "C" STDMETHODIMP DllCanUnloadNow(void)
{
    return _Module.GetLockCount()==0 ? S_OK : S_FALSE;
}

extern "C" STDAPI DllGetClassObject(
    REFCLSID    rclsid,
    REFIID      riid,
    LPVOID      *ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);

}


/*****************************************************************************

ShowMessage

Utility function for displaying messageboxes

******************************************************************************/

BOOL ShowMessage (HWND hParent, INT idCaption, INT idMessage)
{
    MSGBOXPARAMS mbp= {0};
    BOOL bRet;
    INT  i;

    mbp.cbSize = sizeof(mbp);
    mbp.hwndOwner = hParent;
    mbp.hInstance = g_hInst;
    mbp.lpszText = MAKEINTRESOURCE(idMessage);
    mbp.lpszCaption = MAKEINTRESOURCE(idCaption);
    mbp.dwStyle = MB_OK | MB_APPLMODAL;

    i = MessageBoxIndirect (&mbp);
    bRet = (IDOK==i);
    return bRet;
}

/*****************************************************************************

CreateDeviceFromID

Utility for attaching to WIA and getting a root IWiaItem interface

*****************************************************************************/
HRESULT
CreateDeviceFromId (LPWSTR szDeviceId, IWiaItem **ppItem)
{
    IWiaDevMgr *pDevMgr;
    HRESULT hr = CoCreateInstance (CLSID_WiaDevMgr,
                                   NULL,
                                   CLSCTX_LOCAL_SERVER,
                                   IID_IWiaDevMgr,
                                   reinterpret_cast<LPVOID*>(&pDevMgr));
    if (SUCCEEDED(hr))
    {
        BSTR strId = SysAllocString (szDeviceId);
        if (strId)
        {
            hr = pDevMgr->CreateDevice (strId, ppItem);
            SysFreeString (strId);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
        pDevMgr->Release ();
    }
    return hr;
}

/*****************************************************************************\

    GetNamesFromDataObject

    Return the list of selected item identifiers. Each identifier is of the form
    "<DEVICEID>::<FULL PATH NAME>". the list is double-null terminated

*****************************************************************************/

LPWSTR
GetNamesFromDataObject (IDataObject *lpdobj, UINT *puItems)
{
    FORMATETC fmt;
    STGMEDIUM stg;
    LPWSTR szRet = NULL;
    LPWSTR szCurrent;
    UINT nItems;
    size_t size;
    if (puItems)
    {
        *puItems = 0;
    }

    fmt.cfFormat = (CLIPFORMAT) RegisterClipboardFormat (TEXT("WIAItemNames"));
    fmt.dwAspect = DVASPECT_CONTENT;
    fmt.lindex = -1;
    fmt.ptd = NULL;
    fmt.tymed = TYMED_HGLOBAL;

    if (lpdobj && puItems && SUCCEEDED(lpdobj->GetData (&fmt, &stg)))
    {
        szCurrent = reinterpret_cast<LPWSTR>(GlobalLock (stg.hGlobal));

        // count the number of items in the double-null terminated string
        szRet  = szCurrent;
        nItems = 0;
        while (*szRet)
        {
            nItems++;
            while (*szRet)
            {
                szRet++;
            }
            szRet++;
        }        
        size = (szRet-szCurrent+1)*sizeof(WCHAR);
        szRet = new WCHAR[size];
        if (szRet)
        {
            CopyMemory (szRet, szCurrent, size);
            *puItems = nItems;
        }
        GlobalUnlock (stg.hGlobal);
        GlobalFree (stg.hGlobal);
    }
    return szRet;
}

CWiaUIExtension::CWiaUIExtension(void)
{
}

CWiaUIExtension::~CWiaUIExtension(void)
{
}

//
// IWiaUIExtension
//
STDMETHODIMP CWiaUIExtension::DeviceDialog( PDEVICEDIALOGDATA pDeviceDialogData )
{
    //
    // We are not going to implement an actual device dialog here.  Just say "hi" and return.
    //
    MessageBox( NULL, TEXT("CWiaUIExtension::DeviceDialog is being called"), TEXT("DEBUG"), 0 );
    return E_NOTIMPL;
}

STDMETHODIMP CWiaUIExtension::GetDeviceIcon( BSTR bstrDeviceId, HICON *phIcon, ULONG nSize )
{
    //
    // Load an icon, and copy it, using CopyIcon, so it will still be valid if our interface is freed
    //
    HICON hIcon = reinterpret_cast<HICON>(LoadImage( _Module.m_hInst, MAKEINTRESOURCE(IDI_TESTDEVICE), IMAGE_ICON, nSize, nSize, LR_DEFAULTCOLOR ));
    if (hIcon)
    {
        *phIcon = CopyIcon(hIcon);
        DestroyIcon(hIcon);
        return S_OK;
    }
    return E_NOTIMPL;
}

STDMETHODIMP CWiaUIExtension::GetDeviceBitmapLogo( BSTR bstrDeviceId, HBITMAP *phBitmap, ULONG nMaxWidth, ULONG nMaxHeight )
{
    //
    // This method is never actually called currently.  It is only here for future use.
    //
    MessageBox( NULL, TEXT("CWiaUIExtension::GetDeviceBitmapLogo is being called"), TEXT("DEBUG"), 0 );
    return E_NOTIMPL;
}


