/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

        SendRCV.C
        
Abstract:

    This module contains miniport functions for handling Send & Receive
    packets and other helper routines called by these miniport functions.

    In order to excercise the send and receive code path of this driver, 
    you should install more than one instance of the miniport. If there 
    is only one instance installed, the driver throws the send packet on
    the floor and completes the send successfully. If there are more 
    instances present, it indicates the incoming send packet to the other
    instances. For example, if there 3 instances: A, B, & C installed. 
    Packets coming in for A instance would be indicated to B & C; packets 
    coming into B would be indicated to C, & A; and packets coming to C 
    would be indicated to A & B. 
    
Revision History:

Notes:

--*/
#include "miniport.h"


VOID 
MPSendPackets(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets)
/*++

Routine Description:

    Send Packet Array handler. Called by NDIS whenever a protocol
    bound to our miniport sends one or more packets.

    The input packet descriptor pointers have been ordered according 
    to the order in which the packets should be sent over the network 
    by the protocol driver that set up the packet array. The NDIS 
    library preserves the protocol-determined ordering when it submits
    each packet array to MiniportSendPackets

    As a deserialized driver, we are responsible for holding incoming send 
    packets in our internal queue until they can be transmitted over the 
    network and for preserving the protocol-determined ordering of packet
    descriptors incoming to its MiniportSendPackets function. 
    A deserialized miniport driver must complete each incoming send packet
    with NdisMSendComplete, and it cannot call NdisMSendResourcesAvailable. 

    Runs at IRQL <= DISPATCH_LEVEL
    
Arguments:

    MiniportAdapterContext    Pointer to our adapter context
    PacketArray               Set of packets to send
    NumberOfPackets           Length of above array

Return Value:

    None
    
--*/
{
    PMP_ADAPTER       Adapter;
    NDIS_STATUS       Status;
    UINT              PacketCount;

    DEBUGP(MP_TRACE, ("---> MPSendPackets\n"));

    Adapter = (PMP_ADAPTER)MiniportAdapterContext;

    for(PacketCount=0;PacketCount < NumberOfPackets; PacketCount++)
    {
        //
        // Check for a zero pointer
        //
        ASSERT(PacketArray[PacketCount]);

        Status = NICSendPacket(Adapter, PacketArray[PacketCount]);

    }

    DEBUGP(MP_TRACE, ("<--- MPSendPackets\n"));

    return;
}

VOID 
MPReturnPacket(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_PACKET Packet)
/*++

Routine Description:

    NDIS Miniport entry point called whenever protocols are done with
    a packet that we had indicated up and they had queued up for returning
    later.

Arguments:

    MiniportAdapterContext    - pointer to MP_ADAPTER structure
    Packet    - packet being returned.

Return Value:

    None.

--*/
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    DEBUGP(MP_TRACE, ("---> MPReturnPacket\n"));

    NICFreeRecvPacket(Adapter, Packet);
    
    DEBUGP(MP_TRACE, ("<--- MPReturnPacket\n"));
}



