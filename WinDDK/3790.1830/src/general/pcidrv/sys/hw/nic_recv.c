/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY      
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:
    NIC_RECV.C

Abstract:
    This module contains miniport receive routines


Environment:

    Kernel mode


Revision History:
    DChen       11-01-99    created
    EliyasY     Feb 13, 2003 converted to WDM

--*/

#include "precomp.h"

#if defined(EVENT_TRACING)
#include "nic_recv.tmh"
#endif

VOID 
NICHandleRecvInterrupt(
    IN  PFDO_DATA  FdoData
    )
/*++
Routine Description:

    Interrupt handler for receive processing. Put the received packets 
    into an array and call NICServiceReadIrps. If we run low on 
    RFDs, allocate another one.
    
    Assumption: Rcv spinlock has been acquired 

Arguments:

    FdoData     Pointer to our FdoData

Return Value:

    None
    
--*/
{
    PMP_RFD         pMpRfd = NULL;
    PHW_RFD         pHwRfd = NULL;

    PMP_RFD         PacketArray[NIC_DEF_RFDS];              
    PMP_RFD         PacketFreeArray[NIC_DEF_RFDS];
    UINT            PacketArrayCount;
    UINT            PacketFreeCount;
    UINT            Index;
    UINT            LoopIndex = 0;
    UINT            LoopCount = NIC_MAX_RFDS / NIC_DEF_RFDS + 1;    // avoid staying here too long

    BOOLEAN         bContinue = TRUE;
    BOOLEAN         bAllocNewRfd = FALSE;
    USHORT          PacketStatus;
    
    DebugPrint(TRACE, DBG_READ, "---> NICHandleRecvInterrupt\n");

    ASSERT(FdoData->nReadyRecv >= NIC_MIN_RFDS);
    
    while (LoopIndex++ < LoopCount && bContinue)
    {
        PacketArrayCount = 0;
        PacketFreeCount = 0;

        //
        // Process up to the array size RFD's
        //
        while (PacketArrayCount < NIC_DEF_RFDS)
        {
            if (IsListEmpty(&FdoData->RecvList))
            {
                ASSERT(FdoData->nReadyRecv == 0);
                bContinue = FALSE;  
                break;
            }

            //
            // Get the next MP_RFD to process
            //
            pMpRfd = (PMP_RFD)GetListHeadEntry(&FdoData->RecvList);

            //
            // Get the associated HW_RFD
            //
            pHwRfd = pMpRfd->HwRfd;
            
            //
            // Is this packet completed?
            //
            PacketStatus = NIC_RFD_GET_STATUS(pHwRfd);
            if (!NIC_RFD_STATUS_COMPLETED(PacketStatus))
            {
                bContinue = FALSE;
                break;
            }

            //
            // HW specific - check if actual count field has been updated
            //
            if (!NIC_RFD_VALID_ACTUALCOUNT(pHwRfd))
            {
                bContinue = FALSE;
                break;
            }


            //
            // Remove the RFD from the head of the List
            //
            RemoveEntryList((PLIST_ENTRY)pMpRfd);
            FdoData->nReadyRecv--;
            ASSERT(FdoData->nReadyRecv >= 0);
            
            ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_RECV_READY));
            MP_CLEAR_FLAG(pMpRfd, fMP_RFD_RECV_READY);

            //
            // A good packet? drop it if not.
            //
            if (!NIC_RFD_STATUS_SUCCESS(PacketStatus))
            {
                DebugPrint(WARNING, DBG_READ, "Receive failure = %x\n", PacketStatus);
                NICReturnRFD(FdoData, pMpRfd);
                continue;
            }

            //
            // Do not receive any packets until a filter has been set
            //
            if (!FdoData->PacketFilter)
            {
                NICReturnRFD(FdoData, pMpRfd);
                continue;
            }

            //
            // Do not receive any packets until we are at D0
            //
            if (FdoData->DevicePowerState != PowerDeviceD0)
            {
                NICReturnRFD(FdoData, pMpRfd);
                continue;
            }

            pMpRfd->PacketSize = NIC_RFD_GET_PACKET_SIZE(pHwRfd);
            
            KeFlushIoBuffers(pMpRfd->Mdl, TRUE, TRUE); 

            //
            // set the status on the packet, either resources or success
            //
            if (FdoData->nReadyRecv >= MIN_NUM_RFD)
            {
                MP_SET_FLAG(pMpRfd, fMP_RFD_RECV_PEND);
                
                //InsertTailList(&FdoData->RecvPendList, (PLIST_ENTRY)pMpRfd); TODO:
                //MP_INC_RCV_REF(FdoData);

            }
            else
            {
                MP_SET_FLAG(pMpRfd, fMP_RFD_RESOURCES);
                
                PacketFreeArray[PacketFreeCount] = pMpRfd;
                PacketFreeCount++;

                //
                // Reset the RFD shrink count - don't attempt to shrink RFD
                //
                FdoData->RfdShrinkCount = 0;
                
                //
                // Remember to allocate a new RFD later
                //
                bAllocNewRfd = TRUE;
            }

            PacketArray[PacketArrayCount] = pMpRfd;
            PacketArrayCount++;
            //
            // Update the number of outstanding Recvs
            //
            PciDrvIoIncrement(FdoData);
            
        }

        //
        // if we didn't process any receives, just return from here
        //
        if (PacketArrayCount == 0) 
        {
            break;
        }
        
        KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
        KeAcquireSpinLockAtDpcLevel(&FdoData->Lock);
        //
        // If we have a Recv interrupt and have reported a media disconnect status
        // time to indicate the new status
        //

        if (Disconnected == FdoData->MediaState)
        {
            DebugPrint(WARNING, DBG_READ, "Media state changed to Connected\n");

            MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_NO_CABLE);

            FdoData->MediaState = Connected;
            
            KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);
            //
            // Indicate the media event
            //
            NICServiceIndicateStatusIrp(FdoData);
        }
    
        else
        {
            KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);
        }


        NICServiceReadIrps(
            FdoData,
            PacketArray,
            PacketArrayCount);

        //
        // Return all the RFDs to the pool.
        //
        KeAcquireSpinLockAtDpcLevel(&FdoData->RcvLock);

        for (Index = 0; Index < PacketFreeCount; Index++)
        {

            //
            // Get the MP_RFD saved in this packet, in NICAllocRfd
            //
            pMpRfd = PacketFreeArray[Index];
            
            ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_RESOURCES));
            MP_CLEAR_FLAG(pMpRfd, fMP_RFD_RESOURCES);

    
            NICReturnRFD(FdoData, pMpRfd);
        }

    }
    
    //
    // If we ran low on RFD's, we need to allocate a new RFD
    //
    if (bAllocNewRfd)
    {
        //
        // Allocate one more RFD only if it doesn't exceed the max RFD limit
        //
        if (FdoData->CurrNumRfd < FdoData->MaxNumRfd 
                    && !FdoData->AllocNewRfd)
        {
            NTSTATUS status;
            //
            // Increment the IO count to accout for the pending workitem. This
            // will prevent us from freeing resources during stop/remove until
            // the workitem callback is executed.
            //
            PciDrvIoIncrement(FdoData);
            FdoData->AllocNewRfd = TRUE;
            
            //
            // Since we are running at DISPATCH_LEVEL, we will queue a workitem
            // to allocate RFD memory at PASSIVE_LEVEL. Note that 
            // AllocateCommonBuffer and FreeCommonBuffer can be called only at
            // PASSIVE_LEVEL.
            //
            status = PciDrvQueuePassiveLevelCallback(FdoData, 
                                        NICAllocRfdWorkItem,
                                        NULL, NULL);
            if(!NT_SUCCESS(status)){
                PciDrvIoDecrement(FdoData);
                FdoData->AllocNewRfd = FALSE;                
            }
        }
    }

    ASSERT(FdoData->nReadyRecv >= NIC_MIN_RFDS);

    DebugPrint(TRACE, DBG_READ, "<--- NICHandleRecvInterrupt\n");
}

