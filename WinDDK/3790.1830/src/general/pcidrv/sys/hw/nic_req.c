/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY      
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:
    mp_req.c

Abstract:
    This module handle NDIS OID ioctls. This module is not required
    if the upper edge is not NDIS.


Environment:

    Kernel mode


Revision History:
    DChen       11-01-99    created
    EliyasY     Feb 13, 2003 converted to WDM
    
--*/

#include "precomp.h"

#if defined(EVENT_TRACING)
#include "nic_req.tmh"
#endif

//
// Following status values are copied from NDIS.H
//
#define NDIS_STATUS_MEDIA_CONNECT       0x4001000BL
#define NDIS_STATUS_MEDIA_DISCONNECT    0x4001000CL

NTSTATUS
PciDrvPowerBeginQueuingIrps(
    IN  PDEVICE_OBJECT      DeviceObject,
    IN  ULONG               IrpIoCharges,
    IN  BOOLEAN             Query
    );

NTSTATUS
NICHandleQueryOidRequest(
    IN  PFDO_DATA                   FdoData,
    IN  PIRP                        Irp,
    OUT PULONG                      BytesWritten
    )
/*++

Routine Description:

    Query an arbitrary OID value from the miniport.

Arguments:

    FdoData - pointer to open context representing our binding to the miniport
    DataBuffer - place to store the returned value
    BufferLength - length of the above
    BytesWritten - place to return length returned

Return Value:


--*/
{
    NTSTATUS                status = STATUS_SUCCESS;
    PNDISPROT_QUERY_OID     pQuery;
    NDIS_OID                Oid;
    ULONG                   ulInfo = 0;
    ULONG64                 ul64Info = 0;    
    PVOID                   pInfo = (PVOID) &ulInfo;
    ULONG                   ulInfoLen = sizeof(ulInfo);
    PVOID                   InformationBuffer = NULL;
    ULONG                   InformationBufferLength = 0;
    MEDIA_STATE             CurrMediaState;
    ULONG                   ulBytesAvailable = ulInfoLen;
    PVOID                   DataBuffer;    
    ULONG                   BufferLength;
    PIO_STACK_LOCATION      pIrpSp;
    KIRQL                   oldIrql;
    NDIS_PNP_CAPABILITIES   Power_Management_Capabilities;
    
    pIrpSp = IoGetCurrentIrpStackLocation(Irp);
    DataBuffer = Irp->AssociatedIrp.SystemBuffer;
    BufferLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;   
    
    DebugPrint(TRACE, DBG_IOCTLS, "--> HandleQueryOIDRequest\n");
    Oid = 0;

    do
    {
        if (BufferLength < sizeof(NDISPROT_QUERY_OID))
        {
            status = STATUS_BUFFER_OVERFLOW;
            break;
        }

        pQuery = (PNDISPROT_QUERY_OID)DataBuffer;
        Oid = pQuery->Oid;
        InformationBuffer = &pQuery->Data[0];
        InformationBufferLength = BufferLength - 
                            FIELD_OFFSET(NDISPROT_QUERY_OID, Data);
        
        switch(Oid)
        {

        case OID_GEN_LINK_SPEED:
        case OID_GEN_MEDIA_CONNECT_STATUS:
            
            if (InformationBufferLength < ulInfoLen)
            {
                break;
            }
            
            KeAcquireSpinLock(&FdoData->Lock, &oldIrql);
            if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_LINK_DETECTION))
            {
                //
                // The device is busy doing link detection. Let us queue
                // the IRP. When the link detection is over, the watchdog
                // timer DPC will complete this IRP.
                //
                ASSERT(!FdoData->QueryRequest);
                status = PciDrvQueueIoctlIrp(FdoData, Irp);
                KeReleaseSpinLock(&FdoData->Lock, oldIrql);
                break;
            }
            else
            {            
                KeReleaseSpinLock(&FdoData->Lock, oldIrql); 
                if (Oid == OID_GEN_LINK_SPEED)
                {
                    ulInfo = FdoData->usLinkSpeed * 10000;
                } else {
             
                    CurrMediaState = NICIndicateMediaState(FdoData);            
                    ulInfo = CurrMediaState;            
                }
            }
            break;

        case OID_802_3_PERMANENT_ADDRESS:
            pInfo = FdoData->PermanentAddress;
            ulBytesAvailable = ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_CURRENT_ADDRESS:
            pInfo = FdoData->CurrentAddress;
            ulBytesAvailable = ulInfoLen = ETH_LENGTH_OF_ADDRESS;
            break;

        case OID_802_3_MAXIMUM_LIST_SIZE:
            ulInfo = NIC_MAX_MCAST_LIST;
            break;
            
        case OID_GEN_XMIT_OK:
        case OID_GEN_RCV_OK:
        case OID_GEN_XMIT_ERROR:
        case OID_GEN_RCV_ERROR:
        case OID_GEN_RCV_NO_BUFFER:
        case OID_GEN_RCV_CRC_ERROR:
        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
        case OID_802_3_RCV_ERROR_ALIGNMENT:
        case OID_802_3_XMIT_ONE_COLLISION:
        case OID_802_3_XMIT_MORE_COLLISIONS:
        case OID_802_3_XMIT_DEFERRED:
        case OID_802_3_XMIT_MAX_COLLISIONS:
        case OID_802_3_RCV_OVERRUN:
        case OID_802_3_XMIT_UNDERRUN:
        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
        case OID_802_3_XMIT_TIMES_CRS_LOST:
        case OID_802_3_XMIT_LATE_COLLISIONS:
            
            status = NICGetStatsCounters(FdoData, Oid, &ul64Info);
            ulBytesAvailable = ulInfoLen = sizeof(ul64Info);
            
            if (status == STATUS_SUCCESS)
            {
                if (InformationBufferLength < sizeof(ULONG))
                {
                    status = STATUS_BUFFER_TOO_SMALL;
                    *BytesWritten = ulBytesAvailable;
                    break;
                }

                ulInfoLen = MIN(InformationBufferLength, ulBytesAvailable);
                pInfo = &ul64Info;
            }
                    
            break;         

        case OID_PNP_CAPABILITIES:

            NICFillPoMgmtCaps (FdoData, 
                                &Power_Management_Capabilities, 
                                &status,
                                &ulInfoLen);
            if (status == STATUS_SUCCESS)
            {
                pInfo = (PVOID) &Power_Management_Capabilities;
            }
            else
            {
                pInfo = NULL;
            }

            break;

        case OID_PNP_QUERY_POWER:
            // status is pre-set in this routine to Success

            status = STATUS_SUCCESS; 

            break;
            
        default:
            status = STATUS_NOT_SUPPORTED;
            break;
        }
    }while (FALSE);

    if (status == STATUS_SUCCESS)
    {
        if (ulInfoLen <= InformationBufferLength)
        {
            //
            // Copy result into InformationBuffer
            //
            *BytesWritten = ulInfoLen;
            if (ulInfoLen)
            {
                RtlMoveMemory(InformationBuffer, pInfo, ulInfoLen);
            }
        }
        else
        {
            //
            // too short
            //
            *BytesWritten = ulInfoLen;
            status = STATUS_BUFFER_TOO_SMALL;
        }
    }
    
    DebugPrint(LOUD, DBG_IOCTLS, "<--HandleQueryOIDRequest: OID %x, Status %x\n",
                Oid, status);

    return (status);
    
}

