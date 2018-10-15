/*++

Copyright (c) 2004  Microsoft Corporation

Module Name:

    isopnp.c

Abstract:

    Isoch USB device driver for Intel 82930 USB test board
    Plug and Play module

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
IsoUsb_DispatchPnP(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    The plug and play dispatch routines.
    Most of these requests the driver will completely ignore.
    In all cases it must pass on the IRP to the lower driver.

Arguments:

    DeviceObject - pointer to a device object.

    Irp - pointer to an I/O Request Packet.

Return Value:

    NT status code

--*/
{
    PIO_STACK_LOCATION irpStack;
    PDEVICE_EXTENSION  deviceExtension;
    KEVENT             startDeviceEvent;
    NTSTATUS           ntStatus;

    //
    // initialize variables
    //

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = DeviceObject->DeviceExtension;

    //
    // since the device is removed, fail the Irp.
    //

    if(Removed == deviceExtension->DeviceState) {

        ntStatus = STATUS_DELETE_PENDING;

        Irp->IoStatus.Status = ntStatus;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest(Irp, IO_NO_INCREMENT);

        return ntStatus;
    }

    IsoUsb_DbgPrint(3, ("///////////////////////////////////////////\n"));
    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::"));
    IsoUsb_IoIncrement(deviceExtension);

    if(irpStack->MinorFunction == IRP_MN_START_DEVICE) {

        ASSERT(deviceExtension->IdleReqPend == 0);
    }
    else {

        if(deviceExtension->SSEnable) {

            CancelSelectSuspend(deviceExtension);
        }
    }

    IsoUsb_DbgPrint(2, (PnPMinorFunctionString(irpStack->MinorFunction)));

    switch(irpStack->MinorFunction) {

    case IRP_MN_START_DEVICE:

        ntStatus = HandleStartDevice(DeviceObject, Irp);

        break;

    case IRP_MN_QUERY_STOP_DEVICE:

        //
        // if we cannot stop the device, we fail the query stop irp
        //
        ntStatus = CanStopDevice(DeviceObject, Irp);

        if(NT_SUCCESS(ntStatus)) {

            ntStatus = HandleQueryStopDevice(DeviceObject, Irp);

            return ntStatus;
        }
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:

        ntStatus = HandleCancelStopDevice(DeviceObject, Irp);

        break;

    case IRP_MN_STOP_DEVICE:

        ntStatus = HandleStopDevice(DeviceObject, Irp);

        IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::IRP_MN_STOP_DEVICE::"));
        IsoUsb_IoDecrement(deviceExtension);

        return ntStatus;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        //
        // if we cannot remove the device, we fail the query remove irp
        //
        ntStatus = CanRemoveDevice(DeviceObject, Irp);

        if(NT_SUCCESS(ntStatus)) {

            ntStatus = HandleQueryRemoveDevice(DeviceObject, Irp);

            return ntStatus;
        }
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        ntStatus = HandleCancelRemoveDevice(DeviceObject, Irp);

        break;

    case IRP_MN_SURPRISE_REMOVAL:

        ntStatus = HandleSurpriseRemoval(DeviceObject, Irp);

        IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::IRP_MN_SURPRISE_REMOVAL::"));
        IsoUsb_IoDecrement(deviceExtension);

        return ntStatus;

    case IRP_MN_REMOVE_DEVICE:

        ntStatus = HandleRemoveDevice(DeviceObject, Irp);

        return ntStatus;

    case IRP_MN_QUERY_CAPABILITIES:

        ntStatus = HandleQueryCapabilities(DeviceObject, Irp);

        break;

    default:

        IoSkipCurrentIrpStackLocation(Irp);

        ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

        IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::default::"));
        IsoUsb_IoDecrement(deviceExtension);

        return ntStatus;

    } // switch

//
// complete request
//

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

//
// decrement count
//
    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchPnP::"));
    IsoUsb_IoDecrement(deviceExtension);

    return ntStatus;
}


NTSTATUS
HandleStartDevice(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
/*++

Routine Description:

    This is the dispatch routine for RP_MJ_PNP, IRP_MN_START_DEVICE

    The PnP Manager sends this IRP at IRQL PASSIVE_LEVEL in the context
    of a system thread.

    This IRP must be handled first by the underlying bus driver for a
    device and then by each higher driver in the device stack.

Arguments:

    DeviceObject - pointer to a device object.

    Irp - I/O request packet

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION deviceExtension;
    KIRQL             oldIrql;
    LARGE_INTEGER     dueTime;
    NTSTATUS          ntStatus;

    IsoUsb_DbgPrint(3, ("HandleStartDevice - begins\n"));

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    // Pass the IRP_MN_START_DEVICE Irp down the stack first before we
    // do anything.
    //
    ntStatus = IsoUsb_SyncPassDownIrp(DeviceObject,
                                      Irp);

    if (!NT_SUCCESS(ntStatus))
    {
        IsoUsb_DbgPrint(1, ("Lower drivers failed IRP_MN_START_DEVICE\n"));
        return ntStatus;
    }

    // If this is the first time the device as been started, retrieve the
    // Device and Configuration Descriptors from the device.
    //
    if (deviceExtension->DeviceDescriptor == NULL)
    {
        ntStatus = IsoUsb_GetDescriptors(DeviceObject);

        if (!NT_SUCCESS(ntStatus))
        {
            return ntStatus;
        }
    }

    // Now configure the device
    //
    ntStatus = IsoUsb_SelectConfiguration(DeviceObject);

    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

    //
    // enable the symbolic links for system components to open
    // handles to the device
    //

    ntStatus = IoSetDeviceInterfaceState(&deviceExtension->InterfaceName,
                                         TRUE);

    if(!NT_SUCCESS(ntStatus)) {

        IsoUsb_DbgPrint(1, ("IoSetDeviceInterfaceState:enable:failed\n"));
        return ntStatus;
    }

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    SET_NEW_PNP_STATE(deviceExtension, Working);
    deviceExtension->QueueState = AllowRequests;

    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

    //
    // initialize wait wake outstanding flag to false.
    // and issue a wait wake.

    deviceExtension->FlagWWOutstanding = 0;
    deviceExtension->FlagWWCancel = 0;
    deviceExtension->WaitWakeIrp = NULL;

    if(deviceExtension->WaitWakeEnable) {

        IssueWaitWake(deviceExtension);
    }

    ProcessQueuedRequests(deviceExtension);

    if(WinXpOrBetter == deviceExtension->WdmVersion) {

        deviceExtension->SSEnable = deviceExtension->SSRegistryEnable;

        //
        // set timer for selective suspend requests.
        //

        if(deviceExtension->SSEnable) {

            dueTime.QuadPart = -10000 * IDLE_INTERVAL;               // 5000 ms

            KeSetTimerEx(&deviceExtension->Timer,
                         dueTime,
                         IDLE_INTERVAL,                              // 5000 ms
                         &deviceExtension->DeferredProcCall);

            deviceExtension->FreeIdleIrpCount = 0;
        }
    }

    if((Win2kOrBetter == deviceExtension->WdmVersion) ||
       (WinXpOrBetter == deviceExtension->WdmVersion)) {

        deviceExtension->IsDeviceHighSpeed = 0;
        GetBusInterfaceVersion(DeviceObject);
    }

    IsoUsb_DbgPrint(3, ("HandleStartDevice - ends\n"));

    return ntStatus;
}


NTSTATUS
CanStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine determines whether the device can be safely stopped. In our
    particular case, we'll assume we can always stop the device.
    A device might fail the request if it doesn't have a queue for the
    requests it might come or if it was notified that it is in the paging
    path.

Arguments:

    DeviceObject - pointer to the device object.

    Irp - pointer to the current IRP.

Return Value:

    STATUS_SUCCESS if the device can be safely stopped, an appropriate
    NT Status if not.

--*/
{
   //
   // We assume we can stop the device
   //

   UNREFERENCED_PARAMETER(DeviceObject);
   UNREFERENCED_PARAMETER(Irp);

   return STATUS_SUCCESS;
}


NTSTATUS
HandleQueryStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine services the Irps of minor type IRP_MN_QUERY_STOP_DEVICE

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager.

Return Value:

    NT status value

--*/
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleQueryStopDevice - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    //
    // If we can stop the device, we need to set the QueueState to
    // HoldRequests so further requests will be queued.
    //

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    SET_NEW_PNP_STATE(deviceExtension, PendingStop);
    deviceExtension->QueueState = HoldRequests;

    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

    //
    // wait for the existing ones to be finished.
    // first, decrement this operation
    //

    IsoUsb_DbgPrint(3, ("HandleQueryStopDevice::"));
    IsoUsb_IoDecrement(deviceExtension);

    KeWaitForSingleObject(&deviceExtension->StopEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(Irp);

    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    IsoUsb_DbgPrint(3, ("HandleQueryStopDevice - ends\n"));

    return ntStatus;
}


NTSTATUS
HandleCancelStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine services Irp of minor type IRP_MN_CANCEL_STOP_DEVICE

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager.

Return Value:

    NT value

--*/
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleCancelStopDevice - begins\n"));

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    //
    // Send this IRP down and wait for it to come back.
    // Set the QueueState flag to AllowRequests,
    // and process all the previously queued up IRPs.
    //
    // First check to see whether you have received cancel-stop
    // without first receiving a query-stop. This could happen if someone
    // above us fails a query-stop and passes down the subsequent
    // cancel-stop.
    //

    if(PendingStop == deviceExtension->DeviceState) {

        ntStatus = IsoUsb_SyncPassDownIrp(DeviceObject, Irp);

        if(NT_SUCCESS(ntStatus)) {

            KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

            RESTORE_PREVIOUS_PNP_STATE(deviceExtension);
            deviceExtension->QueueState = AllowRequests;
            ASSERT(deviceExtension->DeviceState == Working);

            KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

            ProcessQueuedRequests(deviceExtension);
        }

    }
    else {

        // spurious Irp
        //
        // If the device is already in an active state when the driver
        // receives this IRP, a function driver simply sets status to
        // success and passes the IRP to the next driver. For such a
        // cancel-stop IRP, a function driver need not set a completion
        // routine.
        //
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        IoSkipCurrentIrpStackLocation(Irp);
        ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);
    }

    IsoUsb_DbgPrint(3, ("HandleCancelStopDevice - ends\n"));

    return ntStatus;
}


NTSTATUS
HandleStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine services Irp of minor type IRP_MN_STOP_DEVICE

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager.

Return Value:

    NT status value

