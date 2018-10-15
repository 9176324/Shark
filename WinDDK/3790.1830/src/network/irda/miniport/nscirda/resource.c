/*
 ************************************************************************
 *
 *	RESOURCE.c
 *
 *
 * Portions Copyright (C) 1996-2001 National Semiconductor Corp.
 * All rights reserved.
 * Copyright (C) 1996-2001 Microsoft Corporation. All Rights Reserved.
 *
 *
 *
 *************************************************************************
 */


#include "nsc.h"
#include "resource.tmh"


/*
 *************************************************************************
 *  MyMemAlloc
 *************************************************************************
 *
 */
PVOID NscMemAlloc(UINT size)
{
    NDIS_STATUS stat;
    PVOID memptr;

    stat = NdisAllocateMemoryWithTag(
                                &memptr,
                                size,
                                'rIsN'
                                );

    if (stat == NDIS_STATUS_SUCCESS) {

        NdisZeroMemory((PVOID)memptr, size);

    } else {

        DBGERR(("Memory allocation failed"));
        memptr = NULL;
    }

    return memptr;
}


/*
 *************************************************************************
 *  MyMemFree
 *************************************************************************
 *
 */
VOID NscMemFree(PVOID memptr)
{

    NdisFreeMemory(memptr, 0, 0);
}

PVOID
NscAllocateDmaBuffer(
    NDIS_HANDLE     AdapterHandle,
    ULONG           Size,
    PNSC_DMA_BUFFER_INFO    DmaBufferInfo
    )

{
    NDIS_STATUS     Status;

    NdisZeroMemory(DmaBufferInfo,sizeof(*DmaBufferInfo));

    DmaBufferInfo->Length=Size;
    DmaBufferInfo->AdapterHandle=AdapterHandle;

    NdisMAllocateSharedMemory(
        DmaBufferInfo->AdapterHandle,
        DmaBufferInfo->Length,
        TRUE,
        &DmaBufferInfo->VirtualAddress,
        &DmaBufferInfo->PhysicalAddress
        );

    if (DmaBufferInfo->VirtualAddress == NULL) {
        //
        // new style did not work, try old style
        //
        const NDIS_PHYSICAL_ADDRESS MaxAddress = NDIS_PHYSICAL_ADDRESS_CONST(0x00ffffff, 0);
#if DBG
        DbgPrint("NSCIRDA: NdisMAllocateSharedMemoryFailed(), calling NdisAllocateMemory() instead (ok for XP and W2K)\n");
#endif
        Status=NdisAllocateMemory(
            &DmaBufferInfo->VirtualAddress,
            DmaBufferInfo->Length,
            NDIS_MEMORY_CONTIGUOUS | NDIS_MEMORY_NONCACHED,
            MaxAddress
            );

        if (Status != STATUS_SUCCESS) {
            //
            //  old style allocation failed
            //
            NdisZeroMemory(DmaBufferInfo,sizeof(*DmaBufferInfo));

        } else {
            //
            //  old style work, not a shared allocation
            //
            DmaBufferInfo->SharedAllocation=FALSE;
        }

    } else {
        //
        //  new style worked
        //
        DmaBufferInfo->SharedAllocation=TRUE;
    }

    return DmaBufferInfo->VirtualAddress;

}

VOID
NscFreeDmaBuffer(
    PNSC_DMA_BUFFER_INFO    DmaBufferInfo
    )

{

    if ((DmaBufferInfo->AdapterHandle == NULL) || (DmaBufferInfo->VirtualAddress == NULL)) {
        //
        //  Not been allocated
        //
        ASSERT(0);

        return;
    }

    if (DmaBufferInfo->SharedAllocation) {
        //
        //  allocated with ndis shared memory functions
        //
        NdisMFreeSharedMemory(
            DmaBufferInfo->AdapterHandle,
            DmaBufferInfo->Length,
            TRUE,
            DmaBufferInfo->VirtualAddress,
            DmaBufferInfo->PhysicalAddress\
            );

    } else {
        //
        //  Allocated via old api
        //
#if DBG
        DbgPrint("NSCIRDA: Freeing DMA buffer with NdisFreeMemory() (ok for XP and W2K)\n");
#endif

        NdisFreeMemory(
            DmaBufferInfo->VirtualAddress,
            DmaBufferInfo->Length,
            NDIS_MEMORY_CONTIGUOUS | NDIS_MEMORY_NONCACHED
            );

    }

    NdisZeroMemory(DmaBufferInfo,sizeof(*DmaBufferInfo));

    return;

}


