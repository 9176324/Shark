/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY      
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    NIC_INIT.c

Abstract:

    Contains rotuines to do resource allocation and hardware
    initialization & shutdown.

Environment:

    Kernel mode


Revision History:

    Eliyas Yakub Feb 13, 2003

--*/

#include "precomp.h"

#if defined(EVENT_TRACING)
#include "nic_init.tmh"
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, NICInitializeDeviceExtension)
#pragma alloc_text (PAGE, NICAllocateDeviceResources)
#pragma alloc_text (PAGE, NICMapHWResources)
#pragma alloc_text (PAGE, NICUnmapHWResources)
#pragma alloc_text (PAGE, NICGetDeviceInformation)
#pragma alloc_text (PAGE, NICReadAdapterInfo)
#pragma alloc_text (PAGE, NICAllocAdapterMemory)
#pragma alloc_text (PAGE, NICFreeAdapterMemory)
#pragma alloc_text (PAGE, NICInitRecv)
#pragma alloc_text (PAGE, NICSelfTest)
#pragma alloc_text (PAGE, NICInitializeAdapter)
#pragma alloc_text (PAGE, HwConfigure)
#pragma alloc_text (PAGE, HwClearAllCounters)
#pragma alloc_text (PAGE, HwSetupIAAddress)
#pragma alloc_text (PAGE, GetPCIBusInterfaceStandard)
#pragma alloc_text (PAGE, NICAllocRfd)
#pragma alloc_text (PAGE, NICFreeRfd)
#pragma alloc_text (PAGE, NICAllocRfdWorkItem)
#pragma alloc_text (PAGE, NICFreeRfdWorkItem)
#endif


NTSTATUS
NICInitializeDeviceExtension(
    IN OUT PFDO_DATA FdoData
    )
/*++
Routine Description:

    
Arguments:

    FdoData     Pointer to our FdoData

Return Value:

     None

--*/
{
    NTSTATUS status;

    //
    // Get the BUS_INTERFACE_STANDARD for our device so that we can
    // read & write to PCI config space at IRQL <= DISPATCH_LEVEL.
    //
    status = GetPCIBusInterfaceStandard(FdoData->Self, 
                                        &FdoData->BusInterface);
    if (!NT_SUCCESS (status)){
        return status;    
    }
    
    NICGetDeviceInfSettings(FdoData);
    
    FdoData->ConservationIdleTime = -SECOND_TO_100NS * PCIDRV_DEF_IDLE_TIME;
    FdoData->PerformanceIdleTime = -SECOND_TO_100NS * PCIDRV_DEF_IDLE_TIME;
    
    //
    // Initialize list heads, spinlocks, timers etc.
    //
    InitializeListHead(&FdoData->SendQueueHead);

    InitializeListHead(&FdoData->RecvList);
    InitializeListHead(&FdoData->RecvQueueHead);
    InitializeListHead(&FdoData->PoMgmt.PatternList);

    KeInitializeSpinLock(&FdoData->Lock);
    KeInitializeSpinLock(&FdoData->SendLock);
    KeInitializeSpinLock(&FdoData->RcvLock);    
    KeInitializeSpinLock(&FdoData->RecvQueueLock); 

    //
    // To minimize init-time, queue a DPC to do link detection.
    // This DPC will also be used to check of hardware hang.
    //
    KeInitializeDpc(&FdoData->WatchDogTimerDpc, // Dpc
                                NICWatchDogTimerDpc, // DeferredRoutine
                                FdoData           // DeferredContext
                                ); 

    KeInitializeTimer(&FdoData->WatchDogTimer );
    
    //
    // Event used to make sure the NICWatchDogTimerDpc has completed execution
    // before freeing the resources and unloading the driver.
    //
    KeInitializeEvent(&FdoData->WatchDogTimerEvent, NotificationEvent, TRUE);

    return status;
    
}


NTSTATUS
NICAllocateDeviceResources(
    IN OUT  PFDO_DATA   FdoData,
    IN      PIRP        Irp
    )
/*++
Routine Description:

    Allocates all the hw and software resources required for
    the device, enables interrupt, and initializes the device.

Arguments:

    FdoData     Pointer to our FdoData
    Irp         Pointer to start-device irp.

Return Value:

     None

--*/
{
    NTSTATUS        status;
    LARGE_INTEGER   DueTime;

    do{

        //
        // First make sure this is our device before doing whole lot
        // of other things.
        //
        status = NICGetDeviceInformation(FdoData);
        if (!NT_SUCCESS (status)){
            return status;
        }
        
        status = NICMapHWResources(FdoData, Irp);
        if (!NT_SUCCESS (status)){
            DebugPrint(ERROR, DBG_INIT,"NICMapHWResources failed: 0x%x\n",
                                    status);
            break;
        }       
        
        //
        // Read additional info from NIC such as MAC address
        //
        status = NICReadAdapterInfo(FdoData);
        if (status != STATUS_SUCCESS)
        {
            break;
        }


        status = NICAllocAdapterMemory(FdoData);
        if (status != STATUS_SUCCESS)
        {
            break;
        }    

        NICInitSend(FdoData);
        
        status = NICInitRecv(FdoData);
        if (status != STATUS_SUCCESS)
        {
            break;
        }    

        //
        // Test our adapter hardware
        //
        status = NICSelfTest(FdoData);
        if (status != STATUS_SUCCESS)
        {
            break;
        }    

        status = NICInitializeAdapter(FdoData);
        if (status != STATUS_SUCCESS)
        {
            break;
        }    

        //
        // Enable the interrupt
        //
        NICEnableInterrupt(FdoData);
                 
        //
        // Set the link detection flag to indicate that NICWatchDogTimerDpc
        // will be first doing link-detection.
        //
        MP_SET_FLAG(FdoData, fMP_ADAPTER_LINK_DETECTION);
        FdoData->CheckForHang = FALSE;
        FdoData->bLinkDetectionWait = FALSE;
        FdoData->bLookForLink = FALSE;   
        
        //
        // This event stays cleared until the NICWatchDogTimerDpc exits. 
        //
        KeClearEvent(&FdoData->WatchDogTimerEvent);

        //
        // Watch dog timer is used to do the initial link detection during
        // start and then used to make sure the device is not hung for 
        // any reason.
        //
        DueTime.QuadPart = NIC_LINK_DETECTION_DELAY;
        KeSetTimer( &FdoData->WatchDogTimer,   // Timer
                            DueTime,         // DueTime
                            &FdoData->WatchDogTimerDpc      // Dpc  
                            );                
    }while(FALSE);        

    return status;
    
}


NTSTATUS
NICFreeDeviceResources(
    IN OUT PFDO_DATA FdoData
    )
