#ifndef _WMICON_H_INCLUDED

#define _WMICON_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <tchar.h>
#include <wchar.h>

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
// Define the CLSID_WbemLocator. It is defined in wbemcli.h only for C++ programs.
//

DEFINE_GUID(CLSID_WbemLocator,
 0x4590f811, 0x1d3a, 0x11d0, 0x89, 0x1f,0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24 );

//
// Define the IID_IWbemLocator. It is defined in wbemcli.h only for C++ programs.
//

DEFINE_GUID(IID_IWbemLocator,
 0xdc12a687, 0x737f, 0x11cf, 0x88, 0x4d, 0x00, 0xaa, 0x00, 0x4b, 0x2e, 0x24 );

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
               DWORD                     dwCapabilities);



IWbemServices *ConnectToNamespace (LPTSTR chNamespace);
VOID DisplayPropertyValue (IWbemServices *pIWbemServices, LPTSTR lpClassName,
                           LPTSTR lpInstanceName, LPTSTR lpProperty);
IWbemClassObject *GetInstanceReference (IWbemServices *pIWbemServices,
                                        LPTSTR lpClassName,
                                        LPTSTR lpInstanceName);
BOOL IsInstance (IWbemClassObject *pInst, LPTSTR lpInstanceName);
BOOL GetPropertyValue (IWbemClassObject *pRef, LPTSTR lpPropertyName, 
                       VARIANT *pvaPropertyValue, LPTSTR *lppPropertyType);
VOID DisplayArray (VARIANT *pva);
VOID DisplayScalar (VARIANT *pvaPropValue);
BSTR AnsiToBstr (LPTSTR lpSrc, int nLenSrc);
LPTSTR BstrToAnsi (BSTR lpSrc, int nLenSrc);

#endif // _WMICON_H_INCLUDED