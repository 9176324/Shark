/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY      
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    ISRDPC.C

Abstract:

    Contains routine to handle interrupts, interrupt DPCs and WatchDogTimer DPC

Environment:

    Kernel mode


Revision History:

    Eliyas Yakub Feb 13, 2003

--*/

#include "precomp.h"

#if defined(EVENT_TRACING)
#include "ISRDPC.tmh"
#endif

BOOLEAN 
NICInterruptHandler(
    IN PKINTERRUPT  Interupt, 
    IN PVOID        ServiceContext
    )
/*++
Routine Description:

    Interrupt handler for the device.

Arguments:

    Interupt - Address of the KINTERRUPT Object for our device.
    ServiceContext - Pointer to our adapter

Return Value:

     TRUE if our device is interrupting, FALSE otherwise.                                                    

--*/    
{
    BOOLEAN     InterruptRecognized = FALSE;
    PFDO_DATA   FdoData = (PFDO_DATA)ServiceContext;
    USHORT      IntStatus;

    DebugPrint(TRACE, DBG_INTERRUPT, "--> NICInterruptHandler\n");
    
    do 
    {
        //
        // If the adapter is in low power state, then it should not 
        // recognize any interrupt
        // 
        if (FdoData->DevicePowerState > PowerDeviceD0)
        {
            break;
        }
        //
        // We process the interrupt if it's not disabled and it's active                  
        //
        if (!NIC_INTERRUPT_DISABLED(FdoData) && NIC_INTERRUPT_ACTIVE(FdoData))
        {
            InterruptRecognized = TRUE;
        
            //
            // Disable the interrupt (will be re-enabled in NICDpcForIsr
            //
            NICDisableInterrupt(FdoData);
                
            //
            // Acknowledge the interrupt(s) and get the interrupt status
            //

            NIC_ACK_INTERRUPT(FdoData, IntStatus);

            DebugPrint(TRACE, DBG_INTERRUPT, "Requesting DPC\n");

            IoRequestDpc(FdoData->Self, NULL, FdoData);
            
        }
    }while (FALSE);    

    DebugPrint(TRACE, DBG_INTERRUPT, "<-- NICInterruptHandler\n");

    return InterruptRecognized;
}

VOID 
NICDpcForIsr(
    IN PKDPC            Dpc,
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp, //Unused
    IN PVOID            Context
    )

/*++

Routine Description:

    DPC callback for ISR. 

Arguments:

    DeviceObject - Pointer to the device object.

    Context - MiniportAdapterContext.

    Irp - Unused.

    Context - Pointer to FDO_DATA.

Return Value:

--*/
{
    PFDO_DATA fdoData = (PFDO_DATA) Context;

    DebugPrint(TRACE, DBG_DPC, "--> NICDpcForIsr\n");

    KeAcquireSpinLockAtDpcLevel(&fdoData->RcvLock);

    NICHandleRecvInterrupt(fdoData);

    KeReleaseSpinLockFromDpcLevel(&fdoData->RcvLock);

    //
    // Handle send interrupt    
    //
    KeAcquireSpinLockAtDpcLevel(&fdoData->SendLock);

    (VOID)NICHandleSendInterrupt(fdoData);

    KeReleaseSpinLockFromDpcLevel(&fdoData->SendLock);

    //
    // Start the receive unit if it had stopped
    //
    KeAcquireSpinLockAtDpcLevel(&fdoData->RcvLock);

    (VOID)NICStartRecv(fdoData);

    KeReleaseSpinLockFromDpcLevel(&fdoData->RcvLock);
    
    //
    // Re-enable the interrupt (disabled in MPIsr)
    //
    KeSynchronizeExecution(
        fdoData->Interrupt,
        (PKSYNCHRONIZE_ROUTINE)NICEnableInterrupt,
        fdoData);

    DebugPrint(TRACE, DBG_DPC, "<-- NICDpcForIsr\n");

}


VOID 
NICWatchDogTimerDpc(
    IN  PVOID	    SystemSpecific1,
    IN  PVOID	    FunctionContext,
    IN  PVOID	    SystemSpecific2, 
    IN  PVOID	    SystemSpecific3
    )
