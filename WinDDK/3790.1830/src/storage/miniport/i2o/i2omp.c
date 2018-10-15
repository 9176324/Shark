/*++

Copyright (c) 1998-2002  Microsoft Corporation

Module Name:

    i2omp.c

Abstract:

    This miniport supports some I2O HBA-RAID controllers and is designed to work with
    the StorPort port driver.

Author:

Environment:

    kernel mode only

Revision History:
 
--*/

#include <miniport.h>
#include <storport.h>
#include "i2omp.h"

//
// To keep a count of the total adapters found.
// 
ULONG GlobalAdapterCount;

//
// Indicates whether this is in the context of a 
// crashdump or hibernate.
//
BOOLEAN IOPInitializedForDump = FALSE;


//
// Miniport entry point decls.
//

BOOLEAN
I2OInitialize(
    IN PVOID DeviceExtension
    );

BOOLEAN
I2OBuildIo(
    IN PVOID DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

BOOLEAN
I2OStartIo(
    IN PVOID DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

ULONG
I2OFindAdapter(
    IN PVOID DeviceExtension,
    IN PVOID HwContext,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    );

BOOLEAN
I2OResetBus(
    IN PVOID DeviceExtension,
    IN ULONG PathId
    );

BOOLEAN
I2OInterrupt(
    IN PVOID DeviceExtension
    );

SCSI_ADAPTER_CONTROL_STATUS
I2OAdapterControl (
    IN PVOID DeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    );

//
// Internal function decls.
//
VOID
I2OReadCompletion(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    );

VOID
I2OWriteCompletion(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    );

ULONG
I2OBuildSGList(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PI2O_SG_ELEMENT  SgList
    );

VOID
I2OCompleteRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

BOOLEAN
I2OResetIop(
    IN PDEVICE_EXTENSION DeviceExtension
    );

VOID
I2OInitCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME MessageFrame,
    IN PVOID CallbackContext
    );

BOOLEAN
I2OClaimDevice(
    IN PDEVICE_EXTENSION DeviceExtension
    );

VOID
I2OLctNotifyCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME MessageFrame,
    IN PVOID CallbackContext
    );

BOOLEAN
I2OIssueLctNotify(
    IN PDEVICE_EXTENSION DeviceExtension,
    PIOP_CALLBACK_OBJECT CallBackObject,
    IN ULONG ChangeIndicator
    );

VOID
I2ODriveCapacityCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID CallbackContext
    );

BOOLEAN
I2OSendMessage(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PI2O_MESSAGE_FRAME MessageBuffer
    );

BOOLEAN
I2OGetStatus(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PI2O_EXEC_STATUS_GET_REPLY GetStatusReply,
    IN STOR_PHYSICAL_ADDRESS GetStatusReplyPhys
    );

BOOLEAN
I2OInitOutbound(
    IN PDEVICE_EXTENSION DeviceExtension
    );

BOOLEAN
I2OGetExpectedLCTSizeAndMFrames(
    IN PDEVICE_EXTENSION DeviceExtension
    );

BOOLEAN
I2OGetLCT(
    IN PDEVICE_EXTENSION DeviceExtension
    );

PSCSI_REQUEST_BLOCK
I2OHandleError(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG TransactionContextOffset
    );

BOOLEAN
I2ORegisterForEvents(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG EventMask
    );

VOID
I2ODeviceEvent(
    IN PI2O_UTIL_EVENT_REGISTER_REPLY Msg,
    IN PVOID DeviceContext
    );

BOOLEAN 
I2OSysQuiesce(
    IN PDEVICE_EXTENSION DeviceExtension
    );

ULONG
MapError(
    IN PI2O_SINGLE_REPLY_MESSAGE_FRAME  Msg,
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    );

VOID
I2OAsyncClaimCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME MessageFrame,
    IN PVOID CallbackContext
    );

BOOLEAN
I2OClaimRelease(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG Tid
    );

VOID
I2OClaimReleaseCallback(
    IN PI2O_SINGLE_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    );

BOOLEAN
I2OBuildDeviceName(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PI2O_VOL_ROW_INFO VolumeInfo
    );

ULONG
I2OHandleShutdown(
    IN PDEVICE_EXTENSION DeviceExtension
    );

BOOLEAN
I2OAsyncClaimDevice(
    IN PDEVICE_EXTENSION DeviceExtension
    );
      
VOID
I2OBSADeviceResetCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME  Msg,
    IN PVOID DeviceContext
    );

VOID 
I2OBSADeviceResetTimer(
    IN PVOID deviceExtension
    );

VOID
I2OBSADeviceResetComplete(
    IN PDEVICE_EXTENSION deviceExtension
    );

VOID
I2OBSAPowerMgtCompletion(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    );

ULONG
I2OBSAPowerMgt(IN PDEVICE_EXTENSION DeviceExtension,
               IN PSCSI_REQUEST_BLOCK Srb);

BOOLEAN
I2OGetSetTCLParam(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN GetParams
    );

VOID
I2OSetTCLParamCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    );

VOID
I2OGetTCLParamCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    );

VOID
I2OGetDeviceStateCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    );

ULONG
I2OSetSpecialParams(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN USHORT GroupNumber,
    IN USHORT FieldIdx,
    IN USHORT Value
    );


//
// The code.
//


VOID
DumpTables(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    Dumps the contents of the LCT to the debugger.

Arguments:

    DeviceExtension
    
Return Value:

    NONE

--*/
{
    PI2O_LCT lct = DeviceExtension->Lct;
    PI2O_DEVICE_INFO oldLct;
    ULONG lctEntries;
    ULONG t;
    
    DebugPrint((1,
                "\nLCT TABLE: \n"));

    lctEntries = (lct->TableSize-3) / 9;
    
    DebugPrint((1,"#Entries: %d.\t",  lctEntries));
    DebugPrint((1,"Table Size: %d.\t",  lct->TableSize));
    DebugPrint((1,"CurrentChg: %d.\tUpdateChange: %d.\n---- LCT Entries:\n",
                  lct->CurrentChangeIndicator,
                  DeviceExtension->UpdateLct->CurrentChangeIndicator));

    for (t = 0; t < lctEntries; t++) {
        DebugPrint((1,"Entry #: %d\t", t));
        DebugPrint((1,"LocalTID: 0x%x\t", lct->LCTEntry[t].LocalTID));
        DebugPrint((1,"UserTID: 0x%x\t", lct->LCTEntry[t].UserTID));
        DebugPrint((1,"Class: 0x%x\n", lct->LCTEntry[t].ClassID.Class));
    }
    
    DebugPrint((1,"\nLOCAL DEVICES TABLE: \n"));

    for (t = 0; t < IOP_MAXIMUM_TARGETS; t++) {
        oldLct = &DeviceExtension->DeviceInfo[t];
        if (oldLct->DeviceFlags != DFLAGS_NO_DEVICE) {
            DebugPrint((1,"Index: 0x%x.\t",t));
            DebugPrint((1,"dv->Lct.LocalTID:0x%x\t", oldLct->Lct.LocalTID));
            DebugPrint((1,"dv->LCTIndex: %d\t", oldLct->LCTIndex));
            DebugPrint((1,"dv->DeviceState: 0x%x\t", oldLct->DeviceState));
            DebugPrint((1,"dv->DeviceFlags: 0x%x\t", oldLct->DeviceFlags));
            DebugPrint((1,"dv->Class: 0x%x\n\n", oldLct->Class));
        }
    }
    return;
} 


ULONG
I2OGetNextAvailableId(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG Tid
    )
/*++

Routine Description:

    This routine will find the next available DeviceInfo
    index to use for the newly found device.  It first
    scans the DeviceInfo to see if an entry for the current
    TID is already in the table.  If a match is found then
    return that index.  Otherwise search for the next 
    free location and use that.  This will insure that 
    the same scsi target Ids will be present accross ACPI
    power events.
    
Arguments:

    DeviceExtension
    Tid - The TID for the new device from the LCT

Return Value:

    An available Id or 0xFF if none are available.
    NOTE: Should handle this error case by using multiple Paths.
    
--*/
{
    ULONG Id;

    //
    // See if this Tid already exists in the DeviceInfo.  If it does then
    // use the same entry now to preserve the Target Id assignment.  If
    // not then pick the next available slot.
    //
    for (Id = 0; Id < IOP_MAXIMUM_TARGETS; Id++) {
        if (DeviceExtension->DeviceInfo[Id].Lct.LocalTID == Tid) {
            //
            // Use the same target Id to avoid having them move
            // from under the OS.
            //
            DebugPrint((1,
                        "I20GetNextAvailableID: Re-using device scsi id 0x%x Tid 0x%x\n",
                        Id,
                        Tid));
            DebugPrint((1,
                        "dv->Lct.LocalTID:0x%x\t",
                        DeviceExtension->DeviceInfo[Id].Lct.LocalTID));
            DebugPrint((1,
                        "dv->LCTIndex: %d\t",
                        DeviceExtension->DeviceInfo[Id].LCTIndex));
            DebugPrint((1,"dv->DeviceState: 0x%x\t",
                        DeviceExtension->DeviceInfo[Id].DeviceState));
            DebugPrint((1,
                        "dv->DeviceFlags: 0x%x\t", 
                        DeviceExtension->DeviceInfo[Id].DeviceFlags));
            DebugPrint((1,
                        "dv->Class: 0x%x\n\n",
                        DeviceExtension->DeviceInfo[Id].Class));

            return Id;
        }
    }

    //
    // Not found in the current DeviceInfo so this must be a new device.  Pick the next
    // available id. This will insure that all of the
    // devices that existed before can retain their same Target Id.
    //
    for (Id = 0; Id < IOP_MAXIMUM_TARGETS; Id++) {
        if (Id == 7) {
            continue;
        } else if (DeviceExtension->DeviceInfo[Id].DeviceFlags == DFLAGS_NO_DEVICE) {
            break;
        }
    }
    if (Id == IOP_MAXIMUM_TARGETS) {

        //
        // Out of IDs.
        // NOTE: Should go to multiple bus approach.
        //
        DebugPrint((0,
                    "I2OGetNextAvailableId: Out of IDs\n"));
        Id = 0xFF;
    }
    return Id;
}


VOID
I2OBuildDeviceInfo(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PI2O_LCT Lct,
    IN ULONG EntryIndex,
    IN BOOLEAN NeedsClaim
    )
/*++

Routine Description:

    This routine builds a deviceInfo entry based on the EntryIndex into
    the LCT. Allocates a free targetId and if specified, will setup the
    entry for a claim to be issued.
    
Arguments:

    DeviceExtension
    LCT - The current LCT table
    EntryIndex - The index for the specific device in the LCT
    NeedsClaim - Indicator as to whether tha caller wants the device
                 to be claimed.
Return Value:
    
--*/
{
    ULONG targetId;
    
    //
    // Mark this entry as being an unclaimed block device or scsi perph.
    //
    DeviceExtension->DeviceMap |= (1 << EntryIndex);

    DebugPrint((1,
                "I2OBuildDeviceInfo: DeviceMap now (%x)\n",
                DeviceExtension->DeviceMap));

    //
    // Get the id that will be used. NOTE: There should
    // be a check here for overflow.
    //
    targetId = I2OGetNextAvailableId(DeviceExtension,Lct->LCTEntry[EntryIndex].LocalTID);

    DebugPrint((1,
               "12OBuildDeviceInfo: Type (%x) scsi id 0x%x local TID 0x%x need claim 0x%x\n",
               Lct->LCTEntry[EntryIndex].ClassID.Class,
               targetId,Lct->LCTEntry[EntryIndex].LocalTID,
               NeedsClaim));

    //
    // Copy the LCT entry into the deviceInfo.
    //
    StorPortMoveMemory(&DeviceExtension->DeviceInfo[targetId].Lct,
                       &Lct->LCTEntry[EntryIndex],
                       sizeof(I2O_LCT_ENTRY));

    //
    // Set the rest of the important information.
    //
    DeviceExtension->DeviceInfo[targetId].Class =
                                    (USHORT)Lct->LCTEntry[EntryIndex].ClassID.Class;
    DeviceExtension->DeviceInfo[targetId].LCTIndex = EntryIndex;
    DeviceExtension->DeviceInfo[targetId].DeviceFlags = 0;    
    DeviceExtension->DeviceInfo[targetId].OutstandingIOCount = 0;    

    //
    // Mark it as needing a claim, if specified.
    // 
    if (NeedsClaim) {
        DeviceExtension->DeviceInfo[targetId].DeviceFlags |= DFLAGS_NEED_CLAIM;
    }

    return;
}   


BOOLEAN
I2ORemoveSingleDevice(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG RemoveID             
    )
/*++

Routine Description:
    
    This routine is called when a device in the local table needs to be
    removed because of a SINGLE failed claim.  The entry # associated with 
    the RemoveID is cleared by reseting the device flags and state.

Arguments:

    DeviceExtension 
    RemoveID - The SCSI ID of the device to be removed from the mask.
    
Return Value:
    
    TRUE - The ID was removed successfully.
    FALSE - No entries for the RemoveID were found.

--*/
{
    PI2O_DEVICE_INFO deviceInfo = &DeviceExtension->DeviceInfo[RemoveID];

    //
    // Indicate that it's not there.
    //
    DeviceExtension->DeviceMap &= ~(1 << deviceInfo->LCTIndex);
    DebugPrint((2,
                "I2ORemoveSingleDevice: DeviceMap now (%x)\n",
                DeviceExtension->DeviceMap));
   
    //
    // Clear out the minimum info to insure this device is gone.
    //
    deviceInfo->DeviceFlags = 0;    
    deviceInfo->DeviceState = 0;    

    DebugPrint((1,
                "I2ORemoveSingleDevice: Removing ID (%x), LocalTID (%x)\n",
                RemoveID,
                deviceInfo->Lct.LocalTID));

    return TRUE;
}


BOOLEAN
I2OStrComp(
    IN PCHAR String1,
    IN PCHAR String2
    )
/*++

Routine Description:

    This routine is used to compare String1 and String2. It assumes both are NULL-terminated,
    and that any casing processing has already occurred.

Arguments:

    String1 - Base string.
    String2 - The one to compare.

Return Value:

    TRUE - If they are the same. 

--*/
{
    PCHAR index1 = String1;
    PCHAR index2 = String2;

    while(*index1) {
        if (*index1 != *index2) {
            return FALSE;
        }
        index1++;
        index2++;
    }    
    return TRUE;
}    


BOOLEAN
InLdrOrDump(
    IN PCHAR ArgumentString
    )
/*++

Routine Description:

    This routine is used to determine the context in which FindAdapter
    is being called.

Arguments:

    ArgumentString - String passed to FindAdapter
    
Return Value:

    TRUE - If being used for crashdump/hibernate/ntbootdd.sys

--*/
{
    PCHAR stringIndex;
    stringIndex = ArgumentString;
    
    if (stringIndex == NULL) {
        return FALSE;
    }

    //
    // Lower-case the string.
    //
    while (*stringIndex) {

        if (*stringIndex >= 'A' && *stringIndex <= 'Z') {
            *stringIndex = *stringIndex + ('a' - 'A');
        }
        stringIndex++;
    }    

    //
    // Reset the pointer.
    // 
    stringIndex = ArgumentString;

    //
    // If ntldr=1 or dump=1 return true, as 
    // resources are limited in these contexts.
    //
    if (I2OStrComp("ntldr=1", stringIndex)) {
        
        //
        // In the context of the loader.
        // 
        return TRUE;
        
    } else if (I2OStrComp("dump=1", stringIndex)) {
        
        //
        // In context of a crashdump or hibernate.
        //
        return TRUE;
    }

    //
    // Normal context.
    //
    return FALSE;
}    


ULONG
I2OGetDeviceId(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PPORT_CONFIGURATION_INFORMATION ConfigInfo
    )
/*++

Routine Description:

    Reads the PCI config space and returns the DeviceID of the
    IOP.

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    ConfigInfo - Port Configuration Information containing the Bus and Slot where the IOP resides.
    
Return Value:

    DeviceID

--*/
{
    ULONG dataLength;
    PCI_COMMON_CONFIG pciConfigInfo;

    dataLength = StorPortGetBusData(DeviceExtension,
                                    PCIConfiguration,
                                    ConfigInfo->SystemIoBusNumber,
                                    ConfigInfo->SlotNumber,
                                    &pciConfigInfo,
                                    sizeof(PCI_COMMON_HDR_LENGTH));
    if (dataLength) {
        DebugPrint((1,
                    "DeviceID : %x\n",
                    pciConfigInfo.DeviceID));
    }
    
    return (ULONG)pciConfigInfo.DeviceID;
}


ULONG
I2OFindAdapter(
    IN PVOID DeviceExtension,
    IN PVOID HwContext,
    IN PVOID BusInformation,
    IN PCHAR ArgumentString,
    IN OUT PPORT_CONFIGURATION_INFORMATION ConfigInfo,
    OUT PBOOLEAN Again
    )
/*++

Routine Description:

    This routine is called by StorPortInitialize after it finds
    the device/vendor id's corresponding to what it's passed in DriverEntry.
    Sets up the deviceExtension so that it's ready for the HwInit.  Resets
    the IOP and calulcates the MaximumTransferLength and 
    NumberOfPhysicalBreaks that the IOP can handle.

Arguments:
    DeviceExtension
    HwContext - The HW context that was passed to StorPortInitialize
    BusInformation - PCI bus specific information
    ArgumentString 
    ConfigInfo - Port Configuarion data
    Again - Not used
    
Return Value:

    SP_RETURN_FOUND - The IOP was found and reset
    SP_RETURN_NOT_FOUND - The IOP was not found.
    SP_RETURN_ERROR - Could not map the device base, 
                      could not get a device extension,
                      or could not reset the IOP.
--*/

{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    PACCESS_RANGE accessRange = &((*(ConfigInfo->AccessRanges))[0]);
    ULONG status;
    ULONG_PTR newAddress;
    ULONG totalSize;
    ULONG length = 0;
    ULONG sgSize;
    BOOLEAN ntldrDump = FALSE;
    PI2O_EXEC_STATUS_GET_REPLY statusGet;

    DebugPrint((3,
                "I2oFindAdapter\n"));

    //
    // Determine the context in which we are being called.
    // Resources are constrained in these special environments.
    //
    if (InLdrOrDump(ArgumentString)) {
        deviceExtension->AdapterFlags |= AFLAGS_IN_LDR_DUMP;
        ntldrDump = TRUE;
    }

    //
    // Insure that we have not already initialized this driver 
    // for the dump path.  If we have then report no IOP found.
    //
    if (ntldrDump && IOPInitializedForDump) {
        return SP_RETURN_NOT_FOUND;
    }

    //
    // Map the newly found address space.
    //
    newAddress = (ULONG_PTR)StorPortGetDeviceBase(deviceExtension,
                                           PCIBus,
                                           ConfigInfo->SystemIoBusNumber,
                                           accessRange->RangeStart,
                                           accessRange->RangeLength,
                                           (BOOLEAN)!accessRange->RangeInMemory);
    if (newAddress == (ULONG_PTR)NULL) {
        DebugPrint((0,
                    "I2OFindAdapter: Couldn't map %x for %x bytes\n",
                   (*ConfigInfo->AccessRanges)[0].RangeStart.LowPart,
                   (*ConfigInfo->AccessRanges)[0].RangeLength));
        return SP_RETURN_ERROR;
    }

    deviceExtension->PhysicalMemoryAddress = accessRange->RangeStart.LowPart;
    deviceExtension->PhysicalMemorySize = accessRange->RangeLength;
    deviceExtension->MappedAddress = (PUCHAR)newAddress;

    //
    // Carve up this memory into the various control areas.
    //
    deviceExtension->MessageAlloc   = (PULONG)((PUCHAR)newAddress + MESSAGEFRAME_ALLOC);
    deviceExtension->MessagePost    = (PULONG)((PUCHAR)newAddress + MESSAGE_POST);
    deviceExtension->MessageReply   = (PULONG)((PUCHAR)newAddress + MESSAGE_REPLY);
    deviceExtension->MessageRelease = (PULONG)((PUCHAR)newAddress + MESSAGEFRAME_RELEASE);
    deviceExtension->IntControl     = (PULONG)((PUCHAR)newAddress + INT_CNTL_STAT_OFFSET);
    deviceExtension->IntAsserted    = (PULONG)((PUCHAR)newAddress + INT_ASSERTED_OFFSET);

    //
    // Setup the callback object used during initialization.
    //
    deviceExtension->InitCallback.Callback.Signature       = CALLBACK_OBJECT_SIG;
    deviceExtension->InitCallback.Callback.CallbackAddress = I2OInitCallBack;
    deviceExtension->InitCallback.Callback.CallbackContext = deviceExtension;

    //
    // Setup the callback object used for handling LctNotify messages
    //
    deviceExtension->LctCallback.Callback.Signature       = CALLBACK_OBJECT_SIG;
    deviceExtension->LctCallback.Callback.CallbackAddress = I2OLctNotifyCallBack;
    deviceExtension->LctCallback.Callback.CallbackContext = deviceExtension;
   
    //
    // Setup the callback used to Claim devices asynchronously - if a device is created
    // or a hotplug occurs.
    //
    deviceExtension->AsyncClaimCallback.Callback.Signature       = CALLBACK_OBJECT_SIG;
    deviceExtension->AsyncClaimCallback.Callback.CallbackAddress = I2OAsyncClaimCallBack;
    deviceExtension->AsyncClaimCallback.Callback.CallbackContext = deviceExtension;

    //
    // Setup the callback object used for device reset indications and
    // acknowledgement.
    //
    deviceExtension->DeviceResetCallback.Callback.Signature = CALLBACK_OBJECT_SIG;
    deviceExtension->DeviceResetCallback.Callback.CallbackAddress = I2OBSADeviceResetCallback;
    deviceExtension->DeviceResetCallback.Callback.CallbackContext = deviceExtension;

    //
    // Setup the callback object used for event indications and
    // acknowledgement.
    //
    deviceExtension->EventCallback.Callback.Signature = CALLBACK_OBJECT_SIG;
    deviceExtension->EventCallback.Callback.CallbackAddress = I2ODeviceEvent;
    deviceExtension->EventCallback.Callback.CallbackContext = deviceExtension;

    //
    // Setup the callback object used for claim release indications and
    // acknowledgement.
    //
    deviceExtension->ClaimReleaseCallback.Callback.Signature = CALLBACK_OBJECT_SIG;
    deviceExtension->ClaimReleaseCallback.Callback.CallbackAddress = I2OClaimReleaseCallback;
    deviceExtension->ClaimReleaseCallback.Callback.CallbackContext = deviceExtension;

    //
    // Setup the callback object used for determining the current device state
    // release indications and acknowledgement.
    //
    deviceExtension->GetDeviceStateCallback.Callback.Signature = CALLBACK_OBJECT_SIG;
    deviceExtension->GetDeviceStateCallback.Callback.CallbackAddress = I2OGetDeviceStateCallback;
    deviceExtension->GetDeviceStateCallback.Callback.CallbackContext = deviceExtension;

    //
    //
    // Fill in configInfo fields that make sense
    // to do so. Some of these are just made up.
    //
    //
    ConfigInfo->MaximumTransferLength   = 0x10000;
    ConfigInfo->MaximumNumberOfTargets  = IOP_MAXIMUM_TARGETS;  
    ConfigInfo->NumberOfPhysicalBreaks  = 17;
    ConfigInfo->NumberOfBuses           = IOP_NUMBER_OF_BUSSES;
    ConfigInfo->InitiatorBusId[0]       = 7;
    ConfigInfo->ScatterGather           = TRUE;
    ConfigInfo->CachesData              = TRUE;
    ConfigInfo->Master                  = TRUE;
    ConfigInfo->Dma32BitAddresses       = TRUE;
    ConfigInfo->NeedPhysicalAddresses   = TRUE;
    ConfigInfo->TaggedQueuing           = TRUE;
    ConfigInfo->AlignmentMask           = 0x3;

    //
    // For StorPort MapBuffers is set to STOR_MAP_NON_READ_WRITE_BUFFERS so that 
    // virtual addresses are only generated for non read/write requests.  
    // Selecting the FullDuplex synchronization model
    // allows Storport to run I2OStartIo and I2OInterrupt at the same time, improving 
    // performance on multiprocessor platforms.
    //
    ConfigInfo->MapBuffers              = STOR_MAP_NON_READ_WRITE_BUFFERS;
    ConfigInfo->SynchronizationModel    = StorSynchronizeFullDuplex;

    //
    // Get uncached extension for the reply buffer.
    // 16 is used assuming 17 physical breaks. This will suffice
    // for now, but should be determined from the adapter's capabilities.
    //
    if (ntldrDump) {
        deviceExtension->ReplyBufferSize = 8 * PAGE_SIZE;
    } else {
        deviceExtension->ReplyBufferSize = 16 * PAGE_SIZE;
    }    

    deviceExtension->MessageFrameSize = 128;
    deviceExtension->NumberReplyBuffers = (deviceExtension->ReplyBufferSize) /
                                          deviceExtension->MessageFrameSize;

    //
    // Add to the size enough space for 2 pages. This will
    // be used during initialization to put return data in.
    //
    totalSize = deviceExtension->ReplyBufferSize + (PAGE_SIZE * 2);

    //
    // In addition, the LCTs (both saved and in case of change) need to be in the 
    // common buffer also.
    //
    totalSize += (PAGE_SIZE * 2);

    //
    // Get the Uncached extension that contains the reply buffers, scratch, and LCTs
    //
    deviceExtension->ReplyBuffer = StorPortGetUncachedExtension(deviceExtension,
                                                                ConfigInfo,
                                                                totalSize);
    if (deviceExtension->ReplyBuffer == NULL) {
       
        //
        // Log this error and return now.
        //
        StorPortLogError(deviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         SP_INTERNAL_ADAPTER_ERROR,I2OMP_ERROR_INSUFFICIENT_RESOURCES);
        return SP_RETURN_ERROR;
    }
    //
    // Need the physical address of the reply buffer in order
    // to setup messages.
    //
    length = deviceExtension->ReplyBufferSize;
    deviceExtension->ReplyBufferPA = StorPortGetPhysicalAddress(deviceExtension,
                                                                NULL,
                                                                deviceExtension->ReplyBuffer,
                                                                &length);

    //
    // Offset is used in making physical to virtual calculations.
    //
    deviceExtension->OffsetAddress = (ULONG_PTR)((PUCHAR)deviceExtension->ReplyBuffer -
                                              deviceExtension->ReplyBufferPA.LowPart);
    DebugPrint((1,
               "I2OFindAdapter: ReplyBuffer %x PhysAddress %x. Asked for %x and got %x\n",
                deviceExtension->ReplyBuffer,
                deviceExtension->ReplyBufferPA.LowPart,
                totalSize,
                length));

    //
    // Set the location of the scratch buffer.
    // and get it's physical address.
    //
    deviceExtension->ScratchBuffer = (PUCHAR)((ULONG_PTR)deviceExtension->ReplyBuffer +
                                                     deviceExtension->ReplyBufferSize);
    length = (PAGE_SIZE * 2);
    deviceExtension->ScratchBufferPA = StorPortGetPhysicalAddress(deviceExtension,
                                                                  NULL,
                                                                  deviceExtension->ScratchBuffer,
                                                                  &length);
    //
    // Set the location of the LCT buffer.
    // and get it's physical address.
    //
    deviceExtension->Lct = (PI2O_LCT)((ULONG_PTR)deviceExtension->ScratchBuffer +
                                                     (PAGE_SIZE * 2));
    deviceExtension->UpdateLct = (PI2O_LCT)((ULONG_PTR)deviceExtension->ScratchBuffer +
                                                       (PAGE_SIZE * 3));
    length = PAGE_SIZE;
    deviceExtension->LctPA = StorPortGetPhysicalAddress(deviceExtension,
                                                        NULL,
                                                        deviceExtension->Lct,
                                                        &length);
    length = PAGE_SIZE;
    deviceExtension->UpdateLctPA = StorPortGetPhysicalAddress(deviceExtension,
                                                              NULL,
                                                              deviceExtension->UpdateLct,
                                                              &length);
    //
    // Reset the Iop. 
    //
    if (!I2OResetIop(DeviceExtension)) {
        StorPortLogError(deviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         SP_INTERNAL_ADAPTER_ERROR,I2OMP_ERROR_RESET_FAILED);
        return SP_RETURN_ERROR;
    }

    StorPortNotification(ResetDetected,
                         deviceExtension,
                         0);

    //
    // If the reset completed OK then valid StatusGet information for 
    // this IOP will exist in the deviceExtension->ReplyBuffer.  Use
    // this to update the maximum transfer size and number of 
    // physical breaks that the IOP can handle.
    //
    statusGet = (PI2O_EXEC_STATUS_GET_REPLY)deviceExtension->ReplyBuffer;
    
    //
    // Calculate the maximum size of the scatter gather list based
    // on the InboundMFrameSize (which is the max size of each inbound message
    // in long words.  Convert to bytes, subtract the size of the message 
    // header, and divide by the size of a page list scatter gather element.
    // Note that the BSA write message structure already includes the first 
    // SG element.
    //
    sgSize = ((statusGet->InboundMFrameSize * sizeof(ULONG)) -
                sizeof(I2O_BSA_WRITE_MESSAGE)) / sizeof(ULONG);
                
    //
    // Adjust the PORT_CONFIGURATION_INFORMATION to reflect the IOPs
    // true data transfer capababilities.
    //
    ConfigInfo->MaximumTransferLength = (sgSize - 2) * PAGE_SIZE;
    ConfigInfo->NumberOfPhysicalBreaks = sgSize - 1;

    DebugPrint((1,
               "I2OFindAdapter: InboundMFrameSize 0x%x Max SG Elements 0x%x\n",
                statusGet->InboundMFrameSize,
                sgSize));

    DebugPrint((1,
               "I2OFindAdapter: MaximumTransferLength 0x%x NumberOfPhysicalBreaks 0x%x\n",
                ConfigInfo->MaximumTransferLength,
                ConfigInfo->NumberOfPhysicalBreaks));

    //
    // Indicate the number of adapters found.
    //
    GlobalAdapterCount++;
    deviceExtension->AdapterOrdinal = GlobalAdapterCount - 1;

    deviceExtension->DeviceId = I2OGetDeviceId(deviceExtension,
                                               ConfigInfo);

    if (ntldrDump) {
        //
        // When this routine is called by the dump driver to 
        // handle writing a crash dump or hibernate file to a drive
        // handled by the IOP, this routine will get called for EACH
        // call that driverEntry makes to StorPortIntialize.  In this 
        // case the config info that is passed in will be the config
        // info for the IOP that is in the hibernate or dump path.
        // We only need to initialize the IOP once when in dump mode
        // so set a flag here to insure we don't load again.
        //
        IOPInitializedForDump = TRUE;
    }

    return SP_RETURN_FOUND;
}


ULONG
DriverEntry(
    IN PVOID  DriverObject,
    IN PVOID  RegistryPath
    )

/*++

Routine Description:

    This routine initializes the I2O Block Storage class driver.

Arguments:

    DriverObject - Pointer to driver object created by system.
    RegistryPath - Pointer to the name of the services node for this driver.

Return Value:

    The function value is the final status from the initialization operation.

--*/

{

    HW_INITIALIZATION_DATA hwInitializationData;
    ULONG i;
    UCHAR intelId[4]   = {'8', '0', '8', '6'};
    UCHAR stAgeId[4]   = {'1', '7', '5', 'A'};
    UCHAR deviceId1[4]  = {'9', '6', '2', '1'};
    UCHAR deviceId2[4] = {'9', '6', 'A', '1'};
    UCHAR deviceId3[4] = {'9', '6', '4', '1'};
    UCHAR deviceId4[4] = {'9', '6', '2', '2'};
    UCHAR deviceId5[4] = {'3', '0', '9', '2'};

    ULONG status;
    ULONG status1;
    ULONG status2;
    ULONG status3;
    ULONG status4;
    ULONG status5;
    ULONG retVal;

    DebugPrint((3,
                "I2oDriverEntry\n"));

    for (i = 0; i < sizeof(HW_INITIALIZATION_DATA); i++) {
       ((PCHAR)&hwInitializationData)[i] = 0;
    }

    hwInitializationData.HwInitializationDataSize = sizeof(HW_INITIALIZATION_DATA);

    //
    // Set entry points into the miniport.
    //
    hwInitializationData.HwInitialize = I2OInitialize;
    hwInitializationData.HwStartIo = I2OStartIo;
    hwInitializationData.HwInterrupt = I2OInterrupt;
    hwInitializationData.HwFindAdapter = I2OFindAdapter;
    hwInitializationData.HwResetBus = I2OResetBus;
    hwInitializationData.HwAdapterControl = I2OAdapterControl;
    
    //
    // For StorPort, we need the following entry point as well.
    //
    hwInitializationData.HwBuildIo = I2OBuildIo;

    //
    // Sizes of the structures that port needs to allocate.
    //
    hwInitializationData.DeviceExtensionSize = sizeof(DEVICE_EXTENSION);
    hwInitializationData.SrbExtensionSize = sizeof(SRB_EXTENSION);
    hwInitializationData.SpecificLuExtensionSize = 0;

    //
    // One access range. The memory range of the 960
    // that's accessible will be mapped.
    //
    hwInitializationData.NumberOfAccessRanges = 1;
    hwInitializationData.NeedPhysicalAddresses = TRUE;
    hwInitializationData.TaggedQueuing = TRUE;
    hwInitializationData.AutoRequestSense = TRUE;
    hwInitializationData.MultipleRequestPerLu = TRUE;

    //
    // For StorPort MapBuffers is set to STOR_MAP_NON_READ_WRITE_BUFFERS so that 
    // virtual addresses are only generated for non read/write requests.  
    //
    hwInitializationData.MapBuffers = STOR_MAP_NON_READ_WRITE_BUFFERS;

    //
    // Set PCIBus, vendor/device id.
    //
    hwInitializationData.AdapterInterfaceType = PCIBus;
    hwInitializationData.VendorIdLength = 4;
    hwInitializationData.VendorId = intelId;
    hwInitializationData.DeviceIdLength = 4;
    hwInitializationData.DeviceId = deviceId1;

    //
    // Call StorPort for each supported adapter.
    //
    status = StorPortInitialize(DriverObject,
                                RegistryPath,
                                &hwInitializationData,
                                NULL);
    
    hwInitializationData.DeviceId = deviceId2;
    status1 = StorPortInitialize(DriverObject,
                                 RegistryPath,
                                 &hwInitializationData,
                                 NULL);

    hwInitializationData.DeviceId = deviceId3;
    status2 = StorPortInitialize(DriverObject,
                                 RegistryPath,
                                 &hwInitializationData,
                                 NULL);

    hwInitializationData.DeviceId = deviceId4;
    status3 = StorPortInitialize(DriverObject,
                                 RegistryPath,
                                 &hwInitializationData,
                                 NULL);

    hwInitializationData.DeviceId = deviceId5;
    status4 = StorPortInitialize(DriverObject,
                                 RegistryPath,
                                 &hwInitializationData,
                                 NULL);
    
    hwInitializationData.VendorId = stAgeId;
    hwInitializationData.DeviceId = deviceId4;
    status5 = StorPortInitialize(DriverObject,
                                 RegistryPath,
                                 &hwInitializationData,
                                 NULL);
   
    retVal = (status < status1 ? status : status1);
    retVal = (retVal < status2 ? retVal : status2);
    retVal = (retVal < status3 ? retVal : status3);
    retVal = (retVal < status4 ? retVal : status4);
    retVal = (retVal < status5 ? retVal : status5);

    status = retVal;

    return status;
}


SCSI_ADAPTER_CONTROL_STATUS
I2OAdapterControl (
    IN PVOID DeviceExtension,
    IN SCSI_ADAPTER_CONTROL_TYPE ControlType,
    IN PVOID Parameters
    )
/*++

Routine Description:

    Power/PnP entry point for the miniport.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    ControlType - The PnP operation to be performed
    Parameters - Variable depending upon ControlType. 

Return Value:

    ScsiAdapterControlSuccess

--*/
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
   
    DebugPrint((1,
                "I2oAdapterControl: ControlType (%x) for (%x)\n",
                ControlType,
                DeviceExtension));
    //
    // Determine what the port-driver wants.
    // 
    switch (ControlType) {
        case ScsiQuerySupportedControlTypes: {
            PSCSI_SUPPORTED_CONTROL_TYPE_LIST supportedList = Parameters;

            //
            // Indicate support for this type + Stop and Restart.
            // 
            supportedList->SupportedTypeList[ScsiStopAdapter] = TRUE;
            supportedList->SupportedTypeList[ScsiRestartAdapter] = TRUE;
            supportedList->SupportedTypeList[ScsiQuerySupportedControlTypes] = TRUE;
            
            break;
        }    
        case ScsiStopAdapter: {
            ULONG intControl;                                             

            DebugPrint((1,
                        "I2OAdapterControl ScsiStopAdapter: Entry\n"));

            //
            // Get the TCL parameter from the IOP.  This is needed to
            // properly preserve boot context for the IOP across power
            // transitions and resets.
            //
            I2OGetSetTCLParam(deviceExtension,TRUE);

            //
            // Need to do a SysQ, disable ints
            //
            if (!I2OSysQuiesce(deviceExtension)) {
                DebugPrint((0,
                            "I2OAdapterControl: SysQuiesce failed\n"));
            }

            deviceExtension->AdapterFlags |= AFLAGS_SHUTDOWN;
            intControl = StorPortReadRegisterUlong(deviceExtension,
                                                   deviceExtension->IntControl);
            intControl |= PCI_INT_DISABLE;

            StorPortWriteRegisterUlong(deviceExtension,
                                       deviceExtension->IntControl,
                                       intControl);

            //
            // Indicate that this one is gone.
            //
            GlobalAdapterCount--;

            break;
        }    
        case ScsiRestartAdapter: {

            PI2O_EXEC_STATUS_GET_REPLY statusReply;
            PVOID                  statusBuffer;
            STOR_PHYSICAL_ADDRESS  statusBufferPhysicalAddress;

            //
            // Enable interrupts on the iop.
            //
            if (StorPortReadRegisterUlong(deviceExtension, deviceExtension->IntControl) != 0) {
                StorPortWriteRegisterUlong(deviceExtension,(deviceExtension->IntControl), 0);
            }

            //
            // We need to determine the IOP State.  In case of Hibernate the status
            // will be I2O_IOP_STATE_OPERATIONAL, and we should need to send a 
            // SysQuiesce before reseting the IOP. Unfortunately, coming out of a power-down
            // condition, the IOP is not in a state where the outbound queues can handle
            // the request. Therefore, we do nothing except reset.
            // In the case of StandBy the 
            // IOP state will be I2O_IOP_STATE_READY and we don't need to send a
            // SysQ since in was done in Stop Adapter.
            //
            statusBuffer  = (PVOID)deviceExtension->ScratchBuffer;
            statusBufferPhysicalAddress = deviceExtension->ScratchBufferPA;

            if (!(I2OGetStatus(deviceExtension,
                       statusBuffer,
                       statusBufferPhysicalAddress))) {
                
               DebugPrint((0,
                          "I2OAdapterControl: Couldn't get status\n"));
            } else {
                statusReply = statusBuffer;
            }

            //
            // Reset the IOP.
            //
            if(I2OResetIop(deviceExtension)) {
                DebugPrint((1,
                            "I2OAdapterControl: I2OResetIOP done\n"));
            }


            //
            // Re-run HwInitialize. This will bring the IOP up to an operational state.
            // 
            if (!I2OInitialize(deviceExtension)) {
                DebugPrint((0,
                            "I2OAdapterControl: I2OInit failed\n"));
            } else {

                //
                // Restore the TCL parameter to the IOP.  This is needed to
                // properly preserve boot context for the IOP across power
                // transitions and resets.
                //
                I2OGetSetTCLParam(deviceExtension,FALSE);

                //
                // Send UTIL_SET_PARAMS message to tells the ISM to go to GUI mode
                //
                I2OSetSpecialParams(deviceExtension,
                                    I2O_RAID_COMMON_CONFIG_GROUP_NO,
                                    I2O_GUI_MODE_FIELD_IDX,
                                    2);

                //
                // Adapter ordinal is still set, but indicate the
                // new one is here.
                //
                GlobalAdapterCount++;
                deviceExtension->AdapterFlags &= ~AFLAGS_SHUTDOWN;
               
                //
                // If being verbose, dump the LCT to the debugger.
                //
                DebugPrint((1,
                            "I2OAdapterControl: FINAL LOCAL DEVICE TABLE\n"));
                DumpTables(deviceExtension);

                //
                // Inform storport that it should re-enumerate, in case a device
                // has gone missing, or been added during the hibernate.
                //
                StorPortNotification(BusChangeDetected,
                                     deviceExtension,
                                     0);
            }    
            break;
        }
        default:
            DebugPrint((0,
                       "I2OAdapterControl: Unhandled Control Type 0x%x\n",
                        ControlType));


            break;
    }    

    return ScsiAdapterControlSuccess;
}    


