/*++

Copyright (C) Microsoft Corporation, 1996 - 1999

Module Name:

    mcd.c

Abstract:

Environment:

    Kernel mode

Revision History :

--*/
#include "mchgr.h"

#define MCD_BUFFER_MAXCOUNT             64

#define MCD_REGISTRY_CONTROL_KEY        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\MChgr"
#define MCD_REGISTRY_SERVICES_KEY       L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\MChgr"
#define MCD_PERSISTENCE_KEYVALUE        L"Persistence"
#define MCD_PERSISTENCE_PRIVATE         L"PersistencePrivateCopy"
#define MCD_DEVICE_UIDTYPE              L"UIDType"
#define MCD_DEVICE_UNIQUEID             L"UniqueId"
#define MCD_DOS_DEVICES_PREFIX_W        L"\\DosDevices"
#define MCD_DOS_DEVICES_PREFIX_A        "\\DosDevices"
#define MCD_DEVICE_PREFIX_W             L"\\Device"
#define MCD_DEVICE_PREFIX_A             "\\Device"
#define MCD_DEVICE_NAME_PREFIX_W        L"\\MediumChangerDevice%u"
#define MCD_DEVICE_NAME_PREFIX_A        "\\MediumChangerDevice%u"
#define MCD_DEVICE_NAME_FORMAT_W        L"MediumChangerDevice%u"
#define MCD_DEVICE_NAME_FORMAT_A        "MediumChangerDevice%u"
#define MCD_DEVICE_NAME_PREFIX_LEGACY_W L"\\Changer%u"
#define MCD_DEVICE_NAME_PREFIX_LEGACY_A "\\Changer%u"
#define MCD_NAME_PREFIX_W               L"\\Changer%u"
#define MCD_NAME_PREFIX_A               "\\Changer%u"
#define MCD_NAME_FORMAT_W               L"Changer%u"
#define MCD_NAME_FORMAT_A               "Changer%u"

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(INIT, DriverEntry)

    #pragma alloc_text(PAGE, ChangerUnload)
    #pragma alloc_text(PAGE, CreateChangerDeviceObject)
    #pragma alloc_text(PAGE, ChangerClassCreateClose)
    #pragma alloc_text(PAGE, ChangerClassDeviceControl)
    #pragma alloc_text(PAGE, ChangerAddDevice)
    #pragma alloc_text(PAGE, ChangerStartDevice)
    #pragma alloc_text(PAGE, ChangerInitDevice)
    #pragma alloc_text(PAGE, ChangerRemoveDevice)
    #pragma alloc_text(PAGE, ChangerStopDevice)
    #pragma alloc_text(PAGE, ChangerReadWriteVerification)
    #pragma alloc_text(PAGE, ChangerSymbolicNamePersistencePreference)
    #pragma alloc_text(PAGE, ChangerCreateSymbolicName)
    #pragma alloc_text(PAGE, ChangerCreateUniqueId)
    #pragma alloc_text(PAGE, ChangerCompareUniqueId)
    #pragma alloc_text(PAGE, ChangerGetSubKeyByIndex)
    #pragma alloc_text(PAGE, ChangerGetValuePartialInfo)
    #pragma alloc_text(PAGE, ChangerCreateNewDeviceSubKey)
    #pragma alloc_text(PAGE, ChangerCreateNonPersistentSymbolicName)
    #pragma alloc_text(PAGE, ChangerAssignSymbolicLink)
    #pragma alloc_text(PAGE, ChangerDeassignSymbolicLink)
#endif


NTSTATUS
ChangerClassCreateClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine handles CREATE/CLOSE requests.
    As these are exclusive devices, don't allow multiple opens.

Arguments:

    DeviceObject
    Irp

Return Value:

    NT Status

--*/

{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PMCD_CLASS_DATA    mcdClassData;
    PMCD_INIT_DATA     mcdInitData;
    ULONG              miniclassExtSize;
    NTSTATUS           status = STATUS_SUCCESS;

    PAGED_CODE();

    mcdClassData = (PMCD_CLASS_DATA)(fdoExtension->CommonExtension.DriverData);

    mcdInitData = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);

        return STATUS_NO_SUCH_DEVICE;
    }

    miniclassExtSize = mcdInitData->ChangerAdditionalExtensionSize();

    //
    // The class library's private data is after the miniclass's.
    //

    (ULONG_PTR)mcdClassData += miniclassExtSize;

    if (irpStack->MajorFunction == IRP_MJ_CLOSE) {
        DebugPrint((3,
                    "ChangerClassCreateClose - IRP_MJ_CLOSE\n"));

        //
        // Indicate that the device is available for others.
        //

        mcdClassData->DeviceOpen = 0;
        status = STATUS_SUCCESS;

    } else if (irpStack->MajorFunction == IRP_MJ_CREATE) {

        DebugPrint((3,
                    "ChangerClassCreateClose - IRP_MJ_CREATE\n"));

        //
        // If already opened, return busy.
        //

        if (mcdClassData->DeviceOpen) {

            DebugPrint((1,
                        "ChangerClassCreateClose - returning DEVICE_BUSY. DeviceOpen - %x\n",
                        mcdClassData->DeviceOpen));

            status = STATUS_DEVICE_BUSY;
        } else {

            //
            // Indicate that the device is busy.
            //

            InterlockedIncrement(&mcdClassData->DeviceOpen);
            status = STATUS_SUCCESS;
        }


    }

    Irp->IoStatus.Status = status;
    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);

    return status;

} // end ChangerCreate()


NTSTATUS
ChangerClassDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{
    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    PFUNCTIONAL_DEVICE_EXTENSION    fdoExtension = DeviceObject->DeviceExtension;
    PMCD_INIT_DATA    mcdInitData;
    NTSTATUS               status;
    ULONG ioControlCode;

    PAGED_CODE();


    mcdInitData = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);

        return STATUS_NO_SUCH_DEVICE;
    }

    //
    // Disable media change detection before processing current IOCTL
    //
    ClassDisableMediaChangeDetection(fdoExtension);

    ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;
    switch (ioControlCode) {

    case IOCTL_CHANGER_GET_PARAMETERS:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_GET_PARAMETERS\n"));

        //
        // Validate buffer length.
        //

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(GET_CHANGER_PARAMETERS)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
        } else {
            PGET_CHANGER_PARAMETERS changerParameters = Irp->AssociatedIrp.SystemBuffer;
            RtlZeroMemory(changerParameters, sizeof(GET_CHANGER_PARAMETERS));

            status = mcdInitData->ChangerGetParameters(DeviceObject, Irp);
        }

        break;

    case IOCTL_CHANGER_GET_STATUS:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_GET_STATUS\n"));

        status = mcdInitData->ChangerGetStatus(DeviceObject, Irp);

        break;

    case IOCTL_CHANGER_GET_PRODUCT_DATA:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_GET_PRODUCT_DATA\n"));

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(CHANGER_PRODUCT_DATA)) {

            status = STATUS_INFO_LENGTH_MISMATCH;

        } else {
            PCHANGER_PRODUCT_DATA changerProductData = Irp->AssociatedIrp.SystemBuffer;
            RtlZeroMemory(changerProductData, sizeof(CHANGER_PRODUCT_DATA));

            status = mcdInitData->ChangerGetProductData(DeviceObject, Irp);
        }

        break;

    case IOCTL_CHANGER_SET_ACCESS:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_SET_ACCESS\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CHANGER_SET_ACCESS)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
        } else {

            status = mcdInitData->ChangerSetAccess(DeviceObject, Irp);
        }

        break;

    case IOCTL_CHANGER_GET_ELEMENT_STATUS:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_GET_ELEMENT_STATUS\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CHANGER_READ_ELEMENT_STATUS)) {

            status = STATUS_INFO_LENGTH_MISMATCH;

        } else {

            PCHANGER_READ_ELEMENT_STATUS readElementStatus = Irp->AssociatedIrp.SystemBuffer;
            ULONG length = readElementStatus->ElementList.NumberOfElements * sizeof(CHANGER_ELEMENT_STATUS);
            ULONG lengthEx = readElementStatus->ElementList.NumberOfElements * sizeof(CHANGER_ELEMENT_STATUS_EX);
            ULONG outputBuffLen = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

            //
            // Further validate parameters.
            //
            status = STATUS_SUCCESS;
            if ((outputBuffLen < lengthEx) && (outputBuffLen < length)) {

                status = STATUS_BUFFER_TOO_SMALL;

            } else if ((length == 0) || (lengthEx == 0)) {

                status = STATUS_INVALID_PARAMETER;
            }

            if (NT_SUCCESS(status)) {

                status = mcdInitData->ChangerGetElementStatus(DeviceObject, Irp);
            }

        }

        break;

    case IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CHANGER_INITIALIZE_ELEMENT_STATUS)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
        } else {

            status = mcdInitData->ChangerInitializeElementStatus(DeviceObject, Irp);
        }

        break;

    case IOCTL_CHANGER_SET_POSITION:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_SET_POSITION\n"));


        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CHANGER_SET_POSITION)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
        } else {

            status = mcdInitData->ChangerSetPosition(DeviceObject, Irp);
        }

        break;

    case IOCTL_CHANGER_EXCHANGE_MEDIUM:

        DebugPrint((3, "Mcd.ChangerDeviceControl: IOCTL_CHANGER_EXCHANGE_MEDIUM\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CHANGER_EXCHANGE_MEDIUM)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
        } else {
            status = mcdInitData->ChangerExchangeMedium(DeviceObject, Irp);
        }

        break;

    case IOCTL_CHANGER_MOVE_MEDIUM:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_MOVE_MEDIUM\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CHANGER_MOVE_MEDIUM)) {

            status = STATUS_INFO_LENGTH_MISMATCH;

        } else {

            status = mcdInitData->ChangerMoveMedium(DeviceObject, Irp);
        }

        break;

    case IOCTL_CHANGER_REINITIALIZE_TRANSPORT:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_REINITIALIZE_TRANSPORT\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CHANGER_ELEMENT)) {

            status = STATUS_INFO_LENGTH_MISMATCH;

        } else {

            status = mcdInitData->ChangerReinitializeUnit(DeviceObject, Irp);
        }

        break;

    case IOCTL_CHANGER_QUERY_VOLUME_TAGS:

        DebugPrint((3,
                    "Mcd.ChangerDeviceControl: IOCTL_CHANGER_QUERY_VOLUME_TAGS\n"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(CHANGER_SEND_VOLUME_TAG_INFORMATION)) {

            status = STATUS_INFO_LENGTH_MISMATCH;

        } else if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                   sizeof(READ_ELEMENT_ADDRESS_INFO)) {

            status = STATUS_INFO_LENGTH_MISMATCH;

        } else {

            status = mcdInitData->ChangerQueryVolumeTags(DeviceObject, Irp);
        }

        break;

    default:
        DebugPrint((1,
                    "Mcd.ChangerDeviceControl: Unhandled IOCTL\n"));


        //
        // Pass the request to the common device control routine.
        //

        status = ClassDeviceControl(DeviceObject, Irp);

        //
        // Re-enable media change detection
        //
        ClassEnableMediaChangeDetection(fdoExtension);

        return status;
    }

    Irp->IoStatus.Status = status;

    //
    // Re-enable media change detection
    //
    ClassEnableMediaChangeDetection(fdoExtension);

    if (!NT_SUCCESS(status) && IoIsErrorUserInduced(status)) {

        DebugPrint((1,
                    "Mcd.ChangerDeviceControl: IOCTL %x, status %x\n",
                    irpStack->Parameters.DeviceIoControl.IoControlCode,
                    status));

        IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);
    }

    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject,Irp, IO_NO_INCREMENT);
    return status;
}


