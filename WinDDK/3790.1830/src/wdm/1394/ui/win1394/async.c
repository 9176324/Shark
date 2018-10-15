/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    async.c

Abstract

    1394 async wrappers

Author:

    Peter Binder (pbinder) 5/13/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
05/13/98 pbinder   birth
08/18/98 pbinder   changed for new dialogs
--*/

#define _ASYNC_C
#include "pch.h"
#undef _ASYNC_C

INT_PTR CALLBACK
AllocateAddressRangeDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PALLOCATE_ADDRESS_RANGE      pAllocateAddressRange;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pAllocateAddressRange = (PALLOCATE_ADDRESS_RANGE)lParam;

            _ultoa(pAllocateAddressRange->nLength, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_ALLOC_LENGTH, tmpBuff);

            _ultoa(pAllocateAddressRange->MaxSegmentSize, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_ALLOC_MAX_SEGMENT_SIZE, tmpBuff);

            _ultoa(pAllocateAddressRange->Required1394Offset.Off_High, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_ALLOC_OFFSET_HIGH, tmpBuff);

            _ultoa(pAllocateAddressRange->Required1394Offset.Off_Low, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_ALLOC_OFFSET_LOW, tmpBuff);

            CheckRadioButton( hDlg,
                              IDC_ASYNC_ALLOC_USE_MDL,
                              IDC_ASYNC_ALLOC_USE_NONE,
                              pAllocateAddressRange->fulAllocateFlags + (IDC_ASYNC_ALLOC_USE_MDL-1)
                              );

            if (pAllocateAddressRange->fulFlags & BIG_ENDIAN_ADDRESS_RANGE)
                CheckDlgButton( hDlg, IDC_ASYNC_ALLOC_USE_BIG_ENDIAN, BST_CHECKED);

            if (pAllocateAddressRange->fulAccessType & ACCESS_FLAGS_TYPE_READ)
                CheckDlgButton( hDlg, IDC_ASYNC_ALLOC_ACCESS_READ, BST_CHECKED);

            if (pAllocateAddressRange->fulAccessType & ACCESS_FLAGS_TYPE_WRITE)
                CheckDlgButton( hDlg, IDC_ASYNC_ALLOC_ACCESS_WRITE, BST_CHECKED);

            if (pAllocateAddressRange->fulAccessType & ACCESS_FLAGS_TYPE_LOCK)
                CheckDlgButton( hDlg, IDC_ASYNC_ALLOC_ACCESS_LOCK, BST_CHECKED);

            if (pAllocateAddressRange->fulAccessType & ACCESS_FLAGS_TYPE_BROADCAST)
                CheckDlgButton( hDlg, IDC_ASYNC_ALLOC_ACCESS_BROADCAST, BST_CHECKED);

            if (pAllocateAddressRange->fulNotificationOptions & NOTIFY_FLAGS_AFTER_READ)
                CheckDlgButton( hDlg, IDC_ASYNC_ALLOC_NOTIFY_READ, BST_CHECKED);

            if (pAllocateAddressRange->fulNotificationOptions & NOTIFY_FLAGS_AFTER_WRITE)
                CheckDlgButton( hDlg, IDC_ASYNC_ALLOC_NOTIFY_WRITE, BST_CHECKED);

            if (pAllocateAddressRange->fulNotificationOptions & NOTIFY_FLAGS_AFTER_LOCK)
                CheckDlgButton( hDlg, IDC_ASYNC_ALLOC_NOTIFY_LOCK, BST_CHECKED);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ASYNC_ALLOC_LENGTH, tmpBuff, STRING_SIZE);
                    pAllocateAddressRange->nLength = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_ALLOC_MAX_SEGMENT_SIZE, tmpBuff, STRING_SIZE);
                    pAllocateAddressRange->MaxSegmentSize = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_ALLOC_OFFSET_HIGH, tmpBuff, STRING_SIZE);
                    pAllocateAddressRange->Required1394Offset.Off_High = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_ALLOC_OFFSET_LOW, tmpBuff, STRING_SIZE);
                    pAllocateAddressRange->Required1394Offset.Off_Low = strtoul(tmpBuff, NULL, 16);

                    // fulAllocateFlags
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_USE_MDL))
                        pAllocateAddressRange->fulAllocateFlags = ASYNC_ALLOC_USE_MDL;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_USE_FIFO))
                        pAllocateAddressRange->fulAllocateFlags = ASYNC_ALLOC_USE_FIFO;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_USE_NONE))
                        pAllocateAddressRange->fulAllocateFlags = ASYNC_ALLOC_USE_NONE;                                                          

                    // fulFlags
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_USE_BIG_ENDIAN))
                        pAllocateAddressRange->fulFlags = BIG_ENDIAN_ADDRESS_RANGE;

                    // fulAccessType
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_ACCESS_READ))
                        pAllocateAddressRange->fulAccessType |= ACCESS_FLAGS_TYPE_READ;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_ACCESS_WRITE))
                        pAllocateAddressRange->fulAccessType |= ACCESS_FLAGS_TYPE_WRITE;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_ACCESS_LOCK))
                        pAllocateAddressRange->fulAccessType |= ACCESS_FLAGS_TYPE_LOCK;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_ACCESS_BROADCAST))
                        pAllocateAddressRange->fulAccessType |= ACCESS_FLAGS_TYPE_BROADCAST;                                                             

                    // fulNotifcationOptions
                    pAllocateAddressRange->fulNotificationOptions = NOTIFY_FLAGS_NEVER;
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_NOTIFY_READ))
                        pAllocateAddressRange->fulNotificationOptions |= NOTIFY_FLAGS_AFTER_READ;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_NOTIFY_WRITE))
                        pAllocateAddressRange->fulNotificationOptions |= NOTIFY_FLAGS_AFTER_WRITE;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_ALLOC_NOTIFY_LOCK))
                        pAllocateAddressRange->fulNotificationOptions |= NOTIFY_FLAGS_AFTER_LOCK;

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // AllocateAddressRangeDlgProc

