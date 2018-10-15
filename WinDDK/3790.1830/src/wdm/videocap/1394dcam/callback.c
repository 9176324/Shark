//===========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// PURPOSE.
//
// Copyright (c) 1996 - 2000  Microsoft Corporation.  All Rights Reserved.
//
//===========================================================================
/*++

Module Name:

    dcampkt.c

Abstract:

    This file contains code to handle callback from the bus/class driver.
    They might be running in DISPATCH level.

Author:   

    Yee J. Wu 15-Oct-97

Environment:

    Kernel mode only

Revision History:


--*/


#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "wdm.h"       // for DbgBreakPoint() defined in dbg.h
#include "dbg.h"
#include "dcamdef.h"
#include "dcampkt.h"
#include "sonydcam.h"
#include "capprop.h"


NTSTATUS
DCamToInitializeStateCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PDCAM_IO_CONTEXT pDCamIoContext
    )

/*++

Routine Description:

    Completion routine called after the device is initialize to a known state.

Arguments:

    DriverObject - Pointer to driver object created by system.

    pIrp - Irp that just completed

    pDCamIoContext - A structure that contain the context of this IO completion routine.

Return Value:

    None.

--*/

{
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    PIRB pIrb;

    if(!pDCamIoContext) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }


    pIrb = pDCamIoContext->pIrb;
    pDevExt = pDCamIoContext->pDevExt;
    
    DbgMsg2(("\'DCamToInitializeStateCompletionRoutine: completed DeviceState=%d; pIrp->IoStatus.Status=%x\n", 
        pDCamIoContext->DeviceState, pIrp->IoStatus.Status));

    // Free MDL
    if(pIrb->FunctionNumber == REQUEST_ASYNC_WRITE) {
        DbgMsg3(("DCamToInitializeStateCompletionRoutine: IoFreeMdl\n"));
        IoFreeMdl(pIrb->u.AsyncWrite.Mdl);
    }


    // CAUTION:
    //     Do we need to retry if the return is STATUS_TIMEOUT or invalid generation number ?
    //


    if(pIrp->IoStatus.Status != STATUS_SUCCESS) {
        ERROR_LOG(("DCamToInitializeStateCompletionRoutine: Status=%x != STATUS_SUCCESS; cannot restart its state.\n", pIrp->IoStatus.Status));

        if(pDCamIoContext->pSrb) {
            pDCamIoContext->pSrb->Status = STATUS_UNSUCCESSFUL;
            StreamClassStreamNotification(StreamRequestComplete, pDCamIoContext->pSrb->StreamObject, pDCamIoContext->pSrb);
        }
        DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);

        return STATUS_MORE_PROCESSING_REQUIRED;      
    }

    //
    // Done here if we are in STOP or PAUSE state;
    // else setting to RUN state.
    //
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;

    //
    // No stream is open, job is done.
    //
    if(!pStrmEx) {
        if(pDCamIoContext->pSrb) {
            pDCamIoContext->pSrb->Status = STATUS_SUCCESS;
            StreamClassStreamNotification(StreamRequestComplete, pDCamIoContext->pSrb->StreamObject, pDCamIoContext->pSrb);
        }
        return STATUS_MORE_PROCESSING_REQUIRED;      
    }

    switch(pStrmEx->KSStateFinal) {
    case KSSTATE_STOP:
    case KSSTATE_PAUSE:
        pStrmEx->KSState = pStrmEx->KSStateFinal;
        if(pDCamIoContext->pSrb) {
            pDCamIoContext->pSrb->Status = STATUS_SUCCESS;
            StreamClassStreamNotification(StreamRequestComplete, pDCamIoContext->pSrb->StreamObject, pDCamIoContext->pSrb);
        }
        DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);
        break;

    case KSSTATE_RUN:
        if(pDCamIoContext->pSrb) {
            pDCamIoContext->pSrb->Status = STATUS_SUCCESS;
            StreamClassStreamNotification(StreamRequestComplete, pDCamIoContext->pSrb->StreamObject, pDCamIoContext->pSrb);
        }

        // Restart the stream.
        DCamSetKSStateRUN(pDevExt, pDCamIoContext->pSrb);

        // Need pDCamIoContext->pSrb; so free it after DCamSetKSStateRUN().
        DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);
        break;
    }

    return STATUS_MORE_PROCESSING_REQUIRED;      
}

