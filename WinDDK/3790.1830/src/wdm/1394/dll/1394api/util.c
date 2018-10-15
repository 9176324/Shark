/*++

Copyright (c) 1997-1998 Microsoft Corporation

Module Name: 

    util.c

Abstract

    Misc. functions that are convient to have around.

Author:

    Peter Binder (pbinder) 10/29/97

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
10/29/97 pbinder   birth
04/08/98 pbinder   taken from cidiag
--*/

#define _UTIL_C
#include "pch.h"
#undef _UTIL_C

HANDLE
OpenDevice(
    HANDLE  hWnd,
    PSTR    szDeviceName,
    BOOL    bOverLapped
    )
{
    HANDLE  hDevice;

    if (bOverLapped) {

        hDevice = CreateFile( szDeviceName,
                              GENERIC_WRITE | GENERIC_READ,
                              FILE_SHARE_WRITE | FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              FILE_FLAG_OVERLAPPED,
                              NULL
                              );
    }
    else {
    
        hDevice = CreateFile( szDeviceName,
                              GENERIC_WRITE | GENERIC_READ,
                              FILE_SHARE_WRITE | FILE_SHARE_READ,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL
                              );
    }

	if (hDevice == INVALID_HANDLE_VALUE) {

        TRACE(TL_ERROR, (hWnd, "Failed to open device!\r\n"));
    }

    return(hDevice);
} // OpenDevice

VOID
WriteTextToEditControl(
    HWND    hWndEdit, 
    PCHAR   str
    )
{
    INT     iLength;

    // get the end of buffer for edit control
    iLength = GetWindowTextLength(hWndEdit);

    // set current selection to that offset
    SendMessage(hWndEdit, EM_SETSEL, iLength, iLength);

    // add text
    SendMessage(hWndEdit, EM_REPLACESEL, 0, (LPARAM) str);

} // WriteTextToEditControl

//
// Generic singly linked list routines.
//

//***********************************************************************
//
void 
InsertTailList(
    PLIST_NODE head, 
    PLIST_NODE entry
    )
{
PLIST_NODE pCurrent = head;

    entry->Next = 0;
    while(pCurrent->Next)
        pCurrent = pCurrent->Next;
    pCurrent->Next = entry;

}

//***********************************************************************
//
BOOL 
RemoveEntryList(
    PLIST_NODE head, 
    PLIST_NODE entry
    )
{
PLIST_NODE pCurrent = head;

    while(pCurrent->Next != entry){
        pCurrent = pCurrent->Next;
        if(pCurrent == 0) return FALSE;
    }
    pCurrent->Next = entry->Next;
    return TRUE;
}
    


//***********************************************************************
//
void 
InsertHeadList(
    PLIST_NODE head, 
    PLIST_NODE entry
    )
{
    entry->Next = head->Next;
    head->Next = entry;
}

//****************************************************************************
//
BOOL 
IsNodeOnList(
    PLIST_NODE head, 
    PLIST_NODE entry
    )
{
PLIST_NODE pCurrent = head;

    while(pCurrent->Next != entry){
        pCurrent = pCurrent->Next;
        if(pCurrent == 0) return FALSE;
    }
    return TRUE;
}


