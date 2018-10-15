/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       init.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	09/23/96		kyleb		Added support for NdisAllocateMemoryWithTag
	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.
	05/25/97		awang		Removed Registry SlotNumber & BusNumber and
                               corresponded varialbes.
   03/03/97        hhan        Added in 'ScatterGetherDMA'

--*/

#include "precomp.h"
#pragma hdrstop

#define	MODULE_NUMBER	MODULE_INIT

#pragma NDIS_INIT_FUNCTION(DriverEntry)

NTSTATUS
DriverEntry(
   IN  PDRIVER_OBJECT  DriverObject,
   IN  PUNICODE_STRING RegistryPath
   )
/*++

Routine Description:

   Entry point for the driver.

Arguments:

   DriverObject    -   Pointer to the system allocated DRIVER_OBJECT.
   RegistryPath    -   Pointer to the UNICODE string defining the registry
                       path for the driver's information.

Return Value:

--*/
{
   NDIS_STATUS                     Status;
   NDIS_MINIPORT_CHARACTERISTICS   TbAtm155Chars;
   NDIS_HANDLE                     hWrapper;

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>DriverEntry\n"));

   //
   //	Initialize the wrapper.
   //
   NdisMInitializeWrapper(
      &hWrapper,
      DriverObject,
      RegistryPath,
      NULL);
   if (NULL == hWrapper)
   {
      DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
         ("NdisMInitializeWrapper failed!\n"));

      return(NDIS_STATUS_FAILURE);
   }

   ZERO_MEMORY(&TbAtm155Chars, sizeof(TbAtm155Chars));

   //
   //	Initialize the miniport characteristics.
   //
   TbAtm155Chars.MajorNdisVersion = TBATM155_NDIS_MAJOR_VERSION;
   TbAtm155Chars.MinorNdisVersion = TBATM155_NDIS_MINOR_VERSION;
   TbAtm155Chars.CheckForHangHandler = TbAtm155CheckForHang;
   TbAtm155Chars.DisableInterruptHandler = NULL;
   TbAtm155Chars.EnableInterruptHandler = NULL;
   TbAtm155Chars.HaltHandler = TbAtm155Halt;
   TbAtm155Chars.HandleInterruptHandler = TbAtm155HandleInterrupt;
   TbAtm155Chars.InitializeHandler = TbAtm155Initialize;
   TbAtm155Chars.ISRHandler = TbAtm155ISR;
   TbAtm155Chars.ResetHandler = TbAtm155Reset;
   TbAtm155Chars.ReturnPacketHandler = TbAtm155ReturnPacket;
   TbAtm155Chars.AllocateCompleteHandler = TbAtm155AllocateComplete;
   TbAtm155Chars.CoSendPacketsHandler = TbAtm155SendPackets;
   TbAtm155Chars.CoCreateVcHandler = TbAtm155CreateVc;
   TbAtm155Chars.CoDeleteVcHandler = TbAtm155DeleteVc;
   TbAtm155Chars.CoActivateVcHandler = TbAtm155ActivateVc;
   TbAtm155Chars.CoDeactivateVcHandler = TbAtm155DeactivateVc;
   TbAtm155Chars.CoRequestHandler = TbAtm155Request;

   //
   //	Register the miniport with NDIS.
   //
   Status = NdisMRegisterMiniport(
               hWrapper,
               &TbAtm155Chars,
               sizeof(TbAtm155Chars));
   if (NDIS_STATUS_SUCCESS == Status)
   {
       //
       //	Save the handle to the wrapper.
       //	
       gWrapperHandle = hWrapper;
   }

   IF_DBG(DBG_COMP_INIT, DBG_LEVEL_ERR)
   {
       if (NDIS_STATUS_SUCCESS != Status)
      {
       	DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
   	    	("NdisMRegisterMiniport failed! Status: 0x%x\n", Status));
      }
   }

#if (WINVER < 0x0501)
   //
   //   Get the DMA alignment that we need.
   //
   gDmaAlignmentRequired = NdisGetCacheFillSize();
   if (gDmaAlignmentRequired < TBATM155_MIN_DMA_ALIGNMENT)
   {
       gDmaAlignmentRequired = TBATM155_MIN_DMA_ALIGNMENT;
   }
#endif

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
      ("<==DriverEntry\n"));

   return(Status);
}

VOID
tbAtm155InitConfigurationInformation(
   IN  PTBATM155_REGISTRY_PARAMETER    pRegistryParameter
   )
/*++

Routine Description:

   This routine will initialize the configuration information with defaults.
   This is so that we will have something to initialize the driver with
   in case there are no overrides in the registry.

Arguments:

   pRegistryParameter  -   Pointer to the storage for the configuration
                           information.

Return Value:

   None.

--*/
{
   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155InitConfigurationInformation\n"));

   ZERO_MEMORY(
       pRegistryParameter,
       sizeof(TBATM155_REGISTRY_PARAMETER) * TbAtm155MaxRegistryEntry);

   //
   //	Start off with a default hash table of 13.
   //
   pRegistryParameter[TbAtm155VcHashTableSize].Value = 13;
   pRegistryParameter[TbAtm155TotalRxBuffs].Value = DEFAULT_TOTAL_RECEIVE_BUFFERS;
   pRegistryParameter[TbAtm155BigReceiveBufferSize].Value = DEFAULT_RECEIVE_BIG_BUFFER_SIZE;
   pRegistryParameter[TbAtm155SmallReceiveBufferSize].Value = DEFAULT_RECEIVE_SMALL_BUFFER_SIZE;
   pRegistryParameter[TbAtm155NumberOfMapRegisters].Value = MAXIMUM_MAP_REGISTERS;

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
     	("<==tbAtm155InitConfigurationInformation\n"));
}


NDIS_STATUS
tbAtm155ReadConfigurationInformation(
   IN      NDIS_HANDLE                     ConfigurationHandle,
   IN      PTBATM155_REGISTRY_PARAMETER    pRegistryParameter,
   IN  OUT PUCHAR                          CurrentAddress
   )
/*++

Routine Description:

   This routine will read any configuration information that is stored in the
   registry.
   (11/10/97)
   Added a new registry "Network Address"

Arguments:

   ConfgurationHandle  -   NDIS handled used to open/read config information.
   pRegistryParameter  -   Pointer to storage for registry parameters.

Return Value:

   NDIS_STATUS_FAILURE     if we were unable to open the registry information
                           for the adapter.

   NDIS_STATUS_SUCCESS     otherwise.

--*/
{
   NDIS_STATUS						Status;
   NDIS_HANDLE                     ConfigHandle;
   UINT                            c;
   PUCHAR                          NewNetworkAddress;
   UINT                            NewNetworkAddressLength;


   UNREFERENCED_PARAMETER(pRegistryParameter);

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155ReadConfigurationInformation\n"));
   //
   //	Open the configuration section of the registry for this adapter.
   //
   NdisOpenConfiguration(&Status, &ConfigHandle, ConfigurationHandle);
   if (NDIS_STATUS_SUCCESS != Status)
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Unable to open the TbAtm155's Parameters registry key\n"));

       return(Status);
   }

   /*
    *  See if user has defined new MAC address.
    *
    */
   NdisReadNetworkAddress(
       (PNDIS_STATUS) &Status,
       (PVOID *) &NewNetworkAddress,
       &NewNetworkAddressLength,
       ConfigHandle);

   if((Status == NDIS_STATUS_SUCCESS) && (ATM_MAC_ADDRESS_LENGTH == NewNetworkAddressLength ))
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO, ("MAC Address over-ride!"));

       //
       //	Save the network address from registry to StationAddress
       //
       NdisMoveMemory(
           CurrentAddress,
           NewNetworkAddress,
           NewNetworkAddressLength);

