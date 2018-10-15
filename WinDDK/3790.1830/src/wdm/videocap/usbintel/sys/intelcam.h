#ifndef __INTELCAM_H__
#define __INTELCAM_H__


/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

   intelcam.h

Abstract:

   driver for the intel camera.

Author:

 Notes:

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
  PURPOSE.
   

Environment:

   Kernel mode only


Revision History:

--*/

#include <strmini.h>
#include <ksmedia.h>

#include "usbdi.h"
#include "usbcamdi.h"

#include "prpobj.h"

typedef enum {
   STREAM_Capture,                  // we always assume vidoe stream is stream 0
   STREAM_Still   
};

typedef struct _INTELCAM_DEVICE_CONTEXT {
    ULONG Sig;
    ULONG StartOffset_y;
    ULONG StartOffset_u;
    ULONG StartOffset_v;

    // internal counters
    ULONG IgnorePacketCount;
    USHORT hwSetting;       // we need to save camera h setting for scaling for PM purposes.

    ULONG DefaultFrameRate;
    PUSBD_INTERFACE_INFORMATION Interface;
    //
	// ??
	//
    USBCAMD_PROPERTY CurrentProperty; 
    ULONG   ApplyRegistryValues;
#ifdef USBCAMD2
    ULONG   PinCount;
    UCHAR   PrevStillIndicator;
    BOOLEAN SoftTrigger;
#else
    ULONG   BadPackets;
    BOOLEAN IsVideoFrameGood;
#endif
    BOOLEAN  FirstTime;
    BOOLEAN  StreamOpen;
    USBCAMD_INTERFACE   UsbcamdInterface;
    PDEVICE_OBJECT pDeviceObject;
    PDEVICE_OBJECT pPnPDeviceObject;
} INTELCAM_DEVICE_CONTEXT, *PINTELCAM_DEVICE_CONTEXT;

typedef struct _INTELCAM_FRAME_CONTEXT {
    ULONG Sig;
} INTELCAM_FRAME_CONTEXT, *PINTELCAM_FRAME_CONTEXT;

#define INTELCAM_PIPECONFIG_LISTSIZE 2


//
// constents for values in sync packet
//

#define USBCAMD_SOV      0x80
#define USBCAMD_I        0x01
#define USBCAMD_ERROR    0x02

#define STILL_INDICATOR_MASK 0x40
#define STILL_INDICATOR_BIT 0x06

// test still indicator bit in the sync byte.
#define INTELCAM_STILL_BUTTON_PRESSED(sb)            ( (((sb) & STILL_INDICATOR_MASK)) >> STILL_INDICATOR_BIT)

//USB Vendor Commands (Request)
#define REQUEST_SET_YEAGER_REG              0
#define REQUEST_GET_YEAGER_REG              1
#define REQUEST_SET_PREFERENCE              2
#define REQUEST_GET_PREFERENCE              3
#define REQUEST_SET_MANUFACTURER_SETTING    4

//        
// Preference Type Defines
//

#define INDEX_PREF_WHITEBALANCE 0x0001
#define INDEX_PREF_BRIGHTNESS   0x0002
#define INDEX_PREF_ENHANCEMENT  0x0003
#define INDEX_PREF_EXPOSURE     0x0004
#define INDEX_PREF_SATURATION   0x0005
#define INDEX_PREF_SEEK         0x0006
#define INDEX_PREF_PAN          0x0007
#define INDEX_PREF_SCALING      0x0008
//#define INDEX_PREF_CAP_IMG_SZ     0x0009
#define INDEX_PREF_CAPSTILL     0x000A
#define INDEX_PREF_CAPMOTION    0x000B
#define INDEX_PREF_CAP_ATTN     0x000C
#define INDEX_PREF_INITIALIZE   0x000D
#define INDEX_PREF_FWARE_VER    0x000E
#define INDEX_PREF_EPROM        0x000F
#define INDEX_PREF_POWER        0x0010


#define INTELCAM_DEVICE_SIG 0x45544e49     //"INTE"

#if DBG
#define ASSERT_DEVICE_CONTEXT(d) ASSERT((d)->Sig == INTELCAM_DEVICE_SIG)
#else
#define ASSERT_DEVICE_CONTEXT(d) 
#endif

//
// (Usec) Frame rate macro.
//
#define MICROSEC_PERFRAME(x)    ((x > 0) ? ((LONGLONG)1000000/x) : 0)
#define NUM_100NANOSEC_UNITS_PERFRAME(x) ((x > 0) ? ((LONGLONG)10000000 / x) :0 )

