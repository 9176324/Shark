//////////////////////////////////////////////////////////////////////////////
//
//                     (C) Philips Semiconductors 2001
//  All rights are reserved. Reproduction in whole or in part is prohibited
//  without the written consent of the copyright owner.
//
//  Philips reserves the right to make changes without notice at any time.
//  Philips makes no warranty, expressed, implied or statutory, including but
//  not limited to any implied warranty of merchantability or fitness for any
//  particular purpose, or that the use will not infringe any third party
//  patent, copyright or trademark. Philips must not be liable for any loss
//  or damage arising from its use.
//
//////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <setupapi.h>
#include <stdio.h>

//+---------------------------------------------------------------------------
//
// A Coinstaller must not generate any popup to the user,
// so we generate OutputDebugStrings in case of an error.
//
#if DBG
#define DbgOut(Text) OutputDebugString(TEXT("CoInstaller: " Text "\n"))
#else
#define DbgOut(Text) 
#endif 

//////////////////////////////////////////////////////////////////////////////
//
// Description:         
//  type here function description
//
// Return Value:         
//  type here return value description or remove these lines
//
//////////////////////////////////////////////////////////////////////////////
//+---------------------------------------------------------------------------
//
//  Function:   CoInstallerEntry 
//
//  Purpose:    Responds to co-installer messages for the Europa driver
//
//  Arguments:
//      InstallFunction   [in] 
//      DeviceInfoSet     [in]
//      DeviceInfoData    [in]
//      Context           [inout]
//
//  Returns:    NO_ERROR, ERROR_DI_POSTPROCESSING_REQUIRED, or an error code.
//
HRESULT CoInstallerEntry
(
 IN     DI_FUNCTION               InstallFunction,
 IN     HDEVINFO                  DeviceInfoSet,
 IN     PSP_DEVINFO_DATA          DeviceInfoData,  OPTIONAL
 IN OUT PCOINSTALLER_CONTEXT_DATA Context
 )
{
    HMODULE hD3D9Dll = NULL;
    switch (InstallFunction)
    {
    case DIF_INSTALLDEVICE: 
        // check if DX 9 or higher is available
        hD3D9Dll = LoadLibrary("D3D9.dll");
        if( hD3D9Dll == NULL)
        {
            DbgOut("Error: DX9 or higher not found");
            return ERROR_BAD_ENVIRONMENT;
        }
        else
        {
            FreeLibrary(hD3D9Dll);
        }
        break;
    case DIF_REMOVE:
        DbgOut("DIF_REMOVE");
        break;
    case DIF_SELECTDEVICE:
        DbgOut("DIF_SELECTDEVICE");
        break;
    case DIF_ASSIGNRESOURCES:
        DbgOut("DIF_ASSIGNRESOURCES");
        break;
    case DIF_PROPERTIES:
        DbgOut("DIF_PROPERTIES");
        break;
    case DIF_FIRSTTIMESETUP:
        DbgOut("DIF_FIRSTTIMESETUP");
        break;
    case DIF_FOUNDDEVICE:
        DbgOut("DIF_FOUNDDEVICE");
        break;
    case DIF_SELECTCLASSDRIVERS:
        DbgOut("DIF_SELECTCLASSDRIVERS");
        break;
    case DIF_VALIDATECLASSDRIVERS:
        DbgOut("DIF_VALIDATECLASSDRIVERS");
        break;
    case DIF_INSTALLCLASSDRIVERS:
        DbgOut("DIF_INSTALLCLASSDRIVERS");
        break;
    case DIF_CALCDISKSPACE:
        DbgOut("DIF_CALCDISKSPACE");
        break;
    case DIF_DESTROYPRIVATEDATA:
        DbgOut("DIF_DESTROYPRIVATEDATA");
        break;
    case DIF_VALIDATEDRIVER:
        DbgOut("DIF_VALIDATEDRIVER");
        break;
    case DIF_MOVEDEVICE:
        DbgOut("DIF_MOVEDEVICE");
        break;
    case DIF_DETECT:
        DbgOut("DIF_DETECT");
        break;
    case DIF_INSTALLWIZARD:
        DbgOut("DIF_INSTALLWIZARD");
        break;
    case DIF_DESTROYWIZARDDATA:
        DbgOut("DIF_DESTROYWIZARDDATA");
        break;
    case DIF_PROPERTYCHANGE:
        DbgOut("DIF_PROPERTYCHANGE");
        break;
    case DIF_ENABLECLASS:
        DbgOut("DIF_ENABLECLASS");
        break;
    case DIF_DETECTVERIFY:
        DbgOut("DIF_DETECTVERIFY");
        break;
    case DIF_INSTALLDEVICEFILES:
        DbgOut("DIF_INSTALLDEVICEFILES");
        break;
    case DIF_ALLOW_INSTALL:
        DbgOut("DIF_ALLOW_INSTALL");
        break;
    case DIF_SELECTBESTCOMPATDRV:
        DbgOut("DIF_SELECTBESTCOMPATDRV");
        break;
    case DIF_REGISTERDEVICE:
        DbgOut("DIF_REGISTERDEVICE");
        break;
    case DIF_NEWDEVICEWIZARD_PRESELECT:
        DbgOut("DIF_NEWDEVICEWIZARD_PRESELECT");
        break;
    case DIF_NEWDEVICEWIZARD_SELECT:
        DbgOut("DIF_NEWDEVICEWIZARD_SELECT");
        break;
    case DIF_NEWDEVICEWIZARD_PREANALYZE:
        DbgOut("DIF_NEWDEVICEWIZARD_PREANALYZE");
        break;
    case DIF_NEWDEVICEWIZARD_POSTANALYZE:
        DbgOut("DIF_NEWDEVICEWIZARD_POSTANALYZE");
        break;
    case DIF_NEWDEVICEWIZARD_FINISHINSTALL:
        DbgOut("DIF_NEWDEVICEWIZARD_FINISHINSTALL");
        break;
    case DIF_INSTALLINTERFACES:
        DbgOut("DIF_INSTALLINTERFACES");
        break;
    case DIF_DETECTCANCEL:
        DbgOut("DIF_DETECTCANCEL");
        break;
    case DIF_REGISTER_COINSTALLERS:
        DbgOut("DIF_REGISTER_COINSTALLERS");
        break;
    case DIF_ADDPROPERTYPAGE_ADVANCED:
        DbgOut("DIF_ADDPROPERTYPAGE_ADVANCED");
        break;
    case DIF_ADDPROPERTYPAGE_BASIC:
        DbgOut("DIF_ADDPROPERTYPAGE_BASIC");
        break;
    case DIF_TROUBLESHOOTER:
        DbgOut("DIF_TROUBLESHOOTER");
        break;
    case DIF_POWERMESSAGEWAKE:
        DbgOut("DIF_POWERMESSAGEWAKE");
        break;
    default:
        DbgOut("?????");
        break;
    }
    return NO_ERROR;    
}