#if DBG
       DbgPrint("New MAC Address = %.2x-%.2x-%.2x-%.2x-%.2x-%.2x\n",
           CurrentAddress[0],
           CurrentAddress[1],
           CurrentAddress[2],
           CurrentAddress[3],
           CurrentAddress[4],
           CurrentAddress[5]);
#endif
   }
   else
   {
       // 
       // If fail to get network address from registry, set
       // this station address to be broadcast address to get
       // network address on NIC later.
       // 
       for (c = 0; c < ATM_MAC_ADDRESS_LENGTH; c++)
       {
           CurrentAddress[c] = 0x0ff;
       }
   }

   //
   //	Close the configuration handle.
   //
   NdisCloseConfiguration(ConfigHandle);

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155ReadConfigurationInformation\n"));

   return(NDIS_STATUS_SUCCESS);
}

NDIS_STATUS
tbAtm155ReadPciConfiguration(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  NDIS_HANDLE     ConfigurationHandle
   )
/*++

Routine Description:

   This routine will get the resources that were assigned the adapter.  Verify
   that the adapter is one that we can support and program the PCI slot for
   needed parameters.

Arguments:

   pAdapter            -   Pointer to the adapter to program.
   ConfgurationHandle  -   NDIS handled used to open/read config information.

Return Value:

   NDIS_STATUS_FAILURE     for general failures.
   NDIS_STATUS_ADAPGTER_NOT_FOUND if there is an invalid DeviceID or VendorID.
   NDIS_STATUS_SUCCESS if we have succeeded in getting our resources.
	
--*/
{
   PHARDWARE_INFO                  pHwInfo = pAdapter->HardwareInfo;
   PCM_PARTIAL_RESOURCE_DESCRIPTOR Resource;
   USHORT                          VendorID;
   USHORT                          DeviceID;
   USHORT                          CommandWord;
   NDIS_STATUS                     Status;
   UINT                            c;
   UINT                            Temp;
   UCHAR                           ResourceBuf[ATM_RESOURCE_BUF_SIZE];
   PNDIS_RESOURCE_LIST             ResourceList = (PNDIS_RESOURCE_LIST)ResourceBuf;
   UINT                            ResourceBufSize = ATM_RESOURCE_BUF_SIZE;

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155ReadPciConfiguration\n"));

   //
   //   Query the adapter resources.
   //
   NdisMQueryAdapterResources(
       &Status,
       ConfigurationHandle,
       ResourceList,
       &ResourceBufSize);

   if (NDIS_STATUS_SUCCESS != Status)
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("NdisMQueryAdapterResources() failed: 0x%x\n", Status));

       return(Status);
   }

   //
   //	Walk the resource list to get the adapters configuration
   //	information.
   //
   for (c = 0; c < ResourceList->Count; c++)
   {
       Resource = &ResourceList->PartialDescriptors[c];
       switch (Resource->Type)
       {
           case CmResourceTypeInterrupt:
               //
               //	Save the interrupt number with our adapter block.
               //
               pHwInfo->InterruptLevel = Resource->u.Interrupt.Level;
               pHwInfo->InterruptVector = Resource->u.Interrupt.Vector;

               DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
           	    ("Configured to use interrupt Level: %u interrupt vector: %u\n",
           	        pHwInfo->InterruptLevel,
           	        pHwInfo->InterruptVector));

               break;

           case CmResourceTypeMemory:
           
               //
               //	Save the memory mapped base physical address and it's length.
               //
               pHwInfo->PhysicalIoSpace = Resource->u.Memory.Start;
               pHwInfo->IoSpaceLength = Resource->u.Memory.Length;

               DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
                   ("Config to use IO space to mapped mem 0x%x:0x%x of length 0x%x\n",
                       NdisGetPhysicalAddressHigh(pHwInfo->PhysicalIoSpace),
                       NdisGetPhysicalAddressLow(pHwInfo->PhysicalIoSpace),
                       pHwInfo->IoSpaceLength));

               break;

           case CmResourceTypePort:

               //
               //	Save the port.
               //
               pHwInfo->InitialPort = NdisGetPhysicalAddressLow(Resource->u.Port.Start);
               pHwInfo->NumberOfPorts = Resource->u.Port.Length;

               DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
           	    ("Configured to use port memory 0x%x of length 0x%x\n",
           		    pHwInfo->InitialPort,
           		    pHwInfo->NumberOfPorts));

               break;
       }
   }

   //
   //	Read the VendorID.
   //
   Temp = NdisReadPciSlotInformation(
               pAdapter->MiniportAdapterHandle,
               0,      // NDIS knows the real slot number
               FIELD_OFFSET(PCI_COMMON_CONFIG, VendorID),
               &VendorID,
               sizeof(VendorID));
   if (Temp != sizeof(VendorID))
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Unable to read the VendorID\n"));

       return(NDIS_STATUS_FAILURE);
   }

   //
   //	Verify vendor ID.
   //
   if (TOSHIBA_PCI_VENDOR_ID != VendorID)
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Invalid PCI Vendor ID: 0x%x\n", VendorID));

       return(NDIS_STATUS_ADAPTER_NOT_FOUND);
   }

   //
   //	Read the DeviceID.
   //
   Temp = NdisReadPciSlotInformation(
               pAdapter->MiniportAdapterHandle,
               0,      // NDIS knows the real slot number
               FIELD_OFFSET(PCI_COMMON_CONFIG, DeviceID),
               &DeviceID,
               sizeof(DeviceID));
   if (Temp != sizeof(DeviceID))
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Unable to read the DeviceID\n"));

       return(NDIS_STATUS_FAILURE);
   }

   //
   //  Make sure we recognize the device ID.
   //
   if (TBATM155_PCI_DEVICE_ID != DeviceID)
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Invalid PCI Device ID: 0x%x\n", DeviceID));

       return(NDIS_STATUS_ADAPTER_NOT_FOUND);
   }

   //
   //	Read the Command word in from PCI COMMON Config space.
   //
   Temp = NdisReadPciSlotInformation(
               pAdapter->MiniportAdapterHandle,
               0,      // NDIS knows the real slot number
               FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
               &CommandWord,
               sizeof(CommandWord));
   if (Temp != sizeof(CommandWord))
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Unable to read the CommandWord\n"));

       return(NDIS_STATUS_FAILURE);
   }

   //
   //	Does the command word meet the following requirements?
   //
   if (((CommandWord & PCI_ENABLE_IO_SPACE) != PCI_ENABLE_IO_SPACE) ||
       ((CommandWord & PCI_ENABLE_MEMORY_SPACE) != PCI_ENABLE_MEMORY_SPACE) ||
       ((CommandWord & PCI_ENABLE_BUS_MASTER) != PCI_ENABLE_BUS_MASTER))
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Invalid PCI Command Word: 0x%x\n", CommandWord));

       CommandWord |= PCI_ENABLE_IO_SPACE |
                      PCI_ENABLE_MEMORY_SPACE |
                      PCI_ENABLE_BUS_MASTER;

       Temp = NdisWritePciSlotInformation(
                   pAdapter->MiniportAdapterHandle,
                   0,      // NDIS knows the real slot number
                   FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                   &CommandWord,
                   sizeof(CommandWord));
       if (Temp != sizeof(CommandWord))
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Unable to write the PCI command register\n"));

           return(NDIS_STATUS_FAILURE);
       }

       Temp = NdisReadPciSlotInformation(
                   pAdapter->MiniportAdapterHandle,
                   0,      // NDIS knows the real slot number
                   FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
                   &CommandWord,
                   sizeof(CommandWord));
       if (Temp != sizeof(CommandWord))
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Unable to read the CommandWord\n"));

           return(NDIS_STATUS_FAILURE);
       }

       if (((CommandWord & PCI_ENABLE_IO_SPACE) != PCI_ENABLE_IO_SPACE) ||
           ((CommandWord & PCI_ENABLE_MEMORY_SPACE) != PCI_ENABLE_MEMORY_SPACE) ||
           ((CommandWord & PCI_ENABLE_BUS_MASTER) != PCI_ENABLE_BUS_MASTER))
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Invalid command word: 0x%x\n", CommandWord));

           return(NDIS_STATUS_FAILURE);
       }

       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Successfully wrote command word: 0x%x\n", CommandWord));
   }

   //
   //	For noisy debug dump what we find in the PCI space.
   //
   dbgDumpPciCommonConfig(pAdapter);

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155ReadPciConfiguration\n"));

   return(NDIS_STATUS_SUCCESS);

}


