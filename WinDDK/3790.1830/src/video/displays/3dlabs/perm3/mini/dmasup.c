/**************************************************************************\
*
*                        ************************
*                        * MINIPORT SAMPLE CODE *
*                        ************************
*
* Module Name:
*
*   dmasup.c
*
* Abstract:
*
*  This module contains all the global data used by the Permedia3 driver.
*
* Environment:
*
*   Kernel mode
*
*
* Copyright (c) 1994-1999 3Dlabs Inc. Ltd. All rights reserved.
* Copyright (c) 1995-2002 Microsoft Corporation.  All Rights Reserved.
*
\***************************************************************************/

#include <perm3.h>


#define DMA_BUFFER_PAGES 16
#define DMA_BUFFER_SIZE DMA_BUFFER_PAGES*PAGE_SIZE
#define TOTAL_DMA_BUF_SIZE (DMA_BUFFER_PAGES*MAX_DMA_QUEUE_ENTRIES)


ULONG
Perm3StartDMA(
    PINTERRUPT_CONTROL_BLOCK pICBlk);

ULONG
Perm3WaitDMACompletion(
    PINTERRUPT_CONTROL_BLOCK pICBlk);

// DMA interrupt handler 

BOOLEAN
Perm3DMAInterruptHandler(
    PVOID HwDeviceExtension)
{
    PHW_DEVICE_EXTENSION hwDeviceExtension = HwDeviceExtension;
    PINTERRUPT_CONTROL_BLOCK pICBlk;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];
    PPerm3DMABufHeader pGPUBuf;

    pICBlk = &hwDeviceExtension->InterruptControl.ControlBlock;

    //
    // if suspended till VBLANK we can't do any DMA.
    //

    if (pICBlk->Control & SUSPEND_DMA_TILL_VBLANK) {

        VideoDebugPrint(( 1, "Perm3: DMA suspended till VBLANK\n"));
        return (TRUE);
    }

    //
    // If the previous DMA has not completed we can't do anything. We've
    // cleared the interrupt flag for this interrupt so even if the DMA
    // completes before we return, we'll immediately get another
    // interrupt. Since we will be getting another interrupt we do not
    // have to clear the DMAIntPending flag.
    //

    if (VideoPortReadRegisterUlong(DMA_COUNT) != 0) {

        //
        // DMA in progress, leaving
        //

        ASSERT(FALSE);
        return (TRUE);
    }

    //
    // Give the available buffer to the waiting client
    // Put the current DMA buffer back into the idle queue
    // 

    if (pICBlk->pGPUBuf)
    {
        if (! pICBlk->pClientBuf)
        {
            pICBlk->pClientBuf = pICBlk->pGPUBuf;
        }
        else
        {
            InsertTailList(&pICBlk->idleBufQueue, (PLIST_ENTRY)pICBlk->pGPUBuf);
        }
    }

    //
    // If no DMA buffer is ready for GPU, return
    //

    if (IsListEmpty((PLIST_ENTRY)&pICBlk->filledBufQueue))
    {
        // Indicate DMA is idle.
        pICBlk->pGPUBuf = NULL;

        return (TRUE);
    }

    //
    // Get the next filled DMA buffer for GPU
    //

    pGPUBuf = (PPerm3DMABufHeader)RemoveHeadList(&pICBlk->filledBufQueue);
    pICBlk->pGPUBuf = pGPUBuf;

#if 0
    VideoDebugPrint((0, "Perm3: DMA starting buf 0x%x, ", pGPUBuf->bufPhyAddr));
    VideoDebugPrint((0, "actual count = 0x%x.\n", pGPUBuf->ulActualCount)); 
#endif

    if (pGPUBuf->ulFlags & P3DMABUF_IS_AGP)
    {
        // Set up the AGP bus master
        VideoPortWriteRegisterUlong(DMA_CONTROL, 0x2); // Use AGP master
        VideoPortWriteRegisterUlong(AGP_CONTROL, 0x8); // Disable AGP long read 
    }
    VideoPortWriteRegisterUlong(DMA_ADDRESS, pGPUBuf->bufPhyAddr);
    VideoPortWriteRegisterUlong(DMA_COUNT,   pGPUBuf->ulActualCount);

    return (TRUE);
}


