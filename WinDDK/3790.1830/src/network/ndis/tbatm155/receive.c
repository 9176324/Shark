/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       receive.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920



Abstract:

   This routine contains all of receive related handlers.

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	09/23/96		kyleb		Added support for NdisAllocateMemoryWithTag
	10/01/96		kyleb		Added async receive buffer allocation.
	10/18/96		kyleb		Added async receive indication.
	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.
    02/16/99        hhan        Use synchronize mode to allocate xmit buffer.

--*/

#include "precomp.h"
#pragma hdrstop

#define MODULE_NUMBER   MODULE_RECEIVE

NDIS_STATUS
tbAtm155FreeReceiveBufferInfo(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PRECV_BUFFER_INFO   pRecvInfo
   )
/*++

Routine Description:

   This routine will attempt to free the receive buffer information.

Arguments:

   pAdapter    -   Poitner to the ADAPTER_BLOCK.
   pRecvInfo   -   Pointer to the RECV_BUFFER_INFO structure that
                   describes one or more receive pools to free up.

Return Value:

   NDIS_STATUS_SUCCESS if the data structures were correctly free'd up.

--*/
{

   UNREFERENCED_PARAMETER(pAdapter);

   ASSERT(RECV_BUFFER_INFO_SIG == pRecvInfo->Signature);

   //
   //	Free up the receive information buffer.
   //
   NdisFreeSpinLock(&pRecvInfo->lock);
   FREE_MEMORY(pRecvInfo, sizeof(RECV_BUFFER_INFO));
   return(NDIS_STATUS_SUCCESS);

}



NDIS_STATUS
tbAtm155AllocateReceiveBufferInfo(
   OUT PRECV_BUFFER_INFO   *pRecvInfo,
   IN  PVC_BLOCK           pVc
   )
/*++

Routine Description:

   This routine will allocate receive buffer information.

Arguments:

   pRecvInfo   -   Pointer to storage for the receive buffer
                   information structure.
   pVc         -   Pointer to the VC block that this receive information
                   is for.

Return Value:

   NDIS_STATUS_SUCCESS     if we successfully allocated storage.
   NDIS_STATUS_RESOURCES   if the allocation failed.

--*/
{
   PRECV_BUFFER_INFO   ptmpRecvInfo;
   NDIS_STATUS         Status;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("==>tbAtm155AllocateReceiveBufferInfo\n"));

   //
   //	Allocate memory for the receive buffer information structure.
   //
   ALLOCATE_MEMORY(
       &Status,
       &ptmpRecvInfo,
       sizeof(RECV_BUFFER_INFO),
       '60DA');
   if (NDIS_STATUS_SUCCESS != Status)
   {
       DBGPRINT(DBG_COMP_VC, DBG_LEVEL_ERR,
           ("Unable to allocate RECV_BUFFER_INFO\n"));

       return(Status);
   }

   //
   //	This will initialize the list's heads and counts...
   //
   ZERO_MEMORY(ptmpRecvInfo, sizeof(RECV_BUFFER_INFO));

   //
   //	Allocate the spin lock that will protect this structure.
   //
   NdisAllocateSpinLock(&ptmpRecvInfo->lock);

#if DBG
   //
   // Set the signature for the receive buffer information structure.
   //
   ptmpRecvInfo->Signature = RECV_BUFFER_INFO_SIG;

#endif


   //
   //	Save the VC_BLOCK with the receive information.
   //
   ptmpRecvInfo->pVc = pVc;

   //
   //	Return the buffer information structure back to the caller.
   //
   *pRecvInfo = ptmpRecvInfo;

   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
       ("<==tbAtm155AllocateReceiveBufferInfo\n"));

   return(NDIS_STATUS_SUCCESS);
}



VOID
tbAtm155FreeReceiveBufferPool(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PRECV_BUFFER_POOL   pRecvPool
   )
/*++

Routine Description:

   This routine will free up a receive buffer pool and it's resources.
   If the VC that these resources belong to is deactivating then we also
   need to check to see if we can complete the deactivation.

Arguments:

   pAdapter    -   Pointer to the adapter block.
   pRecvPool   -   Pointer to the receive buffer pool that we are freeing.

Return Value:

   None.

Note:

   The receive buffer information spin lock is already held.

--*/
{
   PRECV_BUFFER_HEADER	ptmpRecvHeader;
   PRECV_BUFFER_HEADER	pNextRecvHeader;

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("==>tbAtm155FreeReceiveBufferPool\n"));

   ASSERT(RECV_BUFFER_POOL_SIG == pRecvPool->Signature);

   //
   //	Walk through the free buffer list and free the NDIS buffers and
   //  packets for each.  Also if the buffers were individually
   //  allocated then we need to free the shared memory for them also.
   //
   ptmpRecvHeader = pRecvPool->FreeBuffersHead;

   for (ptmpRecvHeader = pRecvPool->FreeBuffersHead;
        ptmpRecvHeader != NULL;
        ptmpRecvHeader = pNextRecvHeader)
   {
       //
       //	Save the pointer to the next receive header.
       //
       pNextRecvHeader = ptmpRecvHeader->Next;

       //
       //	Free the flush buffer.
       //
       if (NULL != ptmpRecvHeader->FlushBuffer)
       {
           NdisFreeBuffer(ptmpRecvHeader->FlushBuffer);
       }

       //
       //	Free the NDIS buffer.
       //
       if (NULL != ptmpRecvHeader->NdisBuffer)
       {
           NdisFreeBuffer(ptmpRecvHeader->NdisBuffer);
       }

       //
       //	Free the NDIS packet.
       //
       if (NULL != ptmpRecvHeader->Packet)
       {
           NdisFreePacket(ptmpRecvHeader->Packet);
       }

       //
       //	Was this buffer individually allocated?
       //
       if (!RECV_POOL_TEST_FLAG(pRecvPool, fRECV_POOL_BLOCK_ALLOC))
       {
           NdisMFreeSharedMemory(
               pAdapter->MiniportAdapterHandle,
               ptmpRecvHeader->Alloc[RECV_BUFFER_ORIGINAL].Size,
               TRUE,
               ptmpRecvHeader->Alloc[RECV_BUFFER_ORIGINAL].VirtualAddress,
               ptmpRecvHeader->Alloc[RECV_BUFFER_ORIGINAL].PhysicalAddress);
       }
   }

   //
   //	Were the receive buffers allocated in one big chunk?
   //
   if ((RECV_POOL_TEST_FLAG(pRecvPool, fRECV_POOL_BLOCK_ALLOC)) &&
       (NULL != pRecvPool->VirtualAddress))
   {
       NdisMFreeSharedMemory(
           pAdapter->MiniportAdapterHandle,
           pRecvPool->Size,
           TRUE,
           pRecvPool->VirtualAddress,
           pRecvPool->PhysicalAddress);
   }

   //
   //	Free the buffer pools and packet pools.
   //
   if (NULL != pRecvPool->hFlushBufferPool)
   {
       NdisFreeBufferPool(pRecvPool->hFlushBufferPool);
   }

   if (NULL != pRecvPool->hNdisBufferPool)
   {
       NdisFreeBufferPool(pRecvPool->hNdisBufferPool);
   }

   if (NULL != pRecvPool->hNdisPacketPool)
   {
       NdisFreePacketPool(pRecvPool->hNdisPacketPool);
   }

   //
   //	Free the memory used for the RECV_BUFFER_POOL structure.
   //
   FREE_MEMORY(pRecvPool, sizeof(RECV_BUFFER_POOL));

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("<==tbAtm155FreeReceiveBufferPool\n"));
}



NDIS_STATUS
tbAtm155InitializeReceiveBuffer(
   IN  PRECV_BUFFER_POOL       pRecvPool,
   IN  PRECV_BUFFER_HEADER     pRecvHeader,
   IN  PVOID                   VirtualAddress,
   IN  NDIS_PHYSICAL_ADDRESS   PhysicalAddress,
   IN  ULONG                   SizeOfReceiveBuffer
   )
/*++

Routine Description:

   This routine will initialize the receive header with the allocation info,
   (tempVa, tempPa and size of receive buffer).  It will also allocate
   resources from the receive pool (buffers and packets) for use by the
   receive buffer.

Arguments:

   pRecvPool           -   Pointer to the receive pool that this receive
							header belongs to.
   pRecvHeader         -   Pointer to the receive header that we are
                           initializing.
   VirtualAddress      -   This is the virtual address for the receive buffer.
   PhysicalAddress     -   This is the physical address for the receive
                           buffer.
   SizeOfReceiveBuffer -   Size of the receive buffer in bytes.

Return Value:

   NDIS_STATUS_SUCCESS     if the initialization is successful.


--*/
{
   NDIS_STATUS                     Status;
   PMEDIA_SPECIFIC_INFORMATION     pMediaSpecificInfo;

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("==>tbAtm155InitializeReceiveBuffer\n"));

   ASSERT(RECV_BUFFER_POOL_SIG == pRecvPool->Signature);

   //
   //	Determine the pointers to where the data will start.
   //
   pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].VirtualAddress =
               (PUCHAR)VirtualAddress + pRecvPool->ReceiveHeaderSize;

   NdisSetPhysicalAddressLow(
       pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].PhysicalAddress,
       NdisGetPhysicalAddressLow(PhysicalAddress) + pRecvPool->ReceiveHeaderSize);

   pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].Size = SizeOfReceiveBuffer;
   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("pRecvBuf[ALIGNED]: Va(%lx), Pa(%lx)\n", 
         pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].VirtualAddress,
         pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].PhysicalAddress));

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("pRecvBuf[ALIGNED]: Size(%lx)\n", 
         pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].Size));

   do
   {
       //
       //	Allocate a flush buffer.
       //
       NdisAllocateBuffer(
           &Status,
           &pRecvHeader->FlushBuffer,
           pRecvPool->hFlushBufferPool,
           pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].VirtualAddress,
           pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].Size);
       if (Status != NDIS_STATUS_SUCCESS)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Unable to allocate an NDIS_BUFFER for the flush buffer\n"));

           break;
       }

       //
       //	Allocate a buffer to describe the data to be indicated.
       //
       NdisAllocateBuffer(
           &Status,
           &pRecvHeader->NdisBuffer,
           pRecvPool->hNdisBufferPool,
           (PUCHAR)pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].VirtualAddress,
           pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].Size);
       if (Status != NDIS_STATUS_SUCCESS)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Unabled to allocate an NDIS_BUFFER for the receive buffer\n"));

           break;
       }
	
       //
       //	Allocate a packet from the pool for the receive buffer.
       //
       NdisAllocatePacket(
           &Status,
           &pRecvHeader->Packet,
           pRecvPool->hNdisPacketPool);
       if (Status != NDIS_STATUS_SUCCESS)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Unable to allocate an NDIS_PACKET for the receive packet\n"));

           break;
       }
   } while (FALSE);

   if (NDIS_STATUS_SUCCESS == Status)
   {
		//
		//	Initialize the media specific information.
		//	Our media specific storage is right after the protocol's
		//	reserved area.
		//
		pMediaSpecificInfo = (PMEDIA_SPECIFIC_INFORMATION)((PUCHAR)pRecvHeader->Packet +
								FIELD_OFFSET(NDIS_PACKET, ProtocolReserved) +
								PROTOCOL_RESERVED_SIZE_IN_PACKET +
								sizeof(RECV_PACKET_RESERVED));
       pMediaSpecificInfo->NextEntryOffset = 0;
       pMediaSpecificInfo->ClassId = NdisClassAtmAALInfo;
       pMediaSpecificInfo->Size = sizeof(ATM_AAL_OOB_INFO);

       //
       //	Save the pointer to the media specific information in
       //	the packets OOB information.
       //
       NDIS_SET_PACKET_MEDIA_SPECIFIC_INFO(
           pRecvHeader->Packet,
           pMediaSpecificInfo,
			sizeof(MEDIA_SPECIFIC_INFORMATION) + sizeof(ATM_AAL_OOB_INFO));

       //
       //	Save the pool with the receive buffer.
       //
       pRecvHeader->Pool = pRecvPool;

       //
       //	Chain the buffer to the owning pool.
       //
       pRecvHeader->Next = pRecvPool->FreeBuffersHead;
		pRecvHeader->Prev = NULL;
       pRecvPool->FreeBuffersHead = pRecvHeader;
       pRecvPool->BuffersFreed++;
   }
   else
   {

       DBGPRINT(DBG_COMP_SPECIAL, DBG_LEVEL_ERR,
           ("tbAtm155InitializeReceiveBuffer: failed, freed up allcated buffers.\n"));

       if (NULL != pRecvHeader->FlushBuffer)
       {
           NdisFreeBuffer(pRecvHeader->FlushBuffer);
           pRecvHeader->FlushBuffer = NULL;
       }

       //
       //	Free the NDIS buffer.
       //
       if (NULL != pRecvHeader->NdisBuffer)
       {
           NdisFreeBuffer(pRecvHeader->NdisBuffer);
           pRecvHeader->NdisBuffer = NULL;
       }

       //
       //	Free the NDIS packet.
       //
       if (NULL != pRecvHeader->Packet)
       {
           NdisFreePacket(pRecvHeader->Packet);
           pRecvHeader->Packet = NULL;
       }
   }

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("<==tbAtm155InitializeReceiveBuffer\n"));

   return(Status);
}