void
w1394_AllocateAddressRange(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ALLOCATE_ADDRESS_RANGE      allocateAddressRange;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_AllocateAddressRange\r\n"));

    allocateAddressRange.fulAllocateFlags = ASYNC_ALLOC_USE_MDL;
    allocateAddressRange.fulFlags = 0;
    allocateAddressRange.nLength = 512;
    allocateAddressRange.MaxSegmentSize = 0;
    allocateAddressRange.fulAccessType = ACCESS_FLAGS_TYPE_READ | ACCESS_FLAGS_TYPE_WRITE |
         ACCESS_FLAGS_TYPE_LOCK | ACCESS_FLAGS_TYPE_BROADCAST;
    allocateAddressRange.fulNotificationOptions = NOTIFY_FLAGS_NEVER;
    allocateAddressRange.Required1394Offset.Off_High = 0;
    allocateAddressRange.Required1394Offset.Off_Low = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "AllocateAddressRange",
                        hWnd,
                        AllocateAddressRangeDlgProc,
                        (LPARAM)&allocateAddressRange
                        )) {

        dwRet = AllocateAddressRange( hWnd,
                                      szDeviceName,
                                      &allocateAddressRange,
                                      TRUE
                                      );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AllocateAddressRange\r\n"));
    return;
} // w1394_AllocateAddressRange


INT_PTR CALLBACK
FreeAddressRangeDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PHANDLE      phAddressRange;
    static CHAR         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            phAddressRange = (PHANDLE)lParam;

            // put offset low and high and buffer length in edit controls
            //_ultoa(PtrToUlong(*pHandle), tmpBuff, 16);
            sprintf (tmpBuff, "%.1p", *phAddressRange);
            SetDlgItemText(hDlg, IDC_ASYNC_FREE_ADDRESS_HANDLE, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ASYNC_FREE_ADDRESS_HANDLE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", phAddressRange))
                    {
                        // failed to get the handle, just return here
                        EndDialog(hDlg, TRUE);
                        return FALSE;
                    }

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // FreeAddressRangeDlgProc

void
w1394_FreeAddressRange(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    HANDLE      hAddress;
    DWORD       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_FreeAddressRange\r\n"));

    hAddress = NULL;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "FreeAddressRange",
                        hWnd,
                        FreeAddressRangeDlgProc,
                        (LPARAM)&hAddress
                        )) {

        dwRet = FreeAddressRange( hWnd,
                                  szDeviceName,
                                  hAddress
                                  );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_FreeAddressRange\r\n"));
    return;
} // w1394_FreeAddressRange

