/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    isoch.c

Abstract

    1394 isoch wrappers

Author:

    Peter Binder (pbinder) 5/4/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
05/04/98 pbinder   taken from old win1394
08/18/98 pbinder   changed for new dialogs
--*/

#define _ISOCH_C
#include "pch.h"
#undef _ISOCH_C

INT_PTR CALLBACK
IsochAllocateBandwidthDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_ALLOCATE_BANDWIDTH    pIsochAllocateBandwidth;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochAllocateBandwidth = (PISOCH_ALLOCATE_BANDWIDTH)lParam;

            _ultoa(pIsochAllocateBandwidth->nMaxBytesPerFrameRequested, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ALLOC_BW_BYTES_PER_FRAME, tmpBuff);

            if (pIsochAllocateBandwidth->fulSpeed == SPEED_FLAGS_FASTEST) {

                CheckRadioButton( hDlg,
                                  IDC_ALLOC_BW_100MBPS,
                                  IDC_ALLOC_BW_FASTEST,
                                  IDC_ALLOC_BW_FASTEST
                                  );
            }
            else {

                CheckRadioButton( hDlg,
                                  IDC_ALLOC_BW_100MBPS,
                                  IDC_ALLOC_BW_FASTEST,
                                  pIsochAllocateBandwidth->fulSpeed + (IDC_ALLOC_BW_100MBPS-1)
                                  );
            }

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ALLOC_BW_BYTES_PER_FRAME, tmpBuff, STRING_SIZE);
                    pIsochAllocateBandwidth->nMaxBytesPerFrameRequested = strtoul(tmpBuff, NULL, 16);

                    pIsochAllocateBandwidth->fulSpeed = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ALLOC_BW_100MBPS))
                        pIsochAllocateBandwidth->fulSpeed = SPEED_FLAGS_100;

                    if (IsDlgButtonChecked(hDlg, IDC_ALLOC_BW_200MBPS))
                        pIsochAllocateBandwidth->fulSpeed = SPEED_FLAGS_200;

                    if (IsDlgButtonChecked(hDlg, IDC_ALLOC_BW_400MBPS))
                        pIsochAllocateBandwidth->fulSpeed = SPEED_FLAGS_400;

                    if (IsDlgButtonChecked(hDlg, IDC_ALLOC_BW_1600MBPS))
                        pIsochAllocateBandwidth->fulSpeed = SPEED_FLAGS_1600;

                    if (IsDlgButtonChecked(hDlg, IDC_ALLOC_BW_FASTEST))
                        pIsochAllocateBandwidth->fulSpeed = SPEED_FLAGS_FASTEST;

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
} // IsochAllocateBandwidthDlgProc

void
w1394_IsochAllocateBandwidth(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_ALLOCATE_BANDWIDTH    isochAllocateBandwidth;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394IsochAllocateBandwidth\r\n"));

    isochAllocateBandwidth.nMaxBytesPerFrameRequested = 640;
    isochAllocateBandwidth.fulSpeed = SPEED_FLAGS_200;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochAllocateBandwidth",
                        hWnd,
                        IsochAllocateBandwidthDlgProc,
                        (LPARAM)&isochAllocateBandwidth
                        )) {

        dwRet = IsochAllocateBandwidth( hWnd,
                                        szDeviceName,
                                        &isochAllocateBandwidth
                                        );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394IsochAllocateBandwidth\r\n"));
    return;
} // w1394_IsochAllocateBandwidth

INT_PTR CALLBACK
IsochAllocateChannelDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_ALLOCATE_CHANNEL      pIsochAllocateChannel;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochAllocateChannel = (PISOCH_ALLOCATE_CHANNEL)lParam;

            _ultoa(pIsochAllocateChannel->nRequestedChannel, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ALLOC_CHAN_REQUESTED_CHANNEL, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ALLOC_CHAN_REQUESTED_CHANNEL, tmpBuff, STRING_SIZE);
                    pIsochAllocateChannel->nRequestedChannel = strtoul(tmpBuff, NULL, 16);

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
} // IsochAllocateChannelDlgProc

void
w1394_IsochAllocateChannel(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_ALLOCATE_CHANNEL      isochAllocateChannel;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochAllocateChannel\r\n"));

    isochAllocateChannel.nRequestedChannel = -1;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochAllocateChannel",
                        hWnd,
                        IsochAllocateChannelDlgProc,
                        (LPARAM)&isochAllocateChannel
                        )) {

        dwRet = IsochAllocateChannel( hWnd,
                                      szDeviceName,
                                      &isochAllocateChannel
                                      );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochAllocateChannel\r\n"));
    return;
} // w1394_IsochAllocateChannel

