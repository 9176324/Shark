/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

        Send.C
        
Abstract:

    This module contains miniport functions for handling Send
    packets and other helper routines called by the miniport function.

Author:  Eliyas Yakub (Jan 11, 2003)

    
Revision History:

Notes:

--*/
#include "ndiswdm.h"


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
    UINT              PacketCount;
    PNDIS_PACKET      Packet = NULL;
    NDIS_STATUS       Status = NDIS_STATUS_FAILURE;
    PTCB              pTCB = NULL;
    NTSTATUS          ntStatus;
    
    DEBUGP(MP_TRACE, ("---> MPSendPackets\n"));

    Adapter = (PMP_ADAPTER)MiniportAdapterContext;

    for(PacketCount=0; PacketCount < NumberOfPackets; PacketCount++)
    {
        //
        // Check for a zero pointer
        //
        
        Packet = PacketArray[PacketCount];
        
        if(!Packet){
            ASSERTMSG("Packet pointer is NULL\n", FALSE);
            continue;
        }
        
        DEBUGP(MP_LOUD, ("Adapter %p, Packet %p\n", Adapter, Packet));
        
        Adapter->nPacketsArrived++;
        
        if(MP_IS_READY(Adapter) && MP_TEST_FLAG(Adapter, fMP_POST_WRITES))
        {
            //
            // Get a free TCB block 
            //
            pTCB = (PTCB) NdisInterlockedRemoveHeadList(
                         &Adapter->SendFreeList, 
                         &Adapter->SendLock);        
            if(pTCB == NULL)
            {            
                DEBUGP(MP_LOUD, ("Can't allocate a TCB......!\n")); 

                Status = NDIS_STATUS_PENDING;    

                //
                // Not able to get TCB block for this send. So queue
                // it for later transmission and return NDIS_STATUS_PENDING .
                //
                NdisInterlockedInsertTailList(
                    &Adapter->SendWaitList, 
                    (PLIST_ENTRY)&Packet->MiniportReserved[0], 
                    &Adapter->SendLock);
            }
            else
            {    
                NdisInterlockedIncrement(&Adapter->nBusySend);
                ASSERT(Adapter->nBusySend <= NIC_MAX_BUSY_SENDS);

                //
                // Copy the content of the packet to a TCB block
                // buffer. We need to do this because the target driver
                // is not capable of handling chained buffers.
                //
                if(NICCopyPacket(Adapter, pTCB, Packet)) 
                {
                    Adapter->nWritesPosted++;
                    //
                    // Post a asynchronouse write request.
                    //
                    ntStatus = NICPostWriteRequest(Adapter, pTCB);
                    
                    if ( !NT_SUCCESS ( ntStatus ) ) {
                        DEBUGP (MP_ERROR, ( "NICPostWriteRequest failed %x\n", ntStatus ));
                    } 
                    NT_STATUS_TO_NDIS_STATUS(ntStatus, &Status);
                                        
                }else {
                    DEBUGP(MP_ERROR, ("NICCopyPacket returned error\n"));
                    Status = NDIS_STATUS_FAILURE;                
                }
            }
        }

        if(!NT_SUCCESS(Status)) // yes, NT_SUCCESS macro can be used on NDIS_STATUS
        {

            DEBUGP(MP_LOUD, ("Calling NdisMSendComplete %x \n", Status));
            
            NDIS_SET_PACKET_STATUS(Packet, Status);

            NdisMSendComplete(
                Adapter->AdapterHandle,
                Packet,
                Status);

            if(pTCB && (NdisInterlockedDecrement(&pTCB->Ref) == 0))
            {
                NICFreeSendTCB(Adapter, pTCB);
            }
        }


    }

    DEBUGP(MP_TRACE, ("<--- MPSendPackets\n"));

    return;
}



NTSTATUS
NICPostWriteRequest(
    PMP_ADAPTER Adapter,
    PTCB    pTCB
    )
