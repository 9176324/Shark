/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

        Request.c
        
Abstract:

    This module contains Miniport function and helper routines for handling 
    Set & Query Information requests.
    
Revision History:

Notes:

--*/


#include "ndiswdm.h"

NDIS_OID NICSupportedOids[] =
{
        OID_GEN_SUPPORTED_LIST,
        OID_GEN_HARDWARE_STATUS,
        OID_GEN_MEDIA_SUPPORTED,
        OID_GEN_MEDIA_IN_USE,
        OID_GEN_MAXIMUM_LOOKAHEAD,
        OID_GEN_MAXIMUM_FRAME_SIZE,
        OID_GEN_LINK_SPEED,
        OID_GEN_TRANSMIT_BUFFER_SPACE,
        OID_GEN_RECEIVE_BUFFER_SPACE,
        OID_GEN_TRANSMIT_BLOCK_SIZE,
        OID_GEN_RECEIVE_BLOCK_SIZE,
        OID_GEN_VENDOR_ID,
        OID_GEN_VENDOR_DESCRIPTION,
        OID_GEN_VENDOR_DRIVER_VERSION,
        OID_GEN_CURRENT_PACKET_FILTER,
        OID_GEN_CURRENT_LOOKAHEAD,
        OID_GEN_DRIVER_VERSION,
        OID_GEN_MAXIMUM_TOTAL_SIZE,
        OID_GEN_PROTOCOL_OPTIONS,
        OID_GEN_MAC_OPTIONS,
        OID_GEN_MEDIA_CONNECT_STATUS,
        OID_GEN_MAXIMUM_SEND_PACKETS,
        OID_GEN_XMIT_OK,
        OID_GEN_RCV_OK,
        OID_GEN_XMIT_ERROR,
        OID_GEN_RCV_ERROR,
        OID_GEN_RCV_NO_BUFFER,
        OID_GEN_RCV_CRC_ERROR,
        OID_GEN_TRANSMIT_QUEUE_LENGTH,
        OID_802_3_PERMANENT_ADDRESS,
        OID_802_3_CURRENT_ADDRESS,
        OID_802_3_MULTICAST_LIST,
        OID_802_3_MAC_OPTIONS,
        OID_802_3_MAXIMUM_LIST_SIZE,
        OID_802_3_RCV_ERROR_ALIGNMENT,
        OID_802_3_XMIT_ONE_COLLISION,
        OID_802_3_XMIT_MORE_COLLISIONS,
        OID_802_3_XMIT_DEFERRED,
        OID_802_3_XMIT_MAX_COLLISIONS,
        OID_802_3_RCV_OVERRUN,
        OID_802_3_XMIT_UNDERRUN,
        OID_802_3_XMIT_HEARTBEAT_FAILURE,
        OID_802_3_XMIT_TIMES_CRS_LOST,
        OID_802_3_XMIT_LATE_COLLISIONS
#ifndef INTERFACE_WITH_NDISPROT
        ,
        OID_PNP_CAPABILITIES,
        OID_PNP_SET_POWER,
        OID_PNP_QUERY_POWER,
        OID_PNP_ADD_WAKE_UP_PATTERN,
        OID_PNP_REMOVE_WAKE_UP_PATTERN,
        OID_PNP_ENABLE_WAKE_UP
#endif        
};

#pragma NDIS_PAGEABLE_FUNCTION(NICSendRequest)


NDIS_STATUS 
MPQueryInformation(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN NDIS_OID     Oid,
    IN PVOID        InformationBuffer,
    IN ULONG        InformationBufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded)