NDIS_STATUS
tbAtm155InitializeBlockReceivePool(
   IN  PRECV_BUFFER_POOL   pRecvPool
	)
/*++

Routine Description:

   This routine will initialize receive buffers from a receive buffer pool
   that was allocated in a single chunk of shared memory.

Arguments:

   pRecvPool   -   Pointer to the receive pool to initialize.

Return Value:

   NDIS_STATUS_SUCCESS     if the receive pool is successfully initialized.

--*/
{
   ULONG                   Offset;
   PUCHAR                  StartVa;
   NDIS_PHYSICAL_ADDRESS   StartPa;
   PUCHAR                  AlignedVa;
   NDIS_PHYSICAL_ADDRESS   AlignedPa;
   ULONG                   BufferSize;
   PRECV_BUFFER_HEADER     pRecvHeader;
   UINT                    c;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;


   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("==>tbAtm155InitializeBlockReceivePool\n"));

   ASSERT(RECV_BUFFER_POOL_SIG == pRecvPool->Signature);

   //
   //	Determine the buffer size including header.
   //
   BufferSize = pRecvPool->ReceiveHeaderSize + pRecvPool->ReceiveBufferSize;

   //
   //	Align the block.
   //
   Offset = (ULONG)((ULONG_PTR)pRecvPool->VirtualAddress % gDmaAlignmentRequired);
   if (Offset != 0)
   {
       Offset = gDmaAlignmentRequired - Offset;
   }

   //
   //	Get pointers to the aligned block.
   //
   StartVa = ((PUCHAR)pRecvPool->VirtualAddress + Offset);

   NdisSetPhysicalAddressHigh(StartPa, 0);
   NdisSetPhysicalAddressLow(
       StartPa,
       NdisGetPhysicalAddressLow(pRecvPool->PhysicalAddress) + Offset);

   RECV_POOL_SET_FLAG(pRecvPool, fRECV_POOL_BLOCK_ALLOC);

   //
   //	Initialize the temporary physical address.
   //
   AlignedPa = StartPa;

   for (c = 0, Offset = 0; c < pRecvPool->ReceiveBufferCount; c++)
   {
       //
       //	Determine the virtual address for the receive buffer within
       //	the block.
       //
       AlignedVa = (PUCHAR)StartVa + Offset;

       //
       //	Determine the physical address for the receive buffer within
       //	the block.
       //
       NdisSetPhysicalAddressLow(
           AlignedPa,
           NdisGetPhysicalAddressLow(StartPa) + Offset);

       //
       //	Get the offset for the next buffer within the block.
       //
       Offset += BufferSize;

       //
       //	Get a pointer to the receive header.
       //
       pRecvHeader = (PRECV_BUFFER_HEADER)AlignedVa;

       //
       //	Initialize the header.
       //
       ZERO_MEMORY(pRecvHeader, sizeof(RECV_BUFFER_HEADER));

       //
       //	Pass the receive buffer to a common initialization routine.
       //
       Status = tbAtm155InitializeReceiveBuffer(
                   pRecvPool,
                   pRecvHeader,
                   AlignedVa,
                   AlignedPa,
                   pRecvPool->ReceiveBufferSize);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("tbAtm155InitializeReceiveBuffer failed\n"));

           break;
       }
   }

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("<==tbAtm155InitializeBlockReceivePool\n"));

   return(Status);
}

VOID
tbAtm155BuildIndividualReceiveHeader(
   OUT PRECV_BUFFER_HEADER     *ppRecvHeader,
   OUT PUCHAR                  *pAlignedVa,
   OUT NDIS_PHYSICAL_ADDRESS   *pAlignedPa,
   IN  PUCHAR                  VirtualAddress,
   IN  NDIS_PHYSICAL_ADDRESS   PhysicalAddress,
   IN  ULONG                   TotalBufferSize
)
/*++

Routine Description:

   This routine simply aligns the virutal address to get the receive header
   and saves the original virutal address, physical address, and size with it.
   The reason it's in a routine is that it's called from different places
   in the driver.

Arguments:

   ppRecvHeader    -   Pointer to the storage for the PRECV_BUFFER_HEADER.
   pAlignedVa      -   Pointer to the storage for the aligned virtual address.
   pAlignedPa      -   Pointer to the storage for the aligned physical address.
   VirtualAddress  -   Unaligned virtual address for the receive buffer.
   PhysicalAddress -   Unaligned physical address for the receive buffer.
   TotalBufferSize -   Size of the receive buffer and header.

Return Value:

   None.

--*/
{
   ULONG               Offset;
   PRECV_BUFFER_HEADER pRecvHeader;

   //
   //	Determine the offset into the allocated buffer so that it would
   //	be correctly aligned.
   //
   Offset = (ULONG)((ULONG_PTR)VirtualAddress % gDmaAlignmentRequired);
   if (Offset != 0)
   {
       Offset = gDmaAlignmentRequired - Offset;
   }
   *pAlignedVa = (PUCHAR)VirtualAddress + Offset;

   NdisSetPhysicalAddressLow(
       *pAlignedPa,
       NdisGetPhysicalAddressLow(PhysicalAddress) + Offset);

   //
   //	Get a pointer to the receive buffer header.
   //
   pRecvHeader = (PRECV_BUFFER_HEADER)*pAlignedVa;

   //
   //	Initialize the receive header.
   //
   ZERO_MEMORY(pRecvHeader, sizeof(RECV_BUFFER_HEADER));

   //
   //	Save the original virtual address, physical address, and size
   //	of the shared memory allocation for deletion at a later time.
   //
   pRecvHeader->Alloc[RECV_BUFFER_ORIGINAL].VirtualAddress = VirtualAddress;
   pRecvHeader->Alloc[RECV_BUFFER_ORIGINAL].PhysicalAddress = PhysicalAddress;
   pRecvHeader->Alloc[RECV_BUFFER_ORIGINAL].Size = TotalBufferSize;

   *ppRecvHeader = pRecvHeader;
}


NDIS_STATUS
tbAtm155AllocateIndividualReceiveBuffers(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PRECV_BUFFER_POOL   pRecvPool
   )
/*++

Routine Description:

   This routine will attempt to allocate individual shared memory chunks
   for the receive buffers.

Arguments:

   pAdapter    -   Pointer to the ADAPTER_BLOCK.
   pRecvPool   -   Pointer to the RECV_BUFFER_POOL that will manage the
                   receive buffers.

Return Value:

   NDIS_STATUS_SUCCESS     if we were able to allocate the receive buffers.

--*/
{
   ULONG                   TotalBufferSize;
   UINT                    c;
   PUCHAR                  VirtualAddress;
   NDIS_PHYSICAL_ADDRESS   PhysicalAddress;
   PUCHAR                  AlignedVa;
   NDIS_PHYSICAL_ADDRESS   AlignedPa;
   PRECV_BUFFER_HEADER     pRecvHeader;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;

   ASSERT(RECV_BUFFER_POOL_SIG == pRecvPool->Signature);


   //
   //	We need to readjust the size of the recieve header, with
   //	respect to the alignment, to take into consideration another
   //	ALLOCATION_INFO struct.  this structure will hold the original
   //	virtual address, physical address and size that is returned for
   //	each, individual buffer allocation.
   //
   pRecvPool->ReceiveHeaderSize =
       (((sizeof(RECV_BUFFER_HEADER) + sizeof(ALLOCATION_INFO) +
        gDmaAlignmentRequired - 1) / gDmaAlignmentRequired) *
        gDmaAlignmentRequired);

   //
   //	Determine the actual size for each shared memory allocation.
   //
   TotalBufferSize = pRecvPool->ReceiveHeaderSize +
                       pRecvPool->ReceiveBufferSize;


   //
   //	Loop allocating the buffers.
   //
   for (c = 0; c < pRecvPool->ReceiveBufferCount; c++)
   {
       //
       //	Attempt to allocate the shared memory.
       //
       NdisMAllocateSharedMemory(
           pAdapter->MiniportAdapterHandle,
           TotalBufferSize,
           TRUE,
           &VirtualAddress,
           &PhysicalAddress);
       if (NULL == VirtualAddress)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Unable to allocate an individual shared memory receive buffer\n"));

           Status = NDIS_STATUS_RESOURCES;
           break;
       }

       //
       //	Build the individual receive header.
       //
       tbAtm155BuildIndividualReceiveHeader(
               &pRecvHeader,
               &AlignedVa,
               &AlignedPa,
               VirtualAddress,
               PhysicalAddress,
               TotalBufferSize);

       //
       //	Finish the common buffer initialization.
       //
       Status = tbAtm155InitializeReceiveBuffer(
                       pRecvPool,
                       pRecvHeader,
                       AlignedVa,
                       AlignedPa,
                       pRecvPool->ReceiveBufferSize);
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("tbAtm155InitializeReceiveBuffer failed for individual allocation\n"));

           //
           //	Deallocate the last shared memory allocation now.
           //	If any other individual buffers were allocted they are already
           //	queued to the receive pool and will be cleaned up when we
           //	deallocate the receive pool.
           //
           NdisMFreeSharedMemory(
               pAdapter->MiniportAdapterHandle,
               TotalBufferSize,
               TRUE,
               VirtualAddress,
               PhysicalAddress);

           break;
       }
   }

   return(Status);
}


VOID
tbAtm155QueueReceivePoolToSarInfo(
   IN  PRECV_BUFFER_INFO   pRecvInfo,
   IN  PRECV_BUFFER_POOL   pRecvPool
   )