/*
 *************************************************************************
 *  NewDevice
 *************************************************************************
 *
 */
IrDevice *NewDevice()
{
    IrDevice *newdev;

    newdev = NscMemAlloc(sizeof(IrDevice));
    if (newdev){
        InitDevice(newdev);
    }
    return newdev;
}


/*
 *************************************************************************
 *  FreeDevice
 *************************************************************************
 *
 */
VOID FreeDevice(IrDevice *dev)
{
    CloseDevice(dev);
    NdisFreeSpinLock(&dev->QueueLock);
    NscMemFree((PVOID)dev);
}



/*
 *************************************************************************
 *  InitDevice
 *************************************************************************
 *
 *  Zero out the device object.
 *
 *  Allocate the device object's spinlock, which will persist while
 *  the device is opened and closed.
 *
 */
VOID InitDevice(IrDevice *thisDev)
{
    NdisZeroMemory((PVOID)thisDev, sizeof(IrDevice));
    NdisInitializeListHead(&thisDev->SendQueue);
    NdisAllocateSpinLock(&thisDev->QueueLock);
    NdisInitializeTimer(&thisDev->TurnaroundTimer,
                        DelayedWrite,
                        thisDev);
    NdisInitializeListHead(&thisDev->rcvBufBuf);
    NdisInitializeListHead(&thisDev->rcvBufFree);
    NdisInitializeListHead(&thisDev->rcvBufFull);
    NdisInitializeListHead(&thisDev->rcvBufPend);
}




/*
 *************************************************************************
 *  OpenDevice
 *************************************************************************
 *
 *  Allocate resources for a single device object.
 *
 *  This function should be called with device lock already held.
 *
 */