VOID
ChangerClassError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )

/*++

Routine Description:


Arguments:

    DeviceObject
    Irp

Return Value:

    Final Nt status indicating the results of the operation.

Notes:


--*/

{

    PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
    PFUNCTIONAL_DEVICE_EXTENSION    fdoExtension = DeviceObject->DeviceExtension;
    PMCD_INIT_DATA mcdInitData;

    mcdInitData = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        return;
    }

    if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) {
        switch (senseBuffer->SenseKey & 0xf) {

        case SCSI_SENSE_MEDIUM_ERROR: {
                *Retry = FALSE;

                if (((senseBuffer->AdditionalSenseCode) ==  SCSI_ADSENSE_INVALID_MEDIA) &&
                    ((senseBuffer->AdditionalSenseCodeQualifier) == SCSI_SENSEQ_CLEANING_CARTRIDGE_INSTALLED)) {

                    //
                    // This indicates a cleaner cartridge is present in the changer
                    //
                    *Status = STATUS_CLEANER_CARTRIDGE_INSTALLED;
                }
                break;
            }

        case SCSI_SENSE_ILLEGAL_REQUEST: {
                switch (senseBuffer->AdditionalSenseCode) {
                case SCSI_ADSENSE_ILLEGAL_BLOCK:
                    if (senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_ILLEGAL_ELEMENT_ADDR ) {

                        DebugPrint((1,
                                    "MediumChanger: An operation was attempted on an invalid element\n"));

                        //
                        // Attemped operation to an invalid element.
                        //

                        *Retry = FALSE;
                        *Status = STATUS_ILLEGAL_ELEMENT_ADDRESS;
                    }
                    break;

                case SCSI_ADSENSE_POSITION_ERROR:

                    if (senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_SOURCE_EMPTY) {

                        DebugPrint((1,
                                    "MediumChanger: The specified source element has no media\n"));

                        //
                        // The indicated source address has no media.
                        //

                        *Retry = FALSE;
                        *Status = STATUS_SOURCE_ELEMENT_EMPTY;

                    } else if (senseBuffer->AdditionalSenseCodeQualifier == SCSI_SENSEQ_DESTINATION_FULL) {

                        DebugPrint((1,
                                    "MediumChanger: The specified destination element already has media.\n"));
                        //
                        // The indicated destination already contains media.
                        //

                        *Retry = FALSE;
                        *Status = STATUS_DESTINATION_ELEMENT_FULL;
                    }
                    break;



                default:
                    break;
                } // switch (senseBuffer->AdditionalSenseCode)

                break;
            }

        case SCSI_SENSE_UNIT_ATTENTION: {

                if ((senseBuffer->AdditionalSenseCode == SCSI_ADSENSE_MEDIUM_CHANGED) ||
                    (senseBuffer->AdditionalSenseCode == 0x3f && senseBuffer->AdditionalSenseCodeQualifier == 0x81)) {

                    //
                    // Changers like the Compaq StorageWorks LIB-81 return
                    // 0x06/3f/81 when media is injected/ejected into the
                    // changer instead of 0x06/28/00
                    //

                    //
                    // Need to notify applications of possible media change in
                    // the library. First, set the current media state to
                    // NotPresent and then set the state to present. We need to
                    // do this because, changer devices do not report MediaNotPresent
                    // state. They only convey MediumChanged condition. In order for
                    // classpnp to notify applications of media change, we need to
                    // simulate media notpresent to present state transition
                    //
                    ClassSetMediaChangeState(fdoExtension, MediaNotPresent, FALSE);
                    ClassSetMediaChangeState(fdoExtension, MediaPresent, FALSE);
                }

                break;
            }

        default:
            break;

        } // end switch (senseBuffer->SenseKey & 0xf)
    } // if (Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID)

    //
    // Call Changer MiniDriver error routine only if we
    // are running at or below APC_LEVEL
    //
    if (KeGetCurrentIrql() > APC_LEVEL) {
        return;
    }

    if (mcdInitData->ChangerError) {
        //
        // Allow device-specific module to update this.
        //
        mcdInitData->ChangerError(DeviceObject, Srb, Status, Retry);
    }

    return;
}



NTSTATUS
ChangerAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

/*++

Routine Description:

    This routine creates and initializes a new FDO for the corresponding
    PDO.  It may perform property queries on the FDO but cannot do any
    media access operations.

Arguments:

    DriverObject - MC class driver object.

    Pdo - the physical device object we are being added to

Return Value:

    status

--*/

{
    PULONG devicesFound = NULL;
    NTSTATUS status;
    BOOLEAN persistence = FALSE;
    BOOLEAN legacyNameFormat = !persistence;

    PAGED_CODE();

    //
    // Get the address of the count of the number of changer devices already initialized.
    //

    devicesFound = &IoGetConfigurationInformation()->MediumChangerCount;

    //
    // If this is the first device, we will read the Persistence value off
    // of the Control key (user's choice) and update the private copy of
    // under Services key which will be value that will actually be used.
    // This way, if a user makes a change to the value of Persistence after
    // some devices have showed up, we won't get into the situation where
    // the device before the change use the old value of Persistence and
    // the new devices that show up after the change use the new value.
    //
    // Using the private copy, all devices will use the same value and the
    // change made to Persistence (for the key under Control branch) will
    // take effect only on next reboot.
    //
    if (*devicesFound == 0) {

        //
        // Query User's preference for persistence
        //
        if (!NT_SUCCESS(ChangerSymbolicNamePersistencePreference(DriverObject,
                                                                 MCD_REGISTRY_CONTROL_KEY,
                                                                 MCD_PERSISTENCE_QUERY,
                                                                 MCD_PERSISTENCE_KEYVALUE,
                                                                 &persistence))) {

            ASSERT(FALSE);
        }

        //
        // Set persistence for this entire up-time until next reboot.
        // If we failed to read the user's preference, assume non-persistent
        // symbolic name.
        //
        if (!NT_SUCCESS(ChangerSymbolicNamePersistencePreference(DriverObject,
                                                                 MCD_REGISTRY_SERVICES_KEY,
                                                                 MCD_PERSISTENCE_SET,
                                                                 MCD_PERSISTENCE_PRIVATE,
                                                                 &persistence))) {

            ASSERT(FALSE);
        }
    } else {

        //
        // Since this isn't the first device, just read the value of Persistence
        // from the Services branch.
        //
        if (!NT_SUCCESS(ChangerSymbolicNamePersistencePreference(DriverObject,
                                                                 MCD_REGISTRY_SERVICES_KEY,
                                                                 MCD_PERSISTENCE_QUERY,
                                                                 MCD_PERSISTENCE_PRIVATE,
                                                                 &persistence))) {

            ASSERT(FALSE);
        }
    }

    legacyNameFormat = !persistence;

    status = CreateChangerDeviceObject(DriverObject,
                                       PhysicalDeviceObject,
                                       legacyNameFormat);

    if (NT_SUCCESS(status)) {

        (*devicesFound)++;
    }

    return status;
}



NTSTATUS
ChangerStartDevice(
    IN PDEVICE_OBJECT Fdo
    )

/*++

Routine Description:

    This routine is called after InitDevice, and creates the symbolic link,
    and sets up information in the registry.
    The routine could be called multiple times, in the event of a StopDevice.


Arguments:

    Fdo - a pointer to the functional device object for this device

Return Value:

    status

--*/

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PINQUIRYDATA            inquiryData = NULL;
    ULONG                   pageLength;
    ULONG                   inquiryLength;
    SCSI_REQUEST_BLOCK      srb;
    PCDB                    cdb;
    NTSTATUS                status;
    PMCD_CLASS_DATA         mcdClassData = (PMCD_CLASS_DATA)fdoExtension->CommonExtension.DriverData;
    PMCD_INIT_DATA          mcdInitData;
    ULONG                   miniClassExtSize;

    PAGED_CODE();

    mcdInitData = IoGetDriverObjectExtension(Fdo->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        return STATUS_NO_SUCH_DEVICE;
    }

    miniClassExtSize = mcdInitData->ChangerAdditionalExtensionSize();
    (ULONG_PTR)mcdClassData += miniClassExtSize;

    //
    // Build and send request to get inquiry data.
    //

    inquiryData = ExAllocatePool(NonPagedPoolCacheAligned, MAXIMUM_CHANGER_INQUIRY_DATA);
    if (!inquiryData) {
        //
        // The buffer cannot be allocated.
        //

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(&srb, SCSI_REQUEST_BLOCK_SIZE);

    //
    // Set timeout value.
    //

    srb.TimeOutValue = 2;

    srb.CdbLength = 6;

    cdb = (PCDB)srb.Cdb;

    //
    // Set CDB operation code.
    //

    cdb->CDB6INQUIRY.OperationCode = SCSIOP_INQUIRY;

    //
    // Set allocation length to inquiry data buffer size.
    //

    cdb->CDB6INQUIRY.AllocationLength = MAXIMUM_CHANGER_INQUIRY_DATA;

    status = ClassSendSrbSynchronous(Fdo,
                                     &srb,
                                     inquiryData,
                                     MAXIMUM_CHANGER_INQUIRY_DATA,
                                     FALSE);


    if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_SUCCESS ||
        SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) {

        srb.SrbStatus = SRB_STATUS_SUCCESS;
    }

    if (srb.SrbStatus == SRB_STATUS_SUCCESS) {
        inquiryLength = inquiryData->AdditionalLength + FIELD_OFFSET(INQUIRYDATA, Reserved);

        if (inquiryLength > srb.DataTransferLength) {
            inquiryLength = srb.DataTransferLength;
        }
    } else {

        //
        // The class function will only write inquiryLength of inquiryData
        // to the reg. key.
        //

        inquiryLength = 0;
    }

    ChangerSymbolicNamePersistencePreference(Fdo->DriverObject,
                                             MCD_REGISTRY_SERVICES_KEY,
                                             MCD_PERSISTENCE_QUERY,
                                             MCD_PERSISTENCE_PRIVATE,
                                             &mcdClassData->PersistencePreferred);
    //
    // Don't care about success. If this routine fails for any reason, we will
    // assume old behavior (non persistent symbolic name) is preferred.
    //

    if (!mcdClassData->DosNameCreated) {

        status = ChangerCreateSymbolicName(Fdo, mcdClassData->PersistencePreferred);

        //
        // If we couldn't create a symbolic name, too bad!
        // Just continue. A user mode application won't be able to open a handle
        // to the device using the symbolic name however.
        // But we'll update the Device Number that will be returned in response
        // to IOCTL_STORAGE_GET_DEVICE_NUMBER, to something more meaningful than
        // the bogus value we initialized it to.
        //
        if (!NT_SUCCESS(status)) {

            ASSERT(NT_SUCCESS(status));

            fdoExtension->DeviceNumber = mcdClassData->PnpNameDeviceNumber;
        }
    }

    //
    // Add changer device info to registry
    //

    ClassUpdateInformationInRegistry(Fdo,
                                     "Changer",
                                     fdoExtension->DeviceNumber,
                                     inquiryData,
                                     inquiryLength);

    ExFreePool(inquiryData);

    if ((((MCD_CLASS_DATA UNALIGNED *)mcdClassData)->MediumChangerInterfaceString).Buffer != NULL) {

        status = IoSetDeviceInterfaceState(&(mcdClassData->MediumChangerInterfaceString),
                                           TRUE);

        if (!NT_SUCCESS(status)) {

            DebugPrint((1,
                        "ChangerStartDevice: Unable to register MediumChanger%x (NT Name) interface name - %x.\n",
                        mcdClassData->PnpNameDeviceNumber,
                        status));
        }

    }

    return STATUS_SUCCESS;
}