NTSTATUS
NICHandleSetOidRequest(
    IN  PFDO_DATA                   FdoData,
    IN  PIRP                       Irp
    )
/*++

Routine Description:

    This routine handles ioctl request for Set OIDs. Most of
    the IOCTL requests are only if the upper edge is NDIS.

Arguments:


Return Value:


--*/
{
    NTSTATUS                status = STATUS_SUCCESS;
    PNDISPROT_SET_OID       pSet;
    NDIS_OID                Oid;
    ULONG                   PacketFilter;
    PVOID                   InformationBuffer = NULL;
    ULONG                   InformationBufferLength = 0;
    KIRQL                   oldIrql;
    PVOID                   DataBuffer;    
    ULONG                   BufferLength, unUsed;
    PIO_STACK_LOCATION      pIrpSp;
    DEVICE_POWER_STATE      newDeviceState, oldDeviceState;
    
    pIrpSp = IoGetCurrentIrpStackLocation(Irp);
    DataBuffer = Irp->AssociatedIrp.SystemBuffer;
    BufferLength = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;   
 
    DebugPrint(LOUD, DBG_IOCTLS, "--> HandleSetOIDRequest\n");
    
    Oid = 0;

    do
    {
        if (BufferLength < sizeof(NDISPROT_SET_OID))
        {
            status = STATUS_BUFFER_OVERFLOW;
            break;
        }

        pSet = (PNDISPROT_SET_OID)DataBuffer;
        Oid = pSet->Oid;
        InformationBuffer = &pSet->Data[0];
        InformationBufferLength = BufferLength - FIELD_OFFSET(NDISPROT_SET_OID, Data);
        switch(Oid)
        {

        case OID_802_3_MULTICAST_LIST:
            //
            // Verify the length
            //
            if (InformationBufferLength % ETH_LENGTH_OF_ADDRESS != 0)
            {
                return(STATUS_INVALID_BUFFER_SIZE);
            }

            //
            // Save the number of MC list size
            //
            FdoData->MCAddressCount = InformationBufferLength / ETH_LENGTH_OF_ADDRESS;
            ASSERT(FdoData->MCAddressCount <= NIC_MAX_MCAST_LIST);

            //
            // Save the MC list
            //
            RtlMoveMemory(
                FdoData->MCList, 
                InformationBuffer, 
                InformationBufferLength);

            KeAcquireSpinLock(&FdoData->Lock, &oldIrql);
            KeAcquireSpinLockAtDpcLevel(&FdoData->RcvLock);
            
            status = NICSetMulticastList(FdoData);

            KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
            KeReleaseSpinLock(&FdoData->Lock, oldIrql);
            break;

        case OID_GEN_CURRENT_PACKET_FILTER:
            //
            // Verify the Length
            //
            if (InformationBufferLength != sizeof(ULONG))
            {
                return(STATUS_INVALID_BUFFER_SIZE);
            }
            
            RtlMoveMemory(&PacketFilter, InformationBuffer, sizeof(ULONG));

            //
            // any bits not supported?
            //
            if (PacketFilter & ~NIC_SUPPORTED_FILTERS)
            {
                return(STATUS_NOT_SUPPORTED);
            }

            //
            // any filtering changes?
            //
            if (PacketFilter == FdoData->PacketFilter)
            {
                break;
            }

            KeAcquireSpinLock(&FdoData->Lock, &oldIrql);
            KeAcquireSpinLockAtDpcLevel(&FdoData->RcvLock);
            
            if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_LINK_DETECTION))
            {
            
                //
                // The device is busy doing link detection. Let us queue
                // the IRP. When the link detection is over, the watchdog
                // timer DPC will complete this IRP.
                //
                ASSERT(!FdoData->SetRequest);
                status = PciDrvQueueIoctlIrp(FdoData, Irp);

                KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
                KeReleaseSpinLock(&FdoData->Lock, oldIrql);
                break;
            }

            status = NICSetPacketFilter(
                         FdoData,
                         PacketFilter);

            KeReleaseSpinLockFromDpcLevel(&FdoData->RcvLock);
            KeReleaseSpinLock(&FdoData->Lock, oldIrql);
            
            if (status == STATUS_SUCCESS)
            {
                FdoData->PacketFilter = PacketFilter;
            }

            break;

        case OID_PNP_SET_POWER:

            if (InformationBufferLength != sizeof(NDIS_DEVICE_POWER_STATE ))
            {
                return(STATUS_BUFFER_TOO_SMALL);
            }

            newDeviceState = *(PDEVICE_POWER_STATE UNALIGNED)InformationBuffer;
            oldDeviceState = FdoData->DevicePowerState;
            FdoData->DevicePowerState = newDeviceState;
            
            if (oldDeviceState == PowerDeviceD0) {

                status = PciDrvPowerBeginQueuingIrps(
                    FdoData->Self,
                    1,              // One for current OID request.
                    FALSE           // Do not query for state change.
                    );

                ASSERT(NT_SUCCESS(status));
            }
            
            //
            // Set the power state - Cannot fail this request
            //
            status = NICSetPower(FdoData, newDeviceState );

            if (status != STATUS_SUCCESS)
            {
                DebugPrint(ERROR, DBG_IOCTLS, "SET Power: Hardware error !!!\n");
                break;
            }

            if (newDeviceState == PowerDeviceD0) {

                //
                // Our hardware is now on again. Here we empty our existing queue of
                // requests and let in new ones. 
                //
                FdoData->QueueState = AllowRequests;
                PciDrvProcessQueuedRequests(FdoData);
            }
        
            status = STATUS_SUCCESS; 
            break;

        case OID_PNP_ADD_WAKE_UP_PATTERN:
            //
            // call a function that would program the adapter's wake
            // up pattern, return success
            //
            DebugPrint(TRACE, DBG_IOCTLS, "--> OID_PNP_ADD_WAKE_UP_PATTERN\n");            

            if (IsPoMgmtSupported(FdoData) )
            {
                status = NICAddWakeUpPattern(FdoData,
                                            InformationBuffer, 
                                            InformationBufferLength, 
                                            &unUsed, 
                                            &unUsed); 
            }
            else
            {
                status = STATUS_NOT_SUPPORTED;
            }
            break;

    
        case OID_PNP_REMOVE_WAKE_UP_PATTERN:

            //
            // call a function that would remove the adapter's wake
            // up pattern, return success
            //
            DebugPrint(TRACE, DBG_IOCTLS, "--> OID_PNP_ADD_WAKE_UP_PATTERN\n");            
            
            if (IsPoMgmtSupported(FdoData) )
            {
                status = NICRemoveWakeUpPattern(FdoData, 
                                               InformationBuffer, 
                                               InformationBufferLength,
                                               &unUsed,
                                               &unUsed);

            }
            else
            {
                status = STATUS_NOT_SUPPORTED;
            }
            break;

        case OID_PNP_ENABLE_WAKE_UP:
            //
            // call a function that would enable wake up on the adapter
            // return success
            //
            DebugPrint(TRACE, DBG_IOCTLS, "--> OID_PNP_ENABLE_WAKE_UP\n");            
            if (IsPoMgmtSupported(FdoData))
            {
                ULONG       WakeUpEnable;
                RtlMoveMemory(&WakeUpEnable, InformationBuffer,sizeof(ULONG));
                //
                // The WakeUpEable can only be 0, or NDIS_PNP_WAKE_UP_PATTERN_MATCH since the driver only
                // supports wake up pattern match
                //
                if ((WakeUpEnable != 0)
                       && ((WakeUpEnable & NDIS_PNP_WAKE_UP_PATTERN_MATCH) != NDIS_PNP_WAKE_UP_PATTERN_MATCH ))
                {
                    status = STATUS_NOT_SUPPORTED;
                    FdoData->AllowWakeArming = FALSE;    
                    break;
                }
                //
                // When the driver goes to low power state, it would check WakeUpEnable to decide
                // which wake up methed it should use to wake up the machine. If WakeUpEnable is 0,
                // no wake up method is enabled.
                //
                FdoData->AllowWakeArming = TRUE;
                
                status = STATUS_SUCCESS; 
            }
            else
            {
                status = STATUS_NOT_SUPPORTED;
            }

            break;            
        default:
            status = STATUS_NOT_SUPPORTED;
            break;
        }
    }while (FALSE);

    DebugPrint(LOUD, DBG_IOCTLS, "<-- HandleSetOIDRequest\n");
            
    return (status);
}

