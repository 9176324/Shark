/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       support.c
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920



Abstract:

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	09/23/96		kyleb		Added tbAtm155RemoveHash to remove the VC_BLOCK
								from the hash table when it is deactivated.
	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.

--*/

#include "precomp.h"
#pragma hdrstop

#define    MODULE_NUMBER	MODULE_SUPPORT

#if DBG

VOID
InsertPacketAtTailDpc(
   IN  PPACKET_QUEUE   PacketQ,
   IN  PNDIS_PACKET    Packet
	)
/*++

Routine Description:

   This routine will insert a packet at the tail end of a packet queue.

Arguments:

   PacketQ -   Pointer to the structure representing the packet queue
               to add the packet to.
   Packet  -   Pointer to the packet to queue.

Return Value:

   None.

--*/
{
   if (NULL == PacketQ->Head)
   {
       PacketQ->Head = Packet;
   }
   else
   {
       PACKET_RESERVED_FROM_PACKET(PacketQ->Tail)->Next = Packet;
   }

   PacketQ->Tail = Packet;

   PACKET_RESERVED_FROM_PACKET(Packet)->Next = NULL;

   PacketQ->References++;
}


VOID
InsertPacketAtTail(
   IN  PPACKET_QUEUE   PacketQ,
   IN  PNDIS_PACKET    Packet
   )
/*++

Routine Description:

   This routine will insert a packet at the tail end of a packet queue.

Arguments:

   PacketQ -   Pointer to the structure representing the packet queue
               to add the packet to.
   Packet  -   Pointer to the packet to queue.

Return Value:

   None.

--*/
{
   if (NULL == PacketQ->Head)
   {
   PacketQ->Head = Packet;
   }
   else
   {
       PACKET_RESERVED_FROM_PACKET(PacketQ->Tail)->Next = Packet;
   }

   PacketQ->Tail = Packet;

   PACKET_RESERVED_FROM_PACKET(Packet)->Next = NULL;

   PacketQ->References++;
}

VOID
RemovePacketFromHeadDpc(
   IN  PPACKET_QUEUE   PacketQ,
   OUT PNDIS_PACKET    *Packet
   )
/*++

Routine Description:

   This routine will remove a packet from the head of the queue.

Arguments:

   PacketQ -   Pointer to the structure representing the packet queue
               to add the packet to.
   Packet  -   Pointer to the place to store the packet removed.

Return Value:

--*/
{
   *Packet = PacketQ->Head;
   if (NULL != *Packet)
   {
       PacketQ->Head = PACKET_RESERVED_FROM_PACKET(*Packet)->Next;
       PacketQ->References--;
   }

   PACKET_RESERVED_FROM_PACKET(*Packet)->Next = NULL;
}


VOID
RemovePacketFromHead(
   IN  PPACKET_QUEUE   PacketQ,
   OUT PNDIS_PACKET    *Packet
   )
/*++

Routine Description:

   This routine will remove a packet from the head of the queue.

Arguments:

   PacketQ -   Pointer to the structure representing the packet queue
               to add the packet to.
   Packet  -   Pointer to the place to store the packet removed.

Return Value:

--*/
{
   *Packet = PacketQ->Head;
   if (NULL != *Packet)
   {
       PacketQ->Head = PACKET_RESERVED_FROM_PACKET(*Packet)->Next;
       PacketQ->References--;
   }

   PACKET_RESERVED_FROM_PACKET(*Packet)->Next = NULL;
}


VOID
InitializeMapRegisterQueue(
   IN  PMAP_REGISTER_QUEUE MapRegistersQ
   )
/*++

Routine Description:

   This routine will initialize a MAP_REGISTER queue structure for use.

Arguments:

   MapRegistersQ   -   Pointer to the MAP_REGISTER_QUEUE to initialize.

Return Value:

   None.

--*/
{
   MapRegistersQ->Head = NULL;
   MapRegistersQ->Tail = NULL;
   MapRegistersQ->References = 0;

   NdisAllocateSpinLock(&MapRegistersQ->lock);
}


VOID
InsertMapRegisterAtTail(
   IN  PMAP_REGISTER_QUEUE     MapRegistersQ,
   IN  PMAP_REGISTER           MapRegister
   )

