/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       sw.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920


Abstract:

Author:

   A. Wang

Environment:

   Kernel mode

Revision History:

   09/23/96    kyleb       Added support for NdisAllocateMemoryWithTag
   01/07/97    awang       Initial of Toshiba ATM 155 Device Driver.
   02/10/97    awang       Moved RECV_SEG_INFO from sar.h

--*/

#ifndef __SW_H
#define __SW_H

#define ABS(x)         (((x) < 0) ? -(x) : (x))

//
//	Macros to read from the adapters memory.
//
#define TBATM155_READ_BUFFER_UCHAR(pDst, pSrc, Length)                     \
{                                                                          \
	UINT	_c;                                                             \
                                                                           \
	for (_c = 0; _c < (Length); _c++)                                       \
	{                                                                       \
		NdisReadRegisterUchar((PUCHAR)(pSrc) + _c, (PUCHAR)(pDst) + _c);    \
	}                                                                       \
}

//
//	This macro is used to convert big-endian to host format.
//
#define TBATM155_SWAP_USHORT(x) (USHORT)((((x) >> 8) & 0xFF)   |   \
                                   (((x) << 8) & 0xFF00))

#define TBATM155_SWAP_ULONG(x)  (ULONG)((((x) >> 24) & 0xFF)   |   \
                               (((x) >> 8) & 0xFF00)           |   \
                               (((x) << 8) & 0xFF0000)         |   \
                               (((x) << 24) & 0xFF000000))

// buffer size passed in NdisMQueryAdapterResources                            
// We should only need three adapter resources (IO, interrupt and memory),
// Some devices get extra resources, so have room for 10 resources 
#define ATM_RESOURCE_BUF_SIZE           (sizeof(NDIS_RESOURCE_LIST) + \
                                        (10 * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)))

//
//	Macros used to allocate and free memory.
//
#define ALLOCATE_MEMORY(_pStatus, _pAddress, _Length, _Tag)    \
{                                                              \
   *(_pStatus) = NdisAllocateMemoryWithTag(                    \
                   (PVOID *)(_pAddress),                       \
                   (UINT)(_Length),                            \
                   (ULONG)(_Tag));                             \
}

#define FREE_MEMORY(_Address, _Length)                         \
{                                                              \
   NdisFreeMemory((PVOID)(_Address), (UINT)(_Length), 0);      \
}

#define ZERO_MEMORY(_Address, _Length)                         \
{                                                              \
   NdisZeroMemory((_Address), (_Length));                      \
}

#define TBATM155_PROTOCOL_RESERVED	(sizeof(MEDIA_SPECIFIC_INFORMATION) + sizeof(ATM_AAL_OOB_INFO))

//
//	Contains allocation information.
//
typedef struct	_ALLOCATION_INFO
{
   PVOID                   VirtualAddress;
   NDIS_PHYSICAL_ADDRESS   PhysicalAddress;
   ULONG                   Size;
}
   ALLOCATION_INFO,
   *PALLOCATION_INFO;



//
//	The following is used to keep track of the receive buffers.
//
typedef struct _RECV_BUFFER_HEADER
{
   ULONG                       Signature;

   //
   //	These are the list pointers used to keep track of free
   //	receive buffers, or of a list of receive buffers that
   //	are being used to receive a packet.
   //
   struct _RECV_BUFFER_HEADER  *Next;
   struct _RECV_BUFFER_HEADER  *Prev;
   struct _RECV_BUFFER_HEADER  *Last;
   
   //
   //	Pointer to the pool this receive buffer came from.
   //
   struct _RECV_BUFFER_POOL    *Pool;
   
   //
   //	Flags that describe the state of the receive buffer.
   //
   ULONG                       Flags;
   
   //
   //	Used for cache coherency.
   //
   PNDIS_BUFFER                FlushBuffer;
   
   //
   //	Used to indicate data in an NDIS_PACKET.
   //
   PNDIS_BUFFER                NdisBuffer;
   
   //
   //	Packet that is associated with this buffer.
   //
   PNDIS_PACKET                Packet;
   
   //
   //	Pointer to the VC that this receive is destined for.
   //
   PVC_BLOCK                   pVc;
   
   //
   //	The posted tags information of this receive buffer.
   //
   USHORT                      TagPosted;

   //
   //	If this buffer was allocated from a pool then there is
   //	only allocation information that describes the aligned
   //	buffer to be used for the receive information.
   //	If the buffer was allocated individually then there is
   //	the aligned allocation information as well as the original
   //	allocation information which is to be used to free the
   //	receive buffer if necessary.
   //
   ALLOCATION_INFO             Alloc[1];
};


