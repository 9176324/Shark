/*++

Copyright (c) 1998  Microsoft Corporation

Module Name:

    1394api.h

Abstract


Author:

    Kashif Hasan (khasan) 4/30/01

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/08/98  pbinder   birth
4/30/01  khasan    add multiple include ability
--*/

#ifndef _1394API_H_
#define _1394API_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DRIVER

#define WM_DEVICE_CHANGE                        (WM_USER+301)
#define WM_BUS_RESET                            (WM_USER+302)

#define NOTIFY_DEVICE_CHANGE                    WM_DEVICE_CHANGE
#define NOTIFY_BUS_RESET                        WM_BUS_RESET

//
// flags used for async loopback
//
#define ASYNC_LOOPBACK_READ                     1
#define ASYNC_LOOPBACK_WRITE                    2
#define ASYNC_LOOPBACK_LOCK                     4
#define ASYNC_LOOPBACK_RANDOM_LENGTH            8

//
// flags used for allocateaddressrange specific for
// 1394api.dll
//
#define ASYNC_ALLOC_USE_MDL                     1
#define ASYNC_ALLOC_USE_FIFO                    2
#define ASYNC_ALLOC_USE_NONE                    3


//
// The defines/structs below are taken from 1394.h.
//

//
// Definition of flags for AllocateAddressRange Irb
//
#define BIG_ENDIAN_ADDRESS_RANGE                1

//
// Definition of fulAccessType for AllocateAddressRange
//
#define ACCESS_FLAGS_TYPE_READ                  1
#define ACCESS_FLAGS_TYPE_WRITE                 2
#define ACCESS_FLAGS_TYPE_LOCK                  4
#define ACCESS_FLAGS_TYPE_BROADCAST             8

//
// Definition of fulNotificationOptions for AllocateAddressRange
//
#define NOTIFY_FLAGS_NEVER                      0
#define NOTIFY_FLAGS_AFTER_READ                 1
#define NOTIFY_FLAGS_AFTER_WRITE                2
#define NOTIFY_FLAGS_AFTER_LOCK                 4

//
// Definitions of Lock transaction types
//
#define LOCK_TRANSACTION_MASK_SWAP              1
#define LOCK_TRANSACTION_COMPARE_SWAP           2
#define LOCK_TRANSACTION_FETCH_ADD              3
#define LOCK_TRANSACTION_LITTLE_ADD             4
#define LOCK_TRANSACTION_BOUNDED_ADD            5
#define LOCK_TRANSACTION_WRAP_ADD               6

//
// Definition of fulFlags in Async Read/Write/Lock requests
//

#define ASYNC_FLAGS_NONINCREMENTING             1

//
// flag instucts the port driver to NOT take an int for checking the status
// of this transaction. Always return success...
//

#define ASYNC_FLAGS_NO_STATUS               0x00000002

//
// Definitions of levels of Host controller information
//
#define GET_HOST_UNIQUE_ID                      1
#define GET_HOST_CAPABILITIES                   2
#define GET_POWER_SUPPLIED                      3
#define GET_PHYS_ADDR_ROUTINE                   4
#define GET_HOST_CONFIG_ROM                     5
#define GET_HOST_CSR_CONTENTS                   6
#define GET_HOST_DMA_CAPABILITIES				7

//
// Definitions of capabilities in Host info level 2
//
#define HOST_INFO_PACKET_BASED                  1
#define HOST_INFO_STREAM_BASED                  2
#define HOST_INFO_SUPPORTS_ISOCH_STRIPPING      4
#define HOST_INFO_SUPPORTS_START_ON_CYCLE       8
#define HOST_INFO_SUPPORTS_RETURNING_ISO_HDR    16
#define HOST_INFO_SUPPORTS_ISO_HDR_INSERTION    32

//
// Definitions of Bus Reset flags (used when Bus driver asks Port driver
// to perform a bus reset)
//
#define BUS_RESET_FLAGS_PERFORM_RESET           1
#define BUS_RESET_FLAGS_FORCE_ROOT              2

//
// Definitions of flags for GetMaxSpeedBetweenDevices and
// Get1394AddressFromDeviceObject
//
#define USE_LOCAL_NODE                          1

//
// Definition of flags for BusResetNotification Irb
//
#define REGISTER_NOTIFICATION_ROUTINE           1
#define DEREGISTER_NOTIFICATION_ROUTINE         2

//
// Definitions of Speed flags used throughout 1394 Bus APIs
//
#define SPEED_FLAGS_100                         0x01
#define SPEED_FLAGS_200                         0x02
#define SPEED_FLAGS_400                         0x04
#define SPEED_FLAGS_800                         0x08
#define SPEED_FLAGS_1600                        0x10
#define SPEED_FLAGS_3200                        0x20

#define SPEED_FLAGS_FASTEST                     0x80000000

