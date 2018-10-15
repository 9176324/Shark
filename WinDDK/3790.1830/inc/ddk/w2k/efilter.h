/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	efilter.h

Abstract:

	Header file for the address filtering library for NDIS MAC's.

Author:

	Anthony V. Ercolano (tonye) creation-date 3-Aug-1990

Environment:

	Runs in the context of a single MAC driver.

Notes:

	None.

Revision History:

	Adam Barr (adamba) 28-May-1991

		- renamed MacXXX to EthXXX, changed filter.h to efilter.h


--*/

#ifndef _ETH_FILTER_DEFS_
#define _ETH_FILTER_DEFS_

#define ETH_LENGTH_OF_ADDRESS 6


//
// ZZZ This is a little-endian specific check.
//
#define ETH_IS_MULTICAST(Address) \
	(BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x01))


//
// Check whether an address is broadcast.
//
#define ETH_IS_BROADCAST(Address) 				\
	((((PUCHAR)(Address))[0] == ((UCHAR)0xff)) && (((PUCHAR)(Address))[1] == ((UCHAR)0xff)))


//
// This macro will compare network addresses.
//
//  A - Is a network address.
//
//  B - Is a network address.
//
//  Result - The result of comparing two network address.
//
//  Result < 0 Implies the B address is greater.
//  Result > 0 Implies the A element is greater.
//  Result = 0 Implies equality.
//
// Note that this is an arbitrary ordering.  There is not
// defined relation on network addresses.  This is ad-hoc!
//
//
#define ETH_COMPARE_NETWORK_ADDRESSES(_A, _B, _Result)			\
{																\
	if (*(ULONG UNALIGNED *)&(_A)[2] >							\
		 *(ULONG UNALIGNED *)&(_B)[2])							\
	{															\
		*(_Result) = 1;										 	\
	}															\
	else if (*(ULONG UNALIGNED *)&(_A)[2] <					 	\
				*(ULONG UNALIGNED *)&(_B)[2])					\
	{															\
		*(_Result) = (UINT)-1;									\
	}															\
	else if (*(USHORT UNALIGNED *)(_A) >						\
				*(USHORT UNALIGNED *)(_B))						\
	{															\
		*(_Result) = 1;										 	\
	}															\
	else if (*(USHORT UNALIGNED *)(_A) <						\
				*(USHORT UNALIGNED *)(_B))						\
	{															\
		*(_Result) = (UINT)-1;									\
	}															\
	else														\
	{															\
		*(_Result) = 0;										 	\
	}															\
}

//
// This macro will compare network addresses.
//
//  A - Is a network address.
//
//  B - Is a network address.
//
//  Result - The result of comparing two network address.
//
//  Result != 0 Implies inequality.
//  Result == 0 Implies equality.
//
//
#define ETH_COMPARE_NETWORK_ADDRESSES_EQ(_A,_B, _Result)		\
{																\
	if ((*(ULONG UNALIGNED *)&(_A)[2] ==						\
			*(ULONG UNALIGNED *)&(_B)[2]) &&					\
		 (*(USHORT UNALIGNED *)(_A) ==							\
			*(USHORT UNALIGNED *)(_B)))							\
	{															\
		*(_Result) = 0;											\
	}															\
	else														\
	{															\
		*(_Result) = 1;											\
	}															\
}


//
// This macro is used to copy from one network address to
// another.
//
#define ETH_COPY_NETWORK_ADDRESS(_D, _S) \
{ \
	*((ULONG UNALIGNED *)(_D)) = *((ULONG UNALIGNED *)(_S)); \
	*((USHORT UNALIGNED *)((UCHAR *)(_D)+4)) = *((USHORT UNALIGNED *)((UCHAR *)(_S)+4)); \
}


//
//UINT
//ETH_QUERY_FILTER_CLASSES(
//	IN	PETH_FILTER				Filter
//	)
//
// This macro returns the currently enabled filter classes.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define ETH_QUERY_FILTER_CLASSES(Filter) ((Filter)->CombinedPacketFilter)


//
//UINT
//ETH_QUERY_PACKET_FILTER(
//	IN	PETH_FILTER				Filter,
//	IN	NDIS_HANDLE				NdisFilterHandle
//	)
//
// This macro returns the currently enabled filter classes for a specific
// open instance.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define ETH_QUERY_PACKET_FILTER(Filter, NdisFilterHandle) \
		(((PETH_BINDING_INFO)(NdisFilterHandle))->PacketFilters)


//
//UINT
//ETH_NUMBER_OF_GLOBAL_FILTER_ADDRESSES(
//	IN	PETH_FILTER				Filter
//	)
//
// This macro returns the number of multicast addresses in the
// multicast address list.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define ETH_NUMBER_OF_GLOBAL_FILTER_ADDRESSES(Filter) ((Filter)->NumAddresses)