/*++

Routine Description:

    Entry point called by NDIS to query for the value of the specified OID.
    MiniportQueryInformation runs at IRQL = DISPATCH_LEVEL. 
    
Arguments:

    MiniportAdapterContext      Pointer to the adapter structure
    Oid                         Oid for this query
    InformationBuffer           Buffer for information
    InformationBufferLength     Size of this buffer
    BytesWritten                Specifies how much info is written
    BytesNeeded                 In case the buffer is smaller than 
                                what we need, tell them how much is needed


Return Value:

    Return code from the NdisRequest below.
    
Notes: Read "Minimizing Miniport Driver Initialization Time" in the DDK
    for more info on how to handle certain OIDs that affect the init of
    a miniport.

--*/
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER             Adapter;
    NDIS_HARDWARE_STATUS    HardwareStatus = NdisHardwareStatusReady;
    NDIS_MEDIUM             Medium = NIC_MEDIA_TYPE;
    UCHAR                   VendorDesc[] = NIC_VENDOR_DESC;
    ULONG                   ulInfo = 0;
    USHORT                  usInfo = 0;                                                              
    ULONG64                 ulInfo64 = 0;
    PVOID                   pInfo = (PVOID) &ulInfo;
    ULONG                   ulInfoLen = sizeof(ulInfo);   
    BOOLEAN                 bForwardRequest = TRUE;
    NDIS_PNP_CAPABILITIES   PMCaps;
    
    DEBUGP(MP_VERY_LOUD, ("---> MPQueryInformation %s\n", DbgGetOidName(Oid)));

    Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    // Initialize the result
    *BytesWritten = 0;
    *BytesNeeded = 0;

    switch(Oid)
    {
        case OID_GEN_SUPPORTED_LIST:
            //
            // The OID_GEN_SUPPORTED_LIST OID specifies an array of OIDs
            // for objects that the underlying driver or its NIC supports.
            // Objects include general, media-specific, and implementation-
            // specific objects. NDIS forwards a subset of the returned 
            // list to protocols that make this query. That is, NDIS filters
            // any supported statistics OIDs out of the list because 
            // protocols never make statistics queries. 
            //
            pInfo = (PVOID) NICSupportedOids;
            ulInfoLen = sizeof(NICSupportedOids);
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_HARDWARE_STATUS:
            //
            // Specify the current hardware status of the underlying NIC as
            // one of the following NDIS_HARDWARE_STATUS-type values.
            //
            pInfo = (PVOID) &HardwareStatus;
            ulInfoLen = sizeof(NDIS_HARDWARE_STATUS);
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_MEDIA_SUPPORTED:
            // 
            // Specify the media types that the NIC can support but not
            // necessarily the media types that the NIC currently uses.
            // fallthrough:
        case OID_GEN_MEDIA_IN_USE:
            //
            // Specifiy a complete list of the media types that the NIC
            // currently uses. 
            //
            pInfo = (PVOID) &Medium;
            ulInfoLen = sizeof(NDIS_MEDIUM);
            bForwardRequest = FALSE;            
            break;
            
        case OID_GEN_CURRENT_LOOKAHEAD:
        case OID_GEN_MAXIMUM_LOOKAHEAD:
            //
            // If the miniport driver indicates received data by calling
            // NdisXxxIndicateReceive, it should respond to OID_GEN_MAXIMUM_LOOKAHEAD
            // with the maximum number of bytes the NIC can provide as 
            // lookahead data. If that value is different from the size of the 
            // lookahead buffer supported by bound protocols, NDIS will call 
            // MiniportSetInformation to set the size of the lookahead buffer 
            // provided by the miniport driver to the minimum of the miniport 
            // driver and protocol(s) values. If the driver always indicates
            // up full packets with NdisMIndicateReceivePacket, it should 
            // set this value to the maximum total packet size, which 
            // excludes the header.
            // Upper-layer drivers examine lookahead data to determine whether
            // a packet that is associated with the lookahead data is intended
            // for one or more of their clients. If the underlying driver 
            // supports multipacket receive indications, bound protocols are 
            // given full net packets on every indication. Consequently, 
            // this value is identical to that returned for 
            // OID_GEN_RECEIVE_BLOCK_SIZE. 
            //
            if(Adapter->ulLookAhead == 0)
            {
                Adapter->ulLookAhead = NIC_MAX_LOOKAHEAD;
            }
            ulInfo = Adapter->ulLookAhead;
            bForwardRequest = FALSE;            
            break;            
            
        case OID_GEN_MAXIMUM_FRAME_SIZE:
            //
            // Specifiy the maximum network packet size, in bytes, that the 
            // NIC supports excluding the header. A NIC driver that emulates
            // another medium type for binding to a transport must ensure that
            // the maximum frame size for a protocol-supplied net packet does
            // not exceed the size limitations for the true network medium. 
            //
            ulInfo = ETH_MAX_PACKET_SIZE - ETH_HEADER_SIZE;
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_MAXIMUM_TOTAL_SIZE:
            //
            // Specify the maximum total packet length, in bytes, the NIC  
            // supports including the header. A protocol driver might use 
            // this returned length as a gauge to determine the maximum 
            // size packet that a NIC driver could forward to the 
            // protocol driver. The miniport driver must never indicate
            // up to the bound protocol driver packets received over the 
            // network that are longer than the packet size specified by 
            // OID_GEN_MAXIMUM_TOTAL_SIZE.
            //
        case OID_GEN_TRANSMIT_BLOCK_SIZE:
            //
            // The OID_GEN_TRANSMIT_BLOCK_SIZE OID specifies the minimum
            // number of bytes that a single net packet occupies in the 
            // transmit buffer space of the NIC. For example, a NIC that 
            // has a transmit space divided into 256-byte pieces would have 
            // a transmit block size of 256 bytes. To calculate the total 
            // transmit buffer space on such a NIC, its driver multiplies 
            // the number of transmit buffers on the NIC by its transmit 
            // block size. In our case, the transmit block size is 
            // identical to its maximum packet size. 
            
        case OID_GEN_RECEIVE_BLOCK_SIZE:
            //
            // The OID_GEN_RECEIVE_BLOCK_SIZE OID specifies the amount of 
            // storage, in bytes, that a single packet occupies in the receive 
            // buffer space of the NIC.
            //
            ulInfo = (ULONG) ETH_MAX_PACKET_SIZE;
            bForwardRequest = FALSE;            
            break;
            
        case OID_GEN_MAC_OPTIONS:
            //
            // Specify a bitmask that defines optional properties of the NIC.             
            // This miniport indicates receive with NdisMIndicateReceivePacket 
            // function. It has no MiniportTransferData function. Such a driver 
            // should set this NDIS_MAC_OPTION_TRANSFERS_NOT_PEND flag. 
            //
            // NDIS_MAC_OPTION_NO_LOOPBACK tells NDIS that NIC has no internal
            // loopback support so NDIS will manage loopbacks on behalf of 
            // this driver. 
            //
            // NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA tells the protocol that 
            // our receive buffer is not on a device-specific card. If 
            // NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA is not set, multi-buffer
            // indications are copied to a single flat buffer.
            //
            ulInfo = NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA | 
                NDIS_MAC_OPTION_TRANSFERS_NOT_PEND |
                NDIS_MAC_OPTION_NO_LOOPBACK;
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_LINK_SPEED:
            //
            // Specify the maximum speed of the NIC in kbps.
            //
            ulInfo = Adapter->ulLinkSpeed;
            break;

        case OID_GEN_TRANSMIT_BUFFER_SPACE:
            //
            // Specify the amount of memory, in bytes, on the NIC that 
            // is available for buffering transmit data. A protocol can 
            // use this OID as a guide for sizing the amount of transmit 
            // data per send.            
            //
            ulInfo = ETH_MAX_PACKET_SIZE * NIC_MAX_BUSY_SENDS;
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_RECEIVE_BUFFER_SPACE:
            //
            // Specify the amount of memory on the NIC that is available 
            // for buffering receive data. A protocol driver can use this 
            // OID as a guide for advertising its receive window after it 
            // establishes sessions with remote nodes.            
            //
            ulInfo = ETH_MAX_PACKET_SIZE * NIC_MAX_BUSY_RECVS;
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_VENDOR_ID:
            //
            // Specify a three-byte IEEE-registered vendor code, followed 
            // by a single byte that the vendor assigns to identify a 
            // particular NIC. The IEEE code uniquely identifies the vendor
            // and is the same as the three bytes appearing at the beginning
            // of the NIC hardware address. Vendors without an IEEE-registered
            // code should use the value 0xFFFFFF.
            //
            ulInfo = NIC_VENDOR_ID;
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_VENDOR_DESCRIPTION:
            //
            // Specify a zero-terminated string describing the NIC vendor.
            //
            pInfo = VendorDesc;
            ulInfoLen = sizeof(VendorDesc);
            bForwardRequest = FALSE;            
            break;
            
        case OID_GEN_VENDOR_DRIVER_VERSION:
            //
            // Specify the vendor-assigned version number of the NIC driver. 
            // The low-order half of the return value specifies the minor 
            // version; the high-order half specifies the major version.
            //
            ulInfo = NIC_VENDOR_DRIVER_VERSION;
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_DRIVER_VERSION:
            //
            // Specify the NDIS version in use by the NIC driver. The high 
            // byte is the major version number; the low byte is the minor 
            // version number.
            //
            usInfo = (USHORT) (MP_NDIS_MAJOR_VERSION<<8) + MP_NDIS_MINOR_VERSION;
            pInfo = (PVOID) &usInfo;
            ulInfoLen = sizeof(USHORT);
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_MAXIMUM_SEND_PACKETS:
            //
            // If a miniport driver registers a MiniportSendPackets function,
            // MiniportQueryInformation will be called with the 
            // OID_GEN_MAXIMUM_SEND_PACKETS request. The miniport driver must
            // respond with the maximum number of packets it is prepared to 
            // handle on a single send request. The miniport driver should 
            // pick a maximum that minimizes the number of packets that it 
            // has to queue internally because it has no resources 
            // (its device is full). A miniport driver for a bus-master DMA 
            // NIC should attempt to pick a value that keeps its NIC filled
            // under anticipated loads.
            //
            ulInfo = NIC_MAX_SEND_PKTS;
            bForwardRequest = FALSE;            
            break;

        case OID_GEN_MEDIA_CONNECT_STATUS:
            //
            // Return the connection status of the NIC on the network as one 
            // of the following system-defined values: NdisMediaStateConnected
            // or NdisMediaStateDisconnected.
            //
            ulInfo = NICGetMediaConnectStatus(Adapter);
            bForwardRequest = FALSE;
            break;
            
        case OID_GEN_CURRENT_PACKET_FILTER:
            //
            // Specifiy the types of net packets such as directed, broadcast 
            // multicast, for which a protocol receives indications from a 
            // NIC driver. After NIC is initialized, a protocol driver 
            // can send a set OID_GEN_CURRENT_PACKET_FILTER to a non-zero value, 
            // thereby enabling the miniport driver to indicate receive packets
            // to that protocol.
            //
            ulInfo = Adapter->PacketFilter;
            bForwardRequest = FALSE;            
            break;
                       
        case OID_PNP_CAPABILITIES:
            //
            // Return the wake-up capabilities of its NIC. If you return 
            // NDIS_STATUS_NOT_SUPPORTED, NDIS considers the miniport driver 
            // to be not Power management aware and doesn't send any power
            // or wake-up related queries such as 
            // OID_PNP_SET_POWER, OID_PNP_QUERY_POWER,
            // OID_PNP_ADD_WAKE_UP_PATTERN, OID_PNP_REMOVE_WAKE_UP_PATTERN,
            // OID_PNP_ENABLE_WAKE_UP. Here, we are expecting the driver below
            // us to do the right thing.
            //
            RtlZeroMemory (&PMCaps, sizeof(NDIS_PNP_CAPABILITIES));
            ulInfoLen = sizeof (NDIS_PNP_CAPABILITIES);
            pInfo = (PVOID) &PMCaps;   
            Status = NDIS_STATUS_SUCCESS;
            break;

        case OID_PNP_QUERY_POWER:
            Status = NDIS_STATUS_SUCCESS; 
            break;            
            
            //
            // Following 4 OIDs are for querying Ethernet Operational 
            // Characteristics.
            //
        case OID_802_3_PERMANENT_ADDRESS:
            //
            // Return the MAC address of the NIC burnt in the hardware.
            //
            pInfo = Adapter->PermanentAddress;
            ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_CURRENT_ADDRESS:
            //
            // Return the MAC address the NIC is currently programmed to
            // use. Note that this address could be different from the
            // permananent address as the user can override using 
            // registry. Read NdisReadNetworkAddress doc for more info.
            //
            pInfo = Adapter->CurrentAddress;
            ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_MAXIMUM_LIST_SIZE:
            //
            // The maximum number of multicast addresses the NIC driver
            // can manage. This list is global for all protocols bound 
            // to (or above) the NIC. Consequently, a protocol can receive
            // NDIS_STATUS_MULTICAST_FULL from the NIC driver when 
            // attempting to set the multicast address list, even if 
            // the number of elements in the given list is less than 
            // the number originally returned for this query.
            //
            ulInfo = NIC_MAX_MCAST_LIST;
            bForwardRequest = TRUE;
            break;
            
        case OID_802_3_MAC_OPTIONS:
            //
            // A protocol can use this OID to determine features supported
            // by the underlying driver such as NDIS_802_3_MAC_OPTION_PRIORITY. 
            // Return zero indicating that it supports no options.
            //
            ulInfo = 0;
            break;
            
            //
            // Following list  consists of both general and Ethernet 
            // specific statistical OIDs.
            //
            
        case OID_GEN_XMIT_OK:
            ulInfo64 = Adapter->GoodTransmits;
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);
            }
            else
            {
                ulInfoLen = sizeof(ULONG);
            }
            // We should always report that 8 bytes are required to keep ndistest happy
            *BytesNeeded =  sizeof(ULONG64);                        
            break;
    
        case OID_GEN_RCV_OK:
            ulInfo64 = Adapter->GoodReceives;
            pInfo = &ulInfo64;
            if (InformationBufferLength >= sizeof(ULONG64) ||
                InformationBufferLength == 0)
            {
                ulInfoLen = sizeof(ULONG64);
            }
            else
            {
                ulInfoLen = sizeof(ULONG);
            }
            // We should always report that 8 bytes are required to keep ndistest happy
            *BytesNeeded =  sizeof(ULONG64);                        
            break;
    
        case OID_GEN_XMIT_ERROR:
            ulInfo = Adapter->TxAbortExcessCollisions +
                Adapter->TxDmaUnderrun +
                Adapter->TxLostCRS +
                Adapter->TxLateCollisions+
                Adapter->TransmitFailuresOther;
            break;
    
        case OID_GEN_RCV_ERROR:
            ulInfo = Adapter->RcvCrcErrors +
                Adapter->RcvAlignmentErrors +
                Adapter->RcvResourceErrors +
                Adapter->RcvDmaOverrunErrors +
                Adapter->RcvRuntErrors;
            break;
    
        case OID_GEN_RCV_NO_BUFFER:
            ulInfo = Adapter->RcvResourceErrors;
            break;
    
        case OID_GEN_RCV_CRC_ERROR:
            ulInfo = Adapter->RcvCrcErrors;
            break;
    
        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
            ulInfo = NIC_MAX_BUSY_SENDS;
            break;
    
        case OID_802_3_RCV_ERROR_ALIGNMENT:
            ulInfo = Adapter->RcvAlignmentErrors;
            break;
    
        case OID_802_3_XMIT_ONE_COLLISION:
            ulInfo = Adapter->OneRetry;
            break;
    
        case OID_802_3_XMIT_MORE_COLLISIONS:
            ulInfo = Adapter->MoreThanOneRetry;
            break;
    
        case OID_802_3_XMIT_DEFERRED:
            ulInfo = Adapter->TxOKButDeferred;
            break;
    
        case OID_802_3_XMIT_MAX_COLLISIONS:
            ulInfo = Adapter->TxAbortExcessCollisions;
            break;
    
        case OID_802_3_RCV_OVERRUN:
            ulInfo = Adapter->RcvDmaOverrunErrors;
            break;
    
        case OID_802_3_XMIT_UNDERRUN:
            ulInfo = Adapter->TxDmaUnderrun;
            break;
    
        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
            ulInfo = Adapter->TxLostCRS;
            break;
    
        case OID_802_3_XMIT_TIMES_CRS_LOST:
            ulInfo = Adapter->TxLostCRS;
            break;
    
        case OID_802_3_XMIT_LATE_COLLISIONS:
            ulInfo = Adapter->TxLateCollisions;
            break;
          
        default:
            Status = NDIS_STATUS_NOT_SUPPORTED;
            break;            
    }

    if(Status == NDIS_STATUS_SUCCESS)
    {
        if(ulInfoLen <= InformationBufferLength)
        {
            // Copy result into InformationBuffer
            *BytesWritten = ulInfoLen;
            if(ulInfoLen)
            {
                NdisMoveMemory(InformationBuffer, pInfo, ulInfoLen);
            }
        }
        else
        {
            // too short
            *BytesNeeded = ulInfoLen;
            Status = NDIS_STATUS_BUFFER_TOO_SHORT;
        }
    }

    if(Status == NDIS_STATUS_SUCCESS && bForwardRequest && 
            MP_IS_READY(Adapter) && InformationBufferLength){
            
        Status = NICForwardRequest(Adapter,
                            NdisRequestQueryInformation,
                            Oid,
                            InformationBuffer,
                            ulInfoLen,
                            BytesWritten,
                            BytesNeeded
                            );
        ASSERT(Status == NDIS_STATUS_SUCCESS);
        Status = NDIS_STATUS_PENDING;
    }
    
    DEBUGP(MP_VERY_LOUD, ("<--- MPQueryInformation Status = 0x%08x\n", 
                                                Status));
    
    return(Status);
}     