VOID 
NICReturnRFD(
    IN  PFDO_DATA FdoData,
    IN  PMP_RFD   pMpRfd
    )
/*++
Routine Description:

    Recycle a RFD and put it back onto the receive list 
    Assumption: Rcv spinlock has been acquired 

Arguments:

    FdoData     Pointer to our FdoData
    pMpRfd      Pointer to the RFD 

Return Value:

    None
    
--*/
{
    PMP_RFD   pLastMpRfd;
    PHW_RFD   pHwRfd = pMpRfd->HwRfd;

    ASSERT(pMpRfd->Flags == 0);
    MP_SET_FLAG(pMpRfd, fMP_RFD_RECV_READY);
    
    //
    // HW_SPECIFIC_START
    //
    pHwRfd->RfdCbHeader.CbStatus = 0;
    pHwRfd->RfdActualCount = 0;
    pHwRfd->RfdCbHeader.CbCommand = (RFD_EL_BIT);
    pHwRfd->RfdCbHeader.CbLinkPointer = DRIVER_NULL;

    // Append this RFD to the RFD chain
    if (!IsListEmpty(&FdoData->RecvList))
    {
        pLastMpRfd = (PMP_RFD)GetListTailEntry(&FdoData->RecvList);

        // Link it onto the end of the chain dynamically
        pHwRfd = pLastMpRfd->HwRfd;
        pHwRfd->RfdCbHeader.CbLinkPointer = pMpRfd->HwRfdPhys;
        pHwRfd->RfdCbHeader.CbCommand = 0;
    }

    //
    // HW_SPECIFIC_END
    //

    //
    // The processing on this RFD is done, so put it back on the tail of
    // our list
    //
    InsertTailList(&FdoData->RecvList, (PLIST_ENTRY)pMpRfd);
    FdoData->nReadyRecv++;
    ASSERT(FdoData->nReadyRecv <= FdoData->CurrNumRfd);
}