INT_PTR CALLBACK
AsyncReadDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PASYNC_READ      pAsyncRead;
    static CHAR             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pAsyncRead = (PASYNC_READ)lParam;

            if (pAsyncRead->bRawMode)
                CheckDlgButton( hDlg, IDC_ASYNC_READ_USE_BUS_NODE_NUMBER, BST_CHECKED);

            _ultoa(pAsyncRead->DestinationAddress.IA_Destination_ID.NA_Bus_Number, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_READ_BUS_NUMBER, tmpBuff);

            _ultoa(pAsyncRead->DestinationAddress.IA_Destination_ID.NA_Node_Number, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_READ_NODE_NUMBER, tmpBuff);

            _ultoa(pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_High, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_READ_OFFSET_HIGH, tmpBuff);

            _ultoa(pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_Low, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_READ_OFFSET_LOW, tmpBuff);

            _ultoa(pAsyncRead->nNumberOfBytesToRead, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_READ_BYTES_TO_READ, tmpBuff);

            _ultoa(pAsyncRead->nBlockSize, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_READ_BLOCK_SIZE, tmpBuff);

            if (pAsyncRead->fulFlags & ASYNC_FLAGS_NONINCREMENTING)
                CheckDlgButton( hDlg, IDC_ASYNC_READ_FLAG_NON_INCREMENT, BST_CHECKED);

            if (pAsyncRead->bGetGeneration)
                CheckDlgButton( hDlg, IDC_ASYNC_READ_GET_GENERATION, BST_CHECKED);

            _ultoa(pAsyncRead->ulGeneration, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_READ_GENERATION_COUNT, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_READ_USE_BUS_NODE_NUMBER))
                        pAsyncRead->bRawMode = TRUE;
                    else
                        pAsyncRead->bRawMode = FALSE;

                    GetDlgItemText(hDlg, IDC_ASYNC_READ_BUS_NUMBER, tmpBuff, STRING_SIZE);
                    pAsyncRead->DestinationAddress.IA_Destination_ID.NA_Bus_Number = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_READ_NODE_NUMBER, tmpBuff, STRING_SIZE);
                    pAsyncRead->DestinationAddress.IA_Destination_ID.NA_Node_Number = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_READ_OFFSET_HIGH, tmpBuff, STRING_SIZE);
                    pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_High = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_READ_OFFSET_LOW, tmpBuff, STRING_SIZE);
                    pAsyncRead->DestinationAddress.IA_Destination_Offset.Off_Low = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_READ_BYTES_TO_READ, tmpBuff, STRING_SIZE);
                    pAsyncRead->nNumberOfBytesToRead = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_READ_BLOCK_SIZE, tmpBuff, STRING_SIZE);
                    pAsyncRead->nBlockSize = strtoul(tmpBuff, NULL, 16);

                    pAsyncRead->fulFlags = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_READ_FLAG_NON_INCREMENT))
                        pAsyncRead->fulFlags |= ASYNC_FLAGS_NONINCREMENTING;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_READ_GET_GENERATION))
                        pAsyncRead->bGetGeneration = TRUE;
                    else
                        pAsyncRead->bGetGeneration = FALSE;

                    GetDlgItemText(hDlg, IDC_ASYNC_READ_GENERATION_COUNT, tmpBuff, STRING_SIZE);
                    pAsyncRead->ulGeneration = strtoul(tmpBuff, NULL, 16);

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // AsyncReadDlgProc

void
w1394_AsyncRead(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ASYNC_READ      asyncRead;
    DWORD           dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_AsyncRead\r\n"));

    asyncRead.bRawMode = FALSE;
    asyncRead.bGetGeneration = TRUE;
    asyncRead.DestinationAddress.IA_Destination_ID.NA_Bus_Number = 0x3FF;
    asyncRead.DestinationAddress.IA_Destination_ID.NA_Node_Number = 0;
    asyncRead.DestinationAddress.IA_Destination_Offset.Off_High = 0xFFFF;
    asyncRead.DestinationAddress.IA_Destination_Offset.Off_Low = 0xF0000400;
    asyncRead.nNumberOfBytesToRead = sizeof(ULONG);
    asyncRead.nBlockSize = 0;
    asyncRead.fulFlags = 0;
    asyncRead.ulGeneration = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "AsyncRead",
                        hWnd,
                        AsyncReadDlgProc,
                        (LPARAM)&asyncRead
                        )) {

        dwRet = AsyncRead( hWnd,
                           szDeviceName,
                           &asyncRead,
                           TRUE
                           );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AsyncRead\r\n"));
    return;
} // w1394_AsyncRead

