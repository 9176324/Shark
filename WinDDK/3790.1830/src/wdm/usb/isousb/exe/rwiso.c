/*++

Copyright (c) 1997-2004  Microsoft Corporation

Module Name:

    RwIso.c

Abstract:

    Console test app for IsoUsb.sys driver

Environment:

    user mode only

Notes:

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) 1997-2004 Microsoft Corporation.  All Rights Reserved.


Revision History:

    11/17/97: created

--*/

#include <windows.h>

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "devioctl.h"

#include <setupapi.h>
#include <basetyps.h>

#include "IsoUsr.h"

#include "usbdi.h"

#define NOISY(_x_) printf _x_ ;

char inPipe[32]  = "PIPE04";    // pipe name for iso input pipe on our test board
char outPipe[32] = "PIPE05";    // pipe name for iso output pipe on our test board
char completeDeviceName[256] = "";  //generated from the GUID registered by the driver itself

BOOL fDumpUsbConfig = FALSE;    // flags set in response to console command line switches
BOOL fDumpReadData  = FALSE;
BOOL fRead          = FALSE;
BOOL fWrite         = FALSE;

PVOID gpStreamObj;
char gbuf[256];


BOOL    SelectAltInterface = FALSE;
UCHAR   AltInterface;
UCHAR   AltSetting;


BOOL fIsoStreamStarted = FALSE;
HANDLE ghStreamDev = NULL;
int gMS = 10000; // default to 10 secs stream test

int gDebugLevel = 1;      // higher == more verbose, default is 1, 0 turns off all

ULONG IterationCount = 1; // count of iterations of the test we are to perform
int WriteLen = 0;         // #bytes to write
int ReadLen = 0;          // #bytes to read

void StartIsoStream( void );

void StopIsoStream( void );

void Usage();

BOOL Parse ( int argc, char *argv[] );

// functions


HANDLE
OpenOneDevice (
    IN HDEVINFO                     HardwareDeviceInfo,
    IN PSP_DEVICE_INTERFACE_DATA    DeviceInfoData,
    IN PCHAR                        devName
    )
/*++
Routine Description:

    Given the HardwareDeviceInfo, representing a handle to the plug and
    play information, and deviceInfoData, representing a specific usb device,
    open that device and fill in all the relevant information in the given
    USB_DEVICE_DESCRIPTOR structure.

Arguments:

    HardwareDeviceInfo:  handle to info obtained from Pnp mgr via SetupDiGetClassDevs()
    DeviceInfoData:      ptr to info obtained via SetupDiEnumDeviceInterfaces()

Return Value:

    return HANDLE if the open and initialization was successfull,
    else INVLAID_HANDLE_VALUE.

--*/
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA    functionClassDeviceData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0;
    HANDLE                              hOut = INVALID_HANDLE_VALUE;

    //
    // allocate a function class device data structure to receive the
    // goods about this particular device.
    //
    SetupDiGetDeviceInterfaceDetail(
        HardwareDeviceInfo,
        DeviceInfoData,
        NULL,   // probing so no output buffer yet
        0,      // probing so output buffer length of zero
        &requiredLength,
        NULL);  // not interested in the specific dev-node


    predictedLength = requiredLength;
    // sizeof (SP_FNCLASS_DEVICE_DATA) + 512;

    functionClassDeviceData = malloc (predictedLength);

    if (NULL == functionClassDeviceData)
    {
        return INVALID_HANDLE_VALUE;
    }

    functionClassDeviceData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    //
    // Retrieve the information from Plug and Play.
    //
    if (!SetupDiGetDeviceInterfaceDetail(HardwareDeviceInfo,
                                         DeviceInfoData,
                                         functionClassDeviceData,
                                         predictedLength,
                                         &requiredLength,
                                         NULL))
    {
        free(functionClassDeviceData);

        return INVALID_HANDLE_VALUE;
    }

    strcpy(devName, functionClassDeviceData->DevicePath);

    printf("Attempting to open %s\n", devName);

    hOut = CreateFile(
               functionClassDeviceData->DevicePath,
               GENERIC_READ | GENERIC_WRITE,
               FILE_SHARE_READ | FILE_SHARE_WRITE,
               NULL,            // No SECURITY_ATTRIBUTES structure
               OPEN_EXISTING,   // No special create flags
               0,               // No special attributes
               NULL);           // No template file

    if (INVALID_HANDLE_VALUE == hOut)
    {
        printf("FAILED to open %s\n", devName);
    }

    free(functionClassDeviceData);

    return hOut;
}


