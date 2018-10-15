/*++

Copyright (c) 1997-1998 Microsoft Corporation

Module Name:

    USBVIEW.C

Abstract:

    This is the GUI goop for the USBVIEW application.

Environment:

    user mode

Revision History:

    04-25-97 : created

--*/

//*****************************************************************************
// I N C L U D E S
//*****************************************************************************

#include <windows.h>
#include <basetyps.h>
#include <windowsx.h>
#include <initguid.h>
#include <devioctl.h>
#include <usbioctl.h>
#include <dbt.h>
#include <stdio.h>

#include "resource.h"
#include "usbview.h"

//*****************************************************************************
// D E F I N E S
//*****************************************************************************

// window control defines
//
#define SIZEBAR             0
#define WINDOWSCALEFACTOR   15

//*****************************************************************************
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//*****************************************************************************

int WINAPI
WinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpszCmdLine,
    int       nCmdShow
);

BOOL
CreateMainWindow (
    int nCmdShow
);

VOID
ResizeWindows (
    BOOL    bSizeBar,
    int     BarLocation
);

LRESULT CALLBACK
MainDlgProc (
    HWND   hwnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
);

BOOL
USBView_OnInitDialog (
    HWND    hWnd,
    HWND    hWndFocus,
    LPARAM  lParam
);

VOID
USBView_OnClose (
    HWND hWnd
);

VOID
USBView_OnCommand (
    HWND hWnd,
    int  id,
    HWND hwndCtl,
    UINT codeNotify
);

VOID
USBView_OnLButtonDown (
    HWND hWnd,
    BOOL fDoubleClick,
    int  x,
    int  y,
    UINT keyFlags
);

VOID
USBView_OnLButtonUp (
    HWND hWnd,
    int  x,
    int  y,
    UINT keyFlags
);

VOID
USBView_OnMouseMove (
    HWND hWnd,
    int  x,
    int  y,
    UINT keyFlags
);

VOID
USBView_OnSize (
    HWND hWnd,
    UINT state,
    int  cx,
    int  cy
);

LRESULT
USBView_OnNotify (
    HWND    hWnd,
    int     DlgItem,
    LPNMHDR lpNMHdr
);

BOOL
USBView_OnDeviceChange (
    HWND  hwnd,
    UINT  uEvent,
    DWORD dwEventData
);


VOID DestroyTree (VOID);

VOID RefreshTree (VOID);

LRESULT CALLBACK
AboutDlgProc (
    HWND   hwnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
);

VOID
WalkTree (
    HTREEITEM        hTreeItem,
    LPFNTREECALLBACK lpfnTreeCallback,
    DWORD            dwRefData
);

VOID
ExpandItem (
    HWND      hTreeWnd,
    HTREEITEM hTreeItem
);

//*****************************************************************************
// G L O B A L S    P R I V A T E    T O    T H I S    F I L E
//*****************************************************************************

HINSTANCE       ghInstance;
HWND            ghMainWnd;
HMENU           ghMainMenu;
HWND            ghTreeWnd;
HWND            ghEditWnd;
HWND            ghStatusWnd;
HCURSOR         ghSplitCursor;

int             gBarLocation    = 0;
BOOL            gbButtonDown    = FALSE;
HTREEITEM       ghTreeRoot      = NULL;

BOOL            gDoAutoRefresh  = FALSE;
BOOL            gDoConfigDesc   = FALSE;

// added
int             giGoodDevice;
int             giBadDevice;
int             giComputer;
int             giHub;
int             giNoDevice;
HDEVNOTIFY      gNotifyDevHandle;
HDEVNOTIFY      gNotifyHubHandle;


//*****************************************************************************
//
// WinMain()
//
//*****************************************************************************

