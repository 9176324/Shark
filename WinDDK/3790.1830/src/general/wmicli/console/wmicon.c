// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//
// Copyright 1999 Microsoft Corporation.  All Rights Reserved.
//
// PROGRAM: wmicon.c
//
// AUTHOR:  Alok Sinha January 6, 2000
//
// PURPOSE: Demonstrates how to enumerate WBEM classes, query property types,
//          and methods.
//
// ENVIRONMENT: Windows 2000 user mode application.
//

#include "wmicon.h"

void __cdecl _tmain (int argc, LPTSTR argv[])
{
  IWbemServices *pIWbemServices = NULL;
  HRESULT       hResult;

  _tprintf( TEXT("\n") );

  if ( argc < 5 ) {
     _tprintf(
         TEXT("Command line syntax:\n")
         TEXT("wmicon <namespace> <class name> <instance name> <property name>\n\n")
         TEXT("Names containing spaces must be in \" and embeded \" must be preceded by a \\.\n\n")
         TEXT("Example:\n wmicon ")
         TEXT("root\\WMI  MSNdis_MediaSupported  \"MSNdis_MediaSupported.InstanceName=\\\"WAN Miniport (NetBEUI, Dial Out)\\\"\"  NumberElements\n\n")
         TEXT("Where:\n")
         TEXT("Namespace: root\\WMI\n")
         TEXT("Class: MSNdis_MediaSupported\n")
         TEXT("Instance:  MSNdis_MediaSupported.InstanceName=\"WAN Miniport (NetBEUI, Dial Out)\"\n")
         TEXT("Property: NumberElements\n") );
      return;
  }


  //
  // Initialize COM library. Must be done before invoking any
  // other COM function.
  //

  hResult = CoInitialize( NULL );

  if ( hResult  != S_OK ) {
     _tprintf( TEXT("Error %lx: Failed to initialize COM library, program exiting...\n"),
               hResult );
  }
  else {
     pIWbemServices = ConnectToNamespace( argv[1] );

     if ( pIWbemServices ) {
        DisplayPropertyValue( pIWbemServices, argv[2], argv[3], argv[4] );

        IWbemServices_Release( pIWbemServices );
     }

     CoUninitialize();
  }


  return;
}

//
// The function connects to the namespace specified by the user.
//

IWbemServices *ConnectToNamespace (LPTSTR chNamespace)
{
  IWbemServices *pIWbemServices = NULL;
  IWbemLocator  *pIWbemLocator = NULL;
  BSTR          bstrNamespace;
  HRESULT       hResult;

  //
  // Create an instance of WbemLocator interface.
  //

  hResult = CoCreateInstance(
                           &CLSID_WbemLocator,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           &IID_IWbemLocator,
                           (LPVOID *)&pIWbemLocator );

  if ( hResult != S_OK ) {
     _tprintf( TEXT("Error %lX: Could not create instance of IWbemLocator interface.\n"),
               hResult );

     return NULL;
  }

  //
  // Namespaces are passed to COM in BSTRs.
  //

  bstrNamespace = AnsiToBstr(
                        chNamespace,
                        -1 );

  if ( bstrNamespace == NULL ) {
     _tprintf( TEXT("Out of memory.\n") );

     IWbemLocator_Release( pIWbemLocator );
     return NULL;
  }

  //
  // Using the locator, connect to COM in the given namespace.
  //

  hResult = IWbemLocator_ConnectServer(
                  pIWbemLocator,
                  bstrNamespace,
                  NULL,   // NULL means current account, for simplicity.
                  NULL,   // NULL means current password, for simplicity.
                  0L,     // locale
                  0L,     // securityFlags
                  NULL,   // authority (domain for NTLM)
                  NULL,   // context
                  &pIWbemServices ); // Returned IWbemServices.

  //
  // Done with Namespace.
  //

  SysFreeString( bstrNamespace );

  if ( hResult != WBEM_S_NO_ERROR) {
     _tprintf( TEXT("Error %lX: Failed to connect to namespace %s.\n"),
               hResult, chNamespace );
    
    IWbemLocator_Release( pIWbemLocator );
    return NULL;
  }

  //
  // Switch the security level to IMPERSONATE so that provider(s)
  // will grant access to system-level objects, and so that
  // CALL authorization will be used.
  //

  hResult = CoSetProxyBlanket(
                   (IUnknown *)pIWbemServices, // proxy
                   RPC_C_AUTHN_WINNT,        // authentication service
                   RPC_C_AUTHZ_NONE,         // authorization service
                   NULL,                     // server principle name
                   RPC_C_AUTHN_LEVEL_CALL,   // authentication level
                   RPC_C_IMP_LEVEL_IMPERSONATE, // impersonation level
                   NULL,                     // identity of the client
                   0 );                      // capability flags

  if ( hResult != S_OK ) {
     _tprintf( TEXT("Error %lX: Failed to impersonate.\n"), hResult );

     IWbemLocator_Release( pIWbemLocator );
     IWbemServices_Release( pIWbemServices );
     return NULL;
  }
 
  return pIWbemServices;
}

