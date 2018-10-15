/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

   NDISWDM.C

Abstract:

    This sample provides an example of minimal driver intended for education
    purposes. Neither the driver or its sample test programs are intended
    for use in a production environment. 

Author:  Eliyas Yakub (Jan 11, 2003)

Revision History:

Notes:

--*/
#include "ndiswdm.h"

//
// Functions that are not tagged PAGEABLE by the pragma as shown below are
// by default resident (NONPAGEABLE) in memory. Make sure you don't acquire 
// spinlock or raise the IRQL in a pageable function. It's okay to call
// another nonpageable function that runs at DISPATCH_LEVEL from a
// pageable function.
//
#pragma NDIS_INIT_FUNCTION(DriverEntry)
#pragma NDIS_PAGEABLE_FUNCTION(MPInitialize)
#pragma NDIS_PAGEABLE_FUNCTION(MPHalt)
#pragma NDIS_PAGEABLE_FUNCTION(MPUnload)

#ifdef NDIS51_MINIPORT   
#pragma NDIS_PAGEABLE_FUNCTION(MPPnPEventNotify)
#endif    

MP_GLOBAL_DATA  GlobalData;
INT             MPDebugLevel = MP_INFO;
NDIS_HANDLE     NdisWrapperHandle;

NDIS_STATUS 
DriverEntry(
    PVOID DriverObject,
    PVOID RegistryPath)
/*++
Routine Description:

    In the context of its DriverEntry function, a miniport driver associates
    itself with NDIS, specifies the NDIS version that it is using, and 
    registers its entry points. 


Arguments:
    PVOID DriverObject - pointer to the driver object. 
    PVOID RegistryPath - pointer to the driver registry path.

    Return Value:
    
    NDIS_STATUS_xxx code

--*/
{

    NDIS_STATUS                   Status;
    NDIS_MINIPORT_CHARACTERISTICS MPChar;

    DEBUGP(MP_INFO, ("---> DriverEntry built on "__DATE__" at "__TIME__ "\n"));

    //
    // Associate the miniport driver with NDIS by calling the 
    // NdisMInitializeWrapper. This function allocates a structure
    // to represent this association, stores the miniport driver-
    // specific information that the NDIS Library needs in this 
    // structure, and returns NdisWrapperHandle. The driver must retain and 
    // pass this handle to NdisMRegisterMiniport when it registers its entry 
    // points. NDIS will use NdisWrapperHandle to identify the miniport driver. 
    // The miniport driver must retain this handle but it should never attempt
    // to access or interpret this handle.
    //
    NdisMInitializeWrapper(
            &NdisWrapperHandle,
            DriverObject,
            RegistryPath,
            NULL
            );
    if(!NdisWrapperHandle){
        DEBUGP(MP_ERROR, ("NdisMInitializeWrapper failed\n"));
        return NDIS_STATUS_FAILURE;
    }
    //
    // Fill in the Miniport characteristics structure with the version numbers 
    // and the entry points for driver-supplied MiniportXxx 
    //
    NdisZeroMemory(&MPChar, sizeof(MPChar));
    
    //
    // The NDIS version number, in addition to being included in 
    // NDIS_MINIPORT_CHARACTERISTICS, must also be specified when the 
    // miniport driver source code is compiled.
    //
    MPChar.MajorNdisVersion          = MP_NDIS_MAJOR_VERSION;
    MPChar.MinorNdisVersion          = MP_NDIS_MINOR_VERSION;
    
    MPChar.InitializeHandler         = MPInitialize;
    MPChar.HaltHandler               = MPHalt;
    
    MPChar.SetInformationHandler     = MPSetInformation;
    MPChar.QueryInformationHandler   = MPQueryInformation;
    
    MPChar.SendPacketsHandler        = MPSendPackets;
    MPChar.ReturnPacketHandler       = MPReturnPacket;

    MPChar.ResetHandler              = MPReset;
    MPChar.CheckForHangHandler       = MPCheckForHang; //optional

#ifdef NDIS51_MINIPORT
    MPChar.CancelSendPacketsHandler = MPCancelSendPackets;
    MPChar.PnPEventNotifyHandler    = MPPnPEventNotify;
    MPChar.AdapterShutdownHandler   = MPShutdown;
#endif


    DEBUGP(MP_LOUD, ("Calling NdisMRegisterMiniport...\n"));

    //
    // Registers miniport's entry points with the NDIS library as the first
    // step in NIC driver initialization. The NDIS will call the 
    // MiniportInitialize when the device is actually started by the PNP
    // manager.
    //
    Status = NdisMRegisterMiniport(
                    NdisWrapperHandle,
                    &MPChar,
                    sizeof(NDIS_MINIPORT_CHARACTERISTICS));
    if (Status != NDIS_STATUS_SUCCESS) {
        
        DEBUGP(MP_ERROR, ("Status = 0x%08x\n", Status));
        NdisTerminateWrapper(NdisWrapperHandle, NULL);
        
    } else {
        //
        // Initialize the global variables. The ApaterList in the
        // GloablData structure is used to track the multiple instances
        // of the same adapter. Make sure you do that before registering
        // the unload handler.
        //
        NdisAllocateSpinLock(&GlobalData.Lock);
        NdisInitializeListHead(&GlobalData.AdapterList);        
        //
        // Register an Unload handler for global data cleanup. The unload handler
        // has a more global scope, whereas the scope of the MiniportHalt function
        // is restricted to a particular miniport instance.
        //
        NdisMRegisterUnloadHandler(NdisWrapperHandle, MPUnload);
    }

    
    DEBUGP(MP_TRACE, ("<--- DriverEntry\n"));
    return Status;
    
}

