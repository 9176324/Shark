// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A 
// PARTICULAR PURPOSE.
//
// Copyright 1999 Microsoft Corporation.  All Rights Reserved.
//
// PROGRAM: wmicli.c
//
// AUTHOR:  Alok Sinha August 6, 1999
//
// PURPOSE: Demonstrates how to enumerate WBEM classes, query property types,
//          and methods.
//
// ENVIRONMENT: Windows 2000 user mode application.
//

#include "wmicli.h"

//
// IWbemServices Interface.
//

IWbemServices *pIWbemServices = NULL;

//
// Handle to the status windows.
//

HWND          hwndStatus;
HWND          hwndInstStatus;

//
// Handle to this instance of the application.
//

HINSTANCE     hInstance;

//
// Program entry point.
//

int APIENTRY WinMain (HINSTANCE hInst,
                      HINSTANCE hPrevInstance, 
                      LPSTR lpCmdLine,         
                      int nCmdShow )           
{
  HRESULT   hResult;

  hInstance = hInst;

  //
  // Make sure common control DLL is loaded.
  //

  InitCommonControls();

  //
  // Initialize COM library. Must be done before invoking any
  // other COM function.
  //

  hResult = CoInitialize( NULL );

  if ( hResult != S_OK ) {
     MessageBox(
             NULL,
             TEXT("Failed to initialize COM library, program exiting..."),
             TEXT("WMI Class Browser"),
             MB_OK | MB_ICONERROR );
  }
  else {
     DialogBox(
           hInstance,
           MAKEINTRESOURCE(IDD_WBEMCLIENT), 
           NULL,
           DlgProc ); 

     CoUninitialize();
  }

  return 0;
}

//
// Windows procedure for the main dialog box.
//

INT_PTR CALLBACK DlgProc (HWND hwndDlg,
                          UINT uMsg,
                          WPARAM wParam,
                          LPARAM lParam)
{
  LPNM_TREEVIEW lpnmTreeView;
  BSTR          bstrRoot;
  LV_COLUMN     lvColumn;
  HWND          hwndInstList;
  NM_LISTVIEW   *nmlv;
  int           iInstIndex;

  switch (uMsg) {

     case WM_INITDIALOG:

             //
             // Display the default name space.
             //

             SetDlgItemText(
                        hwndDlg,
                        IDE_NAMESPACE,
                        NAMESPACE );

             //
             // Add a status window to the dialog box.
             //

             hwndStatus = CreateStatusWindow(
                                         WS_CHILD | WS_VISIBLE,
                                         NULL,
                                         hwndDlg,
                                         ID_STATUS );

             //
             // Add a column to the list control in which all
             // the instances of the selected class are listed.
             //

             hwndInstList = GetDlgItem(
                                 hwndDlg,
                                 IDL_INSTANCE );

             lvColumn.mask = LVCF_FMT | LVCF_WIDTH;
             lvColumn.fmt = LVCFMT_LEFT;
             lvColumn.cx = 3000;                  // Arbitrarily large value.
             
             ListView_InsertColumn( hwndInstList, 0, &lvColumn );


             //
             // Connect to the default namespace.
             //

             ConnectToNamespace( hwndDlg );

             //
             // If successfully connected to the namespace, then enumerate
             // all the classes below the specified root class.
             //

             if ( pIWbemServices ) {
                bstrRoot = AnsiToBstr( ROOT_CLASS, -1 );

                if ( bstrRoot ) {
                   EnumClasses(
                      GetDlgItem(hwndDlg, IDT_CLASS), // Window handle to tree.
                      TVI_ROOT,            // Add classed from the root.
                      bstrRoot );          // Root class name.

                   SysFreeString( bstrRoot );
                }
                else {
                   SendMessage(
                          hwndStatus,
                          WM_SETTEXT,
                          0,
                          (LPARAM)TEXT("Out of memory.") );
                }
             }

             //
             // At DWLP_USER offset, we store the list-index of the instance
             // currently selected. Initially, no instance is selected so we
             //  initialize it by -1.
             //

             SetWindowLongPtr(
                      hwndDlg,
                      DWLP_USER,
                      (LONG_PTR)-1 );

             return TRUE; // Tell Windows to continue creating the dialog box.

     case WM_COMMAND:
             if ( (LOWORD(wParam) == IDB_REFRESH) &&
                (HIWORD(wParam) == BN_CLICKED) ) {

                     
                //
                // Refresh the class tree begining at the root.
                //
                // Clear the tree first.
                //

                TreeView_DeleteItem (
                            GetDlgItem(hwndDlg, IDT_CLASS),
                            TVI_ROOT );

                //
                // Connect to the specified namespace.
                //

                ConnectToNamespace( hwndDlg );

                //
                // If successfully connected to the namespace, then
                // enumerate the classes again and build the tree.
                //

                if ( pIWbemServices ) {
                   bstrRoot = AnsiToBstr( ROOT_CLASS, -1 );

                   if ( bstrRoot ) {
                      EnumClasses(
                              GetDlgItem(hwndDlg, IDT_CLASS),
                              TVI_ROOT,
                              bstrRoot );

                      SysFreeString( bstrRoot );
                   }
                   else {
                      SendMessage(
                                 hwndStatus,
                                 WM_SETTEXT,
                                 0,
                                 (LPARAM)TEXT("Out of memory.") );

                   }
                }
             }
             else {
                if ( (LOWORD(wParam) == IDB_INST_DETAIL) &&
                     (HIWORD(wParam) == BN_CLICKED) ) {

                   iInstIndex = (int)GetWindowLongPtr(
                                              hwndDlg,
                                              DWLP_USER );

                   if ( iInstIndex != -1 ) {

                      //
                      // Show properties and methods of the selected instance.
                      //
                      // The last parameter is the list-index of the instance
                      // that has been selected by the user.
                      //

                      DialogBoxParam(
                            hInstance,
                            MAKEINTRESOURCE(IDD_INSTANCE), 
                            hwndDlg,
                            DlgInstProc,
                            iInstIndex );
                   }
                }
                else {
                   if ( (LOWORD(wParam) == IDB_CLASS_DETAIL) &&
                        (HIWORD(wParam) == BN_CLICKED) ) {

                      //
                      // Show properties and methods of the selected class.
                      //

                      DialogBox(
                            hInstance,
                            MAKEINTRESOURCE(IDD_INSTANCE), 
                            hwndDlg,
                            DlgClassProc );
                   }
                }
             }

             break;

     case WM_NOTIFY:

             if ( wParam == IDT_CLASS ) {

                //
                // User has clicked on a class name, list its instances.
                //     

                lpnmTreeView = (LPNM_TREEVIEW)lParam;

                if ( (lpnmTreeView->hdr.code == TVN_SELCHANGED) &&
                     (lpnmTreeView->action != TVC_UNKNOWN) ) {

                     EnumClassInst(
                              hwndDlg,
                              lpnmTreeView );
                }
             }
             else {
                if ( wParam == IDL_INSTANCE ) {
               
                   nmlv = (NM_LISTVIEW *)lParam;

                   switch( nmlv->hdr.code ) {

                      case LVN_ITEMCHANGED:
                               if ( (nmlv->uChanged & LVIF_STATE) &&
                                    (nmlv->uNewState & LVIS_SELECTED) ) {
                                  //
                                  // User has clicked on an instance name. We
                                  // store the list-index of the instance name.
                                  // Later, we will use this list-index to
                                  // determine the instance whose properties
                                  // and methods user wants to see.
                                  //

                                  SetWindowLongPtr(
                                         hwndDlg,
                                         DWLP_USER,
                                         (LONG_PTR)nmlv->iItem );
                               }
                               break;

                      case LVN_DELETEALLITEMS:

                               //
                               // Set the list-index to -1 to indicate that
                               // no instance is currently seleted.
                               //

                               SetWindowLongPtr(
                                      hwndDlg,
                                      DWLP_USER,
                                      (LONG_PTR)-1 );
                              
                   }
                }
             }

             break;

     case WM_SYSCOMMAND:

             //
             // Before exiting...
             //    .Make sure to disconnect from the namespace if connected.
             //

             if ( (0xFFF0 & wParam) == SC_CLOSE ) {

                if ( pIWbemServices ) {

                   IWbemServices_Release( pIWbemServices );
                }

                EndDialog( hwndDlg, 0 );
             }
  }

  return FALSE;
}

//
// Windows procedure to the dialog box listing properties and methods
// of the selected instance.
//

