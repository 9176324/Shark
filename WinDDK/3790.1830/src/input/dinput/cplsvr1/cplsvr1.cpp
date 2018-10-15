//===========================================================================
// CPLSVR1.CPP
//
// Simple sample "Game Controllers" control panel extension server.
//
// Functions:
//  DLLMain()
//  DllGetClassObject()
//  DllCanUnloadNow()
//  CServerClassFactory::CServerClassFactory()
//  CServerClassFactory::~CServerClassFactory()
//  CServerClassFactory::QueryInterface()
//  CServerClassFactory::AddRef()
//  CServerClassFactory::Release()
//  CServerClassFactory::CreateInstance()
//  CServerClassFactory::LockServer()
//  CDIGameCntrlPropSheet_X::CDIGameCntrlPropSheet_X()
//  CDIGameCntrlPropSheet_X::~CDIGameCntrlPropSheet_X()
//  CDIGameCntrlPropSheet_X::QueryInterface()
//  CDIGameCntrlPropSheet_X::AddRef()
//  CDIGameCntrlPropSheet_X::Release()
//  CDIGameCntrlPropSheet_X::GetSheetInfo()
//  CDIGameCntrlPropSheet_X::GetPageInfo()
//  CDIGameCntrlPropSheet_X::SetID()
//  CDIGameCntrlPropSheet_X::Initialize()
//  CDIGameCntrlPropSheet_X::SetDevice()
//  CDIGameCntrlPropSheet_X::GetDevice()
//  CDIGameCntrlPropSheet_X::SetJoyConfig()
//  CDIGameCntrlPropSheet_X::GetJoyConfig()
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

#define INITGUID
#include "cplsvr1.h"
#include "pages.h"

//---------------------------------------------------------------------------
// dll global variables

HINSTANCE           ghInst;                 // DLL instance handle
CRITICAL_SECTION    gcritsect;

// file global variables
static  LONG        glDLLRefCount  = 0;     // DLL reference count

//---------------------------------------------------------------------------
// this array contains the data unique to each specific page
//
// provide the following information for each device page
//  { dialog template, callback function pointer }
static  CPLPAGEINFO     grgcpInfo[NUMPAGES] = {
    { MAKEINTRESOURCEW(IDD_CPLSVR1_PAGE1), CPLSVR1_Page1_DlgProc},
    { MAKEINTRESOURCEW(IDD_CPLSVR1_PAGE2), CPLSVR1_Page2_DlgProc},
    { MAKEINTRESOURCEW(IDD_CPLSVR1_PAGE3), CPLSVR1_Page3_DlgProc},
    { MAKEINTRESOURCEW(IDD_CPLSVR1_PAGE4), CPLSVR1_Page4_DlgProc}};


//===========================================================================
// DLLMain
//
// DLL entry point.
//
// Parameters:
//  HINSTANCE   hInst       - the DLL's instance handle 
//  DWORD       dwReason    - reason why DLLMain was called
//  LPVOID      lpvReserved - 
//
// Returns:
//  BOOL - TRUE if succeeded
//
//===========================================================================
int APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved)
{   
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        // perform any initialization required...
        // remember our hInstance
        ghInst = hInst;
        InitializeCriticalSection(&gcritsect);
        DisableThreadLibraryCalls(hInst);
        break;

    case DLL_PROCESS_DETACH:
        // perform any cleanup required...
        DeleteCriticalSection(&gcritsect);
        break;

    } //** end switch(dwReason)

    // if we get here, we've succeeded
    return TRUE;

} //*** end DLLMain()


