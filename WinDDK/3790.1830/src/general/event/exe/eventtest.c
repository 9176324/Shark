/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    EventTest.c

Abstract:

    Simple console test app for the event.sys driver.

Enviroment:

    User Mode

Revision History:

--*/


//
// INCLUDES
//
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <conio.h>

#include "public.h"

BOOLEAN
ManageDriver(
    IN LPCTSTR  DriverName,
    IN LPCTSTR  ServiceName,
    IN USHORT   Function
    );

BOOLEAN
SetupDriverName(
    PUCHAR DriverLocation
    );

#define USAGE()         {\
        printf("event <delay> <0/1>\n");\
        printf("\twhere <delay> = time to delay the event signal in seconds.\n");\
        printf("\t 0 for IRP based and 1 for event based notification.\n");\
}

//
// MAIN
//
void __cdecl 
main(
    int argc, 
    char ** argv
    )
{
    BOOL    bStatus;
    HANDLE  hDevice;
    ULONG   ulReturnedLength;
    REGISTER_EVENT registerEvent;
    FLOAT fDelay = 3;
    UINT type = EVENT_BASED;
    DWORD errNum = 0;
    UCHAR driverLocation[MAX_PATH];


    if ( (argc < 3) || (argv[1] == NULL) || (argv[2] == NULL) ) {
        USAGE();
        exit(0);
    }
    (void)sscanf( argv[1], "%f", &fDelay );
    (void)sscanf( argv[2], "%d", &type );

    //
    // open the device
    //
    if((hDevice = CreateFile(
                "\\\\.\\Event_Sample",                     // lpFileName
                GENERIC_READ | GENERIC_WRITE,       // dwDesiredAccess
                FILE_SHARE_READ | FILE_SHARE_WRITE, // dwShareMode
                NULL,                               // lpSecurityAttributes
                OPEN_EXISTING,                      // dwCreationDistribution
                0,                                  // dwFlagsAndAttributes
                NULL                                // hTemplateFile
                )) == INVALID_HANDLE_VALUE) {
                
        errNum = GetLastError();

        if (errNum != ERROR_FILE_NOT_FOUND) {

            printf("CreateFile failed!  ERROR_FILE_NOT_FOUND = %d\n", errNum);

            return ;
        }
       
        //
        // The driver is not started yet so let us the install driver.
        // First setup full path to driver name.
        //

        if (!SetupDriverName(driverLocation)) {

            return ;
        }
        
        if (!ManageDriver(DRIVER_NAME,
                          driverLocation,
                          DRIVER_FUNC_INSTALL
                          )) {

            printf("Unable to install driver. \n");

            //
            // Error - remove driver.
            //

            ManageDriver(DRIVER_NAME,
                         driverLocation,
                         DRIVER_FUNC_REMOVE
                         );
            
            return;
        }

        //
        // Now open the device again.
        //
        hDevice = CreateFile(
                    "\\\\.\\Event_Sample",                     // lpFileName
                    GENERIC_READ | GENERIC_WRITE,       // dwDesiredAccess
                    FILE_SHARE_READ | FILE_SHARE_WRITE, // dwShareMode
                    NULL,                               // lpSecurityAttributes
                    OPEN_EXISTING,                      // dwCreationDistribution
                    0,                                  // dwFlagsAndAttributes
                    NULL                                // hTemplateFile
                    );

        if ( hDevice == INVALID_HANDLE_VALUE ){
            printf ( "Error: CreatFile Failed : %d\n", GetLastError());
            return;
        }

    }

    //
    // set the event signal delay. Use relative time for this sample
    //
    registerEvent.DueTime.QuadPart = -((LONGLONG)(fDelay * 10.0E6));
    registerEvent.Type = type;
    
    if(type == EVENT_BASED) {
            //
            // 
            //
            registerEvent.hEvent = CreateEvent( 
                                    NULL,   // lpEventAttributes
                                    TRUE,   // bManualReset
                                    FALSE,  // bInitialState
#ifdef DBG
                                    "TEST_EVENT" // use WinObj to view named events for DBG
#else
                                    NULL    // lpName
#endif
                                    );


            if ( !registerEvent.hEvent ) {
                printf("CreateEvent error = %d\n", GetLastError() );
            } else {

            printf("Event HANDLE = %p\n",  registerEvent.hEvent );
            printf("Press any key to exit.\n");
            while( !_kbhit() ) {
                bStatus = DeviceIoControl(
                                hDevice,                // Handle to device
                                IOCTL_REGISTER_EVENT,        // IO Control code
                                &registerEvent,              // Input Buffer to driver.
                                SIZEOF_REGISTER_EVENT,        // Length of input buffer in bytes.
                                NULL,                   // Output Buffer from driver.
                                0,                      // Length of output buffer in bytes.
                                &ulReturnedLength,      // Bytes placed in buffer.
                                NULL                    // synchronous call
                                );

                if ( !bStatus ) {
                    printf("Ioctl failed with code %d\n", GetLastError() );
                    break;
                } else {
                    printf("Waiting for Event...\n");

                    WaitForSingleObject(registerEvent.hEvent,
                                        INFINITE );

                    printf("Event signalled.\n\n");

                    ResetEvent( registerEvent.hEvent);
                    //printf("Event reset.\n");
                }
            }
        }
            
    }else if (type == IRP_BASED) {

        printf("Press any key to exit.\n");       
        registerEvent.hEvent = NULL;
        registerEvent.Type = IRP_BASED;
        
        while( !_kbhit() ) {
            bStatus = DeviceIoControl(
                            hDevice,                // Handle to device
                            IOCTL_REGISTER_EVENT,        // IO Control code
                            &registerEvent,              // Input Buffer to driver.
                            SIZEOF_REGISTER_EVENT,        // Length of input buffer in bytes.
                            NULL,                   // Output Buffer from driver.
                            0,                      // Length of output buffer in bytes.
                            &ulReturnedLength,      // Bytes placed in buffer.
                            NULL                    // synchronous call
                            );

            if ( !bStatus ) {
                printf("Ioctl failed with code %d\n", GetLastError() );
                break;
            }
            printf("Event occurred.\n\n");
            printf("\nRegistering event again....\n\n");
       }   
        
    }else { //unknown type
        USAGE();
     }

    //
    // close the handle to the device.
    //
    CloseHandle(hDevice);

    //
    // Unload the driver.  Ignore any errors.
    //

    ManageDriver(DRIVER_NAME,
                 driverLocation,
                 DRIVER_FUNC_REMOVE
                 );

    
  
    return;
}