//
// Definitions of Isoch Allocate Resources flags
//
#define RESOURCE_USED_IN_LISTENING              1
#define RESOURCE_USED_IN_TALKING                2
#define RESOURCE_BUFFERS_CIRCULAR               4
#define RESOURCE_STRIP_ADDITIONAL_QUADLETS      8
#define RESOURCE_TIME_STAMP_ON_COMPLETION       16
#define RESOURCE_SYNCH_ON_TIME                  32
#define RESOURCE_USE_PACKET_BASED               64

//
// Definitions of Isoch Descriptor flags
//
#define DESCRIPTOR_SYNCH_ON_SY                  0x00000001
#define DESCRIPTOR_SYNCH_ON_TAG                 0x00000002
#define DESCRIPTOR_SYNCH_ON_TIME                0x00000004
#define DESCRIPTOR_USE_SY_TAG_IN_FIRST          0x00000008
#define DESCRIPTOR_TIME_STAMP_ON_COMPLETION     0x00000010
#define DESCRIPTOR_PRIORITY_TIME_DELIVERY       0x00000020
#define DESCRIPTOR_HEADER_SCATTER_GATHER        0x00000040

//
// Definitions of Isoch synchronization flags
//
#define SYNCH_ON_SY                             DESCRIPTOR_SYNCH_ON_SY
#define SYNCH_ON_TAG                            DESCRIPTOR_SYNCH_ON_TAG
#define SYNCH_ON_TIME                           DESCRIPTOR_SYNCH_ON_TIME

//
// flags for the SetPortProperties request
//
#define SET_LOCAL_HOST_PROPERTIES_NO_CYCLE_STARTS       0x00000001
#define SET_LOCAL_HOST_PROPERTIES_GAP_COUNT             0x00000002
#define SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM           0x00000003

//
// definition of Flags for SET_LOCAL_HOST_PROPERTIES_MODIFY_CROM
//
#define SLHP_FLAG_ADD_CROM_DATA         0x01
#define SLHP_FLAG_REMOVE_CROM_DATA      0x02

//
// Various Interesting 1394 IEEE 1212 locations
//
#define INITIAL_REGISTER_SPACE_HI               0xffff
//#define INITIAL_REGISTER_SPACE_LO               0xf0000000
#define TOPOLOGY_MAP_LOCATION                   0xf0001000
#define SPEED_MAP_LOCATION                      0xf0002000

//
// 1394 Add/Remove Virtual Device format
//
typedef struct _VIRT_DEVICE {
    ULONG           fulFlags;
    ULARGE_INTEGER  InstanceID;
    PSTR            DeviceID;
} VIRT_DEVICE, *PVIRT_DEVICE;

//
// 1394 Cycle Time format
//
typedef struct _CYCLE_TIME {
    ULONG               CL_CycleOffset:12;      // Bits 0-11
    ULONG               CL_CycleCount:13;       // Bits 12-24
    ULONG               CL_SecondCount:7;       // Bits 25-31
} CYCLE_TIME, *PCYCLE_TIME;

//
// 1394 Node Address format
//
typedef struct _NODE_ADDRESS {
    USHORT              NA_Node_Number:6;       // Bits 10-15
    USHORT              NA_Bus_Number:10;       // Bits 0-9
} NODE_ADDRESS, *PNODE_ADDRESS;

//
// 1394 Address Offset format (48 bit addressing)
//
typedef struct _ADDRESS_OFFSET {
    USHORT              Off_High;
    ULONG               Off_Low;
} ADDRESS_OFFSET, *PADDRESS_OFFSET;

//
// 1394 I/O Address format
//
typedef struct _IO_ADDRESS {
    NODE_ADDRESS        IA_Destination_ID;
    ADDRESS_OFFSET      IA_Destination_Offset;
} IO_ADDRESS, *PIO_ADDRESS;

//
// 1394 Self ID packet format
//
typedef struct _SELF_ID {
    ULONG               SID_Phys_ID:6;          // Byte 0 - Bits 0-5
    ULONG               SID_Packet_ID:2;        // Byte 0 - Bits 6-7
    ULONG               SID_Gap_Count:6;        // Byte 1 - Bits 0-5
    ULONG               SID_Link_Active:1;      // Byte 1 - Bit 6
    ULONG               SID_Zero:1;             // Byte 1 - Bit 7
    ULONG               SID_Power_Class:3;      // Byte 2 - Bits 0-2
    ULONG               SID_Contender:1;        // Byte 2 - Bit 3
    ULONG               SID_Delay:2;            // Byte 2 - Bits 4-5
    ULONG               SID_Speed:2;            // Byte 2 - Bits 6-7
    ULONG               SID_More_Packets:1;     // Byte 3 - Bit 0
    ULONG               SID_Initiated_Rst:1;    // Byte 3 - Bit 1
    ULONG               SID_Port3:2;            // Byte 3 - Bits 2-3
    ULONG               SID_Port2:2;            // Byte 3 - Bits 4-5
    ULONG               SID_Port1:2;            // Byte 3 - Bits 6-7
} SELF_ID, *PSELF_ID;