//===========================================================================
// DllGetClassObject
//
// Gets an IClassFactory object.
//
// Parameters:
//  REFCLSID    rclsid  - CLSID value (by reference)
//  REFIID      riid    - IID value (by reference)
//  PPVOID      ppv     - ptr to store interface ptr
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, PPVOID ppv)
{
    CServerClassFactory *pClsFactory    = NULL;
    HRESULT             hRes            = E_NOTIMPL;

    // make sure that if anything fails, we return something reasonable
    *ppv = NULL;

    // did the caller pass in our CLSID?
    if (!IsEqualCLSID(rclsid, CLSID_CplSvr1))
    {
        // no, return class not available error
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    // did the caller request our class factory?
    if (!IsEqualIID(riid, IID_IClassFactory))
    {
        // no, return no interface error
        return E_NOINTERFACE;
    }

    // instantiate class factory object
    pClsFactory = new CServerClassFactory();
    if (NULL == pClsFactory)
    {
        // could not create the object
        //
        // chances are we were out of memory
        return E_OUTOFMEMORY;

    }

    // query for interface riid, and return it via ppv
    hRes = pClsFactory->QueryInterface(riid, ppv);   

    // we're finished with our local object
    pClsFactory->Release();

    // return the result code from QueryInterface
    return hRes;

} //*** end DllGetClassObject()


//===========================================================================
// DllCanUnloadNow
//
// Reports whether or not the DLL can be unloaded.
//
// Parameters: none
//
// Returns
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDAPI DllCanUnloadNow(void)
{
    // unloading should be safe if the global dll refcount is zero
    if (0 == glDLLRefCount)
    {
        // ok to unload
        return S_OK;
    } else
    {
        // not ok to unload
        return S_FALSE;
    }

} //*** end DllCanUnloadNow()


//===========================================================================
// CServerClassFactory::CServerClassFactory
//
// Class constructor.
//
// Parameters: none
//
// Returns:
//  CServerClassFactory* (implicit)
//
//===========================================================================
CServerClassFactory::CServerClassFactory(void)
{
    // initialize and increment the object refcount
    m_ServerCFactory_refcount = 0;
    AddRef();

    // increment the dll refcount
    InterlockedIncrement(&glDLLRefCount);

} //*** end CServerClassFactory::CServerClassFactory()


//===========================================================================
// CServerClassFactory::CServerClassFactory
//
// Class constructor.
//
// Parameters: none
//
// Returns:
//  CServerClassFactory* (implicit)
//
//===========================================================================
CServerClassFactory::~CServerClassFactory(void)
{
    // decrement the dll refcount
    InterlockedDecrement(&glDLLRefCount);

} //*** end CServerClassFactory::~CServerClassFactory()


//===========================================================================
// CServerClassFactory::QueryInterface
//
// Implementation of the QueryInterface() method.
//
// Parameters:
//  REFIID  riid    - the interface that is being looked for
//  PPVOID  ppv     - pointer to target interface pointer
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CServerClassFactory::QueryInterface(REFIID riid, PPVOID ppv)
{
    // make sure that if anything fails, we return something reasonable
    *ppv = NULL;

    // we support IUnknown...
    if (IsEqualIID(riid, IID_IUnknown))
    {
        // return our object as an IUnknown
        *ppv = (LPUNKNOWN)(LPCLASSFACTORY)this;
    } else
    {
        // ... and our interface
        if (IsEqualIID(riid, IID_IClassFactory))
        {
            // return our object as a class factory
            *ppv = (LPCLASSFACTORY)this;
        } else
        {
            // we do not support any other interfaces
            return E_NOINTERFACE;
        }
    }

    // we got this far, so we've succeeded
    // increment our refcount and return
    AddRef();
    return S_OK;

} //*** end CServerClassFactory::QueryInterface()


//===========================================================================
// CServerClassFactory::AddRef
//
// Implementation of the AddRef() method.
//
// Parameters: none
//
// Returns:
//  ULONG   -   updated reference count. 
//              NOTE: apps should NOT rely on this value!
//
//===========================================================================
STDMETHODIMP_(ULONG) CServerClassFactory::AddRef(void)
{
    // update and return our object's reference count   
    InterlockedIncrement((LPLONG)&m_ServerCFactory_refcount);
    return m_ServerCFactory_refcount;


} //*** end CServerClassFactory::AddRef()


//===========================================================================
// CServerClassFactory::Release
//
// Implementation of the Release() method.
//
// Parameters: none
//
// Returns:
//  ULONG   -   updated reference count. 
//              NOTE: apps should NOT rely on this value!
//
//===========================================================================
STDMETHODIMP_(ULONG) CServerClassFactory::Release(void)
{
    // update and return our object's reference count   
    LONG cRef = InterlockedDecrement((LPLONG)&m_ServerCFactory_refcount);

    if (0 == cRef) {
        // it's now safe to call the destructor
        delete this;
    }

    return cRef;
} //*** end CServerClassFactory::Release()


//===========================================================================
// CServerClassFactory::CreateInstance
//
// Implementation of the CreateInstance() method.
//
// Parameters: none
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CServerClassFactory::CreateInstance(LPUNKNOWN pUnkOuter, REFIID riid, PPVOID ppvObj)
{
    CDIGameCntrlPropSheet_X *pdiGCPropSheet = NULL;

    // make sure that if anything fails, we return something reasonable
    *ppvObj = NULL;

    // we want pUnkOuter to be NULL
    //
    // we do not support aggregation
    if (pUnkOuter != NULL)
    {
        // tell the caller that we do not support this feature
        return CLASS_E_NOAGGREGATION;
    }

    // Create a new instance of the game controller property sheet object
    pdiGCPropSheet = new CDIGameCntrlPropSheet_X();
    if (NULL == pdiGCPropSheet)
    {
        // we could not create our object
        //
        // chances are we have run out of memory
        return E_OUTOFMEMORY;
    }

    // initialize the object (memory allocations, etc)
    HRESULT hRes = pdiGCPropSheet->Initialize();
    if (FAILED(hRes))
    {
        OutputDebugString(TEXT("Property sheet object initialization failed\n"));
        pdiGCPropSheet->Release();
        return hRes;
    }

    // query for interface riid, and return it via ppvObj
    hRes = pdiGCPropSheet->QueryInterface(riid, ppvObj);   

    // release the local object
    pdiGCPropSheet->Release();

    // all done, return result from QueryInterface
    return hRes;

} //*** end CServerClassFactory::CreateInstance()


//===========================================================================
// CServerClassFactory::LockServer
//
// Implementation of the LockServer() method.
//
// Parameters: none
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CServerClassFactory::LockServer(BOOL fLock)
{
    // increment/decrement based on fLock
    if (fLock)
    {
        // increment the dll refcount
        InterlockedIncrement(&glDLLRefCount);
    } else
    {
        // decrement the dll refcount
        InterlockedDecrement(&glDLLRefCount);
    }

    // all done
    return S_OK;

} //*** end CServerClassFactory::LockServer()


//===========================================================================
// CDIGameCntrlPropSheet_X::CDIGameCntrlPropSheet_X
//
// Class constructor.
//
// Parameters: none
//
// Returns: nothing
//
//===========================================================================
CDIGameCntrlPropSheet_X::CDIGameCntrlPropSheet_X(void)
{

    // initialize and increment the object refcount
    m_cProperty_refcount = 0;
    AddRef();

    // initialize our device id to -1 just to be safe
    m_nID = -1;

    // initialize all of our pointers
    m_pdigcPageInfo = NULL;
    m_pdiDevice2    = NULL;
    m_pdiJoyCfg     = NULL;

    // increment the dll refcount
    InterlockedIncrement(&glDLLRefCount);

    // Register the button class!
    RegisterCustomButtonClass();

} //*** end CDIGameCntrlPropSheet_X::CDIGameCntrlPropSheet_X()


//===========================================================================
// CDIGameCntrlPropSheet_X::~CDIGameCntrlPropSheet_X
//
// Class destructor.
//
// Parameters: none
//
// Returns: nothing
//
//===========================================================================
CDIGameCntrlPropSheet_X::~CDIGameCntrlPropSheet_X(void)
{
    // free the DIGCPAGEINFO memory
    if (NULL != m_pdigcPageInfo)
    {
        LocalFree(m_pdigcPageInfo);
    }
    // free the caption text memory
    if (NULL != m_wszSheetCaption)
    {
        LocalFree(m_wszSheetCaption);
    }

    // cleanup directinput objects
    // m_pdiDevice2
    if (NULL != m_pdiDevice2)
    {
        m_pdiDevice2->Unacquire();
        m_pdiDevice2->Release();
        m_pdiDevice2 = NULL;
    }
    // m_pdiJoyCfg
    if (NULL != m_pdiJoyCfg)
    {
        m_pdiJoyCfg->Unacquire();
        m_pdiJoyCfg->Release();
        m_pdiJoyCfg = NULL;
    }

    // decrement the dll refcount
    InterlockedDecrement(&glDLLRefCount);

} //*** end CDIGameCntrlPropSheet_X::~CDIGameCntrlPropSheet_X()


//===========================================================================
// CDIGameCntrlPropSheet_X::QueryInterface
//
// Implementation of the QueryInterface() method.
//
// Parameters:
//  REFIID  riid    - the interface that is being looked for
//  PPVOID  ppv     - pointer to target interface pointer
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CDIGameCntrlPropSheet_X::QueryInterface(REFIID riid, PPVOID ppv)
{
    // make sure that if anything fails, we return something reasonable
    *ppv = NULL;

    // we support IUnknown...
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPUNKNOWN)(LPCDIGAMECNTRLPROPSHEET)this;
    } else
    {
        // ... and IID_IDIGameCntrlPropSheet
        if (IsEqualIID(riid, IID_IDIGameCntrlPropSheet))
        {
            *ppv = (LPCDIGAMECNTRLPROPSHEET)this;
        } else
        {
            // we do not support any other interfaces
            return E_NOINTERFACE;
        }
    }

    // we got this far, so we've succeeded
    // increment our refcount and return
    AddRef();
    return S_OK;

} //*** end CDIGameCntrlPropSheet_X::QueryInterface()