--*/
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleStopDevice - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if(WinXpOrBetter == deviceExtension->WdmVersion) {

        if(deviceExtension->SSEnable) {
            //
            // Cancel the timer so that the DPCs are no longer fired.
            // we do not need DPCs because the device is stopping.
            // The timers are re-initialized while handling the start
            // device irp.
            //
            KeCancelTimer(&deviceExtension->Timer);
            //
            // after the device is stopped, it can be surprise removed.
            // we set this to 0, so that we do not attempt to cancel
            // the timer while handling surprise remove or remove irps.
            // when we get the start device request, this flag will be
            // reinitialized.
            //
            deviceExtension->SSEnable = 0;

            //
            // make sure that if a DPC was fired before we called cancel timer,
            // then the DPC and work-time have run to their completion
            //
            KeWaitForSingleObject(&deviceExtension->NoDpcWorkItemPendingEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
            //
            // make sure that the selective suspend request has been completed.
            //
            KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
    }

    //
    // after the stop Irp is sent to the lower driver object,
    // the driver must not send any more Irps down that touch
    // the device until another Start has occurred.
    //

    if(deviceExtension->WaitWakeEnable) {

        CancelWaitWake(deviceExtension);
    }

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    SET_NEW_PNP_STATE(deviceExtension, Stopped);

    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

    //
    // This is the right place to actually give up all the resources used
    // This might include calls to IoDisconnectInterrupt, MmUnmapIoSpace,
    // etc.
    //

    ntStatus = IsoUsb_UnConfigure(DeviceObject);

    ReleaseMemory(DeviceObject);

    Irp->IoStatus.Status = ntStatus;
    Irp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    IsoUsb_DbgPrint(3, ("HandleStopDevice - ends\n"));

    return ntStatus;
}


NTSTATUS
CanRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine determines whether the device can be safely removed. In our
    particular case, we'll assume we can always remove the device.
    A device shouldn't be removed if, for example, it has open handles or
    removing the device could result in losing data (plus the reasons
    mentioned at CanStopDevice). The PnP manager on Windows 2000 fails
    on its own any attempt to remove, if there any open handles to the device.
    However on Win9x, the driver must keep count of open handles and fail
    query_remove if there are any open handles.

Arguments:

    DeviceObject - pointer to the device object.

    Irp - pointer to the current IRP.

Return Value:

    STATUS_SUCCESS if the device can be safely removed, an appropriate
    NT Status if not.

--*/
{
   //
   // We assume we can remove the device
   //

   UNREFERENCED_PARAMETER(DeviceObject);
   UNREFERENCED_PARAMETER(Irp);

   return STATUS_SUCCESS;
}


NTSTATUS
HandleQueryRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine services Irp of minor type IRP_MN_QUERY_REMOVE_DEVICE

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager.

Return Value:

    NT status value

--*/
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleQueryRemoveDevice - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    //
    // If we can allow removal of the device, we should set the QueueState
    // to HoldRequests so further requests will be queued. This is required
    // so that we can process queued up requests in cancel-remove just in
    // case somebody else in the stack fails the query-remove.
    //

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    deviceExtension->QueueState = HoldRequests;
    SET_NEW_PNP_STATE(deviceExtension, PendingRemove);

    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

    IsoUsb_DbgPrint(3, ("HandleQueryRemoveDevice::"));
    IsoUsb_IoDecrement(deviceExtension);

    //
    // wait for all the requests to be completed
    //

    KeWaitForSingleObject(&deviceExtension->StopEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    IsoUsb_DbgPrint(3, ("HandleQueryRemoveDevice - ends\n"));

    return ntStatus;
}


NTSTATUS
HandleCancelRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine services Irp of minor type IRP_MN_CANCEL_REMOVE_DEVICE

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager.

Return Value:

    NT status value

--*/
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleCancelRemoveDevice - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    //
    // We need to reset the QueueState flag to ProcessRequest,
    // since the device resume its normal activities.
    //

    //
    // First check to see whether you have received cancel-remove
    // without first receiving a query-remove. This could happen if
    // someone above us fails a query-remove and passes down the
    // subsequent cancel-remove.
    //

    if(PendingRemove == deviceExtension->DeviceState) {

        ntStatus = IsoUsb_SyncPassDownIrp(DeviceObject, Irp);

        if(NT_SUCCESS(ntStatus)) {

            KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

            deviceExtension->QueueState = AllowRequests;
            RESTORE_PREVIOUS_PNP_STATE(deviceExtension);

            KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);
            //
            // process the queued requests that arrive between
            // QUERY_REMOVE and CANCEL_REMOVE
            //

            ProcessQueuedRequests(deviceExtension);

        }
    }
    else {

        //
        // spurious cancel-remove
        //
        // If the device is already started when the driver receives
        // this IRP, the driver simply sets status to success and passes
        // the IRP to the next driver.  For such a cancel-remove IRP, a
        // function driver need not set a completion routine. The device
        // may not be in the remove-pending state, because, for example,
        // the driver failed the previous IRP_MN_QUERY_REMOVE_DEVICE.
        //
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        IoSkipCurrentIrpStackLocation(Irp);
        ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);
    }

    IsoUsb_DbgPrint(3, ("HandleCancelRemoveDevice - ends\n"));

    return ntStatus;
}