//
// Additional 1394 Self ID packet format (only used when More bit is on)
//
typedef struct _SELF_ID_MORE {
    ULONG               SID_Phys_ID:6;          // Byte 0 - Bits 0-5
    ULONG               SID_Packet_ID:2;        // Byte 0 - Bits 6-7
    ULONG               SID_PortA:2;            // Byte 1 - Bits 0-1
    ULONG               SID_Reserved2:2;        // Byte 1 - Bits 2-3
    ULONG               SID_Sequence:3;         // Byte 1 - Bits 4-6
    ULONG               SID_One:1;              // Byte 1 - Bit 7
    ULONG               SID_PortE:2;            // Byte 2 - Bits 0-1
    ULONG               SID_PortD:2;            // Byte 2 - Bits 2-3
    ULONG               SID_PortC:2;            // Byte 2 - Bits 4-5
    ULONG               SID_PortB:2;            // Byte 2 - Bits 6-7
    ULONG               SID_More_Packets:1;     // Byte 3 - Bit 0
    ULONG               SID_Reserved3:1;        // Byte 3 - Bit 1
    ULONG               SID_PortH:2;            // Byte 3 - Bits 2-3
    ULONG               SID_PortG:2;            // Byte 3 - Bits 4-5
    ULONG               SID_PortF:2;            // Byte 3 - Bits 6-7
} SELF_ID_MORE, *PSELF_ID_MORE;

//
// 1394 Topology Map format
//
typedef struct _TOPOLOGY_MAP {
    USHORT              TOP_Length;             // number of quadlets in map
    USHORT              TOP_CRC;                // 16 bit CRC defined by 1212
    ULONG               TOP_Generation;         // Generation number
    USHORT              TOP_Node_Count;         // Node count
    USHORT              TOP_Self_ID_Count;      // Number of Self IDs
    SELF_ID             TOP_Self_ID_Array[1];    // Array of Self IDs
} TOPOLOGY_MAP, *PTOPOLOGY_MAP;

//
// 1394 Speed Map format
//
typedef struct _SPEED_MAP {
    USHORT              SPD_Length;             // number of quadlets in map
    USHORT              SPD_CRC;                // 16 bit CRC defined by 1212
    ULONG               SPD_Generation;         // Generation number
    UCHAR               SPD_Speed_Code[4032];
} SPEED_MAP, *PSPEED_MAP;

//
// Definitions of the structures that correspond to the Host info levels
//
typedef struct _GET_LOCAL_HOST_INFO1 {
    LARGE_INTEGER       UniqueId;
} GET_LOCAL_HOST_INFO1, *PGET_LOCAL_HOST_INFO1;

typedef struct _GET_LOCAL_HOST_INFO2 {
    ULONG               HostCapabilities;
    ULONG               MaxAsyncReadRequest;
    ULONG               MaxAsyncWriteRequest;
} GET_LOCAL_HOST_INFO2, *PGET_LOCAL_HOST_INFO2;

typedef struct _GET_LOCAL_HOST_INFO3 {
    ULONG               deciWattsSupplied;
    ULONG               Voltage;                    // x10 -> +3.3 == 33
                                                    // +5.0 == 50,+12.0 == 120
                                                    // etc.
} GET_LOCAL_HOST_INFO3, *PGET_LOCAL_HOST_INFO3;

typedef struct _GET_LOCAL_HOST_INFO4 {
    PVOID               PhysAddrMappingRoutine;
    PVOID               Context;
} GET_LOCAL_HOST_INFO4, *PGET_LOCAL_HOST_INFO4;

//
// the caller can set ConfigRomLength to zero, issue the request, which will
// be failed with STATUS_INVALID_BUFFER_SIZE and the ConfigRomLength will be set
// by the port driver to the proper length. The caller can then re-issue the request
// after it has allocated a buffer for the configrom with the correct length
//
typedef struct _GET_LOCAL_HOST_INFO5 {
    PVOID                   ConfigRom;
    ULONG                   ConfigRomLength;
} GET_LOCAL_HOST_INFO5, *PGET_LOCAL_HOST_INFO5;

typedef struct _GET_LOCAL_HOST_INFO6 {
    ADDRESS_OFFSET          CsrBaseAddress;
    ULONG                   CsrDataLength;
    UCHAR                   CsrDataBuffer[1];
} GET_LOCAL_HOST_INFO6, *PGET_LOCAL_HOST_INFO6;

typedef struct _GET_LOCAL_HOST_INFO7
{
    ULONG                   HostDmaCapabilities;
    ULARGE_INTEGER          MaxDmaBufferSize;
} GET_LOCAL_HOST_INFO7, *PGET_LOCAL_HOST_INFO7;