ULONG
I2OGetCallBackCompletion(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG Wait
    )
/*++

Routine Description:

    This routine handles CallBacks while Interrupts are disabled, by polling
    the ISR directly. 
    
Arguments:

    DeviceExtension - The miniport's HwDeviceExtension
    Wait - Number of stall iterations to perform.
    
Return Value:

    The value of the Completion context or 0, if the CallBack was not invoked by
    the ISR

--*/
{
    ULONG i;
    ULONG completionValue = 0;
    
    //
    // Loop for the number of times that the caller requests.
    // 
    for (i = 0; i < Wait; i++) {

        //
        // Stall before invoking the ISR directly.
        // 
        StorPortStallExecution(1000);
        
        if (I2OInterrupt(DeviceExtension)) {

            //
            // The callback routine has been called, and
            // put status in the deviceExtension.
            //
            completionValue = DeviceExtension->CallBackComplete;
            
            //
            // If the callback has been invoked, then return the value
            // to the caller. 
            // NOTE: Should Log an error if the ISR returned true
            // but the CallBack wasn't invoked.
            //
            if (completionValue) {

                break;
            }
        }
    }
    
    return completionValue;
}
            

VOID
I2OInitCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME MessageFrame,
    IN PVOID              CallbackContext
    )
/*++

Routine Description:

    This is the callback routine for many of the initialization messages.
    Sets the completion flag in the callback context to alert the waiting
    thread that the request has completed.

Arguments:
    MessageFrame - The reply frame for the completing request
    CallbackContext - The deviceExtension of the IOP that sent the request
    
Return Value:

--*/
{

    PI2O_SINGLE_REPLY_MESSAGE_FRAME msg = (PI2O_SINGLE_REPLY_MESSAGE_FRAME )MessageFrame;
    PIOP_CALLBACK_OBJECT callbackObject;
    POINTER64_TO_U32  pointer64ToU32;
    PDEVICE_EXTENSION deviceExtension = CallbackContext;
    ULONG    flags;
    PBOOLEAN callBackNotify;

    //
    // Extract the callback object from the reply
    //
    pointer64ToU32.u.LowPart  = msg->StdMessageFrame.InitiatorContext;
    pointer64ToU32.u.HighPart = msg->TransactionContext;
    callbackObject            = pointer64ToU32.Pointer;

    if ((flags = MessageFrame->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {

        DebugPrint((0,
                   "I2OInitCallBack: Failure. Msg Flags - %x\n",
                   flags));
        return;
    }

    if (MessageFrame->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {

        DebugPrint((0,
                   "I2OInitCallBack: Reply status not success - %x\n",
                   MessageFrame->BsaReplyFrame.ReqStatus));
        return;
    }

    //
    // No data transfer, just set the completion flag.
    // 
    callBackNotify = callbackObject->Context;
    *callBackNotify = ((MessageFrame->BsaReplyFrame.ReqStatus << 8) | 1);

    
    return;
}


VOID
I2OAsyncClaimCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME MessageFrame,
    IN PVOID              CallbackContext
    )
/*++

Routine Description:
    
    This is the callback for all claims that happen outside of the init. code.
    Whenever a new device is hotplugged/or created via the admin utility.

Arguments:
    MessageFrame - The reply frame for the completing request
    CallbackContext - The deviceExtension of the IOP that sent the request
    
Return Value:

--*/
{

    PI2O_SINGLE_REPLY_MESSAGE_FRAME msg = (PI2O_SINGLE_REPLY_MESSAGE_FRAME )MessageFrame;
    PIOP_CALLBACK_OBJECT callbackObject;
    POINTER64_TO_U32  pointer64ToU32;
    PDEVICE_EXTENSION deviceExtension = CallbackContext;
    ULONG flags;
    ULONG id;
    ULONG i;
    BOOLEAN error = FALSE;
    BOOLEAN moreClaims = FALSE;

    pointer64ToU32.u.LowPart  = msg->StdMessageFrame.InitiatorContext;
    pointer64ToU32.u.HighPart = msg->TransactionContext;
    callbackObject            = pointer64ToU32.Pointer;

    //
    // ID of the first device needing claimed from the Claim call.
    // after proc'ing this one we need to go to the next one
    //
    id=deviceExtension->CallBackComplete;

    if ((flags = MessageFrame->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {

        DebugPrint((0,
                   "I2OAsyncClaimCallBack: Failure. Msg Flags - %x\n",
                   flags));
        error = TRUE;
    }

    if (MessageFrame->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {

        DebugPrint((0,
                   "I2OAsyncClaimCallBack: Index 0x%x : Reply status not success - %x\n",
                   id,MessageFrame->BsaReplyFrame.ReqStatus));
        error = TRUE;
    }

    if (error) {
        
        //
        // The entry that was built earlier for this device
        // needs to be torn down.
        //
        I2ORemoveSingleDevice(deviceExtension,id);
        
    } else {
        //
        // Indicate that the device has been successfully claimed.
        //
        deviceExtension->DeviceInfo[id].DeviceFlags &= ~(DFLAGS_NEED_CLAIM |
                                                         DFLAGS_CLAIM_ISSUED);
        deviceExtension->DeviceInfo[id].DeviceFlags |= DFLAGS_CLAIMED; 

        //
        // Now that the device has been claimed, mark it as being 
        // available for the OS to use.  Also mark that this device
        // needs event registration.
        //
        deviceExtension->DeviceInfo[id].DeviceFlags |= (DFLAGS_OS_CAN_CLAIM | DFLAGS_NEED_REGISTER);
    }

    DebugPrint((1,
               "I2OAsyncClaimCallBack: Callback for index (%x)\n",
               id));

    //
    // check to see if there are more claims oustanding
    //
    for (i = 0; i < IOP_MAXIMUM_TARGETS; i++) {
        if (deviceExtension->DeviceInfo[i].DeviceFlags & DFLAGS_NEED_CLAIM) {
            moreClaims = TRUE;
        }
    }

    if (moreClaims) {
        
        //
        // There are more claims to process, don't rescan and LCTnotify just yet
        //
        DebugPrint((1,
                    "I2OAsyncClaimCallBack: index=(%x), moreClaims=TRUE, call for next Claim\n",
                    id));

        I2OAsyncClaimDevice(deviceExtension);
    
    } else {
        
        //
        // Register all of the claimed devices for state change events and 
        // check each device's actual device state, altering the miniport's 
        // internal state to match.
        //
        I2ORegisterForEvents(deviceExtension, I2OMP_EVENT_MASK);    
        
        //
        // Get a rescan from StorPort.
        //      
        DebugPrint((1,
                   "I2OAsyncClaimCallBack: index=(%x), moreClaims=FALSE, now rescan\n",
                   id));

        //
        // If requests are being blocked, allow them through.
        //
        if (deviceExtension->AdapterFlags & AFLAGS_LOCKED) {

            deviceExtension->AdapterFlags &= ~AFLAGS_LOCKED;
            
            //
            // Tell storport that requests are OK.
            // 
            StorPortResume(deviceExtension);
        }    
        
        StorPortNotification(BusChangeDetected,
                             deviceExtension,
                             0);
        //
        // Reissue
        //
        I2OIssueLctNotify(deviceExtension,
                              &deviceExtension->LctCallback,
                              deviceExtension->CurrentChangeIndicator);


    }
    return;

}        


BOOLEAN
I2OInterrupt(
    IN PVOID DeviceExtension
    )
/*++

Routine Description:
    This is the Interrupt Service routine.  It will loop
    through and handle all of the reply messages that are
    currently in the outbound queue and will call each
    requests completion routine.  It will also handle
    returning failed messages back to the inbound queue.

Arguments:
    DeviceExtension - The IOP that is interrupting
    
Return Value:
    TRUE - The interrupt was handled by this ISR
    FALSE - The interrupt was not handled by this ISR

--*/
{
    PDEVICE_EXTENSION  deviceExtension = DeviceExtension;
    PI2O_MESSAGE_FRAME messageFrame;
    ULONG              messageFramePhysAddr;
    ULONG              prevMessageFramePhysAddr;
    CALLBACK           callBack;
    PCALLBACK_OBJECT   callObject;
    PULONG             transactionContext;
    POINTER64_TO_U32   pointer64ToU32;
    PI2O_FAILURE_REPLY_MESSAGE_FRAME errorMessage;
    PI2O_UTIL_NOP_MESSAGE origMessage = NULL;
    I2O_UTIL_NOP_MESSAGE  origMessageBuffer;
    BOOLEAN               isInterruptServiced = FALSE;

    messageFramePhysAddr = StorPortReadRegisterUlong(deviceExtension,
                                                     deviceExtension->MessageReply);

    //
    // Code derived from original source for early 960rd/rd stuff.
    // Early hardware could mistakenly return an empty queue.
    //
    if (messageFramePhysAddr == (ULONG)-1) {
        messageFramePhysAddr = StorPortReadRegisterUlong(deviceExtension,
                                                         deviceExtension->MessageReply);
    }

    //
    // Read the IOP's outbound FIFO to get reply message frames until the
    // FIFO is empty.  A FIFO entry contains the physical address (or logical
    // address if map registers are used) to a reply message frame that has
    // been written to host memory by the IOP.
    //
    prevMessageFramePhysAddr = messageFramePhysAddr;

    while (messageFramePhysAddr != (ULONG)-1) {

        //
        // The ISR has read a message frame address from the IOP's outbound FIFO.
        // Therefore, we now treat the interrupt as being serviced by the ISR.
        //
        isInterruptServiced = TRUE;

        //
        // Convert the Physical Address to a Virtual Message Frame pointer.
        //
        messageFrame = (PI2O_MESSAGE_FRAME)(deviceExtension->OffsetAddress +
                                            messageFramePhysAddr);

        //
        // Make sure it's a reply.
        //
        if ((messageFrame->MsgFlags & I2O_MESSAGE_FLAGS_REPLY)) {

            //
            //  Check if this is a failed reply message.
            //
            if (messageFrame->MsgFlags & I2O_MESSAGE_FLAGS_FAIL) {

                errorMessage = (PI2O_FAILURE_REPLY_MESSAGE_FRAME)messageFrame;

                DebugPrint((0,
                            "I2OInterrupt (%x): Failed Message Frame returned\n"
                            "              FailureCode  = 0x%x\n"
                            "              OrigFunction = 0x%x\n",
                            DeviceExtension,
                            errorMessage->FailureCode,
                            errorMessage->StdMessageFrame.Function));

                //
                // Check if the PreservedMFA to the original request message seems valid.
                //
                if ((errorMessage->PreservedMFA.LowPart > MESSAGEFRAME_RELEASE) &&
                    (errorMessage->PreservedMFA.LowPart < deviceExtension->PhysicalMemorySize)) {

                    //
                    // Copy the contents of the original request message frame which is stored in
                    // MMIO space.  Do not dereference the message frame directly with a pointer.
                    //
                    origMessage = (PVOID)((PUCHAR)deviceExtension->MappedAddress +
                                                  errorMessage->PreservedMFA.LowPart);

                    StorPortReadRegisterBufferUlong(deviceExtension,
                                                    (PULONG)origMessage,
                                                    (PULONG)&origMessageBuffer,
                                                    sizeof(origMessageBuffer) / sizeof(ULONG));

                    //
                    // Get the original InitiatorContext if not provided in the failed reply message.
                    //
                    if (errorMessage->StdMessageFrame.InitiatorContext == 0) {
                        errorMessage->StdMessageFrame.InitiatorContext =
                                                      origMessageBuffer.StdMessageFrame.InitiatorContext;
                    }

                } else {
                    DebugPrint((0,
                                "I2OInterrupt(%x): Failed Frame with bad PreservedMFA %x.\n",
                                DeviceExtension,
                                errorMessage->PreservedMFA.LowPart));
                }
            }

            //
            // The InitiatorContext field contains the low 32 bits of the callback object
            // pointer and the TransactionContext (for NT64) contains the high 32 bits.
            // The TransactionContext is appended after the InitiatorContext which can be
            // either 32 bits or 64 bits in size (MsgFlags determines the size).
            //
            if (messageFrame->MsgFlags & I2O_MESSAGE_FLAGS_64BIT_CONTEXT) {
                transactionContext = (PULONG)((PUCHAR)&messageFrame->InitiatorContext + sizeof(U64));
            } else {
                transactionContext = (PULONG)((PUCHAR)&messageFrame->InitiatorContext + sizeof(U32));
            }

            pointer64ToU32.u.LowPart  = messageFrame->InitiatorContext;
            pointer64ToU32.u.HighPart = *transactionContext;

            callObject = pointer64ToU32.Pointer;

            DebugPrint((2,
                        "I2OInterrupt: messageFrame 0x%x, callObject 0x%x\n",
                        messageFrame,
                        callObject));

            //
            // Check if a callObject has been returned in the reply message.
            //
            if (callObject) {

                //
                // Make sure we have a valid callback object.
                //
                if (callObject->Signature == CALLBACK_OBJECT_SIG) {

                    callBack.Function = callObject->CallbackAddress;

                    //
                    // Make the Call back to the specific handler for this message.
                    //
                    (callBack.Function)(messageFrame,
                                        callObject->CallbackContext);

                } else {
                    DebugPrint((0,
                                "I2OInterrupt: Invalid Signature 0x%x in callObject.\n",
                                callObject->Signature));
                }

            } else {
                DebugPrint((0,
                            "I2OInterrupt: InitiatorContext NULL in reply...No callObject.\n"));
            }

            //
            // Check if we encountered a failed reply message where we now need to free
            // the original request message frame (specified by PreservedMFA).
            //
            if (origMessage) {

                DebugPrint((0,
                            "I2OInterrupt: Free original failed frame for PreservedMFA 0x%x.\n",
                            errorMessage->PreservedMFA.LowPart));

                //
                // Free the preserved request message frame back to the IOP.  This is done by
                // changing the original message to I2O_UTIL_NOP and posting it to the IOP.
                //
                origMessageBuffer.StdMessageFrame.VersionOffset    = I2O_VERSION_11;
                origMessageBuffer.StdMessageFrame.MsgFlags         = 0;
                origMessageBuffer.StdMessageFrame.MessageSize = (sizeof(I2O_UTIL_NOP_MESSAGE) >> 2);
                origMessageBuffer.StdMessageFrame.TargetAddress    = I2O_IOP_TID;
                origMessageBuffer.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
                origMessageBuffer.StdMessageFrame.Function         = I2O_UTIL_NOP;
                origMessageBuffer.StdMessageFrame.InitiatorContext = 0;

                StorPortWriteRegisterBufferUlong(deviceExtension,
                                                 (PULONG)origMessage,
                                                 (PULONG)&origMessageBuffer,
                                                 sizeof(I2O_UTIL_NOP_MESSAGE) / sizeof(ULONG));

                StorPortWriteRegisterUlong(deviceExtension,
                                           deviceExtension->MessagePost,
                                           errorMessage->PreservedMFA.LowPart);
            }

            //
            // Write to the IOP's outbound FIFO to free the reply message frame.
            //
            StorPortWriteRegisterUlong(deviceExtension,
                                       deviceExtension->MessageRelease,
                                       messageFramePhysAddr);
        } else {
            StorPortWriteRegisterUlong(deviceExtension,
                                       deviceExtension->MessageRelease,
                                       messageFramePhysAddr);
        }

        //
        // Get the next message in the fifo.
        //
        messageFramePhysAddr = StorPortReadRegisterUlong(deviceExtension,
                                                         deviceExtension->MessageReply);

        //
        // Make sure that the IOP does not return a duplicate message frame.
        //
        if (messageFramePhysAddr == prevMessageFramePhysAddr) {
            DebugPrint((0,
                        "I2OInterrupt: Duplicate reply message frames.\n"));
            StorPortLogError(deviceExtension,
                             NULL,
                             0,
                             0,
                             0,
                             SP_INTERNAL_ADAPTER_ERROR,I2OMP_ERROR_DUPLICATEMSG_FAILED);
            break;
        }

        prevMessageFramePhysAddr = messageFramePhysAddr;

    }

    return isInterruptServiced;
}


VOID
I2OEventAckCompletion(
    IN PI2O_UTIL_EVENT_ACK_REPLY Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    The IOP reply to UTIL_EVENT_ACKNOWLEDGE.  This signals the miniport
    that the device has completed the reset.  We are now done with 
    the sequence of BSA_DEVICE_RESET messages.

Arguments:
    Msg - The event acknowledge reply message.
    DeviceContext - The deviceExtension of the IOP
    
Return Value:

--*/
{
    PDEVICE_EXTENSION deviceExtension = DeviceContext;

    DebugPrint((1,
                "I2OEventAckCompletion: Starting Event Indication %x\n",
                Msg->EventIndicator));

    
    if (--deviceExtension->ResetCnt == 0) {
        
        DebugPrint((1, 
                    "I2OEventAckCompletion: calling I2OBSADeviceResetComplete\n"));
        I2OBSADeviceResetComplete(deviceExtension);
    }

    return;
}


VOID
I2OClaimReleaseCallback(
    IN PI2O_SINGLE_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    The IOP reply to UTIL_CLAIM_RELEASE.  This signals the miniport
    that the device has completed the claim release.

Arguments:
    Msg - The event acknowledge reply message.
    DeviceContext - The deviceExtension of the IOP
    
Return Value:

--*/
{
    PDEVICE_EXTENSION deviceExtension = DeviceContext;

    DebugPrint((1, "I2OClaimReleaseCallback:\n"));
}


VOID 
I2OBSADeviceResetTimer(
    IN PVOID DeviceExtension
    )
/*++

Routine Description:

    This is the safety timer that is used while BSA_DEVICE_RESET is going on.
    It will run every second while the device reset sequence is in process
    and if after 60 seconds, all of the device resets have not completed,
    it will reset the IOP and force a reinitialization.  

Arguments:
    DeviceExtension
    
Return Value:

--*/
{
    PDEVICE_EXTENSION  deviceExtension = DeviceExtension;

    //
    // Make sure that the miniport is still in the middle of the reset state.
    // There is a subtle race condition between shutting the reset timer off
    // in BSADeviceResetComplete and this routine running again before it can 
    // be shut off.   
    //
    if (deviceExtension->AdapterFlags & AFLAGS_DEVICE_RESET) {
        deviceExtension->ResetTimer++;

        DebugPrint((0, 
                    "I2OBSADeviceResetTimer(%x): Start %d\n",
                    deviceExtension,
                    deviceExtension->ResetTimer));

        //
        // if not completed in 60 seconds, then some sort of error in one of the callback's.
        // 
        if (deviceExtension->ResetTimer >= 100) {

            DebugPrint((0,
                        "I2OBSADeviceResetTimer: Timed Out: calling I2OBSADeviceResetComplete\n"));
            //
            // BSA_DEVICE_RESET took too long
            //
            I2OBSADeviceResetComplete(deviceExtension);

            //
            // Something went wrong.  Log an error, reset and re-initialize the IOP 
            // as a last ditch effort to regain control.
            //
            StorPortLogError(DeviceExtension,
                             NULL,
                             0,
                             0,
                             0,
                             SP_INTERNAL_ADAPTER_ERROR,I2OMP_ERROR_DEVICERESET_TIMEOUT);

            //
            // Get the TCL parameter from the IOP.  This is needed to
            // properly preserve boot context for the IOP across power
            // transitions and resets.
            //
            I2OGetSetTCLParam(deviceExtension,TRUE);

            if (!I2OSysQuiesce(deviceExtension)) {
                DebugPrint((0,
                            "I2OBSADeviceResetTimer: SysQuiesce failed\n"));
            }

            I2OResetIop(deviceExtension);

            if (!I2OInitialize(deviceExtension)) {
                DebugPrint((0,
                            "I2OBSADeviceResetTimer: I2OInit failed\n"));
            }    

            //
            // Restore the TCL parameter to the IOP.  This is needed to
            // properly preserve boot context for the IOP across power
            // transitions and resets.
            //
            I2OGetSetTCLParam(deviceExtension,FALSE);
        
            //
            // Send UTIL_SET_PARAMS message to tells the ISM to go to GUI mode
            //
            I2OSetSpecialParams(deviceExtension,
                                I2O_RAID_COMMON_CONFIG_GROUP_NO,
                                I2O_GUI_MODE_FIELD_IDX,
                                2);
        } else {

            // 
            // reset/resume timer
            //
            StorPortNotification(RequestTimerCall,
                             deviceExtension,
                             I2OBSADeviceResetTimer,
                             BSA_RESET_TIMER);
        }
    }
    return;
}


VOID
I2OBSADeviceResetComplete(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This is the final BSA_DEVICE_RESET cleanup that occurs when either;
    (a) all processing from BSA_DEVICE_RESET messages are complete or
    (b) if something went wrong during BSA_DEVICE_RESET, and the resets
    were timed out.

Arguments:
    deviceExtension
    
Return Value:

--*/
{

    DebugPrint((0,
                "I2OBSADeviceResetComplete on (%x)\n",
                DeviceExtension));

    //
    // resume I/O
    //
    DeviceExtension->AdapterFlags &= ~AFLAGS_DEVICE_RESET;

    //
    // cancel ResetTimer
    //
    StorPortNotification(RequestTimerCall,
                         DeviceExtension,
                         I2OBSADeviceResetTimer,
                         0);

    return;
}


VOID
I2ODeviceEvent(
    IN PI2O_UTIL_EVENT_REGISTER_REPLY Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    This is the callback for when a registered event occurs.  This is the
    state engine for controlling when the miniport: 1)disallows OS visability
    and notifies StorPort that the bus topography has changed 
    (cleanly removing the device from the OS) or 2)re-enables the OS visability
    and notifies StorPort that the bus topography has changed 
    (cleanly adding a device to the OS) or 3)removes a device as above
    and also sends a UtilClaimRelease message to allow the IOP to remove the
    device (the miniport will get an LCT notify in this case).

    To keep track of each devices current event state the miniport internally
    maintains a DeviceState in the device info for each device.  Here are the 
    event state transitions that the miniport supports and the I2O device 
    state event that will cause each transition:

    Current State  | Next State        | I2O Event
    ---------------+-------------------+-------------------------------------
    DSTATE_NORMAL  | DSTATE_FAULTED    | I2O_EVENT_STATE_CHANGE_FAULTED
    DSTATE_NORMAL  | DSTATE_NORECOVERY | I2O_EVENT_STATE_CHANGE_NA_NO_RECOVER
    DSTATE_FAULTED | DSTATE_NORMAL     | I2O_EVENT_STATE_CHANGE_NORMAL
    DSTATE_FAULTED | DSTATE_NORECOVERY | I2O_EVENT_STATE_CHANGE_NA_NO_RECOVER
    
    Note that the EventState also contains some other flags used to control
    when the state transitions are done.
    DSTATE_COMPLETE - When this flag is set, all IO for the current device
        has been completed and all steps have been taken to complete this 
        transition (i.e. removing the device, notifying StorPort, claim release).  
        This will be OR'd in with DSTATE_FAULTED or DSTATE_NORECOVERY to signify 
        that it is safe to transition to another state.  Note that it is 
        possible to transition from the DSTATE_FAULTED state to the 
        DTATE_NORECOVERY state without completing all IO.  In this case the
        miniport will not issue the claim release until all IO have completed.

Arguments:

    Msg - the reply message from the IOP
    DeviceContext - The deviceExtension for this IOP
    
Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION deviceExtension = DeviceContext;
    I2O_UTIL_EVENT_ACK_MESSAGE ackMsg;
    POINTER64_TO_U32 pointer64ToU32;
    PI2O_DEVICE_INFO deviceInfo = NULL;
    PI2O_DEVICE_INFO currentDeviceInfo = NULL;
    ULONG i;

    DebugPrint((1,
               "I2ODeviceEvent: EventIndicator %x Tid 0x%x \n",
               Msg->EventIndicator,Msg->StdMessageFrame.TargetAddress));
    //
    // Check for any error status.
    //
    if (Msg->StdMessageFrame.MsgFlags & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((0,
               "I2ODeviceEvent (%x): Message FAILURE \n",
               deviceExtension));
    } else {

        //
        // TODO: Clean this up.
        //
        // Use the TID from the reply message to find the device
        // info in the deviceExtension.
        //
        for (i = 0; i < IOP_MAXIMUM_TARGETS; i++) {
            currentDeviceInfo = &deviceExtension->DeviceInfo[i];
            if ((currentDeviceInfo->Lct.LocalTID == Msg->StdMessageFrame.TargetAddress) &&
                (currentDeviceInfo->DeviceFlags & DFLAGS_CLAIMED)) {
                
                deviceInfo = currentDeviceInfo;
                break;
            }
        }

        if (deviceInfo) {

            //
            // Decode the Event.
            //
            switch (Msg->EventIndicator) {
                case I2O_EVENT_IND_DEVICE_RESET: {
                    //
                    // This is currently a stub...
                    //
                    DebugPrint((1, "I2ODeviceEvent: I2O_EVENT_IND_DEVICE_RESET\n"));
                    break;
                }
                case I2O_EVENT_IND_STATE_CHANGE: {
                    DebugPrint((1,
                                "I2ODeviceEvent: Tid 0x%x Current State 0x%x\n",
                                deviceInfo->Lct.LocalTID,
                                deviceInfo->DeviceState));
                    //
                    // Decode the state transition.
                    //
                    switch (Msg->EventData[0] & 0xff) {
                        case I2O_EVENT_STATE_CHANGE_NORMAL: {
                            DebugPrint((1, "I2ODeviceEvent: I2O_EVENT_STATE_CHANGE_NORMAL\n"));
                            //
                            // This state will occur when a rebuilding drive
                            // completes or a faulted drive is marked normal
                            // by the user.  We are only concerned with the 
                            // latter case..
                            //
                            if (deviceInfo->DeviceState & DSTATE_FAULTED) {
                                //
                                // This device was faulted and is nolonger visible
                                // to the OS. Make it visible to the OS by turning on
                                // DFLAGS_OS_CAN_CLAIM and notifying StorPort of the change.
                                //
                                deviceInfo->DeviceFlags |= DFLAGS_OS_CAN_CLAIM;
                                DebugPrint((1,
                                            "I2O_EVENT_STATE_CHANGE_NORMAL from Faulted.\n"));

                                //
                                // Tell storport to re-enum.
                                // 
                                StorPortNotification(BusChangeDetected,deviceExtension,0);
                            }

                            deviceInfo->DeviceState = DSTATE_NORMAL;
                            break;
                        }
                        case I2O_EVENT_STATE_CHANGE_FAULTED: {
                                                                 
                            DebugPrint((1,
                                        "I2ODeviceEvent: I2O_EVENT_STATE_CHANGE_FAULTED\n"));

                            //
                            // See if all outstanding IO has completed for this device
                            // that has serious problems.  If so then remove the device
                            // from our map and tell StorPort to re-enumerate.
                            //
                            if (deviceInfo->OutstandingIOCount == 0) {
                                DebugPrint((1,
                                            "I2ODeviceEvent: No outstanding IO, Removing.\n"));
                                
                                deviceInfo->DeviceState = DSTATE_FAULTED | DSTATE_COMPLETE;

                            } else {
                                DebugPrint((1,
                                            "I2ODeviceEvent: Waiting for 0x%x IO to complete.\n",
                                            deviceInfo->OutstandingIOCount));
                                
                                deviceInfo->DeviceState = DSTATE_FAULTED;
                            }
                            
                            //
                            // This device has a serious problem and needs to be taken
                            // off-line.  Clear the DFLAGS_OS_CAN_CLAIM flag 
                            // and notify StorPort of the change. 
                            //
                            deviceInfo->DeviceFlags &= ~(DFLAGS_OS_CAN_CLAIM);

                            //
                            // Tell storport to re-enum.
                            // 
                            StorPortNotification(BusChangeDetected,deviceExtension,0);

                            break;
                        }
                        case I2O_EVENT_STATE_CHANGE_NA_NO_RECOVER: {
                            DebugPrint((1,
                                        "I2ODeviceEvent: I2O_EVENT_STATE_CHANGE_NA_NO_RECOVER\n"));

                            if (deviceInfo->DeviceState & DSTATE_NORMAL) {
                                //
                                // The device was in the normal state and now is being 
                                // deleted.  Clear the DFLAGS_OS_CAN_CLAIM flag 
                                // and notify StorPort of the change. 
                                //
                                DebugPrint((1,
                                            "I2ODeviceEvent: Removing.\n"));
                                
                                deviceInfo->DeviceFlags &= ~(DFLAGS_OS_CAN_CLAIM);

                                //
                                // Re-enum necessary.
                                //
                                StorPortNotification(BusChangeDetected,deviceExtension,0);
                            }

                            //
                            // See if all outstanding IO has completed for this device
                            // that is being deleted.  If so then release the claim on the
                            // device and remove it from our LCT.
                            //
                            if (deviceInfo->OutstandingIOCount == 0) {
                                //
                                // All of the IO for this device has been completed.
                                // Send the Claim Release so that the IOP can cleanup the
                                // device.  The miniport will receive an LCT notify when
                                // the device has been removed and will complete internal
                                // cleanup of the device.
                                // 
                                DebugPrint((1, "I2ODeviceEvent: Releasing Claim...\n"));
                                I2OClaimRelease(deviceExtension,deviceInfo->Lct.LocalTID);
                                deviceInfo->DeviceState = DSTATE_NORECOVERY | DSTATE_COMPLETE;
                            } else {
                                //
                                // IO is still outstanding.  Let I2OCompleteRequest handle the
                                // cleanup of this device once the IO count is zero.  
                                //
                                DebugPrint((1,
                                            "I2ODeviceEvent: Waiting For 0x%x IO to complete.\n",
                                            deviceInfo->OutstandingIOCount));

                                deviceInfo->DeviceState = DSTATE_NORECOVERY;
                            }
                        break;
                        }
                    default:
                        DebugPrint((1,
                                    "I2ODeviceEvent: Ignored Event 0x%x\n",
                                    Msg->EventData[0] & 0xff));
                    break;
                    }
                }
            }           
        }
    }
    return;
}


BOOLEAN
I2ORegisterForEvents(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG EventMask
    )
/*++

Routine Description:

    This routine issues the msg to turn on device state event notification.
    This will allow the miniport to detect when volumes has been deleted, 
    failed, or reset.  This routine will send a UTIL EventRegistration 
    request followed by a UTIL ParamsGet message to get the initial state
    of each device.

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    EventMask - The mask of events the miniport is registering for.
    
Return Value:

    TRUE - The message was sent correctly.
    FALSE - The message was not sent.

--*/
{

    I2O_UTIL_EVENT_REGISTER_MESSAGE msg;
    PIOP_CALLBACK_OBJECT            callbackObject;
    POINTER64_TO_U32                pointer64ToU32;
    PI2O_DEVICE_INFO                deviceInfo;
    ULONG                           entry;
    ULONG                           msgsize;
    PI2O_PARAM_SCALAR_OPERATION     scalar;
    BOOLEAN                         status = FALSE;
    ULONG                           i;
    union {
        I2O_UTIL_PARAMS_GET_MESSAGE m;
        U32 blk[128/ sizeof(U32)];
    } getMsg; 

    //
    // Zero the Message Frame.
    //
    memset(&msg, 0, sizeof(I2O_UTIL_EVENT_REGISTER_MESSAGE));

    for (entry = 0; entry < IOP_MAXIMUM_TARGETS; entry++) {

        //
        // TODO: Clean this up by splitting things into other functions.
        //
        deviceInfo = &DeviceExtension->DeviceInfo[entry];
        if ((deviceInfo->DeviceFlags & DFLAGS_CLAIMED) &&
            (deviceInfo->DeviceFlags & DFLAGS_NEED_REGISTER)) {
            //
            // Mark that this device no longer need event registration.
            //
            deviceInfo->DeviceFlags &= ~DFLAGS_NEED_REGISTER;

            DebugPrint((1, "I2ORegisterForEvents: entry(%d) LCTEntry(0x%x)\n",
                        entry,deviceInfo->Lct.LocalTID));

            //
            // Format the UtilEventRegister request message.
            //
            msg.StdMessageFrame.VersionOffset = I2O_VERSION_11;
            msg.StdMessageFrame.MsgFlags = 0;
            msg.StdMessageFrame.MessageSize = sizeof(I2O_UTIL_EVENT_REGISTER_MESSAGE)/4;
            msg.StdMessageFrame.TargetAddress = deviceInfo->Lct.LocalTID;
            msg.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
            msg.StdMessageFrame.Function = I2O_UTIL_EVENT_REGISTER;
            msg.StdMessageFrame.InitiatorContext = 0;

            msg.EventMask = EventMask;

            //
            // Setup the callback.
            //
            callbackObject = &DeviceExtension->EventCallback;
            pointer64ToU32.Pointer64 = 0;
            pointer64ToU32.Pointer   = callbackObject;

            msg.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
            msg.TransactionContext = pointer64ToU32.u.HighPart;

            //
            // Send the message. No reply will occur.
            //
            if (!I2OSendMessage(DeviceExtension,
                                (PI2O_MESSAGE_FRAME)&msg)) {
                DebugPrint((0,
                           "I2ORegisterForEvents: Failed sending message for Tid 0x%x\n",
                           deviceInfo->Lct.LocalTID));
                return FALSE;
            }

            //
            // Since the miniport can not maintain the state of a device 
            // accross power cycles it is necessary to retrieve the state
            // from the IOP each time we register for events.
            //
            DebugPrint((1,
                       "Sending SetParam to determine current device state for Tid 0x%x\n",
                        deviceInfo->Lct.LocalTID));

            //
            // Setup context and callback info.
            //
            callbackObject = &DeviceExtension->GetDeviceStateCallback;
            pointer64ToU32.Pointer64 = 0;
            pointer64ToU32.Pointer = callbackObject;

            //
            // Build the GetParams message to retrieve the current state
            // of the device.  The completion routine will handle any transitions
            // that need to be made in the device state.
            //
            msgsize = (sizeof(I2O_UTIL_PARAMS_SET_MESSAGE) -
                                           sizeof(I2O_SG_ELEMENT) +
                                           sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT) +
                                           sizeof(I2O_PARAM_SCALAR_OPERATION)) +
                                           sizeof(ULONG);

            memset(&getMsg,0, sizeof(getMsg));

            //
            // Setup the message header.
            //
            getMsg.m.StdMessageFrame.VersionOffset    = I2O_VERSION_11 | I2O_PARAMS_SGL_VER_OFFSET;
            getMsg.m.StdMessageFrame.MsgFlags         = 0;
            getMsg.m.StdMessageFrame.TargetAddress    = deviceInfo->Lct.LocalTID; 
            getMsg.m.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
            getMsg.m.StdMessageFrame.Function = I2O_UTIL_PARAMS_GET;
            getMsg.m.StdMessageFrame.InitiatorContext = 0;
            getMsg.m.StdMessageFrame.MessageSize = (USHORT)(msgsize >> 2);
            getMsg.m.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
            getMsg.m.TransactionContext = pointer64ToU32.u.HighPart;

            getMsg.m.SGL.u.ImmediateData.FlagsCount.Count = sizeof(I2O_PARAM_SCALAR_OPERATION) + sizeof(ULONG);
            getMsg.m.SGL.u.ImmediateData.FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                                   I2O_SGL_FLAGS_END_OF_BUFFER |
                                                   I2O_SGL_FLAGS_IMMEDIATE_DATA_ELEMENT;

            scalar = (PI2O_PARAM_SCALAR_OPERATION)((PUCHAR)&getMsg.m.SGL +
                                               sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT));
            scalar->OpList.OperationCount   = 1;
            scalar->OpBlock.GroupNumber     = I2O_RAID_DEVICE_CONFIG_GROUP_NO;
            scalar->OpBlock.FieldCount      = (USHORT) 1;
            scalar->OpBlock.FieldIdx[0]     = I2O_MAP_STATE_FIELD_IDX;
            scalar->OpBlock.Operation       = I2O_PARAMS_OPERATION_FIELD_GET;

            //
            // Send the message. No reply will occur.
            //
            if (!I2OSendMessage(DeviceExtension,
                                (PI2O_MESSAGE_FRAME)&getMsg)) {
                DebugPrint((0,
                           "I2ORegisterForEvents: Failed sending message for Tid 0x%x\n",
                           deviceInfo->Lct.LocalTID));
                return FALSE;
            }
        }
    }
    return TRUE;
}


VOID
I2OGetDeviceStateCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    This is the call back function for I2ORegisterForEvents when requesting
    a get param to determine the devices current state.  It reads the value 
    from the reply message buffer, and update the devices state if needed.

Arguments:

    Msg - The reply message frame.
    DeviceContext - The deviceExtension for this IOP
    
Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT callbackObject;
    POINTER64_TO_U32 pointer64ToU32;
    PI2O_DEVICE_INFO deviceInfo = NULL;
    ULONG  flags;
    PULONG msgBuffer;
    ULONG  i;
    ULONG  deviceState;
    ULONG  notify = 0;
    BOOLEAN found = FALSE;

    DebugPrint((1,
                "I2OGetDeviceStateCallback: Entering for Tid 0x%x\n",
                    Msg->BsaReplyFrame.StdMessageFrame.TargetAddress));

    if ((flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((0,
                   "I2OGetDeviceStateCallback: Failed MsgFlags\n"));
        return;
    }

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer.
    //
    pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
    pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;
    callbackObject = pointer64ToU32.Pointer;

    if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
        DebugPrint((0,
                   "I2OGetDeviceStateCallback: Failed ReqStatus\n"));
        //
        // If the Get Params failed then assume the device is in normal
        // state.  Since passthru devices won't support this parameter
        // it will be possible for it to fail.
        //
        deviceState = 0;
        
    } else {
        //
        // Get a pointer to the data.  Save off the device state parameter.
        //
        msgBuffer = (PULONG)&Msg->TransferCount;
        msgBuffer++;
        msgBuffer++;
        deviceState = *msgBuffer;
    }

    //
    // Use the TID from the reply message to find the device
    // info in the deviceExtension.
    //
    for (i = 0; i < IOP_MAXIMUM_TARGETS; i++) {

        //
        // Get the device.
        //
        deviceInfo = &deviceExtension->DeviceInfo[i];
        
        if ((deviceInfo->Lct.LocalTID == Msg->BsaReplyFrame.StdMessageFrame.TargetAddress) &&
            (deviceInfo->DeviceFlags & DFLAGS_CLAIMED)) {

            found = TRUE;
            break;
        }
    }

    if (found) {
        DebugPrint((1,
                "I2OGetDeviceStateCallback: Current DeviceState 0x%x New State 0x%x\n",
                deviceInfo->DeviceState,
                deviceState));

        //
        // Decode the device state.
        //
        switch (deviceState) {
            case 0: {
                        
                //
                // The device is in Normal state.  See if we think it is
                // in the faulted state and recover if it is.
                //
                if (deviceInfo->DeviceState & DSTATE_FAULTED) {

                    //
                    // This device was faulted and has had its assignedId
                    // returned. Make it visible to the OS by setting the
                    // DFLAGS_OS_CAN_CLAIM flag and notifying StorPort of 
                    // the change.
                    //
                    deviceInfo->DeviceFlags |= DFLAGS_OS_CAN_CLAIM;
                    
                    DebugPrint((1,
                                "I2OGetDeviceState: I2O_STATE_CHANGE_NORMAL from Faulted.g\n"));
                    notify = 1;
                }
                deviceInfo->DeviceState = DSTATE_NORMAL;

                break;
            }
            case 3: {
                DebugPrint((1,
                            "I2OGetDeviceStateCallback: I2O_EVENT_STATE_CHANGE_FAULTED\n"));
                
                if (!(deviceInfo->DeviceState & DSTATE_FAULTED)) {
                    
                    //
                    // See if all outstanding IO has completed for this device
                    // that has serious problems.  If so then remove the device
                    // from our map and tell StorPort to re-enumerate.
                    //
                    if (deviceInfo->OutstandingIOCount == 0) {
                        
                        DebugPrint((1,
                                    "I2OGetDeviceStateCallback: No outstanding IO, Removing\n"));
                        
                        //
                        // This device has a serious problem and needs to be taken
                        // off-line.  Clear the DFLAGS_OS_CAN_CLAIM flag 
                        // and notify StorPort of the change. 
                        //
                        deviceInfo->DeviceState = DSTATE_FAULTED | DSTATE_COMPLETE;
                        
                    } else {
                        DebugPrint((1,
                                    "I2OGetDeviceStateCallback: Waiting for 0x%x IO to complete.\n",
                                    deviceInfo->OutstandingIOCount));
                        
                        deviceInfo->DeviceState = DSTATE_FAULTED;
                    }
                    
                    deviceInfo->DeviceFlags &= ~(DFLAGS_OS_CAN_CLAIM);
                    notify = 1;
                }
                break;
            }
        }
    }

    //
    // Notify StorPort of the bus change.
    //
    if (notify) {
        StorPortNotification(BusChangeDetected,
                             deviceExtension,
                             0);
    }
    return;
}


BOOLEAN
I2OGetHRT(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This is used to get the Hardware Resource Table, which informs us of any adapters
    being controller by this IOP.

Arguments:
    DeviceExtension
    
Return Value:
    TRUE - The request completed OK.
    FALSE - The request was not sent or timed out.

--*/
{

    I2O_EXEC_HRT_GET_MESSAGE getHRTMessage;
    PIOP_CALLBACK_OBJECT     callbackObject;
    POINTER64_TO_U32         pointer64ToU32;
    ULONG                    status;
    ULONG                    i;

    callbackObject = &DeviceExtension->InitCallback;
    callbackObject->Context = &DeviceExtension->CallBackComplete;

    DeviceExtension->CallBackComplete = FALSE;

    //
    // Zero out the IOPGetHRT message frame.
    //
    memset(&getHRTMessage, 0, sizeof(I2O_EXEC_HRT_GET_MESSAGE));

    //
    // Setup a IOPGetHRT message to send to the IOP.
    //
    getHRTMessage.StdMessageFrame.VersionOffset = I2O_VERSION_11 | I2O_HRT_GET_SGL_VER_OFFSET;
    getHRTMessage.StdMessageFrame.MessageSize   = (sizeof(I2O_EXEC_HRT_GET_MESSAGE) -
                                                   sizeof(I2O_SG_ELEMENT) +
                                                   sizeof(I2O_SGE_SIMPLE_ELEMENT)) >> 2;
    getHRTMessage.StdMessageFrame.Function      = I2O_EXEC_HRT_GET;

    //
    // Set the Source and Destination addresses.
    //
    getHRTMessage.StdMessageFrame.TargetAddress    = I2O_IOP_TID;
    getHRTMessage.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer
    // and the TransactionContext (for NT64) contains the high 32 bits.  Contained
    // in the callback object is the event object associated with the IO completion.
    //
    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callbackObject;

    getHRTMessage.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    getHRTMessage.TransactionContext               = pointer64ToU32.u.HighPart;

    //
    // Setup a S/G Pointer to the HRT data area.
    //
    getHRTMessage.SGL.u.Simple[0].FlagsCount.Count = HRT_BUFFER_SIZE;
    getHRTMessage.SGL.u.Simple[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT |
                                                     I2O_SGL_FLAGS_END_OF_BUFFER |
                                                     I2O_SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT;
    getHRTMessage.SGL.u.Simple[0].PhysicalAddress  = (DeviceExtension->ScratchBufferPA.LowPart
                                                      + HRT_OFFSET);

    //
    // Send the Message and wait for the result
    //
    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&getHRTMessage)) {
        DebugPrint((0,
                   "I2OGetHRT: Failed sending HRT\n"));
        return FALSE;
    }

    //
    // Check for completion.
    //
    if (I2OGetCallBackCompletion(DeviceExtension,
                                 100000)) {
        return TRUE;
    } 
    
    //
    // 'Something' bad occurred. This will fail the init. process.
    //
    DebugPrint((0,
               "I2OGetHrt: ISR never handled the hrt request.\n"));
    
    return FALSE;

}


BOOLEAN
I2OSetSysTab(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This sends the ExecSysTabSet msg which among other things, brings the IOP
    to the READY state.

Arguments:
    DeviceExtension
    
Return Value:
    TRUE - The request completed OK.
    FALSE - The request was not sent or timed out.

--*/
{
    I2O_EXEC_SYS_TAB_SET_MESSAGE setSysTabMessage;
    PIOP_CALLBACK_OBJECT   callbackObject;
    POINTER64_TO_U32       pointer64ToU32;
    PI2O_SET_SYSTAB_HEADER sysTabHdr;
    PI2O_IOP_ENTRY         sysTabEntry;
    ULONG                  sysTabCount;
    PVOID                  statusBuffer;
    STOR_PHYSICAL_ADDRESS  statusBufferPhysicalAddress;
    STOR_PHYSICAL_ADDRESS  sysTabPhysicalAddress;
    STOR_PHYSICAL_ADDRESS  inboundMessageFifoAddr;
    ULONG                  sysTabSize;
    ULONG length;
    ULONG i;
    ULONG_PTR physicalAddress;

    callbackObject = &DeviceExtension->InitCallback;
    callbackObject->Context = &DeviceExtension->CallBackComplete;

    DeviceExtension->CallBackComplete = FALSE;
    //
    // Zero out the SysTab message frame.
    //
    memset(&setSysTabMessage,0, sizeof(I2O_EXEC_SYS_TAB_SET_MESSAGE));

    //
    // Get addresses into the uncached extension.
    //
    sysTabHdr = (PI2O_SET_SYSTAB_HEADER)DeviceExtension->ScratchBuffer;
    sysTabPhysicalAddress = DeviceExtension->ScratchBufferPA;
    //
    // and zero it for 2 pages.
    //
    memset(sysTabHdr, 0, (PAGE_SIZE * 2));

    //
    // Fill out the SysTab header.
    //
    sysTabHdr->NumberEntries = 1;
    sysTabHdr->SysTabVersion = I2O_RESOURCE_MANAGER_VERSION;
    sysTabHdr->CurrentChangeIndicator = 0;

    //
    // Point to the first SysTab entry.
    //
    sysTabEntry = (PI2O_IOP_ENTRY)((PUCHAR)sysTabHdr + sizeof(I2O_SET_SYSTAB_HEADER));
    sysTabCount = 0;


    statusBuffer  = (PVOID)((PUCHAR)DeviceExtension->ScratchBuffer + PAGE_SIZE);
    statusBufferPhysicalAddress = DeviceExtension->ScratchBufferPA;
    statusBufferPhysicalAddress.LowPart += PAGE_SIZE;

    //
    // Get the Current Status of the IOP.
    //
    if (!(I2OGetStatus(DeviceExtension,
                       statusBuffer,
                       statusBufferPhysicalAddress))) {
        DebugPrint((0,
                    "I2OSetSysTab: Couldn't get status\n"));
        StorPortLogError(DeviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         SP_INTERNAL_ADAPTER_ERROR,I2OMP_ERROR_STATUSGET_FAILED);
        return FALSE;
    }

    //
    // Copy the Status info from the IOP to the SysTab table entry.  Many fields in the
    // SysTab table entry are overlayed on top of the Status info obtained from the IOP.
    // Other fields need to be filled in explicitly with the necessary info.
    //
    *sysTabEntry = *(PI2O_IOP_ENTRY)statusBuffer;

    sysTabEntry->IOP_ID = IOP_ID_BASE_VALUE;
    sysTabEntry->reserved1 = 0;
    sysTabEntry->reserved2 = 0;
    sysTabEntry->LastChanged = 0;
    sysTabEntry->IopCapabilities =
        ((PI2O_EXEC_STATUS_GET_REPLY)statusBuffer)->IopCapabilities;

    //
    // Get the physical address of this IOP's inbound message FIFO.  This information
    // tells IOPs where to get message frames from other peer IOPs on the system.
    //
    physicalAddress = DeviceExtension->PhysicalMemoryAddress;
    physicalAddress = physicalAddress + MESSAGE_POST;
    inboundMessageFifoAddr.QuadPart = (ULONG)(physicalAddress);
    sysTabEntry->MessengerInfo.InboundMessagePortAddressHigh = inboundMessageFifoAddr.HighPart;
    sysTabEntry->MessengerInfo.InboundMessagePortAddressLow  = inboundMessageFifoAddr.LowPart;

    //
    // Point to the next SysTab entry.
    //
    sysTabCount++;
    sysTabEntry++;

    //
    // Setup a SysTabSet message to send to the IOP.
    //
    setSysTabMessage.StdMessageFrame.VersionOffset = I2O_VERSION_11 | I2O_SYS_TAB_SGL_VER_OFFSET;
    setSysTabMessage.StdMessageFrame.MessageSize   = (sizeof(I2O_EXEC_SYS_TAB_SET_MESSAGE) -
                                                      sizeof(I2O_SG_ELEMENT) +
                                                      sizeof(I2O_SGE_SIMPLE_ELEMENT)) >> 2;
    setSysTabMessage.StdMessageFrame.Function      = I2O_EXEC_SYS_TAB_SET;

    //
    // Set the Source and Destination addresses.
    //
    setSysTabMessage.StdMessageFrame.TargetAddress    = I2O_IOP_TID;
    setSysTabMessage.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;

    //
    // Setup the SGL to point to the SysTab header and table entries.
    //
    // Note:  The 1.5 spec specifies 3 buffers.  The first contains the system table;
    //        the second is the private memory space; and the third is the private I/O
    //        space.  The following SGL does not specify the latter two buffers.
    //
    sysTabSize = sizeof(I2O_SET_SYSTAB_HEADER) + (sizeof(I2O_IOP_ENTRY) * sysTabCount);

    setSysTabMessage.SGL.u.Simple[0].FlagsCount.Count = sysTabSize;
    setSysTabMessage.SGL.u.Simple[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                                        I2O_SGL_FLAGS_END_OF_BUFFER |
                                                        I2O_SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT;
    setSysTabMessage.SGL.u.Simple[0].PhysicalAddress  = sysTabPhysicalAddress.LowPart;

    //
    // Set IOP_ID in the SysTabSet message to inform the IOP its ID number.
    // The host does not support multi-unit protocols with other hosts, so
    // we set the HostUnitID to zero for now.
    //
    setSysTabMessage.IOP_ID     = IOP_ID_BASE_VALUE;
    setSysTabMessage.HostUnitID = 0;

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer
    // and the TransactionContext (for NT64) contains the high 32 bits.  Contained
    // in the callback object is the event object associated with the IO completion.
    //
    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callbackObject;

    setSysTabMessage.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    setSysTabMessage.TransactionContext = pointer64ToU32.u.HighPart;

    //
    // Send the Message and wait for the result
    //
    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&setSysTabMessage)) {
        DebugPrint((0,
                   "I2OSetSysTab: Failed sending setSysTab\n"));
        return FALSE;
    }

    //
    // Check for completion.
    //
    if (I2OGetCallBackCompletion(DeviceExtension,
                                 10000)) {
        return TRUE;
    }

    DebugPrint((0,
               "I2OSetSysTab: ISR never handled the sysTab request.\n"));

    return FALSE;
}


BOOLEAN
I2OEnableSys(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This sends the ExecSysEnable msg which allows the IOP to resume operations.

Arguments:
    DeviceExtension 
    
Return Value:

--*/
{
    PIOP_CALLBACK_OBJECT        callbackObject;
    I2O_EXEC_SYS_ENABLE_MESSAGE enableSysMessage;
    PI2O_EXEC_STATUS_GET_REPLY  getStatusReply;
    STOR_PHYSICAL_ADDRESS       getStatusReplyPhys;
    POINTER64_TO_U32            pointer64ToU32;
    ULONG i;
    ULONG timeout;

    callbackObject = &DeviceExtension->InitCallback;
    callbackObject->Context = &DeviceExtension->CallBackComplete;

    DeviceExtension->CallBackComplete = FALSE;

    //
    // Zero out the SysEnable message frame.
    //
    memset(&enableSysMessage,0, sizeof(I2O_EXEC_SYS_ENABLE_MESSAGE));

    //
    // Setup a SysTabSet message to send to the IOP.
    //
    enableSysMessage.StdMessageFrame.VersionOffset = I2O_VERSION_11;
    enableSysMessage.StdMessageFrame.MessageSize   = sizeof(I2O_EXEC_SYS_ENABLE_MESSAGE) >> 2;
    enableSysMessage.StdMessageFrame.Function      = I2O_EXEC_SYS_ENABLE;

    //
    // Set the Source and Destination addresses.
    //
    enableSysMessage.StdMessageFrame.TargetAddress    = I2O_IOP_TID;
    enableSysMessage.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;

    //
    // Get IOP Status and then send an SysEnable message to all IOPs.
    //
    getStatusReply = (PVOID)((PUCHAR)DeviceExtension->ScratchBuffer + PAGE_SIZE);
    getStatusReplyPhys = DeviceExtension->ScratchBufferPA;
    getStatusReplyPhys.LowPart += PAGE_SIZE;

    //
    // Get the Current Status of the IOP.
    //
    if (!I2OGetStatus(DeviceExtension,
                      getStatusReply,
                      getStatusReplyPhys)) {
        DebugPrint((0,
                   "I2OEnableSys: Couldn't get status.\n"));

        StorPortLogError(DeviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         SP_INTERNAL_ADAPTER_ERROR,I2OMP_ERROR_STATUSGET_FAILED);
        return FALSE;
    }

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer
    // and the TransactionContext (for NT64) contains the high 32 bits.  Contained
    // in the callback object is the event object associated with the IO completion.
    //
    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callbackObject;

    enableSysMessage.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    enableSysMessage.TransactionContext               = pointer64ToU32.u.HighPart;


    DeviceExtension->CallBackComplete = 0;
    if (!I2OSendMessage(DeviceExtension,
                       (PI2O_MESSAGE_FRAME)&enableSysMessage)) {
        DebugPrint((0,
                   "I2OEnableSys: Failure sending enableSys message.\n"));
        return FALSE;
    }

    //
    // Check for completion. Have had problems with the isr never
    // getting hit. Use an extraordinarily huge timeout (10 min).
    // 10 usec * 100 = 1 ms * 1000 * 60 * 10 (this following routine includes
    // a 1000 us spin.
    //
    timeout = 1000 * 60 * 10;

    if (I2OGetCallBackCompletion(DeviceExtension,
                                 timeout)) {
        return TRUE;
    }    

    DebugPrint((0,
               "I2OEnableSys: ISR never handled the enable sys request.\n"));
    return FALSE;
}


BOOLEAN
I2OAsyncClaimDevice(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This sends a Claim msg to any devices needing it. It's used during non-init operations, such
    as when the admin. utility is used to create a new volume, or a device is hot-plugged.

Arguments:

    DeviceExtension - the miniport's HwDeviceExtension
    
Return Value:

    TRUE - if the message was successfully submitted to the IOP.

--*/
{
    I2O_UTIL_CLAIM_MESSAGE msg;
    PI2O_LCT lct = DeviceExtension->Lct;
    PIOP_CALLBACK_OBJECT callBackObject;
    POINTER64_TO_U32 pointer64ToU32;
    PI2O_DEVICE_INFO deviceInfo;
    ULONG i;
    BOOLEAN sentClaim = FALSE;


    callBackObject = &DeviceExtension->AsyncClaimCallback;
    callBackObject->Context = &DeviceExtension->CallBackComplete;
    
    //
    // Zero the Message Frame.
    //
    memset(&msg, 0, sizeof(msg));

    //
    // Check each device for needing a claim sent. 
    //
    for (i = 0; i < IOP_MAXIMUM_TARGETS; i++) {
        
        deviceInfo = &DeviceExtension->DeviceInfo[i];

        //
        // Determine whether this device needs to be claimed.
        // 
        if (deviceInfo->DeviceFlags & DFLAGS_NEED_CLAIM) {

            //
            // Update the claim flags.
            // 
            deviceInfo->DeviceFlags &= ~DFLAGS_NEED_CLAIM;
            deviceInfo->DeviceFlags |= DFLAGS_CLAIM_ISSUED;
            
            sentClaim = TRUE;
            
            DebugPrint((1,
                        "I2OAsyncClaimDevice: Send claim on index 0x%x.\n",
                        i));

            //
            // Use this to pass the devices index between routines.
            //
            DeviceExtension->CallBackComplete = i;
            
            //
            // Format the Claim/Release message.
            //
            msg.StdMessageFrame.VersionOffset = I2O_VERSION_11;
            msg.StdMessageFrame.MsgFlags = 0;
            msg.StdMessageFrame.MessageSize = sizeof(I2O_UTIL_CLAIM_MESSAGE)/4;
            msg.StdMessageFrame.TargetAddress    = deviceInfo->Lct.LocalTID;
            msg.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
            msg.StdMessageFrame.Function = I2O_UTIL_CLAIM;
            msg.StdMessageFrame.InitiatorContext = 0;
            msg.ClaimFlags = I2O_CLAIM_FLAGS_PEER_SERVICE_DISABLED |
                             I2O_CLAIM_FLAGS_MGMT_SERVICE_DISABLED;
            msg.ClaimType  = I2O_CLAIM_TYPE_PRIMARY_USER;

            pointer64ToU32.Pointer64 = 0;
            pointer64ToU32.Pointer   = callBackObject;

            msg.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
            msg.TransactionContext = pointer64ToU32.u.HighPart;


            //
            // Send the message.
            //
            if (!I2OSendMessage(DeviceExtension,
                               (PI2O_MESSAGE_FRAME)&msg)) {

                DebugPrint((0,
                           "I2OClaimDevice: Failure sending claim on index (%x).\n",
                           i));

                return FALSE;
            }

            // 
            // break out of loop, let the callback get any more claims on the list
            // 
            i = IOP_MAXIMUM_TARGETS;  
        }
    }

    //
    // Check whether at least one claim message was sent.
    // 
    if (!sentClaim) {

        //
        // For some reason, nothing was suitable for claiming. Restart the process
        // to be sure.
        // 
        if (DeviceExtension->AdapterFlags & AFLAGS_LOCKED) {

            DeviceExtension->AdapterFlags &= ~AFLAGS_LOCKED;
            StorPortResume(DeviceExtension);
        }    

        StorPortNotification(BusChangeDetected,
                             DeviceExtension,
                             0);
        //
        // Reissue
        //
        I2OIssueLctNotify(DeviceExtension,
                          &DeviceExtension->LctCallback,
                          DeviceExtension->CurrentChangeIndicator);
    }

    return TRUE;
}


BOOLEAN
I2OClaimRelease(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG Tid
    )
/*++

Routine Description:

    This sends a Claim Release msg to the specified device needing it. 
    This is currently only used when a device's state changes to 
    DSTATE_NORECOVERY and all outstanding IO to the device has completed.

Arguments:
    DeviceExtension
    deviceInfo - The device that needs to have its claim released.
    
Return Value:

--*/
{
    I2O_UTIL_CLAIM_RELEASE_MESSAGE msg;
    PIOP_CALLBACK_OBJECT callBackObject;
    POINTER64_TO_U32 pointer64ToU32;

    callBackObject = &DeviceExtension->ClaimReleaseCallback;
    
    //
    // Zero the Message Frame.
    //
    memset(&msg, 0, sizeof(msg));

    //
    // Format the ClaimRelease message.
    //
    msg.StdMessageFrame.VersionOffset = I2O_VERSION_11;
    msg.StdMessageFrame.MsgFlags = 0;
    msg.StdMessageFrame.MessageSize = sizeof(I2O_UTIL_CLAIM_MESSAGE)/4;
    msg.StdMessageFrame.TargetAddress    = Tid;
    msg.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    msg.StdMessageFrame.Function = I2O_UTIL_CLAIM_RELEASE;
    msg.StdMessageFrame.InitiatorContext = 0;
    msg.ReleaseFlags = 0;
    msg.ClaimType  = I2O_CLAIM_TYPE_PRIMARY_USER;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callBackObject;

    msg.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg.TransactionContext = pointer64ToU32.u.HighPart;

    //
    // Send the message.
    //
    if (!I2OSendMessage(DeviceExtension,
                       (PI2O_MESSAGE_FRAME)&msg)) {
        DebugPrint((0,
                   "I2OClaimRelease: Failure sending claim release\n"));
        return FALSE;
    }
    return TRUE;
}


VOID
I2ORemoveMultiDevices(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine will remove all devices from the deviceInfo table
    that have DFLAGS_REMOVE_DEVICE flag set in the DeviceFlags.

Arguments:

    DeviceExtension - The miniport's HwDeviceExtension
    
Return Value:

    NONE

--*/
{
    ULONG tid;

    //
    // Walk the list of devices.
    // 
    for (tid = 0; tid < IOP_MAXIMUM_TARGETS; tid++) {

        //
        // Check whether a remove is needed.
        // 
        if (DeviceExtension->DeviceInfo[tid].DeviceFlags & DFLAGS_REMOVE_DEVICE) {
            
            //
            // Remove the device.
            // 
            I2ORemoveSingleDevice(DeviceExtension,tid);
        }
    }
#if DBG    
    DumpTables(DeviceExtension);
#endif
    return;
} 


ULONG
I2OHandleLctUpdate(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine handles managing the local device list given an LCT update.  It
    adds or deletes local entries based on new/deleted or failed claimed devices.
    The AssignedID for each device is the host SCSI targetID or disk number.

Arguments:

    DeviceExtension - The miniport's HwDeviceExtension
    
Return Value:

    Actions for the caller to perform.
    0 - Do nothing. The LCTs are the same.
    1 - Indicates something was removed.
    2 - Something was added.

--*/
{
    PI2O_LCT lct = DeviceExtension->Lct;
    PI2O_LCT updateLct = DeviceExtension->UpdateLct;
    PI2O_DEVICE_INFO oldLct;
    PI2O_LCT_ENTRY lctEntry;
    ULONG i;
    ULONG t;
    ULONG updated = 0;
    BOOLEAN deleted = FALSE;
    BOOLEAN found = FALSE;
    ULONG targetId;
    ULONG newLCTEntries;
    ULONG oldLCTEntries;
    PI2O_DEVICE_INFO deviceInfo;

    //
    // Determine how many entries are in each of the LCTs.
    // 
    newLCTEntries = (updateLct->TableSize - 3) / 9;
    oldLCTEntries = (lct->TableSize - 3) / 9;

    DebugPrint((1,
                "\nI2OHandleLctUpdate: oldLCTentries = 0x%x newLCTentries=0x%x\n",
                oldLCTEntries,
                newLCTEntries));

    if (newLCTEntries < oldLCTEntries) {

        //
        // Check for REMOVED devices
        // loop through known device list, don't care about other LCT entries that may have gone.
        //
        DebugPrint((1,
                    "I2OHandleLctUpdate: Check for removed devices\n"));
        
        for (i = 0; i < IOP_MAXIMUM_TARGETS; i++) {
            
            oldLct = &DeviceExtension->DeviceInfo[i];
            found = FALSE;
            
            for (t = 0; t < newLCTEntries; t++) {

                //
                // Grab the entry.
                // 
                lctEntry = &updateLct->LCTEntry[t];

                //
                // Check for supported class, don't bother looking at
                // newLCT entries we dont care about.
                //
                if ((lctEntry->ClassID.Class == I2O_CLASS_RANDOM_BLOCK_STORAGE) ||
                    (lctEntry->ClassID.Class == I2O_CLASS_SCSI_PERIPHERAL)) {

                    //
                    // See if there is a matching TID in our old list.
                    // 
                    if (oldLct->Lct.LocalTID == lctEntry->LocalTID) {
            
                        //
                        // Force an exit from this loop, as the entry is in
                        // both lists.
                        // 
                        found = TRUE; 
                        t = newLCTEntries;
                    }
                }
            }

            //
            // See if the device was previously claimed before removing it.
            //
            if ((!found) && (oldLct->DeviceFlags & DFLAGS_CLAIMED)) {
                
                // 
                // TID was on old list but not on new one... kill it    
                // 
                DebugPrint((1,
                            "I2OHandleLctUpdate: Removing LocalTID (%x), LCTIndex 0x%x\n",
                            oldLct->Lct.LocalTID,
                            oldLct->LCTIndex));

                //
                // set flags for cleanup below
                //
                updated = 1; 
                deleted = TRUE;
                oldLct->DeviceFlags |= DFLAGS_REMOVE_DEVICE;
            }
        }
    } else { 

        //
        // Check for ADDED devices, outer loop on newLCT table
        //
        DebugPrint((1,
                    "I2OHandleLctUpdate2: Check for NEW devices\n"));

        for (i = 0; i < newLCTEntries; i++) {
            
            found = FALSE;
            t = 0;
            lctEntry = &updateLct->LCTEntry[i];
            
            do {
                oldLct = &DeviceExtension->DeviceInfo[t];

                //
                // Check for a supported class, unclaimed device.
                //
                if ((lctEntry->ClassID.Class == I2O_CLASS_RANDOM_BLOCK_STORAGE) ||
                    (lctEntry->ClassID.Class == I2O_CLASS_SCSI_PERIPHERAL)) {
                    
                    if (lctEntry->LocalTID == oldLct->Lct.LocalTID) {
                       
                        //
                        // This one is present. Force a break and go to the next
                        // device in the NEW list.
                        // 
                        found = TRUE;
                        t = IOP_MAXIMUM_TARGETS;
                    }
                } else {

                    // 
                    // for classes we don't care about dont try to add them
                    //
                    found = TRUE;
                }
                
                t++;
                
                // 
                // Numberdevices is the count of previously known devices 
                // 
            } while (t < IOP_MAXIMUM_TARGETS);
            
            if (!found) {

                //
                // TID was not on old list but on new one... add it if unclaimed
                //
                if (updateLct->LCTEntry[i].UserTID == 0xFFF) {
                    
                    //
                    // Indicate to the caller that a claim should be sent, along
                    // with telling the port driver that something new is here.
                    // 
                    updated=2;
                    
                    //
                    // Build the structure that represents the device.
                    // 
                    I2OBuildDeviceInfo(DeviceExtension,
                                       updateLct,
                                       i,
                                       TRUE);
                }
            }
        }
        
        DebugPrint((1,
                    "I2OHandleLctUpdate2: Done looking for NEW devices\n"));

    }

    //
    // Handle cleanup now that we've parsed the whole list
    //
    if (deleted) {
        I2ORemoveMultiDevices(DeviceExtension);
    } 

    //
    // Always update local deviceinfo LCTIndex numbers in case they changed, they can
    // when lower TIDs are re-used (not suppsed to happen based on RTOS patch but IS)
    // note this can happen on either create or delete.  If changes are made then insure
    // that the device map is updated.
    //
    for (i = 0; i < IOP_MAXIMUM_TARGETS; i++) {

        //
        // Get the existing LCT
        // 
        oldLct = &DeviceExtension->DeviceInfo[i];

        //
        // Run through each of the entries in the new LCT.
        // 
        for (t = 0; t < newLCTEntries; t++) {

            lctEntry = &updateLct->LCTEntry[t];
            
            if (lctEntry->LocalTID == oldLct->Lct.LocalTID) {
                if (oldLct->LCTIndex != t) {
                    
                    DebugPrint((1,
                    "I2OHandleLctUpdate: Resetting LCTIndex, changed from %d to %d for TID 0x%x\n",
                    oldLct->LCTIndex,
                    t,
                    oldLct->Lct.LocalTID));

                    //
                    // Update the index and the device map.
                    // 
                    DeviceExtension->DeviceMap &= ~(1 << oldLct->LCTIndex);
                    oldLct->LCTIndex = t;
                    DeviceExtension->DeviceMap |= (1 << oldLct->LCTIndex);
                }
            }
        }
    }

    //
    // Finally, copy over the update LCT 
    //
    StorPortMoveMemory(lct,
                       updateLct,
                       updateLct->TableSize * 4);
#if DBG
    DumpTables (DeviceExtension);
#endif

    return updated;    
}


VOID
I2OLctNotifyCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME MessageFrame,
    IN PVOID CallbackContext
    )
/*++

Routine Description:

    This routine is the call back for retrieving the LCT, and parsing it to determine
    whether any devices that we care about have come or gone. If so, the necessary actions
    are taken to update internal structures and notify the port driver.

Arguments:

    MessageFrame - The LCT reply message.
    CallbackContext - The miniport's HwDeviceExtension
    
Return Value:

    NONE

--*/
{

    PI2O_SINGLE_REPLY_MESSAGE_FRAME msg = (PI2O_SINGLE_REPLY_MESSAGE_FRAME )MessageFrame;
    PIOP_CALLBACK_OBJECT callbackObject;
    POINTER64_TO_U32  pointer64ToU32;
    PDEVICE_EXTENSION deviceExtension = CallbackContext;
    PI2O_LCT lct = deviceExtension->UpdateLct;
    ULONG    flags;
    ULONG    deviceAdded;
    ULONG    rescan = 0;
    ULONG lctEntries;
    ULONG t;

    pointer64ToU32.u.LowPart  = msg->StdMessageFrame.InitiatorContext;
    pointer64ToU32.u.HighPart = msg->TransactionContext;
    callbackObject            = pointer64ToU32.Pointer;

    if ((flags = MessageFrame->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {

        DebugPrint((0,
                   "I2OLctCallBack: Failure. Msg Flags - %x\n",
                   flags));
        return;
    }

    if (MessageFrame->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {

        DebugPrint((0,
                   "I2OLctCallBack: Reply status not success - %x\n",
                   MessageFrame->BsaReplyFrame.ReqStatus));
        return;
    }

    //
    // Tell storport to pause the adapter - we don't want to handle requests
    // while dealing with this.
    //
    StorPortPause(deviceExtension,
                  ADAPTER_LCT_TIMEOUT); 
    DebugPrint((0,
                "LCTNotifyCallBack: Pausing (%x)\n",
                deviceExtension));
                  
    //
    // Indicate to ourselves that we are locked.
    //
    deviceExtension->AdapterFlags |= AFLAGS_LOCKED;

    //
    // Get the new change indicator value. 
    //
    if (deviceExtension->CurrentChangeIndicator != lct->CurrentChangeIndicator) {

        DebugPrint((1,
                    "I2OLctNotifyCallBack: Old Ind (%x) Current (%x)\n",
                    deviceExtension->CurrentChangeIndicator,
                    lct->CurrentChangeIndicator));
        
        deviceExtension->CurrentChangeIndicator = lct->CurrentChangeIndicator;

    }

    //
    // Determine what changed.
    // UserTID's will change when the device is claimed.
    // If a device is deleted, it's entry goes away.
    // If one is created, it's entry is built, but it's unclaimed.
    //
    deviceAdded = 0;

    //
    // Parse the new list and determine  what action should be taken.
    // 
    rescan = I2OHandleLctUpdate(deviceExtension);

    //
    // If a device changed (not just claimed) then tell port to re-enum.
    //
    if (rescan > 0) {
        if (rescan > 1) {
            
            DebugPrint((1,
                       "I2OLctNotifyCallback: Call for claim.\n"));

            //
            // This will start the claim process.
            // 
            I2OAsyncClaimDevice(deviceExtension);

        } else {
            
            // 
            // Don't rescan if we still have pending claims even if we deleted something
            // rescan will happen in the claims callback LAST after all knowns claims are done
            //
            // NOTE: If the driver is changed to project bus numbers, then
            // the 0 will need to be changed to indicate which bus was affected.
            //
            DebugPrint((1,
                       "I2OLctNotifyCallback: Call for rescan no claim activity.\n"));
      
            //
            // Indicate that requests are allowed. The adapter gets resumed below.
            //
            deviceExtension->AdapterFlags &= ~AFLAGS_LOCKED;
            
            StorPortNotification(BusChangeDetected,
                                 deviceExtension,
                                 0);
            //
            // Update the expected LCT size in the deviceExtension. 
            //
            I2OGetExpectedLCTSizeAndMFrames(deviceExtension);

            //
            // Reissue the request for LCT notification.
            //
            I2OIssueLctNotify(deviceExtension,
                              &deviceExtension->LctCallback,
                              deviceExtension->CurrentChangeIndicator);
        }
    } else {
    
        //
        // Allow requests to come in. The adapter gets resumed below.
        //
        deviceExtension->AdapterFlags &= ~AFLAGS_LOCKED;
        
        //
        // Update the LCT size and reissue the request for LCT notifications.
        //
        I2OGetExpectedLCTSizeAndMFrames(deviceExtension);

        I2OIssueLctNotify(deviceExtension,
                          &deviceExtension->LctCallback,
                          deviceExtension->CurrentChangeIndicator);

    }

    //
    // See whether requests should be allowed again.
    //
    if (!(deviceExtension->AdapterFlags & AFLAGS_LOCKED)) {

        //
        // Preempt the timeout and directly resume.
        //
        StorPortResume(deviceExtension);
    }    
        
    return;
}




BOOLEAN
I2OIssueLctNotify(
    IN PDEVICE_EXTENSION DeviceExtension,
    PIOP_CALLBACK_OBJECT CallBackObject,
    IN ULONG ChangeIndicator
    )
/*++

Routine Description:

    This sends the LctNotify msg. When a change occurs, we will be called
    back in the routine indicated by CallBackObject.

Arguments:

    DeviceExtension - the miniport's HwDeviceExtension
    CallBackObject - The initialized callback object for LCT notifications
    ChangeIndicator - The current change indicator associated with the last LCT notify.
    
Return Value:

    TRUE - if the message was successfully sent to the IOP.

--*/
{
    I2O_EXEC_LCT_NOTIFY_MESSAGE lctMessage;
    POINTER64_TO_U32 pointer64ToU32;

    memset(&lctMessage, 0, sizeof(I2O_EXEC_LCT_NOTIFY_MESSAGE));

    //
    // Setup a get status message to send to the IOP.
    //
    lctMessage.StdMessageFrame.VersionOffset = I2O_VERSION_11 | I2O_LCT_SGL_VER_OFFSET;
    lctMessage.StdMessageFrame.MessageSize   = (sizeof(I2O_EXEC_LCT_NOTIFY_MESSAGE) -
                                                sizeof(I2O_SG_ELEMENT) +
                                                sizeof(I2O_SGE_SIMPLE_ELEMENT)) >> 2;
    lctMessage.StdMessageFrame.Function      = I2O_EXEC_LCT_NOTIFY;

    //
    // Set the Source and Destination addresses.
    //
    lctMessage.StdMessageFrame.TargetAddress    = I2O_IOP_TID;
    lctMessage.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;

    //
    // Set the Class ID and Change Indicators
    //
    lctMessage.ClassIdentifier = I2O_CLASS_MATCH_ANYCLASS;
    lctMessage.LastReportedChangeIndicator = ChangeIndicator;

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer
    // and the TransactionContext (for NT64) contains the high 32 bits.  Contained
    // in the callback object is the event object associated with the IO completion.
    //
    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = CallBackObject;

    lctMessage.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    lctMessage.TransactionContext               = pointer64ToU32.u.HighPart;

    lctMessage.SGL.u.Simple[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT |
                                                  I2O_SGL_FLAGS_END_OF_BUFFER |
                                                  I2O_SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT;

    lctMessage.SGL.u.Simple[0].FlagsCount.Count = DeviceExtension->ExpectedLCTSize;
    lctMessage.SGL.u.Simple[0].PhysicalAddress  = DeviceExtension->UpdateLctPA.LowPart;


    if (!I2OSendMessage(DeviceExtension,
                       (PI2O_MESSAGE_FRAME)&lctMessage)) {
        DebugPrint((1,
                   "I2OGetLct: Failure sending lctMessage message.\n"));
        return FALSE;
    }
    return TRUE;
}


BOOLEAN
I2OGetLCT(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    Init. wrapper for IssueLctNotify. It sends the msg. synchronously and sets
    up the original deviceInfo array.

Arguments:

    DeviceExtension - The miniport's HwDeviceExtension
    
Return Value:

    TRUE - If the LCT was retrieved.

--*/
{
    I2O_EXEC_LCT_NOTIFY_MESSAGE lctMessage;
    POINTER64_TO_U32 pointer64ToU32;
    PIOP_CALLBACK_OBJECT callBackObject;
    PI2O_LCT_ENTRY lctEntry;
    ULONG targetId;
    ULONG i;

    callBackObject = &DeviceExtension->InitCallback;
    callBackObject->Context = &DeviceExtension->CallBackComplete;

    DeviceExtension->CallBackComplete = FALSE;

    //
    // Send the LCT Notify with a change indicator of 0. meaning return the LCT
    // now.
    //
    if (!I2OIssueLctNotify(DeviceExtension,
                         callBackObject,
                         0)) {
        
        //
        // This is a fatal error. Log it. If HwInit. is the caller, this will fail
        // the adapter start.
        // 
        StorPortLogError(DeviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         SP_INTERNAL_ADAPTER_ERROR,
                         I2OMP_ERROR_LCTNOTIFY_FAILED);
        return FALSE;
    }

    //
    // Check for completion.
    //
    StorPortStallExecution(50000);
    if (I2OGetCallBackCompletion(DeviceExtension,
                                 2000)) {

        //
        // The callback routine has been called, and
        // put status in the deviceExtension.
        //
        if (DeviceExtension->CallBackComplete) {
            PI2O_LCT lct = DeviceExtension->Lct;
            PI2O_LCT updateLct = DeviceExtension->UpdateLct;
            ULONG t = 0;
            ULONG n = 0;

            //
            // Copy over the lct
            //
            StorPortMoveMemory(lct,
                               updateLct,
                               (updateLct->TableSize * 4));
            //
            // Dump the LCT information:
            //
            DebugPrint((1,"Table Size: %d.\t",  lct->TableSize));
            DebugPrint((1,"Boot Device: %d.\t", lct->BootDeviceTID));
            DebugPrint((1,"LctVer: %d.\n",      lct->LctVer));
            DebugPrint((1,"IOP Flags: 0x%x.\t", lct->IopFlags));
            DebugPrint((1,"Current Change Indicator: %d.\n---- LCT Entries:\n",
                        lct->CurrentChangeIndicator));

            //
            // Save off the ChangeIndicator. Once we are fully init'ed, another
            // lctNotify will be sent, so that the IOP can tell us when it's config
            // changes.
            //
            DeviceExtension->CurrentChangeIndicator = lct->CurrentChangeIndicator;
            DeviceExtension->IsmTID = 0x0;

            lctEntry = &lct->LCTEntry[t];
            while (lctEntry->TableEntrySize != 0) {

                DebugPrint((1,"\nTable Entry #: %d.\t",  t));
                DebugPrint((1,"Table Entry Size: %d.\t",  lct->LCTEntry[t].TableEntrySize));
                DebugPrint((1,"Local TID: %d.\t",         lct->LCTEntry[t].LocalTID));
                DebugPrint((1,"reserved: %d.\n",          lct->LCTEntry[t].reserved));
                DebugPrint((1,"Change Indicator: %d.\t",  lct->LCTEntry[t].ChangeIndicator));
                DebugPrint((1,"Device Flags: 0x%x.\n",    lct->LCTEntry[t].DeviceFlags));
                DebugPrint((1,"Class Code: 0x%x.\t",      lct->LCTEntry[t].ClassID.Class));
                DebugPrint((1,"Version: %d.\t",           lct->LCTEntry[t].ClassID.Version));
                DebugPrint((1,"Organization ID: 0x%x.\n", lct->LCTEntry[t].ClassID.OrganizationID));
                DebugPrint((1,"Sub Class Info: 0x%x.\n",  lct->LCTEntry[t].SubClassInfo));
                DebugPrint((1,"User TID: 0x%x.\t",        lct->LCTEntry[t].UserTID));
                DebugPrint((1,"Parent TID: %d.\t",        lct->LCTEntry[t].ParentTID));
                DebugPrint((1,"BIOS Info: 0x%x.\n",       lct->LCTEntry[t].BiosInfo));
                DebugPrint((1,"Identity Tag:0x%x\n ",     lct->LCTEntry[t].SubClassInfo));
                
                //
                // set the ISM TID, value of 0x0 means we couldn't ID it
                //
                if ((lctEntry->ClassID.Class == I2O_CLASS_DDM) &&
                    (lctEntry->ClassID.OrganizationID == 0x0028)) {
                    
                    DeviceExtension->IsmTID = lctEntry->LocalTID;
                    DebugPrint((1,
                                "Setting ISM value to Local TID: %d.\t",
                                lctEntry->LocalTID));
             
                } else if ((lctEntry->ClassID.Class == I2O_CLASS_DDM) &&
                           (lctEntry->LocalTID == 0x08)) {
                
                    //
                    // This is somewhat of a guess based on the behaviour of a 
                    // particular device.
                    // 
                    DeviceExtension->IsmTID = lctEntry->LocalTID;
                    
                }

                //
                // Check for any BSA or SCSI_PERPH types.
                //
                if ((lctEntry->ClassID.Class == I2O_CLASS_RANDOM_BLOCK_STORAGE) ||
                    (lctEntry->ClassID.Class == I2O_CLASS_SCSI_PERIPHERAL)) {
                
                    //
                    // Check for claims.
                    // 
                    if (lctEntry->UserTID == 0xFFF) {
                       
                        //
                        // We can use this one.
                        //
                        I2OBuildDeviceInfo(DeviceExtension,
                                           lct,
                                           t,
                                           TRUE);
                    }
                }

                //
                // Get the next entry.
                //
                t++;
                lctEntry = &lct->LCTEntry[t];
            }

            return TRUE;

        } else {
            DebugPrint((1,
                       "I2OGetLCT: ISR returned true, but CallBackComplete False\n"));
            StorPortLogError(DeviceExtension,
                             NULL,
                             0,
                             0,
                             0,
                             SP_INTERNAL_ADAPTER_ERROR,
                             I2OMP_ERROR_GETLCTCALLBACK_FAILED);
        
        }
    }
    
    StorPortLogError(DeviceExtension,
                     NULL,
                     0,
                     0,
                     0,
                     SP_INTERNAL_ADAPTER_ERROR,
                     I2OMP_ERROR_GETLCTTIMEOUT);

    return FALSE;
}


BOOLEAN
I2OGetExpectedLCTSizeAndMFrames(
    IN PDEVICE_EXTENSION DeviceExtension
    )

/*++

Routine Description: 

    This routine sends an ExecStatusGet message to all IOPs to get their
    expected LCT size and their MaxInboundMFrames and InitialInboundMFrames.

Arguments:

    DeviceExtension - The miniport's HwDeviceExtension

Return Value:

    TRUE | FALSE if not successful

--*/

{
    PI2O_EXEC_STATUS_GET_REPLY  getStatusReply;
    PHYSICAL_ADDRESS            getStatusReplyPhys;
    ULONG                       i;


    DeviceExtension->ExpectedLCTSize = 0;

    getStatusReply = (PVOID)((PUCHAR)DeviceExtension->ScratchBuffer + PAGE_SIZE);
    getStatusReplyPhys = DeviceExtension->ScratchBufferPA;
    getStatusReplyPhys.LowPart += PAGE_SIZE;

    //
    // Get the expected LCT size for the IOP.
    //
    if (I2OGetStatus(DeviceExtension,
                     getStatusReply,
                     getStatusReplyPhys)) {

        if (getStatusReply->ExpectedLCTSize < MIN_EXPECTED_LCT_SIZE) {
            DeviceExtension->ExpectedLCTSize = MIN_EXPECTED_LCT_SIZE;
        } else {
            if (getStatusReply->ExpectedLCTSize > (PAGE_SIZE * 2)) {

                //
                // In a world of hurt, as this is the only space we have.
                //
                DebugPrint((1,
                           "I2OGetExpectedLCTSize: LCT data too large %x, have only %x\n",
                           getStatusReply->ExpectedLCTSize,
                           (PAGE_SIZE * 2)));

                StorPortLogError(DeviceExtension,
                                 NULL,
                                 0,
                                 0,
                                 0,
                                SP_INTERNAL_ADAPTER_ERROR,I2OMP_ERROR_LCTTOOLARGE);
                return FALSE;
            }
            
       
            DeviceExtension->ExpectedLCTSize = getStatusReply->ExpectedLCTSize;
        }

        DebugPrint((1,
                    "I2OGetLCT: Expected LCT size is %x.\n",
                    DeviceExtension->ExpectedLCTSize));

        //
        // Also get miscellaneous configuration information about the IOP.
        //
        DeviceExtension->IOPCapabilities       = getStatusReply->IopCapabilities;
        DeviceExtension->IOPState              = getStatusReply->IopState;
        DeviceExtension->I2OVersion            = getStatusReply->I2oVersion;
        DeviceExtension->MessengerType         = getStatusReply->MessengerType;
        DeviceExtension->MaxInboundMFrames     = getStatusReply->MaxInboundMFrames;
        DeviceExtension->InitialInboundMFrames = getStatusReply->CurrentInboundMFrames;
        DeviceExtension->InboundMFrameSize     = getStatusReply->InboundMFrameSize << 2;

        DebugPrint((1,
                    "I2OGetLCT: MaxInboundFrames = %x InitialInboundFrames = %x FrameSize = %x\n",
                    DeviceExtension->MaxInboundMFrames,
                    DeviceExtension->InitialInboundMFrames,
                    DeviceExtension->InboundMFrameSize));
    }

    return TRUE;
}


BOOLEAN
I2OClaimDevice(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine send synchronous claim msgs during init. for all BSA and SCSI devices
    that are found to be unclaimed.
    
Arguments:

    
Return Value:

--*/
{
    I2O_UTIL_CLAIM_MESSAGE msg;
    PI2O_LCT lct = DeviceExtension->Lct;
    PIOP_CALLBACK_OBJECT callBackObject;
    POINTER64_TO_U32 pointer64ToU32;
    PI2O_DEVICE_INFO deviceInfo;
    USHORT localTid;
    ULONG i;
    ULONG j;
    BOOLEAN failedClaims = FALSE;

    callBackObject = &DeviceExtension->InitCallback;
    callBackObject->Context = &DeviceExtension->CallBackComplete;

    //
    // Find the LCT entry corresponding to this device.
    //
    for (j = 0; j < IOP_MAXIMUM_TARGETS; j++) {
        deviceInfo = &DeviceExtension->DeviceInfo[j];
        DebugPrint((1,
                    "I2OClaimDevice: DeviceInfo(%x) DeviceFlags 0x%x ID 0x%x\n",
                    deviceInfo,
                    deviceInfo->DeviceFlags,
                    j));

        if (deviceInfo->DeviceFlags & DFLAGS_NEED_CLAIM) {
            deviceInfo->DeviceFlags |= DFLAGS_CLAIM_ISSUED;

            //
            // Zero the Message Frame.
            //
            memset(&msg, 0, sizeof(msg));

            //
            // Format the Claim./Release message.
            //
            msg.StdMessageFrame.VersionOffset = I2O_VERSION_11;
            msg.StdMessageFrame.MsgFlags = 0;
            msg.StdMessageFrame.MessageSize = sizeof(I2O_UTIL_CLAIM_MESSAGE)/4;
            msg.StdMessageFrame.TargetAddress    = deviceInfo->Lct.LocalTID;
            msg.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
            msg.StdMessageFrame.Function = I2O_UTIL_CLAIM;
            msg.StdMessageFrame.InitiatorContext = 0;
            msg.ClaimFlags = I2O_CLAIM_FLAGS_PEER_SERVICE_DISABLED |
                             I2O_CLAIM_FLAGS_MGMT_SERVICE_DISABLED;
            msg.ClaimType  = I2O_CLAIM_TYPE_PRIMARY_USER;

            pointer64ToU32.Pointer64 = 0;
            pointer64ToU32.Pointer   = callBackObject;

            msg.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
            msg.TransactionContext = pointer64ToU32.u.HighPart;

            //
            // Send the message.
            //
            DeviceExtension->CallBackComplete = 0;
            if (!I2OSendMessage(DeviceExtension,
                               (PI2O_MESSAGE_FRAME)&msg)) {
                DebugPrint((0,
                           "I2OClaimDevice: Failure sending claim message.\n"));
                return FALSE;
            }

            //
            // Check for completion.
            //
            for (i = 0; i < 10000; i++) {
                StorPortStallExecution(1000);
                if (I2OInterrupt(DeviceExtension)) {
                    //
                    // The callback routine has been called, and
                    // put status in the deviceExtension.
                    //
                    DebugPrint((1,
                                "Check for Claim completion\n"));
                
                    //DumpTables(DeviceExtension);

                    if (DeviceExtension->CallBackComplete) {

                        DebugPrint((1,
                                   "I2OClaimDevice: Claim good, index (%x) lct entry (%d)\n",
                                   j,deviceInfo->LCTIndex));
                        deviceInfo->DeviceFlags &= ~(DFLAGS_NEED_CLAIM | DFLAGS_CLAIM_ISSUED);
                        deviceInfo->DeviceFlags |= DFLAGS_CLAIMED; 

                        //
                        // Now that the device has been claimed, mark it as being 
                        // available for the OS to use.  Also mark that this device
                        // needs event registration.
                        //
                        deviceInfo->DeviceFlags |= (DFLAGS_OS_CAN_CLAIM | DFLAGS_NEED_REGISTER);
                    } else {

                        DebugPrint((0,
                                   "I2OClaimDevice: Claim FAILED, index (%x) lct entry (%d)\n",
                                   j,deviceInfo->LCTIndex));
                        
                        //
                        // Indicate that at least one claim has failed, so
                        // that clean-up will happen.
                        // 
                        failedClaims = TRUE;
                        deviceInfo->DeviceFlags |= DFLAGS_REMOVE_DEVICE;                
                    }
                    break;
                }
            }
        }
    }

    //
    // Clean up entire device list in case of any claim failures
    //
    if (failedClaims) {
        I2ORemoveMultiDevices(DeviceExtension);
    }

#if DBG    
    DumpTables(DeviceExtension);
#endif

    return TRUE;
}


BOOLEAN
I2OInitialize(
    IN PVOID DeviceExtension
    )
/*++

Routine Description:

    This is called by StorPort after FindAdapter has returned successfully. It will
    init. the controller and find all devices for which we have interest.

Arguments:

    
Return Value:

--*/
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    BOOLEAN operationSuccess = FALSE;
    ULONG errorCode;
    ULONG uniqueId;
    BOOLEAN logError = TRUE;
    BOOLEAN returnStatus = FALSE;

    DebugPrint((1,
                "I2OInitialize: Start\n"));
    //
    // All the routines below need a polling loop to see when the
    // interrupt happens, as this routine and the isr are synchronized.
    // Just keep calling the isr, until it returns true.
    //
    if (!I2OInitOutbound(deviceExtension)) {

        //
        // Can't continue. Log the error and fail.
        //
        errorCode = SP_INTERNAL_ADAPTER_ERROR;
        uniqueId = I2OMP_ERROR_INITOUTBOUND_FAILED;

        goto logAndReturn;
    }

    //
    // Enable interrupts on the iop.
    //
    if (StorPortReadRegisterUlong(deviceExtension, deviceExtension->IntControl) != 0) {
        StorPortWriteRegisterUlong(deviceExtension,(deviceExtension->IntControl), 0);
    }

    if (!I2OGetHRT(deviceExtension)) {

        //
        // Log this fatal error.
        // 
        errorCode = SP_INTERNAL_ADAPTER_ERROR;
        uniqueId =  I2OMP_ERROR_GETHRT_FAILED;
        
        goto logAndReturn;
    }

    if (!I2OSetSysTab(deviceExtension)) {
        
        //
        // Set the codes and log the error.
        //
        errorCode = SP_INTERNAL_ADAPTER_ERROR;
        uniqueId = I2OMP_ERROR_SETSYSTAB_FAILED;
        
        goto logAndReturn;
    }

    if (!I2OEnableSys(deviceExtension)) {
        
        //
        // Indicate the failure.
        // 
        errorCode = SP_INTERNAL_ADAPTER_ERROR;
        uniqueId = I2OMP_ERROR_ENABLESYS_FAILED;
        
        goto logAndReturn; 
    }

    //
    // Get the expected LCT size and number of inbound message frames for each IOP.
    //
    if (!I2OGetExpectedLCTSizeAndMFrames(deviceExtension)) {

        //
        // Yet another fatal error.
        // 
        errorCode = SP_INTERNAL_ADAPTER_ERROR;
        uniqueId = I2OMP_ERROR_LCTSIZE_FAILED;

        goto logAndReturn;
    }

    DebugPrint((1,
                "I2OInitialize1: Expected LCT size is %d.\n",
                deviceExtension->ExpectedLCTSize));
    //
    // Get the lct data.
    //
    if (!I2OGetLCT(deviceExtension)) {
       
        //
        // Set the codes and log the error.
        // 
        errorCode = SP_INTERNAL_ADAPTER_ERROR;
        uniqueId = I2OMP_ERROR_GETLCT_FAILED;
        
        goto logAndReturn;
    }

    //
    // Since the miniport supports the Device deletion functionality,
    // it must tell the IOP that it supports this mode.  This insures 
    // that the IOP knows whether this functionality is supported or not.
    // Note that with older FW that does not support this mode, that this
    // request can fail.
    //
    I2OSetSpecialParams(deviceExtension,
                        I2O_RAID_ISM_CONFIG_GROUP_NO,
                        I2O_DELETE_VOLUME_SUPPORT_FIELD_IDX,
                        1);
    
    //
    // Claim any disks that were found.
    //
    if (!I2OClaimDevice(deviceExtension)) {

        //
        // Another fatal error. 
        // 
        errorCode = SP_INTERNAL_ADAPTER_ERROR;
        uniqueId = I2OMP_ERROR_CLAIM_FAILED;

        goto logAndReturn;
    }

    //
    // Register to receive callbacks in case of device resets or state changes.
    //
    if (!I2ORegisterForEvents(deviceExtension,
                              I2OMP_EVENT_MASK)) {
        errorCode = SP_INTERNAL_ADAPTER_ERROR;
        uniqueId = I2OMP_ERROR_EVENTREGISTER_FAILED;

        goto logAndReturn;
    }
   
    //
    // Made it through init. 
    //
    logError = FALSE;
    returnStatus = TRUE;
    
    //
    // Issue a new LctNotify with the changeIndicator specified.
    //
    deviceExtension->LctCallback.Context = deviceExtension;

    DebugPrint((1,
                "I2OInitialize2: Expected LCT size is %d.\n",
                deviceExtension->ExpectedLCTSize));
    
    I2OIssueLctNotify(deviceExtension,
                      &deviceExtension->LctCallback,
                      deviceExtension->CurrentChangeIndicator);

logAndReturn:

    if (logError) {

        //
        // Log the error indicated.
        //
        StorPortLogError(deviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         errorCode,
                         uniqueId);
    }

    return returnStatus;
}


VOID
I2OMediaInfoCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    This is the callback for Read Capacity requests to a removable media device.
    
Arguments:

    Msg - The reply message fram.
    DeviceContext - This miniport's HwDeviceExtension
    
Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION       deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT    callbackObject;
    POINTER64_TO_U32        pointer64ToU32;
    PSCSI_REQUEST_BLOCK     srb;
    ULONG                   flags;
    PULONG                  msgBuffer;
    PI2O_DEVICE_INFO        deviceInfo; 


    //
    // If there are any errors, get the original Srb and update it's status (and sense
    // data, if applicable).
    // 
    if ((flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((1,
                   "I2OMediaInfoCallBack: Failed MsgFlags\n"));

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));
    }

    if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
        DebugPrint((1,
                   "I2OMediaInfoCallBack: Failed ReqStatus\n"));

        srb = I2OHandleError(Msg, deviceExtension, 0);
        
    } else {
        PI2O_BSA_MEDIA_INFO_SCALAR  mediaInfo;
        LARGE_INTEGER lastSector;
        ULONG shift;

        //
        // The InitiatorContext field contains the low 32 bits of the callback pointer.
        //
        pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

        callbackObject = pointer64ToU32.Pointer;
        srb = callbackObject->Context;

        //
        // Get a pointer to the data.
        //
        msgBuffer = (PULONG)&Msg->TransferCount;
        msgBuffer++;
        msgBuffer++;
        mediaInfo = (PI2O_BSA_MEDIA_INFO_SCALAR)msgBuffer;

        //
        // Get the deviceInfo
        //
        deviceInfo = &deviceExtension->DeviceInfo[srb->TargetId];

        //
        // Copy the returned info to the srb data buffer.
        // First determine sector shift from the deviceInfo
        //
        WHICH_BIT(deviceInfo->BlockSize,shift);

        REVERSE_BYTES(&((PREAD_CAPACITY_DATA)srb->DataBuffer)->BytesPerBlock,
                      &mediaInfo->BlockSize);

        lastSector.LowPart  = mediaInfo->Capacity.LowPart;
        lastSector.HighPart = mediaInfo->Capacity.HighPart;

        deviceInfo->Capacity.QuadPart = lastSector.QuadPart;
        lastSector.QuadPart = lastSector.QuadPart >> shift;

        deviceInfo->BlockSize = mediaInfo->BlockSize;

        DebugPrint((1,
                   "\nI2OMediaInfo: SectorSize %x\n",
                   deviceInfo->BlockSize));
        DebugPrint((1,
                   "I2OMediaInfo: Capacity %x%x\n",
                   deviceInfo->Capacity.HighPart,
                   deviceInfo->Capacity.LowPart));

        REVERSE_BYTES(&((PREAD_CAPACITY_DATA)srb->DataBuffer)->LogicalBlockAddress,
                          &lastSector.LowPart);

        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }

    //
    // Complete the request.
    //
    I2OCompleteRequest(deviceExtension,
                       srb);
    return;
}


ULONG
I2ODriveCapacity(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine handles getting READ_CAPACITY data. If it's a fixed disk, the request
    is satisfied in-place by copying previously acquired data. If removable, a UTIL_PARAMS
    message is sent.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension.
    Srb - The SRB containing the READ_CAP CDB.
    
Return Value:

    SUCCESS - If fixed
    PENDING or ERROR - if removable.

--*/
{
    PIOP_CALLBACK_OBJECT        callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    POINTER64_TO_U32            pointer64ToU32;
    PI2O_DEVICE_INFO            deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    PI2O_LCT                    lct;
    struct {
        I2O_UTIL_PARAMS_GET_MESSAGE msg;
        I2O_PARAM_SCALAR_OPERATION  scalarBuffer;
    } s;
    PI2O_PARAM_SCALAR_OPERATION scalar;
    ULONG                       shift;
    LARGE_INTEGER               lastSector;

    //
    // If the device isn't removable, just move the block size
    // and capacity info to the srb buffer and return. This info
    // was captured during the "Inquiry".
    //
    if (!(deviceInfo->Characteristics & I2O_BSA_DEV_CAP_REMOVABLE_MEDIA)) {

        WHICH_BIT(deviceInfo->BlockSize,shift);

        REVERSE_BYTES(&((PREAD_CAPACITY_DATA)Srb->DataBuffer)->BytesPerBlock,
                      &deviceInfo->BlockSize);

        lastSector.LowPart  = deviceInfo->Capacity.LowPart;
        lastSector.HighPart = deviceInfo->Capacity.HighPart;

        lastSector.QuadPart = lastSector.QuadPart >> shift;

        REVERSE_BYTES(&((PREAD_CAPACITY_DATA)Srb->DataBuffer)->LogicalBlockAddress,
                          &lastSector.LowPart);

        return SRB_STATUS_SUCCESS;
    } else {

        DebugPrint((1,
                    "I2ODriveCapacity: Removable device - must get Blocksize and Capacity\n"));
        //
        // As this is removable media the values from DEVICE_INFO_SCALAR only
        // reflect the max, not the current, so send the media info request to get the
        // current values.
        //
        memset(&s,0, sizeof(I2O_UTIL_PARAMS_GET_MESSAGE) + sizeof(I2O_PARAM_SCALAR_OPERATION));

        //
        // Setup the message header.
        //
        s.msg.StdMessageFrame.VersionOffset    = I2O_VERSION_11 | I2O_PARAMS_SGL_VER_OFFSET;
        s.msg.StdMessageFrame.MsgFlags         = 0;
        s.msg.StdMessageFrame.TargetAddress    = deviceInfo->Lct.LocalTID;
        s.msg.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
        s.msg.StdMessageFrame.Function         = I2O_UTIL_PARAMS_GET;
        s.msg.StdMessageFrame.InitiatorContext = 0;

        callback->Callback.Signature       = CALLBACK_OBJECT_SIG;
        callback->Callback.CallbackAddress = I2OMediaInfoCallBack;
        callback->Callback.CallbackContext = DeviceExtension;
        callback->Context = Srb;

        pointer64ToU32.Pointer64 = 0;
        pointer64ToU32.Pointer   = callback;

        s.msg.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
        s.msg.TransactionContext = pointer64ToU32.u.HighPart;

        s.msg.StdMessageFrame.MessageSize = (sizeof(I2O_UTIL_PARAMS_GET_MESSAGE) -
                                           sizeof(I2O_SG_ELEMENT) +
                                           sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT) +
                                           sizeof(I2O_PARAM_SCALAR_OPERATION)) >> 2;

        s.msg.SGL.u.ImmediateData.FlagsCount.Count = sizeof(I2O_PARAM_SCALAR_OPERATION);
        s.msg.SGL.u.ImmediateData.FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                                   I2O_SGL_FLAGS_END_OF_BUFFER |
                                                   I2O_SGL_FLAGS_IMMEDIATE_DATA_ELEMENT;

        scalar = (PI2O_PARAM_SCALAR_OPERATION)((PUCHAR)&s.msg.SGL +
                                               sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT));
        scalar->OpList.OperationCount = 1;
        scalar->OpBlock.Operation     = I2O_PARAMS_OPERATION_FIELD_GET;
        scalar->OpBlock.FieldCount    = (USHORT) -1;
        scalar->OpBlock.FieldIdx[0]   = 0;
        scalar->OpBlock.GroupNumber = I2O_BSA_MEDIA_INFO_GROUP_NO;

        //
        // Send the message.
        //
        if (!I2OSendMessage(DeviceExtension,
                            (PI2O_MESSAGE_FRAME)&s.msg)) {
            DebugPrint((1,
                       "I2ODriveCapacity: Error sending message\n"));
            return SRB_STATUS_ERROR;
        }

        return SRB_STATUS_PENDING;
    }
}


VOID
I2OInquiryCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    This is the callback for INQUIRY requests. It uses info from a UTIL_PARAMS_GET message
    to build the inquiry data for the device.
    
Arguments:
    
    Msg - The reply message frame.
    DeviceContext - This miniport's HwDeviceExtension
    
Return Value:

    NOTHING (Srb DataBuffer is updated with inquiry data, Srb Status is set, and
             Srb is completed).

--*/
{
    PDEVICE_EXTENSION       deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT    callbackObject;
    POINTER64_TO_U32        pointer64ToU32;
    PSCSI_REQUEST_BLOCK     srb;
    PINQUIRYDATA            inquiryData;
    ULONG                   flags;
    PULONG                  msgBuffer;

    //
    // Check for any errors, and extract the Srb.
    // 
    if ((flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((1,
                   "I2OInquiryCallBack: Failed MsgFlags\n"));

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));

    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
        DebugPrint((1,
                   "I2OInquiryCallBack: Failed ReqStatus\n"));
        srb = I2OHandleError(Msg, deviceExtension, 0);
        
    } else {
        PI2O_BSA_DEVICE_INFO_SCALAR  deviceInfo;
        PI2O_DEVICE_INFO lctInfo;
        ULONG i;

        //
        // The InitiatorContext field contains the low 32 bits of the callback pointer.
        //
        pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

        //
        // Get our callback and context (the original SRB).
        //
        callbackObject = pointer64ToU32.Pointer;
        srb = callbackObject->Context;
        inquiryData = srb->DataBuffer;

        //
        // Get the deviceInfo.
        //
        lctInfo = &deviceExtension->DeviceInfo[srb->TargetId];
        
        //
        // Get a pointer to the data.
        //
        msgBuffer = (PULONG)&Msg->TransferCount;
        msgBuffer++;
        msgBuffer++;
        deviceInfo = (PI2O_BSA_DEVICE_INFO_SCALAR)msgBuffer;

        memset(inquiryData, 0, srb->DataTransferLength);

        //
        // Setup the inquiry data buffer based on the info returned.
        //
        inquiryData->DeviceType = deviceInfo->DeviceType;
        inquiryData->DeviceTypeQualifier = 0;

        inquiryData->RemovableMedia = (UCHAR)(deviceInfo->DeviceCapabilitySupport &
                                              I2O_BSA_DEV_CAP_REMOVABLE_MEDIA);
        //
        // Claim that the devices do tagged queueing.
        //
        inquiryData->CommandQueue = 1;

        for (i = 0; i < 8; i++) {
            inquiryData->VendorId[i] = ' ';
        }

        //
        // Use 'intel' for now, should key off of PCI Ids
        // to determine whose name should go here.
        //
        inquiryData->VendorId[0] = 'I';
        inquiryData->VendorId[1] = 'n';
        inquiryData->VendorId[2] = 't';
        inquiryData->VendorId[3] = 'e';
        inquiryData->VendorId[4] = 'l';
        
        for (i = 0; i < 16; i++) {
            inquiryData->ProductId[i] = ' ';
        }

        //
        // For some reason, there wasn't any volume name
        // info. Make some stuff up.
        //
        inquiryData->ProductId[0] = 'I';
        inquiryData->ProductId[1] = '2';
        inquiryData->ProductId[2] = 'O';
        inquiryData->ProductId[3] = ' ';
        inquiryData->ProductId[4] = 'D';
        inquiryData->ProductId[5] = 'i';
        inquiryData->ProductId[6] = 's';
        inquiryData->ProductId[7] = 'k';

        inquiryData->ProductRevisionLevel[0] = '2';
        inquiryData->ProductRevisionLevel[1] = '.';
        inquiryData->ProductRevisionLevel[2] = '0';
        inquiryData->ProductRevisionLevel[3] = '0';

        //
        // Save off the capacity info for GET_CAPACITY, along with the characteristics.
        //
        lctInfo->BlockSize = deviceInfo->BlockSize;
        lctInfo->Capacity.HighPart = deviceInfo->DeviceCapacity.HighPart;

        //
        // Remove one sector worth as this isn't the same as 'last sector'.
        //
        if (deviceInfo->DeviceCapacity.LowPart < lctInfo->BlockSize) {
            lctInfo->Capacity.HighPart--;
        }
        
        lctInfo->Capacity.LowPart = deviceInfo->DeviceCapacity.LowPart - 
                                        lctInfo->BlockSize;
        lctInfo->Characteristics = deviceInfo->DeviceCapabilitySupport;

        DebugPrint((1,
                   "I2OInquiryCallBack: TargetId %x: \n",
                   srb->TargetId));
        DebugPrint((1,
                   "\tBlockSize %x\n",
                   lctInfo->BlockSize));
        DebugPrint((1,
                   "\tCapacity %x%x\n",
                   lctInfo->Capacity.HighPart,
                   lctInfo->Capacity.LowPart));
        DebugPrint((1,
                   "\tCharacteristics %x\n",
                   lctInfo->Characteristics));
        DebugPrint((1,
                   "\tDevice Flags %x\n",
                   lctInfo->DeviceFlags));
        DebugPrint((1,
                   "\tDevice State %x\n",
                   lctInfo->DeviceState));

        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }

    //
    // Complete the request.
    //
    I2OCompleteRequest(deviceExtension,srb);

    return;
}


ULONG
I2OInquiry(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine handles INQUIRY requests. It will ensure that the device represented
    by TargetId exists, then send a UTIL_PARAMS_GET message to gather more information
    about the device.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The SRB containing the INQUIRY CDB
    
Return Value:

    SELTO - If no device.
    ERROR -If SendMsg fails
    PENDING - If msg is successfully sent.
    
--*/
{
    PIOP_CALLBACK_OBJECT        callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    PI2O_LCT                    lct = DeviceExtension->Lct;
    PI2O_PARAM_SCALAR_OPERATION scalar;
    I2O_UTIL_PARAMS_GET_MESSAGE msg;
    I2O_PARAM_SCALAR_OPERATION  scalar2;
    PI2O_DEVICE_INFO            deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    POINTER64_TO_U32            pointer64ToU32;
    ULONG entry;

    //
    // Time out requests to any device that are no visible to the OS or
    // if the Lun != 0.
    //  
    if ((!(deviceInfo->DeviceFlags & DFLAGS_OS_CAN_CLAIM)) || (Srb->Lun!=0)) {
        DebugPrint((2,
                   "I2OInquiry: Returning Timeout for Scsi Id 0x%x Lun 0x%x\n",
                        Srb->TargetId,Srb->Lun));
        return SRB_STATUS_SELECTION_TIMEOUT;
    }
    DebugPrint((1,
                "I2OInquiry: Issuing Inquiry for Scsi Id 0x%x Lun 0x%x Tid 0x%x\n",
                 Srb->TargetId,Srb->Lun,deviceInfo->Lct.LocalTID));

    //
    // Copy the entry into the deviceInfo.
    //
    entry = deviceInfo->LCTIndex;
    StorPortMoveMemory(&deviceInfo->Lct,
                       &lct->LCTEntry[entry],
                       sizeof(I2O_LCT_ENTRY));
        
    memset(&msg,0, sizeof(I2O_UTIL_PARAMS_GET_MESSAGE));

    //
    // Setup the message header.
    //
    msg.StdMessageFrame.VersionOffset    = I2O_VERSION_11 | I2O_PARAMS_SGL_VER_OFFSET;
    msg.StdMessageFrame.MsgFlags         = 0;
    msg.StdMessageFrame.TargetAddress    = deviceInfo->Lct.LocalTID;
    msg.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    msg.StdMessageFrame.Function         = I2O_UTIL_PARAMS_GET;
    msg.StdMessageFrame.InitiatorContext = 0;

    //
    // Indicate the call back object, address, and contexts
    //
    callback->Callback.Signature = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackAddress = I2OInquiryCallBack;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callback;

    msg.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg.TransactionContext = pointer64ToU32.u.HighPart;

    msg.StdMessageFrame.MessageSize = (sizeof(I2O_UTIL_PARAMS_GET_MESSAGE) -
                                       sizeof(I2O_SG_ELEMENT) +
                                       sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT) +
                                       sizeof(I2O_PARAM_SCALAR_OPERATION)) >> 2;

    msg.SGL.u.ImmediateData.FlagsCount.Count = sizeof(I2O_PARAM_SCALAR_OPERATION);
    msg.SGL.u.ImmediateData.FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                                I2O_SGL_FLAGS_END_OF_BUFFER |
                                                I2O_SGL_FLAGS_IMMEDIATE_DATA_ELEMENT;

    //
    // Initialize the scalar operation portion of the message which is contained in the SGL.
    // The scalar params that are returned by the DDM will be contained in the reply message.
    //
    scalar = (PI2O_PARAM_SCALAR_OPERATION)((PUCHAR)&msg.SGL +
                                           sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT));
    scalar->OpList.OperationCount   = 1;
    scalar->OpBlock.Operation       = I2O_PARAMS_OPERATION_FIELD_GET;
    scalar->OpBlock.FieldCount      = (USHORT) -1; // Get all fields in the group
    scalar->OpBlock.FieldIdx[0]     = 0;
    scalar->OpBlock.GroupNumber     = I2O_BSA_DEVICE_INFO_GROUP_NO;

    //
    // Send the message.
    //
    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&msg)) {
        DebugPrint((1,
                   "I2OInquiry: Error sending message\n"));
        return SRB_STATUS_ERROR;
    }

    return SRB_STATUS_PENDING;
}


VOID
I2OVerifyCompletion(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )

/*++

Routine Description:

    Process a verify completion from a block storage device.  This routine
    handles all the aspects of completing the request.

Arguments:

    Msg             - message describing the completed request
    DeviceContext   - device extension for the device this request was issued against

Return Value:

    None

--*/
{
    PDEVICE_EXTENSION   deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT  callbackObject;
    ULONG               flags;
    ULONG               status;
    PSCSI_REQUEST_BLOCK srb;
    POINTER64_TO_U32    pointer64ToU32;
    
    //
    // Check for any error status.
    //
    flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags;

    DebugPrint((2,
               "I2OVerifyCompletion: Msg %x. Flags %x\n",
               Msg,
               flags));


    if (flags & I2O_MESSAGE_FLAGS_FAIL) {

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));

    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {

        srb = I2OHandleError(Msg, deviceExtension, 0);

    } else {

        if (Msg->BsaReplyFrame.ReqStatus == I2O_REPLY_STATUS_PROGRESS_REPORT) {
            DebugPrint((1,
                        "I2OVerifyCompletion: Verify Percent Complete (%lu)\n",
                        ((PI2O_BSA_PROGRESS_REPLY_MESSAGE_FRAME)Msg)->PercentComplete));
            return;
                          
        }
        pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

        callbackObject = pointer64ToU32.Pointer;
        srb  = callbackObject->Context;

        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }

    I2OCompleteRequest(deviceExtension,
                       srb);
}


ULONG
I2OVerify(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine handles SCSIOP_VERIFY requests by sending a MEDIA_VERIFY msg. for 
    the requested blocks.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The SRB containing the VERIFY request.
    
Return Value:

    PENDING or ERROR, if SendMsg failed.

--*/

{
    PIOP_CALLBACK_OBJECT            callback = &((PSRB_EXTENSION)Srb->SrbExtension)->CallBack;
    PI2O_DEVICE_INFO                deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    I2O_BSA_MEDIA_VERIFY_MESSAGE    msg;
    LARGE_INTEGER                   blockOffset;
    POINTER64_TO_U32                pointer64ToU32;
    ULONG                           status;
    ULONG                           tempLBA, tempBlocks;
    
    memset(&msg, 0, sizeof(I2O_BSA_MEDIA_VERIFY));
    
    msg.StdMessageFrame.VersionOffset    = I2O_VERSION_11 | I2O_READ_SGL_VER_OFFSET;
    msg.StdMessageFrame.MsgFlags         = 0;
    msg.StdMessageFrame.MessageSize      = (sizeof(I2O_BSA_MEDIA_VERIFY_MESSAGE) / 4);
    msg.StdMessageFrame.TargetAddress    = deviceInfo->Lct.LocalTID;
    msg.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    msg.StdMessageFrame.Function = I2O_BSA_MEDIA_VERIFY;
   
    //
    // Build the callback object.
    // 
    callback->Callback.Signature       = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackAddress = I2OVerifyCompletion;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callback;

    msg.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg.TransactionContext               = pointer64ToU32.u.HighPart;
    
    msg.ControlFlags = 0x40;
    msg.TimeMultiplier = (UCHAR)Srb->TimeOutValue;

    //
    // Set the range for the verify.
    // 
    msg.ByteCount = ((PCDB)Srb->Cdb)->CDB10.TransferBlocksMsb << 8 |
                    ((PCDB)Srb->Cdb)->CDB10.TransferBlocksLsb;
    tempBlocks = msg.ByteCount;
    msg.ByteCount *= deviceInfo->BlockSize;

    //
    // Set the starting offset.
    // 
    blockOffset.HighPart = 0;
    blockOffset.LowPart = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3 |
                          ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2 << 8 |
                          ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1 << 16 |
                          ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 << 24;
    
    tempLBA = blockOffset.LowPart;
    
    //
    // Convert to byte offset.
    //
    blockOffset.QuadPart *= deviceInfo->BlockSize;
    msg.LogicalByteAddress.HighPart = blockOffset.HighPart;
    msg.LogicalByteAddress.LowPart = blockOffset.LowPart;
    
    DebugPrint((1,
                "I2OVerify: Offset (%I64x), bytes (%x)\n",
                blockOffset.QuadPart,
                msg.ByteCount));
    DebugPrint((1,
                "LBA (%x), NumberBlocks (%x)\n",
                tempLBA,
                tempBlocks));
                
                
    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&msg)) {

        DebugPrint((1,
                   "I2OVerify: Request submission failed.\n"));

        status = SRB_STATUS_ERROR;

    } else {
        status = SRB_STATUS_PENDING;
    }

    return status;
}    