INT_PTR CALLBACK
IsochAllocateResourcesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_ALLOCATE_RESOURCES    pIsochAllocateResources;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochAllocateResources = (PISOCH_ALLOCATE_RESOURCES)lParam;

            _ultoa(pIsochAllocateResources->nChannel, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_CHANNEL, tmpBuff);

            _ultoa(pIsochAllocateResources->nMaxBytesPerFrame, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_BYTES_PER_FRAME, tmpBuff);

            _ultoa(pIsochAllocateResources->nNumberOfBuffers, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_NUM_OF_BUFFERS, tmpBuff);

            _ultoa(pIsochAllocateResources->nMaxBufferSize, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_MAX_BUFFER_SIZE, tmpBuff);

            _ultoa(pIsochAllocateResources->nQuadletsToStrip, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_QUADLETS_TO_STRIP, tmpBuff);

            if (pIsochAllocateResources->fulSpeed == SPEED_FLAGS_FASTEST) {

                CheckRadioButton( hDlg,
                                  IDC_ISOCH_ALLOC_RES_100MBPS,
                                  IDC_ISOCH_ALLOC_RES_FASTEST,
                                  IDC_ISOCH_ALLOC_RES_FASTEST
                                  );
            }
            else {

                CheckRadioButton( hDlg,
                                  IDC_ISOCH_ALLOC_RES_100MBPS,
                                  IDC_ISOCH_ALLOC_RES_FASTEST,
                                  pIsochAllocateResources->fulSpeed + (IDC_ISOCH_ALLOC_RES_100MBPS-1)
                                  );
            }

            if (pIsochAllocateResources->fulFlags & RESOURCE_USED_IN_LISTENING)
                CheckDlgButton( hDlg, IDC_ISOCH_ALLOC_RES_USED_IN_LISTEN, BST_CHECKED);

            if (pIsochAllocateResources->fulFlags & RESOURCE_USED_IN_TALKING)
                CheckDlgButton( hDlg, IDC_ISOCH_ALLOC_RES_USED_IN_TALK, BST_CHECKED);

            if (pIsochAllocateResources->fulFlags & RESOURCE_BUFFERS_CIRCULAR)
                CheckDlgButton( hDlg, IDC_ISOCH_ALLOC_RES_BUFFERS_CIRCULAR, BST_CHECKED);

            if (pIsochAllocateResources->fulFlags & RESOURCE_STRIP_ADDITIONAL_QUADLETS)
                CheckDlgButton( hDlg, IDC_ISOCH_ALLOC_RES_STRIP_QUADLETS, BST_CHECKED);

            if (pIsochAllocateResources->fulFlags & RESOURCE_SYNCH_ON_TIME)
                CheckDlgButton( hDlg, IDC_ISOCH_ALLOC_RES_SYNC_ON_TIME, BST_CHECKED);

            if (pIsochAllocateResources->fulFlags & RESOURCE_USE_PACKET_BASED)
                CheckDlgButton( hDlg, IDC_ISOCH_ALLOC_RES_USE_PACKET_BASED, BST_CHECKED);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_CHANNEL, tmpBuff, STRING_SIZE);
                    pIsochAllocateResources->nChannel = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_BYTES_PER_FRAME, tmpBuff, STRING_SIZE);
                    pIsochAllocateResources->nMaxBytesPerFrame = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_NUM_OF_BUFFERS, tmpBuff, STRING_SIZE);
                    pIsochAllocateResources->nNumberOfBuffers = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_MAX_BUFFER_SIZE, tmpBuff, STRING_SIZE);
                    pIsochAllocateResources->nMaxBufferSize = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ALLOC_RES_QUADLETS_TO_STRIP, tmpBuff, STRING_SIZE);
                    pIsochAllocateResources->nQuadletsToStrip = strtoul(tmpBuff, NULL, 16);

                    pIsochAllocateResources->fulSpeed = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_100MBPS))
                        pIsochAllocateResources->fulSpeed = SPEED_FLAGS_100;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_200MBPS))
                        pIsochAllocateResources->fulSpeed = SPEED_FLAGS_200;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_400MBPS))
                        pIsochAllocateResources->fulSpeed = SPEED_FLAGS_400;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_1600MBPS))
                        pIsochAllocateResources->fulSpeed = SPEED_FLAGS_1600;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_FASTEST))
                        pIsochAllocateResources->fulSpeed = SPEED_FLAGS_FASTEST;

                    pIsochAllocateResources->fulFlags = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_USED_IN_LISTEN))
                        pIsochAllocateResources->fulFlags |= RESOURCE_USED_IN_LISTENING;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_USED_IN_TALK))
                        pIsochAllocateResources->fulFlags |= RESOURCE_USED_IN_TALKING;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_BUFFERS_CIRCULAR))
                        pIsochAllocateResources->fulFlags |= RESOURCE_BUFFERS_CIRCULAR;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_STRIP_QUADLETS))
                        pIsochAllocateResources->fulFlags |= RESOURCE_STRIP_ADDITIONAL_QUADLETS;

//                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_BUFFERS_CIRCULAR))
//                        pIsochAllocateResources->fulFlags |= RESOURCE_TIME_STAMP_ON_COMPLETION;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_SYNC_ON_TIME))
                        pIsochAllocateResources->fulFlags |= RESOURCE_SYNCH_ON_TIME;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ALLOC_RES_USE_PACKET_BASED))
                        pIsochAllocateResources->fulFlags |= RESOURCE_USE_PACKET_BASED;

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
} // IsochAllocateResourcesDlgProc