VOID 
NICServiceIndicateStatusIrp(
    IN PFDO_DATA        FdoData
    )
/*++

Routine Description:

   This routine is used indicate media status of the client.
   If the IRP was cancelled for some reason we will let
   the cancel routine do the IRP completion.
    
Arguments:


Return Value:

    None

--*/
{
    PIRP                        pIrp = NULL;
    PIO_STACK_LOCATION          pIrpSp = NULL;    
    PNDISPROT_INDICATE_STATUS   pIndicateStatus = NULL;
    NTSTATUS                    ntStatus = STATUS_CANCELLED;
    ULONG                       inBufLength, outBufLength;
    KIRQL                       oldIrql;
    

    DebugPrint(TRACE, DBG_IOCTLS, "-->ndisServiceIndicateStatusIrp\n");
    
    KeAcquireSpinLock(&FdoData->Lock, &oldIrql);

    pIrp = FdoData->StatusIndicationIrp;
              
    if(pIrp){
        
        //
        // Clear the cancel routine.
        //
        if(IoSetCancelRoutine(pIrp, NULL)){
            //
            // Cancel routine cannot run now and cannot have already 
            // started to run.
            //
            pIrpSp = IoGetCurrentIrpStackLocation(pIrp);            
            pIndicateStatus = pIrp->AssociatedIrp.SystemBuffer;
            inBufLength = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
            outBufLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;            
            
            //
            // Check to see whether the buffer is large enough.
            //
            if(outBufLength >= sizeof(NDISPROT_INDICATE_STATUS)){

                if(MP_TEST_FLAG(FdoData, fMP_ADAPTER_NO_CABLE)){
                    pIndicateStatus->IndicatedStatus = NDIS_STATUS_MEDIA_DISCONNECT;
                } else {
                    pIndicateStatus->IndicatedStatus = NDIS_STATUS_MEDIA_CONNECT;
                }
                pIndicateStatus->StatusBufferLength = 0;
                pIndicateStatus->StatusBufferOffset = 0;                                 
                ntStatus = STATUS_SUCCESS;
                
            } else {
                ntStatus = STATUS_BUFFER_OVERFLOW;
            }
            
            //
            // Since we are completing the IRP below, clear this field.
            //
            FdoData->StatusIndicationIrp = NULL;                
        }else {
            //
            // Cancel rotuine is running. Leave the irp alone.
            //
            pIrp = NULL;
        }
    }
    
    KeReleaseSpinLock(&FdoData->Lock, oldIrql);

    if(pIrp){
        pIrp->IoStatus.Information = sizeof(NDISPROT_INDICATE_STATUS);
        pIrp->IoStatus.Status = ntStatus;
        IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        PciDrvIoDecrement (FdoData);

    }

    DebugPrint(TRACE, DBG_IOCTLS, "<--ndisServiceIndicateStatusIrp\n");
    
    return;
    
}

