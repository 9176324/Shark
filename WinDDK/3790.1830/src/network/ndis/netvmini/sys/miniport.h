/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

   Miniport.H

Abstract:
    This module contains structure definitons and function prototypes.

Revision History:

Notes:

--*/


#ifndef _MINIPORT_H
#define _MINIPORT_H

#include <ndis.h>

#if defined(NDIS50_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION       5
    #define MP_NDIS_MINOR_VERSION       0
#elif defined(NDIS51_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION       5
    #define MP_NDIS_MINOR_VERSION       1
#else
#error Unsupported NDIS version
#endif

#define     ETH_HEADER_SIZE             14
#define     ETH_MAX_DATA_SIZE           1500
#define     ETH_MAX_PACKET_SIZE         ETH_HEADER_SIZE + ETH_MAX_DATA_SIZE
#define     ETH_MIN_PACKET_SIZE         60

#define ETH_IS_LOCALLY_ADMINISTERED(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x02))
    

#define NIC_TAG                             ((ULONG)'NIMV')

// media type, we use ethernet, change if necessary
#define NIC_MEDIA_TYPE                    NdisMedium802_3

// we use Internal, change to Pci, Isa, etc. properly
#define NIC_INTERFACE_TYPE                NdisInterfaceInternal     

// change to your company name instead of using Microsoft
#define NIC_VENDOR_DESC                 "Microsoft"

// Highest byte is the NIC byte plus three vendor bytes, they are normally  
// obtained from the NIC 
#define NIC_VENDOR_ID                    0x00FFFFFF   

// Update the driver version number every time you release a new driver
// The high word is the major version. The low word is the minor version. 
// Also make sure that VER_FILEVERSION specified in the .RC file also
// matches with the driver version because NDISTESTER checks for that.
//
#define NIC_VENDOR_DRIVER_VERSION       0x00010000

#define NIC_MAX_MCAST_LIST              32
#define NIC_MAX_BUSY_SENDS              20
#define NIC_MAX_SEND_PKTS               5
#define NIC_MAX_BUSY_RECVS              20
#define NIC_MAX_LOOKAHEAD               ETH_MAX_DATA_SIZE
#define NIC_BUFFER_SIZE                 ETH_MAX_PACKET_SIZE
#define NIC_LINK_SPEED                  1000000    // in 100 bps 


#define NIC_SUPPORTED_FILTERS ( \
                NDIS_PACKET_TYPE_DIRECTED   | \
                NDIS_PACKET_TYPE_MULTICAST  | \
                NDIS_PACKET_TYPE_BROADCAST  | \
                NDIS_PACKET_TYPE_PROMISCUOUS | \
                NDIS_PACKET_TYPE_ALL_MULTICAST)

#define fMP_RESET_IN_PROGRESS               0x00000001
#define fMP_DISCONNECTED                    0x00000002 
#define fMP_ADAPTER_HALT_IN_PROGRESS        0x00000004
#define fMP_ADAPTER_SURPRISE_REMOVED         0x00000008
#define fMP_ADAPTER_RECV_LOOKASIDE          0x00000010

//
// Buffer size passed in NdisMQueryAdapterResources                            
// We should only need three adapter resources (IO, interrupt and memory),
// Some devices get extra resources, so have room for 10 resources 
//
#define NIC_RESOURCE_BUF_SIZE           (sizeof(NDIS_RESOURCE_LIST) + \
                                        (10*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)))

//
// Message verbosity: lower values indicate higher urgency
//
#define MP_LOUD    4
#define MP_INFO    3
#define MP_TRACE   2
#define MP_WARNING 1
#define MP_ERROR   0

extern INT MPDebugLevel;    

#if DBG
#define DEBUGP(Level, Fmt) \
{ \
    if (Level <= MPDebugLevel) \
    { \
        DbgPrint("NetVMini.SYS:"); \
        DbgPrint Fmt; \
    } \
}
#else 
#define DEBUGP(Level, Fmt)
#endif


#ifndef min
#define    min(_a, _b)      (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define    max(_a, _b)      (((_a) > (_b)) ? (_a) : (_b))
#endif

//--------------------------------------
// Utility macros        
//--------------------------------------

#define MP_SET_FLAG(_M, _F)             ((_M)->Flags |= (_F))
#define MP_CLEAR_FLAG(_M, _F)            ((_M)->Flags &= ~(_F))
#define MP_TEST_FLAG(_M, _F)            (((_M)->Flags & (_F)) != 0)
#define MP_TEST_FLAGS(_M, _F)            (((_M)->Flags & (_F)) == (_F))