//
// An action routine type.  The routines are called
// when a filter type is set for the first time or
// no more bindings require a particular type of filter.
//
// NOTE: THIS ROUTINE SHOULD ASSUME THAT THE LOCK IS ACQUIRED.
//
typedef
NDIS_STATUS
(*ETH_FILTER_CHANGE)(
	IN	UINT					OldFilterClasses,
	IN	UINT					NewFilterClasses,
	IN	NDIS_HANDLE				MacBindingHandle,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	BOOLEAN					Set
	);

//
// This action routine is called when a new multicast address
// list is given to the filter. The action routine is given
// arrays containing the old and new multicast addresses.
//
// NOTE: THIS ROUTINE SHOULD ASSUME THAT THE LOCK IS ACQUIRED.
//
typedef
NDIS_STATUS
(*ETH_ADDRESS_CHANGE)(
	IN	UINT					OldAddressCount,
	IN	CHAR					OldAddresses[][ETH_LENGTH_OF_ADDRESS],
	IN	UINT					NewAddressCount,
	IN	CHAR					NewAddresses[][ETH_LENGTH_OF_ADDRESS],
	IN	NDIS_HANDLE				MacBindingHandle,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	BOOLEAN					Set
	);

//
// This action routine is called when the mac requests a close for
// a particular binding *WHILE THE BINDING IS BEING INDICATED TO
// THE PROTOCOL*.  The filtering package can't get rid of the open
// right away.  So this routine will be called as soon as the
// NdisIndicateReceive returns.
//
// NOTE: THIS ROUTINE SHOULD ASSUME THAT THE LOCK IS ACQUIRED.
//
typedef
VOID
(*ETH_DEFERRED_CLOSE)(
	IN	NDIS_HANDLE				MacBindingHandle
	);

//
// The binding info is threaded on two lists.  When
// the binding is free it is on a single freelist.
//
// When the binding is being used it is on an index list
// and possibly on seperate broadcast and directed lists.
//
typedef struct _ETH_BINDING_INFO
{
	NDIS_HANDLE					MacBindingHandle;
	NDIS_HANDLE					NdisBindingContext;
	UINT						PacketFilters;
	ULONG						References;
	BOOLEAN						ReceivedAPacket;

	struct _ETH_BINDING_INFO *	NextOpen;

	//
	// THE FIELDS ABOVE ARE ACCESSED BY MACROS. DO NOT DELETE ANY FIELDS ABOVE
	// AND ADD FIELDS BELOW THIS LINE.
	//

	//
	// Pointer to an array of 6 character arrays holding the
	// multicast addresses requested for filtering.
	//
	CHAR						(*MCastAddressBuf)[ETH_LENGTH_OF_ADDRESS];

	//
	// The current number of addresses in the MCastAddressBuf.
	//
	UINT						NumAddresses;

	//
	// Save area while the change is made
	//
	CHAR						(*OldMCastAddressBuf)[ETH_LENGTH_OF_ADDRESS];
	UINT						OldNumAddresses;
	UINT						OldPacketFilters;

	//
	//  The following pointers are used to travers the specific
	//  filter lists.
	//
	struct _ETH_BINDING_INFO *	NextDirected;
	struct _ETH_BINDING_INFO *	NextBM;

} ETH_BINDING_INFO,*PETH_BINDING_INFO;

//
// An opaque type that contains a filter database.
// The MAC need not know how it is structured.
//
typedef struct _ETH_FILTER
{
	//
	// Spin lock used to protect the filter from multiple accesses.
	//
	PNDIS_SPIN_LOCK				Lock;

	//
	// Pointer to an array of 6 character arrays holding the
	// multicast addresses requested for filtering.
	//
	CHAR						(*MCastAddressBuf)[ETH_LENGTH_OF_ADDRESS];

	struct _NDIS_MINIPORT_BLOCK *Miniport;

	//
	// Combination of all the filters of all the open bindings.
	//
	UINT						CombinedPacketFilter;

	//
	// Pointer for traversing the open list.
	//
	PETH_BINDING_INFO			OpenList;

	//
	// Action routines to be invoked on notable changes in the filter.
	//

	ETH_ADDRESS_CHANGE			AddressChangeAction;
	ETH_FILTER_CHANGE			FilterChangeAction;
	ETH_DEFERRED_CLOSE			CloseAction;

	//
	// The maximum number of addresses used for filtering.
	//
	UINT						MaxMulticastAddresses;

	//
	// The current number of addresses in the address filter.
	//
	UINT						NumAddresses;

	//
	// THE FIELDS ABOVE ARE ACCESSED BY MACROS. DO NOT DELETE ANY FIELDS ABOVE
	// AND ADD FIELDS BELOW THIS LINE.
	//

	//
	// Address of the adapter.
	//
	UCHAR						AdapterAddress[ETH_LENGTH_OF_ADDRESS];

	//
	// Save area while the change is made
	//
	UINT						OldCombinedPacketFilter;
	CHAR						(*OldMCastAddressBuf)[ETH_LENGTH_OF_ADDRESS];
	UINT						OldNumAddresses;

	//
	// The list of bindings are seperated for directed and broadcast/multicast
	// Promiscuous bindings are on both lists
	//
   	// List of bindings for directed packets
	//
	PETH_BINDING_INFO			DirectedList;

	//
	// List of bindings for broadcast/multicast packets
	//
	PETH_BINDING_INFO			BMList;

	//
	// This is used duing multicast list set only.
	//
	PETH_BINDING_INFO			MCastSet;

#if	defined(NDIS_WRAPPER)
	UINT						NumOpens;
	NDIS_RW_LOCK				BindListLock;
#endif
} ETH_FILTER,*PETH_FILTER;

