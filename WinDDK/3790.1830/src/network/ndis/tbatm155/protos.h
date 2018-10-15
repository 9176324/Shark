/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       protos.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920

Abstract:

Author:

   A. Wang

Environment:

   Kernel mode

Revision History:

   01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#ifndef __PROTOS_H
#define __PROTOS_H

NTSTATUS
DriverEntry(
   IN  PDRIVER_OBJECT  DriverObject,
   IN  PUNICODE_STRING RegistryPath
   );

NDIS_STATUS
TbAtm155Initialize(
   OUT PNDIS_STATUS    OpenErrorStatus,
   OUT PUINT           SelectedMediumIndex,
   IN  PNDIS_MEDIUM    MediumArray,
   IN  UINT            MediumArraySize,
   IN  NDIS_HANDLE     MiniportAdapterHandle,
   IN  NDIS_HANDLE     ConfigurationHandle
   );



VOID
TbAtm155EnableInterrupt(
   IN  NDIS_HANDLE     MiniportAdapterContext
   );

VOID
TbAtm155DisableInterrupt(
   IN  NDIS_HANDLE     MiniportAdapterContext
   );

NDIS_STATUS
tbAtm155IsIt4KVc(
   IN  PADAPTER_BLOCK  pAdapter
   );

NDIS_STATUS
tbAtm155InitSRAM_1KVCs(
   IN  PADAPTER_BLOCK  pAdapter
   );

NDIS_STATUS
tbAtm155InitSRAM_4KVCs(
   IN  PADAPTER_BLOCK  pAdapter
   );

VOID
TbAtm155ISR(
   OUT PBOOLEAN    InterruptRecognized,
   OUT PBOOLEAN    QueueDpc,
   IN  PVOID       Context
   );

VOID
TbAtm155HandleInterrupt(
   IN  NDIS_HANDLE MiniportAdapterContext
   );


VOID
TbAtm155AllocateComplete(
   IN  NDIS_HANDLE             MiniportAdapterContext,
   IN  PVOID                   VirtualAddress,
   IN  PNDIS_PHYSICAL_ADDRESS  PhysicalAddress,
   IN  ULONG                   Length,
   IN  PVOID                   Context
   );

VOID
TbAtm155AllocateRecvBufferComplete(
   IN  NDIS_HANDLE             MiniportAdapterContext,
   IN  PVOID                   VirtualAddress,
   IN  PNDIS_PHYSICAL_ADDRESS  PhysicalAddress,
   IN  ULONG                   Length,
   IN  PVOID                   Context
   );

VOID
TbAtm155AllocateXmitBufferComplete(
   IN  NDIS_HANDLE             MiniportAdapterContext,
   IN  PVOID                   VirtualAddress,
   IN  PNDIS_PHYSICAL_ADDRESS  PhysicalAddress,
   IN  ULONG                   Length,
   IN  PTRANSMIT_CONTEXT       pTransmitContext
   );

NDIS_STATUS
tbAtm155DetectPhyType(
	IN PADAPTER_BLOCK	pAdapter
   );


///
//	PROTOTYPES FOR REQUEST CODE
///
NDIS_STATUS
TbAtm155SetInformation(
   IN  NDIS_HANDLE MiniportAdapterContext,
   IN  NDIS_OID    Oid,
   IN  PVOID       InformationBuffer,
   IN  ULONG       InformationBufferLength,
   OUT PULONG      BytesRead,
   OUT PULONG      BytesNeeded
   );

NDIS_STATUS
TbAtm155QueryInformation(
   IN  NDIS_HANDLE MiniportAdapterContext,
   IN  NDIS_OID    Oid,
   IN  PVOID       InformationBuffer,
   IN  ULONG       InformationBufferLength,
   OUT PULONG      BytesRead,
   OUT PULONG      BytesNeeded
   );

NDIS_STATUS
TbAtm155Request(
   IN      NDIS_HANDLE     MiniportAdapterContext,
   IN      NDIS_HANDLE     MiniportVcContext	OPTIONAL,
   IN  OUT	PNDIS_REQUEST   NdisCoRequest
   );

///
//	PROTOTYPES FOR RESET CODE
///

NDIS_STATUS
tbAtm155OpenVcChannels(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
	);

VOID
tbAtm155SoftresetNIC(
   IN  PHARDWARE_INFO  pHwInfo
   );

VOID
tbAtm155InitSarParameters(
   IN  PADAPTER_BLOCK  pAdapter
	);

NDIS_STATUS
tbAtm155InitRegisters(
   IN  PADAPTER_BLOCK  pAdapter
   );


UCHAR
tbAtm155CheckConnectionStatus(
   IN  PADAPTER_BLOCK  pAdapter
   );

BOOLEAN
TbAtm155CheckForHang(
   IN  NDIS_HANDLE MiniportAdapterContext
	);

NDIS_STATUS
TbAtm155Reset(
   OUT PBOOLEAN    AddressingReset,
   IN  NDIS_HANDLE MiniportAdapterContext
   );