INT_PTR CALLBACK
AsyncWriteDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PASYNC_WRITE     pAsyncWrite;
    static CHAR             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pAsyncWrite = (PASYNC_WRITE)lParam;

            if (pAsyncWrite->bRawMode)
                CheckDlgButton( hDlg, IDC_ASYNC_WRITE_USE_BUS_NODE_NUMBER, BST_CHECKED);

            _ultoa(pAsyncWrite->DestinationAddress.IA_Destination_ID.NA_Bus_Number, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_WRITE_BUS_NUMBER, tmpBuff);

            _ultoa(pAsyncWrite->DestinationAddress.IA_Destination_ID.NA_Node_Number, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_WRITE_NODE_NUMBER, tmpBuff);

            _ultoa(pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_High, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_WRITE_OFFSET_HIGH, tmpBuff);

            _ultoa(pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_Low, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_WRITE_OFFSET_LOW, tmpBuff);

            _ultoa(pAsyncWrite->nNumberOfBytesToWrite, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_WRITE_BYTES_TO_WRITE, tmpBuff);

            _ultoa(pAsyncWrite->nBlockSize, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_WRITE_BLOCK_SIZE, tmpBuff);

            if (pAsyncWrite->fulFlags & ASYNC_FLAGS_NONINCREMENTING)
                CheckDlgButton( hDlg, IDC_ASYNC_WRITE_FLAG_NON_INCRMENT, BST_CHECKED);

            if (pAsyncWrite->fulFlags & ASYNC_FLAGS_NO_STATUS)
                CheckDlgButton( hDlg, IDC_ASYNC_WRITE_FLAG_NO_STATUS, BST_CHECKED);

            if (pAsyncWrite->bGetGeneration)
                CheckDlgButton( hDlg, IDC_ASYNC_WRITE_GET_GENERATION, BST_CHECKED);

            _ultoa(pAsyncWrite->ulGeneration, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_WRITE_GENERATION_COUNT, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                   if (IsDlgButtonChecked(hDlg, IDC_ASYNC_WRITE_USE_BUS_NODE_NUMBER))
                        pAsyncWrite->bRawMode = TRUE;
                    else
                        pAsyncWrite->bRawMode = FALSE;

                    GetDlgItemText(hDlg, IDC_ASYNC_WRITE_BUS_NUMBER, tmpBuff, STRING_SIZE);
                    pAsyncWrite->DestinationAddress.IA_Destination_ID.NA_Bus_Number = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_WRITE_NODE_NUMBER, tmpBuff, STRING_SIZE);
                    pAsyncWrite->DestinationAddress.IA_Destination_ID.NA_Node_Number = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_WRITE_OFFSET_HIGH, tmpBuff, STRING_SIZE);
                    pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_High = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_WRITE_OFFSET_LOW, tmpBuff, STRING_SIZE);
                    pAsyncWrite->DestinationAddress.IA_Destination_Offset.Off_Low = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_WRITE_BYTES_TO_WRITE, tmpBuff, STRING_SIZE);
                    pAsyncWrite->nNumberOfBytesToWrite = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_WRITE_BLOCK_SIZE, tmpBuff, STRING_SIZE);
                    pAsyncWrite->nBlockSize = strtoul(tmpBuff, NULL, 16);

                    pAsyncWrite->fulFlags = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_WRITE_FLAG_NON_INCRMENT))
                        pAsyncWrite->fulFlags |= ASYNC_FLAGS_NONINCREMENTING;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_WRITE_FLAG_NO_STATUS))
                        pAsyncWrite->fulFlags |= ASYNC_FLAGS_NO_STATUS;


                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_WRITE_GET_GENERATION))
                        pAsyncWrite->bGetGeneration = TRUE;
                    else
                        pAsyncWrite->bGetGeneration = FALSE;

                    GetDlgItemText(hDlg, IDC_ASYNC_WRITE_GENERATION_COUNT, tmpBuff, STRING_SIZE);
                    pAsyncWrite->ulGeneration = strtoul(tmpBuff, NULL, 16);

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // AsyncWriteDlgProc

void
w1394_AsyncWrite(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ASYNC_WRITE     asyncWrite;
    DWORD           dwRet;
    ULONG           i;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_AsyncWrite\r\n"));

    asyncWrite.bRawMode = FALSE;
    asyncWrite.bGetGeneration = TRUE;
    asyncWrite.DestinationAddress.IA_Destination_ID.NA_Bus_Number = 0x3FF;
    asyncWrite.DestinationAddress.IA_Destination_ID.NA_Node_Number = 0;
    asyncWrite.DestinationAddress.IA_Destination_Offset.Off_High = 0;
    asyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low = 0;
    asyncWrite.nNumberOfBytesToWrite = sizeof(ULONG);
    asyncWrite.nBlockSize = 0;
    asyncWrite.fulFlags = 0;
    asyncWrite.ulGeneration = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "AsyncWrite",
                        hWnd,
                        AsyncWriteDlgProc,
                        (LPARAM)&asyncWrite
                        )) {

        dwRet = AsyncWrite( hWnd,
                            szDeviceName,
                            &asyncWrite,
                            TRUE
                            );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AsyncWrite\r\n"));
    return;
} // w1394_AsyncWrite