HANDLE
OpenUsbDevice( LPGUID  pGuid, char *outNameBuf)
/*++
Routine Description:

    Do the required PnP things in order to find the next available
    proper device in the system at this time.

Arguments:

    pGuid:      ptr to GUID registered by the driver itself
    outNameBuf: the generated name for this device

Return Value:

    return HANDLE if the open and initialization was successful,
    else INVLAID_HANDLE_VALUE.
--*/
{
   ULONG                    NumberDevices;
   HANDLE                   hOut = INVALID_HANDLE_VALUE;
   HDEVINFO                 hardwareDeviceInfo;
   SP_DEVICE_INTERFACE_DATA deviceInfoData;
   ULONG                    i;
   BOOLEAN                  done;
   PUSB_DEVICE_DESCRIPTOR   usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR   *UsbDevices = &usbDeviceInst;
   PUSB_DEVICE_DESCRIPTOR   tempDevDesc;

   *UsbDevices = NULL;
   tempDevDesc = NULL;
   NumberDevices = 0;

   //
   // Open a handle to the plug and play dev node.
   // SetupDiGetClassDevs() returns a device information set that contains info on all
   // installed devices of a specified class.
   //
   hardwareDeviceInfo = SetupDiGetClassDevs(
                            pGuid,
                            NULL,   // Define no enumerator (global)
                            NULL,   // Define no
                            (DIGCF_PRESENT | // Only Devices present
                             DIGCF_DEVICEINTERFACE)); // Function class devices.

   //
   // Take a wild guess at the number of devices we have;
   // Be prepared to realloc and retry if there are more than we guessed
   //
   NumberDevices = 4;
   done = FALSE;
   deviceInfoData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

   i=0;

   while (!done)
   {
       NumberDevices *= 2;

       if (*UsbDevices)
       {
           tempDevDesc = realloc(*UsbDevices,
                                 (NumberDevices * sizeof (USB_DEVICE_DESCRIPTOR)));

            if (tempDevDesc)
            {
                *UsbDevices = tempDevDesc;
                tempDevDesc = NULL;
            }
            else
            {
                free(*UsbDevices);
                *UsbDevices = NULL;
            }
       }
       else
       {
           *UsbDevices = calloc(NumberDevices,
                                sizeof (USB_DEVICE_DESCRIPTOR));
       }

       if (NULL == *UsbDevices)
       {
           // SetupDiDestroyDeviceInfoList destroys a device information set
           // and frees all associated memory.
           //
           SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);

           return INVALID_HANDLE_VALUE;
       }

       usbDeviceInst = *UsbDevices + i;

       for (; i < NumberDevices; i++)
       {
           // SetupDiEnumDeviceInterfaces() returns information about
           // device interfaces exposed by one or more devices. Each call
           // returns information about one interface; the routine can be
           // called repeatedly to get information about several
           // interfaces exposed by one or more devices.
           //
           if (SetupDiEnumDeviceInterfaces(hardwareDeviceInfo,
                                           0, // We don't care about specific PDOs
                                           pGuid,
                                           i,
                                           &deviceInfoData))
           {
               hOut = OpenOneDevice(hardwareDeviceInfo,
                                    &deviceInfoData,
                                    outNameBuf);

               if (hOut != INVALID_HANDLE_VALUE)
               {
                   done = TRUE;
                   break;
               }
           }
           else
           {
               if (ERROR_NO_MORE_ITEMS == GetLastError())
               {
                   done = TRUE;
                   break;
               }
           }
       }
   }

   NumberDevices = i;

   // SetupDiDestroyDeviceInfoList() destroys a device information set
   // and frees all associated memory.
   //
   SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);

   free (*UsbDevices);

   return hOut;
}