//
// Definitions of the structures that correspond to the Host info levels
//
typedef struct _SET_LOCAL_HOST_PROPS2 {
    ULONG       GapCountLowerBound;
} SET_LOCAL_HOST_PROPS2, *PSET_LOCAL_HOST_PROPS2;

typedef struct _SET_LOCAL_HOST_PROPS3 {
    ULONG       fulFlags;
    HANDLE      hCromData;
    ULONG       nLength;
    UCHAR       Buffer[1];
//    PMDL        Mdl;
} SET_LOCAL_HOST_PROPS3, *PSET_LOCAL_HOST_PROPS3;

//
// 1394 Phy Configuration packet format
//
typedef struct _PHY_CONFIGURATION_PACKET {
    ULONG               PCP_Phys_ID:6;          // Byte 0 - Bits 0-5
    ULONG               PCP_Packet_ID:2;        // Byte 0 - Bits 6-7
    ULONG               PCP_Gap_Count:6;        // Byte 1 - Bits 0-5
    ULONG               PCP_Set_Gap_Count:1;    // Byte 1 - Bit 6
    ULONG               PCP_Force_Root:1;       // Byte 1 - Bit 7
    ULONG               PCP_Reserved1:8;        // Byte 2 - Bits 0-7
    ULONG               PCP_Reserved2:8;        // Byte 3 - Bits 0-7
    ULONG               PCP_Inverse;            // Inverse quadlet
} PHY_CONFIGURATION_PACKET, *PPHY_CONFIGURATION_PACKET;

#endif // DRIVER

//
// struct used to pass in with IOCTL_ALLOCATE_ADDRESS_RANGE
//
typedef struct _ALLOCATE_ADDRESS_RANGE {
    ULONG           fulAllocateFlags;
    ULONG           fulFlags;
    ULONG           nLength;
    ULONG           MaxSegmentSize;
    ULONG           fulAccessType;
    ULONG           fulNotificationOptions;
    ADDRESS_OFFSET  Required1394Offset;
    HANDLE          hAddressRange;
    UCHAR           Data[1];
} ALLOCATE_ADDRESS_RANGE, *PALLOCATE_ADDRESS_RANGE;

//
// struct used to pass in with IOCTL_ASYNC_READ
//
typedef struct _ASYNC_READ {
    ULONG           bRawMode;
    ULONG           bGetGeneration;
    IO_ADDRESS      DestinationAddress;
    ULONG           nNumberOfBytesToRead;
    ULONG           nBlockSize;
    ULONG           fulFlags;
    ULONG           ulGeneration;
    UCHAR           Data[1];
} ASYNC_READ, *PASYNC_READ;

//
// struct used to pass in with IOCTL_SET_ADDRESS_DATA
//
typedef struct _SET_ADDRESS_DATA {
    HANDLE          hAddressRange;
    ULONG           nLength;
    ULONG           ulOffset;
    UCHAR           Data[1];
} SET_ADDRESS_DATA, *PSET_ADDRESS_DATA,
    GET_ADDRESS_DATA, *PGET_ADDRESS_DATA;

//
// struct used to pass in with IOCTL_ASYNC_WRITE
//
typedef struct _ASYNC_WRITE {
    ULONG           bRawMode;
    ULONG           bGetGeneration;
    IO_ADDRESS      DestinationAddress;
    ULONG           nNumberOfBytesToWrite;
    ULONG           nBlockSize;
    ULONG           fulFlags;
    ULONG           ulGeneration;
    UCHAR           Data[1];
} ASYNC_WRITE, *PASYNC_WRITE;

//
// struct used to pass in with IOCTL_ASYNC_LOCK
//
typedef struct _ASYNC_LOCK {
    ULONG           bRawMode;
    ULONG           bGetGeneration;
    IO_ADDRESS      DestinationAddress;
    ULONG           nNumberOfArgBytes;
    ULONG           nNumberOfDataBytes;
    ULONG           fulTransactionType;
    ULONG           fulFlags;
    ULONG           Arguments[2];
    ULONG           DataValues[2];
    ULONG           ulGeneration;
    ULONG           Buffer[2];
} ASYNC_LOCK, *PASYNC_LOCK;

//
// struct used to pass in with IOCTL_ASYNC_STREAM
//
typedef struct _ASYNC_STREAM {
    ULONG           nNumberOfBytesToStream;
    ULONG           fulFlags;
    ULONG           ulTag;
    ULONG           nChannel;
    ULONG           ulSynch;
    ULONG           nSpeed;
    UCHAR           Data[1];
} ASYNC_STREAM, *PASYNC_STREAM;