NDIS_STATUS 
NICSendPacket(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet)
/*++

Routine Description:

    This routine copies the packet content to a TCB, gets a receive packet,
    associates the TCB buffer to this recive packet and queues
    receive packet with the same data on one or more miniport instances
    controlled by this driver. For receive path to be active, you have
    to install more than one instance of this miniport.
        
Arguments:

    Adapter    - pointer to the MP_ADAPTER structure
    Packet    - packet to be transfered.


Return Value:

    NDIS_STATUS_SUCCESS or NDIS_STATUS_PENDING

--*/
{
    PMP_ADAPTER       DestAdapter;
    NDIS_STATUS       Status = NDIS_STATUS_SUCCESS;
    PTCB              pTCB = NULL;

    DEBUGP(MP_TRACE, ("--> NICSendPacket, Packet= %p\n", Packet));
    
    //
    // Go through the adapter list and queue packet for
    // indication on them if there are any. Otherwise
    // just drop the packet on the floor and tell NDIS that
    // you have completed send.
    //
    NdisAcquireSpinLock(&GlobalData.Lock);
    DestAdapter = (PMP_ADAPTER) &GlobalData.AdapterList;       

    while(MP_IS_READY(Adapter))
    {
        DestAdapter = (PMP_ADAPTER) DestAdapter->List.Flink;
        if((PLIST_ENTRY)DestAdapter == &GlobalData.AdapterList)
        {
            //
            // We have reached the end of the adapter list. So
            //
            break;
        }

        //
        // We wouldn't transmit the packet if:
        // a) The destination adapter is same as the Send Adapter.
        // b) The destination adapter is not ready to receive packets.
        // c) The packet itself is not worth transmitting.
        //
        if(DestAdapter == Adapter ||
            !MP_IS_READY(DestAdapter) || 
            !NICIsPacketTransmittable(DestAdapter, Packet))
        {
            continue;
        }
        
        DEBUGP(MP_LOUD, ("Packet is accepted...\n"));

        if(!pTCB)
        {
            //
            // Get a TCB from the SendFreelist.
            //
            NdisAcquireSpinLock(&Adapter->SendLock);            
            if(IsListEmpty(&Adapter->SendFreeList)){
                
                DEBUGP(MP_WARNING, ("TCB not available......!\n")); 

                Status = NDIS_STATUS_PENDING;    

                //
                // Not able to get TCB block for this send. So queue
                // it for later transmission and break out of the loop.
                //
                InsertTailList(
                    &Adapter->SendWaitList, 
                    (PLIST_ENTRY)&Packet->MiniportReserved[0] );
                
                NdisReleaseSpinLock(&Adapter->SendLock);            
                
                break;
            }
            else
            {
                pTCB = (PTCB) RemoveHeadList(&Adapter->SendFreeList);
                Adapter->nBusySend++;
                ASSERT(Adapter->nBusySend <= NIC_MAX_BUSY_SENDS);
                NdisReleaseSpinLock(&Adapter->SendLock);            
            
                //
                // Copy the packet content into the TCB data buffer, 
                // assuming the NIC is doing a common buffer DMA. For
                // scatter/gather DMA, this copy operation is not required.
                // For efficiency, I could have avoided the copy operation
                // in this driver and directly indicated the send buffers to 
                // other miniport instances since I'm holding the send packet
                // until all the indicated packets are returned. Oh, well!
                //
                if(!NICCopyPacket(Adapter, pTCB, Packet)){
                    DEBUGP(MP_ERROR, ("NICCopyPacket failed\n"));  
                    //
                    // Free the TCB and complete the send.
                    //
                    Status = NDIS_STATUS_FAILURE;
                    break;
                }
            }
        }

        Status = NDIS_STATUS_PENDING;    

        NICQueuePacketForRecvIndication(DestAdapter, pTCB);              

    } // while

    NdisReleaseSpinLock(&GlobalData.Lock);

    NDIS_SET_PACKET_STATUS(Packet, Status);

    if(NDIS_STATUS_SUCCESS == Status || 
        (pTCB && (NdisInterlockedDecrement(&pTCB->Ref) == 0)))
    {
        //
        // We will end up here if the Adapter is not ready, copypacket failed
        // above or the packet we indicated has already returned back to us.
        //
        DEBUGP(MP_LOUD, ("Calling NdisMSendComplete \n"));

        Status = NDIS_STATUS_SUCCESS;

        Adapter->GoodTransmits++;

        NdisMSendComplete(
            Adapter->AdapterHandle,
            Packet,
            Status);

        if(pTCB)
        {
            NICFreeSendTCB(Adapter, pTCB);
        }
    }

    DEBUGP(MP_TRACE, ("<-- NICSendPacket Status = 0x%08x\n", Status));

    return(Status);
}  



VOID
NICQueuePacketForRecvIndication(
    PMP_ADAPTER Adapter,
    PTCB pTCB)
/*++

Routine Description:

    This routine queues the send packet in to the destination
    adapters RecvWaitList and fires a timer DPC so that it 
    cab be indicated as soon as possible.

Arguments:

    Adapter    - pointer to the destination adapter structure
    pTCB      - pointer to TCB block
        

Return Value:

    VOID

--*/
{
    PNDIS_PACKET     SendPacket = pTCB->OrgSendPacket;
    PNDIS_PACKET     RecvPacket = NULL;
    PNDIS_BUFFER     CurrentBuffer = NULL;   
    UINT             NumPhysDesc, BufferCount, PacketLength, RecvPacketLength;             
    PLIST_ENTRY      pEntry;
    PRCB             pRCB;
    NDIS_STATUS      Status;
    

    DEBUGP(MP_TRACE, ("--> NICQueuePacketForRecvIndication\n"));

    //
    // Allocate memory for RCB. 
    //
    pRCB = NdisAllocateFromNPagedLookasideList(&Adapter->RecvLookaside);
    if(!pRCB)
    {
        DEBUGP(MP_ERROR, ("Failed to allocate memory for RCB\n"));
        return;
    }  
    
    //
    // Get a free recv packet descriptor from the list.
    //
    pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                    &Adapter->RecvFreeList, 
                    &Adapter->RecvLock);
    if(!pEntry)
    {
        ++Adapter->RcvResourceErrors;
        NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pRCB);
    }
    else
    {
        ++Adapter->GoodReceives;
    
        RecvPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);
        
        //
        // Prepare the recv packet
        //
        NdisReinitializePacket(RecvPacket);
        *((PTCB *)RecvPacket->MiniportReserved) = pTCB;

        //
        // Chain the TCB buffers to the packet
        //
        NdisChainBufferAtBack(RecvPacket, pTCB->Buffer);
                    
