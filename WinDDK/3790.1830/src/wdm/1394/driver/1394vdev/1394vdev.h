/*++

Copyright (c) 1998-99 Microsoft Corporation

Module Name:

    1394vdev.h

Abstract:

Author:

    Peter Binder (pbinder) 4/13/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/13/98  pbinder   taken from original 1394diag...
--*/

#define ISOCH_DETACH_TIMEOUT_VALUE        (ULONG)(-100 * 100 * 100 * 100) //80 msecs in units of 100nsecs

//
// IEEE 1212 Directory definition
//
typedef struct _DIRECTORY_INFO {
    union {
        USHORT          DI_CRC;
        USHORT          DI_Saved_Length;
    } u;
    USHORT              DI_Length;
} DIRECTORY_INFO, *PDIRECTORY_INFO;

//
// IEEE 1212 Offset entry definition
//
typedef struct _OFFSET_ENTRY {
    ULONG               OE_Offset:24;
    ULONG               OE_Key:8;
} OFFSET_ENTRY, *POFFSET_ENTRY;

//
// Structure to identify myself as a virtual test device across the bus
//
#define MAX_LEN_HOST_NAME 32

typedef struct _POWER_COMPLETION_CONTEXT {

    PDEVICE_OBJECT  DeviceObject;
    PIRP            SIrp;

} POWER_COMPLETION_CONTEXT, *PPOWER_COMPLETION_CONTEXT;

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT          StackDeviceObject;
    PDEVICE_OBJECT          PortDeviceObject;
    PDEVICE_OBJECT          PhysicalDeviceObject;

    UNICODE_STRING          SymbolicLinkName;
    KSPIN_LOCK              ResetSpinLock;
    KSPIN_LOCK              CromSpinLock;
    KSPIN_LOCK              AsyncSpinLock;
    KSPIN_LOCK              IsochSpinLock;
    KSPIN_LOCK              IsochResourceSpinLock;
    
    BOOLEAN                 bShutdown;
    DEVICE_POWER_STATE      CurrentDevicePowerState;
    SYSTEM_POWER_STATE      CurrentSystemPowerState;

    ULONG                   GenerationCount;
    
    LIST_ENTRY              BusResetIrps;
    LIST_ENTRY              CromData;
    LIST_ENTRY              AsyncAddressData;
    LIST_ENTRY              IsochDetachData;
    LIST_ENTRY              IsochResourceData;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;



//
// This is used to keep track of pending irp's for
// notification of bus resets.
//
typedef struct _BUS_RESET_IRP {
    LIST_ENTRY      BusResetIrpList;
    PIRP            Irp;
} BUS_RESET_IRP, *PBUS_RESET_IRP;

//
// This is used to keep track of dynamic crom calls.
//
typedef struct _CROM_DATA {
    LIST_ENTRY      CromList;
    HANDLE          hCromData;
    PVOID           Buffer;
    PMDL            pMdl;
} CROM_DATA, *PCROM_DATA;

//
// This is used to store data for each async address range. 
//
typedef struct _ASYNC_ADDRESS_DATA {
    LIST_ENTRY              AsyncAddressList;
    PDEVICE_EXTENSION       DeviceExtension;
    PVOID                   Buffer;
    ULONG                   nLength;
    ULONG                   nAddressesReturned;
    PADDRESS_RANGE          AddressRange;
    HANDLE                  hAddressRange;
    PMDL                    pMdl;
} ASYNC_ADDRESS_DATA, *PASYNC_ADDRESS_DATA;

#define ISOCH_DETACH_TAG    0xaabbbbaa

// 
// This is used to store data needed when calling IsochDetachBuffers.
// We need to store this data seperately for each call to IsochAttachBuffers.
//
typedef struct _ISOCH_DETACH_DATA {
    LIST_ENTRY              IsochDetachList;
    PDEVICE_EXTENSION       DeviceExtension;
    PISOCH_DESCRIPTOR       IsochDescriptor;
    PIRP                    Irp;
    PIRP                    newIrp;
    PIRB                    DetachIrb;
    PIRB                    AttachIrb;
    NTSTATUS                AttachStatus;
    KTIMER                  Timer;
    KDPC                    TimerDpc;
    HANDLE                  hResource;
    ULONG                   numIsochDescriptors;
    ULONG                   outputBufferLength;
    ULONG                   bDetach;
} ISOCH_DETACH_DATA, *PISOCH_DETACH_DATA;

//
// This is used to store allocated isoch resources.
// We use this information in case of a surprise removal.
//
typedef struct _ISOCH_RESOURCE_DATA {
    LIST_ENTRY      IsochResourceList;
    HANDLE          hResource;
} ISOCH_RESOURCE_DATA, *PISOCH_RESOURCE_DATA;

