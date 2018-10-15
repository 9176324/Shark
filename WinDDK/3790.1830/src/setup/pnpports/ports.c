/** FILE: ports.c ********** Module Header ********************************
 *
 *  Class installer for the Ports class.
 *
 *
 *  Copyright (C) 1990-1999 Microsoft Corporation
 *
 *************************************************************************/
//==========================================================================
//                                Include files
//==========================================================================
// C Runtime
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Application specific
#include "ports.h"
#include <msports.h>


//==========================================================================
//                                Globals
//==========================================================================

HANDLE  g_hInst  = NULL;

TCHAR g_szClose[ 40 ];              //  "Close" string
TCHAR g_szErrMem[ 200 ];            //  Low memory message
TCHAR g_szPortsApplet[ 30 ];        //  "Ports Control Panel Applet" title
TCHAR g_szNull[]  = TEXT("");       //  Null string

TCHAR  m_szColon[]      = TEXT( ":" );
TCHAR  m_szComma[]      = TEXT( "," );
TCHAR  m_szCloseParen[] = TEXT( ")" );
TCHAR  m_szPorts[]      = TEXT( "Ports" );
TCHAR  m_szCOM[]        = TEXT( "COM" );
TCHAR  m_szSERIAL[]     = TEXT( "Serial" );
TCHAR  m_szLPT[]        = TEXT( "LPT" );

//
//  NT Registry keys to find COM port to Serial Device mapping
//
TCHAR m_szRegSerialMap[] = TEXT( "Hardware\\DeviceMap\\SerialComm" );
TCHAR m_szRegParallelMap[] = TEXT( "Hardware\\DeviceMap\\PARALLEL PORTS" );

//
//  Registry Serial Port Advanced I/O settings key and valuenames
//

TCHAR m_szRegServices[]  =
            TEXT( "System\\CurrentControlSet\\Services\\" );

TCHAR m_szRootEnumName[]         = REGSTR_KEY_ROOTENUM;
TCHAR m_szAcpiEnumName[]         = REGSTR_KEY_ACPIENUM;

TCHAR m_szFIFO[]                = TEXT( "ForceFifoEnable" );
TCHAR m_szDosDev[]              = TEXT( "DosDevices" );
TCHAR m_szPollingPeriod[]       = TEXT( "PollingPeriod" );
TCHAR m_szPortName[]            = REGSTR_VAL_PORTNAME;
TCHAR m_szDosDeviceName[]       = TEXT( "DosDeviceName" );
TCHAR m_szFirmwareIdentified[]  = TEXT( "FirmwareIdentified" );
TCHAR m_szPortSubClass[]         = REGSTR_VAL_PORTSUBCLASS;

int m_nBaudRates[] = { 75, 110, 134, 150, 300, 600, 1200, 1800, 2400,
                       4800, 7200, 9600, 14400, 19200, 38400, 57600,
                       115200, 128000, 0 };

TCHAR m_sz9600[] = TEXT( "9600" );

TCHAR m_szDefParams[] = TEXT( "9600,n,8,1" );

short m_nDataBits[] = { 4, 5, 6, 7, 8, 0};

TCHAR *m_pszParitySuf[] = { TEXT( ",e" ),
                            TEXT( ",o" ),
                            TEXT( ",n" ),
                            TEXT( ",m" ),
                            TEXT( ",s" ) };

TCHAR *m_pszLenSuf[] = { TEXT( ",4" ),
                         TEXT( ",5" ),
                         TEXT( ",6" ),
                         TEXT( ",7" ),
                         TEXT( ",8" ) };

TCHAR *m_pszStopSuf[] = { TEXT( ",1" ),
                          TEXT( ",1.5" ),
                          TEXT( ",2 " ) };

TCHAR *m_pszFlowSuf[] = { TEXT( ",x" ),
                          TEXT( ",p" ),
                          TEXT( " " ) };


#define IN_RANGE(value, minval, maxval) ((minval) <= (value) && (value) <= (maxval))


//==========================================================================
//                            Local Function Prototypes
//==========================================================================

DWORD
InstallPnPSerialPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );


DWORD
InstallPnPParallelPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

DWORD
InstallSerialOrParallelPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );


//==========================================================================
//                                Dll Entry Point
//==========================================================================
BOOL APIENTRY DllMain( HANDLE hDll, DWORD dwReason, LPVOID lpReserved )
{
    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hDll;
        DisableThreadLibraryCalls(hDll);
        InitStrings();

        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }

    return TRUE;
}