//
// struct used to pass in with IOCTL_ISOCH_ALLOCATE_BANDWIDTH
//
typedef struct _ISOCH_ALLOCATE_BANDWIDTH {
    ULONG           nMaxBytesPerFrameRequested;
    ULONG           fulSpeed;
    HANDLE          hBandwidth;
    ULONG           BytesPerFrameAvailable;
    ULONG           SpeedSelected;
} ISOCH_ALLOCATE_BANDWIDTH, *PISOCH_ALLOCATE_BANDWIDTH;

//
// struct used to pass in with IOCTL_ISOCH_ALLOCATE_CHANNEL
//
typedef struct _ISOCH_ALLOCATE_CHANNEL {
    ULONG           nRequestedChannel;
    ULONG           Channel;
    LARGE_INTEGER   ChannelsAvailable;
} ISOCH_ALLOCATE_CHANNEL, *PISOCH_ALLOCATE_CHANNEL;

//
// struct used to pass in with IOCTL_ISOCH_ALLOCATE_RESOURCES
//
typedef struct _ISOCH_ALLOCATE_RESOURCES {
    ULONG           fulSpeed;
    ULONG           fulFlags;
    ULONG           nChannel;
    ULONG           nMaxBytesPerFrame;
    ULONG           nNumberOfBuffers;
    ULONG           nMaxBufferSize;
    ULONG           nQuadletsToStrip;
    HANDLE          hResource;
} ISOCH_ALLOCATE_RESOURCES, *PISOCH_ALLOCATE_RESOURCES;

//
// struct used to pass in isoch descriptors
//
typedef struct _RING3_ISOCH_DESCRIPTOR {
    ULONG           fulFlags;
    ULONG           ulLength;
    ULONG           nMaxBytesPerFrame;
    ULONG           ulSynch;
    ULONG           ulTag;
    CYCLE_TIME      CycleTime;
    ULONG           bUseCallback;
    ULONG           bAutoDetach;
    UCHAR           Data[1];
} RING3_ISOCH_DESCRIPTOR, *PRING3_ISOCH_DESCRIPTOR;

//
// struct used to pass in with IOCTL_ISOCH_ATTACH_BUFFERS
//
typedef struct _ISOCH_ATTACH_BUFFERS {
    HANDLE                  hResource;
    ULONG                   nNumberOfDescriptors;
    ULONG                   ulBufferSize;
    PULONG                  pIsochDescriptor;
    RING3_ISOCH_DESCRIPTOR  R3_IsochDescriptor[1];
} ISOCH_ATTACH_BUFFERS, *PISOCH_ATTACH_BUFFERS;

//
// struct used to pass in with IOCTL_ISOCH_DETACH_BUFFERS
//
typedef struct _ISOCH_DETACH_BUFFERS {
    HANDLE          hResource;
    ULONG           nNumberOfDescriptors;
    PULONG          pIsochDescriptor;
} ISOCH_DETACH_BUFFERS, *PISOCH_DETACH_BUFFERS;

//
// struct used to pass in with IOCTL_ISOCH_LISTEN
//
typedef struct _ISOCH_LISTEN {
    HANDLE          hResource;
    ULONG           fulFlags;
    CYCLE_TIME      StartTime;
} ISOCH_LISTEN, *PISOCH_LISTEN;

//
// struct used to pass in with IOCTL_ISOCH_QUERY_RESOURCES
//
typedef struct _ISOCH_QUERY_RESOURCES {
    ULONG           fulSpeed;
    ULONG           BytesPerFrameAvailable;
    LARGE_INTEGER   ChannelsAvailable;
} ISOCH_QUERY_RESOURCES, *PISOCH_QUERY_RESOURCES;

//
// struct used to pass in with IOCTL_ISOCH_SET_CHANNEL_BANDWIDTH
//
typedef struct _ISOCH_SET_CHANNEL_BANDWIDTH {
    HANDLE          hBandwidth;
    ULONG           nMaxBytesPerFrame;
} ISOCH_SET_CHANNEL_BANDWIDTH, *PISOCH_SET_CHANNEL_BANDWIDTH;

//
// struct used to pass in with IOCTL_ISOCH_MODIFY_STREAM_PROPERTIES
//
typedef struct _ISOCH_MODIFY_STREAM_PROPERTIES {
    HANDLE            hResource;
    ULARGE_INTEGER    ChannelMask;
    ULONG             fulSpeed;
} ISOCH_MODIFY_STREAM_PROPERTIES, *PISOCH_MODIFY_STREAM_PROPERTIES;

//
// struct used to pass in with IOCTL_ISOCH_STOP
//
typedef struct _ISOCH_STOP {
    HANDLE          hResource;
    ULONG           fulFlags;
} ISOCH_STOP, *PISOCH_STOP;

//
// struct used to pass in with IOCTL_ISOCH_TALK
//
typedef struct _ISOCH_TALK {
    HANDLE          hResource;
    ULONG           fulFlags;
    CYCLE_TIME      StartTime;
} ISOCH_TALK, *PISOCH_TALK;