VOID
tbAtm155SoftresetNIC(
   IN  PHARDWARE_INFO  pHwInfo
   )
/*++

Routine Description:

   This routine issues a soft reset to the NIC.

Arguments:

   pHwInfo     -   Pointer to the HARDWARE_INFORMATION block that describes the nic.

Return Value:

--*/
{
   TB155PCISAR_CNTRL2      regControl2;

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155SoftresetNIC(\n"));


   TBATM155_READ_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl2,
       &regControl2.reg);

   regControl2.Soft_Reset = 1;     // set the soft reset bit

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl2,
       regControl2.reg);

   NdisStallExecution(1);          // let's give a 1-ms-delay.

   TBATM155_READ_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl2,
       &regControl2.reg);

   if (regControl2.Soft_Reset != 1)
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Soft_reset bit is not set\n"));
   }

   regControl2.Soft_Reset = 0;     // we need to clear the bit.

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl2,
       regControl2.reg);

   TBATM155_READ_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl2,
       &regControl2.reg);

   if (regControl2.Soft_Reset != 0)
   {
       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
           ("Soft_reset bit is not cleared\n"));
   }

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155SoftresetNIC(\n"));

}


NDIS_STATUS
tbAtm155GetHardwareInformation(
   IN	PADAPTER_BLOCK	pAdapter
   )
/*++

Routine Description:

   This routine will set the hardware information including reading
   in the manufacturer address for NIC and setting the
   offsets for the other information that will be needed.

Arguments:

   pAdapter	-	Pointer to the adapter block to save the information.

Return Value:

   NDIS_STATUS_SUCCESS     if everything went ok.
   NDIS_STATUS_FAILURE     otherwise.

--*/
{
   NDIS_STATUS             Status;
   PHARDWARE_INFO          pHwInfo;
   UINT                    c;
   BOOLEAN                 fCopyNetworkAddress = TRUE;


   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155GetHardwareInformation\n"));


   do
   {
       //
       //	Initialize the hardware info.
       //
       pHwInfo = pAdapter->HardwareInfo;

       //
       //	Map the chunk of memory that was given to us.
       //
       Status = NdisMMapIoSpace(
                   &pHwInfo->VirtualIoSpace,
                   pAdapter->MiniportAdapterHandle,
                   pHwInfo->PhysicalIoSpace,
                   pHwInfo->IoSpaceLength);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Unable to map needed memory space\n"));

           break;
       }

       //
       //	Set SAR offset
       //  (if use this VirtualIoSpace, the function NdisReadRegisterUlong
       //   will be used).
       //
       pHwInfo->TbAtm155_SAR = (PTBATM155_SAR)(pHwInfo->VirtualIoSpace);

       //
       //	Set PHY offset
       //
       pHwInfo->Phy = (ULONG)PHY_DEVICE_OFFSET;

       // 
       // Must soft reset the NIC before access serial EEPROM & PHY regiters
       // 
       tbAtm155SoftresetNIC(pHwInfo);

       //
       //	Read in the manufacturer address for the nic.
       //
       Status = tbAtm155Read_PermanentAddress(
                   pAdapter,
                   pHwInfo->PermanentAddress);

       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
               ("Read Node address failed! Status: 0x%x\n", Status));
           break;
       }

       for (fCopyNetworkAddress = TRUE, c = 0; c < ATM_MAC_ADDRESS_LENGTH; c++)
       {
           if (pHwInfo->StationAddress[c] != 0x0ff)
           {
               fCopyNetworkAddress = FALSE;
               break;
           }
       }


       if (fCopyNetworkAddress == TRUE)
       {
           //
           //	Save the permanent address in the station address by default.
           //
           NdisMoveMemory(
               pHwInfo->StationAddress,
               pHwInfo->PermanentAddress,
               ATM_MAC_ADDRESS_LENGTH);
       }

       dbgDumpHardwareInformation(pHwInfo);

   } while (FALSE);
   
   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155GetHardwareInformation\n"));

   return(Status);
}

VOID
tbAtm155FreeResources(
   IN PADAPTER_BLOCK pAdapter
   )