NTSTATUS
ChangerStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )
{
    //
    // Since the symlinks are created in StartDevice and the device interface is
    // also enabled there, this would be the place to delete the symlinks and
    // to disable the device interface. However, classpnp will not call our
    // StartDevice even when the device receives a PnP StartDevice IRP, because
    // the device was already initialized (after the first Start).
    // So we shall delete symlinks and disable device interface only in
    // RemoveDevice.
    //

    PAGED_CODE();
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Type);

    return STATUS_SUCCESS;
}




#define CHANGER_SRB_LIST_SIZE 2



NTSTATUS
ChangerInitDevice(
    IN PDEVICE_OBJECT Fdo
    )

/*++

Routine Description:

    This routine will complete the changer initialization.  This includes
    allocating sense info buffers and srb s-lists. Additionally, the miniclass
    driver's init entry points are called.

    This routine will not clean up allocate resources if it fails - that
    is left for device stop/removal

Arguments:

    Fdo - a pointer to the functional device object for this device

Return Value:

    NTSTATUS

--*/

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PVOID                   senseData = NULL;
    NTSTATUS                status;
    PMCD_INIT_DATA          mcdInitData;
    STORAGE_PROPERTY_ID     propertyId;
    PMCD_CLASS_DATA         mcdClassData;

    PAGED_CODE();

    mcdInitData = IoGetDriverObjectExtension(Fdo->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        return STATUS_NO_SUCH_DEVICE;
    }

    //
    // Allocate request sense buffer.
    //

    senseData = ExAllocatePool(NonPagedPoolCacheAligned,
                               SENSE_BUFFER_SIZE);

    if (senseData == NULL) {

        //
        // The buffer cannot be allocated.
        //

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ChangerInitDeviceExit;
    }

    //
    // Build the lookaside list for srb's for the device. Should only
    // need a couple.
    //

    ClassInitializeSrbLookasideList(&(fdoExtension->CommonExtension), CHANGER_SRB_LIST_SIZE);

    //
    // Set the sense data pointer in the device extension.
    //

    fdoExtension->SenseData = senseData;

    fdoExtension->TimeOutValue = 600;

    //
    // Call port driver to get adapter capabilities.
    //

    propertyId = StorageAdapterProperty;

    status = ClassGetDescriptor(fdoExtension->CommonExtension.LowerDeviceObject,
                                &propertyId,
                                &(fdoExtension->AdapterDescriptor));

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,
                    "ChangerInitDevice: Unable to get adapter descriptor. Status %x\n",
                    status));
        goto ChangerInitDeviceExit;
    }

    //
    // Invoke the device-specific initialization function.
    //

    status = mcdInitData->ChangerInitialize(Fdo);
    if (!NT_SUCCESS(status)) {
        DebugPrint((1,
                    "ChangerInitDevice: Minidriver Init failed. Status %x\n",
                    status));
        goto ChangerInitDeviceExit;
    }

    //
    // Register for media change notification
    //
    ClassInitializeMediaChangeDetection(fdoExtension, "Changer");

    //
    // Register interfaces for this device.
    //

    mcdClassData = (PMCD_CLASS_DATA)(fdoExtension->CommonExtension.DriverData);

    //
    // The class library's private data is after the miniclass's.
    //

    (ULONG_PTR)mcdClassData += mcdInitData->ChangerAdditionalExtensionSize();

    RtlInitUnicodeString(&(mcdClassData->MediumChangerInterfaceString), NULL);

    //
    // Register the device interface. However, enable it only after creating a
    // symbolic link.
    //
    status = IoRegisterDeviceInterface(fdoExtension->LowerPdo,
                                       (LPGUID) &MediumChangerClassGuid,
                                       NULL,
                                       &(mcdClassData->MediumChangerInterfaceString));

    return status;

    //
    // Fall through and return whatever status the miniclass driver returned.
    //

    ChangerInitDeviceExit:

    if (senseData) {
        ExFreePool(senseData);
        fdoExtension->SenseData = NULL;
    }

    return status;

} // End ChangerInitDevice



NTSTATUS
ChangerRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )

/*++

Routine Description:

    This routine is responsible for releasing any resources in use by the
    changer driver.

Arguments:

    DeviceObject - the device object being removed

Return Value:

    none - this routine may not fail

--*/

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PMCD_CLASS_DATA              mcdClassData = (PMCD_CLASS_DATA)fdoExtension->CommonExtension.DriverData;
    PMCD_INIT_DATA               mcdInitData;
    ULONG                        miniClassExtSize;
    NTSTATUS                     status;

    PAGED_CODE();

    if ((Type == IRP_MN_QUERY_REMOVE_DEVICE) ||
        (Type == IRP_MN_CANCEL_REMOVE_DEVICE)) {
        return STATUS_SUCCESS;
    }


    mcdInitData = IoGetDriverObjectExtension(DeviceObject->DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        return STATUS_NO_SUCH_DEVICE;
    }

    miniClassExtSize = mcdInitData->ChangerAdditionalExtensionSize();

    //
    // Free all allocated memory.
    //

    if (Type == IRP_MN_REMOVE_DEVICE) {
        if (fdoExtension->DeviceDescriptor) {
            ExFreePool(fdoExtension->DeviceDescriptor);
            fdoExtension->DeviceDescriptor = NULL;
        }
        if (fdoExtension->AdapterDescriptor) {
            ExFreePool(fdoExtension->AdapterDescriptor);
            fdoExtension->AdapterDescriptor = NULL;
        }
        if (fdoExtension->SenseData) {
            ExFreePool(fdoExtension->SenseData);
            fdoExtension->SenseData = NULL;
        }
        ClassDeleteSrbLookasideList(&fdoExtension->CommonExtension);
    }

    (ULONG_PTR)mcdClassData += miniClassExtSize;

    //
    // Disable the interface before deleting the symbolic link.
    //
    if ((((MCD_CLASS_DATA UNALIGNED *)mcdClassData)->MediumChangerInterfaceString).Buffer != NULL) {
        IoSetDeviceInterfaceState(&(mcdClassData->MediumChangerInterfaceString),
                                  FALSE);

        RtlFreeUnicodeString(&(mcdClassData->MediumChangerInterfaceString));

        //
        // Clear it.
        //

        RtlInitUnicodeString(&(mcdClassData->MediumChangerInterfaceString), NULL);
    }

    //
    // If a PnP StopDevice already happened, the symbolic link has already been
    // deleted.
    //
    if (mcdClassData->DosNameCreated) {

        //
        // Delete the symbolic link \DosDevices\ChangerN.
        //
        ChangerDeassignSymbolicLink(MCD_DOS_DEVICES_PREFIX_W,
                                    mcdClassData->SymbolicNameDeviceNumber,
                                    FALSE);

        mcdClassData->DosNameCreated = FALSE;
    }

    if (mcdClassData->PnpNameLinkCreated) {

        //
        // Delete the symbolic link \Device\ChangerN or \Device\MediumChangerDeviceN depending
        // on persistence preference. (Remember that legacy name format and
        // persistence preference are mutually exclusive).
        //
        ChangerDeassignSymbolicLink(MCD_DEVICE_PREFIX_W,
                                    mcdClassData->SymbolicNameDeviceNumber,
                                    !mcdClassData->PersistencePreferred);

        mcdClassData->PnpNameLinkCreated = FALSE;
    }

    //
    // Remove registry bits.
    //

    if (Type == IRP_MN_REMOVE_DEVICE) {
        IoGetConfigurationInformation()->MediumChangerCount--;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
ChangerReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

/*++

Routine Description:

    This routine is a stub that returns invalid device request.

Arguments:

    DeviceObject - Supplies the device object.

    Irp - Supplies the I/O request packet.

Return Value:

    STATUS_INVALID_DEVICE_REQUEST


--*/

{
    return  STATUS_INVALID_DEVICE_REQUEST;
}


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

/*++

Routine Description:

    This is the entry point for this EXPORT DRIVER.  It does nothing.

--*/

{
    return STATUS_SUCCESS;
}


NTSTATUS
ChangerClassInitialize(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath,
    IN  PMCD_INIT_DATA  MCDInitData
    )

/*++

Routine Description:

    This routine is called by a changer mini-class driver during its
    DriverEntry routine to initialize the driver.

Arguments:

    DriverObject    - Supplies the driver object.

    RegistryPath    - Supplies the registry path for this driver.

    MCDInitData     - Changer Minidriver Init Data

Return Value:

    Status value returned by ClassInitialize
--*/

{
    PMCD_INIT_DATA driverExtension;
    CLASS_INIT_DATA InitializationData;
    NTSTATUS status;

    //
    // Get the driver extension
    //
    status = IoAllocateDriverObjectExtension(
                                            DriverObject,
                                            ChangerClassInitialize,
                                            sizeof(MCD_INIT_DATA),
                                            &driverExtension);
    if (!NT_SUCCESS(status)) {
        if (status == STATUS_OBJECT_NAME_COLLISION) {

            //
            // An extension already exists for this key.  Get a pointer to it
            //

            driverExtension = IoGetDriverObjectExtension(DriverObject,
                                                         ChangerClassInitialize);
            if (driverExtension == NULL) {
                DebugPrint((1,
                            "ChangerClassInitialize : driverExtension NULL\n"));
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {

            //
            // As this failed, the changer init data won't be able to be stored.
            //

            DebugPrint((1,
                        "ChangerClassInitialize: Error %x allocating driver extension\n",
                        status));

            return status;
        }
    }

    RtlCopyMemory(driverExtension, MCDInitData, sizeof(MCD_INIT_DATA));

    //
    // Zero InitData
    //

    RtlZeroMemory (&InitializationData, sizeof(CLASS_INIT_DATA));

    //
    // Set sizes
    //

    InitializationData.InitializationDataSize = sizeof(CLASS_INIT_DATA);

    InitializationData.FdoData.DeviceExtensionSize =
    sizeof(FUNCTIONAL_DEVICE_EXTENSION) +
    MCDInitData->ChangerAdditionalExtensionSize() +
    sizeof(MCD_CLASS_DATA);

    InitializationData.FdoData.DeviceType = FILE_DEVICE_CHANGER;
    InitializationData.FdoData.DeviceCharacteristics = FILE_DEVICE_SECURE_OPEN;

    //
    // Set entry points
    //

    InitializationData.FdoData.ClassStartDevice = ChangerStartDevice;
    InitializationData.FdoData.ClassInitDevice = ChangerInitDevice;
    InitializationData.FdoData.ClassStopDevice = ChangerStopDevice;
    InitializationData.FdoData.ClassRemoveDevice = ChangerRemoveDevice;
    InitializationData.ClassAddDevice = ChangerAddDevice;

    InitializationData.FdoData.ClassReadWriteVerification = NULL;
    InitializationData.FdoData.ClassDeviceControl = ChangerClassDeviceControl;
    InitializationData.FdoData.ClassError = ChangerClassError;
    InitializationData.FdoData.ClassShutdownFlush = NULL;

    InitializationData.FdoData.ClassCreateClose = ChangerClassCreateClose;

    //
    // Stub routine to make the class driver happy.
    //

    InitializationData.FdoData.ClassReadWriteVerification = ChangerReadWriteVerification;

    InitializationData.ClassUnload = ChangerUnload;


    //
    // Routines for WMI support
    //
    InitializationData.FdoData.ClassWmiInfo.GuidCount = 3;
    InitializationData.FdoData.ClassWmiInfo.GuidRegInfo = ChangerWmiFdoGuidList;
    InitializationData.FdoData.ClassWmiInfo.ClassQueryWmiRegInfo = ChangerFdoQueryWmiRegInfo;
    InitializationData.FdoData.ClassWmiInfo.ClassQueryWmiDataBlock = ChangerFdoQueryWmiDataBlock;
    InitializationData.FdoData.ClassWmiInfo.ClassSetWmiDataBlock = ChangerFdoSetWmiDataBlock;
    InitializationData.FdoData.ClassWmiInfo.ClassSetWmiDataItem = ChangerFdoSetWmiDataItem;
    InitializationData.FdoData.ClassWmiInfo.ClassExecuteWmiMethod = ChangerFdoExecuteWmiMethod;
    InitializationData.FdoData.ClassWmiInfo.ClassWmiFunctionControl = ChangerWmiFunctionControl;

    //
    // Call the class init routine
    //

    status = ClassInitialize( DriverObject, RegistryPath, &InitializationData);

    if (!NT_SUCCESS(status)) {
        ChangerUnload(DriverObject);
    }

    return status;
}


VOID
ChangerUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);
    return;
}

NTSTATUS
CreateChangerDeviceObject(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  PhysicalDeviceObject,
    IN  BOOLEAN         LegacyNameFormat
    )

/*++

Routine Description:

    This routine creates an object for the device.
    If legacy name format is going to be used, then we will create \Device\ChangerN for an N that
       doesn't cause an object name clash and symbolic names \DosDevices\ChangerN and \Device\MediumChangerDeviceN
       will be later created. This implicitly means that the symbolic names will not persist.
    However, if legacy name format is not reguired, we will create \Device\MediumChangerDeviceN and later create
       symbolic names \DosDevices\ChangerM and \Device\ChangerM (where M may or may not be the same as N).

Arguments:

    DriverObject - Pointer to driver object created by system.
    PhysicalDeviceObject - DeviceObject of the attached to device.
    LegacyNameFormat - decides device name format as old (legacy) format \Device\ChangerN or new format \Device\MediumChangerDeviceN

Return Value:

    NTSTATUS

--*/

{

    PDEVICE_OBJECT lowerDevice;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = NULL;
    CCHAR          deviceNameBuffer[MCD_BUFFER_MAXCOUNT] = {0};
    NTSTATUS       status;
    PDEVICE_OBJECT deviceObject;
    ULONG          requiredStackSize;
    PVOID          senseData;
    PMCD_CLASS_DATA mcdClassData;
    PMCD_INIT_DATA mcdInitData;
    ULONG          miniclassExtSize;
    ULONG          mcdCount;

    PAGED_CODE();

    DebugPrint((3,"CreateChangerDeviceObject: Enter routine\n"));

    lowerDevice = IoGetAttachedDeviceReference(PhysicalDeviceObject);
    //
    // Claim the device. Note that any errors after this
    // will goto the generic handler, where the device will
    // be released.
    //

    status = ClassClaimDevice(lowerDevice, FALSE);

    if (!NT_SUCCESS(status)) {

        //
        // Someone already had this device.
        //

        ObDereferenceObject(lowerDevice);
        return status;
    }

    //
    // Create device object for this device.
    //
    mcdCount = 0;
    do {
        strcpy(deviceNameBuffer, MCD_DEVICE_PREFIX_A);
        sprintf(deviceNameBuffer + strlen(deviceNameBuffer),
                (LegacyNameFormat ? MCD_DEVICE_NAME_PREFIX_LEGACY_A : MCD_DEVICE_NAME_PREFIX_A),
                mcdCount);

        status = ClassCreateDeviceObject(DriverObject,
                                         deviceNameBuffer,
                                         PhysicalDeviceObject,
                                         TRUE,
                                         &deviceObject);
        mcdCount++;
    } while (status == STATUS_OBJECT_NAME_COLLISION && mcdCount < MAXLONG);

    if (!NT_SUCCESS(status)) {
        DebugPrint((1,"CreateChangerDeviceObjects: Can not create device %s\n",
                    deviceNameBuffer));

        goto CreateChangerDeviceObjectExit;
    }

    //
    // Indicate that IRPs should include MDLs.
    //

    deviceObject->Flags |= DO_DIRECT_IO;

    fdoExtension = deviceObject->DeviceExtension;

    //
    // Get the saved MCD Init Data
    //
    mcdInitData = IoGetDriverObjectExtension(DriverObject,
                                             ChangerClassInitialize);

    if (mcdInitData == NULL) {

        status = STATUS_NO_SUCH_DEVICE;
        goto CreateChangerDeviceObjectExit;
    }

    ASSERT(mcdInitData);

    mcdClassData = (PMCD_CLASS_DATA)(fdoExtension->CommonExtension.DriverData);
    miniclassExtSize = mcdInitData->ChangerAdditionalExtensionSize();
    (ULONG_PTR)mcdClassData += miniclassExtSize;

    //
    // Back pointer to device object.
    //

    fdoExtension->CommonExtension.DeviceObject = deviceObject;

    //
    // This is the physical device.
    //

    fdoExtension->CommonExtension.PartitionZeroExtension = fdoExtension;

    //
    // Initialize lock count to zero. The lock count is used to
    // disable the ejection mechanism when media is mounted.
    //

    fdoExtension->LockCount = 0;

    // Put in a bogus value for now. It will be updated on StartDevice once we
    // create a symbolic link successfully.
    //
    fdoExtension->DeviceNumber = MAXLONG;

    //
    // Set the alignment requirements for the device based on the
    // host adapter requirements
    //

    if (lowerDevice->AlignmentRequirement > deviceObject->AlignmentRequirement) {
        deviceObject->AlignmentRequirement = lowerDevice->AlignmentRequirement;
    }

    //
    // Save the device descriptors
    //

    fdoExtension->AdapterDescriptor = NULL;

    fdoExtension->DeviceDescriptor = NULL;

    //
    // Clear the SrbFlags and disable synchronous transfers
    //

    fdoExtension->SrbFlags = SRB_FLAGS_DISABLE_SYNCH_TRANSFER;

    //
    // Attach to the PDO
    //

    fdoExtension->LowerPdo = PhysicalDeviceObject;

    fdoExtension->CommonExtension.LowerDeviceObject =
    IoAttachDeviceToDeviceStack(deviceObject, PhysicalDeviceObject);

    if (fdoExtension->CommonExtension.LowerDeviceObject == NULL) {

        //
        // The attach failed. Cleanup and return.
        //

        status = STATUS_UNSUCCESSFUL;
        goto CreateChangerDeviceObjectExit;
    }

    //
    // Save system changer number
    //
    mcdClassData->PnpNameDeviceNumber = mcdCount - 1;

    //
    // The device is initialized properly - mark it as such.
    //

    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    ObDereferenceObject(lowerDevice);

    return(STATUS_SUCCESS);

    CreateChangerDeviceObjectExit:

    //
    // Release the device since an error occured.
    //

    // ClassClaimDevice(PortDeviceObject,
    //                      LunInfo,
    //                      TRUE,
    //                      NULL);

    ObDereferenceObject(lowerDevice);

    if (deviceObject != NULL) {
        IoDeleteDevice(deviceObject);
    }

    return status;


} // end CreateChangerDeviceObject()


NTSTATUS
ChangerClassSendSrbSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN WriteToDevice
    )
{
    return ClassSendSrbSynchronous(DeviceObject, Srb,
                                   Buffer, BufferSize,
                                   WriteToDevice);
}


PVOID
ChangerClassAllocatePool(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes
    )

{
    return ExAllocatePoolWithTag(PoolType, NumberOfBytes, 'CMcS');
}


VOID
ChangerClassFreePool(
    IN PVOID PoolToFree
    )
{
    ExFreePool(PoolToFree);
}


NTSTATUS
ChangerCreateSymbolicName(
    IN PDEVICE_OBJECT Fdo,
    IN BOOLEAN PersistencePreference
    )

/*++

Routine Description:

    This routine creates compares the unique identifier of the device with each
    one present in the registry (HKLM\System\CurrentControlSet\Contol\MChgr).
    If a match is found, the entry is used to create the symbolic name.
    If a match is not found, an entry is created for this device.

    If persistence is not requested, it just assigns the first available numeral
    that works.

Arguments:

    Fdo - a pointer to the functional device object for this device
    PersistencePreference - Indicates whether symbolic name needs to be persistent

Return Value:

    status

--*/