//
// Exported functions
//
EXPORT
BOOLEAN
EthCreateFilter(
	IN	UINT					MaximumMulticastAddresses,
	IN	ETH_ADDRESS_CHANGE		AddressChangeAction,
	IN	ETH_FILTER_CHANGE		FilterChangeAction,
	IN	ETH_DEFERRED_CLOSE		CloseAction,
	IN	PUCHAR					AdapterAddress,
	IN	PNDIS_SPIN_LOCK			Lock,
	OUT PETH_FILTER *			Filter
	);

EXPORT
VOID
EthDeleteFilter(
	IN	PETH_FILTER				Filter
	);

EXPORT
BOOLEAN
EthNoteFilterOpenAdapter(
	IN	PETH_FILTER				Filter,
	IN	NDIS_HANDLE				MacBindingHandle,
	IN	NDIS_HANDLE				NdisBindingContext,
	OUT PNDIS_HANDLE			NdisFilterHandle
	);

EXPORT
NDIS_STATUS
EthDeleteFilterOpenAdapter(
	IN	PETH_FILTER				Filter,
	IN	NDIS_HANDLE				NdisFilterHandle,
	IN	PNDIS_REQUEST			NdisRequest
	);

EXPORT
NDIS_STATUS
EthChangeFilterAddresses(
	IN	PETH_FILTER				Filter,
	IN	NDIS_HANDLE				NdisFilterHandle,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	UINT					AddressCount,
	IN	CHAR					Addresses[][ETH_LENGTH_OF_ADDRESS],
	IN	BOOLEAN					Set
	);

EXPORT
BOOLEAN
EthShouldAddressLoopBack(
	IN	PETH_FILTER				Filter,
	IN	CHAR					Address[ETH_LENGTH_OF_ADDRESS]
	);

EXPORT
NDIS_STATUS
EthFilterAdjust(
	IN	PETH_FILTER				Filter,
	IN	NDIS_HANDLE				NdisFilterHandle,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	UINT					FilterClasses,
	IN	BOOLEAN					Set
	);

EXPORT
UINT
EthNumberOfOpenFilterAddresses(
	IN	PETH_FILTER				Filter,
	IN	NDIS_HANDLE				NdisFilterHandle
	);

// EXPORT
// UINT
// EthNumberOfOpenFilterAddresses(
// 	IN	PETH_FILTER				Filter,
// 	IN	NDIS_HANDLE				NdisFilterHandle
// 	);

EXPORT
VOID
EthQueryGlobalFilterAddresses(
	OUT PNDIS_STATUS			Status,
	IN	PETH_FILTER				Filter,
	IN	UINT					SizeOfArray,
	OUT PUINT					NumberOfAddresses,
	IN	OUT CHAR				AddressArray[][ETH_LENGTH_OF_ADDRESS]
	);

EXPORT
VOID
EthQueryOpenFilterAddresses(
	OUT PNDIS_STATUS			Status,
	IN	PETH_FILTER				Filter,
	IN	NDIS_HANDLE				NdisFilterHandle,
	IN	UINT					SizeOfArray,
	OUT PUINT					NumberOfAddresses,
	IN	OUT CHAR				AddressArray[][ETH_LENGTH_OF_ADDRESS]
	);


EXPORT
VOID
EthFilterIndicateReceive(
	IN	PETH_FILTER				Filter,
	IN	NDIS_HANDLE				MacReceiveContext,
	IN	PCHAR					Address,
	IN	PVOID					HeaderBuffer,
	IN	UINT					HeaderBufferSize,
	IN	PVOID					LookaheadBuffer,
	IN	UINT 					LookaheadBufferSize,
	IN	UINT					PacketSize
	);

EXPORT
VOID
EthFilterIndicateReceiveComplete(
	IN	PETH_FILTER				Filter
	);

#endif // _ETH_FILTER_DEFS_