NTSTATUS 
NICStartRecv(
    IN  PFDO_DATA  FdoData
    )
/*++
Routine Description:

    Start the receive unit if it's not in a ready state                    
    Assumption: Rcv spinlock has been acquired 

Arguments:

    FdoData     Pointer to our FdoData

Return Value:

    NT Status code
    
--*/
{
    PMP_RFD         pMpRfd;
    NTSTATUS        status;

    DebugPrint(TRACE, DBG_READ, "---> NICStartRecv\n");

    //
    // If the receiver is ready, then don't try to restart.
    //
    if (NIC_IS_RECV_READY(FdoData))
    {
        DebugPrint(LOUD, DBG_READ, "Receive unit already active\n");
        return STATUS_SUCCESS;
    }

    DebugPrint(LOUD, DBG_READ, "Re-start receive unit...\n");
    ASSERT(!IsListEmpty(&FdoData->RecvList));
    
    //
    // Get the MP_RFD head
    //
    pMpRfd = (PMP_RFD)GetListHeadEntry(&FdoData->RecvList);

    //
    // If more packets are received, clean up RFD chain again
    //
    if (NIC_RFD_GET_STATUS(pMpRfd->HwRfd))
    {
        NICHandleRecvInterrupt(FdoData);
        ASSERT(!IsListEmpty(&FdoData->RecvList));

        //
        // Get the new MP_RFD head
        //
        pMpRfd = (PMP_RFD)GetListHeadEntry(&FdoData->RecvList);
    }

    //
    // Wait for the SCB to clear before we set the general pointer
    //
    if (!WaitScb(FdoData))
    {
        status = STATUS_DEVICE_DATA_ERROR;
        goto exit;
    }

    if (FdoData->DevicePowerState > PowerDeviceD0)
    {
        status = STATUS_DEVICE_DATA_ERROR;
        goto exit;
    }
    //
    // Set the SCB General Pointer to point the current Rfd
    //
    FdoData->CSRAddress->ScbGeneralPointer = pMpRfd->HwRfdPhys;

    //
    // Issue the SCB RU start command
    //
    status = D100IssueScbCommand(FdoData, SCB_RUC_START, FALSE);
    if (status == STATUS_SUCCESS)
    {
        // wait for the command to be accepted
        if (!WaitScb(FdoData))
        {
            status = STATUS_DEVICE_DATA_ERROR;
        }
    }        
    
    exit:

    DebugPrint(TRACE, DBG_READ, "<--- NICStartRecv, Status=%x\n", status);
    return status;
}



VOID 
NICResetRecv(
    IN  PFDO_DATA   FdoData
    )
/*++
Routine Description:

    Reset the receive list                    
    Assumption: Rcv spinlock has been acquired 
    
Arguments:

    FdoData     Pointer to our FdoData

Return Value:

     None

--*/
{
    PMP_RFD   pMpRfd;      
    PHW_RFD   pHwRfd;    
    LONG      RfdCount;

    DebugPrint(TRACE, DBG_READ, "--> NICResetRecv\n");

    ASSERT(!IsListEmpty(&FdoData->RecvList));
    
    //
    // Get the MP_RFD head
    //
    pMpRfd = (PMP_RFD)GetListHeadEntry(&FdoData->RecvList);
    for (RfdCount = 0; RfdCount < FdoData->nReadyRecv; RfdCount++)
    {
        pHwRfd = pMpRfd->HwRfd;
        pHwRfd->RfdCbHeader.CbStatus = 0;

        pMpRfd = (PMP_RFD)GetListFLink(&pMpRfd->List);
    }

    DebugPrint(TRACE, DBG_READ, "<-- NICResetRecv\n");
}


VOID
NICServiceReadIrps(
    PFDO_DATA   FdoData,
    PMP_RFD     *PacketArray,
    ULONG       PacketArrayCount
    )