{
    NTSTATUS                status = STATUS_SUCCESS;
    PUCHAR                  deviceUniqueId = NULL;
    ULONG                   deviceUniqueIdLen = 0;
    UNICODE_STRING          unicodeRegistryPath = {0};
    OBJECT_ATTRIBUTES       objectAttributes = {0};
    HANDLE                  mcdKey;
    UNICODE_STRING          unicodeValueType = {0};
    UNICODE_STRING          unicodeValueUniqueId = {0};
    ULONG                   length = 0;
    ULONG                   subkeyIndex = 0;
    WCHAR                   tempString[MCD_BUFFER_MAXCOUNT] = {0};
    NTSTATUS                error = STATUS_SUCCESS;
    UNICODE_STRING          unicodeRegistryDevice = {0};
    NTSTATUS                result = STATUS_SUCCESS;
    HANDLE                  deviceKey = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION keyValueInfo = NULL;
    PULONG                  value = NULL;
    MCD_UID_TYPE            uidType = 0;
    PUCHAR                  regDeviceUniqueId = NULL;
    ULONG                   regDeviceUniqueIdLen = 0;
    BOOLEAN                 matchFound = FALSE;
    WCHAR                   wideNameBuffer[MCD_BUFFER_MAXCOUNT] = {0};
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PMCD_CLASS_DATA         mcdClassData = (PMCD_CLASS_DATA)(fdoExtension->CommonExtension.DriverData);
    PMCD_INIT_DATA          mcdInitData;
    BOOLEAN                 nonPersistent = FALSE;
    ULONG                   startNumeral = MAXLONG - 1;

    //
    // Persistence preference and use of legacy name format are mutually exclusive.
    // i.e. if persistence preferred, then use of non-legacy name format during object creation,
    //      if non-persistent preferred, then use of legacy name format during object creation.
    //
    BOOLEAN                 nameFormatIsLegacy = !PersistencePreference;

    PAGED_CODE();

    mcdInitData = IoGetDriverObjectExtension(Fdo->DriverObject,
                                             ChangerClassInitialize);
    if (!mcdInitData) {
        ASSERT(mcdInitData);

        return STATUS_NO_SUCH_DEVICE;
    }

    (ULONG_PTR)mcdClassData += mcdInitData->ChangerAdditionalExtensionSize();

    //
    // If non-persistent symbolic names has been requested, the NT name numeral
    // must match the symbolic name numeral.
    //
    if (!PersistencePreference) {

        ChangerCreateNonPersistentSymbolicName(NULL,
                                               &mcdClassData->PnpNameDeviceNumber,
                                               wideNameBuffer);

        RtlInitUnicodeString(&unicodeRegistryDevice,
                             wideNameBuffer);

        //
        // Assign \DosDevices\ChangerN to \Device\ChangerN
        //
        status = ChangerAssignSymbolicLink(mcdClassData->PnpNameDeviceNumber,
                                           &unicodeRegistryDevice,
                                           nameFormatIsLegacy,
                                           MCD_DOS_DEVICES_PREFIX_W,
                                           &mcdClassData->SymbolicNameDeviceNumber);

        if (NT_SUCCESS(status)) {

            mcdClassData->DosNameCreated = TRUE;

            fdoExtension->DeviceNumber = mcdClassData->SymbolicNameDeviceNumber;

            swprintf(wideNameBuffer,
                     MCD_DEVICE_NAME_FORMAT_W,
                     mcdClassData->PnpNameDeviceNumber);

            RtlInitUnicodeString(&unicodeRegistryDevice,
                                 wideNameBuffer);

            //
            // Assign \Device\MediumChangerDeviceN to \Device\ChangerN
            //
            status = ChangerAssignSymbolicLink(mcdClassData->PnpNameDeviceNumber,
                                               &unicodeRegistryDevice,
                                               nameFormatIsLegacy,
                                               MCD_DEVICE_PREFIX_W,
                                               NULL);

            if (NT_SUCCESS(status)) {

                mcdClassData->PnpNameLinkCreated = TRUE;
            }
        }

        return status;

    }

    //
    // If we've gotten here, it means that persistent symbolic names have been
    // requested. To create a persistent symbolic name, the requirement is that
    // the device have a unique Id. So for devices that don't have unique ids,
    // we shall fall back to using non-persistent symbolic names.
    // If a device can have a persistent symbolic name, but one doesn't already
    // exist, we shall assign one that doesn't cause an object name clash,
    // starting from MAXLONG-1 downwards.
    // If a device cannot have a persistent symbolic name, we'll assign one that
    // doesn't cause an object name collision, starting from 0 upwards.
    //

    //
    // Create the device's uniqueId based on vendor, product and serial number
    //
    status = ChangerCreateUniqueId(Fdo,
                                   MCD_UID_CUSTOM,
                                   &deviceUniqueId,
                                   &deviceUniqueIdLen);

    //
    // If a UniqueId couldn't be obtained, we can't guarantee persistence of
    // symbolic name.
    //
    if (!NT_SUCCESS(status)) {

        if (deviceUniqueId) {
            ExFreePool(deviceUniqueId);
        }

        //
        // However, if the reason for failure of Unique Id was because of device
        // not having all the needed inquiry data, create a symbolic name, but it
        // won't be persistent.
        //
        if (STATUS_UNSUCCESSFUL == status) {

            DebugPrint((1,
                        "ChangerCreateSymbolicName: Device %x won't have persistent symbolic name\n",
                        Fdo));

            nonPersistent = TRUE;

        } else {

            DebugPrint((1,
                        "ChangerCreateSymbolicName: Error %x obtaining uniqueId for device %x\n",
                        status,
                        Fdo));

            return status;
        }
    }

    //
    // TODO: In future releases, it may so happen that there is an Industry Standard device
    // unique identifier that is exposed by devices. If that happens, code must be added here
    // to obtain that Industry Standard Device Unique Identifier for this device.
    //

    RtlInitUnicodeString(&unicodeRegistryPath, MCD_REGISTRY_CONTROL_KEY);

    InitializeObjectAttributes(&objectAttributes,
                               &unicodeRegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    //
    // Open the global MChgr key.
    //
    status = ZwOpenKey(&mcdKey,
                       KEY_READ | KEY_WRITE,
                       &objectAttributes);

    if (NT_SUCCESS(status)) {

        //
        // If we're attempting to create a persistent symbolic name, we need
        // to check all the existing subkeys in the registy for a match. If
        // a match is found, we've found the persistent symbolic name for this
        // device. If a match is not found then we need to create a new subkey
        // for this device which will be its symbolic name.
        //
        if (!nonPersistent) {

            RtlInitUnicodeString(&unicodeValueType, MCD_DEVICE_UIDTYPE);
            RtlInitUnicodeString(&unicodeValueUniqueId, MCD_DEVICE_UNIQUEID);

            //
            // Enumerate all the devices that were previously ever seen.
            // For each one, compare its uniqueId with that of this one. If a
            // match is found, we've found our symbolic name. If we iterate
            // through all the ones in the registry and still don't find a match,
            // we need to create a new entry.
            //
            do {
                if (keyValueInfo) {
                    ExFreePool(keyValueInfo);
                    keyValueInfo = NULL;
                }

                status = ChangerGetSubKeyByIndex(mcdKey,
                                                 subkeyIndex,
                                                 &deviceKey,
                                                 tempString);

                if (NT_SUCCESS(status)) {

                    RtlInitUnicodeString(&unicodeRegistryDevice, tempString);

                    //
                    // Get the UID Type
                    //
                    result = ChangerGetValuePartialInfo(deviceKey,
                                                        &unicodeValueType,
                                                        &keyValueInfo,
                                                        NULL);

                    if (!NT_SUCCESS(result)) {

                        //
                        // Since there is a possibility that this was not the key
                        // that matches this device, continue with the next subkey(s).
                        // However, record the fact that we had a problem checking
                        // the details of at least one subkey.
                        //
                        error = result;

                        ZwClose(deviceKey);

                        DebugPrint((1,
                                    "ChangerCreateSymbolicName: Error %x querying %xth subkey's UID type\n",
                                    result,
                                    subkeyIndex));

                        subkeyIndex++;
                        continue;
                    }

                    value = (PULONG)(keyValueInfo->Data);
                    uidType = *value;

                    ExFreePool(keyValueInfo);
                    keyValueInfo = NULL;

                    //
                    // Get the UniqueId
                    //
                    result = ChangerGetValuePartialInfo(deviceKey,
                                                        &unicodeValueUniqueId,
                                                        &keyValueInfo,
                                                        &length);

                    if (!NT_SUCCESS(result)) {

                        //
                        // Since there is a possibility that this was not the key
                        // that matches this device, continue with the next subkey(s).
                        // However, record the fact that we had a problem checking
                        // the details of at least one subkey.
                        //
                        error = result;

                        ZwClose(deviceKey);

                        DebugPrint((1,
                                    "ChangerCreateSymbolicName: Error %x querying %xth subkey's UniqueId\n",
                                    result,
                                    subkeyIndex));

                        subkeyIndex++;
                        continue;
                    }

                    //
                    // TODO: In the future, there may some Industry Standard
                    // device unique identifier that every device exposes.
                    // It that happens, then the type may be that Industry
                    // Standard Device Unique Identifier, in which case,
                    // the unique identifier needs to be compared based on that
                    // identifier or custom UID, depending on the UIDType.
                    //
                    // For now though, only custom UID is available
                    //
                    ASSERT(uidType == MCD_UID_CUSTOM);

                    regDeviceUniqueId = (PUCHAR)(keyValueInfo->Data);
                    regDeviceUniqueIdLen = keyValueInfo->DataLength;

                    matchFound = ChangerCompareUniqueId(deviceUniqueId,
                                                        deviceUniqueIdLen,
                                                        regDeviceUniqueId,
                                                        regDeviceUniqueIdLen,
                                                        uidType);

                } else if (status == STATUS_INSUFFICIENT_RESOURCES) {

                    if (deviceUniqueId) {
                        ExFreePool(deviceUniqueId);
                    }

                    ZwClose(mcdKey);

                    return status;

                } else {
                    if (status != STATUS_NO_MORE_ENTRIES) {
                        error = status;
                    }
                }

                subkeyIndex++;

            } while (status != STATUS_NO_MORE_ENTRIES && !matchFound);

        }

    } else {

        //
        // If we can't open the key, it is pretty much senseless trying to
        // go on.
        //
        if (deviceUniqueId) {
            ExFreePool(deviceUniqueId);
        }

        DebugPrint((1, "ChangerCreateSymbolicName: Error %x opening registry key Control\\MChgr\n", status));

        return status;
    }

    //
    // If we couldn't get a unique identifier for the device, we will assign it
    // a non-persistent symbolic name. To do this, we will start from 0 and go
    // upwards, trying to find a numeral for which there is no corresponding key
    // in the registry (which means that there is no persistent symbolic created
    // with that numeral). Also, this numeral must not cause an object name collision
    // (which could happen if there is already a device with non-persistent symbolic
    // name using this very numeral).
    //
    if (nonPersistent) {

        startNumeral = 0;

        do {

            //
            // Get the first possible symbolic name numeral that doesn't have
            // an entry in the registry.
            //
            status = ChangerCreateNonPersistentSymbolicName(mcdKey,
                                                            &startNumeral,
                                                            wideNameBuffer);

            if (NT_SUCCESS(status)) {

                RtlInitUnicodeString(&unicodeRegistryDevice,
                                     wideNameBuffer);

                //
                // It is possible that the chosen numeral already has been
                // taken up by another device for which a non-persistent symbolic
                // name was created. So if the assigning of the symbolic name
                // using this numeral (startNumeral) fails with a collision, we
                // will continue our search.
                //
                // Assign \DosDevices\ChangerM to \Device\MediumChangerDeviceN
                //
                status = ChangerAssignSymbolicLink(mcdClassData->PnpNameDeviceNumber,
                                                   &unicodeRegistryDevice,
                                                   nameFormatIsLegacy,
                                                   MCD_DOS_DEVICES_PREFIX_W,
                                                   &mcdClassData->SymbolicNameDeviceNumber);

                if (NT_SUCCESS(status)) {

                    mcdClassData->DosNameCreated = TRUE;

                    //
                    // Assign \Device\ChangerM to \Device\MediumChangerDeviceN
                    //
                    status = ChangerAssignSymbolicLink(mcdClassData->PnpNameDeviceNumber,
                                                       &unicodeRegistryDevice,
                                                       nameFormatIsLegacy,
                                                       MCD_DEVICE_PREFIX_W,
                                                       NULL);

                    if (NT_SUCCESS(status)) {

                        mcdClassData->PnpNameLinkCreated = TRUE;

                        //
                        // Assign the device number only if we were able to create
                        // both these symbolic names successfully.
                        //
                        fdoExtension->DeviceNumber = mcdClassData->SymbolicNameDeviceNumber;

                    } else if (STATUS_OBJECT_NAME_COLLISION == status) {

                        //
                        // Delete the DosDevices symbolic link, since that was
                        // successfully created. Clear the flag to indicate that
                        // a symbolic link isn't assigned to this device, and
                        // also clear the value for DeviceNumber.
                        //
                        if (mcdClassData->DosNameCreated) {

                            ChangerDeassignSymbolicLink(MCD_DOS_DEVICES_PREFIX_W,
                                                        mcdClassData->SymbolicNameDeviceNumber,
                                                        FALSE);

                            mcdClassData->DosNameCreated = FALSE;
                        }
                    }
                }
            }

        } while (STATUS_OBJECT_NAME_COLLISION == status);

        if (NULL != mcdKey) {
            ZwClose(mcdKey);
        }

        return status;
    }

    if (NULL != deviceKey) {
        ZwClose(deviceKey);
    }

    //
    // We are going to assign a persistent symbolic name to the device, but we
    // couldn't find one that was previously assigned.
    //
    if (!matchFound) {

        //
        // If error was set to a non-success status, it is possible that the entry
        // that caused this error was actually a match for the current device, so
        // we won't take any chances and will just fail this function call.
        //
        if (!NT_SUCCESS(error)) {

            if (keyValueInfo) {
                ExFreePool(keyValueInfo);
            }

            if (deviceUniqueId) {
                ExFreePool(deviceUniqueId);
            }

            ZwClose(mcdKey);
            return error;
        }
    }

    startNumeral = MAXLONG - 1;

    do {

        //
        // We successfully went through all the devices seen previously for which
        // persistent symbolic names were created and we didn't find a match for
        // this device. So we shall assign a persistent symbolic name to this
        // device and update the registry with this info.
        // In order to assign a symbolic name, we'll start MAXLONG-1 counting
        // downwards, and looking for the first numeral that doesn't have an
        // entry in the registry. We'll try and assign this as the symbolic name.
        // However, that may fail because of that symbolic name having been taken
        // up already by a device with non-persistent symbolic name. In such a
        // case, we'll continue searching.
        //
        if (!matchFound) {

            //
            // A new device found being seen for the very first time, so create a
            // persistent symbolic name for it.
            //
            // For this we first need to create a key (by its symbolic name) for it.
            //
            status = ChangerCreateNewDeviceSubKey(mcdKey,
                                                  &startNumeral,
                                                  &deviceKey,
                                                  wideNameBuffer);

            if (NT_SUCCESS(status)) {

                RtlInitUnicodeString(&unicodeRegistryDevice,
                                     wideNameBuffer);

                //
                // That's it - we managed to create the subkey for the new
                // device. Now update the registry entry with the type and uniqueId
                //
                uidType = MCD_UID_CUSTOM;

                status = ZwSetValueKey(deviceKey,
                                       &unicodeValueType,
                                       0,
                                       REG_DWORD,
                                       (PVOID)&uidType,
                                       sizeof(uidType));

                if (NT_SUCCESS(status)) {

                    status = ZwSetValueKey(deviceKey,
                                           &unicodeValueUniqueId,
                                           0,
                                           REG_BINARY,
                                           (PVOID)deviceUniqueId,
                                           deviceUniqueIdLen);
                }

                if (keyValueInfo) {
                    ExFreePool(keyValueInfo);
                }

                if (!NT_SUCCESS(status)) {

                    //
                    // We failed to update either of the values into the registry.
                    // The key has incomplete information. Delete it.
                    //
                    ZwDeleteKey(deviceKey);
                }

            }
        } else {

            //
            // TODO: If a match was found, but the comparision was based on custom
            // UID, if an Industry Standard device unique identifier is now exposed
            // by devices, and we were able to obtain this identifier for this
            // device, update the registry key for this device with this identifier
            // and new type.
            //
        }


        if (deviceUniqueId) {
            ExFreePool(deviceUniqueId);
        }

        ZwClose(mcdKey);

        if (NT_SUCCESS(status)) {

            //
            // Assign \DosDevices\ChangerM to \Device\MediumChangerDeviceN
            //
            status = ChangerAssignSymbolicLink(mcdClassData->PnpNameDeviceNumber,
                                               &unicodeRegistryDevice,
                                               nameFormatIsLegacy,
                                               MCD_DOS_DEVICES_PREFIX_W,
                                               &mcdClassData->SymbolicNameDeviceNumber);

            if (NT_SUCCESS(status)) {

                mcdClassData->DosNameCreated = TRUE;
                mcdClassData->PersistentSymbolicName = TRUE;

                fdoExtension->DeviceNumber = mcdClassData->SymbolicNameDeviceNumber;

                //
                // Assign \Device\ChangerM to \Device\MediumChangerDeviceN
                //
                error = ChangerAssignSymbolicLink(mcdClassData->PnpNameDeviceNumber,
                                                  &unicodeRegistryDevice,
                                                  nameFormatIsLegacy,
                                                  MCD_DEVICE_PREFIX_W,
                                                  NULL);

                if (NT_SUCCESS(error)) {

                    mcdClassData->PnpNameLinkCreated = TRUE;
                }
            }

            if (!matchFound) {

                if (status == STATUS_OBJECT_NAME_COLLISION) {

                    //
                    // This means that we created a subkey in the registry but there
                    // is likely a non-persistent symbolic name already created for
                    // that numeral, so delete the created key and re-try with
                    // another numeral.
                    //
                    ZwDeleteKey(deviceKey);

                } else {

                    ZwClose(deviceKey);
                }
            }
        }

    } while (status == STATUS_OBJECT_NAME_COLLISION && !matchFound);

    if (NT_SUCCESS(status) && !NT_SUCCESS(error)) {

        status = error;
    }

    return status;
}


NTSTATUS
ChangerCreateUniqueId(
    IN PDEVICE_OBJECT Fdo,
    IN MCD_UID_TYPE UIDType,
    OUT PUCHAR *UniqueId,
    OUT PULONG UniqueIdLen
    )

/*++

Routine Description:

    This routine builds a unique Id for the device based on the type requested.
    If custom, then the unique id is based on its vendor string, product string
    and serial number. The caller is responsible for releasing the resources.
    In the future, if some Industry Standard Device Unique Identifier will be
    exposed by devices, that identifer will be returned.

Arguments:

    Fdo - a pointer to the functional device object for this device
    UIDType - indicates the type of unique identifier being requested
    UniqueId - a pointer to a string that represents the unique id
    UniqueIdLen - returns the length of the UniqueId

Return Value:

    STATUS_UNSUCCESSFUL - if we won't ever be able to create a unique Id
    STATUS_INVALID_PARAMETER - passed in UIDType is not recognized
    STATUS_INSUFFICIENT_RESOURCES - out of memory
    STATUS_SUCCESS - success

--*/

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PSTORAGE_DEVICE_DESCRIPTOR deviceDescriptor = fdoExtension->DeviceDescriptor;
    PUCHAR vendorId;
    PUCHAR productId;
    PUCHAR serialNumber;
    ULONG vendorIdLen;
    ULONG productIdLen;
    ULONG serialNumberLen;
    ULONG length;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG offset = 0;

    PAGED_CODE();

    switch (UIDType) {

    case MCD_UID_CUSTOM: {

            //
            // For custom UID, vendor, product and serial number is needed.
            //
            if ((deviceDescriptor->VendorIdOffset == (ULONG)(-1) ||
                 deviceDescriptor->ProductIdOffset == (ULONG)(-1) ||
                 deviceDescriptor->SerialNumberOffset == (ULONG)(-1)) ||
                !(deviceDescriptor->VendorIdOffset &&
                  deviceDescriptor->ProductIdOffset &&
                  deviceDescriptor->SerialNumberOffset)) {

                *UniqueId = NULL;
                return STATUS_UNSUCCESSFUL;
            }

            vendorId = (PUCHAR)((ULONG_PTR)deviceDescriptor + deviceDescriptor->VendorIdOffset);
            productId = (PUCHAR)((ULONG_PTR)deviceDescriptor + deviceDescriptor->ProductIdOffset);
            serialNumber = (PUCHAR)((ULONG_PTR)deviceDescriptor + deviceDescriptor->SerialNumberOffset);

            vendorIdLen = strlen(vendorId);
            productIdLen = strlen(productId);
            serialNumberLen = strlen(serialNumber);

            length =  vendorIdLen + productIdLen + serialNumberLen + 1;

            *UniqueId = ExAllocatePool(NonPagedPool, length);

            if (!(*UniqueId)) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlCopyMemory(*UniqueId,
                          (PUCHAR)((ULONG_PTR)deviceDescriptor + deviceDescriptor->VendorIdOffset),
                          vendorIdLen);

            offset += vendorIdLen;
            RtlCopyMemory(*UniqueId + offset,
                          (PUCHAR)((ULONG_PTR)deviceDescriptor + deviceDescriptor->ProductIdOffset),
                          productIdLen);

            offset += productIdLen;
            RtlCopyMemory(*UniqueId + offset,
                          (PUCHAR)((ULONG_PTR)deviceDescriptor + deviceDescriptor->SerialNumberOffset),
                          serialNumberLen);

            (*UniqueId)[length-1] = '\0';
            *UniqueIdLen = length;

            break;
        }

    default: {

            status = STATUS_INVALID_PARAMETER;
            break;
        }
    }

    return status;
}