VOID
I2OBuildReadWrite(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine builds an I2O Message (BSA Read/Write)
    from the Srb and stores the message in the Srb Extension. The request actually
    gets sent in StartIO.

Arguments:

    DeviceExtension - Miniport's DeviceExt.
    Srb - Srb describing the request.
    
Return Value:
   
    SRB_STATUS_PENDING, SRB_STATUS_ERROR - based on whether the message
    could be sent.

--*/
{
    PIOP_CALLBACK_OBJECT    callback = &((PSRB_EXTENSION)Srb->SrbExtension)->CallBack;
    PI2O_DEVICE_INFO        deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    POINTER64_TO_U32        pointer64ToU32;
    PI2O_SG_ELEMENT         sgList;
    ULONG                   transferPages;
    ULONG                   status;
    PCDB                    cdb = (PCDB)&Srb->Cdb[0];
    PREAD_WRITE_MSG         msg = &((PSRB_EXTENSION)Srb->SrbExtension)->RWMsg;
    LARGE_INTEGER           blockOffset;

    //
    // Build the block storage read/write request message.
    //
    if (Srb->Cdb[0] == SCSIOP_READ) {

        msg->Read.StdMessageFrame.VersionOffset     = I2O_VERSION_11 | I2O_READ_SGL_VER_OFFSET;
        msg->Read.StdMessageFrame.MsgFlags          = 0;
        msg->Read.StdMessageFrame.MessageSize       = (sizeof(I2O_BSA_READ_MESSAGE) -
                                                      sizeof(I2O_SG_ELEMENT) +
                                                      I2O_SG_ELEMENT_OVERHEAD)/4;
        msg->Read.StdMessageFrame.TargetAddress     = deviceInfo->Lct.LocalTID;
        msg->Read.StdMessageFrame.InitiatorAddress  = I2O_HOST_TID;
        msg->Read.StdMessageFrame.Function          = I2O_BSA_BLOCK_READ;
        msg->Read.StdMessageFrame.InitiatorContext  = 0;
        msg->Read.FetchAhead                        = 0;

        msg->Read.ControlFlags = I2O_BSA_RD_FLAG_CACHE_READ;
        callback->Callback.CallbackAddress = I2OReadCompletion;

    } else {

        msg->Write.StdMessageFrame.VersionOffset    = I2O_VERSION_11 | I2O_WRITE_SGL_VER_OFFSET;
        msg->Write.StdMessageFrame.MsgFlags         = 0;
        msg->Write.StdMessageFrame.MessageSize      = (sizeof(I2O_BSA_WRITE_MESSAGE) -
                                                      sizeof(I2O_SG_ELEMENT) +
                                                      I2O_SG_ELEMENT_OVERHEAD)/4;
        msg->Write.StdMessageFrame.TargetAddress    = deviceInfo->Lct.LocalTID;
        msg->Write.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
        msg->Write.StdMessageFrame.Function         = I2O_BSA_BLOCK_WRITE;
        msg->Write.StdMessageFrame.InitiatorContext = 0;
        msg->Write.Reserved = 0;
        
        if (cdb->CDB10.ForceUnitAccess == TRUE) {

            //
            // This request must be on the media.
            //
            msg->Write.ControlFlags = I2O_BSA_WR_FLAG_WRITE_THRU;

        } else {

            //
            // It's OK to write cache this.
            //
            msg->Write.ControlFlags = I2O_BSA_WR_FLAG_WRITE_TO;
        }

        callback->Callback.CallbackAddress = I2OWriteCompletion;
    }

    msg->Read.TransferByteCount = Srb->DataTransferLength;
    msg->Read.TimeMultiplier    = (UCHAR)Srb->TimeOutValue;

    //
    // Get the block offset.
    //
    blockOffset.HighPart = 0;
    blockOffset.LowPart = ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte3 |
                          ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte2 << 8 |
                          ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte1 << 16 |
                          ((PCDB)Srb->Cdb)->CDB10.LogicalBlockByte0 << 24;

    //
    // Convert to byte offset, as I2O uses byte offsets vs. block.
    //
    blockOffset.QuadPart *= deviceInfo->BlockSize;
    msg->Read.LogicalByteAddress.HighPart = blockOffset.HighPart;
    msg->Read.LogicalByteAddress.LowPart = blockOffset.LowPart;

    //
    // Finish setting up the callback object.
    //
    callback->Callback.Signature = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callback;

    msg->Read.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg->Read.TransactionContext  = pointer64ToU32.u.HighPart;

    //
    // Build the scatter/gather list in the message frame.
    //
    sgList = &(msg->Read.SGL);
    sgList->u.Page[0].FlagsCount.Count = Srb->DataTransferLength;
    sgList->u.Page[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                         I2O_SGL_FLAGS_END_OF_BUFFER |
                                         I2O_SGL_FLAGS_PAGE_LIST_ADDRESS_ELEMENT;

    if (Srb->Cdb[0] == SCSIOP_WRITE) {
        sgList->u.Page[0].FlagsCount.Flags |= I2O_SGL_FLAGS_DIR;
    }

    transferPages = I2OBuildSGList(DeviceExtension,
                                   Srb,
                                   sgList);

    msg->Read.StdMessageFrame.MessageSize += (USHORT)transferPages;
    return;
}


ULONG
I2OSendReadWrite(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine sends an I2O Message (BSA Read/Write) built
    previously by I2OBuildIo.

Arguments:

    DeviceExtension - Miniport's DeviceExt.
    Srb - Srb describing the request.
    
Return Value:
   
    SRB_STATUS_PENDING, SRB_STATUS_ERROR - based on whether the message
    could be sent.

--*/
{
    ULONG                   status;
    PREAD_WRITE_MSG         msg = &((PSRB_EXTENSION)Srb->SrbExtension)->RWMsg;

#if DBG
    if (Srb->Cdb[0] == SCSIOP_READ) {
        DebugPrint((2,
                   "Rd %x %x\n",
                   msg->Read.LogicalByteAddress.LowPart,
                   msg->Read.TransferByteCount));
    } else {
        DebugPrint((2,
                   "Wr %x %x\n",
                   msg->Read.LogicalByteAddress.LowPart,
                   msg->Read.TransferByteCount));
    }

    DebugPrint((2,
               "Msg size %x ",
               msg->Read.StdMessageFrame.MessageSize));
    DebugPrint((2,
               "Control Flags %x\n",
               msg->Read.ControlFlags));
#endif

    // 
    // NOTE: I2OStartIo assumes that only BUSY and PENDING are returned
    // from this function. Change this and StartIo needs to change as well.
    //
    //
    // Send the request message frame.
    //
    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)msg)) {
        //
        // Return SRB_STATUS_BUSY when the IOP does not have 
        // an available inbound message frame.  StorPort will
        // retry this request later.
        //
        status = SRB_STATUS_BUSY;

        //
        // Tell StorPort that the adapter is busy.  Do not
        // start another request until 10 requests have completed.
        // This gives the miniport/StorPort the ability to burst 10 requests
        // once StorPort starts the next IO (assuming the controller is
        // kept busy with outstanding IO). 
        //
        StorPortBusy(DeviceExtension,10);

    } else {
        status = SRB_STATUS_PENDING;
    }

    return status;
}