/*++
Routine Description:

    Copy the data from the recv buffers to pending read IRP buffers
    and complete the IRP. When used as network driver, copy operation
    can be avoided by devising a private interface between us and the 
    NDIS-WDM filter and have the NDIS-WDM edge to indicate our buffers
    directly to NDIS.
    
Arguments:

    FdoData     Pointer to our FdoData

Return Value:

     None

--*/
{
    PIRP                irp;
    PLIST_ENTRY         listEntry;
    PMP_RFD             pMpRfd = NULL;
    ULONG               index, length;
    NTSTATUS            status;
    PVOID               buffer;
    
    DebugPrint(TRACE, DBG_READ, "--> NICServiceReadIrps\n");
    
    //
    // We need to dequeue all the entries in the queue, reset the cancel
    // routine for each of them and then process then:
    // - if the device is active, we will send them down
    // - else we will complete them with STATUS_NO_SUCH_DEVICE 
    //

    for(index=0; index < PacketArrayCount; index++)
    {
        pMpRfd = PacketArray[index];
        ASSERT(pMpRfd);
        
        //
        // Acquire the queue lock before manipulating the list entries.
        //
        KeAcquireSpinLockAtDpcLevel(&FdoData->RcvLock);

        while(TRUE){
            
            if(IsListEmpty(&FdoData->RecvQueueHead))
            {
                KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
                break;
            }

            //
            // Remove a request from the queue.
            //
            listEntry = RemoveHeadList(&FdoData->RecvQueueHead);

            irp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

            //
            // Set the cancel routine to NULL. This is an atomic operation.
            //
            if (IoSetCancelRoutine(irp, NULL))
            {
                //
                // Cancel routine cannot run now and cannot have already 
                // started to run.
                //
                KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
                //
                // Copy data from the RFD buffer to the IRP and complete
                // the IRP
                //
                buffer = MmGetSystemAddressForMdlSafe(irp->MdlAddress, NormalPagePriority);
                if(!buffer){
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    length = 0;
                } else {
                
                    length = min(MmGetMdlByteCount(irp->MdlAddress), pMpRfd->PacketSize);                    
                    RtlCopyMemory(buffer, pMpRfd->Buffer, length);
                    
                    DebugPrintEx((LOUD, DBG_READ, 
                                    "Received Packet Data: %!HEXDUMP!\n",
                                    log_xstr(buffer, (USHORT)length)));
                    
                    status = STATUS_SUCCESS;
                }

                irp->IoStatus.Status = status;
                irp->IoStatus.Information = length;
                IoCompleteRequest(irp, IO_NO_INCREMENT);
                PciDrvIoDecrement (FdoData);
                break;
            }
            else
            {
                //
                // Cancel rotuine is running. Leave the irp alone.
                //
                InitializeListHead(&irp->Tail.Overlay.ListEntry);
                irp = NULL;
                continue; 
            }
        }
        
        KeAcquireSpinLockAtDpcLevel(&FdoData->RcvLock);

        //
        // Decrement the number of outstanding Recvs
        //
        PciDrvIoDecrement (FdoData);        
        
        ASSERT(MP_TEST_FLAG(pMpRfd, fMP_RFD_RECV_PEND));
        MP_CLEAR_FLAG(pMpRfd, fMP_RFD_RECV_PEND);


        if (FdoData->RfdShrinkCount < NIC_RFD_SHRINK_THRESHOLD)
        {
            NICReturnRFD(FdoData, pMpRfd);
        }
        else
        {
            ASSERT(FdoData->CurrNumRfd > FdoData->NumRfd);
            //
            // Queue a passive level callback to free RFD memory.
            // Increment the IO count to accout for the pending workitem. This
            // will prevent us from freeing resources during stop/remove until
            // the workitem callback is executed.
            //
            PciDrvIoIncrement (FdoData);              
            status = PciDrvQueuePassiveLevelCallback(FdoData, 
                                    NICFreeRfdWorkItem, (PVOID)pMpRfd, 
                                    NULL);
            if(NT_SUCCESS(status)){
                
                FdoData->RfdShrinkCount = 0;
                FdoData->CurrNumRfd--;            
                DebugPrint(TRACE, DBG_READ, "Shrink... CurrNumRfd = %d\n", 
                                                    FdoData->CurrNumRfd);
            } else {
                PciDrvIoDecrement (FdoData); // Free the reference                 
                //
                // We couldn't queue a workitem to free memory, so let us
                // put that back in the main pool and try again next time.
                //
                NICReturnRFD(FdoData, pMpRfd);
            }
        }
        
        KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
            
    }// end of loop

    DebugPrint(TRACE, DBG_READ, "<-- NICServiceReadIrps\n");

    return;

}
