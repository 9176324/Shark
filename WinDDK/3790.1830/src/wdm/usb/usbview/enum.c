/*++

Copyright (c) 1997-1998 Microsoft Corporation

Module Name:

    ENUM.C

Abstract:

    This source file contains the routines which enumerate the USB bus
    and populate the TreeView control.

    The enumeration process goes like this:

    (1) Enumerate Host Controllers and Root Hubs
    Host controllers currently have symbolic link names of the form HCDx,
    where x starts at 0.  Use CreateFile() to open each host controller
    symbolic link.  Create a node in the TreeView to represent each host
    controller.

    After a host controller has been opened, send the host controller an
    IOCTL_USB_GET_ROOT_HUB_NAME request to get the symbolic link name of
    the root hub that is part of the host controller.

    (2) Enumerate Hubs (Root Hubs and External Hubs)
    Given the name of a hub, use CreateFile() to hub the hub.  Send the
    hub an IOCTL_USB_GET_NODE_INFORMATION request to get info about the
    hub, such as the number of downstream ports.  Create a node in the
    TreeView to represent each hub.

    (3) Enumerate Downstream Ports
    Given an handle to an open hub and the number of downstream ports on
    the hub, send the hub an IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
    request for each downstream port of the hub to get info about the
    device (if any) attached to each port.  If there is a device attached
    to a port, send the hub an IOCTL_USB_GET_NODE_CONNECTION_NAME request
    to get the symbolic link name of the hub attached to the downstream
    port.  If there is a hub attached to the downstream port, recurse to
    step (2).  Create a node in the TreeView to represent each hub port
    and attached device.

Environment:

    user mode

Revision History:

    04-25-97 : created

--*/

//*****************************************************************************
// I N C L U D E S
//*****************************************************************************

#include <windows.h>
#include <basetyps.h>
#include <winioctl.h>
#include <setupapi.h>
#include <string.h>
#include <stdio.h>

#include "usbview.h"

//*****************************************************************************
// D E F I N E S
//*****************************************************************************

#define NUM_HCS_TO_CHECK 10

//*****************************************************************************
// G L O B A L S
//*****************************************************************************

// List of enumerated host controllers.
//
LIST_ENTRY EnumeratedHCListHead =
{
    &EnumeratedHCListHead,
    &EnumeratedHCListHead
};

//*****************************************************************************
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//*****************************************************************************

VOID
EnumerateHub (
    HTREEITEM                           hTreeParent,
    PCHAR                               HubName,
    PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo,
    PUSB_DESCRIPTOR_REQUEST             ConfigDesc,
    PSTRING_DESCRIPTOR_NODE             StringDescs,
    PCHAR                               DeviceDesc
);

VOID
EnumerateHubPorts (
    HTREEITEM   hTreeParent,
    HANDLE      hHubDevice,
    ULONG       NumPorts
);

PCHAR GetRootHubName (
    HANDLE HostController
);

PCHAR GetExternalHubName (
    HANDLE  Hub,
    ULONG   ConnectionIndex
);

PCHAR GetHCDDriverKeyName (
    HANDLE  HCD
);

PCHAR GetDriverKeyName (
    HANDLE  Hub,
    ULONG   ConnectionIndex
);

PUSB_DESCRIPTOR_REQUEST
GetConfigDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex
);

BOOL
AreThereStringDescriptors (
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
);

PSTRING_DESCRIPTOR_NODE
GetAllStringDescriptors (
    HANDLE                          hHubDevice,
    ULONG                           ConnectionIndex,
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
);

PSTRING_DESCRIPTOR_NODE
GetStringDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    USHORT  LanguageID
);

PSTRING_DESCRIPTOR_NODE
GetStringDescriptors (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    ULONG   NumLanguageIDs,
    USHORT  *LanguageIDs,
    PSTRING_DESCRIPTOR_NODE StringDescNodeTail
);

//*****************************************************************************
// G L O B A L S    P R I V A T E    T O    T H I S    F I L E
//*****************************************************************************

PCHAR ConnectionStatuses[] =
{
    "NoDeviceConnected",
    "DeviceConnected",
    "DeviceFailedEnumeration",
    "DeviceGeneralFailure",
    "DeviceCausedOvercurrent",
    "DeviceNotEnoughPower"
};

ULONG TotalDevicesConnected;


//*****************************************************************************
//
// EnumerateHostController()
//
// hTreeParent - Handle of the TreeView item under which host controllers
// should be added.
//
//*****************************************************************************