NDIS_STATUS 
MPSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded)
/*++

Routine Description:

    This is the handler for an OID set operation. 
    MiniportSetInformation runs at IRQL = DISPATCH_LEVEL.

Arguments:

    MiniportAdapterContext      Pointer to the adapter structure
    Oid                         Oid for this query
    InformationBuffer           Buffer for information
    InformationBufferLength     Size of this buffer
    BytesRead                   Specifies how much info is read
    BytesNeeded                 In case the buffer is smaller than what 
                                we need, tell them how much is needed

Return Value:

    Return code from the NdisRequest below.

--*/
{
    NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER             Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    PNDIS_PM_PACKET_PATTERN pPmPattern = NULL;
    BOOLEAN                 bForwardRequest = FALSE;

    DEBUGP(MP_VERY_LOUD, ("---> MPSetInformation %s\n", DbgGetOidName(Oid)));
    
    *BytesRead = 0;
    *BytesNeeded = 0;

    switch(Oid)
    {
        case OID_802_3_MULTICAST_LIST:
            //
            // Set the multicast address list on the NIC for packet reception.
            // The NIC driver can set a limit on the number of multicast 
            // addresses bound protocol drivers can enable simultaneously. 
            // NDIS returns NDIS_STATUS_MULTICAST_FULL if a protocol driver 
            // exceeds this limit or if it specifies an invalid multicast 
            // address.
            //
            Status = NICSetMulticastList(
                            Adapter,
                            InformationBuffer,
                            InformationBufferLength,
                            BytesRead,
                            BytesNeeded);
            if(!Adapter->Promiscuous) {
                bForwardRequest = TRUE;
            }
            break;

        case OID_GEN_CURRENT_PACKET_FILTER:
            // 
            // Program the hardware to indicate the packets
            // of certain filter types.
            //
            if(InformationBufferLength != sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }

            *BytesRead = InformationBufferLength;
                             
            Status = NICSetPacketFilter(
                            Adapter,
                            *((PULONG)InformationBuffer));
            
            if(!Adapter->Promiscuous) {
                bForwardRequest = TRUE;
            }
            break;

        case OID_GEN_CURRENT_LOOKAHEAD:
            //
            // A protocol driver can set a suggested value for the number
            // of bytes to be used in its binding; however, the underlying
            // NIC driver is never required to limit its indications to 
            // the value set.
            //            
            if(InformationBufferLength != sizeof(ULONG)){
                *BytesNeeded = sizeof(ULONG);
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }                
                
            Adapter->ulLookAhead = *(PULONG)InformationBuffer;                

            *BytesRead = sizeof(ULONG);
            Status = NDIS_STATUS_SUCCESS;
            bForwardRequest = FALSE;            
            
            break;

        case OID_PNP_SET_POWER:
            //
            // This OID notifies a miniport driver that its NIC will be 
            // transitioning to the device power state specified in the 
            // InformationBuffer. The miniport driver must always return 
            // NDIS_STATUS_SUCCESS to an OID_PNP_SET_POWER request. An 
            // OID_PNP_SET_POWER request may or may not be preceded by an 
            // OID_PNP_QUERY_POWER request.
            //
            if (InformationBufferLength != sizeof(NDIS_DEVICE_POWER_STATE ))
            {
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }                   
            *BytesRead = sizeof(NDIS_DEVICE_POWER_STATE);
            Status = NDIS_STATUS_SUCCESS; 
            bForwardRequest = TRUE;            
            break;

        case OID_PNP_ADD_WAKE_UP_PATTERN:
            //
            // This OID is sent by a protocol driver to a miniport driver to 
            // specify a wake-up pattern. The wake-up pattern, along with its mask, 
            // is described by an NDIS_PM_PACKET_PATTERN structure.
            //
            pPmPattern = (PNDIS_PM_PACKET_PATTERN) InformationBuffer;
            if (InformationBufferLength < sizeof(NDIS_PM_PACKET_PATTERN))
            {
                Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                
                *BytesNeeded = sizeof(NDIS_PM_PACKET_PATTERN);
                break;
            }
            if (InformationBufferLength < pPmPattern->PatternOffset + pPmPattern->PatternSize)
            {
                Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                
                *BytesNeeded = pPmPattern->PatternOffset + pPmPattern->PatternSize;
                break;
            }        
            *BytesRead = pPmPattern->PatternOffset + pPmPattern->PatternSize;
            Status = NDIS_STATUS_SUCCESS;
            bForwardRequest = TRUE;            
            break;
    
        case OID_PNP_REMOVE_WAKE_UP_PATTERN:
            //
            // This OID requests the miniport driver to delete a wake-up pattern 
            // that it previously received in an OID_PNP_ADD_WAKE_UP_PATTERN request.
            // The wake-up pattern, along with its mask, is described by an 
            // NDIS_PM_PACKET_PATTERN structure. 
            //
            pPmPattern = (PNDIS_PM_PACKET_PATTERN) InformationBuffer;
            if (InformationBufferLength < sizeof(NDIS_PM_PACKET_PATTERN))
            {
                Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                
                *BytesNeeded = sizeof(NDIS_PM_PACKET_PATTERN);
                break;
            }
            if (InformationBufferLength < pPmPattern->PatternOffset + pPmPattern->PatternSize)
            {
                Status = NDIS_STATUS_BUFFER_TOO_SHORT;
                
                *BytesNeeded = pPmPattern->PatternOffset + pPmPattern->PatternSize;
                break;
            }            
            *BytesRead = pPmPattern->PatternOffset + pPmPattern->PatternSize;
            Status = NDIS_STATUS_SUCCESS;
            bForwardRequest = TRUE;
            
            break;

        case OID_PNP_ENABLE_WAKE_UP:
            //
            // This OID specifies which wake-up capabilities a miniport 
            // driver should enable in its NIC. Before the miniport
            // transitions to a low-power state (that is, before NDIS 
            // sends the miniport driver an OID_PNP_SET_POWER request), 
            // NDIS sends the miniport an OID_PNP_ENABLE_WAKE_UP request to
            // enable the appropriate wake-up capabilities. 
            //
            DEBUGP(MP_INFO, ("--> OID_PNP_ENABLE_WAKE_UP\n"));                        
            if(InformationBufferLength != sizeof(ULONG))
            {
                *BytesNeeded = sizeof(ULONG);
                Status = NDIS_STATUS_INVALID_LENGTH;
                break;
            }
            *BytesRead = sizeof(ULONG);                         
            Status = NDIS_STATUS_SUCCESS; 
            bForwardRequest = TRUE;
            break;

        default:
            Status = NDIS_STATUS_INVALID_OID;
            break;

    }
    
    if(Status == NDIS_STATUS_SUCCESS && bForwardRequest && 
        MP_IS_READY(Adapter) && InformationBufferLength){
        
        Status = NICForwardRequest(Adapter,
                            NdisRequestSetInformation,
                            Oid,
                            InformationBuffer,
                            InformationBufferLength,
                            BytesRead,
                            BytesNeeded
                            );
        ASSERT(Status == NDIS_STATUS_SUCCESS);
        Status = NDIS_STATUS_PENDING;
    }

    DEBUGP(MP_VERY_LOUD, ("<-- MPSetInformation Status = 0x%08x\n", Status));
    
    return(Status);
}