NTSTATUS
DCamSetKSStateInitialize(
    PDCAM_EXTENSION pDevExt
    )
/*++

Routine Description:

    Set KSSTATE to KSSTATE_RUN.

Arguments:

    pDevExt - 

Return Value:

    Nothing

--*/
{

    PSTREAMEX pStrmEx;
    PIRB pIrb;
    PIRP pIrp;
    PDCAM_IO_CONTEXT pDCamIoContext;
    PIO_STACK_LOCATION NextIrpStack;
    NTSTATUS Status;


    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;                  
    

    if(!DCamAllocateIrbIrpAndContext(&pDCamIoContext, &pIrb, &pIrp, pDevExt->BusDeviceObject->StackSize)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    } 



    //
    // Initialize the device to a known state 
    // may need to do this due to power down??
    //

    pDCamIoContext->DeviceState = DCAM_SET_INITIALIZE;  // Keep track of device state that we just set.
    pDCamIoContext->pDevExt     = pDevExt;
    pDCamIoContext->RegisterWorkArea.AsULONG = 0;
    pDCamIoContext->RegisterWorkArea.Initialize.Initialize = TRUE;
    pDCamIoContext->RegisterWorkArea.AsULONG = bswap(pDevExt->RegisterWorkArea.AsULONG);
    pIrb->FunctionNumber = REQUEST_ASYNC_WRITE;
    pIrb->Flags = 0;
    pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;
    pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =  
              pDevExt->BaseRegister + FIELDOFFSET(CAMERA_REGISTER_MAP, Initialize);
    pIrb->u.AsyncWrite.nNumberOfBytesToWrite = sizeof(ULONG);
    pIrb->u.AsyncWrite.nBlockSize = 0;
    pIrb->u.AsyncWrite.fulFlags = 0;
    InterlockedExchange(&pIrb->u.AsyncWrite.ulGeneration, pDevExt->CurrentGeneration);        
    pIrb->u.AsyncWrite.Mdl = 
        IoAllocateMdl(&pDCamIoContext->RegisterWorkArea, sizeof(ULONG), FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool(pIrb->u.AsyncWrite.Mdl);

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;
          
    IoSetCompletionRoutine(
        pIrp,
        DCamToInitializeStateCompletionRoutine,
        pDCamIoContext,
        TRUE,
        TRUE,
        TRUE
        );

    Status = 
        IoCallDriver(
            pDevExt->BusDeviceObject,
            pIrp
            );

    return STATUS_SUCCESS;
}


VOID
DCamBusResetWorkItem(
    IN DEVICE_OBJECT *DeviceObject,
    IN DCAM_EXTENSION *pDevExt
    )
/*++

Routine Description:

    This work item routine react to a busreset while there is an active stream.  It performs
    1. REQUEST_ISOCH_FREE_BANDWIDTH (Must be at PASSIVE_LEVEL) to free the Bandwidth structure
    2. REQUEST_ISOCH_ALLOCATE_CHANNEL (reclaim the original channel first, then any channel)
    3. REQUEST_ISOCH_ALLOCATE_BANDWIDTH
    4. If same channel and has sufficient BW to continue, done!

    Only if lost the original isoch channel: 

    5. REQUEST_ISOCH_ALLOCATE_RESOURCES
    6. Stop isoch and rEsubmit the attached buffers
    7. Restart isoch

--*/
{
    NTSTATUS ntStatus, ntStatusWait;
    STREAMEX *pStrmEx;
    IRB *pIrb = NULL;
    IRP *pIrp = NULL;
    ULONG ulChannel;
    HANDLE hResource;

    PAGED_CODE();

    DbgMsg1(( "%d:%s) %s enter\n", pDevExt->idxDev, pDevExt->pchVendorName, __FUNCTION__ ));

    pStrmEx = ( STREAMEX * ) pDevExt->pStrmEx;
    if( !pStrmEx ) goto Exit; // Stream has stopped!

    // Allocate 1394 IRB and IRP
    pIrb = ExAllocatePoolWithTag(NonPagedPool, sizeof(IRB), 'macd');
    if( !pIrb ) goto Exit;

    pIrp = IoAllocateIrp(pDevExt->BusDeviceObject->StackSize, FALSE);
    if( !pIrp ) {
        ExFreePool(pIrb);
        goto Exit;
    }

    RtlZeroMemory( pIrb, sizeof( IRB ) );

    //
    // Bus reset will free the bandwidth but not the bandwidth structure allocated by the lower driver                
    //

    if( pDevExt->hBandwidth ) {
        pIrb->FunctionNumber = REQUEST_ISOCH_FREE_BANDWIDTH;
        pIrb->Flags = 0;
        pIrb->u.IsochFreeBandwidth.hBandwidth = pDevExt->hBandwidth;
        ntStatus = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);       
        ASSERT( ntStatus == STATUS_SUCCESS || ntStatus == STATUS_INVALID_GENERATION );
        pDevExt->hBandwidth = NULL;
    }

    //
    // NO need to restart streaming if the device has been removed!
    //

    if( pDevExt->bDevRemoved ) goto Exit;


    //
    // Synchronize read request and other work item
    //

    ntStatusWait = KeWaitForSingleObject( &pStrmEx->hMutex, Executive, KernelMode, FALSE, 0 );     


    //
    // Reallocate bandwidth and channel, and resource if necessary.
    // IF THIS FAIL, we are consider illegally streaming, and need to STOP streaming.
    //

    ulChannel = pDevExt->IsochChannel;  // Cache it so we know if it is changed after the new allocation.
    hResource = pDevExt->hResource;

    ntStatus = DCamAllocateIsochResource( pDevExt, pIrb, FALSE );

    if( ntStatus ) {

        ERROR_LOG(( "%d:%s) Reclaim isoch resource failed! ntStatus %x; Treat as device removed.\n\n", 
            pDevExt->idxDev, pDevExt->pchVendorName, ntStatus));
        ASSERT( ntStatus == STATUS_SUCCESS && "Failed to allocate isoch resource after busret!" );

        //
        // No resource so let's treat this situation as
        // Device has been removed because there is no
        // way to restart this.
        // This will stop future SRB_READ until stream is STOP and RUN again.
        //

        pDevExt->bDevRemoved = TRUE;                   

        // 
        // Stop tranmission so it will not send data to the old channel,
        // which might be "owned" by other device.
        //

        if(pStrmEx->KSState == KSSTATE_RUN) {
            // Disable EnableISO
            DCamIsoEnable(pIrb, pDevExt, FALSE);
        }

        KeReleaseMutex(&pStrmEx->hMutex, FALSE); 
        goto Exit;
    }

    //
    // If channel number change due to bus reset, we must
    //    - continue to blocking incoming SRB_READ (with mutex)
    //    - if RUN state, stop transmission
    //    - detach all pending buffer(s)
    //    - free "stale" isoch resource
    //    - if RUN state, program device to use the new channel
    //    - if RUN state, restart transmission
    //

    if(pDevExt->IsochChannel != ISOCH_ANY_CHANNEL &&
       ulChannel != pDevExt->IsochChannel) {

        ERROR_LOG(( "%d:%s) Lost our channel so resubmit and the restart.\n\n", pDevExt->idxDev, pDevExt->pchVendorName ));

        // 
        // Stop tranmission on the channel we no longer own!
        //

        if( pStrmEx->KSState == KSSTATE_RUN ) {
            // Disable EnableISO
            DCamIsoEnable(pIrb, pDevExt, FALSE);
        }


        //
        // Detach pending packets using the hOldRources and reattached using the new hResource
        // Note: incoming SRB_READ is block right now.
        //       free old resource after all pending reads are detached.
        //

        if( pDevExt->PendingReadCount > 0 ) {

            ntStatus = DCamReSubmitPacket( hResource, pDevExt, pStrmEx, pDevExt->PendingReadCount );
        }


        //
        // Free "stale" isoch resource 
        //

        if( pDevExt->hResource != hResource ) {

            pIrb->FunctionNumber = REQUEST_ISOCH_FREE_RESOURCES;
            pIrb->Flags = 0;
            pIrb->u.IsochFreeResources.hResource = hResource;
            ntStatus = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);    
            if( ntStatus ) {
                ERROR_LOG(("Failed REQUEST_ISOCH_FREE_RESOURCES ntStatus %x\n", ntStatus));
                ASSERT(ntStatus == STATUS_SUCCESS);
            }    
        }

        //
        // Getting ready to accept callback
        //
        pDevExt->bStopIsochCallback = FALSE;
        
        //
        // Restore to its initial Streaming state
        // mainly, programming device.
        //

        DCamSetKSStateInitialize( pDevExt );                    
    }

    KeReleaseMutex(&pStrmEx->hMutex, FALSE);   