//===========================================================================
// CDIGameCntrlPropSheet_X::AddRef
//
// Implementation of the AddRef() method.
//
// Parameters: none
//
// Returns:
//  ULONG   -   updated reference count. 
//              NOTE: apps should NOT rely on this value!
//
//===========================================================================
STDMETHODIMP_(ULONG) CDIGameCntrlPropSheet_X::AddRef(void)
{   
    // update and return our object's reference count
    InterlockedIncrement((LPLONG)&m_cProperty_refcount);
    return m_cProperty_refcount;

} //*** end CDIGameCntrlPropSheet_X::AddRef()


//===========================================================================
// CDIGameCntrlPropSheet_X::Release
//
// Implementation of the Release() method.
//
// Parameters: none
//
// Returns:
//  ULONG   -   updated reference count. 
//              NOTE: apps should NOT rely on this value!
//
//===========================================================================
STDMETHODIMP_(ULONG) CDIGameCntrlPropSheet_X::Release(void)
{
    // update and return our object's reference count
    LONG cRef = InterlockedDecrement((LPLONG)&m_cProperty_refcount);
    
    if (0 == cRef) {
        delete this;
    }

    return cRef;
} //*** end CDIGameCntrlPropSheet_X::Release()


//===========================================================================
// CDIGameCntrlPropSheet_X::GetSheetInfo
//
// Implementation of the GetSheetInfo() method.
//
// Parameters:
//  LPDIGCSHEETINFO  *ppSheetInfo  - ptr to DIGCSHEETINFO struct ptr
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CDIGameCntrlPropSheet_X::GetSheetInfo(LPDIGCSHEETINFO *ppSheetInfo)
{
    // pass back the our sheet information
    *ppSheetInfo = &m_digcSheetInfo;

    // all done here
    return S_OK;

} //*** end CDIGameCntrlPropSheet_X::GetSheetInfo()