ULONG 
NICGetMediaConnectStatus(
    PMP_ADAPTER Adapter
    )
/*++
Routine Description:
    This routine will query the hardware and return      
    the media status.

Arguments:
    IN PMP_ADAPTER Adapter - pointer to adapter block
   
Return Value:
    NdisMediaStateDisconnected or
    NdisMediaStateConnected
    
--*/
{
    if(MP_TEST_FLAG(Adapter, fMP_DISCONNECTED))
    {
        return(NdisMediaStateDisconnected);
    }
    else
    {
        return(NdisMediaStateConnected);
    }
}

NDIS_STATUS 
NICSetPacketFilter(
    IN PMP_ADAPTER Adapter,
    IN ULONG PacketFilter)
/*++
Routine Description:
    This routine will set up the adapter so that it accepts packets 
    that match the specified packet filter.  The only filter bits   
    that can truly be toggled are for broadcast and promiscuous     

Arguments:
    IN PMP_ADAPTER Adapter - pointer to adapter block
    IN ULONG PacketFilter - the new packet filter 
    
Return Value:
    NDIS_STATUS_SUCCESS
    NDIS_STATUS_NOT_SUPPORTED
    
--*/

{
    NDIS_STATUS      Status = NDIS_STATUS_SUCCESS;
    
    DEBUGP(MP_TRACE, ("--> NICSetPacketFilter\n"));
    
    // any bits not supported?
    if(PacketFilter & ~NIC_SUPPORTED_FILTERS)
    {
        return(NDIS_STATUS_NOT_SUPPORTED);
    }
    
    // any filtering changes?
    if(PacketFilter != Adapter->PacketFilter)
    {   
        //
        // Change the filtering modes on hardware
        // TODO 
                
                
        // Save the new packet filter value                                                               
        Adapter->PacketFilter = PacketFilter;
    }

    DEBUGP(MP_TRACE, ("<-- NICSetPacketFilter\n"));
    
    return(Status);
}