/*++

Routine Description:

   This routine will free the resources that were allocated for an adapter.

Arguments:

   pAdapter    -   Pointer to the adapter block whose resources we need to
                   free.

Return Value:

--*/
{
   PHARDWARE_INFO      pHwInfo;
   PSAR_INFO           pSar;
   PTX_REPORT_QUEUE    pTxReportQ;    
   PRX_REPORT_QUEUE    pRxReportQ;    


   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155FreeResources\n"));

   if (NULL != pAdapter)
   {
       if (NULL != pAdapter->RegistryParameters)
       {

           FREE_MEMORY(
               pAdapter->RegistryParameters,
               sizeof(TBATM155_REGISTRY_PARAMETER) * TbAtm155MaxRegistryEntry);
       }

       if (NULL != pAdapter->HardwareInfo)
       {
           pHwInfo = pAdapter->HardwareInfo;

           if (HW_TEST_FLAG(pHwInfo, fHARDWARE_INFO_INTERRUPT_REGISTERED))
           {
               NdisMDeregisterInterrupt(&pHwInfo->Interrupt);
           }

           if (NULL != pHwInfo->SarInfo)
           {
               pSar = pHwInfo->SarInfo;

               //
               //	Free up the transmit and receive DMA resources
               //
               tbAtm155FreeReceiveBufferQueue(pAdapter, &pSar->RecvDmaQ.CompletingSmallBufQ);
               tbAtm155FreeReceiveBufferQueue(pAdapter, &pSar->RecvDmaQ.CompletingBigBufQ);

               tbAtm155FreeReceiveBufferQueue(pAdapter, &pSar->FreeSmallBufferQ);
               tbAtm155FreeReceiveBufferQueue(pAdapter, &pSar->FreeBigBufferQ);

               NdisFreeSpinLock(&pSar->RecvDmaQ.lock);


               tbAtm155FreePacketQueue(&pSar->XmitDmaQ.DmaWait);

               NdisFreeSpinLock(&pSar->XmitDmaQ.lock);

               //
               //	Free the spin lock used to protect the free list of
               //	transmit segments.
               //
               NdisFreeSpinLock(&pSar->lockFreeXmitSegment);

               //
               //	Free up the memory.
               //
               FREE_MEMORY(pSar, sizeof(SAR_INFO));
           }

           if (NULL != pHwInfo->PortOffset)
           {
               NdisMDeregisterIoPortRange(
                   pAdapter->MiniportAdapterHandle,
                   pHwInfo->InitialPort,
                   pHwInfo->NumberOfPorts,
                   pHwInfo->PortOffset);
           }
		
           if (NULL != pHwInfo->VirtualIoSpace)
           {
               NdisMUnmapIoSpace(
                   pAdapter->MiniportAdapterHandle,
                   pHwInfo->VirtualIoSpace,
                   pHwInfo->IoSpaceLength);
           }
		
           if (NULL != pHwInfo->pSramAddrTbl)
           {
               if (pHwInfo->fAdapterHw & TBATM155_METEOR_4KVC)
               {
                   FREE_MEMORY(
                       pHwInfo->pSramAddrTbl, 
                       sizeof(SRAM_4K_VC_MODE));
               }
               else
               {
                   FREE_MEMORY(
                       pHwInfo->pSramAddrTbl, 
                       sizeof(SRAM_1K_VC_MODE));
               }
           }

           //
           //	Free the spin lock for the hardware information.
           //
           NdisFreeSpinLock(&pHwInfo->Lock);

           //
           //	Free the memory used for the hardware information.
           //
           FREE_MEMORY(pHwInfo, sizeof(HARDWARE_INFO));
       }

       pRxReportQ = &pAdapter->RxReportQueue;

       if (NULL != pRxReportQ->VirtualAddress)
       {
           //
           // free up the receive report queue.
           //
           NdisMFreeSharedMemory(
               pAdapter->MiniportAdapterHandle,
               pRxReportQ->Size,
               FALSE,
               pRxReportQ->VirtualAddress,
               pRxReportQ->PhysicalAddress);
       }

       pTxReportQ = &pAdapter->TxReportQueue;

       if (NULL != pTxReportQ->VirtualAddress)
       {
           //
           // free up the transmit report queue.
           //
           NdisMFreeSharedMemory(
               pAdapter->MiniportAdapterHandle,
               pTxReportQ->Size,
               FALSE,
               pTxReportQ->VirtualAddress,
               pTxReportQ->PhysicalAddress);

       }

       dbgFreeDebugInformation(pAdapter->DebugInfo);

       ASSERT(IsListEmpty(&pAdapter->ActiveVcList));
       ASSERT(IsListEmpty(&pAdapter->InactiveVcList));

       //
       //	Free up the spin lock for the adapter block.
       //
       NdisFreeSpinLock(&pAdapter->lock);
       NdisFreeSpinLock(&pAdapter->VcHashLock);

       //
       //	Free the memory allocated for the adapter block.
       //
       FREE_MEMORY(pAdapter, sizeof(ADAPTER_BLOCK));

   }

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155FreeResources\n"));
}


NDIS_STATUS
tbAtm155AllocateReportQueues(
   IN  PADAPTER_BLOCK  pAdapter
	)
/*++

Routine Description:

   This routine will allocate receive & transmit report queues.

Arguments:

   pAdapter    -   Pointer to the ADAPTER_BLOCK that describes the nic.

Return Value:

   NDIS_STATUS_SUCCESS     - if allocate report queues OK.
   NDIS_STATUS_RESOURCES   - if allocate report queues failed.

--*/
{
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   PRX_REPORT_QUEUE    pRxReportQ = &pAdapter->RxReportQueue;
   PTX_REPORT_QUEUE    pTxReportQ = &pAdapter->TxReportQueue;
   ULONG               Offset;
   ULONG               DmaAlignmentRequired;
   PUCHAR              StartVa;


	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
		("==>tbAtm155AllocateReportQueues\n"));


   do {

       //
       // Allocate a Transmit report queue
	    //
	    //	- Get the DMA alignment that we need.
	    //

#if (WINVER < 0x0501)
       DmaAlignmentRequired = NdisGetCacheFillSize();
#else
       DmaAlignmentRequired = gDmaAlignmentRequired;
#endif
       if (DmaAlignmentRequired < TBATM155_MIN_QUEUE_ALIGNMENT)
	    {
		    DmaAlignmentRequired = TBATM155_MIN_QUEUE_ALIGNMENT;
	    }

       //
       //  - Calculate the total size of the Tx queue.
       //
       pTxReportQ->Size = 
           ((sizeof(TX_REPORT_QUEUE_ENTRY) * (pHwInfo->MaxIdxOfTxReportQ+1))
            + DmaAlignmentRequired);

       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
           ("pTxReportQ->Size = 0x%lx, size of TX_REPORT_QUEUE_ENTRY (%d)\n",
            pTxReportQ->Size, sizeof(TX_REPORT_QUEUE_ENTRY)));

       NdisMAllocateSharedMemory(
           pAdapter->MiniportAdapterHandle,
           pTxReportQ->Size,
           FALSE,
           &pTxReportQ->VirtualAddress,
           &pTxReportQ->PhysicalAddress);

	    if ((NULL == pTxReportQ->VirtualAddress) ||
			 (0 != pTxReportQ->PhysicalAddress.HighPart))
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Unable to allocate the tx report queue\n"));
           Status = NDIS_STATUS_RESOURCES;
           break;
       }

       //
       //  - Set the aligned Virtual address and Physical address.
       //
       Offset  = DmaAlignmentRequired -
                   (ULONG)((ULONG_PTR)pTxReportQ->VirtualAddress % DmaAlignmentRequired);
       StartVa = ((PUCHAR)pTxReportQ->VirtualAddress + Offset);
       pTxReportQ->TxReportQptrVa = (PTX_REPORT_QUEUE_ENTRY)StartVa;

       NdisSetPhysicalAddressLow(
           pTxReportQ->TxReportQptrPa,
           NdisGetPhysicalAddressLow(pTxReportQ->PhysicalAddress) + Offset);

       //
       //  - Calculate the total size of the Rx queue.
       //
       pRxReportQ->Size = 
           ((sizeof(RX_REPORT_QUEUE_ENTRY) * (pHwInfo->MaxIdxOfRxReportQ+1))
            + DmaAlignmentRequired);

       DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
           ("pRxReportQ->Size = 0x%ld, size of RX_REPORT_QUEUE_ENTRY (%d)\n",
            pRxReportQ->Size, sizeof(RX_REPORT_QUEUE_ENTRY)));

       NdisMAllocateSharedMemory(
           pAdapter->MiniportAdapterHandle,
           pRxReportQ->Size,
           FALSE,
           &pRxReportQ->VirtualAddress,
           &pRxReportQ->PhysicalAddress);

       if ((NULL == pRxReportQ->VirtualAddress) ||
			 (0 != pRxReportQ->PhysicalAddress.HighPart))
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Unable to allocate the rx report queue\n"));
           Status = NDIS_STATUS_RESOURCES;
           break;
       }

       //
       //  - Set the aligned Virtual address and Physical address.
       //
   	Offset  = DmaAlignmentRequired -
				  (ULONG)((ULONG_PTR)pRxReportQ->VirtualAddress % DmaAlignmentRequired);
       StartVa = ((PUCHAR)pRxReportQ->VirtualAddress + Offset);
       pRxReportQ->RxReportQptrVa = (PRX_REPORT_QUEUE_ENTRY)StartVa;

       NdisSetPhysicalAddressLow(
           pRxReportQ->RxReportQptrPa,
           NdisGetPhysicalAddressLow(pRxReportQ->PhysicalAddress) + Offset);

       //
       // Init pHwInfo->PrevTxReportPa and pHwInfo->PrevRxReportPa
       // to the beginning of Physical addresses of Tx/Rx report queues.
       //
       pHwInfo->PrevTxReportPa = NdisGetPhysicalAddressLow(pTxReportQ->TxReportQptrPa);
       pHwInfo->PrevRxReportPa = NdisGetPhysicalAddressLow(pRxReportQ->RxReportQptrPa);

   } while (FALSE);


   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155AllocateReportQueues\n"));

	return(Status);

}