// The funciton displays value and type a property of a given instance.

VOID DisplayPropertyValue (IWbemServices *pIWbemServices, LPTSTR lpClassName,
                           LPTSTR lpInstanceName, LPTSTR lpProperty)
{
  IWbemClassObject     *pInst;
  VARIANT              varPropVal;
  LPTSTR               lpPropertyType;
  BOOL                 bRet;

  pInst = GetInstanceReference( pIWbemServices, lpClassName, lpInstanceName );

  if ( !pInst ) {
     _tprintf( TEXT("Could not find the instance.\n") );
     return;
  }

  bRet = GetPropertyValue( pInst, lpProperty, &varPropVal, &lpPropertyType );
  if ( bRet == FALSE ) {
     IWbemClassObject_Release( pInst );
     return;
  }

  _tprintf( TEXT("Class: %s\n")
            TEXT("Instance: %s\n")
            TEXT("Property: %s\n"),
            lpClassName, lpInstanceName, lpProperty );

  if ( V_VT(&varPropVal) & VT_ARRAY ) {
     if ( lpProperty )
        _tprintf( TEXT("   Property Type: Array of %s\n"), lpPropertyType );

     _tprintf( TEXT("   Property Value:\n") );
     DisplayArray( &varPropVal );
  }
  else {
     if ( lpPropertyType )
        _tprintf( TEXT("   Property Type: %s\n"), lpPropertyType );

     _tprintf( TEXT("   Property Value: ") );
     DisplayScalar( &varPropVal );
  }

  VariantClear( &varPropVal );

  if ( lpPropertyType )
   SysFreeString( (BSTR)lpPropertyType );

  IWbemClassObject_Release( pInst );
  return;
}


//
// The function returns an interface pointer to the instance given its
// list-index.
//

IWbemClassObject *GetInstanceReference (IWbemServices *pIWbemServices,
                                        LPTSTR lpClassName,
                                        LPTSTR lpInstanceName)
{
  IWbemClassObject     *pInst;
  IEnumWbemClassObject *pEnumInst;
  BSTR                 bstrClassName;
  BOOL                 bFound;
  ULONG                ulCount;
  HRESULT              hResult;


  bstrClassName = AnsiToBstr( lpClassName, -1 );

  if ( bstrClassName == NULL ) {
     _tprintf( TEXT("Out of memory.\n") );

     return NULL;
  }

  //
  // pInst pointer must be NULL initially,
  //

  pInst = NULL;

  // 
  // Get Instance Enumerator Interface.
  //

  pEnumInst = NULL;

  hResult = IWbemServices_CreateInstanceEnum(
               pIWbemServices,  
               bstrClassName,          // Name of the root class.
               WBEM_FLAG_SHALLOW |     // Enumerate at current root only.
               WBEM_FLAG_FORWARD_ONLY, // Forward-only enumeration.
               NULL,                   // Context.
               &pEnumInst );          // pointer to class enumerator

  if ( hResult != WBEM_S_NO_ERROR ) {
     _tprintf( TEXT("Error %lX: Failed to get a reference")
               TEXT(" to instance enumerator.\n"), hResult );
  }
  else {

     //
     // Get pointer to the instance.
     //

     hResult = WBEM_S_NO_ERROR;
     bFound = FALSE;
     while ( (hResult == WBEM_S_NO_ERROR) && (bFound == FALSE) ) {

        hResult = IEnumWbemClassObject_Next(
                          pEnumInst,
                          2000,      // two seconds timeout
                          1,         // return just one instance.
                          &pInst,    // pointer to instance.
                          &ulCount); // Number of instances returned.

        if ( ulCount > 0 ) {

           bFound = IsInstance( pInst, lpInstanceName );

           if ( bFound == FALSE )
              IWbemClassObject_Release( pInst );
        }
     }

     if ( bFound == FALSE )
        pInst = NULL;

     //
     // Done with the instance enumerator.
     //

     IEnumWbemClassObject_Release( pEnumInst );
  }

  SysFreeString( bstrClassName );

  return pInst;
}

BOOL IsInstance (IWbemClassObject *pInst, LPTSTR lpInstanceName)
{
  VARIANT              varPropVal;
  LPTSTR               lpPropVal;
  BOOL                 bRet;

  bRet = GetPropertyValue( pInst, TEXT("__RELPATH"), &varPropVal, NULL );

  if ( bRet == TRUE ) {
     lpPropVal = BstrToAnsi( V_BSTR(&varPropVal), -1 );

     if ( lpPropVal ) {
        bRet = _tcsicmp( lpInstanceName, lpPropVal ) == 0;
        SysFreeString( (BSTR)lpPropVal );
     }
     else {
        _tprintf( TEXT("Out of memory.\n") );
        bRet = FALSE;
     }
  }

  return bRet;
}

