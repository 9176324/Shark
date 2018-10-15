/*++

Copyright (c) 1990-2000 Microsoft Corporation, All Rights Reserved
 
Module Name:

    gpdwrite.c

Abstract:  Write a byte to an port using the genport driver.

Author:    Robert R. Howell January 6, 1993


Environment:

    User mode

Revision History:

    Robert B. Nelson (Microsoft) January 12, 1993
    Robert B. Nelson (Microsoft) April 5, 1993
    Robert B. Nelson (Microsoft) May 6, 1993


--*/

#include <windows.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <winioctl.h>
#include "gpioctl.h"        // This defines the IOCTL constants.

void __cdecl main(int argc, char **argv)
{

    // The following is returned by IOCTL.  It is true if the write succeeds.
    BOOL    IoctlResult;

    // The following parameters are used in the IOCTL call
    HANDLE              hndFile;        // Handle to device, obtain from CreateFile
    GENPORT_WRITE_INPUT InputBuffer;    // Input buffer for DeviceIoControl
    LONG                IoctlCode;
    ULONG               DataValue;
    ULONG               DataLength;
    ULONG               ReturnedLength; // Number of bytes returned in output buffer


    if (argc < 4)
    {
        printf("GpdWrite -b|-w|-d <Port#> <Value>\n\n");
        printf("        The byte (-b), word (-w), or doubleword (-d) specified\n");
        printf("        as value is written to the given port.  Ports are numbered\n");
        printf("        as 0, 1, ... relative to the base port assinged to the driver\n");
        printf("        by the PNP manager according to the driver preferences\n");
        printf("        in the INF file. All numbers are printed in hex.\n");
        exit(1);
    }


    hndFile = CreateFile(
                    "\\\\.\\GpdDev",           // Open the Device "file"
                    GENERIC_WRITE,
                    FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL);

    if (hndFile == INVALID_HANDLE_VALUE)        // Was the device opened?
    {
        printf("Unable to open the device.\n");
        exit(1);
    }

    sscanf(argv[2], "%x", &InputBuffer.PortNumber);     // Get the port number
    sscanf(argv[3], "%x", &DataValue);          // Get the data to be written.

    switch (argv[1][1])
    {
        case 'b':
            IoctlCode = IOCTL_GPD_WRITE_PORT_UCHAR;
            InputBuffer.CharData = (UCHAR)DataValue;
            DataLength = offsetof(GENPORT_WRITE_INPUT, CharData) +
                         sizeof(InputBuffer.CharData);
            break;
            
        case 'w':
            IoctlCode = IOCTL_GPD_WRITE_PORT_USHORT;
            InputBuffer.ShortData = (USHORT)DataValue;
            DataLength = offsetof(GENPORT_WRITE_INPUT, ShortData) +
                         sizeof(InputBuffer.ShortData);
            break;

        case 'd':
            IoctlCode = IOCTL_GPD_WRITE_PORT_ULONG;
            InputBuffer.LongData = (ULONG)DataValue;
            DataLength = offsetof(GENPORT_WRITE_INPUT, LongData) +
                         sizeof(InputBuffer.LongData);
            break;
       default:
            printf("Wrong command line argument %c\n", argv[1][1]);
            exit(1);

    }

    IoctlResult = DeviceIoControl(
                        hndFile,                // Handle to device
                        IoctlCode,              // IO Control code for Write
                        &InputBuffer,           // Buffer to driver.  Holds port & data.
                        DataLength,             // Length of buffer in bytes.
                        NULL,                   // Buffer from driver.   Not used.
                        0,                      // Length of buffer in bytes.
                        &ReturnedLength,        // Bytes placed in outbuf.  Should be 0.
                        NULL                    // NULL means wait till I/O completes.
                        );


    if (IoctlResult)                            // Did the IOCTL succeed?
    {
        printf(
            "Wrote data %x to port %x\n", DataValue,
            InputBuffer.PortNumber);
    }
    else
    {
        printf(
            "Ioctl failed with code %ld\n", GetLastError() );
    }


    if (!CloseHandle(hndFile))                  // Close the Device "file".
    {
        printf("Failed to close device.\n");
    }

    exit(0);
}

