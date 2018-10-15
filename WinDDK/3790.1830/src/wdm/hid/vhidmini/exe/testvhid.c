/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    testvhid.c
    
Abstract:


Author:

    Kumar Rajeev  (1/04/2002) 

Environment:

    user mode only

Notes:


Revision History:


--*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <TCHAR.h>
#include "common.h"

//
// Function prototypes
//

VOID
SendHidRequests(
    HANDLE file
    );

BOOL
SearchMatchingHwID (
    IN HDEVINFO            DeviceInfoSet,
    IN PSP_DEVINFO_DATA    DeviceInfoData
    );

BOOL
OpenDeviceInterface (
    IN       HDEVINFO                    HardwareDeviceInfo,
    IN       PSP_DEVICE_INTERFACE_DATA   DeviceInterfaceData
    );

BOOLEAN 
CheckIfOurDevice(
    HANDLE file
    );


//
// Copied this structure from hidport.h
//

typedef struct _HID_DEVICE_ATTRIBUTES {

    ULONG           Size;
    //
    // sizeof (struct _HID_DEVICE_ATTRIBUTES)
    //
    //
    // Vendor ids of this hid device
    //
    USHORT          VendorID;
    USHORT          ProductID;
    USHORT          VersionNumber;
    USHORT          Reserved[11];

} HID_DEVICE_ATTRIBUTES, * PHID_DEVICE_ATTRIBUTES;


//
// Implementation
//

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    HDEVINFO                                                hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA                    deviceInterfaceData;
    SP_DEVINFO_DATA                                 devInfoData;
    GUID            hidguid;
    int              i;

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    HidD_GetHidGuid(&hidguid);

    hardwareDeviceInfo = 
            SetupDiGetClassDevs ((LPGUID)&hidguid, 
                                            NULL, 
                                            NULL, // Define no
                                            (DIGCF_PRESENT | 
                                            DIGCF_INTERFACEDEVICE)); 

    if(INVALID_HANDLE_VALUE == hardwareDeviceInfo){
        printf("SetupDiGetClassDevs failed: %x\n", GetLastError());
        return 1;
    }

    deviceInterfaceData.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    //
    // Enumerate devices of this interface class
    //

    printf("\n....looking for our HID device (with UP=0xFF00 " 
                "and Usage=0x01)\n");

    for(i=0; SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                            0, // No care about specific PDOs
                            (LPGUID)&hidguid,
                            i, //
                            &deviceInterfaceData);
                            i++ ){

        //
        // Open the device interface and Check if it is our device 
        // by matching the Usage page and Usage from Hid_Caps.
        // If this is our device then send the hid request. 
        //

        if(OpenDeviceInterface(hardwareDeviceInfo, &deviceInterfaceData)){
            //
            // device was found and the hid request sent in the above routine 
            // so now cleanup
            //
            SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
            return 0;
        }

        //
        //device was not found so loop around.
        //

    }

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);

    printf("Failure: Could not find our HID device \n");

    return 0;
}


BOOL
OpenDeviceInterface (
    IN       HDEVINFO                    hardwareDeviceInfo,
    IN       PSP_DEVICE_INTERFACE_DATA   deviceInterfaceData
    )
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData = NULL;

    DWORD        predictedLength = 0;
    DWORD        requiredLength = 0;
    HANDLE        file;
    BOOL           deviceFound = FALSE;

    SetupDiGetDeviceInterfaceDetail(
                            hardwareDeviceInfo,
                            deviceInterfaceData,
                            NULL, // probing so no output buffer yet
                            0, // probing so output buffer length of zero
                            &requiredLength,
                            NULL
                            ); // not interested in the specific dev-node

    predictedLength = requiredLength;

    deviceInterfaceDetailData = 
         (PSP_DEVICE_INTERFACE_DETAIL_DATA) malloc (predictedLength);

    if(!deviceInterfaceDetailData)
    {
        printf("Error: OpenDeviceInterface: malloc failed\n");
        return FALSE;
    }

    deviceInterfaceDetailData->cbSize = 
                    sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

    if (!SetupDiGetDeviceInterfaceDetail(
                            hardwareDeviceInfo,
                            deviceInterfaceData,
                            deviceInterfaceDetailData,
                            predictedLength,
                            &requiredLength,
                            NULL)) 
    {
        printf("Error: SetupDiGetInterfaceDeviceDetail failed\n");
        free (deviceInterfaceDetailData);                        
        return FALSE;
    }

    file = CreateFile ( deviceInterfaceDetailData->DevicePath,
                            GENERIC_READ | GENERIC_WRITE, 
                            0, // FILE_SHARE_READ | FILE_SHARE_READ | 
                            NULL, // no SECURITY_ATTRIBUTES structure
                            OPEN_EXISTING, // No special create flags
                            0, // No special attributes
                            NULL); // No template file

    if (INVALID_HANDLE_VALUE == file) {
        printf("Error: CreateFile failed: %d\n", GetLastError());
        free (deviceInterfaceDetailData);
        return FALSE;
    }

    if(CheckIfOurDevice(file)){

        deviceFound  = TRUE;

        printf("...sending control request to our device\n");
  
        SendHidRequests(file);
      
    }

   CloseHandle(file);

    free (deviceInterfaceDetailData);

    return deviceFound;

}


