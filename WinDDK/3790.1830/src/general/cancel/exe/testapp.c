/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    testapp.c

Abstract:   

Author:

    Eliyas Yakub

Environment:

    User mode Win32 console application

Revision History:


--*/
#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "testapp.h"

//
// Globals
//

HANDLE hDevice;
BOOLEAN ExitFlag = FALSE;
HANDLE hThreads[MAXTHREADS];

//
// function prototypes
//

VOID CALLBACK CompletionRoutine(
    DWORD errorcode, 
    DWORD bytesTransfered, 
    LPOVERLAPPED ov
    );

DWORD WINAPI Reader(PVOID);

BOOLEAN
SetupDriverName(
    PUCHAR DriverLocation
    );
    
//
// Main function
//

VOID _cdecl main( ULONG argc, PCHAR argv[] )
{
    ULONG i, Id;
    ULONG   NumberOfThreads = 1;
    DWORD errNum = 0;
    BOOLEAN rCode = TRUE;
    UCHAR driverLocation[MAX_PATH];

    
    if (argc >= 2  && (argv[1][0] == '-' || isalpha(argv[1][0])))
    {
        puts("Usage:testapp <NumberOfThreads>\n");
        return;
    }
    else if (argc >= 2 && ((NumberOfThreads = atoi(argv[1])) > MAXTHREADS))
    {
        printf("Invalid option:Only a maximun of %d threads allowed.\n",
                    MAXTHREADS);
        return;

    }

    //
    // Try to connect to driver.  If this fails, try to load the driver
    // dynamically.
    //

    if ((hDevice = CreateFile("\\\\.\\CancelSamp",
                                 GENERIC_READ,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_FLAG_OVERLAPPED,
                                 NULL
                                 )) == INVALID_HANDLE_VALUE) {

        errNum = GetLastError();

        if (errNum != ERROR_FILE_NOT_FOUND) {

            printf("CreateFile failed!  Error = %d\n", errNum);

            return ;
        }

        //
        // Setup full path to driver name.
        //

        if (!SetupDriverName(driverLocation)) {

            return ;
        }


        //
        // Install driver.
        //

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
        // Try to open the newly installed driver.
        //
        
        hDevice = CreateFile( "\\\\.\\CancelSamp",
                GENERIC_READ,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                NULL);

        if ( hDevice == INVALID_HANDLE_VALUE ){
            printf ( "Error: CreatFile Failed : %d\n", GetLastError());
            return;
        }
    }    

    printf("Number of threads : %d\n", NumberOfThreads);


    printf("Enter 'q' to exit gracefully:");

    for(i=0; i < NumberOfThreads; i++) 
    {
        hThreads[i] = CreateThread( NULL,      // security attributes
                                    0,         // initial stack size
                                    Reader,    // Main() function
                                    NULL,      // arg to Reader thread
                                    0,         // creation flags
                                    (LPDWORD)&Id); // returned thread id

        if ( NULL == hThreads[i] ) {
            printf( " Error CreateThread[%d] Failed: %d\n", i, GetLastError());
            ExitProcess ( 1 );
        }

    }
    

    if(getchar() == 'q')
    {
        ExitFlag = TRUE;

        WaitForMultipleObjects( NumberOfThreads, hThreads, TRUE, INFINITE); 
        
        for(i=0; i < NumberOfThreads; i++)
            CloseHandle(hThreads[i]);

    }

    CloseHandle(hDevice);

    //
    // Unload the driver.  Ignore any errors.
    //

    ManageDriver(DRIVER_NAME,
                 driverLocation,
                 DRIVER_FUNC_REMOVE
                 );

    ExitProcess(1);
    
}


DWORD WINAPI Reader(PVOID dummy)
{
    ULONG data;
    OVERLAPPED ov;
  
    while(!ExitFlag)
    {
        ZeroMemory( &ov, sizeof(ov) );
        ov.Offset = 0;
        ov.OffsetHigh = 0;

        if(!ReadFileEx(hDevice, (PVOID)&data, sizeof(ULONG),  &ov, CompletionRoutine))
        {
            printf ( "Error: Read Failed: %d\n", GetLastError());
            ExitProcess ( 1 );
        }
        SleepEx(INFINITE, TRUE);
    }

    printf("Exiting thread %d \n", GetCurrentThreadId());
    ExitThread(0);
    return 0;
}


VOID CALLBACK CompletionRoutine(
    DWORD errorcode, 
    DWORD bytesTransfered, 
    LPOVERLAPPED ov
    )
{
    fprintf(stdout, "Thread %d read: %d bytes\n", 
           GetCurrentThreadId(), bytesTransfered);
    return;
}


BOOLEAN
SetupDriverName(
    PUCHAR DriverLocation
    )
{
    HANDLE fileHandle;

    DWORD driverLocLen = 0;

    //
    // Get the current directory.
    //

    driverLocLen = GetCurrentDirectory(MAX_PATH,
                                       DriverLocation
                                       );

    if (!driverLocLen) {

        printf("GetCurrentDirectory failed!  Error = %d \n", GetLastError());

        return FALSE;
    }

    //
    // Setup path name to driver file.
    //

    strcat(DriverLocation, "\\");
    strcat(DriverLocation, DRIVER_NAME);
    strcat(DriverLocation, ".sys");

    //
    // Insure driver file is in the specified directory.
    //

    if ((fileHandle = CreateFile(DriverLocation,
                                 GENERIC_READ,
                                 0,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL
                                 )) == INVALID_HANDLE_VALUE) {


        printf("%s.sys is not loaded.\n", DRIVER_NAME);

        //
        // Indicate failure.
        //

        return FALSE;
    }

    //
    // Close open file handle.
    //

    if (fileHandle) {

        CloseHandle(fileHandle);
    }

    //
    // Indicate success.
    //

    return TRUE;


}   // SetupDriverName