//
// The function returns property value and its type of a given class/instance.
//

BOOL GetPropertyValue (IWbemClassObject *pRef, LPTSTR lpPropertyName, 
                       VARIANT *pvaPropertyValue, LPTSTR *lppPropertyType)
{
  IWbemQualifierSet *pQual;
  BSTR              bstrQualName;
  BSTR              bstrPropertyName;
  VARIANT           vaQual;
  HRESULT           hResult;
  BOOL              bRet;


  bRet = FALSE;
  VariantInit( pvaPropertyValue );

  bstrPropertyName = AnsiToBstr( lpPropertyName, -1 );

  if ( !bstrPropertyName ) {
     _tprintf( TEXT("Error out of memory.\n") );
     return FALSE;
  }


  if ( lppPropertyType ) {

     //
     // Get the textual name of the property type.
     //

     hResult = IWbemClassObject_GetPropertyQualifierSet(
                                pRef,
                                bstrPropertyName,
                                &pQual );

     if ( hResult != WBEM_S_NO_ERROR ) {
        _tprintf( TEXT("Error %lX: Failed to get interface pointer")
                  TEXT("to qualifier of %s.\n"),
                  hResult, lpPropertyName );
     }
     else {

        //
        // Get the textual name of the property type.
        //

        bstrQualName = AnsiToBstr( TEXT("CIMTYPE"), -1 );

        if ( !bstrQualName ) {
           _tprintf( TEXT("Out of memory.\n") );
        }
        else {
           hResult = IWbemQualifierSet_Get(
                             pQual,
                             bstrQualName,
                             0,
                             &vaQual,
                             NULL );

           if ( hResult == WBEM_S_NO_ERROR ) {
              *lppPropertyType = BstrToAnsi( V_BSTR(&vaQual), -1 );
           }
           else {
              _tprintf( TEXT("Error %lX: Failed to read property qualifier.\n"),
                        hResult );
           }

           SysFreeString( bstrQualName );
        }

        IWbemClassObject_Release( pQual );
     }
  }

  //
  // Get the property value.
  //

  hResult = IWbemClassObject_Get(
                             pRef,
                             bstrPropertyName,
                             0,
                             pvaPropertyValue,
                             NULL,
                             NULL );

  SysFreeString( bstrPropertyName );

  if ( hResult == WBEM_S_NO_ERROR ) {
     bRet = TRUE;
  }
  else {
     if ( lppPropertyType && *lppPropertyType ) {
        SysFreeString( (BSTR)*lppPropertyType );
     }

     _tprintf( TEXT("Error %lX: Failed to read property value of %s.\n"),
               hResult, lpPropertyName );
  }

  return bRet;
}

VOID DisplayArray (VARIANT *pva)
{
  VARIANTARG     vaArg;
  VARIANT        vaTemp;
  VARIANT        vaElement;
  LPTSTR         lpValue;
  PVOID          pv;
  long           lLower;
  long           lUpper;
  long           i;
  long           lElementSz;
  HRESULT        hResult;

  //
  // The property is a safearray type and each element represents a
  // value. Irrespective of the data type of the property, each
  // element(value) is displayed as a string.

  // 
  // Find out the lower and upper bound of the safearray.
  //

  SafeArrayGetLBound(
              V_ARRAY(pva),
              1,
              &lLower );

  SafeArrayGetUBound(
              V_ARRAY(pva),
              1,
              &lUpper );

  //
  // Determine the size of each element.
  //

  lElementSz = SafeArrayGetElemsize( V_ARRAY(pva) );

  //
  // Get a pointer to the safearray of elements.
  //

  SafeArrayAccessData(
              V_ARRAY(pva),
              &pv );

 
  for (i=lLower; i <= lUpper; ++i) {

     VariantInit( &vaTemp );
     VariantInit( &vaElement );
   
     //
     // Store pointer to the element in a temporary variant.
     //

     V_BYREF(&vaTemp) = pv;

     // The VT flag of the variant containing the safearray
     // has two bits set, one indicating the data type of the
     // element and the other is VT_ARRAY. In the temporary variant,
     // we only set the data type flag since this is pointing to one
     // element only. VT_BYREF is also set since the temporary
     // variant contains a pointer to the element.

     V_VT(&vaTemp) = (V_VT(pva) & ~VT_ARRAY) | VT_BYREF;

     //
     // Make a copy of the temporary variant since the temporary
     // variant is actually pointing to the safearray element.
     //

     hResult = VariantCopy( &vaElement, &vaTemp );
     if(hResult != S_OK) {
       _tprintf( TEXT("VariantCopy failed in DisplayArray \n"));
       break;
     }
    
     //
     // Convert the element into a string for displaying.
     //

     VariantInit( &vaArg );

     hResult = VariantChangeType(
                     &vaArg,
                     &vaElement,
                     0,
                     VT_BSTR );

     //
     // The property value may not have been successfully
     // converted into string.
     //

     if ( hResult == S_OK ) {
        lpValue = BstrToAnsi( V_BSTR(&vaArg), -1 );
     }
     else {
        lpValue = NULL;
     }

     if ( lpValue ) {
        _tprintf( TEXT("      (%d) %s\n"), i-lLower, lpValue );
     }
     else {
        _tprintf( TEXT("      (%d): <non-printable>\n"), i-lLower );
     }

     if ( lpValue ) {
        SysFreeString( (BSTR)lpValue );
     }

     VariantClear( &vaArg );
     VariantClear( &vaElement );
 
     //
     // Get the next element(value).
     //

     (LONG_PTR)pv += lElementSz;
  }

  SafeArrayUnaccessData( V_ARRAY(pva) );

  return;
}