VOID
tbAtm155ProcessReset(
   IN  PADAPTER_BLOCK  pAdapter
   );

VOID
TbAtm155Shutdown(
   IN  PVOID   ShutdownContext
   );

///
//	PROTOTYPES FOR HALTING THE ADAPTER AND CLEANUP
///

VOID
tbAtm155FreeResources(
   IN  PADAPTER_BLOCK  pAdapter
   );

VOID
TbAtm155Halt(
   IN  NDIS_HANDLE MiniportAdapterContext
   );

///
//	PROTOTYPES FOR SEND PATH
///
VOID
TbAtm155SendPackets(
   IN  NDIS_HANDLE     MiniportVcContext,
   IN  PPNDIS_PACKET   PacketArray,
   IN  UINT            NumberOfPackets
   );

NDIS_STATUS
TbAtm155ProcessSegmentQueue(
   IN  PXMIT_SEG_INFO      pXmitSegInfo,
   IN  PNDIS_PACKET        Packet
   );

VOID
TbAtm155TransmitDmaComplete(
   IN  PADAPTER_BLOCK  pAdapter
   );

VOID
tbAtm155TxInterruptOnCompletion(
   IN  PADAPTER_BLOCK  pAdapter
   );



NDIS_STATUS
tbAtm155AllocateTransmitBuffers(
   IN  PVC_BLOCK		pVc
   );

VOID
tbAtm155FreeTransmitBuffers(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PVC_BLOCK           pVc
   );


#if TB_DBG

VOID
tbAtm155TransmitPacket(
   IN	PVC_BLOCK       pVc,
   IN  PNDIS_PACKET    Packet
   );

#endif // end of TB_DBG
    

///
//	PROTOTYPES FOR VC Creation and Deletion
///
NDIS_STATUS
tbAtm155AdjustTrafficParameters(
   IN  PADAPTER_BLOCK          pAdapter,
   IN  PATM_FLOW_PARAMETERS    pFlow,
   IN  BOOLEAN                 fRoundFlowUp,
   OUT PULONG                  pPreScaleVal,
   OUT PULONG                  pNumOfEntries
   );


NDIS_STATUS
TbAtm155CreateVc(
   IN  NDIS_HANDLE     MiniportAdapterContext,
   IN  NDIS_HANDLE     NdisVcHandle,
   OUT PNDIS_HANDLE    MiniportVcContext
   );

NDIS_STATUS
TbAtm155DeleteVc(
   IN  NDIS_HANDLE     MiniportVcContext
	);

NDIS_STATUS
TbAtm155ActivateVc(
   IN  NDIS_HANDLE         MiniportVcContext,
   IN  PCO_CALL_PARAMETERS MediaParameters
   );

VOID
TbAtm155ActivateVcComplete(
   IN  PVC_BLOCK       pVc,
   IN  NDIS_STATUS     Status
   );


NDIS_STATUS
TbAtm155DeactivateVc(
   IN  NDIS_HANDLE     MiniportVcContext
   );

VOID
TbAtm155DeactivateVcComplete(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
   );


///
//	PROTOTYPES FOR SUPPORT
///

#if DBG

VOID
InsertPacketAtTailDpc(
   IN  PPACKET_QUEUE   PacketQ,
   IN  PNDIS_PACKET    Packet
   );

VOID
InsertPacketAtTail(
   IN  PPACKET_QUEUE   PacketQueue,
   IN  PNDIS_PACKET    Packet
   );

VOID
RemovePacketFromHeadDpc(
   IN  PPACKET_QUEUE   PacketQ,
   OUT PNDIS_PACKET    *Packet
   );

VOID
RemovePacketFromHead(
   IN  PPACKET_QUEUE   PacketQ,
   OUT PNDIS_PACKET    *Packet
   );


VOID
InitializeMapRegisterQueue(
	IN	PMAP_REGISTER_QUEUE     MapRegistersQ
	);

VOID
InsertMapRegisterAtTail(
	IN	PMAP_REGISTER_QUEUE     MapRegistersQ,
	IN	PMAP_REGISTER	        MapRegister
	);


VOID
RemoveMapRegisterFromHead(
   IN  PMAP_REGISTER_QUEUE     MapRegistersQ,
   OUT PMAP_REGISTER           *MapRegister
	);


#endif // DBG



VOID
tbAtm155InsertRecvBufferAtTail(
   IN  PRECV_BUFFER_QUEUE      pBufferQ,
   IN  PRECV_BUFFER_HEADER     pRecvHeader,
   IN  BOOLEAN                 CheckIfInitBuffer
   );


VOID
tbAtm155RemoveReceiveBufferFromHead(
   IN  PRECV_BUFFER_QUEUE      pBufferQ,
   OUT PRECV_BUFFER_HEADER     *pRecvHeader
   );


