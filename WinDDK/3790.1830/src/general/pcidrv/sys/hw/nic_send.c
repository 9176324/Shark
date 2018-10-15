/*++
    
Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY      
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:
    nic_send.c

Abstract:
    This module contains routines to write packets.


Environment:

    Kernel mode


Revision History:
    DChen       11-01-99    created
    EliyasY     Feb 13, 2003 converted to WDM
    
--*/

#include "precomp.h"

#if defined(EVENT_TRACING)
#include "nic_send.tmh"
#endif


__inline VOID 
MP_FREE_SEND_PACKET(
    IN  PFDO_DATA   FdoData,
    IN  PMP_TCB     pMpTcb,
    IN  NTSTATUS    Status
    )
/*++
Routine Description:

    Recycle a MP_TCB and complete the packet if necessary
    Assumption: Send spinlock has been acquired 

Arguments:

    FdoData     Pointer to our FdoData
    pMpTcb      Pointer to MP_TCB        
    Status      Irp completion status

Return Value:

    None

--*/
{
    
    PIRP  Irp;

    ASSERT(MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE));

    Irp = pMpTcb->Irp;
    pMpTcb->Irp = NULL;
    pMpTcb->Count = 0;

    MP_CLEAR_FLAGS(pMpTcb);

    FdoData->CurrSendHead = FdoData->CurrSendHead->Next;
    FdoData->nBusySend--;
    ASSERT(FdoData->nBusySend >= 0);

    if (Irp)
    {
        KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);
        NICCompleteSendRequest(FdoData, 
                            Irp, 
                            Status, 
                            pMpTcb->PacketLength,
                            TRUE // Yes, we are calling at DISPATCH_LEVEL
                            );
        
        KeAcquireSpinLockAtDpcLevel(&FdoData->SendLock);
    }
}