#if DBG
        NdisQueryPacket(
            RecvPacket,
            NULL,
            NULL,
            &CurrentBuffer,
            &RecvPacketLength);
    
        ASSERT(CurrentBuffer == pTCB->Buffer);
        
        NdisQueryPacket(
            SendPacket,
            NULL,
            NULL,
            NULL,
            &PacketLength);
    
        if((RecvPacketLength != 60) && (RecvPacketLength != PacketLength))
        {
            DEBUGP(MP_ERROR, ("RecvPacketLength = %d, PacketLength = %d\n",
                RecvPacketLength, PacketLength));
            DEBUGP(MP_ERROR, ("RecvPacket = %p, Packet = %p\n",
                RecvPacket, SendPacket));
            ASSERT(FALSE);
        }
#endif    
                  
        NDIS_SET_PACKET_STATUS(RecvPacket, NDIS_STATUS_SUCCESS);
    
        DEBUGP(MP_LOUD, ("RecvPkt= %p\n", RecvPacket));

        //
        // Initialize RCB 
        //
        NdisInitializeListHead(&pRCB->List);
        pRCB->Packet = RecvPacket;
        
        //
        // Increment the Ref count on the TCB to denote that it's being
        // used. This reference will be removed when the indicated
        // Recv packet finally returns from the protocol.
        //
        NdisInterlockedIncrement(&pTCB->Ref);     

        //
        // Insert the packet in the recv wait queue to be picked up by
        // the receive indication DPC.
        // 
        NdisAcquireSpinLock(&Adapter->RecvLock);
        InsertTailList(&Adapter->RecvWaitList, 
                            &pRCB->List);
        Adapter->nBusyRecv++;     
        ASSERT(Adapter->nBusyRecv <= NIC_MAX_BUSY_RECVS);
        NdisReleaseSpinLock(&Adapter->RecvLock);

        //
        // Fire a timer DPC. By specifing zero timeout, the DPC will
        // be serviced whenever the next system timer interrupt arrives.
        //
        NdisMSetTimer(&Adapter->RecvTimer, 0);
    
    }

    DEBUGP(MP_TRACE, ("<-- NICQueuePacketForRecvIndication\n"));
}

VOID
NICIndicateReceiveTimerDpc(
    IN PVOID             SystemSpecific1,
    IN PVOID             FunctionContext,
    IN PVOID             SystemSpecific2,
    IN PVOID             SystemSpecific3)
/*++

Routine Description:

    Timer callback function for Receive Indication. Please note that receive
    timer DPC is not required when you are talking to a real device. In real
    miniports, this DPC is usually provided by NDIS as MPHandleInterrupt
    callback whenever the device interrupts for receive indication.
        
Arguments:

FunctionContext - Pointer to our adapter

Return Value:

    VOID

--*/
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER)FunctionContext;
    PRCB pRCB = NULL;
    PLIST_ENTRY pEntry = NULL;
    
    DEBUGP(MP_TRACE, ("--->NICIndicateReceiveTimerDpc = %p\n", Adapter));

    //
    // Increment the ref count on the adapter to prevent the driver from
    // unloding while the DPC is running. The Halt handler waits for the
    // ref count to drop to zero before returning. 
    //
    MP_INC_REF(Adapter); 
    
    //
    // Remove the packet from waitlist and indicate it to the protocols
    // above us.
    //
    while (pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                    &Adapter->RecvWaitList, 
                    &Adapter->RecvLock)) {
    
        pRCB = CONTAINING_RECORD(pEntry, RCB, List);

        ASSERT(pRCB);
        ASSERT(pRCB->Packet);
        
        DEBUGP(MP_LOUD, ("Indicating packet = %p\n", pRCB->Packet));
        
        NdisMIndicateReceivePacket(
            Adapter->AdapterHandle,
            &pRCB->Packet,
            1);
        
        //
        // We are done with RCB memory. So free it.
        //
        NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pRCB);
        
    }
    
    MP_DEC_REF(Adapter);
    DEBUGP(MP_TRACE, ("<---NICIndicateReceiveTimerDpc\n"));    
}