#define RECV_BUFFER_HEADER_SIG         0x30303030

//
//	Indices into the ALLOCATION_INFO array...
//
#define RECV_BUFFER_ALIGNED    0
#define RECV_BUFFER_ORIGINAL   1

//
//	This structure is used to keep track of transmit contexts
//
typedef struct _TRANSMIT_CONTEXT
{
   ULONG                       Signature;
   ULONG                       BufferSize;
   ULONG                       DmaAlignment;
   PVC_BLOCK                   pVc;
   PXMIT_SEG_INFO              pTransmitSegInfo;
}TRANSMIT_CONTEXT, *PTRANSMIT_CONTEXT;

#define XMIT_BUFFER_SIG           0x88888888

//
//	This structure is used to keep track of pools of receive
//	buffers.  Receive buffer pools are allocated on a per VC basis.
//	So a buffer pool allocated for VC x is never used for VC y. 
//
struct _RECV_BUFFER_POOL
{
   ULONG                       Signature;

   //
   //	Pointer to the next pool of receive buffers.
   //
   struct _RECV_BUFFER_POOL    *Next;

   //
   //	Back pointer to the owning receive buffer information structure.
   //
   struct _RECV_BUFFER_INFO    *RecvBufferInfo;

   //
   //	Count of buffers allocated to this pool.
   //	This is the same for the packets.
   //
   ULONG                       ReceiveBufferCount;

   //
   //	This is the size of the receive buffer that the adapter can use
   //	to receive into.
   //
   ULONG                       ReceiveBufferSize;

   //
   //	This is the allocated virtual, physical addresses and
   //	the allocated size of the pool.
   //
   ALLOCATION_INFO;

   //
   //	Size of the receive buffer's header.
   //
   ULONG                       ReceiveHeaderSize;
   
   //
   //	Handle for the flush buffers associated with the
   //	pool.
   //
   NDIS_HANDLE                 hFlushBufferPool;
   NDIS_HANDLE                 hNdisBufferPool;

   //
   //	NDIS wrapper handle to a packet pool.
   //
   NDIS_HANDLE                 hNdisPacketPool;

   //
   //	This is a list of buffers that are requested to be returned.
   //  This list is always to be NULL until the pool state is changed
   //  to fRECV_POOL_BEING_FREED.
   //	Once the BuffersFreed count has reached the total count then
   //	this pool can be freed (with consideration for the packets also).
   //	
   PRECV_BUFFER_HEADER         FreeBuffersHead;

   //
   //	Buffers free'd
   //
   ULONG                       BuffersFreed;

   //
   //	Flags describing the buffer pools state.
   //
   ULONG                       Flags;
};


#define RECV_BUFFER_POOL_SIG               0x20202020

//
//	Flag definitions for the receive buffer pool.
//
#define fRECV_POOL_BEING_FREED             0x00000001
#define fRECV_POOL_BLOCK_ALLOC             0x00000002
#define fRECV_POOL_ALLOCATING_BLOCK        0x00000004
#define fRECV_POOL_ALLOCATING_INDIVIDUAL   0x00000008

//
//	Macros used for accessing the receive pool flags
//
#define RECV_POOL_TEST_FLAG(x, f)          (((x)->Flags & (f)) == (f))
#define RECV_POOL_SET_FLAG(x, f)           ((x)->Flags |= (f))
#define RECV_POOL_CLEAR_FLAG(x, f)         ((x)->Flags &= ~(f))

//
//	This structure is used to keep track of multiple pools of receive
//	buffers, guard their access in the case of UBR VCs.
//	It is also used to manage a free list of buffers to be used in
//	the case of a service interrupt.
//
typedef struct _RECV_BUFFER_INFO
{
   ULONG                   Signature;
   
   NDIS_SPIN_LOCK          lock;

   //
   //	Flags for the receive buffer allocation.
   //
   ULONG                   Flags;

   //
   //	Pointer to the owning VC_BLOCK.  If this is NULL then it's
   //	the common UBR receive buffer information.
   //
   PVC_BLOCK               pVc;

   //
   //	This is the amount of information each receive buffer
   //	can hold.
   //
   ULONG                   RecvBufferSize;

   //
   //	Pointer to the list of buffer pools allocated.
   //
   PRECV_BUFFER_POOL       RecvPool;

   //
   //	This is the free list of buffers from all the available pools.
   //
   PRECV_BUFFER_HEADER     FreeBufferListHead;
   PRECV_BUFFER_HEADER     BufferListTail;

   //
   //	Count of available receive buffers in the free list.
   //
   ULONG					FreeReceiveBufferCount;
};