BOOL
GetUsbDeviceFileName( LPGUID  pGuid, char *outNameBuf)
/*++
Routine Description:

    Given a ptr to a driver-registered GUID, give us a string with the
    device name that can be used in a CreateFile() call.

    Actually briefly opens and closes the device and sets outBuf if
    successfull; returns FALSE if not

Arguments:

    pGuid:      ptr to GUID registered by the driver itself
    outNameBuf: the generated zero-terminated name for this device

Return Value:

    TRUE on success else FALSE

--*/
{
    HANDLE  hDev;

    hDev = OpenUsbDevice(pGuid, outNameBuf);

    if (hDev != INVALID_HANDLE_VALUE)
    {
        CloseHandle( hDev );

        return TRUE;
    }

    return FALSE;
}

HANDLE
open_dev()
/*++
Routine Description:

    Called by dumpUsbConfig() to open an instance of our device

Arguments:

    None

Return Value:

    Device handle on success else NULL

--*/
{
    HANDLE  hDEV;

    hDEV = OpenUsbDevice((LPGUID)&GUID_CLASS_I82930_ISO,
                         completeDeviceName);

    if (hDEV == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open (%s) = %d", completeDeviceName, GetLastError());
    }
    else
    {
        printf("DeviceName = (%s)\n", completeDeviceName);
    }

    return hDEV;
}


HANDLE
open_file( char *filename)
/*++
Routine Description:

    Called by main() to open an instance of our device after obtaining its name

Arguments:

    None

Return Value:

    Device handle on success else NULL

--*/
{
    int     success = 1;
    HANDLE  h;

    if (!GetUsbDeviceFileName((LPGUID)&GUID_CLASS_I82930_ISO,
                              completeDeviceName))
    {
        NOISY(("Failed to GetUsbDeviceFileName - err = %d\n", GetLastError()));

        return INVALID_HANDLE_VALUE;
    }

    strcat(completeDeviceName, "\\");

    if ((strlen(completeDeviceName) + strlen(filename)) > 255)
    {
        NOISY(("Failed to open handle - possibly long filename\n"));

        return INVALID_HANDLE_VALUE;
    }

    strcat(completeDeviceName, filename);

    printf("completeDeviceName = (%s)\n", completeDeviceName);

    h = CreateFile(completeDeviceName,
                   GENERIC_WRITE | GENERIC_READ,
                   FILE_SHARE_WRITE | FILE_SHARE_READ,
                   NULL,
                   OPEN_EXISTING,
                   0,
                   NULL);

    if (h == INVALID_HANDLE_VALUE)
    {
        NOISY(("Failed to open (%s) = %d", completeDeviceName, GetLastError()));

        success = 0;
    }
    else
    {
        NOISY(("Opened successfully.\n"));
    }

    return h;
}

BOOL
compare_buffs(char *buff1,
              char *buff2,
              int length)
/*++
Routine Description:

    Called to verify read and write buffers match for loopback test

Arguments:

    buffers to compare and length

Return Value:

    TRUE if buffers match, else FALSE

--*/
{
    int ok = 1;

    if (memcmp(buff1, buff2, length))
    {
        ok = 0;
    }

    return ok;
}

#define NPERLN 8

void
dump(
   UCHAR *b,
   int len
)
/*++
Routine Description:

    Called to do formatted ascii dump to console of the io buffer

Arguments:

    buffer and length

Return Value:

    none

--*/
{
    ULONG   i;
    ULONG   longLen = (ULONG)len / sizeof(ULONG);
    PULONG  pBuf = (PULONG) b;

    // dump an ordinal ULONG for each sizeof(ULONG)'th byte
    printf("\n****** BEGIN DUMP LEN decimal %d, 0x%x\n", len,len);
    for (i=0; i<longLen; i++)
    {
        printf("%04X ", *pBuf++);
        if (i % NPERLN == (NPERLN - 1))
        {
            printf("\n");
        }
    }
    if (i % NPERLN != 0)
    {
        printf("\n");
    }
    printf("\n****** END DUMP LEN decimal %d, 0x%x\n", len,len);
}

//  Begin, routines for USB configuration dump (Cmdline "rwiso -u" )
//