NTSTATUS 
NICWrite(
    IN  PFDO_DATA     FdoData,
    IN  PIRP          Irp
)
/*++

Routine Description:

    This routine handles the hardware specific write request.
    If the device is not ready, fail the request. Otherwise
    get scatter-gather list for the request buffer and send the
    list to the hardware for DMA.
        
Arguments:

    FdoData - Pointer to the device context.
    Irp     - Pointer to the write request.
    
Return Value:

    NT Status code.
    
--*/
{
    NTSTATUS     returnStatus, status;   
    PVOID        virtualAddress;
    ULONG        pageCount = 0, length = 0;
    PMDL         tempMdl, mdl;
    KIRQL        oldIrql;
#if defined(DMA_VER2)       
    PVOID        sgListBuffer;
#endif

    DebugPrint(TRACE, DBG_WRITE, "--> PciDrvWrite %p\n", Irp);

    Irp->Tail.Overlay.DriverContext[3] = NULL;
    Irp->Tail.Overlay.DriverContext[2] = NULL;
    returnStatus = status = STATUS_SUCCESS;
    
    //
    // Is this adapter ready for sending?
    //
    if (MP_SHOULD_FAIL_SEND(FdoData))
    {        
        DebugPrint(ERROR, DBG_WRITE, "Device not ready %p\n", Irp);  
        returnStatus = status = STATUS_DEVICE_NOT_READY;
        goto Error;
    }

    tempMdl = mdl = Irp->MdlAddress;    

    //
    // Check for zero length buffer
    //
    if (mdl == NULL || MmGetMdlByteCount(mdl) == 0)
     {
        DebugPrint(ERROR, DBG_WRITE, "Zero length buffer %p\n", Irp);  
        status = returnStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto Error;
    }

    //
    // Calculate the total packet length and the number of pages
    // spanned by all the buffers by walking the MDL chain.
    // NOTE: If this driver is used in the miniport configuration, it will
    // not get chained MDLs because the upper filter (NDISEDGE.SYS)
    // coalesces the fragements to a single contiguous buffer before presenting
    // the packet to us.
    //
    while(tempMdl != NULL)                         
    {   
        virtualAddress = MmGetMdlVirtualAddress(tempMdl);           
        length += MmGetMdlByteCount(tempMdl);
        pageCount += ADDRESS_AND_SIZE_TO_SPAN_PAGES(virtualAddress, length);
        tempMdl = tempMdl->Next;
    }                                                                   
    
    if (length < NIC_MIN_PACKET_SIZE)
    {
        //
        // This will never happen in our case because the ndis-edge 
        // pads smaller size packets with zero to make it NIC_MIN_PACKET_SIZE
        // long.
        //
        DebugPrint(ERROR, DBG_WRITE, "Packet size is less than %d\n", NIC_MIN_PACKET_SIZE);     
        status = returnStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto Error;
    }

    //
    // Check to see if the packet spans more than the physical pages
    // our hardware can handle or the pageCount exceeds the total number of 
    // map registers allocated. If so, we should coalesce the scattered
    // buffers to fit the limit. We can't really break the transfers and
    // DMA in small chunks because each packets has to be DMA'ed in one shot.
    // The code on how to colesce the packet for this hardware is present 
    // in the original E100BEX sample.
    //        
    if (pageCount > NIC_MAX_PHYS_BUF_COUNT ||
            pageCount > FdoData->AllocatedMapRegisters)
    {
        // TODO: Packet needs to be coalesced
        DebugPrint(ERROR, DBG_WRITE, "Packet needs to be coalesced\n");        
        status = returnStatus = STATUS_INVALID_DEVICE_REQUEST;
        goto Error;
    }    
    //
    // Build a scatter-gather list of the packet buffer and send the packet. 
    //
    // If DMA_VER2 is not defined, use GetScatterGatherList. If the driver
    // is meant to work on XP and above, define DMA_VER2, so that you can
    // use BuildScatterGatherList.
    //
    // Since Build/GetScatterGatherList should be called at DISPATCH_LEVEL
    // let us raise the IRQL.
    //
    
    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

    //
    // Let us mark the IRP pending, because NICProcessSGList is an asynchronous
    // callback and we wouldn't know the status of the IRP. This IRP may either
    // get completed by the DPC handler after the DMA transfer or may 
    // get queued if we are low on resources. So the safest thing
    // to do for us here is return STATUS_PENDING irrespective of what happens
    // to the IRP. 
    //
    IoMarkIrpPending(Irp);
    returnStatus = STATUS_PENDING;
    
#if defined(DMA_VER2)    

    sgListBuffer = ExAllocateFromNPagedLookasideList(
                            &FdoData->SGListLookasideList);
    if (sgListBuffer)
    {
        Irp->Tail.Overlay.DriverContext[2] =  sgListBuffer;        
        status = FdoData->DmaAdapterObject->DmaOperations->BuildScatterGatherList(
                        FdoData->DmaAdapterObject,
                        FdoData->Self,
                        mdl,
                        MmGetMdlVirtualAddress(mdl),
                        length,
                        NICProcessSGList,
                        Irp,
                        TRUE,
                        sgListBuffer,
                        FdoData->ScatterGatherListSize);
        
        if (!NT_SUCCESS(status))
        {
            DebugPrint(ERROR, DBG_WRITE, "BuildScatterGatherList %x\n", status);
            ExFreeToNPagedLookasideList(&FdoData->SGListLookasideList, sgListBuffer);
            Irp->Tail.Overlay.DriverContext[2] =  NULL;
        }
    }
    
#else

    status = FdoData->DmaAdapterObject->DmaOperations->GetScatterGatherList(
                    FdoData->DmaAdapterObject,
                    FdoData->Self,
                    mdl,
                    MmGetMdlVirtualAddress(mdl),
                    length,
                    NICProcessSGList,
                    Irp,
                    TRUE);
    
    if (!NT_SUCCESS(status))
    {
        DebugPrint(ERROR, DBG_WRITE, "GetScatterGatherList %x\n", status);
    }
    
#endif
   
    KeLowerIrql(oldIrql);

Error:
    if(!NT_SUCCESS(status)){
        //
        // Our call to get the scatter-gather list failed. We know the
        // NICProcessSGList is not called for sure in that case. So let us
        // complete the IRP here with failure status. Since we marked the
        // IRP pending, we have no choice but to return status-pending
        // even though we are completing the IRP in the incoming thread
        // context.
        //
        NICCompleteSendRequest(FdoData, Irp, status, 0, FALSE);
    }

    DebugPrint(LOUD, DBG_WRITE, "<-- PciDrvWrite %x\n", returnStatus);

    return returnStatus;
}

