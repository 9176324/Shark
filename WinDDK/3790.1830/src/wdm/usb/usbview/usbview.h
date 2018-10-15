/*++

Copyright (c) 1997-1998 Microsoft Corporation

Module Name:

    USBVIEW.H

Abstract:

    This is the header file for USBVIEW

Environment:

    user mode

Revision History:

    04-25-97 : created

--*/

//*****************************************************************************
// I N C L U D E S
//*****************************************************************************

#include <commctrl.h>
#include <usbioctl.h>
#include <usbiodef.h>

#include "usbdesc.h"
#include "usb100.h"
#include "usb200.h"

//*****************************************************************************
// P R A G M A S
//*****************************************************************************

#pragma intrinsic(strlen, strcpy, memset)

//*****************************************************************************
// D E F I N E S
//*****************************************************************************

#ifdef  DEBUG
#undef  DBG
#define DBG 1
#endif

#if DBG
#define OOPS() Oops(__FILE__, __LINE__)
#else
#define OOPS()
#endif


#if DBG

#define ALLOC(dwBytes) MyAlloc(__FILE__, __LINE__, (dwBytes))

#define REALLOC(hMem, dwBytes) MyReAlloc((hMem), (dwBytes))

#define FREE(hMem)  MyFree((hMem))

#define CHECKFORLEAKS() MyCheckForLeaks()

#else

#define ALLOC(dwBytes) GlobalAlloc(GPTR,(dwBytes))

#define REALLOC(hMem, dwBytes) GlobalReAlloc((hMem), (dwBytes), (GMEM_MOVEABLE|GMEM_ZEROINIT))

#define FREE(hMem)  GlobalFree((hMem))

#define CHECKFORLEAKS()

#endif



//
//  BOOLEAN
//  IsListEmpty(
//      PLIST_ENTRY ListHead
//      );
//

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

//
//  PLIST_ENTRY
//  RemoveHeadList(
//      PLIST_ENTRY ListHead
//      );
//

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

//
//  VOID
//  RemoveEntryList(
//      PLIST_ENTRY Entry
//      );
//

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

//
//  VOID
//  InsertTailList(
//      PLIST_ENTRY ListHead,
//      PLIST_ENTRY Entry
//      );
//

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

//*****************************************************************************
// T Y P E D E F S
//*****************************************************************************

typedef enum _TREEICON
{
    ComputerIcon,
    HubIcon,
    NoDeviceIcon,
    GoodDeviceIcon,
    BadDeviceIcon
} TREEICON;


// Callback function for walking TreeView items
//
typedef VOID
(*LPFNTREECALLBACK)(
    HWND        hTreeWnd,
    HTREEITEM   hTreeItem
);

//
// Structure used to build a linked list of String Descriptors
// retrieved from a device.
//

typedef struct _STRING_DESCRIPTOR_NODE
{
    struct _STRING_DESCRIPTOR_NODE *Next;
    UCHAR                           DescriptorIndex;
    USHORT                          LanguageID;
    USB_STRING_DESCRIPTOR           StringDescriptor[0];
} STRING_DESCRIPTOR_NODE, *PSTRING_DESCRIPTOR_NODE;


//
// Structures assocated with TreeView items through the lParam.  When an item
// is selected, the lParam is retrieved and the structure it which it points
// is used to display information in the edit control.
//

typedef enum _USBDEVICEINFOTYPE
{
    HostControllerInfo,

    RootHubInfo,

    ExternalHubInfo,

    DeviceInfo

} USBDEVICEINFOTYPE, *PUSBDEVICEINFOTYPE;


typedef struct _USBHOSTCONTROLLERINFO
{
    USBDEVICEINFOTYPE                   DeviceInfoType;

    LIST_ENTRY                          ListEntry;

    PCHAR                               DriverKey;

    ULONG                               VendorID;

    ULONG                               DeviceID;

    ULONG                               SubSysID;

    ULONG                               Revision;

} USBHOSTCONTROLLERINFO, *PUSBHOSTCONTROLLERINFO;


typedef struct _USBROOTHUBINFO
{
    USBDEVICEINFOTYPE                   DeviceInfoType;

    PUSB_NODE_INFORMATION               HubInfo;

    PCHAR                               HubName;

} USBROOTHUBINFO, *PUSBROOTHUBINFO;


typedef struct _USBEXTERNALHUBINFO
{
    USBDEVICEINFOTYPE                   DeviceInfoType;

    PUSB_NODE_INFORMATION               HubInfo;

    PCHAR                               HubName;

    PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo;

    PUSB_DESCRIPTOR_REQUEST             ConfigDesc;

    PSTRING_DESCRIPTOR_NODE             StringDescs;

} USBEXTERNALHUBINFO, *PUSBEXTERNALHUBINFO;


typedef struct _USBDEVICEINFO
{
    USBDEVICEINFOTYPE                   DeviceInfoType;

    PUSB_NODE_CONNECTION_INFORMATION_EX ConnectionInfo;

    PUSB_DESCRIPTOR_REQUEST             ConfigDesc;

    PSTRING_DESCRIPTOR_NODE             StringDescs;

} USBDEVICEINFO, *PUSBDEVICEINFO;


//*****************************************************************************
// G L O B A L S
//*****************************************************************************

//
// USBVIEW.C
//

BOOL gDoConfigDesc;
int TotalHubs;

//
// ENUM.C
//

PCHAR ConnectionStatuses[];


//*****************************************************************************
// F U N C T I O N    P R O T O T Y P E S
//*****************************************************************************

//
// USBVIEW.C
//

HTREEITEM
AddLeaf (
    HTREEITEM hTreeParent,
    LPARAM    lParam,
    LPTSTR    lpszText,
    TREEICON  TreeIcon
);

VOID
Oops
(
    CHAR *File,
    ULONG Line
);

//
// DISPLAY.C
//

BOOL
CreateTextBuffer (
);

VOID
DestroyTextBuffer (
);

VOID
UpdateEditControl (
    HWND      hEditWnd,
    HWND      hTreeWnd,
    HTREEITEM hTreeItem
);


VOID __cdecl
AppendTextBuffer (
    LPCTSTR lpFormat,
    ...
);

//
// ENUM.C
//

VOID
EnumerateHostControllers (
    HTREEITEM  hTreeParent,
    ULONG     *DevicesConnected
);


VOID
CleanupItem (
    HWND      hTreeWnd,
    HTREEITEM hTreeItem
);


//
// DEBUG.C
//

HGLOBAL
MyAlloc (
    PCHAR   File,
    ULONG   Line,
    DWORD   dwBytes
);

HGLOBAL
MyReAlloc (
    HGLOBAL hMem,
    DWORD   dwBytes
);

HGLOBAL
MyFree (
    HGLOBAL hMem
);

VOID
MyCheckForLeaks (
    VOID
);


//
// DEVNODE.C
//

PCHAR
DriverNameToDeviceDesc (
    PCHAR   DriverName,
    BOOLEAN DeviceId
);


//
// DISPAUD.C
//

BOOL
DisplayAudioDescriptor (
    PUSB_AUDIO_COMMON_DESCRIPTOR CommonDesc,
    UCHAR                        bInterfaceSubClass
);