int WINAPI
WinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR     lpszCmdLine,
    int       nCmdShow
)
{
    MSG     msg;
    HACCEL  hAccel;

    ghInstance = hInstance;

    ghSplitCursor = LoadCursor(ghInstance,
                               MAKEINTRESOURCE(IDC_SPLIT));

    if (!ghSplitCursor)
    {
        OOPS();
        return 0;
    }

    hAccel = LoadAccelerators(ghInstance,
                              MAKEINTRESOURCE(IDACCEL));

    if (!hAccel)
    {
        OOPS();
        return 0;
    }

    if (!CreateTextBuffer())
    {
        return 0;
    }

    if (!CreateMainWindow(nCmdShow))
    {
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!TranslateAccelerator(ghMainWnd,
                                  hAccel,
                                  &msg) &&
            !IsDialogMessage(ghMainWnd,
                             &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DestroyTextBuffer();

    CHECKFORLEAKS();

    return 1;
}

//*****************************************************************************
//
// CreateMainWindow()
//
//*****************************************************************************

BOOL
CreateMainWindow (
    int nCmdShow
)
{
    RECT rc;

    InitCommonControls();

    ghMainWnd = CreateDialog(ghInstance,
                             MAKEINTRESOURCE(IDD_MAINDIALOG),
                             NULL,
                             MainDlgProc);

    if (ghMainWnd == NULL)
    {
        OOPS();
        return FALSE;
    }

    GetWindowRect(ghMainWnd, &rc);

    gBarLocation = (rc.right - rc.left) / 3;

    ResizeWindows(FALSE, 0);

    ShowWindow(ghMainWnd, nCmdShow);

    UpdateWindow(ghMainWnd);

    return TRUE;
}


//*****************************************************************************
//
// ResizeWindows()
//
// Handles resizing the two child windows of the main window.  If
// bSizeBar is true, then the sizing is happening because the user is
// moving the bar.  If bSizeBar is false, the sizing is happening
// because of the WM_SIZE or something like that.
//
//*****************************************************************************

VOID
ResizeWindows (
    BOOL    bSizeBar,
    int     BarLocation
)
{
    RECT    MainClientRect;
    RECT    MainWindowRect;
    RECT    TreeWindowRect;
    RECT    StatusWindowRect;
    int     right;

    // Is the user moving the bar?
    //
    if (!bSizeBar)
    {
        BarLocation = gBarLocation;
    }

    GetClientRect(ghMainWnd, &MainClientRect);

    GetWindowRect(ghStatusWnd, &StatusWindowRect);

    // Make sure the bar is in a OK location
    //
    if (bSizeBar)
    {
        if (BarLocation <
            GetSystemMetrics(SM_CXSCREEN)/WINDOWSCALEFACTOR)
        {
            return;
        }

        if ((MainClientRect.right - BarLocation) <
            GetSystemMetrics(SM_CXSCREEN)/WINDOWSCALEFACTOR)
        {
            return;
        }
    }

    // Save the bar location
    //
    gBarLocation = BarLocation;

    // Move the tree window
    //
    MoveWindow(ghTreeWnd,
               0,
               0,
               BarLocation,
               MainClientRect.bottom - StatusWindowRect.bottom + StatusWindowRect.top,
               TRUE);

    // Get the size of the window (in case move window failed
    //
    GetWindowRect(ghTreeWnd, &TreeWindowRect);
    GetWindowRect(ghMainWnd, &MainWindowRect);

    right = TreeWindowRect.right - MainWindowRect.left;

    // Move the edit window with respect to the tree window
    //
    MoveWindow(ghEditWnd,
               right+SIZEBAR,
               0,
               MainClientRect.right-(right+SIZEBAR),
               MainClientRect.bottom - StatusWindowRect.bottom + StatusWindowRect.top,
               TRUE);

    // Move the Status window with respect to the tree window
    //
    MoveWindow(ghStatusWnd,
               0,
               MainClientRect.bottom - StatusWindowRect.bottom + StatusWindowRect.top,
               MainClientRect.right,
               StatusWindowRect.bottom - StatusWindowRect.top,
               TRUE);
}


//*****************************************************************************
//
// MainWndProc()
//
//*****************************************************************************

LRESULT CALLBACK
MainDlgProc (
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{

    switch (uMsg)
    {

        HANDLE_MSG(hWnd, WM_INITDIALOG,     USBView_OnInitDialog);
        HANDLE_MSG(hWnd, WM_CLOSE,          USBView_OnClose);
        HANDLE_MSG(hWnd, WM_COMMAND,        USBView_OnCommand);
        HANDLE_MSG(hWnd, WM_LBUTTONDOWN,    USBView_OnLButtonDown);
        HANDLE_MSG(hWnd, WM_LBUTTONUP,      USBView_OnLButtonUp);
        HANDLE_MSG(hWnd, WM_MOUSEMOVE,      USBView_OnMouseMove);
        HANDLE_MSG(hWnd, WM_SIZE,           USBView_OnSize);
        HANDLE_MSG(hWnd, WM_NOTIFY,         USBView_OnNotify);
        HANDLE_MSG(hWnd, WM_DEVICECHANGE,   USBView_OnDeviceChange);
    }

    return 0;
}

//*****************************************************************************
//
// USBView_OnInitDialog()
//
//*****************************************************************************

BOOL
USBView_OnInitDialog (
    HWND    hWnd,
    HWND    hWndFocus,
    LPARAM  lParam
)
{
    HFONT                           hFont;
    HIMAGELIST                      himl;
    HICON                           hicon;
    DEV_BROADCAST_DEVICEINTERFACE   broadcastInterface;


    // Register to receive notification when a USB device is plugged in.
    broadcastInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    broadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

    memcpy( &(broadcastInterface.dbcc_classguid),
            &(GUID_CLASS_USB_DEVICE),
            sizeof(struct _GUID));

    gNotifyDevHandle = RegisterDeviceNotification(hWnd,
                                                  &broadcastInterface,
                                                  DEVICE_NOTIFY_WINDOW_HANDLE);

    // Now register for Hub notifications.
    memcpy( &(broadcastInterface.dbcc_classguid),
            &(GUID_CLASS_USBHUB),
            sizeof(struct _GUID));

    gNotifyHubHandle = RegisterDeviceNotification(hWnd,
                                                  &broadcastInterface,
                                                  DEVICE_NOTIFY_WINDOW_HANDLE);

    //end add

    ghTreeWnd = GetDlgItem(hWnd, IDC_TREE);

    //added
    if ((himl = ImageList_Create(15, 15,
                                 FALSE, 2, 0)) == NULL)
    {
        OOPS();
    }

    hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_ICON));
    giGoodDevice = ImageList_AddIcon(himl, hicon);

    hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_BADICON));
    giBadDevice = ImageList_AddIcon(himl, hicon);

    hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_COMPUTER));
    giComputer = ImageList_AddIcon(himl, hicon);

    hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_HUB));
    giHub = ImageList_AddIcon(himl, hicon);

    hicon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_NODEVICE));
    giNoDevice = ImageList_AddIcon(himl, hicon);


    TreeView_SetImageList(ghTreeWnd, himl, TVSIL_NORMAL);
    // end add


    ghEditWnd = GetDlgItem(hWnd, IDC_EDIT);

    ghStatusWnd = GetDlgItem(hWnd, IDC_STATUS);

    ghMainMenu = GetMenu(hWnd);

    if (ghMainMenu == NULL)
    {
        OOPS();
    }

    hFont  = CreateFont(13,  8, 0, 0,
                        400, 0, 0, 0,
                        0,   1, 2, 1,
                        49,  TEXT("Courier"));

    SendMessage(ghEditWnd,
                WM_SETFONT,
                (WPARAM) hFont,
                0);

    RefreshTree();

    return FALSE;
}

