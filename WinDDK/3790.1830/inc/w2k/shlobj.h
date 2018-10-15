//===========================================================================
//
// Copyright (c) Microsoft Corporation 1991-1998
//
// File: shlobj.h
//
//===========================================================================

#ifndef _SHLOBJ_H_
#define _SHLOBJ_H_

#ifndef _WINRESRC_
#ifndef _WIN32_IE
#define _WIN32_IE 0x0501
#else
#if (_WIN32_IE < 0x0400) && defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
#error _WIN32_IE setting conflicts with _WIN32_WINNT setting
#endif
#endif
#endif

#ifndef SNDMSG
#ifdef __cplusplus
#define SNDMSG ::SendMessage
#else
#define SNDMSG SendMessage
#endif
#endif // ifndef SNDMSG

//
// Define API decoration for direct importing of DLL references.
//
#ifndef WINSHELLAPI
#if defined(_SHELL32_)
#define WINSHELLAPI
#else
#define WINSHELLAPI       DECLSPEC_IMPORT
#endif
#endif // WINSHELLAPI

#ifndef SHSTDAPI
#if defined(_SHELL32_)
#define SHSTDAPI          STDAPI
#define SHSTDAPI_(type)   STDAPI_(type)
#else
#define SHSTDAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define SHSTDAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#endif
#endif // SHSTDAPI

#ifndef SHDOCAPI
#if defined(_SHDOCVW_)
#define SHDOCAPI          STDAPI
#define SHDOCAPI_(type)   STDAPI_(type)
#else
#define SHDOCAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define SHDOCAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#endif
#endif // SHDOCAPI

// shell32 APIs that are also exported from shdocvw
#ifndef SHSTDDOCAPI
#if defined(_SHDOCVW_) || defined(_SHELL32_)
#define SHSTDDOCAPI          STDAPI
#define SHSTDDOCAPI_(type)   STDAPI_(type)
#else
#define SHSTDDOCAPI          EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define SHSTDDOCAPI_(type)   EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#endif
#endif // SHSTDDOCAPI

#ifndef BROWSEUIAPI
#if defined(_BROWSEUI_)
#define BROWSEUIAPI           STDAPI
#define BROWSEUIAPI_(type)    STDAPI_(type)
#else
#define BROWSEUIAPI           EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#define BROWSEUIAPI_(type)    EXTERN_C DECLSPEC_IMPORT type STDAPICALLTYPE
#endif // defined(_BROWSEUI_)
#endif // BROWSEUIAPI

// shell32 APIs that are also exported from shfolder
#ifndef SHFOLDERAPI
#if defined(_SHFOLDER_) || defined(_SHELL32_)
#define SHFOLDERAPI           STDAPI
#else
#define SHFOLDERAPI           EXTERN_C DECLSPEC_IMPORT HRESULT STDAPICALLTYPE
#endif
#endif



#include <ole2.h>
#ifndef _PRSHT_H_
#include <prsht.h>
#endif
#ifndef _INC_COMMCTRL
#include <commctrl.h>   // for LPTBBUTTON
#endif

#ifndef INITGUID
#include <shlguid.h>
#endif /* !INITGUID */

#ifndef RC_INVOKED
#include <pshpack1.h>   /* Assume byte packing throughout */
#endif /* !RC_INVOKED */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif /* __cplusplus */


//===========================================================================
//
// Object identifiers in the explorer's name space (ItemID and IDList)
//
//  All the items that the user can browse with the explorer (such as files,
// directories, servers, work-groups, etc.) has an identifier which is unique
// among items within the parent folder. Those identifiers are called item
// IDs (SHITEMID). Since all its parent folders have their own item IDs,
// any items can be uniquely identified by a list of item IDs, which is called
// an ID list (ITEMIDLIST).
//
//  ID lists are almost always allocated by the task allocator (see some
// description below as well as OLE 2.0 SDK) and may be passed across
// some of shell interfaces (such as IShellFolder). Each item ID in an ID list
// is only meaningful to its parent folder (which has generated it), and all
// the clients must treat it as an opaque binary data except the first two
// bytes, which indicates the size of the item ID.
//
//  When a shell extension -- which implements the IShellFolder interace --
// generates an item ID, it may put any information in it, not only the data
// with that it needs to identifies the item, but also some additional
// information, which would help implementing some other functions efficiently.
// For example, the shell's IShellFolder implementation of file system items
// stores the primary (long) name of a file or a directory as the item
// identifier, but it also stores its alternative (short) name, size and date
// etc.
//
//  When an ID list is passed to one of shell APIs (such as SHGetPathFromIDList),
// it is always an absolute path -- relative from the root of the name space,
// which is the desktop folder. When an ID list is passed to one of IShellFolder
// member function, it is always a relative path from the folder (unless it
// is explicitly specified).
//
//===========================================================================

//
// SHITEMID -- Item ID
//
typedef struct _SHITEMID        // mkid
{
    USHORT      cb;             // Size of the ID (including cb itself)
    BYTE        abID[1];        // The item ID (variable length)
} SHITEMID;
typedef UNALIGNED SHITEMID *LPSHITEMID;
typedef const UNALIGNED SHITEMID *LPCSHITEMID;

//
// ITEMIDLIST -- List if item IDs (combined with 0-terminator)
//
typedef struct _ITEMIDLIST      // idl
{
    SHITEMID    mkid;
} ITEMIDLIST;
typedef UNALIGNED ITEMIDLIST * LPITEMIDLIST;
typedef const UNALIGNED ITEMIDLIST * LPCITEMIDLIST;

//===========================================================================
//
// Task allocator API
//
//  All the shell extensions MUST use the task allocator (see OLE 2.0
// programming guild for its definition) when they allocate or free
// memory objects (mostly ITEMIDLIST) that are returned across any
// shell interfaces. There are two ways to access the task allocator
// from a shell extension depending on whether or not it is linked with
// OLE32.DLL or not (purely for efficiency).
//
// (1) A shell extension which calls any OLE API (i.e., linked with
//  OLE32.DLL) should call OLE's task allocator (by retrieving
//  the task allocator by calling CoGetMalloc API).
//
// (2) A shell extension which does not call any OLE API (i.e., not linked
//  with OLE32.DLL) should call the shell task allocator API (defined
//  below), so that the shell can quickly loads it when OLE32.DLL is not
//  loaded by any application at that point.
//
// Notes:
//  In next version of Windowso release, SHGetMalloc will be replaced by
// the following macro.
//
// #define SHGetMalloc(ppmem)   CoGetMalloc(MEMCTX_TASK, ppmem)
//
//===========================================================================

SHSTDAPI SHGetMalloc(LPMALLOC * ppMalloc);


//===========================================================================
//
// IContextMenu interface
//
// [OverView]
//
//  The shell uses the IContextMenu interface in following three cases.
//
// case-1: The shell is loading context menu extensions.
//
//   When the user clicks the right mouse button on an item within the shell's
//  name space (i.g., file, directory, server, work-group, etc.), it creates
//  the default context menu for its type, then loads context menu extensions
//  that are registered for that type (and its base type) so that they can
//  add extra menu items. Those context menu extensions are registered at
//  HKCR\{ProgID}\shellex\ContextMenuHandlers.
//
// case-2: The shell is retrieving a context menu of sub-folders in extended
//   name-space.
//
//   When the explorer's name space is extended by name space extensions,
//  the shell calls their IShellFolder::GetUIObjectOf to get the IContextMenu
//  objects when it creates context menus for folders under those extended
//  name spaces.
//
// case-3: The shell is loading non-default drag and drop handler for directories.
//
//   When the user performed a non-default drag and drop onto one of file
//  system folders (i.e., directories), it loads shell extensions that are
//  registered at HKCR\{ProgID}\DragDropHandlers.
//
//
// [Member functions]
//
//
// IContextMenu::QueryContextMenu
//
//   This member function may insert one or more menuitems to the specified
//  menu (hmenu) at the specified location (indexMenu which is never be -1).
//  The IDs of those menuitem must be in the specified range (idCmdFirst and
//  idCmdLast). It returns the maximum menuitem ID offset (ushort) in the
//  'code' field (low word) of the scode.
//
//   The uFlags specify the context. It may have one or more of following
//  flags.
//
//  CMF_DEFAULTONLY: This flag is passed if the user is invoking the default
//   action (typically by double-clicking, case 1 and 2 only). Context menu
//   extensions (case 1) should not add any menu items, and returns NOERROR.
//
//  CMF_VERBSONLY: The explorer passes this flag if it is constructing
//   a context menu for a short-cut object (case 1 and case 2 only). If this
//   flag is passed, it should not add any menu-items that is not appropriate
//   from a short-cut.
//    A good example is the "Delete" menuitem, which confuses the user
//   because it is not clear whether it deletes the link source item or the
//   link itself.
//
//  CMF_EXPLORER: The explorer passes this flag if it has the left-side pane
//   (case 1 and 2 only). Context menu extensions should ignore this flag.
//
//   High word (16-bit) are reserved for context specific communications
//  and the rest of flags (13-bit) are reserved by the system.
//
//
// IContextMenu::InvokeCommand
//
//   This member is called when the user has selected one of menuitems that
//  are inserted by previous QueryContextMenu member. In this case, the
//  LOWORD(lpici->lpVerb) contains the menuitem ID offset (menuitem ID -
//  idCmdFirst).
//
//   This member function may also be called programmatically. In such a case,
//  lpici->lpVerb specifies the canonical name of the command to be invoked,
//  which is typically retrieved by GetCommandString member previously.
//
//  Parameters in lpci:
//    cbSize -- Specifies the size of this structure (sizeof(*lpci))
//    hwnd   -- Specifies the owner window for any message/dialog box.
//    fMask  -- Specifies whether or not dwHotkey/hIcon paramter is valid.
//    lpVerb -- Specifies the command to be invoked.
//    lpParameters -- Parameters (optional)
//    lpDirectory  -- Working directory (optional)
//    nShow -- Specifies the flag to be passed to ShowWindow (SW_*).
//    dwHotKey -- Hot key to be assigned to the app after invoked (optional).
//    hIcon -- Specifies the icon (optional).
//    hMonitor -- Specifies the default monitor (optional).
//
//
// IContextMenu::GetCommandString
//
//   This member function is called by the explorer either to get the
//  canonical (language independent) command name (uFlags == GCS_VERB) or
//  the help text ((uFlags & GCS_HELPTEXT) != 0) for the specified command.
//  The retrieved canonical string may be passed to its InvokeCommand
//  member function to invoke a command programmatically. The explorer
//  displays the help texts in its status bar; therefore, the length of
//  the help text should be reasonably short (<40 characters).
//
//  Parameters:
//   idCmd -- Specifies menuitem ID offset (from idCmdFirst)
//   uFlags -- Either GCS_VERB or GCS_HELPTEXT
//   pwReserved -- Reserved (must pass NULL when calling, must ignore when called)
//   pszName -- Specifies the string buffer.
//   cchMax -- Specifies the size of the string buffer.
//
//===========================================================================

// QueryContextMenu uFlags
#define CMF_NORMAL              0x00000000
#define CMF_DEFAULTONLY         0x00000001
#define CMF_VERBSONLY           0x00000002
#define CMF_EXPLORE             0x00000004
#define CMF_NOVERBS             0x00000008
#define CMF_CANRENAME           0x00000010
#define CMF_NODEFAULT           0x00000020
#define CMF_INCLUDESTATIC       0x00000040
#define CMF_FINDHACK            0x00000080
#define CMF_EXTENDEDVERBS       0x00000100      // rarely used verbs
#define CMF_RESERVED            0xffff0000      // View specific

// GetCommandString uFlags
#define GCS_VERBA        0x00000000     // canonical verb
#define GCS_HELPTEXTA    0x00000001     // help text (for status bar)
#define GCS_VALIDATEA    0x00000002     // validate command exists
#define GCS_VERBW        0x00000004     // canonical verb (unicode)
#define GCS_HELPTEXTW    0x00000005     // help text (unicode version)
#define GCS_VALIDATEW    0x00000006     // validate command exists (unicode)
#define GCS_UNICODE      0x00000004     // for bit testing - Unicode string

#ifdef UNICODE
#define GCS_VERB        GCS_VERBW
#define GCS_HELPTEXT    GCS_HELPTEXTW
#define GCS_VALIDATE    GCS_VALIDATEW
#else
#define GCS_VERB        GCS_VERBA
#define GCS_HELPTEXT    GCS_HELPTEXTA
#define GCS_VALIDATE    GCS_VALIDATEA
#endif

#define CMDSTR_NEWFOLDERA   "NewFolder"
#define CMDSTR_VIEWLISTA    "ViewList"
#define CMDSTR_VIEWDETAILSA "ViewDetails"
#define CMDSTR_NEWFOLDERW   L"NewFolder"
#define CMDSTR_VIEWLISTW    L"ViewList"
#define CMDSTR_VIEWDETAILSW L"ViewDetails"

#ifdef UNICODE
#define CMDSTR_NEWFOLDER    CMDSTR_NEWFOLDERW
#define CMDSTR_VIEWLIST     CMDSTR_VIEWLISTW
#define CMDSTR_VIEWDETAILS  CMDSTR_VIEWDETAILSW
#else
#define CMDSTR_NEWFOLDER    CMDSTR_NEWFOLDERA
#define CMDSTR_VIEWLIST     CMDSTR_VIEWLISTA
#define CMDSTR_VIEWDETAILS  CMDSTR_VIEWDETAILSA
#endif

#define CMIC_MASK_HOTKEY        SEE_MASK_HOTKEY
#define CMIC_MASK_ICON          SEE_MASK_ICON
#define CMIC_MASK_FLAG_NO_UI    SEE_MASK_FLAG_NO_UI
#define CMIC_MASK_UNICODE       SEE_MASK_UNICODE
#define CMIC_MASK_NO_CONSOLE    SEE_MASK_NO_CONSOLE
#define CMIC_MASK_HASLINKNAME   SEE_MASK_HASLINKNAME
#define CMIC_MASK_FLAG_SEP_VDM  SEE_MASK_FLAG_SEPVDM
#define CMIC_MASK_HASTITLE      SEE_MASK_HASTITLE
#define CMIC_MASK_ASYNCOK       SEE_MASK_ASYNCOK
#if (_WIN32_IE >= 0x0501)
#define CMIC_MASK_SHIFT_DOWN    0x10000000
#define CMIC_MASK_CONTROL_DOWN  0x20000000
#endif // (_WIN32_IE >= 0x501)


#if (_WIN32_IE >= 0x0400)
#define CMIC_MASK_PTINVOKE      0x20000000
#endif


//NOTE: When SEE_MASK_HMONITOR is set, hIcon is treated as hMonitor
typedef struct _CMINVOKECOMMANDINFO {
    DWORD cbSize;        // sizeof(CMINVOKECOMMANDINFO)
    DWORD fMask;         // any combination of CMIC_MASK_*
    HWND hwnd;           // might be NULL (indicating no owner window)
    LPCSTR lpVerb;       // either a string or MAKEINTRESOURCE(idOffset)
    LPCSTR lpParameters; // might be NULL (indicating no parameter)
    LPCSTR lpDirectory;  // might be NULL (indicating no specific directory)
    int nShow;           // one of SW_ values for ShowWindow() API

    DWORD dwHotKey;
    HANDLE hIcon;
} CMINVOKECOMMANDINFO,  *LPCMINVOKECOMMANDINFO;

typedef struct _CMInvokeCommandInfoEx {
    DWORD cbSize;        // must be sizeof(CMINVOKECOMMANDINFOEX)
    DWORD fMask;         // any combination of CMIC_MASK_*
    HWND hwnd;           // might be NULL (indicating no owner window)
    LPCSTR lpVerb;       // either a string or MAKEINTRESOURCE(idOffset)
    LPCSTR lpParameters; // might be NULL (indicating no parameter)
    LPCSTR lpDirectory;  // might be NULL (indicating no specific directory)
    int nShow;           // one of SW_ values for ShowWindow() API

    DWORD dwHotKey;

    HANDLE hIcon;
    LPCSTR lpTitle;      // For CreateProcess-StartupInfo.lpTitle
    LPCWSTR lpVerbW;        // Unicode verb (for those who can use it)
    LPCWSTR lpParametersW;  // Unicode parameters (for those who can use it)
    LPCWSTR lpDirectoryW;   // Unicode directory (for those who can use it)
    LPCWSTR lpTitleW;       // Unicode title (for those who can use it)
#if (_WIN32_IE >= 0x0400)
    POINT   ptInvoke;       // Point where it's invoked
#endif
} CMINVOKECOMMANDINFOEX,  *LPCMINVOKECOMMANDINFOEX;

#undef  INTERFACE
#define INTERFACE   IContextMenu

DECLARE_INTERFACE_(IContextMenu, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    STDMETHOD(QueryContextMenu)(THIS_
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags) PURE;

    STDMETHOD(InvokeCommand)(THIS_
                             LPCMINVOKECOMMANDINFO lpici) PURE;

    STDMETHOD(GetCommandString)(THIS_
                                UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax) PURE;
};

typedef IContextMenu *  LPCONTEXTMENU;

//
// IContextMenu2 (IContextMenu with one new member)
//
// IContextMenu2::HandleMenuMsg
//
//  This function is called, if the client of IContextMenu is aware of
// IContextMenu2 interface and receives one of following messages while
// it is calling TrackPopupMenu (in the window proc of hwnd):
//      WM_INITPOPUP, WM_DRAWITEM and WM_MEASUREITEM
//  The callee may handle these messages to draw owner draw menuitems.
//

#undef  INTERFACE
#define INTERFACE   IContextMenu2

DECLARE_INTERFACE_(IContextMenu2, IContextMenu)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IContextMenu methods ***

    STDMETHOD(QueryContextMenu)(THIS_
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags) PURE;

    STDMETHOD(InvokeCommand)(THIS_
                             LPCMINVOKECOMMANDINFO lpici) PURE;

    STDMETHOD(GetCommandString)(THIS_
                                UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax) PURE;

    // *** IContextMenu2 methods ***

    STDMETHOD(HandleMenuMsg)(THIS_
                             UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam) PURE;
};

typedef IContextMenu2 * LPCONTEXTMENU2;

//
// IContextMenu3 (IContextMenu with one new member)
//
// IContextMenu3::HandleMenuMsg2
//
//  This function is called, if the client of IContextMenu is aware of
// IContextMenu3 interface and receives a menu message while
// it is calling TrackPopupMenu (in the window proc of hwnd):
//

#undef  INTERFACE
#define INTERFACE   IContextMenu3

DECLARE_INTERFACE_(IContextMenu3, IContextMenu2)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IContextMenu methods ***

    STDMETHOD(QueryContextMenu)(THIS_
                                HMENU hmenu,
                                UINT indexMenu,
                                UINT idCmdFirst,
                                UINT idCmdLast,
                                UINT uFlags) PURE;

    STDMETHOD(InvokeCommand)(THIS_
                             LPCMINVOKECOMMANDINFO lpici) PURE;

    STDMETHOD(GetCommandString)(THIS_
                                UINT_PTR    idCmd,
                                UINT        uType,
                                UINT      * pwReserved,
                                LPSTR       pszName,
                                UINT        cchMax) PURE;

    // *** IContextMenu2 methods ***

    STDMETHOD(HandleMenuMsg)(THIS_
                             UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam) PURE;

    // *** IContextMenu3 methods ***

    STDMETHOD(HandleMenuMsg2)(THIS_
                             UINT uMsg,
                             WPARAM wParam,
                             LPARAM lParam,
                             LRESULT* plResult) PURE;
};

typedef IContextMenu3 * LPCONTEXTMENU3;


//===========================================================================
//
// Interface: IShellExtInit
//
//  The IShellExtInit interface is used by the explorer to initialize shell
// extension objects. The explorer (1) calls CoCreateInstance (or equivalent)
// with the registered CLSID and IID_IShellExtInit, (2) calls its Initialize
// member, then (3) calls its QueryInterface to a particular interface (such
// as IContextMenu or IPropSheetExt and (4) performs the rest of operation.
//
//
// [Member functions]
//
// IShellExtInit::Initialize
//
//  This member function is called when the explorer is initializing either
// context menu extension, property sheet extension or non-default drag-drop
// extension.
//
//  Parameters: (context menu or property sheet extension)
//   pidlFolder -- Specifies the parent folder
//   lpdobj -- Spefifies the set of items selected in that folder.
//   hkeyProgID -- Specifies the type of the focused item in the selection.
//
//  Parameters: (non-default drag-and-drop extension)
//   pidlFolder -- Specifies the target (destination) folder
//   lpdobj -- Specifies the items that are dropped (see the description
//    about shell's clipboard below for clipboard formats).
//   hkeyProgID -- Specifies the folder type.
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IShellExtInit

DECLARE_INTERFACE_(IShellExtInit, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellExtInit methods ***
    STDMETHOD(Initialize)(THIS_ LPCITEMIDLIST pidlFolder,
                          IDataObject *lpdobj, HKEY hkeyProgID) PURE;
};

typedef IShellExtInit * LPSHELLEXTINIT;


//===========================================================================
//
// Interface: IShellPropSheetExt
//
//  The explorer uses the IShellPropSheetExt to allow property sheet
// extensions or control panel extensions to add additional property
// sheet pages.
//
//
// [Member functions]
//
// IShellPropSheetExt::AddPages
//
//  The explorer calls this member function when it finds a registered
// property sheet extension for a particular type of object. For each
// additional page, the extension creates a page object by calling
// CreatePropertySheetPage API and calls lpfnAddPage.
//
//  Parameters:
//   lpfnAddPage -- Specifies the callback function.
//   lParam -- Specifies the opaque handle to be passed to the callback function.
//
//
// IShellPropSheetExt::ReplacePage
//
//  The explorer never calls this member of property sheet extensions. The
// explorer calls this member of control panel extensions, so that they
// can replace some of default control panel pages (such as a page of
// mouse control panel).
//
//  Parameters:
//   uPageID -- Specifies the page to be replaced.
//   lpfnReplace Specifies the callback function.
//   lParam -- Specifies the opaque handle to be passed to the callback function.
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IShellPropSheetExt

DECLARE_INTERFACE_(IShellPropSheetExt, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellPropSheetExt methods ***
    STDMETHOD(AddPages)(THIS_ LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam) PURE;
    STDMETHOD(ReplacePage)(THIS_ UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam) PURE;
};

typedef IShellPropSheetExt * LPSHELLPROPSHEETEXT;


//===========================================================================
//
// IPersistFolder Interface
//
//  The IPersistFolder interface is used by the file system implementation of
// IShellFolder::BindToObject when it is initializing a shell folder object.
//
//
// [Member functions]
//
// IPersistFolder::Initialize
//
//  This member function is called when the explorer is initializing a
// shell folder object.
//
//  Parameters:
//   pidl -- Specifies the absolute location of the folder.
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IPersistFolder

DECLARE_INTERFACE_(IPersistFolder, IPersist)    // fld
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    // *** IPersist methods ***
    STDMETHOD(GetClassID)(THIS_ LPCLSID lpClassID) PURE;

    // *** IPersistFolder methods ***
    STDMETHOD(Initialize)(THIS_ LPCITEMIDLIST pidl) PURE;
};

typedef IPersistFolder *LPPERSISTFOLDER;

#if (_WIN32_IE >= 0x0400)

#undef  INTERFACE
#define INTERFACE   IPersistFolder2

DECLARE_INTERFACE_(IPersistFolder2, IPersistFolder)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    // *** IPersist methods ***
    STDMETHOD(GetClassID)(THIS_ LPCLSID lpClassID) PURE;

    // *** IPersistFolder methods ***
    STDMETHOD(Initialize)(THIS_ LPCITEMIDLIST pidl) PURE;

    // *** IPersistFolder2 methods ***
    STDMETHOD(GetCurFolder)(THIS_ LPITEMIDLIST *ppidl) PURE;
};

#if (_WIN32_IE >= 0x0401)
typedef IPersistFolder2 *LPPERSISTFOLDER2;
#endif

#endif

#if (_WIN32_IE >= 0x0500)
#undef  INTERFACE
#define INTERFACE   IPersistFolder3

#define CSIDL_FLAG_PFTI_TRACKTARGET CSIDL_FLAG_DONT_VERIFY