void
w1394_IsochAllocateResources(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_ALLOCATE_RESOURCES    isochAllocateResources;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochAllocateResources\r\n"));

    isochAllocateResources.fulSpeed = SPEED_FLAGS_200;
    isochAllocateResources.fulFlags = RESOURCE_USED_IN_TALKING;
    isochAllocateResources.nChannel = 0;
    isochAllocateResources.nMaxBytesPerFrame = 640;
    isochAllocateResources.nNumberOfBuffers = 5;
    isochAllocateResources.nMaxBufferSize = 153600;
    isochAllocateResources.nQuadletsToStrip = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochAllocateResources",
                        hWnd,
                        IsochAllocateResourcesDlgProc,
                        (LPARAM)&isochAllocateResources
                        )) {

        dwRet = IsochAllocateResources( hWnd,
                                        szDeviceName,
                                        &isochAllocateResources
                                        );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochAllocateResources\r\n"));
    return;
} // w1394_IsochAllocateResources

INT_PTR CALLBACK
IsochAttachBuffersDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_ATTACH_BUFFERS        pIsochAttachBuffers;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochAttachBuffers = (PISOCH_ATTACH_BUFFERS)lParam;

            sprintf (tmpBuff, "%p", pIsochAttachBuffers->hResource);
            SetDlgItemText(hDlg, IDC_ISOCH_ATTACH_RESOURCE, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->nNumberOfDescriptors, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ATTACH_NUM_DESCRIPTORS, tmpBuff);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_SYNCH_ON_SY)
                CheckDlgButton(hDlg, IDC_ISOCH_ATTACH_SYNCH_ON_SY, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_SYNCH_ON_TAG)
                CheckDlgButton(hDlg, IDC_ISOCH_ATTACH_SYNCH_ON_TAG, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_SYNCH_ON_TIME)
                CheckDlgButton(hDlg, IDC_ISOCH_ATTACH_SYNCH_ON_TIME, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_USE_SY_TAG_IN_FIRST)
                CheckDlgButton(hDlg, IDC_ISOCH_ATTACH_USE_SY_TAG_IN_FIRST, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_TIME_STAMP_ON_COMPLETION)
                CheckDlgButton(hDlg, IDC_ISOCH_ATTACH_TIME_STAMP_COMPLETE, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_PRIORITY_TIME_DELIVERY)
                CheckDlgButton(hDlg, IDC_ISOCH_ATTACH_PRI_TIME_DELIVERY, BST_CHECKED);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].ulLength, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ATTACH_LENGTH, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].ulTag, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ATTACH_TAG_VALUE, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleOffset, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ATTACH_CYCLE_OFFSET, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_SecondCount, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ATTACH_SECOND_COUNT, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].nMaxBytesPerFrame, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ATTACH_BYTES_PER_FRAME, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].ulSynch, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ATTACH_SYNCH_VALUE, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleCount, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_ATTACH_CYCLE_COUNT, tmpBuff);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].bUseCallback)
                CheckDlgButton( hDlg, IDC_ISOCH_ATTACH_USE_CALLBACK, BST_CHECKED);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ISOCH_ATTACH_RESOURCE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", &(pIsochAttachBuffers->hResource)))
                    {
                        // failed to get the handle, just return here
                        EndDialog(hDlg, TRUE);
                        return FALSE;
                    }

                    GetDlgItemText(hDlg, IDC_ISOCH_ATTACH_NUM_DESCRIPTORS, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->nNumberOfDescriptors = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ATTACH_LENGTH, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].ulLength = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ATTACH_TAG_VALUE, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].ulTag = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ATTACH_CYCLE_OFFSET, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleOffset = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ATTACH_SECOND_COUNT, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_SecondCount = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ATTACH_BYTES_PER_FRAME, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].nMaxBytesPerFrame = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ATTACH_SYNCH_VALUE, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].ulSynch = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_ATTACH_CYCLE_COUNT, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleCount = strtoul(tmpBuff, NULL, 16);

                    pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ATTACH_SYNCH_ON_SY))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_SYNCH_ON_SY;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ATTACH_SYNCH_ON_TAG))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_SYNCH_ON_TAG;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ATTACH_SYNCH_ON_TIME))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_SYNCH_ON_TIME;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ATTACH_USE_SY_TAG_IN_FIRST))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_USE_SY_TAG_IN_FIRST;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ATTACH_TIME_STAMP_COMPLETE))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_TIME_STAMP_ON_COMPLETION;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ATTACH_PRI_TIME_DELIVERY))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_PRIORITY_TIME_DELIVERY;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_ATTACH_USE_CALLBACK))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].bUseCallback = TRUE;
                    else
                        pIsochAttachBuffers->R3_IsochDescriptor[0].bUseCallback = FALSE;

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
} // IsochAttachBuffersDlgProc

