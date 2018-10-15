/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

   INIT.C

Abstract:

    This module contains initialization helper routines called during
    MiniportInitialize.

Revision History:

Notes:

--*/
#include "miniport.h"

#pragma NDIS_PAGEABLE_FUNCTION(NICAllocAdapter)
#pragma NDIS_PAGEABLE_FUNCTION(NICFreeAdapter)
#pragma NDIS_PAGEABLE_FUNCTION(NICInitializeAdapter)
#pragma NDIS_PAGEABLE_FUNCTION(NICReadRegParameters)

NDIS_STATUS NICAllocAdapter(
    PMP_ADAPTER *pAdapter)
{
    PMP_ADAPTER Adapter = NULL;
    PNDIS_PACKET Packet;
    PNDIS_BUFFER Buffer;
    PUCHAR pTCBMem;
    PTCB  pTCB;
    NDIS_STATUS Status;

    LONG index;

    DEBUGP(MP_TRACE, ("--> NICAllocAdapter\n"));

    *pAdapter = NULL;

    do
    {
        //
        // Allocate memory for adapter context
        //
        Status = NdisAllocateMemoryWithTag(
            &Adapter, 
            sizeof(MP_ADAPTER), 
            NIC_TAG);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("Failed to allocate memory for adapter context\n"));
            break;
        }
        //
        // Zero the memory block
        //
        NdisZeroMemory(Adapter, sizeof(MP_ADAPTER));
        NdisInitializeListHead(&Adapter->List);

        //
        // Initialize Send & Recv listheads and corresponding 
        // spinlocks.
        //
        NdisInitializeListHead(&Adapter->RecvWaitList);
        NdisInitializeListHead(&Adapter->SendWaitList);
        NdisInitializeListHead(&Adapter->SendFreeList);
        NdisAllocateSpinLock(&Adapter->SendLock);                                                  

        NdisInitializeListHead(&Adapter->RecvFreeList);
        NdisAllocateSpinLock(&Adapter->RecvLock);  

        //
        // Allocate lookside list for Receive Control blocks.
        //
        NdisInitializeNPagedLookasideList(
                    &Adapter->RecvLookaside,
                    NULL, // No Allocate function
                    NULL, // No Free function
                    0,    // Reserved for system use
                    sizeof(RCB),
                    NIC_TAG, 
                    0); // Reserved for system use
                    
        MP_SET_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE); 
        
        //
        // Allocate packet pool for receive indications
        //
        NdisAllocatePacketPool(
            &Status,
            &Adapter->RecvPacketPoolHandle,
            NIC_MAX_BUSY_RECVS,
            PROTOCOL_RESERVED_SIZE_IN_PACKET);
        
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("NdisAllocatePacketPool failed\n"));
            break;
        }
        //
        // Initialize receive packets
        //
        for(index=0; index < NIC_MAX_BUSY_RECVS; index++)
        {
            //
            // Allocate a packet descriptor for receive packets
            // from a preallocated pool.
            //
            NdisAllocatePacket(
                &Status,
                &Packet,
                Adapter->RecvPacketPoolHandle);
            if(Status != NDIS_STATUS_SUCCESS)
            {
                DEBUGP(MP_ERROR, ("NdisAllocatePacket failed\n"));
                break;
            }
            
            NDIS_SET_PACKET_HEADER_SIZE(Packet, ETH_HEADER_SIZE);

            //
            // Insert it into the list of free receive packets.
            //
            NdisInterlockedInsertTailList(
                &Adapter->RecvFreeList, 
                (PLIST_ENTRY)&Packet->MiniportReserved[0], 
                &Adapter->RecvLock);
        }
        
        //
        // Allocate a huge block of memory for all TCB's
        //
        Status = NdisAllocateMemoryWithTag(
            &pTCBMem, 
            sizeof(TCB) * NIC_MAX_BUSY_SENDS, 
            NIC_TAG);
        
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("Failed to allocate memory for TCB's\n"));
            break;
        }
        NdisZeroMemory(pTCBMem, sizeof(TCB) * NIC_MAX_BUSY_SENDS);
        Adapter->TCBMem = pTCBMem;

        //
        // Allocate a buffer pool for send buffers.
        //

        NdisAllocateBufferPool(
            &Status,
            &Adapter->SendBufferPoolHandle,
            NIC_MAX_BUSY_SENDS);
        if(Status != NDIS_STATUS_SUCCESS)
        {
            DEBUGP(MP_ERROR, ("NdisAllocateBufferPool for send buffer failed\n"));
            break;
        }

        //
        // Divide the TCBMem blob into TCBs and create a buffer
        // descriptor for the Data portion of the TCBs.
        //
        for(index=0; index < NIC_MAX_BUSY_SENDS; index++)
        {
            pTCB = (PTCB) pTCBMem;
            //
            // Create a buffer descriptor for the Data portion of the TCBs.
            // Buffer descriptors are nothing but MDLs on NT systems.
            //
            NdisAllocateBuffer(
                &Status,
                &Buffer,
                Adapter->SendBufferPoolHandle,
                (PVOID)&pTCB->Data[0],
                NIC_BUFFER_SIZE);
            if(Status != NDIS_STATUS_SUCCESS)
            {
                DEBUGP(MP_ERROR, ("NdisAllocateBuffer failed\n"));
                break;
            }

            //
            // Initialize the TCB structure.
            // 
            pTCB->Buffer = Buffer;
            pTCB->pData = (PUCHAR) &pTCB->Data[0];       
            pTCB->Adapter = Adapter;

            NdisInterlockedInsertTailList(
                &Adapter->SendFreeList, 
                &pTCB->List, 
                &Adapter->SendLock);

            pTCBMem = pTCBMem + sizeof(TCB);
        }

    } while(FALSE);


    *pAdapter = Adapter;

    //
    // In the failure case, the caller of this routine will end up
    // calling NICFreeAdapter to free all the successfully allocated
    // resources.
    //
    DEBUGP(MP_TRACE, ("<-- NICAllocAdapter\n"));

    return(Status);

}