//*****************************************************************************
//
// USBView_OnClose()
//
//*****************************************************************************

VOID
USBView_OnClose (
    HWND hWnd
)
{
    DestroyTree();

    PostQuitMessage(0);

    EndDialog(hWnd, 0);
}

//*****************************************************************************
//
// USBView_OnCommand()
//
//*****************************************************************************

VOID
USBView_OnCommand (
    HWND hWnd,
    int  id,
    HWND hwndCtl,
    UINT codeNotify
)
{
    MENUITEMINFO menuInfo;

    switch (id)
    {
        case ID_AUTO_REFRESH:
            gDoAutoRefresh = !gDoAutoRefresh;
            menuInfo.cbSize = sizeof(menuInfo);
            menuInfo.fMask  = MIIM_STATE;
            menuInfo.fState = gDoAutoRefresh ? MFS_CHECKED : MFS_UNCHECKED;
            SetMenuItemInfo(ghMainMenu,
                            id,
                            FALSE,
                            &menuInfo);
            break;

        case ID_CONFIG_DESCRIPTORS:
            gDoConfigDesc = !gDoConfigDesc;
            menuInfo.cbSize = sizeof(menuInfo);
            menuInfo.fMask  = MIIM_STATE;
            menuInfo.fState = gDoConfigDesc ? MFS_CHECKED : MFS_UNCHECKED;
            SetMenuItemInfo(ghMainMenu,
                            id,
                            FALSE,
                            &menuInfo);
            break;

        case ID_ABOUT:
            DialogBox(ghInstance,
                      MAKEINTRESOURCE(IDD_ABOUT),
                      ghMainWnd,
                      AboutDlgProc);
            break;

        case ID_EXIT:
            UnregisterDeviceNotification(gNotifyDevHandle);
            UnregisterDeviceNotification(gNotifyHubHandle);
            DestroyTree();
            PostQuitMessage(0);
            EndDialog(hWnd, 0);
            break;

        case ID_REFRESH:
            RefreshTree();
            break;
    }
}