void
w1394_IsochAttachBuffers(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_ATTACH_BUFFERS        isochAttachBuffers;
    RING3_ISOCH_DESCRIPTOR      r3IsochDescriptor;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochAttachBuffers\r\n"));

    isochAttachBuffers.hResource = 0;
    isochAttachBuffers.nNumberOfDescriptors = 1;
    isochAttachBuffers.pIsochDescriptor = 0;
    isochAttachBuffers.ulBufferSize = 0;

    isochAttachBuffers.R3_IsochDescriptor[0].fulFlags = 0;
    isochAttachBuffers.R3_IsochDescriptor[0].ulLength = 153600;
    isochAttachBuffers.R3_IsochDescriptor[0].nMaxBytesPerFrame = 640;
    isochAttachBuffers.R3_IsochDescriptor[0].ulSynch = 0;
    isochAttachBuffers.R3_IsochDescriptor[0].ulTag = 0;
    isochAttachBuffers.R3_IsochDescriptor[0].CycleTime.CL_CycleOffset = 0;
    isochAttachBuffers.R3_IsochDescriptor[0].CycleTime.CL_CycleCount = 0;
    isochAttachBuffers.R3_IsochDescriptor[0].CycleTime.CL_SecondCount = 0;
    isochAttachBuffers.R3_IsochDescriptor[0].bUseCallback = TRUE;
    isochAttachBuffers.R3_IsochDescriptor[0].bAutoDetach = TRUE;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochAttachBuffers",
                        hWnd,
                        IsochAttachBuffersDlgProc,
                        (LPARAM)&isochAttachBuffers
                        )) {

        dwRet = IsochAttachBuffers( hWnd,
                                    szDeviceName,
                                    &isochAttachBuffers,
                                    TRUE,
                                    TRUE
                                    );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochAttachBuffers\r\n"));
    return;
} // w1394_IsochAttachBuffers

INT_PTR CALLBACK
IsochDetachBuffersDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_DETACH_BUFFERS        pIsochDetachBuffers;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochDetachBuffers = (PISOCH_DETACH_BUFFERS)lParam;

            sprintf (tmpBuff, "%p", pIsochDetachBuffers->hResource);
            SetDlgItemText(hDlg, IDC_ISOCH_DETACH_RESOURCE, tmpBuff);

            _ultoa((ULONG)pIsochDetachBuffers->nNumberOfDescriptors, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_DETACH_NUM_DESCRIPTORS, tmpBuff);

            sprintf (tmpBuff, "%p", pIsochDetachBuffers->pIsochDescriptor);
            SetDlgItemText(hDlg, IDC_ISOCH_DETACH_ISOCH_DESCRIPTOR, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ISOCH_DETACH_RESOURCE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", &(pIsochDetachBuffers->hResource)))
                    {
                        // failed to get the handle, just return here
                        EndDialog(hDlg, TRUE);
                        return FALSE;
                    }

                    GetDlgItemText(hDlg, IDC_ISOCH_DETACH_NUM_DESCRIPTORS, tmpBuff, STRING_SIZE);
                    pIsochDetachBuffers->nNumberOfDescriptors = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_DETACH_ISOCH_DESCRIPTOR, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", &(pIsochDetachBuffers->pIsochDescriptor)))
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
} // IsochDetachBuffersDlgProc

void
w1394_IsochDetachBuffers(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_DETACH_BUFFERS        isochDetachBuffers;
    DWORD                       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochDetachBuffers\r\n"));

    isochDetachBuffers.hResource = 0;
    isochDetachBuffers.nNumberOfDescriptors = 0;
    isochDetachBuffers.pIsochDescriptor = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochDetachBuffers",
                        hWnd,
                        IsochDetachBuffersDlgProc,
                        (LPARAM)&isochDetachBuffers
                        )) {

        dwRet = IsochDetachBuffers( hWnd,
                                    szDeviceName,
                                    &isochDetachBuffers
                                    );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochDetachBuffers\r\n"));
    return;
} // w1394_IsochDetachBuffers

INT_PTR CALLBACK
IsochFreeBandwidthDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PHANDLE      pHandle;
    static CHAR         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pHandle = (PHANDLE)lParam;

            // put offset low and high and buffer length in edit controls
            sprintf (tmpBuff, "%p", pHandle);
            SetDlgItemText(hDlg, IDC_ISOCH_FREE_BW_HANDLE, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ISOCH_FREE_BW_HANDLE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", pHandle))
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
} // IsochFreeBandwidthDlgProc

void
w1394_IsochFreeBandwidth(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    HANDLE      hBandwidth;
    DWORD       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochFreeBandwidth\r\n"));

    hBandwidth = NULL;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochFreeBandwidth",
                        hWnd,
                        IsochFreeBandwidthDlgProc,
                        (LPARAM)&hBandwidth
                        )) {

        dwRet = IsochFreeBandwidth( hWnd,
                                    szDeviceName,
                                    hBandwidth
                                    );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochFreeBandwidth\r\n"));
    return;
} // w1394_IsochFreeBandwidth

INT_PTR CALLBACK
IsochFreeChannelDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PULONG       pChannel;
    static CHAR         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pChannel = (PULONG)lParam;

            // put offset low and high and buffer length in edit controls
            _ultoa(*pChannel, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_FREE_CHAN_CHANNEL, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ISOCH_FREE_CHAN_CHANNEL, tmpBuff, STRING_SIZE);
                    *pChannel = strtoul(tmpBuff, NULL, 16);

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
} // IsochFreeChannelDlgProc