VOID DisplayScalar (VARIANT *pvaPropValue)
{
  VARIANTARG    vaTemp;
  LPTSTR        lpValue;
  HRESULT       hResult;


  if ( (V_VT(pvaPropValue) == VT_NULL) ||
       (V_VT(pvaPropValue) == VT_EMPTY) ) {
     _tprintf( TEXT("<empty>\n") );
  }
  else {
     VariantInit( &vaTemp );

     hResult = VariantChangeType(
                        &vaTemp,
                        pvaPropValue,
                        0,
                        VT_BSTR );
     //
     // The property value may not have been successfully
     // converted into string.
     //

     if ( hResult == S_OK ) {

        lpValue = BstrToAnsi( V_BSTR(&vaTemp), -1 );

        _tprintf( TEXT("%s\n"), lpValue );

        VariantClear( &vaTemp );
        SysFreeString( (BSTR)lpValue );
     }
     else
        _tprintf( TEXT("<non-printable>\n") );
  }

  return;
}


//
// The function converts an ANSI string into BSTR and returns it in an
// allocated memory. The memory must be free by the caller using free()
// function. If nLenSrc is -1, the string is null terminated.
//

BSTR AnsiToBstr (LPTSTR lpSrc, int nLenSrc)
{
  BSTR lpDest;

  //
  // In case of ANSI version, we need to change the ANSI string to UNICODE since
  // BSTRs are essentially UNICODE strings.
  //

  #ifndef UNICODE
     int  nLenDest;

     nLenDest = MultiByteToWideChar( CP_ACP, 0, lpSrc, nLenSrc, NULL, 0);

     lpDest = SysAllocStringLen( NULL, nLenDest );

     if ( lpDest ) {
        MultiByteToWideChar( CP_ACP, 0, lpSrc, nLenSrc, lpDest, nLenDest );
     }

  //
  // In case of UNICODE version, we simply allocate memory and copy the string.
  //

  #else
     if ( lpSrc == NULL ) {
        nLenSrc = 0;
     }
     else {
        if ( nLenSrc == -1 ) {
           nLenSrc = _tcslen( lpSrc ) + 1;
        }
     }

     lpDest = SysAllocStringLen( lpSrc, nLenSrc );
  #endif

  return lpDest;
}

//
// The function converts a BSTR string into ANSI and returns it in an allocated
// memory. The memory must be freed by the caller using SysFreeString()
// function. If nLenSrc is -1, the string is null terminated.
//

LPTSTR BstrToAnsi (BSTR lpSrc, int nLenSrc)
{
  LPTSTR lpDest;

  //
  // In case of ANSI version, we need to change BSTRs which are UNICODE strings
  // into ANSI version.
  //

  #ifndef UNICODE
     int   nLenDest;

     nLenDest = WideCharToMultiByte( CP_ACP, 0, lpSrc, nLenSrc, NULL,
                                     0, NULL, NULL );
     lpDest = (LPTSTR)SysAllocStringLen( NULL, (size_t)nLenDest );

     if ( lpDest ) {
        WideCharToMultiByte( CP_ACP, 0, lpSrc, nLenSrc, lpDest,
                             nLenDest, NULL, NULL );
     }
  //
  // In case of UNICODE version, we simply allocate memory and copy the BSTR
  // into allocate memory and return its address.
  //

  #else
     if ( lpSrc ) {
        if ( nLenSrc == -1 ) {
           nLenSrc = _tcslen( lpSrc ) + 1;
        }
     }
     else {
        nLenSrc = 0;
     }

     lpDest = (LPTSTR)SysAllocStringLen( lpSrc, nLenSrc );
  #endif

  return lpDest;
}