#define RECV_BUFFER_INFO_SIG           0x10101010

//							
//	Flag definitions for the RECV_BUFFER_INFO structure.
//
#define fRECV_INFO_COMPLETE_VC_ACTIVATION      0x00000001

//
//	Macros used for accessing the receive buffer flags
//
#define RECV_INFO_TEST_FLAG(x, f)      (((x)->Flags & (f)) == (f))
#define RECV_INFO_SET_FLAG(x, f)       ((x)->Flags |= (f))
#define RECV_INFO_CLEAR_FLAG(x, f)     ((x)->Flags &= ~(f))


//
// Define that what kind to slot type used in a VC.
//
#define RECV_SMALL_BUFFER      BIT(0)
#define RECV_BIG_BUFFER        BIT(1)


struct _RECV_SEG_INFO
{
   //
   //	Pointer to the owning adapter.
   //	
   PADAPTER_BLOCK			    pAdapter;
   
   //
   //	The size of the segment in ULONGs.
   //
   ULONG					    SegmentSize;
   
   //
   //	Pointer to the entry of Recv state table.
   //
   ULONG                       pEntryOfRecvState;

   TBATM155_RX_STATE_ENTRY     InitRecvState;

   //
   //	Queue of packets that are waiting for the receive complete
   //	interrupt.
   //
   RECV_BUFFER_QUEUE           SegCompleting;

   //
   //	This contains receive pools for buffers and packets.
   //
   PRECV_BUFFER_INFO		    pRecvBufferInfo;

   PRECV_BUFFER_QUEUE          FreeSmallBufQ;
   PRECV_BUFFER_QUEUE          FreeBigBufQ;

   NDIS_SPIN_LOCK			    lock;
};


//
// Number and size of allocated PadTrailer buffers;
//
#define TBATM155_MAX_PADTRAILER_BUFFERS    30


// **********************************************************************
// Allocation Map
//
// This structure holds the mapping parameters of a structure allocated
// in memory for transmission.
// **********************************************************************
typedef struct _ALLOCATION_MAP {

   PVOID                 AllocVa;       // virtual address of the memory block allocated to the structure
   NDIS_PHYSICAL_ADDRESS AllocPa;       // physical address of the memory block allocated to the structure
   ULONG                 AllocSize;     // size of the allocated block

   PVOID                 Va;            // virtual address of the aligned structure
   ULONG                 Pa;            // physical address of the aligned structure
   ULONG                 Size;          // size of the structure
   PNDIS_BUFFER          FlushBuffer;   // NDIS Flush Buffer

} ALLOCATION_MAP,*PALLOCATION_MAP;



//
//	Transmit Segmentation Information.
//
//		This is allocated for each segmentation channel.  This structure 
//     contains all of the information, e.g. buffers, size, etc....
//

typedef struct _XMIT_SEG_INFO
{
   PADAPTER_BLOCK          Adapter;                //	Pointer to the adapter.
   
   //
   //	Queue of packets that are waiting for segmentation resources.
   //
   PACKET_QUEUE            SegWait;

   //
   //	List of packets that are awaiting DMA completion interrupt per-VC.
   //
   PACKET_QUEUE            DmaCompleting;
   MAP_REGISTER_QUEUE      InUseMapRegisterQ;

   //
   //	Copy of the 155 PCI SAR transmit pending slot register. This is the
   //  initial set of the register.
   //
   TX_PENDING_SLOTS_CTL	InitXmitSlotDesc;
   TBATM155_TX_STATE_ENTRY InitXmitState;

   //
   //	Pointer to the entry of Tx state table.
   //
   ULONG                   pEntryOfXmitState;

   //
   //	Number of Bytes queued on the channel.
   //
   UINT                    XmitPduBytes;

   //
   //	Handle for the flush buffers associated with the pool.
   //
   NDIS_HANDLE             hFlushBufferPool;

   //
   //  Max Transmit Buffer Index;
   //
   //
   UCHAR                   PadTrailerBufferIndex;

   //
   //  Number of PadTrailerBurffer are free.
   //
   UCHAR                   FreePadTrailerBuffers;

   //
   //  Number of PadTrailerBurffer are current used or going to be used
   //  in the Dmawait queue.
   //
   UCHAR                   BeOrBeingUsedPadTrailerBufs;

   //
   //	Buffers for Padding+Trailer for AAL5 packets.
   //
   ALLOCATION_MAP          PadTrailerBuffers[TBATM155_MAX_PADTRAILER_BUFFERS];

   //
   //	Flags for the transmit segmentation channel.
   //
   ULONG                   flags;

   //
   //	Spin lock for this structure.
   //
   NDIS_SPIN_LOCK          lock;

   //
   //  These units are used as a record when the allocated memory needs to
   //  be free.
   //
   PVOID                   Va;
   NDIS_PHYSICAL_ADDRESS   Pa;
   ULONG                   Size;
   PTRANSMIT_CONTEXT       pTransmitContext;
   
}  
   XMIT_SEG_INFO,
   *PXMIT_SEG_INFO;