INT_PTR CALLBACK DlgInstProc (HWND hwndDlg,
                              UINT uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
  LV_COLUMN         lvColumn;
  HWND              hwndList;
  NM_LISTVIEW       *nmlv;
  LPTSTR            lpInstName;
  IWbemClassObject  *pRef;

  switch (uMsg) {

     case WM_INITDIALOG:

             //
             // Display the name of the selected instance. 
             //

             lpInstName = GetSelectedItemName(
                                   GetParent(hwndDlg),
                                   IDL_INSTANCE,
                                   (int)lParam );
             if ( lpInstName ) {
                SetDlgItemText(
                             hwndDlg,
                             IDE_INST_NAME,
                             lpInstName );

                free( lpInstName );

                //
                // Add a status window.
                //

                hwndInstStatus = CreateStatusWindow(
                                            WS_CHILD | WS_VISIBLE,
                                            NULL,
                                            hwndDlg,
                                            ID_STATUS );

                lvColumn.mask = LVCF_FMT | LVCF_WIDTH;
                lvColumn.fmt = LVCFMT_LEFT;
                lvColumn.cx = 3000;         // Arbitrarily large value.
           
                //
                // Add a column to the listview to list the properties.
                //

                hwndList = GetDlgItem(
                                 hwndDlg,
                                 IDL_PROPERTY );

                ListView_InsertColumn( hwndList, 0, &lvColumn );


                //
                // Add a column to the listview to list the methods.
                //

                hwndList = GetDlgItem(
                                 hwndDlg,
                                 IDL_METHOD );

                ListView_InsertColumn( hwndList, 0, &lvColumn );


                //
                // List all the properties of the selected instance.
                //

                pRef = GetInstReference(
                                  GetParent(hwndDlg),
                                  (long)lParam );

                if ( pRef ) {

                   ListProperties( hwndDlg, pRef );

                   IWbemClassObject_Release( pRef );

                   //
                   // List all the methods of the selected instance.
                   //

                   pRef = GetClassReference( GetParent(hwndDlg) );

                   ListMethods( hwndDlg, pRef );

                   //
                   // At DWLP_USER offset, we store the list-index of the property
                   // or method currently selected. Initially, no property or 
                   // method is selected so we initialize it by -1.
                   //
                
                   SetWindowLongPtr(
                          hwndDlg,
                          DWLP_USER,
                           (LONG_PTR)-1 );

                   IWbemClassObject_Release( pRef );

                   return TRUE; // Let Windows to continue creating dialog box.
                }
                else {
                   return FALSE;
                }
             }
             else {
                return FALSE;
             }

     case WM_COMMAND:
             switch( LOWORD(wParam) ) {  
                        
                case IDB_SHOW_VALUE: // Show value of the selected property.

                         if ( HIWORD(wParam) == BN_CLICKED ) {

                            ShowInstPropertyValue( hwndDlg );
                         }

                         break;

                case IDB_EXECUTE: // Execute the selected method.

                         if ( HIWORD(wParam) == BN_CLICKED ) {

                            ExecuteMethod( hwndDlg );
                         }
             }
                                         
     case WM_NOTIFY:
             if ( wParam == IDL_PROPERTY ) {
            
                nmlv = (NM_LISTVIEW *)lParam;

                if ( nmlv->hdr.code == LVN_ITEMCHANGED ) {
                   if ( (nmlv->uChanged & LVIF_STATE) &&
                        (nmlv->uNewState & LVIS_SELECTED) ) {

                      //
                      // User has clicked on a property. We
                      // store the list-index of the property.
                      // Later, we will use this list-index to
                      // determine the property whose value
                      // user wants to see.
                      //

                      SetWindowLongPtr(
                             hwndDlg,
                             DWLP_USER,
                             (LONG_PTR)nmlv->iItem );

                      EnableWindow(
                            GetDlgItem(hwndDlg, IDB_SHOW_VALUE),
                            TRUE );

                      EnableWindow(
                            GetDlgItem(hwndDlg, IDB_EXECUTE),
                            FALSE );
                   }
                }
             }
             else {
                if ( wParam == IDL_METHOD ) {
            
                   nmlv = (NM_LISTVIEW *)lParam;

                   if ( nmlv->hdr.code == LVN_ITEMCHANGED ) {
                      if ( (nmlv->uChanged & LVIF_STATE) &&
                           (nmlv->uNewState & LVIS_SELECTED) ) {

                         //
                         // User has clicked on a method. We
                         // store the list-index of the method.
                         // Later, we will use this list-index to
                         // determine the method that
                         // user wants to execute.
                         //

                         SetWindowLongPtr(
                                hwndDlg,
                                DWLP_USER,
                                (LONG_PTR)nmlv->iItem );

                         EnableWindow(
                               GetDlgItem(hwndDlg, IDB_SHOW_VALUE),
                               FALSE );

                         EnableWindow(
                               GetDlgItem(hwndDlg, IDB_EXECUTE),
                               TRUE );
                      }
                   }
                }
             }

             break;

     case WM_SYSCOMMAND:

             if ( (0xFFF0 & wParam) == SC_CLOSE ) {

                EndDialog( hwndDlg, 0 );
             }
  }

  return FALSE;
}

//
// Windows procedure to the dialog box listing properties and methods
// of the selected class.
//

INT_PTR CALLBACK DlgClassProc (HWND hwndDlg,
                               UINT uMsg,
                               WPARAM wParam,
                               LPARAM lParam)
{
  IWbemClassObject  *pClass;
  HWND              hwndList;
  LV_COLUMN         lvColumn;
  NM_LISTVIEW       *nmlv;
  LPTSTR            lpClassName;

  switch (uMsg) {

     case WM_INITDIALOG:

             //
             // Display the name of the selected class. 
             //

             lpClassName = GetSelectedClassName( GetParent(hwndDlg) );

             if ( lpClassName ) {
                SetDlgItemText(
                             hwndDlg,
                             IDE_INST_NAME,
                             lpClassName );

                free( lpClassName );

                SetDlgItemText(
                             hwndDlg,
                             IDS_INST_NAME,
                             TEXT("Class") );

                //
                // Add a status window.
                //

                hwndInstStatus = CreateStatusWindow(
                                            WS_CHILD | WS_VISIBLE,
                                            NULL,
                                            hwndDlg,
                                            ID_STATUS );

                lvColumn.mask = LVCF_FMT | LVCF_WIDTH;
                lvColumn.fmt = LVCFMT_LEFT;
                lvColumn.cx = 3000;         // Arbitrarily large value.
           
                //
                // Add a column to the listview to list the properties.
                //

                hwndList = GetDlgItem(
                                 hwndDlg,
                                 IDL_PROPERTY );

                ListView_InsertColumn( hwndList, 0, &lvColumn );


                //
                // Add a column to the listview to list the methods.
                //

                hwndList = GetDlgItem(
                                 hwndDlg,
                                 IDL_METHOD );

                ListView_InsertColumn( hwndList, 0, &lvColumn );


                //
                // List all the properties of the selected instance.
                //

                pClass = GetClassReference( GetParent(hwndDlg) );

                if ( pClass ) {
                   ListProperties( hwndDlg, pClass );

                   //
                   // List all the methods of the selected instance.
                   //

                   ListMethods( hwndDlg, pClass );

                   IWbemClassObject_Release( pClass );

                   //
                   // At DWLP_USER offset, we store the list-index of the property
                   // or method currently selected. Initially, no property or 
                   // method is selected so we initialize it by -1.
                   //
                
                   SetWindowLongPtr(
                          hwndDlg,
                          DWLP_USER,
                           (LONG_PTR)-1 );

                   return TRUE; // Let Windows to continue creating dialog box.
                }
                else {
                   return FALSE;
                }
             }
             else {
                return FALSE;
             }

     case WM_COMMAND:
             
             //
             // Show value of the selected property.
             //

             if ( (LOWORD(wParam) == IDB_SHOW_VALUE) && 
                  (HIWORD(wParam) == BN_CLICKED) ) {

                ShowClassPropertyValue( hwndDlg );
             }
             
             break;
                            
     case WM_NOTIFY:
             if ( wParam == IDL_PROPERTY ) {
            
                nmlv = (NM_LISTVIEW *)lParam;

                if ( nmlv->hdr.code == LVN_ITEMCHANGED ) {
                   if ( (nmlv->uChanged & LVIF_STATE) &&
                        (nmlv->uNewState & LVIS_SELECTED) ) {

                      //
                      // User has clicked on a property. We
                      // store the list-index of the property.
                      // Later, we will use this list-index to
                      // determine the property whose value
                      // user wants to see.
                      //

                      SetWindowLongPtr(
                             hwndDlg,
                             DWLP_USER,
                             (LONG_PTR)nmlv->iItem );

                      EnableWindow(
                            GetDlgItem(hwndDlg, IDB_SHOW_VALUE),
                            TRUE );
                   }
                }
             }
             else {
                if ( wParam == IDL_METHOD ) {
            
                   nmlv = (NM_LISTVIEW *)lParam;

                   if ( nmlv->hdr.code == LVN_ITEMCHANGED ) {
                      if ( (nmlv->uChanged & LVIF_STATE) &&
                           (nmlv->uNewState & LVIS_SELECTED) ) {

                         //
                         // User has clicked on a method. Methods in a class
                         // cannot be executed. So, all we do it is disable
                         // the "Show Value" button on the dialog box and
                         // unselect the selected property by storing a -1.
                         //

                         SetWindowLongPtr(
                                hwndDlg,
                                DWLP_USER,
                                (LONG_PTR)-1 );

                         EnableWindow(
                               GetDlgItem(hwndDlg, IDB_SHOW_VALUE),
                               FALSE );
                      }
                   }
                }
             }

             break;

     case WM_SYSCOMMAND:

             if ( (0xFFF0 & wParam) == SC_CLOSE ) {

                EndDialog( hwndDlg, 0 );
             }
  }

  return FALSE;
}