INT_PTR CALLBACK
AsyncLockDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PASYNC_LOCK      pAsyncLock;
    static CHAR             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pAsyncLock = (PASYNC_LOCK)lParam;

            if (pAsyncLock->bRawMode)
                CheckDlgButton( hDlg, IDC_ASYNC_LOCK_USE_BUS_NODE_NUMBER, BST_CHECKED);

            _ultoa(pAsyncLock->DestinationAddress.IA_Destination_ID.NA_Bus_Number, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOCK_BUS_NUMBER, tmpBuff);

            _ultoa(pAsyncLock->DestinationAddress.IA_Destination_ID.NA_Node_Number, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOCK_NODE_NUMBER, tmpBuff);

            _ultoa(pAsyncLock->DestinationAddress.IA_Destination_Offset.Off_High, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOCK_OFFSET_HIGH, tmpBuff);

            _ultoa(pAsyncLock->DestinationAddress.IA_Destination_Offset.Off_Low, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOCK_OFFSET_LOW, tmpBuff);

            CheckRadioButton( hDlg,
                              IDC_ASYNC_LOCK_32BIT,
                              IDC_ASYNC_LOCK_64BIT,
                              IDC_ASYNC_LOCK_32BIT
                              );

            _ultoa(pAsyncLock->Arguments[0], tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOCK_ARGUMENT1, tmpBuff);

            _ultoa(pAsyncLock->Arguments[1], tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOCK_ARGUMENT2, tmpBuff);

            _ultoa(pAsyncLock->DataValues[0], tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOCK_DATA1, tmpBuff);

            _ultoa(pAsyncLock->DataValues[1], tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOCK_DATA2, tmpBuff);

            if (pAsyncLock->bGetGeneration)
                CheckDlgButton( hDlg, IDC_ASYNC_LOCK_GET_GENERATION, BST_CHECKED);

            _ultoa(pAsyncLock->ulGeneration, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOCK_GENERATION_COUNT, tmpBuff);

            CheckRadioButton( hDlg,
                              IDC_ASYNC_LOCK_MASK_SWAP,
                              IDC_ASYNC_LOCK_WRAP_ADD,
                              pAsyncLock->fulTransactionType + (IDC_ASYNC_LOCK_MASK_SWAP-1)
                              );

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

               case IDOK:

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_USE_BUS_NODE_NUMBER))
                        pAsyncLock->bRawMode = TRUE;
                    else
                        pAsyncLock->bRawMode = FALSE;

                    GetDlgItemText(hDlg, IDC_ASYNC_LOCK_BUS_NUMBER, tmpBuff, STRING_SIZE);
                    pAsyncLock->DestinationAddress.IA_Destination_ID.NA_Bus_Number = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_LOCK_NODE_NUMBER, tmpBuff, STRING_SIZE);
                    pAsyncLock->DestinationAddress.IA_Destination_ID.NA_Node_Number = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_LOCK_OFFSET_HIGH, tmpBuff, STRING_SIZE);
                    pAsyncLock->DestinationAddress.IA_Destination_Offset.Off_High = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_LOCK_OFFSET_LOW, tmpBuff, STRING_SIZE);
                    pAsyncLock->DestinationAddress.IA_Destination_Offset.Off_Low = strtoul(tmpBuff, NULL, 16);

                    pAsyncLock->fulTransactionType = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_MASK_SWAP))
                        pAsyncLock->fulTransactionType = LOCK_TRANSACTION_MASK_SWAP;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_COMPARE_SWAP))
                        pAsyncLock->fulTransactionType = LOCK_TRANSACTION_COMPARE_SWAP;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_LITTLE_ADD))
                        pAsyncLock->fulTransactionType = LOCK_TRANSACTION_LITTLE_ADD;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_FETCH_ADD))
                        pAsyncLock->fulTransactionType = LOCK_TRANSACTION_FETCH_ADD;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_BOUNDED_ADD))
                        pAsyncLock->fulTransactionType = LOCK_TRANSACTION_BOUNDED_ADD;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_WRAP_ADD))
                        pAsyncLock->fulTransactionType = LOCK_TRANSACTION_WRAP_ADD;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_32BIT)) {

                        if ((pAsyncLock->fulTransactionType == LOCK_TRANSACTION_FETCH_ADD) ||
                            (pAsyncLock->fulTransactionType == LOCK_TRANSACTION_LITTLE_ADD)) {

                            pAsyncLock->nNumberOfArgBytes = 0;
                        }
                        else
                            pAsyncLock->nNumberOfArgBytes = sizeof(ULONG);

                        pAsyncLock->nNumberOfDataBytes = sizeof(ULONG);
                    }

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_64BIT)) {

                        if ((pAsyncLock->fulTransactionType == LOCK_TRANSACTION_FETCH_ADD) ||
                            (pAsyncLock->fulTransactionType == LOCK_TRANSACTION_LITTLE_ADD)) {

                            pAsyncLock->nNumberOfArgBytes = 0;
                        }
                        else
                            pAsyncLock->nNumberOfArgBytes = sizeof(ULONG)*2;

                        pAsyncLock->nNumberOfDataBytes = sizeof(ULONG)*2;
                    }

                    GetDlgItemText(hDlg, IDC_ASYNC_LOCK_ARGUMENT1, tmpBuff, STRING_SIZE);
                    pAsyncLock->Arguments[0] = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_LOCK_ARGUMENT2, tmpBuff, STRING_SIZE);
                    pAsyncLock->Arguments[1] = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_LOCK_DATA1, tmpBuff, STRING_SIZE);
                    pAsyncLock->DataValues[0] = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_LOCK_DATA2, tmpBuff, STRING_SIZE);
                    pAsyncLock->DataValues[1] = strtoul(tmpBuff, NULL, 16);

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOCK_GET_GENERATION))
                        pAsyncLock->bGetGeneration = TRUE;
                    else
                        pAsyncLock->bGetGeneration = FALSE;

                    GetDlgItemText(hDlg, IDC_ASYNC_LOCK_GENERATION_COUNT, tmpBuff, STRING_SIZE);
                    pAsyncLock->ulGeneration = strtoul(tmpBuff, NULL, 16);

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // AsyncLockDlgProc