//==========================================================================
//                                Functions
//==========================================================================

DWORD
WINAPI
PortsClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
)
/*++

Routine Description:

    This routine acts as the class installer for Ports devices.

Arguments:

    InstallFunction - Specifies the device installer function code indicating
        the action being performed.

    DeviceInfoSet - Supplies a handle to the device information set being
        acted upon by this install action.

    DeviceInfoData - Optionally, supplies the address of a device information
        element being acted upon by this install action.

Return Value:

    If this function successfully completed the requested action, the return
        value is NO_ERROR.

    If the default behavior is to be performed for the requested action, the
        return value is ERROR_DI_DO_DEFAULT.

    If an error occurred while attempting to perform the requested action, a
        Win32 error code is returned.

--*/
{
    SP_INSTALLWIZARD_DATA   iwd;
    HKEY                hDeviceKey;
    HCOMDB              hComDB;
    DWORD               PortNameSize,
                        Err,
                        size;
    TCHAR               PortName[20];
    BOOL                result;

    switch(InstallFunction) {

        case DIF_INSTALLDEVICE :

            return InstallSerialOrParallelPort(DeviceInfoSet, DeviceInfoData);

        case DIF_REMOVE:

            if (PortTypeSerial == GetPortType(DeviceInfoSet, DeviceInfoData, FALSE)) {
                if (ComDBOpen(&hComDB) == ERROR_SUCCESS) {

                    hDeviceKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                                      DeviceInfoData,
                                                      DICS_FLAG_GLOBAL,
                                                      0,
                                                      DIREG_DEV,
                                                      KEY_READ);

                    if (hDeviceKey !=   INVALID_HANDLE_VALUE) {
                        PortNameSize = sizeof(PortName);
                        Err = RegQueryValueEx(hDeviceKey,
                                              m_szPortName,
                                              NULL,
                                              NULL,
                                              (PBYTE)PortName,
                                              &PortNameSize
                                             );
                        RegCloseKey(hDeviceKey);

                        if (Err == ERROR_SUCCESS) {
                            ComDBReleasePort(hComDB,
                                             myatoi(PortName+wcslen(m_szCOM)));
                        }
                    }

                    ComDBClose(hComDB);
                }
            }

            if (!SetupDiRemoveDevice(DeviceInfoSet, DeviceInfoData)) {
                return GetLastError();
            }

            return NO_ERROR;


        default :
            //
            // Just do the default action.
            //
            return ERROR_DI_DO_DEFAULT;
    }
}


DWORD
InstallSerialOrParallelPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine installs either a serial or a parallel port.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device being installed.

    DeviceInfoData - Supplies the address of the device information element
        being installed.

Return Value:

    If successful, the return value is NO_ERROR, otherwise it is a Win32 error code.

--*/
{
    switch (GetPortType(DeviceInfoSet, DeviceInfoData, TRUE)) {
    case PortTypeParallel:
        return InstallPnPParallelPort(DeviceInfoSet, DeviceInfoData);

    case PortTypeSerial:
        return InstallPnPSerialPort(DeviceInfoSet, DeviceInfoData);

    default:
        return ERROR_DI_DO_DEFAULT;
    }
}

PortType
GetPortType(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN          DoDrvKeyInstall
    )