VOID
ShowEndpointDescriptor (
    PUSB_ENDPOINT_DESCRIPTOR    EndpointDesc
)
/*++
Routine Description:

    Called to do formatted ascii dump to console of a USB endpoint descriptor

Arguments:

    ptr to USB endpoint descriptor,

Return Value:

    none

--*/
{
    printf("--------------------\n");
    printf("Endpoint Descriptor:\n");

    if (USB_ENDPOINT_DIRECTION_IN(EndpointDesc->bEndpointAddress))
    {
        printf("bEndpointAddress:     0x%02X  IN\n",
               EndpointDesc->bEndpointAddress);
    }
    else
    {
        printf("bEndpointAddress:     0x%02X  OUT\n",
               EndpointDesc->bEndpointAddress);
    }

    switch (EndpointDesc->bmAttributes & 0x03)
    {
        case 0x00:
            printf("Transfer Type:     Control\n");
            break;

        case 0x01:
            printf("Transfer Type: Isochronous\n");
            break;

        case 0x02:
            printf("Transfer Type:        Bulk\n");
            break;

        case 0x03:
            printf("Transfer Type:   Interrupt\n");
            break;
    }

    printf("wMaxPacketSize:     0x%04X (%d)\n",
           EndpointDesc->wMaxPacketSize,
           EndpointDesc->wMaxPacketSize);

    printf("bInterval:            0x%02X\n",
           EndpointDesc->bInterval);
}


VOID
ShowInterfaceDescriptor (
    PUSB_INTERFACE_DESCRIPTOR   InterfaceDesc
)
/*++
Routine Description:

    Called to do formatted ascii dump to console of a USB interface descriptor

Arguments:

    ptr to USB interface descriptor

Return Value:

    none

--*/
{
    printf("\n---------------------\n");
    printf("Interface Descriptor:\n");

    printf("bInterfaceNumber:     0x%02X\n",
           InterfaceDesc->bInterfaceNumber);

    printf("bAlternateSetting:    0x%02X\n",
           InterfaceDesc->bAlternateSetting);

    printf("bNumEndpoints:        0x%02X\n",
           InterfaceDesc->bNumEndpoints);

    printf("bInterfaceClass:      0x%02X\n",
           InterfaceDesc->bInterfaceClass);

    printf("bInterfaceSubClass:   0x%02X\n",
           InterfaceDesc->bInterfaceSubClass);

    printf("bInterfaceProtocol:   0x%02X\n",
           InterfaceDesc->bInterfaceProtocol);

    printf("iInterface:           0x%02X\n",
           InterfaceDesc->iInterface);

}


VOID
ShowConfigurationDescriptor (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
/*++
Routine Description:

    Called to do formatted ascii dump to console of a USB config descriptor

Arguments:

    ptr to USB configuration descriptor

Return Value:

    none

--*/
{
    printf("=========================\n");
    printf("Configuration Descriptor:\n");

    printf("wTotalLength:       0x%04X\n",
           ConfigDesc->wTotalLength);

    printf("bNumInterfaces:       0x%02X\n",
           ConfigDesc->bNumInterfaces);

    printf("bConfigurationValue:  0x%02X\n",
           ConfigDesc->bConfigurationValue);

    printf("iConfiguration:       0x%02X\n",
           ConfigDesc->iConfiguration);

    printf("bmAttributes:         0x%02X\n",
           ConfigDesc->bmAttributes);

    if (ConfigDesc->bmAttributes & 0x80)
    {
        printf("  Bus Powered\n");
    }

    if (ConfigDesc->bmAttributes & 0x40)
    {
        printf("  Self Powered\n");
    }

    if (ConfigDesc->bmAttributes & 0x20)
    {
        printf("  Remote Wakeup\n");
    }

    printf("MaxPower:             0x%02X (%d Ma)\n",
           ConfigDesc->MaxPower,
           ConfigDesc->MaxPower * 2);

}


VOID
ShowConfigDesc (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
{
    PUCHAR                  descEnd;
    PUSB_COMMON_DESCRIPTOR  commonDesc;
    BOOLEAN                 ShowUnknown;

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
           (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        ShowUnknown = FALSE;

        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    ShowUnknown = TRUE;
                    break;
                }
                ShowConfigurationDescriptor((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc);
                break;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR))
                {
                    ShowUnknown = TRUE;
                    break;
                }
                ShowInterfaceDescriptor((PUSB_INTERFACE_DESCRIPTOR)commonDesc);
                break;

            case USB_ENDPOINT_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR))
                {
                    ShowUnknown = TRUE;
                    break;
                }
                ShowEndpointDescriptor((PUSB_ENDPOINT_DESCRIPTOR)commonDesc);
                break;

            default:
                ShowUnknown = TRUE;
                break;
        }

        if (ShowUnknown)
        {
            // ShowUnknownDescriptor(commonDesc);
        }

        (PUCHAR)commonDesc += commonDesc->bLength;
    }
}


