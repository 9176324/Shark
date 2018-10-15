/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

	tfilter.h

Abstract:

	Header file for the address filtering library for NDIS MAC's.

Author:

	Anthony V. Ercolano (tonye) creation-date 3-Aug-1990

Environment:

	Runs in the context of a single MAC driver.

Notes:

	None.

Revision History:

	Adam Barr (adamba) 19-Mar-1991

		- Modified for Token-Ring

--*/

#ifndef _TR_FILTER_DEFS_
#define _TR_FILTER_DEFS_

#define TR_LENGTH_OF_FUNCTIONAL		4
#define TR_LENGTH_OF_ADDRESS		6


//
// Only the low 32 bits of the functional/group address
// are needed since the upper 16 bits is always c0-00.
//
typedef ULONG TR_FUNCTIONAL_ADDRESS;
typedef ULONG TR_GROUP_ADDRESS;


#define TR_IS_NOT_DIRECTED(_Address, _Result)								\
{																			\
	*(_Result) = (BOOLEAN)((_Address)[0] & 0x80);							\
}

#define TR_IS_FUNCTIONAL(_Address, _Result)									\
{																			\
	*(_Result) = (BOOLEAN)(((_Address)[0] & 0x80) &&						\
						  !((_Address)[2] & 0x80));							\
}

//
//
#define TR_IS_GROUP(_Address, _Result)										\
{																			\
	*(_Result) = (BOOLEAN)((_Address)[0] & (_Address)[2] & 0x80);			\
}

//
//
#define TR_IS_SOURCE_ROUTING(_Address, _Result)								\
{ 																			\
	*(_Result) = (BOOLEAN)((_Address)[0] & 0x80);							\
}

//
//	Check for NDIS_PACKET_TYPE_MAC_FRAME
//
#define TR_IS_MAC_FRAME(_PacketHeader)	((((PUCHAR)_PacketHeader)[1] & 0xFC) == 0)