VOID
tbAtm155SetBufSizesOfCtrl2Reg(
   IN      PADAPTER_BLOCK          pAdapter,
   OUT     PTB155PCISAR_CNTRL2     pRegControl2
   )
/*++

Routine Description:

   This routine will get buffer sizes for Control 2 regiter of Meteor
   based on the buffer sizes set in registrys.

Arguments:

   pAdapter     -   Pointer to the ADAPTER_BLOCK that describes the nic.
   pRegControl2 -   Pointer to the Control2 of Meteor SAR registers

Return Value:

--*/
{
   PTBATM155_REGISTRY_PARAMETER    pRegistryParameter = pAdapter->RegistryParameters;
   ULONG                           tmp;

   //
   // Get Small buffer sizes first.
   //
   tmp = pRegistryParameter[TbAtm155SmallReceiveBufferSize].Value / sizeof(ULONG);
   if (tmp < TBATM155_CTRL2_MIN_SMALL_BSIZE)
   {
       tmp = TBATM155_CTRL2_MIN_SMALL_BSIZE;
   }
   else if (tmp > TBATM155_CTRL2_MAX_SMALL_BSIZE)
   {
       tmp = TBATM155_CTRL2_MAX_SMALL_BSIZE;
   }

   if (tmp == TBATM155_CTRL2_MAX_SMALL_BSIZE)
   {
       // 
       // According to spec. 512lw is specified by programming 
       // this field with value 0.
       // 
       tmp = 0;
   }

   (*pRegControl2).Small_Slot_Size = tmp;

   //
   // Get Big buffer sizes now.
   //
   tmp = pRegistryParameter[TbAtm155BigReceiveBufferSize].Value;
   if ((tmp > 0) && (tmp <= BLOCK_1K))
   {
       tmp = 0;
   }
   else if ((tmp > BLOCK_1K) && (tmp <= BLOCK_2K))
   {
       tmp = 1;
   }
   else if ((tmp > BLOCK_2K) && (tmp <= BLOCK_4K))
   {
       tmp = 2;
   }
   else if ((tmp > BLOCK_4K) && (tmp <= BLOCK_8K))
   {
       tmp = 3;
   }
   else if ((tmp > BLOCK_8K) && (tmp <= BLOCK_10K))
   {
       tmp = 4;
   }
   else if ((tmp > BLOCK_10K) && (tmp <= BLOCK_16K))
   {
       tmp = 5;
   }
   else
   {
       tmp = 5;
   }

   (*pRegControl2).Big_Slot_Size = tmp;

}


NDIS_STATUS
tbAtm155InitRegisters(
   IN  PADAPTER_BLOCK  pAdapter
   )
/*++

Routine Description:

   This routine will initialize 
       1. receive & transmit report queues.
       2. the CSR registers on the NIC.

Arguments:

   pAdapter    -   Pointer to the ADAPTER_BLOCK that describes the nic.

Return Value:

   NDIS_STATUS_SUCCESS     -   initialize registers successfully.
   NDIS_STATUS_FAILURE     -   failed to initialize the registers.

--*/
{
   PHARDWARE_INFO          pHwInfo = pAdapter->HardwareInfo;
   PRX_REPORT_QUEUE        pRxReportQ = &pAdapter->RxReportQueue;
   PTX_REPORT_QUEUE        pTxReportQ = &pAdapter->TxReportQueue;
   PRX_REPORT_QUEUE_ENTRY  Offset_Rx;
   PTX_REPORT_QUEUE_ENTRY  Offset_Tx;
   TB155PCISAR_CNTRL1      regControl1, regControl1_set;
   TB155PCISAR_CNTRL2      regControl2;
   TX_FS_LIST_PTRS         regTxFsListPtrs;
   TX_ABR_NRM_TRM          regCTxAbrNrmTrm;
   INTR_ENB_REGISTER       regIntr;
   NDIS_STATUS			    Status = NDIS_STATUS_SUCCESS;
   UINT                    i;
   INTR_HLDOFF_REG         regIntrHldoff;


   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155InitRegisters\n"));

   //
   //  Let's initialize SRAM.
   //

   if (pHwInfo->fAdapterHw & TBATM155_METEOR_4KVC)
   {
       Status = tbAtm155InitSRAM_4KVCs(pAdapter);
   }
   else
   {
       Status = tbAtm155InitSRAM_1KVCs(pAdapter);
   }

   if (NDIS_STATUS_SUCCESS != Status)
   {
       DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
           ("tbAtm155InitSRAM failed! Status: 0x%x\n", Status));
       return(Status);
   }

   // 
   // Soft reset the NIC one more time here.
   // 
   tbAtm155SoftresetNIC(pHwInfo);

	//
	//	Program the 155 PCI SAR Control1 register.
	//
   regControl1.reg = 0;
   regControl1.Phy_Cell_Mode_Enb = 1;

   // 
   //  If we are in the middle of the reset, don't start transmitter
   //  and receiver now.
   // 
   if (!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS))
   {
       regControl1.Rx_Enb = 1;
       regControl1.Tx_Enb = 1;
   }

   if (pHwInfo->fAdapterHw & TBATM155_METEOR_4KVC)
   {
       regControl1.VC_Mode_Sel = 3;            // 4K VCs supported.
       regControl1.PCI_MRM_Sel = 1;
   }
   else
   {
       regControl1.VC_Mode_Sel = 1;            // 1K VCs supported.
   }

   regControl1_set = regControl1;

   /* Clear Phy_IF_Reset & Set Cell_mode */
   regControl1.reg = (regControl1_set.reg & 0xfffffc00) | 0x080;;

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
       regControl1.reg);

   //
   //  Initialize PHY Controller based on the detection
   //
   if (pHwInfo->fAdapterHw & TBATM155_PHY_SUNI_LITE)
   {
       Status = tbAtm155InitSuniLitePhyRegisters(pAdapter);
   }
   else
   {
       Status = tbAtm155InitPLC2PhyRegisters(pAdapter);
   }

   if (!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS))
   {
       //
       //	Initialize other common variables.
       //
       pHwInfo->CellClockRate = TBATM155_CELL_CLOCK_RATE;
       pAdapter->MaximumBandwidth = ATM_155 / CELL_SIZE;
       pAdapter->MinimumCbrBandwidth = TBATM155_MIN_CBR_BANDWIDTH;
       pAdapter->MinimumAbrBandwidth = TBATM155_MIN_ABR_BANDWIDTH;

#if    AW_QOS

       //
       //  The Number of available CBR VCs
       //  9/17/97     - Please read the note where declares this
       //                variable (in SW.H).
       //	
       pAdapter->NumOfAvailableCbrVc = MAX_SUPPORTED_CBR_VC;

#endif // end of AW_QOS


       pAdapter->NumOfAvailVc_B = (pHwInfo->SarInfo->MaxRecvBufferCount / 3);
//       pAdapter->NumOfAvailVc_B = pHwInfo->SarInfo->MaxRecvBufferCount;
       pAdapter->NumOfAvailVc_S = pAdapter->NumOfAvailVc_B;

   }

   pAdapter->RemainingTransmitBandwidth = pAdapter->MaximumBandwidth;

   if (NDIS_STATUS_SUCCESS != Status)
   {
       DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_ERR,
           ("Failed to initialize the PHY registers\n"));
   }

   //
   // 1. initialize Transmit report queue.
   // 2. initialize Tx_Report_Ptr register also.
   //
   NdisZeroMemory (
       (PVOID)(pTxReportQ->VirtualAddress),
       (ULONG)(pTxReportQ->Size));
   
   for (Offset_Tx = pTxReportQ->TxReportQptrVa, i = 0;
        i <= pHwInfo->MaxIdxOfTxReportQ;
        Offset_Tx++, i++)
   {
       Offset_Tx->TxReportQDWord.Own = 1;      // set owned by 155 PCI SAR
   }

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Tx_Report_Base,
       NdisGetPhysicalAddressLow(pTxReportQ->TxReportQptrPa));


   //
   // 1. initialize Receive report queue.
   // 2. initialize Rx_Report_Ptr register.
   //
   NdisZeroMemory (
       (PVOID)(pRxReportQ->VirtualAddress),
       (ULONG)(pRxReportQ->Size));
   
   for (Offset_Rx = pRxReportQ->RxReportQptrVa, i = 0;
        i <= pHwInfo->MaxIdxOfRxReportQ;
        Offset_Rx++, i++)
   {
       Offset_Rx->RxReportQDWord0.Own = 1;     // set owned by 155 PCI SAR
   }

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Rx_Report_Base,
       NdisGetPhysicalAddressLow(pRxReportQ->RxReportQptrPa));

	//
	//	Program the 155 PCI SAR Control2 register.
	//
   regControl2.reg = 0;
   tbAtm155SetBufSizesOfCtrl2Reg(pAdapter, &regControl2);

	//
   //  Set Reassembly timeout time.
	//
   regControl2.Rx_RATO_Time = 15;      // Range is 2-15.
   
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl2,
       regControl2.reg);

	//
	//	Program the Tx_ABR_Nrm_Trm register which is used to store
   //  2 sets of Nrm & Trm values for each of the 2 ABR parameter base
   //  sets supported by 155 PCI SAR.
   //  This register should be written at initialization time only.
   //  Line Rate = 353,208 cells/s
   //  Nrm (Base 1 & 2) = 32   
   //  Trm (Base 1 & 2) = 100 ms
	//
   regCTxAbrNrmTrm.reg = 0;
   regCTxAbrNrmTrm.Line_Rate_Exp = 0x12;