#define MP_IS_READY(_M)        (((_M)->Flags & \
                                 (fMP_DISCONNECTED | fMP_RESET_IN_PROGRESS | fMP_ADAPTER_HALT_IN_PROGRESS)) == 0) 

#define MP_INC_REF(_A)              NdisInterlockedIncrement(&(_A)->RefCount)

#define MP_DEC_REF(_A) {\
                            NdisInterlockedDecrement(&(_A)->RefCount);\
                            ASSERT(_A->RefCount >= 0);\
                            if((_A)->RefCount == 0){\
                                NdisSetEvent(&(_A)->RemoveEvent);\
                            }\
                        }

#define MP_GET_REF(_A)              ((_A)->RefCount)

typedef struct _MP_GLOBAL_DATA
{
    LIST_ENTRY      AdapterList;
    NDIS_SPIN_LOCK  Lock;
} MP_GLOBAL_DATA, *PMP_GLOBAL_DATA;

extern MP_GLOBAL_DATA GlobalData;

// TCB (Transmit Control Block)
typedef struct _TCB
{
    LIST_ENTRY              List; // This should be first in the list
    LONG                    Ref;
    PVOID                   Adapter;
    PNDIS_BUFFER            Buffer;
    PNDIS_PACKET            OrgSendPacket;
    PUCHAR                  pData;        
    ULONG                   ulSize;
    UCHAR                   Data[NIC_BUFFER_SIZE];
} TCB, *PTCB;

// RCB (Receive Control Block)
typedef struct _RCB
{
    LIST_ENTRY              List;  // This should be first in the list
    PNDIS_PACKET            Packet;
} RCB, *PRCB;

typedef struct _MP_ADAPTER
{
    LIST_ENTRY              List;
    LONG                    RefCount;
    NDIS_EVENT              RemoveEvent;    
    //
    // Keep track of various device objects.
    //
#if defined(NDIS_WDM)

    PDEVICE_OBJECT          Pdo; 
    PDEVICE_OBJECT          Fdo; 
    PDEVICE_OBJECT          NextDeviceObject; 
#endif
    NDIS_HANDLE             AdapterHandle;    
    ULONG                   Flags;
    UCHAR                   PermanentAddress[ETH_LENGTH_OF_ADDRESS];
    UCHAR                   CurrentAddress[ETH_LENGTH_OF_ADDRESS];
    //
    // Variables to track resources for the send operation
    //
    NDIS_HANDLE             SendBufferPoolHandle;
    LIST_ENTRY              SendFreeList;
    LIST_ENTRY              SendWaitList;
    PUCHAR                  TCBMem;
    LONG                    nBusySend;
    UINT                    RegNumTcb;// number of transmit control blocks the registry says
    NDIS_SPIN_LOCK          SendLock;      
    //
    // Variables to track resources for the Reset operation
    //
    NDIS_TIMER              ResetTimer;
    LONG                    nResetTimerCount;    
    //
    // Variables to track resources for the Receive operation
    //
    NPAGED_LOOKASIDE_LIST   RecvLookaside;
    LIST_ENTRY              RecvFreeList;
    LIST_ENTRY              RecvWaitList;
    NDIS_SPIN_LOCK          RecvLock;
    LONG                    nBusyRecv;
    NDIS_HANDLE             RecvPacketPoolHandle;
    NDIS_HANDLE             RecvPacketPool; // not used 
    NDIS_HANDLE             RecvBufferPool; // not used 
    NDIS_TIMER              RecvTimer;
    
    //
    // Packet Filter and look ahead size.
    //
    ULONG                   PacketFilter;
    ULONG                   ulLookAhead;
    ULONG                   ulLinkSpeed;
    ULONG                   ulMaxBusySends;
    ULONG                   ulMaxBusyRecvs;

    // multicast list
    ULONG                   ulMCListSize;
    UCHAR                   MCList[NIC_MAX_MCAST_LIST][ETH_LENGTH_OF_ADDRESS];
    
    // Packet counts
    ULONG64                 GoodTransmits;
    ULONG64                 GoodReceives;
    ULONG                   NumTxSinceLastAdjust;

    // Count of transmit errors
    ULONG                   TxAbortExcessCollisions;
    ULONG                   TxLateCollisions;
    ULONG                   TxDmaUnderrun;
    ULONG                   TxLostCRS;
    ULONG                   TxOKButDeferred;
    ULONG                   OneRetry;
    ULONG                   MoreThanOneRetry;
    ULONG                   TotalRetries;
    ULONG                   TransmitFailuresOther;

    // Count of receive errors
    ULONG                   RcvCrcErrors;
    ULONG                   RcvAlignmentErrors;
    ULONG                   RcvResourceErrors;
    ULONG                   RcvDmaOverrunErrors;
    ULONG                   RcvCdtFrames;
    ULONG                   RcvRuntErrors;
    
} MP_ADAPTER, *PMP_ADAPTER;