// DESCRIPTION: PERSIST_FOLDER_TARGET_INFO
//    This stucture is used for Folder Shortcuts which allow the shell to
// have a file system folder act like another area in the name space.
// One of pidlTargetFolder, szTargetParsingName, or csidl needs to
// specify the destination name space.
//
// pidlTargetFolder: This is a full pidl to the target folder.  Can be NULL in the IPersistFolder3::InitializeEx()
//                   call but not in the GetFolderTargetInfo() return structure.
// szTargetParsingName: Empty string if not specified. Ortherwise, it is the parsible name
//                       to the target.  This name can be parsed by IShellFolder::
//                       ParseDisplayName() from the desktop.
// szNetworkProvider: Can be an empty string.  If not empty, it specifies the type of network
//                    provider that will be used when binding to the target.  This is used
//                    for performance optimizations for the WNet APIs.
// dwAttributes: -1 if not known.  These are the SFGAO_ flags for IShellFolder::GetAttributesOf()
// csidl: This is -1 if it's not used.  This can be used instead of pidlTargetFolder or
//        szTargetParsingName to indicate the TargetFolder.  See the list of CSIDL_ folders
//        below.  CSIDL_FLAG_PFTI_TRACKTARGET means that the IShellFolder's target folder
//        should change if the user changes the target of the underlying CSIDL value.
//        You can also pass CSIDL_FLAG_CREATE to indicate that the target folder
//        should be created if it does not exist.  No other CSIDL_FLAG_* values are supported.
typedef struct
{
    LPITEMIDLIST  pidlTargetFolder;               // pidl for the folder we want to intiailize
    WCHAR         szTargetParsingName[MAX_PATH];  // optional parsing name for the target
    WCHAR         szNetworkProvider[MAX_PATH];    // optional network provider
    DWORD         dwAttributes;                   // optional FILE_ATTRIBUTES_ flags (-1 if not used)
    int           csidl;                          // optional folder index (SHGetFolderPath()) -1 if not used
} PERSIST_FOLDER_TARGET_INFO;


// DESCRIPTION: IPersistFolder3
//    This interface is implemented by an IShellFolder object that wants non-default
// handling of Folder Shortcuts.  In general, shell name space extensions should use
// pidlRoot (the alias pidl) as their location in the name space and pass it to public
// APIs, such as ShellExecute().  The one exception is that pidlTarget should be used
// when sending ChangeNotifies or registering to listen for change notifies
// (see SFVM_GETNOTIFY).
//
// InitializeEx: This method initializes an IShellFolder and specifies where
//               it is rooted in the name space.
//      pbc: May be NULL.
//      pidlRoot: This is the same parameter as IPersistFolder::Initialize(). Caller allocates
//                and frees this parameter.
//      ppfti: May be NULL, in which case this is the same as a call to IPersistFolder::Initialize().
//             Otherwise this is a Folder Shortcut and this struction specifies the target
//             folder and it's attributes.
// GetFolderTargetInfo: This is used by the caller to find information about
//             the folder shortcut.  This structure may not be initialized by the caller,
//             so the callee needs to initialize every member.  The callee allocates
//             pidlTargetFolder and the caller will free it.  Filling in pidlTargetFolder is
//             ALWAYS required.

DECLARE_INTERFACE_(IPersistFolder3, IPersistFolder2)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    // *** IPersist methods ***
    STDMETHOD(GetClassID)(THIS_ LPCLSID lpClassID) PURE;

    // *** IPersistFolder methods ***
    STDMETHOD(Initialize)(THIS_ LPCITEMIDLIST pidl) PURE;

    // *** IPersistFolder2 methods ***
    STDMETHOD(GetCurFolder)(THIS_ LPITEMIDLIST *ppidl) PURE;

    // *** IPersistFolder3 methods ***
    STDMETHOD(InitializeEx)(THIS_ IBindCtx *pbc, LPCITEMIDLIST pidlRoot, const PERSIST_FOLDER_TARGET_INFO *ppfti) PURE;
    STDMETHOD(GetFolderTargetInfo)(THIS_ PERSIST_FOLDER_TARGET_INFO *ppfti) PURE;
};

//
//  this interface is just the IID.  return back
//  a pointer to the IPersist interface if the object
//  implementation is free threaded.  this is used
//  for performance on free threaded objects.
//
#define IPersistFreeThreadedObject IPersist

#endif

//

//===========================================================================
//
// IRemoteComputer Interface
//
//  The IRemoteComputer interface is used to initialize a name space
// extension invoked on a remote computer object.
//
// [Member functions]
//
// IRemoteComputer::Initialize
//
//  This member function is called when the explorer is initializing or
// enumerating the name space extension. If failure is returned during
// enumeration, the extension won't appear for this computer. Otherwise,
// the extension will appear, and should target the given machine.
//
//  Parameters:
//   pszMachine -- Specifies the name of the machine to target.
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IRemoteComputer

DECLARE_INTERFACE_(IRemoteComputer, IUnknown)    // remc
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IRemoteComputer methods ***
    STDMETHOD(Initialize) (THIS_ const WCHAR *pszMachine, BOOL bEnumerating) PURE;
};

//===========================================================================
//
// IExtractIcon interface
//
//  This interface is used in two different places in the shell.
//
// Case-1: Icons of sub-folders for the scope-pane of the explorer.
//
//  It is used by the explorer to get the "icon location" of
// sub-folders from each shell folders. When the user expands a folder
// in the scope pane of the explorer, the explorer does following:
//  (1) binds to the folder (gets IShellFolder),
//  (2) enumerates its sub-folders by calling its EnumObjects member,
//  (3) calls its GetUIObjectOf member to get IExtractIcon interface
//     for each sub-folders.
//  In this case, the explorer uses only IExtractIcon::GetIconLocation
// member to get the location of the appropriate icon. An icon location
// always consists of a file name (typically DLL or EXE) and either an icon
// resource or an icon index.
//
//
// Case-2: Extracting an icon image from a file
//
//  It is used by the shell when it extracts an icon image
// from a file. When the shell is extracting an icon from a file,
// it does following:
//  (1) creates the icon extraction handler object (by getting its CLSID
//     under the {ProgID}\shell\ExtractIconHanler key and calling
//     CoCreateInstance requesting for IExtractIcon interface).
//  (2) Calls IExtractIcon::GetIconLocation.
//  (3) Then, calls IExtractIcon::ExtractIcon with the location/index pair.
//  (4) If (3) returns NOERROR, it uses the returned icon.
//  (5) Otherwise, it recursively calls this logic with new location
//     assuming that the location string contains a fully qualified path name.
//
//  From extension programmer's point of view, there are only two cases
// where they provide implementations of IExtractIcon:
//  Case-1) providing explorer extensions (i.e., IShellFolder).
//  Case-2) providing per-instance icons for some types of files.
//
// Because Case-1 is described above, we'll explain only Case-2 here.
//
// When the shell is about display an icon for a file, it does following:
//  (1) Finds its ProgID and ClassID.
//  (2) If the file has a ClassID, it gets the icon location string from the
//    "DefaultIcon" key under it. The string indicates either per-class
//    icon (e.g., "FOOBAR.DLL,2") or per-instance icon (e.g., "%1,1").
//  (3) If a per-instance icon is specified, the shell creates an icon
//    extraction handler object for it, and extracts the icon from it
//    (which is described above).
//
//  It is important to note that the shell calls IExtractIcon::GetIconLocation
// first, then calls IExtractIcon::Extract. Most application programs
// that support per-instance icons will probably store an icon location
// (DLL/EXE name and index/id) rather than an icon image in each file.
// In those cases, a programmer needs to implement only the GetIconLocation
// member and it Extract member simply returns S_FALSE. They need to
// implement Extract member only if they decided to store the icon images
// within files themselved or some other database (which is very rare).
//
//
//
// [Member functions]
//
//
// IExtractIcon::GetIconLocation
//
//  This function returns an icon location.
//
//  Parameters:
//   uFlags     [in]  -- Specifies if it is opened or not (GIL_OPENICON or 0)
//   szIconFile [out] -- Specifies the string buffer buffer for a location name.
//   cchMax     [in]  -- Specifies the size of szIconFile (almost always MAX_PATH)
//   piIndex    [out] -- Sepcifies the address of UINT for the index.
//   pwFlags    [out] -- Returns GIL_* flags
//  Returns:
//   NOERROR, if it returns a valid location; S_FALSE, if the shell use a
//   default icon.
//
//  Notes: The location may or may not be a path to a file. The caller can
//   not assume anything unless the subsequent Extract member call returns
//   S_FALSE.
//
//   if the returned location is not a path to a file, GIL_NOTFILENAME should
//   be set in the returned flags.
//
// IExtractIcon::Extract
//
//  This function extracts an icon image from a specified file.
//
//  Parameters:
//   pszFile [in] -- Specifies the icon location (typically a path to a file).
//   nIconIndex [in] -- Specifies the icon index.
//   phiconLarge [out] -- Specifies the HICON variable for large icon.
//   phiconSmall [out] -- Specifies the HICON variable for small icon.
//   nIconSize [in] -- Specifies the size icon required (size of large icon)
//                     LOWORD is the requested large icon size
//                     HIWORD is the requested small icon size
//  Returns:
//   NOERROR, if it extracted the from the file.
//   S_FALSE, if the caller should extract from the file specified in the
//           location.
//
//===========================================================================

// GetIconLocation() input flags

#define GIL_OPENICON     0x0001      // allows containers to specify an "open" look
#define GIL_FORSHELL     0x0002      // icon is to be displayed in a ShellFolder
#define GIL_ASYNC        0x0020      // this is an async extract, return E_ASYNC

// GetIconLocation() return flags

#define GIL_SIMULATEDOC  0x0001      // simulate this document icon for this
#define GIL_PERINSTANCE  0x0002      // icons from this class are per instance (each file has its own)
#define GIL_PERCLASS     0x0004      // icons from this class per class (shared for all files of this type)
#define GIL_NOTFILENAME  0x0008      // location is not a filename, must call ::ExtractIcon
#define GIL_DONTCACHE    0x0010      // this icon should not be cached

#undef  INTERFACE
#define INTERFACE   IExtractIconA

DECLARE_INTERFACE_(IExtractIconA, IUnknown)     // exic
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExtractIcon methods ***
    STDMETHOD(GetIconLocation)(THIS_
                         UINT   uFlags,
                         LPSTR  szIconFile,
                         UINT   cchMax,
                         int   * piIndex,
                         UINT  * pwFlags) PURE;

    STDMETHOD(Extract)(THIS_
                           LPCSTR pszFile,
                           UINT   nIconIndex,
                           HICON   *phiconLarge,
                           HICON   *phiconSmall,
                           UINT    nIconSize) PURE;
};

typedef IExtractIconA * LPEXTRACTICONA;

#undef  INTERFACE
#define INTERFACE   IExtractIconW

DECLARE_INTERFACE_(IExtractIconW, IUnknown)     // exic
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IExtractIcon methods ***
    STDMETHOD(GetIconLocation)(THIS_
                         UINT   uFlags,
                         LPWSTR szIconFile,
                         UINT   cchMax,
                         int   * piIndex,
                         UINT  * pwFlags) PURE;

    STDMETHOD(Extract)(THIS_
                           LPCWSTR pszFile,
                           UINT   nIconIndex,
                           HICON   *phiconLarge,
                           HICON   *phiconSmall,
                           UINT    nIconSize) PURE;
};

typedef IExtractIconW * LPEXTRACTICONW;

#ifdef UNICODE
#define IExtractIcon        IExtractIconW
#define IExtractIconVtbl    IExtractIconWVtbl
#define LPEXTRACTICON       LPEXTRACTICONW
#else
#define IExtractIcon        IExtractIconA
#define IExtractIconVtbl    IExtractIconAVtbl
#define LPEXTRACTICON       LPEXTRACTICONA
#endif

//===========================================================================
//
// IShellIcon Interface
//
// used to get a icon index for a IShellFolder object.
//
// this interface can be implemented by a IShellFolder, as a quick way to
// return the icon for a object in the folder.
//
// a instance of this interface is only created once for the folder, unlike
// IExtractIcon witch is created once for each object.
//
// if a ShellFolder does not implement this interface, the standard
// GetUIObject(....IExtractIcon) method will be used to get a icon
// for all objects.
//
// the following standard imagelist indexs can be returned:
//
//      0   document (blank page) (not associated)
//      1   document (with stuff on the page)
//      2   application (exe, com, bat)
//      3   folder (plain)
//      4   folder (open)
//
// IShellIcon:GetIconOf(pidl, flags, lpIconIndex)
//
//      pidl            object to get icon for.
//      flags           GIL_* input flags (GIL_OPEN, ...)
//      lpIconIndex     place to return icon index.
//
//  returns:
//      NOERROR, if lpIconIndex contains the correct system imagelist index.
//      S_FALSE, if unable to get icon for this object, go through
//               GetUIObject, IExtractIcon, methods.
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IShellIcon

DECLARE_INTERFACE_(IShellIcon, IUnknown)      // shi
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellIcon methods ***
    STDMETHOD(GetIconOf)(THIS_ LPCITEMIDLIST pidl, UINT flags,
                    LPINT lpIconIndex) PURE;
};

typedef IShellIcon *LPSHELLICON;

//===========================================================================
//
// IShellIconOverlayIdentifier
//
// Used to identify a file as a member of the group of files that have this specific
// icon overlay
//
// Users can create new IconOverlayIdentifiers and place them in the following registry
// location together with the Icon overlay image and their priority.
// HKEY_LOCAL_MACHINE "Software\\Microsoft\\Windows\\CurrentVersion\\ShellIconOverlayIdentifiers"
//
// The shell will enumerate through all IconOverlayIdentifiers at start, and prioritize
// them according to internal rules, in case the internal rules don't apply, we use their
// input priority
//
// IShellIconOverlayIdentifier:IsMemberOf(LPCWSTR pwszPath, DWORD dwAttrib)
//      pwszPath        full path of the file
//      dwAttrib        attribute of this file
//
//  returns:
//      S_OK,    if the file is a member
//      S_FALSE, if the file is not a member
//      E_FAIL,  if the operation failed due to bad WIN32_FIND_DATA
//
// IShellIconOverlayIdentifier::GetOverlayInfo(LPWSTR pwszIconFile, int * pIndex, DWORD * dwFlags) PURE;
//      pszIconFile    the path of the icon file
//      pIndex         Depend on the flags, this could contain the IconIndex
//      dwFlags        defined below
//
// IShellIconOverlayIdentifier::GetPriority(int * pIPriority) PURE;
//      pIPriority     the priority of this Overlay Identifier
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IShellIconOverlayIdentifier

DECLARE_INTERFACE_(IShellIconOverlayIdentifier, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellIconOverlayIdentifier methods ***
    STDMETHOD (IsMemberOf)(THIS_ LPCWSTR pwszPath, DWORD dwAttrib) PURE;
    STDMETHOD (GetOverlayInfo)(THIS_ LPWSTR pwszIconFile, int cchMax, int * pIndex, DWORD * pdwFlags) PURE;
    STDMETHOD (GetPriority)(THIS_ int * pIPriority) PURE;
};

#define ISIOI_ICONFILE            0x00000001          // path is returned through pwszIconFile
#define ISIOI_ICONINDEX           0x00000002          // icon index in pwszIconFile is returned through pIndex


//===========================================================================
//
// IShellIconOverlay
//
// Used to return the icon overlay index or its icon index for an IShellFolder object,
// this is always implemented with IShellFolder
//
// IShellIconOverlay:GetOverlayIndex(LPCITEMIDLIST pidl, DWORD * pdwIndex)
//      pidl            object to identify icon overlay for.
//      pdwIndex        the Overlay Index in the system image list
//
// IShellIconOverlay:GetOverlayIconIndex(LPCITEMIDLIST pidl, DWORD * pdwIndex)
//      pdwIconIndex    the Overlay Icon index in the system image list
// This method is only used for those who are interested in seeing the real bits
// of the Overlay Icon
//
//  returns:
//      S_OK,  if the index of an Overlay is found
//      S_FALSE, if no Overlay exists for this file
//      E_FAIL, if pidl is bad
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IShellIconOverlay

DECLARE_INTERFACE_(IShellIconOverlay, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellIconOverlay methods ***
    STDMETHOD(GetOverlayIndex)(THIS_ LPCITEMIDLIST pidl, int * pIndex) PURE;
    STDMETHOD(GetOverlayIconIndex)(THIS_ LPCITEMIDLIST pidl, int * pIconIndex) PURE;
};

#define OI_ASYNC 0xFFFFEEEE

//-------------------------------------------------------------------------
//
// SHGetIconOverlayIndex
//
// This function takes the path and icon/res id to the icon and convert it into
// an overlay index in the system image list.
// Note: there are totally only 15 slots for system image overlays, some of which
// was reserved by the system, or taken by the overlayidentifiers, so it's possible
// that this function would fail and return -1;
//
// To get the default overlays in the system, such as the share hand, link shortcut
// and slow files, pass NULL as the file name, then the IDO_SHGIOI_* flags as the icon index
//-------------------------------------------------------------------------

#define IDO_SHGIOI_SHARE  0x0FFFFFFF
#define IDO_SHGIOI_LINK   0x0FFFFFFE
#define IDO_SHGIOI_SLOWFILE 0x0FFFFFFFD
SHSTDAPI_(int) SHGetIconOverlayIndexA(LPCSTR pszIconPath, int iIconIndex);
SHSTDAPI_(int) SHGetIconOverlayIndexW(LPCWSTR pszIconPath, int iIconIndex);
#ifdef UNICODE
#define SHGetIconOverlayIndex  SHGetIconOverlayIndexW
#else
#define SHGetIconOverlayIndex  SHGetIconOverlayIndexA
#endif // !UNICODE


//===========================================================================
//
// IShellLink Interface
//
//===========================================================================

#ifdef UNICODE
#define IShellLink      IShellLinkW
#define IShellLinkVtbl  IShellLinkWVtbl
#else
#define IShellLink      IShellLinkA
#define IShellLinkVtbl  IShellLinkAVtbl
#endif

// IShellLink::Resolve fFlags
typedef enum {
    SLR_NO_UI           = 0x0001,   // don't post any UI durring the resolve operation
    SLR_ANY_MATCH       = 0x0002,   // no longer used
    SLR_UPDATE          = 0x0004,   // save the link back to it's file if the track made it dirty
    SLR_NOUPDATE        = 0x0008,
    SLR_NOSEARCH        = 0x0010,   // don't execute the search heuristics
    SLR_NOTRACK         = 0x0020,   // don't use NT5 object ID to track the link
    SLR_NOLINKINFO      = 0x0040,   // don't use the net and volume relative info
    SLR_INVOKE_MSI      = 0x0080,   // if we have a darwin link, then call msi to fault in the applicaion
} SLR_FLAGS;

// IShellLink::GetPath fFlags
typedef enum {
    SLGP_SHORTPATH      = 0x0001,
    SLGP_UNCPRIORITY    = 0x0002,
    SLGP_RAWPATH        = 0x0004,
} SLGP_FLAGS;

#undef  INTERFACE
#define INTERFACE   IShellLinkA

DECLARE_INTERFACE_(IShellLinkA, IUnknown)       // sl
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellLink methods ***
    STDMETHOD(GetPath)(THIS_ LPSTR pszFile, int cchMaxPath, WIN32_FIND_DATAA *pfd, DWORD fFlags) PURE;

    STDMETHOD(GetIDList)(THIS_ LPITEMIDLIST * ppidl) PURE;
    STDMETHOD(SetIDList)(THIS_ LPCITEMIDLIST pidl) PURE;

    STDMETHOD(GetDescription)(THIS_ LPSTR pszName, int cchMaxName) PURE;
    STDMETHOD(SetDescription)(THIS_ LPCSTR pszName) PURE;

    STDMETHOD(GetWorkingDirectory)(THIS_ LPSTR pszDir, int cchMaxPath) PURE;
    STDMETHOD(SetWorkingDirectory)(THIS_ LPCSTR pszDir) PURE;

    STDMETHOD(GetArguments)(THIS_ LPSTR pszArgs, int cchMaxPath) PURE;
    STDMETHOD(SetArguments)(THIS_ LPCSTR pszArgs) PURE;

    STDMETHOD(GetHotkey)(THIS_ WORD *pwHotkey) PURE;
    STDMETHOD(SetHotkey)(THIS_ WORD wHotkey) PURE;

    STDMETHOD(GetShowCmd)(THIS_ int *piShowCmd) PURE;
    STDMETHOD(SetShowCmd)(THIS_ int iShowCmd) PURE;

    STDMETHOD(GetIconLocation)(THIS_ LPSTR pszIconPath, int cchIconPath, int *piIcon) PURE;
    STDMETHOD(SetIconLocation)(THIS_ LPCSTR pszIconPath, int iIcon) PURE;

    STDMETHOD(SetRelativePath)(THIS_ LPCSTR pszPathRel, DWORD dwReserved) PURE;

    STDMETHOD(Resolve)(THIS_ HWND hwnd, DWORD fFlags) PURE;

    STDMETHOD(SetPath)(THIS_ LPCSTR pszFile) PURE;
};

#undef  INTERFACE
#define INTERFACE   IShellLinkW

DECLARE_INTERFACE_(IShellLinkW, IUnknown)       // sl
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellLink methods ***
    STDMETHOD(GetPath)(THIS_ LPWSTR pszFile, int cchMaxPath, WIN32_FIND_DATAW *pfd, DWORD fFlags) PURE;

    STDMETHOD(GetIDList)(THIS_ LPITEMIDLIST * ppidl) PURE;
    STDMETHOD(SetIDList)(THIS_ LPCITEMIDLIST pidl) PURE;

    STDMETHOD(GetDescription)(THIS_ LPWSTR pszName, int cchMaxName) PURE;
    STDMETHOD(SetDescription)(THIS_ LPCWSTR pszName) PURE;

    STDMETHOD(GetWorkingDirectory)(THIS_ LPWSTR pszDir, int cchMaxPath) PURE;
    STDMETHOD(SetWorkingDirectory)(THIS_ LPCWSTR pszDir) PURE;

    STDMETHOD(GetArguments)(THIS_ LPWSTR pszArgs, int cchMaxPath) PURE;
    STDMETHOD(SetArguments)(THIS_ LPCWSTR pszArgs) PURE;

    STDMETHOD(GetHotkey)(THIS_ WORD *pwHotkey) PURE;
    STDMETHOD(SetHotkey)(THIS_ WORD wHotkey) PURE;

    STDMETHOD(GetShowCmd)(THIS_ int *piShowCmd) PURE;
    STDMETHOD(SetShowCmd)(THIS_ int iShowCmd) PURE;

    STDMETHOD(GetIconLocation)(THIS_ LPWSTR pszIconPath, int cchIconPath, int *piIcon) PURE;
    STDMETHOD(SetIconLocation)(THIS_ LPCWSTR pszIconPath, int iIcon) PURE;

    STDMETHOD(SetRelativePath)(THIS_ LPCWSTR pszPathRel, DWORD dwReserved) PURE;

    STDMETHOD(Resolve)(THIS_ HWND hwnd, DWORD fFlags) PURE;

    STDMETHOD(SetPath)(THIS_ LPCWSTR pszFile) PURE;
};

#if (_WIN32_IE >= 0x0400)

// IShellLinkDataList::GetFlags()/SetFlags()
typedef enum {
   SLDF_HAS_ID_LIST         = 0x00000001,   // Shell link saved with ID list
   SLDF_HAS_LINK_INFO       = 0x00000002,   // Shell link saved with LinkInfo
   SLDF_HAS_NAME            = 0x00000004,
   SLDF_HAS_RELPATH         = 0x00000008,
   SLDF_HAS_WORKINGDIR      = 0x00000010,
   SLDF_HAS_ARGS            = 0x00000020,
   SLDF_HAS_ICONLOCATION    = 0x00000040,
   SLDF_UNICODE             = 0x00000080,   // the strings are unicode
   SLDF_FORCE_NO_LINKINFO   = 0x00000100,   // don't create a LINKINFO (make a dumb link)
   SLDF_HAS_EXP_SZ          = 0x00000200,   // the link contains expandable env strings
   SLDF_RUN_IN_SEPARATE     = 0x00000400,   // Run the 16-bit target exe in a separate VDM/WOW
   SLDF_HAS_LOGO3ID         = 0x00000800,   // this link is a special Logo3/MSICD link
   SLDF_HAS_DARWINID        = 0x00001000,   // this link is a special Darwin link
   SLDF_RUNAS_USER          = 0x00002000,   // Run this link as a different user
   SLDF_HAS_EXP_ICON_SZ     = 0x00004000    // contains expandable env string for icon path
} SHELL_LINK_DATA_FLAGS;