//
// Windows procedure to the dialog box showing value of the property.
//

INT_PTR CALLBACK DlgScalarPropProc (HWND hwndDlg,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam)
{
  LPSCALAR_VALUE lpScalarValue;
  LPTSTR         lpValue;
  int            iLen;

  switch (uMsg) {

     case WM_INITDIALOG:
             
             SetWindowLongPtr( hwndDlg, DWLP_USER, (LONG_PTR)lParam );
             
             lpScalarValue = (LPSCALAR_VALUE)lParam;

             //
             // Show property name as the title of the dialog box.
             //

             SetWindowText( hwndDlg, lpScalarValue->PropertyName );

             //
             // Display the type of the property.
             //

             if ( lpScalarValue->PropertyType ) {
                SetWindowText(
                         GetDlgItem(hwndDlg, IDS_PROP_TYPE),
                         lpScalarValue->PropertyType );
             }
             else {
                SetWindowText(
                         GetDlgItem(hwndDlg, IDS_PROP_TYPE),
                         TEXT("<not available>") );
             }

             //
             // Now display the property value.
             //

             SetWindowText(
                      GetDlgItem(hwndDlg, IDE_PROP_VALUE),
                      lpScalarValue->PropertyValue );


             return TRUE;
           
     case WM_COMMAND:

             if ( (LOWORD(wParam) == IDB_CANCEL) &&
                  (HIWORD(wParam) == BN_CLICKED) ) {

                EndDialog( hwndDlg, 0 );
             }
             else {
                if ( (LOWORD(wParam) == IDB_OK) &&
                     (HIWORD(wParam) == BN_CLICKED) ) {
                
                   iLen = GetWindowTextLength(
                                      GetDlgItem(hwndDlg, IDE_PROP_VALUE) );

                   //
                   // Include space for NULL.
                   //

                   iLen++;

                   lpValue = (LPTSTR)SysAllocStringLen( NULL, iLen );
                   
                   if ( lpValue ) {                             
                      GetDlgItemText(
                                  hwndDlg,
                                  IDE_PROP_VALUE,
                                  lpValue,
                                  iLen );

                      lpScalarValue = (LPSCALAR_VALUE)GetWindowLongPtr(
                                                                  hwndDlg,
                                                                  DWLP_USER );

                      SysFreeString( (BSTR)lpScalarValue->PropertyValue );

                      lpScalarValue->PropertyValue = lpValue;

                      lpScalarValue->PropertyModified = TRUE;
                   }

                   EndDialog( hwndDlg, 0 );
                }
             }

             break;

     case WM_SYSCOMMAND:

             if ( (0xFFF0 & wParam) == SC_CLOSE ) {
          
                EndDialog( hwndDlg, 0 );
             }
  }

  return FALSE;
}

INT_PTR CALLBACK DlgArrayPropProc (HWND hwndDlg,
                                   UINT uMsg,
                                   WPARAM wParam,
                                   LPARAM lParam)
{
  HWND           hwndList;
  LPARRAY_VALUE  lpArrayValue;
  LV_COLUMN      lvColumn;
  NMLVDISPINFO  *pdi;

  switch (uMsg) {

     case WM_INITDIALOG:

             //
             // Add a column to the listview to list the property values.
             //

             lvColumn.mask = LVCF_FMT | LVCF_WIDTH;
             lvColumn.fmt = LVCFMT_LEFT;
             lvColumn.cx = 3000;         // Arbitrarily large value.
        
             hwndList = GetDlgItem(
                              hwndDlg,
                              IDL_PROPERTY_VALUE );

             ListView_InsertColumn( hwndList, 0, &lvColumn );

             //
             // Store pointer to property information so we can get to
             // it later.
             //

             SetWindowLongPtr( hwndDlg, DWLP_USER, (LONG_PTR)lParam );
             
             lpArrayValue = (LPARRAY_VALUE)lParam;

             //
             // Show the property name as title of the dialog box.
             //

             SetWindowText( hwndDlg, lpArrayValue->PropertyName );

             //
             // Display the type of the property.
             //

             if ( lpArrayValue->PropertyType ) {
                SetWindowText(
                         GetDlgItem(hwndDlg, IDS_PROP_TYPE),
                         lpArrayValue->PropertyType );
             }
             else {
                SetWindowText(
                         GetDlgItem(hwndDlg, IDS_PROP_TYPE),
                         TEXT("<not available>") );
             }
             
            //
             // Show array values.
             //

             DisplayArray( hwndList, lpArrayValue->PropertyValue );

             return TRUE;

     case WM_COMMAND:

             if ( (LOWORD(wParam) == IDB_CANCEL) &&
                  (HIWORD(wParam) == BN_CLICKED) ) {

                EndDialog( hwndDlg, 0 );
             }
             else {
                if ( (LOWORD(wParam) == IDB_OK) &&
                     (HIWORD(wParam) == BN_CLICKED) ) {
                
                   lpArrayValue = (LPARRAY_VALUE)GetWindowLongPtr(
                                                            hwndDlg,
                                                            DWLP_USER );
                   UpdateArray(
                           GetDlgItem(hwndDlg, IDL_PROPERTY_VALUE),
                           lpArrayValue->PropertyValue );

                   lpArrayValue->PropertyModified = TRUE;

                   EndDialog( hwndDlg, 0 );
                }
             }

             break;

     case WM_NOTIFY:

             if ( wParam == IDL_PROPERTY_VALUE ) {
                pdi = (NMLVDISPINFO *)lParam;

                if ( pdi->hdr.code == LVN_ENDLABELEDIT ) {
                   if ( pdi->item.pszText ) {
                      return TRUE;
                   }
                }
             }
             break;

     case WM_SYSCOMMAND:

             if ( (0xFFF0 & wParam) == SC_CLOSE ) {
          
                EndDialog( hwndDlg, 0 );
             }
  }

  return FALSE;
}


//
// The function connects to the namespace specified by the user.
//

VOID ConnectToNamespace (HWND hwndDlg)
{
  IWbemLocator *pIWbemLocator = NULL;
  BSTR         bstrNamespace;
  TCHAR        chNamespace[81];
  ULONG        ulLength;
  HRESULT      hResult;

  SendMessage(
         hwndStatus,
         WM_SETTEXT,
         0,
         (LPARAM)TEXT("Connecting to namespace...") );
  
  //
  // If already connected, release IWbemServices.
  //

  if ( pIWbemServices ) {

     IWbemServices_Release( pIWbemServices );
     pIWbemServices = NULL;
  }

  //
  // Create an instance of WbemLocator interface.
  //

  hResult = CoCreateInstance(
                           &CLSID_WbemLocator,
                           NULL,
                           CLSCTX_INPROC_SERVER,
                           &IID_IWbemLocator,
                           (LPVOID *)&pIWbemLocator );
  if ( hResult == S_OK ) {

     //
     // Get the namespace.
     //

     ulLength = GetDlgItemText( hwndDlg, IDE_NAMESPACE, chNamespace, 80 );

     //
     // Namespaces are passed to COM in BSTRs.
     //

     bstrNamespace = AnsiToBstr(
                           chNamespace,
                           ulLength );

     if ( bstrNamespace != NULL ) {

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

        if ( hResult == WBEM_S_NO_ERROR) {
            
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

           if ( hResult == S_OK ) {

              SendMessage(
                     hwndStatus,
                     WM_SETTEXT,
                     0,
                     (LPARAM)TEXT("Connected successfully.") );
           }
           else {
              SendMessage(
                     hwndStatus,
                     WM_SETTEXT,
                     0,
                     (LPARAM)TEXT("Failed to impersonate.") );
           }
        }
        else {
           SendMessage(
                  hwndStatus,
                  WM_SETTEXT,
                  0,
                  (LPARAM)TEXT("Failed to connect.") );
        }

        //
        // Done with Namespace.
        //

        SysFreeString( bstrNamespace );

     }
     else {
        SendMessage(
               hwndStatus,
               WM_SETTEXT,
               0,
               (LPARAM)TEXT("Out of memory.") );
     }

     //
     // Done with IWbemLocator.
     //

     IWbemLocator_Release( pIWbemLocator );
  }
  else {
     SendMessage(
        hwndStatus,
        WM_SETTEXT,
        0,
        (LPARAM)TEXT("Could not create instance of IWbemLocator interface.") );
  }

  return;
}