//
// struct used to pass in with IOCTL_GET_LOCAL_HOST_INFORMATION
//
typedef struct _GET_LOCAL_HOST_INFORMATION {
    ULONG           Status;
    ULONG           nLevel;
    ULONG           ulBufferSize;
    UCHAR           Information[1];
} GET_LOCAL_HOST_INFORMATION, *PGET_LOCAL_HOST_INFORMATION;

//
// struct used to pass in with IOCTL_GET_1394_ADDRESS_FROM_DEVICE_OBJECT
//
typedef struct _GET_1394_ADDRESS {
    ULONG           fulFlags;
    NODE_ADDRESS    NodeAddress;
} GET_1394_ADDRESS, *PGET_1394_ADDRESS;

//
// struct used to pass in with IOCTL_GET_MAX_SPEED_BETWEEN_DEVICES
//
typedef struct _GET_MAX_SPEED_BETWEEN_DEVICES {
    ULONG           fulFlags;
    ULONG           ulNumberOfDestinations;
    HANDLE          hDestinationDeviceObjects[64];
    ULONG           fulSpeed;
} GET_MAX_SPEED_BETWEEN_DEVICES, *PGET_MAX_SPEED_BETWEEN_DEVICES;

//
// struct used to pass in with IOCTL_SET_DEVICE_XMIT_PROPERTIES
//
typedef struct _DEVICE_XMIT_PROPERTIES {
    ULONG           fulSpeed;
    ULONG           fulPriority;
} DEVICE_XMIT_PROPERTIES, *PDEVICE_XMIT_PROPERTIES;

//
// struct used to pass in with IOCTL_SET_LOCAL_HOST_INFORMATION
//
typedef struct _SET_LOCAL_HOST_INFORMATION {
    ULONG           nLevel;
    ULONG           ulBufferSize;
    UCHAR           Information[1];
} SET_LOCAL_HOST_INFORMATION, *PSET_LOCAL_HOST_INFORMATION;

//
// define's used to make sure the dll/sys driver are in synch
//
#define DIAGNOSTIC_VERSION                      1
#define DIAGNOSTIC_SUB_VERSION                  1

//
// struct used to pass in with IOCTL_GET_DIAG_VERSION
//
typedef struct _VERSION_DATA {
    ULONG           ulVersion;
    ULONG           ulSubVersion;
} VERSION_DATA, *PVERSION_DATA;

#ifndef DRIVER

//
// struct used with start/stop async loopback
//
typedef struct _ASYNC_LOOPBACK_PARAMS {
    HWND                            hWnd;
    PSTR                            szDeviceName;
    BOOLEAN                         bKill;
    HANDLE                          hThread;
    UINT                            ThreadId;
    ULONG                           ulLoopFlag;
    ULONG                           nIterations;
    ULONG                           ulPass;
    ULONG                           ulFail;
    ULONG                           ulIterations;
    ADDRESS_OFFSET                  AddressOffset;
    ULONG                           nMaxBytes;
} ASYNC_LOOPBACK_PARAMS, *PASYNC_LOOPBACK_PARAMS;

//
// struct used with start/stop async loopbac ex
//
typedef struct _ASYNC_LOOPBACK_PARAMS_EX {
    HWND                            hWnd;
    PSTR                            szDeviceName;
    BOOLEAN                         bKill;
    BOOLEAN                         bRawMode;
    HANDLE                          hThread;
    UINT                            ThreadId;
    ULONG                           ulLoopFlag;
    ULONG                           nIterations;
    ULONG                           ulPass;
    ULONG                           ulFail;
    ULONG                           ulIterations;
    IO_ADDRESS                      Destination;
    ULONG                           nMaxBytes;
} ASYNC_LOOPBACK_PARAMS_EX, *PASYNC_LOOPBACK_PARAMS_EX;


//
// struct used with start/stop isoch loopback
//
typedef struct _ISOCH_LOOPBACK_PARAMS {
    HWND                            hWnd;
    PSTR                            szDeviceName;
    BOOLEAN                         bKill;
    BOOLEAN                         bLoopback;
    BOOLEAN                         bAutoFill;
    HANDLE                          hThread;
    UINT                            ThreadId;
    ULONG                           ulLoopFlag;
    ULONG                           nIterations;
    ULONG                           ulPass;
    ULONG                           ulFail;
    ULONG                           ulIterations;
    BOOLEAN                         bAutoAlloc;
    ISOCH_ATTACH_BUFFERS            isochAttachBuffers;
} ISOCH_LOOPBACK_PARAMS, *PISOCH_LOOPBACK_PARAMS;