VOID
NICProcessSGList(
    IN  PDEVICE_OBJECT          DeviceObject,
    IN  PIRP                    Irp,//unused
    IN  PSCATTER_GATHER_LIST    ScatterGather,
    IN  PVOID                   Context
    )
/*++

Routine Description:

    This routine is called at IRQL = DISPATCH_LEVEL when the 
    bus-master adapter is available. 

Arguments:

    DeviceObject - This is the device object for the target device, 
                    previously created by the driver's AddDevice routine. 
    Irp  - Useful only if the driver has a StartIo routine. 

    ScatterGather - Structure describing scatter/gather regions. 

    Context - Pointer to the Request.

Return Value:

    None.

--*/
{
    PFDO_DATA       fdoData;
    PIRP            irp = (PIRP)Context;
    
    DebugPrint(TRACE, DBG_WRITE, "--> NICProcessSGList\n");

    fdoData = DeviceObject->DeviceExtension;

    //
    // Save the ScatterGather pointer in the DriverContext so that we can free
    // the list when we complete the request.
    //
    irp->Tail.Overlay.DriverContext[3] = ScatterGather;
    
    KeAcquireSpinLockAtDpcLevel(&fdoData->SendLock);

    //
    // If tcb is not available or the device is doing link detection (during init),
    // queue the request
    //       
    if (!MP_TCB_RESOURCES_AVAIABLE(fdoData) || 
        MP_TEST_FLAG(fdoData, fMP_ADAPTER_LINK_DETECTION))
    {
        //
        // Instead of locking up the map registers while the request is
        // waiting in the queue or, we could free it up and reallocate it whenever
        // we are ready to handle the request later.
        // 
        //
        DebugPrint(TRACE, DBG_WRITE, "Resource or the link is not available, queue packet\n");
        InsertTailList(&fdoData->SendQueueHead, &irp->Tail.Overlay.ListEntry);
        fdoData->nWaitSend++;
        
    } else {

        NICWritePacket(fdoData, irp, FALSE);
    }
    
    KeReleaseSpinLockFromDpcLevel(&fdoData->SendLock);

    DebugPrint(TRACE, DBG_WRITE, "<-- NICProcessSGList\n");

    return;
}

VOID 
NICWritePacket(
    IN  PFDO_DATA   FdoData,
    IN  PIRP        Irp,
    IN  BOOLEAN     bFromQueue
    )
/*++
Routine Description:

    Do the work to send a packet
    Assumption: Send spinlock has been acquired 

Arguments:

    FdoData     Pointer to our FdoData
    Packet      The packet
    bFromQueue  TRUE if it's taken from the send wait queue

Return Value:

--*/
{
    PMP_TCB         pMpTcb = NULL;
    ULONG           packetLength;
    PVOID           virtualAddress;
        
    DebugPrint(TRACE, DBG_WRITE, "--> NICWritePacket, Irp= %p\n", Irp);
    
    //
    // Get the next free TCB and initialize it to represent the
    // request buffer.
    //
    pMpTcb = FdoData->CurrSendTail;
    ASSERT(!MP_TEST_FLAG(pMpTcb, fMP_TCB_IN_USE));

    //
    // If the adapter is not ready, fail the request.
    //
    if(MP_IS_NOT_READY(FdoData)) {
        MP_FREE_SEND_PACKET(FdoData, pMpTcb, STATUS_DEVICE_NOT_READY);
        return;
    }

    pMpTcb->FirstBuffer = Irp->MdlAddress;    
    virtualAddress = MmGetMdlVirtualAddress(Irp->MdlAddress);    
    pMpTcb->BufferCount = 1;
    pMpTcb->PacketLength = packetLength = MmGetMdlByteCount(Irp->MdlAddress);
    pMpTcb->PhysBufCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(virtualAddress, 
                                            packetLength);
    pMpTcb->Irp = Irp;
    MP_SET_FLAG(pMpTcb, fMP_TCB_IN_USE);

    //
    // Call the send handler, it only needs to deal with the frag list
    //
    NICSendPacket(FdoData, pMpTcb, Irp->Tail.Overlay.DriverContext[3]);

    FdoData->nBusySend++;
    ASSERT(FdoData->nBusySend <= FdoData->NumTcb);
    FdoData->CurrSendTail = FdoData->CurrSendTail->Next;

    DebugPrint(TRACE, DBG_WRITE, "<-- NICWritePacket\n");
    return;

}  