/*++

Routine Description:

   This routine will walk through the receive pool's buffer list, remove the
   receive buffers from it and place them on either "BIG" or "SMALL" 
   free buffer list in the Sar information database.

Arguments:

   pRecvInfo   -   Pointer to the PRECV_BUFFER_INFO that owns the pool.
   pRecvPool   -   Pointer to the PRECV_BUFFER_POOL whose packets we are going
                   to allocate.
Return Value:

   None.

--*/
{
   PRECV_BUFFER_HEADER	    pRecvHeader;
   PRECV_BUFFER_HEADER	    pNextRecvHeader;
   PRECV_SEG_INFO          pRecvSegInfo = pRecvInfo->pVc->RecvSegInfo;
   PRECV_BUFFER_QUEUE      pFreeBufQ;
   PSAR_INFO               pSar = pRecvInfo->pVc->Sar;
   PRECV_BUFFER_POOL       pRecvPoolTail;
   PRECV_BUFFER_POOL       pRecvPoolQ;
   PUSHORT                 pAllocatedRecvBuffers;

   ASSERT(RECV_BUFFER_INFO_SIG == pRecvInfo->Signature);
   ASSERT(RECV_BUFFER_POOL_SIG == pRecvPool->Signature);

   //
   //	Add the new receive buffer to the Sar buffer info.
   //
   NdisAcquireSpinLock(&pRecvInfo->lock);

   if (pRecvInfo->pVc->RecvBufType == RECV_SMALL_BUFFER)
   {
       // 
       //  Let's link to the "Small" buffer pool.
       // 
       pFreeBufQ = pRecvSegInfo->FreeSmallBufQ;
       pRecvPoolTail = pSar->SmallRecvPoolTail;
       pRecvPoolQ = pSar->SmallRecvPoolQ;
       pAllocatedRecvBuffers = &pSar->AllocatedSmallRecvBuffers;
   }
   else
   {
       // 
       //  Let's link to the "Big" buffer pool.
       // 
       pFreeBufQ = pRecvSegInfo->FreeBigBufQ;
       pRecvPoolTail = pSar->BigRecvPoolTail;
       pRecvPoolQ = pSar->BigRecvPoolQ;
       pAllocatedRecvBuffers = &pSar->AllocatedBigRecvBuffers;
   }

   //
   //	Walk the list of receive buffers on the pool and add them
   //	to the free list.
   //
   for (pRecvHeader = pRecvPool->FreeBuffersHead;
        pRecvHeader != NULL;
        pRecvHeader = pNextRecvHeader)
   {
       //
       //	Save the real next pointer.
       //
       pNextRecvHeader = pRecvHeader->Next;

       //
       //  Queue the buffer to the end of queue.
       //
       tbAtm155InsertRecvBufferAtTail(pFreeBufQ, pRecvHeader, TRUE);
   }

   //
   //	There are no more receive buffers in the pool.	
   //	They all have been allocated for use and placed in the
   //	receive buffer information's free list.
   //

   //
   //  Queue pRecvPool to the (Big or Small) Recv_Buffer_Pool Queue.
   //
   if (NULL == pRecvPoolQ)
   {
       // 
       // Queue is empty
       // 
       pRecvPoolQ = pRecvPool;
       pRecvPoolTail = pRecvPool;
   }
   else
   {
       pRecvPoolTail->Next = pRecvPool;
       pRecvPoolTail = pRecvPool;
   }
   *pAllocatedRecvBuffers += (USHORT) (pRecvPool->BuffersFreed);

   pRecvPool->FreeBuffersHead = NULL;
   pRecvPool->BuffersFreed = 0;

   //
   // Reset to NULL to dettach from pVc and its data structures.
   //
   pRecvPool->RecvBufferInfo = NULL;

   NdisReleaseSpinLock(&pRecvInfo->lock);
}



NDIS_STATUS
tbAtm155AllocateReceiveBufferPool(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PRECV_BUFFER_INFO   pRecvInfo,
   IN  ULONG               NumberOfReceiveBuffers,
   IN  ULONG               SizeOfReceiveBuffer
   )
/*++

Routine Description:

   This routine will allocate a pool of receive buffers, NDIS_BUFFERs
   to identify them and packets to package them.

Arguments:

   pAdapter                -   Pointer to the adapter block.
   pRecvInfo               -   Pointer to the receive buffer information that
                               will keep track of the buffer pool that we
                               allocate.
   NumberOfReceiveBuffers  -   Number of buffers to allocate.
   SizeOfReceiveBuffers    -   Size of each receive buffer.

Return Value:

   NDIS_STATUS_SUCCESS     if the allocation was successful.
   NDIS_STATUS_RESOURCES   if the allocation failed due to memory constraints.

--*/
{
   NDIS_STATUS         Status;
   PRECV_BUFFER_POOL   pRecvPool = NULL;
   ULONG               TotalBufferSize;

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("==>tbAtm155AllocateReceiveBufferPool\n"));

   ASSERT(RECV_BUFFER_INFO_SIG == pRecvInfo->Signature);

   do
   {
       //
       //	Allocate storage for the buffer pool.
       //
       ALLOCATE_MEMORY(
           &Status,
           &pRecvPool,
           sizeof(RECV_BUFFER_POOL),
           '80DA');
       if (NDIS_STATUS_SUCCESS != Status)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Unable to allocate a buffer pool\n"));

           break;
       }

       ZERO_MEMORY(pRecvPool, sizeof(RECV_BUFFER_POOL));

       pRecvPool->Signature = RECV_BUFFER_POOL_SIG;

       //
       //	Save a back pointer to the owning receive buffer information
       //	structure.
       //
       pRecvPool->RecvBufferInfo = pRecvInfo;

       //
       //	Allocate the packet pool.
       //
       NdisAllocatePacketPool(
           &Status,
           &pRecvPool->hNdisPacketPool,
           NumberOfReceiveBuffers,
           PROTOCOL_RESERVED_SIZE_IN_PACKET + TBATM155_PROTOCOL_RESERVED);
       if (Status != NDIS_STATUS_SUCCESS)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Unable to allocate NDIS packet pool\n"));

           break;
       }

       //
       //	Allocate a buffer pool.
       //
       NdisAllocateBufferPool(
           &Status,
           &pRecvPool->hFlushBufferPool,
           NumberOfReceiveBuffers);
       if (Status != NDIS_STATUS_SUCCESS)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Unable to allocate a NDIS_BUFFER pool for flush buffers.\n"));

           break;
       }

       //
       //	Allocate a buffer pool.
       //
       NdisAllocateBufferPool(
           &Status,
           &pRecvPool->hNdisBufferPool,
           NumberOfReceiveBuffers);
       if (Status != NDIS_STATUS_SUCCESS)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Unable to allocate a NDIS_BUFFER pool for receive buffers.\n"));

           break;
       }

       //
       //	Save the number of buffers needed and the size for each buffer
       //	with the pool.
       //
       pRecvPool->ReceiveBufferCount = NumberOfReceiveBuffers;

       pRecvPool->ReceiveBufferSize = 
			(((SizeOfReceiveBuffer + gDmaAlignmentRequired - 1) /
			gDmaAlignmentRequired) * gDmaAlignmentRequired);


       //
       //	Using the alignment determine the size of the header
       //	for the receive buffers.
       //
       pRecvPool->ReceiveHeaderSize =
               (((sizeof(RECV_BUFFER_HEADER) + gDmaAlignmentRequired - 1) /
               gDmaAlignmentRequired) * gDmaAlignmentRequired);

       //
       //	Determine the total size needed for each buffer.
       //
       TotalBufferSize = pRecvPool->ReceiveHeaderSize + SizeOfReceiveBuffer;

       //
       //	We are going to first attempt to allocate this in one
       //	large chunk.
       //
       pRecvPool->Size = (NumberOfReceiveBuffers * TotalBufferSize) +
                          gDmaAlignmentRequired;

       //
       //	If we are doing this from the tbAtm155ActivateVc handler then
       //	we need to call async allocate to get our shared memory.
       //
       if (!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_INITIALIZING))
       {
           ASSERT(NULL != pRecvInfo->pVc);

           //
           //	Setup for the staged allocation.
           //
           RECV_POOL_SET_FLAG(pRecvPool, fRECV_POOL_ALLOCATING_BLOCK);

           //
           //	Attempt to allocate the shared memory.
           //
           Status = NdisMAllocateSharedMemoryAsync(
                           pAdapter->MiniportAdapterHandle,
                           pRecvPool->Size,
                           TRUE,
                           pRecvPool);

           //
           //	The call above will either return pending or some error code.
           //
           break;
       }

       //
       //	Allocate a chunk of shared ram for the receive buffers.
       //
       NdisMAllocateSharedMemory(
           pAdapter->MiniportAdapterHandle,
           pRecvPool->Size,
           TRUE,
           &pRecvPool->VirtualAddress,
           &pRecvPool->PhysicalAddress);
       if (NULL != pRecvPool->VirtualAddress)
       {
           //
           //	Initialize the receive pool.
           //
           Status = tbAtm155InitializeBlockReceivePool(pRecvPool);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                   ("Unable to initialize the receive block\n"));

               break;
           }
       }
       else
       {
           //
           //	Do the individual buffer allocations.
           //
           Status = tbAtm155AllocateIndividualReceiveBuffers(
                       pAdapter,
                       pRecvPool);
           if (NDIS_STATUS_SUCCESS != Status)
           {
               DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                   ("Unable to allocate individual receive buffers\n"));

               break;
           }
       }

       //
       //	Did we successfully set up our receive packets and
       //	buffers?
       //
       if (NDIS_STATUS_SUCCESS == Status)
       {
           //
           //	Place the receive pool's buffers onto the receive info's
           //	free buffer list.
           //
           tbAtm155QueueReceivePoolToSarInfo(pRecvInfo, pRecvPool);

       }

   } while (FALSE);			

   //
   //	If we failed the routine then we need to free the resources
   //	that were allocated.
   //
   if ((NDIS_STATUS_SUCCESS != Status) && (NDIS_STATUS_PENDING != Status))
   {
       if (NULL != pRecvPool)
       {
           tbAtm155FreeReceiveBufferPool(pAdapter, pRecvPool);
       }
   }

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("<==tbAtm155AllocateReceiveBufferPool\n"));

   return(Status);
}


BOOLEAN
tbAtm155RxInterruptOnCompletion(
   IN  PADAPTER_BLOCK      pAdapter
	)
