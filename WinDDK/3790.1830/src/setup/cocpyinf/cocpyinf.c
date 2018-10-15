//+-----------------------------------------------------------------------------
//
//  Microsoft Windows
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//  File:       COCPYINF.C
//
//  Contents:   co-installer hook to copy INF's into INF directory
//
//              The assumption here is that the install section may
//              specify, eg, "CopyINF = a.inf,b.inf,c.inf"
//              where the inf's specified must exist in same location
//              as primary inf.
//
//              This example shows both a way to install INF's for a
//              multi-function device for Windows 2000, and also how
//              to add to the INF syntax for other purposes.
//
//  Notes:      For a complete description of CoInstallers, please see the
//                 Microsoft Windows 2000 DDK Documentation
//
//------------------------------------------------------------------------------
#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <malloc.h>


//+-----------------------------------------------------------------------------
//
// WARNING!
//
// A Coinstaller must not generate any popup to the user when called with
// DI_QUIETINSTALL. However, it's generally never a good idea to generate
// popups.
//
// OutputDebugString is a fine way to output information for debugging
//
#if DBG
#define DbgOut(Text) OutputDebugString(TEXT("CoCopyInf: " Text "\n"))
#else
#define DbgOut(Text)
#endif

//
// this is the keyword we've introduced by this co-installer
// note that the name is not case sensative
//
#define COPYINF_KEY TEXT("CopyINF")


//+-----------------------------------------------------------------------------
//
//  Function:   IsWindows2000
//
//  Purpose:    Determine if we are running on Windows 2000
//
//  Arguments:   None
//
//  Returns:  TRUE if on Windows 2000, FALSE otherwise
//
BOOL 
IsWindows2000(
    void
    )
{
    OSVERSIONINFO osvi;

    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (GetVersionEx(&osvi) &&
        (osvi.dwMajorVersion == 5) &&
        (osvi.dwMinorVersion == 0)) {
        
        return TRUE;
    }

    return FALSE;
}