NDIS_STATUS 
MPInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext)
/*++
Routine Description:

    The MiniportInitialize function is a required function that sets up a 
    NIC (or virtual NIC) for network I/O operations, claims all hardware 
    resources necessary to the NIC in the registry, and allocates resources
    the driver needs to carry out network I/O operations.

    MiniportInitialize runs at IRQL = PASSIVE_LEVEL.
    
Arguments:

    Return Value:
    
    NDIS_STATUS_xxx code

--*/
{
    NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER         Adapter=NULL;
    NDIS_HANDLE         ConfigurationHandle;
    UINT                index;
    NTSTATUS            ntStatus;
    ULONG               nameLength;
    
    DEBUGP(MP_TRACE, ("---> MPInitialize\n"));

    PAGED_CODE();

    do {
        //
        // Check to see if our media type exists in an array of supported 
        // media types provided by NDIS.
        //
        for(index = 0; index < MediumArraySize; ++index)
        {
            if(MediumArray[index] == NIC_MEDIA_TYPE) {
                break;
            }
        }

        if(index == MediumArraySize)
        {
            DEBUGP(MP_ERROR, ("Expected media is not in MediumArray.\n"));
            Status = NDIS_STATUS_UNSUPPORTED_MEDIA;
            break;
        }

        //
        // Set the index value as the selected medium for our device.
        //
        *SelectedMediumIndex = index;

        //
        // Allocate adapter context structure and initialize all the 
        // memory resources for sending and receiving packets.
        //
        Status = NICAllocAdapter(&Adapter);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        MP_INC_REF(Adapter);  

        //
        // NdisMGetDeviceProperty function enables us to get the:
        // PDO - created by the bus driver to represent our device.
        // FDO - created by NDIS to represent our miniport as a function 
        //              driver.
        // NextDeviceObject - deviceobject of another driver (filter)
        //                      attached to us at the bottom.
        // Since our driver is talking to NDISPROT, the NextDeviceObject
        // is not useful. But if we were to talk to a driver that we
        // are attached to as part of the devicestack then NextDeviceObject
        // would be our target DeviceObject for sending read/write IRPs.
        //

        NdisMGetDeviceProperty(MiniportAdapterHandle,
                           &Adapter->Pdo,
                           &Adapter->Fdo,
                           &Adapter->NextDeviceObject,
                           NULL,
                           NULL);

        ntStatus = IoGetDeviceProperty (Adapter->Pdo,
                                      DevicePropertyDeviceDescription,
                                      NIC_ADAPTER_NAME_SIZE,
                                      Adapter->AdapterDesc,
                                      &nameLength);

        if (!NT_SUCCESS (ntStatus))
        {
            DEBUGP (MP_ERROR, ("IoGetDeviceProperty failed (0x%x)\n", 
                                            ntStatus));
            Status = NDIS_STATUS_FAILURE;
            break;
        }

        Adapter->AdapterHandle = MiniportAdapterHandle;

        MP_SET_FLAG(Adapter, fMP_INIT_IN_PROGRESS);
        
        //
        // Read Advanced configuration information from the registry
        //
        
        Status = NICReadRegParameters(Adapter, WrapperConfigurationContext);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            break;
        }

        //
        // Inform NDIS about significant features of the NIC. A 
        // MiniportInitialize function must call NdisMSetAttributesEx 
        // (or NdisMSetAttributes) before calling any other NdisMRegisterXxx 
        // or NdisXxx function that claims hardware resources. If your 
        // hardware supports busmaster DMA, you must specify 
        // NDIS_ATTRIBUTE_BUS_MASTER.
        // If this is NDIS51 miniport, it should use safe APIs. But if this 
        // is NDIS 5.0, NDIS might assume that we are not using safe APIs 
        // and try to map every send buffer MDLs. As a result, let us tell 
        // NDIS, even though we are NDIS 5.0, we use safe buffer APIs by 
        // setting the flag NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS
        //
        NdisMSetAttributesEx(
            MiniportAdapterHandle,
            (NDIS_HANDLE) Adapter,
            0,
#ifdef NDIS50_MINIPORT            
            NDIS_ATTRIBUTE_DESERIALIZE | 
            NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS,
#else
            NDIS_ATTRIBUTE_DESERIALIZE, 
#endif               
            NIC_INTERFACE_TYPE);

        // 
        // Get the Adapter Resources & Initialize the hardware.
        //
        
        Status = NICInitializeAdapter(Adapter, WrapperConfigurationContext);
        if(Status != NDIS_STATUS_SUCCESS) {
            Status = NDIS_STATUS_FAILURE;
            break;
        }

      
        
        
    } while(FALSE);

    if(Status == NDIS_STATUS_SUCCESS) {
        //
        // Attach this Adapter to the global list of adapters managed by
        // this driver.
        //
        NICAttachAdapter(Adapter);
    }
    else {
        if(Adapter){
            MPHalt(Adapter);
        }
    }

    DEBUGP(MP_TRACE, ("<--- MPInitialize Status = 0x%08x%\n", Status));

    return Status;

}