Exit:

    if( pIrb ) {
        ExFreePool( pIrb );
        pIrb = NULL;
    }

    if( pIrp ) {
        IoFreeIrp( pIrp );
        pIrp = NULL;
    }

    //
    // Decrement work item count and signal the event only if there is no more work item.
    //
    if( InterlockedDecrement( &pDevExt->PendingWorkItemCount ) == 0 )
        KeSetEvent( &pDevExt->PendingWorkItemEvent, 0, FALSE );

    ERROR_LOG(( "%d:%s) %s exit\n", pDevExt->idxDev, pDevExt->pchVendorName, __FUNCTION__ ));

    return;
}


VOID
DCamBusResetNotification(
    IN PVOID Context
    )
/*++

Routine Description:

    We receive this callback notification after a bus reset and if the device is still attached.
    This can happen when a new device is plugged in or an existing one is removed, or due to 
    awaken from sleep state. We will restore the device to its original streaming state by
    (1) Initialize the device to a known state and then 
    (2) launch a state machine to restart streaming.
    We will stop the state machine if previous state has failed.  This can happen if the generation 
    count is changed before the state mahcine is completed.
    
    The freeing and realocation of isoch bandwidth and channel are done in the bus reset irp.
    It is passed down by stream class in SRB_UNKNOWN_DEVICE_COMMAND. This IRP is guarantee to 
    call after this bus reset notification has returned and while the state machine is going on.   

    This is a callback at IRQL_DPC level; there are many 1394 APIs cannot be called at this level
    if it does blocking using KeWaitFor*Object().  Consult 1394 docuement for the list.

    
Arguments:

    Context - Pointer to the context of this registered notification.

Return Value:

    Nothing

--*/
{

    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) Context;
    PSTREAMEX pStrmEx;
    NTSTATUS Status;
    PIRP pIrp;
    PIRB pIrb;
    PIO_WORKITEM pIOWorkItem;

    
    if(!pDevExt) {
        ERROR_LOG(("DCamBusResetNotification:pDevExt is 0.\n\n"));  
        ASSERT(pDevExt);        
        return;
    }

    //
    // Check a field in the context that must be valid to make sure that it is Ok to continue. 
    //
    if(!pDevExt->BusDeviceObject) {
        ERROR_LOG(("DCamBusResetNotification:pDevExtBusDeviceObject is 0.\n\n"));  
        ASSERT(pDevExt->BusDeviceObject);        
        return;
    }  
    DbgMsg2(("DCamBusResetNotification: pDevExt %x, pDevExt->pStrmEx %x, pDevExt->BusDeviceObject %x\n", 
        pDevExt, pDevExt->pStrmEx, pDevExt->BusDeviceObject));

    //
    //
    // Get the current generation count first
    //
    // CAUTION: 
    //     not all 1394 APIs can be called in DCamSubmitIrpSynch() if in DISPATCH_LEVEL;
    //     Getting generation count require no blocking so it is OK.
    if(!DCamAllocateIrbAndIrp(&pIrb, &pIrp, pDevExt->BusDeviceObject->StackSize)) {
        ERROR_LOG(("DCamBusResetNotification: DcamAllocateIrbAndIrp has failed!!\n\n\n"));
        ASSERT(FALSE);            
        return;   
    } 

    pIrb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
    pIrb->Flags = 0;
    Status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);
    if(Status) {
        ERROR_LOG(("\'DCamBusResetNotification: Status=%x while trying to get generation number\n", Status));
        // Done with them; free resources.
        DCamFreeIrbIrpAndContext(0, pIrb, pIrp);
        return;
    }
    ERROR_LOG(("DCamBusResetNotification: Generation number from %d to %d\n", 
        pDevExt->CurrentGeneration, pIrb->u.GetGenerationCount.GenerationCount));

    InterlockedExchange(&pDevExt->CurrentGeneration, pIrb->u.GetGenerationCount.GenerationCount);


    // Done with them; free resources.
    DCamFreeIrbIrpAndContext(0, pIrb, pIrp);

    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;

    //
    // If the stream was open (pStrmEx != NULL && pStrmEx->pVideoInfoHeader != NULL),
    // then we need to restore its streaming state.
    //
    if (pStrmEx &&
        pStrmEx->pVideoInfoHeader != NULL) {
        DbgMsg2(("\'%d:%s) DCamBusResetNotification: Stream was open; Try allocate them again.\n", pDevExt->idxDev, pDevExt->pchVendorName));
    } else {
        DbgMsg2(("DCamBusResetNotification:Stream has not open on this device.  Done!\n"));
        return;
    }

   
    //
    // We will allow queuing only one work item!
    //
    if( pDevExt->PendingWorkItemCount ) 
        goto Exit;   

    //
    // Get here only because we have an active stream;
    // Start a work item to reclaim isoch resource as a result of busreset; some 1394 request
    // like free BW must be exeuted in PASSIVE level.
    //

    pIOWorkItem = IoAllocateWorkItem( pDevExt -> PhysicalDeviceObject );

    if( !pIOWorkItem ) {
        ERROR_LOG(("%s failed to alocate work item!\n", __FUNCTION__ ));

        // No more resource, there is nothing we can do.
        goto Exit; 
    }

    InterlockedIncrement( &pDevExt->PendingWorkItemCount );

    KeResetEvent( &pDevExt->PendingWorkItemEvent );  // Non-signal so the device removal can wait!

    // Make it critical as we need to reclaim resource ASAP (within 1 sec)
    IoQueueWorkItem( pIOWorkItem, DCamBusResetWorkItem, CriticalWorkQueue, pDevExt );