MEDIA_STATE
NICIndicateMediaState(
    IN PFDO_DATA FdoData
    )
{
    MEDIA_STATE CurrMediaState;
    
    KeAcquireSpinLockAtDpcLevel(&FdoData->Lock);
    
    CurrMediaState = GetMediaState(FdoData);

    if (CurrMediaState != FdoData->MediaState)
    {
        DebugPrint(WARNING, DBG_IOCTLS, "Media state changed to %s\n",
            ((CurrMediaState == Connected)? 
            "Connected": "Disconnected"));

        FdoData->MediaState = CurrMediaState;
        
        if (CurrMediaState == Connected)
        {
            MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_NO_CABLE);
        }
        else
        {
            MP_SET_FLAG(FdoData, fMP_ADAPTER_NO_CABLE);
        }
        
        KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);
        
        // Indicate the media event
        NICServiceIndicateStatusIrp(FdoData);
    }
    else
    {
        KeReleaseSpinLockFromDpcLevel(&FdoData->Lock);
    }

    return CurrMediaState;
}


NTSTATUS 
NICGetStatsCounters(
    IN  PFDO_DATA   FdoData, 
    IN  NDIS_OID    Oid,
    OUT PULONG64    pCounter
    )
/*++
Routine Description:

    Get the value for a statistics OID

Arguments:

    FdoData     Pointer to our FdoData 
    Oid         Self-explanatory   
    pCounter    Pointer to receive the value
    
Return Value:

    NT Status code
    
--*/
{
    NTSTATUS     status = STATUS_SUCCESS;

    DebugPrint(TRACE, DBG_IOCTLS, "--> NICGetStatsCounters\n");

    *pCounter = 0; 

    DumpStatsCounters(FdoData);
            
    switch(Oid)
    {
        case OID_GEN_XMIT_OK:
            *pCounter = FdoData->GoodTransmits;
            break;

        case OID_GEN_RCV_OK:
            *pCounter = FdoData->GoodReceives;
            break;

        case OID_GEN_XMIT_ERROR:
            *pCounter = FdoData->TxAbortExcessCollisions +
                        FdoData->TxDmaUnderrun +
                        FdoData->TxLostCRS +
                        FdoData->TxLateCollisions;
            break;

        case OID_GEN_RCV_ERROR:
            *pCounter = FdoData->RcvCrcErrors +
                        FdoData->RcvAlignmentErrors +
                        FdoData->RcvResourceErrors +
                        FdoData->RcvDmaOverrunErrors +
                        FdoData->RcvRuntErrors;
            break;

        case OID_GEN_RCV_NO_BUFFER:
            *pCounter = FdoData->RcvResourceErrors;
            break;

        case OID_GEN_RCV_CRC_ERROR:
            *pCounter = FdoData->RcvCrcErrors;
            break;

        case OID_GEN_TRANSMIT_QUEUE_LENGTH:
            *pCounter = FdoData->nWaitSend;
            break;

        case OID_802_3_RCV_ERROR_ALIGNMENT:
            *pCounter = FdoData->RcvAlignmentErrors;
            break;

        case OID_802_3_XMIT_ONE_COLLISION:
            *pCounter = FdoData->OneRetry;
            break;

        case OID_802_3_XMIT_MORE_COLLISIONS:
            *pCounter = FdoData->MoreThanOneRetry;
            break;

        case OID_802_3_XMIT_DEFERRED:
            *pCounter = FdoData->TxOKButDeferred;
            break;

        case OID_802_3_XMIT_MAX_COLLISIONS:
            *pCounter = FdoData->TxAbortExcessCollisions;
            break;

        case OID_802_3_RCV_OVERRUN:
            *pCounter = FdoData->RcvDmaOverrunErrors;
            break;

        case OID_802_3_XMIT_UNDERRUN:
            *pCounter = FdoData->TxDmaUnderrun;
            break;

        case OID_802_3_XMIT_HEARTBEAT_FAILURE:
            *pCounter = FdoData->TxLostCRS;
            break;

        case OID_802_3_XMIT_TIMES_CRS_LOST:
            *pCounter = FdoData->TxLostCRS;
            break;

        case OID_802_3_XMIT_LATE_COLLISIONS:
            *pCounter = FdoData->TxLateCollisions;
            break;

        default:
            status = STATUS_NOT_SUPPORTED;
            break;
    }

    DebugPrint(TRACE, DBG_IOCTLS, "<-- NICGetStatsCounters\n");

    return(status);
}

