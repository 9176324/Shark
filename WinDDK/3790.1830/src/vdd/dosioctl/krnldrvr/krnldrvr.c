/*++

Copyright (c) 1991, 1992  Microsoft Corporation

Module Name:

    krnldrvr.c

Abstract:

    This is the NT Kernel driver component of the DOSIOCTL sample.

Environment:

    kernel mode only

Notes:

    This kernel driver's sole responsibility is to respond to a
    single IOCTL and return a fixed DWORD of information. This in
    itself would not be a significant accomplishment. However, in
    the context of the pieces of the DOSIOCTL sample, the interesting
    trick is that this driver places the information directly in a DOS
    application's buffer.

    The way this is accomplished is through a DOS driver and VDD. An
    IOCTL read is issued from the DOS application, and the pointer
    supplied in that request is passed along to this driver. (The
    IOCTL used is BUFFERED, so the I/O subsystem actually does do a
    single copy)

    The piece of information supplied here is the DWORD 0x12345678.
    This data will then be displayed by the DOS application.

    Obviously, the drivers supplied in this sample are only skeletons.
    But having the working communication mechanism in place makes for
    a good starting point for any new driver.

Revision History:

--*/

#include "ntddk.h"
#include "string.h"
#include "krnldrvr.h"


typedef struct _KDVR_DEVICE_EXTENSION {

    ULONG Information;

} KDVR_DEVICE_EXTENSION, *PKDVR_DEVICE_EXTENSION;



//
// Function declarations
//
static
NTSTATUS
KdvrDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

static
NTSTATUS
KdvrDispatchIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
KdvrUnloadDriver(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

    This is the initialization routine for the KRNLDRVR sample device driver.
    This routine creates the device object, symbolic link, and initializes
    a device extension.

Arguments:

    DriverObject - Pointer to driver object created by the system.

Return Value:

    The function value is the final status from the initialization operation.

--*/

{
    UNICODE_STRING nameString, linkString;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;
    PKDVR_DEVICE_EXTENSION extension;


    //
    // Create the device object.
    //

    RtlInitUnicodeString( &nameString, L"\\Device\\Krnldrvr" );

    status = IoCreateDevice( DriverObject,
                             sizeof(KDVR_DEVICE_EXTENSION),
                             &nameString,
                             FILE_DEVICE_UNKNOWN,
                             0,
                             TRUE,
                             &deviceObject );

    if (!NT_SUCCESS( status ))
        return status;

    //
    // Create the symbolic link so the VDD can find us.
    //

    RtlInitUnicodeString( &linkString, L"\\DosDevices\\KRNLDRVR" );

    status = IoCreateSymbolicLink (&linkString, &nameString);

    if (!NT_SUCCESS( status )) {

        IoDeleteDevice (DriverObject->DeviceObject);
        return status;
    }

    //
    // Initialize the driver object with this device driver's entry points.
    //

    DriverObject->DriverUnload = KdvrUnloadDriver;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = KdvrDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = KdvrDispatch;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = KdvrDispatchIoctl;

    extension = deviceObject->DeviceExtension;

    //
    // Initialize the device extension. This piece of information is
    // initialized to a recognizable arbitrary value so that we know that
    // the IOCTL from the DOS app actually worked.
    //

    extension->Information = 0x12345678;

    KrnldrvrDump(KDVRDIAG1, ("Krnldrvr: Initialized\n"));

    return STATUS_SUCCESS;
}

VOID
KdvrUnloadDriver(
    IN PDRIVER_OBJECT DriverObject
    )

/*++

Routine Description:

    This is the unload routine for the KRNLDRVR sample device driver.
    This routine deletes the device object and symbolic link created
    in the DriverEntry routine.

Arguments:

    DriverObject - Pointer to driver object created by the system.

Return Value:

    The function value is the final status from the initialization operation.

--*/

{
    UNICODE_STRING nameString, linkString;

    KrnldrvrDump(KDVRDIAG1, ("Krnldrvr: Unload\n"));

    IoDeleteDevice (DriverObject->DeviceObject);

    RtlInitUnicodeString( &linkString, L"\\DosDevices\\KRNLDRVR" );
    RtlInitUnicodeString( &nameString, L"\\Device\\Krnldrvr" );

    IoDeleteSymbolicLink (&linkString);

}

static
NTSTATUS
KdvrDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is the main dispatch routine for this device driver.
    No real work is done in this sample routine, but output
    generated here is useful for debugging.


Arguments:

    DeviceObject - Pointer to the device object for this driver.
    Irp - Pointer to the request packet representing the I/O request.

Return Value:

    The function value is the status of the operation.


--*/

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;

    UNREFERENCED_PARAMETER( DeviceObject );
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch (irpSp->MajorFunction) {

        case IRP_MJ_CREATE:

            KrnldrvrDump(KDVRDIAG1, ("Krnldrvr: Create\n"));
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0L;
            break;

        case IRP_MJ_CLOSE:

            KrnldrvrDump(KDVRDIAG1, ("Krnldrvr: Close\n\n"));
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information = 0L;
            break;

    }

    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, 0 );
    return status;
}

static
NTSTATUS
KdvrDispatchIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is the dispatch routine for IoDeviceControl().
    The only supported IOCTL here returns a single DWORD of information.

Arguments:

    DeviceObject - Pointer to the device object for this driver.
    Irp - Pointer to the request packet representing the I/O request.

Return Value:

    The function value is the status of the operation.


--*/

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PULONG OutputBuffer;
    PKDVR_DEVICE_EXTENSION extension = DeviceObject->DeviceExtension;


    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_KRNLDRVR_GET_INFORMATION:

            KrnldrvrDump(KDVRDIAG1, ("Krnldrvr: Processing IOCTL\n"));

            Irp->IoStatus.Status = STATUS_SUCCESS;

            OutputBuffer = (PULONG) Irp->AssociatedIrp.SystemBuffer;

            if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
                    sizeof(ULONG)) {

                Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
                break;

            }

            *OutputBuffer = extension->Information;

            Irp->IoStatus.Information = sizeof( ULONG );

            break;

    }


    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, 0 );
    return status;
}