VOID 
MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine Description:

    Halt handler is called when NDIS receives IRP_MN_STOP_DEVICE,
    IRP_MN_SUPRISE_REMOVE or IRP_MN_REMOVE_DEVICE requests from the 
    PNP manager. Here, the driver should free all the resources acquired
    in MiniportInitialize and stop access to the hardware. NDIS will
    not submit any further request once this handler is invoked.

    1) Free and unmap all I/O resources.
    2) Disable interrupt and deregister interrupt handler.
    3) Deregister shutdown handler regsitered by 
        NdisMRegisterAdapterShutdownHandler.
    4) Cancel all queued up timer callbacks.
    5) Finally wait indefinitely for all the outstanding receive 
        packets indicated to the protocol to return. 

    MiniportHalt runs at IRQL = PASSIVE_LEVEL. 


Arguments:

    MiniportAdapterContext	Pointer to the Adapter

Return Value:

    None.

--*/
{
    PMP_ADAPTER       Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    BOOLEAN           bDone=TRUE;
    BOOLEAN           bCancelled;
    LONG              nHaltCount = 0;
                 
    DEBUGP(MP_WARNING, ("---> MPHalt\n"));

    PAGED_CODE();

    MP_SET_FLAG(Adapter, fMP_HALT_IN_PROGRESS);
    
    MP_CLEAR_FLAG(Adapter, fMP_POST_WRITES);
    MP_CLEAR_FLAG(Adapter, fMP_POST_READS);

    NICUnregisterExCallback(Adapter);
    
    //
    // Call Shutdown handler to disable interrupt and turn the hardware off 
    // by issuing a full reset
    //
#if defined(NDIS50_MINIPORT)

    MPShutdown(MiniportAdapterContext);

#elif defined(NDIS51_MINIPORT)

    //
    // On XP and later, NDIS notifies our PNP event handler the 
    // reason for calling Halt. So before accessing the device, check to see
    // if the device is surprise removed, if so don't bother calling
    // the shutdown handler to stop the hardware because it doesn't exist.
    //
    if(!MP_TEST_FLAG(Adapter, fMP_SURPRISE_REMOVED)) {       
        MPShutdown(MiniportAdapterContext);
    } 
    
#endif

    //
    // Free the packets on SendWaitList 
    //
    NICFreeQueuedSendPackets(Adapter);

    NICFreeBusyRecvPackets(Adapter);

    NICFreeBusySendPackets(Adapter);

    NICCancelStatusIndicationIrp(Adapter);
    
#ifdef INTERFACE_WITH_NDISPROT

    if(Adapter->FileHandle) {
        ZwClose(Adapter->FileHandle);
        Adapter->FileHandle = NULL;
    }        

    if (Adapter->FileObject) {
        ObDereferenceObject(Adapter->FileObject);
        Adapter->FileObject = NULL;
    }
#endif

    //
    // Decrement the ref count which was incremented in MPInitialize
    //

    MP_DEC_REF(Adapter);

    //
    // Possible non-zero ref counts mean one or more of the following conditions: 
    // 1) Reset DPC is not finished.
    // 2) Receive Indication DPC is not finished.
    //
    
    DEBUGP(MP_INFO, ("RefCount=%d --- waiting!\n", MP_GET_REF(Adapter)));

    NdisWaitEvent(&Adapter->HaltEvent, 0);
        

    while(TRUE)
    {
        bDone = TRUE;
       
        
        //
        // Are all the packets indicated up returned?
        //
        if(Adapter->nBusyRecv)
        {
            DEBUGP(MP_INFO, ("nBusyRecv = %d\n", Adapter->nBusyRecv));
            bDone = FALSE;
        }
        
        //
        // Are there any outstanding send packets?                                                         
        //
        if(Adapter->nBusySend)
        {
            DEBUGP(MP_INFO, ("nBusySend = %d\n", Adapter->nBusySend));
            bDone = FALSE;
        }

        if(bDone)
        {
            break;   
        }

        if((++nHaltCount % 100) == 0)
        {
            DEBUGP(MP_ERROR, ("Halt timed out!!!\n"));
            ASSERT(FALSE);       
        }
        
        DEBUGP(MP_INFO, ("MPHalt - waiting ...\n"));
        NdisMSleep(1000);        
    }

    ASSERT(bDone);
    
#ifdef NDIS50_MINIPORT
    //
    // Deregister shutdown handler as it's being halted
    //
    NdisMDeregisterAdapterShutdownHandler(Adapter->AdapterHandle);
#endif  

    NICDetachAdapter(Adapter);
    NICFreeRecvResources(Adapter);
    NICFreeSendResources(Adapter);
    NICFreeAdapter(Adapter);
    
    DEBUGP(MP_WARNING, ("<--- MPHalt\n"));
}

