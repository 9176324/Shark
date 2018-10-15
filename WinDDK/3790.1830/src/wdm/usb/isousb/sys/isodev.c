/*++

Copyright (c) 2004  Microsoft Corporation

Module Name:

    isodev.c

Abstract:

    This file contains dispatch routines for create and close.

    This file also contains routines to selectively suspend the device.
    The selective suspend feature is usb specific and not hardware
    specific.

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
#include "isousr.h"
#include "isowmi.h"
#include "isorwr.h"
#include "isostrm.h"

NTSTATUS
IsoUsb_DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    Dispatch routine for create.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet.

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PFILE_OBJECT            fileObject;
    PFILE_OBJECT_CONTENT    fileObjectContent;
    LONG                    i;
    NTSTATUS                ntStatus;

    PAGED_CODE();

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    fileObject = irpStack->FileObject;

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchCreate - begins\n"));

    if (deviceExtension->DeviceState != Working)
    {
        ntStatus = STATUS_INVALID_DEVICE_STATE;
        goto IsoUsb_DispatchCreate_Exit;
    }

    if (fileObject)
    {
        fileObject->FsContext = NULL;
    }
    else
    {
        ntStatus = STATUS_INVALID_PARAMETER;
        goto IsoUsb_DispatchCreate_Exit;
    }

    fileObject->FsContext = ExAllocatePool(NonPagedPool,
                                           sizeof(FILE_OBJECT_CONTENT));

    if (NULL == fileObject->FsContext)
    {
        IsoUsb_DbgPrint(1, ("failed to alloc memory for FILE_OBJECT_CONTENT\n"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto IsoUsb_DispatchCreate_Exit;
    }

    // Exclude configuration changes while trying to open a pipe
    //
    KeWaitForSingleObject(&deviceExtension->ConfigurationSemaphore,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    fileObjectContent = (PFILE_OBJECT_CONTENT) fileObject->FsContext;
    fileObjectContent->PipeInformation = NULL;
    fileObjectContent->StreamInformation = NULL;

    if (fileObject->FileName.Length != 0)
    {
        // Convert the pipe name string into a pipe number
        //
        i = IsoUsb_ParseStringForPipeNumber(&fileObject->FileName);

        IsoUsb_DbgPrint(3, ("create request for pipe # %X\n", i));

        // Bounds check the pipe number
        //
        if ((i < 0) || (i >= (LONG)(deviceExtension->NumberOfPipes)))
        {
            // Failure opening a pipe
            //
            IsoUsb_DbgPrint(1, ("invalid pipe number\n"));

            ExFreePool(fileObject->FsContext);

            fileObject->FsContext = NULL;

            ntStatus = STATUS_INVALID_PARAMETER;
        }
        else
        {
            // Success opening a pipe
            //
            fileObjectContent->PipeInformation =
                (PVOID) deviceExtension->PipeInformation[i];

            InterlockedIncrement(&deviceExtension->OpenHandleCount);

            ntStatus = STATUS_SUCCESS;
        }
    }
    else
    {
        // No pipe name string, Success opening entire device
        //
        InterlockedIncrement(&deviceExtension->OpenHandleCount);

        ntStatus = STATUS_SUCCESS;
    }

    KeReleaseSemaphore(&deviceExtension->ConfigurationSemaphore,
                       IO_NO_INCREMENT,
                       1,
                       FALSE);

    // The device is idle if it has no open handles or pending PnP Irps.
    // Since we just received an open handle request, cancel idle req.
    //
    if (NT_SUCCESS(ntStatus) && deviceExtension->SSEnable)
    {
        CancelSelectSuspend(deviceExtension);
    }

IsoUsb_DispatchCreate_Exit:

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchCreate - ends\n"));

    return ntStatus;
}

NTSTATUS
IsoUsb_DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    Dispatch routine for close.

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PFILE_OBJECT            fileObject;
    PFILE_OBJECT_CONTENT    fileObjectContent;


    NTSTATUS                ntStatus;

    PAGED_CODE();

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    fileObject = irpStack->FileObject;

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchClose - begins\n"));

    KeWaitForSingleObject(&deviceExtension->ConfigurationSemaphore,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    if (fileObject && fileObject->FsContext)
    {
        fileObjectContent = (PFILE_OBJECT_CONTENT) fileObject->FsContext;

        ASSERT(NULL == fileObjectContent->StreamInformation);

        ExFreePool(fileObjectContent);

        fileObject->FsContext = NULL;
    }

    InterlockedDecrement(&deviceExtension->OpenHandleCount);

    KeReleaseSemaphore(&deviceExtension->ConfigurationSemaphore,
                       IO_NO_INCREMENT,
                       1,
                       FALSE);

    //
    // set ntStatus to STATUS_SUCCESS
    //
    ntStatus = STATUS_SUCCESS;

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchClose - ends\n"));

    return ntStatus;
}


NTSTATUS
IsoUsb_IoctlSelectAltInterface(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION       deviceExtension;
    PIO_STACK_LOCATION      irpStack;
    PFILE_OBJECT            fileObject;
    PFILE_OBJECT_CONTENT    fileObjectContent;
    PUCHAR                  ioBuffer;
    ULONG                   inputBufferLength;
    UCHAR                   interfaceNumber;
    UCHAR                   alternateSetting;
    NTSTATUS                ntStatus;

    deviceExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    ioBuffer = (PUCHAR)Irp->AssociatedIrp.SystemBuffer;

    inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;

    if (inputBufferLength != 2 * sizeof(UCHAR))
    {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    interfaceNumber = ioBuffer[0];

    alternateSetting = ioBuffer[1];

    // Exclude other configuration changes and handle opens.
    //
    KeWaitForSingleObject(&deviceExtension->ConfigurationSemaphore,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    fileObject = irpStack->FileObject;

    fileObjectContent = (PFILE_OBJECT_CONTENT) fileObject->FsContext;

    // Make sure there is only one handle open (i.e. the one that is
    // being used to issue this request) and it is for the entire
    // device, not for a pipe.
    //
    if (deviceExtension->OpenHandleCount == 1 &&
        fileObjectContent->PipeInformation == NULL)
    {
        ntStatus = IsoUsb_SelectAlternateInterface(DeviceObject,
                                                   interfaceNumber,
                                                   alternateSetting);
    }
    else
    {
        ntStatus = STATUS_DEVICE_BUSY;
    }

    // Allow other configuration changes and handle opens.
    //
    KeReleaseSemaphore(&deviceExtension->ConfigurationSemaphore,
                       IO_NO_INCREMENT,
                       1,
                       FALSE);

    return ntStatus;
}


NTSTATUS
IsoUsb_DispatchDevCtrl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    Dispatch routine for IRP_MJ_DEVICE_CONTROL

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    ULONG              code;
    PVOID              ioBuffer;
    ULONG              inputBufferLength;
    ULONG              outputBufferLength;
    ULONG              info;
    NTSTATUS           ntStatus;
    PFILE_OBJECT       fileObject;
    PDEVICE_EXTENSION  deviceExtension;
    PIO_STACK_LOCATION irpStack;

    //
    // initialize variables
    //
    info = 0;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;
    code = irpStack->Parameters.DeviceIoControl.IoControlCode;
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    if(deviceExtension->DeviceState != Working) {

        IsoUsb_DbgPrint(1, ("Invalid device state\n"));

        Irp->IoStatus.Status = ntStatus = STATUS_INVALID_DEVICE_STATE;
        Irp->IoStatus.Information = info;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return ntStatus;
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchDevCtrl::"));
    IsoUsb_IoIncrement(deviceExtension);

    //
    // make sure that the selective suspend request has been completed.
    //
    if(deviceExtension->SSEnable) {

        //
        // It is true that the client driver cancelled the selective suspend
        // request in the dispatch routine for create.
        // But there is no guarantee that it has indeed been completed.
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

    switch(code) {

    case IOCTL_ISOUSB_RESET_PIPE:
    {
        PUSBD_PIPE_INFORMATION pipe;

        pipe = NULL;

        if(fileObject && fileObject->FsContext) {

            pipe = (PUSBD_PIPE_INFORMATION)
                   ((PFILE_OBJECT_CONTENT)fileObject->FsContext)->PipeInformation;
        }

        if(pipe == NULL) {

            ntStatus = STATUS_INVALID_PARAMETER;
        }
        else {

            ntStatus = IsoUsb_AbortResetPipe(DeviceObject,
                                             pipe->PipeHandle,
                                             TRUE);
        }

        break;
    }

    case IOCTL_ISOUSB_GET_CONFIG_DESCRIPTOR:
    {
        ULONG length;

        if(deviceExtension->ConfigurationDescriptor) {

            length = deviceExtension->ConfigurationDescriptor->wTotalLength;

            if(outputBufferLength >= length) {

                RtlCopyMemory(ioBuffer,
                              deviceExtension->ConfigurationDescriptor,
                              length);

                info = length;

                ntStatus = STATUS_SUCCESS;
            }
            else {

                ntStatus = STATUS_INVALID_BUFFER_SIZE;
            }
        }
        else {

            ntStatus = STATUS_UNSUCCESSFUL;
        }

        break;
    }

    case IOCTL_ISOUSB_RESET_DEVICE:

        ntStatus = IsoUsb_ResetDevice(DeviceObject);

        break;

    case IOCTL_ISOUSB_START_ISO_STREAM:

        ntStatus = IsoUsb_StartIsoStream(DeviceObject, Irp);

        return STATUS_SUCCESS;

    case IOCTL_ISOUSB_STOP_ISO_STREAM:
    {

        PFILE_OBJECT_CONTENT fileObjectContent;

        if(fileObject && fileObject->FsContext) {

            fileObjectContent = (PFILE_OBJECT_CONTENT)
                                fileObject->FsContext;

            ntStatus = IsoUsb_StopIsoStream(
                            DeviceObject,
                            InterlockedExchangePointer(
                            &fileObjectContent->StreamInformation,
                            NULL),
                            Irp);
        }
        else {

            ntStatus = STATUS_UNSUCCESSFUL;
        }

        break;
    }

    case IOCTL_ISOUSB_SELECT_ALT_INTERFACE:

        ntStatus = IsoUsb_IoctlSelectAltInterface(DeviceObject, Irp);

        break;

    default :

        ntStatus = STATUS_INVALID_DEVICE_REQUEST;

        break;
    }

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = info;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchDevCtrl::"));
    IsoUsb_IoDecrement(deviceExtension);

    return ntStatus;
}


LONG
IsoUsb_ParseStringForPipeNumber(
    IN PUNICODE_STRING PipeName
    )
/*++

Routine Description:

    This routine parses the PipeName for the Pipe#

Arguments:

    PipeName - Unicode string for the pipe name.
               Valid strings have the form:
               \PIPE? or \PIPE?? or \pipe? or \pipe??
               Where ? is 0..9

Return Value:

    Pipe number 0..99, or -1 if the PipeName is invalid

--*/
{
    ULONG digits;
    ULONG uval;
    ULONG umultiplier;

    if (PipeName->Length == sizeof(L"\\PIPE0") - sizeof(L""))
    {
        digits = 1;
    }
    else if (PipeName->Length == sizeof(L"\\PIPE00") - sizeof(L""))
    {
        digits = 2;
    }
    else
    {
        return -1;
    }

    if ((PipeName->Buffer[0] != (WCHAR)'\\') ||

        ((PipeName->Buffer[1] != (WCHAR)'P') &&
         (PipeName->Buffer[1] != (WCHAR)'p')) ||

        ((PipeName->Buffer[2] != (WCHAR)'I') &&
         (PipeName->Buffer[2] != (WCHAR)'i')) ||

        ((PipeName->Buffer[3] != (WCHAR)'P') &&
         (PipeName->Buffer[3] != (WCHAR)'p')) ||

        ((PipeName->Buffer[4] != (WCHAR)'E') &&
         (PipeName->Buffer[4] != (WCHAR)'e')))
    {
        return -1;
    }

    if (PipeName->Buffer[5] < (WCHAR)'0' ||
        PipeName->Buffer[5] > (WCHAR)'9')
    {
        return -1;
    }

    uval = PipeName->Buffer[5] - (WCHAR)'0';

    if (digits == 2)
    {
        if (PipeName->Buffer[6] < (WCHAR)'0' ||
            PipeName->Buffer[6] > (WCHAR)'9')
        {
            return -1;
        }

        uval *= 10;

        uval += PipeName->Buffer[6] - (WCHAR)'0';

    }

    return uval;
}


