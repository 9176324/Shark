#ifndef _WMICLI_H_INCLUDED

#define _WMICLI_H_INCLUDED

//
// Uncomment the following two lines to create UNICODE version.
//

// #define UNICODE
// #define _UNICODE

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <tchar.h>
#include <wchar.h>
#include <commctrl.h>        // For common controls, e.g. Tree

//
// CLSID_WbemLocator GUID is defined in wbemcli.h only for C++ programs.
// So, we need to define it ourselves. In order to define the CLSID_WbemLocator
// we must define the macro INITGUID before including ole2.h file otherwise
// DEFINE_GUID will simply declare the CLSID_WbemLocator as extern.
//
                                                               
#define INITGUID         
#include <ole2.h>

//
// COBJMACROS must be defined to include the macros from wbemcli.h that are used
// to invoke methods of WBEM objects when the code is in C.
//

#define COBJMACROS
#include <wbemcli.h>

//
// Resource editor generated header file.
//

#include "resource.h"

//
// Default namespace
//

#define NAMESPACE              TEXT("root\\CIMV2")

#define MAX_MSG_SIZE           256
#define MAX_DATE_STRING_SIZE   20

//
// To enumerate classes with Network Adapter as the root, comment the following
// line and uncomment the next one.
//

#define ROOT_CLASS             NULL

//#define ROOT_CLASS             TEXT("CIM_NetworkAdapter")

//
// ID of the status window
//

#define ID_STATUS              10

//
// Define the CLSID_WbemLocator. It is defined in wbemcli.h only for C++ programs.
//

DEFINE_GUID(CLSID_WbemLocator,
 0x4590f811, 0x1d3a, 0x11d0, 0x89, 0x1f,0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24 );

//
// Define the IID_IWbemLocator. It is defined in wbemcli.h only for C++ programs.
//

DEFINE_GUID(IID_IWbemLocator,
 0xdc12a687, 0x737f, 0x11cf, 0x88, 0x4d, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24 );

typedef struct _SCALAR_VALUE {
  BOOL             PropertyModified;
  LPTSTR           PropertyName;
  LPTSTR           PropertyType;
  LPTSTR           PropertyValue;
} SCALAR_VALUE, *LPSCALAR_VALUE;

typedef struct _ARRAY_VALUE {
  BOOL             PropertyModified;
  LPTSTR           PropertyName;
  LPTSTR           PropertyType;
  VARIANT          *PropertyValue;
} ARRAY_VALUE, *LPARRAY_VALUE;


typedef struct _PROPERTY_VALUE {
  LPTSTR           PropertyName;
  LPTSTR           PropertyType;
  VARIANT          PropertyValue;
} PROPERTY_VALUE, *LPPROPERTY_VALUE;

typedef struct _METHOD_INFO {
  IWbemClassObject *pInstArgs;
  SAFEARRAY        *psaInArgs;
  SAFEARRAY        *psaType;
  SAFEARRAY        *psaValues;
  LPTSTR           lpMethod;
} METHOD_INFO, *LPMETHOD_INFO;
  
//
// Function prototypes.
//

//
// The prototype for this function is already declared in the OLE header file
// objbase.h but only if the program is compiled as a C++ program. So, we copy
// the prototype here.
//

WINOLEAPI CoSetProxyBlanket (
               IUnknown                 *pProxy,
               DWORD                     dwAuthnSvc,
               DWORD                     dwAuthzSvc,
               OLECHAR                  *pServerPrincName,
               DWORD                     dwAuthnLevel,
               DWORD                     dwImpLevel,
               RPC_AUTH_IDENTITY_HANDLE  pAuthInfo,
               DWORD                     dwCapabilities );



INT_PTR CALLBACK DlgProc (
               HWND hwndDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam);

INT_PTR CALLBACK DlgInstProc (
               HWND hwndDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam);

INT_PTR CALLBACK DlgClassProc (HWND hwndDlg,
                               UINT uMsg,
                               WPARAM wParam,
                               LPARAM lParam);

INT_PTR CALLBACK DlgScalarPropProc (
               HWND hwndDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam);

INT_PTR CALLBACK DlgDatePropProc (
               HWND hwndDlg,
               UINT uMsg,
               WPARAM wParam,
               LPARAM lParam);

VOID ConnectToNamespace ( HWND hwndDlg);

VOID EnumClasses (
              HWND hwndTree,
              HTREEITEM hTreeParent,
              BSTR bstrClassName);

VOID EnumClassInst (
              HWND hwndDlg,
              LPNM_TREEVIEW lpnmTreeView);

VOID ListInstances (
              HWND hwndDlg,
              LPTSTR lpClassName);

VOID ListProperties (
              HWND hwndDlg,
              IWbemClassObject *pRef);

VOID ListMethods (
              HWND hwndDlg,
              IWbemClassObject *pRef);

IWbemClassObject *GetClassReference (HWND hwndDlg);

LPTSTR GetSelectedClassName (HWND hwndDlg);

IWbemClassObject *GetInstReference (
              HWND hwndDlg,
              int iIndex);

LPTSTR GetSelectedItemName (
              HWND hwndParent,
              int iCtlId,
              int iItemIndex);

VOID ShowClassPropertyValue (HWND hwndDlg);

VOID ShowInstPropertyValue (HWND hwndDlg);


VOID ShowScalarProperty (HWND hwndDlg,
                         IWbemClassObject *pRef,
                         LPPROPERTY_VALUE pPropValue);

VOID ShowScalarProperty (HWND hwndDlg,
                         IWbemClassObject *pRef,
                         LPPROPERTY_VALUE pPropValue);

VOID ModifyScalarProperty (IWbemClassObject *pRef,
                           LPSCALAR_VALUE lpScalarValue,
                           USHORT vt);

VOID ShowArrayProperty (HWND hwndDlg,
                        IWbemClassObject *pRef,
                        LPPROPERTY_VALUE pPropValue);

VOID ModifyArrayProperty (IWbemClassObject *pRef,
                          LPARRAY_VALUE lpArrayValue);

BOOL GetPropertyValue (HWND hwndDlg,
                       IWbemClassObject *pRef,
                       LPPROPERTY_VALUE lpPropValue);

VOID ExecuteMethod (HWND hwnddDlg);


VOID DisplayArray (HWND hwndList,
                   VARIANT *pva);

VOID UpdateArray (HWND hwndList,
                  VARIANT *psa);

VOID AddArgColumns (HWND hwndDlg,
                    LPMETHOD_INFO lpMethodInfo);

HTREEITEM AddToTree (
              HWND hwndTree,
              HTREEITEM hParent,
              LPTSTR lpClassName);

BSTR AnsiToBstr (
              LPTSTR lpSrc,
              int nLenSrc);

LPTSTR BstrToAnsi (
              BSTR lpSrc,
              int nLenSrc);


#endif // _WMICLI_H_INCLUDED