// Global function that initialize DMA

BOOLEAN
Perm3InitializeDMA(
    PVOID            HwDeviceExtension)
{
    PHW_DEVICE_EXTENSION        hwDeviceExtension = HwDeviceExtension;
    PINTERRUPT_CONTROL_BLOCK    pICBlk;
    int                         i;
    PPerm3DMABufHeader          pDMABuf;
    PHYSICAL_ADDRESS            lowestAddr, highestAddr, boundaryAddr;
    PHYSICAL_ADDRESS            bufPhyAddr;
#if 1
    BOOLEAN                     bRet, bUseAGP;
    VIDEO_PORT_AGP_SERVICES     AGPIF;
    PVOID                       pPhyReserveContext;
    PVOID                       pVirReserveContext;
    PVOID                       pAGPDMABuf;
    PHYSICAL_ADDRESS            agpBufPhyAddr;
#endif

    pICBlk = &hwDeviceExtension->InterruptControl.ControlBlock;

    // Initialize the queue headers
    InitializeListHead(&pICBlk->idleBufQueue);
    InitializeListHead(&pICBlk->filledBufQueue);

    // This is used to synchronize with interrupt handler
    pICBlk->hwDeviceExtension = hwDeviceExtension;

    // Fill in the StartDMA routine for the clients
    pICBlk->Perm3StartDMA = Perm3StartDMA;
    pICBlk->Perm3WaitDMACompletion = Perm3WaitDMACompletion;

    // Zero out some fields
    pICBlk->pGPUBuf       = NULL;
    pICBlk->CurrentBuffer = NULL;
    pICBlk->MaxAddress    = NULL;
    pICBlk->bStampedDMA   = FALSE;
    
    // Allocate DMA buffer
    lowestAddr.LowPart = lowestAddr.HighPart = 0;
    highestAddr.LowPart = 0xFFFFFFFF; highestAddr.HighPart = 0;
    boundaryAddr.LowPart = boundaryAddr.HighPart = 0;

    // There is only 8 MTTR registers on PII, to avoid running out of them,
    // allocate AGP dma buffers as a large chunk
    // Set the values that will be used by suballocator
    bUseAGP = FALSE;
    pAGPDMABuf = NULL;
    agpBufPhyAddr.LowPart = agpBufPhyAddr.HighPart = 0;

    do
    {
        // This is a PCI card, use the plain physical memory
        if (! hwDeviceExtension->bIsAGP)
        {
            break;
        }
       
        // Get the AGP service interface
        memset(&AGPIF, 0, sizeof(VIDEO_PORT_AGP_SERVICES));
        if (! VideoPortGetAgpServices(HwDeviceExtension, &AGPIF))
        {
            break;
        }
    
        // Reserve WC AGP memory for DMA buffers
        agpBufPhyAddr = AGPIF.AgpReservePhysical(
                                  HwDeviceExtension,
                                  TOTAL_DMA_BUF_SIZE,
#if (_WIN32_WINNT >= 0x501)
                                  VpWriteCombined,
#else
                                  TRUE,
#endif
                                  &pPhyReserveContext);
        if (! agpBufPhyAddr.LowPart)
        {
            // Try NC AGP memory instead
            {
                agpBufPhyAddr = AGPIF.AgpReservePhysical(
                                          HwDeviceExtension,
                                          TOTAL_DMA_BUF_SIZE,
#if (_WIN32_WINNT >= 0x501)
                                          VpNonCached,
#else
                                          FALSE,
#endif
                                          &pPhyReserveContext);
            }
        }

        if (! agpBufPhyAddr.LowPart)
        {
            // Bail out if AGP physical reservation failed
            break;
        }

        // Commit the physical pages to the reserved range
        bRet = AGPIF.AgpCommitPhysical(
                         HwDeviceExtension,
                         pPhyReserveContext,
                         TOTAL_DMA_BUF_SIZE,
                         0);                    // 0 offset
        if (! bRet)
        {
            // Bail out if AGP physical reservation failed
            break;
        }

        // Reserve a range in kernel address space
        pAGPDMABuf = AGPIF.AgpReserveVirtual(
                               HwDeviceExtension,
                               NULL,               // Map in kernel space
                               pPhyReserveContext,
                               &pVirReserveContext);
        if (! pAGPDMABuf)
        {
            // Bail out if AGP physical reservation failed
            break;
        }

        // For reservation in kernel space, this always succeed
        if (pAGPDMABuf = AGPIF.AgpCommitVirtual(
                                   HwDeviceExtension,
                                   pVirReserveContext,
                                   TOTAL_DMA_BUF_SIZE,
                                   0))                           // 0 offset
        {
            bUseAGP = TRUE;
        }

    } while (FALSE);
    
    //
    // Construct the idle DMA buffer queue
    //

    for (i = 0; i < MAX_DMA_QUEUE_ENTRIES; i++) 
    {
        
        if (! bUseAGP)
        {
            // Allocate non-cached page for the DMA
            pDMABuf = (PPerm3DMABufHeader)
                MmAllocateContiguousMemorySpecifyCache(
                    DMA_BUFFER_SIZE,
                    lowestAddr,
                    highestAddr,
                    boundaryAddr,
                    MmNonCached);

            // Get the physical address of the DMA buffer
            bufPhyAddr = MmGetPhysicalAddress(pDMABuf);
        }
        else
        {
            // Sub-allocate AGP DMA buffer
            pDMABuf = 
                (PPerm3DMABufHeader)(((unsigned char *)pAGPDMABuf) + 
                                     i*DMA_BUFFER_SIZE);

            bufPhyAddr.HighPart = 0;
            bufPhyAddr.LowPart = agpBufPhyAddr.LowPart + i*DMA_BUFFER_SIZE;
        }

        if (! pDMABuf)
        {
            break;
        }

        pDMABuf->bufPhyAddr = bufPhyAddr.LowPart + sizeof(Perm3DMABufHeader);

        // Set up the maximum virtual address
        pDMABuf->pBufEnd = ((PDMAEntry)pDMABuf) + DMA_BUFFER_SIZE/sizeof(ULONG);

#ifdef DBG
        // Stamp the DMA buffer for debugging
        memset(pDMABuf + 1, 
               0x4D,
               (pDMABuf->pBufEnd - (PDMAEntry)(pDMABuf + 1))*sizeof(DMAEntry));

        pDMABuf->ulFlags = P3DMABUF_STAMAPED;
#else
        pDMABuf->ulFlags = 0;
#endif

        if (bUseAGP)
        {
            // Mark the DMA buffer as in AGP aperture
            pDMABuf->ulFlags |= P3DMABUF_IS_AGP;
        }
        
        // Add the new buffer into the idle queue
        InsertTailList(&pICBlk->idleBufQueue, (PLIST_ENTRY)pDMABuf);
    }

    // Disaster happened, no single DMA buffer is available
    if (IsListEmpty(&pICBlk->idleBufQueue)) 
    {
        return (FALSE);
    }

    // Record actual number of buffers
    pICBlk->ulNumBuffers = i;

    // Zero out the fields
    pICBlk->pClientBuf    = 
        (PPerm3DMABufHeader)RemoveHeadList(&pICBlk->idleBufQueue);

    pDMABuf = pICBlk->pClientBuf;
    pICBlk->CurrentBuffer = (PDMAEntry)(pDMABuf + 1);
    pICBlk->MaxAddress    = pDMABuf->pBufEnd;
    pICBlk->bStampedDMA   = (BOOLEAN)(pDMABuf->ulFlags & P3DMABUF_STAMAPED);

    return (TRUE);
}


