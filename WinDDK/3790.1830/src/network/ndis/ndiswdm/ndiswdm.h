/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

   ndiswdm.H

Abstract:
    This module contains structure definitons and function prototypes.

Author:  Eliyas Yakub (Jan 11, 2003)

Revision History:

Notes:

--*/


#ifndef _NDISWDM_H
#define _NDISWDM_H

#include <ndis.h>
#include "nuiouser.h"
//
// To use strsafe function on 9x, ME, and Win2K Oses, we 
// have to define NTSTRSAFE_LIB before including this header file and explicitly
// link to ntstrsafe.lib. If your driver is just target for XP and above, there is
// no need to define NTSTRSAFE_LIB and link to the lib.
// 
#define NTSTRSAFE_LIB
#include <ntstrsafe.h>

#if defined(NDIS50_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION       5
    #define MP_NDIS_MINOR_VERSION       0
#elif defined(NDIS51_MINIPORT)
    #define MP_NDIS_MAJOR_VERSION       5
    #define MP_NDIS_MINOR_VERSION       1
#else
#error Unsupported NDIS version
#endif


#define NIC_TAG                             ((ULONG)'MDWN')

// media type, we use ethernet, change if necessary
#define NIC_MEDIA_TYPE                    NdisMedium802_3

// we use Internal, change to Pci, Isa, etc. properly
#define NIC_INTERFACE_TYPE                NdisInterfaceInternal     

//
// Update the driver version number every time you release a new driver
// The high byte is the major version. The low byte is the minor version. 
// Also make sure that VER_FILEVERSION specified in the .RC file also
// matches with the driver version because NDISTESTER checks for that.
//
#define NIC_VENDOR_DRIVER_VERSION       0x00010000                    

// change to your company name instead of using Microsoft
#define NIC_VENDOR_DESC                 "Microsoft"

// Highest byte is the NIC byte plus three vendor bytes, they are normally  
// obtained from the NIC 
#define NIC_VENDOR_ID                    0x00FFFFFF                    

#define PROTOCOL_INTERFACE_NAME L"\\??\\NdisProt"

typedef UCHAR   NIC_MAC_ADDRESS[6];

#define     ETH_HEADER_SIZE             14
#define     ETH_MAX_DATA_SIZE           1500
#define     ETH_MAX_PACKET_SIZE         ETH_HEADER_SIZE + ETH_MAX_DATA_SIZE
#define     ETH_MIN_PACKET_SIZE         60

#define ETH_IS_LOCALLY_ADMINISTERED(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x02))

#define NIC_MAX_MCAST_LIST              32
#define NIC_MAX_BUSY_SENDS              64
#define NIC_MAX_SEND_PKTS               5
#define NIC_MAX_BUSY_RECVS              64
#define NIC_MAX_LOOKAHEAD               ETH_MAX_DATA_SIZE
#define NIC_BUFFER_SIZE                 ETH_MAX_PACKET_SIZE
#define NIC_LINK_SPEED                  500000    // in 100 bps 
#define NIC_SEND_LOW_WATERMARK         NIC_MAX_BUSY_SENDS/4
#define NIC_ADAPTER_NAME_SIZE          128

//
// Buffer size passed in NdisMQueryAdapterResources                            
// We should only need three adapter resources (IO, interrupt and memory),
// Some devices get extra resources, so have room for 10 resources 
//
#define NIC_RESOURCE_BUF_SIZE           (sizeof(NDIS_RESOURCE_LIST) + \
                                        (10*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)))

#define NIC_SUPPORTED_FILTERS ( \
                NDIS_PACKET_TYPE_DIRECTED   | \
                NDIS_PACKET_TYPE_MULTICAST  | \
                NDIS_PACKET_TYPE_BROADCAST  | \
                NDIS_PACKET_TYPE_PROMISCUOUS | \
                NDIS_PACKET_TYPE_ALL_MULTICAST)

#define fMP_RESET_IN_PROGRESS               0x00000001
#define fMP_DISCONNECTED                     0x00000002 
#define fMP_HALT_IN_PROGRESS                0x00000004
#define fMP_SURPRISE_REMOVED                0x00000008
#define fMP_RECV_LOOKASIDE                  0x00000010
#define fMP_INIT_IN_PROGRESS                    0x00000020
#define fMP_SEND_SIDE_RESOURCE_ALLOCATED      0x00000040
#define fMP_RECV_SIDE_RESOURCE_ALLOCATED      0x00000080
#define fMP_POST_READS                     0x00000100
#define fMP_POST_WRITES                     0x00000200