//*****************************************************************************
//
// USBView_OnLButtonDown()
//
//*****************************************************************************

VOID
USBView_OnLButtonDown (
    HWND hWnd,
    BOOL fDoubleClick,
    int  x,
    int  y,
    UINT keyFlags
)
{
    gbButtonDown = TRUE;
    SetCapture(hWnd);
}

//*****************************************************************************
//
// USBView_OnLButtonUp()
//
//*****************************************************************************

VOID
USBView_OnLButtonUp (
    HWND hWnd,
    int  x,
    int  y,
    UINT keyFlags
)
{
    gbButtonDown = FALSE;
    ReleaseCapture();
}

//*****************************************************************************
//
// USBView_OnMouseMove()
//
//*****************************************************************************

VOID
USBView_OnMouseMove (
    HWND hWnd,
    int  x,
    int  y,
    UINT keyFlags
)
{
    SetCursor(ghSplitCursor);

    if (gbButtonDown)
    {
        ResizeWindows(TRUE, x);
    }
}

//*****************************************************************************
//
// USBView_OnSize();
//
//*****************************************************************************

VOID
USBView_OnSize (
    HWND hWnd,
    UINT state,
    int  cx,
    int  cy
)
{
    ResizeWindows(FALSE, 0);
}

//*****************************************************************************
//
// USBView_OnNotify()
//
//*****************************************************************************

LRESULT
USBView_OnNotify (
    HWND    hWnd,
    int     DlgItem,
    LPNMHDR lpNMHdr
)
{
    if (lpNMHdr->code == TVN_SELCHANGED)
    {
        HTREEITEM hTreeItem;

        hTreeItem = ((NM_TREEVIEW *)lpNMHdr)->itemNew.hItem;

        if (hTreeItem)
        {
            UpdateEditControl(ghEditWnd,
                              ghTreeWnd,
                              hTreeItem);
        }
    }

    return 0;
}


//*****************************************************************************
//
// USBView_OnDeviceChange()
//
//*****************************************************************************

BOOL
USBView_OnDeviceChange (
    HWND  hwnd,
    UINT  uEvent,
    DWORD dwEventData
)
{
    if (gDoAutoRefresh)
    {
        switch (uEvent)
        {
            case DBT_DEVICEARRIVAL:
            case DBT_DEVICEREMOVECOMPLETE:
                RefreshTree();
                break;
        }
    }

    return TRUE;
}



//*****************************************************************************
//
// DestroyTree()
//
//*****************************************************************************

VOID DestroyTree (VOID)
{
    // Clear the selection of the TreeView, so that when the tree is
    // destroyed, the control won't try to constantly "shift" the
    // selection to another item.
    //
    TreeView_SelectItem(ghTreeWnd, NULL);

    // Destroy the current contents of the TreeView
    //
    if (ghTreeRoot)
    {
        WalkTree(ghTreeRoot, CleanupItem, 0);

        TreeView_DeleteAllItems(ghTreeWnd);

        ghTreeRoot = NULL;
    }
}

//*****************************************************************************
//
// RefreshTree()
//
//*****************************************************************************

VOID RefreshTree (VOID)
{
    CHAR  statusText[128];
    ULONG devicesConnected;

    // Clear the selection of the TreeView, so that when the tree is
    // destroyed, the control won't try to constantly "shift" the
    // selection to another item.
    //
    TreeView_SelectItem(ghTreeWnd, NULL);

    // Clear the edit control
    //
    SetWindowText(ghEditWnd, "");

    // Destroy the current contents of the TreeView
    //
    if (ghTreeRoot)
    {
        WalkTree(ghTreeRoot, CleanupItem, 0);

        TreeView_DeleteAllItems(ghTreeWnd);

        ghTreeRoot = NULL;
    }

    // Create the root tree node
    //
    ghTreeRoot = AddLeaf(TVI_ROOT, 0, "My Computer", ComputerIcon);

    if (ghTreeRoot != NULL)
    {
        // Enumerate all USB buses and populate the tree
        //
        EnumerateHostControllers(ghTreeRoot, &devicesConnected);

        //
        // Expand all tree nodes
        //
        WalkTree(ghTreeRoot, ExpandItem, 0);

        // Update Status Line with number of devices connected
        //
        wsprintf(statusText, "Devices Connected: %d   Hubs Connected: %d",
                 devicesConnected, TotalHubs);
        SetWindowText(ghStatusWnd, statusText);
    }
    else
    {
        OOPS();
    }

}