/*++

Routine Description:
    
    This DPC is used to do both link detection during hardware init and
    after that for hardware hang detection.
    
Arguments:

    SystemSpecific1     Not used
    FunctionContext     Pointer to our FdoData
    SystemSpecific2     Not used
    SystemSpecific3     Not used

Return Value:

    None
    
--*/
{
    PFDO_DATA           FdoData = (PFDO_DATA)FunctionContext;
    LARGE_INTEGER       DueTime;
    NTSTATUS            status = STATUS_SUCCESS;
    
    UNREFERENCED_PARAMETER(SystemSpecific1);
    UNREFERENCED_PARAMETER(SystemSpecific2);
    UNREFERENCED_PARAMETER(SystemSpecific3);

    DueTime.QuadPart = NIC_CHECK_FOR_HANG_DELAY;

    PciDrvIoIncrement(FdoData);

    if(MP_TEST_FLAG(FdoData, fMP_ADAPTER_HALT_IN_PROGRESS)){
        status = STATUS_DEVICE_REMOVED;
        goto Exit;
    }

    //
    // If the device has been power down, we will return without
    // touching the hardware. The timer will be restarted when
    // the device powers up.
    //
    if(FdoData->DevicePowerState == PowerDeviceD0)
    {
        if(!FdoData->CheckForHang){
            //
            // We are still doing link detection
            //            
            status = NICLinkDetection(FdoData);
            if(status == STATUS_PENDING) {
                // Wait for 100 ms   
                FdoData->bLinkDetectionWait = TRUE;
                DueTime.QuadPart = NIC_LINK_DETECTION_DELAY;
            }else {
                FdoData->CheckForHang = TRUE;
            }        
        }else {
            //
            // Link detection is over, let us check to see
            // if the hardware is hung.
            //
            if(NICCheckForHang(FdoData)){
                
                status = NICReset(FdoData);
                if(!NT_SUCCESS(status)){                    
                    goto Exit;
                }
            }
        }

        KeSetTimer( &FdoData->WatchDogTimer,   // Timer
                            DueTime,         // DueTime
                            &FdoData->WatchDogTimerDpc      // Dpc  
                            );           
     } else{
        goto Exit;
     }

     
     PciDrvIoDecrement(FdoData);
     return;
    
Exit:
    PciDrvIoDecrement(FdoData);  
    KeSetEvent(&FdoData->WatchDogTimerEvent, IO_NO_INCREMENT, FALSE);    
    DebugPrint(INFO, DBG_DPC, "WatchDogTimer is exiting %x\n", status);    
    return;
    
}


BOOLEAN 
NICCheckForHang(
    IN  PFDO_DATA     FdoData
    )
/*++

Routine Description:
    
    CheckForHang handler is called in the context of a timer DPC. 
    take advantage of this fact when acquiring/releasing spinlocks
    
Arguments:

    FdoData  Pointer to our adapter

Return Value:

    TRUE    This NIC needs a reset
    FALSE   Everything is fine

--*/
{
    PMP_TCB             pMpTcb;
    
    //
    // Just skip this part if the adapter is doing link detection
    //
    if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_LINK_DETECTION))
    {
        return(FALSE);   
    }

    //
    // any nonrecoverable hardware error?
    //
    if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_NON_RECOVER_ERROR))
    {
        DebugPrint(WARNING, DBG_DPC, "Non recoverable error - remove\n");
        return (TRUE);
    }
            
    //
    // hardware failure?
    //
    if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_HARDWARE_ERROR))
    {
        DebugPrint(WARNING, DBG_DPC, "hardware error - reset\n");
        return(TRUE);
    }
          
    //
    // Is send stuck?                  
    //
    
    KeAcquireSpinLockAtDpcLevel(&FdoData->SendLock);

    if (FdoData->nBusySend > 0)
    {
        pMpTcb = FdoData->CurrSendHead;
        pMpTcb->Count++;
        if (pMpTcb->Count > NIC_SEND_HANG_THRESHOLD)
        {
            KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);
            DebugPrint(WARNING, DBG_DPC, "Send is stuck - reset\n");
            return(TRUE);
        }
    }
    
    KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);

    KeAcquireSpinLockAtDpcLevel(&FdoData->RcvLock);

    //
    // Update the RFD shrink count                                          
    //
    if (FdoData->CurrNumRfd > FdoData->NumRfd)
    {
        FdoData->RfdShrinkCount++;          
    }

    KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);

    NICIndicateMediaState(FdoData);
    
    return(FALSE);
}

NTSTATUS 
NICReset(
    IN PFDO_DATA FdoData
    )