//===========================================================================
// CDIGameCntrlPropSheet_X::GetPageInfo
//
// Implementation of the GetPageInfo() method.
//
// NOTE: This returns the information for ALL pages.  There is no mechanism
//  in place to request only page n's DIGCPAGEINFO.
//
// Parameters:
//  LPDIGCPAGEINFO  *ppPageInfo  - ptr to DIGCPAGEINFO struct ptr
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CDIGameCntrlPropSheet_X::GetPageInfo(LPDIGCPAGEINFO  *ppPageInfo)
{
    // pass back the our page information
    *ppPageInfo = m_pdigcPageInfo;

    // all done here
    return S_OK;

} //*** end CDIGameCntrlPropSheet_X::GetPageInfo()


//===========================================================================
// CDIGameCntrlPropSheet_X::SetID
//
// Implementation of the SetID() method.
//
// Parameters:
//  USHORT  nID - identifier to set
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CDIGameCntrlPropSheet_X::SetID(USHORT nID)
{

    // store the device id
    m_nID = nID;

    return S_OK;

} //*** end CDIGameCntrlPropSheet_X::SetID()


//===========================================================================
// CDIGameCntrlPropSheet::Initialize
//
// Implementation of the Initialize() method.
//
// Parameters: none
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
HRESULT CDIGameCntrlPropSheet_X::Initialize(void)
{
    // allocate memory for the DIGCPAGEINFO structures
    m_pdigcPageInfo = (DIGCPAGEINFO *)LocalAlloc(LPTR, NUMPAGES * sizeof(DIGCPAGEINFO));
    if (NULL == m_pdigcPageInfo)
    {
        // unable to allocate memory block
        OutputDebugString(TEXT("Unable to allocate m_pdigcPageInfo\n"));
        return E_OUTOFMEMORY;
    }

    // allocate memory for the sheet caption
    m_wszSheetCaption = (WCHAR *)LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));
    if (NULL == m_wszSheetCaption)
    {
        // unable to allocate memory block
        OutputDebugString(TEXT("Unable to allocate m_wszSheetCaption\n"));
        return E_OUTOFMEMORY;
    }

    // populate the DIGCPAGEINFO structure for each sheet
    for (BYTE i = 0; i < NUMPAGES; i++)
    {
        m_pdigcPageInfo[i].dwSize        = sizeof(DIGCPAGEINFO);
        m_pdigcPageInfo[i].fProcFlag     = FALSE;
        m_pdigcPageInfo[i].fpPrePostProc = NULL;
        m_pdigcPageInfo[i].fIconFlag     = TRUE;
        m_pdigcPageInfo[i].lpwszPageIcon = MAKEINTRESOURCEW(IDI_CONFIG);
        m_pdigcPageInfo[i].hInstance     = ghInst;
        m_pdigcPageInfo[i].lParam        = (LPARAM)this;
        // don't override the title provided by the dialog template
        m_pdigcPageInfo[i].lpwszPageTitle    = NULL;
        // the following data is unique to each page
        m_pdigcPageInfo[i].fpPageProc        = grgcpInfo[i].fpPageProc;
        m_pdigcPageInfo[i].lpwszTemplate     = grgcpInfo[i].lpwszDlgTemplate;
    }

    // populate the DIGCSHEETINFO structure
    m_digcSheetInfo.dwSize              = sizeof(DIGCSHEETINFO);
    m_digcSheetInfo.nNumPages           = NUMPAGES;
    m_digcSheetInfo.fSheetIconFlag      = TRUE;
    m_digcSheetInfo.lpwszSheetIcon      = L"IDI_WINFLAG"; //MAKEINTRESOURCEW(IDI_WINFLAG);

    TCHAR   tszBuf[MAX_PATH];

    // load the sheet caption
    LoadString(ghInst, IDS_SHEETCAPTION, tszBuf, MAX_PATH);