NTSTATUS NICSetPacketFilter(
    IN PFDO_DATA    FdoData,
    IN ULONG        PacketFilter
    )
/*++
Routine Description:

    This routine will set up the FdoData so that it accepts packets 
    that match the specified packet filter.  The only filter bits   
    that can truly be toggled are for broadcast and promiscuous     

Arguments:
    
    FdoData         Pointer to our FdoData
    PacketFilter    The new packet filter 
    
Return Value:

    
--*/
{
    NTSTATUS        status = STATUS_SUCCESS;
    UCHAR           NewParameterField;
    UINT            i;
    BOOLEAN         bResult;

    DebugPrint(TRACE, DBG_IOCTLS, "--> NICSetPacketFilter, PacketFilter=%08x\n", PacketFilter);

    //
    // Need to enable or disable broadcast and promiscuous support depending
    // on the new filter
    //
    NewParameterField = CB_557_CFIG_DEFAULT_PARM15;

    if (PacketFilter & NDIS_PACKET_TYPE_BROADCAST) 
    {
        NewParameterField &= ~CB_CFIG_BROADCAST_DIS;
    }
    else 
    {
        NewParameterField |= CB_CFIG_BROADCAST_DIS;
    }

    if (PacketFilter & NDIS_PACKET_TYPE_PROMISCUOUS) 
    {
        NewParameterField |= CB_CFIG_PROMISCUOUS;
    }
    else 
    {
        NewParameterField &= ~CB_CFIG_PROMISCUOUS;
    }

    do
    {
        if ((FdoData->OldParameterField == NewParameterField ) &&
            !(PacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST))
        {
            break;
        }

        //
        // Only need to do something to the HW if the filter bits have changed.
        //
        FdoData->OldParameterField = NewParameterField;
        ((PCB_HEADER_STRUC)FdoData->NonTxCmdBlock)->CbCommand = CB_CONFIGURE;
        ((PCB_HEADER_STRUC)FdoData->NonTxCmdBlock)->CbStatus = 0;
        ((PCB_HEADER_STRUC)FdoData->NonTxCmdBlock)->CbLinkPointer = DRIVER_NULL;

        //
        // First fill in the static (end user can't change) config bytes
        //
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[0] = CB_557_CFIG_DEFAULT_PARM0;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[2] = CB_557_CFIG_DEFAULT_PARM2;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[3] = CB_557_CFIG_DEFAULT_PARM3;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[6] = CB_557_CFIG_DEFAULT_PARM6;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[9] = CB_557_CFIG_DEFAULT_PARM9;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[10] = CB_557_CFIG_DEFAULT_PARM10;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[11] = CB_557_CFIG_DEFAULT_PARM11;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[12] = CB_557_CFIG_DEFAULT_PARM12;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[13] = CB_557_CFIG_DEFAULT_PARM13;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[14] = CB_557_CFIG_DEFAULT_PARM14;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[16] = CB_557_CFIG_DEFAULT_PARM16;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[17] = CB_557_CFIG_DEFAULT_PARM17;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[18] = CB_557_CFIG_DEFAULT_PARM18;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[20] = CB_557_CFIG_DEFAULT_PARM20;

        //
        // Set the Tx underrun retries
        //
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[7] =
            (UCHAR) (CB_557_CFIG_DEFAULT_PARM7 | (FdoData->AiUnderrunRetry << 1));

        //
        // Set the Tx and Rx Fifo limits
        //
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[1] =
            (UCHAR) ((FdoData->AiTxFifo << 4) | FdoData->AiRxFifo);

        //
        // set the MWI enable bit if needed
        //
        if (FdoData->MWIEnable)
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[3] |= CB_CFIG_B3_MWI_ENABLE;

        //
        // Set the Tx and Rx DMA maximum byte count fields.
        //
        if ((FdoData->AiRxDmaCount) || (FdoData->AiTxDmaCount))
        {
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[4] =
                FdoData->AiRxDmaCount;
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[5] =
                (UCHAR) (FdoData->AiTxDmaCount | CB_CFIG_DMBC_EN);
        }
        else
        {
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[4] =
                CB_557_CFIG_DEFAULT_PARM4;
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[5] =
                CB_557_CFIG_DEFAULT_PARM5;
        }

        //
        // Setup for MII or 503 operation.  The CRS+CDT bit should only be
        // set when operating in 503 mode.
        //
        if (FdoData->PhyAddress == 32)
        {
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[8] =
                (CB_557_CFIG_DEFAULT_PARM8 & (~CB_CFIG_503_MII));
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[15] =
                (UCHAR) (NewParameterField | CB_CFIG_CRS_OR_CDT);
        }
        else
        {
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[8] =
                (CB_557_CFIG_DEFAULT_PARM8 | CB_CFIG_503_MII);
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[15] =
                (UCHAR) (NewParameterField & (~CB_CFIG_CRS_OR_CDT));
        }

        //
        // Setup Full duplex stuff
        //

        //
        // If forced to half duplex
        //
        if (FdoData->AiForceDpx == 1) 
        {
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
                (CB_557_CFIG_DEFAULT_PARM19 &
                (~(CB_CFIG_FORCE_FDX| CB_CFIG_FDX_ENABLE)));
        }
        //
        // If forced to full duplex
        //
        else if (FdoData->AiForceDpx == 2)
        {
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
                (CB_557_CFIG_DEFAULT_PARM19 | CB_CFIG_FORCE_FDX);
        }
        //
        // If auto-duplex
        //
        else 
        {
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
                                                CB_557_CFIG_DEFAULT_PARM19;
        }

        //
        // if multicast all is being turned on, set the bit
        //
        if (PacketFilter & NDIS_PACKET_TYPE_ALL_MULTICAST) 
        {
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[21] =
                                 (CB_557_CFIG_DEFAULT_PARM21 | CB_CFIG_MULTICAST_ALL);
        }
        else 
        {
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[21] =
                                                CB_557_CFIG_DEFAULT_PARM21;
        }


        //
        // Wait for the SCB to clear before we check the CU status.
        //
        if (!WaitScb(FdoData))
        {
            status = STATUS_DEVICE_DATA_ERROR;
            break;
        }

        //
        // If we have issued any transmits, then the CU will either be active,
        // or in the suspended state.  If the CU is active, then we wait for
        // it to be suspended.
        //
        if (FdoData->TransmitIdle == FALSE)
        {
            //
            // Wait for suspended state
            //
            MP_STALL_AND_WAIT((FdoData->CSRAddress->ScbStatus & SCB_CUS_MASK) != SCB_CUS_ACTIVE, 5000, bResult);
            if (!bResult)
            {
                MP_SET_HARDWARE_ERROR(FdoData);
                status = STATUS_DEVICE_DATA_ERROR;
                break;
            }

            //
            // Check the current status of the receive unit
            //
            if ((FdoData->CSRAddress->ScbStatus & SCB_RUS_MASK) != SCB_RUS_IDLE)
            {
                // Issue an RU abort.  Since an interrupt will be issued, the
                // RU will be started by the DPC.
                status = D100IssueScbCommand(FdoData, SCB_RUC_ABORT, TRUE);
                if (status != STATUS_SUCCESS)
                {
                    break;
                }
            }
            
            if (!WaitScb(FdoData))
            {
                status = STATUS_DEVICE_DATA_ERROR;
                break;
            }
           
            //
            // Restore the transmit software flags.  After the multicast
            // command is issued, the command unit will be idle, because the
            // EL bit will be set in the multicast commmand block.
            //
            FdoData->TransmitIdle = TRUE;
            FdoData->ResumeWait = TRUE;
        }
        
        //
        // Display config info
        //
        DebugPrint(TRACE, DBG_IOCTLS, "Re-Issuing Configure command for filter change\n");
        DebugPrint(TRACE, DBG_IOCTLS, "Config Block at virt addr %p, phys address %x\n",
            &((PCB_HEADER_STRUC)FdoData->NonTxCmdBlock)->CbStatus, FdoData->NonTxCmdBlockPhys);

        for (i = 0; i < CB_CFIG_BYTE_COUNT; i++)
            DebugPrint(LOUD, DBG_IOCTLS, "  Config byte %x = %.2x\n", i, FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[i]);

        //
        // Submit the configure command to the chip, and wait for it to complete.
        //
        FdoData->CSRAddress->ScbGeneralPointer = FdoData->NonTxCmdBlockPhys;
        status = D100SubmitCommandBlockAndWait(FdoData);
        if (status != STATUS_SUCCESS)
        {
            status = STATUS_DEVICE_NOT_READY;
        }

    } while (FALSE);

    DebugPrint(TRACE, DBG_IOCTLS, "<-- NICSetPacketFilter, Status=%x\n", status);

    return(status);
}