NDIS_STATUS 
MPReset(
    OUT PBOOLEAN AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine Description:

    MiniportReset is a required to issue a hardware reset to the NIC 
    and/or to reset the driver's software state. 

    1) The miniport driver can optionally complete any pending
        OID requests. NDIS will submit no further OID requests 
        to the miniport driver for the NIC being reset until 
        the reset operation has finished. If you return Status_success
        and still have any pending OIDs, NDIS will complete the OID
        on your behalf. So make sure to complete the pending OIDs
        before returning SUCCESS.
        
    2) A deserialized miniport driver must complete any pending send 
        operations. NDIS will not requeue pending send packets for 
        a deserialized driver since NDIS does not maintain the send 
        queue for such a driver. 
        
    3) If MiniportReset returns NDIS_STATUS_PENDING, the driver must 
        complete the original request subsequently with a call to 
        NdisMResetComplete.

    MiniportReset runs at IRQL = DISPATCH_LEVEL.

Arguments:

AddressingReset - If multicast or functional addressing information 
                  or the lookahead size, is changed by a reset, 
                  MiniportReset must set the variable at AddressingReset 
                  to TRUE before it returns control. This causes NDIS to
                  call the MiniportSetInformation function to restore 
                  the information. 

MiniportAdapterContext - Pointer to our adapter