#ifndef UNICODE
    // compiled for ANSI
    //
    // need to convert the string to UNICODE
    MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, tszBuf, sizeof(tszBuf),
                        m_wszSheetCaption, MAX_PATH);
#else
    lstrcpy(m_wszSheetCaption, tszBuf);
#endif

    m_digcSheetInfo.lpwszSheetCaption   = m_wszSheetCaption;

    // all done
    return S_OK;

} //*** end CDIGameCntrlPropSheet::Initialize()


//===========================================================================
// CDIGameCntrlPropSheet_X::SetDevice
//
// Implementation of the SetDevice() method.
//
// Parameters:
//  LPDIRECTINPUTDEVICE2 pdiDevice2 - device object ptr
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CDIGameCntrlPropSheet_X::SetDevice(LPDIRECTINPUTDEVICE2 pdiDevice2)
{
    // store the device object ptr
    m_pdiDevice2 = pdiDevice2;

    return S_OK;

} //*** end CDIGameCntrlPropSheet_X::SetDevice()


//===========================================================================
// CDIGameCntrlPropSheet_X::GetDevice
//
// Implementation of the GetDevice() method.
//
// Parameters:
//  LPDIRECTINPUTDEVICE2 *ppdiDevice2   - ptr to device object ptr
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CDIGameCntrlPropSheet_X::GetDevice(LPDIRECTINPUTDEVICE2 *ppdiDevice2)
{

    // retrieve the device object ptr
    *ppdiDevice2 = m_pdiDevice2;

    return S_OK;

} //*** end CDIGameCntrlPropSheet_X::GetDevice()


//===========================================================================
// CDIGameCntrlPropSheet_X::SetJoyConfig
//
// Implementation of the SetJoyConfig() method.
//
// Parameters:
//  LPDIRECTINPUTJOYCONFIG  pdiJoyCfg - joyconfig object ptr
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CDIGameCntrlPropSheet_X::SetJoyConfig(LPDIRECTINPUTJOYCONFIG pdiJoyCfg)
{
    // store the joyconfig object ptr
    m_pdiJoyCfg = pdiJoyCfg;

    return S_OK;

} //*** end CDIGameCntrlPropSheet_X::SetJoyConfig()


//===========================================================================
// CDIGameCntrlPropSheet_X::SetJoyConfig
//
// Implementation of the SetJoyConfig() method.
//
// Parameters:
//  LPDIRECTINPUTJOYCONFIG  *ppdiJoyCfg - ptr to joyconfig object ptr
//
// Returns:
//  HRESULT - OLE type success/failure code (S_OK if succeeded)
//
//===========================================================================
STDMETHODIMP CDIGameCntrlPropSheet_X::GetJoyConfig(LPDIRECTINPUTJOYCONFIG *ppdiJoyCfg)
{
    // retrieve the joyconfig object ptr
    *ppdiJoyCfg = m_pdiJoyCfg;

    return S_OK;

} //*** end CDIGameCntrlPropSheet_X::GetJoyConfig()