Exit:
    
    DbgMsg2(("\'DCamBusResetNotification: Leaving...; Task complete in the CompletionRoutine.\n"));

    return;
}


NTSTATUS
DCamDetachBufferCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PIRB pIrb
    )
/*++

Routine Description:

    Detaching a buffer has completed.  Attach next buffer.
    Returns more processing required so the IO Manager will leave us alone

Arguments:

    DriverObject - Pointer to driver object created by system.

    pIrp - Irp that just completed

    pIrb - Context set in DCamIsochCallback()

Return Value:

    None.

--*/

{
    IN PISOCH_DESCRIPTOR IsochDescriptor;
    PDCAM_EXTENSION pDevExt;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    KIRQL oldIrql;


    if(!pIrb) {
        ERROR_LOG(("\'DCamDetachBufferCR: pIrb is NULL\n"));
        ASSERT(pIrb);
        IoFreeIrp(pIrp);
        return (STATUS_MORE_PROCESSING_REQUIRED);
    }

    // Get IsochDescriptor from the context (pIrb)
    IsochDescriptor = pIrb->u.IsochDetachBuffers.pIsochDescriptor;
    if(!IsochDescriptor) {
        ERROR_LOG(("\'DCamDetachBufferCR: IsochDescriptor is NULL\n"));
        ASSERT(IsochDescriptor);
        IoFreeIrp(pIrp);
        return (STATUS_MORE_PROCESSING_REQUIRED);
    }

    if(pIrp->IoStatus.Status != STATUS_SUCCESS) {
        ERROR_LOG(("\'DCamDetachBufferCR: pIrp->IoStatus.Status(%x) != STATUS_SUCCESS\n", pIrp->IoStatus.Status));
        ASSERT(pIrp->IoStatus.Status == STATUS_SUCCESS);
        IoFreeIrp(pIrp);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }


    // IsochDescriptorReserved->Srb->Irp->IoStatus = pIrp->IoStatus.Status;
    IoFreeIrp(pIrp);

    // Freed and should not be referenced again!
    IsochDescriptor->DeviceReserved[5] = 0;

    IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) &IsochDescriptor->DeviceReserved[0];
    pDevExt = (PDCAM_EXTENSION) IsochDescriptor->Context1;
    DbgMsg3(("\'DCamDetachBufferCR: IsochDescriptorReserved=%x; DevExt=%x\n", IsochDescriptorReserved, pDevExt));   

    ASSERT(IsochDescriptorReserved);
    ASSERT(pDevExt);
     
    if(pDevExt &&
       IsochDescriptorReserved) {
        //
        // Indicate that the Srb should be complete
        //

        IsochDescriptorReserved->Flags |= STATE_SRB_IS_COMPLETE;
        IsochDescriptorReserved->Srb->Status = STATUS_SUCCESS;
        IsochDescriptorReserved->Srb->CommandData.DataBufferArray->DataUsed = IsochDescriptor->ulLength;
        IsochDescriptorReserved->Srb->ActualBytesTransferred = IsochDescriptor->ulLength;

        DbgMsg3(("\'DCamDetachBufferCR: Completing Srb %x\n", IsochDescriptorReserved->Srb));

        KeAcquireSpinLock(&pDevExt->IsochDescriptorLock, &oldIrql);
        RemoveEntryList(&IsochDescriptorReserved->DescriptorList);  InterlockedDecrement(&pDevExt->PendingReadCount);
        KeReleaseSpinLock(&pDevExt->IsochDescriptorLock, oldIrql);

        ASSERT(IsochDescriptorReserved->Srb->StreamObject);
        ASSERT(IsochDescriptorReserved->Srb->Flags & SRB_HW_FLAGS_STREAM_REQUEST);
        StreamClassStreamNotification(
               StreamRequestComplete, 
               IsochDescriptorReserved->Srb->StreamObject, 
               IsochDescriptorReserved->Srb);

        // Free it here instead of in DCamCompletionRoutine.
        ExFreePool(IsochDescriptor);     
    }    

    return (STATUS_MORE_PROCESSING_REQUIRED);
}