/*++

Routine Description:

   This routine will process the Receive Interrupt On Completion
   for the different receive buffers.  This interrupt will be generated
   when a packet has been received from a Rx Active Slot list of a VC.
   If the packet(s) have been receive without error, pass up to NDIS.
   Otherwise, discard the packet and return the buffers into free pool
   for the next receiving.


Arguments:

   pAdapter    -   Pointer to the adapter block.

Return Value:

   None.

--*/
{
   PHARDWARE_INFO              pHwInfo = pAdapter->HardwareInfo;
   PSAR_INFO                   pSar = pHwInfo->SarInfo;
   PRECV_DMA_QUEUE             pRecvDmaQ = &pSar->RecvDmaQ;
   AAL5_PDU_TRAILER            Trailer;
   PNDIS_PACKET                Packet;
   PVC_BLOCK                   pVc;
   PRX_REPORT_QUEUE            pRxReportQ = &pAdapter->RxReportQueue;
   PRX_REPORT_QUEUE_ENTRY      RxReportQueue;
   RX_REPORT_PTR               RxReportPtrReg;
   ULONG                       CurrentRxReportPtrPa;
   UINT                        CurrentRxReportQIdx;
   UINT                        i;
   ULONG                       Vci;
   PRECV_SEG_INFO              pRecvSegInfo;
   PRECV_BUFFER_HEADER         pRecvBufHead;
   PRECV_BUFFER_QUEUE          pFreeBufQ;

   PRECV_BUFFER_QUEUE          pSegCompleting;

   PNDIS_BUFFER                LastBuffer;
   PUCHAR                      LastBufferVa;
   UINT                        LastBufferLength;

   PNDIS_BUFFER                prevLastBuffer;
   PUCHAR                      prevLastBufferVa;
   UINT                        prevLastBufferLength;
   ULONG                       TrimAmount;
   UINT                        PacketLength;
   UINT                        PhysicalBufferCount;

   PMEDIA_SPECIFIC_INFORMATION pMediaSpecificInfo;
   ULONG                       MediaSpecificInfoSize;
   PATM_AAL_OOB_INFO           pAtmOobInfo;
   LONG                        tmpBufferType;
   BOOLEAN                     fReturnStatus = FALSE;
   BOOLEAN                     fRxSlotsLowOrNone;
   PRX_REPORT_QUEUE_ENTRY      pEntryOfRxReportQ;

   NDIS_STATUS                 Status;
   ULONG                       Dest;
   PULONG                      pTmpFreeSlots;
   PRECV_PACKET_RESERVED       Reserved;
   BOOLEAN                     fCorruptedPacket = FALSE;
   BOOLEAN                     fQueryBufferFail = FALSE;
#if    DBG
   PULONG                      pdbgTotalUsedRxSlots;
#endif // end of DBG


   TBATM155_READ_PORT(
       &pHwInfo->TbAtm155_SAR->Rx_Report_Ptr,
       &RxReportPtrReg.reg);

   //
   //  Got the position of 155 PCI SAR's internal Rx report queue.
   //  This pointer indicates the address in host memory to whcih 155 SAR
   //  will write the next Rx report. So we will only process up to the
   //  previous entry to make sure the status is valid
   //
   CurrentRxReportPtrPa = RxReportPtrReg.reg & ~0x3;

   //
   //  Calcuate the index of Rx Report Queue which is pointed by controller.
   //  HANDLING RULE:
   //      We only handle up to the entry before this one.
   //
   CurrentRxReportQIdx =
           (CurrentRxReportPtrPa - 
           NdisGetPhysicalAddressLow(pRxReportQ->RxReportQptrPa)) /
           sizeof(RX_REPORT_QUEUE_ENTRY);

   //
   //  Get the starting point where the entry of Rx report queue 
   //  needs to be handled.
   //
   RxReportQueue = (PRX_REPORT_QUEUE_ENTRY)pRxReportQ->RxReportQptrVa;

   NdisDprAcquireSpinLock(&pRecvDmaQ->lock);

   DBGPRINT(DBG_COMP_RXIOC, DBG_LEVEL_ERR, 
           ("RxIOC CurRxRQIdx(%x), PrevRxRQIdx(%x)\n", 
           CurrentRxReportQIdx, pRecvDmaQ->PrevRxReportQIndex));

   for (i = pRecvDmaQ->PrevRxReportQIndex;
        i != CurrentRxReportQIdx;
        i = (i == pHwInfo->MaxIdxOfRxReportQ)? 0 : ++i)
   {
       //
       //  Get the VC from report queue.
       //
       pEntryOfRxReportQ = &RxReportQueue[i];
       Vci = (ULONG) pEntryOfRxReportQ->RxReportQDWord1.VC;

       //       
       //  Get the physical address of reported buffer to search
       //  the buffer from CompletingXXXBufQ accordingly.
       //       
       Dest = pEntryOfRxReportQ->RxReportQDWord0.Index;

       tmpBufferType = (pEntryOfRxReportQ->RxReportQDWord0.rqSlotType == 1)?
                       RECV_BIG_BUFFER :
                       RECV_SMALL_BUFFER;

       //
       //  Search reported buffer queue first. 
       //  if not find the buffer then search another buffer queue.
       //
       if (RECV_BIG_BUFFER == tmpBufferType)
       {

#if    DBG
           pdbgTotalUsedRxSlots = &pRecvDmaQ->dbgTotalUsedBigRxSlots;
#endif // end of DBG

           tbAtm155SearchRecvBufferFromQueue(
               &pRecvDmaQ->CompletingBigBufQ,
               Dest,
               &pRecvBufHead);

           if (NULL == pRecvBufHead)
           {
               tmpBufferType = RECV_SMALL_BUFFER;

#if    DBG
               pdbgTotalUsedRxSlots = &pRecvDmaQ->dbgTotalUsedSmallRxSlots;
#endif // end of DBG

               //           
               //  Try from "Small" buffer pool.           
               //           
               tbAtm155SearchRecvBufferFromQueue(
                   &pRecvDmaQ->CompletingSmallBufQ,
                   Dest,
                   &pRecvBufHead);
           }
       }
       else
       {

#if    DBG
           pdbgTotalUsedRxSlots = &pRecvDmaQ->dbgTotalUsedSmallRxSlots;
#endif // end of DBG

           tbAtm155SearchRecvBufferFromQueue(
               &pRecvDmaQ->CompletingSmallBufQ,
               Dest,
               &pRecvBufHead);

           if (NULL == pRecvBufHead)
           {
               tmpBufferType = RECV_BIG_BUFFER;

#if    DBG
               pdbgTotalUsedRxSlots = &pRecvDmaQ->dbgTotalUsedBigRxSlots;
#endif // end of DBG

               //           
               //  Try from "Big" buffer pool.           
               //           
               tbAtm155SearchRecvBufferFromQueue(
                   &pRecvDmaQ->CompletingBigBufQ,
                   Dest,
                   &pRecvBufHead);
           }
       }

       if (NULL == pRecvBufHead)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Can NOT find from either pool.(Dest=%lx)\n", Dest));
           
           DBGPRINT(DBG_COMP_NRXBUFFOUND, DBG_LEVEL_ERR, 
                   ("RxIOC CurRxRQIdx(%x), PrevRxRQIdx(%x)\n", 
                   CurrentRxReportQIdx, pRecvDmaQ->PrevRxReportQIndex));

           DBGPRINT(DBG_COMP_NRXBUFFOUND, DBG_LEVEL_ERR, 
                   ("RxIOC RxReportEntryW0(%lx), RxReportEntryW1(%lx)\n", 
                   pEntryOfRxReportQ->RxReportQueueDword0, 
                   pEntryOfRxReportQ->RxReportQueueDword1));

           continue;       
       }

       if (RECV_BIG_BUFFER == tmpBufferType)
       {
           //           
           //  Make sure return to the corresponding queue.
           //           
           pFreeBufQ = &pSar->FreeBigBufferQ;
           pTmpFreeSlots = &pRecvDmaQ->RemainingReceiveBigSlots;
           fRxSlotsLowOrNone = pSar->fBigllSlotsLowOrNone;
       }
       else
       {
           //           
           //  Make sure return to the corresponding queue.
           //           
           pFreeBufQ = &pSar->FreeSmallBufferQ;
           pTmpFreeSlots = &pRecvDmaQ->RemainingReceiveSmallSlots;
           fRxSlotsLowOrNone = pSar->fSmallSlotsLowOrNone;
       }

       //
       //  Update the remaining free slot count for the buffer queue.
       //
       (*pTmpFreeSlots)++;

       //
       //	flush the cache.
       //
       NdisFlushBuffer(pRecvBufHead->FlushBuffer, FALSE);

       NdisMUpdateSharedMemory(
           pAdapter->MiniportAdapterHandle,
           pRecvBufHead->Alloc[RECV_BUFFER_ALIGNED].Size,
           pRecvBufHead->Alloc[RECV_BUFFER_ALIGNED].VirtualAddress,
           pRecvBufHead->Alloc[RECV_BUFFER_ALIGNED].PhysicalAddress);


#if    DBG
       (*pdbgTotalUsedRxSlots)++;
#endif // end of DBG


       //
       //  Check if the Vc is valid.
       //
       NdisDprReleaseSpinLock(&pRecvDmaQ->lock);
       pVc = tbAtm155UnHashVc(pAdapter, Vci);
       NdisDprAcquireSpinLock(&pRecvDmaQ->lock);

       if (NULL == pVc)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("There is not a valid VC for %u\n", Vci));

           NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvInvalidVpiVci);

           //
           //  Return the receive buffer to the free pool.
           //
           tbAtm155InsertRecvBufferAtTail(pFreeBufQ, pRecvBufHead, TRUE);

           continue;
       }

       //
       //  BUGFIX 158842
       //
       if (!VC_TEST_FLAG(pVc, (fVC_ACTIVE | fVC_RECEIVE)))
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
				("We don't have a valid VC_BLOCK: 0x%x for VCI: %u\n", pVc, Vci));

           //
           //  Return the receive buffer to the free pool.
           //
           tbAtm155InsertRecvBufferAtTail(pFreeBufQ, pRecvBufHead, TRUE);

           continue;
       }

       pRecvSegInfo = pVc->RecvSegInfo;

       if (pVc->RecvBufType != tmpBufferType)
       {
           DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
               ("Buffer types are not matched with the VC's.\n"));

           //
           //
           //  The buffer type is not matched with the type reported
           //  from Rx report queue, return the receive buffer to the
           //  free pool, then go to handle the next report entry.
           //
           tbAtm155InsertRecvBufferAtTail(pFreeBufQ, pRecvBufHead, TRUE);
           
           continue;
       }

       //
       //  Point to the list which queues the receive buffers until
       //  a whole packet is in (indicates by EOP flag in Rx report queue.)
       //
       pSegCompleting = &pRecvSegInfo->SegCompleting;

       //
       //  Let's check if the data in the receive buffer is healthy.
       //
       if (pEntryOfRxReportQ->RxReportQDWord1.Bad)
       {
           //
           //  Data in the buffer is bad.
           //
           //  1. Release any chained buffers in the VC.
           //  2. Return the current receive buffer back to the buffer pool.
           //
           if (NULL != pSegCompleting->BufListHead)
           {
               NdisDprReleaseSpinLock(&pRecvDmaQ->lock);

               // 
               //  It is possible that receive congestion happened.
               //  Abort the receive buffers, which contain the packet,
               //  into the list.
               // 

               tbAtm155ProcessReturnPacket(
                       pAdapter,
                       pSegCompleting->BufListHead->Packet,
                       pSegCompleting->BufListHead,
                       FALSE);
  
               NdisDprAcquireSpinLock(&pRecvDmaQ->lock);
               //
               //  Check if the buffer has been freed in tbAtm155ProcessReturnPacket()
               //
               if (NULL != pRecvSegInfo)
               {
                   pSegCompleting->BufListHead = NULL;
                   pSegCompleting->BufListTail = NULL;
                   pSegCompleting->BufferCount = 0;
               }

               DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR, ("Recved a bad packet.\n"));
           }

           //
           //  Return the receive buffer to the free pool.
           //
           tbAtm155InsertRecvBufferAtTail(pFreeBufQ, pRecvBufHead, TRUE);

           //
           //      There are for
           //          1. Adapters in ADAPTER_BLOCK
           //          2. VC in VC_BLOCK
           //
           switch (pEntryOfRxReportQ->RxReportQDWord1.Status)
           {
               case Crc32ErrDiscard:
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("CRC32 Error detected, discard packet.\n"));
                   NdisInterlockedIncrement((PLONG)&pVc->StatInfo.RecvCrcError);
                   NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvCrcError);
                   break;

               case LengthErrDiscard:
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("Packet Length Error detected, discard packet.\n"));
                   NdisInterlockedIncrement((PLONG)&pVc->StatInfo.RecvPdusError);
                   NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvPdusError);
				    break;

               case AbortedPktDiscard:
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("Aborted Packet detected, discard packet.\n"));
                   NdisInterlockedIncrement((PLONG)&pVc->StatInfo.RecvPdusError);
                   NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvPdusError);
				    break;

               case SlotCongDiscard:
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("Slot Congestion detected, discard packet.\n"));
                   NdisInterlockedIncrement((PLONG)&pVc->StatInfo.RecvCellsDropped);
                   NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvCellsDropped);
				    break;

               case OtherCellErrDiscard:
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("Other Cell Error detected, discard packet.\n"));
                   NdisInterlockedIncrement((PLONG)&pVc->StatInfo.RecvPdusNoBuffer);
                   NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvPdusNoBuffer);
				    break;

               case ReassemblyTimeout:
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("Reassembly Timeout detected, discard packet.\n"));
                   NdisInterlockedIncrement((PLONG)&pVc->StatInfo.RecvReassemblyErr);
                   NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvReassemblyErr);
				    break;

               case PktTooLongErr:
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("Packet Too Long detected, discard packet.\n"));
                   NdisInterlockedIncrement((PLONG)&pVc->StatInfo.RecvPdusError);
                   NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvPdusError);
				    break;

           }

           continue;
       }

       //
       //	Save Vc info to this receive buffer.
       //
       pRecvBufHead->pVc = pVc;

       if ((pEntryOfRxReportQ->RxReportQDWord1.Sop == 0) &&
           (NULL == pSegCompleting->BufListHead))
       {
           // 
           // The SOP of the packet is lost. 
           // Return this receive buffer to the free pool.
           //
           DBGPRINT(DBG_COMP_SPECIAL, DBG_LEVEL_ERR,
                   ("Lost the SOP of the packet, discard packet.\n"));
           tbAtm155InsertRecvBufferAtTail(pFreeBufQ, pRecvBufHead, TRUE);
           continue;
       }

       if (pEntryOfRxReportQ->RxReportQDWord1.Sop)
       {
           if ((pEntryOfRxReportQ->RxReportQDWord1.Eop) && 
               (1 == pEntryOfRxReportQ->RxReportQDWord1.Status))
           {
               // 
               //  AAL5 Reassembly Timeout.
               //  Return this receive buffer to the free pool.
               //
               DBGPRINT(DBG_COMP_SPECIAL, DBG_LEVEL_ERR,
                       ("Experienced reassembly timeout. discard packet.\n"));
               tbAtm155InsertRecvBufferAtTail(pFreeBufQ, pRecvBufHead, TRUE);
               continue;
           }

       	   NdisDprReleaseSpinLock(&pRecvDmaQ->lock);
           NdisDprAcquireSpinLock(&pVc->lock);

           //
           //  Verify if the VC is still active again for just in case.
           //
           if (!VC_TEST_FLAG(pVc, (fVC_ACTIVE | fVC_RECEIVE)))
           {
               DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
				         ("We don't have a valid VC_BLOCK now: 0x%x for VCI: %u\n", pVc, Vci));

               NdisDprReleaseSpinLock(&pVc->lock);
               NdisDprAcquireSpinLock(&pRecvDmaQ->lock);

               //
               //  Return the receive buffer to the free pool.
               //
               tbAtm155InsertRecvBufferAtTail(pFreeBufQ, pRecvBufHead, TRUE);
               continue;
           }

           //
           //	Reference the VC
           //
           tbAtm155ReferenceVc(pVc);

           NdisDprReleaseSpinLock(&pVc->lock);
           NdisDprAcquireSpinLock(&pRecvDmaQ->lock);

           //
           //  This flag indicates if the slot being reported contains
           //  the first byte of an AAL5 packet.
           //  Also, it may be used by the driver to detect error conditions
           //  which may not have been reported due to lack of slots
           //  (receive congestion).
           //
           if (NULL != pSegCompleting->BufListHead)
           {
               NdisDprReleaseSpinLock(&pRecvDmaQ->lock);

               // 
               //  It is possible that receive congestion happened.
               //  Abort the receive buffers in the list.
               // 
               tbAtm155ProcessReturnPacket(
                   pAdapter,
                   pSegCompleting->BufListHead->Packet,
                   pSegCompleting->BufListHead,
                   FALSE);

               NdisDprAcquireSpinLock(&pRecvDmaQ->lock);

               //
               //  Check if the buffer has been freed in tbAtm155ProcessReturnPacket()
               //
               if (NULL != pRecvSegInfo)
               {
                   pSegCompleting->BufListHead = NULL;
                   pSegCompleting->BufListTail = NULL;
                   pSegCompleting->BufferCount = 0;
               }

               NdisInterlockedIncrement((PLONG)&pVc->StatInfo.RecvCellsDropped);
               NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvCellsDropped);
           }

           //
           //	Grab a packet for the receive.
           //
           Packet = pRecvBufHead->Packet;

           //
           //	Save the first receive buffer to be used with the packet.
           //
           RECV_PACKET_RESERVED_FROM_PACKET(Packet)->RecvHeader = pRecvBufHead;

           //
           //	Chain the first buffer to the packet.
           //
           NdisChainBufferAtBack(Packet, pRecvBufHead->NdisBuffer);
       }

       if (pEntryOfRxReportQ->RxReportQDWord1.Eop)
       {
           // 
           //  Need to Adjust the last receive buffer size.
           //  LongWord size.
           // 
           LastBufferLength = pEntryOfRxReportQ->RxReportQDWord1.Size;

           if (LastBufferLength)
           {
               // 
               //  Convert to Byte length.
               // 
               LastBufferLength <<= 2;
           }
           else
           {
               // 
               //  The value 0 is used to represent exactly 16KB
               //  of valid data.
               // 
#if DBG
               DbgPrint("TBATM155: tbAtm155RxInterruptOnCompletion: pEntry %p, LastBufLen zero\n",
               		pEntryOfRxReportQ);
#endif // DBG
               LastBufferLength = BLOCK_16K;
           }
           
           ASSERT (LastBufferLength >=  sizeof(AAL5_PDU_TRAILER));
                      
           if (pEntryOfRxReportQ->RxReportQDWord1.Sop)
           {
               //
               // If there is only 1 buffer for this packet,
               // adjust the length of NdisBuffer.
               //
               NdisAdjustBufferLength(							
                   pRecvBufHead->NdisBuffer,
                   LastBufferLength);
           }
           else
           {
               //
               // If there are more than 1 buffer for this packet,
               // adjust the length of FlushBuffer.
               //
               NdisAdjustBufferLength(							
                   pRecvBufHead->FlushBuffer,
                   LastBufferLength);
           }
       }

       tbAtm155InsertRecvBufferAtTail(pSegCompleting, pRecvBufHead, FALSE);

       //
       //  Get the descriptor for this packet.
       //
       Packet = pSegCompleting->BufListHead->Packet;

       pSegCompleting->BufListHead->Last = pRecvBufHead;

       if (pEntryOfRxReportQ->RxReportQDWord1.Sop == 0)
       {
           //
           //	Chain the buffer to the packet.
           //
           NdisChainBufferAtBack(Packet, pRecvBufHead->FlushBuffer);
       }

       if (pEntryOfRxReportQ->RxReportQDWord1.Eop == 0)
       {
           continue;
       }           

       //
       //	We support AAL5 only
       //
       //
       //	Get the packet information.
       //  (09/15/97) 
       //  WARNING: the returned PhysicalBufferCount is not
       //           the "real" buffer count for the requested packet.
       //           DO NOT use PhysicalBufferCount.
       //
       NdisQueryPacket(
           Packet,
           &PhysicalBufferCount,
           NULL,
           NULL,
           &PacketLength);

       dbgLogRecvPacket(
           pVc->DebugInfo,
           Packet,
           PhysicalBufferCount,
           PacketLength,
           'nikp');

		Reserved = RECV_PACKET_RESERVED_FROM_PACKET(Packet);

       //
       //	The BufferCount field is actually the number of buffers
       //	that were allocated to receive the packet.  If it is
       //	greater than one then the AAL5 PDU trailer and/or padding
       //	may be split between buffers.  We first optimize for
       //	packets that contain only one buffer.
       //
       do
       {
           NdisZeroMemory (&Trailer, sizeof(AAL5_PDU_TRAILER));
           if (1 == pSegCompleting->BufferCount)
           {
               PUCHAR  BufferVa;
               UINT    BufferLength;

               dbgLogRecvPacket(pVc->DebugInfo, Packet, (ULONG)pRecvBuf, 1, '   1');

               //
               //	Get the virtual address and length of the first buffer.
               //

#if BUILD_W2K
               NdisQueryBuffer(
                       pRecvBufHead->NdisBuffer,
                       &BufferVa,
                       &BufferLength);

#else
               NdisQueryBufferSafe(
                       pRecvBufHead->NdisBuffer,
                       &BufferVa,
                       &BufferLength,
                       NormalPagePriority);
#endif
               if (BufferVa == NULL)
               {
                   //
                   // If we cannot get BufferVa due to low resources, just return the packet
                   //  
                   fQueryBufferFail = TRUE;
                   break;
               
               }
               dbgLogRecvPacket(pVc->DebugInfo, Packet, BufferLength, 0, '   2');

               //
               //	Copy the AAL5 PDU trailer from the end of the buffer.
               //
               NdisMoveMemory(
                   &Trailer,
                   BufferVa + BufferLength - sizeof(AAL5_PDU_TRAILER),
                   sizeof(AAL5_PDU_TRAILER));

               //
               //	Need to convert the length into a readable form.
               //
               Trailer.Length = TBATM155_SWAP_USHORT(Trailer.Length);
               Trailer.CRC = TBATM155_SWAP_ULONG(Trailer.CRC);

               //
               //  Check if we get a corrupted packet.
               //
               if ((LastBufferLength < (UINT)Trailer.Length) ||
                   (LastBufferLength > (UINT)(Trailer.Length + MAX_APPEND_BYTES)))
               {
                   fCorruptedPacket = TRUE;
               }
    
               //
               //	Now we need to determine the packet size with out the
               //	trailer and padding.  This is the value that is in
               //	the Trailer.Length.
               //
               NdisAdjustBufferLength(
                   pRecvBufHead->NdisBuffer,
                   Trailer.Length);

               dbgLogRecvPacket(pVc->DebugInfo, Packet, Trailer.Length, 0, '  lt');
           }
           else
           {
               ///
               //	Things are a little more complicated....
               //	Since there is more than one buffer it means that the
               //	padding and/or AAL5 pdu trailer can be split between
               //  them.
               ///
               LastBuffer = pSegCompleting->BufListTail->FlushBuffer;

               //
               //	First look for a packet that has a last buffer that is
               //	large enough for the AAL5_PDU_TRAILER.
               //

#if BUILD_W2K
               NdisQueryBuffer(
                   LastBuffer,
                   &LastBufferVa,
                   &LastBufferLength);

#else
               NdisQueryBufferSafe(
                   LastBuffer,
                   &LastBufferVa,
                   &LastBufferLength,
                   NormalPagePriority);
#endif
               if (LastBufferVa == NULL)
               {
                   //
                   // Cannot get BufferVa due to low resources
                   //
                   fQueryBufferFail = TRUE;
                   break;
               }
                   
               if (LastBufferLength >= sizeof(AAL5_PDU_TRAILER))
               {
                   dbgLogRecvPacket(pVc->DebugInfo, Packet, LastBufferLength, 0, '   2');

                   //
                   //	The last buffer has all of the AAL5 pdu trailer.
                   //  Copy it in at once.
                   //
                   NdisMoveMemory(
               	    &Trailer,
               	    (LastBufferVa + LastBufferLength) - sizeof(AAL5_PDU_TRAILER),
               	    sizeof(AAL5_PDU_TRAILER));
               }
               else
               {
                   dbgLogRecvPacket(pVc->DebugInfo, Packet, LastBufferLength, 0, '   3');

                   //
                   //	First grab whatever is in the last buffer and copy
                   //	it to our local trailer.
                   //
                   NdisMoveMemory(
               	    (PUCHAR)&Trailer + (sizeof(AAL5_PDU_TRAILER) - LastBufferLength),
               	    LastBufferVa,
           	        LastBufferLength);

                   //
                   //	We need to get remaining trailer from the next to last
                   //	buffer...  but the next to last buffer can either be
                   //	represented by the NdisBuffer or FlushBuffer.  If
                   //	the PhysicalBufferCount is 2 then it's the NdisBuffer,
                   //	if not it's the FlushBuffer.
                   //
                   if (2 == pSegCompleting->BufferCount)
                   {    
                       prevLastBuffer = pSegCompleting->BufListTail->Prev->NdisBuffer;
                   }
                   else
                   {    
                       prevLastBuffer = pSegCompleting->BufListTail->Prev->FlushBuffer;
                   }

                   //
                   //  Get the next to last buffers virtual address and
                   //  length.
                   //
#if BUILD_W2K
               NdisQueryBuffer(
                    prevLastBuffer,
                    &prevLastBufferVa,
                    &prevLastBufferLength);

#else
               NdisQueryBufferSafe(
                    prevLastBuffer,
                    &prevLastBufferVa,
                    &prevLastBufferLength,
                    NormalPagePriority);
#endif

                   if (prevLastBufferVa == NULL)
                   {
                       //
                       // If cannot get preLastBufferVa due to low resources
                       // 
                       fQueryBufferFail = TRUE;
                       break;
                   }

                   //
                   //	Copy the first part of the trailer from this buffer
                   //	into our local.
                   //
                   NdisMoveMemory(
                       &Trailer,
           	           (PUCHAR)((prevLastBufferVa + prevLastBufferLength) -
                       (sizeof(AAL5_PDU_TRAILER) - LastBufferLength)),
                       sizeof(AAL5_PDU_TRAILER) - LastBufferLength);
               }

               //
               //	Need to convert the length into a readable form.
               //
               Trailer.Length = TBATM155_SWAP_USHORT(Trailer.Length);
               Trailer.CRC = TBATM155_SWAP_ULONG(Trailer.CRC);

               dbgLogRecvPacket(pVc->DebugInfo, Packet, Trailer.Length, 0, '  lt');

               //
               //	Determine the amount of packet data that needs to be
               //	dumped before we indicate it up.
               //

#if DBG
        
               if (PacketLength < Trailer.Length)
               {
                   DBGPRINT(DBG_COMP_SPECIAL, DBG_LEVEL_ERR,
                       ("tbAtm155RxIOC: PacketLength (%x), Trailer.Length (%x).\n",
                         PacketLength, Trailer.Length));

                   DBGBREAK(DBG_COMP_SPECIAL, DBG_LEVEL_ERR);
               }

#endif

               TrimAmount = PacketLength - Trailer.Length;

               dbgLogRecvPacket(pVc->DebugInfo, Packet, TrimAmount, 0, '  at');

               //
               //  See if the last buffer contains all the data that
               //  needs to be trimmed.
               //
               if (TrimAmount < LastBufferLength)
               {
                   //
                   //	Adjust the last packet.
                   //
                   NdisAdjustBufferLength(
                       LastBuffer,
                       LastBufferLength - TrimAmount);

                   dbgLogRecvPacket(
                       pVc->DebugInfo, 
                       Packet, 
                       LastBufferLength - TrimAmount, 
                       0,
                       'blat');
               }
               else
               {
                   //
                   //	Drop the last buffer from the packet.
                   //	This is safe and we don't need to do anything with
                   //	the buffer since it is saved in the RECV_BUFFER_HEADER.
                   //
                   NdisUnchainBufferAtBack(Packet, &LastBuffer);
    
                   //
                   //	Determine how much data needs to be trimmed from
                   //	the next to last.
                   //
                   TrimAmount -= LastBufferLength;
    
                   //
                   //  If there were only two buffers then we need to
                   //  adjust the NdisBuffer since this is the one that
                   //  is chained to the Packet.  If there are more than
                   //  two buffers in the Packet then we need to adjust
                   //  the FlushBuffer.
                   //
                   if (2 == pSegCompleting->BufferCount)
                   {
                       prevLastBuffer = pSegCompleting->BufListTail->Prev->NdisBuffer;
                   }
                   else
                   {
                       prevLastBuffer = pSegCompleting->BufListTail->Prev->FlushBuffer;
                   }

                   //
                   //	Get the next to last buffers virtual address and length.
                   //

#if BUILD_W2K
               NdisQueryBuffer(
                    prevLastBuffer,
                    NULL,
                    &prevLastBufferLength);

#else
               NdisQueryBufferSafe(
                   prevLastBuffer,
                   NULL,
                   &prevLastBufferLength,
                    NormalPagePriority);
#endif

                   //
                   //	Adjust the next to last (now the last) buffer
                   //	in the packet.
                   //
                   NdisAdjustBufferLength(
                       prevLastBuffer,
                       prevLastBufferLength - TrimAmount);

                   dbgLogRecvPacket(pVc->DebugInfo, Packet, LastBufferLength - TrimAmount, 0, 'atbl');
               }
    
           } // end of if (1 == pSegCompleting->BufferCount)
       }
       while (FALSE);           

       //
       //    Save the trailer information in the packet's OOB
       //    informaton.
       //
       NDIS_GET_PACKET_MEDIA_SPECIFIC_INFO(
             Packet,
             &pMediaSpecificInfo,
             &MediaSpecificInfoSize);

       //
       //    Verify the size class id.
       //
       ASSERT(TBATM155_PROTOCOL_RESERVED == MediaSpecificInfoSize);
       ASSERT(NdisClassAtmAALInfo == pMediaSpecificInfo->ClassId);

       //
       //	Get a pointer to the AAL OOB informaton.
       //
       pAtmOobInfo = (PATM_AAL_OOB_INFO)pMediaSpecificInfo->ClassInformation;
       pAtmOobInfo->AalType = AAL_TYPE_AAL5;

       //
       //	Initialize the AAL5 OOB information.
       //
       pAtmOobInfo->ATM_AAL5_INFO.CellLossPriority = 0;
       pAtmOobInfo->ATM_AAL5_INFO.UserToUserIndication =
                                       Trailer.UserToUserIndication;
       pAtmOobInfo->ATM_AAL5_INFO.CommonPartIndicator =
                                       Trailer.CommonPartIndicator;

       //
       //	This will force a NdisQueryPacket to recalc the
       //	packet information.  Needed for a correct PacketLength.
       //
       NdisRecalculatePacketCounts(Packet);

       NdisQueryPacket(
           Packet,
           NULL,
           NULL,
           NULL,
           &PacketLength);

       if ((PacketLength < Trailer.Length) ||
           (MAX_AAL5_PDU_SIZE < PacketLength) || 
           (0 == PacketLength) ||
           (TRUE == fCorruptedPacket) ||
           (TRUE == fQueryBufferFail))  // Cannot get buffer va due to low resources
       {
           //
           //  Reset this flag for next packet.
           //
           fCorruptedPacket = FALSE;
           fQueryBufferFail = FALSE;

           //
           //  Received a corrupted packet. Drop it.
           //
           NdisDprReleaseSpinLock(&pRecvDmaQ->lock);

           tbAtm155ProcessReturnPacket(
                   pAdapter,
                   pSegCompleting->BufListHead->Packet,
                   pSegCompleting->BufListHead,
                   FALSE);

           NdisDprAcquireSpinLock(&pRecvDmaQ->lock);

           //
           //  Check if the buffer has been freed in tbAtm155ProcessReturnPacket()
           //
           if (NULL != pRecvSegInfo)
           {
               pSegCompleting->BufListHead = NULL;
               pSegCompleting->BufListTail = NULL;
               pSegCompleting->BufferCount = 0;
           }

           NdisInterlockedIncrement((PLONG)&pVc->StatInfo.RecvCellsDropped);
           NdisInterlockedIncrement((PLONG)&pAdapter->StatInfo.RecvCellsDropped);

           continue;
           
       }

       //
       //	Keep track of some statistics.
       //
       pAdapter->StatInfo.RecvPdusOk++;
       pVc->StatInfo.RecvPdusOk++;

       pAdapter->StatInfo.RecvCellsOk += pSegCompleting->BufferCount;
       pVc->StatInfo.RecvCellsOk += pSegCompleting->BufferCount;

       //
       //  Initialize the queue to ready to receive the next packet
       //  for this VC.
       //
       pSegCompleting->BufListHead = NULL;
       pSegCompleting->BufListTail = NULL;
       pSegCompleting->BufferCount = 0;

       //
       //	We need to mark the packet so that return packets knows we are in the receive.
       //
       NdisDprReleaseSpinLock(&pRecvDmaQ->lock);

       Status = NDIS_GET_PACKET_STATUS(Packet);


       NdisDprAcquireSpinLock(&pVc->lock);
       pVc->PktsHoldsByNdis++;
       NdisDprReleaseSpinLock(&pVc->lock);

       NdisMCoIndicateReceivePacket(pVc->NdisVcHandle, &Packet, 1);

       if (NDIS_STATUS_RESOURCES == Status)
       {
           tbAtm155ProcessReturnPacket(pAdapter, Packet, Reserved->RecvHeader, TRUE);
       }

       NdisDprAcquireSpinLock(&pRecvDmaQ->lock);

       //
       //	We have indicated a packet up so we need to return this fact.
       //
       fReturnStatus = TRUE;

       dbgLogRecvPacket(pVc->DebugInfo, NULL, 0, 0, ' CDR');

       DBGPRINT(DBG_COMP_RXIOC, DBG_LEVEL_INFO,
		    ("TbAtm155RxIOC: Pkt %x VC:%lx(OK)\n", Packet, pVc->VpiVci.Vci));

   } // end of lookup Rx report queue.

   pRecvDmaQ->PrevRxReportQIndex = CurrentRxReportQIdx;

   NdisDprReleaseSpinLock(&pRecvDmaQ->lock);

   //
   //  Post more receive buffers to both pools if there are 
   //  available Rx slots.
   //
   tbAtm155QueueRecvBuffersToReceiveSlots(pAdapter, RECV_SMALL_BUFFER);
   tbAtm155QueueRecvBuffersToReceiveSlots(pAdapter, RECV_BIG_BUFFER);

   return (fReturnStatus);

}