//
// We conditionally define it here to break the circular dependency between
// shlobj and shlwapi.
//
#ifndef __DATABLOCKHEADER_DEFINED
#define __DATABLOCKHEADER_DEFINED
typedef struct tagDATABLOCKHEADER
{
    DWORD   cbSize;             // Size of this extra data block
    DWORD   dwSignature;        // signature of this extra data block
} DATABLOCK_HEADER, *LPDATABLOCK_HEADER, *LPDBLIST;
#endif

typedef struct {
#ifdef __cplusplus
    DATABLOCK_HEADER dbh;
#else
    DATABLOCK_HEADER;
#endif
    WORD     wFillAttribute;         // fill attribute for console
    WORD     wPopupFillAttribute;    // fill attribute for console popups
    COORD    dwScreenBufferSize;     // screen buffer size for console
    COORD    dwWindowSize;           // window size for console
    COORD    dwWindowOrigin;         // window origin for console
    DWORD    nFont;
    DWORD    nInputBufferSize;
    COORD    dwFontSize;
    UINT     uFontFamily;
    UINT     uFontWeight;
    WCHAR    FaceName[LF_FACESIZE];
    UINT     uCursorSize;
    BOOL     bFullScreen;
    BOOL     bQuickEdit;
    BOOL     bInsertMode;
    BOOL     bAutoPosition;
    UINT     uHistoryBufferSize;
    UINT     uNumberOfHistoryBuffers;
    BOOL     bHistoryNoDup;
    COLORREF ColorTable[ 16 ];
} NT_CONSOLE_PROPS, *LPNT_CONSOLE_PROPS;
#define NT_CONSOLE_PROPS_SIG 0xA0000002

// This is a FE Console property
typedef struct {
#ifdef __cplusplus
    DATABLOCK_HEADER dbh;
#else
    DATABLOCK_HEADER;
#endif
    UINT     uCodePage;
} NT_FE_CONSOLE_PROPS, *LPNT_FE_CONSOLE_PROPS;
#define NT_FE_CONSOLE_PROPS_SIG 0xA0000004

#if (_WIN32_IE >= 0x0500)
typedef struct {
#ifdef __cplusplus
    DATABLOCK_HEADER dbh;
#else
    DATABLOCK_HEADER;
#endif
    CHAR        szDarwinID[MAX_PATH];  // ANSI darwin ID associated with link
    WCHAR       szwDarwinID[MAX_PATH]; // UNICODE darwin ID associated with link
} EXP_DARWIN_LINK, *LPEXP_DARWIN_LINK;
#define EXP_DARWIN_ID_SIG       0xA0000006
#define EXP_LOGO3_ID_SIG        0xA0000007
#endif

#define EXP_SPECIAL_FOLDER_SIG         0xA0000005   // LPEXP_SPECIAL_FOLDER


typedef struct
{
    DWORD       cbSize;             // Size of this extra data block
    DWORD       dwSignature;        // signature of this extra data block
    DWORD       idSpecialFolder;    // special folder id this link points into
    DWORD       cbOffset;           // ofset into pidl from SLDF_HAS_ID_LIST for child
} EXP_SPECIAL_FOLDER, *LPEXP_SPECIAL_FOLDER;



typedef struct
{
    DWORD       cbSize;             // Size of this extra data block
    DWORD       dwSignature;        // signature of this extra data block
    CHAR        szTarget[ MAX_PATH ];   // ANSI target name w/EXP_SZ in it
    WCHAR       swzTarget[ MAX_PATH ];  // UNICODE target name w/EXP_SZ in it
} EXP_SZ_LINK, *LPEXP_SZ_LINK;
#define EXP_SZ_LINK_SIG                0xA0000001   // LPEXP_SZ_LINK (target)
#define EXP_SZ_ICON_SIG                0xA0000007   // LPEXP_SZ_LINK (icon)

#undef  INTERFACE
#define INTERFACE IShellLinkDataList

DECLARE_INTERFACE_(IShellLinkDataList, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IShellLinkDataList methods ***
    STDMETHOD(AddDataBlock)(THIS_ void * pDataBlock) PURE;
    STDMETHOD(CopyDataBlock)(THIS_ DWORD dwSig, void **ppDataBlock) PURE;
    STDMETHOD(RemoveDataBlock)(THIS_ DWORD dwSig) PURE;
    STDMETHOD(GetFlags)(THIS_ DWORD *pdwFlags) PURE;
    STDMETHOD(SetFlags)(THIS_ DWORD dwFlags) PURE;
};

#endif // (_WIN32_IE >= 0x0400)

#if (_WIN32_IE >= 0x0500)
#undef  INTERFACE
#define INTERFACE IResolveShellLink

DECLARE_INTERFACE_(IResolveShellLink, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IResolveShellLink methods ***
    STDMETHOD(ResolveShellLink)(THIS_ IUnknown* punk, HWND hwnd, DWORD fFlags) PURE;
};
#endif // (_WIN32_IE >= 0x0500)


#ifdef _INC_SHELLAPI    /* for LPSHELLEXECUTEINFO */
//===========================================================================
//
// IShellExecuteHook Interface
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IShellExecuteHookA

DECLARE_INTERFACE_(IShellExecuteHookA, IUnknown) // shexhk
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IShellExecuteHookA methods ***
    STDMETHOD(Execute)(THIS_ LPSHELLEXECUTEINFOA pei) PURE;
};

#undef  INTERFACE
#define INTERFACE   IShellExecuteHookW

DECLARE_INTERFACE_(IShellExecuteHookW, IUnknown) // shexhk
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IShellExecuteHookW methods ***
    STDMETHOD(Execute)(THIS_ LPSHELLEXECUTEINFOW pei) PURE;
};

#ifdef UNICODE
#define IShellExecuteHook       IShellExecuteHookW
#define IShellExecuteHookVtbl   IShellExecuteHookWVtbl
#else
#define IShellExecuteHook       IShellExecuteHookA
#define IShellExecuteHookVtbl   IShellExecuteHookAVtbl
#endif
#endif

//===========================================================================
//
// IURLSearchHook Interface
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IURLSearchHook

DECLARE_INTERFACE_(IURLSearchHook, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IURLSearchHook methods ***
    STDMETHOD(Translate)(THIS_ LPWSTR lpwszSearchURL, DWORD cchBufferSize) PURE;
};

//===========================================================================
//
// INewShortcutHook Interface
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   INewShortcutHookA

DECLARE_INTERFACE_(INewShortcutHookA, IUnknown) // nshhk
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** INewShortcutHook methods ***
    STDMETHOD(SetReferent)(THIS_ LPCSTR pcszReferent, HWND hwnd) PURE;
    STDMETHOD(GetReferent)(THIS_ LPSTR pszReferent, int cchReferent) PURE;
    STDMETHOD(SetFolder)(THIS_ LPCSTR pcszFolder) PURE;
    STDMETHOD(GetFolder)(THIS_ LPSTR pszFolder, int cchFolder) PURE;
    STDMETHOD(GetName)(THIS_ LPSTR pszName, int cchName) PURE;
    STDMETHOD(GetExtension)(THIS_ LPSTR pszExtension, int cchExtension) PURE;
};

#undef  INTERFACE
#define INTERFACE   INewShortcutHookW

DECLARE_INTERFACE_(INewShortcutHookW, IUnknown) // nshhk
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** INewShortcutHook methods ***
    STDMETHOD(SetReferent)(THIS_ LPCWSTR pcszReferent, HWND hwnd) PURE;
    STDMETHOD(GetReferent)(THIS_ LPWSTR pszReferent, int cchReferent) PURE;
    STDMETHOD(SetFolder)(THIS_ LPCWSTR pcszFolder) PURE;
    STDMETHOD(GetFolder)(THIS_ LPWSTR pszFolder, int cchFolder) PURE;
    STDMETHOD(GetName)(THIS_ LPWSTR pszName, int cchName) PURE;
    STDMETHOD(GetExtension)(THIS_ LPWSTR pszExtension, int cchExtension) PURE;
};

#ifdef UNICODE
#define INewShortcutHook        INewShortcutHookW
#define INewShortcutHookVtbl    INewShortcutHookWVtbl
#else
#define INewShortcutHook        INewShortcutHookA
#define INewShortcutHookVtbl    INewShortcutHookAVtbl
#endif

//===========================================================================
//
// ICopyHook Interface
//
//  The copy hook is called whenever file system directories are
//  copy/moved/deleted/renamed via the shell.  It is also called by the shell
//  on changes of status of printers.
//
//  Clients register their id under STRREG_SHEX_COPYHOOK for file system hooks
//  and STRREG_SHEx_PRNCOPYHOOK for printer hooks.
//  the CopyCallback is called prior to the action, so the hook has the chance
//  to allow, deny or cancel the operation by returning the falues:
//     IDYES  -  means allow the operation
//     IDNO   -  means disallow the operation on this file, but continue with
//              any other operations (eg. batch copy)
//     IDCANCEL - means disallow the current operation and cancel any pending
//              operations
//
//   arguments to the CopyCallback
//      hwnd - window to use for any UI
//      wFunc - what operation is being done
//      wFlags - and flags (FOF_*) set in the initial call to the file operation
//      pszSrcFile - name of the source file
//      dwSrcAttribs - file attributes of the source file
//      pszDestFile - name of the destiation file (for move and renames)
//      dwDestAttribs - file attributes of the destination file
//
//
//===========================================================================

#ifndef FO_MOVE //these need to be kept in sync with the ones in shellapi.h

// file operations

#define FO_MOVE           0x0001
#define FO_COPY           0x0002
#define FO_DELETE         0x0003
#define FO_RENAME         0x0004

#define FOF_MULTIDESTFILES         0x0001
#define FOF_CONFIRMMOUSE           0x0002
#define FOF_SILENT                 0x0004  // don't create progress/report
#define FOF_RENAMEONCOLLISION      0x0008
#define FOF_NOCONFIRMATION         0x0010  // Don't prompt the user.
#define FOF_WANTMAPPINGHANDLE      0x0020  // Fill in SHFILEOPSTRUCT.hNameMappings
                                      // Must be freed using SHFreeNameMappings
#define FOF_ALLOWUNDO              0x0040
#define FOF_FILESONLY              0x0080  // on *.*, do only files
#define FOF_SIMPLEPROGRESS         0x0100  // means don't show names of files
#define FOF_NOCONFIRMMKDIR         0x0200  // don't confirm making any needed dirs
#define FOF_NOERRORUI              0x0400  // don't put up error UI
#define FOF_NOCOPYSECURITYATTRIBS  0x0800  // dont copy NT file Security Attributes
#define FOF_NORECURSION            0x1000  // don't recurse into directories.
#if (_WIN32_IE >= 0x500)
#define FOF_NO_CONNECTED_ELEMENTS  0x2000  // don't operate on connected file elements.
#define FOF_WANTNUKEWARNING        0x4000  // during delete operation, warn if nuking instead of recycling (partially overrides FOF_NOCONFIRMATION)
#endif // _WIN32_IE >= 0x500

typedef WORD FILEOP_FLAGS;

// printer operations

#define PO_DELETE       0x0013  // printer is being deleted
#define PO_RENAME       0x0014  // printer is being renamed
#define PO_PORTCHANGE   0x0020  // port this printer connected to is being changed
                                // if this id is set, the strings received by
                                // the copyhook are a doubly-null terminated
                                // list of strings.  The first is the printer
                                // name and the second is the printer port.
#define PO_REN_PORT     0x0034  // PO_RENAME and PO_PORTCHANGE at same time.

// no POF_ flags currently defined

typedef UINT PRINTEROP_FLAGS;

#endif // FO_MOVE

#undef  INTERFACE
#define INTERFACE   ICopyHookA

DECLARE_INTERFACE_(ICopyHookA, IUnknown)        // sl
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ICopyHook methods ***
    STDMETHOD_(UINT,CopyCallback) (THIS_ HWND hwnd, UINT wFunc, UINT wFlags, LPCSTR pszSrcFile, DWORD dwSrcAttribs,
                                   LPCSTR pszDestFile, DWORD dwDestAttribs) PURE;
};

typedef ICopyHookA *    LPCOPYHOOKA;

#undef  INTERFACE
#define INTERFACE   ICopyHookW

DECLARE_INTERFACE_(ICopyHookW, IUnknown)        // sl
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ICopyHook methods ***
    STDMETHOD_(UINT,CopyCallback) (THIS_ HWND hwnd, UINT wFunc, UINT wFlags, LPCWSTR pszSrcFile, DWORD dwSrcAttribs,
                                   LPCWSTR pszDestFile, DWORD dwDestAttribs) PURE;
};

typedef ICopyHookW *    LPCOPYHOOKW;

#ifdef UNICODE
#define ICopyHook       ICopyHookW
#define ICopyHookVtbl   ICopyHookWVtbl
#define LPCOPYHOOK      LPCOPYHOOKW
#else
#define ICopyHook       ICopyHookA
#define ICopyHookVtbl   ICopyHookAVtbl
#define LPCOPYHOOK      LPCOPYHOOKA
#endif

//===========================================================================
//
// IFileViewerSite Interface
//
//===========================================================================

#undef  INTERFACE
#define INTERFACE   IFileViewerSite

DECLARE_INTERFACE_(IFileViewerSite, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IFileViewerSite methods ***
    STDMETHOD(SetPinnedWindow) (THIS_ HWND hwnd) PURE;
    STDMETHOD(GetPinnedWindow) (THIS_ HWND *phwnd) PURE;
};

typedef IFileViewerSite * LPFILEVIEWERSITE;


//===========================================================================
//
// IFileViewer Interface
//
// Implemented in a FileViewer component object.  Used to tell a
// FileViewer to PrintTo or to view, the latter happening though
// ShowInitialize and Show.  The filename is always given to the
// viewer through IPersistFile.
//
//===========================================================================

typedef struct
{
    // Stuff passed into viewer (in)
    DWORD cbSize;           // Size of structure for future expansion...
    HWND hwndOwner;         // who is the owner window.
    int iShow;              // The show command

    // Passed in and updated  (in/Out)
    DWORD dwFlags;          // flags
    RECT rect;              // Where to create the window may have defaults
    IUnknown *punkRel;      // Relese this interface when window is visible

    // Stuff that might be returned from viewer (out)
    OLECHAR strNewFile[MAX_PATH];   // New File to view.

} FVSHOWINFO, *LPFVSHOWINFO;

    // Define File View Show Info Flags.
#define FVSIF_RECT      0x00000001      // The rect variable has valid data.
#define FVSIF_PINNED    0x00000002      // We should Initialize pinned

#define FVSIF_NEWFAILED 0x08000000      // The new file passed back failed
                                        // to be viewed.

#define FVSIF_NEWFILE   0x80000000      // A new file to view has been returned
#define FVSIF_CANVIEWIT 0x40000000      // The viewer can view it.

#undef  INTERFACE
#define INTERFACE   IFileViewerA

DECLARE_INTERFACE(IFileViewerA)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IFileViewer methods ***
    STDMETHOD(ShowInitialize) (THIS_ LPFILEVIEWERSITE lpfsi) PURE;
    STDMETHOD(Show) (THIS_ LPFVSHOWINFO pvsi) PURE;
    STDMETHOD(PrintTo) (THIS_ LPSTR pszDriver, BOOL fSuppressUI) PURE;
};

typedef IFileViewerA * LPFILEVIEWERA;

#undef  INTERFACE
#define INTERFACE   IFileViewerW

DECLARE_INTERFACE(IFileViewerW)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IFileViewer methods ***
    STDMETHOD(ShowInitialize) (THIS_ LPFILEVIEWERSITE lpfsi) PURE;
    STDMETHOD(Show) (THIS_ LPFVSHOWINFO pvsi) PURE;
    STDMETHOD(PrintTo) (THIS_ LPWSTR pszDriver, BOOL fSuppressUI) PURE;
};

typedef IFileViewerW * LPFILEVIEWERW;

#ifdef UNICODE
#define IFileViewer IFileViewerW
#define LPFILEVIEWER LPFILEVIEWERW
#else
#define IFileViewer IFileViewerA
#define LPFILEVIEWER LPFILEVIEWERA
#endif



//==========================================================================
//
// IShellBrowser/IShellView/IShellFolder interface
//
//  These three interfaces are used when the shell communicates with
// name space extensions. The shell (explorer) provides IShellBrowser
// interface, and extensions implements IShellFolder and IShellView
// interfaces.
//
//==========================================================================


//--------------------------------------------------------------------------
//
// Command/menuitem IDs
//
//  The explorer dispatches WM_COMMAND messages based on the range of
// command/menuitem IDs. All the IDs of menuitems that the view (right
// pane) inserts must be in FCIDM_SHVIEWFIRST/LAST (otherwise, the explorer
// won't dispatch them). The view should not deal with any menuitems
// in FCIDM_BROWSERFIRST/LAST (otherwise, it won't work with the future
// version of the shell).
//
//  FCIDM_SHVIEWFIRST/LAST      for the right pane (IShellView)
//  FCIDM_BROWSERFIRST/LAST     for the explorer frame (IShellBrowser)
//  FCIDM_GLOBAL/LAST           for the explorer's submenu IDs
//
//--------------------------------------------------------------------------

#define FCIDM_SHVIEWFIRST           0x0000
#define FCIDM_SHVIEWLAST            0x7fff
#define FCIDM_BROWSERFIRST          0xa000
#define FCIDM_BROWSERLAST           0xbf00
#define FCIDM_GLOBALFIRST           0x8000
#define FCIDM_GLOBALLAST            0x9fff

//
// Global submenu IDs and separator IDs
//
#define FCIDM_MENU_FILE             (FCIDM_GLOBALFIRST+0x0000)
#define FCIDM_MENU_EDIT             (FCIDM_GLOBALFIRST+0x0040)
#define FCIDM_MENU_VIEW             (FCIDM_GLOBALFIRST+0x0080)
#define FCIDM_MENU_VIEW_SEP_OPTIONS (FCIDM_GLOBALFIRST+0x0081)
#define FCIDM_MENU_TOOLS            (FCIDM_GLOBALFIRST+0x00c0) // for Win9x compat
#define FCIDM_MENU_TOOLS_SEP_GOTO   (FCIDM_GLOBALFIRST+0x00c1) // for Win9x compat
#define FCIDM_MENU_HELP             (FCIDM_GLOBALFIRST+0x0100)
#define FCIDM_MENU_FIND             (FCIDM_GLOBALFIRST+0x0140)
#define FCIDM_MENU_EXPLORE          (FCIDM_GLOBALFIRST+0x0150)
#define FCIDM_MENU_FAVORITES        (FCIDM_GLOBALFIRST+0x0170)

//--------------------------------------------------------------------------
// control IDs known to the view
//--------------------------------------------------------------------------

#define FCIDM_TOOLBAR      (FCIDM_BROWSERFIRST + 0)
#define FCIDM_STATUS       (FCIDM_BROWSERFIRST + 1)

#if (_WIN32_IE >= 0x0400)
//--------------------------------------------------------------------------
//
// The resource id of the offline cursor
// This cursor is avaialble in shdocvw.dll
#define IDC_OFFLINE_HAND        103
//
//--------------------------------------------------------------------------
#endif

//--------------------------------------------------------------------------
//
// FOLDERSETTINGS
//
//  FOLDERSETTINGS is a data structure that explorer passes from one folder
// view to another, when the user is browsing. It calls ISV::GetCurrentInfo
// member to get the current settings and pass it to ISV::CreateViewWindow
// to allow the next folder view "inherit" it. These settings assumes a
// particular UI (which the shell's folder view has), and shell extensions
// may or may not use those settings.
//
//--------------------------------------------------------------------------

typedef LPBYTE LPVIEWSETTINGS;

// NB Bitfields.
// FWF_DESKTOP implies FWF_TRANSPARENT/NOCLIENTEDGE/NOSCROLL
typedef enum
    {
    FWF_AUTOARRANGE =       0x0001,
    FWF_ABBREVIATEDNAMES =  0x0002,
    FWF_SNAPTOGRID =        0x0004,
    FWF_OWNERDATA =         0x0008,
    FWF_BESTFITWINDOW =     0x0010,
    FWF_DESKTOP =           0x0020,
    FWF_SINGLESEL =         0x0040,
    FWF_NOSUBFOLDERS =      0x0080,
    FWF_TRANSPARENT  =      0x0100,
    FWF_NOCLIENTEDGE =      0x0200,
    FWF_NOSCROLL     =      0x0400,
    FWF_ALIGNLEFT    =      0x0800,
    FWF_NOICONS      =      0x1000,
    FWF_SHOWSELALWAYS =     0x2000,
    FWF_NOVISIBLE    =      0X4000,
    FWF_SINGLECLICKACTIVATE=0x8000  // TEMPORARY -- NO UI FOR THIS
    } FOLDERFLAGS;

typedef enum
    {
    FVM_ICON =              1,
    FVM_SMALLICON =         2,
    FVM_LIST =              3,
    FVM_DETAILS =           4,
    } FOLDERVIEWMODE;

typedef struct
    {
    UINT ViewMode;       // View mode (FOLDERVIEWMODE values)
    UINT fFlags;         // View options (FOLDERFLAGS bits)
    } FOLDERSETTINGS, *LPFOLDERSETTINGS;

typedef const FOLDERSETTINGS * LPCFOLDERSETTINGS;