/*++

Routine Description:

   This routine will insert an index of map register at the tail end
   of a map registers queue.

Arguments:

   MapRegistersQ   -   Pointer to the structure representing the index of
                       map register to add to.
   MapRegister     -	Pointer to the index of map register to queue.

Return Value:

   None.

--*/
{
   if (NULL == MapRegistersQ->Head)
   {
       MapRegistersQ->Head = MapRegister;
   }
   else
   {
       MapRegistersQ->Tail->Next = MapRegister;
   }

   MapRegistersQ->Tail = MapRegister;
   MapRegister->Next = NULL;
   MapRegistersQ->References++;
}


VOID
RemoveMapRegisterFromHead(
   IN  PMAP_REGISTER_QUEUE     MapRegistersQ,
   OUT PMAP_REGISTER           *MapRegister
	)
/*++

Routine Description:

   This routine will remove an index of map register from the head of
   the queue.

Arguments:

   MapRegistersQ   -   Pointer to the structure representing the index of
                       map register to add to.
   MapRegister     -	Pointer to the index of map register to queue.

Return Value:

--*/
{
   *MapRegister = MapRegistersQ->Head;
   if (NULL != *MapRegister)
   {
       MapRegistersQ->Head = (*MapRegister)->Next;
       (*MapRegister)->Next = NULL;
   }

   MapRegistersQ->References--;

}


#endif     // end of DBG



VOID
tbAtm155InsertRecvBufferAtTail(
   IN  PRECV_BUFFER_QUEUE      pBufferQ,
   IN  PRECV_BUFFER_HEADER     pRecvHeader,
   IN  BOOLEAN                 CheckIfInitBuffer
   )
/*++

Routine Description:

   This routine will insert an receive buffer header into at the tail end
   of a receive buffer queue.

Arguments:

   pBufferQ                -   Pointer to the receive buffer queue of
                               the receive buffer add to.
   pRecvHeader             -   Pointer to the receive buffer to queue.
   CheckIfBufferIsFreed    -   TRUE    Check if the buffer is being freed.
                               FALSE   Skip the checking.

Return Value:

   None.

--*/
{


//
//   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
//       ("==>tbAtm155InsertRecvBufferAtTail\n"));
//

   NdisAcquireSpinLock(&pBufferQ->lock);


   if (NULL != pRecvHeader)
   {
       if (CheckIfInitBuffer == TRUE)
       {
           //
           //	Kill the next pointer for the buffers.
           //
           pRecvHeader->FlushBuffer->Next = NULL;
           pRecvHeader->NdisBuffer->Next = NULL;

           //
           //  Re-adjust both the flush and NDIS buffers back to their
           //  allocation size.
           //
           NdisAdjustBufferLength(
               pRecvHeader->FlushBuffer,
               pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].Size);
   
           NdisAdjustBufferLength(
               pRecvHeader->NdisBuffer,
               pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].Size);
   
           pRecvHeader->pVc = NULL;


#if    DBG

           NdisZeroMemory (
               (PVOID)pRecvHeader->Alloc[RECV_BUFFER_ALIGNED].VirtualAddress,
               1536);

#endif // end of DBG

       } // end of if (CheckIfInitBuffer == TRUE)

       //
       //	Is the free buffer list empty?
       //
       if (NULL == pBufferQ->BufListHead)
       {
           //
           //	Place the receive buffer at the head of the free list.
           //
           pBufferQ->BufListHead = pRecvHeader;
           pBufferQ->BufListTail = pRecvHeader;

           pRecvHeader->Next = NULL;
           pRecvHeader->Prev = NULL;

       }
       else
       {
           //
           //	Insert the receive buffer at the end of the free list.
           //
           pRecvHeader->Prev = pBufferQ->BufListTail;
           pRecvHeader->Next = NULL;
           pBufferQ->BufListTail->Next = pRecvHeader;

           pBufferQ->BufListTail = pRecvHeader;
       }
   
       pBufferQ->BufferCount++;

   } // end of if (NULL != pRecvHeader)

   NdisReleaseSpinLock(&pBufferQ->lock);


//
//   DBGPRINT(DBG_COMP_VC, DBG_LEVEL_INFO,
//       ("<==tbAtm155InsertRecvBufferAtTail\n"));
//

}


