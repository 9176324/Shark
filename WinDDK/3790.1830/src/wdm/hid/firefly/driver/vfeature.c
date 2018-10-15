/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    vfeature.c

Abstract:

    This module sets the state of a vendor specific HID feature.

Environment:

    Kernel mode

Revision History:

    Adrian J. Oney  - Created Mar 16th, 2001

--*/

#include "firefly.h"
#include <hidpddi.h>
#include <hidclass.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FireflySetFeature)
#pragma alloc_text(PAGE, FireflyOpenStack)
#pragma alloc_text(PAGE, FireflySendIoctl)
#endif

NTSTATUS
FireflySetFeature(
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  UCHAR           PageId,
    IN  USHORT          FeatureId,
    IN  BOOLEAN         EnableFeature
    )
/*++

Routine Description:

    This routine sets the HID feature by sending HID ioctls to our device.
    These IOCTLs will be handled by HIDUSB and converted into USB requests
    and send to the device. 

Arguments:

    PhysicalDeviceObject - PDO of our HID device.

    PageID  - UsagePage of the light control feature.

    FeatureId - Usage ID of the feature.

    EnanbleFeature - True to turn the light on, Falst to turn if off.
    

Return Value:

    NT Status code

--*/
{
    NTSTATUS                    status;
    HID_COLLECTION_INFORMATION  collectionInformation;
    PHIDP_PREPARSED_DATA        preparsedData;
    PFILE_OBJECT                fileObject;
    HIDP_CAPS                   caps;
    USAGE                       usage;
    ULONG                       usageLength;
    ULONG                       numDataElements;
    HIDP_DATA                   hpData;
    PCHAR                       report;
    PDEVICE_OBJECT              topOfStack;

    //
    // Preinit for error.
    //
    preparsedData = NULL;
    fileObject = NULL;
    topOfStack = NULL;
    report = NULL;

    //
    // Get the top of the stack.
    //
    topOfStack = IoGetAttachedDeviceReference(PhysicalDeviceObject);

    //
    // First open up ourselves to get a fileobject pointer to our device.
    //
    status = FireflyOpenStack(PhysicalDeviceObject, &fileObject);

    if (!NT_SUCCESS(status)) {

        goto ExitAndFree;
    }

    //
    // Now get the collection information for this device
    //
    status = FireflySendIoctl(
        topOfStack,
        IOCTL_HID_GET_COLLECTION_INFORMATION,
        NULL,
        0,
        &collectionInformation,
        sizeof(HID_COLLECTION_INFORMATION),
        fileObject
        );

    if (!NT_SUCCESS(status)) {

        goto ExitAndFree;
    }

    preparsedData = (PHIDP_PREPARSED_DATA) ExAllocatePoolWithTag(
        NonPagedPool,
        collectionInformation.DescriptorSize,
        POOL_TAG
        );

    if (preparsedData == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitAndFree;
    }

    status = FireflySendIoctl(
        topOfStack,
        IOCTL_HID_GET_COLLECTION_DESCRIPTOR,
        NULL,
        0,
        preparsedData,
        collectionInformation.DescriptorSize,
        fileObject
        );

    if (!NT_SUCCESS(status)) {

        goto ExitAndFree;
    }

    //
    // Now get the capabilities.
    //
    RtlZeroMemory(&caps, sizeof(HIDP_CAPS));

    status = HidP_GetCaps(preparsedData, &caps);

    if (!NT_SUCCESS(status)) {

        goto ExitAndFree;
    }

    //
    // Create a report to send to the device.
    //
    report = (PCHAR) ExAllocatePoolWithTag(
        NonPagedPool,
        caps.FeatureReportByteLength,
        POOL_TAG
        );

    if (report == NULL) {

        goto ExitAndFree;
    }

    //
    // Start with a zeroed report. If we are disabling the feature, this might
    // be all we need to do.
    //
    RtlZeroMemory(report, caps.FeatureReportByteLength);
    status = STATUS_SUCCESS;

    if (EnableFeature) {

        //
        // Edit the report to reflect the enabled feature
        //
        usage = FeatureId;
        usageLength = 1;

        status = HidP_SetUsages(
            HidP_Feature,
            PageId,
            0,
            &usage, // pointer to the usage list
            &usageLength, // number of usages in the usage list
            preparsedData,
            report,
            caps.FeatureReportByteLength
            );
    }

    status = FireflySendIoctl(
        topOfStack,
        IOCTL_HID_SET_FEATURE,
        report,
        caps.FeatureReportByteLength,
        0,
        0,
        fileObject
        );

ExitAndFree:

    //
    // Close our stack and free any allocated memory
    //
    if (fileObject) {

        ObDereferenceObject(fileObject);
    }

    if (topOfStack) {

        ObDereferenceObject(topOfStack);
    }

    if (preparsedData) {

        ExFreePool(preparsedData);
    }

    if (report) {

        ExFreePool(report);
    }

    return status;
}