//--------------------------------------------------------------------------
//
// Interface:   IShellBrowser
//
//  IShellBrowser interface is the interface that is provided by the shell
// explorer/folder frame window. When it creates the "contents pane" of
// a shell folder (which provides IShellFolder interface), it calls its
// CreateViewObject member function to create an IShellView object. Then,
// it calls its CreateViewWindow member to create the "contents pane"
// window. The pointer to the IShellBrowser interface is passed to
// the IShellView object as a parameter to this CreateViewWindow member
// function call.
//
//    +--------------------------+  <-- Explorer window
//    | [] Explorer              |
//    |--------------------------+       IShellBrowser
//    | File Edit View ..        |
//    |--------------------------|
//    |        |                 |
//    |        |              <-------- Content pane
//    |        |                 |
//    |        |                 |       IShellView
//    |        |                 |
//    |        |                 |
//    +--------------------------+
//
//
//
// [Member functions]
//
//
// IShellBrowser::GetWindow(phwnd)
//
//   Inherited from IOleWindow::GetWindow.
//
//
// IShellBrowser::ContextSensitiveHelp(fEnterMode)
//
//   Inherited from IOleWindow::ContextSensitiveHelp.
//
//
// IShellBrowser::InsertMenusSB(hmenuShared, lpMenuWidths)
//
//   Similar to the IOleInPlaceFrame::InsertMenus. The explorer will put
//  "File" and "Edit" pulldown in the File menu group, "View" and "Tools"
//  in the Container menu group and "Help" in the Window menu group. Each
//  pulldown menu will have a uniqu ID, FCIDM_MENU_FILE/EDIT/VIEW/TOOLS/HELP.
//  The view is allowed to insert menuitems into those sub-menus by those
//  IDs must be between FCIDM_SHVIEWFIRST and FCIDM_SHVIEWLAST.
//
//
// IShellBrowser::SetMenuSB(hmenuShared, holemenu, hwndActiveObject)
//
//   Similar to the IOleInPlaceFrame::SetMenu. The explorer ignores the
//  holemenu parameter (reserved for future enhancement)  and performs
//  menu-dispatch based on the menuitem IDs (see the description above).
//  It is important to note that the explorer will add different
//  set of menuitems depending on whether the view has a focus or not.
//  Therefore, it is very important to call ISB::OnViewWindowActivate
//  whenever the view window (or its children) gets the focus.
//
//
// IShellBrowser::RemoveMenusSB(hmenuShared)
//
//   Same as the IOleInPlaceFrame::RemoveMenus.
//
//
// IShellBrowser::SetStatusTextSB(pszStatusText)
//
//   Same as the IOleInPlaceFrame::SetStatusText. It is also possible to
//  send messages directly to the status window via SendControlMsg.
//
//
// IShellBrowser::EnableModelessSB(fEnable)
//
//   Same as the IOleInPlaceFrame::EnableModeless.
//
//
// IShellBrowser::TranslateAcceleratorSB(lpmsg, wID)
//
//   Same as the IOleInPlaceFrame::TranslateAccelerator, but will be
//  never called because we don't support EXEs (i.e., the explorer has
//  the message loop). This member function is defined here for possible
//  future enhancement.
//
//
// IShellBrowser::BrowseObject(pidl, wFlags)
//
//   The view calls this member to let shell explorer browse to another
//  folder. The pidl and wFlags specifies the folder to be browsed.
//
//  Following three flags specifies whether it creates another window or not.
//   SBSP_SAMEBROWSER  -- Browse to another folder with the same window.
//   SBSP_NEWBROWSER   -- Creates another window for the specified folder.
//   SBSP_DEFBROWSER   -- Default behavior (respects the view option).
//
//  Following three flags specifies open, explore, or default mode. These   .
//  are ignored if SBSP_SAMEBROWSER or (SBSP_DEFBROWSER && (single window   .
//  browser || explorer)).                                                  .
//   SBSP_OPENMODE     -- Use a normal folder window
//   SBSP_EXPLOREMODE  -- Use an explorer window
//   SBSP_DEFMODE      -- Use the same as the current window
//
//  Following three flags specifies the pidl.
//   SBSP_ABSOLUTE -- pidl is an absolute pidl (relative from desktop)
//   SBSP_RELATIVE -- pidl is relative from the current folder.
//   SBSP_PARENT   -- Browse the parent folder (ignores the pidl)
//   SBSP_NAVIGATEBACK    -- Navigate back (ignores the pidl)
//   SBSP_NAVIGATEFORWARD -- Navigate forward (ignores the pidl)
//
//  Following two flags control history manipulation as result of navigate
//   SBSP_WRITENOHISTORY -- write no history (shell folder) entry
//   SBSP_NOAUTOSELECT -- suppress selection in history pane
//
// IShellBrowser::GetViewStateStream(grfMode, ppstm)
//
//   The browser returns an IStream interface as the storage for view
//  specific state information.
//
//   grfMode -- Specifies the read/write access (STGM_READ/WRITE/READWRITE)
//   ppstm   -- Specifies the IStream *variable to be filled.
//
//
// IShellBrowser::GetControlWindow(id, phwnd)
//
//   The shell view may call this member function to get the window handle
//  of Explorer controls (toolbar or status winodw -- FCW_TOOLBAR or
//  FCW_STATUS).
//
//
// IShellBrowser::SendControlMsg(id, uMsg, wParam, lParam, pret)
//
//   The shell view calls this member function to send control messages to
//  one of Explorer controls (toolbar or status window -- FCW_TOOLBAR or
//  FCW_STATUS).
//
//
// IShellBrowser::QueryActiveShellView(IShellView * ppshv)
//
//   This member returns currently activated (displayed) shellview object.
//  A shellview never need to call this member function.
//
//
// IShellBrowser::OnViewWindowActive(pshv)
//
//   The shell view window calls this member function when the view window
//  (or one of its children) got the focus. It MUST call this member before
//  calling IShellBrowser::InsertMenus, because it will insert different
//  set of menu items depending on whether the view has the focus or not.
//
//
// IShellBrowser::SetToolbarItems(lpButtons, nButtons, uFlags)
//
//   The view calls this function to add toolbar items to the exporer's
//  toolbar. "lpButtons" and "nButtons" specifies the array of toolbar
//  items. "uFlags" must be one of FCT_MERGE, FCT_CONFIGABLE, FCT_ADDTOEND.
//
//-------------------------------------------------------------------------

//
// Values for wFlags parameter of ISB::BrowseObject() member.
//
#define SBSP_DEFBROWSER         0x0000
#define SBSP_SAMEBROWSER        0x0001
#define SBSP_NEWBROWSER         0x0002

#define SBSP_DEFMODE            0x0000
#define SBSP_OPENMODE           0x0010
#define SBSP_EXPLOREMODE        0x0020
#define SBSP_HELPMODE           0x0040 // IEUNIX : Help window uses this.
#define SBSP_NOTRANSFERHIST     0x0080 // IEUNIX only mode.

#define SBSP_ABSOLUTE           0x0000
#define SBSP_RELATIVE           0x1000
#define SBSP_PARENT             0x2000
#define SBSP_NAVIGATEBACK       0x4000
#define SBSP_NAVIGATEFORWARD    0x8000

#define SBSP_ALLOW_AUTONAVIGATE 0x10000

#define SBSP_INITIATEDBYHLINKFRAME        0x80000000
#define SBSP_REDIRECT                     0x40000000

#define SBSP_WRITENOHISTORY     0x08000000
#define SBSP_NOAUTOSELECT       0x04000000


//
// Values for id parameter of ISB::GetWindow/SendControlMsg members.
//
// WARNING:
//  Any shell extensions which sends messages to those control windows
// might not work in the future version of windows. If you really need
// to send messages to them, (1) don't assume that those control window
// always exist (i.e. GetControlWindow may fail) and (2) verify the window
// class of the window before sending any messages.
//
#define FCW_STATUS      0x0001
#define FCW_TOOLBAR     0x0002
#define FCW_TREE        0x0003
#define FCW_INTERNETBAR 0x0006
#define FCW_PROGRESS    0x0008

#if (_WIN32_IE >= 0x0400)
#endif

//
// Values for uFlags paremeter of ISB::SetToolbarItems member.
//
#define FCT_MERGE       0x0001
#define FCT_CONFIGABLE  0x0002
#define FCT_ADDTOEND    0x0004


#undef  INTERFACE
#define INTERFACE   IShellBrowser

DECLARE_INTERFACE_(IShellBrowser, IOleWindow)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IOleWindow methods ***
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

    // *** IShellBrowser methods *** (same as IOleInPlaceFrame)
    STDMETHOD(InsertMenusSB)(THIS_ HMENU hmenuShared, LPOLEMENUGROUPWIDTHS lpMenuWidths) PURE;
    STDMETHOD(SetMenuSB)(THIS_ HMENU hmenuShared, HOLEMENU holemenuRes, HWND hwndActiveObject) PURE;
    STDMETHOD(RemoveMenusSB) (THIS_ HMENU hmenuShared) PURE;
    STDMETHOD(SetStatusTextSB) (THIS_ LPCOLESTR pszStatusText) PURE;
    STDMETHOD(EnableModelessSB) (THIS_ BOOL fEnable) PURE;
    STDMETHOD(TranslateAcceleratorSB) (THIS_ MSG *pmsg, WORD wID) PURE;

    // *** IShellBrowser methods ***
    STDMETHOD(BrowseObject)(THIS_ LPCITEMIDLIST pidl, UINT wFlags) PURE;
    STDMETHOD(GetViewStateStream)(THIS_ DWORD grfMode, IStream **ppStrm) PURE;
    STDMETHOD(GetControlWindow)(THIS_ UINT id, HWND * lphwnd) PURE;
    STDMETHOD(SendControlMsg)(THIS_ UINT id, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pret) PURE;
    STDMETHOD(QueryActiveShellView)(THIS_ struct IShellView **ppshv) PURE;
    STDMETHOD(OnViewWindowActive)(THIS_ struct IShellView *ppshv) PURE;
    STDMETHOD(SetToolbarItems)(THIS_ LPTBBUTTON lpButtons, UINT nButtons, UINT uFlags) PURE;
};
#define __IShellBrowser_INTERFACE_DEFINED__

typedef IShellBrowser * LPSHELLBROWSER;

enum {
    SBSC_HIDE = 0,
    SBSC_SHOW = 1,
    SBSC_TOGGLE = 2,
    SBSC_QUERY =  3
};

enum {
        SBO_DEFAULT = 0 ,
        SBO_NOBROWSERPAGES = 1
};


//-------------------------------------------------------------------------
// ICommDlgBrowser interface
//
//  ICommDlgBrowser interface is the interface that is provided by the new
// common dialog window to hook and modify the behavior of IShellView.  When
// a default view is created, it queries its parent IShellBrowser for the
// ICommDlgBrowser interface.  If supported, it calls out to that interface
// in several cases that need to behave differently in a dialog.
//
// Member functions:
//
//  ICommDlgBrowser::OnDefaultCommand()
//    Called when the user double-clicks in the view or presses Enter.  The
//   browser should return S_OK if it processed the action itself, S_FALSE
//   to let the view perform the default action.
//
//  ICommDlgBrowser::OnStateChange(ULONG uChange)
//    Called when some states in the view change.  'uChange' is one of the
//   CDBOSC_* values.  This call is made after the state (selection, focus,
//   etc) has changed.  There is no return value.
//
//  ICommDlgBrowser::IncludeObject(LPCITEMIDLIST pidl)
//    Called when the view is enumerating objects.  'pidl' is a relative
//   IDLIST.  The browser should return S_OK to include the object in the
//   view, S_FALSE to hide it
//
//-------------------------------------------------------------------------

#define CDBOSC_SETFOCUS     0x00000000
#define CDBOSC_KILLFOCUS    0x00000001
#define CDBOSC_SELCHANGE    0x00000002
#define CDBOSC_RENAME       0x00000003

#undef  INTERFACE
#define INTERFACE   ICommDlgBrowser

DECLARE_INTERFACE_(ICommDlgBrowser, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ICommDlgBrowser methods ***
    STDMETHOD(OnDefaultCommand) (THIS_ struct IShellView * ppshv) PURE;
    STDMETHOD(OnStateChange) (THIS_ struct IShellView * ppshv,
                ULONG uChange) PURE;
    STDMETHOD(IncludeObject) (THIS_ struct IShellView * ppshv,
                LPCITEMIDLIST pidl) PURE;
};

typedef ICommDlgBrowser * LPCOMMDLGBROWSER;

//-------------------------------------------------------------------------
// ICommDlgBrowser2 interface
//
// Member functions:
//
//  ICommDlgBrowser2::Notify(IShellView *pshv, DWORD dwNotfyType)
//   Called when the view is wants to notify common dialog when an event
//  occurrs.
//
//  CDB2N_CONTEXTMENU_START indicates the context menu has started.
//  CDB2N_CONTEXTMENU_DONE  indicates the context menu has completed.
//
//  ICommDlgBrowser2::GetDefaultMenuText(IShellView *pshv,
//                                      WCHAR *pszText, INT cchMax)
//   Called when the view wants to get the default context menu text.
//  pszText points to buffer and cchMax specifies the size of the
//  buffer in characters.  The browser on return has filled the buffer
//  with the default context menu text.  The Shell will call this method
//  with at least a buffer size of MAX_PATH.  The browser should return
//  S_OK if it returned a new default menu text, S_FALSE to let the view
//  to use the normal default menu text.
//
//  ICommDlgBrowser2::GetViewFlags(DWORD *pdwFlags)
//     Called when the view wants to determine  if special customization needs to
//    be done for the common dialog browser. For example View calls this function to
//    determin if all files(hidden and system)needs to be shown. If the GetViewFlags returns a DWORD with
//    CDB2GVF_SHOWALLFILES  flag set then it will show all the files.
//-------------------------------------------------------------------------

#define CDB2N_CONTEXTMENU_DONE  0x00000001
#define CDB2N_CONTEXTMENU_START 0x00000002

//GetViewFlags
#define CDB2GVF_SHOWALLFILES        0x00000001

#undef  INTERFACE
#define INTERFACE   ICommDlgBrowser2

DECLARE_INTERFACE_(ICommDlgBrowser2, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** ICommDlgBrowser methods ***
    STDMETHOD(OnDefaultCommand) (THIS_ struct IShellView * ppshv) PURE;
    STDMETHOD(OnStateChange) (THIS_ struct IShellView * ppshv,
                ULONG uChange) PURE;
    STDMETHOD(IncludeObject) (THIS_ struct IShellView * ppshv,
                LPCITEMIDLIST pidl) PURE;

    // *** ICommDlgBrowser2 methods ***
    STDMETHOD(Notify) (THIS_ struct IShellView * ppshv,
                DWORD dwNotifyType) PURE;
    STDMETHOD(GetDefaultMenuText) (THIS_ struct IShellView * ppshv,
                WCHAR *pszText, INT cchMax) PURE;
    STDMETHOD(GetViewFlags)(THIS_ DWORD *pdwFlags) PURE;

};

typedef ICommDlgBrowser2 * LPCOMMDLGBROWSER2;


//==========================================================================
//
// Interface:   IShellView
//
// IShellView::GetWindow(phwnd)
//
//   Inherited from IOleWindow::GetWindow.
//
//
// IShellView::ContextSensitiveHelp(fEnterMode)
//
//   Inherited from IOleWindow::ContextSensitiveHelp.
//
//
// IShellView::TranslateAccelerator(lpmsg)
//
//   Similar to IOleInPlaceActiveObject::TranlateAccelerator. The explorer
//  calls this function BEFORE any other translation. Returning S_OK
//  indicates that the message was translated (eaten) and should not be
//  translated or dispatched by the explorer.
//
//
// IShellView::EnableModeless(fEnable)
//   Similar to IOleInPlaceActiveObject::EnableModeless.
//
//
// IShellView::UIActivate(uState)
//
//   The explorer calls this member function whenever the activation
//  state of the view window is changed by a certain event that is
//  NOT caused by the shell view itself.
//
//   SVUIA_DEACTIVATE will be passed when the explorer is about to
//  destroy the shell view window; the shell view is supposed to remove
//  all the extended UIs (typically merged menu and modeless popup windows).
//
//   SVUIA_ACTIVATE_NOFOCUS will be passsed when the shell view is losing
//  the input focus or the shell view has been just created without the
//  input focus; the shell view is supposed to set menuitems appropriate
//  for non-focused state (no selection specific items should be added).
//
//   SVUIA_ACTIVATE_FOCUS will be passed when the explorer has just
//  created the view window with the input focus; the shell view is
//  supposed to set menuitems appropriate for focused state.
//
//   SVUIA_INPLACEACTIVATE(new) will be passed when the shell view is opened
//  within an ActiveX control, which is not a UI active. In this case,
//  the shell view should not merge menus or put toolbas. To be compatible
//  with Win95 client, we don't pass this value unless the view supports
//  IShellView2.
//
//   The shell view should not change focus within this member function.
//  The shell view should not hook the WM_KILLFOCUS message to remerge
//  menuitems. However, the shell view typically hook the WM_SETFOCUS
//  message, and re-merge the menu after calling IShellBrowser::
//  OnViewWindowActivated.
//
//   One of the ACTIVATE / INPLACEACTIVATE messages will be sent when
//  the view window becomes the currently displayed view.  On Win95 systems,
//  this will happen immediately after the CreateViewWindow call.  On IE4, Win98,
//  and NT5 systems this may happen when the view reports it is ready (if the
//  IShellView supports async creation).  This can be used as a hint as to when
//  to make your view window visible.  Note: the Win95/Win98/NT4 common dialogs
//  do not send either of these on creation.
//
//
// IShellView::Refresh()
//
//   The explorer calls this member when the view needs to refresh its
//  contents (such as when the user hits F5 key).
//
//
// IShellView::CreateViewWindow
//
//   This member creates the view window (right-pane of the explorer or the
//  client window of the folder window).
//
//
// IShellView::DestroyViewWindow
//
//   This member destroys the view window.
//
//
// IShellView::GetCurrentInfo
//
//   This member returns the folder settings.
//
//
// IShellView::AddPropertySHeetPages
//
//   The explorer calls this member when it is opening the option property
//  sheet. This allows the view to add additional pages to it.
//
//
// IShellView::SaveViewState()
//
//   The explorer calls this member when the shell view is supposed to
//  store its view settings. The shell view is supposed to get a view
//  stream by calling IShellBrowser::GetViewStateStream and store the
//  current view state into that stream.
//
//
// IShellView::SelectItem(pidlItem, uFlags)
//
//   The explorer calls this member to change the selection state of
//  item(s) within the shell view window.  If pidlItem is NULL and uFlags
//  is SVSI_DESELECTOTHERS, all items should be deselected.
//
//-------------------------------------------------------------------------

//
// shellview select item flags
//
#define SVSI_DESELECT   0x0000
#define SVSI_SELECT     0x0001
#define SVSI_EDIT       0x0003  // includes select
#define SVSI_DESELECTOTHERS 0x0004
#define SVSI_ENSUREVISIBLE  0x0008
#define SVSI_FOCUSED        0x0010
#define SVSI_TRANSLATEPT    0x0020

//
// shellview get item object flags
//
#define SVGIO_BACKGROUND    0x00000000
#define SVGIO_SELECTION     0x00000001
#define SVGIO_ALLVIEW       0x00000002

//
// uState values for IShellView::UIActivate
//
typedef enum {
    SVUIA_DEACTIVATE       = 0,
    SVUIA_ACTIVATE_NOFOCUS = 1,
    SVUIA_ACTIVATE_FOCUS   = 2,
    SVUIA_INPLACEACTIVATE  = 3          // new flag for IShellView2
} SVUIA_STATUS;

#undef  INTERFACE
#define INTERFACE   IShellView

DECLARE_INTERFACE_(IShellView, IOleWindow)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IOleWindow methods ***
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

    // *** IShellView methods ***
    STDMETHOD(TranslateAccelerator) (THIS_ LPMSG lpmsg) PURE;
#ifdef _FIX_ENABLEMODELESS_CONFLICT
    STDMETHOD(EnableModelessSV) (THIS_ BOOL fEnable) PURE;
#else
    STDMETHOD(EnableModeless) (THIS_ BOOL fEnable) PURE;
#endif
    STDMETHOD(UIActivate) (THIS_ UINT uState) PURE;
    STDMETHOD(Refresh) (THIS) PURE;

    STDMETHOD(CreateViewWindow)(THIS_ IShellView  *lpPrevView,
                    LPCFOLDERSETTINGS lpfs, IShellBrowser  * psb,
                    RECT * prcView, HWND  *phWnd) PURE;
    STDMETHOD(DestroyViewWindow)(THIS) PURE;
    STDMETHOD(GetCurrentInfo)(THIS_ LPFOLDERSETTINGS lpfs) PURE;
    STDMETHOD(AddPropertySheetPages)(THIS_ DWORD dwReserved,
                    LPFNADDPROPSHEETPAGE lpfn, LPARAM lparam) PURE;
    STDMETHOD(SaveViewState)(THIS) PURE;
    STDMETHOD(SelectItem)(THIS_ LPCITEMIDLIST pidlItem, UINT uFlags) PURE;
    STDMETHOD(GetItemObject)(THIS_ UINT uItem, REFIID riid,
                    void **ppv) PURE;
};

typedef IShellView *LPSHELLVIEW;

typedef GUID SHELLVIEWID;

#define SV2GV_CURRENTVIEW ((UINT)-1)
#define SV2GV_DEFAULTVIEW ((UINT)-2)

#ifndef RC_INVOKED
#include <pshpack8.h>
#endif /* !RC_INVOKED */

typedef struct _SV2CVW2_PARAMS
{
    DWORD cbSize;

    IShellView *psvPrev;
    FOLDERSETTINGS const *pfs;
    IShellBrowser *psbOwner;
    RECT *prcView;
    SHELLVIEWID const *pvid;

    HWND hwndView;
} SV2CVW2_PARAMS;
typedef SV2CVW2_PARAMS *LPSV2CVW2_PARAMS;

#undef  INTERFACE
#define INTERFACE   IShellView2

DECLARE_INTERFACE_(IShellView2, IShellView)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IOleWindow methods ***
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

    // *** IShellView methods ***
    STDMETHOD(TranslateAccelerator) (THIS_ LPMSG lpmsg) PURE;
#ifdef _FIX_ENABLEMODELESS_CONFLICT
    STDMETHOD(EnableModelessSV) (THIS_ BOOL fEnable) PURE;
#else
    STDMETHOD(EnableModeless) (THIS_ BOOL fEnable) PURE;
#endif
    STDMETHOD(UIActivate) (THIS_ UINT uState) PURE;
    STDMETHOD(Refresh) (THIS) PURE;

    STDMETHOD(CreateViewWindow)(THIS_ IShellView  *lpPrevView,
                    LPCFOLDERSETTINGS lpfs, IShellBrowser  * psb,
                    RECT * prcView, HWND  *phWnd) PURE;
    STDMETHOD(DestroyViewWindow)(THIS) PURE;
    STDMETHOD(GetCurrentInfo)(THIS_ LPFOLDERSETTINGS lpfs) PURE;
    STDMETHOD(AddPropertySheetPages)(THIS_ DWORD dwReserved,
                    LPFNADDPROPSHEETPAGE lpfn, LPARAM lparam) PURE;
    STDMETHOD(SaveViewState)(THIS) PURE;
    STDMETHOD(SelectItem)(THIS_ LPCITEMIDLIST pidlItem, UINT uFlags) PURE;
    STDMETHOD(GetItemObject)(THIS_ UINT uItem, REFIID riid,
                    void **ppv) PURE;

    // *** IShellView2 methods ***
    STDMETHOD(GetView)(THIS_ SHELLVIEWID* pvid, ULONG uView) PURE;
    STDMETHOD(CreateViewWindow2)(THIS_ LPSV2CVW2_PARAMS lpParams) PURE;
    STDMETHOD(HandleRename)(THIS_ LPCITEMIDLIST pidlNew) PURE;
    STDMETHOD(SelectAndPositionItem) (THIS_ LPCITEMIDLIST pidlItem,
        UINT uFlags,POINT* point) PURE;
};

//-------------------------------------------------------------------------
//
// struct STRRET
//
// structure for returning strings from IShellFolder member functions
//
//-------------------------------------------------------------------------
#define STRRET_WSTR     0x0000          // Use STRRET.pOleStr
#define STRRET_OFFSET   0x0001          // Use STRRET.uOffset to Ansi
#define STRRET_CSTR     0x0002          // Use STRRET.cStr

#ifdef NONAMELESSUNION
#define NAMELESS_MEMBER(member) DUMMYUNIONNAME.##member
#else
#define NAMELESS_MEMBER(member) member
#endif


typedef struct _STRRET
{
    UINT uType; // One of the STRRET_* values
    union
    {
        LPWSTR          pOleStr;        // must be freed by caller of GetDisplayNameOf
        LPSTR           pStr;           // NOT USED
        UINT            uOffset;        // Offset into SHITEMID
        char            cStr[MAX_PATH]; // Buffer to fill in (ANSI)
    } DUMMYUNIONNAME;
} STRRET, *LPSTRRET;

#ifndef RC_INVOKED
#include <poppack.h>   /* Assume byte packing throughout */
#endif /* !RC_INVOKED */


//-------------------------------------------------------------------------
//
// SHGetPathFromIDList
//
//  This function assumes the size of the buffer (MAX_PATH). The pidl
// should point to a file system object.
//
//-------------------------------------------------------------------------

SHSTDAPI_(BOOL) SHGetPathFromIDListA(LPCITEMIDLIST pidl, LPSTR pszPath);
SHSTDAPI_(BOOL) SHGetPathFromIDListW(LPCITEMIDLIST pidl, LPWSTR pszPath);
#ifdef UNICODE
#define SHGetPathFromIDList  SHGetPathFromIDListW
#else
#define SHGetPathFromIDList  SHGetPathFromIDListA
#endif // !UNICODE

SHSTDAPI_(int) SHCreateDirectoryExA(HWND hwnd, LPCSTR pszPath, SECURITY_ATTRIBUTES *psa);
SHSTDAPI_(int) SHCreateDirectoryExW(HWND hwnd, LPCWSTR pszPath, SECURITY_ATTRIBUTES *psa);
#ifdef UNICODE
#define SHCreateDirectoryEx  SHCreateDirectoryExW
#else
#define SHCreateDirectoryEx  SHCreateDirectoryExA
#endif // !UNICODE

//-------------------------------------------------------------------------
//
// SHGetSpecialFolderLocation
//
//  Caller should use SHGetMalloc to obtain an allocator that can free the pidl
//
//
//-------------------------------------------------------------------------
//
// registry entries for special paths are kept in :
#define REGSTR_PATH_SPECIAL_FOLDERS    REGSTR_PATH_EXPLORER TEXT("\\Shell Folders")


