/*++

Copyright (c) 1997-1998 Microsoft Corporation

Module Name:

DISPLAY.C

Abstract:

This source file contains the routines which update the edit control
to display information about the selected USB device.

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

#include "vndrlist.h"
#include "usbview.h"

//*****************************************************************************
// D E F I N E S
//*****************************************************************************

#define BUFFERALLOCINCREMENT        8192
#define BUFFERMINFREESPACE          1024

//*****************************************************************************
// T Y P E D E F S
//*****************************************************************************

//*****************************************************************************
// G L O B A L S    P R I V A T E    T O    T H I S    F I L E
//*****************************************************************************

// Workspace for text info which is used to update the edit control
//
CHAR *TextBuffer = NULL;
int   TextBufferLen = 0;
int   TextBufferPos = 0;

//*****************************************************************************
// L O C A L    F U N C T I O N    P R O T O T Y P E S
//*****************************************************************************

VOID
DisplayHubInfo (
    PUSB_HUB_INFORMATION HubInfo
);

VOID
DisplayConnectionInfo (
    PUSB_NODE_CONNECTION_INFORMATION_EX    ConnectInfo,
    PSTRING_DESCRIPTOR_NODE             StringDescs
);

VOID
DisplayPipeInfo (
    ULONG           NumPipes,
    USB_PIPE_INFO  *PipeInfo
);

VOID
DisplayConfigDesc (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
    PSTRING_DESCRIPTOR_NODE         StringDescs
);

VOID
DisplayConfigurationDescriptor (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
    PSTRING_DESCRIPTOR_NODE         StringDescs
);

VOID
DisplayInterfaceDescriptor (
    PUSB_INTERFACE_DESCRIPTOR   InterfaceDesc,
    PSTRING_DESCRIPTOR_NODE     StringDescs
);

VOID
DisplayEndpointDescriptor (
    PUSB_ENDPOINT_DESCRIPTOR    EndpointDesc
);

VOID
DisplayHidDescriptor (
    PUSB_HID_DESCRIPTOR         HidDesc
);

VOID
DisplayStringDescriptor (
    UCHAR                       Index,
    PSTRING_DESCRIPTOR_NODE     StringDescs
);

VOID
DisplayUnknownDescriptor (
    PUSB_COMMON_DESCRIPTOR      CommonDesc
);

PCHAR
GetVendorString (
    USHORT     idVendor
);

//*****************************************************************************
// L O C A L    F U N C T I O N S
//*****************************************************************************


//*****************************************************************************
//
// CreateTextBuffer()
//
//*****************************************************************************

BOOL
CreateTextBuffer (
)
{
    // Allocate the buffer
    //
    TextBuffer = ALLOC(BUFFERALLOCINCREMENT);

    if (TextBuffer == NULL)
    {
        OOPS();

        return FALSE;
    }

    TextBufferLen = BUFFERALLOCINCREMENT;

    // Reset the buffer position and terminate the buffer
    //
    *TextBuffer = 0;
    TextBufferPos = 0;

    return TRUE;
}


//*****************************************************************************
//
// DestroyTextBuffer()
//
//*****************************************************************************

VOID
DestroyTextBuffer (
)
{
    if (TextBuffer != NULL)
    {
        FREE(TextBuffer);

        TextBuffer = NULL;
    }
}


//*****************************************************************************
//
// ResetTextBuffer()
//
//*****************************************************************************

BOOL
ResetTextBuffer (
)
{
    // Fail if the text buffer has not been allocated
    //
    if (TextBuffer == NULL)
    {
        OOPS();

        return FALSE;
    }

    // Reset the buffer position and terminate the buffer
    //
    *TextBuffer = 0;
    TextBufferPos = 0;

    return TRUE;
}

//*****************************************************************************
//
// AppendTextBuffer()
//
//*****************************************************************************

VOID __cdecl
AppendTextBuffer (
    LPCTSTR lpFormat,
    ...
)
{
    va_list arglist;

    va_start(arglist, lpFormat);

    // Make sure we have a healthy amount of space free in the buffer,
    // reallocating the buffer if necessary.
    //
    if (TextBufferLen - TextBufferPos < BUFFERMINFREESPACE)
    {
        CHAR *TextBufferTmp;

        TextBufferTmp = REALLOC(TextBuffer, TextBufferLen+BUFFERALLOCINCREMENT);

        if (TextBufferTmp != NULL)
        {
            TextBuffer = TextBufferTmp;
            TextBufferLen += BUFFERALLOCINCREMENT;
        }
        else
        {
            // If GlobalReAlloc fails, the original memory is not freed,
            // and the original handle and pointer are still valid.
            //
            OOPS();

            return;
        }
    }

    // Add the text to the end of the buffer, updating the buffer position.
    //
    TextBufferPos += wvsprintf(TextBuffer + TextBufferPos,
                               lpFormat,
                               arglist);
}


//
// Hardcoded information about specific EHCI controllers
//
typedef struct _EHCI_CONTROLLER_DATA
{
    USHORT  VendorID;
    USHORT  DeviceID;
    UCHAR   DebugPortNumber;
} EHCI_CONTROLLER_DATA, *PEHCI_CONTROLLER_DATA;

EHCI_CONTROLLER_DATA EhciControllerData[] =
{
    {0x8086, 0x24CD, 1},
    {0x8086, 0x24DD, 1},
    {0x10DE, 0x00D8, 1},
    {0,0,0},
};


//*****************************************************************************
//
// UpdateEditControl()
//
// hTreeItem - Handle of selected TreeView item for which information should
// be displayed in the edit control.
//
//*****************************************************************************

VOID
UpdateEditControl (
    HWND      hEditWnd,
    HWND      hTreeWnd,
    HTREEITEM hTreeItem
)
{
    TV_ITEM tvi;
    PVOID   info;
    ULONG   i;

    // Start with an empty text buffer.
    //
    if (!ResetTextBuffer())
    {
        return;
    }

    //
    // Get the name of the TreeView item, along with the a pointer to the
    // info we stored about the item in the item's lParam.
    //

    tvi.mask = TVIF_HANDLE | TVIF_TEXT | TVIF_PARAM;
    tvi.hItem = hTreeItem;
    tvi.pszText = (LPSTR)TextBuffer;
    tvi.cchTextMax = TextBufferLen-2;  // leave space for "\r\n'

    TreeView_GetItem(hTreeWnd,
                     &tvi);

    info = (PVOID)tvi.lParam;

    //
    // If we didn't store any info for the item, just display the item's
    // name, else display the info we stored for the item.
    //
    if (info)
    {
        PUSB_NODE_INFORMATION               HubInfo = NULL;
        PCHAR                               HubName = NULL;
        PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo = NULL;
        PUSB_DESCRIPTOR_REQUEST             ConfigDesc = NULL;
        PSTRING_DESCRIPTOR_NODE             StringDescs = NULL;

        switch (*(PUSBDEVICEINFOTYPE)info)
        {
            case HostControllerInfo:
                AppendTextBuffer("DriverKey: %s\r\n",
                                 ((PUSBHOSTCONTROLLERINFO)info)->DriverKey);

                AppendTextBuffer("VendorID: %04X\r\n",
                                 ((PUSBHOSTCONTROLLERINFO)info)->VendorID);

                AppendTextBuffer("DeviceID: %04X\r\n",
                                 ((PUSBHOSTCONTROLLERINFO)info)->DeviceID);

                AppendTextBuffer("SubSysID: %08X\r\n",
                                 ((PUSBHOSTCONTROLLERINFO)info)->SubSysID);

                AppendTextBuffer("Revision: %02X\r\n",
                                 ((PUSBHOSTCONTROLLERINFO)info)->Revision);

                for (i = 0; EhciControllerData[i].VendorID; i++)
                {
                    if (((PUSBHOSTCONTROLLERINFO)info)->VendorID ==
                          EhciControllerData[i].VendorID &&
                        ((PUSBHOSTCONTROLLERINFO)info)->DeviceID ==
                          EhciControllerData[i].DeviceID)
                    {
                        AppendTextBuffer("DebugPort: %d\r\n",
                                         EhciControllerData[i].DebugPortNumber);
                    }
                }

                break;

            case RootHubInfo:
                HubInfo = ((PUSBROOTHUBINFO)info)->HubInfo;
                HubName = ((PUSBROOTHUBINFO)info)->HubName;

                AppendTextBuffer("Root Hub: %s\r\n",
                                 HubName);

                break;

            case ExternalHubInfo:
                HubInfo = ((PUSBEXTERNALHUBINFO)info)->HubInfo;
                HubName = ((PUSBEXTERNALHUBINFO)info)->HubName;
                ConnectionInfo = ((PUSBEXTERNALHUBINFO)info)->ConnectionInfo;
                ConfigDesc = ((PUSBEXTERNALHUBINFO)info)->ConfigDesc;
                StringDescs = ((PUSBEXTERNALHUBINFO)info)->StringDescs;

                AppendTextBuffer("External Hub: %s\r\n",
                                 HubName);

                break;

            case DeviceInfo:
                ConnectionInfo = ((PUSBDEVICEINFO)info)->ConnectionInfo;
                ConfigDesc = ((PUSBDEVICEINFO)info)->ConfigDesc;
                StringDescs = ((PUSBDEVICEINFO)info)->StringDescs;
                break;
        }

        if (HubInfo)
        {
            DisplayHubInfo(&HubInfo->u.HubInformation);
        }

        if (ConnectionInfo)
        {
            DisplayConnectionInfo(ConnectionInfo,
                                  StringDescs);
        }

        if (ConfigDesc)
        {
            DisplayConfigDesc((PUSB_CONFIGURATION_DESCRIPTOR)(ConfigDesc + 1),
                              StringDescs);
        }
    }

    // All done formatting text buffer with info, now update the edit
    // control with the contents of the text buffer
    //
    SetWindowText(hEditWnd, TextBuffer);
}


//*****************************************************************************
//
// DisplayHubInfo()
//
// HubInfo - Info about the hub.
//
//*****************************************************************************

VOID
DisplayHubInfo (
    PUSB_HUB_INFORMATION    HubInfo
)
{

    USHORT wHubChar;

    AppendTextBuffer("Hub Power:               %s\r\n",
                     HubInfo->HubIsBusPowered ?
                     "Bus Power" : "Self Power");

    AppendTextBuffer("Number of Ports:         %d\r\n",
                     HubInfo->HubDescriptor.bNumberOfPorts);

    wHubChar = HubInfo->HubDescriptor.wHubCharacteristics;

    switch (wHubChar & 0x0003)
    {
        case 0x0000:
            AppendTextBuffer("Power switching:         Ganged\r\n");
            break;

        case 0x0001:
            AppendTextBuffer("Power switching:         Individual\r\n");
            break;

        case 0x0002:
        case 0x0003:
            AppendTextBuffer("Power switching:         None\r\n");
            break;
    }

    switch (wHubChar & 0x0004)
    {
        case 0x0000:
            AppendTextBuffer("Compound device:         No\r\n");
            break;

        case 0x0004:
            AppendTextBuffer("Compound device:         Yes\r\n");
            break;
    }

    switch (wHubChar & 0x0018)
    {
        case 0x0000:
            AppendTextBuffer("Over-current Protection: Global\r\n");
            break;

        case 0x0008:
            AppendTextBuffer("Over-current Protection: Individual\r\n");
            break;

        case 0x0010:
        case 0x0018:
            AppendTextBuffer("No Over-current Protection (Bus Power Only)\r\n");
            break;
    }

    AppendTextBuffer("\r\n");

}

//*****************************************************************************
//
// DisplayConnectionInfo()
//
// ConnectInfo - Info about the connection.
//
//*****************************************************************************

VOID
DisplayConnectionInfo (
    PUSB_NODE_CONNECTION_INFORMATION_EX    ConnectInfo,
    PSTRING_DESCRIPTOR_NODE             StringDescs
)
{

    if (ConnectInfo->ConnectionStatus == NoDeviceConnected)
    {
        AppendTextBuffer("ConnectionStatus: NoDeviceConnected\r\n");
    }
    else
    {
        PCHAR VendorString;

        AppendTextBuffer("Device Descriptor:\r\n");

        AppendTextBuffer("bcdUSB:             0x%04X\r\n",
                         ConnectInfo->DeviceDescriptor.bcdUSB);

        AppendTextBuffer("bDeviceClass:         0x%02X\r\n",
                         ConnectInfo->DeviceDescriptor.bDeviceClass);

        AppendTextBuffer("bDeviceSubClass:      0x%02X\r\n",
                         ConnectInfo->DeviceDescriptor.bDeviceSubClass);

        AppendTextBuffer("bDeviceProtocol:      0x%02X\r\n",
                         ConnectInfo->DeviceDescriptor.bDeviceProtocol);

        AppendTextBuffer("bMaxPacketSize0:      0x%02X (%d)\r\n",
                         ConnectInfo->DeviceDescriptor.bMaxPacketSize0,
                         ConnectInfo->DeviceDescriptor.bMaxPacketSize0);

        VendorString = GetVendorString(ConnectInfo->DeviceDescriptor.idVendor);

        if (VendorString != NULL)
        {
            AppendTextBuffer("idVendor:           0x%04X (%s)\r\n",
                             ConnectInfo->DeviceDescriptor.idVendor,
                             VendorString);
        }
        else
        {
            AppendTextBuffer("idVendor:           0x%04X\r\n",
                             ConnectInfo->DeviceDescriptor.idVendor);
        }

        AppendTextBuffer("idProduct:          0x%04X\r\n",
                         ConnectInfo->DeviceDescriptor.idProduct);

        AppendTextBuffer("bcdDevice:          0x%04X\r\n",
                         ConnectInfo->DeviceDescriptor.bcdDevice);

        AppendTextBuffer("iManufacturer:        0x%02X\r\n",
                         ConnectInfo->DeviceDescriptor.iManufacturer);

        if (ConnectInfo->DeviceDescriptor.iManufacturer)
        {
            DisplayStringDescriptor(ConnectInfo->DeviceDescriptor.iManufacturer,
                                    StringDescs);
        }

        AppendTextBuffer("iProduct:             0x%02X\r\n",
                         ConnectInfo->DeviceDescriptor.iProduct);

        if (ConnectInfo->DeviceDescriptor.iProduct)
        {
            DisplayStringDescriptor(ConnectInfo->DeviceDescriptor.iProduct,
                                    StringDescs);
        }

        AppendTextBuffer("iSerialNumber:        0x%02X\r\n",
                         ConnectInfo->DeviceDescriptor.iSerialNumber);

        if (ConnectInfo->DeviceDescriptor.iSerialNumber)
        {
            DisplayStringDescriptor(ConnectInfo->DeviceDescriptor.iSerialNumber,
                                    StringDescs);
        }

        AppendTextBuffer("bNumConfigurations:   0x%02X\r\n",
                         ConnectInfo->DeviceDescriptor.bNumConfigurations);

        AppendTextBuffer("\r\nConnectionStatus: %s\r\n",
                         ConnectionStatuses[ConnectInfo->ConnectionStatus]);

        AppendTextBuffer("Current Config Value: 0x%02X\r\n",
                         ConnectInfo->CurrentConfigurationValue);

	switch	(ConnectInfo->Speed){
		case UsbLowSpeed:
			AppendTextBuffer("Device Bus Speed:     Low\r\n");
			break;
		case UsbFullSpeed:
			AppendTextBuffer("Device Bus Speed:     Full\r\n");
			break;
		case UsbHighSpeed:
			AppendTextBuffer("Device Bus Speed:     High\r\n");
			break;
		default:
			AppendTextBuffer("Device Bus Speed:     Unknown\r\n");

	}

        AppendTextBuffer("Device Address:       0x%02X\r\n",
                         ConnectInfo->DeviceAddress);

        AppendTextBuffer("Open Pipes:             %2d\r\n",
                         ConnectInfo->NumberOfOpenPipes);

        if (ConnectInfo->NumberOfOpenPipes)
        {
            DisplayPipeInfo(ConnectInfo->NumberOfOpenPipes,
                            ConnectInfo->PipeList);
        }
    }
}

//*****************************************************************************
//
// DisplayPipeInfo()
//
// NumPipes - Number of pipe for we info should be displayed.
//
// PipeInfo - Info about the pipes.
//
//*****************************************************************************

VOID
DisplayPipeInfo (
    ULONG           NumPipes,
    USB_PIPE_INFO  *PipeInfo
)
{
    ULONG i;

    for (i=0; i<NumPipes; i++)
    {
        DisplayEndpointDescriptor(&PipeInfo[i].EndpointDescriptor);
    }

}


//*****************************************************************************
//
// DisplayConfigDesc()
//
// ConfigDesc - The Configuration Descriptor, and associated Interface and
// EndpointDescriptors
//
//*****************************************************************************

VOID
DisplayConfigDesc (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
    PSTRING_DESCRIPTOR_NODE         StringDescs
)
{
    PUCHAR                  descEnd;
    PUSB_COMMON_DESCRIPTOR  commonDesc;
    UCHAR                   bInterfaceClass;
    UCHAR                   bInterfaceSubClass;
    BOOL                    displayUnknown;

    bInterfaceClass = 0;

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
           (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        displayUnknown = FALSE;

        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    OOPS();
                    displayUnknown = TRUE;
                    break;
                }
                DisplayConfigurationDescriptor((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc,
                                               StringDescs);
                break;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if ((commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR)) &&
                    (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR2)))
                {
                    OOPS();
                    displayUnknown = TRUE;
                    break;
                }
                bInterfaceClass = ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->bInterfaceClass;
                bInterfaceSubClass = ((PUSB_INTERFACE_DESCRIPTOR)commonDesc)->bInterfaceSubClass;

                DisplayInterfaceDescriptor((PUSB_INTERFACE_DESCRIPTOR)commonDesc,
                                           StringDescs);
                break;

            case USB_ENDPOINT_DESCRIPTOR_TYPE:
                if ((commonDesc->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR)) &&
                    (commonDesc->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR2)))
                {
                    OOPS();
                    displayUnknown = TRUE;
                    break;
                }
                DisplayEndpointDescriptor((PUSB_ENDPOINT_DESCRIPTOR)commonDesc);
                break;

            case USB_HID_DESCRIPTOR_TYPE:
                if (commonDesc->bLength < sizeof(USB_HID_DESCRIPTOR))
                {
                    OOPS();
                    displayUnknown = TRUE;
                    break;
                }
                DisplayHidDescriptor((PUSB_HID_DESCRIPTOR)commonDesc);
                break;

            default:
                switch (bInterfaceClass)
                {
                    case USB_DEVICE_CLASS_AUDIO:
                        displayUnknown = !DisplayAudioDescriptor(
                                              (PUSB_AUDIO_COMMON_DESCRIPTOR)commonDesc,
                                              bInterfaceSubClass);
                        break;

                    default:
                        displayUnknown = TRUE;
                        break;
                }
                break;
        }

        if (displayUnknown)
        {
            DisplayUnknownDescriptor(commonDesc);
        }

        (PUCHAR)commonDesc += commonDesc->bLength;
    }
}


//*****************************************************************************
//
// DisplayConfigurationDescriptor()
//
//*****************************************************************************

VOID
DisplayConfigurationDescriptor (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc,
    PSTRING_DESCRIPTOR_NODE         StringDescs
)
{

    AppendTextBuffer("\r\nConfiguration Descriptor:\r\n");

    AppendTextBuffer("wTotalLength:       0x%04X\r\n",
                     ConfigDesc->wTotalLength);

    AppendTextBuffer("bNumInterfaces:       0x%02X\r\n",
                     ConfigDesc->bNumInterfaces);

    AppendTextBuffer("bConfigurationValue:  0x%02X\r\n",
                     ConfigDesc->bConfigurationValue);

    AppendTextBuffer("iConfiguration:       0x%02X\r\n",
                     ConfigDesc->iConfiguration);

    if (ConfigDesc->iConfiguration)
    {
        DisplayStringDescriptor(ConfigDesc->iConfiguration,
                                StringDescs);
    }

    AppendTextBuffer("bmAttributes:         0x%02X (",
                     ConfigDesc->bmAttributes);

    if (ConfigDesc->bmAttributes & 0x80)
    {
        AppendTextBuffer("Bus Powered ");
    }

    if (ConfigDesc->bmAttributes & 0x40)
    {
        AppendTextBuffer("Self Powered ");
    }

    if (ConfigDesc->bmAttributes & 0x20)
    {
        AppendTextBuffer("Remote Wakeup");
    }

    AppendTextBuffer(")\r\n");

    AppendTextBuffer("MaxPower:             0x%02X (%d Ma)\r\n",
                     ConfigDesc->MaxPower,
                     ConfigDesc->MaxPower * 2);

}

//*****************************************************************************
//
// DisplayInterfaceDescriptor()
//
//*****************************************************************************

VOID
DisplayInterfaceDescriptor (
    PUSB_INTERFACE_DESCRIPTOR   InterfaceDesc,
    PSTRING_DESCRIPTOR_NODE     StringDescs
)
{
    PCHAR pStr;

    AppendTextBuffer("\r\nInterface Descriptor:\r\n");

    AppendTextBuffer("bInterfaceNumber:     0x%02X\r\n",
                     InterfaceDesc->bInterfaceNumber);

    AppendTextBuffer("bAlternateSetting:    0x%02X\r\n",
                     InterfaceDesc->bAlternateSetting);

    AppendTextBuffer("bNumEndpoints:        0x%02X\r\n",
                     InterfaceDesc->bNumEndpoints);

    AppendTextBuffer("bInterfaceClass:      0x%02X",
                     InterfaceDesc->bInterfaceClass);

    pStr = "\r\n";

    switch (InterfaceDesc->bInterfaceClass)
    {
        case USB_DEVICE_CLASS_AUDIO:
            pStr = " (Audio)\r\n";
            break;

        case USB_DEVICE_CLASS_HUMAN_INTERFACE:
            pStr = " (HID)\r\n";
            break;

        case USB_DEVICE_CLASS_HUB:
            pStr = " (Hub)\r\n";
            break;

        default:
            break;
    }

    AppendTextBuffer(pStr);

    AppendTextBuffer("bInterfaceSubClass:   0x%02X",
                     InterfaceDesc->bInterfaceSubClass);

    pStr = "\r\n";

    switch (InterfaceDesc->bInterfaceClass)
    {
        case USB_DEVICE_CLASS_AUDIO:
            switch (InterfaceDesc->bInterfaceSubClass)
            {
                case USB_AUDIO_SUBCLASS_AUDIOCONTROL:
                    pStr = " (Audio Control)\r\n";
                    break;

                case USB_AUDIO_SUBCLASS_AUDIOSTREAMING:
                    pStr = " (Audio Streaming)\r\n";
                    break;

                case USB_AUDIO_SUBCLASS_MIDISTREAMING:
                    pStr = " (MIDI Streaming)\r\n";
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }

    AppendTextBuffer(pStr);

    AppendTextBuffer("bInterfaceProtocol:   0x%02X\r\n",
                     InterfaceDesc->bInterfaceProtocol);

    AppendTextBuffer("iInterface:           0x%02X\r\n",
                     InterfaceDesc->iInterface);

    if (InterfaceDesc->iInterface)
    {
        DisplayStringDescriptor(InterfaceDesc->iInterface,
                                StringDescs);
    }

    if (InterfaceDesc->bLength == sizeof(USB_INTERFACE_DESCRIPTOR2))
    {
        PUSB_INTERFACE_DESCRIPTOR2 interfaceDesc2;

        interfaceDesc2 = (PUSB_INTERFACE_DESCRIPTOR2)InterfaceDesc;

        AppendTextBuffer("wNumClasses:        0x%04X\r\n",
                         interfaceDesc2->wNumClasses);
    }

}

//*****************************************************************************
//
// DisplayEndpointDescriptor()
//
//*****************************************************************************

VOID
DisplayEndpointDescriptor (
    PUSB_ENDPOINT_DESCRIPTOR    EndpointDesc
)
{

    AppendTextBuffer("\r\nEndpoint Descriptor:\r\n");

    if (USB_ENDPOINT_DIRECTION_IN(EndpointDesc->bEndpointAddress))
    {
        AppendTextBuffer("bEndpointAddress:     0x%02X  IN\r\n",
               EndpointDesc->bEndpointAddress);
    }
    else
    {
        AppendTextBuffer("bEndpointAddress:     0x%02X  OUT\r\n",
               EndpointDesc->bEndpointAddress);
    }

    switch (EndpointDesc->bmAttributes & 0x03)
    {
        case 0x00:
            AppendTextBuffer("Transfer Type:     Control\r\n");
            break;

        case 0x01:
            AppendTextBuffer("Transfer Type: Isochronous\r\n");
            break;

        case 0x02:
            AppendTextBuffer("Transfer Type:        Bulk\r\n");
            break;

        case 0x03:
            AppendTextBuffer("Transfer Type:   Interrupt\r\n");
            break;

    }

    AppendTextBuffer("wMaxPacketSize:     0x%04X (%d)\r\n",
                     EndpointDesc->wMaxPacketSize,
                     EndpointDesc->wMaxPacketSize);

    if (EndpointDesc->bLength == sizeof(USB_ENDPOINT_DESCRIPTOR))
    {
        AppendTextBuffer("bInterval:            0x%02X\r\n",
                         EndpointDesc->bInterval);
    }
    else
    {
        PUSB_ENDPOINT_DESCRIPTOR2 endpointDesc2;

        endpointDesc2 = (PUSB_ENDPOINT_DESCRIPTOR2)EndpointDesc;

        AppendTextBuffer("wInterval:          0x%04X\r\n",
                         endpointDesc2->wInterval);

        AppendTextBuffer("bSyncAddress:         0x%02X\r\n",
                         endpointDesc2->bSyncAddress);
    }

}


//*****************************************************************************
//
// DisplayHidDescriptor()
//
//*****************************************************************************

VOID
DisplayHidDescriptor (
    PUSB_HID_DESCRIPTOR         HidDesc
)
{
    UCHAR i;

    AppendTextBuffer("\r\nHID Descriptor:\r\n");

    AppendTextBuffer("bcdHID:             0x%04X\r\n",
    HidDesc->bcdHID);

    AppendTextBuffer("bCountryCode:         0x%02X\r\n",
                     HidDesc->bCountryCode);

    AppendTextBuffer("bNumDescriptors:      0x%02X\r\n",
                     HidDesc->bNumDescriptors);

    for (i=0; i<HidDesc->bNumDescriptors; i++)
    {
        AppendTextBuffer("bDescriptorType:      0x%02X\r\n",
                         HidDesc->OptionalDescriptors[i].bDescriptorType);

        AppendTextBuffer("wDescriptorLength:  0x%04X\r\n",
                         HidDesc->OptionalDescriptors[i].wDescriptorLength);
    }

}


#if 0
//*****************************************************************************
//
// DisplayPowerDescriptor()
//
//*****************************************************************************

PCHAR PowerUnits[] =
{
    "0.001 mW",
    "0.01 mW",
    "0.1 mW",
    "1 mW",
    "10 mW",
    "100 mW",
    "1 W"
};

VOID
DisplayPowerDescriptor (
    PUSB_POWER_DESCRIPTOR       PowerDesc
)
{
    UCHAR i;

    AppendTextBuffer("\r\nPower Descriptor:\r\n");

    AppendTextBuffer("bCapabilitiesFlags:   0x%02X (",
                     PowerDesc->bCapabilitiesFlags);

    if (PowerDesc->bCapabilitiesFlags & USB_SUPPORT_D2_WAKEUP)
    {
        AppendTextBuffer("WakeD2 ");
    }
    if (PowerDesc->bCapabilitiesFlags & USB_SUPPORT_D1_WAKEUP)
    {
        AppendTextBuffer("WakeD1 ");
    }
    if (PowerDesc->bCapabilitiesFlags & USB_SUPPORT_D3_COMMAND)
    {
        AppendTextBuffer("D3 ");
    }
    if (PowerDesc->bCapabilitiesFlags & USB_SUPPORT_D2_COMMAND)
    {
        AppendTextBuffer("D2 ");
    }
    if (PowerDesc->bCapabilitiesFlags & USB_SUPPORT_D1_COMMAND)
    {
        AppendTextBuffer("D1 ");
    }
    if (PowerDesc->bCapabilitiesFlags & USB_SUPPORT_D0_COMMAND)
    {
        AppendTextBuffer("D0 ");
    }
    AppendTextBuffer(")\r\n");

    AppendTextBuffer("EventNotification:  0x%04X\r\n",
                     PowerDesc->EventNotification);

    AppendTextBuffer("D1LatencyTime:      0x%04X\r\n",
                     PowerDesc->D1LatencyTime);

    AppendTextBuffer("D2LatencyTime:      0x%04X\r\n",
                     PowerDesc->D2LatencyTime);

    AppendTextBuffer("D3LatencyTime:      0x%04X\r\n",
                     PowerDesc->D3LatencyTime);

    AppendTextBuffer("PowerUnit:            0x%02X (%s)\r\n",
                     PowerDesc->PowerUnit,
                     PowerDesc->PowerUnit < sizeof(PowerUnits)/sizeof(PowerUnits[0])
                     ? PowerUnits[PowerDesc->PowerUnit]
                     : "?");

    AppendTextBuffer("D0PowerConsumption: 0x%04X (%5d)\r\n",
                     PowerDesc->D0PowerConsumption,
                     PowerDesc->D0PowerConsumption);

    AppendTextBuffer("D1PowerConsumption: 0x%04X (%5d)\r\n",
                     PowerDesc->D1PowerConsumption,
                     PowerDesc->D1PowerConsumption);

    AppendTextBuffer("D2PowerConsumption: 0x%04X (%5d)\r\n",
                     PowerDesc->D2PowerConsumption,
                     PowerDesc->D2PowerConsumption);

}
#endif

//*****************************************************************************
//
// DisplayStringDescriptor()
//
//*****************************************************************************

#if 1

VOID
DisplayStringDescriptor (
    UCHAR                       Index,
    PSTRING_DESCRIPTOR_NODE     StringDescs
)
{
    ULONG nBytes;

    while (StringDescs)
    {
        if (StringDescs->DescriptorIndex == Index)
        {
            AppendTextBuffer("0x%04X: \"",
                             StringDescs->LanguageID);

            nBytes = WideCharToMultiByte(
                         CP_ACP,     // CodePage
                         0,          // CodePage
                         StringDescs->StringDescriptor->bString,
                         (StringDescs->StringDescriptor->bLength - 2) / 2,
                         TextBuffer + TextBufferPos,
                         TextBufferLen - TextBufferPos,
                         NULL,       // lpDefaultChar
                         NULL);      // pUsedDefaultChar

            TextBufferPos += nBytes;

            AppendTextBuffer("\"\r\n");
        }

        StringDescs = StringDescs->Next;
    }

}

#else

VOID
DisplayStringDescriptor (
    UCHAR                       Index,
    PSTRING_DESCRIPTOR_NODE     StringDescs
)
{
    UCHAR i;

    while (StringDescs)
    {
        if (StringDescs->DescriptorIndex == Index)
        {
            AppendTextBuffer("String Descriptor:\r\n");

            AppendTextBuffer("LanguageID:         0x%04X\r\n",
                             StringDescs->LanguageID);

            AppendTextBuffer("bLength:              0x%02X\r\n",
                             StringDescs->StringDescriptor->bLength);

            for (i = 0; i < StringDescs->StringDescriptor->bLength-2; i++)
            {
                AppendTextBuffer("%02X ",
                                 ((PUCHAR)StringDescs->StringDescriptor->bString)[i]);

                if (i % 16 == 15)
                {
                    AppendTextBuffer("\r\n");
                }
            }

            if (i % 16 != 0)
            {
                AppendTextBuffer("\r\n");
            }
        }

        StringDescs = StringDescs->Next;
    }

}

#endif

//*****************************************************************************
//
// DisplayUnknownDescriptor()
//
//*****************************************************************************

VOID
DisplayUnknownDescriptor (
    PUSB_COMMON_DESCRIPTOR      CommonDesc
)
{
    UCHAR   i;

    AppendTextBuffer("\r\nUnknown Descriptor:\r\n");

    AppendTextBuffer("bDescriptorType:      0x%02X\r\n",
                     CommonDesc->bDescriptorType);

    AppendTextBuffer("bLength:              0x%02X\r\n",
                     CommonDesc->bLength);

    for (i = 0; i < CommonDesc->bLength; i++)
    {
        AppendTextBuffer("%02X ",
                         ((PUCHAR)CommonDesc)[i]);

        if (i % 16 == 15)
        {
            AppendTextBuffer("\r\n");
        }
    }

    if (i % 16 != 0)
    {
        AppendTextBuffer("\r\n");
    }

}

//*****************************************************************************
//
// GetVendorString()
//
// idVendor - USB Vendor ID
//
// Return Value - Vendor name string associated with idVendor, or NULL if
// no vendor name string is found which is associated with idVendor.
//
//*****************************************************************************

PCHAR
GetVendorString (
    USHORT     idVendor
)
{
    PUSBVENDORID vendorID;

    if (idVendor != 0x0000)
    {
        vendorID = USBVendorIDs;

        while (vendorID->usVendorID != 0x0000)
        {
            if (vendorID->usVendorID == idVendor)
            {
                return (vendorID->szVendor);
            }
            vendorID++;
        }
    }

    return NULL;
}