//
// minimum tx slot requied per packet
//
#define MIN_SLOTS_REQUIED_PER_PACKET   2


//
// Flags to indicate the usages of allocated shared memory in XmitSegInfo.
//
#define fXSC_XMIT_SEGMENT_RESERVED	0x00000001
#define fXSC_XMIT_SEGMENT_LOCKED	0x00000002
#define fXSC_XMIT_SEGMENT_INUSE    (fXSC_XMIT_SEGMENT_RESERVED + fXSC_XMIT_SEGMENT_LOCKED)

#define XSC_TEST_FLAG(_xsc, _f)	(((_xsc)->flags & (_f)) == (_f))
#define XSC_SET_FLAG(_xsc, _f)		((_xsc)->flags |= (_f))
#define XSC_CLEAR_FLAG(_xsc, _f)	((_xsc)->flags &= ~(_f))


#define XSC_LOCK_SEGMENT(_xsc, _L)                         \
{                                                          \
	if (XSC_TEST_FLAG((_xsc), fXSC_XMIT_SEGMENT_INUSE))     \
	{                                                       \
		(_L) = FALSE;                                       \
	}                                                       \
	else                                                    \
	{                                                       \
		(_L) = TRUE;                                        \
		XSC_SET_FLAG((_xsc), fXSC_XMIT_SEGMENT_RESERVED);	\
	}                                                       \
}

#define XSC_UNLOCK_SEGMENT(_xsc, _L)                       \
{                                                          \
	if ((_L))                                               \
	{                                                       \
       XSC_CLEAR_FLAG((_xsc), fXSC_XMIT_SEGMENT_INUSE);    \
	}														\
}


//
//	The following is the format of the reserved section of a send packet
//	that is awaiting segmentation.
// For packets sent onto the network by the miniport, the MiniportReserved
// in NDIS_PACKET, consists of 12 bytes.
//
typedef struct _PACKET_RESERVED
{
	//
	//	Used for queueing the packet.
	//
	PNDIS_PACKET	Next;

	//
	//	Amount of padding bytes are needed by the packet.
	//
	UCHAR			PaddingBytesNeeded;

	//
	//	This is the index of PadTrailerBuffer that was used for padding
   //  and trailer	in this AAL5 packet.
   //  The value 0 will be set for no Map Registers used.
	//
	UCHAR			NumOfUsedMapRegisters;

	//
	//	Number of DMA queue entries used by the packet.
	//
	UCHAR			PhysicalBufferCount;

	//
	//	This is the index of PadTrailerBuffer that was used for padding
   //  and trailer	in this AAL5 packet.
   //  The value 0xFF will be used for a non-AAL5 packet.
	//
	UCHAR			PadTrailerBufIndexInUse;

	//
	//	Vc the packet belongs on.
	//
	PVC_BLOCK		pVc;
}
	PACKET_RESERVED,
	*PPACKET_RESERVED;

#define PACKET_RESERVED_FROM_PACKET(_Pkt)		((PPACKET_RESERVED)((_Pkt)->MiniportReservedEx))


//
//	The following is the format of the reserved section of a receive packet.
// For packets received by the miniport and indicated up to higher level,
// the miniport can use only 8 bytes of this array.
//
typedef struct _RECV_PACKET_RESERVED
{
	PNDIS_PACKET		Next;
	PRECV_BUFFER_HEADER	RecvHeader;
}
	RECV_PACKET_RESERVED,
	*PRECV_PACKET_RESERVED;

#define RECV_PACKET_RESERVED_FROM_PACKET(_Pkt)		((PRECV_PACKET_RESERVED)((_Pkt)->MiniportReserved))