//*****************************************************************************
//
// AboutDlgProc()
//
//*****************************************************************************

LRESULT CALLBACK
AboutDlgProc (
    HWND   hwnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (uMsg)
    {
    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:

            EndDialog (hwnd, 0);
            break;
        }
        break;

    }

    return FALSE;
}


//*****************************************************************************
//
// AddLeaf()
//
//*****************************************************************************

HTREEITEM
AddLeaf (
    HTREEITEM hTreeParent,
    LPARAM    lParam,
    LPTSTR    lpszText,
    TREEICON  TreeIcon
)
{
    TV_INSERTSTRUCT tvins;
    HTREEITEM       hti;

    memset(&tvins, 0, sizeof(tvins));

    // Set the parent item
    //
    tvins.hParent = hTreeParent;

    tvins.hInsertAfter = TVI_LAST;

    // pszText and lParam members are valid
    //
    tvins.item.mask = TVIF_TEXT | TVIF_PARAM;

    // Set the text of the item.
    //
    tvins.item.pszText = lpszText;

    // Set the user context item
    //
    tvins.item.lParam = lParam;

    // Add the item to the tree-view control.
    //
    hti = TreeView_InsertItem(ghTreeWnd, &tvins);

    tvins.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    tvins.item.hItem = hti;

    // Determine which icon to display for the device
    //
    switch (TreeIcon)
    {
        case ComputerIcon:
            tvins.item.iImage = giComputer;
            tvins.item.iSelectedImage = giComputer;
            break;

        case HubIcon:
            tvins.item.iImage = giHub;
            tvins.item.iSelectedImage = giHub;
            break;

        case NoDeviceIcon:
            tvins.item.iImage = giNoDevice;
            tvins.item.iSelectedImage = giNoDevice;
            break;

        case GoodDeviceIcon:
            tvins.item.iImage = giGoodDevice;
            tvins.item.iSelectedImage = giGoodDevice;
            break;

        case BadDeviceIcon:
        default:
            tvins.item.iImage = giBadDevice;
            tvins.item.iSelectedImage = giBadDevice;
            break;
    }

    TreeView_SetItem(ghTreeWnd, &tvins.item);

    return hti;
}

//*****************************************************************************
//
// WalkTree()
//
//*****************************************************************************

VOID
WalkTree (
    HTREEITEM        hTreeItem,
    LPFNTREECALLBACK lpfnTreeCallback,
    DWORD            dwRefData
)
{
    if (hTreeItem)
    {
        // Recursively call WalkTree on the node's first child.
        //
        WalkTree(TreeView_GetChild(ghTreeWnd, hTreeItem),
                 lpfnTreeCallback,
                 dwRefData);

        //
        // Call the lpfnCallBack on the node itself.
        //
        (*lpfnTreeCallback)(ghTreeWnd, hTreeItem);

        //
        //
        // Recursively call WalkTree on the node's first sibling.
        //
        WalkTree(TreeView_GetNextSibling(ghTreeWnd, hTreeItem),
                 lpfnTreeCallback,
                 dwRefData);
    }
}

//*****************************************************************************
//
// ExpandItem()
//
//*****************************************************************************

VOID
ExpandItem (
    HWND      hTreeWnd,
    HTREEITEM hTreeItem
)
{
    //
    // Make this node visible.
    //
    TreeView_Expand(hTreeWnd, hTreeItem, TVE_EXPAND);
}


#if DBG

//*****************************************************************************
//
// Oops()
//
//*****************************************************************************

VOID
Oops
(
    CHAR *File,
    ULONG Line
)
{
    char szBuf[256];

    wsprintf(szBuf, "File: %s, Line %d", File, Line);

    MessageBox(ghMainWnd, szBuf, "Oops!", MB_OK);
}

#endif