VOID
tbAtm155FreeReceiveBuffer(
   IN OUT PRECV_BUFFER_HEADER  *ppRecvHeaderHead,
   IN  PADAPTER_BLOCK          pAdapter,
   IN  PRECV_BUFFER_HEADER     pRecvHeader
   )
/*++

Routine Description:

   This routine will remove a receive header from the receive buffer
   information's global list and free it back to the pool.  If at the end
   of freeing the receive buffers the pool has all of its buffers then the
   pool is freed.

Arguments:

   ppRecvHeaderHead    -   Pointer to the memory holding the head pointer.
   pAdapter            -   Pointer to the ADAPTER_BLOCK.
   pRecvHeader         -   Pointer to the receive buffer to free.

Return Value:

   None.

--*/
{
   PRECV_BUFFER_HEADER pRecvHeaderHead = *ppRecvHeaderHead;
   PRECV_BUFFER_HEADER pNextRecvHeader = pRecvHeader->Next;
   PRECV_BUFFER_HEADER pFreeRecvHeader;
   PRECV_BUFFER_POOL   pRecvPool;

   //
   //	Save the receive header that we are trying to free.
   //
   pFreeRecvHeader = pRecvHeader;

   //
   //	Remove the receive header from the list.
   //
   if (pRecvHeaderHead == pRecvHeader)
   {
       ///
       //	The receive header that we are removing is at the head.
       ///

       //
       //	Set the head pointer to the next recv header.
       //
       pRecvHeaderHead = pNextRecvHeader;

       //
       //	If the list is not empty then terminate the
       //	previous pointer.
       //
       if (pRecvHeaderHead != NULL)
       {
           pRecvHeaderHead->Prev = NULL;
       }
   }
   else
   {
       //
       //	Receive header is somewhere in the middle or end.
       //

       //
       //	Setup the previous nodes forward pointer.
       //
       pRecvHeader->Prev->Next = pRecvHeader->Next;

       //
       //	If there is a next node, setup it's previous pointer.
       //
       if (pRecvHeader->Next != NULL)
       {
           pRecvHeader->Next->Prev = pRecvHeaderHead->Prev;
       }

       //
       //	We need to fix up the next receive header pointer so
       //	that we will get the "true next" receive header.
       //
       pNextRecvHeader = pRecvHeader->Prev;
   }


   //
   //	Place this header on the pool's list.
   //
   pRecvPool = pFreeRecvHeader->Pool;

   ASSERT(RECV_BUFFER_POOL_SIG == pRecvPool->Signature);

   pFreeRecvHeader->Prev = NULL;
   pFreeRecvHeader->Next = pRecvPool->FreeBuffersHead;
   pRecvPool->FreeBuffersHead = pFreeRecvHeader;

   pRecvPool->BuffersFreed++;

   //
   //  If the pool has all of it's receive buffers then we need
   //  to go ahead and free it. Otherwise, wait until all of the
   //  buffers of this pool are returned.
   //
   if (pRecvPool->BuffersFreed == pRecvPool->ReceiveBufferCount)
   {
       tbAtm155FreeReceiveBufferPool(pAdapter, pRecvPool);
   }

   *ppRecvHeaderHead = pRecvHeaderHead;

}