VOID 
NICFreeRecvPacket(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet)
/*++

Routine Description:

    Adapter    - pointer to the adapter structure
    Packet      - pointer to the receive packet
        
Arguments:

    This is called by MPReturnPacket to free the Receive packet
    indicated above. Since we have used the send-side TCB, we 
    will also carefully complete the pending SendPacket if we are 
    the last one to use the TCB buffers.
    
Return Value:

    VOID

--*/
{

    PTCB pTCB = *(PTCB *)Packet->MiniportReserved;
    PMP_ADAPTER SendAdapter = (PMP_ADAPTER)pTCB->Adapter;
    PNDIS_PACKET SendPacket = pTCB->OrgSendPacket;    
    PLIST_ENTRY pEntry;
    
    DEBUGP(MP_TRACE, ("--> NICFreeRecvPacket\n"));
    DEBUGP(MP_INFO, ("Adapter= %p FreePkt= %p Ref=%d\n", 
                            SendAdapter, SendPacket, pTCB->Ref));

    ASSERT(pTCB->Ref > 0);
    ASSERT(Adapter);
    //
    // Put the packet back in the free list for reuse.
    //
    NdisAcquireSpinLock(&Adapter->RecvLock);    

    InsertTailList(
        &Adapter->RecvFreeList, 
        (PLIST_ENTRY)&Packet->MiniportReserved[0]);
    
    Adapter->nBusyRecv--;     
    ASSERT(Adapter->nBusyRecv >= 0);
    
    NdisReleaseSpinLock(&Adapter->RecvLock);    


    //
    // Check to see whether we are the last one to use the TCB
    // by decrementing the refcount. If so, complete the pending
    // Send packet and free the TCB block for reuse.
    // 
    if(NdisInterlockedDecrement(&pTCB->Ref) == 0)
    {
        Adapter->GoodTransmits++;

        NdisMSendComplete(
            SendAdapter->AdapterHandle,
            SendPacket,
            NDIS_STATUS_SUCCESS);
    
        NICFreeSendTCB(SendAdapter, pTCB);
        //
        // Before we exit, since we have the control, let use see if there any 
        // more packets waiting in the queue to be sent.
        //
        if(MP_IS_READY(SendAdapter))
        {
            pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                            &SendAdapter->SendWaitList, 
                            &SendAdapter->SendLock);
            if(pEntry)
            {
                SendPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);
                NICSendPacket(SendAdapter, SendPacket);             
            }
        }
    }

    DEBUGP(MP_TRACE, ("<-- NICFreeRecvPacket\n"));
}

VOID 
NICFreeSendTCB(
    IN PMP_ADAPTER Adapter,
    IN PTCB pTCB)
/*++

Routine Description:

    Adapter    - pointer to the adapter structure
    pTCB      - pointer to TCB block
        
Arguments:

    This routine reinitializes the TCB block and puts it back
    into the SendFreeList for reuse.
    

Return Value:

    VOID

--*/
{
    DEBUGP(MP_TRACE, ("--> NICFreeSendTCB %p\n", pTCB));
    
    pTCB->OrgSendPacket = NULL;
    pTCB->Buffer->Next = NULL;

    ASSERT(!pTCB->Ref);
    
    //
    // Re adjust the length to the originl size
    //
    NdisAdjustBufferLength(pTCB->Buffer, NIC_BUFFER_SIZE);

    //
    // Insert the TCB back in the send free list     
    //
    NdisAcquireSpinLock(&Adapter->SendLock);
    
    NdisInitializeListHead(&pTCB->List);
    InsertHeadList(&Adapter->SendFreeList, &pTCB->List);
    Adapter->nBusySend--;
    ASSERT(Adapter->nBusySend >= 0);
    
    NdisReleaseSpinLock(&Adapter->SendLock); 
    
    DEBUGP(MP_TRACE, ("<-- NICFreeSendTCB\n"));
    
}