/*++

Routine Description:

    This routine posts a write IRP to the target device to send
    the network packet out to the device.
        
Arguments:

    Adapter    - pointer to the MP_ADAPTER structure
    PTCB    -  Pointer to the TCB block that contains the IRP and data.


Return Value:

    NT status code

--*/
{
    PIRP            irp = pTCB->Irp;
    PIO_STACK_LOCATION  nextStack;
    PDEVICE_OBJECT   TargetDeviceObject = Adapter->TargetDeviceObject;

    DEBUGP(MP_TRACE, ("--> NICPostWriteRequest\n"));

    // 
    // Obtain a pointer to the stack location of the first driver that will be
    // invoked.  This is where the function codes and the parameters are set.
    // 
    nextStack = IoGetNextIrpStackLocation( irp );
    nextStack->MajorFunction = IRP_MJ_WRITE;
    nextStack->Parameters.Write.Length = pTCB->ulSize;
    nextStack->FileObject = Adapter->FileObject;
    
    irp->MdlAddress = (PMDL) pTCB->Mdl;

    pTCB->IrpLock = IRPLOCK_CANCELABLE;
        
    IoSetCompletionRoutine(irp,
                   NICWriteRequestCompletion,
                   pTCB,
                   TRUE,
                   TRUE,
                   TRUE);

    //
    // We are making an asynchronous request, so we don't really care
    // about the return status of IoCallDriver.
    //
    (void) IoCallDriver(TargetDeviceObject, irp);

    DEBUGP(MP_TRACE, ("<-- NICPostWriteRequest\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
NICWriteRequestCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
/*++

Routine Description:

    Completion routine for the write request. This routine
    indicates to NDIS that send operation is complete and
    free the TCB if nobody has already done so. This routine
    also handles the case where another thread has canceled the 
    write request.
        
Arguments:

    DeviceObject    -  not used. Should be NULL
    Irp    -   Pointer to our IRP
    Context - pointer to our adapter context structure

Return Value:

    STATUS_MORE_PROCESSING_REQUIRED - because this is an asynchronouse IRP
    and we want to reuse it.
    
--*/
{
    PTCB    pTCB = (PTCB)Context;
    PMP_ADAPTER Adapter = pTCB->Adapter;
    PLIST_ENTRY       pEntry = NULL;
    PNDIS_PACKET      Packet = NULL;

    DEBUGP(MP_TRACE, ("--> NICWriteRequestCompletion\n"));
    
    if(NT_SUCCESS(Irp->IoStatus.Status)) {  
        
        DEBUGP (MP_VERY_LOUD, ("Wrote %d bytes\n", Irp->IoStatus.Information)); 
        Adapter->GoodTransmits++; 
        Adapter->nBytesWritten += (ULONG)Irp->IoStatus.Information;
        
    } else {
    
        DEBUGP (MP_ERROR, ("Write request failed %x\n", Irp->IoStatus.Status));        
        Adapter->nWritesCompletedWithError++;   
        //
        // Clear the flag to prevent any more writes from being
        // posted to the target device.
        //
        //MP_CLEAR_FLAG(Adapter, fMP_POST_WRITES);
       
    }

    //
    // Check to see if another thread has canceled this IRP.
    //
    if (InterlockedExchange((PVOID)&pTCB->IrpLock, IRPLOCK_COMPLETED) 
                    == IRPLOCK_CANCEL_STARTED) {
        // 
        // NICFreeBusySendPackets has got the control of the IRP. It will
        // now take the responsibility of freeing the TCB. 
        // Therefore...
        
        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    
    if(NdisInterlockedDecrement(&pTCB->Ref) == 0)
    {
        DEBUGP(MP_VERY_LOUD, ("Calling NdisMSendComplete \n"));

        Packet = pTCB->OrgSendPacket;

        //
        // For performance, let us free the TCB block before completing send.
        //
        NICFreeSendTCB(Adapter, pTCB);        

        NdisMSendComplete(
            Adapter->AdapterHandle,
            Packet,
            NDIS_STATUS_SUCCESS);
    
        //
        // Before we exit, since we have the control, let use see if there any 
        // more packets waiting in the queue to be sent.
        //
        
        pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                            &Adapter->SendWaitList, 
                            &Adapter->SendLock);
        if(pEntry)
        {
            Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);
            //
            // TODO: This can cause recursion if the target driver completes 
            // write requests synchronously. Use workitem to avoid recursion.
            //
            MPSendPackets(Adapter, &Packet, 1);             
        }
        
    }

    
    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
NICFreeBusySendPackets(
    PMP_ADAPTER Adapter
    )
/*++

Routine Description:

    This routine is called by the Halt handler to cancel any
    outstanding write requests.

    Note: Since we are holding the original sendpacket while
    writing to the device, and since NDIS calls halt
    handler only all the outstanding sends are completed, this
    routine will never find any busy sendpackets in the queue.
    This routine will be useful if you want to cancel busy
    sends outside of halt handler for some reason.
        
Arguments:

    Adapter - pointer to our adapter context structure

Return Value:
    
--*/
{ 
    PLIST_ENTRY         pEntry = NULL;
    PNDIS_PACKET        Packet = NULL;
    PTCB                pTCB = NULL;
    
    DEBUGP(MP_TRACE, ("--> NICFreeBusySendPackets\n"));

    if(!MP_TEST_FLAG(Adapter, fMP_SEND_SIDE_RESOURCE_ALLOCATED)){
        return;
    }        

    NdisAcquireSpinLock(&Adapter->SendLock);
    
    while(!IsListEmpty(&Adapter->SendBusyList))
    {
        pEntry = (PLIST_ENTRY)RemoveHeadList(&Adapter->SendBusyList);

        pTCB = CONTAINING_RECORD(pEntry, TCB, List);
        NdisInitializeListHead(&pTCB->List);

        NdisReleaseSpinLock(&Adapter->SendLock); 
        
        if (InterlockedExchange((PVOID)&pTCB->IrpLock, IRPLOCK_CANCEL_STARTED) 
                                        == IRPLOCK_CANCELABLE) {

            // 
            // we got it to the IRP before it was completed. We can cancel
            // the IRP without fear of losing it, as the completion routine
            // will not let go of the IRP until we say so.
            // 
            IoCancelIrp(pTCB->Irp);
            // 
            // Release the completion routine. If it already got there,
            // then we need to free it yourself. Otherwise, we got
            // through IoCancelIrp before the IRP completed entirely.
            // 
            if (InterlockedExchange((PVOID)&pTCB->IrpLock, IRPLOCK_CANCEL_COMPLETE)
                                        == IRPLOCK_COMPLETED) {
                                        
                if(NdisInterlockedDecrement(&pTCB->Ref) == 0) {
                    
                    DEBUGP(MP_VERY_LOUD, ("Calling NdisMSendComplete \n"));

                    NdisMSendComplete(Adapter->AdapterHandle,
                                        pTCB->OrgSendPacket,
                                        NDIS_STATUS_SUCCESS); 
                    //
                    // Initialize the head so that we don't barf in NICFreeSendTCB
                    // while doing RemoveEntryList.
                    //
                    NdisInitializeListHead(&pTCB->List);                    
                    
                    NICFreeSendTCB(Adapter, pTCB);        
                }else {
                    ASSERTMSG("Only we have the right to free TCB\n", FALSE);                
                }
            }

        }

        NdisAcquireSpinLock(&Adapter->SendLock);
        
    }

    NdisReleaseSpinLock(&Adapter->SendLock); 

    DEBUGP(MP_TRACE, ("<-- NICFreeBusySendPackets\n"));
     
    return ;
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
    pTCB->Mdl = NULL;
    
    ASSERT(pTCB->Irp);    
    ASSERT(!pTCB->Ref);

    //
    // Reinitialize the IRP for reuse.
    //
    IoReuseIrp(pTCB->Irp, STATUS_SUCCESS);    

    // 
    // Set the MDL field to NULL so that we don't end up free the
    // MDL in our call to IoFreeIrp.
    // 
      
    pTCB->Irp->MdlAddress = NULL;
    
    //
    // Re adjust the length to the originl size
    //
    NdisAdjustBufferLength(pTCB->Buffer, NIC_BUFFER_SIZE);

    //
    // Insert the TCB back in the send free list     
    //
    NdisAcquireSpinLock(&Adapter->SendLock);
    
    RemoveEntryList(&pTCB->List);
    
    InsertTailList(&Adapter->SendFreeList, &pTCB->List);

    NdisInterlockedDecrement(&Adapter->nBusySend);
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

    This routine is called by the Halt handler to fail all
    the queued up SendPackets because the device is either
    gone, being stopped for resource rebalance.
    
Arguments:

    Adapter    - pointer to the adapter structure

Return Value:

    VOID

--*/
{
    PLIST_ENTRY       pEntry = NULL;
    PNDIS_PACKET      Packet = NULL;

    DEBUGP(MP_TRACE, ("--> NICFreeQueuedSendPackets\n"));
 
    while(MP_TEST_FLAG(Adapter, fMP_SEND_SIDE_RESOURCE_ALLOCATED))
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

BOOLEAN
NICCopyPacket(
    PMP_ADAPTER Adapter,
    PTCB pTCB, 
    PNDIS_PACKET Packet)
/*++

Routine Description:

    This routine copies the packet data into the TCB data block and
    inserts the TCB in the SendBusyList.
        
Arguments:

    Adapter    - pointer to the MP_ADAPTER structure
    pTCB      - pointer to TCB block
    Packet    - packet to be transfered.


Return Value:

    VOID

--*/
{
    PNDIS_BUFFER   CurrentBuffer = NULL;
    PVOID          VirtualAddress = NULL;
    UINT           CurrentLength;
    UINT           BytesToCopy;
    UINT           BytesCopied = 0;
    UINT           PacketLength;      
    PUCHAR         pDest = pTCB->pData;
    BOOLEAN        bResult = TRUE, bPadding = FALSE;
    PETH_HEADER    pEthHeader;
    UINT           DestBufferSize = NIC_BUFFER_SIZE;        
        
    DEBUGP(MP_TRACE, ("--> NICCopyPacket\n"));

    //
    // Initialize TCB structure
    //
    pTCB->OrgSendPacket = Packet;
    pTCB->Ref = 1;
    NdisInitializeListHead(&pTCB->List);

    //
    // Query the packet to get length and pointer to the first buffer.
    //    
    NdisQueryPacket(Packet,
                    NULL,
                    NULL,
                    &CurrentBuffer,
                    &PacketLength);

    ASSERT(PacketLength <= NIC_BUFFER_SIZE);

    PacketLength = min(PacketLength, NIC_BUFFER_SIZE); 
    
    if(PacketLength < ETH_MIN_PACKET_SIZE)
    {
        PacketLength = ETH_MIN_PACKET_SIZE;   // padding
        bPadding = TRUE;
    }

    if(Adapter->TargetSupportsChainedMdls 
                    && bPadding == FALSE) {
        //
        // If the lower driver supports chained MDLs, we will 
        // assign the MDL to the request and send it down. However if
        // the packet is less than 60 bytes, we need to pad zero to
        // make it atleast 60bytes long. In that case we will just use
        // our preallocated flat buffer. If this driver is adapted for
        // a real device, the user should only preallocate 60 bytes TCB
        // Buffer instead of NIC_BUFFER_SIZE.
        //
        pTCB->Mdl = (PMDL)CurrentBuffer;
        pTCB->ulSize = PacketLength;
        Adapter->nBytesArrived += PacketLength;
        
    } else {

        BytesToCopy = PacketLength;
        
        //
        // Copy the data from chained buffers to our TCB flat buffer.
        //
        while(CurrentBuffer)
        {
            NdisQueryBufferSafe(
                CurrentBuffer,
                &VirtualAddress,
                &CurrentLength,
                NormalPagePriority);
            
            if(!VirtualAddress) {
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

            Adapter->nBytesArrived += BytesCopied;
            
            //
            // If the packet size is less than ETH_MIN_PACKET_SIZE. Pad the buffer
            // up to ETH_MIN_PACKET_SIZE with zero bytes for security reason.
            //
            if(BytesCopied < BytesToCopy)
            {
                NdisZeroMemory(pDest, BytesToCopy-BytesCopied);
                BytesCopied = BytesToCopy;
            }

            //
            // Adjust the buffer length to reflect the new length
            //
            NdisAdjustBufferLength(pTCB->Buffer, BytesCopied);       
            pTCB->ulSize = BytesCopied;

            pTCB->Mdl = pTCB->Buffer;
            
            if(PacketLength >= sizeof(ETH_HEADER)) {
                
                pEthHeader = (PETH_HEADER)pTCB->pData;
                
                //memcpy(pEthHeader->SrcAddr, Adapter->PhyNicMacAddress, 
                //                ETH_LENGTH_OF_ADDRESS);  
                
                DEBUGP(MP_LOUD, ("Src Address = %02x-%02x-%02x-%02x-%02x-%02x", 
                    pEthHeader->SrcAddr[0],
                    pEthHeader->SrcAddr[1],
                    pEthHeader->SrcAddr[2],
                    pEthHeader->SrcAddr[3],
                    pEthHeader->SrcAddr[4],
                    pEthHeader->SrcAddr[5]));

                DEBUGP(MP_LOUD, ("  Dest Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
                    pEthHeader->DstAddr[0],
                    pEthHeader->DstAddr[1],
                    pEthHeader->DstAddr[2],
                    pEthHeader->DstAddr[3],
                    pEthHeader->DstAddr[4],
                    pEthHeader->DstAddr[5]));
                    
            }else{
                ASSERTMSG("Packet length is less than ETH_HEADER\n", FALSE);
                bResult = FALSE;
            }
        }
    }
    
    if(bResult){
        
        NdisInterlockedInsertTailList(
                            &Adapter->SendBusyList, 
                            &pTCB->List, 
                            &Adapter->SendLock);

    }
    
    DEBUGP(MP_TRACE, ("<-- NICCopyPacket\n"));
    return bResult;
}