/*++

Routine Description:

    This routine determines whether the driver node selected for the specified device
    is for a parallel (LPT or ECP) or serial (COM) port.  It knows which is which by
    running the AddReg entries in the driver node's install section, and then looking
    in the devnode's driver key for a 'PortSubClass' value entry.  If this value is
    present, and set to 0, then this is an LPT or ECP port, otherwise we treat it like
    a COM port.  This value was relied upon in Win9x, so it is the safest way for us
    to make this determination.

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device being installed.

    DeviceInfoData - Supplies the address of the device information element
        being installed.

Return Value:

    If the device is an LPT or ECP port, the return value is nonzero, otherwise it is
    FALSE.  (If anything goes wrong, the default is to return FALSE.)

--*/
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF hInf;
    HKEY hkDrv;
    TCHAR ActualInfSection[LINE_LEN];
    DWORD RegDataType;
    BYTE RegData;
    DWORD RegDataSize;
    PortType portType;
    ULONG err;

    portType = PortTypeSerial;
    hInf = INVALID_HANDLE_VALUE;
    hkDrv = 0;
    RegData = 0;

    do {
        //
        // Open up the driver key for this device so we can run our INF registry mods
        // against it.
        //
        hkDrv = SetupDiCreateDevRegKey(DeviceInfoSet,
                                       DeviceInfoData,
                                       DICS_FLAG_GLOBAL,
                                       0,
                                       DIREG_DRV,
                                       NULL,
                                       NULL);

        if (hkDrv == 0) {
            break;
        }

        if (DoDrvKeyInstall) {
            //
            // Retrieve information about the driver node selected for this
            // device.
            //
            DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
            if (!SetupDiGetSelectedDriver(DeviceInfoSet,
                                          DeviceInfoData,
                                          &DriverInfoData)) {
                break;
            }

            DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
            if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                           DeviceInfoData,
                                           &DriverInfoData,
                                           &DriverInfoDetailData,
                                           sizeof(DriverInfoDetailData),
                                           NULL)
               && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
                //
                // For some reason we couldn't get detail data--this should
                // never happen.
                //
                break;
            }

            //
            // Open the INF that installs this driver node, so we can 'pre-run'
            // the AddReg entries in its install section.
            //
            hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                                    NULL,
                                    INF_STYLE_WIN4,
                                    NULL);

            if (hInf == INVALID_HANDLE_VALUE) {
                //
                // For some reason we couldn't open the INF--this should never
                // happen.
                //
                break;
            }

            //
            // Now find the actual (potentially OS/platform-specific) install
            // section name.
            //
            SetupDiGetActualSectionToInstall(
                hInf,
                DriverInfoDetailData.SectionName,
                ActualInfSection,
                sizeof(ActualInfSection) / sizeof(TCHAR),
                NULL,
                NULL);

            //
            // Now run the registry modification (AddReg/DelReg) entries in
            // this section...
            //
            SetupInstallFromInfSection(
                NULL,    // no UI, so don't need to specify window handle
                hInf,
                ActualInfSection,
                SPINST_REGISTRY,
                hkDrv,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL);
        }

        //
        // Check for a REG_BINARY (1 byte) 'PortSubClassOther' value entry first
        //
        RegDataSize = sizeof(RegData);
        err = RegQueryValueEx(hkDrv,
                              TEXT("PortSubClassOther"),
                              NULL,
                              &RegDataType,
                              &RegData,
                              &RegDataSize);

        if (err == ERROR_SUCCESS && RegDataSize == sizeof(BYTE) &&
            RegDataType == REG_BINARY && RegData != 0) {
            portType = PortTypeOther;
            break;
        }

        //
        // Check for a REG_BINARY (1-byte) 'PortSubClass' value entry set to 0.
        //
        RegDataSize = sizeof(RegData);
        if((ERROR_SUCCESS != RegQueryValueEx(hkDrv,
                                             m_szPortSubClass,
                                             NULL,
                                             &RegDataType,
                                             &RegData,
                                             &RegDataSize))
           || (RegDataSize != sizeof(BYTE))
           || (RegDataType != REG_BINARY))
        {
            portType = PortTypeSerial; // not a LPT/ECP device.
        }
        else {
            if (RegData == 0) {
                portType = PortTypeParallel;
            }
            else {
                portType = PortTypeSerial;
            }
        }
    } while (FALSE);

    if (hkDrv != 0) {
        RegCloseKey(hkDrv);
        hkDrv = 0;
    }

    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
        hInf = INVALID_HANDLE_VALUE;
    }

    return portType;
}


DWORD
InstallPnPParallelPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
/*++


Routine Description:

    This routine installs a parallel port. In the DDK implementation, we let the
    default setup installer run and do nothing special.

--*/
{

    DWORD           err = ERROR_DI_DO_DEFAULT;


    //
    // Let the default setup installer install parallel ports for the DDK
    // version of this class installer
    //
    return err;
}


#define NO_COM_NUMBER 0

BOOL
DetermineComNumberFromResources(
    IN  DEVINST            DevInst,
    OUT PDWORD             Num
    )