void dumpUsbConfig()
/*++
Routine Description:

    Called to do formatted ascii dump to console of  USB
    configuration, interface, and endpoint descriptors
    (Cmdline "rwiso -u" )

Arguments:

    none

Return Value:

    none

--*/
{
    HANDLE  hDEV;
    ULONG   success;
    int     siz;
    int     nBytes;
    char    buf[4096];

    PUSB_CONFIGURATION_DESCRIPTOR configDesc;

    hDEV = open_dev();

    if (hDEV && hDEV != INVALID_HANDLE_VALUE)
    {
        siz = sizeof(buf);

        nBytes = 0;

        success = DeviceIoControl(hDEV,
                                  IOCTL_ISOUSB_GET_CONFIG_DESCRIPTOR,
                                  buf,
                                  siz,
                                  buf,
                                  siz,
                                  &nBytes,
                                  NULL);

        NOISY(("request complete, success = %d nBytes = %d\n", success, nBytes));

        configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)buf;

        if (success && configDesc->wTotalLength == nBytes)
        {
            ShowConfigDesc(configDesc);
        }

        CloseHandle(hDEV);
    }
}

//
//  End, routines for USB configuration and pipe info dump  (Cmdline "rwiso -u" )




//  Begin, routines for Iso Streaming
//

void
IsoStream( HANDLE hDEV, BOOL fStop )
/*++
Routine Description:

    Called to start or stop an iso stream
    (Cmdline "RwIso -g" )

Arguments:

    handle to device

Return Value:

    none

--*/
{
    ULONG success;
    int nBytes;
    DWORD ioctl;
    char i;

    if (fStop)
    {
        ioctl = IOCTL_ISOUSB_STOP_ISO_STREAM;

        for (i = 0; i < sizeof(gbuf); i++)
        {
            gbuf[ i ] = 0; // init outbuf to 0's to make sure read was good
        }

        success = DeviceIoControl(hDEV,
                                  ioctl,
                                  &gpStreamObj, //pointer to stream object initted when stream was started
                                  sizeof( PVOID),
                                  gbuf, // output buffer gets back from kernel mode
                                  sizeof(gbuf),
                                  &nBytes,
                                  NULL);

        NOISY(("DeviceIoControl STOP_ISO_STREAM complete, success = %d\n", success));
    }
    else
    {
        ioctl = IOCTL_ISOUSB_START_ISO_STREAM;

        //input is our 256-byte buffer, binary char 0-255
        for (i = 0; i < sizeof(gbuf); i++)
        {
            gbuf[i] = i;
        }

        success = DeviceIoControl(hDEV,
                                  ioctl,
                                  gbuf,
                                  sizeof(gbuf),
                                  &gpStreamObj, // will receive pointer to stream object
                                  sizeof( PVOID),
                                  &nBytes,
                                  NULL);

        NOISY(("DeviceIoControl START_ISO_STREAM complete, success = %d\n", success));
    }

    if (hDEV == INVALID_HANDLE_VALUE)
    {
        NOISY(("DEV not open"));

        return;
    }
}

void StartIsoStream( void )
{
    if (!ghStreamDev)
    {
        ghStreamDev = open_dev();

        if (ghStreamDev != INVALID_HANDLE_VALUE )
        {
            IsoStream(ghStreamDev, FALSE);

            Sleep(gMS);

            StopIsoStream();
        }
    }
}

void StopIsoStream( void )
{
    if (ghStreamDev)
    {
        IsoStream(ghStreamDev, TRUE);

        ghStreamDev = NULL;
    }
}

//
// End, routines for Iso Streaming