ULONG
I2OBuildSGList(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PI2O_SG_ELEMENT  SgList
    )
/*++

Routine Description:

    This routine builds a Page List Element SG List for reads and writes.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The R/W SRB
    SgList - Storage for the SG List
    
Return Value:

    The number of pages mapped.

--*/
{
    PULONG sgAddress = &(SgList->u.Simple[0].PhysicalAddress);
    ULONG  physicalAddress;
    ULONG  transferPages = 0;
    ULONG  length;
    ULONG  currentLength;
    ULONG  nonAlignedData;
    PSTOR_SCATTER_GATHER_LIST SPSgl;
    ULONG  i = 0;

    //
    // Get the SGList from Storport.
    //
    SPSgl = StorPortGetScatterGatherList(DeviceExtension, Srb);

    //
    // Walk the Storport SG list and build the I2O SG List.
    //
    while ((SPSgl) && (SPSgl->NumberOfElements > i)) {
        //
        // Get the physical address of the current data pointer.
        //
        physicalAddress = SPSgl->List[i].PhysicalAddress.LowPart;
        length          = SPSgl->List[i++].Length;

        currentLength = length;

        //
        // After the first page, the rest need to be page aligned.
        // Figure out the data length leading up to the first page
        // boundary.
        //
        nonAlignedData = PAGE_SIZE - (physicalAddress & (PAGE_SIZE - 1));
        if (nonAlignedData) {
            if (nonAlignedData > currentLength) {
                nonAlignedData = currentLength;
            }

            *sgAddress = physicalAddress;
            sgAddress++;
            physicalAddress += nonAlignedData;
            currentLength -= nonAlignedData;
            transferPages++;
        }

        //
        // Now the rest are page aligned.
        //
        while (currentLength >= PAGE_SIZE) {
            *sgAddress = physicalAddress;

            sgAddress++;
            physicalAddress += PAGE_SIZE;
            currentLength -= PAGE_SIZE;
            transferPages++;
        }

        //
        // Finish off the remaining data.
        //
        if (currentLength) {
            *sgAddress = physicalAddress;
            transferPages++;
        }
    }

    return transferPages;
}