VOID
DCamIsochCallback(
    IN PDCAM_EXTENSION pDevExt,
    IN PISOCH_DESCRIPTOR IsochDescriptor
    )

/*++

Routine Description:

    Called when an Isoch Descriptor completes

Arguments:

    pDevExt - Pointer to our DeviceExtension

    IsochDescriptor - IsochDescriptor that completed

Return Value:

    Nothing

--*/

{
    PIRB pIrb;
    PIRP pIrp;
    PSTREAMEX pStrmEx;
    PIO_STACK_LOCATION NextIrpStack;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    PKSSTREAM_HEADER pDataPacket;
    PKS_FRAME_INFO pFrameInfo;
    KIRQL oldIrql;




    //
    // Debug check to make sure we're dealing with a real IsochDescriptor
    //

    ASSERT ( IsochDescriptor );
    IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) &IsochDescriptor->DeviceReserved[0];


    //
    // All Pending read will be either resubmitted, or cancelled (if out of resource).
    //

    if(pDevExt->bStopIsochCallback) {
        ERROR_LOG(("DCamIsochCallback: bStopCallback is set. IsochDescriptor %x (and Reserved %x) is returned and not processed.\n", 
            IsochDescriptor, IsochDescriptorReserved));
        return;
    }
    

    //
    // Synchronization note:
    //
    // We are competing with cancel packet routine in the 
    // event of device removal or setting to STOP state.
    // which ever got the spin lock to set DEATCH_BUFFER 
    // flag take ownership completing the Irp/IsochDescriptor.
    // 

    KeAcquireSpinLock(&pDevExt->IsochDescriptorLock, &oldIrql);
    if(pDevExt->bDevRemoved ||
       (IsochDescriptorReserved->Flags & (STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS)) ) {
        ERROR_LOG(("DCamIsochCallback: bDevRemoved || STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS %x %x\n", 
                IsochDescriptorReserved,IsochDescriptorReserved->Flags));
        ASSERT((!pDevExt->bDevRemoved && !(IsochDescriptorReserved->Flags & (STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS))));
        KeReleaseSpinLock(&pDevExt->IsochDescriptorLock, oldIrql);    
        return;   
    }
    IsochDescriptorReserved->Flags |= STATE_DETACHING_BUFFERS;        
    KeReleaseSpinLock(&pDevExt->IsochDescriptorLock, oldIrql);    


    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;

    ASSERT(pStrmEx == (PSTREAMEX)IsochDescriptorReserved->Srb->StreamObject->HwStreamExtension);

    pStrmEx->FrameCaptured++;
    pStrmEx->FrameInfo.PictureNumber = pStrmEx->FrameCaptured + pStrmEx->FrameInfo.DropCount;

    //
    // Return the timestamp for the frame
    //

    pDataPacket = IsochDescriptorReserved->Srb->CommandData.DataBufferArray;
    pFrameInfo = (PKS_FRAME_INFO) (pDataPacket + 1);

    ASSERT ( pDataPacket );
    ASSERT ( pFrameInfo );

    //
    // Return the timestamp for the frame
    //
    pDataPacket->PresentationTime.Numerator = 1;
    pDataPacket->PresentationTime.Denominator = 1;
    pDataPacket->Duration = pStrmEx->pVideoInfoHeader->AvgTimePerFrame;

    //
    // if we have a master clock
    // 
    if (pStrmEx->hMasterClock) {

        ULONGLONG tmStream;
                    
        tmGetStreamTime(IsochDescriptorReserved->Srb, pStrmEx, &tmStream);
        pDataPacket->PresentationTime.Time = tmStream;
        pDataPacket->OptionsFlags = 0;
          
        pDataPacket->OptionsFlags |= 
             KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
             KSSTREAM_HEADER_OPTIONSF_DURATIONVALID |
             KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;     // Every frame we generate is a key frame (aka SplicePoint)
               
        DbgMsg3(("\'IsochCallback: Time(%dms); P#(%d)=Cap(%d)+Drp(%d); Pend%d\n",
                (ULONG) tmStream/10000,
                (ULONG) pStrmEx->FrameInfo.PictureNumber,
                (ULONG) pStrmEx->FrameCaptured,
                (ULONG) pStrmEx->FrameInfo.DropCount,
                pDevExt->PendingReadCount));

    } else {

        pDataPacket->PresentationTime.Time = 0;
        pDataPacket->OptionsFlags &=                       
            ~(KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
            KSSTREAM_HEADER_OPTIONSF_DURATIONVALID);
    }

    // Set additional info fields about the data captured such as:
    //   Frames Captured
    //   Frames Dropped
    //   Field Polarity
                
    pStrmEx->FrameInfo.ExtendedHeaderSize = pFrameInfo->ExtendedHeaderSize;
    *pFrameInfo = pStrmEx->FrameInfo;