//--------------------------------------
// Miniport routines
//--------------------------------------

NDIS_STATUS 
DriverEntry(
    IN PVOID DriverObject,
    IN PVOID RegistryPath);

NDIS_STATUS 
MPInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext);

VOID 
MPAllocateComplete(
    NDIS_HANDLE MiniportAdapterContext,
    IN PVOID VirtualAddress,
    IN PNDIS_PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length,
    IN PVOID Context);

BOOLEAN 
MPCheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext);

VOID 
MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext);

VOID 
MPHandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext);

VOID 
MPUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );

VOID 
MPDisableInterrupt(
    IN PVOID Adapter);

VOID 
MPEnableInterrupt(
    IN PVOID Adapter);
                              
VOID 
MPIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS 
MPQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded);

NDIS_STATUS 
MPReset(
    OUT PBOOLEAN AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext);

VOID 
MPReturnPacket(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_PACKET Packet);

VOID 
MPSendPackets(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets);

NDIS_STATUS
MPSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded);

VOID 
MPShutdown(
    IN  NDIS_HANDLE MiniportAdapterContext);

                                                      
NDIS_STATUS 
NICSendPacket(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Pakcet);
    
BOOLEAN 
NICCopyPacket(
    PMP_ADAPTER Adapter,
    PTCB pTCB, 
    PNDIS_PACKET Packet);
    
VOID 
NICQueuePacketForRecvIndication(
    PMP_ADAPTER Adapter,
    PTCB pTCB);
                  
VOID 
NICFreeRecvPacket(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet);
    
VOID 
NICFreeSendTCB(
    IN PMP_ADAPTER Adapter,
    IN PTCB pTCB);

VOID 
NICResetCompleteTimerDpc(
    IN    PVOID                    SystemSpecific1,
    IN    PVOID                    FunctionContext,
    IN    PVOID                    SystemSpecific2,
    IN    PVOID                    SystemSpecific3);
    
VOID 
NICFreeQueuedSendPackets(
    PMP_ADAPTER Adapter
    );
    
  
NDIS_STATUS 
NICInitializeAdapter(
    IN PMP_ADAPTER Adapter, 
    IN  NDIS_HANDLE  WrapperConfigurationContext
);
                                
NDIS_STATUS 
NICAllocAdapter(
    PMP_ADAPTER *Adapter
    );

void 
NICFreeAdapter(
    PMP_ADAPTER Adapter
    );
                                                          
void 
NICAttachAdapter(
    PMP_ADAPTER Adapter
    );

void 
NICDetachAdapter(
    PMP_ADAPTER Adapter
    );
                   
NDIS_STATUS 
NICReadRegParameters(
    PMP_ADAPTER Adapter,
    NDIS_HANDLE ConfigurationHandle);

NDIS_STATUS 
NICGetStatsCounters(
    PMP_ADAPTER Adapter, 
    NDIS_OID Oid,
    PULONG pCounter);
    
NDIS_STATUS
NICSetPacketFilter(
    IN PMP_ADAPTER Adapter,
    IN ULONG PacketFilter);

NDIS_STATUS NICSetMulticastList(
    IN PMP_ADAPTER              Adapter,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength,
    OUT PULONG                  pBytesRead,
    OUT PULONG                  pBytesNeeded
    );
    
ULONG
NICGetMediaConnectStatus(
    PMP_ADAPTER Adapter);

#ifdef NDIS51_MINIPORT

VOID 
MPCancelSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId
    );
VOID MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    );

#endif

VOID
NICIndicateReceiveTimerDpc(
    IN PVOID             SystemSpecific1,
    IN PVOID             FunctionContext,
    IN PVOID             SystemSpecific2,
    IN PVOID             SystemSpecific3);

BOOLEAN
NICIsPacketTransmittable(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet
    );

VOID 
NICFreeQueuedRecvPackets(
    PMP_ADAPTER Adapter
    );

PUCHAR 
DbgGetOidName
    (ULONG oid
    );             


#if defined(IOCTL_INTERFACE)
NDIS_STATUS
NICRegisterDevice(
    VOID
    );

NDIS_STATUS
NICDeregisterDevice(
    VOID
    );
NTSTATUS
NICDispatch(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp
    );

#else

#define NICRegisterDevice()
#define NICDeregisterDevice()
#define NICDispatch()

#endif


#endif    // _MINIPORT_H