// Routine to start DMA that runs at the same IRQL as the interrup handler
BOOLEAN
Perm3SyncStartDMA(
    PVOID pContext)
{
    PINTERRUPT_CONTROL_BLOCK    pICBlk = pContext;
    PPerm3DMABufHeader          pDMABuf = pICBlk->pClientBuf;

    // Calc actual amount of DMA entries to transfer
    pDMABuf->ulActualCount = 
        (ULONG)(pICBlk->CurrentBuffer - ((PDMAEntry)(pDMABuf + 1)));

    // Put the current client into the queue ready for DMA
    InsertTailList(&pICBlk->filledBufQueue, (PLIST_ENTRY)pICBlk->pClientBuf);

    // Get a new DMA buffer for the client
    if (IsListEmpty(&pICBlk->idleBufQueue))
    {
        // Client will have to wait for idel DMA buffer to become available
        pICBlk->pClientBuf = NULL;
    }
    else
    {
        pICBlk->pClientBuf = 
            (PPerm3DMABufHeader)RemoveHeadList(&pICBlk->idleBufQueue);
    }

    // DMA is idle, start it manually
    if (! pICBlk->pGPUBuf) 
    {
        // Call the interrupt handler to start the DMA
        Perm3DMAInterruptHandler(pICBlk->hwDeviceExtension);
    }

    return (TRUE);
}