void
w1394_AsyncLock(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ASYNC_LOCK      asyncLock;
    DWORD           dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_AsyncLock\r\n"));

    asyncLock.bRawMode = FALSE;
    asyncLock.bGetGeneration = TRUE;
    asyncLock.DestinationAddress.IA_Destination_ID.NA_Bus_Number = 0x3FF;
    asyncLock.DestinationAddress.IA_Destination_ID.NA_Node_Number = 0;
    asyncLock.DestinationAddress.IA_Destination_Offset.Off_High = 0;
    asyncLock.DestinationAddress.IA_Destination_Offset.Off_Low = 0;
    asyncLock.nNumberOfArgBytes = sizeof(ULONG);
    asyncLock.nNumberOfDataBytes = sizeof(ULONG);
    asyncLock.fulTransactionType = LOCK_TRANSACTION_MASK_SWAP;
    asyncLock.fulFlags = 0;
    asyncLock.Arguments[0] = 0;
    asyncLock.Arguments[1] = 0;
    asyncLock.DataValues[0] = 0;
    asyncLock.DataValues[1] = 0;
    asyncLock.ulGeneration = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "AsyncLock",
                        hWnd,
                        AsyncLockDlgProc,
                        (LPARAM)&asyncLock
                        )) {

        dwRet = AsyncLock( hWnd,
                           szDeviceName,
                           &asyncLock
                           );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AsyncLock\r\n"));
    return;
} // w1394_AsyncLock

INT_PTR CALLBACK
AsyncStreamDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PASYNC_STREAM    pAsyncStream;
    static CHAR             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pAsyncStream = (PASYNC_STREAM)lParam;

            _ultoa(pAsyncStream->nNumberOfBytesToStream, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_STREAM_BYTES_TO_STREAM, tmpBuff);

            _ultoa(pAsyncStream->ulTag, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_STREAM_TAG, tmpBuff);

            _ultoa(pAsyncStream->nChannel, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_STREAM_CHANNEL, tmpBuff);

            _ultoa(pAsyncStream->ulSynch, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_STREAM_SYNCH, tmpBuff);

            if (pAsyncStream->nSpeed == SPEED_FLAGS_FASTEST) {

                CheckRadioButton( hDlg,
                                  IDC_ASYNC_STREAM_100MBPS,
                                  IDC_ASYNC_STREAM_FASTEST,
                                  IDC_ASYNC_STREAM_FASTEST
                                  );
            }
            else {

                CheckRadioButton( hDlg,
                                  IDC_ASYNC_STREAM_100MBPS,
                                  IDC_ASYNC_STREAM_FASTEST,
                                  pAsyncStream->nSpeed + (IDC_ASYNC_STREAM_100MBPS-1)
                                  );
            }

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

               case IDOK:

                    GetDlgItemText(hDlg, IDC_ASYNC_STREAM_BYTES_TO_STREAM, tmpBuff, STRING_SIZE);
                    pAsyncStream->nNumberOfBytesToStream = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_STREAM_TAG, tmpBuff, STRING_SIZE);
                    pAsyncStream->ulTag = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_STREAM_CHANNEL, tmpBuff, STRING_SIZE);
                    pAsyncStream->nChannel = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_STREAM_SYNCH, tmpBuff, STRING_SIZE);
                    pAsyncStream->ulSynch = strtoul(tmpBuff, NULL, 16);

                    pAsyncStream->nSpeed = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_100MBPS))
                        pAsyncStream->nSpeed = SPEED_FLAGS_100;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_200MBPS))
                        pAsyncStream->nSpeed = SPEED_FLAGS_200;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_400MBPS))
                        pAsyncStream->nSpeed = SPEED_FLAGS_400;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_1600MBPS))
                        pAsyncStream->nSpeed = SPEED_FLAGS_1600;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_FASTEST))
                        pAsyncStream->nSpeed = SPEED_FLAGS_FASTEST;

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // AsyncStreamDlgProc

void
w1394_AsyncStream(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ASYNC_STREAM    asyncStream;
    DWORD           dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_AsyncStream\r\n"));

    asyncStream.nNumberOfBytesToStream = 8;
    asyncStream.fulFlags = 0;
    asyncStream.ulTag = 0;
    asyncStream.nChannel = 0;
    asyncStream.ulSynch = 0;
    asyncStream.nSpeed = SPEED_FLAGS_FASTEST;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "AsyncStream",
                        hWnd,
                        AsyncStreamDlgProc,
                        (LPARAM)&asyncStream
                        )) {

        dwRet = AsyncStream( hWnd,
                             szDeviceName,
                             &asyncStream,
                             TRUE
                             );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AsyncStream\r\n"));
    return;
} // w1394_AsyncStream

