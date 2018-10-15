/*++

Copyright (c) 2000 Microsoft Corporation. All rights reserved.

   File:       support.h
 
               Developed for Toshiba by Elisa Research Inc., CA
               http://www.elisaresearch.com
               (510) 770-4920



	This routine contains support structures.

Author:

	A. Wang

Environment:

	Kernel mode

Revision History:

	01/07/97		awang		Initial of Toshiba ATM 155 Device Driver.
	01/29/97		awang		Added DBG and ORG_CODE flags.

--*/

#ifndef __SUPPORT_H
#define __SUPPORT_H


#if DBG
#define TB_DBG         1           // Debugging code.
#endif

#define DBG_USING_LED  0           // Debugging by using LED.

#define TB_CHK4HANG    1           // Handling to check if Xmit engine has hung.

//
//  9/17/97     - Please read the note where declares this
//                variable (in SW.H).
#define AW_QOS         0           // AW's debugging for incompatible QoS issue
#define ABR_CODE       0           // ABR feature enable flag.


//
// Macro for getting Packet info.
//
#define HDR_INDEX(_Packet) (ULONG_PTR)NDIS_PER_PACKET_INFO_FROM_PACKET(_Packet, HeaderIndexInfo)
#define HDR_INDEX_ADDR(_Packet) (PVOID)&(NDIS_PER_PACKET_INFO_FROM_PACKET(_Packet, HeaderIndexInfo))


typedef union _TBATM155_RATE_IN_FP
{
   struct
   {
       USHORT  mant:9;             // mantissa (the least significant bit)
       USHORT  exp:5;
       USHORT  nz:1;
       USHORT  reserved:1;
   };

   USHORT      reg;
}
   TBATM155_RATE_IN_FP,
   *PTBATM155_RATE_IN_FP;


//
// Due to 155 PCI SAR restraint, the following structures is used to
// manage map registers.
//
typedef struct	_MAP_REGISTER
{
   struct	_MAP_REGISTER   *Next;
   USHORT                  MapRegisterIndex;
}
   MAP_REGISTER,
   *PMAP_REGISTER;


typedef struct	_MAP_REGISTER_QUEUE
{
   PMAP_REGISTER       Head;
   PMAP_REGISTER       Tail;
   ULONG               References;
   NDIS_SPIN_LOCK      lock;
}
   MAP_REGISTER_QUEUE,
   *PMAP_REGISTER_QUEUE;


//
//	The following structure is used to manage packet queue's.
//
typedef struct _PACKET_QUEUE
{
   PNDIS_PACKET    Head;
   PNDIS_PACKET    Tail;
   ULONG           References;
   NDIS_SPIN_LOCK  lock;
}
   PACKET_QUEUE,
   *PPACKET_QUEUE;

#define PACKET_QUEUE_EMPTY(_pPacketQ)  ((BOOLEAN)((_pPacketQ)->Head == NULL))

#if	!DBG

/*++

VOID
InsertPacketAtTail(
   IN  PPACKET_QUEUE   PacketQ
   IN  PNDIS_PACKET    Packet
   )

Routine Description:

   This routine will insert a packet at the tail end of a packet queue.

Arguments:

   PacketQ -   Pointer to the structure representing the packet queue
               to add the packet to.
   Packet  -   Pointer to the packet to queue.

Return Value:

   None.

--*/

#define InsertPacketAtTail(_PacketQ, _Packet)                              \
{                                                                          \
   if (NULL == (_PacketQ)->Head)                                           \
   {                                                                       \
       (_PacketQ)->Head = (_Packet);                                       \
   }                                                                       \
   else                                                                    \
   {                                                                       \
       PACKET_RESERVED_FROM_PACKET((_PacketQ)->Tail)->Next = (_Packet);    \
   }                                                                       \
   (_PacketQ)->Tail = (_Packet);                                           \
   PACKET_RESERVED_FROM_PACKET((_Packet))->Next = NULL;                    \
   (_PacketQ)->References++;                                               \
}


/*++

VOID
InsertPacketAtTailDpc(
	IN	PPACKET_QUEUE	PacketQ
	IN	PNDIS_PACKET	Packet
	)

Routine Description:

	This routine will insert a packet at the tail end of a packet queue.

Arguments:

	PacketQ	-	Pointer to the structure representing the packet queue
				to add the packet to.
	Packet	-	Pointer to the packet to queue.

Return Value:

	None.

--*/

#define	InsertPacketAtTailDpc(_PacketQ, _Packet)						\
{																			\
	if (NULL == (_PacketQ)->Head)											\
	{																		\
		(_PacketQ)->Head = (_Packet);										\
	}																		\
	else																	\
	{																		\
		PACKET_RESERVED_FROM_PACKET((_PacketQ)->Tail)->Next = (_Packet);	\
	}																		\
	(_PacketQ)->Tail = (_Packet);											\
	PACKET_RESERVED_FROM_PACKET((_Packet))->Next = NULL;					\
	(_PacketQ)->References++;												\
}