VOID 
NICFreeQueuedSendPackets(
    PMP_ADAPTER Adapter
    )
/*++

Routine Description:

    This routine is called by the Halt or Reset handler to fail all
    the queued up SendPackets because the device is either
    gone, being stopped for resource rebalance, or reset.
    
Arguments:

    Adapter    - pointer to the adapter structure

Return Value:

    VOID

--*/
{
    PLIST_ENTRY       pEntry;
    PNDIS_PACKET      Packet;

    DEBUGP(MP_TRACE, ("--> NICFreeQueuedSendPackets\n"));

    while(TRUE)
    {
        pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                        &Adapter->SendWaitList, 
                        &Adapter->SendLock);
        if(!pEntry)
        {
            break;
        }

        Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);
        NdisMSendComplete(
            Adapter->AdapterHandle,
            Packet,
            NDIS_STATUS_FAILURE);
    }

    DEBUGP(MP_TRACE, ("<-- NICFreeQueuedSendPackets\n"));

}

VOID 
NICFreeQueuedRecvPackets(
    PMP_ADAPTER Adapter
    )
/*++

Routine Description:

    This routine is called by the Halt handler to fail all
    the queued up RecvPackets if it succeeds in cancelling
    the RecvIndicate timer DPC.
    
Arguments:

    Adapter    - pointer to the adapter structure

Return Value:

    VOID

--*/
{
    PLIST_ENTRY       pEntry;
    PRCB pRCB = NULL;

    DEBUGP(MP_TRACE, ("--> NICFreeQueuedRecvPackets\n"));

    while(TRUE)
    {
        pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                        &Adapter->RecvWaitList, 
                        &Adapter->RecvLock);
        if(!pEntry)
        {
            break;
        }

        pRCB = CONTAINING_RECORD(pEntry, RCB, List);

        ASSERT(pRCB);
        ASSERT(pRCB->Packet);
        
        NICFreeRecvPacket(Adapter, pRCB->Packet);
        
        //
        // We are done with RCB memory. So free it.
        //
        NdisFreeToNPagedLookasideList(&Adapter->RecvLookaside, pRCB);
        
    }

    DEBUGP(MP_TRACE, ("<-- NICFreeQueuedRecvPackets\n"));

}



BOOLEAN
NICIsPacketTransmittable(
    PMP_ADAPTER Adapter,
    PNDIS_PACKET Packet
    )
/*++

Routine Description:

    This routines checks to see whether the packet can be accepted
    for transmission based on the currently programmed filter type 
    of the NIC and the mac address of the packet.
    
Arguments:

    Adapter    - pointer to the adapter structure
    Packet      - pointer to the send packet


Return Value:

    True or False

--*/
{
    INT               Equal;            
    UINT              PacketLength;
    PNDIS_BUFFER      FirstBuffer = NULL;
    PUCHAR            Address = NULL;
    UINT              CurrentLength;
    ULONG             index;
    BOOLEAN           result = FALSE;
    
    DEBUGP(MP_LOUD, 
        ("DestAdapter=%p, PacketFilter = 0x%08x\n", 
        Adapter,
        Adapter->PacketFilter));

    
    do {

#ifdef NDIS51_MINIPORT
       NdisGetFirstBufferFromPacketSafe(
            Packet,
            &FirstBuffer,
            &Address,
            &CurrentLength,
            &PacketLength,
            NormalPagePriority);
#else
       NdisGetFirstBufferFromPacket(
            Packet,
            &FirstBuffer,
            &Address,
            &CurrentLength,
            &PacketLength);
#endif
        if(!Address){
            break;
        }
        

        DEBUGP(MP_LOUD, ("Dest Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
                    Address[0], Address[1], Address[2],
                    Address[3], Address[4], Address[5]));
        
        //
        // If the NIC is in promiscuous mode, we will transmit anything
        // and everything.
        //
        if(Adapter->PacketFilter & NDIS_PACKET_TYPE_PROMISCUOUS) {
            result = TRUE;
            break;
        } 
        else if(ETH_IS_BROADCAST(Address)) {
            //
            // If it's a broadcast packet, check our filter settings to see
            // we can transmit that.
            //
            if(Adapter->PacketFilter & NDIS_PACKET_TYPE_BROADCAST) {
                result = TRUE;
                break;
            }
        }
        else if(ETH_IS_MULTICAST(Address)) {
            //
            // If it's a multicast packet, check our filter settings to see
            // we can transmit that.
            //
            if(Adapter->PacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) {
                result = TRUE;
                break;
            }
            else if(Adapter->PacketFilter & NDIS_PACKET_TYPE_MULTICAST) {
                //
                // Check to see if the multicast address is in our list
                //
                Equal = (UINT)-1;
                for(index=0; index <  Adapter->ulMCListSize; index++) {
                    ETH_COMPARE_NETWORK_ADDRESSES_EQ(
                        Address,
                        Adapter->MCList[index], 
                        &Equal);
                    if(Equal == 0){ // 0 Implies equality
                        result = TRUE;
                        break;
                    }
                }
                if(Equal == 0){ // 0 Implies equality
                    result = TRUE;
                    break;
                }
            }
        }
        else if(Adapter->PacketFilter & NDIS_PACKET_TYPE_DIRECTED) {
            //
            // This has to be a directed packet. If so, does packet source 
            // address match with the mac address of the NIC.
            // 
            ETH_COMPARE_NETWORK_ADDRESSES_EQ(
                Address,
                Adapter->CurrentAddress, 
                &Equal);
            if(Equal == 0){
                result = TRUE;
                break;
            }
        }
        //
        // This is a junk packet. We can't transmit this.
        //
        result = FALSE;
        
    }while(FALSE);
    
    return result;
}