INT_PTR CALLBACK
AsyncLoopbackDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PASYNC_LOOPBACK_PARAMS   pLoopbackParams;
    static CHAR                     tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pLoopbackParams = (PASYNC_LOOPBACK_PARAMS)lParam;

            _ultoa(pLoopbackParams->AddressOffset.Off_High, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOOP_OFFSET_HIGH, tmpBuff);

            _ultoa(pLoopbackParams->AddressOffset.Off_Low, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOOP_OFFSET_LOW, tmpBuff);

            _ultoa(pLoopbackParams->nMaxBytes, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOOP_MAX_BYTES, tmpBuff);

            _ultoa(pLoopbackParams->nIterations, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_LOOP_ITERATIONS, tmpBuff);

            if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_READ)
                CheckDlgButton(hDlg, IDC_ASYNC_LOOP_READ, BST_CHECKED);

            if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_WRITE)
                CheckDlgButton(hDlg, IDC_ASYNC_LOOP_WRITE, BST_CHECKED);

            if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_LOCK)
                CheckDlgButton(hDlg, IDC_ASYNC_LOOP_LOCK, BST_CHECKED);

            if (pLoopbackParams->ulLoopFlag & ASYNC_LOOPBACK_RANDOM_LENGTH)
                CheckDlgButton(hDlg, IDC_ASYNC_LOOP_RANDOM_SIZE, BST_CHECKED);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ASYNC_LOOP_OFFSET_HIGH, tmpBuff, STRING_SIZE);
                    pLoopbackParams->AddressOffset.Off_High = (USHORT)strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_LOOP_OFFSET_LOW, tmpBuff, STRING_SIZE);
                    pLoopbackParams->AddressOffset.Off_Low = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_LOOP_MAX_BYTES, tmpBuff, STRING_SIZE);
                    pLoopbackParams->nMaxBytes = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_LOOP_ITERATIONS, tmpBuff, STRING_SIZE);
                    pLoopbackParams->nIterations = strtoul(tmpBuff, NULL, 16);

                    pLoopbackParams->ulLoopFlag = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOOP_READ))
                        pLoopbackParams->ulLoopFlag |= ASYNC_LOOPBACK_READ;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOOP_WRITE))
                        pLoopbackParams->ulLoopFlag |= ASYNC_LOOPBACK_WRITE;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOOP_LOCK))
                        pLoopbackParams->ulLoopFlag |= ASYNC_LOOPBACK_LOCK;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_LOOP_RANDOM_SIZE))
                        pLoopbackParams->ulLoopFlag |= ASYNC_LOOPBACK_RANDOM_LENGTH;

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // AsyncLoopbackDlgProc

void
w1394_AsyncStartLoopback(
    HWND                    hWnd,
    PSTR                    szDeviceName,
    PASYNC_LOOPBACK_PARAMS  asyncLoopbackParams
    )
{
    DWORD       dwRet;
    ULONG       generationCount;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_AsyncStartLoopback\r\n"));

    asyncLoopbackParams->szDeviceName = szDeviceName;
    asyncLoopbackParams->hWnd = hWnd;
    asyncLoopbackParams->bKill = FALSE;

    asyncLoopbackParams->ulLoopFlag = ASYNC_LOOPBACK_READ | ASYNC_LOOPBACK_WRITE | ASYNC_LOOPBACK_RANDOM_LENGTH;
    asyncLoopbackParams->nIterations = 0;

    asyncLoopbackParams->AddressOffset.Off_High = 1;
    asyncLoopbackParams->AddressOffset.Off_Low = 0;

    asyncLoopbackParams->nMaxBytes = 0x200;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "AsyncLoopback",
                        hWnd,
                        AsyncLoopbackDlgProc,
                        (LPARAM)asyncLoopbackParams
                        )) {

        GetLocalTime(&AsyncStartTime);

        AsyncStartLoopback( hWnd,
                            szDeviceName,
                            asyncLoopbackParams
                            );

        asyncLoopbackStarted = TRUE;
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AsyncStartLoopback\r\n"));
    return;
} // w1394_AsyncStartLoopback

void
w1394_AsyncStopLoopback(
    HWND                    hWnd,
    PASYNC_LOOPBACK_PARAMS  asyncLoopbackParams
    )
{
    TRACE(TL_TRACE, (hWnd, "Enter w1394_AsyncStopLoopback\r\n"));

#ifdef LOGGER
    Logger_WriteTestLog( "1394",
                         "Async Loopback",
                         NULL,
                         asyncLoopbackParams->ulPass,
                         asyncLoopbackParams->ulFail,
                         &AsyncStartTime,
                         NULL,
                         NULL
                         );
#endif

    AsyncStopLoopback( asyncLoopbackParams );

    asyncLoopbackStarted = FALSE;

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AsyncStopLoopback\r\n"));
    return;
} // w1394_AsyncStopLoopback