void
w1394_IsochFreeChannel(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ULONG       nChannel;
    DWORD       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochFreeChannel\r\n"));

    nChannel = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochFreeChannel",
                        hWnd,
                        IsochFreeChannelDlgProc,
                        (LPARAM)&nChannel
                        )) {

        dwRet = IsochFreeChannel( hWnd,
                                  szDeviceName,
                                  nChannel
                                  );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochFreeChannel\r\n"));
    return;
} // w1394_IsochFreeChannel

INT_PTR CALLBACK
IsochFreeResourcesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PHANDLE      phResource;
    static CHAR         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            phResource = (HANDLE)lParam;

            sprintf (tmpBuff, "%p", *phResource);
            SetDlgItemText(hDlg, IDC_FREE_RES_RESOURCE_HANDLE, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_FREE_RES_RESOURCE_HANDLE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", phResource))
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
} // IsochFreeResourcesDlgProc

void
w1394_IsochFreeResources(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    HANDLE      hResource;
    DWORD       dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochFreeResources\r\n"));

    hResource = NULL;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochFreeResources",
                        hWnd,
                        IsochFreeResourcesDlgProc,
                        (LPARAM)&hResource
                        )) {

        dwRet = IsochFreeResources( hWnd,
                                    szDeviceName,
                                    hResource
                                    );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochFreeResources\r\n"));
    return;
} // w1394_IsochFreeResources

INT_PTR CALLBACK
IsochListenDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_LISTEN    pIsochListen;
    static CHAR             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochListen = (PISOCH_LISTEN)lParam;

            sprintf (tmpBuff, "%p", pIsochListen->hResource);
            SetDlgItemText(hDlg, IDC_ISOCH_LISTEN_RESOURCE_HANDLE, tmpBuff);

            _ultoa(pIsochListen->StartTime.CL_CycleOffset, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LISTEN_CYCLE_OFFSET, tmpBuff);

            _ultoa(pIsochListen->StartTime.CL_CycleCount, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LISTEN_CYCLE_COUNT, tmpBuff);

            _ultoa(pIsochListen->StartTime.CL_SecondCount, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LISTEN_SECOND_COUNT, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ISOCH_LISTEN_RESOURCE_HANDLE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", &(pIsochListen->hResource)))
                    {
                        // failed to get the handle, just return here
                        EndDialog(hDlg, TRUE);
                        return FALSE;
                    }

                    GetDlgItemText(hDlg, IDC_ISOCH_LISTEN_CYCLE_OFFSET, tmpBuff, STRING_SIZE);
                    pIsochListen->StartTime.CL_CycleOffset = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_LISTEN_CYCLE_COUNT, tmpBuff, STRING_SIZE);
                    pIsochListen->StartTime.CL_CycleCount = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_LISTEN_SECOND_COUNT, tmpBuff, STRING_SIZE);
                    pIsochListen->StartTime.CL_SecondCount = strtoul(tmpBuff, NULL, 16);

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
} // IsochListenDlgProc

void
w1394_IsochListen(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_LISTEN    isochListen;
    DWORD           dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochListen\r\n"));

    isochListen.hResource = NULL;
    isochListen.fulFlags = 0;
    isochListen.StartTime.CL_CycleOffset = 0;
    isochListen.StartTime.CL_CycleCount = 0;
    isochListen.StartTime.CL_SecondCount = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochListen",
                        hWnd,
                        IsochListenDlgProc,
                        (LPARAM)&isochListen
                        )) {

        dwRet = IsochListen( hWnd,
                             szDeviceName,
                             &isochListen
                             );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochListen\r\n"));
    return;
} // w1394_IsochListen

void
w1394_IsochQueryCurrentCycleTime(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    CYCLE_TIME      CycleTime;
    DWORD           dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochQueryCurrentCycleTime\r\n"));

    dwRet = IsochQueryCurrentCycleTime( hWnd,
                                        szDeviceName,
                                        &CycleTime
                                        );

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochQueryCurrentCycleTime\r\n"));
    return;
} // w1394_IsochQueryCurrentCycleTime

INT_PTR CALLBACK
IsochQueryResourcesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_QUERY_RESOURCES       pIsochQueryResources;
    static CHAR                         tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochQueryResources = (PISOCH_QUERY_RESOURCES)lParam;

            if (pIsochQueryResources->fulSpeed == SPEED_FLAGS_FASTEST) {

                CheckRadioButton( hDlg,
                                  IDC_ISOCH_QUERY_RES_100MBPS,
                                  IDC_ISOCH_QUERY_RES_FASTEST,
                                  IDC_ISOCH_QUERY_RES_FASTEST
                                  );
            }
            else {

                CheckRadioButton( hDlg,
                                  IDC_ISOCH_QUERY_RES_100MBPS,
                                  IDC_ISOCH_QUERY_RES_FASTEST,
                                  pIsochQueryResources->fulSpeed + (IDC_ISOCH_QUERY_RES_100MBPS-1)
                                  );
            }

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    pIsochQueryResources->fulSpeed = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_QUERY_RES_100MBPS))
                        pIsochQueryResources->fulSpeed = SPEED_FLAGS_100;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_QUERY_RES_200MBPS))
                        pIsochQueryResources->fulSpeed = SPEED_FLAGS_200;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_QUERY_RES_400MBPS))
                        pIsochQueryResources->fulSpeed = SPEED_FLAGS_400;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_QUERY_RES_1600MBPS))
                        pIsochQueryResources->fulSpeed = SPEED_FLAGS_1600;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_QUERY_RES_FASTEST))
                        pIsochQueryResources->fulSpeed = SPEED_FLAGS_FASTEST;

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
} // IsochQueryResourcesDlgProc