/*++

Routine Description:
    
    Function to reset the device.
    
Arguments:

    FdoData  Pointer to our adapter


Return Value:

    NT Status code.
    
Note:
    NICReset is called at DPC. Take advantage of this fact 
    when acquiring or releasing spinlocks
    
--*/
{
    NTSTATUS     status;    

    DebugPrint(TRACE, DBG_DPC, "---> MPReset\n");

    KeAcquireSpinLockAtDpcLevel(&FdoData->Lock);
    KeAcquireSpinLockAtDpcLevel(&FdoData->SendLock);
    KeAcquireSpinLockAtDpcLevel(&FdoData->RcvLock);

    do
    {
        ASSERT(!MP_TEST_FLAG(FdoData, fMP_ADAPTER_HALT_IN_PROGRESS));
  
        //
        // Is this adapter already doing a reset?
        //
        if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_RESET_IN_PROGRESS))
        {
            status = STATUS_SUCCESS;
            MP_EXIT;
        }

        MP_SET_FLAG(FdoData, fMP_ADAPTER_RESET_IN_PROGRESS);

        //
        // Is this adapter doing link detection?                                      
        //
        if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_LINK_DETECTION))
        {
            DebugPrint(WARNING, DBG_DPC, "Reset is pended...\n");
        
            //FdoData->bResetPending = TRUE;
            status = STATUS_SUCCESS;
            MP_EXIT;
        }
        //
        // Is this adapter going to be removed
        //
        if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_NON_RECOVER_ERROR))
        {
           status = STATUS_DEVICE_DATA_ERROR;
           if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_REMOVE_IN_PROGRESS))
           {
               MP_EXIT;
           }
           //                      
           // This is an unrecoverable hardware failure. 
           // We need to tell PNP system to remove this miniport
           //
           MP_SET_FLAG(FdoData, fMP_ADAPTER_REMOVE_IN_PROGRESS);
           MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_RESET_IN_PROGRESS);
           
           KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
           KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);
           KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);
           
           // TODO: Log an entry into the eventlog

           IoInvalidateDeviceState(FdoData->UnderlyingPDO);
                      
           DebugPrint(ERROR, DBG_DPC, "<--- MPReset, status=%x\n", status);
            
           return status;
        }   
                

        //
        // Disable the interrupt and issue a reset to the NIC
        //
        NICDisableInterrupt(FdoData);
        NICIssueSelectiveReset(FdoData);


        //
        // release all the locks and then acquire back the send lock
        // we are going to clean up the send queues
        // which may involve calling Ndis APIs
        // release all the locks before grabbing the send lock to
        // avoid deadlocks
        //
        KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
        KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);
        KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);
        
        KeAcquireSpinLockAtDpcLevel(&FdoData->SendLock);


        //
        // Free the packets on SendQueueList                                                           
        //
        NICFreeQueuedSendPackets(FdoData);

        //
        // Free the packets being actively sent & stopped
        //
        NICFreeBusySendPackets(FdoData);


        RtlZeroMemory(FdoData->MpTcbMem, FdoData->MpTcbMemSize);

        //
        // Re-initialize the send structures
        //
        NICInitSend(FdoData);
        
        KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);

        //
        // get all the locks again in the right order
        //
        KeAcquireSpinLockAtDpcLevel(&FdoData->Lock);
        KeAcquireSpinLockAtDpcLevel(&FdoData->SendLock);
        KeAcquireSpinLockAtDpcLevel(&FdoData->RcvLock);

        //
        // Reset the RFD list and re-start RU         
        //
        NICResetRecv(FdoData);
        status = NICStartRecv(FdoData);
        if (status != STATUS_SUCCESS) 
        {
            // Are we having failures in a few consecutive resets?                  
            if (FdoData->HwErrCount < NIC_HARDWARE_ERROR_THRESHOLD)
            {
                // It's not over the threshold yet, let it to continue
                FdoData->HwErrCount++;
            }
            else
            {
                //                      
                // This is an unrecoverable hardware failure. 
                // We need to tell PNP system to remove this miniport
                //
                MP_SET_FLAG(FdoData, fMP_ADAPTER_REMOVE_IN_PROGRESS);
                MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_RESET_IN_PROGRESS);
                
                KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
                KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);
                KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);
                
                // TODO: Log an entry into the eventlog
                     
                IoInvalidateDeviceState(FdoData->UnderlyingPDO);
                
                DebugPrint(ERROR, DBG_DPC, "<--- MPReset, status=%x\n", status);
                return(status);
            }
            
            break;
        }
        
        FdoData->HwErrCount = 0;
        MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_HARDWARE_ERROR);

        NICEnableInterrupt(FdoData);

    } while (FALSE);

    MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_RESET_IN_PROGRESS);

    exit:

    KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
    KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);
    KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);



    DebugPrint(TRACE, DBG_DPC, "<--- MPReset, status=%x\n", status);
    return(status);
}

NTSTATUS 
NICLinkDetection(
    PFDO_DATA         FdoData
    )