// Global function that start the DMA and make sure buffer frontIndex is free
ULONG
Perm3StartDMA(
    PINTERRUPT_CONTROL_BLOCK pICBlk)
{
    PPerm3DMABufHeader  pDMABuf;
    ULONG               ulNumWaited;

    pDMABuf = pICBlk->pClientBuf;
    
    // If there is data to DMA
    if (pICBlk->CurrentBuffer == ((PDMAEntry)(pDMABuf + 1))) 
    {
        return (1);
    }

    // Make sure client didn't write across the buffer boundary
    ASSERT(pICBlk->CurrentBuffer <= pDMABuf->pBufEnd);

#ifdef DBG
    pDMABuf->ulFlags &= (~P3DMABUF_STAMAPED);
#endif

    // Start the DMA in sync with the interrupt handler
    // Return value is from pPerm3SyncStartDMA()
    VideoPortSynchronizeExecution(
        pICBlk->hwDeviceExtension, 
        VpHighPriority,
        Perm3SyncStartDMA,
        pICBlk);    // Context for Perm3SyncStartDMA()

    ulNumWaited = 0;
    while (! pICBlk->pClientBuf)
    {
#if 0
        VideoDebugPrint((0, "Perm3: waiting for DMA buffer %d.\n", ulNumWaited)); 
#endif
        VideoPortStallExecution(5);
        ulNumWaited++;
    }

#if 0
    VideoDebugPrint((0, "Perm3: DMA get buf 0x%x.\n", pICBlk->pClientBuf)); 
#endif

    if (ulNumWaited)
    {
        pICBlk->BusyTime += 1;
    }
    else
    {
        pICBlk->IdleTime += 1;
    }

    pDMABuf = pICBlk->pClientBuf;

    // Update the current buffer pointer
    pICBlk->CurrentBuffer = (PDMAEntry)(pDMABuf + 1);
    pICBlk->MaxAddress    = pDMABuf->pBufEnd;
    pICBlk->bStampedDMA   = (BOOLEAN)pDMABuf->ulFlags & P3DMABUF_STAMAPED;

    return (1);
}

ULONG
Perm3WaitDMACompletion(
    PINTERRUPT_CONTROL_BLOCK pICBlk)
{
    ULONG ulCount;
    PHW_DEVICE_EXTENSION hwDeviceExtension = pICBlk->hwDeviceExtension;
    pPerm3ControlRegMap pCtrlRegs = hwDeviceExtension->ctrlRegBase[0];

    // Start the current client buffer which may contain DAM commands
    Perm3StartDMA(pICBlk);
    
    // Wait for last DMA buffer to be completed 
    while (pICBlk->pGPUBuf)
    {
#if 0
        VideoDebugPrint((0, "Perm3: last DMA buffer is not finished.\n"));
#endif
    }

    do 
    {
        ulCount = VideoPortReadRegisterUlong(DMA_COUNT);
#if 0
        VideoDebugPrint((0, "Perm3: DMA count 0x%x.\n", ulCount));
#endif
    } while (ulCount);
 
    return (1);
}

// Global function to release DMA

BOOLEAN
Perm3DestroyDMA(
    PVOID            HwDeviceExtension)
{
    return (TRUE);
}