/*++
Routine Description:

    Free all the software resources. We shouldn't touch the hardware.

Arguments:

    FdoData     Pointer to our FdoData

Return Value:

     None

--*/
{
    NTSTATUS    status;
    KIRQL       oldIrql;
    
    DebugPrint(INFO, DBG_INIT, "-->NICFreeDeviceResources\n");
            
    if(!KeCancelTimer(&FdoData->WatchDogTimer)){
        //
        // Wait for the timer to complete since it has already begun executing.
        //
        DebugPrint(INFO, DBG_INIT, 
                            "Waiting for the watchdogtimer to exit..\n");
        
        status = KeWaitForSingleObject( &FdoData->WatchDogTimerEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
        ASSERT(NT_SUCCESS(status));
    }
       
    //
    // We need to raise the IRQL before acquiring the lock
    // because the functions called inside the guarded
    // region assume that they are called at Dpc level and release
    // and reacquire the lock using DpcLevel spinlock functions.
    //
    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
    KeAcquireSpinLockAtDpcLevel(&FdoData->SendLock);

    //
    // Free the packets on SendWaitList                                                           
    //
    NICFreeQueuedSendPackets(FdoData);

    //
    // Free the packets being actively sent & stopped
    //
    NICFreeBusySendPackets(FdoData);
    
    KeReleaseSpinLockFromDpcLevel(&FdoData->SendLock);
    KeLowerIrql(oldIrql);

    
    NICFreeAdapterMemory(FdoData);

    //
    // Disconnect from the interrupt and unmap any I/O ports
    //
    NICUnmapHWResources(FdoData);

    DebugPrint(INFO, DBG_INIT, "<--NICFreeDeviceResources\n");
    
    return STATUS_SUCCESS;
    
}

NTSTATUS
NICMapHWResources(
    IN OUT PFDO_DATA FdoData,
    IN PIRP Irp
    )
/*++
Routine Description:

    Gets the HW resources assigned by the bus driver from the start-irp
    and maps it to system address space. Initializes the DMA adapter
    and sets up the ISR.

    Three base address registers are supported by the 8255x:
    1) CSR Memory Mapped Base Address Register (BAR 0 at offset 10)
    2) CSR I/O Mapped Base Address Register (BAR 1 at offset 14)
    3) Flash Memory Mapped Base Address Register (BAR 2 at offset 18)
    
    The 8255x requires one BAR for I/O mapping and one BAR for memory 
    mapping of these registers anywhere within the 32-bit memory address space.
    The driver determines which BAR (I/O or Memory) is used to access the 
    Control/Status Registers. 

    Just for illustration, this driver maps both memory and I/O registers and
    shows how to use READ_PORT_xxx or READ_REGISTER_xxx functions to perform 
    I/O in a platform independent basis. On some platforms, the I/O registers
    can get mapped in to memory space and your driver should be able to handle
    this transparently.
    
    One BAR is also required to map the accesses to an optional Flash memory. 
    The 82557 implements this register regardless of the presence or absence 
    of a Flash chip on the adapter. The 82558 and 82559 only implement this 
    register if a bit is set in the EEPROM. The size of the space requested 
    by this register is 1Mbyte, and it is always mapped anywhere in the 32-bit
    memory address space. 
    Note: Although the 82558 only supports up to 64 Kbytes of Flash memory 
    and the 82559 only supports 128 Kbytes of Flash memory, 1 Mbyte of
    address space is still requested. Software should not access Flash 
    addresses above 64 Kbytes for the 82558 or 128 Kbytes for the 82559 
    because Flash accesses above the limits are aliased to lower addresses.    
    
Arguments:

    FdoData     Pointer to our FdoData
    Irp         Pointer to start-device irp.

Return Value:

     None

--*/
{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceTrans;
    PCM_PARTIAL_RESOURCE_LIST       partialResourceListTranslated;
    PIO_STACK_LOCATION              stack;
    ULONG                           i;
    NTSTATUS                        status = STATUS_SUCCESS;
    DEVICE_DESCRIPTION              deviceDescription;    
    ULONG                           MaximumPhysicalMapping;
    PDMA_ADAPTER                    DmaAdapterObject;
    ULONG                           maxMapRegistersRequired, miniMapRegisters;
    ULONG                           MapRegisters;
    BOOLEAN bResPort = FALSE, bResInterrupt = FALSE, bResMemory = FALSE;
    ULONG                           numberOfBARs = 0;
#if defined(DMA_VER2) // To avoid  unreferenced local variables error
    ULONG                           SGMapRegsisters;
    ULONG                           ScatterGatherListSize;        
#endif

    stack = IoGetCurrentIrpStackLocation (Irp);

    PAGED_CODE();

    if (NULL == stack->Parameters.StartDevice.AllocatedResourcesTranslated) {
        status = STATUS_DEVICE_CONFIGURATION_ERROR;
        goto End;
    }
    
    //
    // Parameters.StartDevice.AllocatedResourcesTranslated points
    // to a CM_RESOURCE_LIST describing the hardware resources that
    // the PnP Manager assigned to the device. This list contains
    // the resources in translated form. Use the translated resources
    // to connect the interrupt vector, map I/O space, and map memory.
    //

    partialResourceListTranslated = &stack->Parameters.StartDevice.\
                      AllocatedResourcesTranslated->List[0].PartialResourceList;

    resourceTrans = &partialResourceListTranslated->PartialDescriptors[0];

    for (i = 0;
         i < partialResourceListTranslated->Count;
         i++, resourceTrans++) {

        switch (resourceTrans->Type) {
            
        case CmResourceTypePort:
            //
            // We will increment the BAR count only for valid resources. We will
            // not count the private device types added by the PCI bus driver.
            //
            numberOfBARs++;
            
            DebugPrint(LOUD, DBG_INIT,
                "I/O mapped CSR: (%x) Length: (%d)\n",
                resourceTrans->u.Port.Start.LowPart,
                resourceTrans->u.Port.Length);
            
            //
            // Since we know the resources are listed in the same order the as
            // BARs in the config space, this should be the second one. 
            //
            if(numberOfBARs != 2) {
                DebugPrint(ERROR, DBG_INIT, "I/O mapped CSR is not in the right order\n");                
                status = STATUS_DEVICE_CONFIGURATION_ERROR;
                goto End;                
            }
            
            //
            // The port is in memory space on this machine.
            // We shuld use READ_PORT_Xxx, and WRITE_PORT_Xxx routines
            // to read or write to the port.
            //

            FdoData->IoBaseAddress = ULongToPtr(resourceTrans->u.Port.Start.LowPart);
            FdoData->IoRange = resourceTrans->u.Port.Length;
            //
            // Since all our accesses are USHORT wide, we will create an accessor
            // table just for these two functions.  
            //
            FdoData->ReadPort = NICReadPortUShort; 
            FdoData->WritePort = NICWritePortUShort;
            
            bResPort = TRUE;
            FdoData->MappedPorts = FALSE;            
            break;

        case CmResourceTypeMemory:

            numberOfBARs++;

            if(numberOfBARs == 1) {
                DebugPrint(LOUD, DBG_INIT, "Memory mapped CSR:(%x:%x) Length:(%d)\n",
                                        resourceTrans->u.Memory.Start.LowPart, 
                                        resourceTrans->u.Memory.Start.HighPart, 
                                        resourceTrans->u.Memory.Length);
                //
                // Our CSR memory space should be 0x1000 in size.            
                //
                ASSERT(resourceTrans->u.Memory.Length == 0x1000);
                FdoData->MemPhysAddress = resourceTrans->u.Memory.Start;
                FdoData->CSRAddress = MmMapIoSpace(
                                                resourceTrans->u.Memory.Start,
                                                NIC_MAP_IOSPACE_LENGTH,
                                                MmNonCached);   
                if(FdoData->CSRAddress == NULL) {
                    DebugPrint(ERROR, DBG_INIT, "MmMapIoSpace failed\n"); 			   
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto End;
                }
                DebugPrint(LOUD, DBG_INIT, "CSRAddress=%p\n", FdoData->CSRAddress);

                bResMemory = TRUE;
                
            } else if(numberOfBARs == 2){
            
                DebugPrint(LOUD, DBG_INIT,
                    "I/O mapped CSR in Memory Space: (%x) Length: (%d)\n",
                    resourceTrans->u.Memory.Start.LowPart,
                    resourceTrans->u.Memory.Length);
                //
                // The port is in memory space on this machine.
                // We should call MmMapIoSpace to map the physical to virtual
                // address, and also use the READ/WRITE_REGISTER_xxx function
                // to read or write to the port.
                //

                FdoData->IoBaseAddress = MmMapIoSpace(
                                                resourceTrans->u.Memory.Start,
                                                resourceTrans->u.Memory.Length,
                                                MmNonCached);
                if(FdoData->IoBaseAddress == NULL) {
                       DebugPrint(ERROR, DBG_INIT, "MmMapIoSpace failed\n");              
                       status = STATUS_INSUFFICIENT_RESOURCES;
                       goto End;
                }
                
                FdoData->ReadPort = NICReadRegisterUShort; 
                FdoData->WritePort = NICWriteRegisterUShort;                
                FdoData->MappedPorts = TRUE;
                bResPort = TRUE;
                
            } else if(numberOfBARs == 3){

                DebugPrint(LOUD, DBG_INIT, "Flash memory:(%x:%x) Length:(%d)\n",
                                        resourceTrans->u.Memory.Start.LowPart, 
                                        resourceTrans->u.Memory.Start.HighPart, 
                                        resourceTrans->u.Memory.Length);
                //
                // Our flash memory should be 1MB in size. Since we don't
                // access the memory, let us not bother mapping it.
                //
            } else {
                DebugPrint(ERROR, DBG_INIT, 
                            "Memory Resources are not in the right order\n");                
                status = STATUS_DEVICE_CONFIGURATION_ERROR;
                goto End;                            
            }
            
            break;

        case CmResourceTypeInterrupt:

            ASSERT(!bResInterrupt);
            
            bResInterrupt = TRUE;
            //
            // Save all the interrupt specific information in the device 
            // extension because we will need it to disconnect and connect the
            // interrupt later on during power suspend and resume.
            //
            FdoData->InterruptLevel = (UCHAR)resourceTrans->u.Interrupt.Level;
            FdoData->InterruptVector = resourceTrans->u.Interrupt.Vector;
            FdoData->InterruptAffinity = resourceTrans->u.Interrupt.Affinity;
            
            if (resourceTrans->Flags & CM_RESOURCE_INTERRUPT_LATCHED) {
            
                FdoData->InterruptMode = Latched;

            } else {
            
                FdoData->InterruptMode = LevelSensitive;
            }

            //
            // Because this is a PCI device, we KNOW it must be
            // a LevelSensitive Interrupt.
            //
            
            ASSERT(FdoData->InterruptMode == LevelSensitive);

            DebugPrint(LOUD, DBG_INIT, 
                "Interrupt level: 0x%0x, Vector: 0x%0x, Affinity: 0x%x\n",
                FdoData->InterruptLevel,
                FdoData->InterruptVector,
                (UINT)FdoData->InterruptAffinity); // casting is done to keep WPP happy          
            break;

        default:
            //
            // This could be device-private type added by the PCI bus driver. We
            // shouldn't filter this or change the information contained in it.
            //
            DebugPrint(LOUD, DBG_INIT, "Unhandled resource type (0x%x)\n", 
                                        resourceTrans->Type);
            break;

        }
    }

    //
    // Make sure we got all the 3 resources to work with.
    //
    if (!(bResPort && bResInterrupt && bResMemory)) {
        status = STATUS_DEVICE_CONFIGURATION_ERROR;
        goto End;
    } 
    
    //
    // Disable interrupts here which is as soon as possible
    //
    NICDisableInterrupt(FdoData);

    IoInitializeDpcRequest(FdoData->Self, (PKDEFERRED_ROUTINE)NICDpcForIsr);

    //
    // Register the interrupt
    //
    status = IoConnectInterrupt(&FdoData->Interrupt,
                              NICInterruptHandler,
                              FdoData,                   // ISR Context
                              NULL,
                              FdoData->InterruptVector,
                              FdoData->InterruptLevel,
                              FdoData->InterruptLevel,
                              FdoData->InterruptMode,
                              TRUE, // shared interrupt
                              FdoData->InterruptAffinity,
                              FALSE);   
    if (status != STATUS_SUCCESS)
    {
        DebugPrint(ERROR, DBG_INIT, "IoConnectInterrupt failed %x\n", status);
        goto End;
    }
    
    MP_SET_FLAG(FdoData, fMP_ADAPTER_INTERRUPT_IN_USE);

    //
    // Zero out the entire structure first.
    //
    RtlZeroMemory(&deviceDescription, sizeof(DEVICE_DESCRIPTION));

    // 
    // DMA_VER2 is defined when the driver is built to work on XP and
    // above. The difference between DMA_VER2 and VER0 is that VER2
    // support BuildScatterGatherList & CalculateScatterGatherList functions.
    // BuildScatterGatherList performs the same operation as 
    // GetScatterGatherList, except that it uses the buffer supplied 
    // in the ScatterGatherBuffer parameter to hold the scatter/gather 
    // list that it creates. In contrast, GetScatterGatherList 
    // dynamically allocates a buffer to hold the scatter/gather list. 
    // If insufficient memory is available to allocate the buffer, 
    // GetScatterGatherList can fail with a STATUS_INSUFFICIENT_RESOURCES
    // error. Drivers that must avoid this scenario can pre-allocate a 
    // buffer to hold the scatter/gather list, and use BuildScatterGatherList
    // instead. A driver can use the CalculateScatterGatherList routine 
    // to determine the size of buffer to allocate to hold the 
    // scatter/gather list.
    //
#if defined(DMA_VER2)
    deviceDescription.Version = DEVICE_DESCRIPTION_VERSION2;
#else
    deviceDescription.Version = DEVICE_DESCRIPTION_VERSION;
#endif

    deviceDescription.Master = TRUE;
    deviceDescription.ScatterGather = TRUE;
    deviceDescription.Dma32BitAddresses = TRUE;
    deviceDescription.Dma64BitAddresses = FALSE;
    deviceDescription.InterfaceType = PCIBus;

    //
    // Bare minimum number of map registers required to do
    // a single NIC_MAX_PACKET_SIZE transfer.
    //
    miniMapRegisters = ((NIC_MAX_PACKET_SIZE * 2 - 2) / PAGE_SIZE) + 2;

    //
    // Maximum map registers required to do simultaneous transfer
    // of all TCBs assuming each packet spanning NIC_MAX_PHYS_BUF_COUNT
    // (Each request has chained MDLs).
    //
    maxMapRegistersRequired = FdoData->NumTcb * NIC_MAX_PHYS_BUF_COUNT;

    //
    // The maximum length of buffer for maxMapRegistersRequired number of
    // map registers would be.
    //
    MaximumPhysicalMapping = (maxMapRegistersRequired-1) << PAGE_SHIFT;
    deviceDescription.MaximumLength = MaximumPhysicalMapping;
                                    
    DmaAdapterObject = IoGetDmaAdapter(FdoData->UnderlyingPDO,
                                        &deviceDescription,
                                        &MapRegisters);        
    if (DmaAdapterObject == NULL)
    {
        DebugPrint(ERROR, DBG_INIT, "IoGetDmaAdapter failed\n");                
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto End;
    }

    if(MapRegisters < miniMapRegisters) {
        DebugPrint(ERROR, DBG_INIT, "Not enough map registers: Allocated %d, Required %d\n", 
                                        MapRegisters, miniMapRegisters);                
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto End;
    }

    FdoData->AllocatedMapRegisters = MapRegisters;
    
    //
    // Adjust our TCB count based on the MapRegisters we got.
    //
    FdoData->NumTcb = MapRegisters/miniMapRegisters;

    //
    // Reset it NIC_MAX_TCBS if it exceeds that.
    //
    FdoData->NumTcb = min(FdoData->NumTcb, NIC_MAX_TCBS);
    
    DebugPrint(TRACE, DBG_INIT, "MapRegisters Allocated %d\n", MapRegisters);
    DebugPrint(TRACE, DBG_INIT, "Adjusted TCB count is %d\n", FdoData->NumTcb);

    FdoData->DmaAdapterObject = DmaAdapterObject;
    MP_SET_FLAG(FdoData, fMP_ADAPTER_SCATTER_GATHER);

#if defined(DMA_VER2)
    
    status = DmaAdapterObject->DmaOperations->CalculateScatterGatherList(
                                            DmaAdapterObject,
                                            NULL,
                                            0,
                                            MapRegisters * PAGE_SIZE,
                                            &ScatterGatherListSize,
                                            &SGMapRegsisters);


    ASSERT(NT_SUCCESS(status));
    ASSERT(SGMapRegsisters == MapRegisters);    
    if (!NT_SUCCESS(status))
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto End;
    }
    
    FdoData->ScatterGatherListSize = ScatterGatherListSize;
    
#endif

    //
    // For convenience, let us save the frequently used DMA operational
    // functions in our device context.
    //
    FdoData->AllocateCommonBuffer = 
                   *DmaAdapterObject->DmaOperations->AllocateCommonBuffer; 
    FdoData->FreeCommonBuffer = 
                   *DmaAdapterObject->DmaOperations->FreeCommonBuffer; 
End:
    //
    // If we have jumped here due to any kind of mapping or resource allocation
    // failure, we should clean up. Since we know that if we fail Start-Device,
    // the system is going to send Remove-Device request, we will defer the
    // job of cleaning to NICUnmapHWResources which is called in the Remove path.
    //
    return status;

}