//=================================
#if    ABR_CODE
//=================================

   regCTxAbrNrmTrm.Trm_1 = TBATM155_TRM_100MS;
   regCTxAbrNrmTrm.Nrm_1 = TBATM155_NRM_32CELLS;
   regCTxAbrNrmTrm.Trm_2 = TBATM155_TRM_100MS;
   regCTxAbrNrmTrm.Nrm_2 = TBATM155_NRM_32CELLS;

//=================================
#endif     // end of ABR_CODE
//=================================
   
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Tx_ABR_Nrm_Trm,
       regCTxAbrNrmTrm.reg);


//=================================
#if    ABR_CODE
//=================================

	//
	//	Program the Tx_ABR_ADTF register which is used to store
   //  2 sets of ADTF (ACR Decay Timer Factor) values.
   //  This register should be written at initialization time only.
	//
   //  ADTF (Base 1 & 2) = 500 ms
	//
   regTxAbrADTF.reg = 0;
   regTxAbrADTF.ADTF_1 = TBATM155_ADTF_500MS;
   regTxAbrADTF.ADTF_2 = TBATM155_ADTF_500MS;
   
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Tx_ABR_ADTF,
       regTxAbrADTF.reg);

//=================================
#endif     // end of ABR_CODE
//=================================


   // 
   // Hold Off the number of interrupts of TX/RX IOCs.
   // Holdoff IOC count of TX/RX = 10
   // Holdoff IOC time of TX/RX = 24ms
   // 
   regIntrHldoff.reg = 0;
   regIntrHldoff.Rx_IOC_Hldoff_Wr = 1;
   regIntrHldoff.Tx_IOC_Hldoff_Wr = 1;
   
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Intr_Hldoff,
       regIntrHldoff.reg);

   regIntrHldoff.Rx_IOC_Hldoff_Cnt  = RX_IOC_HLDOFF_CNT;
   regIntrHldoff.Rx_IOC_Hldoff_Time = RX_IOC_HLDOFF_TIME;
   
   regIntrHldoff.Tx_IOC_Hldoff_Cnt  = TX_IOC_HLDOFF_CNT;
   regIntrHldoff.Tx_IOC_Hldoff_Time = TX_IOC_HLDOFF_TIME;
   
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Intr_Hldoff,
       regIntrHldoff.reg);

   regIntrHldoff.Rx_IOC_Hldoff_Wr = 0;
   regIntrHldoff.Tx_IOC_Hldoff_Wr = 0;
   
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Intr_Hldoff,
       regIntrHldoff.reg);


	//
	//	Program the 155 PCI SAR Control1 register.
	//
   regTxFsListPtrs.reg = 0;
   regTxFsListPtrs.Tx_FS_List_Valid = 1;
   regTxFsListPtrs.Tx_FS_List_Tail = pHwInfo->MaxIdxOfTxReportQ;
   
   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Tx_FS_List_ptrs,
       regTxFsListPtrs.reg);

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->SAR_Cntrl1,
       regControl1_set.reg);

	//
	//  Clear Interrupt Status register first
	//
   TBATM155_READ_PORT(
       &pHwInfo->TbAtm155_SAR->Intr_Status,
       &regIntr.reg);

   TBATM155_WRITE_PORT(
       &pHwInfo->TbAtm155_SAR->Intr_Status,
       regIntr.reg);

	//
	//  Set Interrupt Enable register but enable later.
	//
	pHwInfo->InterruptMask = TBATM155_REG_INT_VALID;

	DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
		("<==tbAtm155InitRegisters\n"));

	return(Status);
}


VOID
tbAtm155InitSarParameters(
   IN  PADAPTER_BLOCK  pAdapter
	)
/*++
Routine Description:

   This routine will initialize the SAR data parameters.

Arguments:

   pAdapter    -   Pointer to the adapter block.

Return Value:

--*/
{
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   PSAR_INFO           pSar = pHwInfo->SarInfo;

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155InitSarParameters\n"));

   // 
   // Initialize the variables of keeping track of Tx & Rx slots.
   // 
   pSar->XmitDmaQ.MaximumTransmitSlots     = pHwInfo->NumOfVCs;
   pSar->RecvDmaQ.MaximumReceiveBigSlots   = pHwInfo->NumOfVCs;
   pSar->RecvDmaQ.MaximumReceiveSmallSlots = pHwInfo->NumOfVCs;

   pSar->XmitDmaQ.RemainingTransmitSlots   = pHwInfo->NumOfVCs;
   pSar->XmitDmaQ.PrevTxReportQIndex       = 0;

#if    DBG
       pSar->XmitDmaQ.dbgTotalPostedTxSlots    = 0;
#endif // end of DBG

   pSar->RecvDmaQ.RemainingReceiveBigSlots   = pHwInfo->NumOfVCs;
   pSar->RecvDmaQ.RemainingReceiveSmallSlots = pHwInfo->NumOfVCs;

   pSar->RecvDmaQ.CurrentSlotTagOfBigBufQ    = 1;
   pSar->RecvDmaQ.CurrentSlotTagOfSmallBufQ  = 1;
       
   pSar->RecvDmaQ.PrevRxReportQIndex    = 0;

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155InitSarParameters\n"));

}