NTSTATUS 
NICSendPacket(
    IN  PFDO_DATA     FdoData,
    IN  PMP_TCB         pMpTcb,
    IN  PSCATTER_GATHER_LIST   pFragList
    )
/*++
Routine Description:

    NIC specific send handler
    Assumption: Send spinlock has been acquired 

Arguments:

    FdoData     Pointer to our FdoData
    pMpTcb      Pointer to MP_TCB
    pFragList   The pointer to the frag list to be filled

Return Value:

    NTSTATUS code

--*/
{
    NTSTATUS    status;
    ULONG       index;
    UCHAR       TbdCount = 0;

    PHW_TCB      pHwTcb = pMpTcb->HwTcb;
    PTBD_STRUC   pHwTbd = pMpTcb->HwTbd;

    DebugPrint(TRACE, DBG_WRITE, "--> NICSendPacket\n");

    for (index = 0; index < pFragList->NumberOfElements; index++)
    {
        if (pFragList->Elements[index].Length)
        {
            pHwTbd->TbdBufferAddress = pFragList->Elements[index].Address.LowPart;
            pHwTbd->TbdCount = pFragList->Elements[index].Length;

            pHwTbd++;                    
            TbdCount++;   
        }
    }

    pHwTcb->TxCbHeader.CbStatus = 0;
    pHwTcb->TxCbHeader.CbCommand = CB_S_BIT | CB_TRANSMIT | CB_TX_SF_BIT;

    pHwTcb->TxCbTbdPointer = pMpTcb->HwTbdPhys;
    pHwTcb->TxCbTbdNumber = TbdCount;
    pHwTcb->TxCbCount = 0;
    pHwTcb->TxCbThreshold = (UCHAR) FdoData->AiThreshold;

   
    status = NICStartSend(FdoData, pMpTcb);
    if(!NT_SUCCESS(status)){
        DebugPrint(ERROR, DBG_WRITE, "NICStartSend returned error %x\n", status);
    }
    
    DebugPrint(TRACE, DBG_WRITE, "<-- NICSendPacket\n");

    return status;
}

NTSTATUS 
NICStartSend(
    IN  PFDO_DATA  FdoData,
    IN  PMP_TCB    pMpTcb
    )
/*++
Routine Description:

    Issue a send command to the NIC
    Assumption: Send spinlock has been acquired 

Arguments:

    FdoData     Pointer to our FdoData
    pMpTcb      Pointer to MP_TCB

Return Value:

    NTSTATUS code

--*/
{
    NTSTATUS     status;

    DebugPrint(TRACE, DBG_WRITE, "--> NICStartSend\n");

    //
    // If the transmit unit is idle (very first transmit) then we must
    // setup the general pointer and issue a full CU-start
    //
    if (FdoData->TransmitIdle)
    {
        
        DebugPrint(TRACE, DBG_WRITE, "CU is idle -- First TCB added to Active List\n");

        //
        // Wait for the SCB to clear before we set the general pointer
        //
        if (!WaitScb(FdoData))
        {
            DebugPrint(ERROR, DBG_WRITE, "NICStartSend -- WaitScb returned error\n");
            status = STATUS_DEVICE_DATA_ERROR;
            goto exit;
        }

        //
        // Don't try to start the transmitter if the command unit is not
        // idle ((not idle) == (Cu-Suspended or Cu-Active)).
        //
        if ((FdoData->CSRAddress->ScbStatus & SCB_CUS_MASK) != SCB_CUS_IDLE)
        {
            DebugPrint(ERROR, DBG_WRITE, "FdoData = %p, CU Not IDLE\n", FdoData);
            MP_SET_HARDWARE_ERROR(FdoData);
            KeStallExecutionProcessor(25);
        }

        FdoData->CSRAddress->ScbGeneralPointer = pMpTcb->HwTcbPhys;

        status = D100IssueScbCommand(FdoData, SCB_CUC_START, FALSE);

        FdoData->TransmitIdle = FALSE;
        FdoData->ResumeWait = TRUE;
    }
    else
    {
        //
        // If the command unit has already been started, then append this
        // TCB onto the end of the transmit chain, and issue a CU-Resume.
        //
        DebugPrint(LOUD, DBG_WRITE, "adding TCB to Active chain\n");

        //
        // Clear the suspend bit on the previous packet.
        //
        pMpTcb->PrevHwTcb->TxCbHeader.CbCommand &= ~CB_S_BIT;

        //
        // Issue a CU-Resume command to the device.  We only need to do a
        // WaitScb if the last command was NOT a RESUME.
        //
        status = D100IssueScbCommand(FdoData, SCB_CUC_RESUME, FdoData->ResumeWait);
    }

    exit:
                      
    DebugPrint(TRACE, DBG_WRITE, "<-- NICStartSend\n");

    return status;
}

