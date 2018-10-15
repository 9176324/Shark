/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

   Miniport.C

Abstract:

    The purpose of this sample is to illustrate functionality of a deserialized
    NDIS miniport driver without requiring a physical network adapter. This 
    sample is based on E100BEX sample present in the DDK. It is basically a 
    simplified version of E100bex driver. The driver can be installed either 
    manually using Add Hardware wizard as a root enumerated virtual miniport 
    driver or on a virtual bus (like toaster bus). Since the driver does not 
    interact with any hardware, it makes it very easy to understand the miniport
    interface and the usage of various NDIS functions without the clutter of
    hardware specific code normally found in a fully functional driver. 

    This sample provides an example of minimal driver intended for education
    purposes. Neither the driver or it's sample test programs are intended
    for use in a production environment. 

Author:  Eliyas Yakub (Nov 20th 2002)

Revision History:

Notes:

--*/
#include "miniport.h"

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

    DEBUGP(MP_TRACE, ("---> DriverEntry built on "__DATE__" at "__TIME__ "\n"));

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

    MPChar.AllocateCompleteHandler   = MPAllocateComplete;//optional

    MPChar.DisableInterruptHandler   = MPDisableInterrupt; //optional
    MPChar.EnableInterruptHandler    = MPEnableInterrupt; //optional
    MPChar.HandleInterruptHandler    = MPHandleInterrupt;
    MPChar.ISRHandler                = MPIsr;

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
    NDIS_STATUS          Status = NDIS_STATUS_SUCCESS;
    PMP_ADAPTER          Adapter=NULL;
    NDIS_HANDLE          ConfigurationHandle;
    UINT                 index;
   
    DEBUGP(MP_TRACE, ("---> MPInitialize\n"));

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
        // FDO - created by NDIS to represent our miniport as a function driver.
        // NextDeviceObject - deviceobject of another driver (filter)
        //                      attached to us at the bottom.
        // In a pure NDIS miniport driver, there is no use for this
        // information, but a NDISWDM driver would need to know this so that it 
        // can transfer packets to the lower WDM stack using IRPs.
        //
        NdisMGetDeviceProperty(MiniportAdapterHandle,
                           &Adapter->Pdo,
                           &Adapter->Fdo,
                           &Adapter->NextDeviceObject,
                           NULL,
                           NULL);

        Adapter->AdapterHandle = MiniportAdapterHandle;

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
        // hardware supports busmaster DMA, you must specify NDIS_ATTRIBUTE_BUS_MASTER.
        // If this is NDIS51 miniport, it should use safe APIs. But if this 
        // is NDIS 5.0, the driver claim to use safe APIs by setting 
        // NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS
        //
        NdisMSetAttributesEx(
            MiniportAdapterHandle,
            (NDIS_HANDLE) Adapter,
            0,
#ifdef NDIS50_MINIPORT            
            NDIS_ATTRIBUTE_DESERIALIZE|
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

        //
        // Setup a timer function for Receive Indication
        //
        NdisInitializeTimer(
            &Adapter->RecvTimer, 
            (PNDIS_TIMER_FUNCTION)NICIndicateReceiveTimerDpc, 
            (PVOID)Adapter);
        
        //
        // Setup a timer function for use with our MPReset routine.
        //
        NdisInitializeTimer(
                &Adapter->ResetTimer,
                (PNDIS_TIMER_FUNCTION) NICResetCompleteTimerDpc,
                (PVOID) Adapter);
        
        NdisInitializeEvent(&Adapter->RemoveEvent);
        
        
    } while(FALSE);

    if(Status == NDIS_STATUS_SUCCESS) {
        //
        // Attach this Adapter to the global list of adapters managed by
        // this driver.
        //
        NICAttachAdapter(Adapter);

        //
        // Create an IOCTL interface
        //
        NICRegisterDevice();
    }
    else {
        if(Adapter){
            MP_DEC_REF(Adapter);              
            NICFreeAdapter(Adapter);
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
        NdisMRegisterAdapterShutdownHandler .
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
    LONG              nHaltCount = 0, Count;
                 
    MP_SET_FLAG(Adapter, fMP_ADAPTER_HALT_IN_PROGRESS);

    DEBUGP(MP_TRACE, ("---> MPHalt\n"));

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
    if(!MP_TEST_FLAG(Adapter, fMP_ADAPTER_SURPRISE_REMOVED)) {       
        MPShutdown(MiniportAdapterContext);
    }        
#endif

    //
    // Free the packets on SendWaitList 
    //
    NICFreeQueuedSendPackets(Adapter);

    //
    // Cancel the ResetTimer.
    //    
    NdisCancelTimer(&Adapter->ResetTimer, &bCancelled);

    //
    // Cancel the ReceiveIndication Timer.
    //    
    NdisCancelTimer(&Adapter->RecvTimer, &bCancelled);
    if(bCancelled) {
        //
        // We are able to cancel a queued Timer. So there is a
        // possibility for the packets to be waiting in the 
        // RecvWaitList. So let us free them by calling..
        //
        NICFreeQueuedRecvPackets(Adapter);
    }            
    
    //
    // Decrement the ref count which was incremented in MPInitialize
    //

    MP_DEC_REF(Adapter);
    
    //
    // Possible non-zero ref counts mean one or more of the following conditions: 
    // 1) Reset DPC is still running.
    // 2) Receive Indication DPC is still running.
    //

    DEBUGP(MP_INFO, ("RefCount=%d --- waiting!\n", MP_GET_REF(Adapter)));

    NdisWaitEvent(&Adapter->RemoveEvent, 0);

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

        if(++nHaltCount % 100)
        {
            DEBUGP(MP_ERROR, ("Halt timed out!!!\n"));
            DEBUGP(MP_ERROR, ("RecvWaitList = %p\n", &Adapter->RecvWaitList));
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

    //
    // Unregister the ioctl interface.
    //
    NICDeregisterDevice();

    NICDetachAdapter(Adapter);

    NICFreeAdapter(Adapter);
    
    DEBUGP(MP_TRACE, ("<--- MPHalt\n"));
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
        the reset operation has finished. After the reset, 
        NDIS will resubmit to the miniport driver any OID requests
        that were pending but not completed by the miniport driver
        before the reset.
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
    NDIS_STATUS       Status;
    PMP_ADAPTER       Adapter = (PMP_ADAPTER) MiniportAdapterContext;

    BOOLEAN           bDone = TRUE;

    DEBUGP(MP_TRACE, ("---> MPReset\n"));

    do
    {
        ASSERT(!MP_TEST_FLAG(Adapter, fMP_ADAPTER_HALT_IN_PROGRESS));
        
        if(MP_TEST_FLAG(Adapter, fMP_RESET_IN_PROGRESS))
        {
            Status = NDIS_STATUS_RESET_IN_PROGRESS;
            break;   
        }

        MP_SET_FLAG(Adapter, fMP_RESET_IN_PROGRESS);

        //
        // Complete all the queued up send packets
        //
        NICFreeQueuedSendPackets(Adapter);

        //
        // Check to see if all the packets indicated up are returned.
        //
        if(Adapter->nBusyRecv)
        {
            DEBUGP(MP_INFO, ("nBusyRecv = %d\n", Adapter->nBusyRecv));
            bDone = FALSE;
        }

        //
        // Are there any send packets in the processes of being 
        // transmitted?
        //
        if(Adapter->nBusySend)
        {
            DEBUGP(MP_INFO, ("nBusySend = %d\n", Adapter->nBusySend));
            bDone = FALSE;
        }

        if(!bDone)
        {
            Adapter->nResetTimerCount = 0;
            //
            // We can't complete the reset request now. So let us queue
            // a timer callback for 500ms and check again whether we can 
            // successfully reset the hardware.
            //
            NdisSetTimer(&Adapter->ResetTimer, 500);

            //
            // By returning NDIS_STATUS_PENDING, we are promising NDIS that
            // we will complete the reset request by calling NdisMResetComplete.
            //
            Status = NDIS_STATUS_PENDING;
            break;
        }

        *AddressingReset = FALSE;
        MP_CLEAR_FLAG(Adapter, fMP_RESET_IN_PROGRESS);
        Status = NDIS_STATUS_SUCCESS;

    } while(FALSE);
    
    DEBUGP(MP_TRACE, ("<--- MPReset Status = 0x%08x\n", Status));
    
    return(Status);
}


VOID 
NICResetCompleteTimerDpc(
    IN PVOID             SystemSpecific1,
    IN PVOID             FunctionContext,
    IN PVOID             SystemSpecific2,
    IN PVOID             SystemSpecific3)
/*++

Routine Description:

    Timer callback function for Reset operation. 
        
Arguments:

FunctionContext - Pointer to our adapter

Return Value:

    VOID

--*/
{
    PMP_ADAPTER Adapter = (PMP_ADAPTER)FunctionContext;
    BOOLEAN bDone = TRUE;
    NDIS_STATUS Status = NDIS_STATUS_SUCCESS;

    DEBUGP(MP_TRACE, ("--> NICResetCompleteTimerDpc\n"));

    //
    // Increment the ref count on the adapter to prevent the driver from
    // unloding while the DPC is running. The Halt handler waits for the
    // ref count to drop to zero before returning. 
    //
    MP_INC_REF(Adapter);            

    //
    // Check to see if all the packets indicated up are returned.
    //
    if(Adapter->nBusyRecv)
    {
        DEBUGP(MP_INFO, ("nBusyRecv = %d\n", Adapter->nBusyRecv));
        bDone = FALSE;
    }

    //
    // Are there any send packets in the processes of being 
    // transmitted?
    //
    if(Adapter->nBusySend)
    {
        DEBUGP(MP_INFO, ("nBusySend = %d\n", Adapter->nBusySend));
        bDone = FALSE;
    }

    if(!bDone && ++Adapter->nResetTimerCount <= 20)
    {
        //
        // Let us try one more time.
        //
        NdisSetTimer(&Adapter->ResetTimer, 500);
    }
    else
    {
        if(!bDone)
        {
        
            //
            // We have tried enough. Something is wrong. Let us
            // just complete the reset request with failure.
            //
            DEBUGP(MP_ERROR, ("Reset timed out!!!\n"));
            DEBUGP(MP_ERROR, ("nBusySend = %d\n", Adapter->nBusySend));
            DEBUGP(MP_ERROR, ("RecvWaitList = %p\n", &Adapter->RecvWaitList));
            DEBUGP(MP_ERROR, ("nBusyRecv = %d\n", Adapter->nBusyRecv));
            
            ASSERT(FALSE);       

            Status = NDIS_STATUS_FAILURE;
        }

        DEBUGP(MP_INFO, ("Done - NdisMResetComplete\n"));

        MP_CLEAR_FLAG(Adapter, fMP_RESET_IN_PROGRESS);
        NdisMResetComplete(
            Adapter->AdapterHandle,
            Status,
            FALSE);
        
    }

    MP_DEC_REF(Adapter);     

    DEBUGP(MP_TRACE, ("<-- NICResetCompleteTimerDpc Status = 0x%08x\n", Status));
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
    DEBUGP(MP_TRACE, ("--> MPUnload\n"));

    ASSERT(IsListEmpty(&GlobalData.AdapterList));
    NdisFreeSpinLock(&GlobalData.Lock);
    
    DEBUGP(MP_TRACE, ("<--- MPUnload\n"));   
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
    DEBUGP(MP_LOUD, ("---> MPCheckForHang\n"));
    DEBUGP(MP_LOUD, ("<--- MPCheckForHang\n"));
    return(FALSE);
}


VOID 
MPHandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine Description:
    
    MiniportHandleInterrupt is a DPC function called to do deferred 
    processing of all outstanding interrupt operations. When a NIC 
    generates an interrupt, a miniport's MiniportISR or 
    MiniportDisableInterrupt function dismisses the interrupt on the 
    NIC, saves any necessary state about the operation, and returns 
    control as quickly as possible, thereby deferring most 
    interrupt-driven I/O operations to MiniportHandleInterrupt. This
    handler is called only if the MiniportISR function returned 
    QueueMiniportHandleInterrupt set to TRUE.

    MiniportHandleInterrupt then re-enables interrupts on the NIC, 
    either by letting NDIS call the miniport driver's 
    MiniportEnableInterrupt function after MiniportHandleInterrupt 
    returns control or by enabling the interrupt from within
    MiniportHandleInterrupt, which is faster. 

    Note that more than one instance of this function can execute 
    concurrently in SMP machines.
    
    Runs at IRQL = DISPATCH_LEVEL    
    
Arguments:

    MiniportAdapterContext  Pointer to our adapter

Return Value:

    None
    
--*/
{
    DEBUGP(MP_TRACE, ("---> MPHandleInterrupt\n"));
    DEBUGP(MP_TRACE, ("<--- MPHandleInterrupt\n"));
}

VOID 
MPIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN NDIS_HANDLE MiniportAdapterContext
    )
/*++

Routine Description:
    
    MiniportIsr handler is called to when the device asserts an interrupt.
    MiniportISR dismisses the interrupt on the NIC, saves whatever state
    it must about the interrupt, and defers as much of the I/O processing
    for each interrupt as possible to the MiniportHandleInterrupt function. 
    
    MiniportISR is not re-entrant, although two instantiations of this 
    function can execute concurrently in SMP machines, particularly if 
    the miniport driver supports full-duplex sends and receives. A driver
    writer should not rely on a one-to-one correspondence between the 
    execution of MiniportISR and MiniportHandleInterrupt. 

    If the NIC shares an IRQ with other devices (check NdisMRegisterInterrupt), 
    this function should determine whether the NIC generated the interrupt. 
    If the NIC did not generated the interrupt, MiniportISR should return FALSE 
    immediately so that the driver of the device that generated the interrupt
    is called quickly. 

    Runs at IRQL = DIRQL assigned when the NIC driver's MiniportInitialize
    function called NdisMRegisterInterrupt.   

Arguments:

    InterruptRecognized             TRUE on return if the interrupt comes 
                                    from this NIC    
    QueueMiniportHandleInterrupt    TRUE on return if MiniportHandleInterrupt 
                                    should be called
    MiniportAdapterContext          Pointer to our adapter

Return Value:

    None
    
--*/
{

    DEBUGP(MP_TRACE, ("---> MPIsr\n"));
    DEBUGP(MP_TRACE, ("<--- MPIsr\n"));
}

VOID 
MPDisableInterrupt(
    IN PVOID MiniportAdapterContext
    )
/*++

Routine Description:
    
    MiniportDisableInterrupt typically disables interrupts by writing 
    a mask to the NIC. If a driver does not have this function, typically
    its MiniportISR disables interrupts on the NIC.

    This handler is required by drivers of NICs that support dynamic 
    enabling and disabling of interrupts but do not share an IRQ.

    Runs at IRQL = DIRQL    
    
Arguments:

    MiniportAdapterContext          Pointer to our adapter

Return Value:

    None
    
--*/
{
    DEBUGP(MP_TRACE, ("---> MPDisableInterrupt\n"));
    DEBUGP(MP_TRACE, ("<--- MPDisableInterrupt\n"));

}

VOID 
MPEnableInterrupt(
    IN PVOID MiniportAdapterContext
    )
/*++

Routine Description:
    
    MiniportEnableInterrupt typically enables interrupts by writing a mask
    to the NIC. A NIC driver that exports a MiniportDisableInterrupt function
    need not have a reciprocal MiniportEnableInterrupt function. 
    Such a driver's MiniportHandleInterrupt function is responsible for 
    re-enabling interrupts on the NIC. 

    Runs at IRQL = DIRQL    
    
Arguments:

    MiniportAdapterContext          Pointer to our adapter

Return Value:

    None
    
--*/
{
    DEBUGP(MP_TRACE, ("---> MPEnableInterrupt\n"));
    DEBUGP(MP_TRACE, ("<--- MPEnableInterrupt\n"));

}

VOID 
MPAllocateComplete(
    NDIS_HANDLE MiniportAdapterContext,
    IN PVOID VirtualAddress,
    IN PNDIS_PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length,
    IN PVOID Context
    )
/*++

Routine Description:
    
    This handler is needed if the driver makes calls to 
    NdisMAllocateSharedMemoryAsync. Drivers of bus-master DMA NICs call 
    NdisMAllocateSharedMemoryAsync to dynamically allocate shared memory
    for transfer operations when high network traffic places excessive 
    demands on the shared memory space that the driver allocated during 
    initialization. 

    Runs at IRQL = DISPATCH_LEVEL.
    
Arguments:

    MiniportAdapterContext  Pointer to our adapter
    VirtualAddress          Pointer to the allocated memory block 
    PhysicalAddress         Physical address of the memory block       
    Length                  Length of the memory block                
    Context                 Context in NdisMAllocateSharedMemoryAsync              

Return Value:

    None
    
--*/
{
    DEBUGP(MP_TRACE, ("---> MPAllocateComplete\n"));
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
    
    MiniportPnPEventNotify is to handle PnP notification messages.
    All NDIS 5.1 miniport drivers must export a MiniportPnPEventNotify
    function. Miniport drivers that have a WDM lower edge should export
    a MiniportPnPEventNotify function.

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
    
    //
    // Turn off the warings.
    //
    UNREFERENCED_PARAMETER(Adapter);

    DEBUGP(MP_TRACE, ("---> MPPnPEventNotify\n"));

    switch (PnPEvent)
    {
        case NdisDevicePnPEventQueryRemoved:
            //
            // Called when NDIS receives IRP_MN_QUERY_REMOVE_DEVICE.
            //
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventQueryRemoved\n"));
            break;

        case NdisDevicePnPEventRemoved:
            //
            // Called when NDIS receives IRP_MN_REMOVE_DEVICE.
            // NDIS calls MiniportHalt function after this call returns.
            //
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventRemoved\n"));
            break;       

        case NdisDevicePnPEventSurpriseRemoved:
            //
            // Called when NDIS receives IRP_MN_SUPRISE_REMOVAL. 
            // NDIS calls MiniportHalt function after this call returns.
            //
            MP_SET_FLAG(Adapter, fMP_ADAPTER_SURPRISE_REMOVED);
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventSurpriseRemoved\n"));
            break;

        case NdisDevicePnPEventQueryStopped:
            //
            // Called when NDIS receives IRP_MN_QUERY_STOP_DEVICE. ??
            //
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventQueryStopped\n"));
            break;

        case NdisDevicePnPEventStopped:
            //
            // Called when NDIS receives IRP_MN_STOP_DEVICE.
            // NDIS calls MiniportHalt function after this call returns.
            // 
            //
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventStopped\n"));
            break;      
            
        case NdisDevicePnPEventPowerProfileChanged:
            //
            // After initializing a miniport driver and after miniport driver
            // receives an OID_PNP_SET_POWER notification that specifies 
            // a device power state of NdisDeviceStateD0 (the powered-on state), 
            // NDIS calls the miniport's MiniportPnPEventNotify function with 
            // PnPEvent set to NdisDevicePnPEventPowerProfileChanged. 
            //            
            DEBUGP(MP_INFO, ("MPPnPEventNotify: NdisDevicePnPEventPowerProfileChanged\n"));
            
            if(InformationBufferLength == sizeof(NDIS_POWER_PROFILE)) {
                NdisPowerProfile = (PNDIS_POWER_PROFILE)InformationBuffer;
                if(*NdisPowerProfile == NdisPowerProfileBattery) {
                    DEBUGP(MP_INFO, 
                        ("The host system is running on battery power\n"));
                }
                if(*NdisPowerProfile == NdisPowerProfileAcOnLine) {
                    DEBUGP(MP_INFO, 
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