NDIS_STATUS
tbAtm155InitSarSegment(
   IN  PADAPTER_BLOCK  pAdapter
	)
/*++

Routine Description:

   This routine will initialize the SAR data structure and the drivers state
   information for the SAR segment.

Arguments:

   pAdapter    -   Pointer to the adapter block.

Return Value:

   NDIS_STATUS_SUCCESS     if we have successfully initialzied the SAR.
   NDIS_STATUS_RESOURCES   if we are unable to allocate necessary resources.

--*/
{
   NDIS_STATUS         Status = NDIS_STATUS_SUCCESS;
   PSAR_INFO           pSar;
   PHARDWARE_INFO      pHwInfo = pAdapter->HardwareInfo;
   PTBATM155_REGISTRY_PARAMETER    pRegistryParameter;


   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("==>tbAtm155InitSarSegment\n"));

   do
   {
       //
       //	Allocate memory for the sar.
       //
       ALLOCATE_MEMORY(&Status, &pSar, sizeof(SAR_INFO), '20DA');
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Unable to allocate information for the SAR_INFO\n"));

           break;
       }

       ZERO_MEMORY(pSar, sizeof(SAR_INFO));
	
       NdisAllocateSpinLock(&pSar->lockFreeXmitSegment);
	

       //
       //	Initialize the transmit DMA queue.
       //
       tbAtm155InitializePacketQueue(&pSar->XmitDmaQ.DmaWait);

       NdisAllocateSpinLock(&pSar->XmitDmaQ.lock);

		Status = NdisMInitializeScatterGatherDma(
					pAdapter->MiniportAdapterHandle,
					FALSE,
                   BLOCK_15K);

		if (NDIS_STATUS_SUCCESS != Status)
		{
			DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
				("Unable to allocate scatter gather list\n"));

			break;
		}

#if (WINVER >= 0x0501)
       //
       //   Get the DMA alignment that we need.
       //
       gDmaAlignmentRequired = NdisMGetDmaAlignment(pAdapter->MiniportAdapterHandle);
       if (gDmaAlignmentRequired < TBATM155_MIN_DMA_ALIGNMENT)
       {
           gDmaAlignmentRequired = TBATM155_MIN_DMA_ALIGNMENT;
       }
#endif

       //
       //	Initialize the receive DMA queue.
       //
       NdisAllocateSpinLock(&pSar->RecvDmaQ.lock);

       //
       //	Initialize the waiting receive buffer queues.
       //
       tbAtm155InitializeReceiveBufferQueue(&pSar->RecvDmaQ.CompletingSmallBufQ);
       tbAtm155InitializeReceiveBufferQueue(&pSar->RecvDmaQ.CompletingBigBufQ);

       //
       //	Initialize the free receive buffer queues.
       //
       tbAtm155InitializeReceiveBufferQueue(&pSar->FreeSmallBufferQ);
       tbAtm155InitializeReceiveBufferQueue(&pSar->FreeBigBufferQ);

       pRegistryParameter = pAdapter->RegistryParameters;
       pSar->MaxRecvBufferCount = (USHORT)pRegistryParameter[TbAtm155TotalRxBuffs].Value;

       pHwInfo->SarInfo = pSar;
       tbAtm155InitSarParameters(pAdapter);    
       
       Status = NDIS_STATUS_SUCCESS;
   } while (FALSE);

   //
   //	If we failed somewhere above then we need to cleanup....
   //
   if (NDIS_STATUS_SUCCESS != Status)
   {

       if (NULL != pSar)
       {
           NdisFreeSpinLock(&pSar->lockFreeXmitSegment);
           FREE_MEMORY(pSar, sizeof(SAR_INFO));
       }

   }

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
       ("<==tbAtm155InitSarSegment\n"));

   return(Status);
}


NDIS_STATUS
TbAtm155Initialize(
   OUT PNDIS_STATUS    OpenErrorStatus,
   OUT PUINT           SelectedMediumIndex,
   IN  PNDIS_MEDIUM    MediumArray,
   IN  UINT            MediumArraySize,
   IN  NDIS_HANDLE     MiniportAdapterHandle,
   IN  NDIS_HANDLE     ConfigurationHandle
   )
