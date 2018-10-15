/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    toastva.h

Abstract:

    Header files and resource IDs used by the TOASTVA sample.

--*/

#include <windows.h>
#include <windowsx.h>
#include <string.h>
#include <shellapi.h>
#include <prsht.h>
#include <windef.h>
#include <regstr.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <newdev.h>
#include "resource.h"

//
// String constants
//
#define ENUMERATOR_NAME     L"{b85b7c50-6a01-11d2-b841-00c04fad5171}"
#define HW_ID_TO_UPDATE     L"{b85b7c50-6a01-11d2-b841-00c04fad5171}\\MsToaster"
#define DEVICE_INF_NAME     L"toastpkg.inf"
#define TOASTAPP_SETUP_PATH L"ToastApp\\setup.exe"

//
// global variables
//
extern HINSTANCE g_hInstance;

//
// utility routines
//
BOOL
IsDeviceInstallInProgress(VOID);

VOID
MarkDevicesAsNeedReinstall(
    IN HDEVINFO DeviceInfoSet
    );

DWORD
GetDeviceConfigFlags(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

VOID
SetDeviceConfigFlags(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN DWORD            ConfigFlags
    );

HDEVINFO
GetNonPresentDevices(
    IN  LPCWSTR Enumerator OPTIONAL,
    IN  LPCWSTR HardwareID
    );

VOID
DoValueAddWizard(
    IN LPCWSTR MediaRootDirectory
    );