NTSTATUS
FireflyOpenStack(
    IN  PDEVICE_OBJECT   DeviceObject,
    OUT PFILE_OBJECT    *FileObject
    )
/*++

Routine Description:

    This routine gets an file handle (fileobject) to our HID device. We need to set this
    fileobject in the IOCTL IRP we send to the HID stack because the hidclass expects it.

Arguments:

    DeviceObject - Pointer to the device object.

    Fileobject - Address to write the fileobject pointer.

Return Value:

    NT Status code

--*/
{
    OBJECT_ATTRIBUTES objectAttributes;
    PFILE_OBJECT newFileObject;
    UNICODE_STRING pdoName;
    HANDLE fileHandle;
    ULONG bufferLength;
    PWCHAR buffer;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

    PAGED_CODE();

    //
    // Preinit for error.
    //
    *FileObject = NULL;

    //
    // Get the name of the PDO. Start with an 0 length buffer and then grow it
    // to the right size.
    //
    buffer = NULL;
    bufferLength = 0;
    status = IoGetDeviceProperty(
        DeviceObject,
        DevicePropertyPhysicalDeviceObjectName,
        bufferLength,
        buffer,
        &bufferLength
        );

    if (status != STATUS_BUFFER_TOO_SMALL) {

        //
        // We expect a zero length buffer. Anything else is fatal.
        //
        return STATUS_UNSUCCESSFUL;
    }

    buffer = (PWCHAR) ExAllocatePoolWithTag(
        NonPagedPool,
        bufferLength,
        POOL_TAG
        );

    if (NULL == buffer) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoGetDeviceProperty(
        DeviceObject,
        DevicePropertyPhysicalDeviceObjectName,
        bufferLength,
        buffer,
        &bufferLength
        );

    if (!NT_SUCCESS(status)) {

        ExFreePool(buffer);
        return status;
    }

    //
    // Now we have our name. Open ourselves.
    //
    pdoName.MaximumLength = (USHORT) bufferLength;
    pdoName.Length = (USHORT) bufferLength - sizeof(UNICODE_NULL);
    pdoName.Buffer = buffer;

    //
    // Initialize the object attributes to open the device.
    //
    InitializeObjectAttributes(
        &objectAttributes,
        &pdoName,
        0,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    //
    // Open it up, write access only!
    //
    status = ZwOpenFile(
        &fileHandle,
        FILE_WRITE_ACCESS,
        &objectAttributes,
        &ioStatus,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        FILE_NON_DIRECTORY_FILE
        );

    if (NT_SUCCESS(status)) {

        //
        // The open operation was successful. Dereference the file handle and
        // obtain a pointer to the device object for the handle.
        //
        status = ObReferenceObjectByHandle(
            fileHandle,
            0,
            *IoFileObjectType,
            KernelMode,
            (PVOID *) &newFileObject,
            NULL
            );

        ZwClose(fileHandle);
    }

    if (NT_SUCCESS(status)) {

        *FileObject = newFileObject;
    }

    ExFreePool(buffer);
    return status;
}


NTSTATUS
FireflySendIoctl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  ULONG           ControlCode,
    IN  PVOID           InputBuffer,
    IN  ULONG           InputBufferLength,
    OUT PVOID           OutputBuffer,
    IN  ULONG           OutputBufferLength,
    IN  PFILE_OBJECT    FileObject
    )
/*++

Routine Description:

    This routine sends IOCTL request and waits for it to complete.

Arguments:

    DeviceObject - Pointer to the device object.

    ControlCode - ioctl code

    InputBuffer - pointer to the input buffer

    InputBufferLength - input buffer length

    OutputBuffer - pointer to the output buffer

    OutBufferLength - output buffer length

    FileObject - pointer to the fileobject created on the target device.

Return Value:

    NT Status code

--*/
{
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK userIosb;
    NTSTATUS status;
    KEVENT event;
    PIRP irp;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(
        ControlCode,
        DeviceObject,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength,
        FALSE,
        &event,
        &userIosb
        );

    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    //
    // Associate this IRP with the passed in FileObject.
    //
    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->FileObject = FileObject;

    status = IoCallDriver(DeviceObject, irp);

    if (status == STATUS_PENDING) {

        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = userIosb.Status;
    }

    return status;
}



