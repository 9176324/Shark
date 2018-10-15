///////////////////
// (C) COPYRIGHT MICROSOFT CORP., 1998-2002
//
// FILE: SHELLEXT.CPP
//
// DESCRIPTION: Implements IContextMenu and IShellPropSheetExt interfaces for
// the WIA test camera device
//
#include "precomp.h"
#pragma hdrstop

// Define a language-independent name for our menu verb
static const CHAR  g_PathVerbA[] = "ChangeRoot";
static const WCHAR g_PathVerbW[] = L"ChangeRoot";

#if !defined(ARRAYSIZE)
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#endif


/*****************************************************************************

CShellExt::CShellExt



******************************************************************************/

CShellExt :: CShellExt ()
{

}

/*****************************************************************************

CShellExt::~CShellExt



******************************************************************************/

CShellExt::~CShellExt ()
{



}


/*****************************************************************************

CShellExt::Initialize

Called by the shell when the user invokes context menu or property sheet for
one of our items. For context menus the dataobject may include more than one
selected item.

******************************************************************************/

STDMETHODIMP CShellExt::Initialize (LPCITEMIDLIST pidlFolder,
                                    LPDATAOBJECT lpdobj,
                                    HKEY hkeyProgID)
{

    LONG lType = 0;
    HRESULT hr = NOERROR;
    if (!lpdobj)
    {
        return E_INVALIDARG;
    }


    // For singular selections, the WIA namespace should always provide a
    // dataobject that also supports IWiaItem

    if (FAILED(lpdobj->QueryInterface (IID_IWiaItem, reinterpret_cast<LPVOID*>(&m_pItem))))
    {
        // failing that, get the list of selected items from the data object
        UINT uItems = 0;
        LPWSTR szName;
        LPWSTR szToken;
        IWiaItem *pDevice;

        szName = GetNamesFromDataObject (lpdobj, &uItems);
        // we only support singular objects
        if (uItems != 1)
        {
            hr = E_FAIL;
        }
        else
        {
            // The name is of this format: <device id>::<item name>
            szToken = wcstok (szName, L":");
            if (!szToken)
            {
                hr = E_FAIL;
            }
            // Our extension only supports root items, so make sure there's no item
            // name
            else if (wcstok (NULL, L":"))
            {
                hr = E_FAIL;
            }
            else
            {
                hr = CreateDeviceFromId (szToken, &m_pItem);
            }
        }
        if (szName)
        {
            delete [] szName;
        }
    }
    if (SUCCEEDED(hr))
    {

        m_pItem->GetItemType (&lType);
        if (!(lType & WiaItemTypeRoot))
        {
            hr = E_FAIL; // we only support changing the property on the root item
        }
    }
    return hr;
}

/*****************************************************************************

CShellExt::AddPages

Called by the shell to get our property pages.

******************************************************************************/

STDMETHODIMP CShellExt::AddPages (LPFNADDPROPSHEETPAGE lpfnAddPage,LPARAM lParam)
{
    HPROPSHEETPAGE hpsp;
    PROPSHEETPAGE psp;

    HRESULT hr = E_FAIL;
    // We only have 1 page.
    psp.dwSize = sizeof(psp);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = g_hInst;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_DEVICE_PROPPAGE);
    psp.pfnDlgProc = PropPageProc;
    psp.lParam = reinterpret_cast<LPARAM>(this);

    hpsp = CreatePropertySheetPage (&psp);
    if (hpsp)
    {
        hr = (*lpfnAddPage) (hpsp, lParam) ? S_OK:E_FAIL;
        if (SUCCEEDED(hr))
        {
            InternalAddRef (); // the propsheetpage will release us when it is destroyed
        }
    }
    return E_FAIL;
}

/*****************************************************************************

CShellExt::QueryContextMenu

Called by the shell to get our context menu strings for the selected item.

******************************************************************************/

STDMETHODIMP CShellExt::QueryContextMenu (HMENU hmenu,UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags)
{
    MENUITEMINFO mii = {0};
    TCHAR szPathRoot[MAX_PATH];
    // insert our only menu item at index indexMenu. Remember the cmd value.

    LoadString (g_hInst, ID_CHANGEROOT, szPathRoot, MAX_PATH);
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING | MIIM_ID;
    mii.fState = MFS_ENABLED;
    mii.wID = idCmdFirst;
    mii.dwTypeData = szPathRoot;
    m_idCmd = 0; // we only have 1 item.
    if (InsertMenuItem (hmenu, indexMenu, TRUE, &mii))
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, 1);
    }
    return E_FAIL;
}