BOOLEAN 
CheckIfOurDevice(
    HANDLE file)
{
    PHIDP_PREPARSED_DATA Ppd; // The opaque parser info describing this device
    HIDP_CAPS                       Caps; // The Capabilities of this hid device.
    USAGE                               MyUsagePage = 0xff00;
    USAGE                               MyUsage = 0x0001;

    if (!HidD_GetPreparsedData (file, &Ppd)) 
    {
        printf("Error: HidD_GetPreparsedData failed \n");
        return FALSE;
    }

    if (!HidP_GetCaps (Ppd, &Caps))
    {
        printf("Error: HidP_GetCaps failed \n");
        HidD_FreePreparsedData (Ppd);
        return FALSE;
    }

    if((Caps.UsagePage == MyUsagePage) && (Caps.Usage == MyUsage)){
        printf("Success: Found my device.. \n");
        return TRUE;
        
    }
   
    return FALSE;

}


VOID
SendHidRequests(
    HANDLE file
    )
{
    PHIDMINI_CONTROL_INFO  controlInfo = NULL;
    PHID_DEVICE_ATTRIBUTES devAttributes = NULL;
    ULONG bufferSize;

    //
    // Allocate memory equal to 1 byte for report ID + 1 byte of feature
    // + the sixz of  HID_DEVICE_ATTRIBUTES
    // (the buffer should be atleast equal to feature report length plus one 
    // byte for report ID)
    //

    bufferSize = sizeof (HIDMINI_CONTROL_INFO) 
                   + sizeof (HID_DEVICE_ATTRIBUTES);
    
    controlInfo = (PHIDMINI_CONTROL_INFO) malloc (bufferSize);
    
    if (!controlInfo )
    {
        printf("malloc failed\n");
        return;
    }
    
    //
    // Fill the control structure with the report ID of the control collection and
    // the control code.
    //
    
    controlInfo->ReportId = CONTROL_COLLECTION_REPORT_ID;
    controlInfo->ControlCode = HIDMINI_CONTROL_CODE_GET_ATTRIBUTES;

    //
    // Send Hid control code thru HidD_GetFeature API
    //
        
    if(!HidD_GetFeature(file,     //  HidDeviceObject,
                            controlInfo,    //   ReportBuffer,
                            bufferSize //ReportBufferLength
                            )) 
    {
        printf("failed HidD_GetFeature \n");
        free (controlInfo);
        return;
    }
    else 
    {        
        printf("\nSuccess: HidD_GetFeature with control code \n" 
                "            HIDMINI_CONTROL_CODE_GET_ATTRIBUTES \n");

        devAttributes = (PHID_DEVICE_ATTRIBUTES)controlInfo->ControlBuffer;

        printf("Device Attributes: \n");
        printf("    VendorID: 0x%x, \n"
                "    ProductID: 0x%x, \n"
                "    VersionNumber: 0x%x\n",
                devAttributes->VendorID,
                devAttributes->ProductID,
                devAttributes->VersionNumber);
    }

    free(controlInfo);
    devAttributes = NULL;
    
    return;
}