void
w1394_IsochQueryResources(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_QUERY_RESOURCES   isochQueryResources;
    DWORD                   dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochQueryResources\r\n"));

    isochQueryResources.fulSpeed = SPEED_FLAGS_200;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochQueryResources",
                        hWnd,
                        IsochQueryResourcesDlgProc,
                        (LPARAM)&isochQueryResources
                        )) {

        dwRet = IsochQueryResources( hWnd,
                                     szDeviceName,
                                     &isochQueryResources
                                     );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochQueryResources\r\n"));
    return;
} // w1394_IsochQueryResources

INT_PTR CALLBACK
IsochSetChannelBandwidthDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_SET_CHANNEL_BANDWIDTH     pIsochSetChannelBandwidth;
    static CHAR                             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochSetChannelBandwidth = (PISOCH_SET_CHANNEL_BANDWIDTH)lParam;

            sprintf (tmpBuff, "%p", pIsochSetChannelBandwidth->hBandwidth);
            SetDlgItemText(hDlg, IDC_ISOCH_SET_CHAN_BW_RESOURCE, tmpBuff);

            _ultoa(pIsochSetChannelBandwidth->nMaxBytesPerFrame, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_SET_CHAN_BW_BYTES_PER_FRAME, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ISOCH_SET_CHAN_BW_RESOURCE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", pIsochSetChannelBandwidth->hBandwidth))
                    {
                        // failed to get the handle, just return here
                        EndDialog(hDlg, TRUE);
                        return FALSE;
                    }

                    GetDlgItemText(hDlg, IDC_ISOCH_SET_CHAN_BW_BYTES_PER_FRAME, tmpBuff, STRING_SIZE);
                    pIsochSetChannelBandwidth->nMaxBytesPerFrame = strtoul(tmpBuff, NULL, 16);

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
} // IsochSetChannelBandwidthDlgProc

void
w1394_IsochSetChannelBandwidth(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_SET_CHANNEL_BANDWIDTH         isochSetChannelBandwidth;
    DWORD                               dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochSetChannelBandwidth\r\n"));

    isochSetChannelBandwidth.hBandwidth = NULL;
    isochSetChannelBandwidth.nMaxBytesPerFrame = 640;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochSetChannelBandwidth",
                        hWnd,
                        IsochSetChannelBandwidthDlgProc,
                        (LPARAM)&isochSetChannelBandwidth
                        )) {

        dwRet = IsochSetChannelBandwidth( hWnd,
                                          szDeviceName,
                                          &isochSetChannelBandwidth
                                          );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochSetChannelBandwidth\r\n"));
    return;
} // w1394_IsochSetChannelBandwidth

INT_PTR CALLBACK
IsochStopDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_STOP      pIsochStop;
    static CHAR             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochStop = (PISOCH_STOP)lParam;

            // put offset low and high and buffer length in edit controls
            sprintf (tmpBuff, "%p", pIsochStop->hResource);
            SetDlgItemText(hDlg, IDC_ISOCH_STOP_HANDLE, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ISOCH_STOP_HANDLE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", &(pIsochStop->hResource)))
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
} // IsochStopDlgProc

void
w1394_IsochStop(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_STOP      isochStop;
    DWORD           dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochStop\r\n"));

    isochStop.hResource = NULL;
    isochStop.fulFlags = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochStop",
                        hWnd,
                        IsochStopDlgProc,
                        (LPARAM)&isochStop
                        )) {

        dwRet = IsochStop( hWnd,
                           szDeviceName,
                           &isochStop
                           );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochStop\r\n"));
    return;
} // w1394_IsochStop

INT_PTR CALLBACK
IsochTalkDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_TALK      pIsochTalk;
    static CHAR             tmpBuff[STRING_SIZE];

    switch (uMsg) {

        case WM_INITDIALOG:

            pIsochTalk = (PISOCH_TALK)lParam;

            // put offset low and high and buffer length in edit controls
            sprintf (tmpBuff, "%p", pIsochTalk->hResource);
            SetDlgItemText(hDlg, IDC_ISOCH_TALK_RESOURCE_HANDLE, tmpBuff);

            _ultoa(pIsochTalk->StartTime.CL_CycleOffset, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_TALK_CYCLE_OFFSET, tmpBuff);

            _ultoa(pIsochTalk->StartTime.CL_CycleCount, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_TALK_CYCLE_COUNT, tmpBuff);

            _ultoa(pIsochTalk->StartTime.CL_SecondCount, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_TALK_SECOND_COUNT, tmpBuff);

            return(TRUE); // WM_INITDIALOG

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    GetDlgItemText(hDlg, IDC_ISOCH_TALK_RESOURCE_HANDLE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", &(pIsochTalk->hResource)))
                    {
                        // failed to get the handle, just return here
                        EndDialog(hDlg, TRUE);
                        return FALSE;
                    }

                    GetDlgItemText(hDlg, IDC_ISOCH_TALK_CYCLE_OFFSET, tmpBuff, STRING_SIZE);
                    pIsochTalk->StartTime.CL_CycleOffset = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_TALK_CYCLE_COUNT, tmpBuff, STRING_SIZE);
                    pIsochTalk->StartTime.CL_CycleCount = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_TALK_SECOND_COUNT, tmpBuff, STRING_SIZE);
                    pIsochTalk->StartTime.CL_SecondCount = strtoul(tmpBuff, NULL, 16);

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
} // IsochTalkDlgProc