#define CSIDL_DESKTOP                   0x0000        // <desktop>
#define CSIDL_INTERNET                  0x0001        // Internet Explorer (icon on desktop)
#define CSIDL_PROGRAMS                  0x0002        // Start Menu\Programs
#define CSIDL_CONTROLS                  0x0003        // My Computer\Control Panel
#define CSIDL_PRINTERS                  0x0004        // My Computer\Printers
#define CSIDL_PERSONAL                  0x0005        // My Documents
#define CSIDL_FAVORITES                 0x0006        // <user name>\Favorites
#define CSIDL_STARTUP                   0x0007        // Start Menu\Programs\Startup
#define CSIDL_RECENT                    0x0008        // <user name>\Recent
#define CSIDL_SENDTO                    0x0009        // <user name>\SendTo
#define CSIDL_BITBUCKET                 0x000a        // <desktop>\Recycle Bin
#define CSIDL_STARTMENU                 0x000b        // <user name>\Start Menu
#define CSIDL_DESKTOPDIRECTORY          0x0010        // <user name>\Desktop
#define CSIDL_DRIVES                    0x0011        // My Computer
#define CSIDL_NETWORK                   0x0012        // Network Neighborhood
#define CSIDL_NETHOOD                   0x0013        // <user name>\nethood
#define CSIDL_FONTS                     0x0014        // windows\fonts
#define CSIDL_TEMPLATES                 0x0015
#define CSIDL_COMMON_STARTMENU          0x0016        // All Users\Start Menu
#define CSIDL_COMMON_PROGRAMS           0X0017        // All Users\Programs
#define CSIDL_COMMON_STARTUP            0x0018        // All Users\Startup
#define CSIDL_COMMON_DESKTOPDIRECTORY   0x0019        // All Users\Desktop
#define CSIDL_APPDATA                   0x001a        // <user name>\Application Data
#define CSIDL_PRINTHOOD                 0x001b        // <user name>\PrintHood
#define CSIDL_LOCAL_APPDATA             0x001c        // <user name>\Local Settings\Applicaiton Data (non roaming)
#define CSIDL_ALTSTARTUP                0x001d        // non localized startup
#define CSIDL_COMMON_ALTSTARTUP         0x001e        // non localized common startup
#define CSIDL_COMMON_FAVORITES          0x001f
#define CSIDL_INTERNET_CACHE            0x0020
#define CSIDL_COOKIES                   0x0021
#define CSIDL_HISTORY                   0x0022
#define CSIDL_COMMON_APPDATA            0x0023        // All Users\Application Data
#define CSIDL_WINDOWS                   0x0024        // GetWindowsDirectory()
#define CSIDL_SYSTEM                    0x0025        // GetSystemDirectory()
#define CSIDL_PROGRAM_FILES             0x0026        // C:\Program Files
#define CSIDL_MYPICTURES                0x0027        // C:\Program Files\My Pictures
#define CSIDL_PROFILE                   0x0028        // USERPROFILE
#define CSIDL_SYSTEMX86                 0x0029        // x86 system directory on RISC
#define CSIDL_PROGRAM_FILESX86          0x002a        // x86 C:\Program Files on RISC
#define CSIDL_PROGRAM_FILES_COMMON      0x002b        // C:\Program Files\Common
#define CSIDL_PROGRAM_FILES_COMMONX86   0x002c        // x86 Program Files\Common on RISC
#define CSIDL_COMMON_TEMPLATES          0x002d        // All Users\Templates
#define CSIDL_COMMON_DOCUMENTS          0x002e        // All Users\Documents
#define CSIDL_COMMON_ADMINTOOLS         0x002f        // All Users\Start Menu\Programs\Administrative Tools
#define CSIDL_ADMINTOOLS                0x0030        // <user name>\Start Menu\Programs\Administrative Tools
#define CSIDL_CONNECTIONS               0x0031        // Network and Dial-up Connections

#define CSIDL_FLAG_CREATE               0x8000        // combine with CSIDL_ value to force folder creation in SHGetFolderPath()
#define CSIDL_FLAG_DONT_VERIFY          0x4000        // combine with CSIDL_ value to return an unverified folder path
#define CSIDL_FLAG_MASK                 0xFF00        // mask for all possible flag values


SHSTDAPI SHGetSpecialFolderLocation(HWND hwnd, int csidl, LPITEMIDLIST *ppidl);

#if (_WIN32_IE >= 0x0400)

SHSTDAPI_(BOOL) SHGetSpecialFolderPathA(HWND hwnd, LPSTR pszPath, int csidl, BOOL fCreate);
SHSTDAPI_(BOOL) SHGetSpecialFolderPathW(HWND hwnd, LPWSTR pszPath, int csidl, BOOL fCreate);
#ifdef UNICODE
#define SHGetSpecialFolderPath  SHGetSpecialFolderPathW
#else
#define SHGetSpecialFolderPath  SHGetSpecialFolderPathA
#endif // !UNICODE

#if (_WIN32_IE >= 0x0500)

typedef enum {
    SHGFP_TYPE_CURRENT  = 0,   // current value for user, verify it exists
    SHGFP_TYPE_DEFAULT  = 1,   // default value, may not exist
} SHGFP_TYPE;

SHFOLDERAPI SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
SHFOLDERAPI SHGetFolderPathW(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath);
#ifdef UNICODE
#define SHGetFolderPath  SHGetFolderPathW
#else
#define SHGetFolderPath  SHGetFolderPathA
#endif // !UNICODE
SHSTDAPI SHGetFolderLocation(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPITEMIDLIST *ppidl);


#endif      // _WIN32_IE >= 0x0500

#endif      // _WIN32_IE >= 0x0400



//-------------------------------------------------------------------------
//
// SHBrowseForFolder API
//
//
//-------------------------------------------------------------------------

typedef int (CALLBACK* BFFCALLBACK)(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);

typedef struct _browseinfoA {
    HWND        hwndOwner;
    LPCITEMIDLIST pidlRoot;
    LPSTR        pszDisplayName;	// Return display name of item selected.
    LPCSTR       lpszTitle;			// text to go in the banner over the tree.
    UINT         ulFlags;			// Flags that control the return stuff
    BFFCALLBACK  lpfn;
    LPARAM       lParam;			// extra info that's passed back in callbacks
    int          iImage;			// output var: where to return the Image index.
} BROWSEINFOA, *PBROWSEINFOA, *LPBROWSEINFOA;

typedef struct _browseinfoW {
    HWND        hwndOwner;
    LPCITEMIDLIST pidlRoot;
    LPWSTR       pszDisplayName;	// Return display name of item selected.
    LPCWSTR      lpszTitle;			// text to go in the banner over the tree.
    UINT         ulFlags;			// Flags that control the return stuff
    BFFCALLBACK  lpfn;
    LPARAM       lParam;			// extra info that's passed back in callbacks
    int          iImage;			// output var: where to return the Image index.
} BROWSEINFOW, *PBROWSEINFOW, *LPBROWSEINFOW;

#ifdef UNICODE
#define BROWSEINFO      BROWSEINFOW
#define PBROWSEINFO     PBROWSEINFOW
#define LPBROWSEINFO    LPBROWSEINFOW
#else
#define BROWSEINFO      BROWSEINFOA
#define PBROWSEINFO     PBROWSEINFOA
#define LPBROWSEINFO    LPBROWSEINFOA
#endif

// Browsing for directory.
#define BIF_RETURNONLYFSDIRS   0x0001  // For finding a folder to start document searching
#define BIF_DONTGOBELOWDOMAIN  0x0002  // For starting the Find Computer
#define BIF_STATUSTEXT         0x0004   // Top of the dialog has 2 lines of text for BROWSEINFO.lpszTitle and one line if
                                        // this flag is set.  Passing the message BFFM_SETSTATUSTEXTA to the hwnd can set the
                                        // rest of the text.  This is not used with BIF_USENEWUI and BROWSEINFO.lpszTitle gets
                                        // all three lines of text.
#define BIF_RETURNFSANCESTORS  0x0008
#define BIF_EDITBOX            0x0010   // Add an editbox to the dialog
#define BIF_VALIDATE           0x0020   // insist on valid result (or CANCEL)

#define BIF_NEWDIALOGSTYLE     0x0040   // Use the new dialog layout with the ability to resize
                                        // Caller needs to call OleInitialize() before using this API

#define BIF_USENEWUI           (BIF_NEWDIALOGSTYLE | BIF_EDITBOX)

#define BIF_BROWSEINCLUDEURLS  0x0080   // Allow URLs to be displayed or entered. (Requires BIF_USENEWUI)

#define BIF_BROWSEFORCOMPUTER  0x1000  // Browsing for Computers.
#define BIF_BROWSEFORPRINTER   0x2000  // Browsing for Printers
#define BIF_BROWSEINCLUDEFILES 0x4000  // Browsing for Everything
#define BIF_SHAREABLE          0x8000  // sharable resources displayed (remote shares, requires BIF_USENEWUI)

// message from browser
#define BFFM_INITIALIZED        1
#define BFFM_SELCHANGED         2
#define BFFM_VALIDATEFAILEDA    3   // lParam:szPath ret:1(cont),0(EndDialog)
#define BFFM_VALIDATEFAILEDW    4   // lParam:wzPath ret:1(cont),0(EndDialog)

// messages to browser
#define BFFM_SETSTATUSTEXTA     (WM_USER + 100)
#define BFFM_ENABLEOK           (WM_USER + 101)
#define BFFM_SETSELECTIONA      (WM_USER + 102)
#define BFFM_SETSELECTIONW      (WM_USER + 103)
#define BFFM_SETSTATUSTEXTW     (WM_USER + 104)

SHSTDAPI_(LPITEMIDLIST) SHBrowseForFolderA(LPBROWSEINFOA lpbi);
SHSTDAPI_(LPITEMIDLIST) SHBrowseForFolderW(LPBROWSEINFOW lpbi);

#ifdef UNICODE
#define SHBrowseForFolder   SHBrowseForFolderW
#define BFFM_SETSTATUSTEXT  BFFM_SETSTATUSTEXTW
#define BFFM_SETSELECTION   BFFM_SETSELECTIONW

#define BFFM_VALIDATEFAILED BFFM_VALIDATEFAILEDW
#else
#define SHBrowseForFolder   SHBrowseForFolderA
#define BFFM_SETSTATUSTEXT  BFFM_SETSTATUSTEXTA
#define BFFM_SETSELECTION   BFFM_SETSELECTIONA

#define BFFM_VALIDATEFAILED BFFM_VALIDATEFAILEDA
#endif

//-------------------------------------------------------------------------
//
// SHLoadInProc
//
//   When this function is called, the shell calls CoCreateInstance
//  (or equivalent) with CLSCTX_INPROC_SERVER and the specified CLSID
//  from within the shell's process and release it immediately.
//
//-------------------------------------------------------------------------

SHSTDAPI SHLoadInProc(REFCLSID rclsid);


//-------------------------------------------------------------------------
//
// IEnumIDList interface
//
//  IShellFolder::EnumObjects member returns an IEnumIDList object.
//
//-------------------------------------------------------------------------

typedef struct IEnumIDList      *LPENUMIDLIST;

#undef  INTERFACE
#define INTERFACE       IEnumIDList

DECLARE_INTERFACE_(IEnumIDList, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IEnumIDList methods ***
    STDMETHOD(Next)  (THIS_ ULONG celt,
                      LPITEMIDLIST *rgelt,
                      ULONG *pceltFetched) PURE;
    STDMETHOD(Skip)  (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumIDList **ppenum) PURE;
};


//-------------------------------------------------------------------------
//
// IShellFolder interface
//
//
// [Member functions]
//
// IShellFolder::BindToObject(pidl, pbc, riid, ppv)
//   This function returns an instance of a sub-folder which is specified
//  by the IDList (pidl).
//
// IShellFolder::BindToStorage(pidl, pbc, riid, ppv)
//   This function returns a storage instance of a sub-folder which is
//  specified by the IDList (pidl). The shell never calls this member
//  function in the first release of Win95.
//
// IShellFolder::CompareIDs(lParam, pidl1, pidl2)
//   This function compares two IDLists and returns the result. The shell
//  explorer always passes 0 as lParam, which indicates "sort by name".
//  It should return 0 (as CODE of the scode), if two id indicates the
//  same object; negative value if pidl1 should be placed before pidl2;
//  positive value if pidl2 should be placed before pidl1.
//
// IShellFolder::CreateViewObject(hwndOwner, riid, ppv)
//   This function creates a view object of the folder itself. The view
//  object is a difference instance from the shell folder object.
//   "hwndOwner" can be used  as the owner window of its dialog box or
//  menu during the lifetime of the view object.
//  instance which has only one reference count. The explorer may create
//  more than one instances of view object from one shell folder object
//  and treat them as separate instances.
//
// IShellFolder::GetAttributesOf(cidl, apidl, prgfInOut)
//   This function returns the attributes of specified objects in that
//  folder. "cidl" and "apidl" specifies objects. "apidl" contains only
//  simple IDLists. The explorer initializes *prgfInOut with a set of
//  flags to be evaluated. The shell folder may optimize the operation
//  by not returning unspecified flags.
//
// IShellFolder::GetUIObjectOf(hwndOwner, cidl, apidl, riid, prgfInOut, ppv)
//   This function creates a UI object to be used for specified objects.
//  The shell explorer passes either IID_IDataObject (for transfer operation)
//  or IID_IContextMenu (for context menu operation) as riid.
//
// IShellFolder::GetDisplayNameOf
//   This function returns the display name of the specified object.
//  If the ID contains the display name (in the locale character set),
//  it returns the offset to the name. Otherwise, it returns a pointer
//  to the display name string (UNICODE), which is allocated by the
//  task allocator, or fills in a buffer.
//
// IShellFolder::SetNameOf
//   This function sets the display name of the specified object.
//  If it changes the ID as well, it returns the new ID which is
//  alocated by the task allocator.
//
//-------------------------------------------------------------------------

// IShellFolder::GetDisplayNameOf/SetNameOf uFlags
typedef enum tagSHGDN
{
    SHGDN_NORMAL             = 0x0000,  // default (display purpose)
    SHGDN_INFOLDER           = 0x0001,  // displayed under a folder (relative)
    SHGDN_FOREDITING         = 0x1000,  // for in-place editing
    SHGDN_FORADDRESSBAR      = 0x4000,  // UI friendly parsing name (remove ugly stuff)
    SHGDN_FORPARSING         = 0x8000,  // parsing name for ParseDisplayName()
} SHGNO;

// IShellFolder::EnumObjects grfFlags bits
typedef enum tagSHCONTF
{
    SHCONTF_FOLDERS             = 0x0020,   // only want folders enumerated (SHGAO_FOLDER)
    SHCONTF_NONFOLDERS          = 0x0040,   // include non folders
    SHCONTF_INCLUDEHIDDEN       = 0x0080,   // show items normally hidden
    SHCONTF_INIT_ON_FIRST_NEXT  = 0x0100,   // allow EnumObject() to return before validating enum
    SHCONTF_NETPRINTERSRCH      = 0x0200,   // hint that client is looking for printers
    SHCONTF_SHAREABLE           = 0x0400,   // hint that client is looking sharable resources (remote shares)
} SHCONTF;

// IShellFolder::CompareIDs lParam flags

#define SHCIDS_ALLFIELDS        0x80000000L
#define SHCIDS_COLUMNMASK       0x0000FFFFL

// IShellFolder::GetAttributesOf flags
// DESCRIPTION:
// 	 SFGAO_CANLINK: If this bit is set on an item in the shell folder, a
//            "Create Shortcut" menu item will be added to the File
//            menu and context menus for the item.  If the user selects
//            that command, your IContextMenu::InvokeCommand() will be called
//            with 'link'.
//            	 That flag will also be used to determine if "Create Shortcut"
//            should be added when the item in your folder is dragged to another
//            folder.
#define SFGAO_CANCOPY           DROPEFFECT_COPY // Objects can be copied    (0x1)
#define SFGAO_CANMOVE           DROPEFFECT_MOVE // Objects can be moved     (0x2)
#define SFGAO_CANLINK           DROPEFFECT_LINK // Objects can be linked    (0x4)
#define SFGAO_CANRENAME         0x00000010L     // Objects can be renamed
#define SFGAO_CANDELETE         0x00000020L     // Objects can be deleted
#define SFGAO_HASPROPSHEET      0x00000040L     // Objects have property sheets
#define SFGAO_DROPTARGET        0x00000100L     // Objects are drop target
#define SFGAO_CAPABILITYMASK    0x00000177L
#define SFGAO_LINK              0x00010000L     // Shortcut (link)
#define SFGAO_SHARE             0x00020000L     // shared
#define SFGAO_READONLY          0x00040000L     // read-only
#define SFGAO_GHOSTED           0x00080000L     // ghosted icon
#define SFGAO_HIDDEN            0x00080000L     // hidden object
#define SFGAO_DISPLAYATTRMASK   0x000F0000L
#define SFGAO_FILESYSANCESTOR   0x10000000L     // It contains file system folder
#define SFGAO_FOLDER            0x20000000L     // It's a folder.
#define SFGAO_FILESYSTEM        0x40000000L     // is a file system thing (file/folder/root)
#define SFGAO_HASSUBFOLDER      0x80000000L     // Expandable in the map pane
#define SFGAO_CONTENTSMASK      0x80000000L
#define SFGAO_VALIDATE          0x01000000L     // invalidate cached information
#define SFGAO_REMOVABLE         0x02000000L     // is this removeable media?
#define SFGAO_COMPRESSED        0x04000000L     // Object is compressed (use alt color)
#define SFGAO_BROWSABLE         0x08000000L     // is in-place browsable
#define SFGAO_NONENUMERATED     0x00100000L     // is a non-enumerated object
#define SFGAO_NEWCONTENT        0x00200000L     // should show bold in explorer tree
#define SFGAO_CANMONIKER        0x00400000L     // can create monikers for its objects


// IShellFolder IBindCtx* parameters. the IUnknown for these are
// accessed through IBindCtx::RegisterObjectParam/GetObjectParam

// this object will support IPersist to query a CLSID that should be skipped
// in the binding process. this is to avoid loops or to allow delegation to
// base name space functionality. see SHSkipJunction()

#define STR_SKIP_BINDING_CLSID      L"Skip Binding CLSID"

#undef  INTERFACE
#define INTERFACE       IShellFolder