NTSTATUS
HandleSurpriseRemoval(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine services Irp of minor type IRP_MN_SURPRISE_REMOVAL

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager.

Return Value:

    NT status value

--*/
{
    KIRQL             oldIrql;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleSurpriseRemoval - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    //
    // 1. fail pending requests
    // 2. return device and memory resources
    // 3. disable interfaces
    //

    if(deviceExtension->WaitWakeEnable) {

        CancelWaitWake(deviceExtension);
    }

    if(WinXpOrBetter == deviceExtension->WdmVersion) {

        if(deviceExtension->SSEnable) {

            KeCancelTimer(&deviceExtension->Timer);

            deviceExtension->SSEnable = 0;
            //
            // make sure that if a DPC was fired before we called cancel timer,
            // then the DPC and work-time have run to their completion
            //
            KeWaitForSingleObject(&deviceExtension->NoDpcWorkItemPendingEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
            //
            // make sure that the selective suspend request has been completed.
            //
            KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
    }

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    deviceExtension->QueueState = FailRequests;
    SET_NEW_PNP_STATE(deviceExtension, SurpriseRemoved);

    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

    ProcessQueuedRequests(deviceExtension);

    ntStatus = IoSetDeviceInterfaceState(&deviceExtension->InterfaceName,
                                         FALSE);

    if(!NT_SUCCESS(ntStatus)) {

        IsoUsb_DbgPrint(1, ("IoSetDeviceInterfaceState::disable:failed\n"));
    }

    RtlFreeUnicodeString(&deviceExtension->InterfaceName);

    IsoUsb_WmiDeRegistration(deviceExtension);

    IsoUsb_AbortPipes(DeviceObject);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);

    IsoUsb_DbgPrint(3, ("HandleSurpriseRemoval - ends\n"));

    return ntStatus;
}


NTSTATUS
HandleRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine services Irp of minor type IRP_MN_REMOVE_DEVICE

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager.

Return Value:

    NT status value

--*/
{
    KIRQL             oldIrql;
    KEVENT            event;
    ULONG             requestCount;
    NTSTATUS          ntStatus;
    PDEVICE_EXTENSION deviceExtension;

    IsoUsb_DbgPrint(3, ("HandleRemoveDevice - begins\n"));

    //
    // initialize variables
    //

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    //
    // The Plug & Play system has dictated the removal of this device.  We
    // have no choice but to detach and delete the device object.
    // (If we wanted to express an interest in preventing this removal,
    // we should have failed the query remove IRP).
    //

    if(SurpriseRemoved != deviceExtension->DeviceState) {

        //
        // we are here after QUERY_REMOVE
        //

        KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

        deviceExtension->QueueState = FailRequests;

        KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

        if(deviceExtension->WaitWakeEnable) {

            CancelWaitWake(deviceExtension);
        }

        if(WinXpOrBetter == deviceExtension->WdmVersion) {

            if(deviceExtension->SSEnable) {
                //
                // Cancel the timer so that the DPCs are no longer fired.
                // we do not need DPCs because the device has been removed
                //
                KeCancelTimer(&deviceExtension->Timer);

                deviceExtension->SSEnable = 0;

                //
                // make sure that if a DPC was fired before we called cancel timer,
                // then the DPC and work-time have run to their completion
                //
                KeWaitForSingleObject(&deviceExtension->NoDpcWorkItemPendingEvent,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);

                //
                // make sure that the selective suspend request has been completed.
                //
                KeWaitForSingleObject(&deviceExtension->NoIdleReqPendEvent,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
            }
        }

        ProcessQueuedRequests(deviceExtension);

        ntStatus = IoSetDeviceInterfaceState(&deviceExtension->InterfaceName,
                                             FALSE);

        if(!NT_SUCCESS(ntStatus)) {

            IsoUsb_DbgPrint(1, ("IoSetDeviceInterfaceState::disable:failed\n"));
        }

        RtlFreeUnicodeString(&deviceExtension->InterfaceName);

        IsoUsb_WmiDeRegistration(deviceExtension);

        IsoUsb_AbortPipes(DeviceObject);
    }

    KeAcquireSpinLock(&deviceExtension->DevStateLock, &oldIrql);

    SET_NEW_PNP_STATE(deviceExtension, Removed);

    KeReleaseSpinLock(&deviceExtension->DevStateLock, oldIrql);

    //
    // need 2 decrements
    //

    IsoUsb_DbgPrint(3, ("HandleRemoveDevice::"));
    requestCount = IsoUsb_IoDecrement(deviceExtension);

    ASSERT(requestCount > 0);

    IsoUsb_DbgPrint(3, ("HandleRemoveDevice::"));
    requestCount = IsoUsb_IoDecrement(deviceExtension);

    KeWaitForSingleObject(&deviceExtension->RemoveEvent,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    ReleaseMemory(DeviceObject);

    //
    // We need to send the remove down the stack before we detach,
    // but we don't need to wait for the completion of this operation
    // (and to register a completion routine).
    //

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoSkipCurrentIrpStackLocation(Irp);
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject, Irp);
    //
    // Detach the FDO from the device stack
    //
    IoDetachDevice(deviceExtension->TopOfStackDeviceObject);
    IoDeleteDevice(DeviceObject);

    IsoUsb_DbgPrint(3, ("HandleRemoveDevice - ends\n"));

    return ntStatus;
}


NTSTATUS
HandleQueryCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    This routine services Irp of minor type IRP_MN_QUERY_CAPABILITIES

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager.

Return Value:

    NT status value

--*/
{
    ULONG                i;
    NTSTATUS             ntStatus;
    PDEVICE_EXTENSION    deviceExtension;
    PDEVICE_CAPABILITIES pdc;
    PIO_STACK_LOCATION   irpStack;

    IsoUsb_DbgPrint(3, ("HandleQueryCapabilities - begins\n"));

    //
    // initialize variables
    //

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    pdc = irpStack->Parameters.DeviceCapabilities.Capabilities;

    //
    // We will provide here an example of an IRP that is processed
    // both on its way down and on its way up: there might be no need for
    // a function driver process this Irp (the bus driver will do that).
    // The driver will wait for the lower drivers (the bus driver among
    // them) to process this IRP, then it processes it again.
    //

    if(pdc->Version < 1 || pdc->Size < sizeof(DEVICE_CAPABILITIES)) {

        IsoUsb_DbgPrint(1, ("HandleQueryCapabilities::request failed\n"));
        ntStatus = STATUS_UNSUCCESSFUL;
        return ntStatus;
    }

    //
    // Set some values in deviceCapabilities here...
    //
    //.............................................
    //
    //
    // Prepare to pass the IRP down
    //

    //
    // Add in the SurpriseRemovalOK bit before passing it down.
    //
    pdc->SurpriseRemovalOK = TRUE;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    ntStatus = IsoUsb_SyncPassDownIrp(DeviceObject, Irp);

    //
    // initialize PowerDownLevel to disabled
    //

    deviceExtension->PowerDownLevel = PowerDeviceUnspecified;

    if(NT_SUCCESS(ntStatus)) {

        deviceExtension->DeviceCapabilities = *pdc;

        for(i = PowerSystemSleeping1; i <= PowerSystemSleeping3; i++) {

            if(deviceExtension->DeviceCapabilities.DeviceState[i] <
                                                            PowerDeviceD3) {

                deviceExtension->PowerDownLevel =
                    deviceExtension->DeviceCapabilities.DeviceState[i];
            }
        }

        //
        // since its safe to surprise-remove this device, we shall
        // set the SurpriseRemoveOK flag to supress any dialog to
        // user.
        //

        pdc->SurpriseRemovalOK = 1;
    }

    if(deviceExtension->PowerDownLevel == PowerDeviceUnspecified ||
       deviceExtension->PowerDownLevel <= PowerDeviceD0) {

        deviceExtension->PowerDownLevel = PowerDeviceD2;
    }

    IsoUsb_DbgPrint(3, ("HandleQueryCapabilities - ends\n"));

    return ntStatus;
}


NTSTATUS
IsoUsb_SyncPassDownIrp (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
/*++

Routine Description:

    This routine synchronously passes an Irp down the driver stack.

    This routine must be called at PASSIVE_LEVEL.

--*/
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS            ntStatus;
    KEVENT              localevent;

    IsoUsb_DbgPrint(2, ("enter: IsoUsb_SyncPassDownIrp\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    // Initialize the event we'll wait on
    //
    KeInitializeEvent(&localevent,
                      SynchronizationEvent,
                      FALSE);

    // Copy down Irp params for the next driver
    //
    IoCopyCurrentIrpStackLocationToNext(Irp);

    // Set the completion routine, which will signal the event
    //
    IoSetCompletionRoutine(Irp,
                           IsoUsb_SyncCompletionRoutine,
                           &localevent,
                           TRUE,    // InvokeOnSuccess
                           TRUE,    // InvokeOnError
                           TRUE);   // InvokeOnCancel

    // Pass the Irp down the stack
    //
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            Irp);

    // If the request is pending, block until it completes
    //
    if (ntStatus == STATUS_PENDING)
    {
        KeWaitForSingleObject(&localevent,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ntStatus = Irp->IoStatus.Status;
    }

    IsoUsb_DbgPrint(2, ("exit:  IsoUsb_SyncPassDownIrp %08X\n", ntStatus));

    return ntStatus;
}


NTSTATUS
IsoUsb_SyncCompletionRoutine (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
/*++

Routine Description:

    This is the completion routine used by IsoUsb_SyncPassDownIrp and
    IsoUsb_SyncPassDownIrp

    If the Irp is one we allocated ourself, DeviceObject is NULL.

--*/
{
    PKEVENT kevent;

    kevent = (PKEVENT)Context;

    KeSetEvent(kevent,
               IO_NO_INCREMENT,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
IsoUsb_SyncSendUsbRequest (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PURB             Urb
    )
/*++

Routine Description:

    This routine synchronously passes a URB down the driver stack.

    This routine must be called at PASSIVE_LEVEL.

--*/
{
    PDEVICE_EXTENSION   deviceExtension;
    KEVENT              localevent;
    PIRP                irp;
    PIO_STACK_LOCATION  nextStack;
    NTSTATUS            ntStatus;

    IsoUsb_DbgPrint(2, ("enter: IsoUsb_SyncSendUsbRequest\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    // Initialize the event we'll wait on
    //
    KeInitializeEvent(&localevent,
                      SynchronizationEvent,
                      FALSE);

    // Allocate the Irp
    //
    irp = IoAllocateIrp(deviceExtension->TopOfStackDeviceObject->StackSize, FALSE);

    if (irp == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    // Set the Irp parameters
    //
    nextStack = IoGetNextIrpStackLocation(irp);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

    nextStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_INTERNAL_USB_SUBMIT_URB;

    nextStack->Parameters.Others.Argument1 = Urb;

    // Set the completion routine, which will signal the event
    //
    IoSetCompletionRoutine(irp,
                           IsoUsb_SyncCompletionRoutine,
                           &localevent,
                           TRUE,    // InvokeOnSuccess
                           TRUE,    // InvokeOnError
                           TRUE);   // InvokeOnCancel



    // Pass the Irp & Urb down the stack
    //
    ntStatus = IoCallDriver(deviceExtension->TopOfStackDeviceObject,
                            irp);

    // If the request is pending, block until it completes
    //
    if (ntStatus == STATUS_PENDING)
    {
        LARGE_INTEGER timeout;

        // Specify a timeout of 5 seconds to wait for this call to complete.
        //
        timeout.QuadPart = -10000 * 5000;

        ntStatus = KeWaitForSingleObject(&localevent,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         &timeout);

        if (ntStatus == STATUS_TIMEOUT)
        {
            ntStatus = STATUS_IO_TIMEOUT;

            // Cancel the Irp we just sent.
            //
            IoCancelIrp(irp);

            // And wait until the cancel completes
            //
            KeWaitForSingleObject(&localevent,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);
        }
        else
        {
            ntStatus = irp->IoStatus.Status;
        }
    }

    // Done with the Irp, now free it.
    //
    IoFreeIrp(irp);

    IsoUsb_DbgPrint(2, ("exit:  IsoUsb_SyncSendUsbRequest %08X\n", ntStatus));

    return ntStatus;
}


NTSTATUS
IsoUsb_GetDescriptor (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            Recipient,
    IN UCHAR            DescriptorType,
    IN UCHAR            Index,
    IN USHORT           LanguageId,
    IN ULONG            RetryCount,
    IN ULONG            DescriptorLength,
    OUT PUCHAR         *Descriptor
    )
/*++

Routine Description:

    This routine attempts to retrieve a specified descriptor from the
    device.

    This routine must be called at IRQL PASSIVE_LEVEL

--*/
{
    USHORT      function;
    PURB        urb;
    NTSTATUS    ntStatus;

    IsoUsb_DbgPrint(2, ("enter: IsoUsb_GetDescriptor\n"));

    *Descriptor = NULL;

    // Set the URB function based on Recipient {Device, Interface, Endpoint}
    //
    switch (Recipient)
    {
        case USB_RECIPIENT_DEVICE:
            function = URB_FUNCTION_GET_DESCRIPTOR_FROM_DEVICE;
            break;
        case USB_RECIPIENT_INTERFACE:
            function = URB_FUNCTION_GET_DESCRIPTOR_FROM_INTERFACE;
            break;
        case USB_RECIPIENT_ENDPOINT:
            function = URB_FUNCTION_GET_DESCRIPTOR_FROM_ENDPOINT;
            break;
        default:
            return STATUS_INVALID_PARAMETER;
    }

    // Allocate a descriptor buffer
    //
    *Descriptor = ExAllocatePool(NonPagedPool,
                                 DescriptorLength);

    if (*Descriptor != NULL)
    {
        // Allocate a URB for the Get Descriptor request
        //
        urb = ExAllocatePool(NonPagedPool,
                             sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));

        if (urb != NULL)
        {
            do
            {
                // Initialize the URB
                //
                urb->UrbHeader.Function = function;
                urb->UrbHeader.Length = sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST);
                urb->UrbControlDescriptorRequest.TransferBufferLength = DescriptorLength;
                urb->UrbControlDescriptorRequest.TransferBuffer = *Descriptor;
                urb->UrbControlDescriptorRequest.TransferBufferMDL = NULL;
                urb->UrbControlDescriptorRequest.UrbLink = NULL;
                urb->UrbControlDescriptorRequest.DescriptorType = DescriptorType;
                urb->UrbControlDescriptorRequest.Index = Index;
                urb->UrbControlDescriptorRequest.LanguageId = LanguageId;

                // Send the URB down the stack
                //
                ntStatus = IsoUsb_SyncSendUsbRequest(DeviceObject,
                                                     urb);

                if (NT_SUCCESS(ntStatus))
                {
                    // No error, make sure the length and type are correct
                    //
                    if ((DescriptorLength ==
                         urb->UrbControlDescriptorRequest.TransferBufferLength) &&
                        (DescriptorType ==
                         ((PUSB_COMMON_DESCRIPTOR)*Descriptor)->bDescriptorType))
                    {
                        // The length and type are correct, all done
                        //
                        break;
                    }
                    else
                    {
                        // No error, but the length or type is incorrect
                        //
                        ntStatus = STATUS_DEVICE_DATA_ERROR;
                    }
                }

            } while (RetryCount-- > 0);

            ExFreePool(urb);
        }
        else
        {
            // Failed to allocate the URB
            //
            ExFreePool(*Descriptor);
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        // Failed to allocate the descriptor buffer
        //
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(ntStatus))
    {
        if (*Descriptor != NULL)
        {
            ExFreePool(*Descriptor);
            *Descriptor = NULL;
        }
    }

    IsoUsb_DbgPrint(2, ("exit:  IsoUsb_GetDescriptor %08X\n", ntStatus));

    return ntStatus;
}


NTSTATUS
IsoUsb_GetDescriptors (
    IN PDEVICE_OBJECT   DeviceObject
    )
/*++

Routine Description:

    This routine is called at START_DEVICE time for the FDO to retrieve
    the Device and Configurations descriptors from the device and store
    them in the Device Extension.

    This routine must be called at PASSIVE_LEVEL.

--*/
{
    PDEVICE_EXTENSION   deviceExtension;
    PUCHAR              descriptor;
    ULONG               descriptorLength;
    NTSTATUS            ntStatus;

    IsoUsb_DbgPrint(2, ("enter: IsoUsb_GetDescriptors\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    //
    // Get Device Descriptor
    //
    ntStatus = IsoUsb_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_DEVICE_DESCRIPTOR_TYPE,
                                     0,  // Index
                                     0,  // LanguageId
                                     2,  // RetryCount
                                     sizeof(USB_DEVICE_DESCRIPTOR),
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        IsoUsb_DbgPrint(1, ("Get Device Descriptor failed\n"));
        goto IsoUsb_GetDescriptorsDone;
    }

    ASSERT(deviceExtension->DeviceDescriptor == NULL);
    deviceExtension->DeviceDescriptor = (PUSB_DEVICE_DESCRIPTOR)descriptor;

    //
    // Get Configuration Descriptor (just the Configuration Descriptor)
    //
    ntStatus = IsoUsb_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                     0,  // Index
                                     0,  // LanguageId
                                     2,  // RetryCount
                                     sizeof(USB_CONFIGURATION_DESCRIPTOR),
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        IsoUsb_DbgPrint(1, ("Get Configuration Descriptor failed (1)\n"));
        goto IsoUsb_GetDescriptorsDone;
    }

    descriptorLength = ((PUSB_CONFIGURATION_DESCRIPTOR)descriptor)->wTotalLength;

    ExFreePool(descriptor);

    if (descriptorLength < sizeof(USB_CONFIGURATION_DESCRIPTOR))
    {
        ntStatus = STATUS_DEVICE_DATA_ERROR;
        IsoUsb_DbgPrint(1, ("Get Configuration Descriptor failed (2)\n"));
        goto IsoUsb_GetDescriptorsDone;
    }

    //
    // Get Configuration Descriptor (and Interface and Endpoint Descriptors)
    //
    ntStatus = IsoUsb_GetDescriptor(DeviceObject,
                                     USB_RECIPIENT_DEVICE,
                                     USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                     0,  // Index
                                     0,  // LanguageId
                                     2,  // RetryCount
                                     descriptorLength,
                                     &descriptor);

    if (!NT_SUCCESS(ntStatus))
    {
        IsoUsb_DbgPrint(1, ("Get Configuration Descriptor failed (3)\n"));
        goto IsoUsb_GetDescriptorsDone;
    }

    ASSERT(deviceExtension->ConfigurationDescriptor == NULL);
    deviceExtension->ConfigurationDescriptor = (PUSB_CONFIGURATION_DESCRIPTOR)descriptor;

    if (deviceExtension->ConfigurationDescriptor->bmAttributes & REMOTE_WAKEUP_MASK)
    {
        //
        // this configuration supports remote wakeup
        //
        deviceExtension->WaitWakeEnable = 1;
    }
    else
    {
        deviceExtension->WaitWakeEnable = 0;
    }

#if DBG
    DumpDeviceDesc(deviceExtension->DeviceDescriptor);
    DumpConfigDesc(deviceExtension->ConfigurationDescriptor);
#endif

IsoUsb_GetDescriptorsDone:

    IsoUsb_DbgPrint(2, ("exit:  IsoUsb_GetDescriptors %08X\n", ntStatus));

    return ntStatus;
}


PUSBD_INTERFACE_LIST_ENTRY
IsoUsb_BuildInterfaceList (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigurationDescriptor
    )
/*++

Routine Description:

    This routine builds an array of USBD_INTERFACE_LIST_ENTRY structures
    from a Configuration Descriptor.  This array will then be suitable
    for use in a call to USBD_CreateConfigurationRequestEx().

    It is the responsibility of the caller to free the returned array
    of USBD_INTERFACE_LIST_ENTRY structures.

--*/
{
    PUSBD_INTERFACE_LIST_ENTRY  interfaceList;
    PUSB_INTERFACE_DESCRIPTOR   interfaceDescriptor;
    ULONG                       numInterfaces;
    ULONG                       numInterfacesFound;
    ULONG                       interfaceNumber;

    interfaceList = NULL;

    numInterfaces = ConfigurationDescriptor->bNumInterfaces;

    if (numInterfaces > 0)
    {
        // Allocate a USBD_INTERFACE_LIST_ENTRY structure for each
        // Interface in the Configuration Descriptor, plus one more to
        // null terminate the array.
        //
        interfaceList = ExAllocatePool(NonPagedPool,
                                       (numInterfaces + 1) *
                                       sizeof(USBD_INTERFACE_LIST_ENTRY));

        if (interfaceList)
        {
            // Parse out the Interface Descriptor for Alternate
            // Interface setting zero for each Interface from the
            // Configuration Descriptor.
            //
            // Note that some devices have been implemented which do
            // not consecutively number their Interface Descriptors.
            //
            // interfaceNumber may increment and skip an interface
            // number without incrementing numInterfacesFound.
            //
            numInterfacesFound = 0;

            interfaceNumber = 0;

            // Note that this also null terminates the list.
            //
            RtlZeroMemory(interfaceList,
                          (numInterfaces + 1) *
                          sizeof(USBD_INTERFACE_LIST_ENTRY));

            while (numInterfacesFound < numInterfaces)
            {
                interfaceDescriptor =  USBD_ParseConfigurationDescriptorEx(
                    ConfigurationDescriptor,
                    ConfigurationDescriptor,
                    interfaceNumber,    // InterfaceNumber
                    0,                  // AlternateSetting Zero
                    -1,                 // InterfaceClass, don't care
                    -1,                 // InterfaceSubClass, don't care
                    -1                  // InterfaceProtocol, don't care
                    );

                if (interfaceDescriptor)
                {
                    interfaceList[numInterfacesFound].InterfaceDescriptor =
                        interfaceDescriptor;

                    numInterfacesFound++;
                }

                interfaceNumber++;

                // Prevent an endless loop due to an incorrectly formed
                // Configuration Descriptor.  The maximum interface
                // number is 255.

                if (interfaceNumber > 256)
                {
                    ExFreePool(interfaceList);

                    return NULL;
                }
            }
        }
    }

    ASSERT(interfaceList);
    return interfaceList;
}


NTSTATUS
IsoUsb_SelectConfiguration (
    IN PDEVICE_OBJECT   DeviceObject
    )
/*++

Routine Description:

    This routine is called at START_DEVICE time for the FDO to configure
    the device, i.e. to send an URB_FUNCTION_SELECT_CONFIGURATION
    request down the driver stack for the device.

    It assumes that the Configuration Descriptor has already been
    successfully retrieved from the device and stored in the Device
    Extension.

--*/
{
    PDEVICE_EXTENSION               deviceExtension;
    PUSB_CONFIGURATION_DESCRIPTOR   configurationDescriptor;
    ULONG                           numInterfaces;
    PUSBD_INTERFACE_LIST_ENTRY      interfaceList;
    PURB                            urb;
    PUSBD_INTERFACE_INFORMATION     interfaceInfo;
    PUSBD_INTERFACE_INFORMATION *   interfaceInfoList;
    ULONG                           i;
    ULONG                           j;
    NTSTATUS                        ntStatus;

    IsoUsb_DbgPrint(2, ("enter: IsoUsb_SelectConfiguration\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    configurationDescriptor = deviceExtension->ConfigurationDescriptor;

    numInterfaces = configurationDescriptor->bNumInterfaces;

    // Build an USBD_INTERFACE_LIST_ENTRY array to use as an
    // input/output parameter to USBD_CreateConfigurationRequestEx().
    //
    interfaceList = IsoUsb_BuildInterfaceList(configurationDescriptor);

    if (interfaceList == NULL)
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        goto IsoUsb_SelectConfigurationExit;
    }

    // Create a SELECT_CONFIGURATION URB, turning the Interface
    // Descriptors in the interfaceList into
    // USBD_INTERFACE_INFORMATION structures in the URB.
    //
    urb = USBD_CreateConfigurationRequestEx(configurationDescriptor,
                                            interfaceList);

    // Now done with the interfaceList as soon as the URB is built (or
    // not built).
    //
    ExFreePool(interfaceList);
    interfaceList = NULL;

    if (urb == NULL)
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        goto IsoUsb_SelectConfigurationExit;
    }

    // Now issue the USB request to set the Configuration
    //
    ntStatus = IsoUsb_SyncSendUsbRequest(DeviceObject,
                                         urb);

    if (!NT_SUCCESS(ntStatus))
    {
        // Now done with the URB
        //
        ExFreePool(urb);
        urb = NULL;

        goto IsoUsb_SelectConfigurationExit;
    }

    // Save the configuration handle for this device in the Device
    // Extension.
    //
    deviceExtension->ConfigurationHandle = urb->UrbSelectConfiguration.ConfigurationHandle;

    ASSERT(deviceExtension->InterfaceInfoList == NULL);

    // Allocate an array to hold pointers to one
    // USBD_INTERFACE_INFORMATION structure per configured Interface.
    // Save this array in the Device Extension.
    //
    interfaceInfoList = (PUSBD_INTERFACE_INFORMATION *)
                        ExAllocatePool(NonPagedPool,
                                       numInterfaces *
                                       sizeof(PUSBD_INTERFACE_INFORMATION));

    if (interfaceInfoList == NULL)
    {
        // Now done with the URB
        //
        ExFreePool(urb);
        urb = NULL;

        goto IsoUsb_SelectConfigurationExit;
    }

    deviceExtension->InterfaceInfoList = interfaceInfoList;

    // The end of the SELECT_CONFIGURATION URB contains a contiguous
    // array of variably sized USBD_INTERFACE_INFORMATION structures,
    // one per configured Interface.
    //
    // Get a pointer to the USBD_INTERFACE_INFORMATION structure for
    // the first configured Interface.
    //
    interfaceInfo = &urb->UrbSelectConfiguration.Interface;

    // Iterate over each configured Interface.  Allocate memory for a
    // copy of the USBD_INTERFACE_INFORMATION structure for the
    // Interface and save a copy of the structure from the URB.
    //
    for (i = 0; i < numInterfaces; i++)
    {
        interfaceInfoList[i] =  ExAllocatePool(NonPagedPool,
                                               interfaceInfo->Length);

        if (interfaceInfoList[i] == NULL)
        {
            // Allocation failure, free the previously allocated
            // USBD_INTERFACE_INFORMATION structure copies.
            //
            for (i = 0; i < numInterfaces; i++)
            {
                if (interfaceInfoList[i] != NULL)
                {
                    ExFreePool(interfaceInfoList[i]);
                }
            }

            // Free the list of pointers to allocated
            // USBD_INTERFACE_INFORMATION structure copies.
            //
            ExFreePool(deviceExtension->InterfaceInfoList);
            deviceExtension->InterfaceInfoList = NULL;

            // Now done with the URB
            //
            ExFreePool(urb);
            urb = NULL;

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            goto IsoUsb_SelectConfigurationExit;
        }

        // Save a copy of the USBD_INTERFACE_INFORMATION structure from
        // the URB in the Device Extension InterfaceInfoList.
        //
        RtlCopyMemory(interfaceInfoList[i],
                      interfaceInfo,
                      interfaceInfo->Length);

        // Save pointers back into the USBD_PIPE_INFORMATION structures
        // contained within the above saved USBD_INTERFACE_INFORMATION
        // structure.
        //
        // This is makes it easier to iterate over all currently
        // configured pipes without having to interate over all
        // currently configured interfaces as we are in the middle of
        // doing right now.
        //
        for (j = 0; j < interfaceInfo->NumberOfPipes; j++)
        {
            if (deviceExtension->NumberOfPipes < ISOUSB_MAX_PIPES)
            {
                deviceExtension->PipeInformation[deviceExtension->NumberOfPipes] =
                    &interfaceInfoList[i]->Pipes[j];

                deviceExtension->NumberOfPipes++;
            }
        }

#if DBG
        DumpInterfaceInformation(interfaceInfo);
#endif

        // Advance to the next variably sized USBD_INTERFACE_INFORMATION
        // structure in the SELECT_CONFIGURATION URB.
        //
        interfaceInfo = (PUSBD_INTERFACE_INFORMATION)
                        ((PUCHAR)interfaceInfo + interfaceInfo->Length);
    }

    // Now done with the URB
    //
    ExFreePool(urb);

IsoUsb_SelectConfigurationExit:

    IsoUsb_DbgPrint(2, ("exit:  IsoUsb_SelectConfiguration %08X\n", ntStatus));

    return ntStatus;
}


NTSTATUS
IsoUsb_SelectAlternateInterface (
    IN PDEVICE_OBJECT   DeviceObject,
    IN UCHAR            InterfaceNumber,
    IN UCHAR            AlternateSetting
    )
/*++

Routine Description:

    This routine attempts to select the specified alternate interface
    setting for the specified interface.

    The InterfaceNumber and AlternateSetting parameters will be
    validated by this routine and are not assumed to be valid.

    This routines assumes that the device is currently configured, i.e.
    IsoUsb_SelectConfiguration() has been successfully called and
    IsoUsb_UnConfigure() has not been called.

    This routine assumes that no pipes will be accessed while this
    routine is executing as the deviceExtension->InterfaceInfoList and
    deviceExtension->PipeInformation data is updated by this routine.
    This exclusion synchronization must be handled external to this
    routine.

--*/
{
    PDEVICE_EXTENSION               deviceExtension;
    PUSB_CONFIGURATION_DESCRIPTOR   configurationDescriptor;
    ULONG                           numInterfaces;
    PUSB_INTERFACE_DESCRIPTOR       interfaceDescriptor;
    UCHAR                           numEndpoints;
    USHORT                          urbSize;
    PURB                            urb;
    PUSBD_INTERFACE_INFORMATION     interfaceInfoUrb;
    PUSBD_INTERFACE_INFORMATION     interfaceInfoCopy;
    ULONG                           i;
    ULONG                           j;
    NTSTATUS                        ntStatus;

    IsoUsb_DbgPrint(2, ("enter: IsoUsb_SelectAlternateInterface\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    configurationDescriptor = deviceExtension->ConfigurationDescriptor;

    numInterfaces = configurationDescriptor->bNumInterfaces;

    // Find the Interface Descriptor which matches the InterfaceNumber
    // and AlternateSetting parameters.
    //
    interfaceDescriptor = USBD_ParseConfigurationDescriptorEx(
                              configurationDescriptor,
                              configurationDescriptor,
                              InterfaceNumber,
                              AlternateSetting,
                              -1, // InterfaceClass, don't care
                              -1, // InterfaceSubClass, don't care
                              -1  // InterfaceProtocol, don't care
                              );

    if (interfaceDescriptor == NULL)
    {
        // Interface Descriptor not found, bad InterfaceNumber or
        // AlternateSetting.
        //
        ntStatus = STATUS_INVALID_PARAMETER;

        goto IsoUsb_SelectAlternateInterfaceExit;
    }

    // Allocate a URB_FUNCTION_SELECT_INTERFACE request structure
    //
    numEndpoints = interfaceDescriptor->bNumEndpoints;

    urbSize = GET_SELECT_INTERFACE_REQUEST_SIZE(numEndpoints);

    urb = ExAllocatePool(NonPagedPool,
                         urbSize);

    if (urb == NULL)
    {
        // Could not allocate the URB.
        //
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        goto IsoUsb_SelectAlternateInterfaceExit;
    }

    // Initialize the URB
    //
    RtlZeroMemory(urb, urbSize);

    urb->UrbHeader.Length   = urbSize;

    urb->UrbHeader.Function = URB_FUNCTION_SELECT_INTERFACE;

    urb->UrbSelectInterface.ConfigurationHandle =
        deviceExtension->ConfigurationHandle;

    interfaceInfoUrb = &urb->UrbSelectInterface.Interface;

    interfaceInfoUrb->Length = GET_USBD_INTERFACE_SIZE(numEndpoints);

    interfaceInfoUrb->InterfaceNumber = InterfaceNumber;

    interfaceInfoUrb->AlternateSetting = AlternateSetting;

    for (i = 0; i < numEndpoints; i++)
    {
        interfaceInfoUrb->Pipes[i].MaximumTransferSize =
            USBD_DEFAULT_MAXIMUM_TRANSFER_SIZE;
    }

    // Allocate a USBD_INTERFACE_INFORMATION structure to hold the
    // result of the URB_FUNCTION_SELECT_INTERFACE request.
    //
    interfaceInfoCopy = ExAllocatePool(NonPagedPool,
                                       GET_USBD_INTERFACE_SIZE(numEndpoints));

    // Could not allocate the USBD_INTERFACE_INFORMATION copy.
    //
    if (interfaceInfoCopy == NULL)
    {
        ExFreePool(urb);

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;

        goto IsoUsb_SelectAlternateInterfaceExit;
    }

    // Now issue the USB request to select the alternate interface
    //
    ntStatus = IsoUsb_SyncSendUsbRequest(DeviceObject,
                                         urb);

    if (NT_SUCCESS(ntStatus))
    {
        // Save a copy of the interface information returned by the
        // SELECT_INTERFACE request.  This gives us a list of
        // PIPE_INFORMATION structures for each pipe opened in this
        // selected alternate interface setting.
        //
        ASSERT(interfaceInfoUrb->Length == GET_USBD_INTERFACE_SIZE(numEndpoints));

        RtlCopyMemory(interfaceInfoCopy,
                      interfaceInfoUrb,
                      GET_USBD_INTERFACE_SIZE(numEndpoints));

#if DBG
        DumpInterfaceInformation(interfaceInfoCopy);
#endif
    }
    else
    {
        // How to recover from a select alternate interface failure?
        //
        // The other currently configured interfaces (if any) should
        // not be disturbed by this select alternate interface failure.
        //
        // Just note that this interface now has no currently
        // configured pipes (i.e. interfaceInfoCopy->NumberOfPipes == 0)
        //
        RtlZeroMemory(interfaceInfoCopy,
                      GET_USBD_INTERFACE_SIZE(numEndpoints));

        interfaceInfoCopy->Length = GET_USBD_INTERFACE_SIZE(numEndpoints);

        interfaceInfoCopy->InterfaceNumber = InterfaceNumber;

        interfaceInfoCopy->AlternateSetting = AlternateSetting;
    }

    // Zero out and then reset the currently configured
    // USBD_PIPE_INFORMATION
    //
    deviceExtension->NumberOfPipes = 0;

    RtlZeroMemory(deviceExtension->PipeInformation,
                  sizeof(deviceExtension->PipeInformation));

    for (i = 0; i < numInterfaces; i++)
    {
        // Save pointers back into the USBD_PIPE_INFORMATION structures
        // contained within the above saved USBD_INTERFACE_INFORMATION
        // structure.
        //
        // This is makes it easier to iterate over all currently
        // configured pipes without having to interate over all
        // currently configured interfaces as we are in the middle of
        // doing right now.
        //

        if ((deviceExtension->InterfaceInfoList[i]->InterfaceNumber ==
             InterfaceNumber) &&
            (interfaceInfoCopy != NULL))
        {
            // Free the USBD_INTERFACE_INFORMATION for the previously
            // selected alternate interface setting and swap in the new
            // USBD_INTERFACE_INFORMATION copy.
            //
            ExFreePool(deviceExtension->InterfaceInfoList[i]);

            deviceExtension->InterfaceInfoList[i] = interfaceInfoCopy;

            interfaceInfoCopy = NULL;
        }

        for (j = 0; j < deviceExtension->InterfaceInfoList[i]->NumberOfPipes; j++)
        {
            if (deviceExtension->NumberOfPipes < ISOUSB_MAX_PIPES)
            {
                deviceExtension->PipeInformation[deviceExtension->NumberOfPipes] =
                    &deviceExtension->InterfaceInfoList[i]->Pipes[j];

                deviceExtension->NumberOfPipes++;
            }
        }
    }

    // Done with the URB
    //
    ExFreePool(urb);

IsoUsb_SelectAlternateInterfaceExit:

    IsoUsb_DbgPrint(2, ("exit:  IsoUsb_SelectAlternateInterface %08X\n", ntStatus));

    return ntStatus;
}


NTSTATUS
IsoUsb_UnConfigure (
    IN PDEVICE_OBJECT   DeviceObject
    )
/*++

Routine Description:

    This routine is invoked when the device is removed or stopped.
    This routine de-configures the usb device.

Arguments:

    DeviceObject - pointer to device object

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION   deviceExtension;
    NTSTATUS            ntStatus;
    PURB                urb;
    ULONG               ulSize;
    ULONG               numInterfaces;
    ULONG               i;

    IsoUsb_DbgPrint(2, ("enter: IsoUsb_UnConfigure\n"));

    deviceExtension = DeviceObject->DeviceExtension;

    // Allocate a URB for the SELECT_CONFIGURATION request.  As we are
    // unconfiguring the device, the request needs no pipe and interface
    // information structures.
    //
    ulSize = sizeof(struct _URB_SELECT_CONFIGURATION) -
             sizeof(USBD_INTERFACE_INFORMATION);

    urb = ExAllocatePool(
              NonPagedPool,
              ulSize
              );

    if (urb)
    {
        // Initialize the URB.  A NULL Configuration Descriptor indicates
        // that the device should be unconfigured.
        //
        UsbBuildSelectConfigurationRequest(
            urb,
            (USHORT)ulSize,
            NULL
            );

        // Now issue the USB request to set the Configuration
        //
        ntStatus = IsoUsb_SyncSendUsbRequest(DeviceObject,
                                             urb);

        // Done with the URB now.
        //
        ExFreePool(urb);

        // The device is no longer configured.
        //
        deviceExtension->ConfigurationHandle = 0;

        if (deviceExtension->InterfaceInfoList != NULL)
        {
            numInterfaces = deviceExtension->ConfigurationDescriptor->bNumInterfaces;

            // Free the previously allocated USBD_INTERFACE_INFORMATION
            // structure copies.
            //
            for (i = 0; i < numInterfaces; i++)
            {
                if (deviceExtension->InterfaceInfoList[i] != NULL)
                {
                    ExFreePool(deviceExtension->InterfaceInfoList[i]);
                }
            }

            // Free the list of pointers to allocated
            // USBD_INTERFACE_INFORMATION structure copies.
            //
            ExFreePool(deviceExtension->InterfaceInfoList);
            deviceExtension->InterfaceInfoList = NULL;

            // Zero out the currently configured USBD_PIPE_INFORMATION
            //
            deviceExtension->NumberOfPipes = 0;

            RtlZeroMemory(deviceExtension->PipeInformation,
                          sizeof(deviceExtension->PipeInformation));
        }
    }
    else
    {
        // Could not allocate the URB.
        //
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    IsoUsb_DbgPrint(2, ("exit:  IsoUsb_UnConfigure %08X\n", ntStatus));

    return ntStatus;
}


NTSTATUS
IsoUsb_AbortResetPipe (
    IN PDEVICE_OBJECT       DeviceObject,
    IN USBD_PIPE_HANDLE     PipeHandle,
    IN BOOLEAN              ResetPipe
    )
/*++

Routine Description:

    This routine synchronously submits a URB_FUNCTION_ABORT_PIPE or
    URB_FUNCTION_RESET_PIPE request for the specified pipe.

    This routine must be called at PASSIVE_LEVEL.

Arguments:

    DeviceObject - pointer to device object
    PipeInfo - pointer to USBD_PIPE_INFORMATION

Return Value:

    NT status value

--*/
{
    PURB        urb;
    NTSTATUS    ntStatus;

    IsoUsb_DbgPrint(2, ("enter: IsoUsb_AbortResetPipe\n"));

    // Allocate URB for ABORT_PIPE / RESET_PIPE request
    //
    urb = ExAllocatePool(NonPagedPool,
                         sizeof(struct _URB_PIPE_REQUEST));

    if (urb != NULL)
    {
        // Initialize ABORT_PIPE / RESET_PIPE request URB
        //
        urb->UrbHeader.Length   = sizeof(struct _URB_PIPE_REQUEST);

        if (ResetPipe)
        {
            urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
        }
        else
        {
            urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
        }

        urb->UrbPipeRequest.PipeHandle = PipeHandle;

        // Synchronously submit ABORT_PIPE / RESET_PIPE request URB
        //
        ntStatus = IsoUsb_SyncSendUsbRequest(DeviceObject, urb);

        // Done with URB for ABORT_PIPE / RESET_PIPE request, free it
        //
        ExFreePool(urb);
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    IsoUsb_DbgPrint(2, ("exit:  IsoUsb_AbortResetPipe %08X\n", ntStatus));

    return ntStatus;
}


NTSTATUS
IsoUsb_AbortPipes(
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

    This routine sends an irp/urb pair with
    URB_FUNCTION_ABORT_PIPE request down the stack

Arguments:

    DeviceObject - pointer to device object

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION           deviceExtension;
    ULONG                       i;
    NTSTATUS                    ntStatus;

    IsoUsb_DbgPrint(3, ("IsoUsb_AbortPipes - begins\n"));

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    for (i = 0; i < deviceExtension->NumberOfPipes; i++)
    {
        ntStatus = IsoUsb_AbortResetPipe(
                       DeviceObject,
                       deviceExtension->PipeInformation[i]->PipeHandle,
                       FALSE);
    }

    IsoUsb_DbgPrint(3, ("IsoUsb_AbortPipes - ends\n"));

    return STATUS_SUCCESS;
}


VOID
DpcRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
/*++

Routine Description:

    DPC routine triggered by the timer to check the idle state
    of the device and submit an idle request for the device.

Arguments:

    DeferredContext - context for the dpc routine.
                      DeviceObject in our case.

Return Value:

    None

--*/
{
    NTSTATUS          ntStatus;
    PDEVICE_OBJECT    deviceObject;
    PDEVICE_EXTENSION deviceExtension;
    PIO_WORKITEM      item;

    IsoUsb_DbgPrint(3, ("DpcRoutine - begins\n"));

    deviceObject = (PDEVICE_OBJECT)DeferredContext;
    deviceExtension = (PDEVICE_EXTENSION)deviceObject->DeviceExtension;

    //
    // Clear this event since a DPC has been fired!
    //
    KeClearEvent(&deviceExtension->NoDpcWorkItemPendingEvent);

    if(CanDeviceSuspend(deviceExtension)) {

        IsoUsb_DbgPrint(3, ("Device is Idle\n"));

        item = IoAllocateWorkItem(deviceObject);

        if(item) {

            IoQueueWorkItem(item,
                            IdleRequestWorkerRoutine,
                            DelayedWorkQueue,
                            item);

            ntStatus = STATUS_PENDING;

        }
        else {

            IsoUsb_DbgPrint(3, ("Cannot alloc memory for work item\n"));

            ntStatus = STATUS_INSUFFICIENT_RESOURCES;

            //
            // signal the NoDpcWorkItemPendingEvent.
            //
            KeSetEvent(&deviceExtension->NoDpcWorkItemPendingEvent,
                       IO_NO_INCREMENT,
                       FALSE);
        }
    }
    else {

        IsoUsb_DbgPrint(3, ("Idle event not signaled\n"));

        //
        // signal the NoDpcWorkItemPendingEvent.
        //
        KeSetEvent(&deviceExtension->NoDpcWorkItemPendingEvent,
                   IO_NO_INCREMENT,
                   FALSE);
    }

    IsoUsb_DbgPrint(3, ("DpcRoutine - ends\n"));
}


VOID
IdleRequestWorkerRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID          Context
    )
/*++

Routine Description:

    This is the work item fired from the DPC.
    This workitem checks the idle state of the device
    and submits an idle request.

Arguments:

    DeviceObject - pointer to device object
    Context - context for the work item.

Return Value:

    None

--*/
{
    PIRP                   irp;
    NTSTATUS               ntStatus;
    PDEVICE_EXTENSION      deviceExtension;
    PIO_WORKITEM           workItem;

    IsoUsb_DbgPrint(3, ("IdleRequestWorkerRoutine - begins\n"));

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    workItem = (PIO_WORKITEM) Context;

    if(CanDeviceSuspend(deviceExtension)) {

        IsoUsb_DbgPrint(3, ("Device is idle\n"));

        ntStatus = SubmitIdleRequestIrp(deviceExtension);

        if(!NT_SUCCESS(ntStatus)) {

            IsoUsb_DbgPrint(1, ("SubmitIdleRequestIrp failed\n"));
        }
    }
    else {

        IsoUsb_DbgPrint(3, ("Device is not idle\n"));
    }

    IoFreeWorkItem(workItem);

    //
    // signal the NoDpcWorkItemPendingEvent.
    //
    KeSetEvent(&deviceExtension->NoDpcWorkItemPendingEvent,
               IO_NO_INCREMENT,
               FALSE);

    IsoUsb_DbgPrint(3, ("IdleRequestsWorkerRoutine - ends\n"));
}


VOID
ProcessQueuedRequests(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    Remove and process the entries in the queue. If this routine is called
    when processing IRP_MN_CANCEL_STOP_DEVICE, IRP_MN_CANCEL_REMOVE_DEVICE
    or IRP_MN_START_DEVICE, the requests are passed to the next lower driver.
    If the routine is called when IRP_MN_REMOVE_DEVICE is received, the IRPs
    are complete with STATUS_DELETE_PENDING

Arguments:

    DeviceExtension - pointer to device extension

Return Value:

    None

--*/
{
    KIRQL       oldIrql;
    PIRP        nextIrp,
                cancelledIrp;
    PVOID       cancelRoutine;
    LIST_ENTRY  cancelledIrpList;
    PLIST_ENTRY listEntry;

    IsoUsb_DbgPrint(3, ("ProcessQueuedRequests - begins\n"));

    //
    // initialize variables
    //

    cancelRoutine = NULL;
    InitializeListHead(&cancelledIrpList);

    //
    // 1.  dequeue the entries in the queue
    // 2.  reset the cancel routine
    // 3.  process them
    // 3a. if the device is active, send them down
    // 3b. else complete with STATUS_DELETE_PENDING
    //

    while(1) {

        KeAcquireSpinLock(&DeviceExtension->QueueLock, &oldIrql);

        if(IsListEmpty(&DeviceExtension->NewRequestsQueue)) {

            KeReleaseSpinLock(&DeviceExtension->QueueLock, oldIrql);
            break;
        }

        //
        // Remove a request from the queue
        //

        listEntry = RemoveHeadList(&DeviceExtension->NewRequestsQueue);
        nextIrp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        //
        // set the cancel routine to NULL
        //

        cancelRoutine = IoSetCancelRoutine(nextIrp, NULL);

        //
        // check if its already cancelled
        //

        if(nextIrp->Cancel) {
            if(cancelRoutine) {

                //
                // the cancel routine for this IRP hasnt been called yet
                // so queue the IRP in the cancelledIrp list and complete
                // after releasing the lock
                //

                InsertTailList(&cancelledIrpList, listEntry);
            }
            else {

                //
                // the cancel routine has run
                // it must be waiting to hold the queue lock
                // so initialize the IRPs listEntry
                //

                InitializeListHead(listEntry);
            }

            KeReleaseSpinLock(&DeviceExtension->QueueLock, oldIrql);
        }
        else {

            KeReleaseSpinLock(&DeviceExtension->QueueLock, oldIrql);

            if(FailRequests == DeviceExtension->QueueState) {

                nextIrp->IoStatus.Information = 0;
                nextIrp->IoStatus.Status = STATUS_DELETE_PENDING;
                IoCompleteRequest(nextIrp, IO_NO_INCREMENT);
            }
            else {

                PIO_STACK_LOCATION irpStack;

                IsoUsb_DbgPrint(3, ("ProcessQueuedRequests::"));
                IsoUsb_IoIncrement(DeviceExtension);

                IoSkipCurrentIrpStackLocation(nextIrp);
                IoCallDriver(DeviceExtension->TopOfStackDeviceObject, nextIrp);

                IsoUsb_DbgPrint(3, ("ProcessQueuedRequests::"));
                IsoUsb_IoDecrement(DeviceExtension);
            }
        }
    } // while loop

    //
    // walk through the cancelledIrp list and cancel them
    //

    while(!IsListEmpty(&cancelledIrpList)) {

        PLIST_ENTRY cancelEntry = RemoveHeadList(&cancelledIrpList);

        cancelledIrp = CONTAINING_RECORD(cancelEntry, IRP, Tail.Overlay.ListEntry);

        cancelledIrp->IoStatus.Status = STATUS_CANCELLED;
        cancelledIrp->IoStatus.Information = 0;

        IoCompleteRequest(cancelledIrp, IO_NO_INCREMENT);
    }

    IsoUsb_DbgPrint(3, ("ProcessQueuedRequests - ends\n"));

    return;
}


VOID
GetBusInterfaceVersion(
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

    This routine queries the bus interface version

Arguments:

    DeviceExtension

Return Value:

    VOID

--*/
{
    PIRP                       irp;
    KEVENT                     event;
    NTSTATUS                   ntStatus;
    PDEVICE_EXTENSION          deviceExtension;
    PIO_STACK_LOCATION         nextStack;
    USB_BUS_INTERFACE_USBDI_V1 busInterfaceVer1;

    //
    // initialize vars
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    IsoUsb_DbgPrint(3, ("GetBusInterfaceVersion - begins\n"));

    irp = IoAllocateIrp(deviceExtension->TopOfStackDeviceObject->StackSize,
                        FALSE);

    if(NULL == irp) {

        IsoUsb_DbgPrint(1, ("Failed to alloc irp in GetBusInterfaceVersion\n"));
        return;
    }

    //
    // All pnp Irp's need the status field initialized to
    // STATUS_NOT_SUPPORTED
    //
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    // Set the completion routine, which will signal the event
    //
    IoSetCompletionRoutine(irp,
                           IsoUsb_SyncCompletionRoutine,
                           &event,
                           TRUE,    // InvokeOnSuccess
                           TRUE,    // InvokeOnError
                           TRUE);   // InvokeOnCancel

    nextStack = IoGetNextIrpStackLocation(irp);
    ASSERT(nextStack);
    nextStack->MajorFunction = IRP_MJ_PNP;
    nextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    //
    // Allocate memory for an interface of type
    // USB_BUS_INTERFACE_USBDI_V0 and have the IRP point to it:
    //
    nextStack->Parameters.QueryInterface.Interface =
                                (PINTERFACE) &busInterfaceVer1;

    //
    // Assign the InterfaceSpecificData member of the IRP to be NULL
    //
    nextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    //
    // Set the interface type to the appropriate GUID
    //
    nextStack->Parameters.QueryInterface.InterfaceType =
        &USB_BUS_INTERFACE_USBDI_GUID;

    //
    // Set the size and version of interface in the IRP
    // Currently, there is only one valid version of
    // this interface available to clients.
    //
    nextStack->Parameters.QueryInterface.Size =
        sizeof(USB_BUS_INTERFACE_USBDI_V1);

    nextStack->Parameters.QueryInterface.Version = USB_BUSIF_USBDI_VERSION_1;

    IsoUsb_IoIncrement(deviceExtension);

    ntStatus = IoCallDriver(DeviceObject,
                            irp);

    if(STATUS_PENDING == ntStatus) {

        KeWaitForSingleObject(&event,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);

        ntStatus = irp->IoStatus.Status;
    }

    if(NT_SUCCESS(ntStatus)) {

        deviceExtension->IsDeviceHighSpeed =
                busInterfaceVer1.IsDeviceHighSpeed(
                                       busInterfaceVer1.BusContext);

        IsoUsb_DbgPrint(1, ("IsDeviceHighSpeed = %x\n",
                            deviceExtension->IsDeviceHighSpeed));
    }

    IoFreeIrp(irp);

    IsoUsb_DbgPrint(3, ("GetBusInterfaceVersion::"));
    IsoUsb_IoDecrement(deviceExtension);

    IsoUsb_DbgPrint(3, ("GetBusInterfaceVersion - ends\n"));
}


NTSTATUS
IsoUsb_GetRegistryDword(
    IN     PWCHAR RegPath,
    IN     PWCHAR ValueName,
    IN OUT PULONG Value
    )
/*++

Routine Description:

    This routine reads the specified reqistry value.

Arguments:

    RegPath - registry path
    ValueName - property to be fetched from the registry
    Value - corresponding value read from the registry.

Return Value:

    NT status value

--*/
{
    ULONG                    defaultData;
    WCHAR                    buffer[MAXIMUM_FILENAME_LENGTH];
    NTSTATUS                 ntStatus;
    UNICODE_STRING           regPath;
    RTL_QUERY_REGISTRY_TABLE paramTable[2];

    IsoUsb_DbgPrint(3, ("IsoUsb_GetRegistryDword - begins\n"));

    regPath.Length = 0;
    regPath.MaximumLength = MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);
    regPath.Buffer = buffer;

    RtlZeroMemory(regPath.Buffer, regPath.MaximumLength);
    RtlMoveMemory(regPath.Buffer,
                  RegPath,
                  wcslen(RegPath) * sizeof(WCHAR));

    RtlZeroMemory(paramTable, sizeof(paramTable));

    paramTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[0].Name = ValueName;
    paramTable[0].EntryContext = Value;
    paramTable[0].DefaultType = REG_DWORD;
    paramTable[0].DefaultData = &defaultData;
    paramTable[0].DefaultLength = sizeof(ULONG);

    ntStatus = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE |
                                      RTL_REGISTRY_OPTIONAL,
                                      regPath.Buffer,
                                      paramTable,
                                      NULL,
                                      NULL);

    if(NT_SUCCESS(ntStatus)) {

        IsoUsb_DbgPrint(3, ("success Value = %X\n", *Value));
        return STATUS_SUCCESS;
    }
    else {

        *Value = 0;
        return STATUS_UNSUCCESSFUL;
    }
}


NTSTATUS
IsoUsb_DispatchClean(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
/*++

Routine Description:

    Dispatch routine for IRP_MJ_CLEANUP

Arguments:

    DeviceObject - pointer to device object
    Irp - I/O request packet sent by the pnp manager

Return Value:

    NT status value

--*/
{
    PDEVICE_EXTENSION     deviceExtension;
    KIRQL                 oldIrql;
    LIST_ENTRY            cleanupList;
    PLIST_ENTRY           thisEntry,
                          nextEntry,
                          listHead;
    PIRP                  pendingIrp;
    PIO_STACK_LOCATION    pendingIrpStack,
                          irpStack;
    NTSTATUS              ntStatus;
    PFILE_OBJECT          fileObject;
    PFILE_OBJECT_CONTENT  fileObjectContent;
    PISOUSB_STREAM_OBJECT tempStreamObject;

    //
    // initialize variables
    //
    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    fileObject = irpStack->FileObject;

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchClean::"));
    IsoUsb_IoIncrement(deviceExtension);

    //
    // check if any stream objects need to be cleaned
    //

    if(fileObject && fileObject->FsContext) {

        fileObjectContent = (PFILE_OBJECT_CONTENT)
                            fileObject->FsContext;

        if(fileObjectContent->StreamInformation) {

            tempStreamObject = (PISOUSB_STREAM_OBJECT)
                               InterlockedExchangePointer(
                                    &fileObjectContent->StreamInformation,
                                    NULL);

            if(tempStreamObject &&
               (tempStreamObject->DeviceObject == DeviceObject)) {

                IsoUsb_DbgPrint(3, ("clean dispatch routine"
                                    " found a stream object match\n"));
                IsoUsb_StreamObjectCleanup(tempStreamObject, deviceExtension);
            }
        }
    }

    InitializeListHead(&cleanupList);

    //
    // acquire queue lock
    //
    KeAcquireSpinLock(&deviceExtension->QueueLock, &oldIrql);

    //
    // remove all Irp's that belong to input Irp's fileobject
    //

    listHead = &deviceExtension->NewRequestsQueue;

    for(thisEntry = listHead->Flink, nextEntry = thisEntry->Flink;
       thisEntry != listHead;
       thisEntry = nextEntry, nextEntry = thisEntry->Flink) {

        pendingIrp = CONTAINING_RECORD(thisEntry, IRP, Tail.Overlay.ListEntry);

        pendingIrpStack = IoGetCurrentIrpStackLocation(pendingIrp);

        if(irpStack->FileObject == pendingIrpStack->FileObject) {

            RemoveEntryList(thisEntry);

            //
            // set the cancel routine to NULL
            //
            if(NULL == IoSetCancelRoutine(pendingIrp, NULL)) {

                InitializeListHead(thisEntry);
            }
            else {

                InsertTailList(&cleanupList, thisEntry);
            }
        }
    }

    //
    // Release the spin lock
    //

    KeReleaseSpinLock(&deviceExtension->QueueLock, oldIrql);

    //
    // walk thru the cleanup list and cancel all the Irps
    //

    while(!IsListEmpty(&cleanupList)) {

        //
        // complete the Irp
        //
        thisEntry = RemoveHeadList(&cleanupList);

        pendingIrp = CONTAINING_RECORD(thisEntry, IRP, Tail.Overlay.ListEntry);

        pendingIrp->IoStatus.Information = 0;
        pendingIrp->IoStatus.Status = STATUS_CANCELLED;

        IoCompleteRequest(pendingIrp, IO_NO_INCREMENT);
    }

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    IsoUsb_DbgPrint(3, ("IsoUsb_DispatchClean::"));
    IsoUsb_IoDecrement(deviceExtension);

    return STATUS_SUCCESS;
}


NTSTATUS
ReleaseMemory(
    IN PDEVICE_OBJECT DeviceObject
    )
/*++

Routine Description:

    This routine returns all the memory allocations acquired during
    device startup.

Arguments:

    DeviceObject - pointer to the device object.


Return Value:

    STATUS_SUCCESS if the device can be safely removed, an appropriate
    NT Status if not.

--*/
{
    PDEVICE_EXTENSION   deviceExtension;
    ULONG               numInterfaces;
    ULONG               i;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (deviceExtension->InterfaceInfoList != NULL)
    {
        numInterfaces = deviceExtension->ConfigurationDescriptor->bNumInterfaces;

        // Free the previously allocated USBD_INTERFACE_INFORMATION
        // structure copies.
        //
        for (i = 0; i < numInterfaces; i++)
        {
            if (deviceExtension->InterfaceInfoList[i] != NULL)
            {
                ExFreePool(deviceExtension->InterfaceInfoList[i]);
            }
        }

        // Free the list of pointers to allocated
        // USBD_INTERFACE_INFORMATION structure copies.
        //
        ExFreePool(deviceExtension->InterfaceInfoList);
        deviceExtension->InterfaceInfoList = NULL;
    }

    if (deviceExtension->ConfigurationDescriptor != NULL)
    {
        ExFreePool(deviceExtension->ConfigurationDescriptor);
        deviceExtension->ConfigurationDescriptor = NULL;
    }

    if (deviceExtension->DeviceDescriptor != NULL)
    {
        ExFreePool(deviceExtension->DeviceDescriptor);
        deviceExtension->DeviceDescriptor = NULL;
    }

    return STATUS_SUCCESS;
}


LONG
IsoUsb_IoIncrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine bumps up the I/O count.
    This routine is typically invoked when any of the
    dispatch routines handle new irps for the driver.

Arguments:

    DeviceExtension - pointer to device extension

Return Value:

    new value

--*/
{
    LONG  result = 0;
    KIRQL oldIrql;

    KeAcquireSpinLock(&DeviceExtension->IOCountLock, &oldIrql);

    result = InterlockedIncrement(&DeviceExtension->OutStandingIO);

    //
    // when OutStandingIO bumps from 1 to 2, clear the StopEvent
    //

    if(result == 2) {

        KeClearEvent(&DeviceExtension->StopEvent);
    }

    KeReleaseSpinLock(&DeviceExtension->IOCountLock, oldIrql);

    IsoUsb_DbgPrint(3, ("IsoUsb_IoIncrement::%d\n", result));

    return result;
}


LONG
IsoUsb_IoDecrement(
    IN OUT PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine decrements the outstanding I/O count
    This is typically invoked after the dispatch routine
    has finished processing the irp.

Arguments:

    DeviceExtension - pointer to device extension

Return Value:

    new value

--*/
{
    LONG  result = 0;
    KIRQL oldIrql;

    KeAcquireSpinLock(&DeviceExtension->IOCountLock, &oldIrql);

    result = InterlockedDecrement(&DeviceExtension->OutStandingIO);

    if(result == 1) {

        KeSetEvent(&DeviceExtension->StopEvent, IO_NO_INCREMENT, FALSE);
    }

    if(result == 0) {

        ASSERT(Removed == DeviceExtension->DeviceState);

        KeSetEvent(&DeviceExtension->RemoveEvent, IO_NO_INCREMENT, FALSE);
    }

    KeReleaseSpinLock(&DeviceExtension->IOCountLock, oldIrql);

    IsoUsb_DbgPrint(3, ("IsoUsb_IoDecrement::%d\n", result));

    return result;
}


BOOLEAN
CanDeviceSuspend(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This is the routine where we check if the device
    can selectively suspend.

Arguments:

    DeviceExtension - pointer to device extension

Return Value:

    TRUE - if the device can suspend
    FALSE - otherwise.

--*/
{
    IsoUsb_DbgPrint(3, ("CanDeviceSuspend\n"));

    if((DeviceExtension->OpenHandleCount == 0) &&
        (DeviceExtension->OutStandingIO == 1)) {

        return TRUE;
    }
    else {

        return FALSE;
    }
}


#if DBG


PCHAR
PnPMinorFunctionString (
    UCHAR MinorFunction
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    switch (MinorFunction) {

        case IRP_MN_START_DEVICE:
            return "IRP_MN_START_DEVICE\n";

        case IRP_MN_QUERY_REMOVE_DEVICE:
            return "IRP_MN_QUERY_REMOVE_DEVICE\n";

        case IRP_MN_REMOVE_DEVICE:
            return "IRP_MN_REMOVE_DEVICE\n";

        case IRP_MN_CANCEL_REMOVE_DEVICE:
            return "IRP_MN_CANCEL_REMOVE_DEVICE\n";

        case IRP_MN_STOP_DEVICE:
            return "IRP_MN_STOP_DEVICE\n";

        case IRP_MN_QUERY_STOP_DEVICE:
            return "IRP_MN_QUERY_STOP_DEVICE\n";

        case IRP_MN_CANCEL_STOP_DEVICE:
            return "IRP_MN_CANCEL_STOP_DEVICE\n";

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            return "IRP_MN_QUERY_DEVICE_RELATIONS\n";

        case IRP_MN_QUERY_INTERFACE:
            return "IRP_MN_QUERY_INTERFACE\n";

        case IRP_MN_QUERY_CAPABILITIES:
            return "IRP_MN_QUERY_CAPABILITIES\n";

        case IRP_MN_QUERY_RESOURCES:
            return "IRP_MN_QUERY_RESOURCES\n";

        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
            return "IRP_MN_QUERY_RESOURCE_REQUIREMENTS\n";

        case IRP_MN_QUERY_DEVICE_TEXT:
            return "IRP_MN_QUERY_DEVICE_TEXT\n";

        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
            return "IRP_MN_FILTER_RESOURCE_REQUIREMENTS\n";

        case IRP_MN_READ_CONFIG:
            return "IRP_MN_READ_CONFIG\n";

        case IRP_MN_WRITE_CONFIG:
            return "IRP_MN_WRITE_CONFIG\n";

        case IRP_MN_EJECT:
            return "IRP_MN_EJECT\n";

        case IRP_MN_SET_LOCK:
            return "IRP_MN_SET_LOCK\n";

        case IRP_MN_QUERY_ID:
            return "IRP_MN_QUERY_ID\n";

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            return "IRP_MN_QUERY_PNP_DEVICE_STATE\n";

        case IRP_MN_QUERY_BUS_INFORMATION:
            return "IRP_MN_QUERY_BUS_INFORMATION\n";

        case IRP_MN_DEVICE_USAGE_NOTIFICATION:
            return "IRP_MN_DEVICE_USAGE_NOTIFICATION\n";

        case IRP_MN_SURPRISE_REMOVAL:
            return "IRP_MN_SURPRISE_REMOVAL\n";

        default:
            return "IRP_MN_?????\n";
    }
}


VOID
DumpDeviceDesc (
    PUSB_DEVICE_DESCRIPTOR   DeviceDesc
)
/*++

Routine Description:

    Debug only return to print a Device Descriptor to the debugger.

Arguments:

    DeviceDesc - The Device Descriptor

Return Value:

--*/
{
    IsoUsb_DbgPrint(3, ("------------------\n"));
    IsoUsb_DbgPrint(3, ("Device Descriptor:\n"));

    IsoUsb_DbgPrint(3, ("bcdUSB:             0x%04X\n",
                        DeviceDesc->bcdUSB));

    IsoUsb_DbgPrint(3, ("bDeviceClass:         0x%02X\n",
                        DeviceDesc->bDeviceClass));

    IsoUsb_DbgPrint(3, ("bDeviceSubClass:      0x%02X\n",
                        DeviceDesc->bDeviceSubClass));

    IsoUsb_DbgPrint(3, ("bDeviceProtocol:      0x%02X\n",
                        DeviceDesc->bDeviceProtocol));

    IsoUsb_DbgPrint(3, ("bMaxPacketSize0:      0x%02X (%d)\n",
                        DeviceDesc->bMaxPacketSize0,
                        DeviceDesc->bMaxPacketSize0));

    IsoUsb_DbgPrint(3, ("idVendor:           0x%04X\n",
                        DeviceDesc->idVendor));

    IsoUsb_DbgPrint(3, ("idProduct:          0x%04X\n",
                        DeviceDesc->idProduct));

    IsoUsb_DbgPrint(3, ("bcdDevice:          0x%04X\n",
                        DeviceDesc->bcdDevice));

    IsoUsb_DbgPrint(3, ("iManufacturer:        0x%02X\n",
                        DeviceDesc->iManufacturer));

    IsoUsb_DbgPrint(3, ("iProduct:             0x%02X\n",
                        DeviceDesc->iProduct));

    IsoUsb_DbgPrint(3, ("iSerialNumber:        0x%02X\n",
                        DeviceDesc->iSerialNumber));

    IsoUsb_DbgPrint(3, ("bNumConfigurations:   0x%02X\n",
                        DeviceDesc->bNumConfigurations));

}


VOID
DumpConfigDesc (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
/*++

Routine Description:

    Debug only return to print an entire Configuration Descriptor to the
    debugger.

Arguments:

    ConfigDesc - The Configuration Descriptor, and associated Interface
    and EndpointDescriptors

Return Value:

--*/
{
    PUCHAR                  descEnd;
    PUSB_COMMON_DESCRIPTOR  commonDesc;
    BOOLEAN                 dumpUnknown;

    descEnd = (PUCHAR)ConfigDesc + ConfigDesc->wTotalLength;

    commonDesc = (PUSB_COMMON_DESCRIPTOR)ConfigDesc;

    while ((PUCHAR)commonDesc + sizeof(USB_COMMON_DESCRIPTOR) < descEnd &&
           (PUCHAR)commonDesc + commonDesc->bLength <= descEnd)
    {
        dumpUnknown = FALSE;

        switch (commonDesc->bDescriptorType)
        {
            case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_CONFIGURATION_DESCRIPTOR))
                {
                    dumpUnknown = TRUE;
                    break;
                }
                DumpConfigurationDescriptor((PUSB_CONFIGURATION_DESCRIPTOR)commonDesc);
                break;

            case USB_INTERFACE_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_INTERFACE_DESCRIPTOR))
                {
                    dumpUnknown = TRUE;
                    break;
                }
                DumpInterfaceDescriptor((PUSB_INTERFACE_DESCRIPTOR)commonDesc);
                break;

            case USB_ENDPOINT_DESCRIPTOR_TYPE:
                if (commonDesc->bLength != sizeof(USB_ENDPOINT_DESCRIPTOR))
                {
                    dumpUnknown = TRUE;
                    break;
                }
                DumpEndpointDescriptor((PUSB_ENDPOINT_DESCRIPTOR)commonDesc);
                break;

            default:
                dumpUnknown = TRUE;
                break;
        }

        if (dumpUnknown)
        {
            // DumpUnknownDescriptor(commonDesc);
        }

        (PUCHAR)commonDesc += commonDesc->bLength;
    }
}


VOID
DumpConfigurationDescriptor (
    PUSB_CONFIGURATION_DESCRIPTOR   ConfigDesc
)
/*++

Routine Description:

    Debug only return to print a Configuration Descriptor to the
    debugger.

Arguments:

    ConfigDesc - The Configuration Descriptor

Return Value:

--*/
{
    IsoUsb_DbgPrint(3, ("-------------------------\n"));
    IsoUsb_DbgPrint(3, ("Configuration Descriptor:\n"));

    IsoUsb_DbgPrint(3, ("wTotalLength:       0x%04X\n",
                        ConfigDesc->wTotalLength));

    IsoUsb_DbgPrint(3, ("bNumInterfaces:       0x%02X\n",
                        ConfigDesc->bNumInterfaces));

    IsoUsb_DbgPrint(3, ("bConfigurationValue:  0x%02X\n",
                        ConfigDesc->bConfigurationValue));

    IsoUsb_DbgPrint(3, ("iConfiguration:       0x%02X\n",
                        ConfigDesc->iConfiguration));

    IsoUsb_DbgPrint(3, ("bmAttributes:         0x%02X\n",
                        ConfigDesc->bmAttributes));

    if (ConfigDesc->bmAttributes & 0x80)
    {
        IsoUsb_DbgPrint(3, ("  Bus Powered\n"));
    }

    if (ConfigDesc->bmAttributes & 0x40)
    {
        IsoUsb_DbgPrint(3, ("  Self Powered\n"));
    }

    if (ConfigDesc->bmAttributes & 0x20)
    {
        IsoUsb_DbgPrint(3, ("  Remote Wakeup\n"));
    }

    IsoUsb_DbgPrint(3, ("MaxPower:             0x%02X (%d Ma)\n",
                        ConfigDesc->MaxPower,
                        ConfigDesc->MaxPower * 2));

}


VOID
DumpInterfaceDescriptor (
    PUSB_INTERFACE_DESCRIPTOR   InterfaceDesc
)
/*++

Routine Description:

    Debug only return to print an Interface Descriptor to the debugger.

Arguments:

    InterfaceDesc - The Interface Descriptor.

Return Value:

--*/
{
    IsoUsb_DbgPrint(3, ("---------------------\n"));
    IsoUsb_DbgPrint(3, ("Interface Descriptor:\n"));

    IsoUsb_DbgPrint(3, ("bInterfaceNumber:     0x%02X\n",
                        InterfaceDesc->bInterfaceNumber));

    IsoUsb_DbgPrint(3, ("bAlternateSetting:    0x%02X\n",
                        InterfaceDesc->bAlternateSetting));

    IsoUsb_DbgPrint(3, ("bNumEndpoints:        0x%02X\n",
                        InterfaceDesc->bNumEndpoints));

    IsoUsb_DbgPrint(3, ("bInterfaceClass:      0x%02X\n",
                        InterfaceDesc->bInterfaceClass));

    IsoUsb_DbgPrint(3, ("bInterfaceSubClass:   0x%02X\n",
                        InterfaceDesc->bInterfaceSubClass));

    IsoUsb_DbgPrint(3, ("bInterfaceProtocol:   0x%02X\n",
                        InterfaceDesc->bInterfaceProtocol));

    IsoUsb_DbgPrint(3, ("iInterface:           0x%02X\n",
                        InterfaceDesc->iInterface));
}


VOID
DumpEndpointDescriptor (
    PUSB_ENDPOINT_DESCRIPTOR    EndpointDesc
)
/*++

Routine Description:

    Debug only return to print an Endpoint Descriptor to the debugger.

Arguments:

    EndpointDesc - The Endpoint Descriptor.

Return Value:

--*/
{
    IsoUsb_DbgPrint(3, ("--------------------\n"));
    IsoUsb_DbgPrint(3, ("Endpoint Descriptor:\n"));

    IsoUsb_DbgPrint(3, ("bEndpointAddress:     0x%02X\n",
                        EndpointDesc->bEndpointAddress));

    switch (EndpointDesc->bmAttributes & 0x03)
    {
        case 0x00:
            IsoUsb_DbgPrint(3, ("Transfer Type:     Control\n"));
            break;

        case 0x01:
            IsoUsb_DbgPrint(3, ("Transfer Type: Isochronous\n"));
            break;

        case 0x02:
            IsoUsb_DbgPrint(3, ("Transfer Type:        Bulk\n"));
            break;

        case 0x03:
            IsoUsb_DbgPrint(3, ("Transfer Type:   Interrupt\n"));
            break;
    }

    IsoUsb_DbgPrint(3, ("wMaxPacketSize:     0x%04X (%d)\n",
                        EndpointDesc->wMaxPacketSize,
                        EndpointDesc->wMaxPacketSize));

    IsoUsb_DbgPrint(3, ("bInterval:            0x%02X\n",
                        EndpointDesc->bInterval));
}


VOID
DumpInterfaceInformation (
    PUSBD_INTERFACE_INFORMATION InterfaceInfo
)
/*++

Routine Description:

    Debug only return to print a USBD_INTERFACE_INFORMATION structure to
    the debugger.

Arguments:

    InterfaceDesc - The Interface Descriptor.

Return Value:

--*/
{
    ULONG   i;

    IsoUsb_DbgPrint(3, ("---------\n"));
    IsoUsb_DbgPrint(3, ("Length:           0x%X\n",
                        InterfaceInfo->Length));
    IsoUsb_DbgPrint(3, ("Interface Number: 0x%X\n",
                        InterfaceInfo->InterfaceNumber));
    IsoUsb_DbgPrint(3, ("Alt Setting:      0x%X\n",
                        InterfaceInfo->AlternateSetting));
    IsoUsb_DbgPrint(3, ("Class, Subclass, Protocol: 0x%X 0x%X 0x%X\n",
                        InterfaceInfo->Class,
                        InterfaceInfo->SubClass,
                        InterfaceInfo->Protocol));
    IsoUsb_DbgPrint(3, ("NumberOfPipes:    0x%X\n",
                        InterfaceInfo->NumberOfPipes));

    for (i=0; i < InterfaceInfo->NumberOfPipes; i++) {

        IsoUsb_DbgPrint(3, ("---------\n"));
        IsoUsb_DbgPrint(3, ("PipeType:         0x%X  %s\n",
                            InterfaceInfo->Pipes[i].PipeType,
                            UsbdPipeTypeString(InterfaceInfo->Pipes[i].PipeType)));
        IsoUsb_DbgPrint(3, ("EndpointAddress:  0x%X\n",
                            InterfaceInfo->Pipes[i].EndpointAddress));
        IsoUsb_DbgPrint(3, ("MaxPacketSize:    0x%X\n",
                            InterfaceInfo->Pipes[i].MaximumPacketSize));
        IsoUsb_DbgPrint(3, ("Interval:         0x%X\n",
                            InterfaceInfo->Pipes[i].Interval));
        IsoUsb_DbgPrint(3, ("Handle:           0x%X\n",
                            InterfaceInfo->Pipes[i].PipeHandle));
        IsoUsb_DbgPrint(3, ("MaxTransferSize:  0x%X\n",
                            InterfaceInfo->Pipes[i].MaximumTransferSize));
    }

    IsoUsb_DbgPrint(3, ("---------\n"));
}


PCHAR
UsbdPipeTypeString (
    USBD_PIPE_TYPE  PipeType
    )
/*++

Routine Description:

Arguments:

Return Value:

--*/
{
    switch (PipeType)
    {
        case UsbdPipeTypeControl:
            return "UsbdPipeTypeControl";

        case UsbdPipeTypeIsochronous:
            return "UsbdPipeTypeIsochronous";

        case UsbdPipeTypeBulk:
            return "UsbdPipeTypeBulk";

        case UsbdPipeTypeInterrupt:
            return "UsbdPipeTypeInterrupt";

        default:
            return "UsbdPipeType?????\n";
    }
}


#endif