NTSTATUS 
NICHandleSendInterrupt(
    IN  PFDO_DATA  FdoData
    )
/*++
Routine Description:

    Interrupt handler for sending processing. Re-claim the send resources, 
    complete sends and get more to send from the send wait queue.
    
    Assumption: Send spinlock has been acquired 

Arguments:

    FdoData     Pointer to our FdoData

Return Value:

    NTSTATUS code

--*/
{
    NTSTATUS        status = STATUS_SUCCESS;
    PMP_TCB         pMpTcb;

#if DBG
    LONG            i;
#endif

    DebugPrint(TRACE, DBG_WRITE, "---> NICHandleSendInterrupt\n");

    //
    // Any packets being sent? Any packet waiting in the send queue?
    //
    if (FdoData->nBusySend == 0 &&
        IsListEmpty(&FdoData->SendQueueHead))
    {
        ASSERT(FdoData->CurrSendHead == FdoData->CurrSendTail);
        DebugPrint(TRACE, DBG_WRITE, "<--- NICHandleSendInterrupt\n");
        return status;
    }

    //
    // Check the first TCB on the send list
    //
    while (FdoData->nBusySend > 0)
    {

#if DBG
        pMpTcb = FdoData->CurrSendHead;
        for (i = 0; i < FdoData->nBusySend; i++)
        {
            pMpTcb = pMpTcb->Next;   
        }

        if (pMpTcb != FdoData->CurrSendTail)
        {
            DebugPrint(ERROR, DBG_WRITE, "nBusySend= %d\n", FdoData->nBusySend);
            DebugPrint(ERROR, DBG_WRITE, "CurrSendhead= %p\n", FdoData->CurrSendHead);
            DebugPrint(ERROR, DBG_WRITE, "CurrSendTail= %p\n", FdoData->CurrSendTail);
            ASSERT(FALSE);
        }
#endif      

        pMpTcb = FdoData->CurrSendHead;

        //
        // Is this TCB completed?
        //
        if (pMpTcb->HwTcb->TxCbHeader.CbStatus & CB_STATUS_COMPLETE)
        {
            //
            // Check if this is a multicast hw workaround packet
            //
            if ((pMpTcb->HwTcb->TxCbHeader.CbCommand & CB_CMD_MASK) != CB_MULTICAST)
            {
                MP_FREE_SEND_PACKET(FdoData, pMpTcb, STATUS_SUCCESS);
                
            } else {
                ASSERTMSG("Not sure what to do", FALSE);
            }
        }
        else
        {
            break;
        }
    }

    //
    // If we queued any transmits because we didn't have any TCBs earlier,
    // dequeue and send those packets now, as long as we have free TCBs.
    //
    while (!IsListEmpty(&FdoData->SendQueueHead) &&
        MP_TCB_RESOURCES_AVAIABLE(FdoData))
    {
        PIRP irp;
        PLIST_ENTRY pEntry; 
                   
        pEntry = RemoveHeadList(&FdoData->SendQueueHead); 
        
        ASSERT(pEntry);
        
        FdoData->nWaitSend--;

        irp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);
        DebugPrint(LOUD, DBG_WRITE, "NICHandleSendInterrupt - send a queued packet\n");
        
        NICWritePacket(FdoData, irp, TRUE);
    }

    DebugPrint(TRACE, DBG_WRITE, "<--- NICHandleSendInterrupt\n");
    return status;
}

VOID
NICCompleteSendRequest(
    IN PFDO_DATA   FdoData,
    IN PIRP        Irp,
    IN NTSTATUS    Status,
    IN ULONG       Information,
    IN BOOLEAN     AtDispatchLevel
    )