#define DRIVER_DATA_DONTCARE 0xffffffff
#define FRAME_RATE_LIST_SIZE   8



// **
// format for each type of video stream we support
// **

#ifndef mmioFOURCC    
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
        ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
        ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif  


typedef struct {
    UCHAR            AlternateSetting;  // Alt. interface number.
    ULONG           MaxPktSize;     // Max. Pkt size for this alt. interface
    LONGLONG        QCIFFrameRate;  // frame rate in 100 nano seconds for QCIF size.
} MaxPktSizePerInterface;

#define BUS_BW_ARRAY_SIZE     8



//
// the idle setting consumes no bw
//
#define INTELCAM_IDLE_ALT_SETTING       7
//??#define INTELCAM_IDLE_ALT_SETTING       6

//
// the default setting uses %25 of the BW available
// on the bus.
//
//#define INTELCAM_DEFAULT_ALT_SETTING    0   //10 fps
#define INTELCAM_DEFAULT_FRAME_RATE     20

// alt setting / fps
//  7,      // uses no BW
//  1,      // 6 fps 0x0f0
//  0,      // 10 fps 0x180
//  3,      // 12 fps 0x1d0
//  2,      // 15 fps 0x240
//  4,      // 18 fps 0x2b0
//  5,      // 20 fps 0x300
//  6,      // 25 fps 0x3c0
  
#define INTELCAM_MAX_FRAME_RATE 25

#define INTELCAM_SYNC_PIPE    0
#define INTELCAM_DATA_PIPE    1

#define MAX_TRACE 2
#define MIN_TRACE 1




#if DBG

extern ULONG INTELCAM_DebugTraceLevel;

#define INTELCAM_KdPrint(_t_, _x_) \
    if (INTELCAM_DebugTraceLevel >= _t_) { \
        DbgPrint("'INTELCAM: "); \
        DbgPrint _x_ ;\
    }


#ifdef _X86_
#define TRAP()  _asm {int 3}
#define TEST_TRAP() _asm {int 3}
#define TRAP_ERROR(e) if (!NT_SUCCESS(e)) { _asm {int 3} }
#else
#define TRAP()  DbgBreakPoint()
#define TEST_TRAP() DbgBreakPoint()
#define TRAP_ERROR(e) if (!NT_SUCCESS(e)) { DbgBreakPoint(); }
#endif

#else

#define INTELCAM_KdPrint(_t_, _x_)

#define TEST_TRAP()
#define TRAP()

#endif /* DBG */


NTSTATUS
INTELCAM_Initialize(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );

NTSTATUS
INTELCAM_UnInitialize(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );    

NTSTATUS
INTELCAM_StartVideoCapture(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );

NTSTATUS
INTELCAM_StopVideoCapture(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );    

ULONG
INTELCAM_ProcessUSBPacket(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID CurrentFrameContext,
    PUSBD_ISO_PACKET_DESCRIPTOR SyncPacket,
    PVOID SyncBuffer,
    PUSBD_ISO_PACKET_DESCRIPTOR DataPacket,
    PVOID DataBuffer,
    PBOOLEAN FrameComplete,
    PBOOLEAN NextFrameIsStill
    );


VOID
INTELCAM_NewFrame(
    PVOID DeviceContext,
    PVOID FrameContext
    );



NTSTATUS
INTELCAM_ProcessRawVideoFrame(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID FrameContext,
    PVOID FrameBuffer,
    ULONG FrameLength,
    PVOID RawFrameBuffer,
    ULONG RawFrameLength,
    ULONG NumberOfPackets,
    PULONG BytesReturned
    );


NTSTATUS
INTELCAM_Configure(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PUSBD_INTERFACE_INFORMATION Interface,
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    PLONG DataPipeIndex,
    PLONG SyncPipeIndex
    );    



NTSTATUS
INTELCAM_SaveState(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );

NTSTATUS
INTELCAM_RestoreState(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );    

NTSTATUS
INTELCAM_AllocateBandwidth(
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PVOID DeviceContext,
    OUT PULONG RawFrameLength,
    IN PVOID Format            
    );    

NTSTATUS
INTELCAM_FreeBandwidth(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );      


NTSTATUS
INTELCAM_CameraToDriverDefaults(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext
    );


NTSTATUS
INTELCAM_SaveControlsToRegistry(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID pDeviceContext
    );

PVOID 
INTELCAM_GetAdapterPropertyTable(
    PULONG NumberOfPropertySets
    );    