INT_PTR CALLBACK
AsyncStreamLoopbackDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PASYNC_STREAM_LOOPBACK_PARAMS    pLoopbackParams;
    static CHAR                             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pLoopbackParams = (PASYNC_STREAM_LOOPBACK_PARAMS)lParam;

            _ultoa(pLoopbackParams->asyncStream.nChannel, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_CHANNEL, tmpBuff);

            _ultoa(pLoopbackParams->asyncStream.nNumberOfBytesToStream, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_BYTES_TO_STREAM, tmpBuff);

            _ultoa(pLoopbackParams->asyncStream.ulSynch, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_SYNCH, tmpBuff);

            _ultoa(pLoopbackParams->asyncStream.ulTag, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_TAG, tmpBuff);

            _ultoa(pLoopbackParams->nIterations, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_ITERATIONS, tmpBuff);

            if (pLoopbackParams->asyncStream.nSpeed == SPEED_FLAGS_FASTEST) {

                CheckRadioButton( hDlg,
                                  IDC_ASYNC_STREAM_LOOP_100MBPS,
                                  IDC_ASYNC_STREAM_LOOP_FASTEST,
                                  IDC_ASYNC_STREAM_LOOP_FASTEST
                                  );
            }
            else {

                CheckRadioButton( hDlg,
                                  IDC_ASYNC_STREAM_LOOP_100MBPS,
                                  IDC_ASYNC_STREAM_LOOP_FASTEST,
                                  pLoopbackParams->asyncStream.nSpeed + (IDC_ASYNC_STREAM_LOOP_100MBPS-1)
                                  );
            }
            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_CHANNEL, tmpBuff, STRING_SIZE);
                    pLoopbackParams->asyncStream.nChannel = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_BYTES_TO_STREAM, tmpBuff, STRING_SIZE);
                    pLoopbackParams->asyncStream.nNumberOfBytesToStream = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_SYNCH, tmpBuff, STRING_SIZE);
                    pLoopbackParams->asyncStream.ulSynch = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_TAG, tmpBuff, STRING_SIZE);
                    pLoopbackParams->asyncStream.ulTag = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ASYNC_STREAM_LOOP_ITERATIONS, tmpBuff, STRING_SIZE);
                    pLoopbackParams->nIterations = strtoul(tmpBuff, NULL, 16);

                    pLoopbackParams->asyncStream.nSpeed = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_LOOP_100MBPS))
                        pLoopbackParams->asyncStream.nSpeed = SPEED_FLAGS_100;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_LOOP_200MBPS))
                        pLoopbackParams->asyncStream.nSpeed = SPEED_FLAGS_200;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_LOOP_400MBPS))
                        pLoopbackParams->asyncStream.nSpeed = SPEED_FLAGS_400;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_LOOP_1600MBPS))
                        pLoopbackParams->asyncStream.nSpeed = SPEED_FLAGS_1600;

                    if (IsDlgButtonChecked(hDlg, IDC_ASYNC_STREAM_LOOP_FASTEST))
                        pLoopbackParams->asyncStream.nSpeed = SPEED_FLAGS_FASTEST;

                    EndDialog(hDlg, TRUE);
                    return(TRUE); // IDOK

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE); // IDCANCEL

                default:
                    return(TRUE); // default

            } // switch

            break; // WM_COMMAND

        default:
            break; // default

    } // switch

    return(FALSE);
} // AsyncStreamLoopbackDlgProc

void
w1394_AsyncStreamStartLoopback(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PASYNC_STREAM_LOOPBACK_PARAMS   streamLoopbackParams
    )
{
    DWORD       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_AsyncStreamStartLoopback\r\n"));

    streamLoopbackParams->szDeviceName = szDeviceName;
    streamLoopbackParams->hWnd = hWnd;
    streamLoopbackParams->bKill = FALSE;

    streamLoopbackParams->bAutoAlloc = TRUE;
    streamLoopbackParams->bAutoFill = TRUE;
    streamLoopbackParams->nIterations = 0;

    streamLoopbackParams->asyncStream.nNumberOfBytesToStream = 640;
    streamLoopbackParams->asyncStream.fulFlags = 0;
    streamLoopbackParams->asyncStream.ulTag = 0;
    streamLoopbackParams->asyncStream.nChannel = 0;
    streamLoopbackParams->asyncStream.ulSynch = 0;
    streamLoopbackParams->asyncStream.nSpeed = SPEED_FLAGS_200;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "AsyncStreamLoopback",
                        hWnd,
                        AsyncStreamLoopbackDlgProc,
                        (LPARAM)streamLoopbackParams
                        )) {

        GetLocalTime(&StreamStartTime);

        AsyncStreamStartLoopback( hWnd,
                                  szDeviceName,
                                  streamLoopbackParams
                                  );

        streamLoopbackStarted = TRUE;
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AsyncStreamStartLoopback\r\n"));
    return;
} // w1394_AsyncStreamStartLoopback

void
w1394_AsyncStreamStopLoopback(
    HWND                            hWnd,
    PASYNC_STREAM_LOOPBACK_PARAMS   streamLoopbackParams
    )
{
    TRACE(TL_TRACE, (hWnd, "Enter w1394_AsyncStreamStopLoopback\r\n"));

#ifdef LOGGER
    Logger_WriteTestLog( "1394",
                         "Async Stream Loopback",
                         NULL,
                         streamLoopbackParams->ulPass,
                         streamLoopbackParams->ulFail,
                         &StreamStartTime,
                         NULL,
                         NULL
                         );
#endif

    AsyncStreamStopLoopback( streamLoopbackParams );

    streamLoopbackStarted = FALSE;

    TRACE(TL_TRACE, (hWnd, "Exit w1394_AsyncStreamStopLoopback\r\n"));
    return;
} // w1394_AsyncStreamStopLoopback