/*****************************************************************************

CShellExt::InvokeCommand

Called by the shell when the user clicks one of our menu items

******************************************************************************/

STDMETHODIMP CShellExt::InvokeCommand    (LPCMINVOKECOMMANDINFO lpici)
{
    UINT_PTR idCmd = reinterpret_cast<UINT_PTR>(lpici->lpVerb);
    if (idCmd == m_idCmd)
    {
        return GetNewRootPath (lpici->hwnd);
    }
    // it wasn't our verb.
    return E_FAIL;
}

/*****************************************************************************

CShellExt::GetCommandString

Called by the shell to get our language independent verb name.

******************************************************************************/

STDMETHODIMP CShellExt::GetCommandString (UINT_PTR idCmd, UINT uType,UINT* pwReserved,LPSTR pszName,UINT cchMax)
{

    if (idCmd != m_idCmd)
    {
        // not our verb
        return E_FAIL;
    }

    switch (uType)
    {
        case GCS_VALIDATEA:

            if (pszName)
            {
                lstrcpynA (pszName, g_PathVerbA, cchMax);
            }
            return S_OK;
        case GCS_VALIDATEW:

            if (pszName)
            {
                lstrcpynW (reinterpret_cast<LPWSTR>(pszName), g_PathVerbW, cchMax);
            }
            return S_OK;

        case GCS_VERBA:

            lstrcpynA (pszName, g_PathVerbA, cchMax);
            break;

        case GCS_VERBW:

            lstrcpynW (reinterpret_cast<LPWSTR>(pszName), g_PathVerbW, cchMax);
            break;

        default:
            return E_FAIL;
    }

    return NOERROR;
}


/*****************************************************************************

GetSetRootPath

Utility function for setting or retrieving the test camera's root path property

******************************************************************************/

HRESULT GetSetRootPath (IWiaItem *pCamera, LPTSTR pszPath, int cchMax, BOOL bSet)
{
    IWiaPropertyStorage *pps;
    HRESULT hr;
    PROPVARIANT pv = {0};
    PROPSPEC ps;

    ps.ulKind = PRSPEC_PROPID;
    ps.propid = WIA_DPP_TCAM_ROOT_PATH;


    hr = pCamera->QueryInterface (IID_IWiaPropertyStorage, reinterpret_cast<LPVOID*>(&pps));
    if (SUCCEEDED(hr))
    {
        if (!bSet) // retrieval
        {
            *pszPath = TEXT('\0');
            hr = pps->ReadMultiple (1, &ps, &pv);
            if (SUCCEEDED(hr))
            {
                #ifdef UNICODE
                lstrcpyn (pszPath, pv.bstrVal, cchMax);
                #else
                WideCharToMultiByte (CP_ACP, 0,
                                     pv.bstrVal, -1,
                                     pszPath, cchMax,
                                     NULL, NULL);
                #endif //UNICODE
                PropVariantClear(&pv);
            }
        }
        else // assignment
        {
            pv.vt = VT_BSTR;
            #ifdef UNICODE
            pv.bstrVal =SysAllocString ( pszPath);
            #else
            INT len = lstrlen(pszPath);
            LPWSTR pwszVal = new WCHAR[len+1];
            if (pwszVal)
            {
                MultiByteToWideChar (CP_ACP, 0, pszPath, -1, pwszVal, len+1);
                pv.bstrVal =SysAllocString ( pwszVal);
                delete [] pwszVal;
            }
            #endif // UNICODE
            if (!pv.bstrVal)
            {
                hr = E_OUTOFMEMORY;
            }
            
            if (SUCCEEDED(hr))
            {
                hr = pps->WriteMultiple (1, &ps, &pv, 2);               
            }
            PropVariantClear(&pv);
        }
    }
    if (pps)
    {
        pps->Release ();
    }
    return hr;
}

/*****************************************************************************

CShellExt::PropPageProc

Dialog procedure for the simple property sheet page.

******************************************************************************/