//
// struct used with start/stop async stream loopback
//
typedef struct _ASYNC_STREAM_LOOPBACK_PARAMS {
    HWND                            hWnd;           // filled in by 1394api.dll
    PSTR                            szDeviceName;   // filled in by 1394api.dll
    BOOLEAN                         bKill;          // filled in by 1394api.dll
    HANDLE                          hThread;        // filled in by 1394api.dll
    UINT                            ThreadId;       // filled in by 1394api.dll
    ULONG                           ulPass;         // filled in by 1394api.dll
    ULONG                           ulFail;         // filled in by 1394api.dll
    ULONG                           ulIterations;   // filled in by 1394api.dll
    BOOLEAN                         bAutoAlloc;     // filled in by app
    BOOLEAN                         bAutoFill;      // filled in by app
    ULONG                           nIterations;    // filled in by app
    ASYNC_STREAM                    asyncStream;    // filled in by app
} ASYNC_STREAM_LOOPBACK_PARAMS, *PASYNC_STREAM_LOOPBACK_PARAMS;

typedef struct _DEVICE_LIST {
    CHAR            DeviceName[255];
} DEVICE_LIST, *PDEVICE_LIST;

typedef struct _DEVICE_DATA {
    ULONG           numDevices;
    DEVICE_LIST     deviceList[10];
} DEVICE_DATA, *PDEVICE_DATA;

#if defined _X86_

ULONG static __inline
bswap (ULONG value)
{
    __asm mov eax, value
    __asm bswap eax
}

#else

#define bswap(value)    (((ULONG) (value)) << 24 |\
                        (((ULONG) (value)) & 0x0000FF00) << 8 |\
                        (((ULONG) (value)) & 0x00FF0000) >> 8 |\
                        ((ULONG) (value)) >> 24)

#endif

#define bswapw(value)   (((USHORT) (value)) << 8 |\
                        (((USHORT) (value)) & 0xff00) >> 8)

//
// prototypes
//
DWORD
WINAPI
RegisterClient(
    HWND    hWnd
    );

DWORD
WINAPI
DeRegisterClient(
    HWND    hWnd
    );

DWORD
WINAPI
GetDeviceList(
    PDEVICE_DATA    DeviceData
    );

DWORD
WINAPI
GetVirtualDeviceList(
    PDEVICE_DATA    DeviceData
    );

void
WINAPI
DiagnosticMode(
    HWND    hWnd,
    PSTR    szBusName,
    BOOL    bMode,
    BOOL    bAll
    );

DWORD
WINAPI
AddVirtualDriver (
    HWND            hWnd,
    PVIRT_DEVICE    pVirutualDevice,
    ULONG           BusNumber
    );

DWORD
WINAPI
RemoveVirtualDriver (
    HWND            hWnd,
    PVIRT_DEVICE    pVirutualDevice,
    ULONG           BusNumber
    );

DWORD
WINAPI
SetDebugSpew(
    HWND    hWnd,
    ULONG   SpewLevel
    );

ULONG
WINAPI
GetDiagVersion(
    HWND            hWnd,
    PSTR            szDeviceName,
    PVERSION_DATA   Version,
    BOOL            bMatch
    );

ULONG
WINAPI
AllocateAddressRange(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PALLOCATE_ADDRESS_RANGE     allocateAddressRange,
    BOOL                        bAutoAlloc
    );

ULONG
WINAPI
FreeAddressRange(
    HWND        hWnd,
    PSTR        szDeviceName,
    HANDLE      hAddressRange
    );

ULONG
WINAPI
SetAddressData(
    HWND                hWnd,
    PSTR                szDeviceName,
    PSET_ADDRESS_DATA   setAddressData
    );

ULONG
WINAPI
GetAddressData (
    HWND                hWnd,
    PSTR                szDeviceName,
    PGET_ADDRESS_DATA   getAddressData
    );

ULONG
WINAPI
AsyncRead(
    HWND            hWnd,
    PSTR            szDeviceName,
    PASYNC_READ     asyncRead,
    BOOL            bAutoAlloc
    );

ULONG
WINAPI
AsyncWrite(
    HWND            hWnd,
    PSTR            szDeviceName,
    PASYNC_WRITE    asyncWrite,
    BOOL            bAutoAlloc
    );

ULONG
WINAPI
AsyncLock(
    HWND            hWnd,
    PSTR            szDeviceName,
    PASYNC_LOCK     asyncLock
    );

ULONG
WINAPI
AsyncStream(
    HWND            hWnd,
    PSTR            szDeviceName,
    PASYNC_STREAM   asyncStream,
    BOOL            bAutoAlloc
    );

void
WINAPI
AsyncStartLoopback(
    HWND                    hWnd,
    PSTR                    szDeviceName,
    PASYNC_LOOPBACK_PARAMS  asyncLoopbackParams
    );

void
WINAPI
AsyncStopLoopback(
    PASYNC_LOOPBACK_PARAMS  asyncLoopbackParams
    );

void
WINAPI
AsyncStartLoopbackEx(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PASYNC_LOOPBACK_PARAMS_EX   asyncLoopbackParams
    );