BOOLEAN
ChangerCompareUniqueId(
    IN PUCHAR UniqueId1,
    IN ULONG UniqueId1Len,
    IN PUCHAR UniqueId2,
    IN ULONG UniqueId2Len,
    IN MCD_UID_TYPE UIDType
    )

/*++

Routine Description:

    This routine compares the two passed in unique Ids. The comparsion method is
    based on the type passed in.
    If custom, then comparision is based on its vendor string, product string
    and serial number.

Arguments:

    UniqueId1 - a pointer to the first unique id
    UniqueId1Len - length of the first unique id
    UniqueId2 - a pointer to the second unique id
    UniqueId2Len - length of the second unique id
    UIDType - indicates the basis used for creating the unique id

Return Value:

    TRUE - if IDs match,
    FALSE - otherwise

--*/

{
    BOOLEAN match = FALSE;
    ULONG index = 0;

    PAGED_CODE();

    if (UniqueId1Len && UniqueId1Len == UniqueId2Len) {

        switch (UIDType) {

        case MCD_UID_CUSTOM: {

                for (index = 0; index < UniqueId1Len; index++) {
                    if (UniqueId1[index] != UniqueId2[index]) {
                        break;
                    }
                }

                if (index == UniqueId1Len) {
                    match = TRUE;
                }

                break;
            }

        default: {

                break;
            }
        }
    }

    return match;
}