NTSTATUS
INTELCAM_PropertyRequest(
    BOOLEAN SetProperty,
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID PropertyContext
    );

VOID STREAMAPI
INTELCAM_ReceiveDataPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PVOID DeviceContext,
    IN PBOOLEAN Completed
    );


VOID STREAMAPI
INTELCAM_ReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK Srb,
    IN PVOID DeviceContext,
    IN PBOOLEAN Completed
    );

BOOL 
AdapterVerifyFormat(
        PKS_DATAFORMAT_VIDEOINFOHEADER pKSDataFormatToVerify, 
        ULONG StreamNumber);

#ifdef USBCAMD2

VOID
INTELCAM_NewFrameEx(
    PVOID DeviceContext,
    PVOID FrameContext,
    ULONG StreamNumber,
    PULONG FrameLength
    );

NTSTATUS
INTELCAM_QueryUSBCAMDInterface(
    IN PDEVICE_OBJECT pDeviceObject,
    OUT PUSBCAMD_INTERFACE UsbcamdInterface
    );

BOOLEAN 
INTELCAM_HwInitialize(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

NTSTATUS 
INTELCAM_CompleteInitialization(
    IN PHW_STREAM_REQUEST_BLOCK Srb
    );

NTSTATUS
INTELCAM_ConfigureEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PUSBD_INTERFACE_INFORMATION Interface,
    PUSB_CONFIGURATION_DESCRIPTOR ConfigurationDescriptor,
    ULONG   PipeConfigListSize,
    PUSBCAMD_Pipe_Config_Descriptor pConfigDesc,
    PUSB_DEVICE_DESCRIPTOR pDeviceDesc
    );

NTSTATUS
INTELCAM_StartVideoCaptureEx(
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PVOID DeviceContext,
    IN ULONG StreamNumber
    );

NTSTATUS
INTELCAM_AllocateBandwidthEx(
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PVOID DeviceContext,
    OUT PULONG RawFrameLength,
    IN PVOID Format,
    IN ULONG StreamNumber
    );

NTSTATUS
INTELCAM_FreeBandwidthEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    ULONG StreamNumber
    );

NTSTATUS
INTELCAM_StopVideoCaptureEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    ULONG StreamNumber
    );

ULONG
INTELCAM_ProcessUSBPacketEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID CurrentFrameContext,
    PUSBD_ISO_PACKET_DESCRIPTOR SyncPacket,
    PVOID SyncBuffer,
    PUSBD_ISO_PACKET_DESCRIPTOR DataPacket,
    PVOID DataBuffer,
    PBOOLEAN FrameComplete,
    PULONG PacketFlag,
    PULONG ValidDataOffset
    );


NTSTATUS
INTELCAM_ProcessRawVideoFrameEx(
    PDEVICE_OBJECT BusDeviceObject,
    PVOID DeviceContext,
    PVOID FrameContext,
    PVOID FrameBuffer,
    ULONG FrameLength,
    PVOID RawFrameBuffer,
    ULONG RawFrameLength,
    ULONG NumberOfPackets,
    PULONG BytesReturned,
    ULONG ActualRawFrameLength,
    ULONG StreamNumber
    );

BOOL
GetPropertyValuesFromRegistry(
    PVOID pDC
    );

NTSTATUS 
GetRegistryKeyValue (
    IN HANDLE Handle,
    IN PWCHAR KeyNameString,
    IN ULONG KeyNameStringLength,
    IN PVOID Data,
    IN PULONG DataLength
    );

NTSTATUS 
CreateRegistrySubKey(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    );

NTSTATUS 
CreateRegistryKeySingle(
    IN HANDLE hKey,
    IN ACCESS_MASK desiredAccess,
    PWCHAR pwszSection,
    OUT PHANDLE phKeySection
    );


NTSTATUS
SetRegistryKeyValue(
   HANDLE hKey,
   PWCHAR pwszEntry, 
   LONG nValue
   );

NTSTATUS
GetDataFromCamera(
    IN REQUEST_CUSTOM PropertyID,
    IN PINTELCAM_DEVICE_CONTEXT pDC,
    PHW_STREAM_REQUEST_BLOCK pSrb,
    IN ULONG DefOrCurr
    );

NTSTATUS
SetPropertyCtrl(
    IN REQUEST ReqID,
    IN PINTELCAM_DEVICE_CONTEXT pDC,
    IN OUT PHW_STREAM_REQUEST_BLOCK pSrb
    );




#endif

#endif