//
//	The following enumeration contains the possible registry parameters.
//
typedef enum _TBATM155_REGISTRY_ENTRY
{
   TbAtm155VcHashTableSize = 0,
   TbAtm155TotalRxBuffs,
   TbAtm155BigReceiveBufferSize,
   TbAtm155SmallReceiveBufferSize,
   TbAtm155NumberOfMapRegisters,
   TbAtm155MaxRegistryEntry
}
   TBATM155_REGISTRY_ENTRY;


//
//	The following structure is used to keep track of registry
//	parameters temporarily.
//
typedef struct	_TBATM155_REGISTRY_PARAMETER
{
   BOOLEAN fPresent;
   ULONG   Value;
}
   TBATM155_REGISTRY_PARAMETER,
   *PTBATM155_REGISTRY_PARAMETER;


typedef struct _HARDWARE_INFO
{
   //
   //	Flags information on the HARDWARE_INFO structure.
   //
   ULONG           Flags;
   NDIS_SPIN_LOCK  Lock;

   //
   //	Interrupt information.
   //
   ULONG                   InterruptLevel;
   ULONG                   InterruptVector;
   NDIS_MINIPORT_INTERRUPT Interrupt;
   ULONG                   InterruptMask;
   ULONG                   InterruptStatus;
   
   //
   //	I/O port information.
   //
   PVOID                   PortOffset;
   UINT                    InitialPort;
   ULONG                   NumberOfPorts;

   //
   //	Memory mapped I/O space information.
   //
   NDIS_PHYSICAL_ADDRESS   PhysicalIoSpace;
   PUCHAR                  VirtualIoSpace;
   ULONG                   IoSpaceLength;

   ULONG           CellClockRate;  //  Rate of the cell clock.  This is used
                                   //  in determining cell rate.
   
   ///
   //	The following are I/O space memory offsets and sizes.
   //	NOTE:
   //		The following offsets are from the PciFCode pointer.
   ///
   ULONG           Phy;            //  Mapped pointer to the PHY registers.

   UCHAR           LedVal;         //  Led control value

#if DBG_USING_LED
   UCHAR           dbgLedVal;      //  debugging Led value
#endif // end of DBG_USING_LED

   //
   // Use this variable to verify if SAR is xferring data for LED handling.
   //
   ULONG           PrevTxReportPa;         
   ULONG           PrevRxReportPa;         

	PTBATM155_SAR   TbAtm155_SAR;	//  Mapped pointer to the SAR registers.

	PUCHAR	        PciConfigSpace;	//  Mapped ptr to the PCI configuration space.

	PSAR_INFO	    SarInfo;        //	Pointer to the sar information.

	UCHAR	        fAdapterHw;     //  Adapter's configuration.
	USHORT	        NumOfVCs;       //  Number of VCs supported
	UINT	        MaxIdxOfRxReportQ;  //  Number of VCs supported
	UINT	        MaxIdxOfTxReportQ;  //  Number of VCs supported

   //
   //  Setup Some of common used SRAM tables
   //
	PVOID	        pSramAddrTbl;           // Pointer of SRAM address table.
	ULONG	        pSramRxAAL5SmallFsFIFo; // Point to Rx AAL5 Small FS List
	ULONG	        pSramRxAAL5BigFsFIFo;   // Point to Rx AAL5 Big FS List
	ULONG	        pSramTxVcStateTbl;      // Point to Tx VC State Descriptors
	ULONG	        pSramRxVcStateTbl;      // Point to Tx VC State Descriptors
	ULONG	        pSramAbrValueTbl;       // Point to ABR value table.
	ULONG	        pABR_Parameter_Tbl;     // Point to ABR parameter table.
	ULONG	        pSramCbrScheduleTbl_1;  // Point to CBR schedule table 1.
	ULONG	        pSramCbrScheduleTbl_2;  // Point to CBR schedule table 2.

	//
	//	address of the adapter.
	//
	UCHAR	        PermanentAddress[ATM_ADDRESS_LENGTH];
	UCHAR	        StationAddress[ATM_ADDRESS_LENGTH];

};

//
//	Macros for flag manipulation.
//
#define HW_TEST_FLAG(x, f)         (((x)->Flags & (f)) == (f))
#define HW_SET_FLAG(x, f)          ((x)->Flags |= (f))
#define HW_CLEAR_FLAG(x, f)        ((x)->Flags &= ~(f))


//
//	Flag definitions.
//
#define fHARDWARE_INFO_INTERRUPT_REGISTERED	0x00000001
#define fHARDWARE_INFO_INTERRUPTS_DISABLED		0x00000002