void NICFreeAdapter(
    PMP_ADAPTER Adapter)
{
    NDIS_STATUS    Status;
    PNDIS_PACKET   Packet;
    PNDIS_BUFFER   Buffer;
    PUCHAR         pMem;
    PTCB           pTCB;
    PLIST_ENTRY    pEntry;

    DEBUGP(MP_TRACE, ("--> NICFreeAdapter\n"));

    ASSERT(Adapter);
    ASSERT(!Adapter->RefCount);
    
    //
    // Free all the resources we allocated for send.
    //
    while(!IsListEmpty(&Adapter->SendFreeList))
    {
        pTCB = (PTCB) NdisInterlockedRemoveHeadList(
                     &Adapter->SendFreeList, 
                     &Adapter->SendLock);
        if(!pTCB)
        {
            break;
        }

        if(pTCB->Buffer)
        {
            NdisFreeBuffer(pTCB->Buffer);
        }
    }

    if(Adapter->SendBufferPoolHandle)
    {
        NdisFreeBufferPool(Adapter->SendBufferPoolHandle);
    }

    NdisFreeMemory(Adapter->TCBMem, sizeof(TCB) * NIC_MAX_BUSY_SENDS, 0);
    ASSERT(IsListEmpty(&Adapter->SendFreeList));                  
    ASSERT(IsListEmpty(&Adapter->RecvWaitList));                  
    ASSERT(IsListEmpty(&Adapter->SendWaitList));                  
    NdisFreeSpinLock(&Adapter->SendLock);

    //
    // Free all the resources we allocated for receive.
    //
    
    if (MP_TEST_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE))
    {
        NdisDeleteNPagedLookasideList(&Adapter->RecvLookaside);
        MP_CLEAR_FLAG(Adapter, fMP_ADAPTER_RECV_LOOKASIDE);
    }
  
    while(!IsListEmpty(&Adapter->RecvFreeList))
    {
        pEntry = (PLIST_ENTRY) NdisInterlockedRemoveHeadList(
                        &Adapter->RecvFreeList, 
                        &Adapter->RecvLock);
        if(pEntry)
        {
            Packet = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReserved);
            NdisFreePacket(Packet);
        }
    }

    if(Adapter->RecvPacketPoolHandle)
    {
        NdisFreePacketPool(Adapter->RecvPacketPoolHandle);
    }

    ASSERT(IsListEmpty(&Adapter->RecvFreeList));                  
    NdisFreeSpinLock(&Adapter->RecvLock);

    //
    // Finally free the memory for adapter context.
    //
    NdisFreeMemory(Adapter, sizeof(MP_ADAPTER), 0);  

    DEBUGP(MP_TRACE, ("<-- NICFreeAdapter\n"));
}

