/*++

Copyright (c) 1990-2000 Microsoft Corporation, All Rights Reserved
 
Module Name:

    gpdread.c

Abstract:   Read a byte from a port using the genport driver.


Author:    Robert R. Howell January 6, 1993


Environment:

    User mode

Revision History:

    Robert B. Nelson (Microsoft) January 12, 1993


--*/



#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winioctl.h>
#include "gpioctl.h"        // This defines the IOCTL constants.

void __cdecl main(int argc, char ** argv)
{

    // The following is returned by IOCTL.  It is true if the read succeeds.
    BOOL    IoctlResult;

    // The following parameters are used in the IOCTL call
    HANDLE  hndFile;            // Handle to device, obtain from CreateFile
    ULONG   PortNumber;         // Buffer sent to the driver (Port #).
    union   {
        ULONG   LongData;
        USHORT  ShortData;
        UCHAR   CharData;
    }   DataBuffer;             // Buffer received from driver (Data).
    LONG    IoctlCode;
    ULONG   DataLength;
    DWORD   ReturnedLength;     // Number of bytes returned

    // The input buffer is a ULONG containing the port address.  

    // The port data is returned in the output buffer DataBuffer;

    if ( argc < 3 || argv[1][0] != '-' ||
         ( argv[1][1] != 'b' && argv[1][1] != 'w' && argv[1][1] != 'd' ) )
    {
        printf("GpdRead -b|-w|-d Port#  A byte (-b), word (-w), or a double\n");
        printf("                        word (-d) is read from the given port.\n");
        printf("                        Ports are numbered as 0, 1, ... relative\n");
        printf("                        to the base port assinged to the driver\n");
        printf("                        by PNP manager according to the driver preferences\n");
        printf("                        in the INF file. All numbers are read and printed in hex.\n");
        exit(1);
    }

    hndFile = CreateFile(
                "\\\\.\\GpdDev",                    // Open the Device "file"
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );
                
    if (hndFile == INVALID_HANDLE_VALUE)        // Was the device opened?
    {
        printf("Unable to open the device.\n");
        exit(1);
    }

    switch (argv[1][1])
    {
        case 'b':
            IoctlCode = IOCTL_GPD_READ_PORT_UCHAR;
            DataLength = sizeof(DataBuffer.CharData);
            break;
            
        case 'w':
            IoctlCode = IOCTL_GPD_READ_PORT_USHORT;
            DataLength = sizeof(DataBuffer.ShortData);
            break;

        case 'd':
            IoctlCode = IOCTL_GPD_READ_PORT_ULONG;
            DataLength = sizeof(DataBuffer.LongData);
            break;
       default:
            printf("Invalid command line argument %c\n", argv[1][1]);
            exit(1);

    }

    sscanf(argv[2], "%x", &PortNumber);         // Get the port number to be read

    IoctlResult = DeviceIoControl(
                            hndFile,            // Handle to device
                            IoctlCode,          // IO Control code for Read
                            &PortNumber,        // Buffer to driver.
                            sizeof(PortNumber), // Length of buffer in bytes.
                            &DataBuffer,        // Buffer from driver.
                            DataLength,         // Length of buffer in bytes.
                            &ReturnedLength,    // Bytes placed in DataBuffer.
                            NULL                // NULL means wait till op. completes.
                            );

    if (IoctlResult)                            // Did the IOCTL succeed?
    {
        ULONG   Data;

        if (ReturnedLength != DataLength)
        {
            printf(
                "Ioctl transfered %d bytes, expected %d bytes\n",
                ReturnedLength, DataLength );
        }
        
        switch (ReturnedLength)
        {
            case sizeof(UCHAR):
                Data = DataBuffer.CharData;
                break;
                
            case sizeof(USHORT):
                Data = DataBuffer.ShortData;
                break;
    
            case sizeof(ULONG):
                Data = DataBuffer.LongData;
                break;
            default:
                printf("Invalid return value from the DeviceIoControl\n");
                exit(1);

        }
        
        printf("Read from port %x returned %x\n", PortNumber, Data);
    }
    else
    {
        printf("Ioctl failed with code %ld\n", GetLastError() );
    }


    if (!CloseHandle(hndFile))                  // Close the Device "file".
    {
        printf("Failed to close device.\n");
    }

    exit(0);
}