void
w1394_IsochTalk(
    HWND    hWnd,
    PSTR    szDeviceName
    )
{
    ISOCH_TALK      isochTalk;
    DWORD           dwRet;

    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochTalk\r\n"));

    isochTalk.hResource = NULL;
    isochTalk.fulFlags = 0;
    isochTalk.StartTime.CL_CycleOffset = 0;
    isochTalk.StartTime.CL_CycleCount = 0;
    isochTalk.StartTime.CL_SecondCount = 0;

    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochTalk",
                        hWnd,
                        IsochTalkDlgProc,
                        (LPARAM)&isochTalk
                        )) {

        dwRet = IsochTalk( hWnd,
                           szDeviceName,
                           &isochTalk
                           );
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochTalk\r\n"));
    return;
} // w1394_IsochTalk

INT_PTR CALLBACK
IsochLoopbackDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    )
{
    static PISOCH_LOOPBACK_PARAMS       pIsochLoopbackParams;
    static PISOCH_ATTACH_BUFFERS        pIsochAttachBuffers;
    CHAR                                tmpBuff[STRING_SIZE];

    switch(uMsg) {

        case WM_INITDIALOG:
        
            pIsochLoopbackParams = (PISOCH_LOOPBACK_PARAMS)lParam;
            pIsochAttachBuffers = &pIsochLoopbackParams->isochAttachBuffers;

            CheckRadioButton( hDlg,
                              IDC_ISOCH_LOOP_LISTEN,
                              IDC_ISOCH_LOOP_TALK,
                              pIsochLoopbackParams->ulLoopFlag + (IDC_ISOCH_LOOP_LISTEN-1)
                              );

            if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_USED_IN_LISTENING)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_LISTEN, BST_CHECKED);

            if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_USED_IN_TALKING)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_TALK, BST_CHECKED);

            if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_BUFFERS_CIRCULAR)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_CIRCULAR, BST_CHECKED);

            if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_STRIP_ADDITIONAL_QUADLETS)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_STRIP_QUADS, BST_CHECKED);

            if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_SYNCH_ON_TIME)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_SYNC_TIME, BST_CHECKED);

            if (pIsochLoopbackParams->ulLoopFlag & RESOURCE_USE_PACKET_BASED)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_PACKET_BASED, BST_CHECKED);

            sprintf (tmpBuff, "%p", pIsochAttachBuffers->hResource);
            SetDlgItemText(hDlg, IDC_ISOCH_LOOP_RESOURCE, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->nNumberOfDescriptors, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LOOP_NUM_DESCRIPTORS, tmpBuff);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_SYNCH_ON_SY)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_SYNCH_ON_SY, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_SYNCH_ON_TAG)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_SYNCH_ON_TAG, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_SYNCH_ON_TIME)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_SYNCH_ON_TIME, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_USE_SY_TAG_IN_FIRST)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_USE_SY_TAG_IN_FIRST, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_TIME_STAMP_ON_COMPLETION)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_TIME_STAMP_COMPLETE, BST_CHECKED);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags & DESCRIPTOR_PRIORITY_TIME_DELIVERY)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_PRI_TIME_DELIVERY, BST_CHECKED);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].ulLength, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LOOP_LENGTH, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].ulTag, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LOOP_TAG_VALUE, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleOffset, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LOOP_CYCLE_OFFSET, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_SecondCount, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LOOP_SECOND_COUNT, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].nMaxBytesPerFrame, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LOOP_BYTES_PER_FRAME, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].ulSynch, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LOOP_SYNCH_VALUE, tmpBuff);

            _ultoa((ULONG)pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleCount, tmpBuff, 16);
            SetDlgItemText(hDlg, IDC_ISOCH_LOOP_CYCLE_COUNT, tmpBuff);

            if (pIsochAttachBuffers->R3_IsochDescriptor[0].bUseCallback)
                CheckDlgButton( hDlg, IDC_ISOCH_LOOP_USE_CALLBACK, BST_CHECKED);

            return(TRUE);

        case WM_COMMAND:

            switch (LOWORD(wParam)) {

                case IDOK:

                    pIsochLoopbackParams->ulLoopFlag = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_LISTEN))
                        pIsochLoopbackParams->ulLoopFlag |= RESOURCE_USED_IN_LISTENING;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_TALK))
                        pIsochLoopbackParams->ulLoopFlag |= RESOURCE_USED_IN_TALKING;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_CIRCULAR))
                        pIsochLoopbackParams->ulLoopFlag |= RESOURCE_BUFFERS_CIRCULAR;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_STRIP_QUADS))
                        pIsochLoopbackParams->ulLoopFlag |= RESOURCE_STRIP_ADDITIONAL_QUADLETS;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_SYNC_TIME))
                        pIsochLoopbackParams->ulLoopFlag |= RESOURCE_SYNCH_ON_TIME;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_PACKET_BASED))
                        pIsochLoopbackParams->ulLoopFlag |= RESOURCE_USE_PACKET_BASED;

                    GetDlgItemText(hDlg, IDC_ISOCH_LOOP_RESOURCE, tmpBuff, STRING_SIZE);
                    if (!sscanf (tmpBuff, "%p", &(pIsochAttachBuffers->hResource)))
                    {
                        // failed to get the handle, just return here
                        EndDialog(hDlg, TRUE);
                        return FALSE;
                    }

                    GetDlgItemText(hDlg, IDC_ISOCH_LOOP_NUM_DESCRIPTORS, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->nNumberOfDescriptors = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_LOOP_LENGTH, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].ulLength = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_LOOP_TAG_VALUE, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].ulTag = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_LOOP_CYCLE_OFFSET, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleOffset = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_LOOP_SECOND_COUNT, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_SecondCount = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_LOOP_BYTES_PER_FRAME, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].nMaxBytesPerFrame = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_LOOP_SYNCH_VALUE, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].ulSynch = strtoul(tmpBuff, NULL, 16);

                    GetDlgItemText(hDlg, IDC_ISOCH_LOOP_CYCLE_COUNT, tmpBuff, STRING_SIZE);
                    pIsochAttachBuffers->R3_IsochDescriptor[0].CycleTime.CL_CycleCount = strtoul(tmpBuff, NULL, 16);

                    pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_SYNCH_ON_SY))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_SYNCH_ON_SY;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_SYNCH_ON_TAG))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_SYNCH_ON_TAG;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_SYNCH_ON_TIME))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_SYNCH_ON_TIME;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_USE_SY_TAG_IN_FIRST))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_USE_SY_TAG_IN_FIRST;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_TIME_STAMP_COMPLETE))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_TIME_STAMP_ON_COMPLETION;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_PRI_TIME_DELIVERY))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].fulFlags |= DESCRIPTOR_PRIORITY_TIME_DELIVERY;

                    if (IsDlgButtonChecked(hDlg, IDC_ISOCH_LOOP_USE_CALLBACK))
                        pIsochAttachBuffers->R3_IsochDescriptor[0].bUseCallback = TRUE;
                    else
                        pIsochAttachBuffers->R3_IsochDescriptor[0].bUseCallback = FALSE;

                    EndDialog(hDlg, TRUE);
                    return(TRUE);

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return(TRUE);

                default:
                    return(TRUE);

            } // switch

            break; // WM_COMMAND

        default:
            break;

    } // switch

    return(FALSE);
} // IsochLoopbackDlgProc