NDIS_STATUS 
NICSetMulticastList(
    IN PMP_ADAPTER              Adapter,
    IN PVOID                    InformationBuffer,
    IN ULONG                    InformationBufferLength,
    OUT PULONG                  pBytesRead,
    OUT PULONG                  pBytesNeeded
    )
/*++
Routine Description:
    This routine will set up the adapter for a specified multicast
    address list.                                                 
    
Arguments:
    IN PMP_ADAPTER Adapter - Pointer to adapter block
    InformationBuffer       - Buffer for information
    InformationBufferLength   Size of this buffer
    pBytesRead                Specifies how much info is read
    BytesNeeded               In case the buffer is smaller than 
    
Return Value:

    NDIS_STATUS
    
--*/
{
    NDIS_STATUS            Status = NDIS_STATUS_SUCCESS;
    ULONG                  index;

    DEBUGP(MP_TRACE, ("--> NICSetMulticastList\n"));
    
    //
    // Initialize.
    //
    *pBytesNeeded = 0;
    *pBytesRead = InformationBufferLength;

    do
    {
        if (InformationBufferLength % ETH_LENGTH_OF_ADDRESS)
        {
            Status = NDIS_STATUS_INVALID_LENGTH;
            break;
        }

        if (InformationBufferLength > (NIC_MAX_MCAST_LIST * ETH_LENGTH_OF_ADDRESS))
        {
            Status = NDIS_STATUS_MULTICAST_FULL;
            *pBytesNeeded = NIC_MAX_MCAST_LIST * ETH_LENGTH_OF_ADDRESS;
            break;
        }

        //
        // Protect the list update with a lock if it can be updated by
        // another thread simultaneously.
        //

        NdisZeroMemory(Adapter->MCList,
                       NIC_MAX_MCAST_LIST * ETH_LENGTH_OF_ADDRESS);
        
        NdisMoveMemory(Adapter->MCList,
                       InformationBuffer,
                       InformationBufferLength);
        
        Adapter->ulMCListSize =    InformationBufferLength / ETH_LENGTH_OF_ADDRESS;
        
#if DBG
        // display the multicast list
        for(index = 0; index < Adapter->ulMCListSize; index++)
        {
            DEBUGP(MP_VERY_LOUD, ("MC(%d) = %02x-%02x-%02x-%02x-%02x-%02x\n", 
                index,
                Adapter->MCList[index][0],
                Adapter->MCList[index][1],
                Adapter->MCList[index][2],
                Adapter->MCList[index][3],
                Adapter->MCList[index][4],
                Adapter->MCList[index][5]));
        }
#endif        
    }
    while (FALSE);    

    //
    // Program the hardware to add suport for these muticast addresses
    // 

    DEBUGP(MP_TRACE, ("<-- NICSetMulticastList\n"));
    
    return(Status);

}

