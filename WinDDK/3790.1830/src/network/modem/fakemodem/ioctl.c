/*
 * UNIMODEM "Fakemodem" controllerless driver illustrative example
 *
 * (C) 2000 Microsoft Corporation
 * All Rights Reserved
 *
 */

#include "fakemodem.h"

NTSTATUS
FakeModemIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status=STATUS_UNSUCCESSFUL;
    KIRQL             OldIrql;
    PIO_STACK_LOCATION  IrpSp;

    Irp->IoStatus.Information = 0;

    //  make sure the device is ready for irp's

    status=CheckStateAndAddReference( DeviceObject, Irp);

    if (STATUS_SUCCESS != status) 
    {
        //  not accepting irp's. The irp has already been complted
        return status;

    }

    IrpSp=IoGetCurrentIrpStackLocation(Irp);

    D_TRACE(DbgPrint("FakeModemIoControl: received IOCTL %08lx\n",
                IrpSp->Parameters.DeviceIoControl.IoControlCode);)

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_SERIAL_GET_WAIT_MASK: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < 
                    sizeof(ULONG)) 
            {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *((PULONG)Irp->AssociatedIrp.SystemBuffer)=
                deviceExtension->CurrentMask;

            Irp->IoStatus.Information=sizeof(ULONG);

            break;
        }

        case IOCTL_SERIAL_SET_WAIT_MASK: {

            PIRP    CurrentWaitIrp=NULL;
            ULONG NewMask;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < 
                    sizeof(ULONG)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            } else {

                KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

                //  get rid of the current wait

                CurrentWaitIrp=deviceExtension->CurrentMaskIrp;

                deviceExtension->CurrentMaskIrp=NULL;

                //  save the new mask

                NewMask = *((ULONG *)Irp->AssociatedIrp.SystemBuffer);

                deviceExtension->CurrentMask=NewMask;

                D_TRACE(DbgPrint("FAKEMODEM: set wait mask, %08lx\n",NewMask);)

                KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

                if (CurrentWaitIrp != NULL) {

                    D_TRACE(DbgPrint("FAKEMODEM: set wait mask- complete wait\n");)

                    *((PULONG)CurrentWaitIrp->AssociatedIrp.SystemBuffer)=0;

                    CurrentWaitIrp->IoStatus.Information=sizeof(ULONG);

                    RemoveReferenceAndCompleteRequest(
                        deviceExtension->DeviceObject,
                        CurrentWaitIrp, STATUS_SUCCESS);
                }


            }

            break;
        }

        case IOCTL_SERIAL_WAIT_ON_MASK: {

            PIRP    CurrentWaitIrp=NULL;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < 
                    sizeof(ULONG)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            D_TRACE(DbgPrint("FAKEMODEM: wait on mask\n");)

            KeAcquireSpinLock(&deviceExtension->SpinLock, &OldIrql);

            //
            //  capture the current irp if any
            //
            CurrentWaitIrp=deviceExtension->CurrentMaskIrp;

            deviceExtension->CurrentMaskIrp=NULL;


            if (deviceExtension->CurrentMask == 0) {
                //
                // can only set if mask is not zero
                //
                status=STATUS_UNSUCCESSFUL;

            } else {

                deviceExtension->CurrentMaskIrp=Irp;

                Irp->IoStatus.Status=STATUS_PENDING;
                IoMarkIrpPending(Irp);
#if DBG
                Irp=NULL;
#endif

                status=STATUS_PENDING;
            }

            KeReleaseSpinLock(
                &deviceExtension->SpinLock,
                OldIrql
                );

            if (CurrentWaitIrp != NULL) {

                D_TRACE(DbgPrint("FAKEMODEM: wait on mask- complete wait\n");)

                *((PULONG)CurrentWaitIrp->AssociatedIrp.SystemBuffer)=0;

                CurrentWaitIrp->IoStatus.Information=sizeof(ULONG);

                RemoveReferenceAndCompleteRequest(
                    deviceExtension->DeviceObject,
                    CurrentWaitIrp, STATUS_SUCCESS);
            }


            break;
        }

        case IOCTL_SERIAL_PURGE: {

            PIRP CancelIrp;

            ULONG Mask=*((PULONG)Irp->AssociatedIrp.SystemBuffer);

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(ULONG)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            if (Mask & SERIAL_PURGE_RXABORT) {

                KeAcquireSpinLock(
                    &deviceExtension->SpinLock,
                    &OldIrql
                    );

                while ( !IsListEmpty(&deviceExtension->ReadQueue)) {

                    PLIST_ENTRY         ListElement;
                    KIRQL               CancelIrql;

                    ListElement=RemoveHeadList(
                        &deviceExtension->ReadQueue
                        );

                    CancelIrp=CONTAINING_RECORD(ListElement,IRP,
                            Tail.Overlay.ListEntry);

                    IoAcquireCancelSpinLock(&CancelIrql);

                    if (CancelIrp->Cancel) {
                        //
                        //  this one has been canceled
                        //
                        CancelIrp->IoStatus.Information=STATUS_CANCELLED;

                        IoReleaseCancelSpinLock(CancelIrql);

                        continue;
                    }

                    IoSetCancelRoutine( CancelIrp, NULL);

                    IoReleaseCancelSpinLock(CancelIrql);

                    KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

                    CancelIrp->IoStatus.Information=0;

                    RemoveReferenceAndCompleteRequest(
                        deviceExtension->DeviceObject, CancelIrp, STATUS_CANCELLED);

                    KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);
                }

                CancelIrp=NULL;

                if (deviceExtension->CurrentReadIrp != NULL) 
                {
                    //
                    // get the current one
                    //
                    CancelIrp=deviceExtension->CurrentReadIrp;

                    deviceExtension->CurrentReadIrp=NULL;

                }

                KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

                if (CancelIrp != NULL) {

                    CancelIrp->IoStatus.Information=0;

                    RemoveReferenceAndCompleteRequest(
                        deviceExtension->DeviceObject, CancelIrp, STATUS_CANCELLED);

                }

            }


            break;
        }


        case IOCTL_SERIAL_GET_MODEMSTATUS: {


            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(ULONG)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            Irp->IoStatus.Information=sizeof(ULONG);

            *((PULONG)Irp->AssociatedIrp.SystemBuffer)=
                deviceExtension->ModemStatus;

            break;
        }

        case IOCTL_SERIAL_SET_TIMEOUTS: {

            PSERIAL_TIMEOUTS NewTimeouts =
                ((PSERIAL_TIMEOUTS)(Irp->AssociatedIrp.SystemBuffer));

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                sizeof(SERIAL_TIMEOUTS)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            if ((NewTimeouts->ReadIntervalTimeout == MAXULONG) &&
                (NewTimeouts->ReadTotalTimeoutMultiplier == MAXULONG) &&
                (NewTimeouts->ReadTotalTimeoutConstant == MAXULONG))
            {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

            deviceExtension->CurrentTimeouts.ReadIntervalTimeout =
                NewTimeouts->ReadIntervalTimeout;

            deviceExtension->CurrentTimeouts.ReadTotalTimeoutMultiplier =
                NewTimeouts->ReadTotalTimeoutMultiplier;

            deviceExtension->CurrentTimeouts.ReadTotalTimeoutConstant =
                NewTimeouts->ReadTotalTimeoutConstant;

            deviceExtension->CurrentTimeouts.WriteTotalTimeoutMultiplier =
                NewTimeouts->WriteTotalTimeoutMultiplier;

            deviceExtension->CurrentTimeouts.WriteTotalTimeoutConstant =
                NewTimeouts->WriteTotalTimeoutConstant;

            KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

            break;
        }

        case IOCTL_SERIAL_GET_TIMEOUTS: {

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_TIMEOUTS)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

            *((PSERIAL_TIMEOUTS)Irp->AssociatedIrp.SystemBuffer) = 
                deviceExtension->CurrentTimeouts;

            Irp->IoStatus.Information = sizeof(SERIAL_TIMEOUTS);

            KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

            break;
        }

        case IOCTL_SERIAL_GET_COMMSTATUS: {

            PSERIAL_STATUS SerialStatus=(PSERIAL_STATUS)Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_STATUS)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            RtlZeroMemory( SerialStatus, sizeof(SERIAL_STATUS));

            KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

            SerialStatus->AmountInInQueue=deviceExtension->BytesInReadBuffer;

            KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);


            Irp->IoStatus.Information = sizeof(SERIAL_STATUS);

            break;
        }

        case IOCTL_SERIAL_SET_DTR:
        case IOCTL_SERIAL_CLR_DTR: {


            KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

            if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_SERIAL_SET_DTR) {
                //
                //  raising DTR
                //
                deviceExtension->ModemStatus=SERIAL_DTR_STATE | SERIAL_DSR_STATE;

                D_TRACE(DbgPrint("FAKEMODEM: Set DTR\n");)

            } else {
                //
                //  dropping DTR, drop connection if there is one
                //
                D_TRACE(DbgPrint("FAKEMODEM: Clear DTR\n");)

                if (deviceExtension->CurrentlyConnected == TRUE) {
                    //
                    //  not connected any more
                    //
                    deviceExtension->CurrentlyConnected=FALSE;

                    deviceExtension->ConnectionStateChanged=TRUE;
                }
            }


            KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);

            ProcessConnectionStateChange( DeviceObject);


            break;
        }


        case IOCTL_SERIAL_SET_QUEUE_SIZE: {

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < 
                    sizeof(SERIAL_QUEUE_SIZE))
            {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            break;
        }


        case IOCTL_SERIAL_SET_BAUD_RATE: {

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < 
                    sizeof(SERIAL_BAUD_RATE)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            } else {

                KeAcquireSpinLock( &deviceExtension->SpinLock, &OldIrql);

                deviceExtension->BaudRate = ((PSERIAL_BAUD_RATE)(Irp->AssociatedIrp.SystemBuffer))->BaudRate;
                KeReleaseSpinLock( &deviceExtension->SpinLock, OldIrql);
            }


            break;
        }

        case IOCTL_SERIAL_GET_BAUD_RATE: {

            PSERIAL_BAUD_RATE pBaudRate = (PSERIAL_BAUD_RATE)Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(SERIAL_BAUD_RATE)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;

            } else {

                KeAcquireSpinLock(&deviceExtension->SpinLock, &OldIrql);

                pBaudRate->BaudRate = deviceExtension->BaudRate;

                KeReleaseSpinLock(&deviceExtension->SpinLock, OldIrql);

                Irp->IoStatus.Information = sizeof(SERIAL_BAUD_RATE);

            }

            break;
        }

        case IOCTL_SERIAL_SET_LINE_CONTROL: {

            PSERIAL_LINE_CONTROL pLineControl = ((PSERIAL_LINE_CONTROL)(Irp->AssociatedIrp.SystemBuffer));
            UCHAR LData = 0;
            UCHAR LStop = 0;
            UCHAR LParity = 0;
            UCHAR Mask = 0xff;

            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                    sizeof(SERIAL_LINE_CONTROL)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            switch(pLineControl->WordLength)
            {
                case 5: {

                    LData = SERIAL_5_DATA;
                    Mask = 0x1f;
                    break;

                }
                case 6: {

                    LData = SERIAL_6_DATA;
                    Mask = 0x3f;
                    break;

                }
                case 7: {

                    LData = SERIAL_7_DATA;
                    Mask = 0x7f;
                    break;

                }
                case 8: {

                    LData = SERIAL_8_DATA;
                    break;

                }
                default: {

                    status = STATUS_INVALID_PARAMETER;

                }
            }

            if (status != STATUS_SUCCESS)
            {
                break;
            }

            switch (pLineControl->Parity) {

                case NO_PARITY: {
                    LParity = SERIAL_NONE_PARITY;
                    break;

                }
                case EVEN_PARITY: {
                    LParity = SERIAL_EVEN_PARITY;
                    break;

                }
                case ODD_PARITY: {
                    LParity = SERIAL_ODD_PARITY;
                    break;

                }
                case SPACE_PARITY: {
                    LParity = SERIAL_SPACE_PARITY;
                    break;

                }
                case MARK_PARITY: {
                    LParity = SERIAL_MARK_PARITY;
                    break;

                }
                default: {

                    status = STATUS_INVALID_PARAMETER;
                    break;
                }

            }

            if (status != STATUS_SUCCESS)
            {
                break;
            }

            switch (pLineControl->StopBits) {

                case STOP_BIT_1: {

                    LStop = SERIAL_1_STOP;
                    break;
                }

                case STOP_BITS_1_5: {

                    if (LData != SERIAL_5_DATA) {

                        status = STATUS_INVALID_PARAMETER;
                        break;
                    }
                    LStop = SERIAL_1_5_STOP;
                    break;

                }
                case STOP_BITS_2: {

                    if (LData == SERIAL_5_DATA) {

                        status = STATUS_INVALID_PARAMETER;
                        break;
                    }

                    LStop = SERIAL_2_STOP;
                    break;
                }

                default: {

                    status = STATUS_INVALID_PARAMETER;
                }
                                  
            }

            if (status != STATUS_SUCCESS)
            {
                break;
            }


            KeAcquireSpinLock(&deviceExtension->SpinLock, &OldIrql);

            deviceExtension->LineControl =
                (UCHAR)((deviceExtension->LineControl & SERIAL_LCR_BREAK) |
                        (LData | LParity | LStop));

            deviceExtension->ValidDataMask = Mask;

            KeReleaseSpinLock(&deviceExtension->SpinLock, OldIrql);

            break;
        }

        case IOCTL_SERIAL_GET_LINE_CONTROL: {

            PSERIAL_LINE_CONTROL pLineControl = 
                (PSERIAL_LINE_CONTROL)Irp->AssociatedIrp.SystemBuffer;

            if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
                sizeof(SERIAL_LINE_CONTROL)) {

                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            RtlZeroMemory(Irp->AssociatedIrp.SystemBuffer,
                          IrpSp->Parameters.DeviceIoControl.OutputBufferLength);

            KeAcquireSpinLock(&deviceExtension->SpinLock, &OldIrql);

            if ((deviceExtension->LineControl & SERIAL_DATA_MASK) == SERIAL_5_DATA) {

                pLineControl->WordLength = 5;

            } else if ((deviceExtension->LineControl & SERIAL_DATA_MASK) == SERIAL_6_DATA) {

                pLineControl->WordLength = 6;

            } else if ((deviceExtension->LineControl & SERIAL_DATA_MASK) == SERIAL_7_DATA) {

                pLineControl->WordLength = 7;

            } else if ((deviceExtension->LineControl & SERIAL_DATA_MASK) == SERIAL_8_DATA) {

                pLineControl->WordLength = 8;
            }

            if ((deviceExtension->LineControl & SERIAL_PARITY_MASK) == SERIAL_NONE_PARITY) {

                pLineControl->Parity = NO_PARITY;

            } else if ((deviceExtension->LineControl & SERIAL_PARITY_MASK) == SERIAL_ODD_PARITY) {

                pLineControl->Parity = ODD_PARITY;

            } else if ((deviceExtension->LineControl & SERIAL_PARITY_MASK) == SERIAL_EVEN_PARITY) {

                pLineControl->Parity = EVEN_PARITY;

            } else if ((deviceExtension->LineControl & SERIAL_PARITY_MASK) == SERIAL_MARK_PARITY) {

                pLineControl->Parity = MARK_PARITY;

            } else if ((deviceExtension->LineControl & SERIAL_PARITY_MASK) == SERIAL_SPACE_PARITY) {

                pLineControl->Parity = SPACE_PARITY;
            }

            if (deviceExtension->LineControl & SERIAL_2_STOP) {

                if (pLineControl->WordLength == 5) {

                    pLineControl->StopBits = STOP_BITS_1_5;

                } else {

                    pLineControl->StopBits = STOP_BITS_2;

                }

            } else {

                pLineControl->StopBits = STOP_BIT_1;
            }

            Irp->IoStatus.Information = sizeof(SERIAL_LINE_CONTROL);

            KeReleaseSpinLock(&deviceExtension->SpinLock, OldIrql);

            break;
        }

        case IOCTL_SERIAL_SET_RTS:
        case IOCTL_SERIAL_CLR_RTS:
        case IOCTL_SERIAL_SET_XON:
        case IOCTL_SERIAL_SET_XOFF:
        case IOCTL_SERIAL_SET_CHARS:
        case IOCTL_SERIAL_GET_CHARS:
        case IOCTL_SERIAL_GET_HANDFLOW:
        case IOCTL_SERIAL_SET_HANDFLOW:
        case IOCTL_SERIAL_RESET_DEVICE: {

            break;

        }

        default:

            status=STATUS_NOT_SUPPORTED;

    }

    if (status != STATUS_PENDING) {
        //
        //  complete now if not pending
        //

        if (Irp != NULL)
        {
            RemoveReferenceAndCompleteRequest( DeviceObject, Irp, status);
        }
    }


    RemoveReferenceForDispatch(DeviceObject);

    D_TRACE(DbgPrint("FAKEMODEM: FakeModemIoControl returning %08lx\n",status);)

    return status;


}

