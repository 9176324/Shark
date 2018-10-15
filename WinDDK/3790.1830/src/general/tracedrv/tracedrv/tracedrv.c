/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.


Module Name:

    tracedrv.c   

Abstract:

    Sample kernel mode trace provider/driver.

--*/
#include <stdio.h>
#include <ntddk.h>
#include "drvioctl.h"

#define TRACEDRV_NT_DEVICE_NAME     L"\\Device\\TraceKmp"
#define TRACEDRV_WIN32_DEVICE_NAME  L"\\DosDevices\\TRACEKMP"

//
// Software Tracing Definitions 
//
#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(CtlGuid,(d58c126f, b309, 11d1, 969e, 0000f875a5bc),  \
        WPP_DEFINE_BIT(TRACELEVELONE)                \
        WPP_DEFINE_BIT(TRACELEVELTWO) )

#include "tracedrv.tmh"      //  this is the file that will be auto generated

VOID
TraceEventLogger(
                IN PTRACEHANDLE pLoggerHandle
                );


NTSTATUS
DriverEntry(
           IN PDRIVER_OBJECT DriverObject,
           IN PUNICODE_STRING RegistryPath
           );

NTSTATUS
TracedrvDispatchOpenClose(
                    IN PDEVICE_OBJECT pDO,
                    IN PIRP Irp
                    );

NTSTATUS
TracedrvDispatchDeviceControl(
                             IN PDEVICE_OBJECT pDO,
                             IN PIRP Irp
                             );