Return Value:

    NDIS_STATUS

--*/
{
    PMP_ADAPTER       Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
    
    DEBUGP(MP_TRACE, ("---> MPReset\n"));

    *AddressingReset = FALSE;

    ASSERT(Adapter->ResetPending == FALSE);
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    
    NdisDprAcquireSpinLock(&Adapter->Lock);

    //
    // If there is a pending request, we must wait for it to complete
    // before completing the reset. Since the reset handler is called
    // at DISPATCH_LEVEL, we can't really wait here. So we will return
    // STATUS_PENDING, and in the completion handler for forwarded
    // OID requests, we will call NdisMResetComplete().
    //
    if(Adapter->RequestPending){
        Adapter->ResetPending = TRUE;
        Status = NDIS_STATUS_PENDING;
    }

    NdisDprReleaseSpinLock(&Adapter->Lock);

    DEBUGP(MP_TRACE, ("<--- MPReset\n"));
    
    return Status;
}

VOID 
MPUnload(
    IN  PDRIVER_OBJECT  DriverObject
    )
/*++

Routine Description:
    
    The unload handler is called during driver unload to free up resources
    acquired in DriverEntry. This handler is registered through 
    NdisMRegisterUnloadHandler. Note that an unload handler differs from 
    a MiniportHalt function in that the unload handler has a more global
    scope, whereas the scope of the MiniportHalt function is restricted 
    to a particular miniport driver instance.

    Runs at IRQL = PASSIVE_LEVEL. 
    
Arguments:

    DriverObject        Not used

Return Value:

    None
    
--*/
{
    DEBUGP(MP_WARNING, ("--> MPUnload\n"));

    PAGED_CODE();

    ASSERT(IsListEmpty(&GlobalData.AdapterList));
    NdisFreeSpinLock(&GlobalData.Lock);
    
    DEBUGP(MP_WARNING, ("<--- MPUnload\n"));   
}

VOID 
MPShutdown(
    IN NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine Description:
    
    The MiniportShutdown handler restores a NIC to its initial state when
    the system is shut down, whether by the user or because an unrecoverable
    system error occurred. This is to ensure that the NIC is in a known 
    state and ready to be reinitialized when the machine is rebooted after
    a system shutdown occurs for any reason, including a crash dump.

    Here just disable the interrupt and stop the DMA engine.
    Do not free memory resources or wait for any packet transfers
    to complete.


    Runs at an arbitrary IRQL <= DIRQL. So do not call any passive-level
    function.
    
Arguments:

    MiniportAdapterContext  Pointer to our adapter

Return Value:

    None
    
--*/
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER) MiniportAdapterContext;
    
    DEBUGP(MP_TRACE, ("---> MPShutdown\n"));
    
    DEBUGP(MP_TRACE, ("<--- MPShutdown\n"));

}

BOOLEAN 
MPCheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine Description:
    
    The MiniportCheckForHang handler is called to report the state of the
    NIC, or to monitor the responsiveness of an underlying device driver. 
    This is an optional function. If this handler is not specified, NDIS
    judges the driver unresponsive when the driver holds 
    MiniportQueryInformation or MiniportSetInformation requests for a 
    time-out interval (deafult 4 sec), and then calls the driver's 
    MiniportReset function. A NIC driver's MiniportInitialize function can
    extend NDIS's time-out interval by calling NdisMSetAttributesEx to 
    avoid unnecessary resets. 

    Always runs at IRQL = DISPATCH_LEVEL.
    
Arguments:

    MiniportAdapterContext  Pointer to our adapter

Return Value:

    TRUE    NDIS calls the driver's MiniportReset function.
    FALSE   Everything is fine