NTSTATUS
NICUnmapHWResources(
    IN OUT PFDO_DATA FdoData
    )
/*++
Routine Description:

    Disconnect the interrupt and unmap all the memory and I/O resources.

Arguments:

    FdoData     Pointer to our FdoData

Return Value:

     None

--*/
{
    PDMA_ADAPTER    DmaAdapterObject = FdoData->DmaAdapterObject;

    //
    // Free hardware resources
    //    
    if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_INTERRUPT_IN_USE))
    {
        IoDisconnectInterrupt(FdoData->Interrupt);
        FdoData->Interrupt = NULL;
        MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_INTERRUPT_IN_USE);
    }

    if (FdoData->CSRAddress)
    {
        MmUnmapIoSpace(FdoData->CSRAddress, NIC_MAP_IOSPACE_LENGTH);        
        FdoData->CSRAddress = NULL;
    }

    if(FdoData->MappedPorts){
        MmUnmapIoSpace(FdoData->IoBaseAddress, FdoData->IoRange);
        FdoData->IoBaseAddress = NULL;
    }

    if(DmaAdapterObject && MP_TEST_FLAG(FdoData, fMP_ADAPTER_SCATTER_GATHER)){
        DmaAdapterObject->DmaOperations->PutDmaAdapter(DmaAdapterObject);
        FdoData->DmaAdapterObject = NULL;
        FdoData->AllocateCommonBuffer = NULL;
        FdoData->FreeCommonBuffer = NULL;       
        MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_SCATTER_GATHER);      
    }
    
    return STATUS_SUCCESS;
    
}