//
//	This structure contains statistics.
//
typedef struct _TBATM155_STATISTICS_INFO
{
   ULONG   XmitPdusOk;
   ULONG   XmitPdusError;
   ULONG   RecvPdusOk;
   ULONG   RecvPdusError;
   ULONG   RecvPdusNoBuffer;
   ULONG   RecvCrcError;
   ULONG   RecvCellsOk;
   ULONG   XmitCellsOk;
   ULONG   RecvCellsDropped;
   ULONG   RecvInvalidVpiVci;
   ULONG   RecvReassemblyErr;
}
   TBATM155_STATISTICS_INFO,
   *PTBATM155_STATISTICS_INFO;

#define MAX_NUM_RX_SLOT_QUEUES         3
#define MAX_NUM_TX_SLOT_QUEUES         1

#define TBATM155_MIN_QUEUE_ALIGNMENT	64

#define MIN_INDEX_TX_REPORT_QUEUE      0
#define MIN_INDEX_RX_REPORT_QUEUE      0


//
// This structure and definitions are for Tx report queue.
//
typedef    union   _TX_REPORT_QUEUE_ENTRY
{
   struct  
	{
	    ULONG	Own:1;                  // the least significant bit
       ULONG	reserved:3;
       ULONG	VC:12;
	}
       TxReportQDWord;

	ULONG	TxReportQueueEntry;
}
   TX_REPORT_QUEUE_ENTRY,
   *PTX_REPORT_QUEUE_ENTRY;


typedef	struct	_TX_REPORT_QUEUE
{
   PTX_REPORT_QUEUE_ENTRY  TxReportQptrVa;
	NDIS_PHYSICAL_ADDRESS   TxReportQptrPa;     // point to starting of Tx report queue.
   ALLOCATION_INFO;
}
   TX_REPORT_QUEUE,
   *PTX_REPORT_QUEUE;


//
// This structure is Rx report queue entry
//
typedef struct _RX_REPORT_QUEUE_ENTRY
{
   union
   {
	    struct
	    {
		    ULONG	Own:1;                  // the least significant bit
		    ULONG	Raw:1;
           ULONG   Index:12;               // Range from 0 to 4095
           ULONG   rqSlotType:2;
           ULONG   reserved2:16;
	    }
           RxReportQDWord0;

   	ULONG	RxReportQueueDword0;
   };

   union	
   {
	    struct                              // Rx AAL5 Report Queue Entry
	    {
		    ULONG	VC:12;
		    ULONG	Size:12;
		    ULONG	Efci:1;
		    ULONG	Clp:1;
		    ULONG	Status:3;
		    ULONG	Bad:1;
		    ULONG	Sop:1;
		    ULONG	Eop:1;
	    }
           RxReportQDWord1;

	    struct                              // Rx Raw Cell Report Queue Entry
	    {
		    ULONG	reserved:32;
	    };

   	ULONG	RxReportQueueDword1;
   };

}
   RX_REPORT_QUEUE_ENTRY,
   *PRX_REPORT_QUEUE_ENTRY;

//
// Definitions of bad status of receive report queue.
//
typedef enum _TBATM155_RX_REPORTQ_BAD_STATUS
{
       Crc32ErrDiscard = 0,
       LengthErrDiscard,
       AbortedPktDiscard,
       SlotCongDiscard,
       OtherCellErrDiscard,
       ReassemblyTimeout,
       PktTooLongErr,
       Reserved,
       TbAtm155TxReportQBadStatus
}
   TBATM155_TX_REPORTQ_BAD_STATUS;


typedef	struct	_RX_REPORT_QUEUE
{
   PRX_REPORT_QUEUE_ENTRY  RxReportQptrVa;
   NDIS_PHYSICAL_ADDRESS   RxReportQptrPa;     // point to starting of Rx report queue.

   ALLOCATION_INFO;
}
   RX_REPORT_QUEUE,
   *PRX_REPORT_QUEUE;