VOID
tbAtm155SearchRecvBufferFromQueue(
   IN  PRECV_BUFFER_QUEUE      pBufferQ,
   IN  ULONG                   PostedTag,
   OUT PRECV_BUFFER_HEADER     *pRecvHeader
);


VOID
tbAtm155MergeRecvBuffers2FreeBufferQueue(
   IN PADAPTER_BLOCK      pAdapter,
   IN PRECV_BUFFER_QUEUE  pBufferQ,
   IN PRECV_BUFFER_QUEUE  pReturnBufferQ
	);


VOID
tbAtm155FreeRecvHeader(
   IN  OUT PRECV_BUFFER_HEADER     *ppRecvHeaderHead,
   IN  OUT PRECV_BUFFER_HEADER     *ppNextRecvHeader,
   IN      PADAPTER_BLOCK          pAdapter,
   IN      PRECV_BUFFER_HEADER     pRecvHeader
);


VOID
tbAtm155HashVc(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
   );

PVC_BLOCK
tbAtm155UnHashVc(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  ULONG           Vc
   );

VOID
tbAtm155RemoveHash(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
   );

VOID
tbAtm155InitializePacketQueue(
   IN OUT  PPACKET_QUEUE   PacketQ
	);

VOID
tbAtm155FreePacketQueue(
   IN  PPACKET_QUEUE   PacketQ
   );


VOID
tbAtm155InitializeReceiveBufferQueue(
   IN OUT  PRECV_BUFFER_QUEUE  RecvBufferQ
   );

VOID
tbAtm155FreeReceiveBufferQueue(
   IN PADAPTER_BLOCK           pAdapter,
   IN OUT  PRECV_BUFFER_QUEUE  RecvBufferQ
   );


void
tbAtm155ConvertToFP(
   IN OUT  PULONG  pRate,
   OUT     PUSHORT pRateInFP
   );


///
//	PROTOTYPES FOR RECEIVE processing.
///

NDIS_STATUS
tbAtm155AllocateReceiveBufferInfo(
   OUT PRECV_BUFFER_INFO   *pRecvBufferInfo,
   IN  PVC_BLOCK           pVc
   );

NDIS_STATUS
tbAtm155FreeReceiveBufferInfo(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PRECV_BUFFER_INFO   pRecvInfo
   );

NDIS_STATUS
tbAtm155AllocateReceiveBufferPool(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PRECV_BUFFER_INFO   pRecvBufferInfo,
   IN  ULONG               NumberOfReceiveBuffers,
   IN  ULONG               SizeOfReceiveBuffer
   );

VOID
tbAtm155FreeReceiveBufferPool(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PRECV_BUFFER_POOL   pRecvPool
   );

BOOLEAN
tbAtm155RxInterruptOnCompletion(
   IN  PADAPTER_BLOCK  pAdapter
   );

VOID
tbAtm155QueueRecvBuffersToReceiveSlots(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  UCHAR           BufferType
   );

VOID
tbAtm155ProcessReturnPacket(
    IN  PADAPTER_BLOCK      pAdapter,
    IN  PNDIS_PACKET        Packet,
    IN  PRECV_BUFFER_HEADER pRecvHeader,
    IN  BOOLEAN             fAdjustPktsHoldsByNdis
   );

VOID
TbAtm155ReturnPacket(
   IN  NDIS_HANDLE     MiniportAdapterContext,
   IN  PNDIS_PACKET    Packet
   );

VOID
tbAtm155FreeReceiveBuffer(
   IN OUT PRECV_BUFFER_HEADER  *ppRecvHeaderHead,
   IN  PADAPTER_BLOCK          pAdapter,
   IN  PRECV_BUFFER_HEADER     pRecvHeader
   );


//////////////////////////////////////////////////////////////////////////
//																		
//     Declare Routines in suni_lit.c
//
//////////////////////////////////////////////////////////////////////////

NDIS_STATUS
tbAtm155InitSuniLitePhyRegisters(
   IN  PADAPTER_BLOCK  pAdapter
   );

VOID
tbAtm155SuniInterrupt(
	IN	PADAPTER_BLOCK	pAdapter
	);


NDIS_STATUS
tbAtm155DetectPhyType(
	IN PADAPTER_BLOCK	pAdapter
   );


BOOLEAN
tbAtm155CheckSuniLiteLinkUp(
	IN PADAPTER_BLOCK	pAdapter
   );


//////////////////////////////////////////////////////////////////////////
//																		
//     Declare Routines in plc2.c
//
//////////////////////////////////////////////////////////////////////////

NDIS_STATUS
tbAtm155InitPLC2PhyRegisters(
   IN  PADAPTER_BLOCK  pAdapter
   );

VOID
tbAtm155PLC2Interrupt(
	IN	PADAPTER_BLOCK	pAdapter
	);


BOOLEAN
tbAtm155CheckPLC2LinkUp(
	IN PADAPTER_BLOCK	pAdapter
   );


#endif // __PROTOS_H