//
// Message verbosity: lower values indicate higher urgency
//
#define MP_VERY_LOUD    5
#define MP_LOUD    4
#define MP_TRACE   3
#define MP_INFO    2
#define MP_WARNING 1
#define MP_ERROR   0

extern INT MPDebugLevel;    

#if DBG
#define DEBUGP(Level, Fmt) \
{ \
    if (Level <= MPDebugLevel) \
    { \
        DbgPrint("NDISWDM.SYS:"); \
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
                                 (fMP_DISCONNECTED | fMP_RESET_IN_PROGRESS | fMP_HALT_IN_PROGRESS | fMP_INIT_IN_PROGRESS | fMP_SURPRISE_REMOVED)) == 0) 

#define MP_INC_REF(_A)              NdisInterlockedIncrement(&(_A)->RefCount)

#define MP_DEC_REF(_A) {\
                            NdisInterlockedDecrement(&(_A)->RefCount);\
                            ASSERT(_A->RefCount >= 0);\
                            if((_A)->RefCount == 0){\
                                NdisSetEvent(&(_A)->HaltEvent);\
                            }\
                        }

#define MP_GET_REF(_A)              ((_A)->RefCount)

typedef struct _MP_GLOBAL_DATA
{
    LIST_ENTRY      AdapterList;
    NDIS_SPIN_LOCK  Lock;
} MP_GLOBAL_DATA, *PMP_GLOBAL_DATA;

extern MP_GLOBAL_DATA GlobalData;

#include <pshpack1.h>

typedef struct _ETH_HEADER
{
    UCHAR       DstAddr[ETH_LENGTH_OF_ADDRESS];
    UCHAR       SrcAddr[ETH_LENGTH_OF_ADDRESS];
    USHORT      EthType;
} ETH_HEADER, *PETH_HEADER;

#include <poppack.h>

//
// This structure is used to send requests to the WDM
// driver at PASSIVE_LEVEL.
//
typedef struct _WDM_NDIS_REQUEST
{
    NDIS_WORK_ITEM      WorkItem; // must be first in the list
    NDIS_OID            Oid;
    NDIS_REQUEST_TYPE   RequestType;
    PVOID               InformationBuffer;
    ULONG               InformationBufferLength;
    PULONG              BytesReadOrWritten;
    PULONG              BytesNeeded;

} WDM_NDIS_REQUEST, *PWDM_NDIS_REQUEST;

typedef enum {
 
   IRPLOCK_CANCELABLE,
   IRPLOCK_CANCEL_STARTED,
   IRPLOCK_CANCEL_COMPLETE,
   IRPLOCK_COMPLETED
 
} IRPLOCK;
// 
// An IRPLOCK allows for safe cancellation. The idea is to protect the IRP
// while the canceller is calling IoCancelIrp. This is done by wrapping the
// call in InterlockedExchange(s). The roles are as follows:
// 
// Initiator/completion: Cancelable --> IoCallDriver() --> Completed
// Canceller: CancelStarted --> IoCancelIrp() --> CancelCompleted
// 
// No cancellation:
//   Cancelable-->Completed
// 
// Cancellation, IoCancelIrp returns before completion:
//   Cancelable --> CancelStarted --> CancelCompleted --> Completed
// 
// Canceled after completion:
//   Cancelable --> Completed -> CancelStarted
// 
// Cancellation, IRP completed during call to IoCancelIrp():
//   Cancelable --> CancelStarted -> Completed --> CancelCompleted
// 
//  The transition from CancelStarted to Completed tells the completer to block
//  postprocessing (IRP ownership is transfered to the canceller). Similarly,
//  the canceler learns it owns IRP postprocessing (free, completion, etc)
//  during a Completed->CancelCompleted transition.
// 

// TCB (Transmit Control Block)
typedef struct _TCB
{
    LIST_ENTRY              List; // This must be the first entry
    LONG                    Ref;
    PVOID                   Adapter;
    PIRP                    Irp;
    PMDL                    Mdl;
    IRPLOCK                 IrpLock;
    PNDIS_BUFFER            Buffer;
    PNDIS_PACKET            OrgSendPacket;
    PUCHAR                  pData;        
    ULONG                   ulSize;
    UCHAR                   Data[NIC_BUFFER_SIZE];
} TCB, *PTCB;

// RCB (Receive Control Block)
typedef struct _RCB
{
    LIST_ENTRY              List; // This must be the first entry
    LONG                    Ref;
    PVOID                   Adapter;
    PIRP                    Irp;
    IRPLOCK                 IrpLock;
    PNDIS_BUFFER            Buffer;
    PNDIS_PACKET            Packet;
    PUCHAR                  pData;        
    ULONG                   ulSize;
    UCHAR                   Data[NIC_BUFFER_SIZE];
} RCB, *PRCB;