//+-----------------------------------------------------------------------------
//
//  Function:   PostProcessInstallDevice
//
//  Purpose:    Handle DIF_INSTALLDEVICE PostProcessing
//
//  Arguments:
//      DeviceInfoSet     [in]
//      DeviceInfoData    [in]
//      Context           [in,out]
//
//  Returns:    NO_ERROR or an error code.
//
DWORD
PostProcessInstallDevice (
                    IN     HDEVINFO          DeviceInfoSet,
                    IN     PSP_DEVINFO_DATA  DeviceInfoData,
                    IN OUT PCOINSTALLER_CONTEXT_DATA Context
                    )
{
    DWORD                  Status = NO_ERROR;
    INFCONTEXT             InfContext;
    HINF                   InfFile = INVALID_HANDLE_VALUE;
    SP_DRVINFO_DATA        DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    PSP_INF_INFORMATION    InfInformation = NULL;
    DWORD                  ReqSize;
    SP_ORIGINAL_FILE_INFO  InfOriginalFileInformation;
    TCHAR                  InstallSectionName[LINE_LEN];
    TCHAR                  CombinedPath[MAX_PATH*2];
    TCHAR                  FullSourcePath[MAX_PATH];
    PTSTR                  FileNamePtr;
    INFCONTEXT             InfLineContext;
    DWORD                  FieldIndex;
    DWORD                  FieldCount;
    DWORD                  RetVal;
    

    //
    // Find name of INF and name of install section
    //
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if (!SetupDiGetSelectedDriver( DeviceInfoSet,
                                   DeviceInfoData,
                                   &DriverInfoData)) {
        Status = GetLastError();
        DbgOut("Fail: SetupDiGetSelectedDriver");
        goto clean;
    }


    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if (!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                    DeviceInfoData,
                                    &DriverInfoData,
                                    &DriverInfoDetailData,
                                    sizeof(SP_DRVINFO_DETAIL_DATA),
                                    NULL)) {
        Status = GetLastError();
        if (Status == ERROR_INSUFFICIENT_BUFFER) {
            //
            // We don't need the extended information.  Ignore.
            //
        } else {
            DbgOut("Fail: SetupDiGetDriverInfoDetail");
            goto clean;
        }
    }

    //
    // We have InfFileName, open the INF
    //
    InfFile = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                               NULL,
                               INF_STYLE_WIN4,
                               NULL);
    if (InfFile == INVALID_HANDLE_VALUE) {
        Status = GetLastError();
        DbgOut("Fail: SetupOpenInfFile");
        goto clean;
    }

    //
    // determine original filename to see if it has been
    // previously copied
    // this is long-winded, and is rarely an apropriate thing
    // to do. If we don't do it in this case, we'll try and
    // copy INF files that have already been copied and that
    // we might not have source for.
    //
    ReqSize = 0;
    if (!SetupGetInfInformation(InfFile,
                                INFINFO_INF_SPEC_IS_HINF,
                                NULL,
                                0,
                                &ReqSize)) {
        Status = GetLastError();
        DbgOut("Fail: SetupGetInfInformation");
        goto clean;
    }

    
    InfInformation = (PSP_INF_INFORMATION)malloc(ReqSize);
    if (InfInformation == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        DbgOut("Fail: SetupGetInfInformation - malloc");
        goto clean;
    }
    
    if (!SetupGetInfInformation(InfFile,
                                INFINFO_INF_SPEC_IS_HINF,
                                InfInformation,
                                ReqSize,
                                NULL)) {
        Status = GetLastError();
        DbgOut("Fail: SetupGetInfInformation - 2nd call");
        goto clean;
    }


    InfOriginalFileInformation.cbSize = sizeof(InfOriginalFileInformation);
    if(!SetupQueryInfOriginalFileInformation(InfInformation,
                                     0,
                                     NULL,
                                     &InfOriginalFileInformation)) {
        Status = GetLastError();
        DbgOut("Fail: SetupQueryInfOriginalFileInformation");
        goto clean;
    }


    //
    // determine location of current INF, which will be used
    // as root of inf's specified in CopyInf
    // but we also need FileNamePtr here to do an INF Name check.
    //

    RetVal = GetFullPathName(DriverInfoDetailData.InfFileName,
                             MAX_PATH,
                             CombinedPath,
                             &FileNamePtr);
    if (RetVal == 0 || RetVal > MAX_PATH) {
        Status = GetLastError();
        DbgOut("Fail: GetFullPathName");
        goto clean;
    }


    if (lstrcmpi(FileNamePtr,
                 InfOriginalFileInformation.OriginalInfName)!=0) {
        //
        // we have determined that this is a copy of original INF
        // so we must have already processed CopyINF= bits
        //
        Status = NO_ERROR;
        DbgOut("Skip processing of CopyINF (INF not at source location)");
        goto clean;
    }


    //
    // determine section used for the install which might be different
    // from the section name specified in DriverInfoDetailData.
    //
    if(!SetupDiGetActualSectionToInstall(InfFile,
                                         DriverInfoDetailData.SectionName,
                                         InstallSectionName,
                                         LINE_LEN,
                                         NULL,
                                         NULL)) {

        Status = GetLastError();
        DbgOut("Fail: SetupDiGetActualSectionToInstall");
        goto clean;
    }


    //
    // we look for keyword "CopyInf" in the install section
    //
    if (SetupFindFirstLine(InfFile,
                           InstallSectionName,
                           COPYINF_KEY,
                           &InfLineContext)) 
    {
        DbgOut("Found at least one CopyInf entry");
        do {
            //
            // CopyInf = a.inf,b.inf,c.inf...
            // a.inf will be at index 1.
            //
            FieldCount = SetupGetFieldCount(&InfLineContext);
            for(FieldIndex = 1;FieldIndex<=FieldCount;FieldIndex++) {
                if(SetupGetStringField(&InfLineContext,
                                       FieldIndex,
                                       FileNamePtr,
                                       MAX_PATH,
                                       NULL)) {
                    //
                    // we have a listed INF, obtain canonical pathname
                    // for this INF
                    // (note that we assume the INF is relative
                    //  to original INF and is typically just a filename)
                    //
                    RetVal = GetFullPathName(CombinedPath,
                                             MAX_PATH,
                                             FullSourcePath,
                                             NULL);
                    if (RetVal == 0 || RetVal > MAX_PATH) {
                        Status = GetLastError();
                        DbgOut("Fail: GetFullPathName");
                        //
                        // not fatal
                        //
                    } else if(!SetupCopyOEMInf(FullSourcePath,
                                    NULL,
                                    SPOST_PATH,
                                    0,
                                    NULL,
                                    0,
                                    NULL,
                                    NULL)) {
                        Status = GetLastError();
                        DbgOut("Fail: SetupCopyOEMInf");
                        //
                        // not fatal
                        //
                    }
                } else {
                    Status = GetLastError();
                    DbgOut("Fail: SetupGetStringField");
                    //
                    // not fatal
                    //
                }
            }

        } while (SetupFindNextMatchLine(&InfLineContext,
                                        COPYINF_KEY,
                                        &InfLineContext));
    } else {
        DbgOut("No CopyInf entry found");
    }


    Status = NO_ERROR;

