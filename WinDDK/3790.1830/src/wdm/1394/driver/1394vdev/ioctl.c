/*++

Copyright (c) 1998  Microsoft Corporation

Module Name: 

    ioctl.c

Abstract


Author:

    Peter Binder (pbinder) 4/13/98

Revision History:
Date     Who       What
-------- --------- ------------------------------------------------------------
4/13/98  pbinder   taken from 1394diag/ohcidiag
--*/

#include "pch.h"

NTSTATUS
t1394VDev_IoControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS                ntStatus = STATUS_SUCCESS;
    PIO_STACK_LOCATION      IrpSp;
    PDEVICE_EXTENSION       deviceExtension;
    PVOID                   ioBuffer;
    ULONG                   inputBufferLength;
    ULONG                   outputBufferLength;
    ULONG                   ioControlCode;
    
    ENTER("t1394VDev_IoControl");

    // Get a pointer to the current location in the Irp. This is where
    // the function codes and parameters are located.
    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    // Get a pointer to the device extension
    deviceExtension = DeviceObject->DeviceExtension;

    // Get the pointer to the input/output buffer and it's length
    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    // make sure our device isn't in shutdown mode...
    if (deviceExtension->bShutdown) {

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        ntStatus = STATUS_NO_SUCH_DEVICE;
        return(ntStatus);
    }

    TRACE(TL_TRACE, ("Irp = 0x%x\n", Irp));

    switch (IrpSp->MajorFunction) {

        case IRP_MJ_DEVICE_CONTROL:
            TRACE(TL_TRACE, ("t1394VDev_IoControl: IRP_MJ_DEVICE_CONTROL\n"));

            ioControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

            switch (ioControlCode) {

                case IOCTL_ALLOCATE_ADDRESS_RANGE:
                    {
                        PALLOCATE_ADDRESS_RANGE     AllocateAddressRange;

                        TRACE(TL_TRACE, ("IOCTL_ALLOCATE_ADDRESS_RANGE\n"));

                        if ((inputBufferLength < sizeof(ALLOCATE_ADDRESS_RANGE)) ||
                            (outputBufferLength < sizeof(ALLOCATE_ADDRESS_RANGE))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            AllocateAddressRange = (PALLOCATE_ADDRESS_RANGE)ioBuffer;

                            ntStatus = t1394_AllocateAddressRange( DeviceObject,
                                                                       Irp,
                                                                       AllocateAddressRange->fulAllocateFlags,
                                                                       AllocateAddressRange->fulFlags,
                                                                       AllocateAddressRange->nLength,
                                                                       AllocateAddressRange->MaxSegmentSize,
                                                                       AllocateAddressRange->fulAccessType,
                                                                       AllocateAddressRange->fulNotificationOptions,
                                                                       &AllocateAddressRange->Required1394Offset,
                                                                       &AllocateAddressRange->hAddressRange,
                                                                       (PULONG)&AllocateAddressRange->Data
                                                                       );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_ALLOCATE_ADDRESS_RANGE

                case IOCTL_FREE_ADDRESS_RANGE:
                    TRACE(TL_TRACE, ("IOCTL_FREE_ADDRESS_RANGE\n"));

                    if (inputBufferLength < sizeof(HANDLE)) {

                        ntStatus = STATUS_BUFFER_TOO_SMALL;
                    }
                    else {

                        ntStatus = t1394_FreeAddressRange( DeviceObject,
                                                               Irp,
                                                               *(PHANDLE)ioBuffer
                                                               );
                    }
                    break; // IOCTL_FREE_ADDRESS_RANGE

                case IOCTL_ASYNC_READ:
                    {
                        PASYNC_READ     AsyncRead;

                        TRACE(TL_TRACE, ("IOCTL_ASYNC_READ\n"));

                        if (inputBufferLength < sizeof(ASYNC_READ)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            AsyncRead = (PASYNC_READ)ioBuffer;

                            if ((outputBufferLength < sizeof(ASYNC_READ)) || 
                                (outputBufferLength-sizeof(ASYNC_READ) < AsyncRead->nNumberOfBytesToRead)) {

                                ntStatus = STATUS_BUFFER_TOO_SMALL;
                            }
                            else {

                                ntStatus = t1394_AsyncRead( DeviceObject,
                                                                Irp,
                                                                AsyncRead->bRawMode,
                                                                AsyncRead->bGetGeneration,
                                                                AsyncRead->DestinationAddress,
                                                                AsyncRead->nNumberOfBytesToRead,
                                                                AsyncRead->nBlockSize,
                                                                AsyncRead->fulFlags,
                                                                AsyncRead->ulGeneration,
                                                                (PULONG)&AsyncRead->Data
                                                                );

                                if (NT_SUCCESS(ntStatus))
                                    Irp->IoStatus.Information = outputBufferLength;
                            }
                        }
                    }
                    break; // IOCTL_ASYNC_READ

                case IOCTL_ASYNC_WRITE:
                    {
                        PASYNC_WRITE    AsyncWrite;

                        TRACE(TL_TRACE, ("IOCTL_ASYNC_WRITE\n"));

                        if (inputBufferLength < sizeof(ASYNC_WRITE)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            AsyncWrite = (PASYNC_WRITE)ioBuffer;

                            if (inputBufferLength-sizeof(ASYNC_WRITE) < AsyncWrite->nNumberOfBytesToWrite) {

                                ntStatus = STATUS_BUFFER_TOO_SMALL;
                            }
                            else {

                                ntStatus = t1394_AsyncWrite( DeviceObject,
                                                                 Irp,
                                                                 AsyncWrite->bRawMode,
                                                                 AsyncWrite->bGetGeneration,
                                                                 AsyncWrite->DestinationAddress,
                                                                 AsyncWrite->nNumberOfBytesToWrite,
                                                                 AsyncWrite->nBlockSize,
                                                                 AsyncWrite->fulFlags,
                                                                 AsyncWrite->ulGeneration,
                                                                 (PULONG)&AsyncWrite->Data
                                                                 );
                            }
                        }
                    }
                    break; // IOCTL_ASYNC_WRITE

                case IOCTL_ASYNC_LOCK:
                    {
                        PASYNC_LOCK     AsyncLock;

                        TRACE(TL_TRACE, ("IOCTL_ASYNC_LOCK\n"));

                        if ((inputBufferLength < sizeof(ASYNC_LOCK)) ||
                            (outputBufferLength < sizeof(ASYNC_LOCK))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            AsyncLock = (PASYNC_LOCK)ioBuffer;

                            ntStatus = t1394_AsyncLock( DeviceObject,
                                                            Irp,
                                                            AsyncLock->bRawMode,
                                                            AsyncLock->bGetGeneration,
                                                            AsyncLock->DestinationAddress,
                                                            AsyncLock->nNumberOfArgBytes,
                                                            AsyncLock->nNumberOfDataBytes,
                                                            AsyncLock->fulTransactionType,
                                                            AsyncLock->fulFlags,
                                                            AsyncLock->Arguments,
                                                            AsyncLock->DataValues,
                                                            AsyncLock->ulGeneration,
                                                            (PVOID)&AsyncLock->Buffer
                                                            );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_ASYNC_LOCK

                case IOCTL_ASYNC_STREAM:
                    {
                        PASYNC_STREAM   AsyncStream;

                        TRACE(TL_TRACE, ("IOCTL_ASYNC_STREAM\n"));

                        if (inputBufferLength < sizeof(ASYNC_STREAM)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            AsyncStream = (PASYNC_STREAM)ioBuffer;

                            if (outputBufferLength < sizeof(ASYNC_STREAM)+AsyncStream->nNumberOfBytesToStream) {

                                ntStatus = STATUS_BUFFER_TOO_SMALL;
                            }
                            else {

                                ntStatus = t1394_AsyncStream( DeviceObject,
                                                                  Irp,
                                                                  AsyncStream->nNumberOfBytesToStream,
                                                                  AsyncStream->fulFlags,
                                                                  AsyncStream->ulTag,
                                                                  AsyncStream->nChannel,
                                                                  AsyncStream->ulSynch,
                                                                  (UCHAR)AsyncStream->nSpeed,
                                                                  (PULONG)&AsyncStream->Data
                                                                  );

                                if (NT_SUCCESS(ntStatus))
                                    Irp->IoStatus.Information = outputBufferLength;
                            }
                        }
                    }
                    break; // IOCTL_ASYNC_STREAM

                case IOCTL_ISOCH_ALLOCATE_BANDWIDTH:
                    {
                        PISOCH_ALLOCATE_BANDWIDTH   IsochAllocateBandwidth;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_ALLOCATE_BANDWIDTH\n"));

                        if ((inputBufferLength < sizeof(ISOCH_ALLOCATE_BANDWIDTH)) ||
                            (outputBufferLength < sizeof(ISOCH_ALLOCATE_BANDWIDTH))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochAllocateBandwidth = (PISOCH_ALLOCATE_BANDWIDTH)ioBuffer;

                            ntStatus = t1394_IsochAllocateBandwidth( DeviceObject,
                                                                         Irp,
                                                                         IsochAllocateBandwidth->nMaxBytesPerFrameRequested,
                                                                         IsochAllocateBandwidth->fulSpeed,
                                                                         &IsochAllocateBandwidth->hBandwidth,
                                                                         &IsochAllocateBandwidth->BytesPerFrameAvailable,
                                                                         &IsochAllocateBandwidth->SpeedSelected
                                                                         );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_ISOCH_ALLOCATE_BANDWIDTH

                case IOCTL_ISOCH_ALLOCATE_CHANNEL:
                    {
                        PISOCH_ALLOCATE_CHANNEL     IsochAllocateChannel;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_ALLOCATE_CHANNEL\n"));

                        if ((inputBufferLength < sizeof(ISOCH_ALLOCATE_CHANNEL)) ||
                            (outputBufferLength < sizeof(ISOCH_ALLOCATE_CHANNEL))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochAllocateChannel = (PISOCH_ALLOCATE_CHANNEL)ioBuffer;

                            ntStatus = t1394_IsochAllocateChannel( DeviceObject,
                                                                       Irp,
                                                                       IsochAllocateChannel->nRequestedChannel,
                                                                       &IsochAllocateChannel->Channel,
                                                                       &IsochAllocateChannel->ChannelsAvailable
                                                                       );

                        if (NT_SUCCESS(ntStatus))
                            Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_ISOCH_ALLOCATE_CHANNEL

                case IOCTL_ISOCH_ALLOCATE_RESOURCES:
                    {
                        PISOCH_ALLOCATE_RESOURCES   IsochAllocateResources;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_ALLOCATE_RESOURCES\n"));

                        if ((inputBufferLength < sizeof(ISOCH_ALLOCATE_RESOURCES)) ||
                            (outputBufferLength < sizeof(ISOCH_ALLOCATE_RESOURCES))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochAllocateResources = (PISOCH_ALLOCATE_RESOURCES)ioBuffer;

                            ntStatus = t1394_IsochAllocateResources( DeviceObject,
                                                                         Irp,
                                                                         IsochAllocateResources->fulSpeed,
                                                                         IsochAllocateResources->fulFlags,
                                                                         IsochAllocateResources->nChannel,
                                                                         IsochAllocateResources->nMaxBytesPerFrame,
                                                                         IsochAllocateResources->nNumberOfBuffers,
                                                                         IsochAllocateResources->nMaxBufferSize,
                                                                         IsochAllocateResources->nQuadletsToStrip,
                                                                         &IsochAllocateResources->hResource
                                                                         );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_ISOCH_ALLOCATE_RESOURCES

                case IOCTL_ISOCH_ATTACH_BUFFERS:
                    {
                        PISOCH_ATTACH_BUFFERS       IsochAttachBuffers	= NULL;
						PRING3_ISOCH_DESCRIPTOR		pTempR3Desc			= NULL;
						ULONG						ulBuffSize			= 0;
						ULONG						i					= 0;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_ATTACH_BUFFERS\n"));

                        if (inputBufferLength < (sizeof(ISOCH_ATTACH_BUFFERS))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {
                      
							// verify the passed in buffer is large enough to hold each r3 descriptor
							// plus the data portion
							IsochAttachBuffers  = (PISOCH_ATTACH_BUFFERS)ioBuffer;
							pTempR3Desc			= &(IsochAttachBuffers->R3_IsochDescriptor[0]);
							ulBuffSize			= sizeof (ISOCH_ATTACH_BUFFERS) - sizeof (RING3_ISOCH_DESCRIPTOR);
                            
							for (i = 0; i < IsochAttachBuffers->nNumberOfDescriptors; i++)
							{
                                // check the alignment on the pTempR3Desc
                                if ((ULONG_PTR)pTempR3Desc & 0x3)
                                {
                                    // this is a situation of a descriptor not on a 4 byte alignment, just break
                                    TRACE(TL_WARNING, ("Incorrect alignment detected, pTempR3Desc = 0x%p\n", pTempR3Desc));
                                    ntStatus = STATUS_INVALID_PARAMETER;
                                    break;
                                }

                                // verify the buffer is large enough to hold this R3 Isoch Descriptor
								// but first make sure we won't overflow
                                if ((ulBuffSize + sizeof (RING3_ISOCH_DESCRIPTOR)) < ulBuffSize)
                                {
                                    // this is an overflow situation, just set an error code and break
                                    TRACE(TL_WARNING, ("Overflow of ulBuffSize, sizeof (RING3_ISOCH_DESCRIPTOR) too large\n"));
                                    ntStatus = STATUS_INVALID_PARAMETER;
                                    break;
                                }
                                ulBuffSize += sizeof (RING3_ISOCH_DESCRIPTOR);
								if (inputBufferLength < ulBuffSize)
								{
									TRACE(TL_WARNING, ("Isoch Attach buffer request too small\n"));
									ntStatus = STATUS_BUFFER_TOO_SMALL;
									break;
								}

								// now verify the buffer is large enough to hold the data indicated
								// by the last R3 Isoch Descriptor, but first make sure we won't overflow
                                if ((ulBuffSize + pTempR3Desc->ulLength) < ulBuffSize)
                                {
                                    // this is an overflow situation, just set an error code and break
                                    TRACE(TL_WARNING, ("Overflow of ulBuffSize, R3->length too large = 0x%x\n", pTempR3Desc->ulLength));
                                    ntStatus = STATUS_INVALID_PARAMETER;
                                    break;
                                }                                
								ulBuffSize += pTempR3Desc->ulLength;
								if (inputBufferLength < ulBuffSize)
								{
									TRACE(TL_WARNING, ("Isoch Attach buffer request too small\n"));
									ntStatus = STATUS_BUFFER_TOO_SMALL;
									break;
								}

								// now increment our temp R3 descriptor so the next time through this for loop
								// we are pointing at the correct data
								pTempR3Desc = (PRING3_ISOCH_DESCRIPTOR)((ULONG_PTR)pTempR3Desc +
																		pTempR3Desc->ulLength +
																		sizeof(RING3_ISOCH_DESCRIPTOR));
							}

							// make sure we didn't fail up above
                            if (NT_SUCCESS(ntStatus))
							{
                                ntStatus = t1394_IsochAttachBuffers( DeviceObject,
                                                                         Irp,
                                                                         outputBufferLength,
                                                                         IsochAttachBuffers->hResource,
                                                                         IsochAttachBuffers->nNumberOfDescriptors,
                                                                         (PISOCH_DESCRIPTOR)IsochAttachBuffers->pIsochDescriptor,
                                                                         (PRING3_ISOCH_DESCRIPTOR)&IsochAttachBuffers->R3_IsochDescriptor
                                                                         );

                                if (STATUS_PENDING == ntStatus)
								{
                                    goto t1394VDev_IoControlExit;
								}
                            }
                        }
                    }
                    break; // IOCTL_ISOCH_ATTACH_BUFFERS

                case IOCTL_ISOCH_DETACH_BUFFERS:
                    {
                        PISOCH_DETACH_BUFFERS       IsochDetachBuffers;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_DETACH_BUFFERS\n"));

                        if (inputBufferLength < sizeof(ISOCH_DETACH_BUFFERS)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochDetachBuffers = (PISOCH_DETACH_BUFFERS)ioBuffer;

                            ntStatus = t1394_IsochDetachBuffers( DeviceObject,
                                                                     Irp,
                                                                     IsochDetachBuffers->hResource,
                                                                     IsochDetachBuffers->nNumberOfDescriptors,
                                                                     (PISOCH_DESCRIPTOR)IsochDetachBuffers->pIsochDescriptor
                                                                     );
                        }
                    }

                    break; // IOCTL_ISOCH_DETACH_BUFFERS

                case IOCTL_ISOCH_FREE_BANDWIDTH:
                    {
                        TRACE(TL_TRACE, ("IOCTL_ISOCH_FREE_BANDWIDTH\n"));

                        if (inputBufferLength < sizeof(HANDLE)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            ntStatus = t1394_IsochFreeBandwidth( DeviceObject,
                                                                     Irp,
                                                                     *(PHANDLE)ioBuffer
                                                                     );
                        }
                    }
                    break; // IOCTL_ISOCH_FREE_BANDWIDTH
  
                case IOCTL_ISOCH_FREE_CHANNEL:
                    {
                        TRACE(TL_TRACE, ("IOCTL_ISOCH_FREE_CHANNEL\n"));

                        if (inputBufferLength < sizeof(ULONG)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            ntStatus = t1394_IsochFreeChannel( DeviceObject,
                                                                   Irp,
                                                                   *(PULONG)ioBuffer
                                                                   );
                        }
                    }
                    break; // IOCTL_ISOCH_FREE_CHANNEL
  
                case IOCTL_ISOCH_FREE_RESOURCES:
                    {
                        TRACE(TL_TRACE, ("IOCTL_ISOCH_FREE_RESOURCES\n"));

                        if (inputBufferLength < sizeof(HANDLE)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            ntStatus = t1394_IsochFreeResources( DeviceObject,
                                                                     Irp,
                                                                     *(PHANDLE)ioBuffer
                                                                     );
                        }
                    }
                    break; // IOCTL_ISOCH_FREE_RESOURCES

                case IOCTL_ISOCH_LISTEN:
                    {
                        PISOCH_LISTEN       IsochListen;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_LISTEN\n"));

                        if (inputBufferLength < sizeof(ISOCH_LISTEN)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochListen = (PISOCH_LISTEN)ioBuffer;

                            ntStatus = t1394_IsochListen( DeviceObject,
                                                              Irp,
                                                              IsochListen->hResource,
                                                              IsochListen->fulFlags,
                                                              IsochListen->StartTime
                                                              );
                        }
                    }
                    break; // IOCTL_ISOCH_LISTEN

                case IOCTL_ISOCH_QUERY_CURRENT_CYCLE_TIME:
                    {
                        TRACE(TL_TRACE, ("IOCTL_ISOCH_QUERY_CURRENT_CYCLE_TIME\n"));

                        if (outputBufferLength < sizeof(CYCLE_TIME)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            ntStatus = t1394_IsochQueryCurrentCycleTime( DeviceObject,
                                                                             Irp,
                                                                             (PCYCLE_TIME)ioBuffer
                                                                             );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_ISOCH_QUERY_CURRENT_CYCLE_TIME

                case IOCTL_ISOCH_QUERY_RESOURCES:
                    {
                        PISOCH_QUERY_RESOURCES      IsochQueryResources;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_QUERY_RESOURCES\n"));

                        if ((inputBufferLength < sizeof(ISOCH_QUERY_RESOURCES)) ||
                            (outputBufferLength < sizeof(ISOCH_QUERY_RESOURCES))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochQueryResources = (PISOCH_QUERY_RESOURCES)ioBuffer;

                            ntStatus = t1394_IsochQueryResources( DeviceObject,
                                                                      Irp,
                                                                      IsochQueryResources->fulSpeed,
                                                                      &IsochQueryResources->BytesPerFrameAvailable,
                                                                      &IsochQueryResources->ChannelsAvailable
                                                                      );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_ISOCH_QUERY_RESOURCES

                case IOCTL_ISOCH_SET_CHANNEL_BANDWIDTH:
                    {
                        PISOCH_SET_CHANNEL_BANDWIDTH    IsochSetChannelBandwidth;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_SET_CHANNEL_BANDWIDTH\n"));

                        if (inputBufferLength < sizeof(ISOCH_SET_CHANNEL_BANDWIDTH)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochSetChannelBandwidth = (PISOCH_SET_CHANNEL_BANDWIDTH)ioBuffer;

                            ntStatus = t1394_IsochSetChannelBandwidth( DeviceObject,
                                                                           Irp,
                                                                           IsochSetChannelBandwidth->hBandwidth,
                                                                           IsochSetChannelBandwidth->nMaxBytesPerFrame
                                                                           );
                        }
                    }
                    break; // IOCTL_ISOCH_SET_CHANNEL_BANDWIDTH


                case IOCTL_ISOCH_MODIFY_STREAM_PROPERTIES:
                    {
                        PISOCH_MODIFY_STREAM_PROPERTIES     IsochModifyStreamProperties;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_MODIFY_STREAM_PROPERTIES\n"));

                        if (inputBufferLength < sizeof (ISOCH_MODIFY_STREAM_PROPERTIES)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochModifyStreamProperties = (PISOCH_MODIFY_STREAM_PROPERTIES)ioBuffer;

                            ntStatus = t1394_IsochModifyStreamProperties( DeviceObject,
                                                                            Irp, 
                                                                            IsochModifyStreamProperties->hResource,
                                                                            IsochModifyStreamProperties->ChannelMask,
                                                                            IsochModifyStreamProperties->fulSpeed
                                                                            );
                        }
                    }
                    break; // IOCTL_ISOCH_MODIFY_STREAM_PROPERTIES

                case IOCTL_ISOCH_STOP:
                    {
                        PISOCH_STOP     IsochStop;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_STOP\n"));

                        if (inputBufferLength < sizeof(ISOCH_STOP)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochStop = (PISOCH_STOP)ioBuffer;

                            ntStatus = t1394_IsochStop( DeviceObject,
                                                            Irp,
                                                            IsochStop->hResource,
                                                            IsochStop->fulFlags
                                                            );
                        }
                    }
                    break; // IOCTL_ISOCH_STOP

                case IOCTL_ISOCH_TALK:
                    {
                        PISOCH_TALK     IsochTalk;

                        TRACE(TL_TRACE, ("IOCTL_ISOCH_TALK\n"));

                        if (inputBufferLength < sizeof(ISOCH_TALK)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            IsochTalk = (PISOCH_TALK)ioBuffer;

                            ntStatus = t1394_IsochTalk( DeviceObject,
                                                            Irp,
                                                            IsochTalk->hResource,
                                                            IsochTalk->fulFlags,
                                                            IsochTalk->StartTime
                                                            );
                        }
                    }
                    break; // IOCTL_ISOCH_TALK

                case IOCTL_GET_LOCAL_HOST_INFORMATION:
                    {
                        PGET_LOCAL_HOST_INFORMATION     GetLocalHostInformation;

                        TRACE(TL_TRACE, ("IOCTL_GET_LOCAL_HOST_INFORMATION\n"));

                        if ((inputBufferLength < sizeof(GET_LOCAL_HOST_INFORMATION)) ||
                            (outputBufferLength < sizeof(GET_LOCAL_HOST_INFORMATION))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            GetLocalHostInformation = (PGET_LOCAL_HOST_INFORMATION)ioBuffer;

                            ntStatus = t1394_GetLocalHostInformation( DeviceObject,
                                                                          Irp,
                                                                          GetLocalHostInformation->nLevel,
                                                                          &GetLocalHostInformation->Status,
                                                                          (PVOID)GetLocalHostInformation->Information
                                                                          );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_GET_LOCAL_HOST_INFORMATION

                case IOCTL_GET_1394_ADDRESS_FROM_DEVICE_OBJECT:
                    {
                        PGET_1394_ADDRESS   Get1394Address;

                        TRACE(TL_TRACE, ("IOCTL_GET_1394_ADDRESS_FROM_DEVICE_OBJECT\n"));

                        if ((inputBufferLength < sizeof(GET_1394_ADDRESS)) ||
                            (outputBufferLength < sizeof(GET_1394_ADDRESS))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            Get1394Address = (PGET_1394_ADDRESS)ioBuffer;

                            ntStatus = t1394_Get1394AddressFromDeviceObject( DeviceObject,
                                                                                 Irp,
                                                                                 Get1394Address->fulFlags,
                                                                                 &Get1394Address->NodeAddress
                                                                                 );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_GET_1394_ADDRESS_FROM_DEVICE_OBJECT

                case IOCTL_CONTROL:
                    TRACE(TL_TRACE, ("IOCTL_CONTROL\n"));

                    ntStatus = t1394_Control( DeviceObject,
                                                  Irp
                                                  );

                    break; // IOCTL_CONTROL

                case IOCTL_GET_MAX_SPEED_BETWEEN_DEVICES:
                    {
                        PGET_MAX_SPEED_BETWEEN_DEVICES  MaxSpeedBetweenDevices;

                        TRACE(TL_TRACE, ("IOCTL_GET_MAX_SPEED_BETWEEN_DEVICES\n"));

                        if ((inputBufferLength < sizeof(GET_MAX_SPEED_BETWEEN_DEVICES)) ||
                            (outputBufferLength < sizeof(GET_MAX_SPEED_BETWEEN_DEVICES))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            MaxSpeedBetweenDevices = (PGET_MAX_SPEED_BETWEEN_DEVICES)ioBuffer;

                            ntStatus = t1394_GetMaxSpeedBetweenDevices( DeviceObject,
                                                                            Irp,
                                                                            MaxSpeedBetweenDevices->fulFlags,
                                                                            MaxSpeedBetweenDevices->ulNumberOfDestinations,
                                                                            (PDEVICE_OBJECT *)&MaxSpeedBetweenDevices->hDestinationDeviceObjects[0],
                                                                            &MaxSpeedBetweenDevices->fulSpeed
                                                                            );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }                    
                    break; // IOCTL_GET_MAX_SPEED_BETWEEN_DEVICES

                case IOCTL_SET_DEVICE_XMIT_PROPERTIES:
                    {
                        PDEVICE_XMIT_PROPERTIES     DeviceXmitProperties;

                        TRACE(TL_TRACE, ("IOCTL_SET_DEVICE_XMIT_PROPERTIES\n"));

                        if (inputBufferLength < sizeof(DEVICE_XMIT_PROPERTIES)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            DeviceXmitProperties = (PDEVICE_XMIT_PROPERTIES)ioBuffer;

                            ntStatus = t1394_SetDeviceXmitProperties( DeviceObject,
                                                                          Irp,
                                                                          DeviceXmitProperties->fulSpeed,
                                                                          DeviceXmitProperties->fulPriority
                                                                          );
                        }
                    }
                    break; // IOCTL_SET_DEVICE_XMIT_PROPERTIES

                case IOCTL_GET_CONFIGURATION_INFORMATION:
                    TRACE(TL_TRACE, ("IOCTL_GET_CONFIGURATION_INFORMATION\n"));

                    ntStatus = t1394_GetConfigurationInformation( DeviceObject,
                                                                      Irp
                                                                      );

                    break; // IOCTL_GET_CONFIGURATION_INFORMATION

                case IOCTL_BUS_RESET:
                    {
                        TRACE(TL_TRACE, ("IOCTL_BUS_RESET\n"));

                        if (inputBufferLength < sizeof(ULONG)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            ntStatus = t1394_BusReset( DeviceObject,
                                                           Irp,
                                                           *((PULONG)ioBuffer)
                                                           );
                        }
                    }
                    break; // IOCTL_BUS_RESET

                case IOCTL_GET_GENERATION_COUNT:
                    {
                        TRACE(TL_TRACE, ("IOCTL_GET_GENERATION_COUNT\n"));

                        if (outputBufferLength < sizeof(ULONG)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            ntStatus = t1394_GetGenerationCount( DeviceObject,
                                                                     Irp,
                                                                     (PULONG)ioBuffer
                                                                     );

                            if (NT_SUCCESS(ntStatus))
                                Irp->IoStatus.Information = outputBufferLength;
                        }
                    }
                    break; // IOCTL_GET_GENERATION_COUNT

                case IOCTL_SEND_PHY_CONFIGURATION_PACKET:
                    {
                        TRACE(TL_TRACE, ("IOCTL_SEND_PHY_CONFIGURATION_PACKET\n"));

                        if (inputBufferLength < sizeof(PHY_CONFIGURATION_PACKET)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            ntStatus = t1394_SendPhyConfigurationPacket( DeviceObject,
                                                                             Irp,
                                                                             *(PPHY_CONFIGURATION_PACKET)ioBuffer
                                                                             );
                        }
                    }
                    break; // IOCTL_SEND_PHY_CONFIGURATION_PACKET

                case IOCTL_BUS_RESET_NOTIFICATION:
                    {
                        TRACE(TL_TRACE, ("IOCTL_BUS_RESET_NOTIFICATION\n"));

                        if (inputBufferLength < sizeof(ULONG)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            ntStatus = t1394_BusResetNotification( DeviceObject,
                                                                       Irp,
                                                                       *((PULONG)ioBuffer)
                                                                       );
                        }
                    }
                    break; // IOCTL_BUS_RESET_NOTIFICATION

                case IOCTL_SET_LOCAL_HOST_INFORMATION:
                    {
                        PSET_LOCAL_HOST_INFORMATION     SetLocalHostInformation;

                        TRACE(TL_TRACE, ("IOCTL_SET_LOCAL_HOST_INFORMATION\n"));

                        if (inputBufferLength < sizeof(SET_LOCAL_HOST_INFORMATION)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            SetLocalHostInformation = (PSET_LOCAL_HOST_INFORMATION)ioBuffer;

                            if (inputBufferLength < (sizeof(SET_LOCAL_HOST_INFORMATION) +
                                                     SetLocalHostInformation->ulBufferSize)) {

                                ntStatus = STATUS_BUFFER_TOO_SMALL;
                            }
                            else {

                                ntStatus = t1394_SetLocalHostProperties( DeviceObject,
                                                                             Irp,
                                                                             SetLocalHostInformation->nLevel,
                                                                             (PVOID)&SetLocalHostInformation->Information
                                                                             );

                                if (NT_SUCCESS(ntStatus))
                                    Irp->IoStatus.Information = outputBufferLength;
                            }
                        }
                    }
                    break; // IOCTL_SET_LOCAL_HOST_INFORMATION

                case IOCTL_SET_ADDRESS_DATA:
                    {
                        PSET_ADDRESS_DATA   SetAddressData;

                        TRACE(TL_TRACE, ("IOCTL_SET_ADDRESS_DATA\n"));

                        if (inputBufferLength < sizeof(SET_ADDRESS_DATA)) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            SetAddressData = (PSET_ADDRESS_DATA)ioBuffer;

                            if (inputBufferLength < (sizeof(SET_ADDRESS_DATA)+SetAddressData->nLength)) {

                                ntStatus = STATUS_BUFFER_TOO_SMALL;
                            }
                            else {

                                ntStatus = t1394_SetAddressData( DeviceObject,
                                                                     Irp,
                                                                     SetAddressData->hAddressRange,
                                                                     SetAddressData->nLength,
                                                                     SetAddressData->ulOffset,
                                                                     (PVOID)SetAddressData->Data
                                                                     );
                            }
                        }
                    }
                    break; // IOCTL_SET_ADDRESS_DATA

                case IOCTL_GET_ADDRESS_DATA:
                    {
                        PGET_ADDRESS_DATA   GetAddressData;

                        TRACE(TL_TRACE, ("IOCTL_GET_ADDRESS_DATA\n"));

                        if ((inputBufferLength < sizeof(GET_ADDRESS_DATA)) || 
                            (outputBufferLength < sizeof(GET_ADDRESS_DATA))) {

                            ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            GetAddressData = (PGET_ADDRESS_DATA)ioBuffer;

                            if ((inputBufferLength < (sizeof(GET_ADDRESS_DATA)+GetAddressData->nLength)) ||
                                (outputBufferLength < sizeof (GET_ADDRESS_DATA)+GetAddressData->nLength)) {

                                ntStatus = STATUS_BUFFER_TOO_SMALL;
                            }
                            else {
                                
                                ntStatus = t1394_GetAddressData( DeviceObject,
                                                                     Irp,
                                                                     GetAddressData->hAddressRange,
                                                                     GetAddressData->nLength,
                                                                     GetAddressData->ulOffset,
                                                                     (PVOID)GetAddressData->Data
                                                                     );
                            
                            
                                if (NT_SUCCESS(ntStatus))
                                    Irp->IoStatus.Information = outputBufferLength;
                            }
                        }
                    }
                    break; // IOCTL_GET_ADDRESS_DATA

                case IOCTL_BUS_RESET_NOTIFY: {

                    PBUS_RESET_IRP  BusResetIrp;
                    KIRQL           Irql;
                    
                    TRACE(TL_TRACE, ("IOCTL_BUS_RESET_NOTIFY\n"));

                    BusResetIrp = ExAllocatePool(NonPagedPool, sizeof(BUS_RESET_IRP));

                    if (BusResetIrp) {

                        ntStatus = STATUS_PENDING;
                        BusResetIrp->Irp = Irp;

                        TRACE(TL_TRACE, ("Adding BusResetIrp->Irp = 0x%x\n", BusResetIrp->Irp));

                        // add the irp to the list...
                        KeAcquireSpinLock(&deviceExtension->ResetSpinLock, &Irql);

                        InsertHeadList(&deviceExtension->BusResetIrps, &BusResetIrp->BusResetIrpList);

                        // set the cancel routine for the irp
                        IoSetCancelRoutine(Irp, t1394VDev_CancelIrp);

                        if (Irp->Cancel && IoSetCancelRoutine(Irp, NULL)) {

                            RemoveEntryList(&BusResetIrp->BusResetIrpList);
                            ntStatus = STATUS_CANCELLED;
                        }

                        KeReleaseSpinLock(&deviceExtension->ResetSpinLock, Irql);

                        // goto t1394VDev_IoControlExit on success so we don't complete the irp
                        if (ntStatus == STATUS_PENDING)
                        {
                            // mark it pending
                            IoMarkIrpPending(Irp);
                            goto t1394VDev_IoControlExit;
                        }
                    }
                    else
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                    break; // IOCTL_BUS_RESET_NOTIFY

                case IOCTL_GET_DIAG_VERSION:
                    {
                        PVERSION_DATA   Version;

                        TRACE(TL_TRACE, ("IOCTL_GET_DIAG_VERSION\n"));

                        if ((inputBufferLength < sizeof(VERSION_DATA)) &&
                            (outputBufferLength < sizeof(VERSION_DATA))) {

                                ntStatus = STATUS_BUFFER_TOO_SMALL;
                        }
                        else {

                            Version = (PVERSION_DATA)ioBuffer;
                            Version->ulVersion = DIAGNOSTIC_VERSION;
                            Version->ulSubVersion = DIAGNOSTIC_SUB_VERSION;

                            Irp->IoStatus.Information = outputBufferLength;                            
                        }
                    }
                    break; // IOCTL_GET_DIAG_VERSION

                default:
                    TRACE(TL_ERROR, ("Invalid ioControlCode = 0x%x\n", ioControlCode));
                    ntStatus = STATUS_INVALID_PARAMETER;
                    break; // default

            } // switch

            break; // IRP_MJ_DEVICE_CONTROL

        default:
            TRACE(TL_TRACE, ("Unknown IrpSp->MajorFunction = 0x%x\n", IrpSp->MajorFunction));

            // submit this to the driver below us
            ntStatus = t1394_SubmitIrpAsync (deviceExtension->StackDeviceObject, Irp, NULL);
            return (ntStatus);
            break;

    } // switch


    // only complete if the device is there
    if (ntStatus != STATUS_NO_SUCH_DEVICE) {
    
        Irp->IoStatus.Status = ntStatus;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

t1394VDev_IoControlExit:

    EXIT("t1394VDev_IoControl", ntStatus);
    return(ntStatus);
} // t1394VDev_IoControl