typedef struct _MP_ADAPTER
{
    LIST_ENTRY              List;
    LONG                    RefCount;
    NDIS_SPIN_LOCK          Lock;          
    NDIS_EVENT              InitEvent;    
    NDIS_EVENT              HaltEvent;    
    
    //
    // Keep track of various device objects.
    //
    PDEVICE_OBJECT          Pdo; 
    PDEVICE_OBJECT          Fdo; 
    PDEVICE_OBJECT          NextDeviceObject; 
    NDIS_HANDLE             AdapterHandle;   
    WCHAR                   AdapterName[NIC_ADAPTER_NAME_SIZE];
    WCHAR                   AdapterDesc[NIC_ADAPTER_NAME_SIZE];
    ULONG                   Flags;
    UCHAR                   PermanentAddress[ETH_LENGTH_OF_ADDRESS];
    UCHAR                   CurrentAddress[ETH_LENGTH_OF_ADDRESS];
    WDM_NDIS_REQUEST        WdmNdisRequest;
    BOOLEAN                 Promiscuous;
    BOOLEAN                 RequestPending;
    BOOLEAN                 ResetPending;
    BOOLEAN                 IsHardwareDevice;
    BOOLEAN                 TargetSupportsChainedMdls;
    //
    // Variables to track resources for the send operation
    //
    NDIS_HANDLE             SendBufferPoolHandle;
    LIST_ENTRY              SendFreeList;
    LIST_ENTRY              SendWaitList;
    LIST_ENTRY              SendBusyList;    
    PUCHAR                  TCBMem;
    LONG                    nBusySend;
    NDIS_SPIN_LOCK          SendLock;      

    //
    // Variables to track resources for the Receive operation
    //
    LIST_ENTRY              RecvFreeList;
    LIST_ENTRY              RecvBusyList;
    NDIS_SPIN_LOCK          RecvLock;
    LONG                    nBusyRecv;
    NDIS_HANDLE             RecvPacketPoolHandle;
    NDIS_HANDLE             RecvBufferPoolHandle; 
    PUCHAR                  RCBMem;
    NDIS_WORK_ITEM          ReadWorkItem;
    LONG                    IsReadWorkItemQueued;
    //
    // Packet Filter and look ahead size.
    //
    ULONG                   PacketFilter;
    ULONG                   ulLookAhead;
    ULONG                   ulLinkSpeed;

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

    //
    // Talking to NDISPROT protocol
    //
    HANDLE                  FileHandle;
    PFILE_OBJECT            FileObject;
    PDEVICE_OBJECT          TargetDeviceObject; 
    UCHAR                   PhyNicMacAddress[ETH_LENGTH_OF_ADDRESS];
    PCALLBACK_OBJECT        CallbackObject;
    PVOID                   CallbackRegisterationHandle;    
    PIRP                    StatusIndicationIrp;
    IRPLOCK                 StatusIndicationIrpLock;
    NDISPROT_INDICATE_STATUS NdisProtIndicateStatus;
    
    //
    // Statistic for debugging & validation purposes
    //
    ULONG                   nReadsPosted;
    ULONG                   nReadsCompletedWithError;
    ULONG                   nPacketsIndicated;
    ULONG                   nPacketsReturned;
    ULONG                   nBytesRead;
    ULONG                   nPacketsArrived;
    ULONG                   nWritesPosted;
    ULONG                   nWritesCompletedWithError;    
    ULONG                   nBytesArrived;
    ULONG                   nBytesWritten;
    ULONG                   nReadWorkItemScheduled;
} MP_ADAPTER, *PMP_ADAPTER;

