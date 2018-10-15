/*++

Copyright (c) 2004  Microsoft Corporation

Module Name:

    isorwr.c

Abstract:

    This file has dispatch routines for read and write.

Environment:

    Kernel mode

Notes:

    Copyright (c) 2004 Microsoft Corporation.
    All Rights Reserved.

--*/

#include "isousb.h"
#include "isopnp.h"
#include "isopwr.h"
#include "isodev.h"
#include "isowmi.h"
#include "isousr.h"
#include "isorwr.h"
#include "isostrm.h"


NTSTATUS
IsoUsb_DispatchReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine does some validation and
    invokes appropriate function to perform
    Isoch transfer

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    ULONG                  totalLength;
    ULONG                  packetSize;
    NTSTATUS               ntStatus;
    PFILE_OBJECT           fileObject;
    PDEVICE_EXTENSION      deviceExtension;
    PIO_STACK_LOCATION     irpStack;
    PFILE_OBJECT_CONTENT   fileObjectContent;
    PUSBD_PIPE_INFORMATION pipeInformation;

    //
    // initialize vars
    //
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    totalLength = 0;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchReadWrite - begins\n"));

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchReadWrite::"));
    IsoUsb_IoIncrement(deviceExtension);

    if(deviceExtension->DeviceState != Working) {

        IsoUsb_DbgPrint(1, ("Invalid device state\n"));

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

    //
    // make sure that the selective suspend request has been completed.
    //
    if(deviceExtension->SSEnable) {

        //
        // It is true that the client driver cancelled the selective suspend
        // request in the dispatch routine for create Irps.
        // But there is no guarantee that it has indeed completed.
        // so wait on the NoIdleReqPendEvent and proceed only if this event
        // is signalled.
        //
        IsoUsb_DbgPrint(3, ("Waiting on the IdleReqPendEvent\n"));


        KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
    }

    //
    // obtain the pipe information for read
    // and write from the fileobject.
    //
    if(fileObject && fileObject->FsContext) {

        fileObjectContent = (PFILE_OBJECT_CONTENT) fileObject->FsContext;

        pipeInformation = (PUSBD_PIPE_INFORMATION)
                          fileObjectContent->PipeInformation;
    }
    else {

        IsoUsb_DbgPrint(1, ("Invalid device state\n"));

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

    if((pipeInformation == NULL) ||
       (UsbdPipeTypeIsochronous != pipeInformation->PipeType)) {

        IsoUsb_DbgPrint(1, ("Incorrect pipe\n"));

        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

    if(Irp->MdlAddress) {

        totalLength = MmGetMdlByteCount(Irp->MdlAddress);
    }

    if(totalLength == 0) {

        IsoUsb_DbgPrint(1, ("Transfer data length = 0\n"));

        ntStatus = STATUS_SUCCESS;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

    //
    // each packet can hold this much info
    //
    packetSize = pipeInformation->MaximumPacketSize;

    if(packetSize == 0) {

        IsoUsb_DbgPrint(1, ("Invalid parameter\n"));

        ntStatus = STATUS_INVALID_PARAMETER;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

    //
    // atleast packet worth of data to be transferred.
    //
    if(totalLength < packetSize) {

        IsoUsb_DbgPrint(1, ("Atleast packet worth of data..\n"));

        ntStatus = STATUS_INVALID_PARAMETER;
        goto IsoUsb_DispatchReadWrite_Exit;
    }

    // perform reset. if there are some active transfers queued up
    // for this endpoint then the reset pipe will fail.
    //
    IsoUsb_AbortResetPipe(DeviceObject,
                          pipeInformation->PipeHandle,
                          TRUE);

    if(deviceExtension->IsDeviceHighSpeed) {

        ntStatus = PerformHighSpeedIsochTransfer(DeviceObject,
                                                 pipeInformation,
                                                 Irp,
                                                 totalLength);

    }
    else {

        ntStatus = PerformFullSpeedIsochTransfer(DeviceObject,
                                                 pipeInformation,
                                                 Irp,
                                                 totalLength);

    }

    return ntStatus;

IsoUsb_DispatchReadWrite_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchReadWrite::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("-------------------------------\n"));

    return ntStatus;
}


NTSTATUS
PerformFullSpeedIsochTransfer(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PIRP                   Irp,
    IN ULONG                  TotalLength
    )
/*++

Routine Description:

    This routine splits up a main isoch transfer request into one or
    more sub requests as necessary.  Each isoch irp/urb pair can span at
    most 255 packets.

    1. It creates a SUB_REQUEST_CONTEXT for each irp/urb pair and
       attaches it to the main request irp.

    2. It intializes all of the sub request irp/urb pairs, and sub mdls
       too.

    3. It passes down the driver stack all of the sub request irps.

    4. It leaves the completion of the main request irp as the
       responsibility of the sub request irp completion routine, except
       in the exception case where the main request irp is canceled
       prior to passing any of the the sub request irps down the driver
       stack.

Arguments:

    DeviceObject - pointer to device object
    PipeInformation - USBD_PIPE_INFORMATION
    Irp - I/O request packet
    TotalLength - no. of bytes to be transferred

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      irpStack;
    BOOLEAN                 read;
    ULONG                   packetSize;
    ULONG                   stageSize;
    ULONG                   numIrps;
    PMAIN_REQUEST_CONTEXT   mainRequestContext;
    PSUB_REQUEST_CONTEXT *  subRequestContextArray;
    PSUB_REQUEST_CONTEXT    subRequestContext;
    PLIST_ENTRY             subRequestEntry;
    CCHAR                   stackSize;
    PUCHAR                  virtualAddress;
    ULONG                   i;
    ULONG                   j;
    KIRQL                   oldIrql;
    NTSTATUS                ntStatus;
    PIO_STACK_LOCATION      nextStack;

    //
    // initialize vars
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    read = (irpStack->MajorFunction == IRP_MJ_READ) ? TRUE : FALSE;

    IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer - begins\n"));

    //
    // each packet (frame) can hold this much info
    //
    packetSize = PipeInformation->MaximumPacketSize;

    IsoUsb_DbgPrint(3, ("totalLength = %d\n", TotalLength));
    IsoUsb_DbgPrint(3, ("packetSize = %d\n", packetSize));

    //
    // There is an inherent limit on the number of packets that can be
    // passed down the stack with each irp/urb pair (255)
    //
    // If the number of required packets is > 255, we shall create
    // "(required-packets / 255) [+ 1]" number of irp/urb pairs.
    //
    // Each irp/urb pair transfer is also called a stage transfer.
    //
    if (TotalLength > (packetSize * 255))
    {
        stageSize = packetSize * 255;

        numIrps = (TotalLength + stageSize - 1) / stageSize;
    }
    else
    {
        stageSize = TotalLength;

        numIrps = 1;
    }

    IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer::stageSize = %d\n",
                        stageSize));

    IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer::numIrps = %d\n",
                        numIrps));

    // Initialize the main request Irp read/write context, which is
    // overlaid on top of Irp->Tail.Overlay.DriverContext.
    //
    mainRequestContext = (PMAIN_REQUEST_CONTEXT)
                         Irp->Tail.Overlay.DriverContext;

    InitializeListHead(&mainRequestContext->SubRequestList);

    stackSize = deviceExtension->TopOfStackDeviceObject->StackSize;

    virtualAddress = (PUCHAR) MmGetMdlVirtualAddress(Irp->MdlAddress);


    // Allocate an array to keep track of the sub requests that will be
    // allocated below.  This array exists only during the execution of
    // this routine and is used only to keep track of the sub requests
    // before calling them down the driver stack.
    //
    subRequestContextArray = (PSUB_REQUEST_CONTEXT *)
                             ExAllocatePool(NonPagedPool,
                                            numIrps * sizeof(PSUB_REQUEST_CONTEXT));

    if (subRequestContextArray == NULL)
    {
        IsoUsb_DbgPrint(1, ("failed to allocate subRequestContextArray\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        goto PerformFullSpeedIsochTransfer_Free;
    }

    RtlZeroMemory(subRequestContextArray, numIrps * sizeof(PSUB_REQUEST_CONTEXT));

    //
    // Allocate the sub requests
    //
    for (i = 0; i < numIrps; i++)
    {
        PIRP    subIrp;
        PURB    subUrb;
        PMDL    subMdl;
        ULONG   nPackets;
        ULONG   urbSize;
        ULONG   offset;

        // The following outer scope variables are updated during each
        // iteration of the loop:  virtualAddress, TotalLength, stageSize

        //
        // For every stage of transfer we need to do the following
        // tasks:
        //
        // 1. Allocate a sub request context (SUB_REQUEST_CONTEXT).
        // 2. Allocate a sub request irp.
        // 3. Allocate a sub request urb.
        // 4. Allocate a sub request mdl.
        // 5. Initialize the above allocations.
        //

        //
        // 1. Allocate a Sub Request Context (SUB_REQUEST_CONTEXT)
        //

        subRequestContext = (PSUB_REQUEST_CONTEXT)
                            ExAllocatePool(NonPagedPool,
                                           sizeof(SUB_REQUEST_CONTEXT));

        if (subRequestContext == NULL)
        {
            IsoUsb_DbgPrint(1, ("failed to allocate subRequestContext\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto PerformFullSpeedIsochTransfer_Free;
        }

        RtlZeroMemory(subRequestContext, sizeof(SUB_REQUEST_CONTEXT));

        // Attach it to the main request irp.
        //
        InsertTailList(&mainRequestContext->SubRequestList,
                       &subRequestContext->ListEntry);

        // Remember it independently so we can refer to it later without
        // walking the sub request list.
        //
        subRequestContextArray[i] = subRequestContext;

        subRequestContext->MainIrp = Irp;

        // The reference count on the sub request prevents it from being
        // freed until the completion routine for the sub request
        // executes.
        //
        subRequestContext->ReferenceCount = 1;

        //
        // 2. Allocate a sub request irp
        //

        subIrp = IoAllocateIrp(stackSize, FALSE);

        if (subIrp == NULL)
        {
            IsoUsb_DbgPrint(1, ("failed to allocate subIrp\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto PerformFullSpeedIsochTransfer_Free;
        }

        subRequestContext->SubIrp = subIrp;


        //
        // 3. Allocate a sub request urb.
        //

        nPackets = (stageSize + packetSize - 1) / packetSize;

        IsoUsb_DbgPrint(3, ("nPackets = %d for Irp/URB pair %d\n", nPackets, i));

        ASSERT(nPackets <= 255);

        urbSize = GET_ISO_URB_SIZE(nPackets);

        subUrb = (PURB)ExAllocatePool(NonPagedPool, urbSize);

        if (subUrb == NULL)
        {
            IsoUsb_DbgPrint(1, ("failed to allocate subUrb\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto PerformFullSpeedIsochTransfer_Free;
        }

        subRequestContext->SubUrb = subUrb;

        //
        // 4. Allocate a sub request mdl.
        //
        subMdl = IoAllocateMdl((PVOID) virtualAddress,
                               stageSize,
                               FALSE,
                               FALSE,
                               NULL);

        if (subMdl == NULL)
        {
            IsoUsb_DbgPrint(1, ("failed to allocate subMdl\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto PerformFullSpeedIsochTransfer_Free;
        }

        subRequestContext->SubMdl = subMdl;

        IoBuildPartialMdl(Irp->MdlAddress,
                          subMdl,
                          (PVOID)virtualAddress,
                          stageSize);

        // Update loop variables for next iteration.
        //
        virtualAddress += stageSize;

        TotalLength    -= stageSize;

        //
        // Initialize the sub request urb.
        //
        RtlZeroMemory(subUrb, urbSize);

        subUrb->UrbIsochronousTransfer.Hdr.Length = (USHORT)urbSize;

        subUrb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;

        subUrb->UrbIsochronousTransfer.PipeHandle = PipeInformation->PipeHandle;

        if (read)
        {
            IsoUsb_DbgPrint(3, ("read\n"));

            subUrb->UrbIsochronousTransfer.TransferFlags =
                USBD_TRANSFER_DIRECTION_IN;
        }
        else
        {
            IsoUsb_DbgPrint(3, ("write\n"));

            subUrb->UrbIsochronousTransfer.TransferFlags =
                USBD_TRANSFER_DIRECTION_OUT;
        }

        subUrb->UrbIsochronousTransfer.TransferBufferLength = stageSize;

        subUrb->UrbIsochronousTransfer.TransferBufferMDL = subMdl;

/*
        This is a way to set the start frame and NOT specify ASAP flag.

        subUrb->UrbIsochronousTransfer.StartFrame =
                        IsoUsb_GetCurrentFrame(DeviceObject, Irp) +
                        SOME_LATENCY;
*/
        //
        // when the client driver sets the ASAP flag, it basically
        // guarantees that it will make data available to the HC
        // and that the HC should transfer it in the next transfer frame
        // for the endpoint.(The HC maintains a next transfer frame
        // state variable for each endpoint). By resetting the pipe,
        // we make the pipe as virgin. If the data does not get to the HC
        // fast enough, the USBD_ISO_PACKET_DESCRIPTOR - Status is
        // USBD_STATUS_BAD_START_FRAME on uhci. On ohci it is 0xC000000E.
        //

        subUrb->UrbIsochronousTransfer.TransferFlags |=
            USBD_START_ISO_TRANSFER_ASAP;

        subUrb->UrbIsochronousTransfer.NumberOfPackets = nPackets;

        //
        // Set the offsets for every packet for reads/writes
        //
        if (read)
        {
            offset = 0;

            for (j = 0; j < nPackets; j++)
            {
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = 0;

                if (stageSize > packetSize)
                {
                    offset    += packetSize;
                    stageSize -= packetSize;
                }
                else
                {
                    offset    += stageSize;
                    stageSize  = 0;
                }
            }
        }
        else
        {
            offset = 0;

            for (j = 0; j < nPackets; j++)
            {
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;

                if (stageSize > packetSize)
                {
                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = packetSize;
                    offset    += packetSize;
                    stageSize -= packetSize;
                }
                else
                {
                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = stageSize;
                    offset    += stageSize;
                    stageSize  = 0;

                    ASSERT(offset == (subUrb->UrbIsochronousTransfer.IsoPacket[j].Length +
                                      subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset));
                }
            }
        }

        // Initialize the sub irp stack location
        //
        nextStack = IoGetNextIrpStackLocation(subIrp);

        nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

        nextStack->Parameters.Others.Argument1 = (PVOID) subUrb;

        nextStack->Parameters.DeviceIoControl.IoControlCode =
            IOCTL_INTERNAL_USB_SUBMIT_URB;

        IoSetCompletionRoutine(subIrp,
                               (PIO_COMPLETION_ROUTINE)IsoUsb_SubRequestComplete,
                               (PVOID)subRequestContext,
                               TRUE,
                               TRUE,
                               TRUE);

        // Update loop variables for next iteration.
        //
        if (TotalLength > (packetSize * 255))
        {
            stageSize = packetSize * 255;
        }
        else
        {
            stageSize = TotalLength;
        }
    }

    //
    // While we were busy create subsidiary irp/urb pairs..
    // the main read/write irp may have been cancelled !!
    //

    if (!Irp->Cancel)
    {
        //
        // normal processing
        //

        IsoUsb_DbgPrint(3, ("normal processing\n"));

        IoMarkIrpPending(Irp);

        // The cancel routine might run simultaneously as soon as it is
        // set.  Do not access the main request Irp in any way after
        // setting the cancel routine.
        //
        // Note that it is still safe to access the sub requests up to
        // the point where each sub request is called down the driver
        // stack due to the sub request reference count which must be
        // decremented by the completion routine.  Do not access a sub
        // request in any way after it is called down the driver stack.
        //
        // After setting the main request Irp cancel routine we are
        // committed to calling each of the sub requests down the
        // driver stack.
        //
        IoSetCancelRoutine(Irp, IsoUsb_CancelReadWrite);

        for (i = 0; i < numIrps; i++)
        {
            subRequestContext = subRequestContextArray[i];

            IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer::"));

            // Increment the device object reference before this sub
            // request is called down the driver stack.  The reference
            // will be decremented in either the sub request completion
            // routine or in the main request cancel routine, at the
            // time when the sub request is freed.
            //
            IsoUsb_IoIncrement(deviceExtension);

            IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                         subRequestContext->SubIrp);
        }

        // The sub requests are freed in either the sub request
        // completion routine or in the main request Irp cancel routine.
        //
        // Main request Irp is completed only in sub request completion
        // routine.

        ExFreePool(subRequestContextArray);

        return STATUS_PENDING;
    }
    else
    {
        //
        // The Cancel flag for the Irp has been set.
        //
        IsoUsb_DbgPrint(3, ("Cancel flag set\n"));

        ntStatus = STATUS_CANCELLED;
    }

    //
    // Resource allocation failure, or the main request Irp was
    // cancelled before the cancel routine was set.  Free any resource
    // allocations and complete the main request Irp.
    //
    // No sub requests were ever called down the driver stack in this
    // case.
    //

PerformFullSpeedIsochTransfer_Free:

    if (subRequestContextArray != NULL)
    {
        for (i = 0; i < numIrps; i++)
        {
            subRequestContext = subRequestContextArray[i];

            if (subRequestContext != NULL)
            {
                if (subRequestContext->SubIrp != NULL)
                {
                    IoFreeIrp(subRequestContext->SubIrp);
                }

                if (subRequestContext->SubUrb != NULL)
                {
                    ExFreePool(subRequestContext->SubUrb);
                }

                if (subRequestContext->SubMdl != NULL)
                {
                    IoFreeMdl(subRequestContext->SubMdl);
                }

                ExFreePool(subRequestContext);
            }
        }

        ExFreePool(subRequestContextArray);
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("PerformFullSpeedIsochTransfer::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("-------------------------------\n"));

    return ntStatus;
}


NTSTATUS
PerformHighSpeedIsochTransfer(
    IN PDEVICE_OBJECT         DeviceObject,
    IN PUSBD_PIPE_INFORMATION PipeInformation,
    IN PIRP                   Irp,
    IN ULONG                  TotalLength
    )
/*++

Routine Description:

    High Speed Isoch Transfer requires packets in multiples of 8.
    (Argument: 8 micro-frames per ms frame)
    Another restriction is that each Irp/Urb pair can be associated
    with a max of 1024 packets.

    Here is one of the ways of creating Irp/Urb pairs.
    Depending on the characteristics of real-world device,
    the algorithm may be different

    This algorithm will distribute data evenly among all the packets.

    Input:
    TotalLength - no. of bytes to be transferred.

    Other parameters:
    packetSize - max size of each packet for this pipe.

    Implementation Details:

    Step 1:
    ASSERT(TotalLength >= 8)

    Step 2:
    Find the exact number of packets required to transfer all of this data

    numberOfPackets = (TotalLength + packetSize - 1) / packetSize

    Step 3:
    Number of packets in multiples of 8.

    if(0 == (numberOfPackets % 8)) {

        actualPackets = numberOfPackets;
    }
    else {

        actualPackets = numberOfPackets +
                        (8 - (numberOfPackets % 8));
    }

    Step 4:
    Determine the min. data in each packet.

    minDataInEachPacket = TotalLength / actualPackets;

    Step 5:
    After placing min data in each packet,
    determine how much data is left to be distributed.

    dataLeftToBeDistributed = TotalLength -
                              (minDataInEachPacket * actualPackets);

    Step 6:
    Start placing the left over data in the packets
    (above the min data already placed)

    numberOfPacketsFilledToBrim = dataLeftToBeDistributed /
                                  (packetSize - minDataInEachPacket);

    Step 7:
    determine if there is any more data left.

    dataLeftToBeDistributed -= (numberOfPacketsFilledToBrim *
                                (packetSize - minDataInEachPacket));

    Step 8:
    The "dataLeftToBeDistributed" is placed in the packet at index
    "numberOfPacketsFilledToBrim"

    Algorithm at play:

    TotalLength  = 8193
    packetSize   = 8
    Step 1

    Step 2
    numberOfPackets = (8193 + 8 - 1) / 8 = 1025

    Step 3
    actualPackets = 1025 + 7 = 1032

    Step 4
    minDataInEachPacket = 8193 / 1032 = 7 bytes

    Step 5
    dataLeftToBeDistributed = 8193 - (7 * 1032) = 969.

    Step 6
    numberOfPacketsFilledToBrim = 969 / (8 - 7) = 969.

    Step 7
    dataLeftToBeDistributed = 969 - (969 * 1) = 0.

    Step 8
    Done :)

    Another algorithm
    Completely fill up (as far as possible) the early packets.
    Place 1 byte each in the rest of them.
    Ensure that the total number of packets is multiple of 8.

    This routine the does the following:

    1. It creates a SUB_REQUEST_CONTEXT for each irp/urb pair and
       attaches it to the main request irp.

    2. It intializes all of the sub request irp/urb pairs, and sub mdls
       too.

    3. It passes down the driver stack all of the sub request irps.

    4. It leaves the completion of the main request irp as the
       responsibility of the sub request irp completion routine, except
       in the exception case where the main request irp is canceled
       prior to passing any of the the sub request irps down the driver
       stack.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      irpStack;
    BOOLEAN                 read;
    ULONG                   packetSize;
    ULONG                   numberOfPackets;
    ULONG                   actualPackets;
    ULONG                   minDataInEachPacket;
    ULONG                   numberOfPacketsFilledToBrim;
    ULONG                   dataLeftToBeDistributed;
    ULONG                   numIrps;
    PMAIN_REQUEST_CONTEXT   mainRequestContext;
    PSUB_REQUEST_CONTEXT *  subRequestContextArray;
    PSUB_REQUEST_CONTEXT    subRequestContext;
    PLIST_ENTRY             subRequestEntry;
    CCHAR                   stackSize;
    PUCHAR                  virtualAddress;
    ULONG                   i;
    ULONG                   j;
    KIRQL                   oldIrql;
    NTSTATUS                ntStatus;
    PIO_STACK_LOCATION      nextStack;

    //
    // initialize vars
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    read = (irpStack->MajorFunction == IRP_MJ_READ) ? TRUE : FALSE;

    if (TotalLength < 8)
    {
        ntStatus = STATUS_INVALID_PARAMETER;

        goto PerformHighSpeedIsochTransfer_Exit;
    }

    //
    // each packet can hold this much info
    //
    packetSize = PipeInformation->MaximumPacketSize;

    numberOfPackets = (TotalLength + packetSize - 1) / packetSize;

    if (0 == (numberOfPackets % 8))
    {
        actualPackets = numberOfPackets;
    }
    else
    {
        //
        // we need multiple of 8 packets only.
        //
        actualPackets = numberOfPackets +
                        (8 - (numberOfPackets % 8));
    }

    minDataInEachPacket = TotalLength / actualPackets;

    if (minDataInEachPacket == packetSize)
    {
        numberOfPacketsFilledToBrim = actualPackets;
        dataLeftToBeDistributed     = 0;

        IsoUsb_DbgPrint(1, ("TotalLength = %d\n", TotalLength));
        IsoUsb_DbgPrint(1, ("PacketSize  = %d\n", packetSize));
        IsoUsb_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                            numberOfPacketsFilledToBrim,
                            packetSize));
    }
    else
    {
        dataLeftToBeDistributed = TotalLength -
                              (minDataInEachPacket * actualPackets);

        numberOfPacketsFilledToBrim = dataLeftToBeDistributed /
                                  (packetSize - minDataInEachPacket);

        dataLeftToBeDistributed -= (numberOfPacketsFilledToBrim *
                                (packetSize - minDataInEachPacket));


        IsoUsb_DbgPrint(1, ("TotalLength = %d\n", TotalLength));
        IsoUsb_DbgPrint(1, ("PacketSize  = %d\n", packetSize));
        IsoUsb_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                            numberOfPacketsFilledToBrim,
                            packetSize));

        if (dataLeftToBeDistributed)
        {
            IsoUsb_DbgPrint(1, ("One packet has %d bytes\n",
                                minDataInEachPacket + dataLeftToBeDistributed));
            IsoUsb_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                                actualPackets - (numberOfPacketsFilledToBrim + 1),
                                minDataInEachPacket));
        }
        else
        {
            IsoUsb_DbgPrint(1, ("Each of %d packets has %d bytes\n",
                                actualPackets - numberOfPacketsFilledToBrim,
                                minDataInEachPacket));
        }
    }

    //
    // determine how many stages of transfer needs to be done.
    // in other words, how many irp/urb pairs required.
    // this irp/urb pair is also called the subsidiary irp/urb pair
    //
    numIrps = (actualPackets + 1023) / 1024;

    IsoUsb_DbgPrint(1, ("PeformHighSpeedIsochTransfer::numIrps = %d\n", numIrps));


    // Initialize the main request Irp read/write context, which is
    // overlaid on top of Irp->Tail.Overlay.DriverContext.
    //
    mainRequestContext = (PMAIN_REQUEST_CONTEXT)
                         Irp->Tail.Overlay.DriverContext;

    InitializeListHead(&mainRequestContext->SubRequestList);

    stackSize = deviceExtension->TopOfStackDeviceObject->StackSize;

    virtualAddress = (PUCHAR) MmGetMdlVirtualAddress(Irp->MdlAddress);


    // Allocate an array to keep track of the sub requests that will be
    // allocated below.  This array exists only during the execution of
    // this routine and is used only to keep track of the sub requests
    // before calling them down the driver stack.
    //
    subRequestContextArray = (PSUB_REQUEST_CONTEXT *)
                             ExAllocatePool(NonPagedPool,
                                            numIrps * sizeof(PSUB_REQUEST_CONTEXT));

    if (subRequestContextArray == NULL)
    {
        IsoUsb_DbgPrint(1, ("failed to allocate subRequestContextArray\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        goto PerformHighSpeedIsochTransfer_Free;
    }

    RtlZeroMemory(subRequestContextArray, numIrps * sizeof(PSUB_REQUEST_CONTEXT));

    for (i = 0; i < numIrps; i++)
    {
        PIRP    subIrp;
        PURB    subUrb;
        PMDL    subMdl;
        ULONG   nPackets;
        ULONG   urbSize;
        ULONG   stageSize;
        ULONG   offset;

        // The following outer scope variables are updated during each
        // iteration of the loop:  virtualAddress, TotalLength,
        // actualPackets, numberOfPacketsFilledToBrim,
        // dataLeftToBeDistributed

        //
        // For every stage of transfer we need to do the following
        // tasks:
        //
        // 1. Allocate a sub request context (SUB_REQUEST_CONTEXT).
        // 2. Allocate a sub request irp.
        // 3. Allocate a sub request urb.
        // 4. Allocate a sub request mdl.
        // 5. Initialize the above allocations.
        //

        //
        // 1. Allocate a Sub Request Context (SUB_REQUEST_CONTEXT)
        //

        subRequestContext = (PSUB_REQUEST_CONTEXT)
                            ExAllocatePool(NonPagedPool,
                                           sizeof(SUB_REQUEST_CONTEXT));

        if (subRequestContext == NULL)
        {
            IsoUsb_DbgPrint(1, ("failed to allocate subRequestContext\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto PerformHighSpeedIsochTransfer_Free;
        }

        RtlZeroMemory(subRequestContext, sizeof(SUB_REQUEST_CONTEXT));

        // Attach it to the main request irp.
        //
        InsertTailList(&mainRequestContext->SubRequestList,
                       &subRequestContext->ListEntry);

        // Remember it independently so we can refer to it later without
        // walking the sub request list.
        //
        subRequestContextArray[i] = subRequestContext;

        subRequestContext->MainIrp = Irp;

        // The reference count on the sub request prevents it from being
        // freed until the completion routine for the sub request
        // executes.
        //
        subRequestContext->ReferenceCount = 1;

        //
        // 2. Allocate a sub request irp
        //

        subIrp = IoAllocateIrp(stackSize, FALSE);

        if (subIrp == NULL)
        {
            IsoUsb_DbgPrint(1, ("failed to allocate subIrp\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto PerformHighSpeedIsochTransfer_Free;
        }

        subRequestContext->SubIrp = subIrp;

        //
        // 3. Allocate a sub request urb.
        //

        if (actualPackets <= 1024)
        {
            nPackets = actualPackets;
            actualPackets = 0;
        }
        else
        {
            nPackets = 1024;
            actualPackets -= 1024;
        }

        IsoUsb_DbgPrint(1, ("nPackets = %d for Irp/URB pair %d\n", nPackets, i));

        ASSERT(nPackets <= 1024);

        urbSize = GET_ISO_URB_SIZE(nPackets);

        subUrb = (PURB)ExAllocatePool(NonPagedPool, urbSize);

        if (subUrb == NULL)
        {
            IsoUsb_DbgPrint(1, ("failed to allocate subUrb\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto PerformHighSpeedIsochTransfer_Free;
        }

        subRequestContext->SubUrb = subUrb;

        //
        // 4. Compute stageSize and allocate a sub request mdl.
        //

        if (nPackets > numberOfPacketsFilledToBrim)
        {
            stageSize =  packetSize * numberOfPacketsFilledToBrim;

            stageSize += (minDataInEachPacket *
                          (nPackets - numberOfPacketsFilledToBrim));

            stageSize += dataLeftToBeDistributed;
        }
        else
        {
            stageSize = packetSize * nPackets;
        }

        subMdl = IoAllocateMdl((PVOID) virtualAddress,
                               stageSize,
                               FALSE,
                               FALSE,
                               NULL);

        if (subMdl == NULL)
        {
            IsoUsb_DbgPrint(1, ("failed to allocate subMdl\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto PerformHighSpeedIsochTransfer_Free;
        }

        subRequestContext->SubMdl = subMdl;

        IoBuildPartialMdl(Irp->MdlAddress,
                          subMdl,
                          (PVOID) virtualAddress,
                          stageSize);

        // Update loop variables for next iteration.
        //
        virtualAddress += stageSize;

        TotalLength    -= stageSize;

        //
        // Initialize the sub request urb.
        //
        RtlZeroMemory(subUrb, urbSize);

        subUrb->UrbIsochronousTransfer.Hdr.Length = (USHORT)urbSize;

        subUrb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;

        subUrb->UrbIsochronousTransfer.PipeHandle = PipeInformation->PipeHandle;

        if (read)
        {
            IsoUsb_DbgPrint(1, ("read\n"));

            subUrb->UrbIsochronousTransfer.TransferFlags =
                USBD_TRANSFER_DIRECTION_IN;
        }
        else
        {
            IsoUsb_DbgPrint(1, ("write\n"));

            subUrb->UrbIsochronousTransfer.TransferFlags =
                USBD_TRANSFER_DIRECTION_OUT;
        }

        subUrb->UrbIsochronousTransfer.TransferBufferLength = stageSize;

        subUrb->UrbIsochronousTransfer.TransferBufferMDL = subMdl;

/*
        This is a way to set the start frame and NOT specify ASAP flag.

        subUrb->UrbIsochronousTransfer.StartFrame =
                        IsoUsb_GetCurrentFrame(DeviceObject, Irp) +
                        SOME_LATENCY;
*/
        subUrb->UrbIsochronousTransfer.TransferFlags |=
            USBD_START_ISO_TRANSFER_ASAP;

        subUrb->UrbIsochronousTransfer.NumberOfPackets = nPackets;

        //
        // set the offsets for every packet for reads/writes
        //
        if (read)
        {
            offset = 0;

            for (j = 0; j < nPackets; j++)
            {
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = 0;

                if (numberOfPacketsFilledToBrim)
                {
                    offset += packetSize;
                    numberOfPacketsFilledToBrim--;
                    stageSize -= packetSize;
                }
                else if (dataLeftToBeDistributed)
                {
                    offset += (minDataInEachPacket + dataLeftToBeDistributed);
                    stageSize -= (minDataInEachPacket + dataLeftToBeDistributed);
                    dataLeftToBeDistributed = 0;
                }
                else
                {
                    offset += minDataInEachPacket;
                    stageSize -= minDataInEachPacket;
                }
            }

            ASSERT(stageSize == 0);
        }
        else
        {
            offset = 0;

            for (j = 0; j < nPackets; j++)
            {
                subUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = offset;

                if (numberOfPacketsFilledToBrim)
                {
                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = packetSize;
                    offset += packetSize;
                    numberOfPacketsFilledToBrim--;
                    stageSize -= packetSize;
                }
                else if (dataLeftToBeDistributed)
                {
                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length =
                                        minDataInEachPacket + dataLeftToBeDistributed;
                    offset += (minDataInEachPacket + dataLeftToBeDistributed);
                    stageSize -= (minDataInEachPacket + dataLeftToBeDistributed);
                    dataLeftToBeDistributed = 0;
                }
                else
                {
                    subUrb->UrbIsochronousTransfer.IsoPacket[j].Length = minDataInEachPacket;
                    offset += minDataInEachPacket;
                    stageSize -= minDataInEachPacket;
                }
            }

            ASSERT(stageSize == 0);
        }

        // Initialize the sub irp stack location
        //
        nextStack = IoGetNextIrpStackLocation(subIrp);

        nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

        nextStack->Parameters.Others.Argument1 = (PVOID) subUrb;

        nextStack->Parameters.DeviceIoControl.IoControlCode =
            IOCTL_INTERNAL_USB_SUBMIT_URB;

        IoSetCompletionRoutine(subIrp,
                               (PIO_COMPLETION_ROUTINE)IsoUsb_SubRequestComplete,
                               (PVOID)subRequestContext,
                               TRUE,
                               TRUE,
                               TRUE);
    }

    //
    // While we were busy create subsidiary irp/urb pairs..
    // the main read/write irp may have been cancelled !!
    //

    if (!Irp->Cancel)
    {
        //
        // normal processing
        //

        IsoUsb_DbgPrint(3, ("normal processing\n"));

        IoMarkIrpPending(Irp);

        // The cancel routine might run simultaneously as soon as it is
        // set.  Do not access the main request Irp in any way after
        // setting the cancel routine.
        //
        // Note that it is still safe to access the sub requests up to
        // the point where each sub request is called down the driver
        // stack due to the sub request reference count which must be
        // decremented by the completion routine.  Do not access a sub
        // request in any way after it is called down the driver stack.
        //
        // After setting the main request Irp cancel routine we are
        // committed to calling each of the sub requests down the
        // driver stack.
        //
        IoSetCancelRoutine(Irp, IsoUsb_CancelReadWrite);

        for (i = 0; i < numIrps; i++)
        {
            subRequestContext = subRequestContextArray[i];

            IsoUsb_DbgPrint(3, ("PerformHighSpeedIsochTransfer::"));

            // Increment the device object reference before this sub
            // request is called down the driver stack.  The reference
            // will be decremented in either the sub request completion
            // routine or in the main request cancel routine, at the
            // time when the sub request is freed.
            //
            IsoUsb_IoIncrement(deviceExtension);

            IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                         subRequestContext->SubIrp);
        }

        // The sub requests are freed in either the sub request
        // completion routine or in the main request Irp cancel routine.
        //
        // Main request Irp is completed only in sub request completion
        // routine.

        ExFreePool(subRequestContextArray);

        return STATUS_PENDING;
    }
    else
    {
        //
        // The Cancel flag for the Irp has been set.
        //
        IsoUsb_DbgPrint(3, ("Cancel flag set\n"));

        ntStatus = STATUS_CANCELLED;
    }

    //
    // Resource allocation failure, or the main request Irp was
    // cancelled before the cancel routine was set.  Free any resource
    // allocations and complete the main request Irp.
    //
    // No sub requests were ever called down the driver stack in this
    // case.
    //