//
// This recursive function populates the Tree control in the dialog box with
// the names of the classs derived from the root class. It performs a
// depth-first recursive search.
//


VOID EnumClasses (HWND hwndTree,
                  HTREEITEM hTreeParent,
                  BSTR bstrClassName)
{
  IEnumWbemClassObject *pEnumClass;
  IWbemClassObject     *pClass;
  HRESULT              hResult;
  BSTR                 bstrPropName;
  VARIANT              varPropVal;
  LPTSTR                lpPropVal;
  ULONG                ulFound;
  HTREEITEM            hTreeItem;

  if ( hTreeParent == TVI_ROOT ) {

     SendMessage(
            hwndStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Enumerating classes...") );
  }

  // 
  // Get Class Enumerator Interface.
  //

  hResult = IWbemServices_CreateClassEnum(
               pIWbemServices,  
               bstrClassName,     // name of the root class
               WBEM_FLAG_SHALLOW, // Enumerate classes directly below the root.
               NULL,              // Context.
               &pEnumClass );     // pointer to class enumerator

  if ( hResult == WBEM_S_NO_ERROR ) {

     bstrPropName = SysAllocString( L"__CLASS" );

     if ( bstrPropName ) {

        //
        // Begin enumerating classes.
        //

        ulFound = 0;

        //
        // pClass pointer must be NULL initially,
        //

        pClass = NULL;

        hResult = IEnumWbemClassObject_Next(
                          pEnumClass,
                          2000,      // two seconds timeout
                          1,         // return just one class.
                          &pClass,   // pointer to class.
                          &ulFound); // Number of classes returned.

        while ( (hResult == WBEM_S_NO_ERROR) && (ulFound == 1) ) {

           VariantInit( &varPropVal );

           //
           // Get the class name stored in __CLASS property.
           //

           hResult = IWbemClassObject_Get(
                          pClass,
                          bstrPropName, // property name 
                          0L,           // Reserved, must be zero.
                          &varPropVal,  // property value(class name) returned.
                          NULL,         // CIM type not needed.
                          NULL );       // Flavor not needed.

           if ( hResult == WBEM_S_NO_ERROR ) {

              //
              // Add the class name to the tree of classes.
              //

              lpPropVal = BstrToAnsi(
                               V_BSTR(&varPropVal),
                               -1 );

              if ( lpPropVal != NULL ) {
                 hTreeItem = AddToTree(
                                   hwndTree,
                                   hTreeParent,
                                   lpPropVal );
                 SysFreeString( (BSTR)lpPropVal );
              }
              else {
                 SendMessage(
                     hwndStatus,
                     WM_SETTEXT,
                     0,
                     (LPARAM)TEXT("Out of memory during class enumeration") );
                 hResult = WBEM_E_OUT_OF_MEMORY;
              }
           }
           else {
              SendMessage(
                     hwndStatus,
                     WM_SETTEXT,
                     0,
                     (LPARAM)TEXT("Failed to get __CLASS property.") );
           }

           if(hResult != WBEM_S_NO_ERROR) {
             break;
           }

           //
           // Find classes derived from this class.
           //

           EnumClasses(
                     hwndTree,
                     hTreeItem,
                     V_BSTR(&varPropVal) );

           //
           // Done with this class.
           //

           IWbemClassObject_Release( pClass );

           //
           // IMPORTANT: Not document anywhere but it seems that pClass must
           //            point to the previously found class.
           //

           ulFound = 0;
           hResult = IEnumWbemClassObject_Next(
                             pEnumClass,
                             2000,        // two seconds timeout.
                             1,           // return just one class.
                             &pClass,     // pointer to returned class.
                             &ulFound);   // Number of classes returned.
        }

        //
        // Done with the property name.
        //

        SysFreeString( bstrPropName );
     }
     else {
        SendMessage(
               hwndStatus,
               WM_SETTEXT,
               0,
               (LPARAM)TEXT("Out of memory, class enumeration aborted.") );
     }

     //
     // Done with this enumerator.
     //

     IEnumWbemClassObject_Release( pEnumClass );

     if ( hTreeParent == TVI_ROOT ) {

        SendMessage(
               hwndStatus,
               WM_SETTEXT,
               0,
               (LPARAM)TEXT("Class enumeration done.") );
     }
  }
  else {
     SendMessage(
            hwndStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Failed to create class enumerator interface.") );
  }

  return;
}

//
// The function poplulates the list box in the dialog box with the
// instances of the selected class.
//

VOID EnumClassInst (HWND hwndDlg,
                    LPNM_TREEVIEW lpnmTreeView)
{
  TV_ITEM   tvItem;
  LRESULT   lResult;
  LPTSTR    lpClassName;

  //
  // First, find out the length of the class name string
  // stored in lParam and allocate memory.
  //

  ZeroMemory( &tvItem, sizeof(TV_ITEM) );
  tvItem.mask = TVIF_PARAM;
  tvItem.hItem = lpnmTreeView->itemNew.hItem;

  lResult = TreeView_GetItem(
                     GetDlgItem(hwndDlg, IDT_CLASS),
                     &tvItem );

  
  lpClassName = (LPTSTR)malloc( ((size_t)tvItem.lParam) * sizeof(TCHAR) );

  if ( lpClassName ) {

     //
     // Get the name of the selected class.
     //

     tvItem.mask = TVIF_TEXT;
     tvItem.pszText = lpClassName;
     tvItem.cchTextMax = (int)tvItem.lParam;

     lResult = TreeView_GetItem(
                        GetDlgItem(hwndDlg, IDT_CLASS),
                        &tvItem );


     if ( (BOOL)lResult == TRUE ) {

        ListInstances( hwndDlg, lpClassName );
     }
     else {
        SendMessage(
               hwndStatus,
               WM_SETTEXT,
               0,
               (LPARAM)TEXT("Failed to get the class name.") );
     }

     free( lpClassName );
  }
  else {
     SendMessage(
            hwndStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Out of memory.") );
  }

  return;
}

//
// The function populates the list box in the dialog box with instances of the
// selected class. It is called by EnumClassInst().
//

VOID ListInstances (HWND hwndDlg,
                    LPTSTR lpClassName)
{
  HWND                 hwndInstList;
  IEnumWbemClassObject *pEnumInst;
  IWbemClassObject     *pInst;
  HRESULT              hResult;
  BSTR                 bstrPropName;
  BSTR                 bstrClassName;
  VARIANT              varPropVal;
  LPTSTR               lpPropVal;
  LV_ITEM              lvItem;
  ULONG                ulFound;
  int                  i;


  hwndInstList = GetDlgItem(
                         hwndDlg,
                         IDL_INSTANCE );
  //
  // Clear the list first.
  //

  ListView_DeleteAllItems( hwndInstList );

  bstrClassName = AnsiToBstr( lpClassName, -1 );

  //
  // Get pointer to the class.
  //

  if ( bstrClassName != NULL ) {

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

     if ( hResult == WBEM_S_NO_ERROR ) {

        bstrPropName = SysAllocString( L"__RELPATH" );

        if ( bstrPropName ) {

           //
           // Begin enumerating instances.
           //

           ulFound = 0;

           //
           // pInst pointer must be NULL initially,
           //

           pInst = NULL;

           hResult = IEnumWbemClassObject_Next(
                             pEnumInst,
                             2000,      // two seconds timeout
                             1,         // return just one instance.
                             &pInst,    // pointer to instance.
                             &ulFound); // Number of instances returned.
           i = 0;
           while ( (hResult == WBEM_S_NO_ERROR) && (ulFound == 1) ) {

              VariantInit( &varPropVal );

              //
              // Get the instance name stored in Caption property.
              //

              hResult = IWbemClassObject_Get(
                             pInst,
                             bstrPropName, // property name 
                             0L,           // Reserved, must be zero.
                             &varPropVal,  // property value returned.
                             NULL,         // CIM type not needed.
                             NULL );       // Flavor not needed.

              if ( hResult == WBEM_S_NO_ERROR ) {

                 //
                 // Add the instance name to the list box.
                 //

                 lpPropVal = BstrToAnsi(
                                  V_BSTR(&varPropVal),
                                  -1 );

                 if ( lpPropVal != NULL ) {
                 
                    //
                    // Along with the instance name, we also store
                    // the length of the name in bytes in lParam.
                    //

                    ZeroMemory( &lvItem, sizeof(LV_ITEM) );

                    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
                    lvItem.iItem = i++;
                    lvItem.pszText = lpPropVal;
                    lvItem.cchTextMax = _tcslen( lpPropVal ) + 1;
                    lvItem.lParam = (LPARAM)lvItem.cchTextMax;

                    ListView_InsertItem( hwndInstList, &lvItem );

                    SysFreeString( (BSTR)lpPropVal );
                 }
                 else {
                    SendMessage(
                           hwndStatus,
                           WM_SETTEXT,
                           0,
                           (LPARAM)TEXT("Out of memory.") );
                 }
              }
              else {
                 SendMessage(
                        hwndStatus,
                        WM_SETTEXT,
                        0,
                        (LPARAM)TEXT("Failed to get Caption property") );
              }

              //
              // Done with this instance.
              //

              IWbemClassObject_Release( pInst );

              //
              // IMPORTANT: Not documented anywhere but it seems that pInst 
              //            must point to the previously found class.
              //

              ulFound = 0;
              hResult = IEnumWbemClassObject_Next(
                                pEnumInst,
                                2000,        // two seconds timeout.
                                1,           // return just one class.
                                &pInst,     // pointer to returned class.
                                &ulFound);   // Number of classes returned.
           }

           //
           // Done with the property name.
           //

           SysFreeString( bstrPropName );
        }
        else {
           SendMessage(
                  hwndStatus,
                  WM_SETTEXT,
                  0,
                  (LPARAM)TEXT("Out of memory, class enumeration aborted.") );
        }

        //
        // Done with this enumerator.
        //

        IEnumWbemClassObject_Release( pEnumInst );

     }
     else {
        SendMessage(
             hwndStatus,
             WM_SETTEXT,
             0,
             (LPARAM)TEXT("Failed to create instance enumerator interface.") );
     }

     SysFreeString( bstrClassName );
  }
  else {
     SendMessage(
            hwndStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Out of memory during instance enumeration.") );
  }

  return;
}

//
// The function populates the list box in the dialog box with properties of the
// selected class/instance.
//

VOID ListProperties (HWND hwndDlg,
                     IWbemClassObject *pRef)
{
  HWND                 hwndPropList;
  SAFEARRAY            *psaPropNames;
  LV_ITEM              lvItem;
  BSTR                 bstrPropName;
  LPTSTR               lpPropName;
  long                 lLower;
  long                 lUpper;
  int                  i;
  int                  j;
  HRESULT              hResult;



  hwndPropList = GetDlgItem(
                       hwndDlg,
                       IDL_PROPERTY );

  //
  // Clear the list first.
  //

  ListView_DeleteAllItems( hwndPropList );

     
  //
  // psaPropNames must be null prior to making the call.
  //

  psaPropNames = NULL;

  //
  // Get all the properties.
  //

  hResult = IWbemClassObject_GetNames(
                       pRef,   
                       NULL,              // No qualifier names.
                       WBEM_FLAG_ALWAYS | // All non-system properties
                       WBEM_FLAG_NONSYSTEM_ONLY, 
                       NULL,             // No qualifier values.
                       &psaPropNames);   // Returned property names

  if ( hResult == WBEM_S_NO_ERROR ) {

     //
     // Get the number of properties returned.
     //

     SafeArrayGetLBound( psaPropNames, 1, &lLower );
     SafeArrayGetUBound( psaPropNames, 1, &lUpper );

     //
     // For each property...
     //

     j = 0;
     for (i = lLower; i <= lUpper; i++) {

         //
         // Add the property name into the list box.
         //

         //
         // bstrPropName must be null before making the call.
         //

         bstrPropName = NULL;

         hResult = SafeArrayGetElement( psaPropNames, &i, &bstrPropName);

         if ( SUCCEEDED(hResult) ) {

            lpPropName = BstrToAnsi( bstrPropName, -1 );

            if ( lpPropName ) {

               //
               // Along with the name, we also store the length of name
               // in bytes in lParam member.
               //

               ZeroMemory( &lvItem, sizeof(LV_ITEM) );

               lvItem.mask = LVIF_TEXT | LVIF_PARAM;
               lvItem.iItem = j++;
               lvItem.pszText = lpPropName;
               lvItem.cchTextMax = _tcslen( lpPropName ) + 1;
               lvItem.lParam = (LPARAM)lvItem.cchTextMax;

               ListView_InsertItem( hwndPropList, &lvItem );

               SysFreeString( (BSTR)lpPropName );
            }
            else {
               SendMessage(
                      hwndInstStatus,
                      WM_SETTEXT,
                      0,
                      (LPARAM)TEXT("Out of memory.") );
            }

            //
            // Done with the property name.
            //

            SysFreeString( bstrPropName );
         }
         else {
            SendMessage(
                   hwndInstStatus,
                   WM_SETTEXT,
                   0,
                   (LPARAM)TEXT("Failed to get property names.") );
         }
     }

     //
     // Done with the array of properties.
     //

     SafeArrayDestroy( psaPropNames );
  }
  else {
     SendMessage(
            hwndInstStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Failed to get properties.") );
  }

  return;
}

//
// The function populates the list box in the dialog box with method names of
// the selected class/instance.
//

VOID ListMethods (HWND hwndDlg,
                  IWbemClassObject *pRef)
{
  HWND                 hwndMethodList;
  LV_ITEM              lvItem;
  BSTR                 bstrMethodName;
  LPTSTR               lpMethodName;
  HRESULT              hResult;
  int                  i;

  hwndMethodList = GetDlgItem(
                         hwndDlg,
                         IDL_METHOD );

  //
  // Clear the list first.
  //

  ListView_DeleteAllItems( hwndMethodList );


  //
  // Reset the enumeration to the begining.
  //
          
  hResult = IWbemClassObject_BeginMethodEnumeration(
                               pRef,
                               0 );

  if ( hResult == WBEM_S_NO_ERROR ) {

     //
     // Get the name of the first method.
     //

     //
     // bstrMethodName must be NULL prior to calling.
     //

     bstrMethodName = NULL;

     hResult = IWbemClassObject_NextMethod(
                                  pRef,
                                  0,
                                  &bstrMethodName,
                                  NULL,
                                  NULL );

     i = 0;
     while ( (hResult != (HRESULT)WBEM_S_NO_MORE_DATA) &&
             (hResult == WBEM_S_NO_ERROR) ) {

        //
        // Add the method name to the list box.
        //

        lpMethodName = BstrToAnsi( bstrMethodName, -1 );

        if ( lpMethodName ) {

           //
           // Along with the name, we also store the length of name
           // in bytes in lParam member.
           //

           ZeroMemory( &lvItem, sizeof(LV_ITEM) );

           lvItem.mask = LVIF_TEXT | LVIF_PARAM;
           lvItem.iItem = i++;
           lvItem.pszText = lpMethodName;
           lvItem.cchTextMax = _tcslen( lpMethodName ) + 1;
           lvItem.lParam = (LPARAM)lvItem.cchTextMax;

           ListView_InsertItem( hwndMethodList, &lvItem );

           SysFreeString( (BSTR)lpMethodName );
        }
        else {
           SendMessage(
                  hwndInstStatus,
                  WM_SETTEXT,
                  0,
                  (LPARAM)TEXT("Out of memory") );
        }

        //
        // Done with the method name.
        //

        SysFreeString( bstrMethodName );

        //
        // Get the next method name.
        //

        //
        // bstrMethodName must be NULL prior to the call.
        //

        bstrMethodName = NULL;

        hResult = IWbemClassObject_NextMethod(
                                     pRef,
                                     0,
                                     &bstrMethodName,
                                     NULL,
                                     NULL );
     }

     //
     // Enumeration ended either because of error or no more methods in
     // the class. Indicate termination of method enumeration.
     //

     IWbemClassObject_EndMethodEnumeration( pRef );
  }
  else {
     SendMessage(
            hwndInstStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Failed to begin method enumeration.") );
  }

  return;
}

//
// The function returns an interface pointer to the class currently
// selected by the user.
//

IWbemClassObject *GetClassReference (HWND hwndDlg)
{
  IWbemClassObject     *pClass;
  BSTR                 bstrClassName;
  LPTSTR               lpClassName;
  HRESULT              hResult;


  //
  // pClass pointer must be NULL initially,
  //

  pClass = NULL;

  lpClassName = GetSelectedClassName( hwndDlg );

  if ( lpClassName ) {

     bstrClassName = AnsiToBstr( lpClassName, -1 );

     if ( bstrClassName != NULL ) {

        // 
        // Get class reference.
        //

        hResult = IWbemServices_GetObject(
                     pIWbemServices,  
                     bstrClassName,          // Name of the root class.
                     0,                      // Make a synchronous call.
                     NULL,                   // Context.
                     &pClass,                // pointer to returned class.
                     NULL );

       if ( hResult != WBEM_S_NO_ERROR ) {

          SendMessage(
              hwndInstStatus,
              WM_SETTEXT,
              0,
              (LPARAM)TEXT("Failed to get a reference to class interface.") );
       }
     }
     else {
        SendMessage(
               hwndInstStatus,
               WM_SETTEXT,
               0,
               (LPARAM)TEXT("Out of memory.") );
     }

     free( lpClassName );   
  }
  else {
     pClass = NULL;

     SendMessage(
            hwndInstStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Out of memory.") );
  }

  return pClass;
}

//
// The function returns the textual name of the class currently
// selected by the user.
//

LPTSTR GetSelectedClassName (HWND hwndDlg)
{
  LPTSTR    lpClassName;
  TV_ITEM   tvItem;


  // First find out the length of the class name string
  // that is stored in lParam and allocate memory.
  //

  ZeroMemory( &tvItem, sizeof(TV_ITEM) );
  tvItem.mask = TVIF_PARAM;
  tvItem.hItem = TreeView_GetSelection( GetDlgItem(hwndDlg, IDT_CLASS) );

  TreeView_GetItem(
           GetDlgItem(hwndDlg, IDT_CLASS),
           &tvItem );

  lpClassName = (LPTSTR)malloc( ((size_t)tvItem.lParam) * sizeof(TCHAR) );

  if ( lpClassName ) {
     tvItem.mask = TVIF_TEXT;
     tvItem.pszText = lpClassName;
     tvItem.cchTextMax = (int)tvItem.lParam;
     tvItem.hItem = TreeView_GetSelection( GetDlgItem(hwndDlg, IDT_CLASS) );

     TreeView_GetItem(
              GetDlgItem(hwndDlg, IDT_CLASS),
              &tvItem );
  }
  
  return lpClassName;        
}

//
// The function returns an interface pointer to the instance given its
// list-index.
//

IWbemClassObject *GetInstReference (HWND hwndDlg,
                                    int iInstIndex)
{
  IWbemClassObject     *pInst;
  IEnumWbemClassObject *pEnumInst;
  BSTR                 bstrClassName;
  LPTSTR               lpClassName;
  int                  i;
  ULONG                ulFound = 0;
  HRESULT              hResult;
  TV_ITEM              tvItem;


  //
  // pInst pointer must be NULL initially,
  //

  pInst = NULL;

  //
  // First, find out the length of the class name string
  // stored in lParam and allocate memory.
  //

  tvItem.mask = TVIF_PARAM;
  tvItem.hItem = TreeView_GetSelection( GetDlgItem(hwndDlg, IDT_CLASS) );

  TreeView_GetItem(
           GetDlgItem(hwndDlg, IDT_CLASS),
           &tvItem );

  lpClassName = (LPTSTR)malloc( ((size_t)tvItem.lParam) * sizeof(TCHAR) );

  if ( lpClassName ) {

     tvItem.mask = TVIF_TEXT;
     tvItem.pszText = lpClassName;
     tvItem.cchTextMax = (int)tvItem.lParam;
     tvItem.hItem = TreeView_GetSelection( GetDlgItem(hwndDlg, IDT_CLASS) );

     TreeView_GetItem(
              GetDlgItem(hwndDlg, IDT_CLASS),
              &tvItem );
            
     bstrClassName = AnsiToBstr( tvItem.pszText, -1 );

     if ( bstrClassName != NULL ) {

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

        if ( hResult == WBEM_S_NO_ERROR ) {


           //
           // Get pointer to the instance.
           //

           hResult = WBEM_S_NO_ERROR;

           for (i=0; (hResult == WBEM_S_NO_ERROR) && (i <= iInstIndex); ++i) {
              ulFound = 0;

              hResult = IEnumWbemClassObject_Next(
                                pEnumInst,
                                2000,      // two seconds timeout
                                1,         // return just one instance.
                                &pInst,    // pointer to instance.
                                &ulFound); // Number of instances returned.

              if ( (ulFound > 0) && (i < iInstIndex) ) {

                 IWbemClassObject_Release( pInst );
              }
           }

           //
           // Done with the instance enumerator.
           //

           IEnumWbemClassObject_Release( pEnumInst );
        }
        else {
          SendMessage(
           hwndInstStatus,
           WM_SETTEXT,
           0,
           (LPARAM)TEXT("Failed to get a reference to instance enumerator.") );
        }

        SysFreeString( bstrClassName );
     }
     else {
        SendMessage(
               hwndInstStatus,
               WM_SETTEXT,
               0,
               (LPARAM)TEXT("Out of memory.") );
     }

     free( lpClassName );
  }
  else {
     SendMessage(
            hwndInstStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Out of memory.") );
     ulFound = 0;
  }

  if ( ulFound ) {

     return pInst;
  }
  else {
     SendMessage(
            hwndInstStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Class not found.") );

     return NULL;
  }
}

//
// With a given id of the list box, list-index of the entry selected in 
// a given dialog box, the function returns the textual name of the entry.
//
// This is a generic function to find out what instance, property of an
// the instance or method of an instance is currently selected.
//

LPTSTR GetSelectedItemName (HWND hwndParent,
                            int iCtlId,
                            int iItemIndex)
{
  HWND     hwndItemList;
  LV_ITEM  lvItem;
  LPTSTR   lpItemName;

  hwndItemList = GetDlgItem(
                         hwndParent,
                         iCtlId );
  
  //
  // First, find out the length of the item name and allocate memory.
  //

  ZeroMemory( &lvItem, sizeof(LV_ITEM) );
  lvItem.iItem = iItemIndex;
  lvItem.mask = LVIF_PARAM;

  ListView_GetItem( hwndItemList, &lvItem );

  lpItemName = (LPTSTR)malloc( ((size_t)lvItem.lParam) * sizeof(TCHAR) );

  if ( lpItemName ) {

     //
     // Find out the selected item.
     //

     lvItem.mask = LVIF_TEXT;
     lvItem.pszText = lpItemName;
     lvItem.cchTextMax = (int)lvItem.lParam;

     ListView_GetItem( hwndItemList, &lvItem );
  }
  else {
     SendMessage(
            hwndStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Out of memory.") );
  }

  return lpItemName;
}


//
// The function displays the value of a property of a class.
//

VOID ShowClassPropertyValue (HWND hwndDlg)
{
  IWbemClassObject  *pRef;
  PROPERTY_VALUE    propValue;


  // In order to show the value of a property, we need to get a pointer 
  // to the class to which that property belongs.

  pRef = GetClassReference( GetParent(hwndDlg) );

  if ( pRef ) {

     if ( GetPropertyValue(hwndDlg, pRef, &propValue) == TRUE ) {

        if ( V_ISARRAY(&propValue.PropertyValue) ) {
           ShowArrayProperty( hwndDlg, pRef, &propValue );
        }
        else {
           ShowScalarProperty( hwndDlg, pRef, &propValue );
        }

        IWbemClassObject_Release( pRef );

        free( propValue.PropertyName );

        if ( propValue.PropertyType ) {
           SysFreeString( (BSTR)propValue.PropertyType );
        }

        VariantClear( &propValue.PropertyValue );
     }
  }

  return;
}


//
// The function displays the value of currently selected property of instance.
//

VOID ShowInstPropertyValue (HWND hwndDlg)
{
  IWbemClassObject  *pRef;
  PROPERTY_VALUE    propValue;
  int               iInstIndex;


   //
   // Retrieve the list-index of the instance selected in the parent
   // (main) dialog box.
   //

   iInstIndex = (int)GetWindowLongPtr(
                              GetParent(hwndDlg),
                              DWLP_USER );

   //
   // Get an interface pointer to the instance.
   //

   pRef = GetInstReference(
                      GetParent(hwndDlg),
                      iInstIndex );

   if ( pRef ) {

     //
     // Read the property value and its type.
     //

     if ( GetPropertyValue(hwndDlg, pRef, &propValue) == TRUE ) {

        if ( V_ISARRAY(&propValue.PropertyValue) ) {
           ShowArrayProperty( hwndDlg, pRef, &propValue );
        }
        else {
           ShowScalarProperty( hwndDlg, pRef, &propValue );
        }

        IWbemClassObject_Release( pRef );

        free( propValue.PropertyName );

        if ( propValue.PropertyType ) {
           SysFreeString( (BSTR)propValue.PropertyType );
        }

        VariantClear( &propValue.PropertyValue );
     }
  }

  return;
}

VOID ShowScalarProperty (HWND hwndDlg,
                         IWbemClassObject *pRef,
                         LPPROPERTY_VALUE pPropValue)
{
  SCALAR_VALUE  ScalarValue;
  VARIANTARG    vaTemp;
  HRESULT       hResult;


  ZeroMemory( &ScalarValue, sizeof(SCALAR_VALUE) );

  ScalarValue.PropertyName = pPropValue->PropertyName;
  ScalarValue.PropertyType = pPropValue->PropertyType;
  
  if ( (V_VT(&pPropValue->PropertyValue) == VT_NULL) ||
       (V_VT(&pPropValue->PropertyValue) == VT_EMPTY) ) {

     //
     // Check for NULL
     //

     ScalarValue.PropertyValue = (LPTSTR)SysAllocStringLen( NULL, 60 );
     _stprintf( ScalarValue.PropertyValue, TEXT("<empty>") );
  }
  else {
     VariantInit( &vaTemp );

     hResult = VariantChangeType(
                        &vaTemp,
                        &pPropValue->PropertyValue,
                        0,
                        VT_BSTR );
     //
     // The property value may not have been successfully
     // converted into string.
     //

     if ( hResult == S_OK ) {

        ScalarValue.PropertyValue = BstrToAnsi( V_BSTR(&vaTemp), -1 );

        VariantClear( &vaTemp );
     }
     else {

        //
        // Check for NULL
        //

        ScalarValue.PropertyValue = (LPTSTR)SysAllocStringLen( NULL, 60 );
        _stprintf( ScalarValue.PropertyValue, TEXT("<non-printable>") );
     }
  }

  DialogBoxParam(    
        hInstance,
        MAKEINTRESOURCE(IDD_PROP_VALUE_SCALAR), 
        hwndDlg,
        DlgScalarPropProc,
        (LPARAM)&ScalarValue );

  if ( ScalarValue.PropertyModified ) {
     ModifyScalarProperty(
                  pRef,
                  &ScalarValue,
                  V_VT(&pPropValue->PropertyValue) );
  }

  if ( ScalarValue.PropertyValue ) {
     SysFreeString( (BSTR)ScalarValue.PropertyValue );
  }

  return;
}

//
// The function modifies values of the currently selected property.
//

VOID ModifyScalarProperty (IWbemClassObject *pRef,
                           LPSCALAR_VALUE lpScalarValue,
                           USHORT vt)
{
  BSTR     bstrName;
  VARIANT  vabstrValue;
  VARIANT  vaValue;
  HRESULT  hResult;

  bstrName = AnsiToBstr( lpScalarValue->PropertyName, -1 );

  if ( bstrName ) {

     //
     // Convert the value from string to the correct type based on the
     // property type.
     //

     VariantInit( &vabstrValue );
     V_VT(&vabstrValue) = VT_BSTR;
     V_BSTR(&vabstrValue) = AnsiToBstr( lpScalarValue->PropertyValue, -1 );
     
     VariantInit( &vaValue );
     hResult = VariantChangeType( 
                       &vaValue,
                       &vabstrValue,
                       0,
                       vt );

     //
     // If successfully coerced the value from string into property type,
     // then we modify the property.
     //

     if ( SUCCEEDED(hResult) ) {
        hResult = IWbemClassObject_Put(
                             pRef,
                             bstrName,
                             0,
                             &vaValue,
                             0 );
        if ( hResult == WBEM_S_NO_ERROR ) {
           hResult = IWbemServices_PutInstance(
                                   pIWbemServices,
                                   pRef,
                                   WBEM_FLAG_UPDATE_ONLY,
                                   NULL,
                                   NULL );
           if ( hResult == WBEM_S_NO_ERROR ) {
              SendMessage(
                     hwndInstStatus,
                     WM_SETTEXT,
                     0,
                     (LPARAM)TEXT("Property value modified.") );
           }
           else {
              SendMessage(
                     hwndInstStatus,
                     WM_SETTEXT,
                     0,
                     (LPARAM)TEXT("Failed to update the instance.") );
           }
        }
        else {
           SendMessage(
                  hwndInstStatus,
                  WM_SETTEXT,
                  0,
                  (LPARAM)TEXT("Failed to modify property value.") );
        }
     }
     else {
        SendMessage(
               hwndInstStatus,
               WM_SETTEXT,
               0,
               (LPARAM)TEXT("Invalid value.") );
     }
  } 

  return;
}

VOID ShowArrayProperty (HWND hwndDlg,
                        IWbemClassObject *pRef,
                        LPPROPERTY_VALUE pPropValue)
{
  ARRAY_VALUE  ArrayValue;


  ZeroMemory( &ArrayValue, sizeof(SCALAR_VALUE) );

  ArrayValue.PropertyName = pPropValue->PropertyName;
  ArrayValue.PropertyType = pPropValue->PropertyType;
  ArrayValue.PropertyValue = &pPropValue->PropertyValue;

  DialogBoxParam(    
        hInstance,
        MAKEINTRESOURCE(IDD_PROP_VALUE_ARRAY), 
        hwndDlg,
        DlgArrayPropProc,
        (LPARAM)&ArrayValue );

  if ( ArrayValue.PropertyModified ) {
     ModifyArrayProperty( pRef, &ArrayValue );
  }

  return;
}

VOID ModifyArrayProperty (IWbemClassObject *pRef,
                          LPARRAY_VALUE lpArrayValue)
{
}

//
// The function returns property value and its type of a given class/instance.
//

BOOL GetPropertyValue (HWND hwndDlg,
                       IWbemClassObject *pRef,
                       LPPROPERTY_VALUE lpPropValue)
{
  IWbemQualifierSet *pQual;
  BSTR              bstrQualName;
  BSTR              bstrPropertyName;
  VARIANT           vaQual;
  int               iPropertyIndex;
  HRESULT           hResult;
  BOOL              bRet;


  bRet = FALSE;
  ZeroMemory( lpPropValue, sizeof(PROPERTY_VALUE) );

  VariantInit( &lpPropValue->PropertyValue );

  //
  // Find out the list-index of the property whose values we need to find.
  //

  iPropertyIndex = (int)GetWindowLongPtr(
                                 hwndDlg,
                                 DWLP_USER );

  //
  // Find out the name of the property. This is actually __RELPATH
  // since that is what we display while listing the properties.
  //

  lpPropValue->PropertyName = GetSelectedItemName(
                                          hwndDlg,
                                          IDL_PROPERTY,
                                          iPropertyIndex );

  if ( lpPropValue->PropertyName ) {

     bstrPropertyName = AnsiToBstr( lpPropValue->PropertyName, -1 );

     if ( bstrPropertyName ) {

        //
        // Get the textual name of the property type.
        //

        hResult = IWbemClassObject_GetPropertyQualifierSet(
                                   pRef,
                                   bstrPropertyName,
                                   &pQual );

        if ( hResult == WBEM_S_NO_ERROR ) {

           //
           // Get the textual name of the property type.
           //

           bstrQualName = AnsiToBstr( TEXT("CIMTYPE"), -1 );

           if ( bstrQualName ) {

              hResult = IWbemQualifierSet_Get(
                                pQual,
                                bstrQualName,
                                0,
                                &vaQual,
                                NULL );

              if ( hResult == WBEM_S_NO_ERROR ) {
                 lpPropValue->PropertyType = BstrToAnsi( V_BSTR(&vaQual), -1 );
              }

              SysFreeString( bstrQualName );
           }
           else {
              SendMessage(
                     hwndInstStatus,
                     WM_SETTEXT,
                     0,
                     (LPARAM)TEXT("Out of memory.") );
           }

           IWbemClassObject_Release( pQual );
        }
 
        //
        // Get the property value and its cim type.
        //

        hResult = IWbemClassObject_Get(
                                   pRef,
                                   bstrPropertyName,
                                   0,
                                   &lpPropValue->PropertyValue,
                                   NULL,
                                   NULL );

        SysFreeString( bstrPropertyName );

        if ( hResult == WBEM_S_NO_ERROR ) {
           bRet = TRUE;
        }
        else {
           free( lpPropValue->PropertyName );
           lpPropValue->PropertyName = NULL;

           if ( lpPropValue->PropertyType ) {
              SysFreeString( (BSTR)lpPropValue->PropertyType );
              lpPropValue->PropertyType = NULL;
           }

           SendMessage(
                  hwndInstStatus,
                  WM_SETTEXT,
                  0,
                  (LPARAM)TEXT("Failed to read the property value.") );
        }
     }
     else {

        free( lpPropValue->PropertyName );
        lpPropValue->PropertyName = NULL;

        SendMessage(
               hwndInstStatus,
               WM_SETTEXT,
               0,
               (LPARAM)TEXT("Out of memory.") );
     }
  }
  else {
     SendMessage(
            hwndInstStatus,
            WM_SETTEXT,
            0,
            (LPARAM)TEXT("Out of memory.") );
  }

  return bRet;
}


//
// The function, executes the selected method of an
// an instance.
//

VOID ExecuteMethod (HWND hwndDlg)
{
  
  MessageBox(
       hwndDlg,
       TEXT("Feature not implemented in this version of software."),
       TEXT("Execute Method"), MB_OK );
  return;
}

VOID DisplayArray (HWND hwndList,
                   VARIANT *pva)
{
  VARIANTARG     vaArg;
  VARIANT        vaTemp;
  VARIANT        vaElement;
  LPTSTR         lpValue;
  PVOID          pv;
  LV_ITEM        lvItem;
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

     ZeroMemory( &lvItem, sizeof(LV_ITEM) );

     lvItem.mask = LVIF_TEXT | LVIF_PARAM;
     lvItem.iItem = i - lLower;

     if ( lpValue ) {
        lvItem.pszText = lpValue;
        lvItem.lParam = _tcslen( lpValue ) + 1;
     }
     else {
        lvItem.pszText = TEXT("<non-printable>");
     }

     lvItem.cchTextMax = _tcslen( lvItem.pszText ) + 1;
    
     ListView_InsertItem( hwndList, &lvItem );

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

VOID UpdateArray (HWND hwndList,
                  VARIANT *pva)
{
  VARIANT        vaValue;
  VARIANT        vaTemp;
  LPTSTR         lpStr;
  BSTR           bstrStr;
  LV_ITEM        lvItem;
  LV_ITEM        lvItemLen;
  long           lLower;
  long           lUpper;
  long           i;
  HRESULT   hr = S_OK;
  
  SafeArrayGetLBound(
              V_ARRAY(pva),
              1,
              &lLower );

  SafeArrayGetUBound(
              V_ARRAY(pva),
              1,
              &lUpper );


  ZeroMemory( &lvItemLen, sizeof(LV_ITEM) );
  lvItemLen.mask = LVIF_PARAM;

  ZeroMemory( &lvItem, sizeof(LV_ITEM) );
  lvItem.mask = LVIF_TEXT;

  for (i=lLower; i <= lUpper && hr == S_OK; ++i) {

     lvItemLen.iItem = i - lLower;
     ListView_GetItem( hwndList, &lvItemLen );

     if ( lvItemLen.lParam ) {

        //
        // Check for NULL.
        //

        lpStr = (LPTSTR)malloc( sizeof(TCHAR) * lvItemLen.lParam );

        lvItem.iItem = i - lLower;
        lvItem.pszText = lpStr;
        lvItem.cchTextMax = (int)lvItemLen.lParam;

        ListView_GetItem( hwndList, &lvItem );

        //
        // Check for null.
        //

        bstrStr = AnsiToBstr( lpStr, -1 );

        VariantInit( &vaValue );
        VariantInit( &vaTemp );

        V_VT(&vaTemp) = VT_BSTR;
        V_BSTR(&vaTemp) = bstrStr;

        hr = VariantChangeType(
                    &vaValue,
                    &vaTemp,
                    0,
                    (VARTYPE)(V_VT(pva) & ~VT_ARRAY) ); 
        if(hr == S_OK) {
          switch( V_VT(&vaValue) ) {
             case VT_BSTR: 
             case VT_DISPATCH: 
             case VT_UNKNOWN: 
                     hr = SafeArrayPutElement( V_ARRAY(pva), &i, V_BYREF(&vaValue) );
                     break;
  
             default:
                     hr = SafeArrayPutElement( V_ARRAY(pva), &i, &(V_BYREF(&vaValue)) );
          }
        }
        free( lpStr );
        SysFreeString( bstrStr );
        VariantClear( &vaValue );
     }
   }

   return;
}

VOID AddArgColumns (HWND hwndDlg,
                    LPMETHOD_INFO lpMethodInfo)
{
  HWND              hwndList;
  LV_COLUMN         lvColumn;
  LV_ITEM           lvItem;
  BSTR              *bstrInArgsNames;
  BSTR              *bstrInArgsType;
  LPTSTR            lpArgType;
  long              lLower;
  long              lUpper;
  int               i;
  TCHAR             *chColHdr[] = { TEXT("Argument"),
                                    TEXT("Type"),
                                    TEXT("Value") };

  hwndList = GetDlgItem(
                    hwndDlg,
                    IDL_INPUT_ARGS );

  lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
  lvColumn.fmt = LVCFMT_LEFT;
  lvColumn.cx = 100;         // Arbitrarily large value.
  lvColumn.iSubItem = 0;

  for (i=0; i < 3; ++i) {
     lvColumn.pszText = chColHdr[i];
     lvColumn.cchTextMax = _tcslen( chColHdr[i] );
     ListView_InsertColumn( hwndList, i, &lvColumn );
  }

  //
  // Add input arguments to the list, if any.
  //

  if ( lpMethodInfo->psaInArgs ) {

     //
     // First add the argument names.
     //

     SafeArrayAccessData(
                 lpMethodInfo->psaInArgs,
                 (VOID **)&bstrInArgsNames );
     SafeArrayGetLBound(lpMethodInfo->psaInArgs, 1, &lLower );
     SafeArrayGetUBound(lpMethodInfo->psaInArgs, 1, &lUpper );

     lvItem.mask = LVIF_TEXT;
     lvItem.iSubItem = 0;

     for (i=lLower; i <= lUpper; ++i) {
        lvItem.iItem = i - lLower;
        lvItem.pszText = BstrToAnsi( bstrInArgsNames[i], -1 );
        lvItem.cchTextMax = _tcslen( lvColumn.pszText );

        ListView_InsertItem( hwndList, &lvItem );

        SysFreeString( (BSTR)lvItem.pszText );
     }

     SafeArrayUnaccessData( lpMethodInfo->psaInArgs );

     //
     // Add argument types.
     //

     SafeArrayAccessData(
                 lpMethodInfo->psaType,
                 (VOID **)&bstrInArgsType );
     SafeArrayGetLBound(lpMethodInfo->psaType, 1, &lLower );
     SafeArrayGetUBound(lpMethodInfo->psaType, 1, &lUpper );

     for (i=lLower; i <= lUpper; ++i) {
        lpArgType = BstrToAnsi( bstrInArgsType[i], -1 );

        ListView_SetItemText( hwndList, i-lLower, 1, lpArgType );

        SysFreeString( (BSTR)lpArgType );
     }

     SafeArrayUnaccessData( lpMethodInfo->psaType );
  }

  //
  // Add three columns to listview of output Arguments
  // to show argument name, type and value.
  //

  hwndList = GetDlgItem(
                    hwndDlg,
                    IDL_OUTPUT_ARGS );

  for (i=0; i < 3; ++i) {
     lvColumn.pszText = chColHdr[i];
     lvColumn.cchTextMax = _tcslen( chColHdr[i] );
     ListView_InsertColumn( hwndList, i, &lvColumn );
  }

  return;
}

//
// The function add a class name to the tree as a child to the specified
// class. Child of a class is a derived class of the parent class. It returns
// a handle of the class just added to the tree.
//

HTREEITEM AddToTree (HWND hwndTree,
                     HTREEITEM hParent,
                     LPTSTR lpClassName)
{
  TV_INSERTSTRUCT tvInsertStruc;
  
  ZeroMemory(
        &tvInsertStruc,
        sizeof(TV_INSERTSTRUCT) );

  tvInsertStruc.hParent = hParent;
  tvInsertStruc.hInsertAfter = TVI_LAST;
  tvInsertStruc.item.mask = TVIF_TEXT | TVIF_PARAM;
  tvInsertStruc.item.pszText = lpClassName;
  tvInsertStruc.item.cchTextMax = _tcslen( lpClassName ) + 1;
  tvInsertStruc.item.lParam = (LPARAM)tvInsertStruc.item.cchTextMax;

  return TreeView_InsertItem(
                  hwndTree,
                  &tvInsertStruc );

}


//
// The function converts an ANSI string into BSTR and returns it in an
// allocated memory. The memory must be free by the caller using free()
// function. If nLenSrc is -1, the string is null terminated.
//

BSTR AnsiToBstr (LPTSTR lpSrc,
                 int nLenSrc)
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
           nLenSrc = _tcslen( lpSrc ) + sizeof(TCHAR);
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

LPTSTR BstrToAnsi (BSTR lpSrc,
                   int nLenSrc)
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
           nLenSrc = _tcslen( lpSrc ) + sizeof(TCHAR);
        }
     }
     else {
        nLenSrc = 0;
     }

     lpDest = (LPTSTR)SysAllocStringLen( lpSrc, nLenSrc );
  #endif

  return lpDest;
}