BOOLEAN
NICCopyPacket(
    PMP_ADAPTER Adapter,
    PTCB pTCB, 
    PNDIS_PACKET Packet)
/*++

Routine Description:

    This routine copies the packet data into the TCB data block.
        
Arguments:

    Adapter    - pointer to the MP_ADAPTER structure
    pTCB      - pointer to TCB block
    Packet    - packet to be transfered.


Return Value:

    VOID

--*/
{
    PNDIS_BUFFER   MyBuffer;
    PNDIS_BUFFER   CurrentBuffer;
    PVOID          VirtualAddress;
    UINT           CurrentLength;
    UINT           BytesToCopy;
    UINT           BytesCopied = 0;
    UINT           BufferCount;
    UINT           PacketLength;    
    UINT           DestBufferSize = NIC_BUFFER_SIZE;        
    PUCHAR         pDest;
    BOOLEAN        bResult = TRUE;
    
    DEBUGP(MP_TRACE, ("--> NICCopyPacket\n"));

    pTCB->OrgSendPacket = Packet;
    pTCB->Ref = 1;

    MyBuffer = pTCB->Buffer;
    pDest = pTCB->pData;
    
    MyBuffer->Next = NULL;

    NdisQueryPacket(Packet,
                    NULL,
                    &BufferCount,
                    &CurrentBuffer,
                    &PacketLength);

    ASSERT(PacketLength <= NIC_BUFFER_SIZE);

    BytesToCopy = min(PacketLength, NIC_BUFFER_SIZE); 
    
    if(BytesToCopy < ETH_MIN_PACKET_SIZE)
    {
        BytesToCopy = ETH_MIN_PACKET_SIZE;   // padding
    }
             
    while(CurrentBuffer && DestBufferSize)
    {
        NdisQueryBufferSafe(
            CurrentBuffer,
            &VirtualAddress,
            &CurrentLength,
            NormalPagePriority);
        
        if(VirtualAddress == NULL){
            bResult = FALSE;
            break;
        }

        CurrentLength = min(CurrentLength, DestBufferSize); 

        if(CurrentLength)
        {
            // Copy the data.
            NdisMoveMemory(pDest, VirtualAddress, CurrentLength);
            BytesCopied += CurrentLength;
            DestBufferSize -= CurrentLength;
            pDest += CurrentLength;
        }

        NdisGetNextBuffer(
            CurrentBuffer,
            &CurrentBuffer);
    }

    if(bResult) {
        if(BytesCopied < BytesToCopy)
        {
            //
            // This would be the case if the packet size is less than 
            // ETH_MIN_PACKET_SIZE. Pad the buffer
            // up to ETH_MIN_PACKET_SIZE with zero bytes to avoid
            // information disclosure security attack. Remeber I of
            // STRIDE?
            //            
            //
            NdisZeroMemory(pDest, BytesToCopy-BytesCopied);
            BytesCopied = BytesToCopy;
        }
        NdisAdjustBufferLength(MyBuffer, BytesCopied);        
    }

    DEBUGP(MP_TRACE, ("<-- NICCopyPacket\n"));

    return bResult;
}