INT_PTR CALLBACK CShellExt::PropPageProc (HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    CShellExt *pThis;

    pThis = reinterpret_cast<CShellExt *>(GetWindowLongPtr (hwnd, DWLP_USER));
    TCHAR szPath[MAX_PATH] ;
    INT_PTR iRet = TRUE;
    switch (msg)
    {
        case WM_INITDIALOG:
            pThis = reinterpret_cast<CShellExt *>(reinterpret_cast<LPPROPSHEETPAGE>(lp)->lParam);
            SetWindowLongPtr (hwnd, DWLP_USER, reinterpret_cast<LONG_PTR>(pThis));
            GetSetRootPath (pThis->m_pItem, szPath, ARRAYSIZE(szPath), FALSE);
            SetDlgItemText (hwnd, IDC_IMAGEROOT, szPath);
            break;

        case WM_COMMAND:
            if (LOWORD(wp) == IDC_IMAGEROOT && HIWORD(wp) == EN_CHANGE)
            {
                SendMessage(GetParent(hwnd),
                            PSM_CHANGED,
                            reinterpret_cast<WPARAM>(hwnd), 0);
                return TRUE;
            }
            iRet = FALSE;
            break;
        case WM_NOTIFY:
        {
            LONG lCode = PSNRET_NOERROR;
            LPPSHNOTIFY psn = reinterpret_cast<LPPSHNOTIFY>(lp);
            switch (psn->hdr.code)
            {
                case PSN_APPLY:
                    // Set the new root path property
                    GetDlgItemText (hwnd, IDC_IMAGEROOT, szPath, MAX_PATH);
                    if (FAILED(GetSetRootPath (pThis->m_pItem, szPath, ARRAYSIZE(szPath), TRUE)))
                    {
                        ::ShowMessage (hwnd, IDS_ERRCAPTION, IDS_ERRPROPSET);
                        lCode = PSNRET_INVALID_NOCHANGEPAGE;
                    }
                    break;

                case PSN_KILLACTIVE:
                    // Validate the new path is valid
                    GetDlgItemText (hwnd, IDC_IMAGEROOT, szPath, MAX_PATH);
                    if (! (GetFileAttributes(szPath) & FILE_ATTRIBUTE_DIRECTORY))
                    {
                        ::ShowMessage (hwnd, IDS_ERRCAPTION, IDS_BADPATH);
                        lCode = TRUE;
                    }
                    break;

                default:
                    iRet = FALSE;
                    break;
            }
            SetWindowLongPtr (hwnd, DWLP_MSGRESULT, lCode);
        }
            break;
        case WM_CLOSE:
            pThis->InternalRelease ();
            break;

        default:
            iRet = FALSE;
            break;

    }
    return iRet;
}

/*****************************************************************************
BrowseCallback

Sets the default selection in the folder selection dialog to the current
camera image root path

******************************************************************************/

INT CALLBACK BrowseCallback (HWND hwnd, UINT msg, LPARAM lp, WPARAM wp)
{
    IWiaItem *pItem = reinterpret_cast<IWiaItem*>(lp);
    if (BFFM_INITIALIZED == msg)
    {

        TCHAR szPath[MAX_PATH];

        // find the current root path
        GetSetRootPath (pItem, szPath, ARRAYSIZE(szPath), FALSE);
        // send the appropriate message to the dialog
        SendMessage (hwnd,
                     BFFM_SETSELECTION,
                     reinterpret_cast<LPARAM>(szPath),
                     TRUE);


    }
    return 0;
}


/*****************************************************************************

CShellExt::GetNewRootPath

Invoked in response to choosing our context menu item

******************************************************************************/

HRESULT CShellExt::GetNewRootPath(HWND hwnd)
{
    BROWSEINFO bi = {0};
    LPITEMIDLIST pidl;
    TCHAR szPath[MAX_PATH];
    TCHAR szTitle[MAX_PATH];


    // Init the dialog with the current path

    LoadString (g_hInst, IDS_BROWSETITLE, szTitle, MAX_PATH);
    bi.hwndOwner = hwnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = szPath;
    bi.lpszTitle = szTitle;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI | BIF_EDITBOX;
    bi.lpfn = NULL;


    pidl = SHBrowseForFolder (&bi);
    if (pidl)
    {
        SHGetPathFromIDList (pidl, szPath);
        CoTaskMemFree(pidl);
        if (FAILED(GetSetRootPath (m_pItem, szPath, ARRAYSIZE(szPath), TRUE)))
        {
            ::ShowMessage (hwnd, IDS_ERRCAPTION, IDS_ERRPROPSET);
            return S_FALSE;
        }
        return NOERROR;
    }
    return S_FALSE; // don't return failure code to the shell.
}