VOID
tbAtm155RemoveReceiveBufferFromHead(
   IN  PRECV_BUFFER_QUEUE      pBufferQ,
   OUT PRECV_BUFFER_HEADER     *pRecvHeader
   )
/*++

Routine Description:

   This routine will remove an index of map register from the head of
   the queue.

Arguments:

   pBufferQ        -   Pointer to the receive buffer queue of
                       the receive buffer add to.
	pRecvHeader     -   Pointer to the receive buffer to be removed from
                       the receive buffer queue.

Return Value:

--*/
{

   NdisAcquireSpinLock(&pBufferQ->lock);

   *pRecvHeader = pBufferQ->BufListHead;

   if (NULL != *pRecvHeader)
   {
       pBufferQ->BufListHead = (*pRecvHeader)->Next;

       if (NULL == (*pRecvHeader)->Next)
       {
           //
           //  Reset the pointer of pBufferQ->BufListTail if the
           //  queue is empty
           //
           pBufferQ->BufListTail = NULL;
       }
       else
       {
           pBufferQ->BufListHead->Prev = NULL;
           (*pRecvHeader)->Next = NULL;
       }

       pBufferQ->BufferCount--;

   }

   NdisReleaseSpinLock(&pBufferQ->lock);
}


VOID
tbAtm155SearchRecvBufferFromQueue(
   IN  PRECV_BUFFER_QUEUE      pBufferQ,
   IN  ULONG                   PostedTag,
   OUT PRECV_BUFFER_HEADER     *pRecvHeader
)
/*++

Routine Description:

   This routine will search the receive in the buffer queue with the 
   with the specific physcial address.

Arguments:

   pBufferQ        -   Pointer to the receive buffer queue of
                       the receive buffer add to.
   PostedTag       -   The Tag was reported in Rx report queue.
	pRecvHeader     -	Pointer to the receive buffer we are looking for.
                       NULL will be return, if the buffer is not found.

Return Value:

	None.

--*/
{
   PRECV_BUFFER_HEADER     ptmpRecvHeader;
   ULONG                   SearchBC;
   BOOLEAN                 BufferIsFound = FALSE;


   NdisDprAcquireSpinLock(&pBufferQ->lock);

   for (ptmpRecvHeader = pBufferQ->BufListHead, SearchBC = pBufferQ->BufferCount;
        ((NULL != ptmpRecvHeader) && SearchBC);
        ptmpRecvHeader = ptmpRecvHeader->Next, SearchBC--)
   {

       if (PostedTag == ptmpRecvHeader->TagPosted)
       {
           //
           //  Found the receive buffer we are looking for.
           //  Remove from this list.
           //
           if ((NULL != ptmpRecvHeader->Next) && (NULL != ptmpRecvHeader->Prev))
           {
               //
               //  The receive buffer is in the middle of the list. 
               //
               ptmpRecvHeader->Next->Prev = ptmpRecvHeader->Prev;
               ptmpRecvHeader->Prev->Next = ptmpRecvHeader->Next;
           }
   	    else if (NULL == ptmpRecvHeader->Next)
	        {
               //
               //  The receive buffer is at the end of the list. 
               //  Update the BufListTail.
               //  If this buffer is the last one in the queue,
               //  pBufferQ->BufListTail will be set to NULL.
               //
               pBufferQ->BufListTail = ptmpRecvHeader->Prev;

               if (NULL == ptmpRecvHeader->Prev)
               {
                   // 
                   // The queue is empty now.
                   // 
			        pBufferQ->BufListHead = NULL;
               }

               //
               //  If this buffer is not the last one in the queue,
               //  set pBufferQ->BufListTail->Next to NULL.
               //
               if (NULL != pBufferQ->BufListTail)
               {
                   // 
                   // This buffer is at the end of queue.
                   // 
                   pBufferQ->BufListTail->Next = NULL;
               }
		    }
           else
           {
               //
               //  The receive is at the Beginning of the list. 
               //  Update the BufListHead
               //
			    pBufferQ->BufListHead = ptmpRecvHeader->Next;
			    pBufferQ->BufListHead->Prev = NULL;
           }
           ptmpRecvHeader->Next = NULL;
           ptmpRecvHeader->Prev = NULL;
           pBufferQ->BufferCount--;
           BufferIsFound = TRUE;

           break;  
   	}
   }

   if (BufferIsFound == TRUE)
   {
       *pRecvHeader = ptmpRecvHeader;
   }
   else
   {
       *pRecvHeader = NULL;
   }

   NdisDprReleaseSpinLock(&pBufferQ->lock);

}