/*++

Routine Description:

    This routine retrieves the base IO port and IRQ for the specified device instance
    in a particular logconfig.

    If a successful match is found, then *Num == found number, otherwise
    *Num == NO_COM_NUMBER.

Arguments:

    DevInst - Supplies the handle of a device instance to retrieve configuration for.

Return Value:

    If success, the return value is TRUE, otherwise it is FALSE.

--*/
{
    LOG_CONF    logConfig;
    RES_DES     resDes;
    CONFIGRET   cr;
    BOOL        success;
    IO_RESOURCE ioResource;
    WORD        base;
    ULONGLONG base2;

    success = FALSE;    // assume failure.
    *Num = NO_COM_NUMBER;

    //
    // If the device does not have a boot config, use the com db
    //
    if (CM_Get_First_Log_Conf(&logConfig,
                              DevInst,
                              BOOT_LOG_CONF) != CR_SUCCESS) {
        return success;
    }

    //
    // First, get the Io base port
    //
    if (CM_Get_Next_Res_Des(&resDes,
                            logConfig,
                            ResType_IO,
                            NULL,
                            0) != CR_SUCCESS) {
        goto clean0;
    }

    cr = CM_Get_Res_Des_Data(resDes,
                             &ioResource,
                             sizeof(IO_RESOURCE),
                             0);

    CM_Free_Res_Des_Handle(resDes);

    if (cr != CR_SUCCESS) {
        goto clean0;
    }

    //
    // Values for resources from ISA Architecture
    //
    base = (WORD) ioResource.IO_Header.IOD_Alloc_Base;
    if (IN_RANGE(base, 0x3f8, 0x3ff)) {
        *Num = 1;
    }
    else if (IN_RANGE(base, 0x2f8, 0x2ff)) {
        *Num = 2;
    }
    else if (IN_RANGE(base, 0x3e8, 0x3ef)) {
        *Num = 3;
    }
    else if (IN_RANGE(base, 0x2e8, 0x2ef)) {
        *Num = 4;
    }

    if (*Num != NO_COM_NUMBER) {
        success = TRUE;
    }

clean0:
    CM_Free_Log_Conf_Handle(logConfig);

    return success;
}

#define DEF_MIN_COM_NUM (5)