VOID
tbAtm155ProcessReturnPacket(
    IN  PADAPTER_BLOCK      pAdapter,
    IN  PNDIS_PACKET        Packet,
    IN  PRECV_BUFFER_HEADER pRecvHeader,
    IN  BOOLEAN             fAdjustPktsHoldsByNdis
   )
/*++

Routine Description:

   This routine handles received buffers that are returned by NDIS.
   These are returned to the corresponding pools and queues in order
   to be re-used later.

Arguments:

   pAdapter            -   Pointer to the ADAPTER_BLOCK.
   Packet              -   Packet being returned
   pRecvHeader         -   Pointer to the receive buffer to free.

Return Value:

   None.

--*/
{
   PRECV_BUFFER_HEADER ptmpRecvHeader;
   PRECV_BUFFER_HEADER pNextRecvHeader;
   PVC_BLOCK           pVc = pRecvHeader->pVc;
   PRECV_BUFFER_INFO   pRecvInfo = pRecvHeader->Pool->RecvBufferInfo;
   UCHAR               BufferType = pVc->RecvBufType;
   PSAR_INFO           pSar = pAdapter->HardwareInfo->SarInfo;
   PRECV_BUFFER_QUEUE  pBufferQ;

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("==>tbAtm155ProcessReturnPacket\n"));

   ASSERT((pRecvInfo == NULL) || (RECV_BUFFER_INFO_SIG == pRecvInfo->Signature));

   //
   //	Reinitialize the packet.
   //
   NdisReinitializePacket(Packet);

   if (BufferType == RECV_BIG_BUFFER)
   {
       pBufferQ = &pSar->FreeBigBufferQ;
   }
   else
   {
       pBufferQ = &pSar->FreeSmallBufferQ;
   }

   //
   //	Walk the list and reset the buffer lengths.
   //
   for (ptmpRecvHeader = pRecvHeader; 
        NULL != ptmpRecvHeader; 
        ptmpRecvHeader = pNextRecvHeader)
   {
       dbgLogRecvPacket(pVc->DebugInfo, Packet, (ULONG)ptmpRecvHeader, cReturnedHeaders, 'dher');

       //
       //	Get a pointer to the next receive header.
       //
       pNextRecvHeader = ptmpRecvHeader->Next;

       tbAtm155InsertRecvBufferAtTail(pBufferQ, ptmpRecvHeader, TRUE);

   } // end of FOR

   dbgLogRecvPacket(pVc->DebugInfo, pRecvInfo, pBufferQ->BufferCount,  0, 'tncp');

   if (!ADAPTER_TEST_FLAG(pAdapter, fADAPTER_RESET_IN_PROGRESS))
   {
       tbAtm155QueueRecvBuffersToReceiveSlots(pAdapter, (UCHAR)BufferType);
   }

   //
   //	Decrement the outstanding references on the VC.
   //
   NdisAcquireSpinLock(&pVc->lock);
   tbAtm155DereferenceVc(pVc);

   if (TRUE == fAdjustPktsHoldsByNdis) 
   {
       pVc->PktsHoldsByNdis--;
   }

   if ((1 == pVc->References) && (VC_TEST_FLAG(pVc, fVC_DEACTIVATING)))
   {
       NdisReleaseSpinLock(&pVc->lock);

       //
       //	Finish VC deactivation.
       //

       DBGPRINT(DBG_COMP_SPECIAL, DBG_LEVEL_INFO,
           ("calling TbAtm155DeactivateVcComplete\n"));
       TbAtm155DeactivateVcComplete(pVc->Adapter, pVc);
   }
   else
   {
       NdisReleaseSpinLock(&pVc->lock);
   }

   dbgLogRecvPacket(pVc->DebugInfo, Packet, (ULONG)pVc, 0, 'PTER');

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("<==tbAtm155ProcessReturnPacket\n"));

}