NDIS_STATUS
    NICForwardRequest(
    IN PMP_ADAPTER                  Adapter,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      BytesReadOrWritten,
    OUT PULONG                      BytesNeeded
    )
/*++

Routine Description:

    Utility routine that forwards an NDIS request made on a Adapter to the
    lower binding. Since at most a single request can be pended on a Adapter,
    we use the pre-allocated request structure embedded in the Adapter struct.

Arguments:


Return Value:

    NDIS_STATUS_SUCCESS if the request is sent down.

--*/
{
    PWDM_NDIS_REQUEST   wdmNdisRequest = &Adapter->WdmNdisRequest;

    DEBUGP(MP_TRACE, ("--> NICForwardRequest\n"));

    wdmNdisRequest->Oid = Oid;
    wdmNdisRequest->RequestType = RequestType;
    wdmNdisRequest->InformationBuffer = InformationBuffer;
    wdmNdisRequest->InformationBufferLength = InformationBufferLength;
    wdmNdisRequest->BytesNeeded = BytesNeeded;
    wdmNdisRequest->BytesReadOrWritten = BytesReadOrWritten;

    NdisAcquireSpinLock(&Adapter->Lock);
    Adapter->RequestPending = TRUE;    
    NdisReleaseSpinLock(&Adapter->Lock);
    
    MP_INC_REF(Adapter);

    NdisInitializeWorkItem(&wdmNdisRequest->WorkItem, 
                        NICRequestWorkItemCallback, Adapter);
    
    NdisScheduleWorkItem(&wdmNdisRequest->WorkItem);     

    DEBUGP(MP_TRACE, ("<-- NICForwardRequest\n"));
    
    return (NDIS_STATUS_SUCCESS);
}