VOID
I2OShutdownFlushCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID  CallbackContext
    )
/*++

Routine Description:

    This is the call-back for the ShutdownFlush requests.

Arguments:

    Msg - The reply msg frame.
    CallbackContext - This miniport's HwDeviceExtension.
    
Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION  deviceExtension = CallbackContext;
    PIOP_CALLBACK_OBJECT callbackObject;
    POINTER64_TO_U32     pointer64ToU32;
    PSCSI_REQUEST_BLOCK  srb;
    ULONG                status = SRB_STATUS_SUCCESS;
    ULONG                flags;
    PULONG               msgBuffer;

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer
    // and the TransactionContext (for NT64) contains the high 32 bits.  Contained
    // in the callback object is the syncReq pointer associated with the IO completion.
    //
    pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
    pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;


    if ((flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((1,
                   "I2OShutdownFlushCallBack: Failed MsgFlags\n"));

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));
    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
        DebugPrint((1,
                   "I2OShutdownFlushCallBack: Failed ReqStatus\n"));

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             0);
    } else {

        callbackObject = pointer64ToU32.Pointer;
        srb = callbackObject->Context;
            
        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }

    if (((PI2O_SINGLE_REPLY_MESSAGE_FRAME) Msg)->StdMessageFrame.Function == I2O_EXEC_SYS_QUIESCE) {
        DebugPrint((1,
                   "I2OShutdownFlushCallBack: Sys Quiesce Message Successful!!\n"));
    }

    //
    // Complete the requst.
    // 
    I2OCompleteRequest(deviceExtension,
                       srb);
    return;
}


ULONG
I2OShutdownFlush(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine is called in response to a SRB_FUNCTION_FLUSH or SCSIOP_SYNC_CACHE. It
    will send a CACHE_FLUSH msg to the IOP async.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - the Srb containing the FLUSH or SYNC_CACHE request.
    
Return Value:

    PENDING or ERROR, if SendMsg failed.

--*/

{
    PIOP_CALLBACK_OBJECT callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    PI2O_DEVICE_INFO     deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    POINTER64_TO_U32     pointer64ToU32;
    I2O_BSA_CACHE_FLUSH_MESSAGE flush;
    ULONG status = SRB_STATUS_PENDING;

    //
    // Build the flush message.
    // 
    flush.StdMessageFrame.VersionOffset= I2O_VERSION_11;
    flush.StdMessageFrame.MsgFlags = 0;
    flush.StdMessageFrame.MessageSize = sizeof(I2O_BSA_CACHE_FLUSH_MESSAGE)/4;
    flush.StdMessageFrame.TargetAddress = deviceInfo->Lct.LocalTID;
    flush.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    flush.StdMessageFrame.Function = I2O_BSA_CACHE_FLUSH;
    flush.StdMessageFrame.InitiatorContext = 0;


    //
    // Build the call-back object.
    // 
    callback->Callback.Signature       = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackAddress = I2OShutdownFlushCallBack;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callback;

    flush.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    flush.TransactionContext = pointer64ToU32.u.HighPart;

    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&flush)) {
        DebugPrint((1,
                   "I2OShutdownFlush: Failure sending message\n"));
        status = SRB_STATUS_ERROR;
    }
    return status;
}


ULONG
I2OShutdownFlushSync(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine will flush the IOP's cache synchronously by sending a CACHE_FLUSH message,
    then waiting on completion.

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The shutdown SRB
    
Return Value:

    SRB_STATUS_SUCCESS or ERROR, if SendMessage fails.

--*/
{
    PI2O_DEVICE_INFO            deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    I2O_BSA_CACHE_FLUSH_MESSAGE flush;
    ULONG status = SRB_STATUS_ERROR;
    ULONG i;
    POINTER64_TO_U32 pointer64ToU32;
    PIOP_CALLBACK_OBJECT callBackObject;

    callBackObject = &DeviceExtension->InitCallback;
    callBackObject->Context = &DeviceExtension->CallBackComplete;

    DebugPrint((3,
                "I2OShutdownFlushSync: Entering ShutdownFlush\n"));

    memset(&flush, 0 , sizeof(I2O_BSA_CACHE_FLUSH_MESSAGE));
        
    flush.StdMessageFrame.VersionOffset= I2O_VERSION_11;
    flush.StdMessageFrame.MsgFlags = 0;
    flush.StdMessageFrame.MessageSize = sizeof(I2O_BSA_CACHE_FLUSH_MESSAGE)/4;
    flush.StdMessageFrame.TargetAddress = deviceInfo->Lct.LocalTID;
    flush.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    flush.StdMessageFrame.Function = I2O_BSA_CACHE_FLUSH;
    flush.StdMessageFrame.InitiatorContext = 0;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callBackObject;

    flush.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    flush.TransactionContext = pointer64ToU32.u.HighPart;
    DeviceExtension->CallBackComplete = 0;

    DebugPrint((1,
                  "I2OShutdownFlushSync: Send Synch Flush to TID 0x%x\n",
                   deviceInfo->Lct.LocalTID));
                                            
    if (I2OSendMessage(DeviceExtension,(PI2O_MESSAGE_FRAME)&flush)) {

        //
        // Check for completion.
        //
        if (I2OGetCallBackCompletion(DeviceExtension,
                                     100000)) {
            status = SRB_STATUS_SUCCESS;
        } 
    }    
    DebugPrint((1,
             "I2OShutdownFlushSync: Flush TID 0x%x returned status %x\n",
              deviceInfo->Lct.LocalTID,
              status));

    return status;
}


ULONG
I2OModeSense(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This will build a fake mode sense buffer, based on the page code.
    Only the cache page and all pages are supported, though if page 0x00
    is sent the block header and descriptor will be returned.
    
Arguments:

    DeviceExtension - Our per device info.
    Srb - The request.
    
Return Value:

    Success, or invalid request if it's an unsupported mode page.

--*/
{
    PMODE_CACHING_PAGE cachePage;
    PCDB cdb = (PCDB)&Srb->Cdb[0];
    PMODE_PARAMETER_HEADER header;
    PMODE_PARAMETER_BLOCK blockDescriptor;
    ULONG status = SRB_STATUS_INVALID_REQUEST;
    ULONG requestLength;

    //
    // Capture the requested transfer length, as the class
    // drivers will send down mode_sense commands for only the
    // header sometimes.
    // 
    requestLength = Srb->DataTransferLength;

    //
    // If it's all pages or cache page.
    //
    if ((cdb->MODE_SENSE.PageCode == MODE_PAGE_CACHING) ||
        (cdb->MODE_SENSE.PageCode == MODE_SENSE_RETURN_ALL)) {
        
        //
        // Build the mode header. Note the BlockDescriptorLength
        // is zero. Even if Dbd was not set, this isn't an error. 
        // 
        header = Srb->DataBuffer;

        // 
        // Data length is currently zero. If there is room
        // for the page(s) themselves, it will get updated.
        // 
        header->ModeDataLength = 0; 
        header->MediumType = 0;
        header->DeviceSpecificParameter = MODE_DSP_FUA_SUPPORTED;
        header->BlockDescriptorLength = 0;

        //
        // Check to see whether the caller just wants the
        // header, or if they actually want the data.
        // 
        requestLength -= sizeof(MODE_PARAMETER_HEADER);
        if (requestLength >= sizeof(MODE_CACHING_PAGE)) {

            // 
            // Data length is (everything - the ModeDataLength byte).
            // 
            header->ModeDataLength = sizeof(MODE_CACHING_PAGE) + 3;

            //
            // Set-up the pointer in order to build the cache page.
            // 
            cachePage = (PMODE_CACHING_PAGE)header;
            (ULONG_PTR)cachePage += sizeof(MODE_PARAMETER_HEADER);

            cachePage->PageCode = MODE_PAGE_CACHING;
            cachePage->Reserved = 0;
            cachePage->PageSavable = 0;
            cachePage->PageLength = 0xA;
            cachePage->ReadDisableCache = 0;
            cachePage->MultiplicationFactor = 0;

            //
            // TODO: Make this match the returned value in the CacheControl scalar
            //
            cachePage->WriteCacheEnable = 1;
            cachePage->Reserved2 = 0;
            cachePage->WriteRetensionPriority = 0;
            cachePage->ReadRetensionPriority = 0;
            cachePage->DisablePrefetchTransfer[0] = 0;
            cachePage->DisablePrefetchTransfer[1] = 0;
            cachePage->MinimumPrefetch[0] = 0;
            cachePage->MinimumPrefetch[1] = 0;
            cachePage->MaximumPrefetch[0] = 0;
            cachePage->MaximumPrefetch[1] = 0;
            cachePage->MaximumPrefetchCeiling[0] = 0;
            cachePage->MaximumPrefetchCeiling[1] = 0;

            Srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER) +
                                      sizeof(MODE_CACHING_PAGE);

        } else {
            Srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER);
        }    
        status = SRB_STATUS_SUCCESS;
        
    } else if (cdb->MODE_SENSE.PageCode == 0) {
        
        //
        // This indicates that the header and block descriptor
        // are needed, but no mode page.
        //
        header = Srb->DataBuffer;
        
        header->ModeDataLength = 0; 
        header->MediumType = 0;
        header->DeviceSpecificParameter = MODE_DSP_FUA_SUPPORTED;
        header->BlockDescriptorLength = 0; 

        requestLength -= sizeof(MODE_PARAMETER_HEADER);
        if (requestLength >= sizeof(MODE_PARAMETER_BLOCK)) {

            //
            // Update the header to include the BlockDescriptor length.
            // 
            header->BlockDescriptorLength = sizeof(MODE_PARAMETER_BLOCK);

            //
            // Move to the actual block descriptor in the buffer.
            // 
            blockDescriptor = (PMODE_PARAMETER_BLOCK)header;
            (ULONG_PTR)blockDescriptor += sizeof(MODE_PARAMETER_HEADER);

            //
            // Indicate 'default' media type.
            //
            blockDescriptor->DensityCode = 0;

            //
            // Indicate that this applies to ALL blocks.
            //
            blockDescriptor->NumberOfBlocks[0] = 0;
            blockDescriptor->NumberOfBlocks[1] = 0;
            blockDescriptor->NumberOfBlocks[2] = 0;
            blockDescriptor->Reserved = 0;

            //
            // BUGBUG - should be based on mediaInfo.
            //
            blockDescriptor->BlockLength[0] = 0;
            blockDescriptor->BlockLength[1] = 1;
            blockDescriptor->BlockLength[2] = 0;

            Srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER) +
                                      sizeof(MODE_PARAMETER_BLOCK);
        } else {
            Srb->DataTransferLength = sizeof(MODE_PARAMETER_HEADER);
        }

        status = SRB_STATUS_SUCCESS;
        
    } else {

        //
        // Not supported.
        //
        status = SRB_STATUS_INVALID_REQUEST;
    }    

    return status;

}


ULONG
I2OBuildControlSGList(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PVOID DataBuffer,
    IN PI2O_SG_ELEMENT SgList,
    IN ULONG TransferLength
    )
/*++

Routine Description:

    This routine is used to build the SG List for MINIPORT IOCTLs.

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The SrbIOControl
    DataBuffer - The buffer to map
    SgList - Storage for the SGL
    TransferLength - Size of buffer.
    
Return Value:

    Number of pages mapped.

--*/
{
    PULONG sgAddress;
    PVOID  dataPointer;
    ULONG  physicalAddress;
    ULONG  bytesLeft;
    ULONG  transferPages = 0;
    ULONG  length;
    ULONG  currentLength;
    ULONG  nonAlignedData;

    dataPointer = DataBuffer;
    bytesLeft = TransferLength;
    sgAddress = &(SgList->u.Simple[0].PhysicalAddress);


    do {
        //
        // Get the physical address of the current data pointer.
        //
        physicalAddress =
            StorPortConvertPhysicalAddressToUlong(
                StorPortGetPhysicalAddress(DeviceExtension,
                                           Srb,
                                           dataPointer,
                                           &length));
        if (length > bytesLeft) {
            length = bytesLeft;
        }

        currentLength = length;

        //
        // After the first page, the rest need to be page aligned.
        // Figure out the data length leading up to the first page
        // boundary.
        //
        nonAlignedData = PAGE_SIZE - (physicalAddress & (PAGE_SIZE - 1));
        if (nonAlignedData) {
            if (nonAlignedData > currentLength) {
                nonAlignedData = currentLength;
            }

            *sgAddress = physicalAddress;
            sgAddress++;
            physicalAddress += nonAlignedData;
            currentLength -= nonAlignedData;
            transferPages++;
        }
        //
        // Now the rest are page aligned.
        //
        while (currentLength >= PAGE_SIZE) {
            *sgAddress = physicalAddress;
            sgAddress++;
            physicalAddress += PAGE_SIZE;
            currentLength -= PAGE_SIZE;
            transferPages++;
        }

        //
        // Finish off the remaining data.
        //
        if (currentLength) {
            *sgAddress = physicalAddress;
            transferPages++;
        }

        bytesLeft -= length;
        (PUCHAR)dataPointer += length;
    } while (bytesLeft);

    DebugPrint((2,
               "I2OBuildControlSG: sgList %x, transferPages %x\n",
               SgList,
               transferPages));

    return transferPages;
}


VOID
I2OConfigDialogCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )

/*++

Routine Description:

    Process a passthrough request completion. 

Arguments:

    Msg             - message describing the completed request
    DeviceContext   - device extension for the device this request was issued against

Return Value:

    None

--*/
{
    PDEVICE_EXTENSION   deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT  callbackObject;
    ULONG               flags;
    PSCSI_REQUEST_BLOCK srb;
    POINTER64_TO_U32    pointer64ToU32;
    PSRB_IO_CONTROL srbIoControl;
    PI2O_PARAMS_HEADER headerInfo;
    PUCHAR dataBuffer;
    
    //
    // Check for any error status.
    //
    flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags;

    DebugPrint((1,
               "I2OConfigDialogCallback: Msg %x. Flags %x\n",
               Msg,
               flags));

    if (flags & I2O_MESSAGE_FLAGS_FAIL) {

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));

    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
        srb = I2OHandleError(Msg, deviceExtension, 0);

    } else {

        pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

        callbackObject = pointer64ToU32.Pointer;
        srb  = callbackObject->Context;
    }
    
    //
    // Get the replyFrame from the data buffer.
    //
    srbIoControl = (PSRB_IO_CONTROL)srb->DataBuffer;

    //
    // Get the original header information.
    //
    dataBuffer = (PUCHAR)srb->DataBuffer;
    dataBuffer += sizeof(SRB_IO_CONTROL);
    headerInfo = (PI2O_PARAMS_HEADER)dataBuffer;

   
    //
    // Update the transfer length.
    //
    headerInfo->DataTransferLength = Msg->TransferCount;

    //
    // Jam in the status info.
    //
    headerInfo->RequestStatus = Msg->BsaReplyFrame.ReqStatus;
    headerInfo->DetailedStatusCode = Msg->BsaReplyFrame.DetailedStatusCode;
     
    //
    // Mark the ioctl status as successful.
    //
    srbIoControl->ReturnCode = I2O_IOCTL_STATUS_SUCCESS;
    
    //
    // It's always considered successful. Any errors are noted
    // in the Request Header.
    //
    srb->SrbStatus = SRB_STATUS_SUCCESS;        
    
    //
    // Complete the request.
    //
    I2OCompleteRequest(deviceExtension,
                       srb);
    
}


VOID
I2OGetParamsCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )

/*++

Routine Description:

    Process a Get/SetParams passthrough requestion completion. 

Arguments:

    Msg             - message describing the completed request
    DeviceContext   - device extension for the device this request was issued against

Return Value:

    None

--*/
{
    PDEVICE_EXTENSION   deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT  callbackObject;
    ULONG               flags;
    PSCSI_REQUEST_BLOCK srb;
    POINTER64_TO_U32    pointer64ToU32;
    PSRB_IO_CONTROL srbIoControl;
    PI2O_PARAMS_HEADER headerInfo;
    PUCHAR dataBuffer;
    
    //
    // Check for any error status.
    //
    flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags;

    DebugPrint((2,
               "I2OGetParamsCallback: Msg %x. Flags %x\n",
               Msg,
               flags));

    if (Msg->BsaReplyFrame.StdMessageFrame.MessageSize > 5) {
        DebugPrint((1,
                    "ReplyMessage has payload\n"));
    }
    if (flags & I2O_MESSAGE_FLAGS_FAIL) {

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));

    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
        srb = I2OHandleError(Msg, deviceExtension, 0);

    } else {

        pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

        callbackObject = pointer64ToU32.Pointer;
        srb  = callbackObject->Context;
    }
    
    //
    // Get the replyFrame from the data buffer.
    //
    srbIoControl = (PSRB_IO_CONTROL)srb->DataBuffer;

    //
    // Get the original header information.
    //
    dataBuffer = (PUCHAR)srb->DataBuffer;
    dataBuffer += sizeof(SRB_IO_CONTROL);
    headerInfo = (PI2O_PARAMS_HEADER)dataBuffer;

    //
    // Update the transfer length.
    //
    headerInfo->DataTransferLength = Msg->TransferCount;

    //
    // Jam in the status info.
    //
    headerInfo->RequestStatus = Msg->BsaReplyFrame.ReqStatus;
    headerInfo->DetailedStatusCode = Msg->BsaReplyFrame.DetailedStatusCode;
     
    //
    // Mark the ioctl status as successful.
    //
    srbIoControl->ReturnCode = I2O_IOCTL_STATUS_SUCCESS;
    
    //
    // It's always considered successful. Any errors are noted
    // in the Request Header.
    //
    srb->SrbStatus = SRB_STATUS_SUCCESS;        
    
    //
    // Complete the request.
    //
    I2OCompleteRequest(deviceExtension,
                       srb);
    
}