NTSTATUS
ChangerGetSubKeyByIndex(
    IN HANDLE RootKey,
    IN ULONG SubKeyIndex,
    OUT PHANDLE SubKey,
    OUT PWCHAR ReturnString
    )

/*++

Routine Description:

    This routine gets returns the name of the SubKeyIndex-th subkey relative to
    the RootKey. Caller must pass in a buffer that is big enough to store the
    name. Caller is also responsible to close the handle to the subkey.

Arguments:

    RootKey - a handle to the Root Key relative to which the subkey is being indexed
    SubKeyIndex - index of subkey that's being queried
    SubKey - pointer that will hold the handle to the subkey referenced by passed in index
    ReturnString - pointer the pre-allocated buffer in which the name is passed back

Return Value:

    status

--*/

{
    PKEY_BASIC_INFORMATION  keyBasicInfo = NULL;
    ULONG                   length = sizeof(KEY_BASIC_INFORMATION);
    NTSTATUS                status = STATUS_SUCCESS;
    UNICODE_STRING          unicodeRegistryDevice = {0};
    OBJECT_ATTRIBUTES       deviceAttributes = {0};

    PAGED_CODE();

    if (!ReturnString) {

        return STATUS_INVALID_PARAMETER;
    }

    do {
        if (keyBasicInfo) {
            ExFreePool(keyBasicInfo);
        }

        keyBasicInfo = ExAllocatePool(NonPagedPoolCacheAligned, length);
        if (!keyBasicInfo) {

            DebugPrint((1,
                        "ChangerGetSubKeyNameByIndex: Out of memory enumerating global MChgr's %xth subkey\n",
                        SubKeyIndex));

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        status = ZwEnumerateKey(RootKey,
                                SubKeyIndex,
                                KeyBasicInformation,
                                keyBasicInfo,
                                length,
                                &length);

    } while (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW);

    if (NT_SUCCESS(status)) {

        //
        // Get the subkey name. It will be of the form ChangerN (non-NULL
        // terminated)
        //
        RtlCopyMemory(ReturnString, keyBasicInfo->Name, keyBasicInfo->NameLength);
        ReturnString[keyBasicInfo->NameLength / sizeof(WCHAR)] = L'\0';

        RtlInitUnicodeString(&unicodeRegistryDevice, ReturnString);

        InitializeObjectAttributes(&deviceAttributes,
                                   &unicodeRegistryDevice,
                                   OBJ_CASE_INSENSITIVE,
                                   RootKey,
                                   NULL);

        status = ZwOpenKey(SubKey,
                           GENERIC_READ,
                           &deviceAttributes);
    }

    if (keyBasicInfo) {
        ExFreePool(keyBasicInfo);
    }

    return status;
}


NTSTATUS
ChangerGetValuePartialInfo(
    IN HANDLE Key,
    IN PUNICODE_STRING ValueName,
    OUT PKEY_VALUE_PARTIAL_INFORMATION *KeyValueInfo,
    OUT PULONG KeyValueInfoSize
    )

/*++

Routine Description:

    This routine returns the KeyValuePartialInformation for the passed in key's
    particular value. It returns the structure and its size. It is the
    responsibility of the caller to free the KeyValueInfo buffer

Arguments:

    Key - handle to the key being queried
    ValueName - pointer to the unicode string representation of the value being queried
    KeyValueInfo - pointer to the KeyValuePartialInfomation that contains the results of the query
    KeyValueInfoSize - pointer whose contents get updated with the size of KeyValueInfo. This can be NULL.

Return Value:

    status

--*/

{
    PKEY_VALUE_PARTIAL_INFORMATION  keyValueInfo = NULL;
    ULONG                           length = sizeof(KEY_VALUE_PARTIAL_INFORMATION);
    NTSTATUS                        status = STATUS_SUCCESS;

    PAGED_CODE();

    do {
        if (keyValueInfo) {
            ExFreePool(keyValueInfo);
        }

        keyValueInfo = ExAllocatePool(NonPagedPoolCacheAligned, length);
        if (!keyValueInfo) {

            DebugPrint((1,
                        "ChangerGetValuePartialInfo: Out of memory querying Value: %s\n",
                        ValueName->Buffer));

            return STATUS_INSUFFICIENT_RESOURCES;
        }

        status = ZwQueryValueKey(Key,
                                 ValueName,
                                 KeyValuePartialInformation,
                                 keyValueInfo,
                                 length,
                                 &length);

    } while (status == STATUS_BUFFER_TOO_SMALL || status == STATUS_BUFFER_OVERFLOW);

    if (NT_SUCCESS(status)) {

        *KeyValueInfo = keyValueInfo;

        if (KeyValueInfoSize) {
            *KeyValueInfoSize = length;
        }

    } else {

        ExFreePool(keyValueInfo);
    }

    return status;
}


NTSTATUS
ChangerCreateNewDeviceSubKey(
    IN HANDLE RootKey,
    IN OUT PULONG FirstNumeralToUse,
    OUT PHANDLE NewSubKey,
    OUT PWCHAR NewSubKeyName
    )

/*++

Routine Description:

    This routine creates a new subkey relative to RootKey. The handle to this
    new subkey as well as its name (as string of WCHAR) are returned. Caller
    must pass in a buffer large enough to contain the new subkey's name.

Arguments:

    RootKey - handle to the root key relative to which the subkey needs to be created
    FirstNumeralToUse - Hueristic value to use as the initial trial value
    NewSubKey - pointer in which handle to newly created subkey is returned
    NewSubKeyName - pointer to pre-allocated buffer to hold the name of the new subkey

Return Value:

    status

--*/

{
    ULONG               index = 0;
    WCHAR               wideNameBuffer[MCD_BUFFER_MAXCOUNT] = {0};
    UNICODE_STRING      unicodeDeviceName = {0};
    OBJECT_ATTRIBUTES   deviceAttributes = {0};
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               disposition;

    PAGED_CODE();

    if (!NewSubKeyName) {

        return STATUS_INVALID_PARAMETER;
    }

    for (index = *FirstNumeralToUse; index < MAXLONG && index != MAXLONG; index--) {

        swprintf(wideNameBuffer,
                 MCD_NAME_FORMAT_W,
                 index);

        RtlInitUnicodeString(&unicodeDeviceName,
                             wideNameBuffer);


        InitializeObjectAttributes(&deviceAttributes,
                                   &unicodeDeviceName,
                                   OBJ_CASE_INSENSITIVE,
                                   RootKey,
                                   NULL);

        status = ZwCreateKey(NewSubKey,
                             KEY_READ | KEY_WRITE,
                             &deviceAttributes,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             &disposition);

        if (!NT_SUCCESS(status)) {

            DebugPrint((1,
                        "ChangerCreateNewDeviceSubKey: Failed with status %x to create subkey for new device\n",
                        status));

            //
            // No problem. Let us just try using another ChangerN
            //
            continue;

        } else {

            ASSERT(REG_CREATED_NEW_KEY == disposition || REG_OPENED_EXISTING_KEY == disposition);

            if (REG_OPENED_EXISTING_KEY == disposition) {

                DebugPrint((3,
                            "ChangerCreateNewDeviceSubKey: Changer%u already exists\n",
                            index));

                //
                // Some other thread must have created this key before we
                // could. No problem, we'll just try another.
                //
                ZwClose(*NewSubKey);
                continue;

            } else {

                //
                // That's it - we managed to create the new key
                //
                wcscpy(NewSubKeyName, wideNameBuffer);
                *FirstNumeralToUse = index - 1;
                break;

            }
        }
    }

    //
    // TODO: Revisit this algo.
    // This algorithm has the shortcoming that it doesn't allow for ChangerN where
    // N equals MAXLONG. Also, since many apps append %d (i.e. \\.\Changer%d) instead
    // of %u, we cannot use the upper 2G numbers in the ULONG space.
    //
    if (index == MAXLONG) {
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}


NTSTATUS
ChangerCreateNonPersistentSymbolicName(
    IN HANDLE RootKey,
    IN PULONG FirstNumeralToUse,
    OUT PWCHAR NonPersistentSymbolicName
    )

/*++

Routine Description:

    This routine determines the non-persistent symbolic name for the device.
    If the RootKey is non-NULL, the non-persistent symbolic name must be one
    that isn't present in the registry.
    If the passed in RootKey is NULL, then we just use the passed in numeral.

Arguments:

    RootKey - handle to the root key.
    FirstNumeralToUse - pointer to determine which numeral to count down from
    NonPersistentSymbolicName - pointer to pre-allocated buffer to hold the name of the new subkey

Return Value:

    status

--*/

{
    ULONG               index = 0;
    WCHAR               wideNameBuffer[MCD_BUFFER_MAXCOUNT] = {0};
    UNICODE_STRING      unicodeDeviceName = {0};
    OBJECT_ATTRIBUTES   deviceAttributes = {0};
    NTSTATUS            status = STATUS_SUCCESS;
    HANDLE              hSubKey;

    PAGED_CODE();

    if (!NonPersistentSymbolicName || !FirstNumeralToUse) {

        return STATUS_INVALID_PARAMETER;
    }

    if (NULL == RootKey) {

        swprintf(NonPersistentSymbolicName,
                 MCD_NAME_FORMAT_W,
                 *FirstNumeralToUse);

    } else {

        for (index = *FirstNumeralToUse; index < MAXLONG && index != MAXLONG; index++) {

            swprintf(wideNameBuffer,
                     MCD_NAME_FORMAT_W,
                     index);

            RtlInitUnicodeString(&unicodeDeviceName,
                                 wideNameBuffer);

            InitializeObjectAttributes(&deviceAttributes,
                                       &unicodeDeviceName,
                                       OBJ_CASE_INSENSITIVE,
                                       RootKey,
                                       NULL);

            status = ZwOpenKey(&hSubKey,
                               KEY_READ,
                               &deviceAttributes);

            if (NT_SUCCESS(status)) {

                //
                // Symbolic name already exists, can't use it.
                //
                DebugPrint((3,
                            "ChangerCreateNonPersistentSymbolicName: Changer%u already exists\n",
                            index));

                ZwClose(hSubKey);

                continue;

            } else if (STATUS_INVALID_HANDLE == status || STATUS_OBJECT_NAME_NOT_FOUND == status) {

                //
                // That's it. This one is worth a try.
                //
                wcscpy(NonPersistentSymbolicName, wideNameBuffer);
                *FirstNumeralToUse = index + 1;
                status = STATUS_SUCCESS;

                break;

            } else {

                DebugPrint((1,
                            "ChangerCreateNonPersistentSymbolicName: Failed with status %x while opening key Changer%u\n",
                            status,
                            index));

                //
                // No problem. Let us just try using another ChangerN
                //
                continue;

            }
        }

        //
        // TODO: Revisit this algo.
        // This algorithm has the shortcoming that it doesn't allow for ChangerN where
        // N equals MAXLONG. Also, since many apps append %d (i.e. \\.\Changer%d) instead
        // of %u, we cannot use the upper 2G numbers in the ULONG space.
        //
        if (index == MAXLONG) {
            status = STATUS_UNSUCCESSFUL;
        }
    }

    return status;
}


NTSTATUS
ChangerAssignSymbolicLink(
    IN ULONG DeviceNumber,
    IN PUNICODE_STRING RegDeviceName,
    IN BOOLEAN LegacyNameFormat,
    IN WCHAR *SymbolicNamePrefix,
    OUT PULONG SymbolicNameNumeral
    )

/*++

Routine Description:

    This routine creates the symbolic link for the changer device represented by
    passed in DeviceNumber. If the optional SymbolicNameNumeral is non-NULL,
    it abstracts the symbolic name numeral from the passed in RegDeviceName
    and returns it in SymbolicNameNumeral.

Arguments:

    DeviceNumber - the N in device name \Device\ChangerN
    RegDeviceName - pointer to unicode representation of the device's registry key name
    LegacyNameFormat - if TRUE, device was created as \Device\ChangerN, else device was created as \Device\MediumChangerDeviceN
    SymbolicNamePrefix - Prefix to use when creating the symbolic name
    SymbolicNameNumeral - optional pointer to return the N from the device's registry key name ChangerN

Return Value:

    status

--*/

{
    WCHAR                   wideDeviceNameBuffer[MCD_BUFFER_MAXCOUNT] = {0};
    UNICODE_STRING          deviceUnicodeString = {0};
    WCHAR                   wideSymbolicNameBuffer[MCD_BUFFER_MAXCOUNT] = {0};
    UNICODE_STRING          dosUnicodeString = {0};
    NTSTATUS                status = STATUS_SUCCESS;
    ANSI_STRING             ansiSymbolicName = {0};
    ULONG                   prefixLength = 0;

    PAGED_CODE();

    wcscpy(wideDeviceNameBuffer, MCD_DEVICE_PREFIX_W);
    swprintf(wideDeviceNameBuffer + wcslen(wideDeviceNameBuffer),
             (LegacyNameFormat ? MCD_DEVICE_NAME_PREFIX_LEGACY_W : MCD_DEVICE_NAME_PREFIX_W),
             DeviceNumber);

    RtlInitUnicodeString(&deviceUnicodeString,
                         wideDeviceNameBuffer);

    wcsncpy(wideSymbolicNameBuffer,
            SymbolicNamePrefix,
            MCD_BUFFER_MAXCOUNT-1);
    wcsncat(wideSymbolicNameBuffer,
            L"\\",
            MCD_BUFFER_MAXCOUNT - 1 - wcslen(wideSymbolicNameBuffer));
    wcsncat(wideSymbolicNameBuffer,
            RegDeviceName->Buffer,
            MCD_BUFFER_MAXCOUNT - 1 - wcslen(wideSymbolicNameBuffer));

    RtlInitUnicodeString(&dosUnicodeString,
                         wideSymbolicNameBuffer);

    if (SymbolicNameNumeral) {

        status = RtlUnicodeStringToAnsiString(&ansiSymbolicName,
                                              &dosUnicodeString,
                                              TRUE);

        if (NT_SUCCESS(status)) {

            if (0 == wcscmp(SymbolicNamePrefix, MCD_DOS_DEVICES_PREFIX_W)) {

                prefixLength = strlen(MCD_DOS_DEVICES_PREFIX_A);

            } else if (0 == wcscmp(SymbolicNamePrefix, MCD_DEVICE_PREFIX_W)) {

                prefixLength = strlen(MCD_DEVICE_PREFIX_A);

            } else {

                ASSERT(prefixLength);
            }

            status = RtlCharToInteger(ansiSymbolicName.Buffer + prefixLength + strlen("\\Changer"), 10, SymbolicNameNumeral);

            if (!NT_SUCCESS(status)) {

                DebugPrint((1,
                            "ChangerAssignSymbolicLink: Failed to retreive symbolic name numeral. Status %x\n",
                            status));
            }

            RtlFreeAnsiString(&ansiSymbolicName);

        } else {

            DebugPrint((1,
                        "ChangerAssignSymbolicLink: Failed to convert device name from unicode to ansi. Status %x\n",
                        status));
        }
    }

    status = IoAssignArcName(&dosUnicodeString,
                             &deviceUnicodeString);

    if (!NT_SUCCESS(status)) {

        DebugPrint((1,
                    "ChangerAssignSymbolicLink: Failed to assign symbolic link to device. Status %x\n",
                    status));
    }

    return status;
}


NTSTATUS
ChangerDeassignSymbolicLink(
    PWCHAR SymbolicNamePrefix,
    ULONG SymbolicNameNumeral,
    BOOLEAN LegacyNameFormat
    )

/*++

Routine Description:

    This routine deletes the symbolic name represented by SymbolicNamePrefix and SymbolicNameNumeral.

Arguments:

    SymbolicNamePrefix - Prefix to use when creating the symbolic name
    SymbolicNameNumeral - symbolic link's numeral suffix
    LegacyNameFormat - indicates if MediumChangerDeviceN is to be used instead of ChangerN when forming the symbolic link name
                       (LegacyNameFormat devices will have symlink \Device\MediumChangerDeviceN and \DosDevices\ChangerN
                        others should have \Device\ChangerN and \DosDevices\ChangerN)
                       This parameter has meaning only for \Device\..., since in the case of \DosDevices it will always
                       be ChangerN regardless of whether the device was created as \Device\ChangerN or \Device\MediumChangerDeviceN

Return Value:

    status

--*/

{
    WCHAR                   wideSymbolicNameBuffer[MCD_BUFFER_MAXCOUNT] = {0};
    UNICODE_STRING          symbolicNameUnicodeString = {0};
    ULONG                   prefixLength = 0;

    PAGED_CODE();

    wcsncpy(wideSymbolicNameBuffer,
            SymbolicNamePrefix,
            MCD_BUFFER_MAXCOUNT-1);

    swprintf(wideSymbolicNameBuffer + wcslen(wideSymbolicNameBuffer),
             (LegacyNameFormat ? MCD_DEVICE_NAME_PREFIX_W : MCD_NAME_PREFIX_W),
             SymbolicNameNumeral);

    RtlInitUnicodeString(&symbolicNameUnicodeString,
                         wideSymbolicNameBuffer);

    return IoDeleteSymbolicLink(&symbolicNameUnicodeString);
}


NTSTATUS
ChangerSymbolicNamePersistencePreference(
    PDRIVER_OBJECT DriverObject,
    PWCHAR KeyName,
    MCD_PERSISTENCE_OPERATION Operation,
    PWCHAR ValueName,
    PBOOLEAN PersistencePreference
    )

/*++

Routine Description:

    This routine either querys the ValueName (relative to the KeyName) and
    returns it in PersistencePreference, or sets the value pointed to by
    PersistencePreference in the registry for ValueName (relative to KeyName).
    The basic use of this routine is to determine whether symbolic name
    persistence needs to be attempted.
    By default, persistence will not be attempted. If persistence is wished for,
    the user (with admin previlege) will need to explicitly changed the value of
    this key in the registry.

Arguments:

    DriverObject - driver object
    KeyName - full path to the registry key
    Operation - determines whether to query the valuename or set it
    ValueName - valuename to which the operation needs to be applied
    PersistencePreference - value read from the registry or to be set

Return Value:

    status

--*/

{
    UNICODE_STRING      unicodeRegistryPath = {0};
    OBJECT_ATTRIBUTES   objectAttributes = {0};
    NTSTATUS            status;
    HANDLE              mcdKey;
    ULONG               disposition;
    UNICODE_STRING      unicodeKeyValue = {0};
    ULONG               persistence = 0;
    PKEY_VALUE_PARTIAL_INFORMATION keyValueInfo = NULL;
    PULONG              value = NULL;

    PAGED_CODE();
    UNREFERENCED_PARAMETER(DriverObject);

    if (!KeyName || !ValueName || !PersistencePreference) {

        return STATUS_INVALID_PARAMETER;
    }

    RtlInitUnicodeString(&unicodeRegistryPath, KeyName);

    InitializeObjectAttributes(&objectAttributes,
                               &unicodeRegistryPath,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = ZwCreateKey(&mcdKey,
                         KEY_READ | KEY_WRITE,
                         &objectAttributes,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         &disposition);

    if (NT_SUCCESS(status)) {

        ASSERT(REG_CREATED_NEW_KEY == disposition || REG_OPENED_EXISTING_KEY == disposition);

        RtlInitUnicodeString(&unicodeKeyValue, ValueName);

        //
        // If the key is created, initialize Persistence to 0.
        // If it already exists, read its value.
        //
        if (REG_CREATED_NEW_KEY == disposition) {

            status = ZwSetValueKey(mcdKey,
                                   &unicodeKeyValue,
                                   0,
                                   REG_DWORD,
                                   &persistence,
                                   sizeof(persistence));

            ASSERT(NT_SUCCESS(status));

            *PersistencePreference = persistence ? TRUE : FALSE;

            status = STATUS_SUCCESS;

        } else {

            if (Operation == MCD_PERSISTENCE_QUERY) {

                status = ChangerGetValuePartialInfo(mcdKey,
                                                    &unicodeKeyValue,
                                                    &keyValueInfo,
                                                    NULL);

                if (NT_SUCCESS(status)) {

                    value = (PULONG)keyValueInfo->Data;
                    *PersistencePreference = (*value > 0) ? TRUE : FALSE;

                } else if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

                    ZwSetValueKey(mcdKey,
                                  &unicodeKeyValue,
                                  0,
                                  REG_DWORD,
                                  &persistence,
                                  sizeof(persistence));

                    status = STATUS_SUCCESS;

                }

                if (keyValueInfo) {
                    ExFreePool(keyValueInfo);
                }

            } else if (Operation == MCD_PERSISTENCE_SET) {

                persistence = (*PersistencePreference) ? 1 : 0;

                status = ZwSetValueKey(mcdKey,
                                       &unicodeKeyValue,
                                       0,
                                       REG_DWORD,
                                       &persistence,
                                       sizeof(persistence));

            } else {

                status = STATUS_INVALID_PARAMETER;
            }
        }

        ZwClose(mcdKey);

    } else {
        DebugPrint((1,
                    "ChangerSymbolicNamePersistencePreference: Error %x opening registry key Control\\MChgr\n",
                    status));
    }

    return status;
}


#if DBG
    #define MCHGR_DEBUG_PRINT_BUFF_LEN 127
ULONG MCDebug = 0;
UCHAR DebugBuffer[MCHGR_DEBUG_PRINT_BUFF_LEN + 1];
#endif


#if DBG

VOID
ChangerClassDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

/*++

Routine Description:

    Debug print for all medium changer drivers

Arguments:

    Debug print level between 0 and 3, with 3 being the most verbose.

Return Value:

    None

--*/

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= MCDebug) {

        _vsnprintf(DebugBuffer, MCHGR_DEBUG_PRINT_BUFF_LEN,
                   DebugMessage, ap);
        DebugBuffer[MCHGR_DEBUG_PRINT_BUFF_LEN] = '\0';

        DbgPrintEx(DPFLTR_MCHGR_ID, DPFLTR_INFO_LEVEL, DebugBuffer);
    }

    va_end(ap);

} // end MCDebugPrint()

#else

//
// DebugPrint stub
//

VOID
ChangerClassDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )
{
}

#endif