VOID
TbAtm155ReturnPacket(
   IN  NDIS_HANDLE     MiniportAdapterContext,
   IN  PNDIS_PACKET    Packet
   )
/*++

Routine Description:

   This is the handler called by NDIS to return a received packet that
   was indicated up to protocols.

Arguments:

   MiniportAdapterContext -  Pointer to our Adapter block
   Packet                 -  Pointer to packet being returned

Return Value:

   None.

--*/
{
   PADAPTER_BLOCK          pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
   PRECV_BUFFER_HEADER     pRecvHeader = RECV_PACKET_RESERVED_FROM_PACKET(Packet)->RecvHeader;


   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("==>TbAtm155ReturnPacket\n"));

   tbAtm155ProcessReturnPacket(pAdapter, Packet, pRecvHeader, TRUE);

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("<==TbAtm155ReturnPacket\n"));

}




VOID
TbAtm155AllocateComplete(
   IN  NDIS_HANDLE             MiniportAdapterContext,
   IN  PVOID                   VirtualAddress,
   IN  PNDIS_PHYSICAL_ADDRESS  PhysicalAddress,
   IN  ULONG                   Length,
   IN  PVOID                   Context
   )
/*++

Routine Description:

   This is the handler for the async shared memory allocations.

Arguments:

   MiniportAdapterContext  -   Pointer to the ADAPTER_BLOCK.
   VirtualAddress          -   Pointer to the shared memory va.
   PhysicalAddress         -   Pointer to the shared memory pa.
   Length                  -   Length of the allocated shared memory block.
   Context                 -   Pointer to the receive buffer pool this
                               shared memory block is for.

Return Value:

   None.

--*/
{
   PTRANSMIT_CONTEXT       pTransmitContext;

   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("==>TbAtm155AllocateComplete\n"));

   //
   // In this routine, we will handle async shared memory allocation not only 
   // requested by tbAtm155AllocateReceiveSegment, but also requested by
   // tbAtm155AllocateTransmitSegment.
   //
   pTransmitContext = (PTRANSMIT_CONTEXT)Context;

   if(pTransmitContext->Signature != XMIT_BUFFER_SIG)
   {
       TbAtm155AllocateRecvBufferComplete(
                   MiniportAdapterContext,
                   VirtualAddress,
                   PhysicalAddress,
                   Length,
                   Context);
   }
   else
   {
       TbAtm155AllocateXmitBufferComplete(
                   MiniportAdapterContext,
                   VirtualAddress,
                   PhysicalAddress,
                   Length,
                   pTransmitContext);
   }
       
   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_INFO,
       ("<==TbAtm155AllocateComplete\n"));
}