Note: 
    CheckForHang handler is called in the context of a timer DPC. 
    take advantage of this fact when acquiring/releasing spinlocks

--*/
{
    //DEBUGP(MP_LOUD, ("---> MPCheckForHang\n"));
    //DEBUGP(MP_LOUD, ("<--- MPCheckForHang\n"));
    return(FALSE);
}


#ifdef NDIS51_MINIPORT
VOID 
MPCancelSendPackets(
    IN  NDIS_HANDLE     MiniportAdapterContext,
    IN  PVOID           CancelId
    )
/*++

Routine Description:
    
    MiniportCancelSendPackets cancels the transmission of all packets that
    are marked with a specified cancellation identifier. Miniport drivers
    that queue send packets for more than one second should export this
    handler. When a protocol driver or intermediate driver calls the
    NdisCancelSendPackets function, NDIS calls the MiniportCancelSendPackets 
    function of the appropriate lower-level driver (miniport driver or 
    intermediate driver) on the binding.

    Runs at IRQL <= DISPATCH_LEVEL.
  
    Available - NDIS5.1 (WinXP) and later.
    
Arguments:

    MiniportAdapterContext      Pointer to our adapter
    CancelId                    All the packets with this Id should be cancelled

Return Value:

    None
    
--*/
{
    PNDIS_PACKET    Packet;
    PVOID           PacketId;
    PLIST_ENTRY     thisEntry, nextEntry, listHead;
    SINGLE_LIST_ENTRY SendCancelList;
    PSINGLE_LIST_ENTRY entry;
    
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;

#define MP_GET_PACKET_MR(_p)    (PSINGLE_LIST_ENTRY)(&(_p)->MiniportReserved[0]) 

    DEBUGP(MP_TRACE, ("---> MPCancelSendPackets\n"));

    SendCancelList.Next = NULL;
    
    NdisAcquireSpinLock(&Adapter->SendLock);

    //
    // Walk through the send wait queue and complete the sends with matching Id
    //
    listHead = &Adapter->SendWaitList;
    
    for(thisEntry = listHead->Flink,nextEntry = thisEntry->Flink;
       thisEntry != listHead;
       thisEntry = nextEntry,nextEntry = thisEntry->Flink) {
        Packet = CONTAINING_RECORD(thisEntry, NDIS_PACKET, MiniportReserved);

        PacketId = NdisGetPacketCancelId(Packet);
        if (PacketId == CancelId)
        {       
            //
            // This packet has the right CancelId
            //
            RemoveEntryList(thisEntry);
            //
            // Put this packet on SendCancelList
            //
            PushEntryList(&SendCancelList, MP_GET_PACKET_MR(Packet));
        }
    }
       
    NdisReleaseSpinLock(&Adapter->SendLock);

    //
    // Get the packets from SendCancelList and complete them if any
    //

    entry = PopEntryList(&SendCancelList);
    
    while (entry)
    {
        Packet = CONTAINING_RECORD(entry, NDIS_PACKET, MiniportReserved);

        NdisMSendComplete(
            Adapter->AdapterHandle,
            Packet,
            NDIS_STATUS_REQUEST_ABORTED);
        
        entry = PopEntryList(&SendCancelList);        
    }

    DEBUGP(MP_TRACE, ("<--- MPCancelSendPackets\n"));

}

VOID MPPnPEventNotify(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                   InformationBuffer,
    IN  ULONG                   InformationBufferLength
    )