NTSTATUS
NICGetDeviceInformation(
    IN PFDO_DATA FdoData
)
/*++
Routine Description:

    This function reads the PCI config space and make sure that it's our 
    device and stores the device IDs and power information in the device
    extension. Should be done in the StartDevice.
 
Arguments:

    FdoData     Pointer to our FdoData

Return Value:

     None

--*/
{
    NTSTATUS            status = STATUS_SUCCESS;
    UCHAR               buffer[NIC_PCI_E100_HDR_LENGTH ];
    PPCI_COMMON_CONFIG  pPciConfig = (PPCI_COMMON_CONFIG) buffer;
    USHORT              usPciCommand;       
    ULONG               bytesRead =0;
    
    DebugPrint(TRACE, DBG_INIT, "---> NICGetDeviceInformation\n");

    bytesRead = FdoData->BusInterface.GetBusData(
                        FdoData->BusInterface.Context,
                         PCI_WHICHSPACE_CONFIG, //READ
                         buffer,
                         FIELD_OFFSET(PCI_COMMON_CONFIG, VendorID),
                         NIC_PCI_E100_HDR_LENGTH);
    
    if (bytesRead != NIC_PCI_E100_HDR_LENGTH) {                            
        DebugPrint(ERROR, DBG_INIT, 
                        "GetBusData (NIC_PCI_E100_HDR_LENGTH) failed =%d\n",
                         bytesRead);
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    
    //     
    // Is this our device?
    //
    
    if (pPciConfig->VendorID != NIC_PCI_VENDOR_ID || 
        pPciConfig->DeviceID != NIC_PCI_DEVICE_ID)
    {
        DebugPrint(ERROR, DBG_INIT, 
                        "VendorID/DeviceID don't match - %x/%x\n", 
                        pPciConfig->VendorID, pPciConfig->DeviceID);
        //return STATUS_DEVICE_DOES_NOT_EXIST;

    }

    //
    // save info from config space
    //        
    FdoData->RevsionID = pPciConfig->RevisionID;
    FdoData->SubVendorID = pPciConfig->u.type0.SubVendorID;
    FdoData->SubSystemID = pPciConfig->u.type0.SubSystemID;

    NICExtractPMInfoFromPciSpace (FdoData, (PUCHAR)pPciConfig);
    
    usPciCommand = pPciConfig->Command;
    
    if ((usPciCommand & PCI_ENABLE_WRITE_AND_INVALIDATE) && (FdoData->MWIEnable)){
        FdoData->MWIEnable = TRUE;
    } else {
        FdoData->MWIEnable = FALSE;
    }

    DebugPrint(TRACE, DBG_INIT, "<-- NICGetDeviceInformation\n");
    
    return status;
}

NTSTATUS
NICReadAdapterInfo(
    IN PFDO_DATA FdoData
    )
/*++
Routine Description:

    Read the mac addresss from the adapter

Arguments:

    FdoData     Pointer to our device context

Return Value:

    NTSTATUS code

--*/    
{
    NTSTATUS     status = STATUS_SUCCESS;
    USHORT          usValue; 
    int             i;

    DebugPrint(TRACE, DBG_INIT, "--> NICReadAdapterInfo\n");

    FdoData->EepromAddressSize = GetEEpromAddressSize(
            GetEEpromSize(FdoData, (PUCHAR)FdoData->IoBaseAddress));
    
    DebugPrint(LOUD, DBG_INIT, "EepromAddressSize = %d\n", 
                                FdoData->EepromAddressSize);
        
    
    //
    // Read node address from the EEPROM
    //
    for (i=0; i< ETH_LENGTH_OF_ADDRESS; i += 2)
    {
        usValue = ReadEEprom(FdoData, (PUCHAR)FdoData->IoBaseAddress,
                      (USHORT)(EEPROM_NODE_ADDRESS_BYTE_0 + (i/2)),
                      FdoData->EepromAddressSize);

        *((PUSHORT)(&FdoData->PermanentAddress[i])) = usValue;
    }

    DebugPrint(INFO, DBG_INIT, 
        "Permanent Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
        FdoData->PermanentAddress[0], FdoData->PermanentAddress[1], 
        FdoData->PermanentAddress[2], FdoData->PermanentAddress[3], 
        FdoData->PermanentAddress[4], FdoData->PermanentAddress[5]);

    if (ETH_IS_MULTICAST(FdoData->PermanentAddress) || 
        ETH_IS_BROADCAST(FdoData->PermanentAddress))
    {
        DebugPrint(ERROR, DBG_INIT, "Permanent address is invalid\n"); 

        status = STATUS_INVALID_ADDRESS;         
    }
    else
    {
        if (!FdoData->bOverrideAddress)
        {
            ETH_COPY_NETWORK_ADDRESS(FdoData->CurrentAddress, FdoData->PermanentAddress);
        }

        DebugPrint(INFO, DBG_INIT,
            "Current Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
            FdoData->CurrentAddress[0], FdoData->CurrentAddress[1],
            FdoData->CurrentAddress[2], FdoData->CurrentAddress[3],
            FdoData->CurrentAddress[4], FdoData->CurrentAddress[5]);
    }

    DebugPrint(TRACE, DBG_INIT, "<-- NICReadAdapterInfo, status=%x\n", 
                    status);

    return status;
}



NTSTATUS 
NICAllocAdapterMemory(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    Allocate all the memory blocks for send, receive and others

Arguments:

    FdoData     Pointer to our adapter

Return Value:


--*/    
{
    NTSTATUS        status = STATUS_SUCCESS;
    PUCHAR          pMem;
    ULONG           MemPhys;
    PDMA_ADAPTER    DmaAdapterObject;
    
    DmaAdapterObject = FdoData->DmaAdapterObject;
               
    DebugPrint(TRACE, DBG_INIT, "--> NICAllocAdapterMemory\n");

    DebugPrint(LOUD, DBG_INIT, "NumTcb=%d\n", FdoData->NumTcb);
    
    FdoData->NumTbd = FdoData->NumTcb * NIC_MAX_PHYS_BUF_COUNT;

    do
    {
 
#if defined(DMA_VER2)

        ExInitializeNPagedLookasideList(&FdoData->SGListLookasideList,
                                            NULL,
                                            NULL,
                                            0,
                                            FdoData->ScatterGatherListSize,
                                            PCIDRV_POOL_TAG,
                                            0);
        MP_SET_FLAG(FdoData, fMP_ADAPTER_SEND_SGL_LOOKASIDE);

#endif

        //
        // Send + Misc
        //
        //
        // Allocate MP_TCB's
        // 
        FdoData->MpTcbMemSize = FdoData->NumTcb * sizeof(MP_TCB);

        pMem = ExAllocatePoolWithTag(NonPagedPool, 
                            FdoData->MpTcbMemSize, PCIDRV_POOL_TAG);
        if (NULL == pMem )
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            DebugPrint(ERROR, DBG_INIT, "Failed to allocate MP_TCB's\n");
            break;
        }
        
        RtlZeroMemory(pMem, FdoData->MpTcbMemSize);
        FdoData->MpTcbMem = pMem;

        // HW_START

        //
        // Allocate shared memory for send
        // 
        FdoData->HwSendMemAllocSize = FdoData->NumTcb * (sizeof(TXCB_STRUC) + 
                              NIC_MAX_PHYS_BUF_COUNT * sizeof(TBD_STRUC));

        FdoData->HwSendMemAllocVa = FdoData->AllocateCommonBuffer(
                DmaAdapterObject,
                FdoData->HwSendMemAllocSize,
                &FdoData->HwSendMemAllocPa,
                FALSE                           // NOT CACHED
                );

        if (!FdoData->HwSendMemAllocVa)
        {
            DebugPrint(ERROR, DBG_INIT, 
                                "Failed to allocate send memory\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(FdoData->HwSendMemAllocVa, 
                        FdoData->HwSendMemAllocSize);

        //
        // Allocate shared memory for other uses
        // 
        FdoData->HwMiscMemAllocSize =
            sizeof(SELF_TEST_STRUC) + ALIGN_16 +
            sizeof(DUMP_AREA_STRUC) + ALIGN_16 +
            sizeof(NON_TRANSMIT_CB) + ALIGN_16 +
            sizeof(ERR_COUNT_STRUC) + ALIGN_16;
       
        //
        // Allocate the shared memory for the command block data structures.
        //
        FdoData->HwMiscMemAllocVa = FdoData->AllocateCommonBuffer(
                DmaAdapterObject,
                FdoData->HwMiscMemAllocSize,
                &FdoData->HwMiscMemAllocPa,
                FALSE                           // NOT CACHED
                );
        
         if (!FdoData->HwMiscMemAllocVa)
        {
            DebugPrint(ERROR, DBG_INIT, 
                                "Failed to allocate misc memory\n");
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(FdoData->HwMiscMemAllocVa, 
                        FdoData->HwMiscMemAllocSize);

        pMem = FdoData->HwMiscMemAllocVa; 
        MemPhys = FdoData->HwMiscMemAllocPa.LowPart ;

        FdoData->SelfTest = (PSELF_TEST_STRUC)MP_ALIGNMEM(pMem, ALIGN_16);
        FdoData->SelfTestPhys = MP_ALIGNMEM_PHYS(MemPhys, ALIGN_16);
        pMem = (PUCHAR)FdoData->SelfTest + sizeof(SELF_TEST_STRUC);
        MemPhys = FdoData->SelfTestPhys + sizeof(SELF_TEST_STRUC);

        FdoData->NonTxCmdBlock = (PNON_TRANSMIT_CB)MP_ALIGNMEM(pMem, ALIGN_16);
        FdoData->NonTxCmdBlockPhys = MP_ALIGNMEM_PHYS(MemPhys, ALIGN_16);
        pMem = (PUCHAR)FdoData->NonTxCmdBlock + sizeof(NON_TRANSMIT_CB);
        MemPhys = FdoData->NonTxCmdBlockPhys + sizeof(NON_TRANSMIT_CB);

        FdoData->DumpSpace = (PDUMP_AREA_STRUC)MP_ALIGNMEM(pMem, ALIGN_16);
        FdoData->DumpSpacePhys = MP_ALIGNMEM_PHYS(MemPhys, ALIGN_16);
        pMem = (PUCHAR)FdoData->DumpSpace + sizeof(DUMP_AREA_STRUC);
        MemPhys = FdoData->DumpSpacePhys + sizeof(DUMP_AREA_STRUC);

        FdoData->StatsCounters = (PERR_COUNT_STRUC)MP_ALIGNMEM(pMem, ALIGN_16);
        FdoData->StatsCounterPhys = MP_ALIGNMEM_PHYS(MemPhys, ALIGN_16);

        // HW_END

        //
        // Recv
        //

        ExInitializeNPagedLookasideList(
            &FdoData->RecvLookaside,
            NULL,
            NULL,
            0,
            sizeof(MP_RFD),
            PCIDRV_POOL_TAG,
            0);

        MP_SET_FLAG(FdoData, fMP_ADAPTER_RECV_LOOKASIDE);

        // set the max number of RFDs
        // disable the RFD grow/shrink scheme if user specifies a NumRfd value 
        // larger than NIC_MAX_GROW_RFDS
        FdoData->MaxNumRfd = max(FdoData->NumRfd, NIC_MAX_GROW_RFDS);
        
        DebugPrint(LOUD, DBG_INIT, "NumRfd = %d\n", FdoData->NumRfd);
        DebugPrint(LOUD, DBG_INIT, "MaxNumRfd = %d\n", 
                                        FdoData->MaxNumRfd);

        //
        // The driver should allocate more data than sizeof(RFD_STRUC) to allow the
        // driver to align the data(after ethernet header) at 8 byte boundary
        //
        FdoData->HwRfdSize = sizeof(RFD_STRUC) + MORE_DATA_FOR_ALIGN;      

        status = STATUS_SUCCESS;

    } while (FALSE);

    DebugPrint(TRACE, DBG_INIT, 
                        "<-- NICAllocAdapterMemory, status=%x\n", status);

    return status;

}

VOID 
NICFreeAdapterMemory(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    Free all the resources and MP_ADAPTER data block

Arguments:

    FdoData     Pointer to our adapter

Return Value:

    None                                                    

--*/    
{
    PMP_RFD         pMpRfd;
    
    DebugPrint(TRACE, DBG_INIT, "--> NICFreeAdapterMemory\n");

    // No active and waiting sends
    ASSERT(FdoData->nBusySend == 0);
    ASSERT(FdoData->nWaitSend == 0);
    ASSERT(IsListEmpty(&FdoData->SendQueueHead));

#if defined(DMA_VER2)
    if(MP_TEST_FLAG(FdoData, fMP_ADAPTER_SEND_SGL_LOOKASIDE))
    {
        ExDeleteNPagedLookasideList(&FdoData->SGListLookasideList);
        MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_SEND_SGL_LOOKASIDE);        
    }
#endif

    ASSERT(FdoData->nReadyRecv == FdoData->CurrNumRfd);

    while (!IsListEmpty(&FdoData->RecvList))
    {
        pMpRfd = (PMP_RFD)RemoveHeadList(&FdoData->RecvList);
        NICFreeRfd(FdoData, pMpRfd);
    }
    
    if (MP_TEST_FLAG(FdoData, fMP_ADAPTER_RECV_LOOKASIDE))
    {
        ExDeleteNPagedLookasideList(&FdoData->RecvLookaside);
        MP_CLEAR_FLAG(FdoData, fMP_ADAPTER_RECV_LOOKASIDE);
    }

    // Free the shared memory for HW_TCB structures
    if (FdoData->HwSendMemAllocVa)
    {
        FdoData->FreeCommonBuffer(
                        FdoData->DmaAdapterObject,
                        FdoData->HwSendMemAllocSize,
                        FdoData->HwSendMemAllocPa,
                        FdoData->HwSendMemAllocVa,
                        FALSE);
        FdoData->HwSendMemAllocVa = NULL;
    }

    // Free the shared memory for other command data structures                       
    if (FdoData->HwMiscMemAllocVa)
    {
        FdoData->FreeCommonBuffer(
                        FdoData->DmaAdapterObject,
                        FdoData->HwMiscMemAllocSize,
                        FdoData->HwMiscMemAllocPa,
                        FdoData->HwMiscMemAllocVa,
                        FALSE);
        FdoData->HwMiscMemAllocVa = NULL;
        FdoData->SelfTest = NULL;
        FdoData->StatsCounters = NULL;
        FdoData->NonTxCmdBlock = NULL;
        FdoData->DumpSpace = NULL;
    }


    // Free the memory for MP_TCB structures
    if (FdoData->MpTcbMem)
    {
        ExFreePoolWithTag(FdoData->MpTcbMem, PCIDRV_POOL_TAG);
        FdoData->MpTcbMem = NULL;
    }

    //Free all the wake up patterns on this adapter
    NICRemoveAllWakeUpPatterns(FdoData);
    
    DebugPrint(TRACE, DBG_INIT, "<-- NICFreeAdapterMemory\n");
}



VOID 
NICInitSend(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    Initialize send data structures. Can be called at DISPATCH_LEVEL.

Arguments:

    FdoData     Pointer to our adapter

Return Value:

    None                                                    

--*/    
{
    PMP_TCB         pMpTcb;
    PHW_TCB         pHwTcb;
    ULONG           HwTcbPhys;
    LONG            TcbCount;

    PTBD_STRUC      pHwTbd;  
    ULONG           HwTbdPhys;     

    DebugPrint(TRACE, DBG_INIT, "--> NICInitSend\n");

    FdoData->TransmitIdle = TRUE;
    FdoData->ResumeWait = TRUE;

    // Setup the initial pointers to the SW and HW TCB data space
    pMpTcb = (PMP_TCB) FdoData->MpTcbMem;
    pHwTcb = (PHW_TCB) FdoData->HwSendMemAllocVa;
    HwTcbPhys = FdoData->HwSendMemAllocPa.LowPart;

    // Setup the initial pointers to the TBD data space.
    // TBDs are located immediately following the TCBs
    pHwTbd = (PTBD_STRUC) (FdoData->HwSendMemAllocVa +
                 (sizeof(TXCB_STRUC) * FdoData->NumTcb));
    HwTbdPhys = HwTcbPhys + (sizeof(TXCB_STRUC) * FdoData->NumTcb);

    // Go through and set up each TCB
    for (TcbCount = 0; TcbCount < FdoData->NumTcb; TcbCount++)
    {
        pMpTcb->HwTcb = pHwTcb;                 // save ptr to HW TCB
        pMpTcb->HwTcbPhys = HwTcbPhys;      // save HW TCB physical address

        pMpTcb->HwTbd = pHwTbd;                 // save ptr to TBD array
        pMpTcb->HwTbdPhys = HwTbdPhys;      // save TBD array physical address

        if (TcbCount){
            pMpTcb->PrevHwTcb = pHwTcb - 1;
        }
        else {
            pMpTcb->PrevHwTcb   = (PHW_TCB)((PUCHAR)FdoData->HwSendMemAllocVa +
                                  ((FdoData->NumTcb - 1) * sizeof(HW_TCB)));
        }
        pHwTcb->TxCbHeader.CbStatus = 0;        // clear the status 
        pHwTcb->TxCbHeader.CbCommand = CB_EL_BIT | CB_TX_SF_BIT | CB_TRANSMIT;


        // Set the link pointer in HW TCB to the next TCB in the chain.  
        // If this is the last TCB in the chain, then set it to the first TCB.
        if (TcbCount < FdoData->NumTcb - 1)
        {
            pMpTcb->Next = pMpTcb + 1;
            pHwTcb->TxCbHeader.CbLinkPointer = HwTcbPhys + sizeof(HW_TCB);
        }
        else
        {
            pMpTcb->Next = (PMP_TCB) FdoData->MpTcbMem;
            pHwTcb->TxCbHeader.CbLinkPointer = 
                FdoData->HwSendMemAllocPa.LowPart;
        }

        pHwTcb->TxCbThreshold = (UCHAR) FdoData->AiThreshold;
        pHwTcb->TxCbTbdPointer = HwTbdPhys;

        pMpTcb++; 
        pHwTcb++;
        HwTcbPhys += sizeof(TXCB_STRUC);
        pHwTbd = (PTBD_STRUC)((PUCHAR)pHwTbd + sizeof(TBD_STRUC) * NIC_MAX_PHYS_BUF_COUNT);
        HwTbdPhys += sizeof(TBD_STRUC) * NIC_MAX_PHYS_BUF_COUNT;
    }

    // set the TCB head/tail indexes
    // head is the olded one to free, tail is the next one to use
    FdoData->CurrSendHead = (PMP_TCB) FdoData->MpTcbMem;
    FdoData->CurrSendTail = (PMP_TCB) FdoData->MpTcbMem;

    DebugPrint(TRACE, DBG_INIT, "<-- NICInitSend\n");
}

NTSTATUS 
NICInitRecv(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    Initialize receive data structures

Arguments:

    FdoData     Pointer to our adapter

Return Value:

--*/    
{
    NTSTATUS        status = STATUS_INSUFFICIENT_RESOURCES;
    PMP_RFD         pMpRfd;      
    LONG            RfdCount;
    PDMA_ADAPTER    DmaAdapterObject = FdoData->DmaAdapterObject;

    DebugPrint(TRACE, DBG_INIT, "--> NICInitRecv\n");


    // Setup each RFD
    for (RfdCount = 0; RfdCount < FdoData->NumRfd; RfdCount++)
    {
        pMpRfd = ExAllocateFromNPagedLookasideList(&FdoData->RecvLookaside);
        if (!pMpRfd)
        {
            //ErrorValue = ERRLOG_OUT_OF_LOOKASIDE_MEMORY;
            continue;
        }
        //
        // Allocate the shared memory for this RFD.
        //
        pMpRfd->OriginalHwRfd = FdoData->AllocateCommonBuffer(
            DmaAdapterObject,
            FdoData->HwRfdSize,
            &pMpRfd->OriginalHwRfdPa,
            FALSE);

        if (!pMpRfd->OriginalHwRfd)
        {
            //ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
            ExFreeToNPagedLookasideList(&FdoData->RecvLookaside, pMpRfd);
            continue;
        }

        //
        // Get a 8-byts aligned memory from the original HwRfd
        //
        pMpRfd->HwRfd = (PHW_RFD)DATA_ALIGN(pMpRfd->OriginalHwRfd);
        
        //
        // Now HwRfd is already 8-bytes aligned, and the size of HwPfd header(not data part) is a multiple of 8,
        // If we shift HwRfd 0xA bytes up, the Ethernet header size is 14 bytes long, then the data will be at
        // 8 byte boundary. 
        // 
        pMpRfd->HwRfd = (PHW_RFD)((PUCHAR)(pMpRfd->HwRfd) + HWRFD_SHIFT_OFFSET);
        //
        // Update physical address accordingly
        // 
        pMpRfd->HwRfdPa.QuadPart = pMpRfd->OriginalHwRfdPa.QuadPart + 
                         BYTES_SHIFT(pMpRfd->HwRfd, pMpRfd->OriginalHwRfd);


        status = NICAllocRfd(FdoData, pMpRfd);
        if (!NT_SUCCESS(status))
        {
            ExFreeToNPagedLookasideList(&FdoData->RecvLookaside, pMpRfd);
            continue;
        }
        //
        // Add this RFD to the RecvList
        // 
        FdoData->CurrNumRfd++;                      
        NICReturnRFD(FdoData, pMpRfd);
    }

    if (FdoData->CurrNumRfd > NIC_MIN_RFDS)
    {
        status = STATUS_SUCCESS;
    }
    //
    // FdoData->CurrNumRfd < NIC_MIN_RFDs
    //
    if (status != STATUS_SUCCESS)
    {
        // TODO: Log an entry into the eventlog
    }

    DebugPrint(TRACE, DBG_INIT, "<-- NICInitRecv, status=%x\n", status);

    return status;
}

NTSTATUS
NICAllocRfd(
    IN  PFDO_DATA     FdoData,
    IN  PMP_RFD       pMpRfd
    )
/*++
Routine Description:

    Allocate MDL and associate with a RFD.
    Should be called at PASSIVE_LEVEL.

Arguments:

    FdoData     Pointer to our adapter
    pMpRfd      pointer to a RFD

Return Value:


--*/    
{
    NTSTATUS            status = STATUS_SUCCESS;
    PHW_RFD             pHwRfd;    

    PAGED_CODE();
    
    do{
        pHwRfd = pMpRfd->HwRfd;
        pMpRfd->HwRfdPhys = pMpRfd->HwRfdPa.LowPart;

        pMpRfd->Flags = 0;
        pMpRfd->Mdl = IoAllocateMdl((PVOID)&pHwRfd->RfdBuffer.RxMacHeader,
                                    NIC_MAX_PACKET_SIZE,
                                    FALSE,
                                    FALSE,
                                    NULL);
        if (!pMpRfd->Mdl)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        
        MmBuildMdlForNonPagedPool(pMpRfd->Mdl);
        
        pMpRfd->Buffer = &pHwRfd->RfdBuffer.RxMacHeader;

        // Init each RFD header
        pHwRfd->RfdRbdPointer = DRIVER_NULL;
        pHwRfd->RfdSize = NIC_MAX_PACKET_SIZE;
    }while(FALSE);

    if (!NT_SUCCESS(status))
    {
        if (pMpRfd->HwRfd)
        {
            //
            // Free HwRfd, we need to free the original memory 
            // pointed by OriginalHwRfd.
            // 
            FdoData->FreeCommonBuffer(
                FdoData->DmaAdapterObject,
                FdoData->HwRfdSize,
                pMpRfd->OriginalHwRfdPa,        
                pMpRfd->OriginalHwRfd,
                FALSE);
    
            pMpRfd->HwRfd = NULL;
            pMpRfd->OriginalHwRfd = NULL;
        }
    }

    return status;

}

VOID 
NICFreeRfd(
    IN  PFDO_DATA     FdoData,
    IN  PMP_RFD         pMpRfd
    )
/*++
Routine Description:

    Free a RFD.
    Should be called at PASSIVE_LEVEL.

Arguments:

    FdoData     Pointer to our adapter
    pMpRfd      Pointer to a RFD

Return Value:

    None                                                    

--*/    
{
    PAGED_CODE();
    
    ASSERT(pMpRfd->HwRfd);    
    ASSERT(pMpRfd->Mdl);  

    IoFreeMdl(pMpRfd->Mdl);
    
    //
    // Free HwRfd, we need to free the original memory pointed 
    // by OriginalHwRfd.
    // 
    
    FdoData->FreeCommonBuffer(
        FdoData->DmaAdapterObject,
        FdoData->HwRfdSize,
        pMpRfd->OriginalHwRfdPa,        
        pMpRfd->OriginalHwRfd,
        FALSE);
    
    pMpRfd->HwRfd = NULL;
    pMpRfd->OriginalHwRfd = NULL;
    
    ExFreeToNPagedLookasideList(&FdoData->RecvLookaside, pMpRfd);
}

VOID
NICAllocRfdWorkItem(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PWORK_ITEM_CONTEXT Context
)
/*++

Routine Description:

   Worker routine to allocate memory for RFD at PASSIVE_LEVEL.

Arguments:

    DeviceObject - pointer to a device object.

    Context - pointer to the workitem.

Return Value:

   VOID

--*/
{
    PFDO_DATA   FdoData = (PFDO_DATA)DeviceObject->DeviceExtension;
    KIRQL       oldIrql;
    PMP_RFD     TempMpRfd;
    NTSTATUS    status;

    DebugPrint(TRACE, DBG_READ, "---> NICAllocRfdWorkItem\n");

    PAGED_CODE();
    
    TempMpRfd = ExAllocateFromNPagedLookasideList(&FdoData->RecvLookaside);
    if (TempMpRfd)
    {
        //
        // Allocate the shared memory for this RFD.
        //
        TempMpRfd->OriginalHwRfd = FdoData->AllocateCommonBuffer(
                                        FdoData->DmaAdapterObject,
                                        FdoData->HwRfdSize,
                                        &TempMpRfd->OriginalHwRfdPa,
                                        FALSE);
        if (!TempMpRfd->OriginalHwRfd)
        {
            ExFreeToNPagedLookasideList(&FdoData->RecvLookaside, TempMpRfd);
            DebugPrint(ERROR, DBG_READ, "Recv: AllocateCommonBuffer failed\n");                    
            goto Exit;    
        }

        //
        // First get a HwRfd at 8 byte boundary from OriginalHwRfd
        // 
        TempMpRfd->HwRfd = (PHW_RFD)DATA_ALIGN(TempMpRfd->OriginalHwRfd);
        //
        // Then shift HwRfd so that the data(after ethernet header) is at 8 bytes boundary
        //
        TempMpRfd->HwRfd = (PHW_RFD)((PUCHAR)TempMpRfd->HwRfd + HWRFD_SHIFT_OFFSET);
        //
        // Update physical address as well
        // 
        TempMpRfd->HwRfdPa.QuadPart = TempMpRfd->OriginalHwRfdPa.QuadPart + 
                            BYTES_SHIFT(TempMpRfd->HwRfd, TempMpRfd->OriginalHwRfd);

        status = NICAllocRfd(FdoData, TempMpRfd);
        if (!NT_SUCCESS(status))
        {
            //
            // NICAllocRfd frees common buffer when it returns an error.
            // So, let us not worry about freeing that here.
            //
            ExFreeToNPagedLookasideList(&FdoData->RecvLookaside, TempMpRfd);
            DebugPrint(ERROR, DBG_READ, "Recv: NICAllocRfd failed %x\n", status);                    
            goto Exit;
        }

        KeAcquireSpinLock(&FdoData->RcvLock, &oldIrql);
        
        //
        // Add this RFD to the RecvList
        //
        FdoData->CurrNumRfd++;                      
        NICReturnRFD(FdoData, TempMpRfd);

        KeReleaseSpinLock(&FdoData->RcvLock, oldIrql);
        
        ASSERT(FdoData->CurrNumRfd <= FdoData->MaxNumRfd);
        DebugPrint(TRACE, DBG_READ, "CurrNumRfd=%d\n", FdoData->CurrNumRfd);                
        
    }    

Exit:
    PciDrvIoDecrement(FdoData);
    FdoData->AllocNewRfd = FALSE;                

    IoFreeWorkItem((PIO_WORKITEM)Context->WorkItem);
    ExFreePool((PVOID)Context);

    DebugPrint(TRACE, DBG_READ, "<--- NICAllocRfdWorkItem\n");

    return;
}
    
VOID
NICFreeRfdWorkItem(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PWORK_ITEM_CONTEXT   Context
)
/*++

Routine Description:

   Worker routine to RFD memory at PASSIVE_LEVEL.

Arguments:

    DeviceObject - pointer to a device object.

    Context - pointer to the workitem.

Return Value:

   VOID

--*/
{
    PFDO_DATA   fdoData = (PFDO_DATA)DeviceObject->DeviceExtension;
    PMP_RFD     pMpRfd = Context->Argument1;

    DebugPrint(TRACE, DBG_READ, "---> NICFreeRfdWorkItem\n");
    
    PAGED_CODE();
    
    NICFreeRfd(fdoData, pMpRfd);

    IoFreeWorkItem((PIO_WORKITEM)Context->WorkItem);
    ExFreePool((PVOID)Context);

    PciDrvIoDecrement (fdoData);
    
    DebugPrint(TRACE, DBG_READ, "<--- NICFreeRfdWorkItem\n");

    return;
}


NTSTATUS 
NICSelfTest(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    Perform a NIC self-test

Arguments:

    FdoData     Pointer to our adapter

Return Value:

    NT status code

--*/    
{
    NTSTATUS     status = STATUS_SUCCESS;
    ULONG           SelfTestCommandCode;

    DebugPrint(TRACE, DBG_INIT, "--> NICSelfTest\n");

    DebugPrint(LOUD, DBG_INIT, "SelfTest=%p, SelfTestPhys=%x\n", 
        FdoData->SelfTest, FdoData->SelfTestPhys);

    //
    // Issue a software reset to the adapter
    // 
    HwSoftwareReset(FdoData);

    //
    // Execute The PORT Self Test Command On The 82558.
    // 
    ASSERT(FdoData->SelfTestPhys != 0);
    SelfTestCommandCode = FdoData->SelfTestPhys;

    //
    // Setup SELF TEST Command Code in D3 - D0
    // 
    SelfTestCommandCode |= PORT_SELFTEST;

    //
    // Initialize the self-test signature and results DWORDS
    // 
    FdoData->SelfTest->StSignature = 0;
    FdoData->SelfTest->StResults = 0xffffffff;

    //
    // Do the port command
    // 
    FdoData->CSRAddress->Port = SelfTestCommandCode;

    MP_STALL_EXECUTION(NIC_DELAY_POST_SELF_TEST_MS);

    //
    // if The First Self Test DWORD Still Zero, We've timed out.  If the second
    // DWORD is not zero then we have an error.
    // 
    if ((FdoData->SelfTest->StSignature == 0) || (FdoData->SelfTest->StResults != 0))
    {
        DebugPrint(ERROR, DBG_INIT, "StSignature=%x, StResults=%x\n", 
            FdoData->SelfTest->StSignature, FdoData->SelfTest->StResults);

       status = STATUS_DEVICE_CONFIGURATION_ERROR;
    }

    DebugPrint(TRACE, DBG_INIT, "<-- NICSelfTest, status=%x\n", status);

    return status;
}

NTSTATUS 
NICInitializeAdapter(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    Initialize the adapter and set up everything

Arguments:

    FdoData     Pointer to our adapter

Return Value:

 NT Status Code
 
--*/    
{
    NTSTATUS     status;

    DebugPrint(TRACE, DBG_INIT, "--> NICInitializeAdapter\n");

    do
    {

        // set up our link indication variable
        // it doesn't matter what this is right now because it will be
        // set correctly if link fails
        FdoData->MediaState = Connected;

        // Issue a software reset to the D100
        HwSoftwareReset(FdoData);

        // Load the CU BASE (set to 0, because we use linear mode)
        FdoData->CSRAddress->ScbGeneralPointer = 0;
        status = D100IssueScbCommand(FdoData, SCB_CUC_LOAD_BASE, FALSE);
        if (status != STATUS_SUCCESS)
        {
            break;
        }

        // Wait for the SCB command word to clear before we set the general pointer
        if (!WaitScb(FdoData))
        {
            status = STATUS_DEVICE_DATA_ERROR;
            break;
        }

        // Load the RU BASE (set to 0, because we use linear mode)
        FdoData->CSRAddress->ScbGeneralPointer = 0;
        status = D100IssueScbCommand(FdoData, SCB_RUC_LOAD_BASE, FALSE);
        if (status != STATUS_SUCCESS)
        {
            break;
        }

        // Configure the adapter
        status = HwConfigure(FdoData);
        if (status != STATUS_SUCCESS)
        {
            break;
        }

        status = HwSetupIAAddress(FdoData);
        if (status != STATUS_SUCCESS) 
        {
            break;
        }

        // Clear the internal counters
        HwClearAllCounters(FdoData);


    } while (FALSE);
    
    if (status != STATUS_SUCCESS)
    {
        // TODO: Log an entry into the eventlog
    }

    DebugPrint(TRACE, DBG_INIT, "<-- NICInitializeAdapter, status=%x\n", status);

    return status;
}    

VOID 
NICShutdown(
    IN  PFDO_DATA     FdoData)
/*++

Routine Description:
    
    Shutdown the device
    
Arguments:

    FdoData -  Pointer to our adapter

Return Value:

    None
    
--*/
{
    DebugPrint(INFO, DBG_INIT, "---> NICShutdown\n");

    if(FdoData->CSRAddress) {        
        //
        // Disable interrupt and issue a full reset
        //
        NICDisableInterrupt(FdoData);
        NICIssueFullReset(FdoData);
        //
        // Reset the PHY chip.  We do this so that after a warm boot, the PHY will
        // be in a known state, with auto-negotiation enabled.
        //
        ResetPhy(FdoData);    
    }
    DebugPrint(INFO, DBG_INIT, "<--- NICShutdown\n");
}


VOID 
HwSoftwareReset(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    Issue a software reset to the hardware    

Arguments:

    FdoData     Pointer to our adapter

Return Value:

    None                                                    

--*/    
{
    DebugPrint(TRACE, DBG_HW_ACCESS, "--> HwSoftwareReset\n");

    // Issue a PORT command with a data word of 0
    FdoData->CSRAddress->Port = PORT_SOFTWARE_RESET;

    // wait after the port reset command
    KeStallExecutionProcessor(NIC_DELAY_POST_RESET);

    // Mask off our interrupt line -- its unmasked after reset
    NICDisableInterrupt(FdoData);

    DebugPrint(TRACE, DBG_HW_ACCESS, "<-- HwSoftwareReset\n");
}



NTSTATUS
HwConfigure(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    Configure the hardware    

Arguments:

    FdoData     Pointer to our adapter

Return Value:

    NT Status


--*/    
{
    NTSTATUS         status;
    PCB_HEADER_STRUC    NonTxCmdBlockHdr = 
                                (PCB_HEADER_STRUC)FdoData->NonTxCmdBlock;
    UINT                i;

    DebugPrint(TRACE, DBG_HW_ACCESS, "--> HwConfigure\n");

    //
    // Init the packet filter to nothing.
    //
    FdoData->OldPacketFilter = FdoData->PacketFilter;
    FdoData->PacketFilter = 0;
    
    //
    // Store the current setting for BROADCAST/PROMISCUOS modes
    FdoData->OldParameterField = CB_557_CFIG_DEFAULT_PARM15;
    
    // Setup the non-transmit command block header for the configure command.
    NonTxCmdBlockHdr->CbStatus = 0;
    NonTxCmdBlockHdr->CbCommand = CB_CONFIGURE;
    NonTxCmdBlockHdr->CbLinkPointer = DRIVER_NULL;

    // Fill in the configure command data.

    // First fill in the static (end user can't change) config bytes
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[0] = CB_557_CFIG_DEFAULT_PARM0;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[2] = CB_557_CFIG_DEFAULT_PARM2;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[3] = CB_557_CFIG_DEFAULT_PARM3;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[6] = CB_557_CFIG_DEFAULT_PARM6;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[9] = CB_557_CFIG_DEFAULT_PARM9;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[10] = CB_557_CFIG_DEFAULT_PARM10;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[11] = CB_557_CFIG_DEFAULT_PARM11;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[12] = CB_557_CFIG_DEFAULT_PARM12;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[13] = CB_557_CFIG_DEFAULT_PARM13;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[14] = CB_557_CFIG_DEFAULT_PARM14;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[16] = CB_557_CFIG_DEFAULT_PARM16;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[17] = CB_557_CFIG_DEFAULT_PARM17;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[18] = CB_557_CFIG_DEFAULT_PARM18;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[20] = CB_557_CFIG_DEFAULT_PARM20;
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[21] = CB_557_CFIG_DEFAULT_PARM21;

    // Now fill in the rest of the configuration bytes (the bytes that contain
    // user configurable parameters).

    // Set the Tx and Rx Fifo limits
    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[1] =
        (UCHAR) ((FdoData->AiTxFifo << 4) | FdoData->AiRxFifo);

    if (FdoData->MWIEnable)
    {
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[3] |= CB_CFIG_B3_MWI_ENABLE;
    }

    // Set the Tx and Rx DMA maximum byte count fields.
    if ((FdoData->AiRxDmaCount) || (FdoData->AiTxDmaCount))
    {
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[4] =
            FdoData->AiRxDmaCount;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[5] =
            (UCHAR) (FdoData->AiTxDmaCount | CB_CFIG_DMBC_EN);
    }
    else
    {
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[4] =
            CB_557_CFIG_DEFAULT_PARM4;
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[5] =
            CB_557_CFIG_DEFAULT_PARM5;
    }


    FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[7] =
        (UCHAR) ((CB_557_CFIG_DEFAULT_PARM7 & (~CB_CFIG_URUN_RETRY)) |
        (FdoData->AiUnderrunRetry << 1)
        );

    // Setup for MII or 503 operation.  The CRS+CDT bit should only be set
    // when operating in 503 mode.
    if (FdoData->PhyAddress == 32)
    {
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[8] =
            (CB_557_CFIG_DEFAULT_PARM8 & (~CB_CFIG_503_MII));
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[15] =
            (CB_557_CFIG_DEFAULT_PARM15 | CB_CFIG_CRS_OR_CDT);
    }
    else
    {
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[8] =
            (CB_557_CFIG_DEFAULT_PARM8 | CB_CFIG_503_MII);
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[15] =
            ((CB_557_CFIG_DEFAULT_PARM15 & (~CB_CFIG_CRS_OR_CDT)) | CB_CFIG_BROADCAST_DIS);
    }


    // Setup Full duplex stuff

    // If forced to half duplex
    if (FdoData->AiForceDpx == 1)
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
            (CB_557_CFIG_DEFAULT_PARM19 &
            (~(CB_CFIG_FORCE_FDX| CB_CFIG_FDX_ENABLE)));

    // If forced to full duplex
    else if (FdoData->AiForceDpx == 2)
        FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
            (CB_557_CFIG_DEFAULT_PARM19 | CB_CFIG_FORCE_FDX);

    // If auto-duplex
    else
    {
        // We must force full duplex on if we are using PHY 0, and we are
        // supposed to run in FDX mode.  We do this because the D100 has only
        // one FDX# input pin, and that pin will be connected to PHY 1.
        if ((FdoData->PhyAddress == 0) && (FdoData->usDuplexMode == 2))
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
                (CB_557_CFIG_DEFAULT_PARM19 | CB_CFIG_FORCE_FDX);
        else
            FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[19] =
            CB_557_CFIG_DEFAULT_PARM19;
    }


    // display the config info to the debugger
    DebugPrint(LOUD, DBG_HW_ACCESS, "   Issuing Configure command\n");
    DebugPrint(LOUD, DBG_HW_ACCESS, "   Config Block at virt addr %p phys address %x\n",
        &NonTxCmdBlockHdr->CbStatus, FdoData->NonTxCmdBlockPhys);

    for (i=0; i < CB_CFIG_BYTE_COUNT; i++)
        DebugPrint(LOUD, DBG_HW_ACCESS, "   Config byte %x = %.2x\n", 
            i, FdoData->NonTxCmdBlock->NonTxCb.Config.ConfigBytes[i]);

    // Wait for the SCB command word to clear before we set the general pointer
    if (!WaitScb(FdoData))
    {
        status = STATUS_DEVICE_DATA_ERROR;
    }
    else
    {
        ASSERT(FdoData->CSRAddress->ScbCommandLow == 0);
        FdoData->CSRAddress->ScbGeneralPointer = FdoData->NonTxCmdBlockPhys;
    
        // Submit the configure command to the chip, and wait for it to complete.
        status = D100SubmitCommandBlockAndWait(FdoData);
    }

    DebugPrint(TRACE, DBG_HW_ACCESS, 
                            "<-- HwConfigure, status=%x\n", status);

    return status;
}


NTSTATUS 
HwSetupIAAddress(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    Set up the individual MAC address                             

Arguments:

    FdoData     Pointer to our adapter

Return Value:

    NT Status code

--*/    
{
    NTSTATUS         status;
    UINT                i;
    PCB_HEADER_STRUC    NonTxCmdBlockHdr = 
                            (PCB_HEADER_STRUC)FdoData->NonTxCmdBlock;

    DebugPrint(TRACE, DBG_HW_ACCESS, "--> HwSetupIAAddress\n");

    // Individual Address Setup
    NonTxCmdBlockHdr->CbStatus = 0;
    NonTxCmdBlockHdr->CbCommand = CB_IA_ADDRESS;
    NonTxCmdBlockHdr->CbLinkPointer = DRIVER_NULL;

    // Copy in the station's individual address
    for (i = 0; i < ETH_LENGTH_OF_ADDRESS; i++)
        FdoData->NonTxCmdBlock->NonTxCb.Setup.IaAddress[i] = FdoData->CurrentAddress[i];

    // Update the command list pointer.  We don't need to do a WaitSCB here
    // because this command is either issued immediately after a reset, or
    // after another command that runs in polled mode.  This guarantees that
    // the low byte of the SCB command word will be clear.  The only commands
    // that don't run in polled mode are transmit and RU-start commands.
    ASSERT(FdoData->CSRAddress->ScbCommandLow == 0);
    FdoData->CSRAddress->ScbGeneralPointer = FdoData->NonTxCmdBlockPhys;

    // Submit the IA configure command to the chip, and wait for it to complete.
    status = D100SubmitCommandBlockAndWait(FdoData);

    DebugPrint(TRACE, DBG_HW_ACCESS, "<-- HwSetupIAAddress, status=%x\n", status);

    return status;
}

NTSTATUS 
HwClearAllCounters(
    IN  PFDO_DATA     FdoData
    )
/*++
Routine Description:

    This routine will clear the hardware error statistic counters
    
Arguments:

    FdoData     Pointer to our adapter

Return Value:

    NT status code


--*/    
{
    NTSTATUS     status;
    BOOLEAN         bResult;

    DebugPrint(TRACE, DBG_HW_ACCESS, "--> HwClearAllCounters\n");

    do
    {
        // Load the dump counters pointer.  Since this command is generated only
        // after the IA setup has complete, we don't need to wait for the SCB
        // command word to clear
        ASSERT(FdoData->CSRAddress->ScbCommandLow == 0);
        FdoData->CSRAddress->ScbGeneralPointer = FdoData->StatsCounterPhys;

        // Issue the load dump counters address command
        status = D100IssueScbCommand(FdoData, SCB_CUC_DUMP_ADDR, FALSE);
        if (status != STATUS_SUCCESS) 
            break;

        // Now dump and reset all of the statistics
        status = D100IssueScbCommand(FdoData, SCB_CUC_DUMP_RST_STAT, TRUE);
        if (status != STATUS_SUCCESS) 
            break;

        // Now wait for the dump/reset to complete, timeout value 2 secs
        MP_STALL_AND_WAIT(FdoData->StatsCounters->CommandComplete == 0xA007, 2000, bResult);
        if (!bResult)
        {
            MP_SET_HARDWARE_ERROR(FdoData);
            status = STATUS_DEVICE_DATA_ERROR;
            break;
        }

        // init packet counts
        FdoData->GoodTransmits = 0;
        FdoData->GoodReceives = 0;

        // init transmit error counts
        FdoData->TxAbortExcessCollisions = 0;
        FdoData->TxLateCollisions = 0;
        FdoData->TxDmaUnderrun = 0;
        FdoData->TxLostCRS = 0;
        FdoData->TxOKButDeferred = 0;
        FdoData->OneRetry = 0;
        FdoData->MoreThanOneRetry = 0;
        FdoData->TotalRetries = 0;

        // init receive error counts
        FdoData->RcvCrcErrors = 0;
        FdoData->RcvAlignmentErrors = 0;
        FdoData->RcvResourceErrors = 0;
        FdoData->RcvDmaOverrunErrors = 0;
        FdoData->RcvCdtFrames = 0;
        FdoData->RcvRuntErrors = 0;

    } while (FALSE);

    DebugPrint(TRACE, DBG_HW_ACCESS,
                    "<-- HwClearAllCounters, status=%x\n", status);

    return status;
}

VOID
NICGetDeviceInfSettings(
    IN OUT  PFDO_DATA   FdoData
    )
{

    //
    // Number of ReceiveFrameDescriptors
    //
    if(!PciDrvReadRegistryValue(FdoData,
                                L"NumRfd", 
                                &FdoData->NumRfd)){
        FdoData->NumRfd = 32;
    }
    
    FdoData->NumRfd = min(FdoData->NumRfd, NIC_MAX_RFDS);
    FdoData->NumRfd = max(FdoData->NumRfd, 1);
    
    //
    // Number of Transmit Control Blocks
    //
    if(!PciDrvReadRegistryValue(FdoData,
                                L"NumTcb", 
                                &FdoData->NumTcb)){
        FdoData->NumTcb  = NIC_DEF_TCBS;
        
    }

    FdoData->NumTcb = min(FdoData->NumTcb, NIC_MAX_TCBS);
    FdoData->NumTcb = max(FdoData->NumTcb, 1);
    
    //
    // Max number of buffers required for coalescing fragmented packet.
    // Not implemented in this sample
    //
    if(!PciDrvReadRegistryValue(FdoData,
                                L"NumCoalesce", 
                                &FdoData->NumBuffers)){
        FdoData->NumBuffers  = 8;
    }
    
    FdoData->NumBuffers = min(FdoData->NumBuffers, 32);
    FdoData->NumBuffers = max(FdoData->NumBuffers, 1);

    //
    // Get the Link Speed & Duplex.
    //
    if(!PciDrvReadRegistryValue(FdoData,
                                L"SpeedDuplex", 
                                &FdoData->SpeedDuplex)){
        FdoData->SpeedDuplex  = 0;
    }    
    FdoData->SpeedDuplex = min(FdoData->SpeedDuplex, 4);
    FdoData->SpeedDuplex = max(FdoData->SpeedDuplex, 0);
    //
    // Decode SpeedDuplex
    // Value 0 means Auto detect
    // Value 1 means 10Mb-Half-Duplex
    // Value 2 means 10Mb-Full-Duplex 
    // Value 3 means 100Mb-Half-Duplex
    // Value 4 means 100Mb-Full-Duplex
    //
    switch(FdoData->SpeedDuplex)
    {
        case 1:
        FdoData->AiTempSpeed = 10; FdoData->AiForceDpx = 1;
        break;
        
        case 2:
        FdoData->AiTempSpeed = 10; FdoData->AiForceDpx = 2;
        break;
        
        case 3:
        FdoData->AiTempSpeed = 100; FdoData->AiForceDpx = 1;
        break;
        
        case 4:
        FdoData->AiTempSpeed = 100; FdoData->AiForceDpx = 2;
        break;
    }

    //
    // Find out whether the driver is installed as a miniport.
    //
    PciDrvReadRegistryValue(FdoData, L"IsUpperEdgeNdis", 
                                    &FdoData->IsUpperEdgeNdis);
    
    DebugPrint(INFO, DBG_INIT, "Upper edge is %sNDIS\n", 
                                FdoData->IsUpperEdgeNdis?"":"not ");
    
    
    //
    // Rest of these values are currently not configured thru INF.
    //
    FdoData->PhyAddress  = 0xFF;
    FdoData->Connector  = 0;
    FdoData->AiTxFifo  = DEFAULT_TX_FIFO_LIMIT;
    FdoData->AiRxFifo  = DEFAULT_RX_FIFO_LIMIT;
    FdoData->AiTxDmaCount  = 0;
    FdoData->AiRxDmaCount  = 0;
    FdoData->AiUnderrunRetry  = DEFAULT_UNDERRUN_RETRY;
    FdoData->AiThreshold  = 200;
    FdoData->MWIEnable = 1;    
    FdoData->Congest = 0;

    return;    
 }

NTSTATUS
GetPCIBusInterfaceStandard(
    IN  PDEVICE_OBJECT DeviceObject,
    OUT PBUS_INTERFACE_STANDARD    BusInterfaceStandard
    )
/*++
Routine Description:

    This routine gets the bus interface standard information from the PDO.
    
Arguments:
    DeviceObject - Device object to query for this information.
    BusInterface - Supplies a pointer to the retrieved information.
    
Return Value:

    NT status.
    
--*/ 
{
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_OBJECT targetObject;

    DebugPrint(TRACE, DBG_INIT, "GetPciBusInterfaceStandard entered.\n");
    
    KeInitializeEvent( &event, NotificationEvent, FALSE );
    
    targetObject = IoGetAttachedDeviceReference( DeviceObject );
    
    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        targetObject,
                                        NULL,
                                        0,
                                        NULL,
                                        &event,
                                        &ioStatusBlock );
    if (irp == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto End;
    }

    irpStack = IoGetNextIrpStackLocation( irp );
    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
    irpStack->Parameters.QueryInterface.InterfaceType = 
                        (LPGUID) &GUID_BUS_INTERFACE_STANDARD ;
    irpStack->Parameters.QueryInterface.Size = sizeof(BUS_INTERFACE_STANDARD);
    irpStack->Parameters.QueryInterface.Version = 1;
    irpStack->Parameters.QueryInterface.Interface = 
                                        (PINTERFACE)BusInterfaceStandard;
    
    irpStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;
    
    //    
    // Initialize the status to error in case the bus driver does not 
    // set it correctly.
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    
    status = IoCallDriver( targetObject, irp );
    if (status == STATUS_PENDING) {
        
        status = KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL);
        ASSERT(NT_SUCCESS(status));
        status = ioStatusBlock.Status;
        
    }
    
End:
    // Done with reference
    ObDereferenceObject( targetObject );
    return status;
    
} 



#if 0
NTSTATUS
ReadWriteConfigSpace(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          ReadOrWrite, // 0 for read 1 for write
    IN PVOID          Buffer,
    IN ULONG          Offset,
    IN ULONG          Length
    )
/*++
Routine Description:

    Worker function to read & write to PCI config space at
    PASSIVE_LEVEL

Arguments:

    DeviceObject - Pointer to the PDO
    ReadOrWrite  - Whichspace - 0 for read 1 for write
    Buffer       - Buffer to be written to or read from
    Offset       - Offset in to the PCI config space
    Length       - Length of the buffer

Return Value:

     NT Status code

--*/
{
    KEVENT event;
    NTSTATUS status;
    PIRP irp;
    IO_STATUS_BLOCK ioStatusBlock;
    PIO_STACK_LOCATION irpStack;
    PDEVICE_OBJECT targetObject;

    PAGED_CODE();
    
    KeInitializeEvent( &event, NotificationEvent, FALSE );
    
    targetObject = IoGetAttachedDeviceReference( DeviceObject );
    
    irp = IoBuildSynchronousFsdRequest( IRP_MJ_PNP,
                                        targetObject,
                                        NULL,
                                        0,
                                        NULL,
                                        &event,
                                        &ioStatusBlock );
    
    if (irp == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto End;
    }
    
    irpStack = IoGetNextIrpStackLocation( irp );
    if (ReadOrWrite == 0) {
        irpStack->MinorFunction = IRP_MN_READ_CONFIG;
    }else {
        irpStack->MinorFunction = IRP_MN_WRITE_CONFIG;
    }
    
    irpStack->Parameters.ReadWriteConfig.WhichSpace = PCI_WHICHSPACE_CONFIG;
    irpStack->Parameters.ReadWriteConfig.Buffer = Buffer;
    irpStack->Parameters.ReadWriteConfig.Offset = Offset;
    irpStack->Parameters.ReadWriteConfig.Length = Length;
    
    //
    // Initialize the status to error in case the bus driver does not 
    // set it correctly.
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED ;
    
    status = IoCallDriver( targetObject, irp );
    
    if (status == STATUS_PENDING) {
        
        KeWaitForSingleObject( &event, Executive, KernelMode, FALSE, NULL );
        status = ioStatusBlock.Status;
        
    }
    
End:
    // Done with reference
    ObDereferenceObject( targetObject );
    return status;
} 

#endif