void SelectAlternateInterface()
{
    HANDLE  hDEV;
    ULONG   success;
    int     siz;
    int     nBytes;
    UCHAR   buf[2];

    hDEV = open_dev();

    if (hDEV && hDEV != INVALID_HANDLE_VALUE)
    {
        siz = sizeof(buf);

        buf[0] = AltInterface;

        buf[1] = AltSetting;

        nBytes = 0;

        success = DeviceIoControl(hDEV,
                                  IOCTL_ISOUSB_SELECT_ALT_INTERFACE,
                                  buf,
                                  siz,
                                  buf,
                                  siz,
                                  &nBytes,
                                  NULL);

        NOISY(("request complete, success = %d nBytes = %d\n", success, nBytes));

        CloseHandle(hDEV);
    }
}



void
Usage()
/*++
Routine Description:

    Called by main() to dump usage info to the console when
    the app is called with no parms or with an invalid parm

Arguments:

    None

Return Value:

    None

--*/
{
    printf("Usage for Read/Write test:\n");
    printf("-r [n] where n is number of bytes to read\n");
    printf("-w [n] where n is number of bytes to write\n");
    printf("-c [n] where n is number of iterations (default = 1)\n");
    printf("-i [s] where s is the input pipe\n");
    printf("-o [s] where s is the output pipe\n");
    printf("-v verbose -- dumps read data\n");

    printf("\nUsage for USB and Endpoint info:\n");
    printf("-u to dump USB configuration and pipe info \n");
    printf("-g [s] Run Isochronous test stream for 's' seconds \n");

}


BOOL
Parse (
    int argc,
    char *argv[]
)
/*++
Routine Description:

    Called by main() to parse command line parms

Arguments:

    argc and argv that was passed to main()

Return Value:

    Sets global flags as per user function request

--*/
{
    int i;

    if (argc < 2 )
    {
        // give usage if invoked with no parms
        Usage();
        return FALSE;
    }

    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-' ||
            argv[i][0] == '/')
        {
            switch (argv[i][1])
            {
                case 'r':
                case 'R':
                    if (++i == argc)
                    {
                        Usage();
                        return FALSE;
                    }
                    else
                    {
                        ReadLen = atoi(argv[i]);
                        fRead = TRUE;
                    }
                    break;

                case 'w':
                case 'W':
                    if (++i == argc)
                    {
                        Usage();
                        return FALSE;
                    }
                    else
                    {
                        WriteLen = atoi(argv[i]);
                        fWrite = TRUE;
                    }
                    break;

                case 'c':
                case 'C':
                    if (++i == argc)
                    {
                        Usage();
                        return FALSE;
                    }
                    else
                    {
                        IterationCount = atoi(argv[i]);
                    }
                    break;

                case 'i':
                case 'I':
                    if (++i == argc)
                    {
                        Usage();
                        return FALSE;
                    }
                    else
                    {
                         strncpy(inPipe,
                                 argv[i],
                                 sizeof(inPipe)-1);

                        inPipe[sizeof(inPipe)-1] = '\0';
                    }
                    break;

                case 'u':
                case 'U':
                    fDumpUsbConfig = TRUE;
                    break;

                case 'v':
                case 'V':
                    fDumpReadData = TRUE;
                    break;

                case 'o':
                case 'O':
                    if (++i == argc)
                    {
                        Usage();
                        return FALSE;
                    }
                    else
                    {
                         strncpy(outPipe,
                                 argv[i],
                                 sizeof(outPipe)-1);

                         outPipe[sizeof(outPipe)-1] = '\0';
                    }
                    break;

                case 'g':
                case 'G':
                    if (++i == argc)
                    {
                        Usage();
                        return FALSE;
                    }
                    else
                    {
                        gMS = 1000 * atoi(argv[i]);
                        StartIsoStream();
                    }
                    break;

                case 'x':
                case 'X':
                    StopIsoStream();
                    break;

                case 'a':
                case 'A':
                    if (i+2 >= argc)
                    {
                        Usage();
                        return FALSE;
                    }
                    else
                    {
                        SelectAltInterface = TRUE;
                        AltInterface = (UCHAR)atoi(argv[i+1]);
                        AltSetting   = (UCHAR)atoi(argv[i+2]);
                        i += 2;
                    }
                    break;

                default:
                    Usage();
                    return FALSE;
            }
        }
        else
        {
            Usage();
            return FALSE;
        }
    }

    return TRUE;
}