VOID
TracedrvDriverUnload(
                    IN PDRIVER_OBJECT DriverObject
                    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text( INIT, DriverEntry )
    #pragma alloc_text( PAGE, TracedrvDispatchOpenClose )
    #pragma alloc_text( PAGE, TracedrvDispatchDeviceControl )
    #pragma alloc_text( PAGE, TracedrvDriverUnload )
#endif // ALLOC_PRAGMA


#define MAXEVENTS 100


NTSTATUS
DriverEntry(
           IN PDRIVER_OBJECT DriverObject,
           IN PUNICODE_STRING RegistryPath
           )
/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:
    DriverObject - pointer to the driver object
    RegistryPath - pointer to a unicode string representing the path
               to driver-specific key in the registry

Return Value:

   STATUS_SUCCESS if successful
   STATUS_UNSUCCESSFUL  otherwise

--*/
{
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING deviceName;
    UNICODE_STRING linkName;
    PDEVICE_OBJECT pTracedrvDeviceObject;


    KdPrint(("TraceDrv: DriverEntry\n"));

    //
    // Create Dispatch Entry Points.  
    //
    DriverObject->DriverUnload = TracedrvDriverUnload;
    DriverObject->MajorFunction[ IRP_MJ_CREATE ] = TracedrvDispatchOpenClose;
    DriverObject->MajorFunction[ IRP_MJ_CLOSE ] = TracedrvDispatchOpenClose;
    DriverObject->MajorFunction[ IRP_MJ_DEVICE_CONTROL ] = 
                                                                        TracedrvDispatchDeviceControl;    

#if defined(TARGETING_Win2K)
    //
    // You need to include this macro only on Win2K.
    //
    WPP_SYSTEMCONTROL(DriverObject);

#endif 


    RtlInitUnicodeString( &deviceName, TRACEDRV_NT_DEVICE_NAME );

    //
    // Create the Device object
    //
    status = IoCreateDevice(
                           DriverObject,
                           0,
                           &deviceName,
                           FILE_DEVICE_UNKNOWN,
                           0,
                           FALSE,
                           &pTracedrvDeviceObject);

    if ( !NT_SUCCESS( status )) {
        return status;
    }

    RtlInitUnicodeString( &linkName, TRACEDRV_WIN32_DEVICE_NAME );
    status = IoCreateSymbolicLink( &linkName, &deviceName );

    if ( !NT_SUCCESS( status )) {
        IoDeleteDevice( pTracedrvDeviceObject );
        return status;
    }


    //
    // Choose a buffering mechanism
    //
    pTracedrvDeviceObject->Flags |= DO_BUFFERED_IO;

#if defined(TARGETING_Win2K)

    //
    // This macro is required to initialize software tracing. 
    // For Win2K use the deviceobject as the first argument.
    // 
    WPP_INIT_TRACING(pTracedrvDeviceObject,RegistryPath);

#else  
    //
    // This macro is required to initialize software tracing on XP and beyond
    // For XP and beyond use the DriverObject as the first argument.
    // 
    WPP_INIT_TRACING(DriverObject,RegistryPath);

#endif

    return STATUS_SUCCESS;
}

NTSTATUS
TracedrvDispatchOpenClose(
                    IN PDEVICE_OBJECT pDO,
                    IN PIRP Irp
                    )
/*++

Routine Description:

   Dispatch routine to handle Create/Close IRPs.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}


NTSTATUS
TracedrvDispatchDeviceControl(
                             IN PDEVICE_OBJECT pDO,
                             IN PIRP Irp
                             )
/*++

Routine Description:

   Dispatch routine to handle IOCTL IRPs.

Arguments:

   DeviceObject - pointer to a device object.

   Irp - pointer to an I/O Request Packet.

Return Value:

   NT status code

--*/
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack =  IoGetCurrentIrpStackLocation( Irp );
    HANDLE ThreadHandle ;
    ULONG ControlCode =  irpStack->Parameters.DeviceIoControl.IoControlCode;
    ULONG i=0;
    static ULONG ioctlCount = 0;
    
    Irp->IoStatus.Information = 
                            irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    switch ( ControlCode ) {
    case IOCTL_TRACEKMP_TRACE_EVENT:
        //
        // Every time we get this IOCTL, we also log a trace Message if
        // Trace level one is enabled. This is used
        // to illustrate that the event can be caused by user-mode.
        //
        
        ioctlCount++;

        // 
        // Log a simple Message
        //

        DoTraceMessage(TRACELEVELONE, "IOCTL = %d", ioctlCount);

        while (i++ < MAXEVENTS) {
            //
            // Log events in a loop.
            //
            DoTraceMessage(TRACELEVELONE,  "Hello, %d %s", i, "Hi" );
            if ( !(i%100) ) {
                KdPrint(("TraceDrv:%d:Another Hundred Events Written.\n", ioctlCount));
            }
        }

        status = STATUS_SUCCESS;

        Irp->IoStatus.Information = 0;
        break;

        //
        // Not one we recognize. Error.
        //
    default:
        status = STATUS_INVALID_PARAMETER; 
        Irp->IoStatus.Information = 0;
        break;
    }

    //
    // Get rid of this request
    //
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}


VOID
TracedrvDriverUnload(
                    IN PDRIVER_OBJECT DriverObject
                    )
/*++

Routine Description:

    Free all the resources allocated in DriverEntry.

Arguments:

    DriverObject - pointer to a driver object.

Return Value:

    VOID.

--*/
{
    PDEVICE_OBJECT pDevObj;
    UNICODE_STRING linkName;

    KdPrint(("TraceDrv: Unloading \n"));

    //
    // Get pointer to Device object
    //    
    pDevObj = DriverObject->DeviceObject;

#if !defined(TARGETING_Win2K)
    // 
    // Cleanup using DriverObject on XP and beyond.
    //
    WPP_CLEANUP(DriverObject);
#else 
    // 
    // Cleanup using DeviceObject on Win2K. Make sure
    // this is same deviceobject that used for initializing.
    //
    WPP_CLEANUP(pDevObj);
#endif

    //
    // Form the Win32 symbolic link name.
    //
    RtlInitUnicodeString( &linkName, TRACEDRV_WIN32_DEVICE_NAME );

    //        
    // Remove symbolic link from Object
    // namespace...
    //
    IoDeleteSymbolicLink( &linkName );

    //
    // Unload the callbacks from the kernel to this driver
    //
    IoDeleteDevice( pDevObj );

}