/*++

Routine Description:
    
    MPPnPEventNotify is to handle PnP notification messages.
    An NDIS miniport driver with a WDM lower edge should export a
    MiniportPnPEventNotify function so that it can be notified
    when its NIC is removed without prior notification through
    the user interface. When a miniport driver receives 
    notification of a surprise removal, it should note internally
    that the device has been removed and cancel any pending IRPs
    that it sent down to the underlying bus driver. After calling
    the MiniportPnPEventNotify function to indicate the surprise
    removal, NDIS calls the miniport’s MiniportHalt function. 
    If the miniport driver receives any requests to send packets
    or query or set OIDs before its MiniportHalt function is
    called, it should immediately complete such requests with a
    status value of NDIS_STATUS_NOT_ACCEPTED.

    All NDIS 5.1 miniport drivers must export a MPPnPEventNotify
    function. 
    
    Runs at IRQL = PASSIVE_LEVEL in the context of system thread.

    Available - NDIS5.1 (WinXP) and later.
    
Arguments:

    MiniportAdapterContext      Pointer to our adapter
    PnPEvent                    Self-explanatory 
    InformationBuffer           Self-explanatory 
    InformationBufferLength     Self-explanatory 

Return Value:

    None
    
--*/
{
    PMP_ADAPTER     Adapter = (PMP_ADAPTER)MiniportAdapterContext;
    PNDIS_POWER_PROFILE NdisPowerProfile;
    
    DEBUGP(MP_TRACE, ("---> MPPnPEventNotify\n"));

    //
    // NDIS currently sends only SurpriseRemoved and 
    // PowerProfileChange Notification events.
    //
    switch (PnPEvent)
    {
        case NdisDevicePnPEventQueryRemoved:
            //
            // Called when NDIS receives IRP_MN_QUERY_REMOVE_DEVICE.
            //
            DEBUGP(MP_LOUD, ("MPPnPEventNotify: NdisDevicePnPEventQueryRemoved\n"));
            break;

        case NdisDevicePnPEventRemoved:
            //
            // Called when NDIS receives IRP_MN_REMOVE_DEVICE.
            // NDIS calls MiniportHalt function after this call returns.
            //
            DEBUGP(MP_LOUD, ("MPPnPEventNotify: NdisDevicePnPEventRemoved\n"));
            break;       

        case NdisDevicePnPEventSurpriseRemoved:
            //
            // Called when NDIS receives IRP_MN_SUPRISE_REMOVAL. 
            // NDIS calls MiniportHalt function after this call returns.
            //
            MP_SET_FLAG(Adapter, fMP_SURPRISE_REMOVED);
            DEBUGP(MP_LOUD, ("MPPnPEventNotify: NdisDevicePnPEventSurpriseRemoved\n"));
            break;

        case NdisDevicePnPEventQueryStopped:
            //
            // Called when NDIS receives IRP_MN_QUERY_STOP_DEVICE. ??
            //
            DEBUGP(MP_LOUD, ("MPPnPEventNotify: NdisDevicePnPEventQueryStopped\n"));
            break;

        case NdisDevicePnPEventStopped:
            //
            // Called when NDIS receives IRP_MN_STOP_DEVICE.
            // NDIS calls MiniportHalt function after this call returns.
            // 
            //
            DEBUGP(MP_LOUD, ("MPPnPEventNotify: NdisDevicePnPEventStopped\n"));
            break;      
            
        case NdisDevicePnPEventPowerProfileChanged:
            //
            // After initializing a miniport driver and after miniport driver
            // receives an OID_PNP_SET_POWER notification that specifies 
            // a device power state of NdisDeviceStateD0 (the powered-on state), 
            // NDIS calls the miniport's MiniportPnPEventNotify function with 
            // PnPEvent set to NdisDevicePnPEventPowerProfileChanged. 
            //            
            DEBUGP(MP_LOUD, ("MPPnPEventNotify: NdisDevicePnPEventPowerProfileChanged\n"));
            
            if(InformationBufferLength == sizeof(NDIS_POWER_PROFILE)) {
                NdisPowerProfile = (PNDIS_POWER_PROFILE)InformationBuffer;
                if(*NdisPowerProfile == NdisPowerProfileBattery) {
                    DEBUGP(MP_LOUD, 
                        ("The host system is running on battery power\n"));
                }
                if(*NdisPowerProfile == NdisPowerProfileAcOnLine) {
                    DEBUGP(MP_LOUD, 
                        ("The host system is running on AC power\n"));
               }
            }
            break;      
            
        default:
            DEBUGP(MP_ERROR, ("MPPnPEventNotify: unknown PnP event %x \n", PnPEvent));
            break;         
    }

    DEBUGP(MP_TRACE, ("<--- MPPnPEventNotify\n"));

}

#endif