ULONG
I2OParamsConfigDialog(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine handles the configuration dialog passthrough messages, sent
    by the management driver at the request of some config. utility.

Arguments:

    DeviceExtension - Driver's context.
    Srb - SrbControl Srb.
    
Return Value:

    PENDING - If the request was issued successfully.
    ERROR - No message frames available.

--*/
{
    PIOP_CALLBACK_OBJECT callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    POINTER64_TO_U32 pointer64ToU32;
    PI2O_PARAMS_HEADER headerInfo;
    PI2O_SG_ELEMENT sgList;
    ULONG transferPages;
    ULONG status;
    ULONG messageSize;
    PUCHAR index;
    PUCHAR dataBuffer;

    union {
        I2O_UTIL_CONFIG_DIALOG_MESSAGE m;
        U32 blk[128/ sizeof(U32)];
    } msg;
    
    //
    // Get the header info.
    //
    index = Srb->DataBuffer;
    index += sizeof(SRB_IO_CONTROL);
    headerInfo = (PI2O_PARAMS_HEADER)index;
    
    //
    // Build the message frame.
    //
    memset(&msg, 0, sizeof(msg));
    msg.m.StdMessageFrame.VersionOffset = I2O_VERSION_11 | I2O_CONFIG_SGL_VER_OFFSET;

    msg.m.StdMessageFrame.Function = I2O_UTIL_CONFIG_DIALOG;

    msg.m.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    msg.m.StdMessageFrame.TargetAddress = headerInfo->TargetId;
    msg.m.PageNumber = headerInfo->RequestSpecificInfo.RequestPage;
    messageSize = (sizeof(I2O_UTIL_CONFIG_DIALOG_MESSAGE) -
                   sizeof(I2O_SG_ELEMENT)) >> 2;

    //
    // Setup context and callback info.
    //
    callback->Callback.Signature = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackAddress = I2OConfigDialogCallback;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer = callback;

    msg.m.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg.m.TransactionContext = pointer64ToU32.u.HighPart;

    //
    // Build the sg list. First elements will be the write data, after that
    // will be the read stuff.
    //
    sgList = &msg.m.SGL;
    transferPages = 0;

    if (headerInfo->ReadLength) {
        ULONG sgListSize;
        

        sgList->u.Page[0].FlagsCount.Count = headerInfo->ReadLength;
        sgList->u.Page[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT |
                                             I2O_SGL_FLAGS_END_OF_BUFFER |
                                             I2O_SGL_FLAGS_PAGE_LIST_ADDRESS_ELEMENT;

        //
        // Get the read portion of the data buffer.
        //
        dataBuffer = index;
        dataBuffer += headerInfo->ReadOffset;

        //
        // Build the SG list for the read.
        //
        transferPages = I2OBuildControlSGList(DeviceExtension,
                                              Srb,
                                              dataBuffer,
                                              sgList,
                                              headerInfo->ReadLength);
        
        //
        // Bump the sglist to the next element.
        //
        sgListSize = I2O_SG_ELEMENT_OVERHEAD + (transferPages * sizeof(ULONG));
        sgList = (PI2O_SG_ELEMENT)((PUCHAR)sgList + sgListSize);

        messageSize += transferPages + 1;
    }

    if (headerInfo->WriteLength) {

        //
        // If a write buffer SGL has been built, need to clear the last element
        // flag.
        //
        if (headerInfo->ReadLength) {
            msg.m.SGL.u.Page[0].FlagsCount.Flags &= ~I2O_SGL_FLAGS_LAST_ELEMENT;
        }
       
        sgList->u.Page[0].FlagsCount.Count = headerInfo->WriteLength;
        sgList->u.Page[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                             I2O_SGL_FLAGS_END_OF_BUFFER |
                                             I2O_SGL_FLAGS_DIR |
                                             I2O_SGL_FLAGS_PAGE_LIST_ADDRESS_ELEMENT;
        //
        // Get the write buffer.
        //
        dataBuffer = index;
        dataBuffer += headerInfo->WriteOffset;

        //
        // Build the SG list for the write
        // 
        transferPages = I2OBuildControlSGList(DeviceExtension,
                                              Srb,
                                              dataBuffer,
                                              sgList,
                                              headerInfo->WriteLength);
        messageSize += transferPages + 1;
    }


    //
    // Set the message size in ULONGs.
    //
    msg.m.StdMessageFrame.MessageSize = (USHORT)messageSize;
       
    //
    // Issue the request.
    //
    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&msg)) {
        status = SRB_STATUS_ERROR;
    } else {
        status = SRB_STATUS_PENDING;
    }    
   
    return status;
}    


ULONG
I2OParamsGetSet(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb,
    IN BOOLEAN GetParams
    )
/*++

Routine Description:

    This routine handles sending the Params Get/Set messages issued from the management
    driver at the request of some config. utility.
    
Arguments:

    DeviceExtension - Context
    Srb - The request to frame and issue.
    GetParams - Indicates whether this is a Get or a Set.
    
Return Value:

    PENDING - If the request was issued.
    ERROR - No message frames available.

--*/
{
    PIOP_CALLBACK_OBJECT callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    POINTER64_TO_U32 pointer64ToU32;
    PI2O_PARAMS_HEADER headerInfo;
    PI2O_SG_ELEMENT sgList;
    ULONG transferPages;
    ULONG status;
    ULONG messageSize;
    PUCHAR index;
    PUCHAR dataBuffer;

    union {
        I2O_UTIL_PARAMS_GET_MESSAGE m;
        U32 blk[128/ sizeof(U32)];
    } msg;
    

    //
    // Get the header info.
    //
    index = Srb->DataBuffer;
    index += sizeof(SRB_IO_CONTROL);
    headerInfo = (PI2O_PARAMS_HEADER)index;

    //
    // Build the message frame.
    //
    memset(&msg, 0, sizeof(msg));
    msg.m.StdMessageFrame.VersionOffset = I2O_VERSION_11 | I2O_PARAMS_SGL_VER_OFFSET;

    //
    // Determine whether this is a get or a set.
    //
    if (GetParams) {
        msg.m.StdMessageFrame.Function = I2O_UTIL_PARAMS_GET;
    } else {
        msg.m.StdMessageFrame.Function = I2O_UTIL_PARAMS_SET;
    }    

    msg.m.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    msg.m.StdMessageFrame.TargetAddress = headerInfo->TargetId;
    messageSize = (sizeof(I2O_UTIL_PARAMS_GET_MESSAGE) -
                   sizeof(I2O_SG_ELEMENT)) >> 2;

    //
    // Setup context and callback info.
    //
    callback->Callback.Signature = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackAddress = I2OGetParamsCallback;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer = callback;

    msg.m.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg.m.TransactionContext = pointer64ToU32.u.HighPart;

    //
    // Build the sg list. First elements will be the write data, after that
    // will be the read stuff.
    //
    sgList = &msg.m.SGL;
    transferPages = 0;

    if (headerInfo->WriteLength) {
        ULONG sgListSize;
        
        sgList->u.Page[0].FlagsCount.Count = headerInfo->WriteLength;
        sgList->u.Page[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                             I2O_SGL_FLAGS_END_OF_BUFFER |
                                             I2O_SGL_FLAGS_DIR |
                                             I2O_SGL_FLAGS_PAGE_LIST_ADDRESS_ELEMENT;
        //
        // Get the write buffer.
        //
        dataBuffer = index;
        dataBuffer += headerInfo->WriteOffset;

        //
        // Build the SG list for the write
        // 
        transferPages = I2OBuildControlSGList(DeviceExtension,
                                              Srb,
                                              dataBuffer,
                                              sgList,
                                              headerInfo->WriteLength);
        //
        // Bump the sglist to the next element.
        //
        sgListSize = I2O_SG_ELEMENT_OVERHEAD + (transferPages * sizeof(ULONG));
        sgList = (PI2O_SG_ELEMENT)((PUCHAR)sgList + sgListSize);
        messageSize += transferPages + 1;
    }

    if (headerInfo->ReadLength) {
        
        //
        // If a write buffer SGL has been built, need to clear the last element
        // flag.
        //
        if (headerInfo->WriteLength) {
            msg.m.SGL.u.Page[0].FlagsCount.Flags &= ~I2O_SGL_FLAGS_LAST_ELEMENT;
        }

        sgList->u.Page[0].FlagsCount.Count = headerInfo->ReadLength;
        sgList->u.Page[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT |
                                             I2O_SGL_FLAGS_END_OF_BUFFER |
                                             I2O_SGL_FLAGS_PAGE_LIST_ADDRESS_ELEMENT;

        //
        // Get the read portion of the data buffer.
        //
        dataBuffer = index;
        dataBuffer += headerInfo->ReadOffset;

        //
        // Build the SG list for the read.
        //
        transferPages = I2OBuildControlSGList(DeviceExtension,
                                              Srb,
                                              dataBuffer,
                                              sgList,
                                              headerInfo->ReadLength);
        
        messageSize += transferPages + 1;
    }

    //
    // Set the message size in ULONGs.
    //
    msg.m.StdMessageFrame.MessageSize = (USHORT)messageSize;
       
    //
    // Issue the request.
    //
    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&msg)) {
        status = SRB_STATUS_ERROR;
    } else {
        status = SRB_STATUS_PENDING;
    }    
   
    return status;
}    


ULONG
I2OSrbControl(
    IN PVOID DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This is the SRB_IO_CONTROL handler for this driver. These requests come from
    the management driver.
    
Arguments:

    DeviceExtension - Context
    Srb - The request to process.
    
Return Value:

    Value from the helper routines, or if handled in-line, SUCCESS or INVALID_REQUEST. 
    
--*/
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    PSRB_IO_CONTROL srbIoControl;
    PUCHAR dataBuffer;
    PUCHAR tmpData;
    ULONG controlCode;
    ULONG msgSize;
    ULONG transferPages = 0;
    ULONG status;
    ULONG length;
    UCHAR offset;
    UCHAR version;


    //
    // Start off being paranoid.
    //
    if (Srb->DataBuffer == NULL) {
        return SRB_STATUS_INVALID_REQUEST;
    }

    //
    // Ensure the signature is correct.
    //
    if (strcmp(((PSRB_IO_CONTROL)(Srb->DataBuffer))->Signature, IOCTL_SIGNATURE)) {

        return SRB_STATUS_INVALID_REQUEST;
    }    
    
    //
    // Extract the io_control
    //
    srbIoControl = (PSRB_IO_CONTROL)Srb->DataBuffer;

    //
    // Get the control code.
    // 
    controlCode = srbIoControl->ControlCode;

    //
    // Get the data buffer. If this is a send message request, it gets
    // fixed up to be an I2O message later.
    //
    dataBuffer = ((PUCHAR)Srb->DataBuffer) + sizeof(SRB_IO_CONTROL);

    //
    // Based on the control code, figure out what to do.
    //
    switch (controlCode) {
        case I2O_IOCTL_GET_CONFIG_INFO:
            DebugPrint((1,
                        "I2OSrbControl: Get Config Info\n"));
            return SRB_STATUS_INVALID_REQUEST;
                        
            
        case I2O_IOCTL_REGISTER: {
            PI2O_REGISTRATION_INFO signatureInfo = (PI2O_REGISTRATION_INFO)dataBuffer;
            UCHAR miniportSig[] = "IntelI2O";
            ULONG i;
            
            for (i = 0; i < 8; i++) {
                signatureInfo->Signature[i] = miniportSig[i];
            }
            signatureInfo->Size = sizeof(I2O_REGISTRATION_INFO);
            srbIoControl->ReturnCode = I2O_IOCTL_STATUS_SUCCESS;
            return SRB_STATUS_SUCCESS;
        }
        break;

        case I2O_IOCTL_GET_IOP_COUNT: {
            PI2O_IOP_COUNT hbaCount = (PI2O_IOP_COUNT)dataBuffer;

            //
            // Return the global count of IOPs.
            // 
            hbaCount->IOPCount = GlobalAdapterCount;
            srbIoControl->ReturnCode = I2O_IOCTL_STATUS_SUCCESS;

            return SRB_STATUS_SUCCESS;
        }    
        break;

        case I2O_IOCTL_PARAMS_GET:

            //
            // Call the helper to handle getting the information contained
            // in the Srb.
            //
            status = I2OParamsGetSet(deviceExtension,
                                     Srb,
                                     TRUE);
            break;
        case I2O_IOCTL_PARAMS_SET:

            //
            // Call the routine to set whatever parameters are indicated in
            // the Srb.
            //
            status = I2OParamsGetSet(deviceExtension,
                                     Srb,
                                     FALSE);
            break;
    
        case I2O_IOCTL_CONFIG_DIALOG:

            //
            // This will deal with handling the web-page display.
            //
            status = I2OParamsConfigDialog(deviceExtension,
                                           Srb);

            break;

        case I2O_IOCTL_SHUTDOWN_ADAPTER:

            // 
            // The management driver will be notified of system shutdown events
            // and will send the I2O_IOCTL_SHUTDOWN_ADAPTER request directly
            // to the miniport.  This functionality was added to insure that all
            // shutdown requests make it to the IOP.  If the volume is a dynamic disk
            // upper-layer software fails to pass the shutdown notification down the stack and 
            // the IOP does not get shutdown properly.   This code is a duplicate of 
            // the SRB_FUNCTION_SHUTDOWN case in I2OSrbControl.
            //

            //
            // See if the IOP supports the special prepare for shutdown mode.
            //
            if (deviceExtension->IsmTID == 0x0) {

                //
                // no ISM available or card doesn't support the mode, just send flush
                //
                DebugPrint((1,
                            "NoSpecialShutdown: Shutdown called, flush instead\n"));
                
                status = I2OShutdownFlushSync(deviceExtension,
                          Srb);

            } else {
                if (!(deviceExtension->AdapterFlags & AFLAGS_SHUTDOWN_RECVD)) {                       
                    //
                    // send some special commands to the ISM to
                    // invoke a 'get ready for shutdown mode'
                    //
                    status = I2OHandleShutdown(deviceExtension);

                } else {
                    status = SRB_STATUS_SUCCESS;
                }    
                deviceExtension->AdapterFlags |= AFLAGS_SHUTDOWN_RECVD;
            }

            //
            // Indicate that the system is shutting down. This flag is used to
            // help keep the cache flushed so that the volumes remain consistent.
            //
            deviceExtension->TempShutdown = TRUE;
            break;

        case I2O_IOCTL_RESET_ADAPTER:
        case I2O_IOCTL_RESCAN_ADAPTER:

        default:

            DebugPrint((1,
                        "I2OSrbControl: Control Code (%x)\n",
                        controlCode));
            return SRB_STATUS_INVALID_REQUEST;
    }        

    return status;
}


VOID 
I2OExecCDBCompletion (
    IN PI2O_SCSI_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    Completion callback for requests sent to SCSI devices.

Arguments:

    Msg - The reply message fram.e
    DeviceContext - This miniport's HwDeviceExtension
    
Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION      deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT   callbackObject;
    POINTER64_TO_U32       pointer64ToU32;
    PI2O_DEVICE_INFO       deviceInfo;
    PSCSI_REQUEST_BLOCK    srb;
 
    //
    // Verify the Msg Frame, and extract the Srb.
    //
    if (Msg->StdReplyFrame.StdMessageFrame.MsgFlags & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((1,
                    "I2OExecCDBCompletion: ERROR Invalid Flags in SCSI Srb Message\n"));

        srb = I2OHandleError( (PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME)Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));
    } else {

        pointer64ToU32.u.LowPart  = Msg->StdReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->StdReplyFrame.TransactionContext;
        callbackObject            = pointer64ToU32.Pointer;

        srb = callbackObject->Context;
    }

    //
    // Get this device's information block.
    // 
    deviceInfo = &deviceExtension->DeviceInfo[srb->TargetId];

    //
    // Check to see whether the request completed successfully.
    // 
    if (Msg->StdReplyFrame.ReqStatus != 0x00) {
        
            DebugPrint((1,
                        "I2OExecCDBCompletion: ERROR ReqStatus = %x DSC = %x\n", 
                        Msg->StdReplyFrame.ReqStatus,
                        Msg->StdReplyFrame.DetailedStatusCode));

        
            //
            // Map the I2O error into an Srb Status and possibly SCSI Status 
            // value.
            //
            switch (Msg->StdReplyFrame.DetailedStatusCode) {
                case I2O_SCSI_HBA_DSC_DATA_OVERRUN:

                    srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
                    srb->DataTransferLength =
                            ((PI2O_SCSI_ERROR_REPLY_MESSAGE_FRAME)Msg)->TransferCount;
                    break;
                case I2O_SCSI_HBA_DSC_UNEXPECTED_BUS_FREE:
                    srb->SrbStatus = SRB_STATUS_UNEXPECTED_BUS_FREE;
                    break;
                case I2O_SCSI_HBA_DSC_REQUEST_LENGTH_ERROR:

                    //
                    // On at least one IOP, this means data overrun
                    // Treat it as such and update DataTransferLength;
                    //
                    srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
                    srb->DataTransferLength =
                            ((PI2O_SCSI_ERROR_REPLY_MESSAGE_FRAME)Msg)->TransferCount;
                    break;
                case I2O_SCSI_DSC_CHECK_CONDITION: {
                    PSENSE_DATA senseData = srb->SenseInfoBuffer;
                    
                    DebugPrint((1,
                                "I2OExecCDBCompletion(%x): CHECK CONDITION on (%x).\n",
                                deviceExtension,
                                deviceInfo));
                   
                    srb->SrbStatus = SRB_STATUS_ERROR;
                    if (senseData) {
                        StorPortMoveMemory(senseData, 
                                      ((PI2O_SCSI_ERROR_REPLY_MESSAGE_FRAME)Msg)->SenseData, 
                                      srb->SenseInfoBufferLength);
                    
                        senseData->Valid = 1;
                        srb->SrbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                        DebugPrint((1,
                                    "SenseKey (%x) ASC (%x) ASCQ (%x)\n",
                                    senseData->SenseKey,
                                    senseData->AdditionalSenseCode,
                                    senseData->AdditionalSenseCodeQualifier));
                                    
                    }
                    srb->ScsiStatus = SCSISTAT_CHECK_CONDITION;
                    break;
                }    

                case I2O_SCSI_DSC_BUSY:

                    srb->ScsiStatus = SCSISTAT_BUSY;
                    srb->SrbStatus = SRB_STATUS_ERROR;
                    break;
                case I2O_SCSI_DSC_RESERVATION_CONFLICT:
                    
                    srb->ScsiStatus = SCSISTAT_RESERVATION_CONFLICT;
                    srb->SrbStatus = SRB_STATUS_ERROR;
                    break;
                case I2O_SCSI_DSC_COMMAND_TERMINATED:

                    srb->ScsiStatus = SCSISTAT_COMMAND_TERMINATED;
                    srb->SrbStatus = SRB_STATUS_ERROR;
                    break;
                case I2O_SCSI_DSC_TASK_SET_FULL:

                    srb->ScsiStatus = SCSISTAT_QUEUE_FULL;
                    srb->SrbStatus = SRB_STATUS_ERROR;
                    break;               
                default:
                    srb->ScsiStatus = SCSISTAT_GOOD;
                    srb->SrbStatus = SRB_STATUS_SUCCESS;
        }
    } else {
        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }    

    DebugPrint((2,
                "I2OExecCDBCompletion: Done for Srb: %x, srbStatus: %x, srbScsiStatus: %x\n", 
                srb,
                srb->SrbStatus,srb->ScsiStatus));

    I2OCompleteRequest(deviceExtension,
                       srb);
    
    return;
}


VOID
I2OBuildExecCDBToIOP (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine is the BuildIO part of 'scsi passthrough requests' handling,
    (requests sent to a non-RAID device, such as tape, or a disk marked as 'passthrough'.

Arguments:

    DeviceExtension - This drivers context.
    Srb - The Srb containing the CDB to send.
    
Return Value:

    NOTHING.

--*/
{
    PI2O_LCT                lct = DeviceExtension->Lct;
    PIOP_CALLBACK_OBJECT    callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    PI2O_DEVICE_INFO        deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    PPASSTHRU_MSG           msg = &((PSRB_EXTENSION)Srb->SrbExtension)->PTMsg;
    POINTER64_TO_U32        pointer64ToU32;
    ULONG                   status;
    PI2O_SG_ELEMENT         sgList;
    ULONG                   transferPages;
    ULONG                   i;
    
    //
    // Build the message frame.
    //          
    memset(msg,0, sizeof(msg));
    msg->m.StdMessageFrame.VersionOffset    = I2O_VERSION_11 | I2O_PARAMS_SGL_VER_OFFSET;
    msg->m.StdMessageFrame.MsgFlags         = 0;
    msg->m.StdMessageFrame.TargetAddress    = deviceInfo->Lct.LocalTID;
    msg->m.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    msg->m.StdMessageFrame.Function         = I2O_SCSI_SCB_EXEC;
    msg->m.StdMessageFrame.InitiatorContext = 0;
    msg->m.StdMessageFrame.MessageSize      = (sizeof(I2O_SCSI_SCB_EXECUTE_MESSAGE) -
                                                sizeof(I2O_SG_ELEMENT) +
                                                sizeof(I2O_SGE_PAGE_ELEMENT)) >> 2;

    //
    // Setup the msg flags based on the srb values.
    //
    if(Srb->SrbFlags & SRB_FLAGS_QUEUE_ACTION_ENABLE) {
        msg->m.SCBFlags |= I2O_SCB_FLAG_SIMPLE_QUEUE_TAG;
    } else {
        
        msg->m.SCBFlags &= ~I2O_SCB_FLAG_TAG_TYPE_MASK;
    }  

    if(Srb->SrbFlags & SRB_FLAGS_DISABLE_DISCONNECT) {
        msg->m.SCBFlags &= ~I2O_SCB_FLAG_ENABLE_DISCONNECT;
    } else {
        msg->m.SCBFlags |= I2O_SCB_FLAG_ENABLE_DISCONNECT;
    }    

    if(Srb->SrbFlags & SRB_FLAGS_DISABLE_AUTOSENSE) {
        msg->m.SCBFlags &= ~I2O_SCB_FLAG_AUTOSENSE_MASK;
    } else {
        msg->m.SCBFlags |= I2O_SCB_FLAG_SENSE_DATA_IN_MESSAGE;
    }        

    if(Srb->SrbFlags & SRB_FLAGS_DATA_IN) {
        msg->m.SCBFlags |= I2O_SCB_FLAG_XFER_FROM_DEVICE;
    }    

    if(Srb->SrbFlags & SRB_FLAGS_DATA_OUT) {
        msg->m.SCBFlags |= I2O_SCB_FLAG_XFER_TO_DEVICE;
    }    

    msg->m.CDBLength = Srb->CdbLength;
    msg->m.ByteCount = Srb->DataTransferLength;

    //
    // Copy over the cdb.
    //
    for (i = 0; i < msg->m.CDBLength; i++) {
        msg->m.CDB[i] = Srb->Cdb[i];
    }

    //
    // Setup context and callback info.
    //
    callback->Callback.Signature = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackAddress = I2OExecCDBCompletion;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context  = Srb;
    
    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callback;
    msg->m.StdMessageFrame.InitiatorContext    = pointer64ToU32.u.LowPart;
    msg->m.TransactionContext = pointer64ToU32.u.HighPart;

    //
    // Build sg list
    //
    sgList = &(msg->m.SGL);
    sgList->u.Page[0].FlagsCount.Count = msg->m.ByteCount;
    sgList->u.Page[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                         I2O_SGL_FLAGS_END_OF_BUFFER |
                                         I2O_SGL_FLAGS_PAGE_LIST_ADDRESS_ELEMENT;

    if(Srb->SrbFlags & SRB_FLAGS_DATA_OUT) {
        sgList->u.Page[0].FlagsCount.Flags |= I2O_SGL_FLAGS_DIR;
    }    

    if (Srb->DataTransferLength) {
        transferPages = I2OBuildSGList(DeviceExtension,
                                       Srb,
                                       sgList);
        msg->m.StdMessageFrame.MessageSize += (USHORT)transferPages;
    }
}


ULONG 
I2OSendExecCDBToIOP (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine is the StartIo part of 'scsi passthrough requests' handling.
    
Arguments:

    DeviceExtension - Context
    Srb - The request to process.
    
Return Value:

    SELTO - If the device doesn't exist.
    PENDING - Submitted.
    ERROR - No message frames available.

--*/

{
    PPASSTHRU_MSG           msg = &((PSRB_EXTENSION)Srb->SrbExtension)->PTMsg;
    PIOP_CALLBACK_OBJECT    callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    PI2O_DEVICE_INFO        deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    ULONG                   status;
    
    DebugPrint((2,
                "ExecCDBToIOP: Srb (%x) Scsi Id (0x%x) Tid (0x%x) Func (0x%x) Cdb (0x%x)\n",
                Srb,
                Srb->TargetId,
                deviceInfo->Lct.LocalTID,
                Srb->Function,
                Srb->Cdb[0]));

    //
    // If this is an inquiry, need to find the entry and copy it over.
    //
    if (Srb->Cdb[0] == SCSIOP_INQUIRY) {
        //
        // Time out requests to any device that are no visible to the OS or
        // if the Lun != 0.
        //  
        if ((!(deviceInfo->DeviceFlags & DFLAGS_OS_CAN_CLAIM)) || (Srb->Lun!=0)) {
            return SRB_STATUS_SELECTION_TIMEOUT;
        }
    }

    //
    // Issue the request.
    //
    DebugPrint((3,
                "I2OSendExecCDBToIOP:  Sending Message %x, Srb %x, TID %x\n",
                msg,
                Srb,
                deviceInfo->Lct.LocalTID));

    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)msg)) {
        //
        // Return SRB_STATUS_BUSY when the IOP does not have 
        // an available inbound message frame.  StorPort will
        // retry this request later.
        //
        status = SRB_STATUS_BUSY;

        //
        // Tell StorPort that the adapter is busy.  Do not
        // start another request until 10 requests have completed.
        // This gives the miniport/StorPort the ability to burst 10 requests
        // once StorPort has started the next IO (assuming the controller is
        // kept busy with outstanding IO). 
        //
        StorPortBusy(DeviceExtension,10);

    } else {
        status = SRB_STATUS_PENDING;
    }    
   
    return status;
}


VOID
I2OSingleDeviceResetCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME  Msg,
    IN PVOID CallbackContext
    )
/*++

Routine Description: 

    This is the callback routine for a device reset. 
    It will complete the reset SRB and then all outstanding requests to the device. 
    Following that, the device is un-paused to allow I/O activity to restart.

Arguments:  

    Msg - The reset message reply.
    CallbackContext - This miniport's HwDeviceExtension
    
Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION    deviceExtension = CallbackContext;
    PIOP_CALLBACK_OBJECT callbackObject;
    POINTER64_TO_U32     pointer64ToU32;
    PI2O_DEVICE_INFO deviceInfo; 
    PSCSI_REQUEST_BLOCK  srb;
    ULONG status = SRB_STATUS_SUCCESS;
    ULONG flags;
    UCHAR pathId;
    UCHAR targetId;
    UCHAR lun;

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer
    // and the TransactionContext (for NT64) contains the high 32 bits.  Contained
    // in the callback object is the syncReq pointer associated with the IO completion.
    //
    pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
    pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

    //
    // Handle any errors, extract the Srb and set appropriate status values.
    // 
    if ((flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((1,
                   "I2OSingleDeviceResetCallBack: Failed MsgFlags\n"));

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));

    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             0);
    } else {

        callbackObject = pointer64ToU32.Pointer;

        //
        // Get the Srb and set status.
        // 
        srb = callbackObject->Context;

        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }

    //
    // Get the device info. that corresponds to the device that just got reset.
    // 
    deviceInfo = &deviceExtension->DeviceInfo[srb->TargetId];

    //
    // Capture the scsi address info. locally.
    //
    pathId = srb->PathId;
    targetId = srb->TargetId;
    lun = srb->Lun;

    //
    // Complete the reset request.
    // 
    I2OCompleteRequest(deviceExtension,
                         srb);

    //
    // Now complete everything else. Indicated a hard-reset, so all outstanding
    // requests were lost.
    //
    StorPortCompleteRequest(deviceExtension,
                            pathId,
                            targetId,
                            lun,
                            SRB_STATUS_BUS_RESET);


    //
    // Tell storport to resume IO's to the device regardless of success or failure.
    //
    deviceInfo->DeviceFlags &= ~DFLAGS_PAUSED;
    StorPortResumeDevice(deviceExtension,
                         pathId,
                         targetId,
                         lun);
    return;
}    


ULONG
I2OResetDevice (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description: 

    This routine will issue a reset message to the target indicated by Srb->TargetId.
    The device gets paused, so that no other requests are received until the reset is
    complete.
    
Arguments:  

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The Target/Lun reset SRB
    
Return Value:

    PENDING or ERROR, if the message couldn't be sent.

--*/
{
    PI2O_DEVICE_INFO deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    POINTER64_TO_U32 pointer64ToU32;
    PIOP_CALLBACK_OBJECT callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    ULONG status = SRB_STATUS_PENDING;
    I2O_BSA_DEVICE_RESET_MESSAGE reset;

    //
    // Pause this device.
    //
    StorPortPauseDevice(DeviceExtension,
                        Srb->PathId,
                        Srb->TargetId,
                        Srb->Lun,
                        BSA_RESET_PAUSE_TIMEOUT);

    deviceInfo->DeviceFlags |= DFLAGS_PAUSED;

    //
    // The device is a BSA device and the Claim has been
    // successfully processed, so issue the reset message.
    //
    memset(&reset, 0 , sizeof(I2O_BSA_DEVICE_RESET_MESSAGE));
        
    reset.StdMessageFrame.VersionOffset= I2O_VERSION_11;
    reset.StdMessageFrame.MsgFlags = 0;
    reset.StdMessageFrame.MessageSize = sizeof(I2O_BSA_DEVICE_RESET_MESSAGE)/4;
    reset.StdMessageFrame.TargetAddress = deviceInfo->Lct.LocalTID;
    reset.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    reset.StdMessageFrame.Function = I2O_BSA_DEVICE_RESET;
    reset.StdMessageFrame.InitiatorContext = 0;
    reset.ControlFlags = I2O_BSA_FLAG_HARD_RESET;
    
    //
    // Build the call-back object.
    // 
    callback->Callback.Signature       = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackAddress = I2OSingleDeviceResetCallBack;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callback;

    reset.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    reset.TransactionContext = pointer64ToU32.u.HighPart;

    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&reset)) {

        //
        // Indicate an error. A bus reset should be issued by storport if this
        // fails.
        // 
        status = SRB_STATUS_ERROR;

        //
        // Resume I/O activity for the device.
        // 
        deviceInfo->DeviceFlags &= ~DFLAGS_PAUSED;
        StorPortResumeDevice(DeviceExtension,
                             Srb->PathId,
                             Srb->TargetId,
                             Srb->Lun);

    }

    return status;
}

BOOLEAN
I2OBSADeviceResetAll (
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description: 

    Handle BusReset for clustering support

Arguments:  
    
Return Value:

--*/
{
    POINTER64_TO_U32 pointer64ToU32;
    PIOP_CALLBACK_OBJECT callBackObject;
    PI2O_DEVICE_INFO lctEntry;
    BOOLEAN status = TRUE;
    I2O_BSA_DEVICE_RESET_MESSAGE reset;
    ULONG j;

    callBackObject = &DeviceExtension->DeviceResetCallback;

    DebugPrint((1,
                "I2OBSADeviceReset: Begin ****\n"));

    //
    // Set ResetCnt before starting BSA_DEVICE_RESET
    //
    DeviceExtension->ResetCnt = 0;
    for (j = 0; j < IOP_MAXIMUM_TARGETS; j++) {
        lctEntry = &DeviceExtension->DeviceInfo[j];
        if ((lctEntry->DeviceFlags & DFLAGS_CLAIMED) && 
            (lctEntry->Class == I2O_CLASS_RANDOM_BLOCK_STORAGE)) {
            DeviceExtension->ResetCnt++;
        }
    }
    if (DeviceExtension->ResetCnt == 0) {
        I2OBSADeviceResetComplete(DeviceExtension);
        return TRUE;
    }

    //
    // Find the LCT entry corresponding to this device.
    //
    for (j = 0; j < IOP_MAXIMUM_TARGETS; j++) {
        lctEntry = &DeviceExtension->DeviceInfo[j];

        if ((lctEntry->DeviceFlags & DFLAGS_CLAIMED) && 
            (lctEntry->Class == I2O_CLASS_RANDOM_BLOCK_STORAGE)) {

            //
            // The device is a BSA device and the Claim has been
            // successfully processed, so issue the reset message.
            //
            memset(&reset, 0 , sizeof(I2O_BSA_DEVICE_RESET_MESSAGE));
                
            reset.StdMessageFrame.VersionOffset= I2O_VERSION_11;
            reset.StdMessageFrame.MsgFlags = 0;
            reset.StdMessageFrame.MessageSize = sizeof(I2O_BSA_DEVICE_RESET_MESSAGE)/4;
            reset.StdMessageFrame.TargetAddress = lctEntry->Lct.LocalTID;
            reset.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
            reset.StdMessageFrame.Function = I2O_BSA_DEVICE_RESET;
            reset.StdMessageFrame.InitiatorContext = 0;
            reset.ControlFlags = I2O_BSA_FLAG_HARD_RESET;
            
            DebugPrint((1,
                          "I2OBSADeviceReset: Send I2ODeviceReset to TID 0x%x\n",
                           lctEntry->Lct.LocalTID));
                                                    
            pointer64ToU32.Pointer64 = 0;
            pointer64ToU32.Pointer   = callBackObject;

            reset.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
            reset.TransactionContext = pointer64ToU32.u.HighPart;

            DeviceExtension->CallBackComplete = 0;

            if (!I2OSendMessage(DeviceExtension,
                                (PI2O_MESSAGE_FRAME)&reset)) {
                DebugPrint((1,
                           "I2OBSADeviceReset: Failure sending message\n"));
                status = FALSE;
            }

        }
    }

    return status;
}


VOID
I2OBSADeviceResetCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME  Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    This the the call-back for BSA_DEVICE_RESET. For each device present, a reset message
    was sent. Once all are complete, final processing of the reset request is handled. 

Arguments:

    Msg - The reply message frame.
    DeviceContext - This miniport's HwDeviceExtension
    
Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION deviceExtension = DeviceContext;

    //
    // Verify the Msg Frame
    //
    if (Msg->BsaReplyFrame.StdMessageFrame.MsgFlags & I2O_MESSAGE_FLAGS_FAIL) {

        //
        //TODO: LOG the failure.
        //
        DebugPrint((1,
                    "I2OBSADeviceResetCallback: ERROR Invalid Flags in DeviceReset Message\n"));
    }

    if (--deviceExtension->ResetCnt == 0) {
        
        DebugPrint((1, 
                    "I2OBSADeviceResetCallback: calling I2OBSADeviceResetComplete\n"));

        //
        // Finalise processing on the reset request.
        //
        I2OBSADeviceResetComplete(deviceExtension);
    }

    return;
}


VOID
I2OReserveReleaseCallBack(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID              CallbackContext
    )
/*++

Routine Description:

    This is the call-back for Reserve and Release messages.

Arguments:

    Msg - The reply message frame.
    CallbackContext - This miniport's HwDeviceExtension
    
Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION  deviceExtension = CallbackContext;
    PIOP_CALLBACK_OBJECT callbackObject;
    POINTER64_TO_U32     pointer64ToU32;
    PSCSI_REQUEST_BLOCK  srb;
    ULONG                status = SRB_STATUS_SUCCESS;
    ULONG                flags;
    PULONG               msgBuffer;

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer
    // and the TransactionContext (for NT64) contains the high 32 bits.  Contained
    // in the callback object is the syncReq pointer associated with the IO completion.
    //
    pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
    pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;


    //
    // Handle any errors, extract the Srb and set appropriate status values.
    // 
    if ((flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((1,
                   "I2OReserveReleaseCallBack: Failed MsgFlags\n"));

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));
    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {

        DebugPrint((1,
                   "I2OReserveReleaseCallBack: Failed ReqStatus\n"));

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             0);
    } else {

        callbackObject = pointer64ToU32.Pointer;

        //
        // Get the Srb and set status.
        // 
        srb = callbackObject->Context;

        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }

    //
    // Complete the request.
    // 
    I2OCompleteRequest(deviceExtension,
                         srb);
    return;
}


ULONG 
I2OReserve (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine will issue a DEVICE_RESERVE message in response to a SCSIOP_RESERVE.

Arguments:  

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The SRB containing the RESERVE CDB
    
Return Value:

    PENDING or ERROR, if SendMsg fails.

--*/
{
    PI2O_DEVICE_INFO deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    PIOP_CALLBACK_OBJECT callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    POINTER64_TO_U32     pointer64ToU32;
    I2O_UTIL_DEVICE_RESERVE_MESSAGE reserve;
    ULONG status = SRB_STATUS_PENDING;

    memset(&reserve, 0 , sizeof(I2O_UTIL_DEVICE_RESERVE));
       
    //
    // Build the reserve message.
    // 
    reserve.StdMessageFrame.VersionOffset= I2O_VERSION_11;
    reserve.StdMessageFrame.MsgFlags = 0;
    reserve.StdMessageFrame.MessageSize = sizeof(I2O_UTIL_DEVICE_RESERVE_MESSAGE)/4;
    reserve.StdMessageFrame.TargetAddress = deviceInfo->Lct.LocalTID;
    reserve.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    reserve.StdMessageFrame.Function = I2O_UTIL_DEVICE_RESERVE;
    reserve.StdMessageFrame.InitiatorContext = 0;
    
    DebugPrint((1,
                  "I2OReserve: Send reserve to TID 0x%x\n",
                   deviceInfo->Lct.LocalTID));
    //
    // Build the call-back object.
    // 
    callback->Callback.Signature       = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackAddress = I2OReserveReleaseCallBack;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callback;

    reserve.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    reserve.TransactionContext = pointer64ToU32.u.HighPart;

    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&reserve)) {
        DebugPrint((1,
                   "I2OReserve: Failure sending message\n"));
        status = SRB_STATUS_ERROR;
    }

    return status;
}


ULONG 
I2ORelease (
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine sends a DEVICE_RELEASE message in response to the SCSIOP_RELEASE

Arguments:  

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The SRB containing thre RELEASE
    
Return Value:

    PENDING or ERROR, if SendMessage fails.

--*/
{
    PI2O_DEVICE_INFO deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId];
    PIOP_CALLBACK_OBJECT callback = (PIOP_CALLBACK_OBJECT)Srb->SrbExtension;
    POINTER64_TO_U32     pointer64ToU32;
    I2O_UTIL_DEVICE_RELEASE_MESSAGE release;
    ULONG status = SRB_STATUS_PENDING;

    memset(&release, 0 , sizeof(I2O_UTIL_DEVICE_RELEASE_MESSAGE));

    //
    // Build the release message.
    // 
    release.StdMessageFrame.VersionOffset= I2O_VERSION_11;
    release.StdMessageFrame.MsgFlags = 0;
    release.StdMessageFrame.MessageSize = sizeof(I2O_UTIL_DEVICE_RELEASE_MESSAGE)/4;
    release.StdMessageFrame.TargetAddress = deviceInfo->Lct.LocalTID;
    release.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    release.StdMessageFrame.Function = I2O_UTIL_DEVICE_RELEASE;
    release.StdMessageFrame.InitiatorContext = 0;
    
    DebugPrint((1,
                "I2ORelease: Send release to TID 0x%x\n",
                deviceInfo->Lct.LocalTID));
       
    //
    // Build the call-back object.
    // 
    callback->Callback.Signature       = CALLBACK_OBJECT_SIG;
    callback->Callback.CallbackAddress = I2OReserveReleaseCallBack;
    callback->Callback.CallbackContext = DeviceExtension;
    callback->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callback;

    release.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    release.TransactionContext = pointer64ToU32.u.HighPart;

    if (!I2OSendMessage(DeviceExtension,
                        (PI2O_MESSAGE_FRAME)&release)) {
        DebugPrint((1,
                   "I2ORelease: Failure sending message\n"));

        status = SRB_STATUS_ERROR;
    }
    
    return status;
}


