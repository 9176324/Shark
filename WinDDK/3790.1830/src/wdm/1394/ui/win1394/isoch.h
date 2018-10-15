/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    isoch.h

Abstract

    1394 isoch wrappers

Author:

    Peter Binder (pbinder) 5/4/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
05/04/98 pbinder   taken from old win1394
--*/

INT_PTR CALLBACK
IsochAllocateBandwidthDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochAllocateBandwidth(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochAllocateChannelDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochAllocateChannel(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochAllocateResourcesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochAllocateResources(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochAttachBuffersDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochAttachBuffers(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochDetachBuffersDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochDetachBuffers(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochFreeBandwidthDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochFreeBandwidth(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochFreeChannelDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochFreeChannel(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochFreeResourcesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochFreeResources(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochListenDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochListen(
    HWND    hWnd,
    PSTR    szDeviceName
    );

void
w1394_IsochQueryCurrentCycleTime(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochQueryResourcesDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochQueryResources(
    HWND    hWnd,
    PSTR    szDeviceName
    );

void
w1394_IsochSetChannelBandwidth(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochStopDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochStop(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochTalkDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochTalk(
    HWND    hWnd,
    PSTR    szDeviceName
    );

INT_PTR CALLBACK
IsochLoopbackDlgProc(
    HWND        hDlg,
    UINT        uMsg,
    WPARAM      wParam,
    LPARAM      lParam
    );

void
w1394_IsochStartLoopback(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_LOOPBACK_PARAMS      IsochLoopbackParams
    );

void
w1394_IsochStopLoopback(
    HWND                        hWnd,
    PISOCH_LOOPBACK_PARAMS      IsochLoopbackParams
    );