/*++
Routine Description:

    This routine complete the IRP and free all the resources associated
    with the IRP. This function can be called at passive or dispatch
    level. 
    
Arguments:

    FdoData - Pointer to the device context.
    Irp     - Pointer to the write request.
    Status  - Final completion status of the IRP
    Information - Value to be set in the information field of the IRP
    AtDispatchLevel - Raising IRQL is as expensive as checking the current
                IRQL level. So, this parameter enables us to know the caller's
                current IRQL level and avoid raising and lowering IRQL 
                unnecessarily.
    
Return Value:

    VOID
    
--*/
{
    PSCATTER_GATHER_LIST sgl = Irp->Tail.Overlay.DriverContext[3];
    PVOID    sglBuffer = Irp->Tail.Overlay.DriverContext[2];    
    KIRQL oldIrql;

    DebugPrint(TRACE, DBG_WRITE, "NICCompleteSendRequest, Pkt= %p Sgl %p\n", 
                            Irp,
                            sgl);        

    
    if(sgl){
        //
        // Since PutScatterGatherList must be called at DISPATCH_LEVEL,
        // raise the IRQL if we are not at that level. 
        //
        if(!AtDispatchLevel) {        
            KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
        }
        
        ASSERT(FdoData->DmaAdapterObject);
        FdoData->DmaAdapterObject->DmaOperations->PutScatterGatherList(
                                FdoData->DmaAdapterObject,
                                sgl,
                                TRUE);

        if(!AtDispatchLevel) {                
            KeLowerIrql(oldIrql);
        }
    }

    if(sglBuffer){
        ExFreeToNPagedLookasideList(&FdoData->SGListLookasideList, sglBuffer);
    }
    
    Irp->Tail.Overlay.DriverContext[3] = NULL;
    Irp->Tail.Overlay.DriverContext[2] = NULL;
    
    Irp->IoStatus.Information = Information;
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    PciDrvIoDecrement (FdoData);
    
}

VOID
NICFreeBusySendPackets(
    IN  PFDO_DATA  FdoData
    )
/*++
Routine Description:

    Free and complete the stopped active sends
    Assumption: Send spinlock has been acquired 
    
Arguments:

    FdoData     Pointer to our FdoData

Return Value:

     None

--*/
{
    PMP_TCB     pMpTcb;
    NTSTATUS    status = MP_GET_STATUS_FROM_FLAGS(FdoData);

    DebugPrint(TRACE, DBG_WRITE, "--> NICFreeBusySendPackets\n");

    //
    // Any packets being sent? Check the first TCB on the send list
    //
    while (FdoData->nBusySend > 0)
    {
        pMpTcb = FdoData->CurrSendHead;

        //
        // Is this TCB completed?
        //
        if ((pMpTcb->HwTcb->TxCbHeader.CbCommand & CB_CMD_MASK) != CB_MULTICAST)
        {
            MP_FREE_SEND_PACKET(FdoData, pMpTcb, status);
        }
        else
        {
            break;
        }
    }

    DebugPrint(TRACE, DBG_WRITE, "<-- NICFreeBusySendPackets\n");
}


VOID 
NICFreeQueuedSendPackets(
    IN  PFDO_DATA  FdoData
    )
/*++
Routine Description:

    Free and complete the pended sends on SendQueueHead
    Assumption: spinlock has been acquired 
    
Arguments:

    FdoData     Pointer to our FdoData

Return Value:

     None

--*/
{
    PLIST_ENTRY     pEntry;
    PIRP            irp;
    NTSTATUS        status = MP_GET_STATUS_FROM_FLAGS(FdoData);

    DebugPrint(TRACE, DBG_WRITE, "--> NICFreeQueuedSendPackets\n");

    while (!IsListEmpty(&FdoData->SendQueueHead))
    {
        pEntry = RemoveHeadList(&FdoData->SendQueueHead); 
        FdoData->nWaitSend--;
        KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);

        ASSERT(pEntry);
        irp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);
        
        NICCompleteSendRequest(FdoData, irp, status, 0, TRUE); 
        

        KeAcquireSpinLockAtDpcLevel(&FdoData->SendLock);
    }

    DebugPrint(TRACE, DBG_WRITE, "<-- NICFreeQueuedSendPackets\n");

}