DECLARE_INTERFACE_(IShellFolder, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellFolder methods ***
    STDMETHOD(ParseDisplayName)(THIS_ HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName,
                                ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes) PURE;

    STDMETHOD(EnumObjects)(THIS_ HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList) PURE;

    STDMETHOD(BindToObject)(THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(BindToStorage)(THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(CompareIDs)(THIS_ LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;
    STDMETHOD(CreateViewObject)(THIS_ HWND hwndOwner, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetAttributesOf)(THIS_ UINT cidl, LPCITEMIDLIST * apidl, ULONG * rgfInOut) PURE;
    STDMETHOD(GetUIObjectOf)(THIS_ HWND hwndOwner, UINT cidl, LPCITEMIDLIST * apidl,
                             REFIID riid, UINT * prgfInOut, void **ppv) PURE;
    STDMETHOD(GetDisplayNameOf)(THIS_ LPCITEMIDLIST pidl, DWORD uFlags, LPSTRRET lpName) PURE;
    STDMETHOD(SetNameOf)(THIS_ HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName,
                         DWORD uFlags, LPITEMIDLIST *ppidlOut) PURE;
};

typedef IShellFolder * LPSHELLFOLDER;

//
//  Helper function which returns a IShellFolder interface to the desktop
// folder. This is equivalent to call CoCreateInstance with CLSID_ShellDesktop.
//
//  CoCreateInstance(CLSID_Desktop, NULL,
//                   CLSCTX_INPROC, IID_IShellFolder, &pshf);
//
SHSTDAPI SHGetDesktopFolder(IShellFolder **ppshf);


// IShellFolder IBindCtx* parameters. the IUnknown for these are
// accessed through IBindCtx::RegisterObjectParam/GetObjectParam
// use this to provide the data needed create IDLists through
// IShellFolder::ParseDisplayName(). this data applies to the last element
// of the name that is parsed (c:\foo\bar.txt, data applies to bar.txt)
// this makes creating these IDLists much faster that suppling the name only

#define STR_FILE_SYS_BIND_DATA      L"File System Bind Data"

#undef  INTERFACE
#define INTERFACE   IFileSystemBindData

DECLARE_INTERFACE_(IFileSystemBindData, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IFileSystemBindData methods ***
    STDMETHOD(SetFindData)(THIS_ const WIN32_FIND_DATAW *pfd) PURE;
    STDMETHOD(GetFindData)(THIS_ WIN32_FIND_DATAW *pfd) PURE;
};


typedef struct _SHELLDETAILS
{
    int     fmt;            // LVCFMT_* value (header only)
    int     cxChar;         // Number of "average" characters (header only)
    STRRET  str;            // String information
} SHELLDETAILS, *LPSHELLDETAILS;

#undef  INTERFACE
#define INTERFACE   IShellDetails

DECLARE_INTERFACE_(IShellDetails, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellDetails methods ***
    STDMETHOD(GetDetailsOf)(THIS_ LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *pDetails) PURE;
    STDMETHOD(ColumnClick)(THIS_ UINT iColumn) PURE;
};


#if (_WIN32_IE >= 0x0500)

//-------------------------------------------------------------------------
//
// IEnumExtraSearch interface
//
//  IShellFolder2::EnumSearches member returns an IEnumExtraSearch object.
//
//-------------------------------------------------------------------------

typedef struct tagEXTRASEARCH
{
    GUID    guidSearch;
    WCHAR   wszFriendlyName[80];
    WCHAR   wszUrl[2084];
}EXTRASEARCH, *LPEXTRASEARCH;

typedef struct IEnumExtraSearch *LPENUMEXTRASEARCH;

#undef  INTERFACE
#define INTERFACE       IEnumExtraSearch

DECLARE_INTERFACE_(IEnumExtraSearch, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IEnumExtraSearch methods ***
    STDMETHOD(Next)  (THIS_ ULONG celt, EXTRASEARCH *rgelt, ULONG *pceltFetched) PURE;
    STDMETHOD(Skip)  (THIS_ ULONG celt) PURE;
    STDMETHOD(Reset) (THIS) PURE;
    STDMETHOD(Clone) (THIS_ IEnumExtraSearch **ppenum) PURE;
};

//--------------------------------------------------------------------------
// IShellFolder2
//
// [member functions]
//
// IShellFolder2::GetDefaultSearchGUID(LPGUID pGuid)
//   Returns the guid of the search that is to be invoked when user clicks
//   on the search toolbar button
//
// IShellFolder2::EnumSearches(IEnumExtraSearch **ppenum)
//   gives an enumerator of the searches to be added to the search menu
//--------------------------------------------------------------------------

// IShellFolder2::GetDefaultColumnState values
typedef enum {
    SHCOLSTATE_TYPE_STR     = 0x00000001,
    SHCOLSTATE_TYPE_INT     = 0x00000002,
    SHCOLSTATE_TYPE_DATE    = 0x00000003,
    SHCOLSTATE_TYPEMASK     = 0x0000000F,
    SHCOLSTATE_ONBYDEFAULT  = 0x00000010,   // should on by default in details view
    SHCOLSTATE_SLOW         = 0x00000020,   // will be slow to compute, do on a background thread
    SHCOLSTATE_EXTENDED     = 0x00000040,   // provided by a handler, not the folder
    SHCOLSTATE_SECONDARYUI  = 0x00000080,   // not displayed in context menu, but listed in the "More..." dialog
    SHCOLSTATE_HIDDEN       = 0x00000100,   // not displayed in the UI
} SHCOLSTATE;

typedef struct {
    GUID fmtid;
    DWORD pid;
} SHCOLUMNID, *LPSHCOLUMNID;
typedef const SHCOLUMNID* LPCSHCOLUMNID;

#undef  INTERFACE
#define INTERFACE       IShellFolder2

DECLARE_INTERFACE_(IShellFolder2, IShellFolder)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS)  PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

    // *** IShellFolder methods ***
    STDMETHOD(ParseDisplayName)(THIS_ HWND hwnd, LPBC pbc, LPOLESTR pszDisplayName,
                                ULONG *pchEaten, LPITEMIDLIST *ppidl, ULONG *pdwAttributes) PURE;
    STDMETHOD(EnumObjects)(THIS_ HWND hwnd, DWORD grfFlags, IEnumIDList **ppenumIDList) PURE;
    STDMETHOD(BindToObject)(THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(BindToStorage)(THIS_ LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, void **ppv) PURE;
    STDMETHOD(CompareIDs)(THIS_ LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;
    STDMETHOD(CreateViewObject)(THIS_ HWND hwnd, REFIID riid, void **ppv) PURE;
    STDMETHOD(GetAttributesOf)(THIS_ UINT cidl, LPCITEMIDLIST * apidl, ULONG * drgfInOut) PURE;
    STDMETHOD(GetUIObjectOf)(THIS_ HWND hwnd, UINT cidl, LPCITEMIDLIST * apidl,
                             REFIID riid, UINT * prgfInOut, void **ppv) PURE;
    STDMETHOD(GetDisplayNameOf)(THIS_ LPCITEMIDLIST pidl, DWORD uFlags, STRRET *psr) PURE;
    STDMETHOD(SetNameOf)(THIS_ HWND hwnd, LPCITEMIDLIST pidl, LPCOLESTR pszName,
                         DWORD uFlags, LPITEMIDLIST *ppidl) PURE;

    // *** IShellFolder2 methods ***
    STDMETHOD(GetDefaultSearchGUID)(THIS_ GUID *pguid) PURE;
    STDMETHOD(EnumSearches)(THIS_ IEnumExtraSearch **ppenum) PURE;
    STDMETHOD(GetDefaultColumn) (THIS_ DWORD dwRes, ULONG *pSort, ULONG *pDisplay) PURE;
    STDMETHOD(GetDefaultColumnState)(THIS_ UINT iColumn, DWORD *pcsFlags) PURE;
    STDMETHOD(GetDetailsEx)(THIS_ LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv) PURE;
    STDMETHOD(GetDetailsOf)(THIS_ LPCITEMIDLIST pidl, UINT iColumn, SHELLDETAILS *psd) PURE;
    STDMETHOD(MapColumnToSCID)(THIS_ UINT iColumn, SHCOLUMNID *pscid) PURE;
};

#endif



//-------------------------------------------------------------------------
//
// ITaskbarList interface
//
//
// [Member functions]
//
// ITaskbarList::HrInit()
//   This function must be called first to validate use of other members.
//
// ITaskbarList::AddTab(hwnd)
//   This function adds a tab for hwnd to the taskbar.
//
// ITaskbarList::DeleteTab(hwnd)
//   This function deletes a tab for hwnd from the taskbar.
//
// ITaskbarList::ActivateTab(hwnd)
//   This function activates the tab associated with hwnd on the taskbar.
//
// ITaskbarList::SetActivateAlt(hwnd)
//   This function marks hwnd in the taskbar as the active tab
//
//-------------------------------------------------------------------------

#undef INTERFACE
#define INTERFACE ITaskbarList

DECLARE_INTERFACE_(ITaskbarList, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** ITaskbarList specific methods ***
    STDMETHOD(HrInit) (THIS) PURE;
    STDMETHOD(AddTab) (THIS_ HWND hwnd) PURE;
    STDMETHOD(DeleteTab) (THIS_ HWND hwnd) PURE;
    STDMETHOD(ActivateTab) (THIS_ HWND hwnd) PURE;
    STDMETHOD(SetActiveAlt) (THIS_ HWND hwnd) PURE;
};

//-------------------------------------------------------------------------
//
// IObjMgr interface
//
//
// [Member functions]
//
// IObjMgr::Append(punk)
//   This function adds an object to the end of a list of objects.
//
// IObjMgr::Remove(punk)
//   This function removes an object from a list of objects.
//
// This is implemented by CLSID_ACLMulti so each AutoComplete List
// (CLSID_ACLHistory, CLSID_ACListISF, CLSID_ACLMRU) can be added.
// CLSID_ACLMulti's IEnumString will then be the union of the results
// from the COM Objects added.
//-------------------------------------------------------------------------

#undef INTERFACE
#define INTERFACE IObjMgr

DECLARE_INTERFACE_(IObjMgr, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IObjMgr specific methods ***
    STDMETHOD(Append) (THIS_ IUnknown *punk) PURE;
    STDMETHOD(Remove) (THIS_ IUnknown *punk) PURE;
};


//-------------------------------------------------------------------------
//
// ICurrentWorkingDirectory interface
//
//
// [Member functions]
//
// ICurrentWorkingDirectory::GetDirectory(LPWSTR pwzPath, DWORD cchSize)
//   This function gets the Current Working Directory from a COM object that
//   stores such state.
//
// ICurrentWorkingDirectory::SetDirectory(LPCWSTR pwzPath)
//   This function sets the Current Working Directory of a COM object that
//   stores such state.
//
// This function can be used generically.  One COM object that implements it
// is CLSID_ACListISF so that the AutoComplete engine can complete relative
// paths.  SetDirectory() will set the "Current Working Directory" and
// AutoComplete with then complete both absolute and relative paths.
// For Example, if ::SetDirectory(L"C:\Program Files") is called, then
// the user can AutoComplete "..\winnt".  In order to set the current
// working directory for non-file system paths, "ftp://ftp.microsoft.com/" or
// "Control Panel" for example, use IPersistFolder.
//-------------------------------------------------------------------------

#undef INTERFACE
#define INTERFACE ICurrentWorkingDirectory

DECLARE_INTERFACE_(ICurrentWorkingDirectory, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** ICurrentWorkingDirectory specific methods ***
    STDMETHOD(GetDirectory) (THIS_ LPWSTR pwzPath, DWORD cchSize) PURE;
    STDMETHOD(SetDirectory) (THIS_ LPCWSTR pwzPath) PURE;
};


//-------------------------------------------------------------------------
//
// IACList interface
//
//
// [Member functions]
//
// IObjMgr::Expand(LPCOLESTR)
//   This function tells an autocomplete list to expand a specific string.
//
// If the user enters a multi-level path, AutoComplete (CLSID_AutoComplete)
// will use this interface to tell the "AutoComplete Lists" where to expand
// the results.
//
// For Example, if the user enters "C:\Program Files\Micros", AutoComplete
// first completely enumerate the "AutoComplete Lists" via IEnumString.  Then it
// will call the "AutoComplete Lists" with IACList::Expand(L"C:\Program Files").
// It will then enumerate the IEnumString interface again to get results in
// that directory.
//-------------------------------------------------------------------------

#undef INTERFACE
#define INTERFACE IACList

DECLARE_INTERFACE_(IACList, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // *** IACList specific methods ***
    STDMETHOD(Expand) (THIS_ LPCOLESTR pszExpand) PURE;
};

//-------------------------------------------------------------------------
//
// IACList2 interface
//
// [Description]
//		This interface exists to allow the caller to set filter criteria
// for an AutoComplete List.  AutoComplete Lists generates the list of
// possible AutoComplete completions.  CLSID_ACListISF is one AutoComplete
// List COM object that implements this interface.
//-------------------------------------------------------------------------

#undef INTERFACE
#define INTERFACE IACList2

typedef enum _tagAUTOCOMPLETELISTOPTIONS
{
    ACLO_NONE            = 0,    // don't enumerate anything
    ACLO_CURRENTDIR      = 1,    // enumerate current directory
    ACLO_MYCOMPUTER      = 2,    // enumerate MyComputer
    ACLO_DESKTOP         = 4,    // enumerate Desktop Folder
    ACLO_FAVORITES       = 8,    // enumerate Favorites Folder
    ACLO_FILESYSONLY     = 16,   // enumerate only the file system
} AUTOCOMPLETELISTOPTIONS;

DECLARE_INTERFACE_(IACList2, IACList)
{
    // *** IACList2 specific methods ***
    STDMETHOD(SetOptions)(THIS_ DWORD dwFlag) PURE;
    STDMETHOD(GetOptions)(THIS_ DWORD* pdwFlag) PURE;
};


/*-------------------------------------------------------------------------*\
    INTERFACE: IProgressDialog

    DESCRIPTION:
        CLSID_ProgressDialog/IProgressDialog exist to allow a caller to create
    a progress dialog, set it's title, animation, text lines, progress, and
    it will do all the work of updating on a background thread, being modless,
    handling the user cancelling the operation, and estimating the time remaining
    until the operation completes.

    USAGE:
        This is how the dialog is used during operations that require progress
    and the ability to cancel:
    {
        DWORD dwComplete, dwTotal;
        IProgressDialog * ppd;
        CoCreateInstance(CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, IID_IProgressDialog, (void **)&ppd);
        ppd->SetTitle(L"My Slow Operation");                                // Set the title of the dialog.
        ppd->SetAnimation(hInstApp, IDA_OPERATION_ANIMATION);               // Set the animation to play.
        ppd->StartProgressDialog(hwndParent, punk, PROGDLG_AUTOTIME, NULL); // Display and enable automatic estimated time remaining.
        ppd->SetCancelMsg(L"Please wait while the current operation is cleaned up", NULL);   // Will only be displayed if Cancel button is pressed.

        dwComplete = 0;
        dwTotal = CalcTotalUnitsToDo();

        // Reset because CalcTotalUnitsToDo() took a long time and the estimated time
        // is based on the time between ::StartProgressDialog() and the first
        // ::SetProgress() call.
        ppd->Timer(PDTIMER_RESET, NULL);

        for (nIndex = 0; nIndex < nTotal; nIndex++)
        {
            if (TRUE == ppd->HasUserCancelled())
                break;

            ppd->SetLine(2, L"I'm processing item n", FALSE, NULL);
            dwComplete += DoSlowOperation();

            ppd->SetProgress(dwCompleted, dwTotal);
        }

        ppd->StopProgressDialog();
        ppd->Release();
    }
\*-------------------------------------------------------------------------*/

// Flags for IProgressDialog::StartProgressDialog() (dwFlags)
#define PROGDLG_NORMAL          0x00000000      // default normal progress dlg behavior
#define PROGDLG_MODAL           0x00000001      // the dialog is modal to its hwndParent (default is modeless)
#define PROGDLG_AUTOTIME        0x00000002      // automatically updates the "Line3" text with the "time remaining" (you cant call SetLine3 if you passs this!)
#define PROGDLG_NOTIME          0x00000004      // we dont show the "time remaining" if this is set. We need this if dwTotal < dwCompleted for sparse files
#define PROGDLG_NOMINIMIZE      0x00000008      // Do not have a minimize button in the caption bar.
#define PROGDLG_NOPROGRESSBAR   0x00000010      // Don't display the progress bar

// Time Actions (dwTimerAction)
#define PDTIMER_RESET       0x00000001       // Reset the timer so the progress will be calculated from now until the first ::SetProgress() is called so
                                             // those this time will correspond to the values passed to ::SetProgress().  Only do this before ::SetProgress() is called.


#undef  INTERFACE
#define INTERFACE   IProgressDialog

DECLARE_INTERFACE_(IProgressDialog, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IProgressDialog specific methods
    STDMETHOD(StartProgressDialog)(THIS_ HWND hwndParent, IUnknown * punkEnableModless, DWORD dwFlags, LPCVOID pvResevered) PURE;
    STDMETHOD(StopProgressDialog)(THIS) PURE;
    STDMETHOD(SetTitle)(THIS_ LPCWSTR pwzTitle) PURE;
    STDMETHOD(SetAnimation)(THIS_ HINSTANCE hInstAnimation, UINT idAnimation) PURE;
    STDMETHOD_(BOOL,HasUserCancelled) (THIS) PURE;
    STDMETHOD(SetProgress)(THIS_ DWORD dwCompleted, DWORD dwTotal) PURE;
    STDMETHOD(SetProgress64)(THIS_ ULONGLONG ullCompleted, ULONGLONG ullTotal) PURE;
    STDMETHOD(SetLine)(THIS_ DWORD dwLineNum, LPCWSTR pwzString, BOOL fCompactPath, LPCVOID pvResevered) PURE;
    STDMETHOD(SetCancelMsg)(THIS_ LPCWSTR pwzCancelMsg, LPCVOID pvResevered) PURE;
    STDMETHOD(Timer)(THIS_ DWORD dwTimerAction, LPCVOID pvResevered) PURE;
};


//==========================================================================
// IInputObjectSite/IInputObject interfaces
//
//  These interfaces allow us (or ISVs) to install/update external Internet
// Toolbar for IE and the shell. The frame will simply get the CLSID from
// registry (to be defined) and CoCreateInstance it.
//
//==========================================================================

//-------------------------------------------------------------------------
//
// IInputObjectSite interface
//
//   A site implements this interface so the object can communicate
// focus change to it.
//
// [Member functions]
//
// IInputObjectSite::OnFocusChangeIS(punkObj, fSetFocus)
//   Object (punkObj) is getting or losing the focus.
//
//-------------------------------------------------------------------------


#undef  INTERFACE
#define INTERFACE   IInputObjectSite

DECLARE_INTERFACE_(IInputObjectSite, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IInputObjectSite specific methods ***
    STDMETHOD(OnFocusChangeIS)(THIS_ IUnknown* punkObj, BOOL fSetFocus) PURE;
};


//-------------------------------------------------------------------------
//
// IInputObject interface
//
//   An object implements this interface so the site can communicate
// activation and accelerator events to it.
//
// [Member functions]
//
// IInputObject::UIActivateIO(fActivate, lpMsg)
//   Activates or deactivates the object.  lpMsg may be NULL.  Returns
//   S_OK if the activation succeeded.
//
// IInputObject::HasFocusIO()
//   Returns S_OK if the object has the focus, S_FALSE if not.
//
// IInputObject::TranslateAcceleratorIO(lpMsg)
//   Allow the object to process the message.  Returns S_OK if the
//   message was processed (eaten).
//
//-------------------------------------------------------------------------


#undef  INTERFACE
#define INTERFACE   IInputObject

DECLARE_INTERFACE_(IInputObject, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IInputObject specific methods ***
    STDMETHOD(UIActivateIO)(THIS_ BOOL fActivate, LPMSG lpMsg) PURE;
    STDMETHOD(HasFocusIO)(THIS) PURE;
    STDMETHOD(TranslateAcceleratorIO)(THIS_ LPMSG lpMsg) PURE;
};


//==========================================================================
// IDockingWindowSite/IDockingWindow/IDockingWindowFrame interfaces
// IInputObjectSite/IInputObject interfaces
//
//  These interfaces allow us (or ISVs) to install/update external Internet
// Toolbar for IE and the shell. The frame will simply get the CLSID from
// registry (to be defined) and CoCreateInstance it.
//
//==========================================================================


//-------------------------------------------------------------------------
//
// IDockingWindowSite interface
//
//   A site implements this interface so the object can negotiate for
// and inquire about real estate on the site.
//
// [Member functions]
//
// IDockingWindowSite::GetBorderDW(punkObj, prcBorder)
//   Site returns the bounding rectangle of the given source object
//   (punkObj).
//
// IDockingWindowSite::RequestBorderSpaceDW(punkObj, pbw)
//   Object requests that the site makes room for it, as specified in
//   *pbw.
//
// IDockingWindowSite::SetBorderSpaceDW(punkObj, pbw)
//   Object requests that the site set the border spacing to the size
//   specified in *pbw.
//
//-------------------------------------------------------------------------


#undef  INTERFACE
#define INTERFACE   IDockingWindowSite

DECLARE_INTERFACE_(IDockingWindowSite, IOleWindow)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IOleWindow methods ***
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

    // *** IDockingWindowSite methods ***
    STDMETHOD(GetBorderDW) (THIS_ IUnknown* punkObj, LPRECT prcBorder) PURE;
    STDMETHOD(RequestBorderSpaceDW) (THIS_ IUnknown* punkObj, LPCBORDERWIDTHS pbw) PURE;
    STDMETHOD(SetBorderSpaceDW) (THIS_ IUnknown* punkObj, LPCBORDERWIDTHS pbw) PURE;
};



//-------------------------------------------------------------------------
//
// IDockingWindowFrame interface
//
//
// [Member functions]
//
// IDockingWindowFrame::AddToolbar(punkSrc, pwszItem, dwReserved)
//
// IDockingWindowFrame::RemoveToolbar(punkSrc, dwRemoveFlags)
//
// IDockingWindowFrame::FindToolbar(pwszItem, riid, ppv)
//
//-------------------------------------------------------------------------


// flags for RemoveToolbar
#define DWFRF_NORMAL            0x0000
#define DWFRF_DELETECONFIGDATA  0x0001


// flags for AddToolbar
#define DWFAF_HIDDEN  0x0001   // add hidden

#undef  INTERFACE
#define INTERFACE   IDockingWindowFrame

DECLARE_INTERFACE_(IDockingWindowFrame, IOleWindow)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IOleWindow methods ***
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

    // *** IDockingWindowFrame methods ***
    STDMETHOD(AddToolbar) (THIS_ IUnknown* punkSrc, LPCWSTR pwszItem, DWORD dwAddFlags) PURE;
    STDMETHOD(RemoveToolbar) (THIS_ IUnknown* punkSrc, DWORD dwRemoveFlags) PURE;
    STDMETHOD(FindToolbar) (THIS_ LPCWSTR pwszItem, REFIID riid, void **ppv) PURE;
};



//-------------------------------------------------------------------------
//
// IDockingWindow interface
//
//   An object (docking window) implements this interface so the site can
// communicate with it.  An example of a docking window is a toolbar.
//
// [Member functions]
//
// IDockingWindow::ShowDW(fShow)
//   Shows or hides the docking window.
//
// IDockingWindow::CloseDW(dwReserved)
//   Closes the docking window.  dwReserved must be 0.
//
// IDockingWindow::ResizeBorderDW(prcBorder, punkToolbarSite, fReserved)
//   Resizes the docking window's border to *prcBorder.  fReserved must
//   be 0.
// IObjectWithSite::SetSite(punkSite)
//   IDockingWindow usually paired with IObjectWithSite.
//   Provides the IUnknown pointer of the site to the docking window.
//
//
//-------------------------------------------------------------------------


#undef  INTERFACE
#define INTERFACE   IDockingWindow

DECLARE_INTERFACE_(IDockingWindow, IOleWindow)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IOleWindow methods ***
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

    // *** IDockingWindow methods ***
    STDMETHOD(ShowDW)         (THIS_ BOOL fShow) PURE;
    STDMETHOD(CloseDW)        (THIS_ DWORD dwReserved) PURE;
    STDMETHOD(ResizeBorderDW) (THIS_ LPCRECT   prcBorder,
                                     IUnknown* punkToolbarSite,
                                     BOOL      fReserved) PURE;
};


//-------------------------------------------------------------------------
//
// IDeskBand interface
//
//
// [Member functions]
//
// IDeskBand::GetBandInfo(dwBandID, dwViewMode, pdbi)
//   Returns info on the given band in *pdbi, according to the mask
//   field in the DESKBANDINFO structure and the given viewmode.
//
//-------------------------------------------------------------------------


// Mask values for DESKBANDINFO
#define DBIM_MINSIZE    0x0001
#define DBIM_MAXSIZE    0x0002
#define DBIM_INTEGRAL   0x0004
#define DBIM_ACTUAL     0x0008
#define DBIM_TITLE      0x0010
#define DBIM_MODEFLAGS  0x0020
#define DBIM_BKCOLOR    0x0040

typedef struct {
    DWORD       dwMask;
    POINTL      ptMinSize;
    POINTL      ptMaxSize;
    POINTL      ptIntegral;
    POINTL      ptActual;
    WCHAR       wszTitle[256];
    DWORD       dwModeFlags;
    COLORREF    crBkgnd;
} DESKBANDINFO;

// DESKBANDINFO dwModeFlags values
#define DBIMF_NORMAL            0x0000
#define DBIMF_VARIABLEHEIGHT    0x0008
#define DBIMF_DEBOSSED          0x0020
#define DBIMF_BKCOLOR           0x0040

// GetBandInfo view mode values
#define DBIF_VIEWMODE_NORMAL         0x0000
#define DBIF_VIEWMODE_VERTICAL       0x0001
#define DBIF_VIEWMODE_FLOATING       0x0002
#define DBIF_VIEWMODE_TRANSPARENT    0x0004

#undef  INTERFACE
#define INTERFACE   IDeskBand

DECLARE_INTERFACE_(IDeskBand, IDockingWindow)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IOleWindow methods ***
    STDMETHOD(GetWindow) (THIS_ HWND * lphwnd) PURE;
    STDMETHOD(ContextSensitiveHelp) (THIS_ BOOL fEnterMode) PURE;

    // *** IDockingWindow methods ***
    STDMETHOD(ShowDW)         (THIS_ BOOL fShow) PURE;
    STDMETHOD(CloseDW)        (THIS_ DWORD dwReserved) PURE;
    STDMETHOD(ResizeBorderDW) (THIS_ LPCRECT   prcBorder,
                                     IUnknown* punkToolbarSite,
                                     BOOL      fReserved) PURE;
    // *** IDeskBand methods ***
    STDMETHOD(GetBandInfo)    (THIS_ DWORD dwBandID, DWORD dwViewMode,
                                DESKBANDINFO* pdbi) PURE;

};

// Command Target IDs
enum {
    DBID_BANDINFOCHANGED    = 0,
    DBID_SHOWONLY           = 1,
    DBID_MAXIMIZEBAND       = 2,      // Maximize the specified band (VT_UI4 == dwID)
    DBID_PUSHCHEVRON        = 3,
    DBID_DELAYINIT          = 4,      // Note: _bandsite_ calls _band_ with this code
    DBID_FINISHINIT         = 5,      // Note: _bandsite_ calls _band_ with this code
};


#if (_WIN32_IE >= 0x0400)

//-------------------------------------------------------------------------
//
// IRunnableTask interface
//
//   This is a free threaded interface used for putting items on a background
// scheduler for execution within the view.  It allows a scheduler to start and
// stop tasks on as many worker threads as it deems necessary.
//
// Run(), Kill() and Suspend() may be called from different threads.
//
// [Member functions]
//
// IRunnableTask::Run(void)
//   Initiate the task to run.  This should return E_PENDING if the task
//   has been suspended.
//
// IRunnableTask::Kill(void)
//
// IRunnableTask::Suspend(void)
//
// IRunnableTask::Resume(void)
//
// IRunnableTask::IsRunning(void)
//
//-------------------------------------------------------------------------

// Convenient state values
#define IRTIR_TASK_NOT_RUNNING  0
#define IRTIR_TASK_RUNNING      1
#define IRTIR_TASK_SUSPENDED    2
#define IRTIR_TASK_PENDING      3
#define IRTIR_TASK_FINISHED     4

#undef  INTERFACE
#define INTERFACE   IRunnableTask

DECLARE_INTERFACE_( IRunnableTask, IUnknown )
{
    // *** IUnknown methods ***
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // *** IRunnableTask methods ***
    STDMETHOD (Run)(THIS) PURE;
    STDMETHOD (Kill)(THIS_ BOOL fWait ) PURE;
    STDMETHOD (Suspend)(THIS) PURE;
    STDMETHOD (Resume)(THIS) PURE;
    STDMETHOD_(ULONG, IsRunning)(THIS) PURE;
};

typedef IRunnableTask * LPRUNNABLETASK;
#endif


#if (_WIN32_IE >= 0x0400)

// --- IExtractImage
// this interface is provided for objects to provide a thumbnail image.
// IExtractImage::GetLocation()
//      Gets a path description of the image that is to be extracted. This is used to
//      identify the image in the view so that multiple instances of the same image can reuse the
//      original image. If *pdwFlags == IEIFLAG_ASYNC and the result is E_PENDING, then *pdwPriority
//      is used to return the priority of the item, this is usually a measure of how long it will take
//      to perform the extraction. *pdwFlags can return IEIFLAG_CACHE if the view should cache a copy
//      of the image for future reference and faster access. This flag is use dto tell the difference
//      between file formats that cache a thumbnail image  such as Flashpix or Office documents, and those
//      that don't cache one.
// IExtractImage::Extract()
//      Extract the thumbnail of the specified size. If GetLocation() returned the values indicating
//      it is free-threaded and can be placed on a background thread. If the object
//      supports IRunnableTask as well, then long extractions can be started and paused as appropriate.
//      At this point it is asssumed the object is free-threaded.
//      If dwRecClrDepth contains a recommended Colour depth
//      If *phBmpthumbnail is non NULL, then it contains the destination bitmap that should be used.

#define IEI_PRIORITY_MAX        ITSAT_MAX_PRIORITY
#define IEI_PRIORITY_MIN        ITSAT_MIN_PRIORITY
#define IEIT_PRIORITY_NORMAL     ITSAT_DEFAULT_PRIORITY