PerformHighSpeedIsochTransfer_Free:

    if (subRequestContextArray != NULL)
    {
        for (i = 0; i < numIrps; i++)
        {
            subRequestContext = subRequestContextArray[i];

            if (subRequestContext != NULL)
            {
                if (subRequestContext->SubIrp != NULL)
                {
                    IoFreeIrp(subRequestContext->SubIrp);
                }

                if (subRequestContext->SubUrb != NULL)
                {
                    ExFreePool(subRequestContext->SubUrb);
                }

                if (subRequestContext->SubMdl != NULL)
                {
                    IoFreeMdl(subRequestContext->SubMdl);
                }

                ExFreePool(subRequestContext);
            }
        }

        ExFreePool(subRequestContextArray);
    }

PerformHighSpeedIsochTransfer_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("PerformHighSpeedIsochTransfer::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("-------------------------------\n"));

    return ntStatus;
}


NTSTATUS
IsoUsb_SubRequestComplete(
    IN PDEVICE_OBJECT DeviceObjectIsNULL,
    IN PIRP           Irp,
    IN PVOID          Context
    )
/*++

Routine Description:

    This routine handles the completion of a Sub Request Irp that was
    created to handle part (or all) of the transfer for a Main Request
    Irp.

    It updates the transfer length (IoStatus.Information) of the Main
    Request Irp, and completes the Main Request Irp if this Sub Request
    Irp is the final outstanding one.

    The Sub Request Irp and Sub Request Context may either be freed
    here, or by IsoUsb_CancelReadWrite(), according to which routine is
    the last one to reference the Sub Request Irp.

Arguments:

    DeviceObject - NULL as this Sub Request Irp was allocated without a
    stack location for this driver to use itself.

    Irp - Sub Request Irp

    Context - Sub Request Context (PSUB_REQUEST_CONTEXT)

Return Value:

    STATUS_MORE_PROCESSING_REQUIRED - Tells IoMgr not to free the Sub
    Request Irp as it is either explicitly freed here, or by
    IsoUsb_CancelReadWrite()

--*/
{
    PSUB_REQUEST_CONTEXT    subRequestContext;
    PURB                    subUrb;
    PIRP                    mainIrp;
    PMAIN_REQUEST_CONTEXT   mainRequestContext;
    PDEVICE_OBJECT          deviceObject;
    NTSTATUS                ntStatus;
    ULONG                   information;
    ULONG                   i;
    KIRQL                   irql;
    BOOLEAN                 completeMainRequest;

    subRequestContext = (PSUB_REQUEST_CONTEXT)Context;

    subUrb = subRequestContext->SubUrb;

    mainIrp = subRequestContext->MainIrp;

    // The main request Irp context is overlaid on top of
    // Irp->Tail.Overlay.DriverContext.  Get a pointer to it.
    //
    mainRequestContext = (PMAIN_REQUEST_CONTEXT)
                         mainIrp->Tail.Overlay.DriverContext;

    deviceObject = IoGetCurrentIrpStackLocation(mainIrp)->DeviceObject;

    ntStatus = Irp->IoStatus.Status;

    if (NT_SUCCESS(ntStatus) && USBD_SUCCESS(subUrb->UrbHeader.Status))
    {
        information = subUrb->UrbIsochronousTransfer.TransferBufferLength;

        IsoUsb_DbgPrint(1, ("TransferBufferLength = %d\n", information));
    }
    else
    {
        information = 0;

        IsoUsb_DbgPrint(1, ("read-write irp failed with status %X\n", ntStatus));
        IsoUsb_DbgPrint(1, ("urb header status %X\n", subUrb->UrbHeader.Status));
    }

    for (i = 0; i < subUrb->UrbIsochronousTransfer.NumberOfPackets; i++)
    {
        IsoUsb_DbgPrint(3, ("IsoPacket[%d].Length = %X IsoPacket[%d].Status = %X\n",
                            i,
                            subUrb->UrbIsochronousTransfer.IsoPacket[i].Length,
                            i,
                            subUrb->UrbIsochronousTransfer.IsoPacket[i].Status));
    }

    // Prevent the cancel routine from executing simultaneously
    //
    IoAcquireCancelSpinLock(&irql);

    mainIrp->IoStatus.Information += information;

    // Remove the sub request from the main request sub request list.
    //
    RemoveEntryList(&subRequestContext->ListEntry);

    // If the sub request list is now empty clear the main request
    // cancel routine and note that the main request should be
    // completed.
    //
    if (IsListEmpty(&mainRequestContext->SubRequestList))
    {
        completeMainRequest = TRUE;

        IoSetCancelRoutine(mainIrp, NULL);
    }
    else
    {
        completeMainRequest = FALSE;
    }

    // The cancel routine may now execute simultaneously, unless of
    // course the cancel routine was just cleared above.
    //
    // Do not access the main Irp or the mainRequestContext in any way
    // beyond this point, unless this is the single instance of the sub
    // request completion routine which will complete the main Irp.
    //
    IoReleaseCancelSpinLock(irql);

    if (InterlockedDecrement(&subRequestContext->ReferenceCount) == 0)
    {
        // If the reference count is now zero then the cancel routine
        // will not free the sub request.  (Either the cancel routine
        // ran and accessed the sub request and incremented the
        // reference count and the decremented it again without freeing
        // the sub request, or the cancel routine did not access the
        // sub request and can no longer access it because it has been
        // removed from the main request sub request list.)
        //
        IoFreeIrp(subRequestContext->SubIrp);

        ExFreePool(subRequestContext->SubUrb);

        IoFreeMdl(subRequestContext->SubMdl);

        ExFreePool(subRequestContext);

        // Decrement the device object reference that was incremented
        // before this sub request was called down the driver stack.
        //
        IsoUsb_IoDecrement(deviceObject->DeviceExtension);
    }
    else
    {
        // In this case the cancel routine for the main request must be
        // executing and is accessing the sub request after
        // incrementing its reference count.  When the cancel routine
        // decrements the reference count again it will take care of
        // freeing the sub request.
    }

    if (completeMainRequest)
    {
        // The final sub request for the main request has completed so
        // now complete the main request.
        //
        mainIrp->IoStatus.Status = STATUS_SUCCESS;

        IoCompleteRequest(mainIrp, IO_NO_INCREMENT);

        IsoUsb_IoDecrement(deviceObject->DeviceExtension);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}


VOID
IsoUsb_CancelReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This is the cancellation routine for the main read/write Irp.

    It cancels all currently outstanding sub requests for the main
    request.

    Completing the main request is the responsibility of the sub
    request completion routine after all outstanding sub requests have
    completed.

Return Value:

    None

--*/
{
    PMAIN_REQUEST_CONTEXT   mainRequestContext;
    LIST_ENTRY              cancelList;
    PLIST_ENTRY             subRequestEntry;
    PSUB_REQUEST_CONTEXT    subRequestContext;

    // The main request Irp context is overlaid on top of
    // Irp->Tail.Overlay.DriverContext.  Get a pointer to it.
    //
    mainRequestContext = (PMAIN_REQUEST_CONTEXT)
                         Irp->Tail.Overlay.DriverContext;

    // The mainRequestContext SubRequestList cannot be simultaneously
    // changed by anything else as long as the cancel spin lock is still
    // held, but can be changed immediately by the completion routine
    // after the cancel spin lock is released.
    //
    // Iterate over the mainRequestContext SubRequestList and add all of
    // the currently outstanding sub requests to the list of sub
    // requests to be cancelled.
    //
    InitializeListHead(&cancelList);

    subRequestEntry = mainRequestContext->SubRequestList.Flink;

    while (subRequestEntry != &mainRequestContext->SubRequestList)
    {
        subRequestContext = CONTAINING_RECORD(subRequestEntry,
                                              SUB_REQUEST_CONTEXT,
                                              ListEntry);

        // Prevent the sub request from being freed as soon as the
        // cancel spin lock is released by incrementing the reference
        // count on the sub request.
        //
        InterlockedIncrement(&subRequestContext->ReferenceCount);

        InsertTailList(&cancelList, &subRequestContext->CancelListEntry);

        subRequestEntry = subRequestEntry->Flink;
    }

    // The main read/write Irp can be completed immediately after
    // releasing the cancel spin lock.  Do not access the main
    // read/write Irp or the mainRequestContext in any way beyond this
    // point.
    //
    IoReleaseCancelSpinLock(Irp->CancelIrql);



    // Iterate over the list that was built of sub requests to cancel
    // and cancel each sub request.
    //
    while (!IsListEmpty(&cancelList))
    {
        subRequestEntry = RemoveHeadList(&cancelList);

        subRequestContext = CONTAINING_RECORD(subRequestEntry,
                                              SUB_REQUEST_CONTEXT,
                                              CancelListEntry);

        IoCancelIrp(subRequestContext->SubIrp);


        if (InterlockedDecrement(&subRequestContext->ReferenceCount) == 0)
        {
            // If the reference count is now zero then the completion
            // routine already ran for the sub request but did not free
            // the sub request so it can be freed now.
            //
            IoFreeIrp(subRequestContext->SubIrp);

            ExFreePool(subRequestContext->SubUrb);

            IoFreeMdl(subRequestContext->SubMdl);

            ExFreePool(subRequestContext);

            // Decrement the device object reference that was
            // incremented before this sub request was called down the
            // driver stack.
            //
            IsoUsb_IoDecrement(DeviceObject->DeviceExtension);
        }
        else
        {
            // The completion routine for the sub request has not yet
            // executed and decremented the sub request reference count.
            // Do not free the sub request here.  It will be freed when
            // the sub request completion routine executes.
        }
    }
}


ULONG
IsoUsb_GetCurrentFrame(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine send an irp/urb pair with
    function code URB_FUNCTION_GET_CURRENT_FRAME_NUMBER
    to fetch the current frame

Arguments:

    DeviceObject - pointer to device object
    PIRP - I/O request packet

Return Value:

    Current frame

--*/
{
    KEVENT                               event;
    PDEVICE_EXTENSION                    deviceExtension;
    PIO_STACK_LOCATION                   nextStack;
    struct _URB_GET_CURRENT_FRAME_NUMBER urb;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    //
    // initialize the urb
    //

    IsoUsb_DbgPrint(3, ("IsoUsb_GetCurrentFrame - begins\n"));

    urb.Hdr.Function = URB_FUNCTION_GET_CURRENT_FRAME_NUMBER;
    urb.Hdr.Length = sizeof(urb);
    urb.FrameNumber = (ULONG) -1;

    nextStack = IoGetNextIrpStackLocation(Irp);
    nextStack->Parameters.Others.Argument1 = (PVOID) &urb;
    nextStack->Parameters.DeviceIoControl.IoControlCode =
                                IOCTL_INTERNAL_USB_SUBMIT_URB;
    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    KeInitializeEvent(&event,
                      NotificationEvent,
                      FALSE);

    IoSetCompletionRoutine(Irp,
                           IsoUsb_StopCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    IsoUsb_DbgPrint(3, ("IsoUsb_GetCurrentFrame::"));
    IsoUsb_IoIncrement(deviceExtension);

    IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                 Irp);

    KeWaitForSingleObject((PVOID) &event,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    IsoUsb_DbgPrint(3, ("IsoUsb_GetCurrentFrame::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("IsoUsb_GetCurrentFrame - ends\n"));

    return urb.FrameNumber;
}


NTSTATUS
IsoUsb_StopCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN PVOID          Context
    )
/*++

Routine Description:

    This is the completion routine for request to retrieve the frame number

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet
    Context - context passed to the completion routine

Return Value:

    NT status value

--*/
{
    PKEVENT event;

    IsoUsb_DbgPrint(3, ("IsoUsb_StopCompletion - begins\n"));

    event = (PKEVENT) Context;

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    IsoUsb_DbgPrint(3, ("IsoUsb_StopCompletion - ends\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;
}