VOID 
NICRequestWorkItemCallback(
    PNDIS_WORK_ITEM WorkItem, 
    PVOID Context) 
{

    PMP_ADAPTER Adapter = (PMP_ADAPTER)Context;
    PWDM_NDIS_REQUEST wdmNdisRequest = (PWDM_NDIS_REQUEST)WorkItem;
    NDIS_STATUS Status;
    BOOLEAN isResetPending = FALSE;
    
    DEBUGP(MP_TRACE, ("--> NICRequestWorkItemCallback\n"));

    ASSERT(wdmNdisRequest == &Adapter->WdmNdisRequest);

    Status = NICSendRequest(Adapter, 
                    wdmNdisRequest->RequestType,
                    wdmNdisRequest->Oid,
                    wdmNdisRequest->InformationBuffer,
                    wdmNdisRequest->InformationBufferLength,
                    wdmNdisRequest->BytesReadOrWritten,
                    wdmNdisRequest->BytesNeeded
                    );

    //
    // Make sure the multicast list reported by the physical doesn't exceed our
    // hardcoded limit of NIC_MAX_MCAST_LIST.
    //
    if(wdmNdisRequest->Oid == OID_802_3_MAXIMUM_LIST_SIZE){
        if(*(PLONG)wdmNdisRequest->InformationBuffer > NIC_MAX_MCAST_LIST) {
            *(PLONG)wdmNdisRequest->InformationBuffer = NIC_MAX_MCAST_LIST;
        }
    }

    //
    // Check to see if there is any pending reset. If so we should complet it
    // after completing the request.
    //
    NdisAcquireSpinLock(&Adapter->Lock);

    Adapter->RequestPending = FALSE;    
    if(Adapter->ResetPending){
        
        Adapter->ResetPending = FALSE;
        isResetPending = TRUE;
    }     
    
    NdisReleaseSpinLock(&Adapter->Lock);
    
    //
    // Inform NDIS to complete the pending request and reset.
    //
    if(wdmNdisRequest->RequestType == NdisRequestSetInformation){            
        NdisMSetInformationComplete(Adapter->AdapterHandle, Status);
    }else {
        NdisMQueryInformationComplete(Adapter->AdapterHandle, Status);
    }
    
    //
    // NDIS will not send another OID request as long as a reset is pending.
    // So we don't have to worry about another requests sneeking in before
    // we complete the reset.
    //
    if(isResetPending){
        DEBUGP(MP_LOUD, ("Called NdisMResetComplete\n"));        
        NdisMResetComplete(Adapter->AdapterHandle, NDIS_STATUS_SUCCESS, FALSE);
    }
        
    MP_DEC_REF(Adapter);     

    DEBUGP(MP_TRACE, ("<-- NICRequestWorkItemCallback: %x\n", Status));

    return;
}


NDIS_STATUS
NICSendRequest(
    IN PMP_ADAPTER                  Adapter,
    IN NDIS_REQUEST_TYPE            RequestType,
    IN NDIS_OID                     Oid,
    IN PVOID                        InformationBuffer,
    IN ULONG                        InformationBufferLength,
    OUT PULONG                      BytesReadOrWritten,
    OUT PULONG                      BytesNeeded
    )
/*++

Routine Description:

    Utility routine that forwards an NDIS request made on a Adapter to the
    target driver. 
    
Arguments:


Return Value:


--*/
{
    NDIS_STATUS         Status;
    PNDISPROT_QUERY_OID pQueryOid = NULL;
    PNDISPROT_SET_OID   pSetOid = NULL;    
    PVOID               buffer;
    NTSTATUS            ntStatus;
    ULONG               length, bytes;
    
    DEBUGP(MP_TRACE, ("--> NICSendRequest\n"));


    PAGED_CODE();

    //
    // Allocate memory for adapter context
    //
    
    if(NdisRequestQueryInformation){
        length = sizeof(NDISPROT_QUERY_OID) + InformationBufferLength - sizeof(ULONG);
    }else{
        length = sizeof(NDISPROT_SET_OID) + InformationBufferLength - sizeof(ULONG);
    }
    
    Status = NdisAllocateMemoryWithTag(&buffer, length, NIC_TAG);
    if(Status != NDIS_STATUS_SUCCESS)
    {
        DEBUGP(MP_ERROR, ("NICSendRequest failed to allocate memory\n"));
        goto End;
    }

    *BytesNeeded = 0;
    
    switch (RequestType)
    {
        case NdisRequestQueryInformation:
            
            
            pQueryOid = (PNDISPROT_QUERY_OID) buffer;            
            pQueryOid->Oid = Oid;
            
            ntStatus = NICMakeSynchronousIoctl(
                            Adapter->TargetDeviceObject,
                            Adapter->FileObject,
                            IOCTL_NDISPROT_QUERY_OID_VALUE,
                            pQueryOid,
                            length,
                            pQueryOid,
                            length,
                            &bytes
                            );
            if(!NT_SUCCESS(ntStatus)) {
                
                DEBUGP(MP_ERROR, ("NICMakeSynchronousIoctl failed %x\n", 
                                    ntStatus));
                *BytesNeeded = bytes;
                
            } else {
            
            
                RtlCopyMemory(InformationBuffer, pQueryOid->Data, 
                                        InformationBufferLength);
                *BytesReadOrWritten = bytes - FIELD_OFFSET(NDISPROT_QUERY_OID, Data);
            }
            
            NT_STATUS_TO_NDIS_STATUS(ntStatus, &Status);
            break;

        case NdisRequestSetInformation:

            pSetOid = (PNDISPROT_SET_OID) buffer;            
            pSetOid->Oid = Oid;
            
            RtlCopyMemory(pSetOid->Data, InformationBuffer, 
                                    InformationBufferLength);
            
            ntStatus = NICMakeSynchronousIoctl(
                            Adapter->TargetDeviceObject,
                            Adapter->FileObject,
                            IOCTL_NDISPROT_SET_OID_VALUE,
                            pSetOid,
                            length,
                            NULL,
                            0,
                            &bytes
                            );
            if(!NT_SUCCESS(ntStatus)) {
                
                DEBUGP(MP_ERROR, ("NICMakeSynchronousIoctl failed %x\n", ntStatus)); 
                *BytesNeeded = bytes;
            }else {            
                //
                // Unfortunately, the NDISPROT driver returns 0 as the number of
                // bytes read. So let us just use the InformationBufferLength.
                //
                *BytesReadOrWritten = InformationBufferLength;
            }
            
            NT_STATUS_TO_NDIS_STATUS(ntStatus, &Status);            
            break;

        default:
            ASSERT(FALSE);
            break;
    }

End:
    if(buffer){
        NdisFreeMemory(buffer, length, 0);
    }
    
    DEBUGP(MP_TRACE, ("<-- NICSendRequest %x\n", Status));
    
    return (Status);
}

