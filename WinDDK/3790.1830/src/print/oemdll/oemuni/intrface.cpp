//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//  PARTICULAR PURPOSE.
//
//  Copyright  1998 - 2003  Microsoft Corporation.  All Rights Reserved.
//
//  FILE:	Intrface.cpp
//    
//
//  PURPOSE:  Interface for User Mode COM Customization DLL.
//
//
//	Functions:
//
//		
//
//
//  PLATFORMS:	Windows 2000, Windows XP, Windows Server 2003
//
//

#include "precomp.h"
#include <INITGUID.H>
#include <PRCOMOEM.H>

#include "oemuni.h"
#include "debug.h"
#include "intrface.h"
#include "name.h"

// StrSafe.h needs to be included last
// to disallow bad string functions.
#include <STRSAFE.H>


////////////////////////////////////////////////////////
//      Internal Globals
////////////////////////////////////////////////////////

static long g_cComponents = 0;     // Count of active components
static long g_cServerLocks = 0;    // Count of locks






////////////////////////////////////////////////////////////////////////////////
//
// IOemUni body
//
IOemUni::~IOemUni()
{
    // Make sure that helper interface is released.
    if(NULL != m_pOEMHelp)
    {
        m_pOEMHelp->Release();
        m_pOEMHelp = NULL;
    }

    // If this instance of the object is being deleted, then the reference 
    // count should be zero.
    assert(0 == m_cRef);
}