/*++

VOID
RemovePacketFromHead(
	IN	PPACKET_QUEUE	PacketQ,
	OUT	PNDIS_PACKET	*Packet
	)

Routine Description:

	This routine will remove a packet from the head of the queue.

Arguments:

	PacketQ	-	Pointer to the structure representing the packet queue
				to add the packet to.
	Packet	-	Pointer to the place to store the packet removed.

Return Value:

--*/

#define RemovePacketFromHead(_PacketQ, _Packet)							\
{																			\
	*(_Packet) = (_PacketQ)->Head;											\
																			\
	if (NULL != *(_Packet))													\
	{																		\
		(_PacketQ)->Head = PACKET_RESERVED_FROM_PACKET(*(_Packet))->Next;	\
		(_PacketQ)->References--;											\
	}																		\
	PACKET_RESERVED_FROM_PACKET(*(_Packet))->Next = NULL;					\
}

/*++

VOID
RemovePacketFromHeadDpc(
	IN	PPACKET_QUEUE	PacketQ,
	OUT	PNDIS_PACKET	*Packet
	)

Routine Description:

	This routine will remove a packet from the head of the queue.

Arguments:

	PacketQ	-	Pointer to the structure representing the packet queue
				to add the packet to.
	Packet	-	Pointer to the place to store the packet removed.

Return Value:

--*/

#define RemovePacketFromHeadDpc(_PacketQ, _Packet)							\
{																			\
	*(_Packet) = (_PacketQ)->Head;											\
																			\
	if (NULL != *(_Packet))													\
	{																		\
		(_PacketQ)->Head = PACKET_RESERVED_FROM_PACKET(*(_Packet))->Next;	\
		(_PacketQ)->References--;											\
	}																		\
	PACKET_RESERVED_FROM_PACKET(*(_Packet))->Next = NULL;					\
}



/*++
VOID
InitializeMapRegisterQueue(
	IN	PMAP_REGISTER_QUEUE     MapRegistersQ,
	)

Routine Description:

	This routine will initialize a MAP_REGISTER queue structure for use.

Arguments:

	MapRegistersQ	-	Pointer to the MAP_REGISTER_QUEUE to initialize.

Return Value:

	None.

--*/
#define	InitializeMapRegisterQueue(_MapRegistersQ)      \
{                                                          \
   (_MapRegistersQ)->Head = NULL;                          \
   (_MapRegistersQ)->Tail = NULL;                          \
   (_MapRegistersQ)->References = 0;                       \
                                                           \
   NdisAllocateSpinLock(&(_MapRegistersQ)->lock);          \
}


/*++

VOID
InsertMapRegisterAtTail(
	IN	PMAP_REGISTER_QUEUE     MapRegistersQ,
	IN	PMAP_REGISTER	        MapRegister
	)


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

#define	InsertMapRegisterAtTail(_MapRegistersQ, _MapRegister)		\
{                                                                      \
	if (NULL == (_MapRegistersQ)->Head)                                 \
	{                                                                   \
		(_MapRegistersQ)->Head = (_MapRegister);                        \
	}                                                                   \
	else                                                                \
	{                                                                   \
		(_MapRegistersQ)->Tail->Next = (_MapRegister);                  \
	}                                                                   \
                                                                       \
	(_MapRegistersQ)->Tail = (_MapRegister);                            \
	(_MapRegister)->Next = NULL;                                        \
	(_MapRegistersQ)->References++;                                     \
}                                                                      


/*++

VOID
RemoveMapRegisterFromHead(
	IN	PMAP_REGISTER_QUEUE     MapRegistersQ,
	OUT	PMAP_REGISTER	        *MapRegister
	)

Routine Description:

	This routine will remove an index of map register from the head of
   the queue.

Arguments:

	MapRegistersQ   -   Pointer to the structure representing the index of
                       map register to add to.
	MapRegister     -	Pointer to the index of map register to queue.

Return Value:

--*/

#define    RemoveMapRegisterFromHead(_MapRegisterQ, _MapRegister)          \
{                                                                          \
   *(_MapRegister) = (_MapRegisterQ)->Head;                                \
   if (NULL != *(_MapRegister))                                            \
   {                                                                       \
       (_MapRegisterQ)->Head = (*(_MapRegister))->Next;                    \
       (*(_MapRegister))->Next = NULL;                                     \
   }                                                                       \
   (_MapRegisterQ)->References--;                                          \
}

#endif



#endif // __SUPPORT_H