VOID
TbAtm155AllocateRecvBufferComplete(
   IN  NDIS_HANDLE             MiniportAdapterContext,
   IN  PVOID                   VirtualAddress,
   IN  PNDIS_PHYSICAL_ADDRESS  PhysicalAddress,
   IN  ULONG                   Length,
   IN  PVOID                   Context
   )
{
   PADAPTER_BLOCK          pAdapter = (PADAPTER_BLOCK)MiniportAdapterContext;
   PRECV_BUFFER_HEADER     pRecvHeader;
   ULONG                   TotalBufferSize;
   PUCHAR                  AlignedVa;
   NDIS_PHYSICAL_ADDRESS   AlignedPa;
   NDIS_STATUS             Status = NDIS_STATUS_SUCCESS;
   PRECV_BUFFER_POOL       pRecvPool;
   PRECV_BUFFER_INFO       pRecvInfo;
   PVC_BLOCK               pVc;
   BOOLEAN                 fActivateVcComplete = FALSE;

   pRecvPool = (PRECV_BUFFER_POOL)Context;
   pRecvInfo = pRecvPool->RecvBufferInfo;
   pVc = pRecvInfo->pVc;

   ASSERT(pVc != NULL);

   ASSERT(RECV_BUFFER_INFO_SIG == pRecvInfo->Signature);
   ASSERT(RECV_BUFFER_POOL_SIG == pRecvPool->Signature);

   do
   {
       //
       //	First we need to see what this allocation was for....
       //
       if (RECV_POOL_TEST_FLAG(pRecvPool, fRECV_POOL_ALLOCATING_BLOCK))
       {
           //
           //	Did we get the big chunk allocated?
           //
           if (NULL != VirtualAddress)
           {
               //
               //	Initialize the receive pool's allocation information.
               //
               pRecvPool->VirtualAddress = VirtualAddress;
               pRecvPool->PhysicalAddress = *PhysicalAddress;
               pRecvPool->Size = Length;

               //
               //	Initialize the receive pool.
               //
               Status = tbAtm155InitializeBlockReceivePool(pRecvPool);
               if (NDIS_STATUS_SUCCESS != Status)
               {
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("Unable to initialize the receive block\n"));

                   if (NULL != pRecvPool)
                   {
                        tbAtm155FreeReceiveBufferPool(pAdapter, pRecvPool);
                   }
                   break;
               }

               //
               //	Place the receive pool's buffers onto the receive info's
               //	free buffer list.
               //
               tbAtm155QueueReceivePoolToSarInfo(pRecvInfo, pRecvPool);

               //
               //	Do we need to complete the VC activation?
               //
               if (RECV_INFO_TEST_FLAG(pRecvInfo, fRECV_INFO_COMPLETE_VC_ACTIVATION))
               {

                   ASSERT(NULL != pRecvInfo->pVc);

                   NdisAcquireSpinLock(&pVc->lock);

                   if (VC_TEST_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES))
                   {
                       //
                       //  Got "Out Of Memory Resources" status when
                       //  allocating padtrailerbuffers.
                       //
                       VC_CLEAR_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES);

                       NdisReleaseSpinLock(&pVc->lock);

                       Status = NDIS_STATUS_RESOURCES;
                       break;
                   }

                   VC_SET_FLAG(pVc, fVC_RECEIVE);
                   VC_CLEAR_FLAG(pVc, fVC_ALLOCATING_RXBUF);

                   //
                   //	Clear this flag.
                   //
                   RECV_INFO_CLEAR_FLAG(
                       pRecvInfo,
                       fRECV_INFO_COMPLETE_VC_ACTIVATION);

                   //
                   //  Check if we are still waiting for PadTrailerBuffers
                   //  allocated.
                   // 
                   if (!VC_TEST_FLAG(pVc, fVC_ALLOCATING_TXBUF))
                   {
                       //
                       //  Done with all of memeory allocation.
                       //  Complete the VC.
                       //
                       fActivateVcComplete = TRUE;
                   }

                   NdisReleaseSpinLock(&pVc->lock);
                   break;
               }
           }
           else
           {
               ///
               //	We need to allocate individual receive buffers.
               ///

               //
               //	We need to readjust the size of the recieve header, with
               //	respect to the alignment, to take into consideration another
               //	ALLOCATION_INFO struct.  this structure will hold the original
               //	virtual address, physical address and size that is returned for
               //	each, individual buffer allocation.
               //
               pRecvPool->ReceiveHeaderSize =
                   (((sizeof(RECV_BUFFER_HEADER) + sizeof(ALLOCATION_INFO) +
                   gDmaAlignmentRequired - 1) / gDmaAlignmentRequired) *
                   gDmaAlignmentRequired);

               //
               //	Determine the actual size for each shared memory allocation.
               //
               TotalBufferSize = pRecvPool->ReceiveHeaderSize +
                                   pRecvPool->ReceiveBufferSize;

               //
               //	Mark the receive pool as allocating an individual buffer.
               //	And clear the block alloc flag.
               //
               RECV_POOL_SET_FLAG(pRecvPool, fRECV_POOL_ALLOCATING_INDIVIDUAL);
               RECV_POOL_CLEAR_FLAG(pRecvPool, fRECV_POOL_ALLOCATING_BLOCK);

               //
               //	Call the async allocate for the individual buffer.
               //
               Status = NdisMAllocateSharedMemoryAsync(
                           pAdapter->MiniportAdapterHandle,
                           TotalBufferSize,
                           TRUE,
                           pRecvPool);
               if (NDIS_STATUS_PENDING != Status)
               {
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("Unable to allocate shared memory async for an individual buffer\n"));
                   
                   if (NULL != pRecvPool)
                   {
                        tbAtm155FreeReceiveBufferPool(pAdapter, pRecvPool);
                   }

                   break;
               }
           }
       }
       else if (RECV_POOL_TEST_FLAG(pRecvPool, fRECV_POOL_ALLOCATING_INDIVIDUAL))
       {
           if (VirtualAddress != NULL)
           {
               //
               //	Initialize the receive buffer.
               //
               tbAtm155BuildIndividualReceiveHeader(
                       &pRecvHeader,
                       &AlignedVa,
                       &AlignedPa,
                       VirtualAddress,
                       *PhysicalAddress,
                       Length);

               //
               //	Finish the common buffer initialization.
               //
               Status = tbAtm155InitializeReceiveBuffer(
                           pRecvPool,
                           pRecvHeader,
                           AlignedVa,
                           AlignedPa,
                           pRecvPool->ReceiveBufferSize);
               if (NDIS_STATUS_SUCCESS != Status)
               {
                   DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                       ("tbAtm155InitializeReceiveBuffer failed for individual allocation\n"));

                   //
                   //	Deallocate the last shared memory allocation now.
                   //	If any other individual buffers were allocted they are already
                   //	queued to the receive pool and will be cleaned up when we
                   //	deallocate the receive pool.
                   //
                   NdisMFreeSharedMemory(
                       pAdapter->MiniportAdapterHandle,
                       Length,
                       TRUE,
                       VirtualAddress,
                       *PhysicalAddress);

                   break;
               }

               //
               //	If we have all of the buffer we need to satisfy the
               //	the pool allocation then we can add this to the
               //	receive information.
               //
               if (pRecvPool->BuffersFreed == pRecvPool->ReceiveBufferCount)
               {
                   tbAtm155QueueReceivePoolToSarInfo(pRecvInfo, pRecvPool);

                   //
                   //	Do we need to complete the VC activation?
                   //
                   if (RECV_INFO_TEST_FLAG(pRecvInfo, fRECV_INFO_COMPLETE_VC_ACTIVATION))
                   {
                       ASSERT(NULL != pRecvInfo->pVc);

                       NdisAcquireSpinLock(&pVc->lock);

                       if (VC_TEST_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES))
                       {
                           //
                           //  Got "Out Of Memory Resources" status when
                           //  allocating PadTrailerBuffers.
                           //
                           VC_CLEAR_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES);

                           NdisReleaseSpinLock(&pVc->lock);

                           Status = NDIS_STATUS_RESOURCES;
                           break;
                       }

                       VC_SET_FLAG(pVc, fVC_RECEIVE);
                       VC_CLEAR_FLAG(pVc, fVC_ALLOCATING_RXBUF);

                       //
                       //	Clear this flag.
                       //
                       RECV_INFO_CLEAR_FLAG(
                           pRecvInfo,
                           fRECV_INFO_COMPLETE_VC_ACTIVATION);

                       //
                       //  Check if we are still waiting for PadTrailerBuffers
                       //  allocated.
                       // 
                       if (!VC_TEST_FLAG(pVc, fVC_ALLOCATING_TXBUF))
                       {
                           //
                           //  Done with all of memeory allocation.
                           //  Complete the VC.
                           //
                           fActivateVcComplete = TRUE;
                       }

                       NdisReleaseSpinLock(&pVc->lock);
                       break;
                   }
               }
               else
               {
                   //
                   //	We need more buffers! Fire off another async
                   //	allocate call.
                   //
                   Status = NdisMAllocateSharedMemoryAsync(
                               pAdapter->MiniportAdapterHandle,
                               Length,
                               TRUE,
                               pRecvPool);
                   if (NDIS_STATUS_PENDING != Status)
                   {
                       DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR,
                           ("Unable to allocate shared memory async for an individual buffer\n"));

                       if (NULL != pRecvPool)
                       {
                            tbAtm155FreeReceiveBufferPool(pAdapter, pRecvPool);
                       }
                       break;
                   }
               }
           }
           else
           {
               //
               //	We failed an individual allocation, set the failure
               //	status.
               //
               DBGPRINT(DBG_COMP_RECV, DBG_LEVEL_ERR, ("NDIS_STATUS_RESOURCES.\n"));
               if (NULL != pRecvPool)
               {
                   tbAtm155FreeReceiveBufferPool(pAdapter, pRecvPool);
               }

               Status = NDIS_STATUS_RESOURCES;
           }
       }
   } while (FALSE);

   //
   //   1. If we are done with Activate VC.
   //	2. If the we failed somwhere then we need to cleanup the receive
   //	   pool.
   //
   if (TRUE == fActivateVcComplete)
   {
       // Yes, it is done. Let's call ActivateVcCompete.
       TbAtm155ActivateVcComplete(
           pRecvInfo->pVc,
           NDIS_STATUS_SUCCESS);
   }
   else if ((NDIS_STATUS_SUCCESS != Status) && (NDIS_STATUS_PENDING != Status))
   {
       //
       //	If there is a VC completion pending then complete the VC with
       //	failure.
       //
       if (RECV_INFO_TEST_FLAG(pRecvInfo, fRECV_INFO_COMPLETE_VC_ACTIVATION))
       {
           ASSERT(NULL != pRecvInfo->pVc);

           NdisAcquireSpinLock(&pVc->lock);

           VC_SET_FLAG(pVc, fVC_RECEIVE);
           VC_CLEAR_FLAG(pVc, fVC_ALLOCATING_RXBUF);

           if (VC_TEST_FLAG(pVc, fVC_ALLOCATING_TXBUF))
           {
               //
               // Wait until PadTrailerBuffers allocated to
               // complete the VC
               //
               VC_SET_FLAG(pVc, fVC_MEM_OUT_OF_RESOURCES);

               NdisReleaseSpinLock(&pVc->lock);

           }
           else
           {
               NdisReleaseSpinLock(&pVc->lock);

               //
               //  Done with all of allocation. Complete the VC.
               //
               TbAtm155ActivateVcComplete(pRecvInfo->pVc, Status);
           }
       }
   }

   return;

}


VOID
tbAtm155QueueRecvBuffersToReceiveSlots(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  UCHAR           BufferType
   )
/*++

Routine Description:

   This routine post receive buffers to the corresponding receive slot
   for getting ready to receive data on network.

Arguments:

   pAdapter    -   Poitner to the ADAPTER_BLOCK.
   BufferType  -   Indicates buffer type to post to rx slots from either
                   "Big" or "Small" buffer pool.

Return Value:

   None.

--*/
{
   PSAR_INFO               pSar = pAdapter->HardwareInfo->SarInfo;
   PRECV_DMA_QUEUE         pRecvDmaQ;
   PULONG                  pTmpFreeSlots;
   PRECV_BUFFER_QUEUE      pFreeBufQ;
   PRECV_BUFFER_QUEUE      pCompletingBufQ;
   PRECV_BUFFER_HEADER	    pRecvHeader;
   ULONG                   RxSlotNum;
   RX_PENDING_SLOTS_CTL    RxPendingSlotCtrlReg;
   PTBATM155_SAR           TbAtm155Sar = pAdapter->HardwareInfo->TbAtm155_SAR;
   PUSHORT                 pSlotTagOfBufQ;


   pRecvDmaQ = &pSar->RecvDmaQ;

   NdisAcquireSpinLock(&pRecvDmaQ->lock);

   RxPendingSlotCtrlReg.reg = 0;

   if (BufferType == RECV_SMALL_BUFFER)
   {
       // 
       //  Got more buffers in "Small" buffer pool.
       //  Set the related variables for posting more buffers into 
       //  small receive slot if there are free slots available.
       // 
       pFreeBufQ       = &pSar->FreeSmallBufferQ;
       pSlotTagOfBufQ  = &pRecvDmaQ->CurrentSlotTagOfSmallBufQ;
       pCompletingBufQ = &pRecvDmaQ->CompletingSmallBufQ;
       pTmpFreeSlots   = &pRecvDmaQ->RemainingReceiveSmallSlots;

       //
       //  Set to "Small" slot type.
       //
       RxPendingSlotCtrlReg.Slot_Type = 0;

       //
       //  Set to "Small" slot type which will be report to Rx report queue.
       //
       RxPendingSlotCtrlReg.rqSlotType = 0;
   }
   else
   {
       // 
       //  Got more buffers in "Big" buffer pool.
       //  Set the related variables for posting more buffers into big
       //  receive slot if there are free slots available.
       // 
       pFreeBufQ       = &pSar->FreeBigBufferQ;
       pSlotTagOfBufQ  = &pRecvDmaQ->CurrentSlotTagOfBigBufQ;
       pCompletingBufQ = &pRecvDmaQ->CompletingBigBufQ;
       pTmpFreeSlots   = &pRecvDmaQ->RemainingReceiveBigSlots;

       //
       //  Set to "Big" slot type.
       //
       RxPendingSlotCtrlReg.Slot_Type = 1;
       
       //
       //  Set to "Big" slot type which will be report to Rx report queue.
       //
       RxPendingSlotCtrlReg.rqSlotType = 1;
   }

   for (RxSlotNum = 0; *pTmpFreeSlots > 0; (*pTmpFreeSlots)--)
   {

       //
       //  Retrieve receive buffer from the head of buffer queue.
       //
       tbAtm155RemoveReceiveBufferFromHead(pFreeBufQ, &pRecvHeader);
       if (NULL == pRecvHeader)
       {
           // 
           //  There is no available receive slots available, Exit.
           // 
           break;
       }

       // 
       //  If It is first time to post this Rx buffer, write 0.
       // 
       RxPendingSlotCtrlReg.VC = 0;

       //
       //  Check if this receive buffer has been assigned an 
       //  unique "Slot Tag".
       //
       if (0 == pRecvHeader->TagPosted)
       {
           //
           //  No, let's assigned one.
           //
           pRecvHeader->TagPosted = *pSlotTagOfBufQ;

           //
           // Update the Slot Tag for next receive buffer.
           //
           (*pSlotTagOfBufQ)++;

       }
       else
       {
           //
           //  Indicate the Vc associated with the rx buffer was reported.
           //
           if (NULL != pRecvHeader->pVc)
           {
               RxPendingSlotCtrlReg.VC = pRecvHeader->pVc->VpiVci.Vci;
           }
       }

       //
       //  Post the unique slot tag to this buffer slot.
       //
       RxPendingSlotCtrlReg.Slot_Tag = pRecvHeader->TagPosted;

       //
       //  Insert the receive buffer into the tail of the waiting FIFO.
       //  Wait until InterruptOfComplete is generated.
       //
       tbAtm155InsertRecvBufferAtTail(pCompletingBufQ, pRecvHeader, FALSE);

       //        
       //  Write the receive buffer into receive slot.       
       //        
       TBATM155_WRITE_PORT(
           &TbAtm155Sar->Rx_Pending_Slots[RxSlotNum].Cntrl,
           RxPendingSlotCtrlReg.reg);

       TBATM155_WRITE_PORT(
           &TbAtm155Sar->Rx_Pending_Slots[RxSlotNum].Base_Addr,
		    NdisGetPhysicalAddressLow(pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].PhysicalAddress));

       //               
       //  Move to next slot registers for the next posting.
       //               
       RxSlotNum = (RxSlotNum + 1) % MAX_SLOTS_NUMBER;
   }

	NdisReleaseSpinLock(&pRecvDmaQ->lock);
}