#define IEIFLAG_ASYNC       0x0001      // ask the extractor if it supports ASYNC extract (free threaded)
#define IEIFLAG_CACHE       0x0002      // returned from the extractor if it does NOT cache the thumbnail
#define IEIFLAG_ASPECT      0x0004      // passed to the extractor to beg it to render to the aspect ratio of the supplied rect
#define IEIFLAG_OFFLINE     0x0008      // if the extractor shouldn't hit the net to get any content neede for the rendering
#define IEIFLAG_GLEAM       0x0010      // does the image have a gleam ? this will be returned if it does
#define IEIFLAG_SCREEN      0x0020      // render as if for the screen  (this is exlusive with IEIFLAG_ASPECT )
#define IEIFLAG_ORIGSIZE    0x0040      // render to the approx size passed, but crop if neccessary

#undef  INTERFACE
#define INTERFACE   IExtractImage

DECLARE_INTERFACE_ ( IExtractImage, IUnknown )
{
    // IUnknown methods
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

    // *** IExtractImage methods ***
    STDMETHOD (GetLocation) ( THIS_ LPWSTR pszPathBuffer,
                              DWORD cch,
                              DWORD * pdwPriority,
                              const SIZE * prgSize,
                              DWORD dwRecClrDepth,
                              DWORD *pdwFlags ) PURE;

    STDMETHOD (Extract)( THIS_ HBITMAP * phBmpThumbnail) PURE;
};
typedef IExtractImage * LPEXTRACTIMAGE;


/* ***************** IExtractImage2
 * GetDateStamp : returns the date stamp associated with the image. If this image is already cached,
 *                then it is easy to find out if the image is out of date.
 */

#undef  INTERFACE
#define INTERFACE   IExtractImage

DECLARE_INTERFACE_ ( IExtractImage2, IExtractImage )
{
    // IUnknown methods
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

    // *** IExtractImage methods ***
    STDMETHOD (GetLocation) ( THIS_ LPWSTR pszPathBuffer,
                              DWORD cch,
                              DWORD * pdwPriority,
                              const SIZE * prgSize,
                              DWORD dwRecClrDepth,
                              DWORD *pdwFlags ) PURE;

    STDMETHOD (Extract)( THIS_ HBITMAP * phBmpThumbnail) PURE;

    // *** IExtractImage2 methods ***
    STDMETHOD (GetDateStamp)( FILETIME * pDateStamp ) PURE;
};
typedef IExtractImage2 * LPEXTRACTIMAGE2;

#endif


#if (_WIN32_IE >= 0x400)
//
// We need to make sure that WININET.H is included before this interface is
// used because the COMPONENT structure uses INTERNET_MAX_URL_LENGTH
//
#ifdef _WININET_
//
//  Flags and structures used by IActiveDesktop
//

typedef struct _tagWALLPAPEROPT
{
    DWORD   dwSize;     // size of this Structure.
    DWORD   dwStyle;    // WPSTYLE_* mentioned above
}
WALLPAPEROPT;

typedef WALLPAPEROPT  *LPWALLPAPEROPT;
typedef const WALLPAPEROPT *LPCWALLPAPEROPT;

typedef struct _tagCOMPONENTSOPT
{
    DWORD   dwSize;             //Size of this structure
    BOOL    fEnableComponents;  //Enable components?
    BOOL    fActiveDesktop;     // Active desktop enabled ?
}
COMPONENTSOPT;

typedef COMPONENTSOPT   *LPCOMPONENTSOPT;
typedef const COMPONENTSOPT   *LPCCOMPONENTSOPT;

typedef struct _tagCOMPPOS
{
    DWORD   dwSize;             //Size of this structure
    int     iLeft;              //Left of top-left corner in screen co-ordinates.
    int     iTop;               //Top of top-left corner in screen co-ordinates.
    DWORD   dwWidth;            // Width in pixels.
    DWORD   dwHeight;           // Height in pixels.
    int     izIndex;            // Indicates the Z-order of the component.
    BOOL    fCanResize;         // Is the component resizeable?
    BOOL    fCanResizeX;        // Resizeable in X-direction?
    BOOL    fCanResizeY;        // Resizeable in Y-direction?
    int     iPreferredLeftPercent;    //Left of top-left corner as percent of screen width
    int     iPreferredTopPercent;     //Top of top-left corner as percent of screen height
}
COMPPOS;

typedef COMPPOS *LPCOMPPOS;
typedef const COMPPOS *LPCCOMPPOS;

typedef struct  _tagCOMPSTATEINFO
{
    DWORD   dwSize;             // Size of this structure.
    int     iLeft;              // Left of the top-left corner in screen co-ordinates.
    int     iTop;               // Top of top-left corner in screen co-ordinates.
    DWORD   dwWidth;            // Width in pixels.
    DWORD   dwHeight;           // Height in pixels.
    DWORD   dwItemState;        // State of the component (full-screen mode or split-screen or normal state.
}
COMPSTATEINFO;

typedef COMPSTATEINFO   *LPCOMPSTATEINFO;
typedef const COMPSTATEINFO *LPCCOMPSTATEINFO;



#define COMPONENT_TOP (0x3fffffff)  // izOrder value meaning component is at the top


// iCompType values
#define COMP_TYPE_HTMLDOC       0
#define COMP_TYPE_PICTURE       1
#define COMP_TYPE_WEBSITE       2
#define COMP_TYPE_CONTROL       3
#define COMP_TYPE_CFHTML        4
#define COMP_TYPE_MAX           4

// The following is the COMPONENT structure used in IE4.01, IE4.0 and Memphis. It is kept here for compatibility
// reasons.
typedef struct _tagIE4COMPONENT
{
    DWORD   dwSize;             //Size of this structure
    DWORD   dwID;               //Reserved: Set it always to zero.
    int     iComponentType;     //One of COMP_TYPE_*
    BOOL    fChecked;           // Is this component enabled?
    BOOL    fDirty;             // Had the component been modified and not yet saved to disk?
    BOOL    fNoScroll;          // Is the component scrollable?
    COMPPOS cpPos;              // Width, height etc.,
    WCHAR   wszFriendlyName[MAX_PATH];          // Friendly name of component.
    WCHAR   wszSource[INTERNET_MAX_URL_LENGTH]; //URL of the component.
    WCHAR   wszSubscribedURL[INTERNET_MAX_URL_LENGTH]; //Subscrined URL
}
IE4COMPONENT;

typedef IE4COMPONENT *LPIE4COMPONENT;
typedef const IE4COMPONENT *LPCIE4COMPONENT;

//
// The following is the new NT5 component structure. Note that the initial portion of this component exactly
// matches the IE4COMPONENT structure. All new fields are added at the bottom and the dwSize field is used to
// distinguish between IE4COMPONENT and the new COMPONENT structures.
//
typedef struct _tagCOMPONENT
{
    DWORD   dwSize;             //Size of this structure
    DWORD   dwID;               //Reserved: Set it always to zero.
    int     iComponentType;     //One of COMP_TYPE_*
    BOOL    fChecked;           // Is this component enabled?
    BOOL    fDirty;             // Had the component been modified and not yet saved to disk?
    BOOL    fNoScroll;          // Is the component scrollable?
    COMPPOS cpPos;              // Width, height etc.,
    WCHAR   wszFriendlyName[MAX_PATH];          // Friendly name of component.
    WCHAR   wszSource[INTERNET_MAX_URL_LENGTH]; //URL of the component.
    WCHAR   wszSubscribedURL[INTERNET_MAX_URL_LENGTH]; //Subscrined URL

    //New fields are added below. Everything above here must exactly match the IE4COMPONENT Structure.
    DWORD           dwCurItemState; // Current state of the Component.
    COMPSTATEINFO   csiOriginal;    // Original state of the component when it was first added.
    COMPSTATEINFO   csiRestored;    // Restored state of the component.
}
COMPONENT;

typedef COMPONENT *LPCOMPONENT;
typedef const COMPONENT *LPCCOMPONENT;


// Defines for dwCurItemState
#define IS_NORMAL               0x00000001
#define IS_FULLSCREEN           0x00000002
#define IS_SPLIT                0x00000004
#define IS_VALIDSIZESTATEBITS   (IS_NORMAL | IS_SPLIT | IS_FULLSCREEN)  // The set of IS_* state bits which define the "size" of the component - these bits are mutually exclusive.
#define IS_VALIDSTATEBITS       (IS_NORMAL | IS_SPLIT | IS_FULLSCREEN | 0x80000000 | 0x40000000)  // All of the currently defined IS_* bits.

////////////////////////////////////////////
// Flags for IActiveDesktop::ApplyChanges()
#define AD_APPLY_SAVE             0x00000001
#define AD_APPLY_HTMLGEN          0x00000002
#define AD_APPLY_REFRESH          0x00000004
#define AD_APPLY_ALL              (AD_APPLY_SAVE | AD_APPLY_HTMLGEN | AD_APPLY_REFRESH)
#define AD_APPLY_FORCE            0x00000008
#define AD_APPLY_BUFFERED_REFRESH 0x00000010
#define AD_APPLY_DYNAMICREFRESH   0x00000020

////////////////////////////////////////////
// Flags for IActiveDesktop::GetWallpaperOptions()
//           IActiveDesktop::SetWallpaperOptions()
#define WPSTYLE_CENTER      0
#define WPSTYLE_TILE        1
#define WPSTYLE_STRETCH     2
#define WPSTYLE_MAX         3


////////////////////////////////////////////
// Flags for IActiveDesktop::ModifyComponent()

#define COMP_ELEM_TYPE          0x00000001
#define COMP_ELEM_CHECKED       0x00000002
#define COMP_ELEM_DIRTY         0x00000004
#define COMP_ELEM_NOSCROLL      0x00000008
#define COMP_ELEM_POS_LEFT      0x00000010
#define COMP_ELEM_POS_TOP       0x00000020
#define COMP_ELEM_SIZE_WIDTH    0x00000040
#define COMP_ELEM_SIZE_HEIGHT   0x00000080
#define COMP_ELEM_POS_ZINDEX    0x00000100
#define COMP_ELEM_SOURCE        0x00000200
#define COMP_ELEM_FRIENDLYNAME  0x00000400
#define COMP_ELEM_SUBSCRIBEDURL 0x00000800
#define COMP_ELEM_ORIGINAL_CSI  0x00001000
#define COMP_ELEM_RESTORED_CSI  0x00002000
#define COMP_ELEM_CURITEMSTATE  0x00004000

#define COMP_ELEM_ALL   (COMP_ELEM_TYPE | COMP_ELEM_CHECKED | COMP_ELEM_DIRTY |                     \
                         COMP_ELEM_NOSCROLL | COMP_ELEM_POS_LEFT | COMP_ELEM_SIZE_WIDTH  |          \
                         COMP_ELEM_SIZE_HEIGHT | COMP_ELEM_POS_ZINDEX | COMP_ELEM_SOURCE |          \
                         COMP_ELEM_FRIENDLYNAME | COMP_ELEM_POS_TOP | COMP_ELEM_SUBSCRIBEDURL |     \
                         COMP_ELEM_ORIGINAL_CSI | COMP_ELEM_RESTORED_CSI | COMP_ELEM_CURITEMSTATE)


////////////////////////////////////////////
// Flags for IActiveDesktop::AddDesktopItemWithUI()
typedef enum tagDTI_ADTIWUI
{
    DTI_ADDUI_DEFAULT               = 0x00000000,
    DTI_ADDUI_DISPSUBWIZARD         = 0x00000001,
    DTI_ADDUI_POSITIONITEM          = 0x00000002,
};


////////////////////////////////////////////
// Flags for IActiveDesktop::AddUrl()
#define ADDURL_SILENT           0X0001


////////////////////////////////////////////
// Default positions for ADI
#define COMPONENT_DEFAULT_LEFT    (0xFFFF)
#define COMPONENT_DEFAULT_TOP     (0xFFFF)




//
//  Interface for manipulating the Active Desktop.
//

#undef INTERFACE
#define INTERFACE IActiveDesktop