BOOLEAN OpenDevice(IrDevice *thisDev)
{
    BOOLEAN result = FALSE;
    NDIS_STATUS stat;
    UINT bufIndex;

    DBGOUT(("OpenDevice()"));

    if (!thisDev){
        return FALSE;
    }


    /*
     *  Allocate the NDIS packet and NDIS buffer pools
     *  for this device's RECEIVE buffer queue.
     *  Our receive packets must only contain one buffer apiece,
     *  so #buffers == #packets.
     */

    NdisAllocatePacketPool(&stat, &thisDev->packetPoolHandle, NUM_RCV_BUFS, 6 * sizeof(PVOID));
    if (stat != NDIS_STATUS_SUCCESS){
        goto _openDone;
    }

    NdisAllocateBufferPool(&stat, &thisDev->bufferPoolHandle, NUM_RCV_BUFS);
    if (stat != NDIS_STATUS_SUCCESS){
        goto _openDone;
    }


    //
    //  allocate the receive buffers used to hold receives SIR frames
    //
    for (bufIndex = 0; bufIndex < NUM_RCV_BUFS; bufIndex++){

        PVOID buf;

        buf = NscMemAlloc(RCV_BUFFER_SIZE);

        if (!buf){
            goto _openDone;
        }

        // We treat the beginning of the buffer as a LIST_ENTRY.

        // Normally we would use NDISSynchronizeInsertHeadList, but the
        // Interrupt hasn't been registered yet.
        InsertHeadList(&thisDev->rcvBufBuf, (PLIST_ENTRY)buf);

    }

    //
    //  initialize the data structures that keep track of receives buffers
    //
    for (bufIndex = 0; bufIndex < NUM_RCV_BUFS; bufIndex++){

        rcvBuffer *rcvBuf = NscMemAlloc(sizeof(rcvBuffer));

        if (!rcvBuf)
        {
            goto _openDone;
        }

        rcvBuf->state = STATE_FREE;
        rcvBuf->isDmaBuf = FALSE;

        /*
         *  Allocate a data buffer
         *
         *  This buffer gets swapped with the one on comPortInfo
         *  and must be the same size.
         */
        rcvBuf->dataBuf = NULL;

        /*
         *  Allocate the NDIS_PACKET.
         */
        NdisAllocatePacket(&stat, &rcvBuf->packet, thisDev->packetPoolHandle);
        if (stat != NDIS_STATUS_SUCCESS){

            NscMemFree(rcvBuf);
            rcvBuf=NULL;
            goto _openDone;
        }

        /*
         *  For future convenience, set the MiniportReserved portion of the packet
         *  to the index of the rcv buffer that contains it.
         *  This will be used in ReturnPacketHandler.
         */
        *(ULONG_PTR *)rcvBuf->packet->MiniportReserved = (ULONG_PTR)rcvBuf;

        rcvBuf->dataLen = 0;

        InsertHeadList(&thisDev->rcvBufFree, &rcvBuf->listEntry);

    }



    /*
     *  Set mediaBusy to TRUE initially.  That way, we won't
     *  IndicateStatus to the protocol in the ISR unless the
     *  protocol has expressed interest by clearing this flag
     *  via MiniportSetInformation(OID_IRDA_MEDIA_BUSY).
     */
    thisDev->mediaBusy = FALSE;
    thisDev->haveIndicatedMediaBusy = TRUE;

    /*
     *  Will set speed to 9600 baud initially.
     */
    thisDev->linkSpeedInfo = &supportedBaudRateTable[BAUDRATE_9600];

    thisDev->lastPacketAtOldSpeed = NULL;
    thisDev->setSpeedAfterCurrentSendPacket = FALSE;

    result = TRUE;

    _openDone:
    if (!result){
        /*
         *  If we're failing, close the device to free up any resources
         *  that were allocated for it.
         */
        CloseDevice(thisDev);
        DBGOUT(("OpenDevice() failed"));
    }
    else {
        DBGOUT(("OpenDevice() succeeded"));
    }
    return result;

}



/*
 *************************************************************************
 *  CloseDevice
 *************************************************************************
 *
 *  Free the indicated device's resources.
 *
 *
 *  Called for shutdown and reset.
 *  Don't clear ndisAdapterHandle, since we might just be resetting.
 *  This function should be called with device lock held.
 *
 *
 */
VOID CloseDevice(IrDevice *thisDev)
{
    PLIST_ENTRY ListEntry;

    DBGOUT(("CloseDevice()"));

    if (!thisDev){
        return;
    }

    /*
     *  Free all resources for the RECEIVE buffer queue.
     */

    while (!IsListEmpty(&thisDev->rcvBufFree))
    {
        rcvBuffer *rcvBuf;

        ListEntry = RemoveHeadList(&thisDev->rcvBufFree);
        rcvBuf = CONTAINING_RECORD(ListEntry,
                                   rcvBuffer,
                                   listEntry);

        if (rcvBuf->packet){
            NdisFreePacket(rcvBuf->packet);
            rcvBuf->packet = NULL;
        }

        NscMemFree(rcvBuf);
    }


    while (!IsListEmpty(&thisDev->rcvBufBuf))
    {
        ListEntry = RemoveHeadList(&thisDev->rcvBufBuf);
        NscMemFree(ListEntry);
    }
    /*
     *  Free the packet and buffer pool handles for this device.
     */
    if (thisDev->packetPoolHandle){
        NdisFreePacketPool(thisDev->packetPoolHandle);
        thisDev->packetPoolHandle = NULL;
    }

    if (thisDev->bufferPoolHandle){
        NdisFreeBufferPool(thisDev->bufferPoolHandle);
        thisDev->bufferPoolHandle = NULL;
    }

    //
    //  the send queue should be empty now
    //
    ASSERT(IsListEmpty(&thisDev->SendQueue));


    thisDev->mediaBusy = FALSE;
    thisDev->haveIndicatedMediaBusy = FALSE;

    thisDev->linkSpeedInfo = NULL;

}