void NICAttachAdapter(PMP_ADAPTER Adapter)
{
    DEBUGP(MP_TRACE, ("--> NICAttachAdapter\n"));

    NdisInterlockedInsertTailList(
        &GlobalData.AdapterList, 
        &Adapter->List, 
        &GlobalData.Lock);

    DEBUGP(MP_TRACE, ("<-- NICAttachAdapter\n"));
}

void NICDetachAdapter(PMP_ADAPTER Adapter)
{
    DEBUGP(MP_TRACE, ("--> NICDetachAdapter\n"));

    NdisAcquireSpinLock(&GlobalData.Lock);
    RemoveEntryList(&Adapter->List);
    NdisReleaseSpinLock(&GlobalData.Lock); 
    DEBUGP(MP_TRACE, ("<-- NICDetachAdapter\n"));
}

NDIS_STATUS 
NICReadRegParameters(
    PMP_ADAPTER Adapter,
    NDIS_HANDLE WrapperConfigurationContext)
/*++
Routine Description:

    Read device configuration parameters from the registry
 
Arguments:

    Adapter                         Pointer to our adapter
    WrapperConfigurationContext     For use by NdisOpenConfiguration

    Should be called at IRQL = PASSIVE_LEVEL.
    
Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_FAILURE
    NDIS_STATUS_RESOURCES                                       

--*/    
{
    NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
    NDIS_HANDLE     ConfigurationHandle;
    PUCHAR          NetworkAddress;
    UINT            Length;
    PUCHAR          pAddr;
    static ULONG    g_ulAddress = 0;
    
    DEBUGP(MP_TRACE, ("--> NICReadRegParameters\n"));

    //
    // Open the registry for this adapter to read advanced 
    // configuration parameters stored by the INF file.
    //
    NdisOpenConfiguration(
        &Status,
        &ConfigurationHandle,
        WrapperConfigurationContext);
    if(Status != NDIS_STATUS_SUCCESS)
    {
        DEBUGP(MP_ERROR, ("NdisOpenConfiguration failed\n"));
        return NDIS_STATUS_FAILURE;
    }

    //
    // Read all of our configuration parameters using NdisReadConfiguration
    // and parse the value.
    //

    //
    // Just for testing purposes, let us make up a dummy mac address.
    // In order to avoid conflicts with MAC addresses, it is usually a good
    // idea to check the IEEE OUI list (e.g. at 
    // http://standards.ieee.org/regauth/oui/oui.txt). According to that
    // list 00-50-F2 is owned by Microsoft.
    //
    // An important rule to "generating" MAC addresses is to have the 
    // "locally administered bit" set in the address, which is bit 0x02 for 
    // LSB-type networks like Ethernet. Also make sure to never set the 
    // multicast bit in any MAC address: bit 0x01 in LSB networks.
    //

    pAddr = (PUCHAR) &g_ulAddress;

    ++g_ulAddress;
    Adapter->PermanentAddress[0] = 0x02;
    Adapter->PermanentAddress[1] = 0x50;   
    Adapter->PermanentAddress[2] = 0xF2;   
    Adapter->PermanentAddress[3] = 0x00;    
    Adapter->PermanentAddress[4] = 0x00;
    Adapter->PermanentAddress[5] = pAddr[0];

    ETH_COPY_NETWORK_ADDRESS(
                        Adapter->CurrentAddress,
                        Adapter->PermanentAddress);

    //
    // Read NetworkAddress registry value and use it as the current address 
    // if there is a software configurable NetworkAddress specified in 
    // the registry.
    //
    NdisReadNetworkAddress(
        &Status,
        &NetworkAddress,
        &Length,
        ConfigurationHandle);

    if((Status == NDIS_STATUS_SUCCESS) && (Length == ETH_LENGTH_OF_ADDRESS) )
    {
          if ((ETH_IS_MULTICAST(NetworkAddress) 
                    || ETH_IS_BROADCAST(NetworkAddress))
                    || !ETH_IS_LOCALLY_ADMINISTERED (NetworkAddress))
            {
                DEBUGP(MP_ERROR, 
                    ("Overriding NetworkAddress is invalid - %02x-%02x-%02x-%02x-%02x-%02x\n", 
                    NetworkAddress[0], NetworkAddress[1], NetworkAddress[2],
                    NetworkAddress[3], NetworkAddress[4], NetworkAddress[5]));
            }
            else
            {
                ETH_COPY_NETWORK_ADDRESS(Adapter->CurrentAddress, NetworkAddress);
            }    
    }
    
    DEBUGP(MP_LOUD, ("Permanent Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
        Adapter->PermanentAddress[0],
        Adapter->PermanentAddress[1],
        Adapter->PermanentAddress[2],
        Adapter->PermanentAddress[3],
        Adapter->PermanentAddress[4],
        Adapter->PermanentAddress[5]));

    DEBUGP(MP_LOUD, ("Current Address = %02x-%02x-%02x-%02x-%02x-%02x\n", 
        Adapter->CurrentAddress[0],
        Adapter->CurrentAddress[1],
        Adapter->CurrentAddress[2],
        Adapter->CurrentAddress[3],
        Adapter->CurrentAddress[4],
        Adapter->CurrentAddress[5]));

    Adapter->ulLinkSpeed = NIC_LINK_SPEED;

    //
    // Close the configuration registry
    //
    NdisCloseConfiguration(ConfigurationHandle);
    DEBUGP(MP_TRACE, ("<-- NICReadRegParameters\n"));

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS NICInitializeAdapter(
    IN  PMP_ADAPTER  Adapter,
    IN  NDIS_HANDLE  WrapperConfigurationContext
    )
/*++
Routine Description:

    Query assigned resources and initialize the adapter.

Arguments:

    Adapter     Pointer to our adapter

Return Value:

    NDIS_STATUS_SUCCESS
    NDIS_STATUS_ADAPTER_NOT_FOUND  

--*/    
{

    
    NDIS_STATUS         Status = NDIS_STATUS_ADAPTER_NOT_FOUND;      
    UCHAR               resBuf[NIC_RESOURCE_BUF_SIZE];
    PNDIS_RESOURCE_LIST resList = (PNDIS_RESOURCE_LIST)resBuf;
    UINT                bufSize = NIC_RESOURCE_BUF_SIZE;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResDesc;
    ULONG               index;

    DEBUGP(MP_TRACE, ("---> InitializeAdapter\n"));

    do
    {
        //     
        // Get the resources assigned by the PNP manager. NDIS gets
        // these resources in IRP_MN_START_DEVICE request.
        //
        NdisMQueryAdapterResources(
            &Status, 
            WrapperConfigurationContext, 
            resList, 
            &bufSize);
    
        if (Status == NDIS_STATUS_SUCCESS)
        {
            for (index=0; index < resList->Count; index++)
            {
                pResDesc = &resList->PartialDescriptors[index];

                switch(pResDesc->Type)
                {
                    case CmResourceTypePort:
                        DEBUGP(MP_INFO, ("IoBaseAddress = 0x%x\n", 
                            NdisGetPhysicalAddressLow(pResDesc->u.Port.Start)));
                        DEBUGP(MP_INFO, ("IoRange = x%x\n", 
                                    pResDesc->u.Port.Length));
                        break;

                    case CmResourceTypeInterrupt:                   
                        DEBUGP(MP_INFO, ("InterruptLevel = x%x\n", 
                                            pResDesc->u.Interrupt.Level));
                        break;

                    case CmResourceTypeMemory:
                        DEBUGP(MP_INFO, ("MemPhysAddress(Low) = 0x%0x\n", 
                           NdisGetPhysicalAddressLow(pResDesc->u.Memory.Start)));
                        DEBUGP(MP_INFO, ("MemPhysAddress(High) = 0x%0x\n", 
                            NdisGetPhysicalAddressHigh(pResDesc->u.Memory.Start)));
                        break;
                }
            } 
        }
        
        Status = NDIS_STATUS_SUCCESS;

        //
        // Map bus-relative IO range to system IO space using 
        // NdisMRegisterIoPortRange
        //

        //        
        // Map bus-relative registers to virtual system-space
        // using NdisMMapIoSpace
        //
        

        //
        // Disable interrupts here as soon as possible
        //
                     
        //
        // Register the interrupt using NdisMRegisterInterrupt
        //
        
        //
        // Initialize the hardware with mapped resources
        //
        
#ifdef NDIS50_MINIPORT
        //
        // Register a shutdown handler for NDIS50 or earlier miniports
        // For NDIS51 miniports, set AdapterShutdownHandler.
        //
        NdisMRegisterAdapterShutdownHandler(
            Adapter->AdapterHandle,
            (PVOID) Adapter,
            (ADAPTER_SHUTDOWN_HANDLER) MPShutdown);
#endif         

        //
        // Enable the interrupt
        //
        
    } while (FALSE);
     
    DEBUGP(MP_TRACE, ("<--- InitializeAdapter, Status=%x\n", Status));

    return Status;

}