#define NDIS_STATUS_TO_NT_STATUS(_NdisStatus, _pNtStatus)                           \
{                                                                                   \
    /*                                                                              \
     *  The following NDIS status codes map directly to NT status codes.            \
     */                                                                             \
    if (((NDIS_STATUS_SUCCESS == (_NdisStatus)) ||                                  \
        (NDIS_STATUS_PENDING == (_NdisStatus)) ||                                   \
        (NDIS_STATUS_BUFFER_OVERFLOW == (_NdisStatus)) ||                           \
        (NDIS_STATUS_FAILURE == (_NdisStatus)) ||                                   \
        (NDIS_STATUS_RESOURCES == (_NdisStatus)) ||                                 \
        (NDIS_STATUS_NOT_SUPPORTED == (_NdisStatus))))                              \
    {                                                                               \
        *(_pNtStatus) = (NTSTATUS)(_NdisStatus);                                    \
    }                                                                               \
    else if (NDIS_STATUS_BUFFER_TOO_SHORT == (_NdisStatus))                         \
    {                                                                               \
        /*                                                                          \
         *  The above NDIS status codes require a little special casing.            \
         */                                                                         \
        *(_pNtStatus) = STATUS_BUFFER_TOO_SMALL;                                    \
    }                                                                               \
    else if (NDIS_STATUS_INVALID_LENGTH == (_NdisStatus))                           \
    {                                                                               \
        *(_pNtStatus) = STATUS_INVALID_BUFFER_SIZE;                                 \
    }                                                                               \
    else if (NDIS_STATUS_INVALID_DATA == (_NdisStatus))                             \
    {                                                                               \
        *(_pNtStatus) = STATUS_INVALID_PARAMETER;                                   \
    }                                                                               \
    else if (NDIS_STATUS_ADAPTER_NOT_FOUND == (_NdisStatus))                        \
    {                                                                               \
        *(_pNtStatus) = STATUS_NO_MORE_ENTRIES;                                     \
    }                                                                               \
    else if (NDIS_STATUS_ADAPTER_NOT_READY == (_NdisStatus))                        \
    {                                                                               \
        *(_pNtStatus) = STATUS_DEVICE_NOT_READY;                                    \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        *(_pNtStatus) = STATUS_UNSUCCESSFUL;                                        \
    }                                                                               \
}

#define NT_STATUS_TO_NDIS_STATUS(_NtStatus, _pNdisStatus)                           \
{                                                                                   \
    /*                                                                              \
     *  The following NDIS status codes map directly to NT status codes.            \
     */                                                                             \
    if (((STATUS_SUCCESS == (_NtStatus)) ||                                  \
        (STATUS_PENDING == (_NtStatus)) ||                                   \
        (STATUS_BUFFER_OVERFLOW == (_NtStatus)) ||                           \
        (STATUS_UNSUCCESSFUL == (_NtStatus)) ||                                   \
        (STATUS_INSUFFICIENT_RESOURCES == (_NtStatus)) ||                                 \
        (STATUS_NOT_SUPPORTED == (_NtStatus))))                              \
    {                                                                               \
        *(_pNdisStatus) = (NDIS_STATUS)(_NtStatus);                                    \
    }                                                                               \
    else if (STATUS_BUFFER_TOO_SMALL == (_NtStatus))                         \
    {                                                                               \
        /*                                                                          \
         *  The above NDIS status codes require a little special casing.            \
         */                                                                         \
        *(_pNdisStatus) = NDIS_STATUS_BUFFER_TOO_SHORT;                            \
    }                                                                               \
    else if (STATUS_INVALID_BUFFER_SIZE == (_NtStatus))                           \
    {                                                                               \
        *(_pNdisStatus) = NDIS_STATUS_INVALID_LENGTH;                                 \
    }                                                                               \
    else if (STATUS_INVALID_PARAMETER == (_NtStatus))                             \
    {                                                                               \
        *(_pNdisStatus) = NDIS_STATUS_INVALID_DATA;                                   \
    }                                                                               \
    else if (STATUS_NO_MORE_ENTRIES == (_NtStatus))                        \
    {                                                                               \
        *(_pNdisStatus) = NDIS_STATUS_ADAPTER_NOT_FOUND;                                     \
    }                                                                               \
    else if (STATUS_DEVICE_NOT_READY == (_NtStatus))                        \
    {                                                                               \
        *(_pNdisStatus) = NDIS_STATUS_ADAPTER_NOT_READY;                                    \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        *(_pNdisStatus) = NDIS_STATUS_FAILURE;                                        \
    }                                                                               \
}




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


BOOLEAN 
MPCheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext);

VOID 
MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS 
MPReset(
    OUT PBOOLEAN AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext
    );

VOID 
MPUnload(
    IN  PDRIVER_OBJECT  DriverObject
    );

NDIS_STATUS 
MPQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded);


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

                                                        
BOOLEAN 
NICCopyPacket(
    PMP_ADAPTER Adapter,
    PTCB pTCB, 
    PNDIS_PACKET Packet);
    
                 
VOID 
NICFreeRecvPacket(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet);
    
VOID 
NICFreeSendTCB(
    IN PMP_ADAPTER Adapter,
    IN PTCB pTCB);

   
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

VOID 
NICFreeAdapter(
    PMP_ADAPTER Adapter
    );

NDIS_STATUS 
NICAllocSendResources(
    PMP_ADAPTER Adapter);