typedef struct _ADAPTER_BLOCK
{
#if DBG
   //
   //	Structure that contains debug log information.
   //
   struct _TBATM155_LOG_INFO   *DebugInfo;

   //
   //	contains packet size to detect if packet is not 48 byte boundary.
   //
   ULONG       CurrPacketSize;
   ULONG       PrevPacketSize;

#endif

   //
   //	Handle for use in calls into NDIS.
   //
   NDIS_HANDLE             MiniportAdapterHandle;

   ULONG                   References;
   NDIS_SPIN_LOCK          lock;

   //
   //	Flags describing the adapter state.
   //
   ULONG                   Flags;

   //
   //	Current hardware status.
   //
   NDIS_HARDWARE_STATUS        HardwareStatus;

   //
   //	Current link status.
   //
   NDIS_MEDIA_STATE            MediaConnectStatus;

   ///
   //	Contains hardware information.
   ///
   PHARDWARE_INFO              HardwareInfo;

   //
   //	Registry information.
   //
   PTBATM155_REGISTRY_PARAMETER    RegistryParameters;

   ///
   //	Adapter statistics.
   ///
   TBATM155_STATISTICS_INFO        StatInfo;

   //
   //	Interrupt Tx fatal error count.
   //
   ULONG                   TxFatalError;

   //
   //	PCI error interrupt.
   //
   ULONG                   PciErrorCount;

   //
   //	Tx Free Slot Underflow interrupt.
   //
   ULONG                   TxFreeSlotUnflCount;

   //
   //	Rx Free Slot Overflow interrupt.
   //
   ULONG                   RxFreeSlotOvflCount;

   ///
   //	List of the Vc's
   //
   //	We maintain 2 lists of VCs. Those that have been activated
   //	and thoes that are not.
   ///
   LIST_ENTRY              InactiveVcList;
   LIST_ENTRY              ActiveVcList;

   //
   //	The following are in cells/sec.
   //	
   ULONG                   MaximumBandwidth;
   ULONG                   MinimumCbrBandwidth;
   ULONG                   MinimumAbrBandwidth;
   ULONG                   RemainingTransmitBandwidth;

#if    AW_QOS

   //
   //	The Number of available CBR VCs
   //  9/17/97
   //      Should never limit the CBR Vc connection (theoretically).
   //      In order to pass NDIS tester with limited equipments,
   //      we use "patch" way to make it pass the tester.
   //      Later on, need to come back to find out the cause(s) of
   //      the failure "NDIS_STATUS_INCOMPATABLE QOS" when we get an
   //      ATM analyzer.
   //	
   USHORT                  NumOfAvailableCbrVc;
#endif // end of AW_QOS

   USHORT                  NumOfAvailVc_B;
   USHORT                  NumOfAvailVc_S;

   //
   //	The Number of entries are used in CBR schedule table
   //	
   ULONG                   TotalEntriesUsedonCBRs;

   //
   //	Transmit report queue information
   //
   TX_REPORT_QUEUE         TxReportQueue;    

   //
   //	Receive report queue information
   //
   RX_REPORT_QUEUE         RxReportQueue;    

   //
   //	Pointer to Map Register buffer
   //
   PMAP_REGISTER           pMapRegisters;

   //
   //   Spinlock for VcHashList below.
   //
   NDIS_SPIN_LOCK          VcHashLock;

   //
   //	The following cannot be moved!!!!
   //	This is the hash list of a given VCI to it's PVC_BLOCK
   //
   PVC_BLOCK               VcHashList[1];
};

//
//	Macros for adapter flag manipulation
//
#define ADAPTER_SET_FLAG(_adapter, _f)     (_adapter)->Flags |= (_f)
#define ADAPTER_CLEAR_FLAG(_adapter, _f)   (_adapter)->Flags &= ~(_f)
#define ADAPTER_TEST_FLAG(_adapter, _f)    (((_adapter)->Flags & (_f)) == (_f))

//
//	Flags for describing the Adapter state.
//
#define fADAPTER_INITIALIZING              0x00000001
#define fADAPTER_HARDWARE_FAILURE          0x00000002
#define fADAPTER_RESET_REQUESTED           0x00000004
#define fADAPTER_RESET_IN_PROGRESS         0x00000008
#define fADAPTER_PROCESSING_INTERRUPTS     0x00000010
#if TB_CHK4HANG
 #define fADAPTER_EXPECT_TXIOC             0x00000020
#endif // end of TB_CHK4HANG
#define fADAPTER_PROCESSING_SENDPACKETS    0x00000040
#define fADAPTER_SHUTTING_DOWN             0x80000000