int _cdecl main(
    int  argc,
    char *argv[])
/*++
Routine Description:

    Entry point to RwIso.exe
    Parses cmdline, performs user-requested tests

Arguments:

    argc, argv  standard console  'c' app arguments

Return Value:

    Zero

--*/

{
    char *pinBuf = NULL, *poutBuf = NULL;
    ULONG nBytesRead, nBytesWrite, nBytes;
    ULONG i, j;
    int ok;
    ULONG success;
    HANDLE hRead = INVALID_HANDLE_VALUE, hWrite = INVALID_HANDLE_VALUE;
    char buf[1024];
    clock_t start, finish;
    ULONG totalBytes = 0L;
    double seconds;
    ULONG fail = 0L;

    if (!Parse(argc, argv))
    {
        return 0;
    }

    if (SelectAltInterface)
    {
        printf("Interface %d\nSetting %d\n",
               AltInterface, AltSetting);

        SelectAlternateInterface();
    }

    // dump USB configuation and pipe info
    //
    if (fDumpUsbConfig)
    {
        dumpUsbConfig();
    }

    // doing a read, write, or both test
    //
    if ((fRead) || (fWrite))
    {
        if (fRead) {
            //
            // open the output file
            //
            if ( fDumpReadData )
            {
                // round size to sizeof ULONG for readable dumping
                while (ReadLen % sizeof( ULONG))
                {
                    ReadLen++;
                }
            }

            hRead = open_file( inPipe);

            pinBuf = malloc(ReadLen);

        }

        if (fWrite)
        {
            if ( fDumpReadData )
            {
                // round size to sizeof ULONG for readable dumping
                //
                while (WriteLen % sizeof( ULONG ))
                {
                    WriteLen++;
                }
            }

            hWrite = open_file( outPipe);

            poutBuf = malloc(WriteLen);
        }


        for (i=0; i<IterationCount; i++)
        {
            if (fWrite && poutBuf && hWrite != INVALID_HANDLE_VALUE)
            {
                PULONG pOut = (PULONG) poutBuf;
                ULONG  numLongs = WriteLen / sizeof( ULONG );

                //
                // put some data in the output buffer
                //
                for (j=0; j<numLongs; j++)
                {
                    *(pOut+j) = j;
                }

                //
                // send the write
                //
                WriteFile(hWrite,
                          poutBuf,
                          WriteLen,
                          &nBytesWrite,
                          NULL);

                printf("<%s> W (%04.4d) : request %06.6d bytes -- %06.6d bytes written\n",
                       outPipe, i, WriteLen, nBytesWrite);

                assert(nBytesWrite == WriteLen);
            }

            if (fRead && pinBuf)
            {
                success = ReadFile(hRead,
                                   pinBuf,
                                   ReadLen,
                                   &nBytesRead,
                                   NULL);

                printf("<%s> R (%04.4d) : request %06.6d bytes -- %06.6d bytes read\n",
                       inPipe, i, ReadLen, nBytesRead);

                if (fWrite)
                {
                    //
                    // validate the input buffer against what
                    // we sent to the 82930 (loopback test)
                    //
                    ok = compare_buffs(pinBuf, poutBuf,  nBytesRead);

                    if (fDumpReadData)
                    {
                        printf("Dumping read buffer\n");

                        dump(pinBuf, nBytesRead);

                        printf("Dumping write buffer\n");

                        dump(poutBuf, nBytesRead);
                    }

                    assert(ok);

                    if (ok != 1)
                    {
                        fail++;
                    }

                    assert(ReadLen == WriteLen);
                    assert(nBytesRead == ReadLen);
                    assert(nBytesWrite == WriteLen);
                }
            }
        }

        if (pinBuf)
        {
            free(pinBuf);
        }

        if (poutBuf)
        {
            free(poutBuf);
        }

        // Close devices if needed
        //
        if (hRead != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hRead);
        }

        if( hWrite != INVALID_HANDLE_VALUE)
        {
            CloseHandle(hWrite);
        }

    }

    // Stop iso stream if we started one
    //
    StopIsoStream();

    return 0;
}