VOID
EnumerateHostController (
    HTREEITEM  hTreeParent,
    HANDLE     hHCDev,
    PCHAR      leafName
)
{
    PCHAR       driverKeyName;
    PCHAR       deviceDesc;
    PCHAR       deviceId;
    HTREEITEM   hHCItem;
    PCHAR       rootHubName;
    PLIST_ENTRY listEntry;
    PUSBHOSTCONTROLLERINFO hcInfo;
    PUSBHOSTCONTROLLERINFO hcInfoInList;

    // Allocate a structure to hold information about this host controller.
    //
    hcInfo = (PUSBHOSTCONTROLLERINFO)ALLOC(sizeof(USBHOSTCONTROLLERINFO));

    if (hcInfo != NULL)
    {
        hcInfo->DeviceInfoType = HostControllerInfo;

        // Obtain the driver key name for this host controller.
        //
        driverKeyName = GetHCDDriverKeyName(hHCDev);

        if (driverKeyName)
        {
            // Don't enumerate this host controller again if it already
            // on the list of enumerated host controllers.
            //
            listEntry = EnumeratedHCListHead.Flink;

            while (listEntry != &EnumeratedHCListHead)
            {
                hcInfoInList = CONTAINING_RECORD(listEntry,
                                                 USBHOSTCONTROLLERINFO,
                                                 ListEntry);

                if (strcmp(driverKeyName, hcInfoInList->DriverKey) == 0)
                {
                    // Already on the list, exit
                    //
                    FREE(driverKeyName);
                    FREE(hcInfo);
                    return;
                }

                listEntry = listEntry->Flink;
            }

            // Obtain the device id string for this host controller.
            // (Note: this a tmp global string buffer, make a copy of
            // this string if it will used later.)
            //
            deviceId = DriverNameToDeviceDesc(driverKeyName, TRUE);

            if (deviceId)
            {
                ULONG   ven, dev, subsys, rev;

                if (sscanf(deviceId,
                           "PCI\\VEN_%x&DEV_%x&SUBSYS_%x&REV_%x",
                           &ven, &dev, &subsys, &rev) != 4)
                {
                    OOPS();
                }

                hcInfo->DriverKey = driverKeyName;

                hcInfo->VendorID = ven;
                hcInfo->DeviceID = dev;
                hcInfo->SubSysID = subsys;
                hcInfo->Revision = rev;
            }
            else
            {
                OOPS();
            }

            // Obtain the device description string for this host controller.
            // (Note, this a tmp global string buffer, make a copy of
            // this string if it will be used later.)
            //
            deviceDesc = DriverNameToDeviceDesc(driverKeyName, FALSE);

            if (deviceDesc)
            {
                leafName = deviceDesc;
            }
            else
            {
                OOPS();
            }

            // Add this host controller to the USB device tree view.
            //
            hHCItem = AddLeaf(hTreeParent,
                              (LPARAM)hcInfo,
                              leafName,
                              GoodDeviceIcon);

            if (hHCItem)
            {
                // Add this host controller to the list of enumerated
                // host controllers.
                //
                InsertTailList(&EnumeratedHCListHead,
                               &hcInfo->ListEntry);

                // Get the name of the root hub for this host
                // controller and then enumerate the root hub.
                //
                rootHubName = GetRootHubName(hHCDev);

                if (rootHubName != NULL)
                {
                    EnumerateHub(hHCItem,
                                 rootHubName,
                                 NULL,      // ConnectionInfo
                                 NULL,      // ConfigDesc
                                 NULL,      // StringDescs
                                 "RootHub"  // DeviceDesc
                                );
                }
                else
                {
                    // Failure obtaining root hub name.

                    OOPS();
                }
            }
            else
            {
                // Failure adding host controller to USB device tree
                // view.

                OOPS();

                FREE(driverKeyName);
                FREE(hcInfo);
            }
        }
        else
        {
            // Failure obtaining driver key name.

            OOPS();

            FREE(hcInfo);
        }
    }
}

//*****************************************************************************
//
// EnumerateHostControllers()
//
// hTreeParent - Handle of the TreeView item under which host controllers
// should be added.
//
//*****************************************************************************