/*++

Routine Description:
    
    Timer function for postponed link negotiation. Called from
    the NICWatchDogTimerDpc. After the link detection is over
    we will complete any pending ioctl or send IRPs.
    
Arguments:

    FdoData     Pointer to our FdoData

Return Value:

    NT status
    
--*/
{
    NTSTATUS                status = STATUS_SUCCESS;
    MEDIA_STATE             currMediaState;
    PNDISPROT_QUERY_OID     pQuery = NULL;
    PNDISPROT_SET_OID       pSet = NULL;
    PVOID                   DataBuffer;    
    ULONG                   BytesWritten;
    NDIS_OID                oid;
    PIRP                    queryRequest = NULL;
    PIRP                    setRequest = NULL;
    PVOID                   informationBuffer;

    //
    // Handle the link negotiation.
    //
    if (FdoData->bLinkDetectionWait)
    {
        status = ScanAndSetupPhy(FdoData);
    }
    else
    {
        status = PhyDetect(FdoData);
    }
    
    if (status == STATUS_PENDING)
    {
        //
        // We are not done with link detection yet.
        // 
        return status;
    }

    //
    // Reset some variables for link detection
    //
    FdoData->bLinkDetectionWait = FALSE;
    
    DebugPrint(LOUD, DBG_DPC, "NICLinkDetection - negotiation done\n");

    KeAcquireSpinLockAtDpcLevel(&FdoData->Lock);
    MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_LINK_DETECTION);
    KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);

    //
    // Any OID query request pending?                                                        
    //
    if (FdoData->QueryRequest)
    {
        //
        // Clear the cancel routine.
        //
        queryRequest = FdoData->QueryRequest;
        
        if(IoSetCancelRoutine(queryRequest, NULL)){
            //
            // Cancel routine cannot run now and cannot have already 
            // started to run.
            //
            DataBuffer = queryRequest->AssociatedIrp.SystemBuffer;    
            pQuery = (PNDISPROT_QUERY_OID)DataBuffer;
            oid = pQuery->Oid;
            informationBuffer = &pQuery->Data[0];
           
            switch(pQuery->Oid)
            {
                case OID_GEN_LINK_SPEED:
                    *((PULONG)informationBuffer) = FdoData->usLinkSpeed * 10000;
                    BytesWritten = sizeof(ULONG);

                    break;

                case OID_GEN_MEDIA_CONNECT_STATUS:
                default:
                    ASSERT(oid == OID_GEN_MEDIA_CONNECT_STATUS);
                    
                    currMediaState = NICIndicateMediaState(FdoData);
                    
                    RtlMoveMemory(informationBuffer,
                                   &currMediaState,
                                   sizeof(NDIS_MEDIA_STATE));
                    
                    BytesWritten = sizeof(NDIS_MEDIA_STATE);
            }

            FdoData->QueryRequest = NULL;
            queryRequest->IoStatus.Information = sizeof(ULONG);
            queryRequest->IoStatus.Status = STATUS_SUCCESS;
            IoCompleteRequest(queryRequest, IO_NO_INCREMENT);
            PciDrvIoDecrement (FdoData);
        }
    }

    //
    // Any OID set request pending?                             
    //
    if (FdoData->SetRequest)
    {
        ULONG    PacketFilter; 

        setRequest = FdoData->SetRequest;
        if(IoSetCancelRoutine(setRequest, NULL)){
        
            DataBuffer = setRequest->AssociatedIrp.SystemBuffer;    
            pSet = (PNDISPROT_SET_OID)DataBuffer;
            oid = pSet->Oid;
            informationBuffer = &pSet->Data[0];

            if (oid == OID_GEN_CURRENT_PACKET_FILTER)
            {

                RtlMoveMemory(&PacketFilter, informationBuffer, sizeof(ULONG));

                KeAcquireSpinLockAtDpcLevel(&FdoData->Lock);

                status = NICSetPacketFilter(
                             FdoData,
                             PacketFilter);

                KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);
     
                if (status == STATUS_SUCCESS)
                {
                    FdoData->PacketFilter = PacketFilter;
                }
     
                FdoData->SetRequest = NULL;

                setRequest->IoStatus.Information = 0;
                setRequest->IoStatus.Status = STATUS_SUCCESS;
                IoCompleteRequest(setRequest, IO_NO_INCREMENT);
                PciDrvIoDecrement (FdoData);
            }
        }
    }

    //
    // Any read pending?
    //
    KeAcquireSpinLockAtDpcLevel(&FdoData->RcvLock);

    //
    // Start the NIC receive unit                                                     
    //
    status = NICStartRecv(FdoData);
    if (status != STATUS_SUCCESS)
    {
        MP_SET_HARDWARE_ERROR(FdoData);
    }
    
    KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);

    KeAcquireSpinLockAtDpcLevel(&FdoData->SendLock);

    //
    // Send packets which have been queued while link detection was going on. 
    //
    while (!IsListEmpty(&FdoData->SendQueueHead) &&
        MP_TCB_RESOURCES_AVAIABLE(FdoData))
    {
        PIRP        irp;
        PLIST_ENTRY pEntry;
        
        pEntry = RemoveHeadList(&FdoData->SendQueueHead); 
        
        ASSERT(pEntry);
        
        FdoData->nWaitSend--;

        irp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);

        DebugPrint(INFO, DBG_DPC, 
                        "NICLinkDetection - send a queued packet\n");

        NICWritePacket(FdoData, irp, TRUE);
    }

    KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);

    return status;    
}