void
WINAPI
AsyncStopLoopbackEx(
    PASYNC_LOOPBACK_PARAMS_EX  asyncLoopbackParams
    );



void
WINAPI
AsyncStreamStartLoopback(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PASYNC_STREAM_LOOPBACK_PARAMS   streamLoopbackParams
    );

void
WINAPI
AsyncStreamStopLoopback(
    PASYNC_STREAM_LOOPBACK_PARAMS   streamLoopbackParams
    );

ULONG
WINAPI
IsochAllocateBandwidth(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_ALLOCATE_BANDWIDTH   isochAllocateBandwidth
    );

ULONG
WINAPI
IsochAllocateChannel(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_ALLOCATE_CHANNEL     isochAllocateChannel
    );

ULONG
WINAPI
IsochAllocateResources(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_ALLOCATE_RESOURCES   isochAllocateResources
    );

ULONG
WINAPI
IsochAttachBuffers(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_ATTACH_BUFFERS       isochAttachBuffers,
    BOOL                        bAutoAlloc,
    BOOL                        bAutoFill
    );

ULONG
WINAPI
IsochDetachBuffers(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_DETACH_BUFFERS       isochDetachBuffers
    );

ULONG
WINAPI
IsochFreeBandwidth(
    HWND    hWnd,
    PSTR    szDeviceName,
    HANDLE  hBandwidth
    );

ULONG
WINAPI
IsochFreeChannel(
    HWND    hWnd,
    PSTR    szDeviceName,
    ULONG   nChannel
    );

ULONG
WINAPI
IsochFreeResources(
    HWND    hWnd,
    PSTR    szDeviceName,
    HANDLE  hResource
    );

ULONG
WINAPI
IsochListen(
    HWND            hWnd,
    PSTR            szDeviceName,
    PISOCH_LISTEN   isochListen
    );

ULONG
WINAPI
IsochQueryCurrentCycleTime(
    HWND            hWnd,
    PSTR            szDeviceName,
    PCYCLE_TIME     CycleTime
    );

ULONG
WINAPI
IsochQueryResources(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PISOCH_QUERY_RESOURCES      isochQueryResources
    );

ULONG
WINAPI
IsochSetChannelBandwidth(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PISOCH_SET_CHANNEL_BANDWIDTH    isochSetChannelBandwidth
    );

ULONG
WINAPI
IsochStop(
    HWND            hWnd,
    PSTR            szDeviceName,
    PISOCH_STOP     isochStop
    );

ULONG
WINAPI
IsochTalk(
    HWND            hWnd,
    PSTR            szDeviceName,
    PISOCH_TALK     isochTalk
    );

void
WINAPI
IsochStartLoopback(
    HWND                    hWnd,
    PSTR                    szDeviceName,
    PISOCH_LOOPBACK_PARAMS  isochLoopbackParams
    );

void
WINAPI
IsochStopLoopback(
    PISOCH_LOOPBACK_PARAMS  isochLoopbackParams
    );

ULONG
WINAPI
GetLocalHostInformation(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PGET_LOCAL_HOST_INFORMATION     GetLocalHostInfo,
    BOOL                            bAutoAlloc
    );

ULONG
WINAPI
Get1394AddressFromDeviceObject(
    HWND                hWnd,
    PSTR                szDeviceName,
    PGET_1394_ADDRESS   Get1394Address
    );

ULONG
WINAPI
Control(
    HWND    hWnd,
    PSTR    szDeviceName
    );

ULONG
WINAPI
GetMaxSpeedBetweenDevices(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PGET_MAX_SPEED_BETWEEN_DEVICES  GetMaxSpeedBetweenDevices
    );

ULONG
WINAPI
SetDeviceXmitProperties(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PDEVICE_XMIT_PROPERTIES     DeviceXmitProperties
    );

ULONG
WINAPI
GetConfigurationInformation(
    HWND    hWnd,
    PSTR    szDeviceName
    );

ULONG
WINAPI
BusReset(
    HWND    hWnd,
    PSTR    szDeviceName,
    ULONG   fulFlags
    );

ULONG
WINAPI
GetGenerationCount(
    HWND    hWnd,
    PSTR    szDeviceName,
    PULONG  GenerationCount
    );

ULONG
WINAPI
SendPhyConfigurationPacket(
    HWND                        hWnd,
    PSTR                        szDeviceName,
    PPHY_CONFIGURATION_PACKET   PhyConfigurationPacket
    );

ULONG
WINAPI
BusResetNotification(
    HWND    hWnd,
    PSTR    szDeviceName,
    ULONG   fulFlags
    );

ULONG
WINAPI
SetLocalHostInformation(
    HWND                            hWnd,
    PSTR                            szDeviceName,
    PSET_LOCAL_HOST_INFORMATION     SetLocalHostInfo
    );

#endif // DRIVER

#ifdef __cplusplus
}
#endif

#endif // _1394API_H_