typedef struct _VC_BLOCK
{
#if DBG
	//
	//	Structure that contains debug log information.
	//
	struct _TBATM155_LOG_INFO   *DebugInfo;
#endif

   LIST_ENTRY              Link;
		
   PVC_BLOCK               NextVcHash;     //  Pointer to the next VC in
                                           //  the hash list.
	
   PADAPTER_BLOCK          Adapter;
   PHARDWARE_INFO          HwInfo;
   NDIS_HANDLE             NdisVcHandle;
   PSAR_INFO               Sar;
   PXMIT_DMA_QUEUE         XmitDmaQ;
   PRECV_DMA_QUEUE         RecvDmaQ;

   ULONG                   References;     //  Number of outstanding references
                                           //  on this VC.

   ULONG                   PktsHoldsByNdis; //  The number of packets needed to be 
                                            //  returned from Ndis.

#if DBG
   ULONG                   SegWaitRef;
   ULONG                   SegCompRef;
   ULONG                   DmaWaitRef;
   ULONG                   DmaCompRef;

   ULONG                   RecvDmaCompRef;
#endif
	
   NDIS_SPIN_LOCK          lock;           //  Protection for this structure.
	
   ULONG                   Flags;          //  Flags describing vc state.

   //
   //	VC specific statistics.
   //
   TBATM155_STATISTICS_INFO    StatInfo;

   //
   //	ATM media parameters for this VC.
   //
   ULONG                   MediaFlags;

   ATM_VPIVCI              VpiVci;		//	VCI assigned to the VC.
   ATM_AAL_TYPE            AALType;	//	AAL type supported by this VC.

   //
   //	Rate information.
   //
   ATM_FLOW_PARAMETERS     Transmit;
   ATM_FLOW_PARAMETERS     Receive;

   //
   //	The following is the transmit buffer descriptor that is
   //	pre-built on VCI activation.
   //
   PXMIT_SEG_INFO          XmitSegInfo;
   PRECV_SEG_INFO          RecvSegInfo;

   //
   //  Based on the MaxSduSize, it will be set to either
   //  RECV_BIG_BUFFER or RECV_SMALL_BUFFER which indicate the type
   //  of slot being posted, "BIG" or "SMALL".
   //
   UCHAR                   RecvBufType;

   //
   //	CBR schedule table setup pre VC with CBR flow control.
   //
   ULONG                   CbrNumOfEntris;
   UCHAR                   CbrPreScaleVal;

   //
   //	Pointer to the call parameters.
   //	This is used for async completion of the VC activation.
   //
   PCO_CALL_PARAMETERS     CoCallParameters;
};

//
//	Macros for VC flag manipulation
//
#define VC_SET_FLAG(_vc, _f)       (_vc)->Flags |= (_f)
#define VC_CLEAR_FLAG(_vc, _f)     (_vc)->Flags &= ~(_f)
#define VC_TEST_FLAG(_vc, _f)      (((_vc)->Flags & (_f)) == (_f))

//
//	Flags describing VC state.
//
#define fVC_ACTIVE                 0x00000001
#define fVC_DEACTIVATING           0x00000002
#define fVC_TRANSMIT               0x00000004
#define fVC_RECEIVE                0x00000008
#define fVC_SET_CLP                0x00000010
#define fVC_RESET_IN_PROGRESS      0x00000020
#define fVC_INDICATE_END_OF_TX     0x00000040
#define fVC_ALLOCATING_TXBUF       0x00000100
#define fVC_ALLOCATING_RXBUF       0x00000200
#define fVC_MEM_OUT_OF_RESOURCES   0x00000400

#if DBG
 #define fVC_DBG_RXVC              0x02000000
 #define fVC_DBG_TXVC              0x04000000
#endif    // end of DBG

#define fVC_ERROR                  0x80000000


//
//
//
#define tbAtm155ReferenceAdapter(_adapter)     (_adapter)->References++
#define tbAtm155DereferenceAdapter(_adapter)   (_adapter)->References--

#if DBG

#define tbAtm155ReferenceVc(_vc)                                           \
{                                                                          \
   (_vc)->References++;                                                    \
   if ((_vc)->References > 150)                                            \
   {                                                                       \
       DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,                             \
        ("Reference %u Vc 0x%x\n", (_vc)->References, (_vc)->VpiVci.Vci)); \
   }                                                                       \
}

#define tbAtm155DereferenceVc(_vc)                                         \
{                                                                          \
   (_vc)->References--;                                                    \
   if ((_vc)->References > 150)                                            \
   {                                                                       \
       DBGPRINT(DBG_COMP_SEND, DBG_LEVEL_INFO,                             \
        ("Deeference %u Vc 0x%x\n", (_vc)->References, (_vc)->VpiVci.Vci)); \
   }                                                                       \
}

#else      // else of DBG

#define tbAtm155ReferenceVc(_vc)           (_vc)->References++
#define tbAtm155DereferenceVc(_vc)         (_vc)->References--

#endif


#endif // __SW_H
					