//
// Check whether an address is broadcast. This is a little-endian check.
//
#define TR_IS_BROADCAST(_Address, _Result)										\
{																				\
	*(_Result) = (BOOLEAN)(((*(UNALIGNED USHORT *)&(_Address)[0] == 0xFFFF) ||	\
							(*(UNALIGNED USHORT *)&(_Address)[0] == 0x00C0)) && \
							(*(UNALIGNED ULONG  *)&(_Address)[2] == 0xFFFFFFFF));\
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
//  Result < 0 Implies the B address is greater.
//  Result > 0 Implies the A element is greater.
//  Result = 0 Implies equality.
//
// Note that this is an arbitrary ordering.  There is not
// defined relation on network addresses.  This is ad-hoc!
//
//
#define TR_COMPARE_NETWORK_ADDRESSES(_A, _B, _Result)			\
{																\
	if (*(ULONG UNALIGNED *)&(_A)[2] >							\
		*(ULONG UNALIGNED *)&(_B)[2])							\
	{															\
		*(_Result) = 1;											\
	}															\
	else if (*(ULONG UNALIGNED *)&(_A)[2] <						\
			 *(ULONG UNALIGNED *)&(_B)[2])						\
	{															\
		*(_Result) = (UINT)-1;									\
	}															\
	else if (*(USHORT UNALIGNED *)(_A) >						\
			 *(USHORT UNALIGNED *)(_B))							\
	{															\
		*(_Result) = 1;											\
	}															\
	else if (*(USHORT UNALIGNED *)(_A) <						\
			 *(USHORT UNALIGNED *)(_B))							\
	{															\
		*(_Result) = (UINT)-1;									\
	}															\
	else														\
	{															\
		*(_Result) = 0;											\
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
#define TR_COMPARE_NETWORK_ADDRESSES_EQ(_A, _B, _Result)					\
{																			\
	if ((*(ULONG UNALIGNED  *)&(_A)[2] == *(ULONG UNALIGNED  *)&(_B)[2]) &&	\
		(*(USHORT UNALIGNED *)&(_A)[0] == *(USHORT UNALIGNED *)&(_B)[0]))	\
	{																		\
		*(_Result) = 0;														\
	}																		\
	else																	\
	{																		\
		*(_Result) = 1;														\
	}																		\
}


//
// This macro is used to copy from one network address to
// another.
//
#define TR_COPY_NETWORK_ADDRESS(_D, _S)										\
{																			\
	*((ULONG UNALIGNED *)(_D)) = *((ULONG UNALIGNED *)(_S));				\
	*((USHORT UNALIGNED *)((UCHAR *)(_D)+4)) =								\
							*((USHORT UNALIGNED *)((UCHAR *)(_S)+4));		\
}


//
//UINT
//TR_QUERY_FILTER_CLASSES(
//	IN PTR_FILTER Filter
//	)
//
// This macro returns the currently enabled filter classes.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define TR_QUERY_FILTER_CLASSES(Filter) ((Filter)->CombinedPacketFilter)


//
//UINT
//TR_QUERY_PACKET_FILTER(
//	IN PTR_FILTER Filter,
//	IN NDIS_HANDLE NdisFilterHandle
//	)
//
// This macro returns the currently enabled filter classes for a specific
// open instance.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define TR_QUERY_PACKET_FILTER(Filter, NdisFilterHandle) \
		(((PTR_BINDING_INFO)NdisFilterHandle)->PacketFilters)



//
//UINT
//TR_QUERY_FILTER_ADDRESSES(
//	IN PTR_FILTER Filter
//	)
//
// This macro returns the currently enabled functional address.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define TR_QUERY_FILTER_ADDRESSES(Filter) ((Filter)->CombinedFunctionalAddress)



//
//UINT
//TR_QUERY_FILTER_GROUP(
//	IN PTR_FILTER Filter
//	)
//
// This macro returns the currently enabled Group address.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define TR_QUERY_FILTER_Group(Filter) ((Filter)->GroupAddress)
#define TR_QUERY_FILTER_GROUP(Filter) ((Filter)->GroupAddress)



//
//UINT
//TR_QUERY_FILTER_BINDING_ADDRESS(
//	IN PTR_FILTER Filter
//	IN NDIS_HANDLE NdisFilterHandle,
//	)
//
// This macro returns the currently desired functional addresses
// for the specified binding.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define TR_QUERY_FILTER_BINDING_ADDRESS(Filter, NdisFilterHandle) \
					(((PTR_BINDING_INFO)NdisFilterHandle)->FunctionalAddress)




//
//BOOLEAN
//TR_QUERY_FILTER_BINDING_GROUP(
//	IN PTR_FILTER Filter
//	IN NDIS_HANDLE NdisFilterHandle,
//	)
//
// This macro returns TRUE if the specified binding is using the
// current group address.
//
// NOTE: THIS MACRO ASSUMES THAT THE FILTER LOCK IS HELD.
//
#define TR_QUERY_FILTER_BINDING_GROUP(Filter, NdisFilterHandle) \
					(((PTR_BINDING_INFO)NdisFilterHandle)->UsingGroupAddress)


//
// An action routine type.  The routines are called
// when a filter type is set for the first time or
// no more bindings require a particular type of filter.
//
// NOTE: THIS ROUTINE SHOULD ASSUME THAT THE LOCK IS ACQUIRED.
//
typedef
NDIS_STATUS
(*TR_FILTER_CHANGE)(
	IN	UINT					OldFilterClasses,
	IN	UINT					NewFilterClasses,
	IN	NDIS_HANDLE				MacBindingHandle,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	BOOLEAN					Set
	);

//
// This action routine is called when the functional address
// for the card has changed. It is passed the old functional
// address as well as the new one.
//
// NOTE: THIS ROUTINE SHOULD ASSUME THAT THE LOCK IS ACQUIRED.
//
typedef
NDIS_STATUS
(*TR_ADDRESS_CHANGE)(
	IN	TR_FUNCTIONAL_ADDRESS	OldFunctionalAddress,
	IN	TR_FUNCTIONAL_ADDRESS	NewFunctionalAddress,
	IN	NDIS_HANDLE				MacBindingHandle,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	BOOLEAN					Set
	);


//
// This action routine is called when the group address
// for the card has changed. It is passed the old group
// address as well as the new one.
//
// NOTE: THIS ROUTINE SHOULD ASSUME THAT THE LOCK IS ACQUIRED.
//
typedef
NDIS_STATUS
(*TR_GROUP_CHANGE)(
	IN	TR_GROUP_ADDRESS		OldGroupAddress,
	IN	TR_GROUP_ADDRESS		NewGroupAddress,
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
(*TR_DEFERRED_CLOSE)(
	IN	NDIS_HANDLE				MacBindingHandle
	);


//
// The binding info is threaded on two lists.  When
// the binding is free it is on a single freelist.
//
// When the binding is being used it is on an index list
// and possibly on seperate broadcast and directed lists.
//
typedef struct _TR_BINDING_INFO
{
	NDIS_HANDLE					MacBindingHandle;
	NDIS_HANDLE					NdisBindingContext;
	UINT						PacketFilters;
	TR_FUNCTIONAL_ADDRESS		FunctionalAddress;
	struct _TR_BINDING_INFO *	NextOpen;
	ULONG						dummy1;
	UINT						References;
	BOOLEAN						UsingGroupAddress;
	BOOLEAN						ReceivedAPacket;

	//
	//	the following pointers are used to traverse the specific
	//	filter lists.
	//
	TR_FUNCTIONAL_ADDRESS		OldFunctionalAddress;
	UINT						OldPacketFilters;
	BOOLEAN						OldUsingGroupAddress;
	struct _TR_BINDING_INFO	*	NextDirected;
	struct _TR_BINDING_INFO *	NextBFG;

} TR_BINDING_INFO,*PTR_BINDING_INFO;

//
// An opaque type that contains a filter database.
// The MAC need not know how it is structured.
//
typedef struct _TR_FILTER
{

	//
	// Spin lock used to protect the filter from multiple accesses.
	//
	PNDIS_SPIN_LOCK			Lock;

	//
	// ORing together of all the FunctionalAddresses.
	//
	TR_FUNCTIONAL_ADDRESS	CombinedFunctionalAddress;

	//
	// Current group address in use.
	//
	TR_FUNCTIONAL_ADDRESS	GroupAddress;

	//
	// Reference count on group address;
	//
	UINT					GroupReferences;

	//
	// Combination of all the filters of all the open bindings.
	//
	UINT					CombinedPacketFilter;

	//
	// Pointer to list of current opens.
	//
	PTR_BINDING_INFO		OpenList;

	//
	// Address of the adapter associated with this filter.
	//
	UCHAR					AdapterAddress[TR_LENGTH_OF_ADDRESS];

	//
	// Action routines to be invoked on notable changes in the filter.
	//
	TR_ADDRESS_CHANGE		AddressChangeAction;
	TR_GROUP_CHANGE			GroupChangeAction;
	TR_FILTER_CHANGE		FilterChangeAction;
	TR_DEFERRED_CLOSE		CloseAction;

	//
	//	Filter specific list for optimization.
	//
	TR_FUNCTIONAL_ADDRESS	OldCombinedFunctionalAddress;
	TR_FUNCTIONAL_ADDRESS	OldGroupAddress;
	UINT					OldGroupReferences;
	UINT					OldCombinedPacketFilter;
	//
	// The list of bindings are seperated for directed and broadcast/multicast
	// Promiscuous bindings are on both lists
	//
	PTR_BINDING_INFO		DirectedList;	// List of bindings for directed packets
	PTR_BINDING_INFO		BFGList;		// List of bindings for broadcast/functional packets

	struct _NDIS_MINIPORT_BLOCK *Miniport;
#if	defined(NDIS_WRAPPER)
	UINT						NumOpens;
	NDIS_RW_LOCK				BindListLock;
#endif
} TR_FILTER, *PTR_FILTER;

//
// Exported functions
//
EXPORT
BOOLEAN
TrCreateFilter(
	IN	TR_ADDRESS_CHANGE		AddressChangeAction,
	IN	TR_GROUP_CHANGE			GroupChangeAction,
	IN	TR_FILTER_CHANGE		FilterChangeAction,
	IN	TR_DEFERRED_CLOSE		CloseAction,
	IN	PUCHAR					AdapterAddress,
	IN	PNDIS_SPIN_LOCK			Lock,
	OUT PTR_FILTER *			Filter
	);

EXPORT
VOID
TrDeleteFilter(
	IN	PTR_FILTER				Filter
	);

EXPORT
BOOLEAN
TrNoteFilterOpenAdapter(
	IN	PTR_FILTER				Filter,
	IN	NDIS_HANDLE				MacBindingHandle,
	IN	NDIS_HANDLE				NdisBindingContext,
	OUT PNDIS_HANDLE			NdisFilterHandle
	);

EXPORT
NDIS_STATUS
TrDeleteFilterOpenAdapter(
	IN	PTR_FILTER				Filter,
	IN	NDIS_HANDLE				NdisFilterHandle,
	IN	PNDIS_REQUEST			NdisRequest
	);

EXPORT
NDIS_STATUS
TrChangeFunctionalAddress(
	IN	PTR_FILTER				Filter,
	IN	NDIS_HANDLE				NdisFilterHandle,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	CHAR					FunctionalAddressArray[TR_LENGTH_OF_FUNCTIONAL],
	IN	BOOLEAN					Set
	);

EXPORT
NDIS_STATUS
TrChangeGroupAddress(
	IN	PTR_FILTER				Filter,
	IN	NDIS_HANDLE				NdisFilterHandle,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	CHAR					GroupAddressArray[TR_LENGTH_OF_FUNCTIONAL],
	IN	BOOLEAN					Set
	);

EXPORT
BOOLEAN
TrShouldAddressLoopBack(
	IN	PTR_FILTER				Filter,
	IN	CHAR					DestinationAddress[TR_LENGTH_OF_ADDRESS],
	IN	CHAR					SourceAddress[TR_LENGTH_OF_ADDRESS]
	);

EXPORT
NDIS_STATUS
TrFilterAdjust(
	IN	PTR_FILTER				Filter,
	IN	NDIS_HANDLE				NdisFilterHandle,
	IN	PNDIS_REQUEST			NdisRequest,
	IN	UINT					FilterClasses,
	IN	BOOLEAN					Set
	);

EXPORT
VOID
TrFilterIndicateReceive(
	IN	PTR_FILTER				Filter,
	IN	NDIS_HANDLE				MacReceiveContext,
	IN	PVOID					HeaderBuffer,
	IN	UINT					HeaderBufferSize,
	IN	PVOID					LookaheadBuffer,
	IN	UINT					LookaheadBufferSize,
	IN	UINT					PacketSize
	);

EXPORT
VOID
TrFilterIndicateReceiveComplete(
	IN	PTR_FILTER				Filter
	);

#endif // _TR_FILTER_DEFS_