void
w1394_IsochStartLoopback(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_LOOPBACK_PARAMS      IsochLoopbackParams
    )
{
    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochStartLoopback\r\n"));

    IsochLoopbackParams->ulLoopFlag = RESOURCE_USED_IN_TALKING;
    IsochLoopbackParams->nIterations = 0;
    IsochLoopbackParams->bAutoFill = TRUE;
    IsochLoopbackParams->bAutoAlloc = TRUE;

    IsochLoopbackParams->isochAttachBuffers.hResource = NULL;
    IsochLoopbackParams->isochAttachBuffers.nNumberOfDescriptors = 1;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].fulFlags = 0;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].ulLength = 153600;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].nMaxBytesPerFrame = 640;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].ulSynch = 0;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].ulTag = 0;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].CycleTime.CL_CycleOffset = 0;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].CycleTime.CL_CycleCount = 0;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].CycleTime.CL_SecondCount = 0;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].bUseCallback = TRUE;
    IsochLoopbackParams->isochAttachBuffers.R3_IsochDescriptor[0].bAutoDetach = TRUE;

    // do our dialog box
    if (DialogBoxParam( (HINSTANCE) GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                        "IsochLoopback",
                        hWnd,
                        IsochLoopbackDlgProc,
                        (LPARAM)IsochLoopbackParams
                        )) {

        GetLocalTime(&IsochStartTime);

        IsochStartLoopback(hWnd, szDeviceName, IsochLoopbackParams);

        isochLoopbackStarted = TRUE;
    }

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochStartLoopback\r\n"));
    return;
} // w1394_IsochStartLoopback

void
w1394_IsochStopLoopback(
    HWND                        hWnd,
    PISOCH_LOOPBACK_PARAMS      IsochLoopbackParams
    )
{
    TRACE(TL_TRACE, (hWnd, "Enter w1394_IsochStopLoopback\r\n"));

#ifdef LOGGER
    Logger_WriteTestLog( "1394",
                         "Isoch Loopback",
                         NULL,
                         IsochLoopbackParams->ulPass,
                         IsochLoopbackParams->ulFail,
                         &IsochStartTime,
                         NULL,
                         NULL
                         );
#endif
    IsochStopLoopback( IsochLoopbackParams );

    isochLoopbackStarted = FALSE;

    TRACE(TL_TRACE, (hWnd, "Exit w1394_IsochStopLoopback\r\n"));
    return;
} // w1394_IsochStopLoopback