clean:


    if (InfInformation) {
        free(InfInformation);
    }
    if (InfFile) {
        SetupCloseInfFile(InfFile);
    }
    //
    // since what we are doing does not affect primary device installation
    // always return success
    //
    return NO_ERROR;
}

//+-----------------------------------------------------------------------------
//
//  Function:   CoCopyINF
//
//  Purpose:    Responds to co-installer messages
//
//  Arguments:
//      InstallFunction   [in]
//      DeviceInfoSet     [in]
//      DeviceInfoData    [in]
//      Context           [in,out]
//
//  Returns:    NO_ERROR, ERROR_DI_POSTPROCESSING_REQUIRED, or an error code.
//
DWORD
__stdcall CoCopyINF (
               IN     DI_FUNCTION               InstallFunction,
               IN     HDEVINFO                  DeviceInfoSet,
               IN     PSP_DEVINFO_DATA          DeviceInfoData,
               IN OUT PCOINSTALLER_CONTEXT_DATA Context
               )
{
    DWORD Status = NO_ERROR;

    switch (InstallFunction)
    {
        case DIF_INSTALLDEVICE:
            //
            // We should not copy any INF files until the install has completed
            // like the primary INF, all secondary INF's must exist on each disk
            // of a multi-disk install.
            //

            if(!Context->PostProcessing)
            {
                DbgOut("DIF_INSTALLDEVICE");

                //
                // We only want the special processing on Windows 2000
                //
                if( IsWindows2000() )
                {
                    //
                    // indicate that we need to be called after the device has been
                    // installed.
                    //
                    DbgOut("Windows 2000, requiring post-processing");
                    Status = ERROR_DI_POSTPROCESSING_REQUIRED;
                }
                else
                {
                    Status = NO_ERROR;
                    DbgOut("Not on Windows 2000, no post-processing required");
                }
                
            }
            else // post processing
            {
                //
                // if driver installation failed, we're not interested
                // in processing CopyINF entries.
                //
                if (Context->InstallResult != NO_ERROR) {
                    DbgOut("DIF_INSTALLDEVICE PostProcessing on failure");
                    Status = Context->InstallResult;
                    break;
                }
                DbgOut("DIF_INSTALLDEVICE PostProcessing");

                Status = PostProcessInstallDevice(DeviceInfoSet,
                                                  DeviceInfoData,
                                                  Context);
            }
        break;

    default:
        break;
    }

    return Status;
}