DWORD
InstallPnPSerialPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
/*++

Routine Description:

    This routine performs the installation of a PnP ISA serial port device (may
    actually be a modem card).  This involves the following steps:

        1.  Select a COM port number and serial device name for this port
            (This involves duplicate detection, since PnP ISA cards will
            sometimes have a boot config, and thus be reported by ntdetect/ARC
            firmware.)
        2.  Create a subkey under the serial driver's Parameters key, and
            set it up just as if it was a manually-installed port.
        3.  Display the resource selection dialog, and allow the user to
            configure the settings for the port.
        4.  Write out the settings to the serial port's key in legacy format
            (i.e., the way serial.sys expects to see it).
        5.  Write out PnPDeviceId value to the serial port's key, which gives
            the device instance name with which this port is associated.
        6.  Write out PortName value to the devnode key, so that modem class
            installer can continue with installation (if this is really a
            PnP ISA modem).

Arguments:

    DeviceInfoSet - Supplies a handle to the device information set containing
        the device being installed.

    DeviceInfoData - Supplies the address of the device information element
        being installed.

Return Value:

    If successful, the return value is NO_ERROR, otherwise it is a Win32 error code.

--*/
{
    HKEY        hKey;
    HCOMDB      hComDB;
    TCHAR       comPort[40],
                szPortName[20],
                charBuffer[MAX_PATH],
                friendlyNameFormat[LINE_LEN],
                deviceDesc[LINE_LEN];
    PTCHAR      comLocation;
    DWORD       comPortSize1,comPortSize2,
                comPortNumber = NO_COM_NUMBER,
                portsReported;
    DWORD       dwFirmwareIdentified, dwSize;
    BYTE        portUsage[32];
    BOOL        res;
    DWORD       firmwarePort = FALSE;

#if MAX_DEVICE_ID_LEN > MAX_PATH
#error MAX_DEVICE_ID_LEN is greater than MAX_PATH.  Update charBuffer.
#endif

    ZeroMemory(comPort, sizeof(comPort));

    ComDBOpen(&hComDB);

    if ((hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                     DeviceInfoData,
                                     DICS_FLAG_GLOBAL,
                                     0,
                                     DIREG_DEV,
                                     KEY_READ)) != INVALID_HANDLE_VALUE) {

        comPortSize1 = sizeof(comPort);
        comPortSize2 = sizeof(comPort);
        if (RegQueryValueEx(hKey,
                            m_szPortName,
                            NULL,
                            NULL,
                            (PBYTE)comPort,
                            &comPortSize1) == ERROR_SUCCESS) {
            firmwarePort = TRUE;
        }
        else if (RegQueryValueEx(hKey,
                                 m_szDosDeviceName,
                                 NULL,
                                 NULL,
                                 (PBYTE) comPort,
                                 &comPortSize2) == ERROR_SUCCESS) {
            //
            // ACPI puts the name of the port as DosDeviceName, use this name
            // as the basis for what to call this port
            //
            firmwarePort = TRUE;
        }
        else {
            //
            // Our final check is to check the enumerator.  We care about two
            // cases:
            //
            // 1)  If the enumerators is ACPI.  If so, blindly consider this
            //     a firmware port (and get the BIOS mfg to provide a _DDN method
            //     for this device!)
            //
            // 2)  The port is "root" enumerated, yet it's not marked as
            // DN_ROOT_ENUMERATED.  This is the
            // way we distinguish PnPBIOS-reported devnodes.  Note that, in
            // general, these devnodes would've been caught by the check for a
            // "PortName" value above, but this won't be present if we couldn't
            // find a matching ntdetect-reported device from which to migrate
            // the COM port name.
            //
            // Note also that this check doesn't catch ntdetect or firmware
            // reported devices.  In these cases, we should already have a
            // PortName, thus the check above should catch those devices.  In
            // the unlikely event that we encounter an ntdetect or firmware
            // devnode that doesn't already have a COM port name, then it'll
            // get an arbitrary one assigned.  Oh well.
            //
            if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_ENUMERATOR_NAME,
                                                 NULL,
                                                 (PBYTE)charBuffer,
                                                 sizeof(charBuffer),
                                                 NULL)) {
                if (lstrcmpi(charBuffer, m_szAcpiEnumName) == 0) {
                    firmwarePort = TRUE;
                }
                else if (lstrcmpi(charBuffer, m_szRootEnumName) == 0) {
                    ULONG status, problem;

                    if ((CM_Get_DevNode_Status(&status,
                                               &problem,
                                               (DEVNODE)(DeviceInfoData->DevInst),
                                               0) == CR_SUCCESS)
                        && !(status & DN_ROOT_ENUMERATED))
                    {
                        firmwarePort = TRUE;
                    }
                }
            }

            dwSize = sizeof(dwFirmwareIdentified);
            if (firmwarePort == FALSE &&
                RegQueryValueEx(hKey,
                                m_szFirmwareIdentified,
                                NULL,
                                NULL,
                                (PBYTE) &dwFirmwareIdentified,
                                &dwSize) == ERROR_SUCCESS) {

                //
                // ACPI puts the value "FirmwareIdentified" if it has enumerated
                // this port.  We only rely on this if a DDN isn't present and we
                // couldn't get the enumerator name
                //
                firmwarePort = TRUE;
            }

            ZeroMemory(charBuffer, sizeof(charBuffer));
        }

        RegCloseKey(hKey);
    }

    if (firmwarePort) {
        //
        // Try to find "COM" in the name.  If it is found, simply extract
        // the number that follows it and use that as the com number.
        //
        // Otherwise:
        // 1) try to determine the number of the com port based on its
        //    IO range, otherwise
        // 2) look through the com db and try to find an unused port from
        //    1 to 4, if none are present then let the DB pick the next open
        //    port number
        //
        if (comPort[0] != (TCHAR) 0) {
            _wcsupr(comPort);
            comLocation = wcsstr(comPort, m_szCOM);
            if (comLocation) {
                comPortNumber = myatoi(comLocation + wcslen(m_szCOM));
            }
        }

        if (comPortNumber == NO_COM_NUMBER &&
            !DetermineComNumberFromResources((DEVINST) DeviceInfoData->DevInst,
                                             &comPortNumber) &&
            (hComDB != HCOMDB_INVALID_HANDLE_VALUE) &&
            (ComDBGetCurrentPortUsage(hComDB,
                                      portUsage,
                                      MAX_COM_PORT / 8,
                                      CDB_REPORT_BITS,
                                      &portsReported) == ERROR_SUCCESS)) {
            if (!(portUsage[0] & 0x1)) {
                comPortNumber = 1;
            }
            else if (!(portUsage[0] & 0x2)) {
                comPortNumber = 2;
            }
            else if (!(portUsage[0] & 0x4)) {
                comPortNumber = 3;
            }
            else if (!(portUsage[0] & 0x8)) {
                comPortNumber = 4;
            }
            else {
                comPortNumber = NO_COM_NUMBER;
            }
        }
    }

    if (comPortNumber == NO_COM_NUMBER) {
        if (hComDB == HCOMDB_INVALID_HANDLE_VALUE) {
            //
            // Couldn't open the DB, pick a com port number that doesn't conflict
            // with any firmware ports
            //
            comPortNumber = DEF_MIN_COM_NUM;
        }
        else {
            //
            // Let the db find the next number
            //
            ComDBClaimNextFreePort(hComDB,
                                   &comPortNumber);
        }
    }
    else {
        //
        // We have been told what number to use, claim it irregardless of what
        // has already been claimed
        //
        ComDBClaimPort(hComDB,
                       comPortNumber,
                       TRUE,
                       NULL);
    }

    if (hComDB != HCOMDB_INVALID_HANDLE_VALUE) {
        ComDBClose(hComDB);
    }

    //
    // Generate the serial and COM port names based on the numbers we picked.
    //
    wsprintf(szPortName, TEXT("%s%d"), m_szCOM, comPortNumber);

    //
    // Write out Device Parameters\PortName and PollingPeriod
    //
    if((hKey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                      DeviceInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      DIREG_DEV,
                                      NULL,
                                      NULL)) != INVALID_HANDLE_VALUE) {
        DWORD PollingPeriod = PollingPeriods[POLL_PERIOD_DEFAULT_IDX];

        //
        // A failure is not catastrophic, serial will just not know what to call
        // the port
        //
        RegSetValueEx(hKey,
                      m_szPortName,
                      0,
                      REG_SZ,
                      (PBYTE) szPortName,
                      ByteCountOf(lstrlen(szPortName) + 1)
                      );

        RegSetValueEx(hKey,
                      m_szPollingPeriod,
                      0,
                      REG_DWORD,
                      (PBYTE) &PollingPeriod,
                      sizeof(DWORD)
                      );

        RegCloseKey(hKey);
    }

    //
    // Now do the installation for this device.
    //
    if(!SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData)) {
        return GetLastError();
    }

    //
    // Write out the friendly name based on the device desc
    //
    if (LoadString(g_hInst,
                   IDS_FRIENDLY_FORMAT,
                   friendlyNameFormat,
                   CharSizeOf(friendlyNameFormat)) &&
        SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_DEVICEDESC,
                                         NULL,
                                         (PBYTE)deviceDesc,
                                         sizeof(deviceDesc),
                                         NULL)) {
        wsprintf(charBuffer, friendlyNameFormat, deviceDesc, szPortName);
    }
    else {
        lstrcpy(charBuffer, szPortName);
    }

    // Write the string friendly name string out
    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_FRIENDLYNAME,
                                     (PBYTE)charBuffer,
                                     ByteCountOf(lstrlen(charBuffer) + 1)
                                     );

    //
    // Write out the default settings to win.ini (really a registry key) if they
    // don't already exist.
    //
    wcscat(szPortName, m_szColon);
    charBuffer[0] = TEXT('\0');
    GetProfileString(m_szPorts,
                     szPortName,
                     TEXT(""),
                     charBuffer,
                     sizeof(charBuffer) / sizeof(TCHAR) );
    //
    // Check to see if the default string provided was copied in, if so, write
    // out the port defaults
    //
    if (charBuffer[0] == TEXT('\0')) {
        WriteProfileString(m_szPorts, szPortName, m_szDefParams);
    }

    return NO_ERROR;
}


void InitStrings(void)
{
    DWORD  dwClass, dwShare;
    TCHAR  szClass[ 40 ];

    LoadString(g_hInst,
               INITS,
               g_szErrMem,
               CharSizeOf(g_szErrMem));
    LoadString(g_hInst,
               IDS_INIT_NAME,
               g_szPortsApplet,
               CharSizeOf(g_szPortsApplet));

    //
    //  Get the "Close" string
    //
    LoadString(g_hInst,
               IDS_INIT_CLOSE,
               g_szClose,
               CharSizeOf(g_szClose));
}

