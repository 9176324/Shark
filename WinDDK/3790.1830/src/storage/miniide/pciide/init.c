/*++

Copyright (C) Microsoft Corporation, 1999 - 1999

Module Name:

    init.c

Abstract:

    Generic PCI IDE mini driver

Revision History:

--*/

#include "pciide.h"

//
// Driver Entry Point
//                               
NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{
    //
    // call system pci ide driver (pciidex)
    // for initializeation
    //
    return PciIdeXInitialize (
        DriverObject,
        RegistryPath,
        GenericIdeGetControllerProperties,
        sizeof (DEVICE_EXTENSION)
        );
}


//
// Called on every I/O. Returns 1 if DMA is allowed.
// Returns 0 if DMA is not allowed.
//
ULONG
GenericIdeUseDma(
    IN PVOID DeviceExtension,
    IN PVOID cdbcmd,
    IN UCHAR slave)
/**++
 * Arguments : DeviceExtension
               Cdb
               Slave =1, if slave
                     =0, if master
--**/
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    PUCHAR cdb= cdbcmd;

    return 1;
}

//
// Query controller properties
//                     
NTSTATUS 
GenericIdeGetControllerProperties (
    IN PVOID                      DeviceExtension,
    IN PIDE_CONTROLLER_PROPERTIES ControllerProperties
    )
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    NTSTATUS    status;
    ULONG       i;
    ULONG       j;
    ULONG       xferMode;

    //
    // make sure we are in sync
    //                              
    if (ControllerProperties->Size != sizeof (IDE_CONTROLLER_PROPERTIES)) {

        return STATUS_REVISION_MISMATCH;
    }

    //
    // see what kind of PCI IDE controller we have
    //                               
    status = PciIdeXGetBusData (
                 deviceExtension,
                 &deviceExtension->pciConfigData, 
                 0,
                 sizeof (PCIIDE_CONFIG_HEADER)
                 );

    if (!NT_SUCCESS(status)) {

        return status;
    }

    //
    // assume we support all DMA mode if PCI master bit is set
    //                            
    xferMode = PIO_SUPPORT;
    if ((deviceExtension->pciConfigData.MasterIde) &&
        (deviceExtension->pciConfigData.Command.b.MasterEnable)) {

        xferMode |= SWDMA_SUPPORT | MWDMA_SUPPORT | UDMA_SUPPORT;
    }

    
    //
    // fill in the controller properties
    //            
    for (i=0; i< MAX_IDE_CHANNEL; i++) {

        for (j=0; j< MAX_IDE_DEVICE; j++) {

            ControllerProperties->SupportedTransferMode[i][j] =
                deviceExtension->SupportedTransferMode[i][j] = xferMode;
        }
    }

    ControllerProperties->PciIdeChannelEnabled     = GenericIdeChannelEnabled;
    ControllerProperties->PciIdeSyncAccessRequired = GenericIdeSyncAccessRequired;
    ControllerProperties->PciIdeTransferModeSelect = NULL;
    ControllerProperties->PciIdeUdmaModesSupported = GenericIdeUdmaModesSupported;
    ControllerProperties->PciIdeUseDma = GenericIdeUseDma;
    ControllerProperties->AlignmentRequirement=1;

    return STATUS_SUCCESS;
}

IDE_CHANNEL_STATE
GenericIdeChannelEnabled (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG Channel
    )
{
    //
    // Can't tell if a channel is enabled or not.  
    //
    return ChannelStateUnknown;
}



BOOLEAN 
GenericIdeSyncAccessRequired (
    IN PDEVICE_EXTENSION DeviceExtension
    )
{
    ULONG i;

    return FALSE;
}


NTSTATUS
GenericIdeUdmaModesSupported (
    IN IDENTIFY_DATA    IdentifyData,
    IN OUT PULONG       BestXferMode,
    IN OUT PULONG       CurrentMode
    )
{
    ULONG bestXferMode =0;
    ULONG currentMode = 0;

    if (IdentifyData.TranslationFieldsValid & (1 << 2)) {

        if (IdentifyData.UltraDMASupport) {

            GetHighestTransferMode( IdentifyData.UltraDMASupport,
                                       bestXferMode);
            *BestXferMode = bestXferMode;
        }

        if (IdentifyData.UltraDMAActive) {

            GetHighestTransferMode( IdentifyData.UltraDMAActive,
                                       currentMode);
            *CurrentMode = currentMode;
        }
    }

    return STATUS_SUCCESS;
}