#ifdef SUPPORT_RGB24
    // Swaps B and R or BRG24 to RGB24.
    // There are 640x480 pixels so 307200 swaps are needed.
    if(pDevExt->CurrentModeIndex == VMODE4_RGB24 && pStrmEx->pVideoInfoHeader) {
        PBYTE pbFrameBuffer;
        BYTE bTemp;
        ULONG i, ulLen;

#ifdef USE_WDM110   // Win2000
        // Driver verifier flag to use this but if this is used, this driver will not load for any Win9x OS.
        pbFrameBuffer = (PBYTE) MmGetSystemAddressForMdlSafe(IsochDescriptorReserved->Srb->Irp->MdlAddress, NormalPagePriority);
#else    // Win9x
        pbFrameBuffer = (PBYTE) MmGetSystemAddressForMdl    (IsochDescriptorReserved->Srb->Irp->MdlAddress);
#endif
        if(pbFrameBuffer) {
            // calculate number of pixels
            ulLen = abs(pStrmEx->pVideoInfoHeader->bmiHeader.biWidth) * abs(pStrmEx->pVideoInfoHeader->bmiHeader.biHeight);
            ASSERT(ulLen == pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage/3);
            if(ulLen > pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage)
                ulLen = pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage/3;

            for (i=0; i < ulLen; i++) {
                // swap R and B
                bTemp = pbFrameBuffer[0];
                pbFrameBuffer[0] = pbFrameBuffer[2];
                pbFrameBuffer[2] = bTemp;
                pbFrameBuffer += 3;  // next RGB24 pixel
            }
        }
    }
#endif

    // Reuse the Irp and Irb
    pIrp = (PIRP) IsochDescriptor->DeviceReserved[5];
    ASSERT(pIrp);            

    pIrb = (PIRB) IsochDescriptor->DeviceReserved[6];
    ASSERT(pIrb);            

#if DBG
    // Same isochdescriptor should only be callback once.    
    ASSERT((IsochDescriptor->DeviceReserved[7] == 0x87654321));
    IsochDescriptor->DeviceReserved[7]++;
#endif

    pIrb->FunctionNumber = REQUEST_ISOCH_DETACH_BUFFERS;
    pIrb->u.IsochDetachBuffers.hResource = pDevExt->hResource;
    pIrb->u.IsochDetachBuffers.nNumberOfDescriptors = 1;
    pIrb->u.IsochDetachBuffers.pIsochDescriptor = IsochDescriptor;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;

    IoSetCompletionRoutine(
        pIrp,
        DCamDetachBufferCR,  // Detach complete and will attach queued buffer.
        pIrb,
        TRUE,
        TRUE,
        TRUE
        );
          
    IoCallDriver(pDevExt->BusDeviceObject, pIrp);
            
}