NTSTATUS
IsoUsb_ResetDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

    This routine resets the device

Arguments:

    DeviceObject - pointer to device object

Return Value:

    NT status value

--*/
{
    NTSTATUS ntStatus;
    ULONG    portStatus;

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetDevice - begins\n"));

    ntStatus = IsoUsb_GetPortStatus(DeviceObject, &portStatus);

    if((NT_SUCCESS(ntStatus))                 &&
       (!(portStatus & USBD_PORT_ENABLED))    &&
       (portStatus & USBD_PORT_CONNECTED)) {

        ntStatus = IsoUsb_ResetParentPort(DeviceObject);
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetDevice - ends\n"));

    return ntStatus;
}

NTSTATUS
IsoUsb_GetPortStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PULONG     PortStatus
    )
/*++

Routine Description:

    This routine fetches the port status value

Arguments:

    DeviceObject - pointer to device object
    PortStatus - pointer to ULONG to contain the status value

Return Value:

    NT status value

--*/
{
    NTSTATUS           ntStatus;
    KEVENT             event;
    PIRP               irp;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION  deviceExtension;

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    *PortStatus = 0;

    IsoUsb_DbgPrint(3, ("IsoUsb_GetPortStatus - begins\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_INTERNAL_USB_GET_PORT_STATUS,
                    deviceExtension->TopOfStackDeviceObject,
                    NULL,
                    0,
                    NULL,
                    0,
                    TRUE,
                    &event,
                    &ioStatus);

    if(NULL == irp) {

        IsoUsb_DbgPrint(1, ("memory alloc for irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    ASSERT(nextStack != NULL);

    nextStack->Parameters.Others.Argument1 = PortStatus;

    IsoUsb_DbgPrint(3, ("IsoUsb_GetPortStatus::"));
    IsoUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(STATUS_PENDING == ntStatus) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }
    else {

        ioStatus.Status = ntStatus;
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_GetPortStatus::"));
    IsoUsb_IoDecrement(deviceExtension);

    ntStatus = ioStatus.Status;

    IsoUsb_DbgPrint(3, ("IsoUsb_GetPortStatus - ends\n"));

    return ntStatus;
}

NTSTATUS
IsoUsb_ResetParentPort(
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

    This routine will submit an IOCTL_INTERNAL_USB_RESET_PORT,
    down the stack

Arguments:

    DeviceObject - pointer to device object

Return Value:

    NT status value

--*/
{
    NTSTATUS           ntStatus;
    KEVENT             event;
    PIRP               irp;
    IO_STATUS_BLOCK    ioStatus;
    PIO_STACK_LOCATION nextStack;
    PDEVICE_EXTENSION  deviceExtension;

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetParentPort - begins\n"));

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_INTERNAL_USB_RESET_PORT,
                    deviceExtension->TopOfStackDeviceObject,
                    NULL,
                    0,
                    NULL,
                    0,
                    TRUE,
                    &event,
                    &ioStatus);

    if(NULL == irp) {

        IsoUsb_DbgPrint(1, ("memory alloc for irp failed\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    nextStack = IoGetNextIrpStackLocation(irp);

    ASSERT(nextStack != NULL);

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetParentPort"));
    IsoUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, irp);

    if(STATUS_PENDING == ntStatus) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    }
    else {

        ioStatus.Status = ntStatus;
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetParentPort"));
    IsoUsb_IoDecrement(deviceExtension);

    ntStatus = ioStatus.Status;

    IsoUsb_DbgPrint(3, ("IsoUsb_ResetParentPort - ends\n"));

    return ntStatus;
}


NTSTATUS
SubmitIdleRequestIrp(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine builds an idle request irp with an associated callback routine
    and a completion routine in the driver and passes the irp down the stack.

Arguments:

    DeviceExtension - pointer to device extension

Return Value:

    NT status value

--*/
{
    PIRP                    irp;
    NTSTATUS                ntStatus;
    KIRQL                   oldIrql;
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;
    PIO_STACK_LOCATION      nextStack;

    //
    // initialize variables
    //

    irp = NULL;
    idleCallbackInfo = NULL;

    IsoUsb_DbgPrint(3, ("SubmitIdleRequest - begins\n"));

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

    if(PowerDeviceD0 != DeviceExtension->DevPower) {

        ntStatus = STATUS_POWER_STATE_INVALID;
        goto SubmitIdleRequestIrp_Exit;
    }

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    if(InterlockedExchange(&DeviceExtension->IdleReqPend, 1)) {

        IsoUsb_DbgPrint(1, ("Idle request pending..\n"));

        KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

        ntStatus = STATUS_DEVICE_BUSY;

        goto SubmitIdleRequestIrp_Exit;
    }

    //
    // clear the NoIdleReqPendEvent because we are about
    // to submit an idle request. Since we are so early
    // to clear this event, make sure that if we fail this
    // request we set back the event.
    //
    KeClearEvent(&DeviceExtension->NoIdleReqPendEvent);

    idleCallbackInfo = ExAllocatePool(NonPagedPool,
                                      sizeof(struct _USB_IDLE_CALLBACK_INFO));

    if(idleCallbackInfo) {

        idleCallbackInfo->IdleCallback = IdleNotificationCallback;

        idleCallbackInfo->IdleContext = (PVOID)DeviceExtension;

        ASSERT(DeviceExtension->IdleCallbackInfo == NULL);

        DeviceExtension->IdleCallbackInfo = idleCallbackInfo;
        //
        // we use IoAllocateIrp to create an irp to selectively suspend the
        // device. This irp lies pending with the hub driver. When appropriate
        // the hub driver will invoked callback, where we power down. The completion
        // routine is invoked when we power back.
        //
        irp = IoAllocateIrp(DeviceExtension->TopOfStackDeviceObject->StackSize,
                            FALSE);

        if(irp == NULL) {

            IsoUsb_DbgPrint(1, ("cannot build idle request irp\n"));

            KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                       IO_NO_INCREMENT,
                       FALSE);

            InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

            KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

            ExFreePool(idleCallbackInfo);

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto SubmitIdleRequestIrp_Exit;
        }

        nextStack = IoGetNextIrpStackLocation(irp);

        nextStack->MajorFunction =
                    IRP_MJ_INTERNAL_DEVICE_CONTROL;

        nextStack->Parameters.DeviceIoControl.IoControlCode =
                    IOCTL_INTERNAL_USB_SUBMIT_IDLE_NOTIFICATION;

        nextStack->Parameters.DeviceIoControl.Type3InputBuffer =
                    idleCallbackInfo;

        nextStack->Parameters.DeviceIoControl.InputBufferLength =
                    sizeof(struct _USB_IDLE_CALLBACK_INFO);


        IoSetCompletionRoutine(irp,
                               IdleNotificationRequestComplete,
                               DeviceExtension,
                               TRUE,
                               TRUE,
                               TRUE);

        DeviceExtension->PendingIdleIrp = irp;
        //
        // we initialize the count to 2.
        // The reason is, if the CancelSelectSuspend routine manages
        // to grab the irp from the device extension, then the last of the
        // CancelSelectSuspend routine/IdleNotificationRequestComplete routine
        // to execute will free this irp. We need to have this schema so that
        // 1. completion routine does not attempt to touch the irp freed by
        //    CancelSelectSuspend routine.
        // 2. CancelSelectSuspend routine doesnt wait for ever for the completion
        //    routine to complete!
        //
        DeviceExtension->FreeIdleIrpCount = 2;

        KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

        //
        // check if the device is idle.
        // A check here ensures that a race condition did not
        // completely reverse the call sequence of SubmitIdleRequestIrp
        // and CancelSelectiveSuspend
        //

        if(!CanDeviceSuspend(DeviceExtension) ||
           PowerDeviceD0 != DeviceExtension->DevPower) {

            //
            // device cannot suspend - abort.
            // also irps created using IoAllocateIrp
            // needs to be deallocated.
            //

            IsoUsb_DbgPrint(1, ("Device is not idle\n"));

            KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

            DeviceExtension->IdleCallbackInfo = NULL;

            DeviceExtension->PendingIdleIrp = NULL;

            KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                       IO_NO_INCREMENT,
                       FALSE);

            InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

            KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

            if(idleCallbackInfo) {

                ExFreePool(idleCallbackInfo);
            }

            //
            // it is still safe to touch the local variable "irp" here.
            // the irp has not been passed down the stack, the irp has
            // no cancellation routine. The worse position is that the
            // CancelSelectSuspend has run after we released the spin
            // lock above. It is still essential to free the irp.
            //

            if(irp) {

                IoFreeIrp(irp);
            }

            ntStatus = STATUS_UNSUCCESSFUL;

            goto SubmitIdleRequestIrp_Exit;
        }

        IsoUsb_DbgPrint(3, ("Cancel the timers\n"));

        //
        // Cancel the timer so that the DPCs are no longer fired.
        // Thus, we are making judicious usage of our resources.
        // we do not need DPCs because we already have an idle irp pending.
        // The timers are re-initialized in the completion routine.
        //
        KeCancelTimer(&DeviceExtension->Timer);

        ntStatus = IoCallDriver(DeviceExtension->TopOfStackDeviceObject, irp);

        if(!NT_SUCCESS(ntStatus)) {

            IsoUsb_DbgPrint(1, ("IoCallDriver failed\n"));

            goto SubmitIdleRequestIrp_Exit;
        }
    }
    else {

        IsoUsb_DbgPrint(1, ("Memory allocation for idleCallbackInfo failed\n"));

        KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                   IO_NO_INCREMENT,
                   FALSE);

        InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

        KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

SubmitIdleRequestIrp_Exit:

    IsoUsb_DbgPrint(3, ("SubmitIdleRequest - ends\n"));

    return ntStatus;
}


VOID
IdleNotificationCallback(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

  "A pointer to a callback function in your driver is passed down the stack with
   this IOCTL, and it is this callback function that is called by USBHUB when it
   safe for your device to power down."

  "When the callback in your driver is called, all you really need to do is to
   to first ensure that a WaitWake Irp has been submitted for your device, if
   remote wake is possible for your device and then request a SetD2 (or DeviceWake)"

Arguments:

Return Value:

--*/
{
    NTSTATUS                ntStatus;
    POWER_STATE             powerState;
    KEVENT                  irpCompletionEvent;
    PIRP_COMPLETION_CONTEXT irpContext;

    IsoUsb_DbgPrint(3, ("IdleNotificationCallback - begins\n"));

    //
    // Dont idle, if the device was just disconnected or being stopped
    // i.e. return for the following DeviceState(s)
    // NotStarted, Stopped, PendingStop, PendingRemove, SurpriseRemoved, Removed
    //

    if(DeviceExtension->DeviceState != Working) {

        return;
    }

    //
    // If there is not already a WW IRP pending, submit one now
    //
    if(DeviceExtension->WaitWakeEnable) {

        IssueWaitWake(DeviceExtension);
    }


    //
    // power down the device
    //

    irpContext = (PIRP_COMPLETION_CONTEXT)
                 ExAllocatePool(NonPagedPool,
                                sizeof(IRP_COMPLETION_CONTEXT));

    if(!irpContext) {

        IsoUsb_DbgPrint(1, ("Failed to alloc memory for irpContext\n"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else {

        //
        // increment the count. In the HoldIoRequestWorkerRoutine, the
        // count is decremented twice (one for the system Irp and the
        // other for the device Irp. An increment here compensates for
        // the sytem irp..The decrement corresponding to this increment
        // is in the completion function
        //

        IsoUsb_DbgPrint(3, ("IdleNotificationCallback::"));
        IsoUsb_IoIncrement(DeviceExtension);

        powerState.DeviceState = DeviceExtension->PowerDownLevel;

        KeInitializeEvent(&irpCompletionEvent, NotificationEvent, FALSE);

        irpContext->DeviceExtension = DeviceExtension;
        irpContext->Event = &irpCompletionEvent;

        ntStatus = PoRequestPowerIrp(
                          DeviceExtension->PhysicalDeviceObject,
                          IRP_MN_SET_POWER,
                          powerState,
                          (PREQUEST_POWER_COMPLETE) PoIrpCompletionFunc,
                          irpContext,
                          NULL);

        if(STATUS_PENDING == ntStatus) {

            IsoUsb_DbgPrint(3, ("IdleNotificationCallback::"
                           "waiting for the power irp to complete\n"));

            KeWaitForSingleObject(&irpCompletionEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
    }

    if(!NT_SUCCESS(ntStatus)) {

        if(irpContext) {

            ExFreePool(irpContext);
        }
    }

    IsoUsb_DbgPrint(3, ("IdleNotificationCallback - ends\n"));
}


NTSTATUS
IdleNotificationRequestComplete(
    IN PDEVICE_OBJECT    DeviceObject,
    IN PIRP              Irp,
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

  Completion routine for idle notification irp

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet
    DeviceExtension - pointer to device extension

Return Value:

    NT status value

--*/
{
    NTSTATUS                ntStatus;
    POWER_STATE             powerState;
    KIRQL                   oldIrql;
    PIRP                    idleIrp;
    LARGE_INTEGER           dueTime;
    PUSB_IDLE_CALLBACK_INFO idleCallbackInfo;

    IsoUsb_DbgPrint(3, ("IdleNotificationRequestCompete - begins\n"));

    idleIrp = NULL;

    //
    // check the Irp status
    //

    ntStatus = Irp->IoStatus.Status;

    if(!NT_SUCCESS(ntStatus) && ntStatus != STATUS_NOT_SUPPORTED) {

        IsoUsb_DbgPrint(1, ("Idle irp completes with error::"));

        switch(ntStatus) {

        case STATUS_INVALID_DEVICE_REQUEST:

            IsoUsb_DbgPrint(1, ("STATUS_INVALID_DEVICE_REQUEST\n"));

            break;

        case STATUS_CANCELLED:

            IsoUsb_DbgPrint(1, ("STATUS_CANCELLED\n"));

            break;

        case STATUS_POWER_STATE_INVALID:

            IsoUsb_DbgPrint(1, ("STATUS_POWER_STATE_INVALID\n"));

            goto IdleNotificationRequestComplete_Exit;

        case STATUS_DEVICE_BUSY:

            IsoUsb_DbgPrint(1, ("STATUS_DEVICE_BUSY\n"));

            break;

        default:

            IsoUsb_DbgPrint(1, ("default\n"));

            break;
        }

        //
        // if in error, issue a SetD0
        //

        if(PowerDeviceD0 != DeviceExtension->DevPower) {
            IsoUsb_DbgPrint(3, ("IdleNotificationRequestComplete::"));
            IsoUsb_IoIncrement(DeviceExtension);

            powerState.DeviceState = PowerDeviceD0;

            ntStatus = PoRequestPowerIrp(
                              DeviceExtension->PhysicalDeviceObject,
                              IRP_MN_SET_POWER,
                              powerState,
                              (PREQUEST_POWER_COMPLETE) PoIrpAsyncCompletionFunc,
                              DeviceExtension,
                              NULL);

            if(!NT_SUCCESS(ntStatus)) {

                IsoUsb_DbgPrint(1, ("PoRequestPowerIrp failed\n"));
            }
        }
    }

IdleNotificationRequestComplete_Exit:

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    idleCallbackInfo = DeviceExtension->IdleCallbackInfo;

    DeviceExtension->IdleCallbackInfo = NULL;

    idleIrp = (PIRP) InterlockedExchangePointer(
                                        &DeviceExtension->PendingIdleIrp,
                                        NULL);

    InterlockedExchange(&DeviceExtension->IdleReqPend, 0);

    KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

    if(idleCallbackInfo) {

        ExFreePool(idleCallbackInfo);
    }

    //
    // since we allocated the irp, we need to free it.
    // return STATUS_MORE_PROCESSING_REQUIRED so that
    // the kernel does not touch it.
    //

    if(idleIrp) {

        IsoUsb_DbgPrint(3, ("completion routine has a valid irp and frees it\n"));

        IoFreeIrp(Irp);

        KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                   IO_NO_INCREMENT,
                   FALSE);
    }
    else {

        //
        // The CancelSelectiveSuspend routine has grabbed the Irp from the device
        // extension. Now the last one to decrement the FreeIdleIrpCount should
        // free the irp.
        //
        if(0 == InterlockedDecrement(&DeviceExtension->FreeIdleIrpCount)) {

            IsoUsb_DbgPrint(3, ("completion routine frees the irp\n"));

            IoFreeIrp(Irp);

            KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                       IO_NO_INCREMENT,
                       FALSE);
        }
    }

    if(DeviceExtension->SSEnable) {

        IsoUsb_DbgPrint(3, ("Set the timer to fire DPCs\n"));

        dueTime.QuadPart = -10000 * IDLE_INTERVAL;               // 5000 ms

        KeSetTimerEx(&DeviceExtension->Timer,
                     dueTime,
                     IDLE_INTERVAL,                              // 5000 ms
                     &DeviceExtension->DeferredProcCall);

        IsoUsb_DbgPrint(3, ("IdleNotificationRequestCompete - ends\n"));
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
CancelSelectSuspend(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine is invoked to cancel selective suspend request.

Arguments:

    DeviceExtension - pointer to device extension

Return Value:

    None.

--*/
{
    PIRP  irp;
    KIRQL oldIrql;

    irp = NULL;

    IsoUsb_DbgPrint(3, ("CancelSelectSuspend - begins\n"));

    KeAcquireSpinLock(&DeviceExtension->IdleReqStateLock, &oldIrql);

    if(!CanDeviceSuspend(DeviceExtension))
    {
        IsoUsb_DbgPrint(3, ("Device is not idle\n"));

        irp = (PIRP) InterlockedExchangePointer(
                            &DeviceExtension->PendingIdleIrp,
                            NULL);
    }

    KeReleaseSpinLock(&DeviceExtension->IdleReqStateLock, oldIrql);

    //
    // since we have a valid Irp ptr,
    // we can call IoCancelIrp on it,
    // without the fear of the irp
    // being freed underneath us.
    //
    if(irp) {

        //
        // This routine has the irp pointer.
        // It is safe to call IoCancelIrp because we know that
        // the compleiton routine will not free this irp unless...
        //

        if(IoCancelIrp(irp)) {

            IsoUsb_DbgPrint(3, ("IoCancelIrp returns TRUE\n"));
        }
        else {
            IsoUsb_DbgPrint(3, ("IoCancelIrp returns FALSE\n"));
        }

        //
        // ....we decrement the FreeIdleIrpCount from 2 to 1.
        // if completion routine runs ahead of us, then this routine
        // decrements the FreeIdleIrpCount from 1 to 0 and hence shall
        // free the irp.
        //

        if(0 == InterlockedDecrement(&DeviceExtension->FreeIdleIrpCount)) {

            IsoUsb_DbgPrint(3, ("CancelSelectSuspend frees the irp\n"));

            IoFreeIrp(irp);

            KeSetEvent(&DeviceExtension->NoIdleReqPendEvent,
                       IO_NO_INCREMENT,
                       FALSE);
        }
    }

    IsoUsb_DbgPrint(3, ("CancelSelectSuspend - ends\n"));

    return;
}

VOID
PoIrpCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
/*++

Routine Description:

    Completion routine for power irp PoRequested in IdleNotification
    RequestComplete routine.

Arguments:

    DeviceObject - pointer to device object
    MinorFunciton - minor function for the irp.
    PowerState - irp power state
    Context - context passed to the completion function
    IoStatus - status block.

Return Value:

    None

--*/
{
    PIRP_COMPLETION_CONTEXT irpContext;

    //
    // initialize variables
    //
    irpContext = NULL;


    if(Context) {

        irpContext = (PIRP_COMPLETION_CONTEXT) Context;
    }

    //
    // all we do is set the event and decrement the count
    //

    if(irpContext) {

        KeSetEvent(irpContext->Event, 0, FALSE);

        IsoUsb_DbgPrint(3, ("PoIrpCompletionFunc::"));
        IsoUsb_IoDecrement(irpContext->DeviceExtension);

        ExFreePool(irpContext);
    }

    return;
}

VOID
PoIrpAsyncCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
/*++

Routine Description:

  Completion routine for PoRequest wait wake irp

Arguments:

    DeviceObject - pointer to device object
    MinorFunciton - minor function for the irp.
    PowerState - irp power state
    Context - context passed to the completion function
    IoStatus - status block.

Return Value:

    None

--*/
{
    PDEVICE_EXTENSION DeviceExtension;

    //
    // initialize variables
    //
    DeviceExtension = (PDEVICE_EXTENSION) Context;

    //
    // all we do is decrement the count
    //

    IsoUsb_DbgPrint(3, ("PoIrpAsyncCompletionFunc::"));
    IsoUsb_IoDecrement(DeviceExtension);

    return;
}

VOID
WWIrpCompletionFunc(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
/*++

Routine Description:

  Completion routine for idle notification irp

Arguments:

Return Value:

--*/
{
    PDEVICE_EXTENSION DeviceExtension;

    //
    // initialize variables
    //
    DeviceExtension = (PDEVICE_EXTENSION) Context;

    //
    // all we do is decrement the count
    //

    IsoUsb_DbgPrint(3, ("WWIrpCompletionFunc::"));
    IsoUsb_IoDecrement(DeviceExtension);

    return;
}