VOID
tbAtm155MergeRecvBuffers2FreeBufferQueue(
   IN PADAPTER_BLOCK      pAdapter,
   IN PRECV_BUFFER_QUEUE  pFreeBufferQ,
   IN PRECV_BUFFER_QUEUE  pReturnBufferQ
   )
/*++

Routine Description:

   This routine will merge pRetrunBufferQ into pFreeBufferQ and
   put the buffers back to their pools if the pools are being
   freed.

Arguments:

Return Value:

--*/
{

   NdisAcquireSpinLock(&pFreeBufferQ->lock);

   if (NULL != pReturnBufferQ->BufListHead)
   {
       if (NULL == pFreeBufferQ->BufListHead)
       {
           pFreeBufferQ->BufListHead = pReturnBufferQ->BufListHead;
           pFreeBufferQ->BufListTail = pReturnBufferQ->BufListTail;
       }
       else
       {
           //
           //	Chain the first buffer in the returned queue with the
           //	current tail of the free buffer list.
           //
           pReturnBufferQ->BufListTail->Prev = pFreeBufferQ->BufListTail;
           pFreeBufferQ->BufListTail->Next = pReturnBufferQ->BufListHead;
	
           //
           //	Setup the new tail of the free buffer list.
           //
           pFreeBufferQ->BufListTail = pReturnBufferQ->BufListTail;
           pFreeBufferQ->BufListTail->Next = NULL;
       }

       //
       //	Increment the receive information's free buffer count.
       //	This is the total number of returned headers minus the
       //	ones that were freed back to their pools.
       //
       pFreeBufferQ->BufferCount += pReturnBufferQ->BufferCount;

	    DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
		    ("tbAtm155MergeRecvBuffers2FreeBufferQueue: %lu\n", pFreeBufferQ->BufferCount));
       
	    DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
		    ("AllocatedBigRecvBuffers: %lu\n", pAdapter->HardwareInfo->SarInfo->AllocatedBigRecvBuffers));
       
	    DBGPRINT(DBG_COMP_RESET, DBG_LEVEL_INFO,
		    ("AllocatedSmallRecvBuffers: %lu\n", pAdapter->HardwareInfo->SarInfo->AllocatedSmallRecvBuffers));
       

       // initialize the pReturnBufferQ for re-use.
       pReturnBufferQ->BufListHead = NULL;
       pReturnBufferQ->BufListTail = NULL;
       pReturnBufferQ->BufferCount = 0;
   }

   NdisReleaseSpinLock(&pFreeBufferQ->lock);

}


VOID
tbAtm155HashVc(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  PVC_BLOCK       pVc
   )
/*++

Routine Description:

   This routine will save the VC pointer in a hash table.
   This table is used to find the VC in a quick manner given only
   it's VC number.

Arguments:

   pAdapter    -   Adapter block to hash the VC to.
   pVc         -   Pointer to the VC to hash.

Return Value:

   None.

--*/
{
   ULONG	HashIndex;

   //
   //	Get the bucket in the hash table where this VC will be placed.
   //
   HashIndex = pVc->VpiVci.Vci % pAdapter->RegistryParameters[TbAtm155VcHashTableSize].Value;

   //
   //	Just store this vc at the head.
   //
   NdisAcquireSpinLock(&pAdapter->VcHashLock);
   pVc->NextVcHash = pAdapter->VcHashList[HashIndex];
   pAdapter->VcHashList[HashIndex] = pVc;
   NdisReleaseSpinLock(&pAdapter->VcHashLock);
}

PVC_BLOCK
tbAtm155UnHashVc(
   IN  PADAPTER_BLOCK  pAdapter,
   IN  ULONG           Vc
   )