BOOLEAN
I2OBuildIo(
    IN PVOID DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    Main entry point for all OS requests from StorPort.  
    This routine is only used to build IO requests in the SrbExtension
    at lower IRQL levels than the I2OStartIo routine saving valuable
    CPU time at higher IRQ levels.   After this routine is called,
    StorPort will immediately call the I2OStartIo routine to complete
    the transfer of the request that is built here.  Note that for
    porting simplicity, the only requests that will be built in this
    routine are reads and writes. 

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The request
    
Return Value:

    TRUE - This routine handled the request.

--*/
{
    PDEVICE_EXTENSION   deviceExtension = DeviceExtension;
    PI2O_DEVICE_INFO    deviceInfo = &deviceExtension->DeviceInfo[Srb->TargetId];

    if (deviceExtension->AdapterFlags & AFLAGS_SHUTDOWN) {
        DebugPrint((0,
                    "Getting a request after being shutdown\n"));
    }    
    
    //
    // I2OBuildIo is only used to build requests that are either to a SCSI passthru
    // device or read write requests from the OS.  Both of these types of requests
    // make use of DMA SGLists and should be built in this routine for optimum
    // performance.
    //
    if (Srb->Function == SRB_FUNCTION_EXECUTE_SCSI) {

        //
        // Check for pass-through devices.
        //
        if ((deviceInfo->Class == I2O_CLASS_SCSI_PERIPHERAL) &&
            (deviceInfo->DeviceFlags & DFLAGS_CLAIMED)) {
                
            //
            // Build the I2O request for this passthru device.
            //
            //
            I2OBuildExecCDBToIOP(deviceExtension,Srb);

        } else if ((Srb->Cdb[0] == SCSIOP_WRITE) || (Srb->Cdb[0] == SCSIOP_READ)) {

            // 
            // Build the OS read or write request.
            //
            I2OBuildReadWrite(deviceExtension,Srb);
        }
    }
    
    return TRUE;
}


BOOLEAN
I2OStartIo(
    IN PVOID DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:
    Main entry point for all OS requests.  This routine will
    decode the Srb->Function and the Cdb opcode to determine
    how to handle this request.  First it insures that the 
    device and the controller are currently accepting requests
    and if not, returns the requst with an appropriate error
    code.

Arguments:
    DeviceExtension
    Srb - The request
    
Return Value:
    TRUE - The request was handled.
--*/

{
    PDEVICE_EXTENSION   deviceExtension = DeviceExtension;
    ULONG               adapterFlags = deviceExtension->AdapterFlags;
    PI2O_DEVICE_INFO    deviceInfo = &deviceExtension->DeviceInfo[Srb->TargetId];
    BOOLEAN             allow = FALSE;
    ULONG               status;
    
    //
    // Increment the Outstanding IO Count for this device.
    //
    deviceInfo->OutstandingIOCount++;
    deviceInfo->TotalRequestCount++;
#if DBG    

    //
    // Keep a high-water mark.
    //
    if (deviceInfo->OutstandingIOCount > deviceInfo->MaxIOCount) {
        deviceInfo->MaxIOCount = deviceInfo->OutstandingIOCount;
        
    }  
#endif    

    //
    // Check some flag values to ensure it's OK to continue.
    //
    // Locked indicates that some async handling of LCT/Claims/Removes
    // it going on.
    // InShutdown says that the interrupts are disabled due to a shutdown
    // request.
    //
    if ((adapterFlags & (AFLAGS_LOCKED | AFLAGS_SHUTDOWN | AFLAGS_DEVICE_RESET)) ||
        ((deviceInfo->DeviceFlags & DFLAGS_PAUSED) &&
         (Srb->Function != SRB_FUNCTION_IO_CONTROL))) {

        //
        // If locked or shutdown, nothing goes through.
        // 
        DebugPrint((0,
                    "I2OStartIo (%x): AFlags set (%x), Device (%x) Dflags (%x) returning busy\n",
                    deviceExtension,
                    deviceExtension->AdapterFlags,
                    deviceInfo,
                    deviceInfo->DeviceFlags));

        Srb->SrbStatus = SRB_STATUS_BUSY;

        I2OCompleteRequest(deviceExtension,
                           Srb);

        return TRUE;
    }

    //
    // Make sure that this device is not curently being 
    // removed or having problems.  If it is, then don't 
    // allow any new IO.  The only exception is that inquiries
    // are always allowed in so that state changes can be
    // picked up by the OS.
    //
    if ((deviceInfo->DeviceState & DSTATE_NORECOVERY) ||
        (deviceInfo->DeviceState & DSTATE_FAULTED)) {
        
        if ((!(deviceInfo->DeviceState & DSTATE_COMPLETE)) || 
            (Srb->Function != SRB_FUNCTION_EXECUTE_SCSI) || 
            (Srb->Cdb[0] != SCSIOP_INQUIRY)) {
            
            Srb->SrbStatus = SRB_STATUS_NO_DEVICE;
            I2OCompleteRequest(deviceExtension,Srb);
            return TRUE;
        }
    }

    switch (Srb->Function) {
        case SRB_FUNCTION_EXECUTE_SCSI: {
            
            //
            // SCSI devices get special treatment. Handle these via the 'passthrough' 
            // routine.
            //
            if ((deviceExtension->DeviceInfo[Srb->TargetId].Class == I2O_CLASS_SCSI_PERIPHERAL) &&
                (deviceInfo->DeviceFlags & DFLAGS_CLAIMED)) {

                status = I2OSendExecCDBToIOP(deviceExtension,
                                             Srb);
                Srb->SrbStatus = (UCHAR)status;
                break;
            }

            switch (Srb->Cdb[0]) {
                case SCSIOP_MEDIUM_REMOVAL:

                    //
                    // BsaPowerManagement 6.4.6.13
                    //
                    Srb->SrbStatus = SRB_STATUS_SUCCESS;
                    break;
                case SCSIOP_REQUEST_SENSE:

                    //
                    // TODO: Any thing that can be done?
                    //
                    DebugPrint((1,
                               "I2OStartIo: Request Sense\n"));
                    Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
                    break;

                case SCSIOP_MODE_SENSE:

                    DebugPrint((1,
                               "I2OStartIo: Mode sense\n"));

                    status = I2OModeSense(deviceExtension,
                                          Srb);

                    Srb->SrbStatus = (UCHAR)status;
                    break;

                case SCSIOP_READ_CAPACITY:

                    status = I2ODriveCapacity(deviceExtension,
                                            Srb);
                    Srb->SrbStatus = (UCHAR)status;
                    break;

                case SCSIOP_WRITE:
                case SCSIOP_READ:
                    status = I2OSendReadWrite(deviceExtension,
                                          Srb);
                    Srb->SrbStatus = (UCHAR)status;

                    //
                    // Only two status values returned here - pending or busy.
                    // If it's pending, return immediately. There is a race
                    // condition present where ReadCompletion and I2OCompleteRequest
                    // can complete the same request.
                    // If BUSY, then the request wasn't submitted so it's OK
                    // to go into I2OCompeleteRequest.
                    //
                    if (Srb->SrbStatus != SRB_STATUS_BUSY ) {
                        return TRUE;
                    }
                    break;

                case SCSIOP_INQUIRY:

                    status = I2OInquiry(deviceExtension,
                                        Srb);

                    Srb->SrbStatus = (UCHAR)status;
                    break;

                case SCSIOP_TEST_UNIT_READY:
                    
                    //
                    // Claim success for now.
                    // TODO: 6.4.6.14
                    //
                    status = SRB_STATUS_SUCCESS;
                    Srb->SrbStatus = (UCHAR)status;
                    break;

                case SCSIOP_VERIFY:
                    
                    status = I2OVerify(deviceExtension,
                                       Srb);
                    Srb->SrbStatus = (UCHAR)status;
                    break;
                    
                case SCSIOP_SYNCHRONIZE_CACHE:

                    status = I2OShutdownFlush(deviceExtension,
                                              Srb);
                    Srb->SrbStatus = (UCHAR)status;
                    break;
                    
                case SCSIOP_START_STOP_UNIT:

                    status = I2OBSAPowerMgt(deviceExtension, Srb);
                    Srb->SrbStatus = (UCHAR)status;
                    break;
                    
                case SCSIOP_RESERVE_UNIT:

                    status = I2OReserve(deviceExtension,
                                              Srb);
                    Srb->SrbStatus = (UCHAR)status;
                    break;

                case SCSIOP_RELEASE_UNIT:
                    status = I2ORelease(deviceExtension,
                                              Srb);
                    Srb->SrbStatus = (UCHAR)status;
                    break;

                default:

                    DebugPrint((1,
                               "I2OStartIo: Unhandled opcode %x\n",
                               Srb->Cdb[0]));
                    Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
                    break;
            }
            break;

        }
        case SRB_FUNCTION_SHUTDOWN:

            DebugPrint((1,
                        "I2OStartIO: Shutdown\n"));

            if (deviceExtension->IsmTID == 0x0) {

                //
                // no ISM available or card doesn't support the mode, just send flush
                //
                DebugPrint((1,
                            "NoSpecialShutdown: Shutdown called, flush instead\n"));
                status = I2OShutdownFlushSync(deviceExtension,
                          Srb);
            } else {
                if (!(deviceExtension->AdapterFlags & AFLAGS_SHUTDOWN_RECVD)) {                       
                    //
                    // send some special commands to the ISM to
                    // invoke a 'get ready for shutdown mode'
                    //

                    status = I2OHandleShutdown(deviceExtension);
                } else {
                    status = SRB_STATUS_SUCCESS;
                }    
                deviceExtension->AdapterFlags |= AFLAGS_SHUTDOWN_RECVD;
            }

            // 
            // Indicate that the system is shutting down. This flag is used to
            // help keep the cache flushed so that the volumes remain consistent.
            //
            deviceExtension->TempShutdown = TRUE;

            Srb->SrbStatus = (UCHAR)status;
            break;
            
        case SRB_FUNCTION_FLUSH:

            status = I2OShutdownFlush(deviceExtension,
                                      Srb);
            Srb->SrbStatus = (UCHAR)status;

            break;

        case SRB_FUNCTION_IO_CONTROL:

            status = I2OSrbControl(deviceExtension,
                                   Srb);
            Srb->SrbStatus = (UCHAR)status;
            break;

        case SRB_FUNCTION_RESET_LOGICAL_UNIT:
        case SRB_FUNCTION_RESET_DEVICE: {

            //
            // These both get treated as the same thing as there are no differences
            // between targets and luns in this driver (NOTE: except maybe for some
            // SCSI Pass-through devices).
            //
            // Reset the device.
            //
            status = I2OResetDevice(deviceExtension,
                                    Srb);

            Srb->SrbStatus = (UCHAR)status;
            break;
        }                                            
                                                
        default:

            DebugPrint((1,
                       "I2OStartIo: Unhandled Srb function %x\n",
                       Srb->Function));
            Srb->SrbStatus = SRB_STATUS_INVALID_REQUEST;
            break;
    }

    I2OCompleteRequest(deviceExtension,
                       Srb);

    return TRUE;
}


BOOLEAN 
I2OSysQuiesce(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine will send the SUS_QUIESCE message to shutdown external operations on the IOP. 

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    
Return Value:

    TRUE if the SysQ was successful.

--*/

{
    I2O_EXEC_SYS_QUIESCE_MESSAGE sysQuiesce;
    BOOLEAN status = FALSE;
    PIOP_CALLBACK_OBJECT callBackObject;
    POINTER64_TO_U32 pointer64ToU32;
    ULONG i;

    callBackObject = &DeviceExtension->InitCallback;
    callBackObject->Context = &DeviceExtension->CallBackComplete;

    memset(&sysQuiesce, 0 , sizeof(I2O_EXEC_SYS_QUIESCE_MESSAGE));
    
    sysQuiesce.StdMessageFrame.VersionOffset= I2O_VERSION_11;
    sysQuiesce.StdMessageFrame.MsgFlags = 0;
    sysQuiesce.StdMessageFrame.MessageSize = sizeof(I2O_EXEC_SYS_QUIESCE_MESSAGE)/4;
    sysQuiesce.StdMessageFrame.TargetAddress = 0;
    sysQuiesce.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    sysQuiesce.StdMessageFrame.Function = I2O_EXEC_SYS_QUIESCE;
    sysQuiesce.StdMessageFrame.InitiatorContext = 0;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callBackObject;

    sysQuiesce.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    sysQuiesce.TransactionContext = pointer64ToU32.u.HighPart;

    DeviceExtension->CallBackComplete = 0;

    if (I2OSendMessage(DeviceExtension,(PI2O_MESSAGE_FRAME)&sysQuiesce)) {

        //
        // Check for completion.
        //
        if (I2OGetCallBackCompletion(DeviceExtension,
                                     200000)) {
            return TRUE;
        } 
    }     

    return FALSE; 
}


BOOLEAN
I2OIopClear(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine is used in Reset processing to Clear outstanding msgs on the IOP.
    It's currently not called, but keep it around just in case.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension.
    
Return Value:

    TRUE - if successfull.

--*/

{
    I2O_EXEC_IOP_CLEAR_MESSAGE msg;
    BOOLEAN status = FALSE;
    PIOP_CALLBACK_OBJECT callBackObject;
    POINTER64_TO_U32 pointer64ToU32;
    ULONG i;

    callBackObject = &DeviceExtension->InitCallback;
    callBackObject->Context = &DeviceExtension->CallBackComplete;

    memset(&msg, 0 , sizeof(I2O_EXEC_IOP_CLEAR_MESSAGE));

    msg.StdMessageFrame.VersionOffset= I2O_VERSION_11;
    msg.StdMessageFrame.MsgFlags = 0;
    msg.StdMessageFrame.MessageSize = sizeof(I2O_EXEC_IOP_CLEAR_MESSAGE)/4;
    msg.StdMessageFrame.TargetAddress = 0;
    msg.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    msg.StdMessageFrame.Function = I2O_EXEC_IOP_CLEAR;
    msg.StdMessageFrame.InitiatorContext = 0;
                                            

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callBackObject;

    msg.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg.TransactionContext = pointer64ToU32.u.HighPart;

    DeviceExtension->CallBackComplete = 0;

    if (I2OSendMessage(DeviceExtension,(PI2O_MESSAGE_FRAME)&msg)) {

        //
        // Check for completion.
        //
        if (I2OGetCallBackCompletion(DeviceExtension,
                                     100000)) {
            return TRUE;
        } 
    }     

    return FALSE; 
}



BOOLEAN
I2OResetBus(
    IN PVOID DeviceExtension,
    IN ULONG PathId
    )
/*++

Routine Description:

    This routine is called by the port-driver to clear error conditions. It will reset
    the IOP or the devices depending upon whether the IOP is stuck.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    PathID - Indicates which 'port' on the adapter needs the reset. It's always 0 for this
             driver.
    
Return Value:

    TRUE - If the reset was successful.

--*/
{
    PDEVICE_EXTENSION deviceExtension = DeviceExtension;
    PI2O_EXEC_STATUS_GET_REPLY statusReply;
    PVOID                  statusBuffer;
    STOR_PHYSICAL_ADDRESS  statusBufferPhysicalAddress;
    BOOLEAN hardReset = FALSE;
    BOOLEAN quiesceIOP = TRUE;
    BOOLEAN operational = FALSE;

    //
    // When in the crashdump or hibernate paths the dump
    // driver will issue a bus reset after initializing 
    // the driver (via the normal driverEntry path).  Since
    // the IOP has just been reset as part of the 
    // IOP initialization, don't issue another one.
    //
    // If the driver is already in the middle of a Bus Reset request,
    // don't allow another one to get through or it will corrupt the
    // reset count for the already outstanding resets.  When running 
    // in a clustering environment and doing fail overs, some machines 
    // seem to get impatient and send multiple resets.
    //
    if ((deviceExtension->AdapterFlags & AFLAGS_IN_LDR_DUMP) ||
        (deviceExtension->AdapterFlags & AFLAGS_DEVICE_RESET)) {
        
        StorPortCompleteRequest(DeviceExtension,
                                (UCHAR)PathId,
                                0xFF,
                                0xFF,
                                SRB_STATUS_BUS_RESET);
        return TRUE;
    }

    //
    // Determine the IOP's state.
    //
    statusBuffer  = (PVOID)deviceExtension->ScratchBuffer;
    statusBufferPhysicalAddress = deviceExtension->ScratchBufferPA;
    
    if (!(I2OGetStatus(deviceExtension,
                       statusBuffer,
                       statusBufferPhysicalAddress))) {
        DebugPrint((1,
                    "I2OResetBus: Couldn't get status\n"));
        //
        // If the IOP is unresponsive and does not return status then don't 
        // bother to SysQuiesce first.  
        //
        hardReset = TRUE;
        quiesceIOP = FALSE;

    } else {

        statusReply = statusBuffer;

        DebugPrint((1,
                    "I2OResetBus: IopState (%x)\n",
                    statusReply->IopState));

        //
        // Based on the reply, determine what actions should be carried out.
        // 
        switch (statusReply->IopState) {
            case I2O_IOP_STATE_FAILED:
            case I2O_IOP_STATE_FAULTED:
                
                quiesceIOP = TRUE;
                hardReset = TRUE;
                break;
                
            case I2O_IOP_STATE_OPERATIONAL:
                operational = TRUE;
                break;

            default:
                //
                // If the IOP state is Init, Reset, Hold or Ready
                // then it never completed system initialization
                // and should be reset.
                //
                hardReset = TRUE;
                break;
        }
    }        

    if (operational) {
        
        // 
        // Mark the flags as to suspend I/O while the reset is taking place.
        // 
        deviceExtension->AdapterFlags |= AFLAGS_DEVICE_RESET;
        
        //
        // start ResetTimer in case something goes wrong
        // 
        deviceExtension->ResetTimer = 0;
        
        StorPortNotification(RequestTimerCall,
                             deviceExtension,
                             I2OBSADeviceResetTimer,
                             BSA_RESET_TIMER);
        
        //
        // Send BSADeviceReset to BSADevices
        //
        I2OBSADeviceResetAll(deviceExtension);

        return TRUE;
    }

    if (quiesceIOP) {

        //
        // Send the SysQ.
        //
        if (!I2OSysQuiesce(deviceExtension)) {
            DebugPrint((1,
                        "I2OResetBus: SysQ failed\n"));
            hardReset = TRUE;
        }    
    }
    
    if (hardReset == FALSE) {
      
        //
        // NOTE: This code-path is currently not executed.
        // Send the IOP Clear
        //
        if (!I2OIopClear(deviceExtension)) {
            DebugPrint((1,
                        "ResetBus: IopClear failed\n"));
        }    
        if (!I2OSetSysTab(DeviceExtension)) {
            DebugPrint((1,
                        "ResetBus: SetSysTab failed\n"));
        }
        if (!I2OEnableSys(DeviceExtension)) {
            DebugPrint((1,
                        "ResetBus: EnableSys failed\n"));
        }
    } else {

        //
        // Get the TCL parameter from the IOP.  This is needed to
        // properly preserve boot context for the IOP across power
        // transitions and resets.
        //
        I2OGetSetTCLParam(deviceExtension,TRUE);

        if (!I2OSysQuiesce(deviceExtension)) {
            DebugPrint((1,
                        "ResetBus: SysQuiesce failed\n"));
        }

        I2OResetIop(deviceExtension);
        if (!I2OInitialize(deviceExtension)) {
            DebugPrint((1,
                        "ResetBus: I2OInit failed\n"));
        }    

        //
        // Restore the TCL parameter to the IOP.  This is needed to
        // properly preserve boot context for the IOP across power
        // transitions and resets.
        //
        I2OGetSetTCLParam(deviceExtension,FALSE);

        //
        // Send UTIL_SET_PARAMS message to tells the ISM to go to GUI mode
        //
        I2OSetSpecialParams(deviceExtension,
                            I2O_RAID_COMMON_CONFIG_GROUP_NO,
                            I2O_GUI_MODE_FIELD_IDX,
                            2);
    }    

    //
    // Need to rebuild everything.
    //
    
    StorPortCompleteRequest(DeviceExtension,
                            (UCHAR)PathId,
                            0xFF,
                            0xFF,
                            SRB_STATUS_BUS_RESET);

    statusBuffer  = (PVOID)deviceExtension->ScratchBuffer;
    statusBufferPhysicalAddress = deviceExtension->ScratchBufferPA;
    if (!(I2OGetStatus(deviceExtension,
                       statusBuffer,
                       statusBufferPhysicalAddress))) {
    }
    statusReply = statusBuffer;
    DebugPrint((1,
                "ResetBus: Status is now (%x)\n",
                statusReply->IopState));
    

    return TRUE;

}


BOOLEAN
I2OSendMessage(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PI2O_MESSAGE_FRAME   MessageBuffer
    )
/*++

Routine Description:

    This routine sends an I2O MessageFrame to the IOP.

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    MessageBuffer = Pointer to the I2O Message Buffer to send to the IOP.

Return Value:

    TRUE | FALSE if not successful (FIFO EMPTY)

--*/

{
    ULONG  messageFrameAddress;
    ULONG  submitState;
    ULONG  i;

    //
    // Read from the IOP's inbound FIFO to get an offset to a free message frame.
    //
    messageFrameAddress = StorPortReadRegisterUlong(DeviceExtension,
                                                    DeviceExtension->MessageAlloc);

    if (messageFrameAddress == (ULONG)-1) {
        DebugPrint((1,
                    "I2OSendMessage: Inbound fifo empty on send\n"));
        return FALSE;
    }

    DebugPrint((2,
               "I2OSendMessage: Function %x\n",
               MessageBuffer->Function));
    DebugPrint((2,
               "VersionOffset %x\n",
               MessageBuffer->VersionOffset));
    DebugPrint((2,
               "MsgFlags %x\n",
               MessageBuffer->MsgFlags));
    DebugPrint((2,
               "MessageSize %x\n",
               MessageBuffer->MessageSize));
    DebugPrint((2,
               "TargetAddress %x\n",
               MessageBuffer->TargetAddress));
    DebugPrint((2,
               "InitiatorAddress %x\n",
               MessageBuffer->InitiatorAddress));

    if ((MessageBuffer->Function == I2O_BSA_BLOCK_READ) ||
        (MessageBuffer->Function == I2O_BSA_BLOCK_WRITE)) {
        PI2O_BSA_READ_MESSAGE msg = (PI2O_BSA_READ_MESSAGE)MessageBuffer;
        PI2O_SG_ELEMENT  sgList = &msg->SGL;

        DebugPrint((2,
                   "ControlFlags %x\n",
                   msg->ControlFlags));
        DebugPrint((2,
                   "TimeMultiplier %x\n",
                   msg->TimeMultiplier));
        DebugPrint((2,
                   "FetchAhead(Reserved): %x\n",
                   msg->FetchAhead));
        DebugPrint((2,
                   "XferByteCount %x\n",
                    msg->TransferByteCount));
        DebugPrint((2,
                   "LogicalByteAddress %x\n",
                   msg->LogicalByteAddress));
        DebugPrint((2,
                   "SGL.Flags %x\n",
                   sgList->u.Page[0].FlagsCount.Flags));
        DebugPrint((2,
                   "SGL.Count %x\n",
                   sgList->u.Page[0].FlagsCount.Count));
        DebugPrint((2,
                   "SGL.PhsAddress %x\n",
                   sgList->u.Page[0].PhysicalAddress[0]));
    }
    //
    // Copy the message to the IOP memory.
    //
    StorPortWriteRegisterBufferUlong(DeviceExtension,
                                     (PULONG)((PUCHAR)DeviceExtension->MappedAddress +
                                                      messageFrameAddress),
                                (PULONG)MessageBuffer,
                                MessageBuffer->MessageSize);
    //
    // Write to the IOP's inbound FIFO to get the IOP to process the message.
    //
    StorPortWriteRegisterUlong(DeviceExtension,
                               DeviceExtension->MessagePost,
                               messageFrameAddress);

    return TRUE;
}


BOOLEAN
I2OGetStatus(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PI2O_EXEC_STATUS_GET_REPLY GetStatusReply,
    IN PHYSICAL_ADDRESS  GetStatusReplyPhys
    )
/*++

Routine Description:

    This routine performs a GET_IO_STATUS message to the IOP.

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    GetStatusReply - Virtual Address of the reply buffer.
    GetStatusReplyPhys - Physical Address of the Reply Buffer..

Return Value:

    TRUE | FALSE if not successful

--*/

{
    I2O_EXEC_STATUS_GET_MESSAGE getStatus;
    ULONG                       retry;

    //
    // Zero out the GetStatus message frames.
    //
    memset(&getStatus, 0, sizeof(I2O_EXEC_STATUS_GET_MESSAGE));

    //
    // Setup a get status message to send to the IOP.
    //
    getStatus.VersionOffset = I2O_VERSION_11;
    getStatus.MessageSize   = (U16)((sizeof(I2O_EXEC_STATUS_GET_MESSAGE) + 3) >> 2);
    getStatus.Function      = I2O_EXEC_STATUS_GET;

    //
    // Set the Source and Destination addresses.
    //
    getStatus.TargetAddress    = I2O_IOP_TID;
    getStatus.InitiatorAddress = I2O_HOST_TID;

    getStatus.ReplyBufferAddressHigh = GetStatusReplyPhys.HighPart;
    getStatus.ReplyBufferAddressLow  = GetStatusReplyPhys.LowPart;
    getStatus.ReplyBufferLength      = sizeof(I2O_EXEC_STATUS_GET_REPLY);

    //
    // Zero out the status field and clear the SyncByte.
    //
    GetStatusReply->IopState = 0;
    GetStatusReply->SyncByte = 0;

    //
    // Send the Message
    //
    retry = 50000;
    while (!I2OSendMessage(DeviceExtension,
                       (PI2O_MESSAGE_FRAME)&getStatus)) {
        StorPortStallExecution(100);
        retry--;
        if (!retry) {
            DebugPrint((0,
                       "I2OGetStatus: Send message failed\n"));
            return FALSE;
        }
    }

    //
    // Read the Reply Status.
    //
    retry = 300000;
    while (!GetStatusReply->SyncByte) {
        StorPortStallExecution(100);
        retry--;
        if (!retry) {
            DebugPrint((0,
                        "I2OGetSTatus: Timed out\n"));
            return FALSE;
        }
    }

    DebugPrint((0,
                "I2OSetStatus: IOP State: %x\n",
                GetStatusReply->IopState));
    //
    // Anyone who calls this should be looking for the appropriate state.
    //
    return TRUE;
}


BOOLEAN
I2OInitOutbound(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine is used to init the outbound queue.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    
Return Value:

    TRUE - if successful.

--*/
{
    I2O_EXEC_OUTBOUND_INIT_MESSAGE initOutbound;
    PI2O_EXEC_OUTBOUND_INIT_STATUS initOutboundReply;
    ULONG messageFrameAddress;
    ULONG i;
    ULONG retry;

    //
    // Zero out the initOutbound message frame.
    //
    memset(&initOutbound, 0, sizeof(I2O_EXEC_OUTBOUND_INIT_MESSAGE));

    //
    // Setup the Initialize Outbound FIFO Message.
    //
    initOutbound.StdMessageFrame.VersionOffset = I2O_VERSION_11 | I2O_OUT_INIT_SGL_VER_OFFSET;
    initOutbound.StdMessageFrame.MessageSize = (sizeof(I2O_EXEC_OUTBOUND_INIT_MESSAGE) -
                                                sizeof(I2O_SG_ELEMENT) +
                                                sizeof(I2O_SGE_SIMPLE_ELEMENT)) >> 2;
    initOutbound.StdMessageFrame.Function = I2O_EXEC_OUTBOUND_INIT;

    //
    // Set the Source and Destination addresses.
    //
    initOutbound.StdMessageFrame.TargetAddress = I2O_IOP_TID;
    initOutbound.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    initOutbound.TransactionContext = DeviceExtension->ReplyBufferPA.LowPart;
    initOutbound.HostPageFrameSize  = PAGE_SIZE;
    initOutbound.InitCode           = I2O_MESSAGE_IF_INIT_CODE_OS;
    initOutbound.OutboundMFrameSize = (U16)(DeviceExtension->MessageFrameSize >> 2);

    //
    // Setup the SGL entry for the InitOutbound message.
    // Note:  The 1.5 spec changes the definition of the first SGL entry.  It is now
    //        the reply location where status is deposited by the IOP.  It also happens
    //        to be the TransactionContext field of the message in the pre 1.5 spec.  In
    //        order to maintain compatibility between pre and post 1.5 compliant IOPs,
    //        we set the SGL and the TransactionContext field to the same logical address.
    //
    initOutbound.SGL.u.Simple[0].FlagsCount.Count = 4;
    initOutbound.SGL.u.Simple[0].FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT |
                                                    I2O_SGL_FLAGS_SIMPLE_ADDRESS_ELEMENT |
                                                    I2O_SGL_FLAGS_END_OF_BUFFER;
    initOutbound.SGL.u.Simple[0].PhysicalAddress  = DeviceExtension->ReplyBufferPA.LowPart;

    //
    // Setup a pointer to the reply buffer and clear the status.
    //
    initOutboundReply = (PI2O_EXEC_OUTBOUND_INIT_STATUS)DeviceExtension->ReplyBuffer;
    initOutboundReply->InitStatus = 0;

    //
    // Send the InitOutbound Message
    //
    if (!I2OSendMessage(DeviceExtension,
                       (PI2O_MESSAGE_FRAME)&initOutbound)) {
        DebugPrint((1,
                   "I2OInitOubound: Send message failed\n"));
        return FALSE;
    }

    //
    // Read the Reply Status.
    //
    retry = 600000;
    while (!initOutboundReply->InitStatus ||
           (initOutboundReply->InitStatus == I2O_EXEC_OUTBOUND_INIT_IN_PROGRESS )) {
        StorPortStallExecution(100);
        retry--;
        if (!retry) {

            DebugPrint((1,
                        "I2OInitOutbound: Timeout on Init Outbound Message.\n"));
            return FALSE;
        }
    }
    
    DebugPrint((1,
               "I2OInitOutbound status %x\n",
               initOutboundReply->InitStatus));
    //
    // 2 - rejected
    // 3 - failed
    // 4 - complete
    // 

    //
    // Post to the IOP's outbound FIFO the addresses of the reply MessageFrames.
    //
    for (i=0; i < DeviceExtension->NumberReplyBuffers; i++) {
        messageFrameAddress = DeviceExtension->ReplyBufferPA.LowPart +
                                  (i * DeviceExtension->MessageFrameSize);

        //
        // Post the buffer (reply message frame).
        //
        StorPortWriteRegisterUlong(DeviceExtension,
                                   DeviceExtension->MessageRelease,
                                   messageFrameAddress);
    }

    return TRUE;
}


BOOLEAN
I2OResetIop(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine will reset the IOP. The device array is marked to show that
    all claims have been lost, and a reset message sent.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    
Return Value:

    TRUE - If the reset was successful.

--*/
{
    ULONG retry = 5000;
    ULONG  msg;
    ULONG  numRetry;
    ULONG  i;
    I2O_EXEC_IOP_RESET_MESSAGE  resetMsg;
    PI2O_EXEC_IOP_RESET_STATUS  resetReply;

    //
    // Ensure that the fifo is ready.
    //
    do {
        StorPortStallExecution(1000);
        msg = StorPortReadRegisterUlong(DeviceExtension,
                                        DeviceExtension->MessageAlloc);
    } while ((msg == (ULONG)-1) && retry--);

    if (msg == (ULONG)-1) {

        DebugPrint((1,
                   "I2OReset: Couldn't get a msg frame\n"));

        //
        // Log this error. The adapter is probably hosed.
        // 
        StorPortLogError(DeviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         SP_INTERNAL_ADAPTER_ERROR,
                         I2OMP_ERROR_INSUFFICIENT_RESOURCES);
        return FALSE;
    }

    //
    // Clear the OS_CAN_CLAIM flag from all devices before the reset.
    //
    for (i=0; i < IOP_MAXIMUM_TARGETS; i++) {
        DeviceExtension->DeviceInfo[i].DeviceFlags &= ~DFLAGS_OS_CAN_CLAIM;
    }

    //
    // Build the reset message.
    //
    for (i = 0; i < I2O_EXEC_IOP_RESET_RESERVED_SZ; i++) {
        resetMsg.Reserved[i] = 0;
    }
    resetMsg.MsgFlags = 0;
    resetMsg.VersionOffset = I2O_VERSION_11;
    resetMsg.MessageSize = (USHORT)((sizeof(I2O_EXEC_IOP_RESET_MESSAGE) + 3) >> 2);
    resetMsg.Function = I2O_EXEC_IOP_RESET;

    //
    // Set the Source and Destination addresses.
    //
    resetMsg.TargetAddress = I2O_IOP_TID;
    resetMsg.InitiatorAddress = I2O_HOST_TID;
    resetMsg.StatusWordHighAddress = 0;
    resetMsg.StatusWordLowAddress = (ULONG)DeviceExtension->ReplyBufferPA.LowPart;

    //
    // Setup a pointer to the reply buffer and clear the status.
    //
    resetReply = (PI2O_EXEC_IOP_RESET_STATUS)DeviceExtension->ReplyBuffer;
    resetReply->ResetStatus = 0;

    //
    // Send the reset
    //
    if (!I2OSendMessage(DeviceExtension,
                       (PI2O_MESSAGE_FRAME)&resetMsg)) {

        DebugPrint((1,
                   "I2OResetIop: Couldn't send the reset.\n"));
        return FALSE;
    }

    //
    // Wait on reset status for 20 seconds.
    //
    numRetry = 200000;
    while (!resetReply->ResetStatus) {

        StorPortStallExecution(100);
        numRetry--;
        if (numRetry == 0) {

            DebugPrint((0,
                        "I2OReset: Timed out waiting. Status %x\n",
                        resetReply->ResetStatus));
            
            StorPortLogError(DeviceExtension,
                             NULL,
                             0,
                             0,
                             0,
                             SP_INTERNAL_ADAPTER_ERROR,I2OMP_ERROR_RESETTIMEOUT);
            return FALSE;
        }
    }
    DebugPrint((0,
               "I2OResetIop: Reset Status %x\n",
               resetReply->ResetStatus));

    retry = 5000;

    //
    // Ensure that the fifo is ready.
    //
    do {
        StorPortStallExecution(1000);
        msg = StorPortReadRegisterUlong(DeviceExtension,
                                        DeviceExtension->MessageAlloc);
    } while ((msg == (ULONG)-1) && retry--);

    if (msg == (ULONG)-1) {

        DebugPrint((0,
                   "I2OReset: Couldn't get a msg frame after IOPReset\n"));
        
        StorPortLogError(DeviceExtension,
                         NULL,
                         0,
                         0,
                         0,
                         SP_INTERNAL_ADAPTER_ERROR,
                         I2OMP_ERROR_INSUFFICIENT_RESOURCES);
        return FALSE;
    }

    //
    // Ensure that the status is good by sending an ExecStatusGet.
    //
    if (!I2OGetStatus(DeviceExtension,
                      (PI2O_EXEC_STATUS_GET_REPLY)DeviceExtension->ReplyBuffer,
                      DeviceExtension->ReplyBufferPA)) {
        DebugPrint((0,"I2OReset: Status is not good\n"));

        return FALSE;
        
    }

    DebugPrint((0,
                "I2OReset: Status good\n"));
    return TRUE;
}


PSCSI_REQUEST_BLOCK
I2OHandleError(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PDEVICE_EXTENSION DeviceExtension,
    IN ULONG TransactionContextOffset
    )
/*++

Routine Description:
    
    This routine will extract the Srb from the original message or from the context, then
    update the status values by mapping the error into srb and scsi status values.
    
Arguments:

    Msg - The error message fram
    DeviceExtension - This miniport's HwDeviceExtension
    TransactionContextOffset - The offset of the TC in the original message .
    
Return Value:

    The SRB

--*/
{
    PI2O_FAILURE_REPLY_MESSAGE_FRAME failMsg = (PI2O_FAILURE_REPLY_MESSAGE_FRAME)Msg;
    PSCSI_REQUEST_BLOCK   srb;
    PIOP_CALLBACK_OBJECT callbackObject;
    POINTER64_TO_U32      pointer64ToU32;
    ULONG_PTR addressToRead;
    ULONG flags;
    ULONG i;

    flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags;

    DebugPrint((1,
               "I2OHandleError: Msg %x Flags %x\n",
               Msg,
               flags));

    if (flags & I2O_MESSAGE_FLAGS_FAIL) {

        //
        // If the message failed we need to reach across into IOP memory and retrieve
        // the Srb out of the original message that we sent.

        addressToRead = (ULONG_PTR)(DeviceExtension->MappedAddress);
        addressToRead += (ULONG_PTR)(failMsg->PreservedMFA.LowPart + INITIATOR_CONTEXT_OFFSET);

        StorPortReadRegisterBufferUlong(DeviceExtension,
                                        (PULONG)addressToRead,
                                        (PULONG)&pointer64ToU32.u.LowPart,
                                        1);

        addressToRead = (ULONG_PTR)((PUCHAR)DeviceExtension->MappedAddress);
        addressToRead += (ULONG_PTR)(failMsg->PreservedMFA.LowPart + TransactionContextOffset);

        StorPortReadRegisterBufferUlong(DeviceExtension,
                                        (PULONG)addressToRead,
                                        (PULONG)&pointer64ToU32.u.HighPart,
                                        1);

    } else {
        pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;
    }

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer
    // and the TransactionContext (for NT64) contains the high 32 bits.  Contained
    // in the callback object is the srb associated with the IO completion.
    //
    callbackObject = pointer64ToU32.Pointer;
    srb = callbackObject->Context;

    if (flags & I2O_MESSAGE_FLAGS_FAIL) {

        if (failMsg->FailureCode == I2O_FAILURE_CODE_TRANSPORT_INVALID_TARGET_ID) {

            srb->SrbStatus = SRB_STATUS_SELECTION_TIMEOUT;
        }    
    } else {

        //
        // Translate status to srb status. 
        //
        srb->SrbStatus = (UCHAR)MapError((PI2O_SINGLE_REPLY_MESSAGE_FRAME)Msg,
                                         DeviceExtension,
                                         srb);
    }    

    return srb;
}


VOID
I2OReadCompletion(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )

/*++

Routine Description:

    Process a read completion from a block storage device.  This routine
    handles all the aspects of completing a request.

Arguments:

    Msg             - message describing the completed request
    DeviceContext   - device extension for the device this request was issued against

Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION   deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT  callbackObject;
    ULONG               flags;
    ULONG               status;
    PSCSI_REQUEST_BLOCK srb;
    PSCSI_REQUEST_BLOCK writeSrb;
    POINTER64_TO_U32    pointer64ToU32;

    //
    // Check for any error status.
    //
    flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags;

    if (flags & I2O_MESSAGE_FLAGS_FAIL) {

        //
        // This will set the status values and extract the srb.
        //
        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));
        DebugPrint((1,
                   "I2OReadCompletion: Read Message FAILURE Tid 0x%x\n",
                   Msg->BsaReplyFrame.StdMessageFrame.TargetAddress));

    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
       
        //
        // Get the srb and the error status values.
        // 
        srb = I2OHandleError(Msg,
                             deviceExtension,
                             0);
        DebugPrint((1,
                   "I2OReadCompletion: Read Message ERROR  Tid 0x%x\n",
                   Msg->BsaReplyFrame.StdMessageFrame.TargetAddress));

    } else {

        pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

        callbackObject = pointer64ToU32.Pointer;
        srb  = callbackObject->Context;

        //
        // Update data transfer length.
        //
        if (srb->DataTransferLength > Msg->TransferCount) {
            
            srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
            srb->DataTransferLength = Msg->TransferCount;

        } else if (srb->DataTransferLength < Msg->TransferCount) {
            srb->SrbStatus = SRB_STATUS_ERROR;
        } else {
            srb->SrbStatus = SRB_STATUS_SUCCESS;
        }
    }

    //
    // Complete the request.
    // 
    I2OCompleteRequest(deviceExtension,
                       srb);
    return;
}


VOID
I2OWriteCompletion(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    This is the callback for write operations. It will verify a successful completion or 
    determine what error did occur, then complete the Srb.
    
Arguments:

    Msg - The write message frame.
    DeviceContext - the deviceExtension.
    
    
Return Value:

    NOTHING

--*/

{

    PDEVICE_EXTENSION   deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT  callbackObject;
    ULONG               flags;
    ULONG               status;
    PSCSI_REQUEST_BLOCK srb;
    POINTER64_TO_U32    pointer64ToU32;


    //
    // Check for any error status.
    //
    flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags;

    if (flags & I2O_MESSAGE_FLAGS_FAIL) {

        //
        // Extract the srb and get the error information.
        // 
        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));
        DebugPrint((1,
                   "I2OWriteCompletion:  Write Message FAILURE Tid 0x%x\n",
                   Msg->BsaReplyFrame.StdMessageFrame.TargetAddress));

    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
        
        //
        // This will set the appropriate status values and extract the srb.
        // 
        srb = I2OHandleError(Msg,
                             deviceExtension,
                             0);
        DebugPrint((1,
                   "I2OWriteCompletion: Write Message ERROR Tid 0x%x\n",
                   Msg->BsaReplyFrame.StdMessageFrame.TargetAddress));
    } else {

        pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

        callbackObject = pointer64ToU32.Pointer;
        srb  = callbackObject->Context;

        //
        // Update data transfer length.
        //
        if (srb->DataTransferLength > Msg->TransferCount) {
            srb->SrbStatus = SRB_STATUS_DATA_OVERRUN;
            srb->DataTransferLength = Msg->TransferCount;
        } else if (srb->DataTransferLength < Msg->TransferCount) {
            srb->SrbStatus = SRB_STATUS_ERROR;
        } else {
            srb->SrbStatus = SRB_STATUS_SUCCESS;
        }
    }

    //
    // Check for post sysQ writes.
    // there's 2 shutdown flags, one tells us the system is shutdown and this
    // one tells us whether we need a pause or not depending on the IO being read or write
    //
    //
    if ((deviceExtension->TempShutdown) &&
        (!deviceExtension->SpecialShutdown)) {
        
        //
        //  delay before replying to allow ISM to go clean for R5
        //  
        deviceExtension->PostShutdownActivity = TRUE;
    }

    //
    // Complete the request.
    //
    I2OCompleteRequest(deviceExtension,
                       srb);
    return;
}


VOID
I2OCompleteRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine handles completing the request, and does some checking
    for devices in error states.
    
Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    Srb - Completing Srb
    
Return Value:

    NOTHING

--*/
{
    PI2O_DEVICE_INFO deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId]; 
    ULONG            status;

    //
    // post sysQ activity, delay before replying to allow ISM to go clean for R5
    // only do this for the case where these's no ISM or its a card that doesn't
    // support the special mode invoked in StartIO AND we're both shutdown and
    // the last IO was a write. 
    //
    if ((!DeviceExtension->SpecialShutdown) &&
        (DeviceExtension->PostShutdownActivity)) {

        //
        // Flush the cache.
        //
        status = I2OShutdownFlushSync(DeviceExtension,
                                      Srb);
        DebugPrint((1,
                   "I2OCompleteRequest: Send Sync Flush, status= (%x)\n",
                   status));

        status = I2OSetSpecialParams(DeviceExtension,
                                    I2O_RAID_COMMON_CONFIG_GROUP_NO,
                                    I2O_GUI_MODE_FIELD_IDX,
                                    2);
        DebugPrint((1,
                    "I2OCompleteRequest: Send SetParams to go clean, status= (%x)\n",
                    status));

        DeviceExtension->PostShutdownActivity = FALSE;

    }

    //
    // If the request isn't pending - it's either completed succesfully or
    // with an error, indicate this.
    //
    if (Srb->SrbStatus != SRB_STATUS_PENDING) {

        //
        // Decrement the Outstanding IO Count.
        //
        deviceInfo->OutstandingIOCount--;

        //
        // See if the count has gone to zero, and check for any devices
        // in various not-well conditions.
        //
        if (deviceInfo->OutstandingIOCount == 0) {
            
            //
            // What state is the device currently in?
            //
            if ((deviceInfo->DeviceState & DSTATE_NORECOVERY) &&
                (!(deviceInfo->DeviceState & DSTATE_COMPLETE))) {
                
                //
                // The miniport has completed its cleanup.  Release the claim.
                // 
                DebugPrint((1,
                            "I2OCompleteRequest: DSTATE_NORECOVERY IO Complete.\n"));
               
                I2OClaimRelease(DeviceExtension,deviceInfo->Lct.LocalTID);

                deviceInfo->DeviceState = DSTATE_NORECOVERY | DSTATE_COMPLETE;
                
            } else if ((deviceInfo->DeviceState & DSTATE_FAULTED) &&
                       (!(deviceInfo->DeviceState & DSTATE_COMPLETE))) {
                //
                // This device has a serious problem and needs to be taken
                // off-line.  Clear the DFLAGS_OS_CAN_CLAIM flag 
                // and notify StorPort of the change. 
                //
                deviceInfo->DeviceState = DSTATE_FAULTED | DSTATE_COMPLETE;
            }
        }

        //
        // Indicate the  request is complete.
        // 
        StorPortNotification(RequestComplete,
                             DeviceExtension,
                             Srb);
    }

    return;
}    


ULONG
MapError(
    IN PI2O_SINGLE_REPLY_MESSAGE_FRAME  Msg,
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )

/*++

Routine Description:

    This routine maps I2O errors to SRB statuses and sense data.

Arguments:

    Msg - The error message frame
    DeviceExtension - This miniport's HwDeviceExtension
    Srb - The failed Srb

Return Value:

    SRB status

--*/
{
    PI2O_FAILURE_REPLY_MESSAGE_FRAME failMsg = (PI2O_FAILURE_REPLY_MESSAGE_FRAME)Msg;
    PSENSE_DATA  senseBuffer = (PSENSE_DATA)Srb->SenseInfoBuffer;
    UCHAR srbStatus;
    UCHAR scsiStatus;

    scsiStatus = 0;
    srbStatus = SRB_STATUS_ERROR;

    if (Msg->StdMessageFrame.MsgFlags & I2O_MESSAGE_FLAGS_FAIL) {

        DebugPrint((1,
                   "I2OMapError: I2O_MESSAGE_FLAGS_FAIL FailureCode = 0x%x\n",
                   failMsg->FailureCode));

        //
        // These don't seem to fall into any good sense key bucket.
        // Just set it as an error and return.
        //
        srbStatus = SRB_STATUS_ERROR;

    } else {

        DebugPrint((1,
                   "I2OMapError: ReqStatus = 0x%x\n",
                   Msg->ReqStatus));

        //
        // Check for a transaction error, or whether the Function was not a BSA
        // function.
        // 
        if ((Msg->ReqStatus == I2O_REPLY_STATUS_TRANSACTION_ERROR) ||
            (Msg->StdMessageFrame.Function < I2O_BSA_DEVICE_RESET) || 
            (Msg->StdMessageFrame.Function > I2O_BSA_BLOCK_REASSIGN)) { 

            //
            // For reply messages that specify I2O_REPLY_STATUS_TRANSACTION_ERROR or
            // where the Function does not specify a block storage class function,
            // the DetailedStatusCode is a generic (not BSA specific) status code.
            //
            DebugPrint((1,
                       "I2OMapError: DetailedStatusCode %x\n",
                       Msg->DetailedStatusCode));

            switch (Msg->DetailedStatusCode) {

            case I2O_DETAIL_STATUS_NO_SUCH_PAGE:
            case I2O_DETAIL_STATUS_TCL_ERROR:

                //
                // Set this to success, so that the application that wanted
                // the HTML, will instead get the page that describes the error.
                // 
                srbStatus = SRB_STATUS_SUCCESS;  
                break;

            case I2O_DETAIL_STATUS_UNSUPPORTED_FUNCTION:
                //
                // Requested function is not supported.
                //
                srbStatus = SRB_STATUS_INVALID_REQUEST;
                break;
            case I2O_DETAIL_STATUS_DEVICE_LOCKED:
                //
                // Resource locked.  Resource exclusively reserved by another party.
                //
                srbStatus = SRB_STATUS_ERROR;
                scsiStatus = SCSISTAT_RESERVATION_CONFLICT;
                break;
            case I2O_DETAIL_STATUS_TIMEOUT:
                //
                // Service or device did not respond within the allocated time.
                //
                srbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                break;

            case I2O_DETAIL_STATUS_BAD_KEY:
            case I2O_DETAIL_STATUS_REPLY_BUFFER_FULL:
            case I2O_DETAIL_STATUS_INSUFFICIENT_RESOURCE_SOFT:
            case I2O_DETAIL_STATUS_INSUFFICIENT_RESOURCE_HARD:
            case I2O_DETAIL_STATUS_CHAIN_BUFFER_TOO_LARGE:
            case I2O_DETAIL_STATUS_DEVICE_RESET:
            case I2O_DETAIL_STATUS_INAPPROPRIATE_FUNCTION:
            case I2O_DETAIL_STATUS_INVALID_INITIATOR_ADDRESS:
            case I2O_DETAIL_STATUS_INVALID_MESSAGE_FLAGS:
            case I2O_DETAIL_STATUS_INVALID_OFFSET:
            case I2O_DETAIL_STATUS_INVALID_PARAMETER:
            case I2O_DETAIL_STATUS_INVALID_REQUEST:
            case I2O_DETAIL_STATUS_INVALID_TARGET_ADDRESS:
            case I2O_DETAIL_STATUS_MESSAGE_TOO_LARGE:
            case I2O_DETAIL_STATUS_MESSAGE_TOO_SMALL:
            case I2O_DETAIL_STATUS_MISSING_PARAMETER:
            case I2O_DETAIL_STATUS_UNKNOWN_ERROR:
            case I2O_DETAIL_STATUS_UNKNOWN_FUNCTION:
            case I2O_DETAIL_STATUS_UNSUPPORTED_VERSION:
            case I2O_DETAIL_STATUS_DEVICE_BUSY:
            case I2O_DETAIL_STATUS_DEVICE_NOT_AVAILABLE:
            default:

                srbStatus = SRB_STATUS_ERROR;
                DebugPrint((1,
                            "I2OMapError: DetailedStatusCode %x\n",
                            Msg->DetailedStatusCode));

            }
        } else {

            //
            // For all other reply messages, DetailedStatusCode is a BSA specific status code.
            //
            DebugPrint((1,
                       "I2OMapError: BSA DetailedStatusCode %x\n",
                       Msg->DetailedStatusCode));

            switch (Msg->DetailedStatusCode) {

            case I2O_BSA_DSC_MEDIA_ERROR:

                //
                // An unfortunately named status. Really means that the device was
                // forced to retry. 
                //
                srbStatus = SRB_STATUS_SUCCESS;

                break;

            case I2O_BSA_DSC_ACCESS_ERROR:
                //
                // Only a warning - prototcol retry.  Retry count supplied in message.
                // A bus protocol error was successfully retried.
                //
                srbStatus = SRB_STATUS_SUCCESS;
                break;

            case I2O_BSA_DSC_DEVICE_FAILURE:
                //
                // Device does not respond or responds with fault.
                //
                srbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                break;

            case I2O_BSA_DSC_DEVICE_NOT_READY:
                //
                // Device is not ready for access.
                //
                scsiStatus = SCSISTAT_CHECK_CONDITION;
                srbStatus = SRB_STATUS_ERROR;

                if (Srb->SenseInfoBuffer) {
                    senseBuffer->ErrorCode = 0x70;
                    senseBuffer->Valid     = 1;
                    senseBuffer->AdditionalSenseLength = 0xb;
                    senseBuffer->SenseKey =  SCSI_SENSE_NOT_READY;
                    senseBuffer->AdditionalSenseCode = 0;
                    senseBuffer->AdditionalSenseCodeQualifier = 0;

                    srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                }
                break;

            case I2O_BSA_DSC_MEDIA_NOT_PRESENT:
                //
                // Removable media not loaded.
                //
                scsiStatus = SCSISTAT_CHECK_CONDITION;
                srbStatus = SRB_STATUS_ERROR;

                if (Srb->SenseInfoBuffer) {
                    senseBuffer->ErrorCode = 0x70;
                    senseBuffer->Valid     = 1;
                    senseBuffer->AdditionalSenseLength = 0xb;
                    senseBuffer->SenseKey =  SCSI_SENSE_UNIT_ATTENTION;
                    senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_NO_MEDIA_IN_DEVICE;
                    senseBuffer->AdditionalSenseCodeQualifier = 0;

                    srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                }
                break;

            case I2O_BSA_DSC_MEDIA_LOCKED:
                //
                // Media locked by another party.
                //
                srbStatus = SRB_STATUS_ERROR;
                scsiStatus = SCSISTAT_RESERVATION_CONFLICT;
                break;

            case I2O_BSA_DSC_MEDIA_FAILURE:
                //
                // Operation failed due to an error on the media.
                //
                scsiStatus = SCSISTAT_CHECK_CONDITION;
                srbStatus = SRB_STATUS_ERROR;

                if (Srb->SenseInfoBuffer) {
                    senseBuffer->ErrorCode = 0x70;
                    senseBuffer->Valid     = 1;
                    senseBuffer->AdditionalSenseLength = 0xb;
                    senseBuffer->SenseKey =  SCSI_SENSE_MEDIUM_ERROR;
                    senseBuffer->AdditionalSenseCode = 0;
                    senseBuffer->AdditionalSenseCodeQualifier = 0;

                    srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                }
                break;

            case I2O_BSA_DSC_PROTOCOL_FAILURE:
                //
                // Operation failed due to a communication problem with the device.
                // RDR  Symbios has this as the data underrun case.
                //
                srbStatus = SRB_STATUS_PHASE_SEQUENCE_FAILURE;
                break;

            case I2O_BSA_DSC_BUS_FAILURE:
                //
                // Operation failed due to a problem with the operation of the bus.
                //
                srbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                break;

            case I2O_BSA_DSC_ACCESS_VIOLATION:
                //
                // Device is locked for exclusive access by another party.
                //
                srbStatus = SRB_STATUS_ERROR;
                scsiStatus = SCSISTAT_RESERVATION_CONFLICT;
                break;

            case I2O_BSA_DSC_WRITE_PROTECTED:
                //
                // Media is write protected or read only.
                //
                scsiStatus = SCSISTAT_CHECK_CONDITION;
                srbStatus = SRB_STATUS_ERROR;

                if (Srb->SenseInfoBuffer) {
                    senseBuffer->ErrorCode = 0x70;
                    senseBuffer->Valid     = 1;
                    senseBuffer->AdditionalSenseLength = 0xb;
                    senseBuffer->SenseKey =  SCSI_SENSE_UNIT_ATTENTION;
                    senseBuffer->AdditionalSenseCode = SCSI_ADWRITE_PROTECT;
                    senseBuffer->AdditionalSenseCodeQualifier = 0;

                    srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                }
                break;

            case I2O_BSA_DSC_DEVICE_RESET:
                //
                // Device has been reset.  After a reset, all requests aside from utility
                // messages are returned with this status until the reset is acknowledged.
                //
                scsiStatus = SCSISTAT_CHECK_CONDITION;
                srbStatus = SRB_STATUS_ERROR;

                if (Srb->SenseInfoBuffer) {
                    senseBuffer->ErrorCode = 0x70;
                    senseBuffer->Valid     = 1;
                    senseBuffer->AdditionalSenseLength = 0xb;
                    senseBuffer->SenseKey =  SCSI_SENSE_UNIT_ATTENTION;
                    senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_BUS_RESET;
                    senseBuffer->AdditionalSenseCodeQualifier = 0;

                    srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                }

                break;

            case I2O_BSA_DSC_VOLUME_CHANGED:
                //
                // Volume has changed.  After a volume change, all requests aside from utility
                // messages are returned with this status until the event is acknowledged.
                //
                scsiStatus = SCSISTAT_CHECK_CONDITION;
                srbStatus = SRB_STATUS_ERROR;

                if (Srb->SenseInfoBuffer) {
                    senseBuffer->ErrorCode = 0x70;
                    senseBuffer->Valid     = 1;
                    senseBuffer->AdditionalSenseLength = 0xb;
                    senseBuffer->SenseKey =  SCSI_SENSE_UNIT_ATTENTION;
                    senseBuffer->AdditionalSenseCode = SCSI_ADSENSE_MEDIUM_CHANGED;
                    senseBuffer->AdditionalSenseCodeQualifier = 0;

                    srbStatus |= SRB_STATUS_AUTOSENSE_VALID;
                }
                break;

            case I2O_BSA_DSC_TIMEOUT:
                //
                // Operation failed because the timeout value specified for the request
                // has been exceeded.
                //
                srbStatus = SRB_STATUS_SELECTION_TIMEOUT;
                break;

            default:
                DebugPrint((1,
                            "I2OMapError: Unknown Detailed status %x\n",
                            Msg->DetailedStatusCode));

            }
        }
    }

    DebugPrint((1,
                "I2OMapError: ScsiStatus (%x) SrbStatus (%x)\n",
                scsiStatus,
                srbStatus));

    Srb->ScsiStatus = scsiStatus;
    Srb->SrbStatus = srbStatus;

    return srbStatus;
}


ULONG
I2OHandleShutdown(
    IN PDEVICE_EXTENSION DeviceExtension
    )
/*++

Routine Description:

    This routine is used during system shutdown to put the IOP into a ready-to-shutdown
    mode (all writes go to media directly and the volumes are marked clean) and to flush 
    the adapter's cache.

Arguments:

    DeviceExtension - The miniport's HwDeviceExtension

Return Value:
    TRUE - Everything is OK
    FALSE - Error condition

--*/
{
    PI2O_DEVICE_INFO lctEntry;
    ULONG i;
    ULONG j;
    I2O_BSA_CACHE_FLUSH_MESSAGE flush;
    ULONG shutdownStatus    = SRB_STATUS_ERROR;
    ULONG status            = SRB_STATUS_SUCCESS;
    POINTER64_TO_U32 pointer64ToU32;
    PIOP_CALLBACK_OBJECT callBackObject;
    ULONG   maxLoopCnt = 10000;

    callBackObject = &DeviceExtension->InitCallback;
    callBackObject->Context = &DeviceExtension->CallBackComplete;

    // 
    // First send a special setParams that, followed by BSA cache
    // flushes to all controlled devices, puts the ISM into a special
    // mode where it knows a shutdown or restart could occur following
    // any IO after this sequence of events, we need to wait on the
    // cache flushes to complete. If there are multiple vols defined
    // we'll get called for each one, we only need to do this for the first one
    // so if we've done it once simply return success for subsequent devices
    //
    shutdownStatus = I2OSetSpecialParams(DeviceExtension,
                                         I2O_RAID_ISM_CONFIG_GROUP_NO,
                                         I2O_READY_FOR_SHUTDOWN_FIELD_IDX,
                                         1);

    DebugPrint((1,
               "I2OHandleShutdown: I2OSetSpecialParams returned status %x\n",
                shutdownStatus));

    if (shutdownStatus == SRB_STATUS_SUCCESS) {

        //
        // ISM supports the special mode, flag it
        //
        DeviceExtension->SpecialShutdown = TRUE;
    }

    
    //
    // Run through all the devices that we currently control and
    // send BSA cache flush, do it even if the above call fails
    //
    for (j = 0; j < IOP_MAXIMUM_TARGETS; j++) {
        
        lctEntry = &DeviceExtension->DeviceInfo[j];
        
        if (lctEntry->DeviceFlags & DFLAGS_CLAIMED) {
            memset(&flush, 0 , sizeof(I2O_BSA_CACHE_FLUSH_MESSAGE));
        
            //
            // Build the flush message.
            // 
            flush.StdMessageFrame.VersionOffset= I2O_VERSION_11;
            flush.StdMessageFrame.MsgFlags = 0;
            flush.StdMessageFrame.MessageSize = sizeof(I2O_BSA_CACHE_FLUSH_MESSAGE)/4;
            flush.StdMessageFrame.TargetAddress = lctEntry->Lct.LocalTID;
            flush.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
            flush.StdMessageFrame.Function = I2O_BSA_CACHE_FLUSH;
            flush.StdMessageFrame.InitiatorContext = 0;

            pointer64ToU32.Pointer64 = 0;
            pointer64ToU32.Pointer   = callBackObject;

            flush.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
            flush.TransactionContext = pointer64ToU32.u.HighPart;

            DeviceExtension->CallBackComplete = 0;

            DebugPrint((1,
                       "I2OHandleShutdown: Send Synch Flush to TID 0x%x\n",
                        lctEntry->Lct.LocalTID));
                
            status = SRB_STATUS_ERROR;
            if (I2OSendMessage(DeviceExtension,(PI2O_MESSAGE_FRAME)&flush)) {
                
                //
                // Check for completion.
                //
                if (I2OGetCallBackCompletion(DeviceExtension,
                                             maxLoopCnt)) {
                    status = SRB_STATUS_SUCCESS;
                }
            }    
        }
    }

    if (shutdownStatus == SRB_STATUS_ERROR) {

        //
        // If the ISM does not support the special interface, try to mark 
        // all volumes clean with a last resort method
        //
        shutdownStatus = I2OSetSpecialParams(DeviceExtension,
                                            I2O_RAID_COMMON_CONFIG_GROUP_NO,
                                            I2O_GUI_MODE_FIELD_IDX,
                                            2);
        DebugPrint((1,
                    "I2OHandleShutdown: Send SetParams to go clean, status= (%x)\n",
                    shutdownStatus));

    }

    return status;
}


ULONG
I2OSetSpecialParams(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN USHORT GroupNumber,
    IN USHORT FieldIdx,
    IN USHORT Value
    )
/*++

Routine Description:

    This routine is used to send param set requests to internal IOP
    parameter groups.  It is a synchonous routine that will use the
    standard InitCallback object and the CallbackComplete flag
    found in the device extension. 

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    GroupNumber - The parameter group to set
    FieldIdx - The offset to set
    Value - The value to set

Return Value:

    TRUE - Everything is OK
    FALSE - Error condition

--*/

{
    union {
        I2O_UTIL_PARAMS_SET_MESSAGE m;
        U32 blk[128/ sizeof(U32)];
    } msg;
    ULONG msgsize;
    ULONG status = SRB_STATUS_ERROR;
    PI2O_PARAM_SCALAR_OPERATION scalar;
    ULONG i;
    POINTER64_TO_U32 pointer64ToU32;
    PIOP_CALLBACK_OBJECT callBackObject;

    msgsize = (sizeof(I2O_UTIL_PARAMS_SET_MESSAGE) -
                                       sizeof(I2O_SG_ELEMENT) +
                                       sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT) +
                                       sizeof(I2O_PARAM_SCALAR_OPERATION)) +
                                       sizeof(ULONG);
    memset(&msg,0, sizeof(msg));

    //
    // Setup the message header.
    //
    msg.m.StdMessageFrame.VersionOffset    = I2O_VERSION_11 | I2O_PARAMS_SGL_VER_OFFSET;
    msg.m.StdMessageFrame.MsgFlags         = 0;
    msg.m.StdMessageFrame.TargetAddress    = DeviceExtension->IsmTID;
    msg.m.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    msg.m.StdMessageFrame.Function         = I2O_UTIL_PARAMS_SET;
    msg.m.StdMessageFrame.InitiatorContext = 0;
    msg.m.StdMessageFrame.MessageSize = (USHORT)(msgsize >> 2);

    msg.m.SGL.u.ImmediateData.FlagsCount.Count = sizeof(I2O_PARAM_SCALAR_OPERATION) + sizeof(ULONG);
    msg.m.SGL.u.ImmediateData.FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                               I2O_SGL_FLAGS_END_OF_BUFFER |
                                               I2O_SGL_FLAGS_IMMEDIATE_DATA_ELEMENT;

    //
    // Build the special callback object.
    //
    callBackObject = &DeviceExtension->InitCallback;
    callBackObject->Context = &DeviceExtension->CallBackComplete;
    DeviceExtension->CallBackComplete = 0;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callBackObject;

    msg.m.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg.m.TransactionContext = pointer64ToU32.u.HighPart;
    
    
    scalar = (PI2O_PARAM_SCALAR_OPERATION)((PUCHAR)&msg.m.SGL +
                                           sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT));
    scalar->OpList.OperationCount = 1;
    scalar->OpBlock.Operation     = I2O_PARAMS_OPERATION_FIELD_SET;
    scalar->OpBlock.GroupNumber   = GroupNumber;
    scalar->OpBlock.FieldCount    = (USHORT) 1;
    scalar->OpBlock.FieldIdx[0]   = FieldIdx;
    
    // 
    // Set the value to be set
    // 
    (ULONG) *((ULONG *)((UCHAR *)scalar + sizeof(I2O_PARAM_SCALAR_OPERATION))) = Value;

    //
    // Send the message.
    //
    DebugPrint((1,
                "I2OSetSpecialParams: Sending SetParams Group 0x%x FieldIdx 0x%x Value 0x%x\n",
                GroupNumber,
                FieldIdx,
                Value));

    if (I2OSendMessage(DeviceExtension,(PI2O_MESSAGE_FRAME)&msg)) {
        
        //
        // Check for completion.
        //
        if (I2OGetCallBackCompletion(DeviceExtension,
                                     10000)) {

            //
            // Indicate that this was successful.
            //
            status = SRB_STATUS_SUCCESS;
        }    
    }

    return status;
}


ULONG
I2OBSAPowerMgt(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PSCSI_REQUEST_BLOCK Srb
    )
/*++

Routine Description:

    This routine will send a power management request to the given device.

Arguments:

    DeviceExtension - This miniport's HwDeviceExt.
    Srb - the power management request

Return Value:

    PENDING or ERROR, if SendMsg failed.

--*/
{
    PIOP_CALLBACK_OBJECT callBackObject = &((PSRB_EXTENSION)Srb->SrbExtension)->CallBack;
    PI2O_DEVICE_INFO     deviceInfo = &DeviceExtension->DeviceInfo[Srb->TargetId]; 
    POINTER64_TO_U32     pointer64ToU32;
    I2O_BSA_POWER_MANAGEMENT_MESSAGE    msg;
    ULONG status = SRB_STATUS_PENDING;

    //
    // Build the block storage power mgt request message.
    //
    msg.StdMessageFrame.VersionOffset    = I2O_VERSION_11;
    msg.StdMessageFrame.MsgFlags         = 0;
    msg.StdMessageFrame.MessageSize      = (sizeof(I2O_BSA_POWER_MANAGEMENT_MESSAGE));
    msg.StdMessageFrame.TargetAddress    = deviceInfo->Lct.LocalTID;
    msg.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    msg.StdMessageFrame.Function         = I2O_BSA_POWER_MANAGEMENT;
    msg.StdMessageFrame.InitiatorContext = 0;

    //
    // Build the call-back object.
    //
    callBackObject->Callback.Signature       = CALLBACK_OBJECT_SIG;
    callBackObject->Callback.CallbackAddress = I2OBSAPowerMgtCompletion;
    callBackObject->Callback.CallbackContext = DeviceExtension;
    callBackObject->Context = Srb;

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer   = callBackObject;

    msg.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg.TransactionContext = pointer64ToU32.u.HighPart;
                                            
    msg.ControlFlags = I2O_BSA_FLAG_PROGRESS_REPORT;
    msg.TimeMultiplier = (UCHAR)Srb->TimeOutValue;
   
    //
    // Check the 'Start' bit to determine whether to power up or down.
    // 
    if ((UCHAR)Srb->Cdb[4] & 0x1){

        msg.Operation = I2O_BSA_POWER_MGT_POWER_UP;
    }
    else {
        msg.Operation = I2O_BSA_POWER_MGT_POWER_DOWN_RETAIN;
    }
    
    DebugPrint((1,
               "I2OBSAPowerMgt: Operation is 0x%x for TID:0x%x\n", 
                    msg.Operation,
                    deviceInfo->Lct.LocalTID));

    if (!I2OSendMessage(DeviceExtension,
                       (PI2O_MESSAGE_FRAME)&msg)) {
        DebugPrint((1,
                   "I2OBSAPowerMgt: Send Message failed.\n"));

        status = SRB_STATUS_ERROR;
    } 

    return status;    

}


VOID
I2OBSAPowerMgtCompletion(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    Process a BSAPowerMgt completion from a block storage device.  This routine
    handles all the aspects of completing the request.

Arguments:

    Msg             - message describing the completed request
    DeviceContext   - device extension for the device this request was issued against

Return Value:

    NOTHING

--*/
{
    PDEVICE_EXTENSION   deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT  callbackObject;
    ULONG               flags;
    ULONG               status;
    PSCSI_REQUEST_BLOCK srb;
    POINTER64_TO_U32    pointer64ToU32;
    
    //
    // Check for any error status.
    //
    flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags;

    //
    // Check for any errors, and extract the Srb.
    // 
    if (flags & I2O_MESSAGE_FLAGS_FAIL) {

        srb = I2OHandleError(Msg,
                             deviceExtension,
                             FIELD_OFFSET(I2O_SINGLE_REPLY_MESSAGE_FRAME,
                             TransactionContext));
        DebugPrint((1,
               "I2OBSAPowerMgtCompletion: !!!!!! Message FAILURE ****\n"));

    } else if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {

        srb = I2OHandleError(Msg, deviceExtension, 0);
        DebugPrint((1,
               "I2OBSAPowerMgtCompletion: !!!!!! Message ERROR ****\n"));
    } else {

        if (Msg->BsaReplyFrame.ReqStatus == I2O_REPLY_STATUS_PROGRESS_REPORT) {
            
            DebugPrint((1,
                        "I2OBSAPowerMgtCompletion: BSAPowerMgt Percent Complete (%lu)\n",
                        ((PI2O_BSA_PROGRESS_REPLY_MESSAGE_FRAME)Msg)->PercentComplete));

            //
            // Another callback will occur.
            //
            return;
                          
        }
        pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
        pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

        callbackObject = pointer64ToU32.Pointer;
        srb  = callbackObject->Context;

        srb->SrbStatus = SRB_STATUS_SUCCESS;
    }

    I2OCompleteRequest(deviceExtension,
                       srb);
    return;
}


BOOLEAN
I2OGetSetTCLParam(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN BOOLEAN GetParams
    )
/*++

Routine Description:

    In order for the RAID card to come back to the same state that it was before a 
    power management event, we must save one word in the ISM. This word keeps track of 
    the current booted device on the RAID card.  

    This routine sends UTIL_PARAM_GET and UTIL_PARAM_SET requests to the IOP and saves 
    private parameter group number 0x8080 field 21 in the device extension.

Arguments:

    DeviceExtension - This miniport's HwDeviceExtension
    GetParams - Indicates whether to get or set the 'word'.

Return Value:

    TRUE - Everything is OK
    FALSE - Error condition

--*/

{
    union {
        I2O_UTIL_PARAMS_SET_MESSAGE m;
        U32 blk[128/ sizeof(U32)];
    } msg; 
    
    ULONG msgsize;
    ULONG i;
    PI2O_PARAM_SCALAR_OPERATION scalar;
    POINTER64_TO_U32 pointer64ToU32;
    IOP_CALLBACK_OBJECT callBackObject;
    BOOLEAN status = FALSE;


    //
    // Setup context and callback info.
    //
    callBackObject.Callback.Signature       = CALLBACK_OBJECT_SIG;
    callBackObject.Callback.CallbackContext = DeviceExtension;
    callBackObject.Context = &DeviceExtension->CallBackComplete;
    DeviceExtension->CallBackComplete = 0;

    msgsize = (sizeof(I2O_UTIL_PARAMS_SET_MESSAGE) -
                                       sizeof(I2O_SG_ELEMENT) +
                                       sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT) +
                                       sizeof(I2O_PARAM_SCALAR_OPERATION)) +
                                       sizeof(ULONG);

    memset(&msg,0, sizeof(msg));

    //
    // Setup the message header.
    //
    msg.m.StdMessageFrame.VersionOffset    = I2O_VERSION_11 | I2O_PARAMS_SGL_VER_OFFSET;
    msg.m.StdMessageFrame.MsgFlags         = 0;
    msg.m.StdMessageFrame.TargetAddress    = DeviceExtension->IsmTID; // RAID ISM TID     
    msg.m.StdMessageFrame.InitiatorAddress = I2O_HOST_TID;
    
    if (GetParams) {
        msg.m.StdMessageFrame.Function = I2O_UTIL_PARAMS_GET;
    } else {
        msg.m.StdMessageFrame.Function = I2O_UTIL_PARAMS_SET;
    }    
    
    msg.m.StdMessageFrame.InitiatorContext = 0;

    msg.m.StdMessageFrame.MessageSize = (USHORT)(msgsize >> 2);

    msg.m.SGL.u.ImmediateData.FlagsCount.Count = sizeof(I2O_PARAM_SCALAR_OPERATION) + sizeof(ULONG);
    msg.m.SGL.u.ImmediateData.FlagsCount.Flags = I2O_SGL_FLAGS_LAST_ELEMENT  |
                                               I2O_SGL_FLAGS_END_OF_BUFFER |
                                               I2O_SGL_FLAGS_IMMEDIATE_DATA_ELEMENT;

    scalar = (PI2O_PARAM_SCALAR_OPERATION)((PUCHAR)&msg.m.SGL +
                                           sizeof(I2O_SGE_IMMEDIATE_DATA_ELEMENT));
    scalar->OpList.OperationCount = 1;
    scalar->OpBlock.GroupNumber   = I2O_RAID_COMMON_CONFIG_GROUP_NO;
    scalar->OpBlock.FieldCount    = (USHORT) 1;
    scalar->OpBlock.FieldIdx[0]   = I2O_TCL_FIELD_IDX;

    if(GetParams){
        scalar->OpBlock.Operation = I2O_PARAMS_OPERATION_FIELD_GET;
        
        //
        // Setup context and callback info.
        //
        callBackObject.Callback.CallbackAddress = I2OGetTCLParamCallback;

    } else {
    
        scalar->OpBlock.Operation = I2O_PARAMS_OPERATION_FIELD_SET;

        (ULONG) *( (ULONG *) ((UCHAR *)scalar + sizeof(I2O_PARAM_SCALAR_OPERATION))) = 
            DeviceExtension->TCLParam;    // value written at Group#0x8080 Field#1
    
        //
        // Setup context and callback info.
        //
        callBackObject.Callback.CallbackAddress = I2OSetTCLParamCallback;
    }    

    pointer64ToU32.Pointer64 = 0;
    pointer64ToU32.Pointer = &callBackObject;

    msg.m.StdMessageFrame.InitiatorContext = pointer64ToU32.u.LowPart;
    msg.m.TransactionContext = pointer64ToU32.u.HighPart;
    DeviceExtension->CallBackComplete = 0;
    
    //
    // Send the message.
    //
    if (I2OSendMessage(DeviceExtension,(PI2O_MESSAGE_FRAME)&msg)) {

        //
        // Check for completion.
        //
        if(I2OGetCallBackCompletion(DeviceExtension,
                                    10000)) {
            return TRUE;
        }
    }
    return FALSE;
}


VOID
I2OGetTCLParamCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    This is the call back function for I2OGetSetTCLParam when requesting
    a get param.  It reads the value from the reply message buffer, 
    updates the TCLParam in the DeviceExtension, and updates the completion
    indicator in the callback object.

Arguments:

    Msg - The Reply Message Frame
    DeviceContext - The HwDeviceExtension
    
Return Value:

    NONE

--*/
{
    PDEVICE_EXTENSION  deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT callbackObject;
    POINTER64_TO_U32     pointer64ToU32;
    ULONG                flags;
    PULONG               msgBuffer;
    PBOOLEAN callBackNotify;

    if ((flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((1,
                   "I2OGetTCLParamCallBack: Failed MsgFlags\n"));
    }

    if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
        DebugPrint((1,
                   "I2OGetTCLParamCallBack: Failed ReqStatus\n"));
    } 

    //
    // The InitiatorContext field contains the low 32 bits of the callback pointer.
    //
    pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
    pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

    callbackObject = pointer64ToU32.Pointer;

    //
    // Get a pointer to the data.  Save off the TCL boot device parameter.
    //
    msgBuffer = (PULONG)&Msg->TransferCount;
    msgBuffer++;
    msgBuffer++;
    deviceExtension->TCLParam = *msgBuffer;

    //
    // No data transfer, just set the completion flag.
    // 
    callBackNotify = callbackObject->Context;
    *callBackNotify = ((Msg->BsaReplyFrame.ReqStatus << 8) | 1);

    return;
}


VOID
I2OSetTCLParamCallback(
    IN PI2O_BSA_SUCCESS_REPLY_MESSAGE_FRAME Msg,
    IN PVOID DeviceContext
    )
/*++

Routine Description:

    This is the call back function for I2OGetSetTCLParam when requesting
    a set param.  It checks for errors and updates the completion
    indicator in the callback object.

Arguments:

    Msg - The Reply Message Frame
    DeviceContext - The HwDeviceExtension
    
Return Value:

    NONE

--*/
{
    PDEVICE_EXTENSION   deviceExtension = DeviceContext;
    PIOP_CALLBACK_OBJECT  callbackObject;
    ULONG               flags;
    PBOOLEAN callBackNotify;
    POINTER64_TO_U32     pointer64ToU32;
    
    //
    // Check for any error status.
    //
    if ((flags = Msg->BsaReplyFrame.StdMessageFrame.MsgFlags) & I2O_MESSAGE_FLAGS_FAIL) {
        DebugPrint((1,
                   "I2OSetTCLParamCallBack: Failed MsgFlags\n"));
    }

    if (Msg->BsaReplyFrame.ReqStatus != I2O_REPLY_STATUS_SUCCESS) {
        DebugPrint((1,
                   "I2OSetTCLParamCallBack: Failed ReqStatus\n"));
    } 

    pointer64ToU32.u.LowPart  = Msg->BsaReplyFrame.StdMessageFrame.InitiatorContext;
    pointer64ToU32.u.HighPart = Msg->BsaReplyFrame.TransactionContext;

    callbackObject = pointer64ToU32.Pointer;

    //
    // No data transfer, just set the completion flag.
    // 
    callBackNotify = callbackObject->Context;
    *callBackNotify = ((Msg->BsaReplyFrame.ReqStatus << 8) | 1);

    return;
}