VOID
EnumerateHostControllers (
    HTREEITEM  hTreeParent,
    ULONG     *DevicesConnected
)
{
    char        HCName[16];
    int         HCNum;
    HANDLE      hHCDev;
    PCHAR       leafName;

    HDEVINFO                         deviceInfo;
    SP_DEVICE_INTERFACE_DATA         deviceInfoData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA deviceDetailData;
    ULONG                            index;
    ULONG                            requiredLength;

    TotalDevicesConnected = 0;
    TotalHubs = 0;

    // Iterate over some Host Controller names and try to open them.
    //
    for (HCNum = 0; HCNum < NUM_HCS_TO_CHECK; HCNum++)
    {
        wsprintf(HCName, "\\\\.\\HCD%d", HCNum);

        hHCDev = CreateFile(HCName,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

        // If the handle is valid, then we've successfully opened a Host
        // Controller.  Display some info about the Host Controller itself,
        // then enumerate the Root Hub attached to the Host Controller.
        //
        if (hHCDev != INVALID_HANDLE_VALUE)
        {
            leafName = HCName + sizeof("\\\\.\\") - sizeof("");

            EnumerateHostController(hTreeParent,
                                    hHCDev,
                                    leafName);

            CloseHandle(hHCDev);
        }
    }

    // Now iterate over host controllers using the new GUID based interface
    //
    deviceInfo = SetupDiGetClassDevs((LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER,
                                     NULL,
                                     NULL,
                                     (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

    deviceInfoData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    for (index=0;
         SetupDiEnumDeviceInterfaces(deviceInfo,
                                     0,
                                     (LPGUID)&GUID_CLASS_USB_HOST_CONTROLLER,
                                     index,
                                     &deviceInfoData);
         index++)
    {
        SetupDiGetDeviceInterfaceDetail(deviceInfo,
                                        &deviceInfoData,
                                        NULL,
                                        0,
                                        &requiredLength,
                                        NULL);

        deviceDetailData = GlobalAlloc(GPTR, requiredLength);

        deviceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

        SetupDiGetDeviceInterfaceDetail(deviceInfo,
                                        &deviceInfoData,
                                        deviceDetailData,
                                        requiredLength,
                                        &requiredLength,
                                        NULL);

        hHCDev = CreateFile(deviceDetailData->DevicePath,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

        // If the handle is valid, then we've successfully opened a Host
        // Controller.  Display some info about the Host Controller itself,
        // then enumerate the Root Hub attached to the Host Controller.
        //
        if (hHCDev != INVALID_HANDLE_VALUE)
        {
            leafName = deviceDetailData->DevicePath;

            EnumerateHostController(hTreeParent,
                                    hHCDev,
                                    leafName);

            CloseHandle(hHCDev);
        }

        GlobalFree(deviceDetailData);
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);

    *DevicesConnected = TotalDevicesConnected;
}

//*****************************************************************************
//
// EnumerateHub()
//
// hTreeParent - Handle of the TreeView item under which this hub should be
// added.
//
// HubName - Name of this hub.  This pointer is kept so the caller can neither
// free nor reuse this memory.
//
// ConnectionInfo - NULL if this is a root hub, else this is the connection
// info for an external hub.  This pointer is kept so the caller can neither
// free nor reuse this memory.
//
// ConfigDesc - NULL if this is a root hub, else this is the Configuration
// Descriptor for an external hub.  This pointer is kept so the caller can
// neither free nor reuse this memory.
//
// StringDescs - NULL if this is a root hub.
//
//*****************************************************************************

VOID
EnumerateHub (
    HTREEITEM                           hTreeParent,
    PCHAR                               HubName,
    PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo,
    PUSB_DESCRIPTOR_REQUEST             ConfigDesc,
    PSTRING_DESCRIPTOR_NODE             StringDescs,
    PCHAR                               DeviceDesc
)
{
    PUSB_NODE_INFORMATION   hubInfo;
    HANDLE                  hHubDevice;
    HTREEITEM               hItem;
    PCHAR                   deviceName;
    BOOL                    success;
    ULONG                   nBytes;
    PVOID                   info;
    CHAR                    leafName[512]; // XXXXX how big does this have to be?

    // Initialize locals to not allocated state so the error cleanup routine
    // only tries to cleanup things that were successfully allocated.
    //
    info        = NULL;
    hubInfo     = NULL;
    hHubDevice  = INVALID_HANDLE_VALUE;

    // Allocate some space for a USBDEVICEINFO structure to hold the
    // hub info, hub name, and connection info pointers.  GPTR zero
    // initializes the structure for us.
    //
    if (ConnectionInfo != NULL)
    {
        info = ALLOC(sizeof(USBEXTERNALHUBINFO));
    }
    else
    {
        info = ALLOC(sizeof(USBROOTHUBINFO));
    }

    if (info == NULL)
    {
        OOPS();
        goto EnumerateHubError;
    }

    // Allocate some space for a USB_NODE_INFORMATION structure for this Hub,
    //
    hubInfo = (PUSB_NODE_INFORMATION)ALLOC(sizeof(USB_NODE_INFORMATION));

    if (hubInfo == NULL)
    {
        OOPS();
        goto EnumerateHubError;
    }

    // Keep copies of the Hub Name, Connection Info, and Configuration
    // Descriptor pointers
    //
    if (ConnectionInfo != NULL)
    {
        ((PUSBEXTERNALHUBINFO)info)->DeviceInfoType = ExternalHubInfo;

        ((PUSBEXTERNALHUBINFO)info)->HubInfo = hubInfo;

        ((PUSBEXTERNALHUBINFO)info)->HubName = HubName;

        ((PUSBEXTERNALHUBINFO)info)->ConnectionInfo = ConnectionInfo;

        ((PUSBEXTERNALHUBINFO)info)->ConfigDesc = ConfigDesc;

        ((PUSBEXTERNALHUBINFO)info)->StringDescs = StringDescs;
    }
    else
    {
        ((PUSBROOTHUBINFO)info)->DeviceInfoType = RootHubInfo;

        ((PUSBROOTHUBINFO)info)->HubInfo = hubInfo;

        ((PUSBROOTHUBINFO)info)->HubName = HubName;
    }

    // Allocate a temp buffer for the full hub device name.
    //
    deviceName = (PCHAR)ALLOC(strlen(HubName) + sizeof("\\\\.\\"));

    if (deviceName == NULL)
    {
        OOPS();
        goto EnumerateHubError;
    }

    // Create the full hub device name
    //
    strcpy(deviceName, "\\\\.\\");
    strcpy(deviceName + sizeof("\\\\.\\") - 1, HubName);

    // Try to hub the open device
    //
    hHubDevice = CreateFile(deviceName,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL);

    // Done with temp buffer for full hub device name
    //
    FREE(deviceName);

    if (hHubDevice == INVALID_HANDLE_VALUE)
    {
        OOPS();
        goto EnumerateHubError;
    }

    //
    // Now query USBHUB for the USB_NODE_INFORMATION structure for this hub.
    // This will tell us the number of downstream ports to enumerate, among
    // other things.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_NODE_INFORMATION,
                              hubInfo,
                              sizeof(USB_NODE_INFORMATION),
                              hubInfo,
                              sizeof(USB_NODE_INFORMATION),
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto EnumerateHubError;
    }

    // Build the leaf name from the port number and the device description
    //
    if (ConnectionInfo)
    {
        wsprintf(leafName, "[Port%d] ", ConnectionInfo->ConnectionIndex);
        strcat(leafName, ConnectionStatuses[ConnectionInfo->ConnectionStatus]);
        strcat(leafName, " :  ");
    }
    else
    {
        leafName[0] = 0;
    }

    if (DeviceDesc)
    {
        strcat(leafName, DeviceDesc);
    }
    else
    {
        strcat(leafName, HubName);
    }

    // Now add an item to the TreeView with the PUSBDEVICEINFO pointer info
    // as the LPARAM reference value containing everything we know about the
    // hub.
    //
    hItem = AddLeaf(hTreeParent,
                    (LPARAM)info,
                    leafName,
                    HubIcon);

    if (hItem == NULL)
    {
        OOPS();
        goto EnumerateHubError;
    }

    // Now recursively enumrate the ports of this hub.
    //
    EnumerateHubPorts(
        hItem,
        hHubDevice,
        hubInfo->u.HubInformation.HubDescriptor.bNumberOfPorts
        );


    CloseHandle(hHubDevice);
    return;

EnumerateHubError:
    //
    // Clean up any stuff that got allocated
    //

    if (hHubDevice != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hHubDevice);
        hHubDevice = INVALID_HANDLE_VALUE;
    }

    if (hubInfo)
    {
        FREE(hubInfo);
    }

    if (info)
    {
        FREE(info);
    }

    if (HubName)
    {
        FREE(HubName);
    }

    if (ConnectionInfo)
    {
        FREE(ConnectionInfo);
    }

    if (ConfigDesc)
    {
        FREE(ConfigDesc);
    }

    if (StringDescs != NULL)
    {
        PSTRING_DESCRIPTOR_NODE Next;

        do {

            Next = StringDescs->Next;
            FREE(StringDescs);
            StringDescs = Next;

        } while (StringDescs != NULL);
    }
}

//*****************************************************************************
//
// EnumerateHubPorts()
//
// hTreeParent - Handle of the TreeView item under which the hub port should
// be added.
//
// hHubDevice - Handle of the hub device to enumerate.
//
// NumPorts - Number of ports on the hub.
//
//*****************************************************************************

VOID
EnumerateHubPorts (
    HTREEITEM   hTreeParent,
    HANDLE      hHubDevice,
    ULONG       NumPorts
)
{
    HTREEITEM   hItem;
    ULONG       index;
    BOOL        success;

    PUSB_NODE_CONNECTION_INFORMATION_EX connectionInfoEx;
    PUSB_DESCRIPTOR_REQUEST             configDesc;
    PSTRING_DESCRIPTOR_NODE             stringDescs;
    PUSBDEVICEINFO                      info;

    PCHAR driverKeyName;
    PCHAR deviceDesc;
    CHAR  leafName[512]; // XXXXX how big does this have to be?
    int   icon;

    // Loop over all ports of the hub.
    //
    // Port indices are 1 based, not 0 based.
    //
    for (index=1; index <= NumPorts; index++)
    {
        ULONG nBytesEx;

        // Allocate space to hold the connection info for this port.
        // For now, allocate it big enough to hold info for 30 pipes.
        //
        // Endpoint numbers are 0-15.  Endpoint number 0 is the standard
        // control endpoint which is not explicitly listed in the Configuration
        // Descriptor.  There can be an IN endpoint and an OUT endpoint at
        // endpoint numbers 1-15 so there can be a maximum of 30 endpoints
        // per device configuration.
        //
        // Should probably size this dynamically at some point.
        //
        nBytesEx = sizeof(USB_NODE_CONNECTION_INFORMATION_EX) +
                   sizeof(USB_PIPE_INFO) * 30;

        connectionInfoEx = (PUSB_NODE_CONNECTION_INFORMATION_EX)ALLOC(nBytesEx);

        if (connectionInfoEx == NULL)
        {
            OOPS();
            break;
        }

        //
        // Now query USBHUB for the USB_NODE_CONNECTION_INFORMATION_EX structure
        // for this port.  This will tell us if a device is attached to this
        // port, among other things.
        //
        connectionInfoEx->ConnectionIndex = index;

        success = DeviceIoControl(hHubDevice,
                                  IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
                                  connectionInfoEx,
                                  nBytesEx,
                                  connectionInfoEx,
                                  nBytesEx,
                                  &nBytesEx,
                                  NULL);

        if (!success)
        {
            PUSB_NODE_CONNECTION_INFORMATION    connectionInfo;
            ULONG                               nBytes;

            // Try using IOCTL_USB_GET_NODE_CONNECTION_INFORMATION
            // instead of IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX
            //
            nBytes = sizeof(USB_NODE_CONNECTION_INFORMATION) +
                     sizeof(USB_PIPE_INFO) * 30;

            connectionInfo = (PUSB_NODE_CONNECTION_INFORMATION)ALLOC(nBytes);

            connectionInfo->ConnectionIndex = index;

            success = DeviceIoControl(hHubDevice,
                                      IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
                                      connectionInfo,
                                      nBytes,
                                      connectionInfo,
                                      nBytes,
                                      &nBytes,
                                      NULL);

            if (!success)
            {
                OOPS();

                FREE(connectionInfo);
                FREE(connectionInfoEx);
                continue;
            }

            // Copy IOCTL_USB_GET_NODE_CONNECTION_INFORMATION into
            // IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX structure.
            //
            connectionInfoEx->ConnectionIndex =
                connectionInfo->ConnectionIndex;

            connectionInfoEx->DeviceDescriptor =
                connectionInfo->DeviceDescriptor;

            connectionInfoEx->CurrentConfigurationValue =
                connectionInfo->CurrentConfigurationValue;

            connectionInfoEx->Speed =
                connectionInfo->LowSpeed ? UsbLowSpeed : UsbFullSpeed;

            connectionInfoEx->DeviceIsHub =
                connectionInfo->DeviceIsHub;

            connectionInfoEx->DeviceAddress =
                connectionInfo->DeviceAddress;

            connectionInfoEx->NumberOfOpenPipes =
                connectionInfo->NumberOfOpenPipes;

            connectionInfoEx->ConnectionStatus =
                connectionInfo->ConnectionStatus;

            memcpy(&connectionInfoEx->PipeList[0],
                   &connectionInfo->PipeList[0],
                   sizeof(USB_PIPE_INFO) * 30);

            FREE(connectionInfo);
        }

        // Update the count of connected devices
        //
        if (connectionInfoEx->ConnectionStatus == DeviceConnected)
        {
            TotalDevicesConnected++;
        }

        if (connectionInfoEx->DeviceIsHub)
        {
            TotalHubs++;
        }

        // If there is a device connected, get the Device Description
        //
        deviceDesc = NULL;
        if (connectionInfoEx->ConnectionStatus != NoDeviceConnected)
        {
            driverKeyName = GetDriverKeyName(hHubDevice,
                                             index);

            if (driverKeyName)
            {
                deviceDesc = DriverNameToDeviceDesc(driverKeyName, FALSE);

                FREE(driverKeyName);
            }
        }

        // If there is a device connected to the port, try to retrieve the
        // Configuration Descriptor from the device.
        //
        if (gDoConfigDesc &&
            connectionInfoEx->ConnectionStatus == DeviceConnected)
        {
            configDesc = GetConfigDescriptor(hHubDevice,
                                             index,
                                             0);
        }
        else
        {
            configDesc = NULL;
        }

        if (configDesc != NULL &&
            AreThereStringDescriptors(&connectionInfoEx->DeviceDescriptor,
                                      (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc+1)))
        {
            stringDescs = GetAllStringDescriptors(
                              hHubDevice,
                              index,
                              &connectionInfoEx->DeviceDescriptor,
                              (PUSB_CONFIGURATION_DESCRIPTOR)(configDesc+1));
        }
        else
        {
            stringDescs = NULL;
        }

        // If the device connected to the port is an external hub, get the
        // name of the external hub and recursively enumerate it.
        //
        if (connectionInfoEx->DeviceIsHub)
        {
            PCHAR extHubName;

            extHubName = GetExternalHubName(hHubDevice,
                                            index);

            if (extHubName != NULL)
            {
                EnumerateHub(hTreeParent, //hPortItem,
                             extHubName,
                             connectionInfoEx,
                             configDesc,
                             stringDescs,
                             deviceDesc);
            }
        }
        else
        {
            // Allocate some space for a USBDEVICEINFO structure to hold the
            // Config Descriptors, Strings Descriptors, and connection info
            // pointers.  GPTR zero initializes the structure for us.
            //
            info = (PUSBDEVICEINFO) ALLOC(sizeof(USBDEVICEINFO));

            if (info == NULL)
            {
                OOPS();
                if (configDesc != NULL)
                {
                    FREE(configDesc);
                }
                FREE(connectionInfoEx);
                break;
            }

            info->DeviceInfoType = DeviceInfo;

            info->ConnectionInfo = connectionInfoEx;

            info->ConfigDesc = configDesc;

            info->StringDescs = stringDescs;

            wsprintf(leafName, "[Port%d] ", index);

            strcat(leafName, ConnectionStatuses[connectionInfoEx->ConnectionStatus]);

            if (deviceDesc)
            {
                strcat(leafName, " :  ");
                strcat(leafName, deviceDesc);
            }

            if (connectionInfoEx->ConnectionStatus == NoDeviceConnected)
            {
                icon = NoDeviceIcon;
            }
            else if (connectionInfoEx->CurrentConfigurationValue)
            {
                icon = GoodDeviceIcon;
            }
            else
            {
                icon = BadDeviceIcon;
            }

            hItem = AddLeaf(hTreeParent, //hPortItem,
                            (LPARAM)info,
                            leafName,
                            icon);
        }
    }
}


//*****************************************************************************
//
// WideStrToMultiStr()
//
//*****************************************************************************

PCHAR WideStrToMultiStr (PWCHAR WideStr)
{
    ULONG nBytes;
    PCHAR MultiStr;

    // Get the length of the converted string
    //
    nBytes = WideCharToMultiByte(
                 CP_ACP,
                 0,
                 WideStr,
                 -1,
                 NULL,
                 0,
                 NULL,
                 NULL);

    if (nBytes == 0)
    {
        return NULL;
    }

    // Allocate space to hold the converted string
    //
    MultiStr = ALLOC(nBytes);

    if (MultiStr == NULL)
    {
        return NULL;
    }

    // Convert the string
    //
    nBytes = WideCharToMultiByte(
                 CP_ACP,
                 0,
                 WideStr,
                 -1,
                 MultiStr,
                 nBytes,
                 NULL,
                 NULL);

    if (nBytes == 0)
    {
        FREE(MultiStr);
        return NULL;
    }

    return MultiStr;
}


//*****************************************************************************
//
// GetRootHubName()
//
//*****************************************************************************

PCHAR GetRootHubName (
    HANDLE HostController
)
{
    BOOL                success;
    ULONG               nBytes;
    USB_ROOT_HUB_NAME   rootHubName;
    PUSB_ROOT_HUB_NAME  rootHubNameW;
    PCHAR               rootHubNameA;

    rootHubNameW = NULL;
    rootHubNameA = NULL;

    // Get the length of the name of the Root Hub attached to the
    // Host Controller
    //
    success = DeviceIoControl(HostController,
                              IOCTL_USB_GET_ROOT_HUB_NAME,
                              0,
                              0,
                              &rootHubName,
                              sizeof(rootHubName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetRootHubNameError;
    }

    // Allocate space to hold the Root Hub name
    //
    nBytes = rootHubName.ActualLength;

    rootHubNameW = ALLOC(nBytes);

    if (rootHubNameW == NULL)
    {
        OOPS();
        goto GetRootHubNameError;
    }

    // Get the name of the Root Hub attached to the Host Controller
    //
    success = DeviceIoControl(HostController,
                              IOCTL_USB_GET_ROOT_HUB_NAME,
                              NULL,
                              0,
                              rootHubNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetRootHubNameError;
    }

    // Convert the Root Hub name
    //
    rootHubNameA = WideStrToMultiStr(rootHubNameW->RootHubName);

    // All done, free the uncoverted Root Hub name and return the
    // converted Root Hub name
    //
    FREE(rootHubNameW);

    return rootHubNameA;


GetRootHubNameError:
    // There was an error, free anything that was allocated
    //
    if (rootHubNameW != NULL)
    {
        FREE(rootHubNameW);
        rootHubNameW = NULL;
    }

    return NULL;
}


//*****************************************************************************
//
// GetExternalHubName()
//
//*****************************************************************************

PCHAR GetExternalHubName (
    HANDLE  Hub,
    ULONG   ConnectionIndex
)
{
    BOOL                        success;
    ULONG                       nBytes;
    USB_NODE_CONNECTION_NAME	extHubName;
    PUSB_NODE_CONNECTION_NAME   extHubNameW;
    PCHAR                       extHubNameA;

    extHubNameW = NULL;
    extHubNameA = NULL;

    // Get the length of the name of the external hub attached to the
    // specified port.
    //
    extHubName.ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              &extHubName,
                              sizeof(extHubName),
                              &extHubName,
                              sizeof(extHubName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetExternalHubNameError;
    }

    // Allocate space to hold the external hub name
    //
    nBytes = extHubName.ActualLength;

    if (nBytes <= sizeof(extHubName))
    {
        OOPS();
        goto GetExternalHubNameError;
    }

    extHubNameW = ALLOC(nBytes);

    if (extHubNameW == NULL)
    {
        OOPS();
        goto GetExternalHubNameError;
    }

    // Get the name of the external hub attached to the specified port
    //
    extHubNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_NAME,
                              extHubNameW,
                              nBytes,
                              extHubNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetExternalHubNameError;
    }

    // Convert the External Hub name
    //
    extHubNameA = WideStrToMultiStr(extHubNameW->NodeName);

    // All done, free the uncoverted external hub name and return the
    // converted external hub name
    //
    FREE(extHubNameW);

    return extHubNameA;


GetExternalHubNameError:
    // There was an error, free anything that was allocated
    //
    if (extHubNameW != NULL)
    {
        FREE(extHubNameW);
        extHubNameW = NULL;
    }

    return NULL;
}


//*****************************************************************************
//
// GetDriverKeyName()
//
//*****************************************************************************

PCHAR GetDriverKeyName (
    HANDLE  Hub,
    ULONG   ConnectionIndex
)
{
    BOOL                                success;
    ULONG                               nBytes;
    USB_NODE_CONNECTION_DRIVERKEY_NAME  driverKeyName;
    PUSB_NODE_CONNECTION_DRIVERKEY_NAME driverKeyNameW;
    PCHAR                               driverKeyNameA;

    driverKeyNameW = NULL;
    driverKeyNameA = NULL;

    // Get the length of the name of the driver key of the device attached to
    // the specified port.
    //
    driverKeyName.ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetDriverKeyNameError;
    }

    // Allocate space to hold the driver key name
    //
    nBytes = driverKeyName.ActualLength;

    if (nBytes <= sizeof(driverKeyName))
    {
        OOPS();
        goto GetDriverKeyNameError;
    }

    driverKeyNameW = ALLOC(nBytes);

    if (driverKeyNameW == NULL)
    {
        OOPS();
        goto GetDriverKeyNameError;
    }

    // Get the name of the driver key of the device attached to
    // the specified port.
    //
    driverKeyNameW->ConnectionIndex = ConnectionIndex;

    success = DeviceIoControl(Hub,
                              IOCTL_USB_GET_NODE_CONNECTION_DRIVERKEY_NAME,
                              driverKeyNameW,
                              nBytes,
                              driverKeyNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetDriverKeyNameError;
    }

    // Convert the driver key name
    //
    driverKeyNameA = WideStrToMultiStr(driverKeyNameW->DriverKeyName);

    // All done, free the uncoverted driver key name and return the
    // converted driver key name
    //
    FREE(driverKeyNameW);

    return driverKeyNameA;


GetDriverKeyNameError:
    // There was an error, free anything that was allocated
    //
    if (driverKeyNameW != NULL)
    {
        FREE(driverKeyNameW);
        driverKeyNameW = NULL;
    }

    return NULL;
}


//*****************************************************************************
//
// GetHCDDriverKeyName()
//
//*****************************************************************************

PCHAR GetHCDDriverKeyName (
    HANDLE  HCD
)
{
    BOOL                    success;
    ULONG                   nBytes;
    USB_HCD_DRIVERKEY_NAME  driverKeyName;
    PUSB_HCD_DRIVERKEY_NAME driverKeyNameW;
    PCHAR                   driverKeyNameA;

    driverKeyNameW = NULL;
    driverKeyNameA = NULL;

    // Get the length of the name of the driver key of the HCD
    //
    success = DeviceIoControl(HCD,
                              IOCTL_GET_HCD_DRIVERKEY_NAME,
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &driverKeyName,
                              sizeof(driverKeyName),
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetHCDDriverKeyNameError;
    }

    // Allocate space to hold the driver key name
    //
    nBytes = driverKeyName.ActualLength;

    if (nBytes <= sizeof(driverKeyName))
    {
        OOPS();
        goto GetHCDDriverKeyNameError;
    }

    driverKeyNameW = ALLOC(nBytes);

    if (driverKeyNameW == NULL)
    {
        OOPS();
        goto GetHCDDriverKeyNameError;
    }

    // Get the name of the driver key of the device attached to
    // the specified port.
    //
    success = DeviceIoControl(HCD,
                              IOCTL_GET_HCD_DRIVERKEY_NAME,
                              driverKeyNameW,
                              nBytes,
                              driverKeyNameW,
                              nBytes,
                              &nBytes,
                              NULL);

    if (!success)
    {
        OOPS();
        goto GetHCDDriverKeyNameError;
    }

    // Convert the driver key name
    //
    driverKeyNameA = WideStrToMultiStr(driverKeyNameW->DriverKeyName);

    // All done, free the uncoverted driver key name and return the
    // converted driver key name
    //
    FREE(driverKeyNameW);

    return driverKeyNameA;


GetHCDDriverKeyNameError:
    // There was an error, free anything that was allocated
    //
    if (driverKeyNameW != NULL)
    {
        FREE(driverKeyNameW);
        driverKeyNameW = NULL;
    }

    return NULL;
}


//*****************************************************************************
//
// GetConfigDescriptor()
//
// hHubDevice - Handle of the hub device containing the port from which the
// Configuration Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the Configuration Descriptor will be requested.
//
// DescriptorIndex - Configuration Descriptor index, zero based.
//
//*****************************************************************************

PUSB_DESCRIPTOR_REQUEST
GetConfigDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex
)
{
    BOOL    success;
    ULONG   nBytes;
    ULONG   nBytesReturned;

    UCHAR   configDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
                             sizeof(USB_CONFIGURATION_DESCRIPTOR)];

    PUSB_DESCRIPTOR_REQUEST         configDescReq;
    PUSB_CONFIGURATION_DESCRIPTOR   configDesc;


    // Request the Configuration Descriptor the first time using our
    // local buffer, which is just big enough for the Cofiguration
    // Descriptor itself.
    //
    nBytes = sizeof(configDescReqBuf);

    configDescReq = (PUSB_DESCRIPTOR_REQUEST)configDescReqBuf;
    configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq+1);

    // Zero fill the entire request structure
    //
    memset(configDescReq, 0, nBytes);

    // Indicate the port from which the descriptor will be requested
    //
    configDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)
                                        | DescriptorIndex;

    configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              configDescReq,
                              nBytes,
                              configDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);

    if (!success)
    {
        OOPS();
        return NULL;
    }

    if (nBytes != nBytesReturned)
    {
        OOPS();
        return NULL;
    }

    if (configDesc->wTotalLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        OOPS();
        return NULL;
    }

    // Now request the entire Configuration Descriptor using a dynamically
    // allocated buffer which is sized big enough to hold the entire descriptor
    //
    nBytes = sizeof(USB_DESCRIPTOR_REQUEST) + configDesc->wTotalLength;

    configDescReq = (PUSB_DESCRIPTOR_REQUEST)ALLOC(nBytes);

    if (configDescReq == NULL)
    {
        OOPS();
        return NULL;
    }

    configDesc = (PUSB_CONFIGURATION_DESCRIPTOR)(configDescReq+1);

    // Indicate the port from which the descriptor will be requested
    //
    configDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    configDescReq->SetupPacket.wValue = (USB_CONFIGURATION_DESCRIPTOR_TYPE << 8)
                                        | DescriptorIndex;

    configDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              configDescReq,
                              nBytes,
                              configDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);

    if (!success)
    {
        OOPS();
        FREE(configDescReq);
        return NULL;
    }

    if (nBytes != nBytesReturned)
    {
        OOPS();
        FREE(configDescReq);
        return NULL;
    }

    if (configDesc->wTotalLength != (nBytes - sizeof(USB_DESCRIPTOR_REQUEST)))
    {
        OOPS();
        FREE(configDescReq);
        return NULL;
    }

    return configDescReq;
}