/*++

Routine Description:

   This routine will return the pointer to the VC_BLOCK that is
   represented by the VC number.

Arguments:

   pAdapter    -   Pointer to the adapter block that the VC belongs to.
   Vc          -   The number of the VC that we want to retreive.

Return Value:

   PVC_BLOCK if successful.  NULL if the VC has not been created or
   activated on this adapter.

--*/
{
   ULONG		HashIndex;
   PVC_BLOCK	pVc;

   //
   //	Get the hash bucket that the VC is in.	
   //
   HashIndex = Vc % pAdapter->RegistryParameters[TbAtm155VcHashTableSize].Value;

   NdisAcquireSpinLock(&pAdapter->VcHashLock);

   pVc = pAdapter->VcHashList[HashIndex];
   while ((pVc != NULL) && (pVc->VpiVci.Vci != Vc))
   {
       pVc = pVc->NextVcHash;
   }

   NdisReleaseSpinLock(&pAdapter->VcHashLock);

   return(pVc);
}

VOID
tbAtm155RemoveHash(
   IN  PADAPTER_BLOCK      pAdapter,
   IN  PVC_BLOCK           pVc
   )
/*++

Routine Description:

   This routine will remove VC from the hash table.

Arguments:

   pAdapter    -   Pointer to the ADAPTER_BLOCK.
   pVc         -   Pointer to the VC_BLOCK

Return Value:

   None.

--*/
{
   ULONG       HashIndex;
   PVC_BLOCK   ptmpVc;
   PVC_BLOCK   pPrevVc = NULL;

   //
   //	Get the hash table index.
   //
   HashIndex = pVc->VpiVci.Vci % pAdapter->RegistryParameters[TbAtm155VcHashTableSize].Value;

   NdisAcquireSpinLock(&pAdapter->VcHashLock);

   //
   //	Get a temp list pointer to manipulate.
   //
   ptmpVc = pAdapter->VcHashList[HashIndex];

   //
   //	Walk the list looking for the VC that we need to remove.
   //
   while ((NULL != ptmpVc) && (pVc != ptmpVc))
   {
       //
       //	Save the previous hashed VC_BLOCK.
       //
       pPrevVc = ptmpVc;

       //
       //	Get a pointer to the next hashed VC_BLOCK.
       //
       ptmpVc = ptmpVc->NextVcHash;
   }

   //
   //	Make sure that the VC was hashed.
   //	If it wasn't then ignore the call.
   //
   if (NULL != ptmpVc)
   {
       //
       //	Was the VC_BLOCK the head of the hash list?
       //
       if (NULL == pPrevVc)
       {
           pAdapter->VcHashList[HashIndex] = ptmpVc->NextVcHash;
       }
       else
       {
           pPrevVc->NextVcHash = ptmpVc->NextVcHash;
       }
   }
   NdisReleaseSpinLock(&pAdapter->VcHashLock);
}


VOID
tbAtm155InitializePacketQueue(
   IN  OUT PPACKET_QUEUE   PacketQ
   )
/*++

Routine Description:

   This routine will initialize a PACKET_QUEUE structure for use.

Arguments:

   PacketQ -   Pointer to the PACKET_QUEUE to initialize.

Return Value:

   None.

--*/
{
   PacketQ->Head = NULL;
   PacketQ->Tail = NULL;

   NdisAllocateSpinLock(&PacketQ->lock);
}


VOID
tbAtm155FreePacketQueue(
   IN  OUT PPACKET_QUEUE   PacketQ
   )
/*++

Routine Description:

   This routine will free a PACKET_QUEUE structure from use.

Arguments:

   PacketQ -   Pointer to the PACKET_QUEUE to free.

Return Value:

   None.

--*/
{
   ASSERT(NULL == PacketQ->Head);

   NdisFreeSpinLock(&PacketQ->lock);
}


VOID
tbAtm155InitializeReceiveBufferQueue(
   IN  OUT PRECV_BUFFER_QUEUE  RecvBufferQ
   )
/*++

Routine Description:

   This routine will initialize a RECV_BUFFER_QUEUE structure for use.

Arguments:

   RecvBufferQ -   Pointer to the RECV_BUFFER_QUEUE to initialize.

Return Value:

   None.

--*/
{
   RecvBufferQ->BufListHead = NULL;
   RecvBufferQ->BufListTail = NULL;
   RecvBufferQ->BufferCount = 0;

   NdisAllocateSpinLock(&RecvBufferQ->lock);
}