/*++

Routine Description:

   This routine is the MiniportIntialize handler.  This is called by the
   NDIS wrapper to initialize the adapter.

Arguments:

   OpenErrorStatus         -   Not used.
   SelectedMediumIndex     -   On exit this will contain the media that the
                               driver supports.
   MediumArray             -   Array of media types that the adapter can
                               choose to support.
   MediumArraySize         -   Number of elements in the above array.
   MiniportAdapterHandle   -   Handle that NDIS needs to process requests
                               made by this driver.
   ConfigHandle            -   Configuration handle that is used to read
                               registry information.

Return Value:

   NDIS_STATUS_SUCCESS     if we successfully initialize the adapter.
   Failure code            otherwise.

--*/
{
   UINT                            c;
   PADAPTER_BLOCK                  pAdapter;
   PHARDWARE_INFO                  pHwInfo;
   NDIS_STATUS                     Status;
   PTBATM155_REGISTRY_PARAMETER    pRegistryParameters;
   ULONG                           i;
	UCHAR	                        CurrentAddress[ATM_ADDRESS_LENGTH];

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO, ("==>TbAtm155Initialize\n"));

   //
   //	We don't use the OpenErrorStatus.
   //
   *OpenErrorStatus = NDIS_STATUS_SUCCESS;

   do
   {
       //
       //	Initialize for clean-up.
       //
       pAdapter = NULL;

       //
       //	Do we support any of the given media types?
       //
       for (c = 0; c < MediumArraySize; c++)
       {
           if (MediumArray[c] == NdisMediumAtm)
           {
               break;
           }
       }
	
       //
       //	If we went through the whole media list without finding
       //	a supported media type let the wrapper know.
       //
       if (c == MediumArraySize)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Media not supported by version of ndis\n"));

           Status = NDIS_STATUS_UNSUPPORTED_MEDIA;

           break;
       }
	
       *SelectedMediumIndex = c;

       //
       //	Allocate memory for the registry parameters.
       //
       ALLOCATE_MEMORY(
           &Status,
           &pRegistryParameters,
           sizeof(TBATM155_REGISTRY_PARAMETER) * TbAtm155MaxRegistryEntry,
           '30DA');
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Unable to allocate memroy for the registry parameters\n"));

           break;
       }

       //
       //	Will initialize the default registry paramters.
       //
       tbAtm155InitConfigurationInformation(pRegistryParameters);

       ZERO_MEMORY(CurrentAddress, ATM_ADDRESS_LENGTH);

       //
       //	Read our parameters out of the registry.
       //
       Status = tbAtm155ReadConfigurationInformation(
                   ConfigurationHandle,
                   pRegistryParameters,
                   CurrentAddress);

       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to read the configuration information from the registry\n"));

           break;
       }

       //
       //	Allocate memory for our adapter block and initialize it.
       //
       ALLOCATE_MEMORY(
           &Status,
           &pAdapter,
           sizeof(ADAPTER_BLOCK) +
           (pRegistryParameters[TbAtm155VcHashTableSize].Value * sizeof(PVC_BLOCK)),
           '40DA');
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to allocate memory for the adapter block\n"));
           break;
       }

       ZERO_MEMORY(
           pAdapter,
           sizeof(ADAPTER_BLOCK) +
           (pRegistryParameters[TbAtm155VcHashTableSize].Value * sizeof(PVC_BLOCK)));

       //
       //	Attempt to allocate storage for the debug logs.
       //
       dbgInitializeDebugInformation(&pAdapter->DebugInfo);

       //
       //	Save the Wrapper's context information with our adapter block.
       //
       pAdapter->MiniportAdapterHandle = MiniportAdapterHandle;

       //
       //	Save the registry paramters.
       //
       pAdapter->RegistryParameters = pRegistryParameters;

       //
       //	Initialize the adapter spin lock.
       //
       NdisAllocateSpinLock(&pAdapter->lock);
       NdisAllocateSpinLock(&pAdapter->VcHashLock);

       //
       //	Spin lock and other odd allocations/initializations.
       //
       InitializeListHead(&pAdapter->ActiveVcList);
       InitializeListHead(&pAdapter->InactiveVcList);

       //
       //	Mark the adapter block as initializing.
       //
       ADAPTER_SET_FLAG(pAdapter, fADAPTER_INITIALIZING);


       //
       //   Allocate memory for the hardware information.
       //
       ALLOCATE_MEMORY(
           &Status,
           &pAdapter->HardwareInfo,
           sizeof(HARDWARE_INFO),
           '50DA');
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to allocate memory for the hardware information\n"));

           break;
       }

       ZERO_MEMORY(pAdapter->HardwareInfo, sizeof(HARDWARE_INFO));
		
       pHwInfo = pAdapter->HardwareInfo;

       NdisAllocateSpinLock(&pHwInfo->Lock);

       //
       //	Save whatever content into the station address by default.
       //
       NdisMoveMemory(
           pHwInfo->StationAddress,
           CurrentAddress,
           ATM_MAC_ADDRESS_LENGTH);

       //
       //	Get the registry parameters.
       //
       //  Make sure the number of receive buffers is not over 
       //  the thresholds.
       //
       if (pRegistryParameters[TbAtm155TotalRxBuffs].fPresent)
       {
           i = pRegistryParameters[TbAtm155TotalRxBuffs].Value;
           if (i > MAXIMUM_RECEIVE_BUFFERS)
           {
               i = MAXIMUM_RECEIVE_BUFFERS;
           }
           else if (i < MINIMUM_RECEIVE_BUFFERS)
           {
               i = MINIMUM_RECEIVE_BUFFERS;
           }
           pRegistryParameters[TbAtm155TotalRxBuffs].Value = i;
       }

       //
       //	Set the atributes for the adapter.
       //
       NdisMSetAttributesEx(
           MiniportAdapterHandle,
           (NDIS_HANDLE)pAdapter,
           0,
           NDIS_ATTRIBUTE_BUS_MASTER | NDIS_ATTRIBUTE_USES_SAFE_BUFFER_APIS,
           NdisInterfacePci);

       //
       //	Assign the PCI resources.
       //
       Status = tbAtm155ReadPciConfiguration(pAdapter, ConfigurationHandle);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to read the PCI configuration information\n"));
           break;
       }

       //
       //	Register the Port addresses.
       //
       Status = NdisMRegisterIoPortRange(
                       &pHwInfo->PortOffset,
                       pAdapter->MiniportAdapterHandle,
                       pHwInfo->InitialPort,
                       pHwInfo->NumberOfPorts);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to register the I/O port range\n"));
           break;
       }
       else
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO,
           	    ("Mapped memory pHwInfo->PortOffset:0x%x\n", pHwInfo->PortOffset));
       }

       //
       //	Get the Hardware Information 
       //  including reading in manufacturer address for the NIC.
       //
       Status = tbAtm155GetHardwareInformation(pAdapter);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to read the EEPROM information from the adapter\n"));
           break;										
       }

       //
       //	Detect Meteor type.
       //
       Status = tbAtm155IsIt4KVc(pAdapter);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to detect Meteor type..\n"));

           break;
       }

       //
       //	Initialize the SAR
       //
       Status = tbAtm155InitSarSegment(pAdapter);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to initialize the SAR information buffer.\n"));

           break;
       }

       //
       // Allocate receive & transmit report queues.
       //
       Status = tbAtm155AllocateReportQueues(pAdapter);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT,DBG_LEVEL_ERR,
               ("tbAtm155AllocateReportQueues failed! Status: 0x%x\n", Status));
           break;
       }


       Status = tbAtm155DetectPhyType(pAdapter);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to detect the PHY chip.\n"));

           break;
       }

       //
       //	Initialize the SAR registers including soft reset
       //  the controller.
       //
       Status = tbAtm155InitRegisters(pAdapter);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to initialize the 155 SAR registers\n"));

           break;
       }

       //
       //	Register the interrupt.
       //
       Status = NdisMRegisterInterrupt(
                   &pHwInfo->Interrupt,
                   pAdapter->MiniportAdapterHandle,
                   pHwInfo->InterruptVector,
                   pHwInfo->InterruptLevel,
                   TRUE,
                   TRUE,
                   NdisInterruptLevelSensitive);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_ERR,
               ("Failed to register the interrupt with ndis\n"));
           break;
       }

       // set up our link indication variable
       // it doesn't matter what this is right now because it will be
       // set correctly if link fails
       pAdapter->MediaConnectStatus = NdisMediaStateConnected;

       HW_SET_FLAG(pHwInfo, fHARDWARE_INFO_INTERRUPT_REGISTERED);

       //
       //	Return success.
       //
       Status = NDIS_STATUS_SUCCESS;

       // 
       // Pass the initialization. 
       // Initially, the LED_LNKUP_ON_ORANGE should be set
       // to indicate passing the initialization, then 
       // the LED_LNKUP_ON_GREEN will be set after PLC-2 detected
       // media connected interrupt. 
       // Somehow, the interrupt won't be generated on some 
       // systems. Instead of confusing users, set the LED
       // to be LED_LNKUP_ON_GREEN.
       // 
       pHwInfo->LedVal |= LED_LNKUP_ON_GREEN;

       //
       //	Register a shutdown handler for the adapter.
       //
       NdisMRegisterAdapterShutdownHandler(
           pAdapter->MiniportAdapterHandle,
           pAdapter,
           TbAtm155Shutdown);


#if DBG_USING_LED
       
       pHwInfo->dbgLedVal = pHwInfo->LedVal;

       TBATM155_PH_WRITE_DEV(
           pAdapter,
           LED_OFFSET,
           pHwInfo->dbgLedVal,
           &Status);

#endif // end of DBG_USING_LED


       //
       //	Enable interrupts on the adapter.
       //
       TBATM155_WRITE_PORT(
           &pHwInfo->TbAtm155_SAR->Intr_Enb,
           pHwInfo->InterruptMask);


       //
       //	Clear the adapter initializing flag.
       //
       ADAPTER_CLEAR_FLAG(pAdapter, fADAPTER_INITIALIZING);

   } while (FALSE);

   //
   //	Should we clean up?
   //
   if (NDIS_STATUS_SUCCESS != Status)
   {
       tbAtm155FreeResources(pAdapter);
   }

   DBGPRINT(DBG_COMP_INIT, DBG_LEVEL_INFO, ("<==TbAtm155Initialize\n"));

   return(Status);
}
	