PUCHAR 
DbgGetOidName(ULONG oid)
{
    PCHAR oidName;

    switch (oid){

        #undef MAKECASE
        #define MAKECASE(oidx) case oidx: oidName = #oidx; break;

        MAKECASE(OID_GEN_SUPPORTED_LIST)
        MAKECASE(OID_GEN_HARDWARE_STATUS)
        MAKECASE(OID_GEN_MEDIA_SUPPORTED)
        MAKECASE(OID_GEN_MEDIA_IN_USE)
        MAKECASE(OID_GEN_MAXIMUM_LOOKAHEAD)
        MAKECASE(OID_GEN_MAXIMUM_FRAME_SIZE)
        MAKECASE(OID_GEN_LINK_SPEED)
        MAKECASE(OID_GEN_TRANSMIT_BUFFER_SPACE)
        MAKECASE(OID_GEN_RECEIVE_BUFFER_SPACE)
        MAKECASE(OID_GEN_TRANSMIT_BLOCK_SIZE)
        MAKECASE(OID_GEN_RECEIVE_BLOCK_SIZE)
        MAKECASE(OID_GEN_VENDOR_ID)
        MAKECASE(OID_GEN_VENDOR_DESCRIPTION)
        MAKECASE(OID_GEN_CURRENT_PACKET_FILTER)
        MAKECASE(OID_GEN_CURRENT_LOOKAHEAD)
        MAKECASE(OID_GEN_DRIVER_VERSION)
        MAKECASE(OID_GEN_MAXIMUM_TOTAL_SIZE)
        MAKECASE(OID_GEN_PROTOCOL_OPTIONS)
        MAKECASE(OID_GEN_MAC_OPTIONS)
        MAKECASE(OID_GEN_MEDIA_CONNECT_STATUS)
        MAKECASE(OID_GEN_MAXIMUM_SEND_PACKETS)
        MAKECASE(OID_GEN_VENDOR_DRIVER_VERSION)
        MAKECASE(OID_GEN_SUPPORTED_GUIDS)
        MAKECASE(OID_GEN_NETWORK_LAYER_ADDRESSES)
        MAKECASE(OID_GEN_TRANSPORT_HEADER_OFFSET)
        MAKECASE(OID_GEN_MEDIA_CAPABILITIES)
        MAKECASE(OID_GEN_PHYSICAL_MEDIUM)
        MAKECASE(OID_GEN_XMIT_OK)
        MAKECASE(OID_GEN_RCV_OK)
        MAKECASE(OID_GEN_XMIT_ERROR)
        MAKECASE(OID_GEN_RCV_ERROR)
        MAKECASE(OID_GEN_RCV_NO_BUFFER)
        MAKECASE(OID_GEN_DIRECTED_BYTES_XMIT)
        MAKECASE(OID_GEN_DIRECTED_FRAMES_XMIT)
        MAKECASE(OID_GEN_MULTICAST_BYTES_XMIT)
        MAKECASE(OID_GEN_MULTICAST_FRAMES_XMIT)
        MAKECASE(OID_GEN_BROADCAST_BYTES_XMIT)
        MAKECASE(OID_GEN_BROADCAST_FRAMES_XMIT)
        MAKECASE(OID_GEN_DIRECTED_BYTES_RCV)
        MAKECASE(OID_GEN_DIRECTED_FRAMES_RCV)
        MAKECASE(OID_GEN_MULTICAST_BYTES_RCV)
        MAKECASE(OID_GEN_MULTICAST_FRAMES_RCV)
        MAKECASE(OID_GEN_BROADCAST_BYTES_RCV)
        MAKECASE(OID_GEN_BROADCAST_FRAMES_RCV)
        MAKECASE(OID_GEN_RCV_CRC_ERROR)
        MAKECASE(OID_GEN_TRANSMIT_QUEUE_LENGTH)
        MAKECASE(OID_GEN_GET_TIME_CAPS)
        MAKECASE(OID_GEN_GET_NETCARD_TIME)
        MAKECASE(OID_GEN_NETCARD_LOAD)
        MAKECASE(OID_GEN_DEVICE_PROFILE)
        MAKECASE(OID_GEN_INIT_TIME_MS)
        MAKECASE(OID_GEN_RESET_COUNTS)
        MAKECASE(OID_GEN_MEDIA_SENSE_COUNTS)
        MAKECASE(OID_PNP_CAPABILITIES)
        MAKECASE(OID_PNP_SET_POWER)
        MAKECASE(OID_PNP_QUERY_POWER)
        MAKECASE(OID_PNP_ADD_WAKE_UP_PATTERN)
        MAKECASE(OID_PNP_REMOVE_WAKE_UP_PATTERN)
        MAKECASE(OID_PNP_ENABLE_WAKE_UP)
        MAKECASE(OID_802_3_PERMANENT_ADDRESS)
        MAKECASE(OID_802_3_CURRENT_ADDRESS)
        MAKECASE(OID_802_3_MULTICAST_LIST)
        MAKECASE(OID_802_3_MAXIMUM_LIST_SIZE)
        MAKECASE(OID_802_3_MAC_OPTIONS)
        MAKECASE(OID_802_3_RCV_ERROR_ALIGNMENT)
        MAKECASE(OID_802_3_XMIT_ONE_COLLISION)
        MAKECASE(OID_802_3_XMIT_MORE_COLLISIONS)
        MAKECASE(OID_802_3_XMIT_DEFERRED)
        MAKECASE(OID_802_3_XMIT_MAX_COLLISIONS)
        MAKECASE(OID_802_3_RCV_OVERRUN)
        MAKECASE(OID_802_3_XMIT_UNDERRUN)
        MAKECASE(OID_802_3_XMIT_HEARTBEAT_FAILURE)
        MAKECASE(OID_802_3_XMIT_TIMES_CRS_LOST)
        MAKECASE(OID_802_3_XMIT_LATE_COLLISIONS)

        default: 
            oidName = "<** UNKNOWN OID **>";
            break;
    }

    return oidName;
}