VOID
tbAtm155FreeReceiveBufferQueue(
   IN  PADAPTER_BLOCK          pAdapter,
   IN  OUT PRECV_BUFFER_QUEUE  RecvBufferQ
   )
/*++

Routine Description:

   This routine will free a RECV_BUFFER_QUEUE structure for use.

Arguments:

   RecvBufferQ	-	Pointer to the RECV_BUFFER_QUEUE to free.

Return Value:

   None.

--*/
{
   PRECV_BUFFER_HEADER	    *ppRecvHeaderHead;
   
   for (ppRecvHeaderHead = &RecvBufferQ->BufListHead; 
        NULL != *ppRecvHeaderHead; )
   {
       // 
       //  Free the buffer pool when all of the buffers has been freed
       //  into the pool.
       //
       tbAtm155FreeReceiveBuffer(ppRecvHeaderHead, pAdapter, *ppRecvHeaderHead);
       RecvBufferQ->BufferCount--;
   }

   ASSERT(0 == RecvBufferQ->BufferCount);

   if (NULL == RecvBufferQ->BufListHead)
   {
       //  Reset the Pointer
       RecvBufferQ->BufListTail = NULL;
   }

   ASSERT(NULL == RecvBufferQ->BufListHead);
   ASSERT(NULL == RecvBufferQ->BufListTail);
   ASSERT(0 == RecvBufferQ->BufferCount);

   NdisFreeSpinLock(&RecvBufferQ->lock);
}



ULONG Exp_Tbl[24] = {
         1L,       2L,       4L,       8L,     // exp 0
        16L,      32L,      64L,     128L,     // exp 4
       256L,     512L,    1024L,    2048L,     // exp 8
      4096L,    8192L,   16386L,   32678L,     // exp 12
     65536L,  131072L,  262144L,  524288L,     // exp 16
   1048576L, 2097152L, 4194394L, 8288608L      // exp 20
};

void
tbAtm155ConvertToFP(
   IN OUT  PULONG  pRate,
   OUT     PUSHORT pRateInFP
   )
/*++

Routine Description:

   This routine will convert from rate, PCR, into a binary floating
   point representation. The Equate is

       Rate = [2^e (1 + m/512)] * nz

           reserved bit -> Bit 15

           nz -> Bit 14
                 0 the rate is zero
                 1 the rate is as given by the fields e and m.

           e  -> Bit 13 through 9
                 range from 0 to 31. The exponent is a 5 bit unsigned

           m  -> Bit 9 through 0
                 The mantissa is a 5 bit unsigned


Arguments:

   Rate        -   The rate needs to be converted.
   pRateInFP   -   point to the variable to write 

Return Value:

   None.

--*/
{
   ULONG               i_rate = *pRate;
   ULONG               temp;
   TBATM155_RATE_IN_FP rate_FP;
   
   NDIS_STATUS Status = NDIS_STATUS_FAILURE;

   do {

       //
       // Just initialize
       //
       rate_FP.reg = 0;      // default nz to be 0.
       rate_FP.nz = 1;      // default nz to be 1.

       if (i_rate == 0)
       {
           //
           // Detected input rate is 0, set nz and return ot caller.
           //
           rate_FP.nz = 0;
           break;
       }
       
       for (temp = 0; temp < 24; temp++)
       {
           if (i_rate <= Exp_Tbl[temp])
           {
               if (i_rate != Exp_Tbl[temp])
               {
                   //
                   // i_rate cannot be less than 1 and not equal to 0
                   // 
                   ASSERT(temp > 0);
                   temp--;
               }
               rate_FP.exp = (USHORT)temp;
               temp = Exp_Tbl[temp];
               Status = NDIS_STATUS_SUCCESS;
               break;
           }
       }

       if (Status == NDIS_STATUS_FAILURE)
       {
           // 
           // If there are trouble, give the lowest rate of transfer
           // 
           i_rate = Exp_Tbl[3];
           temp   = i_rate;
           *pRate = i_rate;
       }

       rate_FP.mant = (USHORT)(((i_rate - temp) * 512) / temp);

   } while (FALSE);

   *pRateInFP = rate_FP.reg;
}