HRESULT __stdcall IOemUni::QueryInterface(const IID& iid, void** ppv)
{    
    if (iid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this); 
        VERBOSE(DLLTEXT("IOemUni::QueryInterface IUnknown.\r\n")); 
    }
    else if (iid == IID_IPrintOemUni)
    {
        *ppv = static_cast<IPrintOemUni*>(this);
        VERBOSE(DLLTEXT("IOemUni::QueryInterface IPrintOemUni.\r\n")); 
    }
    else
    {
        *ppv = NULL;
#if DBG && defined(USERMODE_DRIVER)
        TCHAR szOutput[80] = {0};
        StringFromGUID2(iid, szOutput, COUNTOF(szOutput)); // can not fail!
        VERBOSE(DLLTEXT("IOemUni::QueryInterface %s not supported.\r\n"), szOutput); 
#endif
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG __stdcall IOemUni::AddRef()
{
    VERBOSE(DLLTEXT("IOemUni::AddRef() entry.\r\n"));
    return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall IOemUni::Release() 
{
   VERBOSE(DLLTEXT("IOemUni::Release() entry.\r\n"));
   ASSERT( 0 != m_cRef);
   ULONG cRef = InterlockedDecrement(&m_cRef);
   if (0 == cRef)
   {
      delete this;
        
   }
   return cRef;
}


HRESULT __stdcall IOemUni::GetInfo (
    DWORD   dwMode,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
    VERBOSE(DLLTEXT("IOemUni::GetInfo(%d) entry.\r\n"), dwMode);

    // Validate parameters.
    if( (NULL == pcbNeeded)
        ||
        ( (OEMGI_GETSIGNATURE != dwMode)
          &&
          (OEMGI_GETVERSION != dwMode)
          &&
          (OEMGI_GETPUBLISHERINFO != dwMode)
        )
      )
    {
        WARNING(DLLTEXT("IOemUni::GetInfo() exit pcbNeeded is NULL! ERROR_INVALID_PARAMETER.\r\n"));
        SetLastError(ERROR_INVALID_PARAMETER);
        return E_FAIL;
    }

    // Set expected buffer size.
    if(OEMGI_GETPUBLISHERINFO != dwMode)
    {
        *pcbNeeded = sizeof(DWORD);
    }
    else
    {
        *pcbNeeded = sizeof(PUBLISHERINFO);
        return E_FAIL;
    }

    // Check buffer size is sufficient.
    if((cbSize < *pcbNeeded) || (NULL == pBuffer))
    {
        VERBOSE(DLLTEXT("IOemUni::GetInfo() exit insufficient buffer!\r\n"));
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return E_FAIL;
    }

    switch(dwMode)
    {
        // OEM DLL Signature
        case OEMGI_GETSIGNATURE:
            *(PDWORD)pBuffer = OEM_SIGNATURE;
            break;

        // OEM DLL version
        case OEMGI_GETVERSION:
            *(PDWORD)pBuffer = OEM_VERSION;
            break;

        case OEMGI_GETPUBLISHERINFO:
            Dump((PPUBLISHERINFO)pBuffer);
            // Fall through to default case.

        // dwMode not supported.
        default:
            // Set written bytes to zero since nothing was written.
            WARNING(DLLTEXT("IOemUni::GetInfo() exit mode not supported.\r\n"));
            *pcbNeeded = 0;
            SetLastError(ERROR_NOT_SUPPORTED);
            return E_FAIL;
    }

    VERBOSE(DLLTEXT("IOemUni::GetInfo() exit S_OK, (*pBuffer is %#x).\r\n"), *(PDWORD)pBuffer);

    return S_OK;
}

HRESULT __stdcall IOemUni::PublishDriverInterface(
    IUnknown *pIUnknown)
{
    VERBOSE(DLLTEXT("IOemUni::PublishDriverInterface() entry.\r\n"));

    // Need to store pointer to Driver Helper functions, if we already haven't.
    if (this->m_pOEMHelp == NULL)
    {
        HRESULT hResult;


        // Get Interface to Helper Functions.
        hResult = pIUnknown->QueryInterface(IID_IPrintOemDriverUni, (void** ) &(this->m_pOEMHelp));

        if(!SUCCEEDED(hResult))
        {
            // Make sure that interface pointer reflects interface query failure.
            this->m_pOEMHelp = NULL;

            return E_FAIL;
        }
    }

    return S_OK;
}


HRESULT __stdcall IOemUni::EnableDriver(
    DWORD          dwDriverVersion,
    DWORD          cbSize,
    PDRVENABLEDATA pded)
{
    VERBOSE(DLLTEXT("IOemUni::EnableDriver() entry.\r\n"));

    OEMEnableDriver(dwDriverVersion, cbSize, pded);

    // Even if nothing is done, need to return S_OK so 
    // that DisableDriver() will be called, which releases
    // the reference to the Printer Driver's interface.
    // If error occurs, return E_FAIL.
    return S_OK;
}

HRESULT __stdcall IOemUni::DisableDriver(VOID)
{
    VERBOSE(DLLTEXT("IOemUni::DisaleDriver() entry.\r\n"));

    OEMDisableDriver();

    // Release reference to Printer Driver's interface.
    if (this->m_pOEMHelp)
    {
        this->m_pOEMHelp->Release();
        this->m_pOEMHelp = NULL;
    }

    return S_OK;
}

HRESULT __stdcall IOemUni::DisablePDEV(
    PDEVOBJ         pdevobj)
{
    VERBOSE(DLLTEXT("IOemUni::DisablePDEV() entry.\r\n"));

    OEMDisablePDEV(pdevobj);

    return S_OK;
};

HRESULT __stdcall IOemUni::EnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded,
    OUT PDEVOEM    *pDevOem)
{
    VERBOSE(DLLTEXT("IOemUni::EnablePDEV() entry.\r\n"));

    *pDevOem = OEMEnablePDEV(pdevobj, pPrinterName, cPatterns,  phsurfPatterns,
                             cjGdiInfo, pGdiInfo, cjDevInfo, pDevInfo, pded);

    return (NULL != *pDevOem ? S_OK : E_FAIL);
}


HRESULT __stdcall IOemUni::ResetPDEV(
    PDEVOBJ         pdevobjOld,
    PDEVOBJ        pdevobjNew)
{
    BOOL    bResult;


    VERBOSE(DLLTEXT("IOemUni::ResetPDEV() entry.\r\n"));


    bResult = OEMResetPDEV(pdevobjOld, pdevobjNew);

    return (bResult ? S_OK : E_FAIL);
}


HRESULT __stdcall IOemUni::DevMode(
    DWORD       dwMode,
    POEMDMPARAM pOemDMParam)
{   
    VERBOSE(DLLTEXT("IOemUni:DevMode(%d, %#x) entry.\n"), dwMode, pOemDMParam); 
    return hrOEMDevMode(dwMode, pOemDMParam);
}

HRESULT __stdcall IOemUni::GetImplementedMethod(PSTR pMethodName)
{
    HRESULT Result = S_FALSE;


    VERBOSE(DLLTEXT("IOemUni::GetImplementedMethod() entry.\r\n"));
    VERBOSE(DLLTEXT("        Function:%hs:"),pMethodName);

    // Unidrv only calls GetImplementedMethod for optional
    // methods.  The required methods are assumed to be
    // supported.

    // Return S_OK for supported function (i.e. implemented),
    // and S_FALSE for functions that aren't supported (i.e. not implemented).
    switch (*pMethodName)
    {
        case 'C':
            if (!strcmp(NAME_CommandCallback, pMethodName))
            {
                Result = S_OK;
            }
            else if (!strcmp(NAME_Compression, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'D':
            if (!strcmp(NAME_DownloadFontHeader, pMethodName))
            {
                Result = S_FALSE;
            }
            else if (!strcmp(NAME_DownloadCharGlyph, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'F':
            if (!strcmp(NAME_FilterGraphics, pMethodName))
            {
                Result = S_OK;
            }
            break;

        case 'H':
            if (!strcmp(NAME_HalftonePattern, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'I':
            if (!strcmp(NAME_ImageProcessing, pMethodName))
            {
                Result = S_OK;
            }
            break;

        case 'M':
            if (!strcmp(NAME_MemoryUsage, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'O':
            if (!strcmp(NAME_OutputCharStr, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'S':
            if (!strcmp(NAME_SendFontCmd, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'T':
            if (!strcmp(NAME_TextOutAsBitmap, pMethodName))
            {
                Result = S_FALSE;
            }
            else if (!strcmp(NAME_TTDownloadMethod, pMethodName))
            {
                Result = S_FALSE;
            }
            else if (!strcmp(NAME_TTYGetInfo, pMethodName))
            {
                Result = S_FALSE;
            }
            break;

        case 'W':
            if(!strcmp(NAME_WritePrinter, pMethodName))
            {
                Result = S_FALSE;
            }
            break;
    }

    VERBOSE( Result == S_OK ? TEXT("Supported\r\n") : TEXT("NOT supported\r\n"));

    return Result;
}

HRESULT __stdcall IOemUni::CommandCallback(
    PDEVOBJ     pdevobj,
    DWORD       dwCallbackID,
    DWORD       dwCount,
    PDWORD      pdwParams,
    OUT INT     *piResult)
{
    VERBOSE(DLLTEXT("IOemUni::CommandCallback() entry.\r\n"));
    VERBOSE(DLLTEXT("        dwCallbackID = %d\r\n"), dwCallbackID);
    VERBOSE(DLLTEXT("        dwCount      = %d\r\n"), dwCount);

    *piResult = 0;

    return S_OK;
}

HRESULT __stdcall IOemUni::ImageProcessing(
    PDEVOBJ             pdevobj,  
    PBYTE               pSrcBitmap,
    PBITMAPINFOHEADER   pBitmapInfoHeader,
    PBYTE               pColorTable,
    DWORD               dwCallbackID,
    PIPPARAMS           pIPParams,
    OUT PBYTE           *ppbResult)
{
    VERBOSE(DLLTEXT("IOemUni::ImageProcessing() entry.\r\n"));

    return S_OK;
}

HRESULT __stdcall IOemUni::FilterGraphics(
    PDEVOBJ     pdevobj,
    PBYTE       pBuf,
    DWORD       dwLen)
{
    DWORD dwResult;
    VERBOSE(DLLTEXT("IOemUni::FilterGraphis() entry.\r\n"));
    m_pOEMHelp->DrvWriteSpoolBuf(pdevobj, pBuf, dwLen, &dwResult);
    
    if (dwResult == dwLen)
        return S_OK;
    else
        return E_FAIL;
}

HRESULT __stdcall IOemUni::Compression(
    PDEVOBJ     pdevobj,
    PBYTE       pInBuf,
    PBYTE       pOutBuf,
    DWORD       dwInLen,
    DWORD       dwOutLen,
    OUT INT     *piResult)
{
    VERBOSE(DLLTEXT("IOemUni::Compression() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}


HRESULT __stdcall IOemUni::HalftonePattern(
    PDEVOBJ     pdevobj,
    PBYTE       pHTPattern,
    DWORD       dwHTPatternX,
    DWORD       dwHTPatternY,
    DWORD       dwHTNumPatterns,
    DWORD       dwCallbackID,
    PBYTE       pResource,
    DWORD       dwResourceSize)
{
    VERBOSE(DLLTEXT("IOemUni::HalftonePattern() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::MemoryUsage(
    PDEVOBJ         pdevobj,   
    POEMMEMORYUSAGE pMemoryUsage)
{
    VERBOSE(DLLTEXT("IOemUni::MemoryUsage() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::DownloadFontHeader(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult) 
{
    VERBOSE(DLLTEXT("IOemUni::DownloadFontHeader() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::DownloadCharGlyph(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    HGLYPH      hGlyph,
    PDWORD      pdwWidth,
    OUT DWORD   *pdwResult) 
{
    VERBOSE(DLLTEXT("IOemUni::DownloadCharGlyph() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::TTDownloadMethod(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    OUT DWORD   *pdwResult) 
{
    VERBOSE(DLLTEXT("IOemUni::TTDownloadMethod() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::OutputCharStr(
    PDEVOBJ     pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD       dwType,
    DWORD       dwCount,
    PVOID       pGlyph) 
{
    VERBOSE(DLLTEXT("IOemUni::OutputCharStr() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::SendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv) 
{
    VERBOSE(DLLTEXT("IOemUni::SendFontCmd() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::DriverDMS(
    PVOID   pDevObj,
    PVOID   pBuffer,
    DWORD   cbSize,
    PDWORD  pcbNeeded)
{
    VERBOSE(DLLTEXT("IOemUni::DriverDMS() entry.\r\n"));

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::TextOutAsBitmap(
    SURFOBJ    *pso,
    STROBJ     *NAME_o,
    FONTOBJ    *pfo,
    CLIPOBJ    *pco,
    RECTL      *prclExtra,
    RECTL      *prclOpaque,
    BRUSHOBJ   *pboFore,
    BRUSHOBJ   *pboOpaque,
    POINTL     *pptlOrg,
    MIX         mix)
{
    VERBOSE(DLLTEXT("IOemUni::TextOutAsBitmap() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}

HRESULT __stdcall IOemUni::TTYGetInfo(
    PDEVOBJ     pdevobj,
    DWORD       dwInfoIndex,
    PVOID       pOutputBuf,
    DWORD       dwSize,
    DWORD       *pcbcNeeded)
{
    VERBOSE(DLLTEXT("IOemUni::TTYGetInfo() entry.\r\n"));

    // When implemented, the return from GetImplementedMethod
    // for this method name must be S_OK.

    return E_NOTIMPL;
}


////////////////////////////////////////////////////////////////////////////////
//
// oem class factory
//
class IOemCF : public IClassFactory
{
public:
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)  (THIS);
    STDMETHOD_(ULONG,Release) (THIS);
   
    // *** IClassFactory methods ***
    STDMETHOD(CreateInstance) (THIS_
                               LPUNKNOWN pUnkOuter,
                               REFIID riid,
                               LPVOID FAR* ppvObject);
    STDMETHOD(LockServer)     (THIS_ BOOL bLock);


    // Constructor
    IOemCF(): m_cRef(1) { };
    ~IOemCF() { };

protected:
    LONG m_cRef;

};

///////////////////////////////////////////////////////////
//
// Class factory body
//
HRESULT __stdcall IOemCF::QueryInterface(const IID& iid, void** ppv)
{    
    if ((iid == IID_IUnknown) || (iid == IID_IClassFactory))
    {
        *ppv = static_cast<IOemCF*>(this); 
    }
    else
    {
        *ppv = NULL;
#if DBG && defined(USERMODE_DRIVER)
        TCHAR szOutput[80] = {0};
        StringFromGUID2(iid, szOutput, COUNTOF(szOutput)); // can not fail!
        VERBOSE(DLLTEXT("IOemCF::QueryInterface %s not supported.\r\n"), szOutput); 
#endif
        return E_NOINTERFACE;
    }
    reinterpret_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG __stdcall IOemCF::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG __stdcall IOemCF::Release() 
{
   ASSERT( 0 != m_cRef);
   ULONG cRef = InterlockedDecrement(&m_cRef);
   if (0 == cRef)
   {
      delete this;
        
   }
   return cRef;
}

// IClassFactory implementation
HRESULT __stdcall IOemCF::CreateInstance(IUnknown* pUnknownOuter,
                                           const IID& iid,
                                           void** ppv) 
{
    //VERBOSE(DLLTEXT("Class factory:\t\tCreate component."));

    // Cannot aggregate.
    if (pUnknownOuter != NULL)
    {
        return CLASS_E_NOAGGREGATION;
    }

    // Create component.
    IOemUni* pOemCP = new IOemUni;
    if (pOemCP == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Get the requested interface.
    HRESULT hr = pOemCP->QueryInterface(iid, ppv);

    // Release the IUnknown pointer.
    // (If QueryInterface failed, component will delete itself.)
    pOemCP->Release();
    return hr;
}

// LockServer
HRESULT __stdcall IOemCF::LockServer(BOOL bLock) 
{
    if (bLock)
    {
        InterlockedIncrement(&g_cServerLocks); 
    }
    else
    {
        InterlockedDecrement(&g_cServerLocks);
    }
    return S_OK;
}


//
// Registration functions
//

//
// Can DLL unload now?
//
STDAPI DllCanUnloadNow()
{
    //
    // To avoid leaving OEM DLL still in memory when Unidrv or Pscript drivers 
    // are unloaded, Unidrv and Pscript driver ignore the return value of 
    // DllCanUnloadNow of the OEM DLL, and always call FreeLibrary on the OEMDLL.
    //
    // If OEM DLL spins off a working thread that also uses the OEM DLL, the 
    // thread needs to call LoadLibrary and FreeLibraryAndExitThread, otherwise 
    // it may crash after Unidrv or Pscript calls FreeLibrary.
    //

    VERBOSE(DLLTEXT("DllCanUnloadNow entered.\r\n"));

    if ((g_cComponents == 0) && (g_cServerLocks == 0))
    {
        return S_OK;
    }
    else
    {
        return S_FALSE;
    }
}

//
// Get class factory
//
STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv)
{
    VERBOSE(DLLTEXT("DllGetClassObject:\tCreate class factory.\r\n"));

    // Can we create this component?
    if (clsid != CLSID_OEMRENDER)
    {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    // Create class factory.
    IOemCF* pFontCF = new IOemCF;  // Reference count set to 1
                                         // in constructor
    if (pFontCF == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Get requested interface.
    HRESULT hr = pFontCF->QueryInterface(iid, ppv);
    pFontCF->Release();

    return hr;
}