NTSTATUS 
NICSetMulticastList(
    IN  PFDO_DATA  FdoData
    )
/*++
Routine Description:

    This routine will set up the FdoData for a specified multicast address list
    
Arguments:
    
    FdoData     Pointer to our FdoData
    
Return Value:

    
--*/
{
    NTSTATUS        status;
    PUCHAR          McAddress;
    UINT            i, j;
    BOOLEAN         bResult;

    DebugPrint(TRACE, DBG_IOCTLS, "--> NICSetMulticastList\n");

    //
    // Setup the command block for the multicast command.
    //
    for (i = 0; i < FdoData->MCAddressCount; i++)
    {
        DebugPrint(TRACE, DBG_IOCTLS, "MC(%d) = %02x-%02x-%02x-%02x-%02x-%02x\n", 
            i,
            FdoData->MCList[i][0],
            FdoData->MCList[i][1],
            FdoData->MCList[i][2],
            FdoData->MCList[i][3],
            FdoData->MCList[i][4],
            FdoData->MCList[i][5]);

        McAddress = &FdoData->NonTxCmdBlock->NonTxCb.Multicast.McAddress[i*ETHERNET_ADDRESS_LENGTH];

        for (j = 0; j < ETH_LENGTH_OF_ADDRESS; j++)
            *(McAddress++) = FdoData->MCList[i][j];
    }

    FdoData->NonTxCmdBlock->NonTxCb.Multicast.McCount =
        (USHORT)(FdoData->MCAddressCount * ETH_LENGTH_OF_ADDRESS);
    ((PCB_HEADER_STRUC)FdoData->NonTxCmdBlock)->CbStatus = 0;
    ((PCB_HEADER_STRUC)FdoData->NonTxCmdBlock)->CbCommand = CB_MULTICAST;

    //
    // Wait for the SCB to clear before we check the CU status.
    //
    if (!WaitScb(FdoData))
    {
        status = STATUS_DEVICE_DATA_ERROR;
        MP_EXIT;
    }

    //
    // If we have issued any transmits, then the CU will either be active, or
    // in the suspended state.  If the CU is active, then we wait for it to be
    // suspended.
    //
    if (FdoData->TransmitIdle == FALSE)
    {
        //
        // Wait for suspended state
        //
        MP_STALL_AND_WAIT((FdoData->CSRAddress->ScbStatus & SCB_CUS_MASK) != SCB_CUS_ACTIVE, 5000, bResult);
        if (!bResult)
        {
            MP_SET_HARDWARE_ERROR(FdoData);
            status = STATUS_DEVICE_DATA_ERROR;
        }

        //
        // Restore the transmit software flags.  After the multicast command is
        // issued, the command unit will be idle, because the EL bit will be
        // set in the multicast commmand block.
        //
        FdoData->TransmitIdle = TRUE;
        FdoData->ResumeWait = TRUE;
    }

    //
    // Update the command list pointer.
    //
    FdoData->CSRAddress->ScbGeneralPointer = FdoData->NonTxCmdBlockPhys;

    //
    // Submit the multicast command to the FdoData and wait for it to complete.
    //
    status = D100SubmitCommandBlockAndWait(FdoData);
    if (status != STATUS_SUCCESS)
    {
        status = STATUS_DEVICE_NOT_READY;
    }
    
    exit:

    DebugPrint(TRACE, DBG_IOCTLS, "<-- NICSetMulticastList, status=%x\n", status);

    return(status);

}

 
VOID
NICFillPoMgmtCaps (
    IN PFDO_DATA                 FdoData, 
    IN OUT PNDIS_PNP_CAPABILITIES  pPower_Management_Capabilities, 
    IN OUT PNDIS_STATUS            pStatus,
    IN OUT PULONG                  pulInfoLen
    )