//*****************************************************************************
//
// AreThereStringDescriptors()
//
// DeviceDesc - Device Descriptor for which String Descriptors should be
// checked.
//
// ConfigDesc - Configuration Descriptor (also containing Interface Descriptor)
// for which String Descriptors should be checked.
//
//*****************************************************************************

BOOL
AreThereStringDescriptors (
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
{
    PUCHAR                  descEnd;
    PUSB_COMMON_DESCRIPTOR  commonDesc;

    //
    // Check Device Descriptor strings
    //

    if (DeviceDesc->iManufacturer ||
        DeviceDesc->iProduct      ||
        DeviceDesc->iSerialNumber
       )
    {
        return TRUE;
    }


    //
    // Check the Configuration and Interface Descriptor strings
    //

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
           (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    OOPS();
                    break;
                }
                if (((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
                {
                    return TRUE;
                }
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
                    commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
                {
                    OOPS();
                    break;
                }
                if (((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
                {
                    return TRUE;
                }
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;

            default:
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;
        }
        break;
    }

    return FALSE;
}


//*****************************************************************************
//
// GetAllStringDescriptors()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptors will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptors will be requested.
//
// DeviceDesc - Device Descriptor for which String Descriptors should be
// requested.
//
// ConfigDesc - Configuration Descriptor (also containing Interface Descriptor)
// for which String Descriptors should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE
GetAllStringDescriptors (
    HANDLE                          hHubDevice,
    ULONG                           ConnectionIndex,
    PUSB_DEVICE_DESCRIPTOR          DeviceDesc,
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
{
    PSTRING_DESCRIPTOR_NODE supportedLanguagesString;
    PSTRING_DESCRIPTOR_NODE stringDescNodeTail;
    ULONG                   numLanguageIDs;
    USHORT                  *languageIDs;

    PUCHAR                  descEnd;
    PUSB_COMMON_DESCRIPTOR  commonDesc;

    //
    // Get the array of supported Language IDs, which is returned
    // in String Descriptor 0
    //
    supportedLanguagesString = GetStringDescriptor(hHubDevice,
                                                   ConnectionIndex,
                                                   0,
                                                   0);

    if (supportedLanguagesString == NULL)
    {
        return NULL;
    }

    numLanguageIDs = (supportedLanguagesString->StringDescriptor->bLength - 2) / 2;

    languageIDs = &supportedLanguagesString->StringDescriptor->bString[0];

    stringDescNodeTail = supportedLanguagesString;

    //
    // Get the Device Descriptor strings
    //

    if (DeviceDesc->iManufacturer)
    {
        stringDescNodeTail = GetStringDescriptors(hHubDevice,
                                                  ConnectionIndex,
                                                  DeviceDesc->iManufacturer,
                                                  numLanguageIDs,
                                                  languageIDs,
                                                  stringDescNodeTail);
    }

    if (DeviceDesc->iProduct)
    {
        stringDescNodeTail = GetStringDescriptors(hHubDevice,
                                                  ConnectionIndex,
                                                  DeviceDesc->iProduct,
                                                  numLanguageIDs,
                                                  languageIDs,
                                                  stringDescNodeTail);
    }

    if (DeviceDesc->iSerialNumber)
    {
        stringDescNodeTail = GetStringDescriptors(hHubDevice,
                                                  ConnectionIndex,
                                                  DeviceDesc->iSerialNumber,
                                                  numLanguageIDs,
                                                  languageIDs,
                                                  stringDescNodeTail);
    }


    //
    // Get the Configuration and Interface Descriptor strings
    //

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
           (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    OOPS();
                    break;
                }
                if (((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration)
                {
                    stringDescNodeTail = GetStringDescriptors(
                                             hHubDevice,
                                             ConnectionIndex,
                                             ((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc)->iConfiguration,
                                             numLanguageIDs,
                                             languageIDs,
                                             stringDescNodeTail);
                }
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR) &&
                    commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2))
                {
                    OOPS();
                    break;
                }
                if (((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface)
                {
                    stringDescNodeTail = GetStringDescriptors(
                                             hHubDevice,
                                             ConnectionIndex,
                                             ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->iInterface,
                                             numLanguageIDs,
                                             languageIDs,
                                             stringDescNodeTail);
                }
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;

            default:
                (PUCHAR)commonDesc += commonDesc->bLength;
                continue;
        }
        break;
    }

    return supportedLanguagesString;
}


//*****************************************************************************
//
// GetStringDescriptor()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptor will be requested.
//
// DescriptorIndex - String Descriptor index.
//
// LanguageID - Language in which the string should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE
GetStringDescriptor (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    USHORT  LanguageID
)
{
    BOOL    success;
    ULONG   nBytes;
    ULONG   nBytesReturned;

    UCHAR   stringDescReqBuf[sizeof(USB_DESCRIPTOR_REQUEST) +
                             MAXIMUM_USB_STRING_LENGTH];

    PUSB_DESCRIPTOR_REQUEST stringDescReq;
    PUSB_STRING_DESCRIPTOR  stringDesc;
    PSTRING_DESCRIPTOR_NODE stringDescNode;

    nBytes = sizeof(stringDescReqBuf);

    stringDescReq = (PUSB_DESCRIPTOR_REQUEST)stringDescReqBuf;
    stringDesc = (PUSB_STRING_DESCRIPTOR)(stringDescReq+1);

    // Zero fill the entire request structure
    //
    memset(stringDescReq, 0, nBytes);

    // Indicate the port from which the descriptor will be requested
    //
    stringDescReq->ConnectionIndex = ConnectionIndex;

    //
    // USBHUB uses URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE to process this
    // IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION request.
    //
    // USBD will automatically initialize these fields:
    //     bmRequest = 0x80
    //     bRequest  = 0x06
    //
    // We must inititialize these fields:
    //     wValue    = Descriptor Type (high) and Descriptor Index (low byte)
    //     wIndex    = Zero (or Language ID for String Descriptors)
    //     wLength   = Length of descriptor buffer
    //
    stringDescReq->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8)
                                        | DescriptorIndex;

    stringDescReq->SetupPacket.wIndex = LanguageID;

    stringDescReq->SetupPacket.wLength = (USHORT)(nBytes - sizeof(USB_DESCRIPTOR_REQUEST));

    // Now issue the get descriptor request.
    //
    success = DeviceIoControl(hHubDevice,
                              IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
                              stringDescReq,
                              nBytes,
                              stringDescReq,
                              nBytes,
                              &nBytesReturned,
                              NULL);

    //
    // Do some sanity checks on the return from the get descriptor request.
    //

    if (!success)
    {
        OOPS();
        return NULL;
    }

    if (nBytesReturned < 2)
    {
        OOPS();
        return NULL;
    }

    if (stringDesc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE)
    {
        OOPS();
        return NULL;
    }

    if (stringDesc->bLength != nBytesReturned - sizeof(USB_DESCRIPTOR_REQUEST))
    {
        OOPS();
        return NULL;
    }

    if (stringDesc->bLength % 2 != 0)
    {
        OOPS();
        return NULL;
    }

    //
    // Looks good, allocate some (zero filled) space for the string descriptor
    // node and copy the string descriptor to it.
    //

    stringDescNode = (PSTRING_DESCRIPTOR_NODE)ALLOC(sizeof(STRING_DESCRIPTOR_NODE) +
                                                    stringDesc->bLength);

    if (stringDescNode == NULL)
    {
        OOPS();
        return NULL;
    }

    stringDescNode->DescriptorIndex = DescriptorIndex;
    stringDescNode->LanguageID = LanguageID;

    memcpy(stringDescNode->StringDescriptor,
           stringDesc,
           stringDesc->bLength);

    return stringDescNode;
}


//*****************************************************************************
//
// GetStringDescriptors()
//
// hHubDevice - Handle of the hub device containing the port from which the
// String Descriptor will be requested.
//
// ConnectionIndex - Identifies the port on the hub to which a device is
// attached from which the String Descriptor will be requested.
//
// DescriptorIndex - String Descriptor index.
//
// NumLanguageIDs -  Number of languages in which the string should be
// requested.
//
// LanguageIDs - Languages in which the string should be requested.
//
//*****************************************************************************

PSTRING_DESCRIPTOR_NODE
GetStringDescriptors (
    HANDLE  hHubDevice,
    ULONG   ConnectionIndex,
    UCHAR   DescriptorIndex,
    ULONG   NumLanguageIDs,
    USHORT  *LanguageIDs,
    PSTRING_DESCRIPTOR_NODE StringDescNodeTail
)
{
    ULONG i;

    for (i=0; i<NumLanguageIDs; i++)
    {
        StringDescNodeTail->Next = GetStringDescriptor(hHubDevice,
                                                       ConnectionIndex,
                                                       DescriptorIndex,
                                                       *LanguageIDs);

        if (StringDescNodeTail->Next)
        {
            StringDescNodeTail = StringDescNodeTail->Next;
        }

        LanguageIDs++;
    }

    return StringDescNodeTail;
}

//*****************************************************************************
//
// CleanupItem()
//
//*****************************************************************************

VOID
CleanupItem (
    HWND      hTreeWnd,
    HTREEITEM hTreeItem
)
{
    TV_ITEM tvi;
    PVOID   info;

    tvi.mask = TVIF_HANDLE | TVIF_PARAM;
    tvi.hItem = hTreeItem;

    TreeView_GetItem(hTreeWnd,
                     &tvi);

    info = (PVOID)tvi.lParam;

    if (info)
    {
        PCHAR                               DriverKey = NULL;
        PUSB_NODE_INFORMATION               HubInfo = NULL;
        PCHAR                               HubName = NULL;
        PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfoEx = NULL;
        PUSB_DESCRIPTOR_REQUEST             ConfigDesc = NULL;
        PSTRING_DESCRIPTOR_NODE             StringDescs = NULL;

        switch (*(PUSBDEVICEINFOTYPE)info)
        {
            case HostControllerInfo:
                //
                // Remove this host controller from the list of enumerated
                // host controllers.
                //
                RemoveEntryList(&((PUSBHOSTCONTROLLERINFO)info)->ListEntry);

                DriverKey = ((PUSBHOSTCONTROLLERINFO)info)->DriverKey;
                break;

            case RootHubInfo:
                HubInfo = ((PUSBROOTHUBINFO)info)->HubInfo;
                HubName = ((PUSBROOTHUBINFO)info)->HubName;
                break;

            case ExternalHubInfo:
                HubInfo = ((PUSBEXTERNALHUBINFO)info)->HubInfo;
                HubName = ((PUSBEXTERNALHUBINFO)info)->HubName;
                ConnectionInfoEx = ((PUSBEXTERNALHUBINFO)info)->ConnectionInfo;
                ConfigDesc = ((PUSBEXTERNALHUBINFO)info)->ConfigDesc;
                StringDescs = ((PUSBEXTERNALHUBINFO)info)->StringDescs;
                break;

            case DeviceInfo:
                ConnectionInfoEx = ((PUSBDEVICEINFO)info)->ConnectionInfo;
                ConfigDesc = ((PUSBDEVICEINFO)info)->ConfigDesc;
                StringDescs = ((PUSBDEVICEINFO)info)->StringDescs;
                break;
        }

        if (DriverKey)
        {
            FREE(DriverKey);
        }

        if (HubInfo)
        {
            FREE(HubInfo);
        }

        if (HubName)
        {
            FREE(HubName);
        }

        if (ConfigDesc)
        {
            FREE(ConfigDesc);
        }

        if (StringDescs)
        {
            PSTRING_DESCRIPTOR_NODE Next;

            do {

                Next = StringDescs->Next;
                FREE(StringDescs);
                StringDescs = Next;

            } while (StringDescs);
        }

        if (ConnectionInfoEx)
        {
            FREE(ConnectionInfoEx);
        }

        FREE(info);
    }
}