NDIS_STATUS 
NICAllocRecvResources(
    PMP_ADAPTER Adapter);

VOID 
NICFreeSendResources(
    PMP_ADAPTER Adapter);

VOID 
NICFreeRecvResources(
    PMP_ADAPTER Adapter);
    
VOID 
NICAttachAdapter(
    PMP_ADAPTER Adapter
    );

VOID 
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

NDIS_STATUS 
NICSetMulticastList(
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

PUCHAR 
DbgGetOidName
    (ULONG oid
    );             


VOID 
NICInitWorkItemCallback(
    PNDIS_WORK_ITEM WorkItem, 
    PVOID Void);

NTSTATUS 
NICInitAdapterWorker(
    PMP_ADAPTER Adapter    
    ) ;
    
VOID
NICPostReadsWorkItemCallBack(
    PNDIS_WORK_ITEM WorkItem, 
    PVOID Dummy
    );

NTSTATUS
NICMakeSynchronousIoctl(
    IN PDEVICE_OBJECT       TopOfDeviceStack,
    IN PFILE_OBJECT         FileObject,
    IN ULONG                IoctlControlCode,
    IN OUT PVOID            InputBuffer,
    IN ULONG                InputBufferLength,
    IN OUT PVOID            OutputBuffer,
    IN ULONG                OutputBufferLength,
    OUT PULONG              BytesReadOrWritten    
    );

NTSTATUS
NICMakeSynchronousIoCtlCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
NICPostWriteRequest(
    PMP_ADAPTER Adapter,
    PTCB    pTCB
    );

NTSTATUS
NICWriteRequestCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

VOID
NICFreeBusySendPackets(
    PMP_ADAPTER Adapter
    );

NTSTATUS
NICPostReadRequest(
    PMP_ADAPTER Adapter,
    PRCB    pRCB
    );

NTSTATUS
NICReadRequestCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

VOID 
NICFreeRCB(
    IN PRCB pRCB);

VOID
NICFreeBusyRecvPackets(
    PMP_ADAPTER Adapter
    );

NDIS_STATUS
NICSendRequest(
    IN PMP_ADAPTER                  Adapter,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      BytesReadOrWritten,
    OUT PULONG                      BytesNeeded
    );

NDIS_STATUS
NICForwardRequest(
    IN PMP_ADAPTER                  Adapter,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      BytesReadOrWritten,
    OUT PULONG                      BytesNeeded
    );

VOID 
NICRequestWorkItemCallback(
    IN PNDIS_WORK_ITEM WorkItem, 
    IN PVOID Dummy) ;

VOID
NICIndicateReceivedPacket(
    IN PRCB             pRCB,
    IN ULONG            BytesRead
    );

BOOLEAN
NICIsPacketAcceptable(
    IN PMP_ADAPTER Adapter,
    IN PUCHAR   pDstMac
    );

BOOLEAN
NICIsMulticastMatch(
    IN PMP_ADAPTER                  Adapter,
    IN PUCHAR                       pDstMac
    );


#ifdef INTERFACE_WITH_NDISPROT

VOID 
NICUnregisterExCallback(
    IN PMP_ADAPTER Adapter
);

BOOLEAN 
NICRegisterExCallback(
    PMP_ADAPTER Adapter);


VOID
NICExCallback(
    PVOID   CallBackContext,
    PVOID   Source,
    PVOID   AreYouThere
    );

NTSTATUS
NICOpenNdisProtocolInterface(
    PMP_ADAPTER Adapter
    );


#else

#define NICUnregisterExCallback(Adapter)  
#define NICRegisterExCallback(Adapter) TRUE
#define NICExCallback() 

#endif


NTSTATUS
NICPostAsynchronousStatusIndicationIrp(
    IN PMP_ADAPTER Adapter
);

NTSTATUS
NICStatusIndicationIrpCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

VOID
NICCancelStatusIndicationIrp(
    IN PMP_ADAPTER Adapter
    );

#if defined(NDIS50_MINIPORT)
//
// Following function prototypes are not defined in the Win2K WDM.H.
// Please note IoReuseIrp is not supported on Win9x.
//
NTKERNELAPI
VOID
IoReuseIrp(
    IN OUT PIRP Irp,
    IN NTSTATUS Iostatus
    );

NTSYSAPI
NTSTATUS
NTAPI
ZwOpenFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG ShareAccess,
    IN ULONG OpenOptions
    );


NTKERNELAPI
VOID
ExFreePoolWithTag(
    IN PVOID P,
    IN ULONG Tag
    );
#endif

#endif    // NDISWDM_H