/*++
Routine Description:

    Fills in the Power  Managment structure depending the capabilities of 
    the software driver and the card.

    Currently this is only supported on 82559 Version of the driver

Arguments:
    
    FdoData                 Pointer to the FdoData structure
    pPower_Management_Capabilities - Power management struct as defined in the DDK, 
    pStatus                 Status to be returned by the request,
    pulInfoLen              Length of the pPowerManagmentCapabilites
    
Return Value:

    Success or failure depending on the type of card
--*/

{

    BOOLEAN bIsPoMgmtSupported; 
    
    bIsPoMgmtSupported = IsPoMgmtSupported(FdoData);

    if (bIsPoMgmtSupported == TRUE)
    {
        pPower_Management_Capabilities->Flags = NDIS_DEVICE_WAKE_UP_ENABLE; 
        pPower_Management_Capabilities->WakeUpCapabilities.MinMagicPacketWakeUp = NdisDeviceStateUnspecified;
        pPower_Management_Capabilities->WakeUpCapabilities.MinPatternWakeUp = NdisDeviceStateD3;
        pPower_Management_Capabilities->WakeUpCapabilities.MinLinkChangeWakeUp  = NdisDeviceStateUnspecified;
        *pulInfoLen = sizeof (*pPower_Management_Capabilities);
        *pStatus =  STATUS_SUCCESS;
    }
    else
    {
        RtlZeroMemory (pPower_Management_Capabilities, sizeof(*pPower_Management_Capabilities));
        *pStatus =  STATUS_NOT_SUPPORTED;
        *pulInfoLen = 0;
            
    }
}