//
// 1394api.c
//
NTSTATUS
t1394_GetLocalHostInformation(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            nLevel,
    IN OUT PULONG       UserStatus,
    IN OUT PVOID        Information
    );

NTSTATUS
t1394_Get1394AddressFromDeviceObject(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            fulFlags,
    OUT PNODE_ADDRESS   pNodeAddress
    );

NTSTATUS
t1394_Control(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
t1394_GetMaxSpeedBetweenDevices(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            fulFlags,
    IN ULONG            ulNumberOfDestinations,
    IN PDEVICE_OBJECT   hDestinationDeviceObjects[64],
    OUT PULONG          fulSpeed
    );

NTSTATUS
t1394_SetDeviceXmitProperties(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            fulSpeed,
    IN ULONG            fulPriority
    );

NTSTATUS
t1394_GetConfigurationInformation(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
t1394_BusReset(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            fulFlags
    );

NTSTATUS
t1394_GetGenerationCount(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN OUT PULONG       GenerationCount
    );

NTSTATUS
t1394_SendPhyConfigurationPacket(
    IN PDEVICE_OBJECT               DeviceObject,
    IN PIRP                         Irp,
    IN PHY_CONFIGURATION_PACKET     PhyConfigurationPacket
    );

NTSTATUS
t1394_BusResetNotification(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            fulFlags
    );

NTSTATUS
t1394_SetLocalHostProperties(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            nLevel,
    IN PVOID            Information
    );

void
t1394_BusResetRoutine(
    IN PVOID    Context
    );

//
// asyncapi.c
//
NTSTATUS
t1394_AllocateAddressRange(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN ULONG                fulAllocateFlags,
    IN ULONG                fulFlags,
    IN ULONG                nLength,
    IN ULONG                MaxSegmentSize,
    IN ULONG                fulAccessType,
    IN ULONG                fulNotificationOptions,
    IN OUT PADDRESS_OFFSET  Required1394Offset,
    OUT PHANDLE             phAddressRange,
    IN OUT PULONG           Data
    );

NTSTATUS
t1394_FreeAddressRange(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN HANDLE           hAddressRange
    );

NTSTATUS
t1394_SetAddressData(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN HANDLE               hAddressRange,
    IN ULONG                nLength,
    IN ULONG                ulOffset,
    IN PVOID                Data
    );

NTSTATUS
t1394_GetAddressData(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN HANDLE               hAddressRange,
    IN ULONG                nLength,
    IN ULONG                ulOffset,
    IN PVOID                Data
    );

NTSTATUS
t1394_AsyncRead(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            bRawMode,
    IN ULONG            bGetGeneration,
    IN IO_ADDRESS       DestinationAddress,
    IN ULONG            nNumberOfBytesToRead,
    IN ULONG            nBlockSize,
    IN ULONG            fulFlags,
    IN ULONG            ulGeneration,
    IN OUT PULONG       Data
    );

NTSTATUS
t1394_AsyncWrite(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            bRawMode,
    IN ULONG            bGetGeneration,
    IN IO_ADDRESS       DestinationAddress,
    IN ULONG            nNumberOfBytesToWrite,
    IN ULONG            nBlockSize,
    IN ULONG            fulFlags,
    IN ULONG            ulGeneration,
    IN OUT PULONG       Data
    );

NTSTATUS
t1394_AsyncLock(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            bRawMode,
    IN ULONG            bGetGeneration,
    IN IO_ADDRESS       DestinationAddress,
    IN ULONG            nNumberOfArgBytes,
    IN ULONG            nNumberOfDataBytes,
    IN ULONG            fulTransactionType,
    IN ULONG            fulFlags,
    IN ULONG            Arguments[2],
    IN ULONG            DataValues[2],
    IN ULONG            ulGeneration,
    IN OUT PVOID        Buffer
    );

NTSTATUS
t1394_AsyncStream(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            nNumberOfBytesToStream,
    IN ULONG            fulFlags,
    IN ULONG            ulTag,
    IN ULONG            nChannel,
    IN ULONG            ulSynch,
    IN UCHAR            nSpeed,
    IN OUT PULONG       Data
    );

//
// isochapi.c
//
NTSTATUS
t1394_IsochAllocateBandwidth(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            nMaxBytesPerFrameRequested,
    IN ULONG            fulSpeed,
    OUT PHANDLE         phBandwidth,
    OUT PULONG          pBytesPerFrameAvailable,
    OUT PULONG          pSpeedSelected
    );

NTSTATUS
t1394_IsochAllocateChannel(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            nRequestedChannel,
    OUT PULONG          pChannel,
    OUT PLARGE_INTEGER  pChannelsAvailable
    );

NTSTATUS
t1394_IsochAllocateResources(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            fulSpeed,
    IN ULONG            fulFlags,
    IN ULONG            nChannel,
    IN ULONG            nMaxBytesPerFrame,
    IN ULONG            nNumberOfBuffers,
    IN ULONG            nMaxBufferSize,
    IN ULONG            nQuadletsToStrip,
    OUT PHANDLE         phResource
    );

NTSTATUS
t1394_IsochAttachBuffers(
    IN PDEVICE_OBJECT               DeviceObject,
    IN PIRP                         Irp,
    IN ULONG                        outputBufferLength,
    IN HANDLE                       hResource,
    IN ULONG                        nNumberOfDescriptors,
    OUT PISOCH_DESCRIPTOR           pIsochDescriptor,
    IN OUT PRING3_ISOCH_DESCRIPTOR  R3_IsochDescriptor
    );

NTSTATUS
t1394_IsochDetachBuffers(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN HANDLE               hResource,
    IN ULONG                nNumberOfDescriptors,
    IN PISOCH_DESCRIPTOR    IsochDescriptor
    );

NTSTATUS
t1394_IsochFreeBandwidth(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN HANDLE           hBandwidth
    );

NTSTATUS
t1394_IsochFreeChannel(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            nChannel
    );

NTSTATUS
t1394_IsochFreeResources(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN HANDLE           hResource
    );

NTSTATUS
t1394_IsochListen(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN HANDLE           hResource,
    IN ULONG            fulFlags,
    IN CYCLE_TIME       StartTime
    );

NTSTATUS
t1394_IsochQueryCurrentCycleTime(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    OUT PCYCLE_TIME     pCurrentCycleTime
    );

NTSTATUS
t1394_IsochQueryResources(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN ULONG            fulSpeed,
    OUT PULONG          pBytesPerFrameAvailable,
    OUT PLARGE_INTEGER  pChannelsAvailable
    );

NTSTATUS
t1394_IsochSetChannelBandwidth(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN HANDLE           hBandwidth,
    IN ULONG            nMaxBytesPerFrame
    );

NTSTATUS
t1394_IsochModifyStreamProperties(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN HANDLE               hResource,
    IN ULARGE_INTEGER       ChannelMask,
    IN ULONG                fulSpeed
    );

NTSTATUS
t1394_IsochStop(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN HANDLE           hResource,
    IN ULONG            fulFlags
    );

NTSTATUS
t1394_IsochTalk(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN HANDLE           hResource,
    IN ULONG            fulFlags,
    CYCLE_TIME          StartTime
    );

void
t1394_IsochCallback(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PISOCH_DETACH_DATA   IsochDetachData
    );

void
t1394_IsochTimeout(
    IN PKDPC                Dpc,
    IN PISOCH_DETACH_DATA   IsochDetachData,
    IN PVOID                SystemArgument1,
    IN PVOID                SystemArgument2
    );

void
t1394_IsochCleanup(
    IN PISOCH_DETACH_DATA   IsochDetachData
    );

NTSTATUS
t1394_IsochDetachCompletionRoutine(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PISOCH_DETACH_DATA   IsochDetachData
    );

NTSTATUS
t1394_IsochAttachCompletionRoutine(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PISOCH_DETACH_DATA   IsochDetachData
    );

//
// util.c
//
NTSTATUS
t1394_SubmitIrpSynch(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIRB                 Irb
    );
    
NTSTATUS
t1394_SubmitIrpAsync(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIRB                 Irb
    );
    
NTSTATUS
t1394_SynchCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PKEVENT          Event
    );

BOOLEAN
t1394_IsOnList(
	PLIST_ENTRY		Entry,
	PLIST_ENTRY		List
	);

VOID
t1394_UpdateGenerationCount (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );


//
// 1394vdev.c
//
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

NTSTATUS
t1394VDev_Create(
    IN PDEVICE_OBJECT   DriverObject,
    IN PIRP             Irp
    );

NTSTATUS
t1394VDev_Close(
    IN PDEVICE_OBJECT   DriverObject,
    IN PIRP             Irp
    );

void
t1394VDev_Unload(
    IN PDRIVER_OBJECT   DriverObject
    );
    
void
t1394VDev_CancelIrp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );
    
//
// ioctl.c
//
NTSTATUS
t1394VDev_IoControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

//
// pnp.c
//
NTSTATUS
t1394VDev_PnpAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    );

NTSTATUS
t1394VDev_Pnp(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
t1394VDev_PnpStartDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
t1394VDev_PnpStopDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
t1394VDev_PnpRemoveDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

//
// power.c
//
NTSTATUS
t1394VDev_Power(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
t1394VDev_SystemSetPowerIrpCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            NotUsed
    );

VOID
t1394VDev_DeviceSetPowerIrpCompletion(
    PDEVICE_OBJECT DeviceObject,
    UCHAR MinorFunction,
    POWER_STATE state,
    POWER_COMPLETION_CONTEXT* PowerContext,
    PIO_STATUS_BLOCK IoStatus
    );