DECLARE_INTERFACE_( IActiveDesktop, IUnknown )
{
    // IUnknown methods
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

    // IActiveDesktop methods
    STDMETHOD (ApplyChanges)(THIS_ DWORD dwFlags) PURE;
    STDMETHOD (GetWallpaper)(THIS_ LPWSTR pwszWallpaper, UINT cchWallpaper, DWORD dwReserved) PURE;
    STDMETHOD (SetWallpaper)(THIS_ LPCWSTR pwszWallpaper, DWORD dwReserved) PURE;
    STDMETHOD (GetWallpaperOptions)(THIS_ LPWALLPAPEROPT pwpo, DWORD dwReserved) PURE;
    STDMETHOD (SetWallpaperOptions)(THIS_ LPCWALLPAPEROPT pwpo, DWORD dwReserved) PURE;
    STDMETHOD (GetPattern)(THIS_ LPWSTR pwszPattern, UINT cchPattern, DWORD dwReserved) PURE;
    STDMETHOD (SetPattern)(THIS_ LPCWSTR pwszPattern, DWORD dwReserved) PURE;
    STDMETHOD (GetDesktopItemOptions)(THIS_ LPCOMPONENTSOPT pco, DWORD dwReserved) PURE;
    STDMETHOD (SetDesktopItemOptions)(THIS_ LPCCOMPONENTSOPT pco, DWORD dwReserved) PURE;
    STDMETHOD (AddDesktopItem)(THIS_ LPCCOMPONENT pcomp, DWORD dwReserved) PURE;
    STDMETHOD (AddDesktopItemWithUI)(THIS_ HWND hwnd, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
    STDMETHOD (ModifyDesktopItem)(THIS_ LPCCOMPONENT pcomp, DWORD dwFlags) PURE;
    STDMETHOD (RemoveDesktopItem)(THIS_ LPCCOMPONENT pcomp, DWORD dwReserved) PURE;
    STDMETHOD (GetDesktopItemCount)(THIS_ LPINT lpiCount, DWORD dwReserved) PURE;
    STDMETHOD (GetDesktopItem)(THIS_ int nComponent, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
    STDMETHOD (GetDesktopItemByID)(THIS_ ULONG_PTR dwID, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
    STDMETHOD (GenerateDesktopItemHtml)(THIS_ LPCWSTR pwszFileName, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
    STDMETHOD (AddUrl)(THIS_ HWND hwnd, LPCWSTR pszSource, LPCOMPONENT pcomp, DWORD dwFlags) PURE;
    STDMETHOD (GetDesktopItemBySource)(THIS_ LPCWSTR pwszSource, LPCOMPONENT pcomp, DWORD dwReserved) PURE;
};

typedef IActiveDesktop * LPACTIVEDESKTOP;


#endif // _WININET_

#if (_WIN32_IE >= 0x0500)

#define MAX_COLUMN_NAME_LEN 80
#define MAX_COLUMN_DESC_LEN 128

typedef struct {
    ULONG   dwFlags ;             // initialization flags
    ULONG   dwReserved ;          // reserved for future use.
    WCHAR   wszFolder[MAX_PATH];  // fully qualified folder path (or empty if multiple folders)
} SHCOLUMNINIT, *LPSHCOLUMNINIT;
typedef const SHCOLUMNINIT* LPCSHCOLUMNINIT;

typedef struct {
    SHCOLUMNID  scid;                           // OUT the unique identifier of this column
    VARTYPE     vt;                             // OUT the native type of the data returned
    DWORD       fmt;                            // OUT this listview format (LVCFMT_LEFT, usually)
    UINT        cChars;                         // OUT the default width of the column, in characters
    DWORD       csFlags;                        // OUT SHCOLSTATE flags
    WCHAR wszTitle[MAX_COLUMN_NAME_LEN];        // OUT the title of the column
    WCHAR wszDescription[MAX_COLUMN_DESC_LEN];  // OUT full description of this column
} SHCOLUMNINFO, *LPSHCOLUMNINFO ;
typedef const SHCOLUMNINFO* LPCSHCOLUMNINFO ;

#define SHCDF_UPDATEITEM        0x00000001      // this flag is a hint that the file has changed since the last call to GetItemData

typedef struct {
    ULONG   dwFlags ;            // combination of SHCDF_ flags.
    DWORD   dwFileAttributes ;   // file attributes.
    ULONG   dwReserved ;         // reserved for future use.
    WCHAR*  pwszExt ;            // address of file name extension
    WCHAR   wszFile[MAX_PATH] ;  // Absolute path of file.
} SHCOLUMNDATA, *LPSHCOLUMNDATA ;
typedef const SHCOLUMNDATA* LPCSHCOLUMNDATA ;

#undef INTERFACE
#define INTERFACE IColumnProvider

// Note: these objects must be threadsafe!  GetItemData _will_ be called
// simultaneously from multiple threads.
DECLARE_INTERFACE_(IColumnProvider, IUnknown)
{
    // IUnknown methods
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // IColumnProvider methods
    STDMETHOD (Initialize)(THIS_ LPCSHCOLUMNINIT psci) PURE;
    STDMETHOD (GetColumnInfo)(THIS_ DWORD dwIndex, SHCOLUMNINFO *psci) PURE;
    STDMETHOD (GetItemData)(THIS_ LPCSHCOLUMNID pscid, LPCSHCOLUMNDATA pscd, VARIANT *pvarData) PURE;
};


///////////////////////////////////////////////////////
//
// Drag and Drop helper
//
// Purpose: To expose the Shell drag images
//
// This interface is implemented in the shell by CLSID_DragDropHelper.
//
// To use:
//   If you are the source of a drag (i.e. in response to LV_DRAGBEGIN or
//    equivelent begin drag message) call
//    IDragSourceHelper::InitializeFromWindow
//              (<hwnd of window supporting DI_GETDRAGIMAGE>,
//               <pointer to POINT indicating offset to the mouse from
//                  the upper left corner of the image>,
//               <pointer to data object>)
//
//      NOTE: The Data object must support IDataObject::SetData with multiple
//            data types and GetData must implement data type cloning
//            (Including HGLOBAL), not just aliasing.
//
//   If you wish to have an image while over your application add the
//    IDragImages::Dr* calls to your IDropTarget implementation. For Example:
//
//    STDMETHODIMP CUserDropTarget::DragEnter(IDataObject* pDataObject,
//                                            DWORD grfKeyState,
//                                            POINTL pt, DWORD* pdwEffect)
//    {
//          // Process your DragEnter
//          // Call IDragImages::DragEnter last.
//          _pDropTargetHelper->DragEnter(_hwndDragOver, pDataObject,
//                                        (POINT*)&pt, *pdwEffect);
//          return hres;
//    }
//
//
//   If you wish to be able to source a drag image from a custom control,
//     implement a handler for the RegisterWindowMessage(DI_GETDRAGIMAGE).
//     The LPARAM is a pointer to an SHDRAGIMAGE structure.
//
//      sizeDragImage  -   Calculate the length and width required to render
//                          the images.
//      ptOffset       -   Calculate the offset from the upper left corner to
//                          the mouse cursor within the image
//      hbmpDragImage  -   CreateBitmap( sizeDragImage.cx, sizeDragImage.cy,
//                           GetDeviceCaps(hdcScreen, PLANES),
//                           GetDeviceCaps(hdcScreen, BITSPIXEL),
//                           NULL);
//
//   Drag Images will only be displayed on Windows NT 5.0 or later.
//
//
//   Note about IDropTargetHelper::Show - This method is provided for
//     showing/hiding the Drag image in low color depth video modes. When
//     painting to a window that is currently being dragged over (i.e. For
//     indicating a selection) you need to hide the drag image by calling this
//     method passing FALSE. After the window is done painting, Show the image
//     again by passing TRUE.

// This is sent to a window to get the rendered images to a bitmap
typedef struct
{
    SIZE        sizeDragImage;      // OUT - The length and Width of the
                                    //        rendered image
    POINT       ptOffset;           // OUT - The Offset from the mouse cursor to
                                    //        the upper left corner of the image
    HBITMAP     hbmpDragImage;      // OUT - The Bitmap containing the rendered
                                    //        drag images
    COLORREF    crColorKey;         // OUT - The COLORREF that has been blitted
                                    //        to the background of the images
} SHDRAGIMAGE, *LPSHDRAGIMAGE;

// Call RegisterWindowMessage to get the ID
#define DI_GETDRAGIMAGE     TEXT("ShellGetDragImage")

#undef INTERFACE
#define INTERFACE IDropTargetHelper

DECLARE_INTERFACE_( IDropTargetHelper, IUnknown )
{
    // IUnknown methods
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

    // IDropTargetHelper
    STDMETHOD (DragEnter)(THIS_ HWND hwndTarget, IDataObject* pDataObject,
                          POINT* ppt, DWORD dwEffect) PURE;
    STDMETHOD (DragLeave)(THIS) PURE;
    STDMETHOD (DragOver)(THIS_ POINT* ppt, DWORD dwEffect) PURE;
    STDMETHOD (Drop)(THIS_ IDataObject* pDataObject, POINT* ppt,
                     DWORD dwEffect) PURE;
    STDMETHOD (Show)(THIS_ BOOL fShow) PURE;

};

#undef INTERFACE
#define INTERFACE IDragSourceHelper

DECLARE_INTERFACE_( IDragSourceHelper, IUnknown )
{
    // IUnknown methods
    STDMETHOD (QueryInterface)(THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG, AddRef) ( THIS ) PURE;
    STDMETHOD_(ULONG, Release) ( THIS ) PURE;

    // IDragSourceHelper
    STDMETHOD (InitializeFromBitmap)(THIS_ LPSHDRAGIMAGE pshdi,
                                     IDataObject* pDataObject) PURE;
    STDMETHOD (InitializeFromWindow)(THIS_ HWND hwnd, POINT* ppt,
                                     IDataObject* pDataObject) PURE;
};
#endif // _WIN32_IE >= 0x0500
#endif // _WIN32_IE

//==========================================================================
// Clipboard format which may be supported by IDataObject from system
// defined shell folders (such as directories, network, ...).
//==========================================================================

#define CFSTR_SHELLIDLIST       TEXT("Shell IDList Array")      // CF_IDLIST
#define CFSTR_SHELLIDLISTOFFSET TEXT("Shell Object Offsets")    // CF_OBJECTPOSITIONS
#define CFSTR_NETRESOURCES      TEXT("Net Resource")            // CF_NETRESOURCE
#define CFSTR_FILEDESCRIPTORA   TEXT("FileGroupDescriptor")     // CF_FILEGROUPDESCRIPTORA
#define CFSTR_FILEDESCRIPTORW   TEXT("FileGroupDescriptorW")    // CF_FILEGROUPDESCRIPTORW
#define CFSTR_FILECONTENTS      TEXT("FileContents")            // CF_FILECONTENTS
#define CFSTR_FILENAMEA         TEXT("FileName")                // CF_FILENAMEA
#define CFSTR_FILENAMEW         TEXT("FileNameW")               // CF_FILENAMEW
#define CFSTR_PRINTERGROUP      TEXT("PrinterFriendlyName")     // CF_PRINTERS
#define CFSTR_FILENAMEMAPA      TEXT("FileNameMap")             // CF_FILENAMEMAPA
#define CFSTR_FILENAMEMAPW      TEXT("FileNameMapW")            // CF_FILENAMEMAPW
#define CFSTR_SHELLURL          TEXT("UniformResourceLocator")
#define CFSTR_PREFERREDDROPEFFECT TEXT("Preferred DropEffect")
#define CFSTR_PERFORMEDDROPEFFECT TEXT("Performed DropEffect")
#define CFSTR_PASTESUCCEEDED    TEXT("Paste Succeeded")
#define CFSTR_INDRAGLOOP        TEXT("InShellDragLoop")
#define CFSTR_DRAGCONTEXT       TEXT("DragContext")
#define CFSTR_MOUNTEDVOLUME     TEXT("MountedVolume")
#define CFSTR_PERSISTEDDATAOBJECT     TEXT("PersistedDataObject")
#define CFSTR_TARGETCLSID		TEXT("TargetCLSID")				// HGLOBAL with a CLSID of the drop target
#define CFSTR_LOGICALPERFORMEDDROPEFFECT  TEXT("Logical Performed DropEffect")

#ifdef UNICODE
#define CFSTR_FILEDESCRIPTOR    CFSTR_FILEDESCRIPTORW
#define CFSTR_FILENAME          CFSTR_FILENAMEW
#define CFSTR_FILENAMEMAP       CFSTR_FILENAMEMAPW
#else
#define CFSTR_FILEDESCRIPTOR    CFSTR_FILEDESCRIPTORA
#define CFSTR_FILENAME          CFSTR_FILENAMEA
#define CFSTR_FILENAMEMAP       CFSTR_FILENAMEMAPA
#endif

#define DVASPECT_SHORTNAME      2 // use for CF_HDROP to get short name version of file paths
#define DVASPECT_COPY           3 // use to indicate format is a "Copy" of the data (FILECONTENTS, FILEDESCRIPTOR, etc)
#define DVASPECT_LINK           4 // use to indicate format is a "Shortcut" to the data (FILECONTENTS, FILEDESCRIPTOR, etc)

//
// format of CF_NETRESOURCE
//
typedef struct _NRESARRAY {     // anr
    UINT cItems;
    NETRESOURCE nr[1];
} NRESARRAY, * LPNRESARRAY;

//
// format of CF_IDLIST
//
typedef struct _IDA {
    UINT cidl;          // number of relative IDList
    UINT aoffset[1];    // [0]: folder IDList, [1]-[cidl]: item IDList
} CIDA, * LPIDA;

//
// FILEDESCRIPTOR.dwFlags field indicate which fields are to be used
//
typedef enum {
    FD_CLSID            = 0x0001,
    FD_SIZEPOINT        = 0x0002,
    FD_ATTRIBUTES       = 0x0004,
    FD_CREATETIME       = 0x0008,
    FD_ACCESSTIME       = 0x0010,
    FD_WRITESTIME       = 0x0020,
    FD_FILESIZE         = 0x0040,
    FD_PROGRESSUI       = 0x4000,       // Show Progress UI w/Drag and Drop
    FD_LINKUI           = 0x8000,       // 'link' UI is prefered
} FD_FLAGS;

typedef struct _FILEDESCRIPTORA { // fod
    DWORD dwFlags;

    CLSID clsid;
    SIZEL sizel;
    POINTL pointl;

    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    CHAR   cFileName[ MAX_PATH ];
} FILEDESCRIPTORA, *LPFILEDESCRIPTORA;

typedef struct _FILEDESCRIPTORW { // fod
    DWORD dwFlags;

    CLSID clsid;
    SIZEL sizel;
    POINTL pointl;

    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    WCHAR  cFileName[ MAX_PATH ];
} FILEDESCRIPTORW, *LPFILEDESCRIPTORW;

#ifdef UNICODE
#define FILEDESCRIPTOR      FILEDESCRIPTORW
#define LPFILEDESCRIPTOR    LPFILEDESCRIPTORW
#else
#define FILEDESCRIPTOR      FILEDESCRIPTORA
#define LPFILEDESCRIPTOR    LPFILEDESCRIPTORA
#endif

//
// format of CF_FILEGROUPDESCRIPTOR
//
typedef struct _FILEGROUPDESCRIPTORA { // fgd
     UINT cItems;
     FILEDESCRIPTORA fgd[1];
} FILEGROUPDESCRIPTORA, * LPFILEGROUPDESCRIPTORA;

typedef struct _FILEGROUPDESCRIPTORW { // fgd
     UINT cItems;
     FILEDESCRIPTORW fgd[1];
} FILEGROUPDESCRIPTORW, * LPFILEGROUPDESCRIPTORW;

#ifdef UNICODE
#define FILEGROUPDESCRIPTOR     FILEGROUPDESCRIPTORW
#define LPFILEGROUPDESCRIPTOR   LPFILEGROUPDESCRIPTORW
#else
#define FILEGROUPDESCRIPTOR     FILEGROUPDESCRIPTORA
#define LPFILEGROUPDESCRIPTOR   LPFILEGROUPDESCRIPTORA
#endif

//
// format of CF_HDROP and CF_PRINTERS, in the HDROP case the data that follows
// is a double null terinated list of file names, for printers they are printer
// friendly names
//
typedef struct _DROPFILES {
   DWORD pFiles;                       // offset of file list
   POINT pt;                           // drop point (client coords)
   BOOL fNC;                           // is it on NonClient area
                                       // and pt is in screen coords
   BOOL fWide;                         // WIDE character switch
} DROPFILES, *LPDROPFILES;


//====== File System Notification APIs ===============================
//

//
//  File System Notification flags
//

#define SHCNE_RENAMEITEM          0x00000001L
#define SHCNE_CREATE              0x00000002L
#define SHCNE_DELETE              0x00000004L
#define SHCNE_MKDIR               0x00000008L
#define SHCNE_RMDIR               0x00000010L
#define SHCNE_MEDIAINSERTED       0x00000020L
#define SHCNE_MEDIAREMOVED        0x00000040L
#define SHCNE_DRIVEREMOVED        0x00000080L
#define SHCNE_DRIVEADD            0x00000100L
#define SHCNE_NETSHARE            0x00000200L
#define SHCNE_NETUNSHARE          0x00000400L
#define SHCNE_ATTRIBUTES          0x00000800L
#define SHCNE_UPDATEDIR           0x00001000L
#define SHCNE_UPDATEITEM          0x00002000L
#define SHCNE_SERVERDISCONNECT    0x00004000L
#define SHCNE_UPDATEIMAGE         0x00008000L
#define SHCNE_DRIVEADDGUI         0x00010000L
#define SHCNE_RENAMEFOLDER        0x00020000L
#define SHCNE_FREESPACE           0x00040000L

#if (_WIN32_IE >= 0x0400)
// SHCNE_EXTENDED_EVENT: the extended event is identified in dwItem1,
// packed in LPITEMIDLIST format (same as SHCNF_DWORD packing).
// Additional information can be passed in the dwItem2 parameter
// of SHChangeNotify (called "pidl2" below), which if present, must also
// be in LPITEMIDLIST format.
//
// Unlike the standard events, the extended events are ORDINALs, so we
// don't run out of bits.  Extended events follow the SHCNEE_* naming
// convention.
//
// The dwItem2 parameter varies according to the extended event.

#define SHCNE_EXTENDED_EVENT      0x04000000L
#endif      // _WIN32_IE >= 0x0400

#define SHCNE_ASSOCCHANGED        0x08000000L

#define SHCNE_DISKEVENTS          0x0002381FL
#define SHCNE_GLOBALEVENTS        0x0C0581E0L // Events that dont match pidls first
#define SHCNE_ALLEVENTS           0x7FFFFFFFL
#define SHCNE_INTERRUPT           0x80000000L // The presence of this flag indicates
                                            // that the event was generated by an
                                            // interrupt.  It is stripped out before
                                            // the clients of SHCNNotify_ see it.

#if (_WIN32_IE >= 0x0400)
// SHCNE_EXTENDED_EVENT extended events.  These events are ordinals.
// This is not a bitfield.

#define SHCNEE_ORDERCHANGED         2L  // pidl2 is the changed folder
#define SHCNEE_MSI_CHANGE           4L  // pidl2 is a SHChangeProductKeyAsIDList
#define SHCNEE_MSI_UNINSTALL        5L  // pidl2 is a SHChangeProductKeyAsIDList
#endif


// Flags
// uFlags & SHCNF_TYPE is an ID which indicates what dwItem1 and dwItem2 mean
#define SHCNF_IDLIST      0x0000        // LPITEMIDLIST
#define SHCNF_PATHA       0x0001        // path name
#define SHCNF_PRINTERA    0x0002        // printer friendly name
#define SHCNF_DWORD       0x0003        // DWORD
#define SHCNF_PATHW       0x0005        // path name
#define SHCNF_PRINTERW    0x0006        // printer friendly name
#define SHCNF_TYPE        0x00FF
#define SHCNF_FLUSH       0x1000
#define SHCNF_FLUSHNOWAIT 0x2000

#ifdef UNICODE
#define SHCNF_PATH      SHCNF_PATHW
#define SHCNF_PRINTER   SHCNF_PRINTERW
#else
#define SHCNF_PATH      SHCNF_PATHA
#define SHCNF_PRINTER   SHCNF_PRINTERA
#endif


//
//  APIs
//
SHSTDAPI_(void) SHChangeNotify(LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2);

//
// IShellChangeNotify
//
#undef  INTERFACE
#define INTERFACE  IShellChangeNotify

DECLARE_INTERFACE_(IShellChangeNotify, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IShellChangeNotify methods ***
    STDMETHOD(OnChange) (THIS_ LONG lEvent, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2) PURE;
} ;

//
// IQueryInfo
//
#undef  INTERFACE
#define INTERFACE  IQueryInfo

DECLARE_INTERFACE_(IQueryInfo, IUnknown)
{
    // *** IUnknown methods ***
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, void **ppv) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

    // *** IQueryInfo methods ***
    STDMETHOD(GetInfoTip)(THIS_ DWORD dwFlags, WCHAR **ppwszTip) PURE;
    STDMETHOD(GetInfoFlags)(THIS_ DWORD *pdwFlags) PURE;
} ;

#define QITIPF_DEFAULT          0x00000000
#define QITIPF_USENAME          0x00000001
#define QITIPF_LINKNOTARGET     0x00000002
#define QITIPF_LINKUSETARGET    0x00000004


#define QIF_CACHED           0x00000001
#define QIF_DONTEXPANDFOLDER 0x00000002


//
// SHAddToRecentDocs
//
#define SHARD_PIDL      0x00000001L
#define SHARD_PATHA     0x00000002L
#define SHARD_PATHW     0x00000003L

#ifdef UNICODE
#define SHARD_PATH  SHARD_PATHW
#else
#define SHARD_PATH  SHARD_PATHA
#endif

SHSTDAPI_(void) SHAddToRecentDocs(UINT uFlags, LPCVOID pv);
#if (_WIN32_IE >= 0x0400)
typedef struct _SHChangeProductKeyAsIDList {
    USHORT cb;
    WCHAR wszProductKey[39];
    USHORT cbZero;
} SHChangeProductKeyAsIDList, *LPSHChangeProductKeyAsIDList;

SHSTDAPI_(void) SHUpdateImageA(LPCSTR pszHashItem, int iIndex, UINT uFlags, int iImageIndex);
SHSTDAPI_(void) SHUpdateImageW(LPCWSTR pszHashItem, int iIndex, UINT uFlags, int iImageIndex);
#ifdef UNICODE
#define SHUpdateImage  SHUpdateImageW
#else
#define SHUpdateImage  SHUpdateImageA
#endif // !UNICODE
#endif /* _WIN32_IE */


SHSTDAPI SHGetInstanceExplorer(IUnknown **ppunk);

//
// SHGetDataFromIDListA/W
//
// SHGetDataFromIDList nFormat values TCHAR
#define SHGDFIL_FINDDATA        1
#define SHGDFIL_NETRESOURCE     2
#define SHGDFIL_DESCRIPTIONID   3

#define SHDID_ROOT_REGITEM          1
#define SHDID_FS_FILE               2
#define SHDID_FS_DIRECTORY          3
#define SHDID_FS_OTHER              4
#define SHDID_COMPUTER_DRIVE35      5
#define SHDID_COMPUTER_DRIVE525     6
#define SHDID_COMPUTER_REMOVABLE    7
#define SHDID_COMPUTER_FIXED        8
#define SHDID_COMPUTER_NETDRIVE     9
#define SHDID_COMPUTER_CDROM        10
#define SHDID_COMPUTER_RAMDISK      11
#define SHDID_COMPUTER_OTHER        12
#define SHDID_NET_DOMAIN            13
#define SHDID_NET_SERVER            14
#define SHDID_NET_SHARE             15
#define SHDID_NET_RESTOFNET         16
#define SHDID_NET_OTHER             17

typedef struct _SHDESCRIPTIONID {
    DWORD   dwDescriptionId;
    CLSID   clsid;
} SHDESCRIPTIONID, *LPSHDESCRIPTIONID;

// these delegate to IShellFolder2::GetItemData()

SHSTDAPI SHGetDataFromIDListA(IShellFolder *psf, LPCITEMIDLIST pidl, int nFormat, void *pv, int cb);
SHSTDAPI SHGetDataFromIDListW(IShellFolder *psf, LPCITEMIDLIST pidl, int nFormat, void *pv, int cb);
#ifdef UNICODE
#define SHGetDataFromIDList  SHGetDataFromIDListW
#else
#define SHGetDataFromIDList  SHGetDataFromIDListA
#endif // !UNICODE


//===========================================================================


//
// PROPIDs for Internet Shortcuts (FMTID_Intshcut) to be used with
// IPropertySetStorage/IPropertyStorage
//
// The known property ids and their variant types are:
//      PID_IS_URL          [VT_LPWSTR]   URL
//      PID_IS_NAME         [VT_LPWSTR]   Name of the internet shortcut
//      PID_IS_WORKINGDIR   [VT_LPWSTR]   Working directory for the shortcut
//      PID_IS_HOTKEY       [VT_UI2]      Hotkey for the shortcut
//      PID_IS_SHOWCMD      [VT_I4]       Show command for shortcut
//      PID_IS_ICONINDEX    [VT_I4]       Index into file that has icon
//      PID_IS_ICONFILE     [VT_LPWSTR]   File that has the icon
//      PID_IS_WHATSNEW     [VT_LPWSTR]   What's New text
//      PID_IS_AUTHOR       [VT_LPWSTR]   Author
//      PID_IS_DESCRIPTION  [VT_LPWSTR]   Description text of site
//      PID_IS_COMMENT      [VT_LPWSTR]   User annotated comment
//

#define PID_IS_URL           2
#define PID_IS_NAME          4
#define PID_IS_WORKINGDIR    5
#define PID_IS_HOTKEY        6
#define PID_IS_SHOWCMD       7
#define PID_IS_ICONINDEX     8
#define PID_IS_ICONFILE      9
#define PID_IS_WHATSNEW      10
#define PID_IS_AUTHOR        11
#define PID_IS_DESCRIPTION   12
#define PID_IS_COMMENT       13

//
// PROPIDs for Internet Sites (FMTID_InternetSite) to be used with
// IPropertySetStorage/IPropertyStorage
//
// The known property ids and their variant types are:
//      PID_INTSITE_WHATSNEW     [VT_LPWSTR]   What's New text
//      PID_INTSITE_AUTHOR       [VT_LPWSTR]   Author
//      PID_INTSITE_LASTVISIT    [VT_FILETIME] Time site was last visited
//      PID_INTSITE_LASTMOD      [VT_FILETIME] Time site was last modified
//      PID_INTSITE_VISITCOUNT   [VT_UI4]      Number of times user has visited
//      PID_INTSITE_DESCRIPTION  [VT_LPWSTR]   Description text of site
//      PID_INTSITE_COMMENT      [VT_LPWSTR]   User annotated comment
//      PID_INTSITE_RECURSE      [VT_UI4]      Levels to recurse (0-3)
//      PID_INTSITE_WATCH        [VT_UI4]      PIDISM_ flags
//      PID_INTSITE_SUBSCRIPTION [VT_UI8]      Subscription cookie
//      PID_INTSITE_URL          [VT_LPWSTR]   URL
//      PID_INTSITE_TITLE        [VT_LPWSTR]   Title
//      PID_INTSITE_CODEPAGE     [VT_UI4]      Codepage of the document
//      PID_INTSITE_TRACKING     [VT_UI4]      Tracking
//      PID_INTSITE_ICONINDEX    [VT_I4]       Retrieve the index to the icon
//      PID_INTSITE_ICONFILE     [VT_LPWSTR]   Retrieve the file containing the icon index.


#define PID_INTSITE_WHATSNEW      2
#define PID_INTSITE_AUTHOR        3
#define PID_INTSITE_LASTVISIT     4
#define PID_INTSITE_LASTMOD       5
#define PID_INTSITE_VISITCOUNT    6
#define PID_INTSITE_DESCRIPTION   7
#define PID_INTSITE_COMMENT       8
#define PID_INTSITE_FLAGS         9
#define PID_INTSITE_CONTENTLEN    10
#define PID_INTSITE_CONTENTCODE   11
#define PID_INTSITE_RECURSE       12
#define PID_INTSITE_WATCH         13
#define PID_INTSITE_SUBSCRIPTION  14
#define PID_INTSITE_URL           15
#define PID_INTSITE_TITLE         16
#define PID_INTSITE_CODEPAGE      18
#define PID_INTSITE_TRACKING      19
#define PID_INTSITE_ICONINDEX     20
#define PID_INTSITE_ICONFILE      21


// Flags for PID_IS_FLAGS
#define PIDISF_RECENTLYCHANGED  0x00000001
#define PIDISF_CACHEDSTICKY     0x00000002
#define PIDISF_CACHEIMAGES      0x00000010
#define PIDISF_FOLLOWALLLINKS   0x00000020

// Values for PID_INTSITE_WATCH
#define PIDISM_GLOBAL           0       // Monitor based on global setting
#define PIDISM_WATCH            1       // User says watch
#define PIDISM_DONTWATCH        2       // User says don't watch


////////////////////////////////////////////////////////////////////
//
// The shell keeps track of some per-user state to handle display
// options that is of major interest to ISVs.
// The key one requested right now is "DoubleClickInWebView".
//
//  SysFiles are these windows special files:
//      "dll sys vxd 386 drv"
//
//  hidden files are files with the FILE_ATTRIBUTE_HIDDEN attribute
//
//  system files are files with the FILE_ATTRIBUTE_SYSTEM attribute
//
//      fShowAllObjects fShowSysFiles   Result
//      --------------- -------------   ------
//      0               0               hide hidden + SysFiles + system files
//      0               1               hide hidden files.
//      1               0               show all files.
//      1               1               show all files.
//
typedef struct {
    BOOL fShowAllObjects : 1;
    BOOL fShowExtensions : 1;
    BOOL fNoConfirmRecycle : 1;
    BOOL fShowSysFiles : 1;
    BOOL fShowCompColor : 1;
    BOOL fDoubleClickInWebView : 1;
    BOOL fDesktopHTML : 1;
    BOOL fWin95Classic : 1;
    BOOL fDontPrettyPath : 1;
    BOOL fShowAttribCol : 1;
    BOOL fMapNetDrvBtn : 1;
    BOOL fShowInfoTip : 1;
    BOOL fHideIcons : 1;
    UINT fRestFlags : 3;
} SHELLFLAGSTATE, * LPSHELLFLAGSTATE;

#define SSF_SHOWALLOBJECTS          0x00000001
#define SSF_SHOWEXTENSIONS          0x00000002
#define SSF_SHOWCOMPCOLOR           0x00000008
#define SSF_SHOWSYSFILES            0x00000020
#define SSF_DOUBLECLICKINWEBVIEW    0x00000080
#define SSF_SHOWATTRIBCOL           0x00000100
#define SSF_DESKTOPHTML             0x00000200
#define SSF_WIN95CLASSIC            0x00000400
#define SSF_DONTPRETTYPATH          0x00000800
#define SSF_SHOWINFOTIP             0x00002000
#define SSF_MAPNETDRVBUTTON         0x00001000
#define SSF_NOCONFIRMRECYCLE        0x00008000
#define SSF_HIDEICONS               0x00004000

// SHGetSettings(LPSHELLFLAGSTATE lpss, DWORD dwMask)
//
// Specify the bits you are interested in in dwMask and they will be
// filled out in the lpss structure.
//
// When these settings change, a WM_SETTINGCHANGE message is sent
// with the string lParam value of "ShellState".
//
SHSTDAPI_(void) SHGetSettings(LPSHELLFLAGSTATE lpsfs, DWORD dwMask);

// SHBindToParent(LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlLast)
//
// Given a pidl, you can get an interface pointer (as specified by riid) of the pidl's parent folder (in ppv)
// If ppidlLast is non-NULL, you can also get the pidl of the last item.
//
STDAPI SHBindToParent(LPCITEMIDLIST pidl, REFIID riid, void **ppv, LPCITEMIDLIST *ppidlLast);



// SHPathPrepareForWrite(HWND hwnd, IUnknown *punkEnableModless, LPCTSTR pszPath, DWORD dwFlags)
//
// DESCRIPTION:
//     This API will prepare the path for the caller.  This includes:
// 1. Prompting for the ejectable media to be re-inserted. (Floppy, CD-ROM, ZIP drive, etc.)
// 2. Prompting for the media to be formatted. (Floppy, hard drive, etc.)
// 3. Remount mapped drives if the connection was lost. (\\unc\share mapped to N: becomes disconnected)
// 4. If the path doesn't exist, create it.  (SHPPFW_DIRCREATE and SHPPFW_ASKDIRCREATE)
// 5. Display an error if the media is read only. (SHPPFW_NOWRITECHECK not set)
//
// PARAMETERS:
//      hwnd: Parernt window for UI.  NULL means don't display UI. OPTIONAL
//      punkEnableModless: Parent that will be set to modal during UI using IOleInPlaceActiveObject::EnableModeless(). OPTIONAL
//      pszPath: Path to verify is valid for writting.  This can be a UNC or file drive path.  The path
//               should only contain directories.  Pass SHPPFW_IGNOREFILENAME if the last path segment
//               is always filename to ignore.
//      dwFlags: SHPPFW_* Flags to modify behavior
//
//-------------------------------------------------------------------------
#define SHPPFW_NONE             0x00000000
#define SHPPFW_DEFAULT          SHPPFW_DIRCREATE        // May change
#define SHPPFW_DIRCREATE        0x00000001              // Create the directory if it doesn't exist without asking the user.
#define SHPPFW_ASKDIRCREATE     0x00000002              // Create the directory if it doesn't exist after asking the user.
#define SHPPFW_IGNOREFILENAME   0x00000004              // Ignore the last item in pszPath because it's a file.  Example: pszPath="C:\DirA\DirB", only use "C:\DirA".
#define SHPPFW_NOWRITECHECK     0x00000008              // Caller only needs to read from the drive, so don't check if it's READ ONLY.

STDAPI SHPathPrepareForWriteA(HWND hwnd, IUnknown *punkEnableModless, LPCSTR pszPath, DWORD dwFlags);
STDAPI SHPathPrepareForWriteW(HWND hwnd, IUnknown *punkEnableModless, LPCWSTR pszPath, DWORD dwFlags);
#ifdef UNICODE
#define SHPathPrepareForWrite  SHPathPrepareForWriteW
#else
#define SHPathPrepareForWrite  SHPathPrepareForWriteA
#endif // !UNICODE



#ifdef __urlmon_h__
//    NOTE: urlmon.h must be included before shlobj.h to access this function.
//
//    SoftwareUpdateMessageBox
//
//    Provides a standard message box for the alerting the user that a software
//    update is available or installed. No UI will be displayed if there is no
//    update available or if the available update version is less than or equal
//    to the Advertised update version.
//
//    hWnd                - [in] Handle of owner window
//    szDistUnit          - [in] Unique identifier string for a code distribution unit. For
//                               ActiveX controls and Active Setup installed components, this
//                               is typically a GUID string.
//    dwFlags             - [in] Must be 0.
//    psdi                - [in,out] Pointer to SOFTDISTINFO ( see URLMon.h ). May be NULL.
//                                cbSize should be initialized
//                                by the caller to sizeof(SOFTDISTINFO), dwReserved should be set to 0.
//
//    RETURNS:
//
//    IDNO     - The user chose cancel. If *pbRemind is FALSE, the caller should save the
//               update version from the SOFTDISTINFO and pass it in as the Advertised
//               version in future calls.
//
//    IDYES    - The user has selected Update Now/About Update. The caller should navigate to
//               the SOFTDISTINFO's pszHREF to initiate the install or learn about it.
//               The caller should save the update version from the SOFTDISTINFO and pass
//               it in as the Advertised version in future calls.
//
//    IDIGNORE - There is no pending software update. Note: There is
//               no Ignore button in the standard UI. This occurs if the available
//               version is less than the installed version or is not present or if the
//               Advertised version is greater than or equal to the update version.
//
//    IDABORT  - An error occured. Call GetSoftwareUpdateInfo() for a more specific HRESULT.
//               Note: There is no Abort button in the standard UI.


SHDOCAPI_(DWORD) SoftwareUpdateMessageBox( HWND hWnd,
                                           LPCWSTR szDistUnit,
                                           DWORD dwFlags,
                                           LPSOFTDISTINFO psdi );
#endif // if __urlmon_h__



#ifdef __cplusplus
}

#endif  /* __cplusplus */

#ifndef RC_INVOKED
#include <poppack.h>
#endif  /* !RC_INVOKED */

#endif // _SHLOBJ_H_

